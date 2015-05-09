#pragma once

#include "ReformMediaServerDef.h"
#include <map>

using namespace std;

#define MAX_CONNECTION_NUMBER	32		// 最大连接数

typedef struct _SvrSocket
{
	SOCKET	sock;		// Socket
	IN_ADDR ip;			// IP地址
	int		port;		// Socket端口号
	BOOL	bRegister;	// 注册

	_SvrSocket()
	{
		sock		= NULL;
		port		= 0;
		bRegister	= false;
		// memset(ip, 0, sizeof(ip));
	}
} SvrSocket, *LPSvrSocket;

typedef map<SOCKET, LPSvrSocket> MapSvrSocket,*LPMapSvrSocket;
typedef MapSvrSocket::value_type vtSvrSocket;
typedef MapSvrSocket::iterator itSvrSocket;

class CServerSocket
{
public:
	CServerSocket(void);
public:
	~CServerSocket(void);

public:
	SOCKET m_hSocket;					// 监听socket 句柄
	sockaddr_in m_addr;					// 每次客户端连接时，客户端的地址
	UINT m_uPort;						// 端口号
	BOOL m_bInit;						// 初始化winsock的信号
	MapSvrSocket m_mapSvrSocket;		// 客户端的winsock的map
	SOCKET m_hClientSocketLast;			// 客户端最后一次连接的socket 句柄

	CRITICAL_SECTION m_CriticalSection;

public:
	int InitAndListen(UINT nPort);
	virtual int CheckSocketReadable(SOCKET sock);
	virtual int CheckSocketWriteable(SOCKET sock);
	virtual int SendMsg(char *buff, USHORT len, SOCKET sock);
	virtual int ReceiveMsg(char* buff, SOCKET sock);
	virtual void CloseClientSocket(SOCKET sock);
	void GetSvrSockMap(MapSvrSocket & mapSevSocket);

private:
	int InitSocket();
	void GetError(DWORD error);
};