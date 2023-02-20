#ifndef __IDEFINE_H
#define __IDEFINE_H

#include <vector>

#ifdef ____WIN32_
#include <winsock2.h>
#pragma comment(lib,"ws2_32") 
#endif

//
#define MAX_USER_SOCKETFD   1000000 
#define MAX_SERVER_ID       100000  
#define MAX_EXE_LEN         200      
#define MAX_FILENAME_LEN    250     
#define SIO_KEEPALIVE_VALS  IOC_IN | IOC_VENDOR | 4 
#define MAX_MD5_LEN  35 

#define MAX_IP_LEN   20 
#define MAX_COMMAND_LEN     65535
#define MAX_IP_ONE_COUNT    20

#define LOG_MSG printf 

#define CMD_HEART     60000
#define CMD_RCODE     65001
#define CMD_SECURITY  65002


#ifdef ____WIN32_
#define RELEASE_POINTER(p)	{if(p != NULL) {delete p; p = NULL;}}
#define RELEASE_POINTERARRAY(p)	{if(p != NULL) {delete[] p; p = NULL;}}
#define RELEASE_HANDLE(h)	{if(h != NULL && h != INVALID_HANDLE_VALUE) { CloseHandle(h); h = INVALID_HANDLE_VALUE; }}
#define RELEASE_SOCKET(s)	{if(s != INVALID_SOCKET) { closesocket(s); s = INVALID_SOCKET; }}
#endif

typedef signed char          s8;
typedef signed short         s16;
typedef signed int           s32;
typedef unsigned char        u8;
typedef unsigned short       u16;
typedef unsigned int         u32;
typedef float		f32;
typedef double		f64;

#ifdef ____WIN32_
typedef signed long long     s64;
typedef unsigned long long   u64;
#else
typedef signed long int     s64;
typedef unsigned long int   u64;

#endif

#define this_constructor(struct_type) struct_type(){ memset(this, 0, sizeof(struct_type)); }


namespace func
{
	enum SOCKET_CLOSE
	{
		S_CLOSE_FREE = 0,
		S_CLOSE_ACCEPT = 1,   //���ӳ���ر�
		S_CLOSE_SHUTDOWN = 2, //�ر�����
		S_CLOSE_CLOSE = 3     //��ʽ�ر�
	};
	enum SOCKET_CONTEXT_STATE
	{
		SC_WAIT_ACCEPT = 0,
		SC_WAIT_RECV = 1,
		SC_WAIT_SEND = 2,
		SC_WAIT_RESET = 3,
	};

	enum TIMER_CONTEXT_TYPES
	{
		TM_DEFAULT=0,
		TM_ROOM = 1,
		TM_USER = 2,
		TM_MONTERS = 3,
		TM_BUFF =4,
	};

	//������
	enum S_SOCKET_STATE
	{
		S_Free = 0,
		S_Connect = 1,
		S_ConnectSecure = 2,
		S_Login = 3,
		S_NeedSave = 4,
		S_Saving = 5
	};
	//�ͻ���SOCKETö��״̬
	enum C_SOCKET_STATE
	{
		C_Free = 0,
		C_ConnectTry = 1,
		C_Connect = 2,
		C_ConnectSecure = 3,
		C_Login = 4
	};

	//0 ��� 1-DB 2-���ķ����� 3-��Ϸ������ 4-���ط����� 5-��¼������
	enum E_SERVER_TYPE
	{
		S_TYPE_USER = 0x00,
		S_TYPE_DB,
		S_TYPE_CENTER,
		S_TYPE_GAME,
		S_TYPE_GATE,
		S_TYPE_LOGIN,
		S_TYPE_REDIS
	};

	struct ConfigXML
	{
		s32   ID;     //������ID
		u8    Type;   //���������� 1-DB 2-���ķ����� 3-��ͼ������ 4-���ط����� 
		u16   Port;   //�������˿ں�
		u16   HttpPort;//Http�������˿ں�
		s32   MaxUser;//����������
		s32   MaxConnect;//���ͻ�����������
		u8    RCode;
		s32   Version;
		s32   ReceOne;
		s32   ReceMax;
		s32   SendOne;
		s32   SendMax;
		s32   HeartTime;//����ʱ��
		s32   HeartTimeMax;
		s32   AutoTime;//�Զ�����ʱ��
		s32   MaxAccpet; //���Ͷ����������
		s32   MaxRece;   //����յ���Ϣ����
		s32   MaxSend;   //�������Ϣ����
		s32   TimerOne;   //��ʱ���������մ�С
		char  SafeCode[20];
		char  Head[3];
		char  IP[MAX_IP_LEN];
	};
	struct ServerListXML
	{
		s32   ID;
		u16   Port;
		char  IP[MAX_IP_LEN];
	};


	extern char FileExePath[MAX_EXE_LEN];
	extern ConfigXML* __ServerInfo;
	extern ConfigXML* __ClientInfo;
	extern std::vector<ServerListXML*> __ServerListInfo;
	extern std::vector<ServerListXML*> __ServerList;
	extern u8 GetServerType(s32 sid);
	extern void(*MD5str)(char* output, unsigned char* input, int len);
	extern bool InitData();

	extern const char* getShutDownError(int id);
	extern const char* getCloseSocketError(int id);
	extern void setConsoleColor(u16 index);
	
}
#endif 