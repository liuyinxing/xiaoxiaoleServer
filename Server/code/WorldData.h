#ifndef  ____WORLDDATA_H
#define  ____WORLDDATA_H

#include "UserData.h"
#include "INetBase.h"

namespace app
{


	extern void sendErrInfo(net::ITcpServer* ts, s32 connectid, u16 cmd, u16 childcmd);
	//检查连接有效性
	extern net::S_CLIENT_BASE* checkUserConnect(const u32 connectid, const s64 memid, const u16 cmd, const u16 childcmd);



};

#endif