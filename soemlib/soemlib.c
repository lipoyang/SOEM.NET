#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#include "ethercat.h"

// IO�f�[�^�o�b�t�@
static char IOmap[4096];
// ���҂����WKC�l
static int expectedWKC;
// �l�b�g���[�N�A�_�v�^�̃��X�g
static ec_adaptert * adapters = NULL;

// �J��
// nif: �l�b�g���[�N�C���^�[�t�F�[�XID
// return: 0=���s / 1=����
int __stdcall soem_open(char* nif)
{
	int ret = ec_init(nif);
	return ret;
}

// ����
void __stdcall soem_close(void)
{
	ec_close();
}

#define ALL_SLAVES_OP_STATE	0	// �S�ẴX���[�u��OP��ԂɂȂ�܂��� (����)
#define NO_SLAVES_FOUND		1	// �X���[�u���݂���܂���
#define NOT_ALL_OP_STATE	2	// OP��ԂɂȂ�Ȃ��X���[�u������܂�

// �R���t�B�O����
// return ����
int __stdcall soem_config(void)
{
	int oloop, iloop, chk;
	
	// �X���[�u�������A�����R���t�B�O����
	if ( ec_config_init(FALSE) > 0 )
	{
		printf("%d slaves found and configured.\n",ec_slavecount);
		
		ec_config_map(&IOmap);
		ec_configdc();
		
		printf("Slaves mapped, state to SAFE_OP.\n");
		
		// �S�ẴX���[�u�� SAFE_OP ��ԂɒB����̂�҂�
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
		
		// �S�ẴX���[�u��OP��Ԃ�v��
		ec_slave[0].state = EC_STATE_OPERATIONAL;
		/* send one valid process data to make outputs in slaves happy*/ // ���Ӗ��s��
		ec_send_processdata();
		ec_receive_processdata(EC_TIMEOUTRET);
		ec_writestate(0);
		chk = 40;
		// �S�ẴX���[�u��OP��ԂɒB����̂�҂�
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

// �X���[�u�̐����擾
// return: �X���[�u�̐�
int __stdcall soem_getSlaveCount(void)
{
	return ec_slavecount;
}

// �X���[�u�̏�Ԃ��X�V
// return: �S�X���[�u�̒��ōł��Ⴂ���
int __stdcall soem_updateState(void)
{
	int ret = ec_readstate();
	return ret;
}

// �X���[�u�̏�Ԃ��擾
// slave: �X���[�u�̃C���N�������^���A�h���X
// return: ���
int __stdcall soem_getState(int slave)
{
	return ec_slave[slave].state;
}

// �X���[�u��AL�X�e�[�^�X�R�[�h���擾
// slave: �X���[�u�̃C���N�������^���A�h���X
// return: AL�X�e�[�^�X�R�[�h
int __stdcall soem_getALStatusCode(int slave)
{
	return ec_slave[slave].ALstatuscode;
}

// �X���[�u��AL�X�e�[�^�X�̐������擾
// slave: �X���[�u�̃C���N�������^���A�h���X
// desc: AL�X�e�[�^�X�̐��� (�ő�31����)
void __stdcall soem_getALStatusDesc(int slave, char* desc)
{
	snprintf(desc, 31, "%s", ec_ALstatuscode2string( ec_slave[slave].ALstatuscode ));
}

// �X���[�u�̏�ԕύX��v��
void __stdcall soem_requestState(int slave, int state)
{
	ec_slave[slave].state = state;
	ec_writestate(slave);
}

// �X���[�u�̖��O���擾
// slave: �X���[�u�̃C���N�������^���A�h���X
// name: �X���[�u�̖��O (�ő�31����)
void __stdcall soem_getName(int slave, char* name)
{
	snprintf(name, 31, "%s", ec_slave[slave].name );
}

// �X���[�u�̃x���_�ԍ�/���i�ԍ�/�o�[�W�����ԍ����擾
// slave: �X���[�u�̃C���N�������^���A�h���X
// id: {�x���_�ԍ�, ���i�ԍ�, �o�[�W�����ԍ�}
void __stdcall soem_getId(int slave, unsigned long* id)
{
	id[0] = ec_slave[slave].eep_man;
	id[1] = ec_slave[slave].eep_id;
	id[2] = ec_slave[slave].eep_rev;
}

// PDO�]������
// return:  0=���s / 1=����
int __stdcall soem_transferPDO(void)
{
	ec_send_processdata();
	int wkc = ec_receive_processdata(EC_TIMEOUTRET);

	if(wkc >= expectedWKC){
		return 1;
	}else{
		return 0;
	}
}

// �ėp�X���[�uPDO����
// slave: �X���[�u�̃C���N�������^���A�h���X
// offset: �I�t�Z�b�g�A�h���X
// return: ���͒l
uint8_t __stdcall soem_getInputPDO(int slave, int offset)
{
	uint8_t ret = 0;
	
	if(slave <= ec_slavecount)
	{
		ret = ec_slave[slave].inputs[offset];
	}
	return ret;
}

// �ėp�X���[�uPDO�o��
// slave: �X���[�u�̃C���N�������^���A�h���X
// offset: �I�t�Z�b�g�A�h���X
// value: �o�͒l
void __stdcall soem_setOutputPDO(int slave, int offset, uint8_t value)
{
	if(slave <= ec_slavecount)
	{
		ec_slave[slave].outputs[offset] = value;
	}
}

// �l�b�g���[�N�A�_�v�^�̌���
// return: ���������l�b�g���[�N�A�_�v�^�̐�
int __stdcall soem_findAdapters(void)
{
	adapters = ec_find_adapters();
	
	int num = 0;
	ec_adaptert * adapter = adapters;
	while (adapter != NULL)
	{
		adapter = adapter->next;
		num++;
	}
	return num;
}

// �l�b�g���[�N�A�_�v�^���̎擾
// index: �l�b�g���[�N�A�_�v�^�̔ԍ�(����������)
// name: �l�b�g���[�N�A�_�v�^�̖��O
void __stdcall soem_getAdapterName(int index, char* name)
{
	int num = 0;
	ec_adaptert * adapter = adapters;
	while (adapter != NULL)
	{
		if(num == index){
			strcpy(name, adapter->name);
			return;
		}
		adapter = adapter->next;
		num++;
	}
	name[0] = '\0';
}

// �l�b�g���[�N�A�_�v�^�̐����̎擾
// index: �l�b�g���[�N�A�_�v�^�̔ԍ�(����������)
// desc: �l�b�g���[�N�A�_�v�^�̐���
void __stdcall soem_getAdapterDesc(int index, char* desc)
{
	int num = 0;
	ec_adaptert * adapter = adapters;
	while (adapter != NULL)
	{
		if(num == index){
			strcpy(desc, adapter->desc);
			return;
		}
		adapter = adapter->next;
		num++;
	}
	desc[0] = '\0';
}

