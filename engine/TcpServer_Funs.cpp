#include "TcpServer.h"
#ifdef ____WIN32_
#include "IOPool.h"
#include "sha1/sha1.h"
#include "base.h"
#include "SDK/protobuf/socket.pb.h"

using namespace net;
using namespace func;

//业务层指令集合
std::vector<IContainer*> __Commands;
std::vector<IContainer*> __TimerContainer;
//******************************************************************
void  TcpServer::initCommands()
{
	__Commands.reserve(MAX_COMMAND_LEN);
	for (int i = 0; i < MAX_COMMAND_LEN; i++)
	{
		__Commands.emplace_back(nullptr);
	}
}
//注册
void  TcpServer::registerCommand(int cmd, void* container)
{
	if (cmd >= MAX_COMMAND_LEN) return;
	IContainer * icon = (IContainer*)container;
	if (icon == nullptr) return;
	__Commands[cmd] = icon;
}
//注册定时器使用容器
void  TcpServer::registerTimerContainer(void* container)
{
	IContainer* icon = (IContainer*)container;
	if (icon == nullptr) return;
	auto it = __TimerContainer.begin();
	while (it!=__TimerContainer.end())
	{
		if (*it == icon) return;
	}
	__TimerContainer.emplace_back(icon);
}

//******************************************************************

s32 net::TcpServer::onTimerMessage(void* tc, s32 sendBytes)
{

	DataContext* tm_context = (DataContext*)tc;
	auto it = __TimerContainer.begin();
	while (it == __TimerContainer.end());
	{
		(*it)->onTimerMessage(tc, sendBytes);
		++it;
	}
	return 0;
}

//主线程下 解析命令
void TcpServer::parseCommand()
{
	for (s32 i = 0; i < Linkers->length; i++)
	{
		auto c = Linkers->Value(i);
		if (c->ID == -1) continue;
		if (c->state == func::S_Free) continue;
		if (c->state >= func::S_NeedSave) continue;
		checkConnect(c);
		if (c->closeState == func::S_CLOSE_SHUTDOWN) continue;
		parseCommand(c);
		this->postSend(c);
	}
}
void net::TcpServer::getSecurityCount(int& connum, int& securtiynum)
{
	connum = m_ConnectCount;
	securtiynum = m_SecurityCount;
}
//消费者 解析命令
void  TcpServer::parseCommand(S_CLIENT_BASE* c)
{
	if (!c->is_RecvCompleted) return;

	while (c->recv_Tail - c->recv_Head > 7)
	{
		//1、解析头
		char head[2];
		head[0] = c->recvBuf[c->recv_Head] ^ c->rCode[0];
		head[1] = c->recvBuf[c->recv_Head + 1] ^ c->rCode[1];

		if (head[0] != __ServerInfo->Head[0] || head[1] != __ServerInfo->Head[1])
		{
			shutDown(c->socketfd, 0, c, 2001);
			return;
		}

		s32 length = (*(u32*)(c->recvBuf + c->recv_Head + 2)) ^ c->rCode[0];
		u16 newMainID = (*(u16*)(c->recvBuf + c->recv_Head + 6)) ^ c->rCode[0];
		u16 newHandleCode = (*(u16*)(c->recvBuf + c->recv_Head + 8)) ^ c->rCode[1];
		//2、长度不够 需要继续等待 
		if (c->recv_Tail < c->recv_Head + length) break;

		c->recv_TempHead = c->recv_Head + 10;
		c->recv_TempTail = c->recv_Head + length;

		parseCommand(c, newMainID, newHandleCode,length-10);

		if (c->state <= func::S_Connect)
		{
			LOG_MSG("clinet已经reset....\n");
			return;
		}
		//4、增加读取长度
		c->recv_Head += length;

	}

	c->is_RecvCompleted = false;
}
	void  net::TcpServer::ParseProtoBuffData(S_CLIENT_BASE* c,::google::protobuf::MessageLite* messageLite, size_t len)
	{
		char myRode[512];
		char* p = myRode;
		if (len > 512)
		{
			p = new char[len];
		}
		_read(c->ID,p, len);
		messageLite->ParsePartialFromArray(p, len);
		if (len > 512)
		{
			delete[] p;
			p = nullptr;
		}
	}

//解析详细头指令
void net::TcpServer::parseCommand(S_CLIENT_BASE* c, u16 newMainID, const u16 uHandleCode, const u32 len)
{
	c->time_Heart = (int)time(NULL);

	if (newMainID < 65000)
	{
		if (newMainID == CMD_HEART)
		{
			u32 value = 0;
			socketprotobuf::cs_heart_time m;
			ParseProtoBuffData(c, &m, len);
			SendGameDataMessageLite(c->ID, socketprotobuf::cmd_heart, 0, &m);
			return;

			//LOG_MSG("recv CMD_HEART...%d \n", value);
		}

		auto container = __Commands[newMainID];
		if (container == nullptr)
		{
			LOG_MSG("command not register...%d \n", newMainID);
			return;
		}
		//做数字签名算法验证
	/*	char clientSha[29];
		MY_SHA1 sha;
		u32 digest[5];*/
	//	read(c->ID, clientSha, 29);
	//	sha << c->rCode;
	//	sha.Result(digest);
	/*	std::string server_key = myEngine::base64_encode(reinterpret_cast<const unsigned char*>(digest), 20);
		if (strcmp(server_key.c_str(), clientSha))
		{
			LOG_MSG("sha1 recode is error ...%s \n", c->rCode);
			return;
		}*/
	
		//触发事件
		container->onServerCommand(this, c, newMainID);
		return;
	}
	switch (newMainID)
	{
	case socketprotobuf::cmd_security://安全连接
		if (uHandleCode != 0) return;
		char a[40];
		sprintf_s(a, "%s_%s", __ServerInfo->SafeCode, c->rCode);
		memset(c->md5, 0, sizeof(c->md5));
		if (func::MD5str != NULL) func::MD5str(c->md5, (unsigned char*)a, strlen(a));
		socketprotobuf::cs_md5_security m;
		ParseProtoBuffData(c, &m, len);
		u16 handleCode = (u16)0;
		do {
			if (m.version() != __ServerInfo->Version)
			{
				handleCode = (u16)1;
				break;
			}
			int error = stricmp(c->md5, m.md5().c_str());
			if (error != 0)
			{
				handleCode = (u16)2;
				break;
			}
			c->state = S_SOCKET_STATE::S_ConnectSecure;
			c->clientID = m.id();
			c->clientType = m.type();
			//通过客户端ID 绑定关系 
			if (c->clientType != func::S_TYPE_USER) insertClientConnection(c->clientID, c);
			//保护
			this->updateRecurityConnect(true);
			if (onSecureEvent != nullptr) this->onSecureEvent(this, c, 0);
		} while (0);
		SendGameDataMessageLite(c->ID, socketprotobuf::cmd_security, handleCode);
		break;	
	}
}
//检查连接
void net::TcpServer::checkConnect(S_CLIENT_BASE* c)
{
	s32 temp = 0;
	//0、检查安全关闭
	if (c->closeState == SOCKET_CLOSE::S_CLOSE_SHUTDOWN)
	{
		temp = (s32)time(NULL) - c->time_Close;
		if (c->is_RecvCompleted && c->is_SendCompleted)
		{
			closeSocket(c->socketfd, c, 2001);
		}
		else if (temp > 5)
		{
			//LOG_MSG("安全关闭5秒...%d %d \n", c->is_RecvCompleted, c->is_SendCompleted);
			closeSocket(c->socketfd, c, 2002);
		}
		return;
	}
	//1、检查连接
	temp = (s32)time(NULL) - c->time_Connet;
	if (c->state == S_SOCKET_STATE::S_Connect)
	{
		if (temp > 10)
		{
			if (this->onTimeOutEvent != nullptr) onTimeOutEvent(this, c, 2002);
			shutDown(c->socketfd, 0, c, 2002);
			return;
		}
	}

	//2、检查心跳30秒
	temp = (s32)time(NULL) - c->time_Heart;
	if (temp > __ServerInfo->HeartTime)
	{
		if (this->onTimeOutEvent != nullptr) onTimeOutEvent(this, c, 2003);
		shutDown(c->socketfd, 0, c, 2003);
		return;
	}
}
//*************************************************************************
//*************************************************************************
//*************************************************************************
S_CLIENT_BASE* TcpServer::client(const int id)
{
	if (id < 0 || id >= Linkers->length) return nullptr;

	S_CLIENT_BASE * c = Linkers->Value(id);
	return c;
}

S_CLIENT_BASE* net::TcpServer::client(SOCKET socketfd, bool isseriuty)
{
	if (socketfd < 0 || socketfd >= MAX_USER_SOCKETFD) return nullptr;
	S_CLIENT_BASE_INDEX * cindex = LinkersIndexs->Value(socketfd);
	if (cindex == nullptr) return nullptr;
	if (cindex->index < 0) return nullptr;

	S_CLIENT_BASE * c = client(cindex->index);
	if (c == nullptr)
	{
		int fd = socketfd;
		printf("Client c == null %d-%d line:%d\n", fd, cindex->index, __LINE__);
		return nullptr;
	}
	if (isseriuty)
	{
		if (!c->isT(socketfd)) return nullptr;
	}

	return c;
}
S_CLIENT_BASE* net::TcpServer::client(const int id, const u32 clientid)
{
	if (clientid >= MAX_SERVER_ID)  return nullptr;
	

	if (id == MAX_SERVER_ID)
	{
		S_CLIENT_BASE* c = m_Connection[clientid];
		return c;
	}

	if (id < 0 || id >= Linkers->length) return nullptr;
	S_CLIENT_BASE* c = Linkers->Value(id);
	if (c->state >= func::S_ConnectSecure &&
		c->clientID > 0 &&
		c->clientID == clientid) return c;

	c = m_Connection[clientid];
	return c;


}

void net::TcpServer::closeClient(const s32 id)
{
	auto c = client(id);
	if (c == nullptr) return;

	shutDown(c->socketfd, 0, c, 4001);
}

//自定义的结构体,用于TCP服务器
typedef struct tcp_keepalive
{
	unsigned long onoff;
	unsigned long keepalivetime;
	unsigned long keepaliveinterval;
}TCP_KEEPALIVE, * PTCP_KEEPALIVE;

//用于检测突然断线,只适用于windows 2000后平台
//即客户端也需要win2000以上平台
int TcpServer::setHeartCheck(SOCKET s)
{
	DWORD dwError = 0L, dwBytes = 0;
	TCP_KEEPALIVE sKA_Settings = { 0 }, sReturned = { 0 };
	sKA_Settings.onoff = 1;
	sKA_Settings.keepalivetime = 5500; // Keep Alive in 5.5 sec.
	sKA_Settings.keepaliveinterval = 1000; // Resend if No-Reply

	dwError = WSAIoctl(s,
		SIO_KEEPALIVE_VALS,
		&sKA_Settings, sizeof(sKA_Settings),
		&sReturned, sizeof(sReturned),
		&dwBytes,
		NULL,
		NULL);
	if (dwError == SOCKET_ERROR)
	{
		dwError = WSAGetLastError();
		LOG_MSG("SetHeartCheck->WSAIoctl()发生错误,错误代码: %ld  \n", dwError);
		return -1;
	}
	return 0;
}

S_CLIENT_BASE* net::TcpServer::getFreeLinker()
{
	std::lock_guard<std::mutex> guard(this->m_findlink_mutex);
	
	for (int i = 0; i < Linkers->length; i++)
	{
		S_CLIENT_BASE* client = Linkers->Value(i);
		if (client->state == S_SOCKET_STATE::S_Free)
		{
			client->Reset();
			client->ID = i;
			client->state = S_SOCKET_STATE::S_Connect;
			return client;
		}
	}
	return nullptr;
}


bool  TcpServer::isID_T(const s32 id)
{
	if (id < 0 || id >= Linkers->length) return false;
	return true;
}
bool  TcpServer::isSecure_T(const s32 id, s32 secure)
{
	if (id < 0 || id >= Linkers->length) return false;
	S_CLIENT_BASE * c = Linkers->Value(id);
	if (c->state < secure) return false;
	return true;
}
bool  TcpServer::isSecure_F_Close(const s32 id, s32 secure)
{
	if (id < 0 || id >= Linkers->length) return false;
	S_CLIENT_BASE * c = Linkers->Value(id);
	if (c->state >= secure) return false;
	shutDown(c->socketfd, 0, c, 2006);
	return true;
}

//*****************************************************************
//*****************************************************************
//*****************************************************************
//发送数据包封装格式

void net::TcpServer::SendGameDataMessageLite(const int id, const u16 uMainID, const u16 uHandleCode, ::google::protobuf::MessageLite* messageLite)
{
	auto c = client(id);
	if (c == nullptr) return;

	//头尾相等
	if (c->send_Head == c->send_Tail)
	{
		c->send_Tail = 0;
		c->send_Head = 0;
	}
	c->send_TempTail = c->send_Tail;

	if (c->state > 0 &&
		c->is_Sending == false &&
		c->socketfd != INVALID_SOCKET &&
		c->send_TempTail + 10 <= __ServerInfo->SendMax)
	{
		c->is_Sending = true;
		c->sendBuf[c->send_Tail + 0] = __ServerInfo->Head[0] ^ c->rCode[0];
		c->sendBuf[c->send_Tail + 1] = __ServerInfo->Head[1] ^ c->rCode[1];

		u16 newcmd = uMainID ^ c->rCode[0];
		char* a = (char*)&newcmd;
		u16 newHandleCode = uHandleCode ^ c->rCode[1];
		char* b = (char*)&newHandleCode;
		c->sendBuf[c->send_Tail + 6] = a[0];
		c->sendBuf[c->send_Tail + 7] = a[1];
		c->sendBuf[c->send_Tail + 8] = b[0];
		c->sendBuf[c->send_Tail + 9] = b[1];
		c->send_TempTail += 10;
		if (messageLite)_send(id, messageLite);
		end(id);
		return;
	}

	shutDown(c->socketfd, 0, c, 2004);
}



void net::TcpServer::end(const int id)
{
	auto c = client(id);
	if (c == nullptr) return;

	if (c->is_Sending == false ||
		c->send_Tail + 10 > __ServerInfo->SendMax ||
		c->send_TempTail > __ServerInfo->SendMax ||
		c->send_Tail >= c->send_TempTail)
	{
		shutDown(c->socketfd, 0, c, 2005);
		return;
	}
	c->is_Sending = false;
	u32 len = (c->send_TempTail - c->send_Tail) ^ c->rCode[1];
	char* a = (char*)& len;
	c->sendBuf[c->send_Tail + 2] = a[0];
	c->sendBuf[c->send_Tail + 3] = a[1];
	c->sendBuf[c->send_Tail + 4] = a[2];
	c->sendBuf[c->send_Tail + 5] = a[3];
	//最后结束赋值
	c->send_Tail = c->send_TempTail;


}

void net::TcpServer::_send(const int id, ::google::protobuf::MessageLite* messageLite)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (c->is_Sending && c->send_TempTail + messageLite->ByteSize() <= __ServerInfo->SendMax)
	{
		char buff[512];
		c->pbuff =  buff;
		u32 usize = messageLite->ByteSize();
		if(usize > 512)
		{
			c->pbuff = new char[usize];
		}
		messageLite->SerializePartialToArray(c->pbuff, messageLite->ByteSize());
		for (int i = 0; i < messageLite->ByteSize(); i++)
		{
			c->sendBuf[c->send_TempTail + i] = c->pbuff[i];
		}
		c->send_TempTail += messageLite->ByteSize();
		if (usize>512)
		{
			delete[] c->pbuff;
			c->pbuff = nullptr;
		}
		return;
	}
	c->is_Sending = false;
}




//*********************************************************************
//*********************************************************************
//*********************************************************************
//验证客户端有效性
bool isValidClient(S_CLIENT_BASE* c, s32 value)
{
	if (c->ID == -1 ||
		c->state == func::S_Free ||
		c->recv_TempTail == 0 ||
		c->recvBuf == nullptr ||
		c->recv_TempHead + value > c->recv_TempTail)
	{
		return false;
	}
	return true;
}
void net::TcpServer::_read(const int id, void* v, const u32 len)
{
	auto c = client(id);
	if (c == nullptr) return;
	if (isValidClient(c, len) == false)
	{
		v = 0;
		return;
	}
	memcpy(v, &c->recvBuf[c->recv_TempHead], len);
	c->recv_TempHead += len;
}

void net::TcpServer::setOnClientAccept(TCPSERVERNOTIFY_EVENT event)
{
	onAcceptEvent = event;
}

void net::TcpServer::setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event)
{
	onSecureEvent = event;
}

void net::TcpServer::setOnClientDisconnect(TCPSERVERNOTIFY_EVENT event)
{
	onDisconnectEvent = event;
}

void net::TcpServer::setOnClientTimeout(TCPSERVERNOTIFY_EVENT event)
{
	onTimeOutEvent = event;
}

void net::TcpServer::setOnClientExcept(TCPSERVERNOTIFY_EVENT event)
{
	onTimeOutEvent = event;
}

#endif