#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#include "ethercat.h"

// IOデータバッファ
static char IOmap[4096];
// 期待されるWKC値
static int expectedWKC;

// 開く
// nif: ネットワークインターフェースID
// return: 0=失敗 / 1=成功
__declspec(dllexport) int __stdcall soem_open(char* nif)
{
	int ret = ec_init(nif);
	return ret;
}

// 閉じる
__declspec(dllexport) void __stdcall soem_close(void)
{
	ec_close();
}

#define ALL_SLAVES_OP_STATE	0	// 全てのスレーブがOP状態になりました (成功)
#define NO_SLAVES_FOUND		1	// スレーブがみつかりません
#define NOT_ALL_OP_STATE	2	// OP状態にならないスレーブがあります

// コンフィグする
// return 結果
__declspec(dllexport) int __stdcall soem_config(void)
{
	int oloop, iloop, chk;
	
	// スレーブを見つけ、自動コンフィグする
	if ( ec_config_init(FALSE) > 0 )
	{
		printf("%d slaves found and configured.\n",ec_slavecount);
		
		ec_config_map(&IOmap);
		ec_configdc();
		
		printf("Slaves mapped, state to SAFE_OP.\n");
		
		// 全てのスレーブが SAFE_OP 状態に達するのを待つ
		ec_statecheck(0, EC_STATE_SAFE_OP,	EC_TIMEOUTSTATE * 4);
		oloop = ec_slave[0].Obytes;
		if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
		if (oloop > 8) oloop = 8;
		iloop = ec_slave[0].Ibytes;
		if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
		if (iloop > 8) iloop = 8;
		printf("segments : %d : %d %d %d %d\n",
			ec_group[0].nsegments,
			ec_group[0].IOsegment[0],
			ec_group[0].IOsegment[1],
			ec_group[0].IOsegment[2],
			ec_group[0].IOsegment[3]);
		printf("Request operational state for all slaves\n");
		expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
		printf("Calculated workcounter %d\n", expectedWKC);
		
		// 全てのスレーブにOP状態を要求
		ec_slave[0].state = EC_STATE_OPERATIONAL;
		/* send one valid process data to make outputs in slaves happy*/ // ←意味不明
		ec_send_processdata();
		ec_receive_processdata(EC_TIMEOUTRET);
		ec_writestate(0);
		chk = 40;
		// 全てのスレーブがOP状態に達するのを待つ
		do
		{
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
		}
		while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
		
		if (ec_slave[0].state == EC_STATE_OPERATIONAL )
		{
			return ALL_SLAVES_OP_STATE;
		}
		else
		{
			return NOT_ALL_OP_STATE;
		}
	}
	else
	{
		return NO_SLAVES_FOUND;
	}
}

// スレーブの数を取得
// return: スレーブの数
__declspec(dllexport) int __stdcall soem_getSlaveCount(void)
{
	return ec_slavecount;
}

// スレーブの状態を更新
// return: 全スレーブの中で最も低い状態
__declspec(dllexport) int __stdcall soem_updateState(void)
{
	int ret = ec_readstate();
	return ret;
}

// スレーブの状態を取得
// slave: スレーブのインクリメンタルアドレス
// return: 状態
__declspec(dllexport) int __stdcall soem_getState(int slave)
{
	return ec_slave[slave].state;
}

// スレーブのALステータスコードを取得
// slave: スレーブのインクリメンタルアドレス
// return: ALステータスコード
__declspec(dllexport) int __stdcall soem_getALStatusCode(int slave)
{
	return ec_slave[slave].ALstatuscode;
}

// スレーブのALステータスの説明を取得
// slave: スレーブのインクリメンタルアドレス
// desc: ALステータスの説明 (最大31文字)
__declspec(dllexport) void __stdcall soem_getALStatusDesc(int slave, char* desc)
{
	snprintf(desc, 31, "%s", ec_ALstatuscode2string( ec_slave[slave].ALstatuscode ));
}

// スレーブの状態変更を要求
__declspec(dllexport) void __stdcall soem_requestState(int slave, int state)
{
	ec_slave[slave].state = state;
	ec_writestate(slave);
}

// スレーブの名前を取得
// slave: スレーブのインクリメンタルアドレス
// name: スレーブの名前 (最大31文字)
__declspec(dllexport) void __stdcall soem_getName(int slave, char* name)
{
	snprintf(name, 31, "%s", ec_slave[slave].name );
}

// スレーブのベンダ番号/製品番号/バージョン番号を取得
// slave: スレーブのインクリメンタルアドレス
// id: {ベンダ番号, 製品番号, バージョン番号}
__declspec(dllexport) void __stdcall soem_getId(int slave, unsigned long* id)
{
	id[0] = ec_slave[slave].eep_man;
	id[1] = ec_slave[slave].eep_id;
	id[2] = ec_slave[slave].eep_rev;
}

// PDO転送する
// return:  0=失敗 / 1=成功
__declspec(dllexport) int __stdcall soem_transferPDO(void)
{
	ec_send_processdata();
	int wkc = ec_receive_processdata(EC_TIMEOUTRET);

	if(wkc >= expectedWKC){
		return 1;
	}else{
		return 0;
	}
}

// 汎用スレーブPDO入力
// slave: スレーブのインクリメンタルアドレス
// offset: オフセットアドレス
// return: 入力値
__declspec(dllexport) uint8_t __stdcall soem_getInputPDO(int slave, int offset)
{
	uint8_t ret = 0;
	
	if(slave <= ec_slavecount)
	{
		ret = ec_slave[slave].inputs[offset];
	}
	return ret;
}

// 汎用スレーブPDO出力
// slave: スレーブのインクリメンタルアドレス
// offset: オフセットアドレス
// value: 出力値
__declspec(dllexport) void __stdcall soem_setOutPDO(int slave, int offset, uint8_t value)
{
	if(slave <= ec_slavecount)
	{
		ec_slave[slave].outputs[offset] = value;
	}
}

