using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net.NetworkInformation;
using System.Threading; // スレッド
using SOEM; // EtherCAT

namespace EasyTest
{
    public partial class FormMain : Form
    {
        // 通信スレッド
        Thread threadEtherCAT;
        // 通信中フラグ
        bool isConnected = false;
        // ロック用オブジェクト
        object m_lockobj = new object();
        // サーボの角度指令値
        int servo1_val = 90;
        int servo2_val = 90;

        // 初期化
        public FormMain()
        {
            InitializeComponent();

            // サーボの角度指令値の初期値をUIに反映
            trackBar1.Value = servo1_val;
            trackBar2.Value = servo2_val;

            // UIの表示切替え
            buttonDisconnect.Enabled = false;

            // ネットワークインターフェースの列挙
            updateNicList();

        }

        // Updateボタン
        private void buttonUpdate_Click(object sender, EventArgs e)
        {
            // ネットワークインターフェースの列挙
            updateNicList();
        }

        // ネットワークインターフェースの列挙
        void updateNicList()
        {
            // コンボボックスのクリア
            comboNicName.Items.Clear();

            // 前回選択したネットワークインターフェース名を取得
            string nicName = Properties.Settings.Default.NicName;
            // 全てのネットワークインターフェースを取得
            var interfaces = NetworkInterface.GetAllNetworkInterfaces();
            // 取得したネットワークインターフェースをコンボボックスに追加する
            bool selected = false;
            foreach (var ni in interfaces)
            {
                var nic = new NicInfo(ni.Name, ni.Id);
                comboNicName.Items.Add(nic);

                // 前回選択したネットワークインターフェースがあれば選択
                if (nic.Name.Equals(nicName))
                {
                    comboNicName.SelectedItem = nic;
                    selected = true;
                }
            }
            if (!selected && (interfaces.Length > 0))
            {
                comboNicName.SelectedIndex = 0;
            }
        }

        // Connectボタン
        private void buttonConnect_Click(object sender, EventArgs e)
        {
            try
            {
                // 接続処理
                int result = EtherCAT.open(@"\Device\NPF_" + ((NicInfo)comboNicName.SelectedItem).ID);
                if(result == EtherCAT.FAILED)
                {
                    MessageBox.Show("ネットワークインターフェースが開けません", 
                        "エラー", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                result = EtherCAT.config();
                if(result == EtherCAT.NO_SLAVES_FOUND)
                {
                    MessageBox.Show("スレーブデバイスが見つかりません",
                        "エラー", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                if(result == EtherCAT.NOT_ALL_OP_STATE)
                {
                    MessageBox.Show("OPERATIONAL状態に移行できないデバイスがあります",
                        "エラー", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                // 設定の保存
                Properties.Settings.Default.NicName = comboNicName.Text;
                Properties.Settings.Default.Save();

                // 通信中フラグをセット
                isConnected = true;

                // UIの表示切替え
                buttonConnect.Enabled = false;
                buttonDisconnect.Enabled = true;
                buttonUpdate.Enabled = false;
                comboNicName.Enabled = false;

                // シリアル通信スレッドを起動
                threadEtherCAT = new Thread(new ThreadStart(threadFunc_EtherCAT));
                threadEtherCAT.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "エラー", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }

        // Disconnectボタン
        private void buttonDisconnect_Click(object sender, EventArgs e)
        {
            // シリアル通信中フラグをクリア
            isConnected = false;
        }


        // EtherCAT通信スレッド処理
        void threadFunc_EtherCAT()
        {
            while (true)
            {
                // サーボの角度指令値を出力バッファに設定
                lock (m_lockobj)
                {
                    EtherCAT.setOutPDO(1, 0, (byte)servo1_val);
                    EtherCAT.setOutPDO(2, 0, (byte)servo2_val);
                }
                // EtherCATのPDO転送
                EtherCAT.transferPDO();

                // ボリュームの値を入力バッファから取得
                int[] vol = new int[2];
                for(int i = 0; i < 2; i++)
                {
                    byte vol_h = EtherCAT.getInputPDO(1+i, 0);
                    byte vol_l = EtherCAT.getInputPDO(1+i, 1);
                    vol[i] = ((int)vol_h << 8) | (int)vol_l;
                }
                // UIに反映
                this.BeginInvoke((Action)(() =>
                {
                    joystickBar1.Value = vol[0];
                    joystickBar2.Value = vol[1];
                }));

                // 停止判定
                if (!isConnected)
                {
                    break;
                }

                // 20msecスリープ
                Thread.Sleep(10);

            }// while(true)

            // 通信終了処理
            this.BeginInvoke((Action)(() =>
            {
                // 切断処理
                EtherCAT.requestState(EtherCAT.ALL_SLAVES, EtherCAT.STATE_INIT);
                EtherCAT.close();

                // UIの表示切替え
                buttonConnect.Enabled = true;
                buttonDisconnect.Enabled = false;
                buttonUpdate.Enabled = true;
                comboNicName.Enabled = true;
            }));
        }

        // フォームが閉じる前にスレッドの後始末を確認
        private void FormMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (isConnected)
            {
                isConnected = false;
            }
            if (threadEtherCAT != null)
            {
                threadEtherCAT.Join();
            }
        }

        // トラックバー1(サーボ1に対応)が変更されたとき
        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            lock (m_lockobj)
            {
                servo1_val = trackBar1.Value;
            }
        }

        // トラックバー2(サーボ2に対応)が変更されたとき
        private void trackBar2_Scroll(object sender, EventArgs e)
        {
            lock (m_lockobj)
            {
                servo2_val = trackBar2.Value;
            }
        }
    }
}
