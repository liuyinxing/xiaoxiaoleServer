#include "TcpClient.h"

#ifndef ____WIN32_
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netinet/tcp.h>
#endif
#include <string.h>


namespace net
{
	ITcpClient* NewTcpClient()
	{
		return new TcpClient();
	}

	TcpClient::TcpClient()
	{
#ifdef ____WIN32_
		socketfd = INVALID_SOCKET;
#else
		socketfd = -1;
#endif

		onAcceptEvent = nullptr;
		onSecureEvent = nullptr;
		onDisconnectEvent = nullptr;
		onExceptEvent = nullptr;

	
	}

	TcpClient::~TcpClient()
	{
#ifdef ____WIN32_
		RELEASE_SOCKET(socketfd);
		WSACleanup(); //�ͷ��׽�����Դ;  
#else
		close(socketfd);
#endif

		
	}

	//���пͻ���
	void TcpClient::runClient(u32 sid, char* ip, int port)
	{
		m_data.init(sid);
		//����ip �˿�
		m_data.time_AutoConnect = 0;
		if (ip != NULL)  strcpy(m_data.ip, ip);
		if (port > 0)    m_data.port = port;

		s32 err = initSocket();
		if (err < 0)
		{
			LOG_MSG("--------------client is error:%d...", err);
			return;
		}

		runThread();
		initCommands();
	}

	//���÷�����
	bool TcpClient::setNonblockingSocket()
	{
#ifdef ____WIN32_
		unsigned long ul = 1;
		s32 errorcode = ioctlsocket(socketfd, FIONBIO, (unsigned long*)& ul);
		if (errorcode == SOCKET_ERROR)
		{
			LOG_MSG("--------------client errorcode is error..%d line:%d", errorcode, __LINE__);
			return -1;
		}
#else
		//fcntlϵͳ���ÿ����������Ѵ򿪵��ļ����������и��ֿ��Ʋ����Ըı��Ѵ��ļ��ĵĸ�������
		int flags = fcntl(socketfd, F_GETFL);//��ȡ�ļ��򿪷�ʽ�ı�־����־ֵ������open����һ��
		if (flags < 0)
			return false;
		flags |= O_NONBLOCK;
		if (fcntl(socketfd, F_SETFL, flags) < 0) //�����ļ�״̬��־
			return false;
		return true;
#endif
	}

//�ͻ��˱�̵Ĳ��裺
//(1)�������׽��ֿ⣬�����׽���(WSAStartup() / socket())��
//(2)���������������������(connect())��
//(3)���ͷ������˽���ͨ��(send() / recv())��
//(4)���ر��׽��֣��رռ��ص��׽��ֿ�(closesocket() / WSACleanup())��

	s32 TcpClient::initSocket()
	{
#ifdef ____WIN32_
		//1����ʼ��Windows Sockets DLL
		WSADATA  wsData;
		s32 errorcode = WSAStartup(MAKEWORD(2, 2), &wsData);
		if (errorcode != 0) 
		{
			WSACleanup();
			return -1;
		}
#endif
		//1�������׽���  
		socketfd = socket(AF_INET, SOCK_STREAM, 0);
		if (socketfd < 0)
		{
			int fd = socketfd;
			LOG_MSG("--------------client socket is error..%d line:%d", fd, __LINE__);
			return -1;
		}
		//2�������׽��ַ�����ģʽ
		setNonblockingSocket();
		//3�����÷��ͽ��ܻ�����
		int rece = func::__ClientInfo->ReceOne;
		int send = func::__ClientInfo->SendOne;
		setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char*)& rece, sizeof(rece));
		setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char*)& send, sizeof(send));

		return 0;
	}




	//select��������ֵ����ֵ��select���� ��ֵ��ĳЩ�ļ��ɶ�д����� 0���ȴ���ʱ��û�пɶ�д�������ļ�
	void TcpClient::connect_Select()
	{
		struct timeval tv;
		tv.tv_sec  = 5;
		tv.tv_usec = 0;

		fd_set wset;
		FD_ZERO(&wset);
		FD_SET(socketfd, &wset);

		int error = select(socketfd + 1, NULL, &wset, NULL, &tv);
		if (error <= 0)
		{
			this->disconnectServer(1008, "select timeout");
			return;
		}
		
		if (FD_ISSET(socketfd, &wset)) connectServer();
	}

	bool TcpClient::connectServer()
	{
		if (m_data.port < 1000) return false;

		m_data.state = func::C_ConnectTry;
		//1����Ҫ���ӵķ�����׽��ֽṹ��Ϣ  
		struct sockaddr_in addrServer;
#ifdef ____WIN32_

		addrServer.sin_addr.S_un.S_addr = inet_addr(m_data.ip);
#else
		addrServer.sin_addr.s_addr = inet_addr(m_data.ip);
#endif
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(m_data.port);//ʹ�������ֽ���

		//2�����ӷ�����
		int value = connect(socketfd, (struct sockaddr*)&addrServer, sizeof(addrServer));
		if (value == 0)//���ӳɹ�
		{
			m_data.state = func::C_Connect;
			if (onAcceptEvent != nullptr) onAcceptEvent(this, 1);

			//5������ngle�㷨
			const char chOpt = 1;
			int   nErr = setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

			//LOG_MSG("--------------client connect success...%d-%d\n", m_data.serverID, m_data.port);
			return true;
		}
		//�������Ӵ���
		if (value < 0)
		{
#ifdef ____WIN32_
			int err = WSAGetLastError();

			//ָ����һ����Ч����
			if (WSAEINVAL == err)
			{
				LOG_MSG("--------------client ָ����һ����Ч����.%d-%d\n", m_data.serverID, m_data.port);
				return false;
			}
			//��˼�������������������ɣ�����������
			if (WSAEWOULDBLOCK == err)
			{
				connect_Select();
				return false;
			}
			//�����Ѿ����
			if (WSAEISCONN == err)
			{
				m_data.state = func::C_Connect;
				LOG_MSG("--------------client connect success2...%d-%d\n", m_data.serverID, m_data.port);
				if (onAcceptEvent != nullptr)  onAcceptEvent(this, 2);

				//5������ngle�㷨
				const char chOpt = 1;
				int   nErr = setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));
				return true;
			}
#else
			//LOG_MSG("connect socket error:%d \n", errno);

			//EINTR ��ʾ��ʲô�ź��ж�
			//��Ӧ�ó�����socket������O_NONBLOCK���Ժ�������ͻ��汻ռ����
			//send�ͻ᷵��EAGAIN��EWOULDBLOCK�Ĵ���
			//�ڽ�socket����O_NONBLOCK���Ժ�ͨ��socket����һ��100K��С�����ݣ�
			//��һ�γɹ�������10K���ݣ�֮��������Ͳ�δ�ɹ���errno��ֵΪEAGAIN����
			//����recv ��������û������ ��������
			//
			if (errno == EINTR || errno == EAGAIN)
			{
				return false;
			}
			//��˼�������������������ɣ�����������
			if (errno == EINPROGRESS)
			{
				connect_Select();
				return false;
			}
			if (errno == EISCONN)
			{
				m_data.state = func::C_Connect;
				if (onAcceptEvent != nullptr)  onAcceptEvent(this, 2);
				return true;
			}
#endif

			this->disconnectServer(1009, "[connect fail]");
			return false;
		}

		return false;
	}
	void TcpClient::disconnectServer(const s32 errcode, const char* err)
	{
		if (m_data.state == func::C_Free) return;
		if (socketfd == -1) return;
#ifdef ____WIN32_
		if (socketfd == INVALID_SOCKET) return;
		RELEASE_SOCKET(socketfd);
#else
		close(socketfd);
		socketfd = -1;
#endif
		m_data.reset();
		initSocket();
		//LOG_MSG("---------client disconnectServer %d-%s \n", errcode, err);
		if (onDisconnectEvent != nullptr) onDisconnectEvent(this, errcode);
	}

	//�Զ�����
	void  TcpClient::onAutoConnect()
	{
		if (m_data.port < 1000) return;

		auto c = getData();
		s32 tempTime = (s32)time(NULL) - m_data.time_AutoConnect;
		if (tempTime >= func::__ClientInfo->AutoTime)
		{
			c->reset();

			int fd = socketfd;
			LOG_MSG("----------�Զ����ӷ�������... \n");
			connectServer();
			m_data.time_AutoConnect = (s32)time(NULL);
		}
	}


	//��������
	int TcpClient::onRecv()
	{

		auto c = this->getData();
		memset(c->recvBuf_Temp, 0, func::__ClientInfo->ReceOne);
		int receBytes = recv(socketfd, c->recvBuf_Temp, func::__ClientInfo->ReceOne, 0);
		if (receBytes > 0)
		{
			//��������
			int err = onSaveData(receBytes);
			if (err < 0)
			{
				this->disconnectServer(1007, "recvFull...");
				return err;
			}
			return 0;
		}
		if (receBytes == 0) //�������ر�����
		{
			this->disconnectServer(1003, "receBytes=0...");
			return -1;
		}

#ifdef ____WIN32_
		//�������ݳ���
		if (receBytes < 0)
		{
			int err = WSAGetLastError();
			switch (err)
			{
			case WSAEINTR:
			case WSAEWOULDBLOCK:
				return 0;
			default:
				LOG_MSG("recv error...%d-%d \n", (int)c->socketfd, err);
				this->disconnectServer(1001, "[recv error]");
				return -1;
			}
		}
#else
		//�������ݳ���
		if (receBytes < 0)
		{
			switch (errno)
			{
			case EINTR://�ź�   
			case EAGAIN:
				return 0;
			default:
				this->disconnectServer(1001, "server close-1");
				return -1;
			}
		}
#endif
	}
	//������ ��������
	int TcpClient::onSaveData(int recvBytes)
	{
		auto c = this->getData();
		if (c->recv_Tail == c->recv_Head)
		{
			c->recv_Tail = 0;
			c->recv_Head = 0;
		}
		if (c->recv_Tail + recvBytes >= func::__ClientInfo->ReceMax)
		{
			printf("----------client .rece full %d-%d.. \n", c->recv_Head, c->recv_Tail);
			return -1;
		}
		if (c->recv_Head > c->recv_Tail)
		{
			printf("----------client .B>E %d-%d.. \n", c->recv_Head, c->recv_Tail);
		}

		memcpy(&c->recvBuf[c->recv_Tail], c->recvBuf_Temp, recvBytes);
		c->recv_Tail += recvBytes;
		return 0;
	}
	//������ ��������
	int TcpClient::onSend()
	{
		auto c = this->getData();

		if (c->send_Tail <= c->send_Head) return 0;
		if (c->state < func::C_Connect) return -1;

		int sendlen = c->send_Tail - c->send_Head;
		if (sendlen < 1) return -1;

		//��������
		int sendbytes = send(socketfd, &c->sendBuf[c->send_Head], sendlen, 0);
		if (sendbytes > 0)
		{
			c->send_Head += sendbytes;
			return 0;
		}
#ifdef ____WIN32_
		if (sendbytes < 0)
		{
			int errcode = WSAGetLastError();
			switch (errcode)
			{
			case WSAEINTR:
			case WSAEWOULDBLOCK:
				return 0;
			default:
				this->disconnectServer(4002, "send close-1");
				return -1;
			}
		}
#else
		if (sendbytes < 0)
		{
			switch (errno)
			{
			case EINTR://�ź�   
			case EAGAIN:
				return 0;
			default:
				this->disconnectServer(4002, "send close-1");
				return -1;
			}
		}
#endif
	}



}

