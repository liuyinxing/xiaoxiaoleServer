#ifndef ____COMMANDDATA_CHAT_H
#define ____COMMANDDATA_CHAT_H

#include "CommandData.h"


namespace app
{

#pragma pack(push, packing)
#pragma pack(1)

	//900 玩家进入
	struct S_CMD_USER_ENTRY :S_COMMAND_GATEBASE
	{
		this_constructor(S_CMD_USER_ENTRY)
		u8   line;//所在线路
		u32  userindex;
		s32  mapid;//当前所在地图ID
		u16  level;//等级
		char name[USER_MAX_MEMBER];   //账号 
		char nick[USER_MAX_MEMBER];
	};
	//redis服务器上进入游戏保存的数据结构
	struct S_REDIS_USR_ENTEY :S_CMD_USER_ENTRY
	{
		u32  server_connectid;//服务器上的连接ID
		u32  server_clientid; //服务器上的客户端ID
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