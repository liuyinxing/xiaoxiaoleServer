#ifndef  ____TCPCLIENT_H
#define  ____TCPCLIENT_H

#include "INetBase.h"
#include <atomic>
#include <mutex>
#include  <thread>
#include <vector>
namespace net
{
	struct pos1
	{
		int x = -1;
		int y = -1;
		inline void reset()
		{
			x = -1;
			y = -1;
		}
	};
	class TcpClient :public ITcpClient
	{
	private:
#ifdef ____WIN32_
		SOCKET   socketfd;
#else
		int      socketfd;
#endif
		S_SERVER_BASE  m_data;
		std::shared_ptr<std::thread> m_workthread;

		TCPCLIENTNOTIFY_EVENT      onAcceptEvent;
		TCPCLIENTNOTIFY_EVENT      onSecureEvent;
		TCPCLIENTNOTIFY_EVENT      onDisconnectEvent;
		TCPCLIENTNOTIFY_EVENT      onExceptEvent;

		s32 initSocket();
		bool setNonblockingSocket();
		void connect_Select();
		void onAutoConnect();
		int onRecv();
		int onSend();
		int onSaveData(int recvBytes);
		
		void  onHeart();
		void  parseCommand(u16 cmd, const u16 uHandleCode,const u32 len);
		void  ParseProtoBuffData(::google::protobuf::MessageLite* messageLite, size_t len);
		void  initCommands();

		void runThread();
		static void run(TcpClient* tcp);
	public:
		TcpClient();
		virtual ~TcpClient();
		virtual inline S_SERVER_BASE* getData() { return &m_data; };
	
#ifdef ____WIN32_
		virtual inline SOCKET getSocket() { return socketfd; };
#else
		virtual inline int getSocket() { return socketfd; };
#endif 
		virtual void runClient(u32 sid,char* ip, int port);
		virtual bool connectServer();
		virtual void disconnectServer(const s32 errcode, const char* err);

		virtual void sendSha1();

		virtual void  SendGameDataMessageLite(const u16 uMainID, const u16 uHandleCode, ::google::protobuf::MessageLite* messageLite=nullptr);
		virtual void  end();
		virtual void _send(::google::protobuf::MessageLite* messageLite);
		virtual void _read(void* v, const u32 len);
		virtual void  parseCommand();
		virtual void  registerCommand(int cmd, void* container);
		virtual void  setOnConnect(TCPCLIENTNOTIFY_EVENT event);
		virtual void  setOnConnectSecure(TCPCLIENTNOTIFY_EVENT event);
		virtual void  setOnDisconnect(TCPCLIENTNOTIFY_EVENT event);
		virtual void  setOnExcept(TCPCLIENTNOTIFY_EVENT event);
		
	};
}

#endif