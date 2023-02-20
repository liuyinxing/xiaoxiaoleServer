#include "TcpClient.h"
#include "IContainer.h"
#include"base.h"
#include <string.h>
#include "sha1/sha1.h"
#include "SDK/protobuf/socket.pb.h"
#include "INetBase.h"
namespace net
{
	//业务层指令集合
	std::vector<IContainer*> __Commands;

	//******************************************************************
	void  TcpClient::initCommands()
	{
		if (__Commands.size() == MAX_COMMAND_LEN) return;
		__Commands.reserve(MAX_COMMAND_LEN);
		for (int i = 0; i < MAX_COMMAND_LEN; i++)
		{
			__Commands.emplace_back(nullptr);
		}
	}

	//注册
	void  TcpClient::registerCommand(int cmd, void* container)
	{
		if (cmd >= MAX_COMMAND_LEN) return;
		IContainer * icon = (IContainer*)container;
		if (icon == nullptr) return;
		__Commands[cmd] = icon;
	}
	//更新
	void TcpClient::parseCommand()
	{
		if (socketfd < 0) return;
		auto c = getData();
		if (c->state < func::C_Connect) return;

		//发送心跳包
		onHeart();
		//解析指令
		//if (c->recv_Tail <= c->recv_Head) return;
		while (c->recv_Tail - c->recv_Head > 9)
		{
			//1、解析头
			char head[2];
			head[0] = c->recvBuf[c->recv_Head] ^ c->rCode[0];
			head[1] = c->recvBuf[c->recv_Head + 1] ^ c->rCode[1];
			
			if (head[0] != func::__ClientInfo->Head[0] || head[1] != func::__ClientInfo->Head[1])
			{
				disconnectServer(2001, "head error...");
				return;
			}

			s32 cl = (*(u32*)(c->recvBuf + c->recv_Head + 2)) ^ c->rCode[1];
			u16 newMainID = (*(u16*)(c->recvBuf + c->recv_Head + 6)) ^ c->rCode[0];
			u16 newHandleCode = (*(u16*)(c->recvBuf + c->recv_Head + 8)) ^ c->rCode[1];
			//2、长度不够 需要继续等待
			if (c->recv_Tail < c->recv_Head + cl) break;
			c->recv_TempHead = c->recv_Head + 10;
			c->recv_TempTail = c->recv_Head + cl;
			parseCommand(newMainID, newHandleCode,cl-10);
			if (c->state < func::C_Connect) return;

			//4、增加读取长度
			c->recv_Head += cl;

			//printf("readdata : %d ..%d:%d.\n", c->State, c->Rece_B, c->Rece_E);
		}
		//发送数据
		this->onSend();
	}
	void TcpClient::onHeart()
	{
		auto c = getData();
		if (c->state < func::C_ConnectSecure) return;
		s32 tempTime = (s32)time(NULL) - m_data.time_Heart;
		if (tempTime >= func::__ClientInfo->HeartTime)
		{
			u32 ftime = clock();
			m_data.time_Heart = (s32)time(NULL);
			socketprotobuf::cs_heart_time m;
			m.set_ftime(ftime);
			SendGameDataMessageLite(socketprotobuf::cmd_heart, 0, &m);
		}
	}
	void TcpClient::ParseProtoBuffData(::google::protobuf::MessageLite* messageLite, size_t len)
	{
		char myRode[512];
		char* p = myRode;
		if (len > 512)
		{
			p = new char[len];
		}
		_read(p, len);
		messageLite->ParsePartialFromArray(p, len);
		if (len > 512)
		{
			delete[] p;
			p = nullptr;
		}
	}

	void TcpClient::parseCommand(u16 newMainID, const u16 uHandleCode,const u32 len)
	{

		//LOG_MSG("cmd....%d \n", cmd);
		if (newMainID < 65000)
		{
			if (newMainID == CMD_HEART)
			{
				if (this->getData()->serverType == func::S_TYPE_GAME)
				{
					int ftime;
					//this->read(ftime);
					ftime = clock() - ftime;
				//	LOG_MSG("heart....time:%d 毫秒 \n", ftime);
				}
			}

			auto container = __Commands[newMainID];
			if (container == nullptr)
			{
				//LOG_MSG("--------client command not register...%d \n", cmd);
				return;
			}

			//触发事件
			container->onClientCommand(this, newMainID);
			return;
		}

		switch (newMainID)
		{
		case socketprotobuf::cmd_rcode:
		{
			auto c = getData();
			socketprotobuf::sc_send_Rcode s;
			ParseProtoBuffData(&s, len);
			LOG_MSG("%s", s.rcode().c_str());
			std::string s1 = (myEngine::base64_decode(s.rcode().c_str()));
			strcpy(c->rCode, s1.c_str());
			char a[40];
			sprintf(a, "%s_%s", func::__ClientInfo->SafeCode, c->rCode);
			if (func::MD5str != NULL) func::MD5str(c->md5, (unsigned char*)a, strlen(a));
			std::string md5(c->md5);
			//发送MD5验证
			socketprotobuf::cs_md5_security m;
			m.set_id((s32)0);
			m.set_type((s32)0);
			m.set_version(func::__ClientInfo->Version);
			m.set_md5(md5);
			SendGameDataMessageLite(socketprotobuf::cmd_security, 0, &m);
		}
		break;
		case CMD_SECURITY:
		{
			auto c = getData();
			//printf("-----------client securrity...%d \n", kind);
			if (uHandleCode > 0)
			{
				//1 版本不对 2 MD5错误 
				if (onExceptEvent != nullptr) onExceptEvent(this, uHandleCode);
				break;
			}
			c->state = func::C_ConnectSecure;
			LOG_MSG("%d", c->state);
			if (onSecureEvent != nullptr) onSecureEvent(this, 0);
		}

		break;
		}
	}
	//**********************************************************************
	//**********************************************************************

	void TcpClient::sendSha1()
	{
	/*	auto c = getData();
		char clientSha[29];
		MY_SHA1 sha;
		u32 digest[5];
		sha << c->rCode;
		sha.Result(digest);
		std::string server_key = myEngine::base64_encode(reinterpret_cast<const unsigned char*>(digest), 20);
		strcpy(clientSha, server_key.c_str());
		_send(clientSha, 29);*/
	}

	void TcpClient::SendGameDataMessageLite(const u16 uMainID, const u16 uHandleCode, ::google::protobuf::MessageLite* messageLite)
	{
		auto c = getData();
		//头尾相等
		if (c->send_Head == c->send_Tail)
		{
			c->send_Tail = 0;
			c->send_Head = 0;
		}
		c->send_TempTail = c->send_Tail;

		if (c->state >= func::C_Connect &&
			c->is_Sending == false &&
			socketfd > 0 &&
			c->send_Tail + 10 <= func::__ClientInfo->SendMax)
		{
			c->is_Sending = true;
			c->sendBuf[c->send_Tail + 0] = func::__ClientInfo->Head[0] ^ c->rCode[0];
			c->sendBuf[c->send_Tail + 1] = func::__ClientInfo->Head[1] ^ c->rCode[1];
			u16 newcmd = uMainID ^ c->rCode[0];
			char* a = (char*)&newcmd;
			u16 newHandleCode = uHandleCode ^ c->rCode[1];
			char* b = (char*)&newHandleCode;
			c->sendBuf[c->send_Tail + 6] = a[0];
			c->sendBuf[c->send_Tail + 7] = a[1];
			c->sendBuf[c->send_Tail + 8] = b[0];
			c->sendBuf[c->send_Tail + 9] = b[1];
			c->send_TempTail += 10;
			//有命令的时候 验证sha签名算法
		/*	if (uMainID < 60000)
			{
				sendSha1();
			}*/
			if (messageLite)_send(messageLite);
			end();
			return;
		}
		disconnectServer(6001, "b error...");
	}


	void TcpClient::end()
	{
		auto c = getData();
		if (c->state == func::C_Free ||
			c->is_Sending == false ||
			socketfd < 0 ||
			c->send_Tail + 10 > func::__ClientInfo->SendMax ||
			c->send_TempTail > func::__ClientInfo->SendMax ||
			c->send_Tail >= c->send_TempTail)
		{
			disconnectServer(6002, "e error...");
			return;
		}

		c->is_Sending = false;
		u32 len = (c->send_TempTail - c->send_Tail) ^ c->rCode[0];
		char* a = (char*)& len;
		c->sendBuf[c->send_Tail + 2] = a[0];
		c->sendBuf[c->send_Tail + 3] = a[1];
		c->sendBuf[c->send_Tail + 4] = a[2];
		c->sendBuf[c->send_Tail + 5] = a[3];

		//最后结束赋值
		c->send_Tail = c->send_TempTail;
	}



	void net::TcpClient::_send(::google::protobuf::MessageLite* messageLite)
	{
		auto c = getData();
		if (c == nullptr) return;
		if (c->is_Sending && c->send_TempTail + messageLite->ByteSize() <= func::__ClientInfo->SendMax)
		{
			char buff[512];
			char * pbuff = buff;
			u32 usize = messageLite->ByteSize();
			if (usize > 512)
			{
				pbuff = new char[usize];
			}
			messageLite->SerializePartialToArray(pbuff, messageLite->ByteSize());
			for (int i = 0; i < messageLite->ByteSize(); i++)
			{
				c->sendBuf[c->send_TempTail + i] =pbuff[i];
			}
			c->send_TempTail += messageLite->ByteSize();
			if (usize > 512)
			{
				delete[] pbuff;
				pbuff = nullptr;
			}
			return;
		}
		c->is_Sending = false;
	}


	//*******************************************************************
	//*******************************************************************
	//*******************************************************************
	bool isValid(S_SERVER_BASE* c, s32 value)
	{
		if (c->state == func::C_Free ||
			c->recv_TempTail == 0 ||
			c->recv_TempHead + value > c->recv_TempTail)
		{
			return false;
		}
		return true;
	}



	//void TcpClient::(void* v, const u32 len)
	//{
	//	auto c = getData();
	//	if (isValid(c, len) == false)
	//	{
	//		v = 0;
	//		return;
	//	}
	//	memcpy(v, &c->recvBuf[c->recv_TempHead], len);
	//	c->recv_TempHead += len;
	//}


	void TcpClient::setOnConnect(TCPCLIENTNOTIFY_EVENT event)
	{
		onAcceptEvent = event;
	}

	void TcpClient::setOnConnectSecure(TCPCLIENTNOTIFY_EVENT event)
	{
		onSecureEvent = event;
	}

	void TcpClient::setOnDisconnect(TCPCLIENTNOTIFY_EVENT event)
	{
		onDisconnectEvent = event;
	}

	void TcpClient::setOnExcept(TCPCLIENTNOTIFY_EVENT event)
	{
		onExceptEvent = event;
	}


}