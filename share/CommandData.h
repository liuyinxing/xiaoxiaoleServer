#ifndef ____COMMANDDATA_H
#define ____COMMANDDATA_H

#include "UserData.h"


#define CMD_10      10 //�˺ŵ�¼ ��֤ 
#define CMD_20      20 //�˺�ע��
#define CMD_30      30 //���µ�¼ʱ��
#define CMD_40      40 //������������
#define CMD_80      80 //����redis
#define CMD_90      90 //����

#define CMD_100     100 //��¼��Ϸ ��ȡ��ɫ����
#define CMD_200     200 //ƥ��ɹ�  ��ʼ��Ϸ
#define CMD_300     300 //������ɫ
#define CMD_400     400 //ɾ����ɫ



namespace app
{

#pragma pack(push, packing)
#pragma pack(1)

	//������������
	struct S_COMMAND_BASE
	{
		u32  server_connectid;//�������ϵ�����ID
		u32  server_clientid; //�������ϵĿͻ���ID
		u32  user_connectid;  //���ط������ϵ� ����ID
		s64  memid;//�˺�id
	};
	struct S_COMMAND_GATEBASE
	{
		u32  user_connectid;  //���ط������ϵ� ����ID
		s64  memid;//�˺�id
	};


	//����
	struct S_CHAT_DATA :S_COMMAND_BASE
	{
		this_constructor(S_CHAT_DATA)
		u32 userindex;
		char nick[20];
		char text[50];
	};


#pragma pack(pop, packing)
}
#endif