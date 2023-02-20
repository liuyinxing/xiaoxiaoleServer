#ifndef  ____TCPPOOL_H
#define  ____TCPPOOL_H
//对象回收池 
#ifdef ____WIN32_

#include "IDefine.h"
#include <concurrent_queue.h>
#define ACCEPT_BUF_LENGTH   ((sizeof(struct sockaddr_in) + 16))*2



class IOContext
{
public:
	IOContext();
	~IOContext();
public:
	WSAOVERLAPPED m_OverLapped;//重叠结构 相当于一个ID
	SOCKET m_Socket;
	int    m_Mode;//用于记录是什么模式 accept recv send timer
};

//1、Accept类
class AcceptContext :public IOContext
{
public:
	AcceptContext(int mode, SOCKET listensocket, SOCKET clientsocket);
	~AcceptContext(void);
public:
	SOCKET  listenfd;
	unsigned char m_buf[ACCEPT_BUF_LENGTH];
public:
	void clear();
	void setSocket(SOCKET server, SOCKET client);
	static AcceptContext* pop();
	static void push(AcceptContext* acc);
	static int getCount();

};

//2、Rece类
class RecvContext :public IOContext
{
private:
	char* m_Buffs;// 这个是WSABUF里具体存字符的缓冲区
public:
	WSABUF  m_wsaBuf; //每次的操作缓冲区

public:
	RecvContext(const int mode);
	~RecvContext(void);
public:
	void clear();
	static int getCount();
	static RecvContext* pop();
	static void push(RecvContext* buff);

};

//3、Send类
class SendContext :public IOContext
{
private:
	char* m_Buffs;// 这个是WSABUF里具体存字符的缓冲区
public:
	WSABUF  m_wsaBuf; //每次的操作缓冲区

public:
	SendContext(const int mode);
	~SendContext(void);

public:
	void clear();
	static int getCount();
	int setSend(SOCKET s, char* data, const int sendByte);
	static SendContext* pop();
	static void push(SendContext* buff);
};
//window定时器消息结构

class DataContext
{
public:
	char* m_Buffs;
	u32   buffsSize;
	s32     timerId;
	int     m_Types;
public:
	DataContext(const int mode);
	DataContext(const int mode,s32 timerId);
	~DataContext(void);

public:
	void clear();
	static int getCount();
	static DataContext* pop();
	static DataContext* popTimer();
	static void push(DataContext* buff);
	static bool addData(HANDLE _CompletionPort, s32 _timerID, UINT uDataKind, void* pdata = nullptr, UINT size = NULL);
};

#endif
#endif