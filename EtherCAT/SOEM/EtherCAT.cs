using System;
using System.Text;
using System.Runtime.InteropServices; // DllImport

namespace SOEM
{
    // EtherCATマスター (SOEMの.NETラッパー)
    public class EtherCAT
    {
        // コンフィグ結果の定数
        public const int ALL_SLAVES_OP_STATE = 0; // 全てのスレーブがOP状態になりました (成功)
        public const int NO_SLAVES_FOUND = 1; // スレーブがみつかりません
        public const int NOT_ALL_OP_STATE = 2; // OP状態にならないスレーブがあります

        // スレーブ状態の定数
        public const int STATE_NONE = 0x00; // 有効な状態でない
        public const int STATE_INIT = 0x01; // Init状態
        public const int STATE_PRE_OP = 0x02; // Pre-operational状態
        public const int STATE_BOOT = 0x03; // Boot state状態
        public const int STATE_SAFE_OP = 0x04; // Safe-operational状態
        public const int STATE_OPERATIONAL = 0x08; // Operational状態
        public const int STATE_ACK = 0x10; // Error or ACK 状態
        public const int STATE_ERROR = 0x10; // Error or ACK 状態

        // 全スレーブを指定する場合は0
        public const int ALL_SLAVES = 0;

        // 成否の定数
        public const int SUCCESS = 1;
        public const int FAILED = 0;

        // 開く
        // nif: ネットワークインターフェースID
        // return: 結果 FAILED / SUCCESS
        [DllImport("soemlib.dll", EntryPoint = "soem_open")]
        extern public static int open(string nif);

        // 閉じる
        [DllImport("soemlib.dll", EntryPoint = "soem_close")]
        extern public static void close();

        // コンフィグする
        // return 結果 ALL_SLAVES_OP_STATE / NO_SLAVES_FOUND / NOT_ALL_OP_STATE
        [DllImport("soemlib.dll", EntryPoint = "soem_config")]
        extern public static int config();


        // スレーブの数を取得
        // return: スレーブの数
        [DllImport("soemlib.dll", EntryPoint = "soem_getSlaveCount")]
        extern public static int getSlaveCount();


        // スレーブの状態を更新
        // return: 全スレーブの中で最も低い状態
        [DllImport("soemlib.dll", EntryPoint = "soem_updateState")]
        extern public static int updateState();

        // スレーブの状態を取得
        // slave: スレーブのインクリメンタルアドレス
        // return: 状態
        [DllImport("soemlib.dll", EntryPoint = "soem_getState")]
        extern public static int getState(int slave);

        // スレーブのALステータスコードを取得
        // slave: スレーブのインクリメンタルアドレス
        // return: ALステータスコード
        [DllImport("soemlib.dll", EntryPoint = "soem_getALStatusCode")]
        extern public static int getALStatusCode(int slave);

        // スレーブのALステータスの説明を取得
        // slave: スレーブのインクリメンタルアドレス
        // desc: ALステータスの説明
        [DllImport("soemlib.dll", EntryPoint = "soem_getALStatusDesc")]
        extern public static void getALStatusDesc(int slave, StringBuilder desc);

        // スレーブの状態変更を要求
        [DllImport("soemlib.dll", EntryPoint = "soem_requestState")]
        extern public static void requestState(int slave, int state);

        // スレーブの名前を取得
        // slave: スレーブのインクリメンタルアドレス
        // name: スレーブの名前
        [DllImport("soemlib.dll", EntryPoint = "soem_getName")]
        extern public static void getName(int slave, StringBuilder name);

        // スレーブのベンダ番号/製品番号/バージョン番号を取得
        // slave: スレーブのインクリメンタルアドレス
        // id: {ベンダ番号, 製品番号, バージョン番号}
        [DllImport("soemlib.dll", EntryPoint = "soem_getId")]
        extern public static void getId(int slave, ulong[] id);

        // PDO転送する
        // return:  0=失敗 / 1=成功
        [DllImport("soemlib.dll", EntryPoint = "soem_transferPDO")]
        extern public static int transferPDO();

        // 汎用スレーブPDO入力
        // slave: スレーブのインクリメンタルアドレス
        // offset: オフセットアドレス
        // return: 入力値
        [DllImport("soemlib.dll", EntryPoint = "soem_getInputPDO")]
        extern public static byte getInputPDO(int slave, int offset);

        // 汎用スレーブPDO出力
        // slave: スレーブのインクリメンタルアドレス
        // offset: オフセットアドレス
        // value: 出力値
        [DllImport("soemlib.dll", EntryPoint = "soem_setOutputPDO")]
        extern public static void setOutputPDO(int slave, int offset, byte value);

        // ネットワークアダプタの検索
        // return: 見つかったネットワークアダプタの数
        [DllImport("soemlib.dll", EntryPoint = "soem_findAdapters")]
        extern public static int findAdapters();

        // ネットワークアダプタ名の取得
        // index: ネットワークアダプタの番号(見つかった順)
        // name: ネットワークアダプタの名前
        [DllImport("soemlib.dll", EntryPoint = "soem_getAdapterName")]
        extern public static void getAdapterName(int index, string name);

        // ネットワークアダプタの説明の取得
        // index: ネットワークアダプタの番号(見つかった順)
        // desc: ネットワークアダプタの説明
        [DllImport("soemlib.dll", EntryPoint = "soem_getAdapterDesc")]
        extern public static void getAdapterDesc(int index, string desc);
    }
}
