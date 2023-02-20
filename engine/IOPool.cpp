
#include "IOPool.h"

#ifdef ____WIN32_
Concurrency::concurrent_queue<AcceptContext*> __accepts;
Concurrency::concurrent_queue<RecvContext*>   __recvs;
Concurrency::concurrent_queue<SendContext*>   __sends;
Concurrency::concurrent_queue<DataContext*>   __DataQueue;

IOContext::IOContext()
{
	m_Socket = INVALID_SOCKET;
	m_Mode = 0;
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

IOContext::~IOContext()
{
}
//**************************************************************
//**************************************************************
//**************************************************************
AcceptContext::AcceptContext(int mode, SOCKET listensocket, SOCKET clientsocket)
{
	m_Mode = mode;
	listenfd = listensocket;
	m_Socket = clientsocket;

	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

AcceptContext::~AcceptContext(void)
{
	m_Mode = func::SC_WAIT_ACCEPT;
	listenfd = NULL;
	m_Socket = NULL;

	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

void AcceptContext::clear()
{
	listenfd = NULL;
	m_Socket = NULL;
	memset(&m_OverLapped, 0, sizeof(m_OverLapped));
}

void AcceptContext::setSocket(SOCKET server, SOCKET client)
{
	listenfd = server;
	m_Socket = client;
}
int AcceptContext::getCount()
{
	return __accepts.unsafe_size();
}
AcceptContext* AcceptContext::pop()
{
	AcceptContext* buff = nullptr;
	if (__accepts.empty() == true)
	{
		buff = new  AcceptContext(func::SC_WAIT_ACCEPT, NULL, NULL);

	}
	else
	{
		__accepts.try_pop(buff);
		if (buff == nullptr)
		{
			buff = new AcceptContext(func::SC_WAIT_ACCEPT, NULL, NULL);
		}
	}

	return buff;
}

void AcceptContext::push(AcceptContext* acc)
{
	if (acc == nullptr) return;
	if (__accepts.unsafe_size() > 1000)
	{
		delete acc;
		return;
	}
	acc->clear();
	__accepts.push(acc);
}
//**************************************************************
//**************************************************************
//**************************************************************
//接收数据缓存池
RecvContext::RecvContext(const int mode)
{
	m_Socket = INVALID_SOCKET;
	m_Mode = mode;

	m_Buffs = new char[func::__ServerInfo->ReceOne];

	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));
	ZeroMemory(m_Buffs, func::__ServerInfo->ReceOne);
	m_wsaBuf.buf = m_Buffs;
	m_wsaBuf.len = func::__ServerInfo->ReceOne;
}

RecvContext::~RecvContext(void)
{
	m_Socket = INVALID_SOCKET;
	m_Mode = -1;
}

void RecvContext::clear()
{
	m_Socket = INVALID_SOCKET;

	ZeroMemory(m_Buffs, func::__ServerInfo->ReceOne);
	m_wsaBuf.buf = m_Buffs;
	m_wsaBuf.len = func::__ServerInfo->ReceOne;
}

int RecvContext::getCount()
{
	return __recvs.unsafe_size();
}

RecvContext* RecvContext::pop()
{
	RecvContext* buff = nullptr;
	if (__recvs.empty() == true)
	{
		buff = new  RecvContext(func::SC_WAIT_RECV);

	}
	else
	{
		__recvs.try_pop(buff);
		if (buff == nullptr)
		{
			buff = new RecvContext(func::SC_WAIT_RECV);
		}

	}

	return buff;
}

void RecvContext::push(RecvContext* buff)
{
	if (buff == nullptr) return;
	buff->clear();
	__recvs.push(buff);
}
//**************************************************************
//**************************************************************
//**************************************************************
//发送数据缓存池
SendContext::SendContext(const int mode)
{
	m_Socket = INVALID_SOCKET;
	m_Mode = mode;
	m_Buffs = new char[func::__ServerInfo->SendOne];
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));
	ZeroMemory(m_Buffs, func::__ServerInfo->SendOne);
	m_wsaBuf.buf = m_Buffs;
	m_wsaBuf.len = func::__ServerInfo->SendOne;
}

SendContext::~SendContext(void)
{
	m_Mode = func::SC_WAIT_SEND;
	ZeroMemory(&m_OverLapped, sizeof(m_OverLapped));
}

void SendContext::clear()
{
	m_Socket = INVALID_SOCKET;
	ZeroMemory(m_Buffs, func::__ServerInfo->SendOne);
	m_wsaBuf.buf = m_Buffs;
	m_wsaBuf.len = func::__ServerInfo->SendOne;
}

int SendContext::getCount()
{
	return __sends.unsafe_size();
}

int SendContext::setSend(SOCKET s, char* data, const int sendByte)
{
	m_Socket = s;
	if (&m_wsaBuf)
	{
		if (sendByte != 0 && data)
		{
			memcpy(m_Buffs, data, sendByte);
			m_wsaBuf.buf = m_Buffs;
			m_wsaBuf.len = sendByte;
		}
	}
	return sendByte;
}

SendContext* SendContext::pop()
{
	SendContext* buff = nullptr;
	if (__sends.empty() == true)
	{
		buff = new  SendContext(func::SC_WAIT_SEND);
	}
	else
	{
		__sends.try_pop(buff);
		if (buff == nullptr)
		{
			buff = new  SendContext(func::SC_WAIT_SEND);
		}
	}
	return buff;
}

void SendContext::push(SendContext* buff)
{
	if (buff == nullptr) return;
	buff->clear();
	__sends.push(buff);
}

//TimerContent

DataContext::DataContext(const int mode)
{
	m_Types = mode;
	m_Buffs = new char[func::__ServerInfo->TimerOne];
	ZeroMemory(m_Buffs, func::__ServerInfo->TimerOne);
	buffsSize = func::__ServerInfo->TimerOne;
}

DataContext::DataContext(const int mode,s32 _timerId)
{
	timerId = _timerId;
	m_Types = mode;
	m_Buffs = new char[func::__ServerInfo->TimerOne];
	ZeroMemory(m_Buffs, func::__ServerInfo->TimerOne);
	buffsSize = func::__ServerInfo->TimerOne;
}

DataContext::~DataContext(void)
{
	m_Types = func::TM_DEFAULT;
	timerId = 0;
	if (m_Buffs != nullptr)
	{
		delete[] m_Buffs;
		m_Buffs = nullptr;
	}
}

void DataContext::clear()
{
	timerId = 0;
	while (getCount()!=0)
	{
		auto tc = popTimer();
		if (tc != nullptr)
		{
			delete tc;
			tc = nullptr;
		}
	}
	ZeroMemory(m_Buffs, func::__ServerInfo->TimerOne);
	buffsSize = func::__ServerInfo->TimerOne;
}

int DataContext::getCount()
{
	return __DataQueue.unsafe_size();
}

bool DataContext::addData(HANDLE _CompletionPort, s32 _timerID, UINT uDataKind,void* pdata, UINT size)
{
	if (!_CompletionPort) return false;
	DataContext* buff = nullptr;
	buff = new DataContext(func::TM_DEFAULT);
	buff->timerId = _timerID;
	buff->m_Types = uDataKind;
	if (pdata != nullptr)
	{
		memcpy(buff->m_Buffs, pdata, size);
		buff->buffsSize = size;
	}
	::PostQueuedCompletionStatus(_CompletionPort, buff->buffsSize, NULL, NULL);	//通知完成端口
	DataContext::push(buff);
	return true;
}

void DataContext::push(DataContext* buff)
{
	if (buff == nullptr) return;
	__DataQueue.push(buff);
}



DataContext* DataContext::popTimer()
{
	DataContext* buff = nullptr;
	__DataQueue.try_pop(buff);
	return buff;
}





#endif