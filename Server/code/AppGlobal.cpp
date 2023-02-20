#include "AppGlobal.h"
#include <string>
#include "ShareFunction.h"
#include "LogFile.h"
#include "CommandData.h"
#include "UserManager.h"
namespace app
{
	net::ITcpServer* __TcpServer = nullptr;

	//工作线程
	void onClientAccept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)   return;

		LOG_MSG("new connect...%d [%s-%d] %d\n", (int)c->socketfd, c->ip, c->port,c->theadID);
	}
	//主线程
	void onClientSecureConnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)   return;
		std::string type;
		share::serverTypeToString(c->clientID, type);

		c->temp_LoginTime = 0;

		int aa = 0;
		int bb = 0;
		tcp->getSecurityCount(aa, bb);
		func::setConsoleColor(10);
		LOG_MSG("security connet...%d [%s:%d][connect:%d-%d] [clientid:%d-%s]\n",
			(int)c->socketfd, c->ip, c->port, aa, bb, c->clientID, type.c_str());
		func::setConsoleColor(7);

		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);

		io::pushLog(io::EFT_ERR, "security connet...[%d:%d] [%s:%d][connect:%d-%d] [clientid:%d-%s] [memory:%.2f/%.2f]\n",
			(int)c->socketfd,c->ID, c->ip, c->port, aa, bb, c->clientID, type.c_str(), curmemory1, maxmemory1);
		c->temp_LoginTime = time(NULL);
	}
	//主线程
	void onClientDisconnect(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)   return;
	
		int aa = 0;
		int bb = 0;
		tcp->getSecurityCount(aa, bb);
		const char* str1 = func::getShutDownError(c->shutdown_kind);
		const char* str2 = func::getCloseSocketError(code);

		std::string type;
		share::serverTypeToString(c->clientID, type);

		func::setConsoleColor(8);
		LOG_MSG("disconnect-%d [%s::%s][con:%d-%d] [clientid:%d-%s] err:%d\n", 
			(int)c->socketfd, str1, str2, aa, bb, c->clientID, type.c_str(),code);
		func::setConsoleColor(7);

		u32 curmemory = 0;
		u32 maxmemory = 0;
		share::updateMemory(curmemory, maxmemory);
		f32 curmemory1 = (f32)curmemory / (1024 * 1024);
		f32 maxmemory1 = (f32)maxmemory / (1024 * 1024);

		

#ifdef TEST_UDP_SERVER
		//清理UDP连接
		auto udp = __IUdpServer->findClient(c->ID);
		if (udp != NULL)
		{
			udp->memid = 0;
			__IUdpServer->clearConnect(udp);

			func::setConsoleColor(8);
			LOG_MSG("udp tcp_disconnect %d\n", c->ID);
			func::setConsoleColor(7);
		}
#endif

		if (c->state == func::S_Connect)
		{
			c->Reset();
		}
		else if (c->state == func::S_ConnectSecure)
		{
			if (c->memid > 0)
			{

				//auto user = __UserManager->findUser(c->ID);
				//if (user != NULL) user->reset();

				io::pushLog(io::EFT_ERR, "disconnect-%d [%s::%s][con:%d-%d] [clientid:%d-%s] [memory:%.4f/%.4f] err:%d %lld\n",
					(int)c->socketfd, str1, str2, aa, bb, c->clientID, type.c_str(), curmemory1, maxmemory1, code,c->memid);

			}
			LOG_MSG("AppGlobal secure leave...%d-%d \n", c->ID, (int)c->socketfd);
			c->Reset();
		}
		else if (c->state == func::S_Login)
		{
			auto user = __UserManager->findUser(c->ID);
			if (user != NULL)
			{
				io::pushLog(io::EFT_ERR, "disconnect-%d [%s::%s][con:%d-%d] [clientid:%d-%s] [memory:%.4f/%.4f] err:%d %lld %s\n",
					(int)c->socketfd, str1, str2, aa, bb, c->clientID, type.c_str(), curmemory1, maxmemory1, code, c->memid,user->name);
				//user->reset();
			}
			
			LOG_MSG("AppGlobal login leave...%d-%d \n", c->ID, (int)c->socketfd);
			c->Reset();
		}

	}
	//主线程
	void onClientTimeout(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)   return;
	}
	//工作线程或者主线程
	void onClientExcept(net::ITcpServer* tcp, net::S_CLIENT_BASE* c, const s32 code)
	{
		if (c == nullptr || tcp == nullptr)   return;
	}


	//客户端
	////////////////
    //工作线程
	void onConnect(net::ITcpClient* tcp, const s32 code)
	{
		//LOG_MSG("-------client connect...%d %d\n", code, tcp->getData()->ID);
	}
	//主线程
	void onSecureConnect(net::ITcpClient* tcp, const s32 code)
	{
		std::string type;
		share::serverTypeToString(tcp->getData()->serverID, type);

		func::setConsoleColor(13);
		LOG_MSG("------client security...[%s-%d] [serverid:%d-%s] \n",
			tcp->getData()->ip, tcp->getData()->port, tcp->getData()->serverID, type.c_str());
		func::setConsoleColor(7);

		io::pushLog(io::EFT_ERR, "------client security...[%s-%d] [serverid:%d-%s] \n",
			tcp->getData()->ip, tcp->getData()->port, tcp->getData()->serverID, type.c_str());

		//连接的聊天服务器 则需要把全部链接玩家信息推送给聊天服务器
		u32 server_type = share::serverIDToType(tcp->getData()->ID);
		switch (server_type)
		{
		case func::S_TYPE_CENTER:
			break;
		}
		
	}
	//工作线程OR主线程
	void onDisconnect(net::ITcpClient* tcp, const s32 code)
	{
		std::string type;
		share::serverTypeToString(tcp->getData()->serverID, type);

		func::setConsoleColor(8);
		LOG_MSG("------client disconnect...%d [%s-%d] [serverid:%d-%s] \n",
			code, tcp->getData()->ip, tcp->getData()->port, tcp->getData()->serverID, type.c_str());
		func::setConsoleColor(7);

		io::pushLog(io::EFT_ERR, "------client disconnect...%d [%s-%d] [serverid:%d-%s] \n",
			code, tcp->getData()->ip, tcp->getData()->port, tcp->getData()->serverID, type.c_str());

		u8 stype = share::serverIDToType(tcp->getData()->serverID);
		if (__UserManager != nullptr) __UserManager->setTcpClientDisconnect(stype);
	}
	//主线程
	void onExcept(net::ITcpClient* tcp, const s32 code)
	{
		LOG_MSG("-------client onOnExcept...%d\n", code);
	}


#ifdef TEST_UDP_SERVER
	net::IUdpServer* __IUdpServer = nullptr;
	//维护新的连接
	void onUdpToClientAccept(net::IUdpServer* udp, net::S_UDP_BASE* c, const s32 code, const char* err)
	{
		char s[30];
		func::formatTime(time(NULL), s);

		func::setConsoleColor(10);
		LOG_MSG("udp connect %s...id:%d [%s:%d] \n", s, c->ID, c->strip, c->port);
		func::setConsoleColor(7);
	}
	//用户失去连接
	//1 心跳包判断玩家掉线
	//2 10054 
	//3 客户端显示调用65003 离开
	void onUdpToClientDisconnect(net::IUdpServer* udp, net::S_UDP_BASE* c, const s32 code, const char* err)
	{
		//char s[30];
		//func::formatTime(time(NULL), s);

		//func::setConsoleColor(8);
		////LOG_MSG("udp disconnect %s...id:%d [%s:%d] err:%d/%s\n", s, c->ID, c->strip, c->port, code, err);
		//func::setConsoleColor(7);

		//onPlayerLeave(udp, c);
		//udp->clearConnect(c);
	}
#endif

}

