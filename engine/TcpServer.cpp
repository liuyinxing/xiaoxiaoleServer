#include "TcpServer.h"
#ifdef ____WIN32_

#include "IOPool.h"
#include "include/openssl/sha.h"
#include "ShellRandom.hpp"
#include "sha1/sha1.h"
#include "base.h"
#include "SDK/protobuf/socket.pb.h"
#include<cstring>
using namespace net;
using namespace func;


ITcpServer* net::NewTcpServer()
{
	return new net::TcpServer();
}

TcpServer::TcpServer()
{
	m_ThreadNum     = 0;
	m_AcceptEx      = NULL;
	m_GetAcceptEx   = NULL;
	listenfd        = INVALID_SOCKET;
	m_Completeport  = NULL;
	m_ConnectCount  = 0;
	m_SecurityCount = 0;
	m_IsRunning     = false;
	m_hWindow = NULL;
	b_windowRun = false;
	onAcceptEvent     = nullptr;
	onSecureEvent     = nullptr;
	onTimeOutEvent    = nullptr;
	onDisconnectEvent = nullptr;
	onExceptEvent     = nullptr;
}

TcpServer::~TcpServer()
{
}

void TcpServer::runServer(s32 num)
{
	for (u32 i = 0; i < MAX_SERVER_ID; i++)
	{
		m_Connection[i] = nullptr;
	}
	//1�����������û�
	Linkers = new HashArray<S_CLIENT_BASE>(__ServerInfo->MaxConnect);
	for (int i = 0; i < Linkers->length; i++)
	{
		S_CLIENT_BASE* client = Linkers->Value(i);
		client->Init();
	}
	//2�������û�����
	LinkersIndexs = new HashArray<S_CLIENT_BASE_INDEX>(MAX_USER_SOCKETFD);
	for (int i = 0; i < MAX_USER_SOCKETFD; i++)
	{
		S_CLIENT_BASE_INDEX* client = LinkersIndexs->Value(i);
		client->Reset();
	}
	//3��ָ���ʼ��
	initCommands();

	//4����ʼ��socket
	s32 err = initSocket();
	if (err < 0)
	{
		LOG_MSG("InitServer err...%d �˿ں���ʹ��...\n", err);

		RELEASE_HANDLE(m_Completeport);
		RELEASE_SOCKET(listenfd);
		if (err != -2) WSACleanup();

		Sleep(3000);
		exit(1);
		return;
	}
	//6����ʼ��Ͷ������
	initPost();
	//6����ʼ�����߳�
	runThread(num);

}
void net::TcpServer::stopServer()
{
	for (int i = 0; i < m_ThreadNum; i++)
	{
		PostQueuedCompletionStatus(m_Completeport, 0, (DWORD)1, NULL);
	}
	m_IsRunning = false;
	RELEASE_HANDLE(m_Completeport);
	RELEASE_SOCKET(listenfd);
	WSACleanup();
	LOG_MSG("stop server success...\n");
}
s32 TcpServer::initSocket()
{
	//0��������ɶ˿�
	m_Completeport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_Completeport == NULL) return -1;

	//1����ʼ��Windows Sockets DLL
	WSADATA  wsData;
	int errorcode = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (errorcode != 0) return -2;

	//2�������׽��� �ص�IO WSA_FLAG_OVERLAPPED
	listenfd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenfd == INVALID_SOCKET) return -3;

	//3�������׽��ַ�����ģʽ �����׽ӿڵ�ģʽ��
    //������ֹ�׽ӿ�s�ķ�����ģʽ
	unsigned long ul = 1;
	errorcode = ioctlsocket(listenfd, FIONBIO, (unsigned long*)& ul);
	if (errorcode == SOCKET_ERROR) return -4;

	//4���رռ���socket�Ľ����뷢�ͻ�����
	int bufferSize = 0;
	setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, (char*)& bufferSize, sizeof(int));
	setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, (char*)& bufferSize, sizeof(int));

	//5���Ѽ���socket����ɶ˿ڰ
	HANDLE  handle = CreateIoCompletionPort((HANDLE)listenfd, m_Completeport, (DWORD)SC_WAIT_ACCEPT, 0);
	if (handle == nullptr) return -5;

	//6�����׽��� 
	//htons�ǽ����ͱ����������ֽ�˳��ת��������ֽ�˳��
	//���������ڵ�ַ�ռ�洢��ʽ��Ϊ��λ�ֽڴ�����ڴ�ĵ͵�ַ��
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(__ServerInfo->Port);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	errorcode = ::bind(listenfd, (struct sockaddr*) & serAddr, sizeof(serAddr));
	if (errorcode == SOCKET_ERROR) return -6;
	//7������ �Ѿ�����������ֵĶ������� ������ϵͳ��ÿһ���˿����ļ������еĳ���,
	errorcode = listen(listenfd, SOMAXCONN);
	if (errorcode == SOCKET_ERROR) return -7;

	//8�� AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptEx = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes = 0;
	if (m_AcceptEx == nullptr)
	{

		//WSAIoctl()������һ���׽ӿڵ�ģʽ�����óɹ���WSAIoctl()��������0��
		//����Ļ���������SOCKET_ERROR����Ӧ�ó����ͨ��WSAGetLastError()����ȡ��Ӧ�Ĵ�����롣
		errorcode = WSAIoctl(listenfd,//����Ҫ���Ƶ��׽ӿڵľ����
			SIO_GET_EXTENSION_FUNCTION_POINTER,//�����еĲ����Ŀ��ƴ��룬��������Ҫ�ٿص�����
			&GuidAcceptEx,//���뻺�����ĵ�ַ���������ָ��һ���������׽��ֽ��п��ƣ����������һ��guid��ָ��������ƺ�����guid����
			sizeof(GuidAcceptEx), //���뻺�����Ĵ�С������Ϊguid�Ĵ�С����sizeof(&guid)����
			&m_AcceptEx, //����������ĵ�ַ�����Ｔ�Ǻ���ָ��ĵ�ַ����
			sizeof(m_AcceptEx),//����������Ĵ�С������ָ��Ĵ�С��
			&dwBytes,//���ʵ���ֽ����ĵ�ַ
			NULL,//WSAOVERLAPPED�ṹ�ĵ�ַ��һ��ΪNULL����
			NULL);//һ��ָ�������������õ�����ָ�루һ��ΪNULL����
	}
	if (m_AcceptEx == nullptr || errorcode == SOCKET_ERROR) return -8;

	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
	if (m_GetAcceptEx == NULL)
	{
		errorcode = WSAIoctl(listenfd,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidGetAcceptEx,
			sizeof(GuidGetAcceptEx),
			&m_GetAcceptEx,
			sizeof(m_GetAcceptEx),
			&dwBytes,
			NULL,
			NULL);
	}
	if (m_GetAcceptEx == NULL || errorcode == SOCKET_ERROR) return -9;


	return 0;
}
//��ʼ��Ͷ��
void net::TcpServer::initPost()
{
	//Ͷ������
	for (int i = 0; i < __ServerInfo->MaxAccpet; i++)
		postAccept();
}
s32 net::TcpServer::postAccept()
{
	//1������һ���µ�socketfd 
	SOCKET socketfd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (socketfd == INVALID_SOCKET)
	{
		return -1;
	}
	//2������Ϊ������ģʽ
	ULONG ul = 1;
	int errorCode = ioctlsocket(socketfd, FIONBIO, &ul);
	if (errorCode == SOCKET_ERROR)
	{
		closeSocket(socketfd, nullptr, 1001);
		return -1;
	}
	//3����ȡһ��������ճ����accept����
	AcceptContext* context = AcceptContext::pop();
	context->setSocket(listenfd, socketfd);
	//4��Ͷ��AcceptEx
	unsigned long dwBytes = 0;
	bool isaccept = m_AcceptEx(context->listenfd,        //1������soceket
								context->m_Socket,       //2��������socket
								context->m_buf,          //3�����ܻ����� a���ͻ��˷�����һ������ b��server��ַ c��client��ַ
								0,                       //4��0����ȵ����ݵ���ֱ�ӷ��� ��0�ȴ�����
								sizeof(SOCKADDR_IN) + 16,//5�����ص�ַ��С�����ȱ���Ϊ��ַ���� + 16�ֽ�
								sizeof(SOCKADDR_IN) + 16,//6��Զ�˵�ַ��С�����ȱ���Ϊ��ַ���� + 16�ֽ�
								&dwBytes,                //7��ͬ����ʽ������ �������첽IOû�ã����ùܣ�
								&context->m_OverLapped); //8�������ص�I / O��Ҫ�õ����ص��ṹ
	if (isaccept == false)
	{
		int error = WSAGetLastError();
		if (ERROR_IO_PENDING != error)
		{
			closeSocket(socketfd, nullptr, 1002);
			AcceptContext::push(context);
			return -1;
		}
	}

	return 0;
}

s32 net::TcpServer::onAccpet(void* context)
{
	AcceptContext* acc = (AcceptContext*)context;
	if (acc == nullptr) return -1;

	SOCKADDR_IN * ClientAddr = NULL;
	SOCKADDR_IN * LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	int localLen = sizeof(SOCKADDR_IN);
	int errorCode = 0;

	//1����ȡ�ͻ������ݵ�ַ��Ϣ
	m_GetAcceptEx(acc->m_buf,                 //1��ָ�򴫵ݸ�AcceptEx�������յ�һ�����ݵĻ�����
					0,                        //2����������С������ʹ��ݸ�AccpetEx������һ��
					sizeof(SOCKADDR_IN) + 16, //3�����ص�ַ��С������ʹ��ݸ�AccpetEx����һ��
					sizeof(SOCKADDR_IN) + 16, //4��Զ�̵�ַ��С������ʹ��ݸ�AccpetEx����һ��
					(LPSOCKADDR*)& LocalAddr, //5�������������ӵı��ص�ַ
					&localLen,                //6���������ر��ص�ַ�ĳ���
					(LPSOCKADDR*)& ClientAddr,//7����������Զ�̵�ַ
					&remoteLen);              //8����������Զ�̵�ַ�ĳ���

	//���˵�IP �������
	//int count = addConnectIP(inet_ntoa(ClientAddr->sin_addr));
	//if (count > MAX_IP_ONE_COUNT) return 10;


	//����������SOCKET  һЩ����COPY���½�����SOCKET
	//������ shutdown���û᲻�ɹ�
	setsockopt(acc->m_Socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)& listenfd, sizeof(listenfd));
	//2�����÷��ͽ��ܻ�����
	int rece = __ServerInfo->ReceOne;
	int send = __ServerInfo->SendOne;
	errorCode = setsockopt(acc->m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)& send, sizeof(send));
	errorCode = setsockopt(acc->m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)& rece, sizeof(rece));

	//3��socket���ã���ֹ�����ִ���TIME_WAIT״̬��ʱ������socket
	int nOpt = 1;
	errorCode = setsockopt(acc->m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)& nOpt, sizeof(nOpt));
	if (errorCode == SOCKET_ERROR) return 3;

	//4���������ر�socket��ʱ��,��ִ���������Ĳ����ֹرգ�����ִ��RESET
	int dontLinget = 1;
	errorCode = setsockopt(acc->m_Socket, SOL_SOCKET, SO_DONTLINGER, (char*)& dontLinget, sizeof(dontLinget));
	if (errorCode == SOCKET_ERROR) return 4;

	//5������ngle�㷨
	const char chOpt = 1;
	int   nErr = setsockopt(acc->m_Socket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

	//6������������������
	if (this->setHeartCheck(acc->m_Socket) < 0) return 5;

	//7�������ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)acc->m_Socket, m_Completeport, 0, 0);
	if (hTemp == NULL) return 6;

	//ֱ�Ӷ�λ �������
	S_CLIENT_BASE_INDEX * cindex = getClientIndex(acc->m_Socket);
	if (cindex == nullptr) return 7;

	//���� ���Ǿ���Ҫ�õ��û����������
	S_CLIENT_BASE * c = getFreeLinker();
	if (c == nullptr)
	{
		LOG_MSG("server full...\n");
		return 8;
	}
	//��������
	cindex->index = c->ID;

	

	//�������������û�����
	memcpy(c->ip, inet_ntoa(ClientAddr->sin_addr), MAX_IP_LEN);
	c->socketfd    = acc->m_Socket;
	c->port        = ntohs(ClientAddr->sin_port);
	c->state       = S_SOCKET_STATE::S_Connect;
	c->time_Connet = (s32)time(NULL);
	c->time_Heart  = (s32)time(NULL);

	//LOG_MSG("new connect...%s-%d \n", c->ip, c->port);

	//Ͷ�ݽ�������
	int ret = this->postRecv(acc->m_Socket);
	if (ret != 0)
	{
		//deleteConnectIP(c->ip);

		c->Reset();
		return 9;
	}

	updateConnect(true);
	//����������� ����һ����ȫ����ͻ���
	std::string randomString =ShellRandom::getRandomString(20, 33, 122);
	std::string rcode = myEngine::base64_encode(reinterpret_cast<const unsigned char*>(randomString.c_str()), 20);
	socketprotobuf::sc_send_Rcode r;
	r.set_rcode(rcode);
	this->SendGameDataMessageLite(c->ID, socketprotobuf::cmd_rcode, 0,&r);
	strcpy(c->rCode, randomString.c_str());
	if (onAcceptEvent != nullptr) this->onAcceptEvent(this, c, 0);
	AcceptContext::push(acc);
	this->postAccept();
	return 0;
}
//rece Ͷ������
s32 net::TcpServer::postRecv(SOCKET s)
{
	RecvContext* context = RecvContext::pop();
	context->m_Socket = s;

	unsigned long bytes = 0;
	unsigned long flag = 0;

	int err = WSARecv(context->m_Socket,     //1�� �������׽���
					&context->m_wsaBuf,      //2�� ���ջ�����
					1,                       //3�� wsaBuf������WSABUF�ṹ����Ŀ
					&bytes,                  //4�� ������ղ����������,���غ������������յ����ֽ���
					&flag,                   //5�� ���������׽��ֵ���Ϊ һ������Ϊ0
					&(context->m_OverLapped),//6�� �ص��ṹ
					NULL);                   //7�� һ��ָ����ղ�����������õ����̵�ָ��
	

	if (SOCKET_ERROR == err)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			RecvContext::push(context);
			return -1;
		}
	}
	return 0;
}
//recv ��������
s32 net::TcpServer::onRecv(void* context, s32 recvBytes, u32 tid)
{
	RecvContext* rece = (RecvContext*)context;
	if (rece == NULL) return -1;

	S_CLIENT_BASE * c = client(rece->m_Socket, true);
	if (c == nullptr)
	{
		shutDown(rece->m_Socket, 0, NULL, 1001);
		RecvContext::push(rece);
		return -1;
	}
	c->theadID = tid;
	//��������
	s32 error = onRecv_SaveData(c, rece->m_wsaBuf.buf, recvBytes);
	if (error < 0)
	{
		c->is_RecvCompleted = true;
		shutDown(rece->m_Socket, 0, NULL, 1002);
		RecvContext::push(rece);
		return -1;
	}

	//����Ͷ�ݽ�������
	int ret = postRecv(rece->m_Socket);
	if (ret < 0)
	{
		c->is_RecvCompleted = true;
		shutDown(rece->m_Socket, 0, c, 1003);
		RecvContext::push(rece);
		return -1;
	}
	c->is_RecvCompleted = true;
	RecvContext::push(rece);
	return 0;
}
//������--��������
int TcpServer::onRecv_SaveData(S_CLIENT_BASE* c, char* buf, s32 recvBytes)
{
	if (buf == nullptr) return -1;

	if (c->recv_Head == c->recv_Tail)
	{
		c->recv_Tail = 0;
		c->recv_Head = 0;
	}
	//buff����������
	if (c->recv_Tail + recvBytes > __ServerInfo->ReceMax)
	{
		return -1;
	}
	memcpy(&c->recvBuf[c->recv_Tail], buf, recvBytes);
	c->recv_Tail += recvBytes;
	c->is_RecvCompleted = true;
	return 0;

}




//������ �����첽�ļ���������
s32  TcpServer::postSend(S_CLIENT_BASE* c)
{
	if (!c->is_SendCompleted) return 1;
	if (c->ID < 0 ||
		c->state == S_SOCKET_STATE::S_Free ||
		c->closeState == func::S_CLOSE_SHUTDOWN ||
		c->socketfd == INVALID_SOCKET) return 2;

	if (c->send_Tail <= c->send_Head) return 3;

	s32 sendBytes = c->send_Tail - c->send_Head;
	if (sendBytes <= 0) return 1;
	if (sendBytes > __ServerInfo->SendOne) sendBytes = __ServerInfo->SendOne;
	c->is_SendCompleted = false;

	SendContext * context = SendContext::pop();
	context->setSend(c->socketfd, &c->sendBuf[c->send_Head], sendBytes);

	unsigned long dwBytes = 0;
	unsigned long err = WSASend(context->m_Socket,
								&context->m_wsaBuf,
								1,
								&dwBytes,
								0,
								&context->m_OverLapped,
								NULL);
	int error = WSAGetLastError();
	if (err == SOCKET_ERROR)
	{
		if (error != WSA_IO_PENDING)
		{
			shutDown(context->m_Socket, 0, c, 1004);
			SendContext::push(context);
			return -2;
		}
	}
	return 0;
}


//������ �������ݳɹ�
s32 net::TcpServer::onSend(void* context, s32 sendBytes)
{
	SendContext* sc = (SendContext*)context;
	if (sc == NULL) return -1;

	if (sc->m_wsaBuf.len != sendBytes)
	{
		shutDown(sc->m_Socket, 0, NULL, 1005);
		SendContext::push(sc);
		return -1;
	}
	S_CLIENT_BASE* c = client(sc->m_Socket, true);
	if (c == nullptr)
	{
		shutDown(sc->m_Socket, 0, NULL, 1006);
		SendContext::push(sc);
		return -1;
	}

	if (c->ID < 0 ||
		c->state == S_SOCKET_STATE::S_Free ||
		c->closeState == func::S_CLOSE_SHUTDOWN ||
		c->socketfd == INVALID_SOCKET)
	{
		c->is_SendCompleted = true;
		SendContext::push(sc);
		return -1;
	}
	//���ͳɹ�
	c->send_Head += sendBytes;
	c->is_SendCompleted = true;
	SendContext::push(sc);
	return 0;
}



int net::TcpServer::WindowMsgThread()
{
	try
	{
		//ע�ᴰ����
		LOGBRUSH		LogBrush;
		WNDCLASS		WndClass;
		TCHAR			szClassName[] = TEXT("CMainManageWindow");
		LogBrush.lbColor = RGB(0, 0, 0);
		LogBrush.lbStyle = BS_SOLID;
		LogBrush.lbHatch = 0;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hCursor = NULL;
		WndClass.hIcon = NULL;
		WndClass.lpszMenuName = NULL;
		WndClass.lpfnWndProc = WindowProcFunc;
		WndClass.lpszClassName = szClassName;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;
		WndClass.hInstance = NULL;
		WndClass.hbrBackground = (HBRUSH)::CreateBrushIndirect(&LogBrush);
		::RegisterClass(&WndClass);

		//��������
		m_hWindow = ::CreateWindow(szClassName, NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, this);
		if (m_hWindow == NULL) throw TEXT("���ڽ���ʧ��");
		this->SetTimer(1000, 100);
	}
	catch (...)
	{
		LOG_MSG("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
		//��������
		b_windowRun = FALSE;
		return -1;
	}
	b_windowRun = TRUE;
	MSG	Message;
	while (::GetMessage(&Message, NULL, 0, 0))
	{
		if (!::TranslateAccelerator(Message.hwnd, NULL, &Message))
		{
			::TranslateMessage(&Message);
			::DispatchMessage(&Message);
		}
	}
	return 0;
}


//��ʱ��֪ͨ��Ϣ
bool net::TcpServer::WindowTimerMessage(UINT uTimerID)
{
	if (m_hWindow != NULL)
	{
		bool isAdd = DataContext::addData(m_Completeport, uTimerID, HD_TIMER_MESSAGE);
		if (isAdd) return true;
	}

	return false;
}


LRESULT CALLBACK net::TcpServer::WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:		//���ڽ�����Ϣ
	{
		DWORD iIndex = TlsAlloc();
		TcpServer* _tcpServer = (TcpServer*)((CREATESTRUCT*)lParam)->lpCreateParams;
		TlsSetValue(iIndex, _tcpServer);
		::SetWindowLong(hWnd, GWL_USERDATA, iIndex);
		break;
	}
	case WM_TIMER:		//��ʱ����Ϣ
	{
		DWORD iIndex = ::GetWindowLong(hWnd, GWL_USERDATA);
		TcpServer* _tcpServer = (TcpServer*)::TlsGetValue(iIndex);
		if ((_tcpServer != NULL) && (_tcpServer->WindowTimerMessage((UINT)wParam) == false)) ::KillTimer(hWnd, (UINT)wParam);
		break;
	}
	case WM_CLOSE:		//���ڹر���Ϣ
	{
		DestroyWindow(hWnd);
		break;
	}
	case WM_DESTROY:	//���ڹر���Ϣ
	{
		DWORD iIndex = ::GetWindowLong(hWnd, GWL_USERDATA);
		TcpServer* _tcpServer = (TcpServer*)::TlsGetValue(iIndex);
		if (_tcpServer != NULL) _tcpServer->m_hWindow = NULL;
		::TlsFree(iIndex);
		PostQuitMessage(0);
		break;
		}
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool net::TcpServer::SetTimer(UINT uTimerID, UINT uElapse)
{
	if ((m_hWindow != NULL) && (IsWindow(m_hWindow) == TRUE))
	{
		::SetTimer(m_hWindow, uTimerID, uElapse, NULL);
		return true;
	}
	return false;
}

//�����ʱ��
bool TcpServer::KillTimer(UINT uTimerID)
{
	if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE))
	{
		::KillTimer(m_hWindow, uTimerID);
		return true;
	}
	return false;
}





//**************************************************************
//**************************************************************
//**************************************************************
s32 TcpServer::closeSocket(SOCKET socketfd, S_CLIENT_BASE* c, int kind)
{
	if (socketfd == SOCKET_ERROR || socketfd == INVALID_SOCKET) return -1;

	int sfd = socketfd;
	int shutkind = 0;
	if (c != nullptr)
	{
		if (c->state == func::S_Free) return 0;
		if (c->state >= func::S_ConnectSecure)
		{
			this->updateRecurityConnect(false);
		}
		shutkind = c->shutdown_kind;

		//����ͻ�������
		clearClientConnection(c->clientID);
	}
	//��������������
	switch (kind)
	{
	case 1001:
	case 1002:
	case 3004:
		RELEASE_SOCKET(socketfd);
		break;
	default:
		//if (c != nullptr) deleteConnectIP(c->ip);
		this->updateConnect(false);
		shutdown(socketfd, SD_BOTH);
		RELEASE_SOCKET(socketfd);
		break;
	}
	


	if (onDisconnectEvent != nullptr) this->onDisconnectEvent(this, c, kind);
	//��ʼ��
	//if (c != nullptr)
	//{
	//	if (c->state == S_SOCKET_STATE::S_Connect ||
	//		c->state == S_SOCKET_STATE::S_ConnectSecure)
	//	{
	//		c->Reset();
	//	}
	//}
	//int a = m_ConnectCount;
	//int b = m_SecurityCount;
	//const char* str1 = func::getShutDownError(shutkind);
	//const char* str2 = func::getCloseSocketError(kind);
	//printf("closeSocket-%d [%s::%s][con:%d-%d]\n", sfd, str1, str2, a, b);
	return 0;
}


void net::TcpServer::shutDown(SOCKET s, const s32 mode, S_CLIENT_BASE* c, int kind)
{
	if (c != nullptr)
	{
		if (c->state == func::S_Free) return;
		if (c->closeState == SOCKET_CLOSE::S_CLOSE_SHUTDOWN) return;

		c->shutdown_kind = kind;
		c->time_Close = (s32)time(NULL);
		c->closeState = SOCKET_CLOSE::S_CLOSE_SHUTDOWN;
		memset(c->rCode,0,21);
		shutdown(s, SD_BOTH);
		//ȡ��IO����
		CancelIoEx((HANDLE)s, nullptr);

		if (onExceptEvent != nullptr) this->onExceptEvent(this, c, kind);
		return;
	}

	auto c2 = client(s, true);
	if (c2 == nullptr)
	{
		printf("find socketfd is error:client2=null %d- kind:%d line:%d\n", (int)s, kind, __LINE__);
		return;
	}
	if (c2->state == func::S_Free) return;
	if (c2->closeState == SOCKET_CLOSE::S_CLOSE_SHUTDOWN) return;

	switch (mode)
	{
	case func::SC_WAIT_RECV:
		c2->is_RecvCompleted = true;
		break;
	case func::SC_WAIT_SEND:
		c2->is_SendCompleted = true;
		break;
	}

	c2->shutdown_kind = kind;
	c2->time_Close = (s32)time(NULL);
	c2->closeState = SOCKET_CLOSE::S_CLOSE_SHUTDOWN;
	shutdown(s, SD_BOTH);

	if (onExceptEvent != nullptr) this->onExceptEvent(this, c2, kind);
}



#endif