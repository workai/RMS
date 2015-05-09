#include "PushServer.h"
#include <process.h>

bool CPushServer::_isRun = false;
int CPushServer::_port;
SOCKET CPushServer::_bind_socket = INVALID_SOCKET;
fd_set CPushServer::_rfds;
hash_map<SOCKET, int> CPushServer::_ClientList;
hash_map<int, SOCKET> CPushServer::_ReadyList;

BOOL CPushServer::init(int port)
{
	_port = port;
	_bind_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(_bind_socket == INVALID_SOCKET)
	{
		//g_pLog->WriteLog("Init MediaServer Fail:%d\n", GetLastError());
		return FALSE;
	}
	unsigned long ul = 1;
	int nRet = ioctlsocket(_bind_socket, FIONBIO, (unsigned long *)&ul);
	if(nRet == SOCKET_ERROR)
	{
		//g_pLog->WriteLog("rtspserv initialization failure(%d)!\n", GetLastError());
		closesocket(_bind_socket);
		_bind_socket = INVALID_SOCKET;
		return FALSE;
	}
	struct sockaddr_in server_sockaddr;						// 服务器的RTSP地址信息    
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(port);	// RTSP的端口
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(_bind_socket,(struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == SOCKET_ERROR)
	{
		DWORD err = GetLastError();
		closesocket(_bind_socket);
		_bind_socket = INVALID_SOCKET;
		//g_pLog->WriteLog("Init MediaServer Fail:bind[%d](%d)\n", port, err);
		return FALSE;
	}
	if(listen(_bind_socket, 100) == SOCKET_ERROR)
	{	
		//g_pLog->WriteLog("Init MediaServer Fail:listen(%d)\n", GetLastError());
		closesocket(_bind_socket);
		_bind_socket = INVALID_SOCKET;
		return FALSE;
	}
	
	unsigned int theId = 0;
	_beginthreadex( NULL, 0,  _run, NULL, 0, &theId );
	return FALSE;
}

unsigned int WINAPI CPushServer::_run(LPVOID inThread)
{
	_isRun = true;
	int ret = 0;
	int err = 0;
	while(_isRun)
	{
		setfds();
		ret = select(_bind_socket+1, &_rfds, 0, 0, 0);
		if( ret == SOCKET_ERROR)
		{
			DWORD err = GetLastError();
			if(err == WSAENOTSOCK)
			{
			}
			else
			{
				closesocket(_bind_socket);
				_bind_socket = INVALID_SOCKET;
				//g_pLog->WriteLog("RecvThread Exit:%d\n", GetLastError());
				break;
			}
		}
		else if(ret == 0)
			continue;

		//new connect
		if(FD_ISSET(_bind_socket,&_rfds))
		{
			int s = accept(_bind_socket, 0, 0);
			if(s == INVALID_SOCKET)
				err = WSAGetLastError();
			else
			{
				_ClientList[s] = -1;
			}
		}
		else
		{
			//receive from client
			for(hash_map<SOCKET, int>::iterator iter = _ClientList.begin();iter!=_ClientList.end();iter++)
			{
				SOCKET s = iter->first;
				if(FD_ISSET(s,&_rfds))
				{
					if(iter->second == -1)
					{
						iter->second = 1;
						//recv session
						int session=0;
						if(recvMSG(s, (char*)&session, sizeof(session)))
						{
							_ReadyList[session] = s;
						}
						else
						{
							closesocket(s);
							_ClientList.erase(iter);
						}
					}
					else
					{
						//recv and discard
						char buf[1024]={'\0'};
						if(recvMSG(s, buf, 1024) == -1)
						{
							closesocket(s);
							_ClientList.erase(iter);
						}
					}
					break;
				}			
			}
		}
	}
	_isRun = false;
	return 0;
}

BOOL CPushServer::pushMSG(const int session, const char* buf, const int len)
{
	hash_map<int, SOCKET>::iterator iter = _ReadyList.find(session);
	if(iter != _ReadyList.end())
	{
		//send
		if(send(iter->second, buf, len, 0) == len)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

void CPushServer::setfds()
{
	FD_ZERO(&_rfds);
	if(_bind_socket != INVALID_SOCKET)
		FD_SET(_bind_socket, &_rfds);
	for(hash_map<SOCKET, int>::iterator iter = _ClientList.begin();iter!=_ClientList.end();)
	{
		if(iter->first != INVALID_SOCKET)
		{
			FD_SET(iter->first, &_rfds);
			iter++;
		}
		else
		{
			iter = _ClientList.erase(iter);
		}
	}
}

BOOL CPushServer::recvMSG(SOCKET s, char* buf, int buf_len)
{
	if(buf_len == 0)
		return TRUE;
	int ret = recv(s, buf, buf_len, 0);
	if(ret == buf_len)
		return TRUE;
	else if(ret <= 0)
		return -1;
	return FALSE;
}

BOOL CPushServer::isReady(const int session)
{
	hash_map<int, SOCKET>::iterator iter = _ReadyList.find(session);
	if(iter != _ReadyList.end())
	{
		return TRUE;
	}
	return FALSE;
}

