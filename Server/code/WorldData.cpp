#include "WorldData.h"
#include "UserManager.h"
#include "CommandData2.h"

namespace app
{
	void sendErrInfo(net::ITcpServer* ts, s32 connectid, u16 cmd, u16 childcmd)
	{
	/*	ts->begin(connectid, cmd);
		ts->sss(connectid, childcmd);
		ts->end(connectid);*/
	}
	//���������Ч��
	net::S_CLIENT_BASE* checkUserConnect(const u32 connectid, const s64 memid, const u16 cmd, const u16 childcmd)
	{
		//1����֤��һ�������
		return nullptr;

	}



}