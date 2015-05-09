#ifndef _PUSHSERVER_H_
#define _PUSHSERVER_H_

#include <WinSock2.h>
#include <Windows.h>
#include <hash_map>
#include "CLock.h"

using namespace std;

class CPushServer
{
private:
	static bool _isRun;
	static SOCKET _bind_socket;
	static int _port;
	static fd_set _rfds;
	static CLock _lock;
	static hash_map<SOCKET, int> _ClientList;
	static hash_map<int, SOCKET> _ReadyList;

private:
	static unsigned int WINAPI _run(LPVOID inThread);
	static void setfds();
	static BOOL recvMSG(SOCKET s, char* buf, const int buf_len);

public:
	static BOOL init(int port);
	static BOOL pushMSG(const int session, const char* buf, const int len);
	static BOOL isReady(const int session);
	static BOOL getPort(int & port)
	{
		if(_isRun)
		{
			port = _port;
			return TRUE;
		}
		return FALSE;
	}
};

#endif