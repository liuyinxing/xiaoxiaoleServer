#ifndef  ____TCPSERVER_H
#define  ____TCPSERVER_H

#ifdef ____WIN32_

#include "INetBase.h"
#include "IContainer.h"
#include <mutex>
#include <thread> 
#include <map>
#include <MSWSock.h>
#pragma comment(lib,"mswsock")
#define GWL_USERDATA        (-21)
#define HD_TIMER_MESSAGE				4							//定时器消息处理
namespace net
{
	class TcpServer :public ITcpServer
	{
	public:
		TcpServer(void);
		virtual ~TcpServer(void);
	private:
		s32      m_ConnectCount; //当前连接数
		s32      m_SecurityCount;//安全连接数
		bool     m_IsRunning;
		bool     b_windowRun;
		s32      m_ThreadNum;//线程数量
		SOCKET   listenfd;   //监听套接字句柄
		HANDLE   m_Completeport;//完成端口句柄
		LPFN_ACCEPTEX m_AcceptEx;	//AcceptEx函数地址
		LPFN_GETACCEPTEXSOCKADDRS  m_GetAcceptEx;//获取客户端信息函数地址
		HWND									m_hWindow;					///窗口句柄
		std::shared_ptr<std::thread>			m_hWindowThread;			///窗口线程
		std::shared_ptr<std::thread>  m_workthread[10];
		//std::shared_ptr<std::thread> m_sendthread;

		std::mutex		 m_findlink_mutex;
		std::mutex       m_ConnectMutex;
		std::mutex       m_SecurityMutex;
		
		HashArray<S_CLIENT_BASE>* Linkers;//连接玩家
		HashArray<S_CLIENT_BASE_INDEX>* LinkersIndexs;//连接玩家索引数组

		//5个函数指针 用于业务层通知事件
		TCPSERVERNOTIFY_EVENT      onAcceptEvent;
		TCPSERVERNOTIFY_EVENT      onSecureEvent;
		TCPSERVERNOTIFY_EVENT      onTimeOutEvent;
		TCPSERVERNOTIFY_EVENT      onDisconnectEvent;
		TCPSERVERNOTIFY_EVENT      onExceptEvent;

		//过滤单IP N个连接 
		std::map<std::string, s32>   m_LimitsIPs;
		std::mutex                   m_LimitIPMutex;
		//客户端ID 注册
		S_CLIENT_BASE*               m_Connection[MAX_SERVER_ID];
	private:
		//投递accept
		s32 postAccept();//投递连接
		s32 onAccpet(void* context);//连接事件
		//投递recv
		s32 postRecv(SOCKET s);
		s32 onRecv(void* context, s32 recvBytes, u32 tid);
		s32 onRecv_SaveData(S_CLIENT_BASE* c, char* buf, s32 recvBytes);
		//投递send
		s32 postSend(S_CLIENT_BASE* c);
		s32 onSend(void* context, s32 sendBytes);
		//定时器
		s32 onTimerMessage(void* tc, s32 sendBytes);
		int WindowMsgThread();
		bool WindowTimerMessage(UINT uTimerID);
		static LRESULT CALLBACK WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		bool SetTimer(UINT uTimerID, UINT uElapse);
		//清除定时器
		virtual bool KillTimer(UINT uTimerID);
	
		

		s32 closeSocket(SOCKET socketfd, S_CLIENT_BASE* c, int kind);
		void shutDown(SOCKET s, const s32 mode, S_CLIENT_BASE* c, int kind);
		int setHeartCheck(SOCKET s);
		S_CLIENT_BASE* getFreeLinker();

		//***********************************************************
		inline void insertClientConnection(s32 serverid,S_CLIENT_BASE* c)
		{
			if (serverid < 0 || serverid >= MAX_SERVER_ID) return;
			m_Connection[serverid] = c;
		}
		inline void clearClientConnection(s32 serverid)
		{
			if (serverid < 0 || serverid >= MAX_SERVER_ID) return;
			m_Connection[serverid] = nullptr;
		}

		inline HANDLE getCompletePort()
		{
			return m_Completeport;
		}
		//安全连接数量
		inline void updateRecurityConnect(bool isadd)
		{
			{
				std::lock_guard<std::mutex> guard(this->m_SecurityMutex);

				if (isadd) m_SecurityCount++;
				else m_SecurityCount--;
			}

		}
		//连接数量
		inline void updateConnect(bool isadd)
		{
			{
				std::lock_guard<std::mutex> guard(this->m_ConnectMutex);

				if (isadd) m_ConnectCount++;
				else m_ConnectCount--;
			}
		}
		inline S_CLIENT_BASE_INDEX* getClientIndex(const int socketfd)
		{
			if (socketfd < 0 || socketfd >= MAX_USER_SOCKETFD) return nullptr;
			S_CLIENT_BASE_INDEX * c = LinkersIndexs->Value(socketfd);
			return c;
		}



	private:
		s32   initSocket();
		void  initPost();
		void  initCommands();

		void  runThread(int num);
		void  parseCommand(S_CLIENT_BASE* c);
		void  ParseProtoBuffData(S_CLIENT_BASE* c, ::google::protobuf::MessageLite* messageLite, size_t len);
		void  parseCommand(S_CLIENT_BASE* c, u16 newMainID, const u16 uHandleCode, const u32 len);
		void  checkConnect(S_CLIENT_BASE* c);

		static void run(TcpServer * tcp, int id);
		//static void run_send(TcpServer * tcp);
	public:
		virtual void  runServer(s32 num);
		virtual void  stopServer();
		virtual S_CLIENT_BASE* client(SOCKET socketfd, bool isseriuty);
		virtual S_CLIENT_BASE* client(const int id);
		virtual S_CLIENT_BASE* client(const int id, const u32 clientid);
		virtual void  closeClient(const s32 id);

		virtual bool  isID_T(const s32 id);
		virtual bool  isSecure_T(const s32 id, s32 secure);
		virtual bool  isSecure_F_Close(const s32 id, s32 secure);

		virtual void  parseCommand();
		virtual void  getSecurityCount(int& connum, int& securtiynum);

		virtual void SendGameDataMessageLite(const int id, const u16 uMainID, const u16 uHandleCode, ::google::protobuf::MessageLite* messageLite = nullptr);
		virtual void  end(const int id);
		virtual void  _send(const int id, ::google::protobuf::MessageLite* messageLite);
		virtual void _read(const int id, void* v, const u32 len);
		virtual void  setOnClientAccept(TCPSERVERNOTIFY_EVENT event);
		virtual void  setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event);
		virtual void  setOnClientDisconnect(TCPSERVERNOTIFY_EVENT event);
		virtual void  setOnClientTimeout(TCPSERVERNOTIFY_EVENT event);
		virtual void  setOnClientExcept(TCPSERVERNOTIFY_EVENT event);
		virtual void  registerCommand(int cmd, void* container);
		virtual void  registerTimerContainer(void* container);
	};
}

#endif 
#endif 