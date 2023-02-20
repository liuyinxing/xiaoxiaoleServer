#ifndef ____COMMANDDATA_H
#define ____COMMANDDATA_H

#include "UserData.h"


#define CMD_10      10 //账号登录 验证 
#define CMD_20      20 //账号注册
#define CMD_30      30 //更新登录时间
#define CMD_40      40 //更新网关人数
#define CMD_80      80 //测试redis
#define CMD_90      90 //心跳

#define CMD_100     100 //登录游戏 获取角色数据
#define CMD_200     200 //匹配成功  开始游戏
#define CMD_300     300 //创建角色
#define CMD_400     400 //删除角色



namespace app
{

#pragma pack(push, packing)
#pragma pack(1)

	//基础查找数据
	struct S_COMMAND_BASE
	{
		u32  server_connectid;//服务器上的连接ID
		u32  server_clientid; //服务器上的客户端ID
		u32  user_connectid;  //网关服务器上的 连接ID
		s64  memid;//账号id
	};
	struct S_COMMAND_GATEBASE
	{
		u32  user_connectid;  //网关服务器上的 连接ID
		s64  memid;//账号id
	};


	//聊天
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