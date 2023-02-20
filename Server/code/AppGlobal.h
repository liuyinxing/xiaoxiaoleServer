#ifndef  ____APPGLOBAL_H
#define  ____APPGLOBAL_H


#include "INetBase.h"
#include "IContainer.h"
#include <time.h>
#include <map>
#include "UserData.h"

namespace app
{
	extern net::ITcpServer* __TcpServer;


	//ÊÂ¼þAPI
	extern void  onClientAccept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void  onClientSecureConnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void  onClientDisconnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void  onClientTimeout(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);
	extern void  onClientExcept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code);


}



#endif