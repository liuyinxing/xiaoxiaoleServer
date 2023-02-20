#ifndef  ____INETBASE_H
#define  ____INETBASE_H 

#include "IDefine.h"
#include<vector>
#include <UserData.h>
#include <google/protobuf/message_lite.h>
namespace net
{

#pragma pack(push,packing)
#pragma pack(1)


	struct S_CLIENT_BASE_INDEX
	{

		s32 index;
		inline void Reset() { index = -1; }
	};


	struct S_CLIENT_BASE
	{
		//�̶�����
#ifdef ____WIN32_
		SOCKET       socketfd;
#else
		int          socketfd;
#endif 
		//��̬�仯 
		s8              state;//�û�״̬ 0���� 1���� 2��¼
		s8              closeState;//�ر�״̬ ������ɶ˿ڶ��� û��Ͷ�ݲŰ�ȫ�ر�
		char            ip[MAX_IP_LEN];
		u16			    port;
		int				ID; //��Ӧ������
		char            rCode[21];
		//��Ϸ��¼����
		s32             clientID;   //�������ܷ�����ID 
		u8              clientType; //0 1-DB 2-Center 3-Game 4-Gate 5-Login 
		s32				clientMap[13][13];
	
		//��������
		bool			is_RecvCompleted;
		char*           recvBuf;
		s32				recv_Head;//ͷ ������ʹ��
		s32				recv_Tail;//β ������ʹ��
		s32				recv_TempHead;//��ʱͷ
		s32				recv_TempTail;//��ʱβ

		//��������
		bool			is_Sending;
		char*			pbuff;
		bool            is_SendCompleted;
		char*           sendBuf;
		s32				send_Head;
		s32				send_Tail;
		s32				send_TempTail;
		//ʱ����
		s32             time_Connet;
		s32             time_Heart;
		s32             time_Close;
		u8              theadID;
		s32             shutdown_kind;
		char            md5[MAX_MD5_LEN];
		char			name[USER_MAX_MEMBER];
		u64             memid;
		u32             temp_LoginTime;//��¼��Ϸ
		void Init();
		void Reset();


#ifdef ____WIN32_
		inline bool isT(SOCKET client)
		{
			if (socketfd == client) return true;
			return false;
		}
#else
		inline bool isT(int client)
		{
			if (socketfd == client) return true;
			return false;
		}
#endif

	};

	//************************************************************
    //************************************************************
    //************************************************************
    //�ͻ���-�ṹ��
	struct S_SERVER_BASE
	{
#ifdef ____WIN32_
		SOCKET       socketfd;
#else
		s32          socketfd;
#endif 
		s32				ID;   //�ͻ���ID
		char			ip[16];
		u16		        port;
		s32             serverID;//0 1000-DB 2000-Center 3000-Game 4000-Gate 5000-Login 
		u8              serverType;//0 1-DB 2-Center 3-Game 4-Gate 5-Login
									//��̬�仯�ֶ�
		u8              state;//�û�״̬ 0���� 1�������� 2��ȫ����  3���ӳɹ�
		char            rCode[21];
		char*           recvBuf_Temp;
		char*           recvBuf;
		s32				recv_Head;//ͷ ������ʹ��
		s32				recv_Tail;//β ������ʹ��
		s32				recv_TempHead;//��ʱͷ
		s32				recv_TempTail;//��ʱβ
		bool			is_Recved;

		char*           sendBuf;
		s32				send_Head;
		s32				send_Tail;
		s32				send_TempTail;
		bool			is_Sending;
		bool            is_SendCompleted;

		s32             time_Heart;
		s32             time_AutoConnect;
		char            md5[MAX_MD5_LEN];

		void init(int sid);
		void reset();
	};
#pragma pack(pop, packing)

	class ITcpServer;
	class ITcpClient;

	typedef void(*TCPSERVERNOTIFY_EVENT) (ITcpServer* tcp, S_CLIENT_BASE* c, const s32 code);
	typedef void(*TCPSERVERNOTIFYERRO_EVENT) (ITcpServer* tcp, S_CLIENT_BASE* c, const s32 code, const char* err);
	typedef void(*TCPCLIENTNOTIFY_EVENT)(ITcpClient* tcp, const s32 code);


	//�������ǵķ������ӿ� ���麯�� 
	class ITcpServer
	{
	public:
		virtual ~ITcpServer() {}
		virtual void  runServer(s32 num) = 0;
		virtual void  stopServer() = 0;
#ifdef ____WIN32_
		virtual S_CLIENT_BASE* client(SOCKET socketfd, bool isseriuty) = 0;
#else
		virtual S_CLIENT_BASE* client(int socketfd, bool isseriuty) = 0;
#endif
		virtual S_CLIENT_BASE* client(const int id) = 0;
		virtual S_CLIENT_BASE* client(const int id, const u32 clientid) = 0;
		virtual void  closeClient(const s32 id) = 0;
		virtual bool  isID_T(const s32 id) = 0;
		virtual bool  isSecure_T(const s32 id, s32 secure) = 0;
		virtual bool  isSecure_F_Close(const s32 id, s32 secure) = 0;
		virtual void  parseCommand() = 0;
		virtual void  getSecurityCount(int& connum, int& securtiynum) = 0;
		//��װ�������ݰ�
		virtual void SendGameDataMessageLite(const int id, const u16 uMainID, const u16 uHandleCode, ::google::protobuf::MessageLite* messageLite = nullptr)=0;
		virtual bool KillTimer(UINT uTimerID)=0;
		virtual void  end(const int id) = 0;
		virtual void  _send(const int id, ::google::protobuf::MessageLite* messageLite)=0;
		//�����������ݰ�
		virtual void _read(const int id, void* v, const u32 len)=0;

		virtual void  setOnClientAccept(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnClientSecureConnect(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnClientDisconnect(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnClientTimeout(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  setOnClientExcept(TCPSERVERNOTIFY_EVENT event) = 0;
		virtual void  registerCommand(int cmd, void* container) = 0;
		virtual void  registerTimerContainer(void* container) =0 ; 
	};


	class ITcpClient
	{
	public:
		virtual ~ITcpClient() {}

		virtual S_SERVER_BASE* getData() = 0;
#ifdef ____WIN32_
		virtual SOCKET getSocket() = 0;
#else
		virtual int getSocket() = 0;
#endif 
		virtual void runClient(u32 sid, char* ip, int por) = 0;
		virtual bool connectServer() = 0;
		virtual void disconnectServer(const s32 errcode, const char* err) = 0;

		virtual void SendGameDataMessageLite(const u16 uMainID, const u16 uHandleCode, ::google::protobuf::MessageLite* messageLite =nullptr)=0;
		virtual void  end() = 0;
		virtual void _send(::google::protobuf::MessageLite* messageLite)=0;
		virtual void  _read(void* v, const u32 len) = 0;

		virtual void  parseCommand() = 0;
		virtual void  registerCommand(int cmd, void* container) = 0;
		virtual void  setOnConnect(TCPCLIENTNOTIFY_EVENT event) = 0;
		virtual void  setOnConnectSecure(TCPCLIENTNOTIFY_EVENT event) = 0;
		virtual void  setOnDisconnect(TCPCLIENTNOTIFY_EVENT event) = 0;
		virtual void  setOnExcept(TCPCLIENTNOTIFY_EVENT event) = 0;

	};
	extern ITcpServer* NewTcpServer();
	extern ITcpClient* NewTcpClient();

}

#endif