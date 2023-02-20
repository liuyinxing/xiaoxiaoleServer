#ifndef ____COMMANDDATA_CHAT_H
#define ____COMMANDDATA_CHAT_H

#include "CommandData.h"


namespace app
{

#pragma pack(push, packing)
#pragma pack(1)

	//900 ��ҽ���
	struct S_CMD_USER_ENTRY :S_COMMAND_GATEBASE
	{
		this_constructor(S_CMD_USER_ENTRY)
		u8   line;//������·
		u32  userindex;
		s32  mapid;//��ǰ���ڵ�ͼID
		u16  level;//�ȼ�
		char name[USER_MAX_MEMBER];   //�˺� 
		char nick[USER_MAX_MEMBER];
	};
	//redis�������Ͻ�����Ϸ��������ݽṹ
	struct S_REDIS_USR_ENTEY :S_CMD_USER_ENTRY
	{
		u32  server_connectid;//�������ϵ�����ID
		u32  server_clientid; //�������ϵĿͻ���ID
	};

	struct S_ADD_FRIEND :S_COMMAND_BASE
	{
		this_constructor(S_ADD_FRIEND)
		u32  userindex;
		u64  targetmemid;
		char nick[USER_MAX_NICK];
	};
	

	struct S_CMD_COPY :S_COMMAND_GATEBASE
	{
		this_constructor(S_CMD_COPY)
		u32  userindex;
		s32  mapid;
		s32  roomindex;
		s32  teamindex;
	};

#pragma pack(pop, packing)
}
#endif