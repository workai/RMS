#include "ServerSocket.h"

#define PACKHEAD_LEN			6
#define	MAXDELAYTIME			2		// 最大延时2秒
#define MAX_LEN					4096	// 最大长度

extern int scount;
extern int ocount;

CServerSocket::CServerSocket(void)
{
	m_bInit = FALSE;
	m_hSocket = NULL;
	m_hClientSocketLast = NULL;
	m_uPort = 0;

	InitSocket();

	InitializeCriticalSection(&m_CriticalSection);
}

CServerSocket::~CServerSocket(void)
{
	for (itSvrSocket itsvrsock=m_mapSvrSocket.begin();
		 itsvrsock!=m_mapSvrSocket.begin(); ++itsvrsock)
	{
		LPSvrSocket pSvrSocket = (LPSvrSocket)itsvrsock->second;
		CloseClientSocket(pSvrSocket->sock);
	}

	WSACleanup();

	DeleteCriticalSection(&m_CriticalSection);
}

// 初始化
int CServerSocket::InitSocket()
{

	WSADATA wsaData;
	WORD version = MAKEWORD(2,2);

	int ret = WSAStartup(version, &wsaData);
	if (ret == 0)
	{
		m_bInit = TRUE;
	}

	return ret;
}

// 初始化
int CServerSocket::InitAndListen(UINT nPort)
{
	SOCKET tmpsock;

	if (!m_bInit)
		return -2;

	if (m_hSocket == NULL)
	{
		m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//ASSERT(m_hSocket != NULL);

		m_addr.sin_family = AF_INET;
		m_addr.sin_addr.S_un.S_addr = INADDR_ANY;
		m_addr.sin_port = htons(nPort);

		int ret = 0;
		int error = 0;
		
		//int bufsize=0;
		//int oplen=sizeof(int);
		//if(getsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, &oplen) == SOCKET_ERROR)
		//{
		//	GetError(GetLastError());
		//	return -1;
		//}
		//int revbufsize = 1024000;
		//if(setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&revbufsize, oplen) == SOCKET_ERROR)
		//{
		//	GetError(GetLastError());
		//	return -1;
		//}
		//if(getsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, &oplen) == SOCKET_ERROR)
		//{
		//	GetError(GetLastError());
		//	return -1;
		//}

		// 绑定一个套接字到本机地址
		ret = bind(m_hSocket, (LPSOCKADDR)&m_addr, sizeof(m_addr));

		if (ret == SOCKET_ERROR)
		{
			closesocket(m_hSocket);
			m_hSocket = NULL;
			GetError(GetLastError());
			return -1;
		}

		// 开始侦听过程，等待客户的连接
		ret = listen(m_hSocket, 5);
		if (ret == SOCKET_ERROR)
		{
			closesocket(m_hSocket);
			m_hSocket = NULL;
			GetError(GetLastError());
			return -1;
		}
	}

	SOCKADDR_IN saddr;
	int len = sizeof(saddr);
	tmpsock = accept(m_hSocket, (PSOCKADDR)&saddr, &len);

	if (tmpsock == INVALID_SOCKET)
	{
		GetError(GetLastError());
		return -1;
	}

	// 连接数是否最大
	EnterCriticalSection(&m_CriticalSection);

	if (m_mapSvrSocket.size() < MAX_CONNECTION_NUMBER)
	{
		m_hClientSocketLast = tmpsock;

		LPSvrSocket pSvrSocket = new SvrSocket;
		pSvrSocket->sock = tmpsock;
		pSvrSocket->port = saddr.sin_port;
		pSvrSocket->ip = saddr.sin_addr;

		m_mapSvrSocket.insert(vtSvrSocket(tmpsock, pSvrSocket));
		LeaveCriticalSection(&m_CriticalSection);
	}
	else
	{
		GetError(100);
		LeaveCriticalSection(&m_CriticalSection);
		return -3;
	}

	return 0;
}

// 检测套接字是否可读
int CServerSocket::CheckSocketReadable(SOCKET sock)
{
	fd_set fdset;
	struct timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=5;
	FD_ZERO(&fdset);
	FD_SET(sock,&fdset);
	int iRet = select(sock+1, &fdset, NULL, NULL, &tv);
	if (iRet == 0)
	{
		return -1;
	}

	return 0;
}

// 检测套接字是否可写
int CServerSocket::CheckSocketWriteable(SOCKET sock)
{
	fd_set fdset;
	struct timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=5;
	FD_ZERO(&fdset);
	FD_SET(sock,&fdset);
	int iRet = select(sock+1, NULL, &fdset, NULL, &tv);
	if (iRet == 0)
	{
		return -1;
	}

	return 0;
}

// 发送消息
int CServerSocket::SendMsg(char *buff, USHORT len, SOCKET sock)
{
	//ASSERT(buff != NULL);
	if (!m_bInit)
		return -1;

	int end = send(sock, buff, len, 0);
	if (end == SOCKET_ERROR)
	{
		GetError(GetLastError());
		return -1;
	}
	else if(end < len)
	{
		return -2;
	}

#ifdef _DEBUG
	ocount++;
#endif

	return end;
}

// 接收消息
int CServerSocket::ReceiveMsg(char* buff, SOCKET sock)
{
	if (!m_bInit)
		return -4;

	int	iTotal = 0;
	int	iRet = 0;
	time_t startTime = time(NULL);

	while (1)
	{
		iRet = recv(sock, buff + iTotal, 1, 0);
		if (iRet <= 0)
		{
			GetError(GetLastError());
			return -1;
		}

		if (*(int*)buff == 0xCD)
		{
			iTotal = 1;

			iRet = recv(sock, buff + iTotal, 1, 0);

			if (*(int*)(buff+1) == 0xAB)
			{
				iTotal = 2;
				break;
			}
		}

		iTotal = 0;
		if (time(NULL) - startTime > MAXDELAYTIME)
		{
			printf("接收报头文超时\n");
			return 0;
		}
	}

	iRet = recv(sock, buff + iTotal, 6, 0);
	if (iRet <= 0)
	{
		GetError(GetLastError());
		printf("客户端%d已经关闭\n", sock);
		return -1;
	}

	iTotal = iTotal + iRet;
	USHORT nDataLen = *(USHORT*)(buff + 6);	// 数据长度
	if (nDataLen > MAX_LEN)
	{
		printf("数据长度不对\n");
		return 0;
	}

	iRet = 0;
	int	nCheck = 0;

	while (nCheck < nDataLen)
	{
		iRet = recv(sock, buff + iTotal, nDataLen - nCheck, 0);
		if (iRet <= 0)
		{
			GetError(GetLastError());
			printf("客户端%d已经关闭\n", m_hSocket);
			return -1;
		}

		iTotal = iTotal + iRet;
		nCheck += iRet;

		if (time(NULL) - startTime > 5)
		{
			printf("接收数据区超时\n");
			return 0;
		}
	}

	return iTotal;
}

// 服务器接收消息函数
void CServerSocket::GetSvrSockMap(MapSvrSocket & mapSevSocket)
{
	EnterCriticalSection(&m_CriticalSection);
	mapSevSocket = m_mapSvrSocket;
	LeaveCriticalSection(&m_CriticalSection);
}

// 关闭Socket连接
void CServerSocket::CloseClientSocket(SOCKET sock)
{
	EnterCriticalSection(&m_CriticalSection);

	itSvrSocket itSocket = m_mapSvrSocket.find(sock);
	if (itSocket != m_mapSvrSocket.end())
	{
		delete (LPSvrSocket)itSocket->second;
		m_mapSvrSocket.erase(itSocket);
	}

	closesocket(sock);

	LeaveCriticalSection(&m_CriticalSection);
}

// 显示错误原因
void CServerSocket::GetError(DWORD error)
{
	char strError[256] = {'\0'};

	switch(error)
	{
	case WSANOTINITIALISED:
		strcpy_s(strError,"初始化错误");
		break;
	case WSAENOTCONN:
		strcpy_s(strError,"对方没有启动");
		break;
	case WSAEWOULDBLOCK :
		strcpy_s(strError,"对方已经关闭");
		break;
	case WSAECONNREFUSED:
		strcpy_s(strError,"连接的尝试被拒绝");
		break;
	case WSAENOTSOCK:
		strcpy_s(strError,"在一个非套接字上尝试了一个操作");
		break;
	case WSAEADDRINUSE:
		strcpy_s(strError,"特定的地址已在使用中");
		break;
	case WSAECONNRESET:
		strcpy_s(strError,"与主机的连接被关闭");
		break;
	case 2:
		strcpy_s(strError,"对方已经有一个套接字在连接状态，对于同一个用户不能进行多个套接字连接。");
		break;
	case 3:
		strcpy_s(strError,"套接字非空，检查是否已经使用该socket进行了侦听？");
		break;
	case 100:
		strcpy_s(strError,"连接队列已满，请稍候!");
		break;
	case 101:
		strcpy_s(strError,"队列空，操作非法");
		break;
	default:
		strcpy_s(strError,"低级错误");
		break;
	}
	
	//g_pLog->WriteLog("SOCKET 错误：%s\r\n", strError);
}