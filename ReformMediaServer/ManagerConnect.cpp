// ManagerConnect.cpp: implementation of the CManagerConnet class.
//
//////////////////////////////////////////////////////////////////////

#include "StdHeader.h"
#include "ManagerConnect.h"

#include "VideoSend.h"
//#include "FileServerDlg.h"
#include "Log.h"
#include "SysInfo.h"
#include "ManagerRtsp.h"

#include "Channel.h"
#include "ManagerDVR.h"
#include "DVR.h"



//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CManagerConnect::CManagerConnect()
{
	m_clientnum = 0;
	int i;
	for(i = 0; i < MAX_CLIENT_SIZE; i++)
	{
		m_SocketArray[i] = NULL;
	}
}

CManagerConnect::~CManagerConnect()
{
	if (m_Listen != INVALID_SOCKET)
	{
		closesocket(m_Listen);
		m_Listen = INVALID_SOCKET;
	}


//	WaitForSingleObject( m_VideoSendMutex ,  INFINITE ) ;
	int i;
	for( i = 0; i < MAX_CLIENT_SIZE; i++)
	{
		if(m_SocketArray[i] != NULL)
		{
			delete m_SocketArray[i];
			m_SocketArray[i] = NULL;
		}
	}
//	ReleaseMutex( m_VideoSendMutex ) ;
}

BOOL CManagerConnect::OnAccept()
{
	CVideoSend* pConnect = new CVideoSend();
	pConnect->InitSock();
	int sendid = -1;

	if(!AddClient(pConnect, sendid))
	{
		CString csAdd;
		unsigned int iPort;
		pConnect->GetPeerName(csAdd,iPort);
		g_pLog->WriteLog("ERROR：MC添加连接:ip = %s,port = %d\n",csAdd,iPort);

		delete pConnect;
		pConnect = NULL;		
		return FALSE;
	}

	SOCKET sock;
	sock = accept(m_Listen, NULL,0);

	if(pConnect->AcceptSock(sock, sendid))
	{
 		CString csAdd;
		unsigned int iPort;

		pConnect->GetPeerName(csAdd,iPort);
		pConnect->m_csAdd=csAdd;
		pConnect->m_sockid = sendid;

//		strcpy(pConnect->m_csAdd,csAdd.GetBuffer(csAdd.GetLength()));
		pConnect->m_Port = iPort;
		g_pLog->WriteLog("MC添加连接:ip = %s,port = %d, sock = %d, num:%d\n",csAdd,iPort,sock, sendid);
	}
	else
	{
		delete pConnect;
		pConnect = NULL;
		return FALSE;
	}

	return TRUE;
}

BOOL CManagerConnect::InitListenSocket()
{
////////VIDEO
	if((m_Listen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED) ) == INVALID_SOCKET)
	{
		int hr;
		hr = WSAGetLastError();
		g_pLog->WriteLog("创建socket失败(%d)\n", hr);
		WSACleanup();
		return FALSE;
	}

	unsigned long ul = 1;
	int nRet = ioctlsocket(m_Listen,FIONBIO,(unsigned long *)&ul);

	if(nRet == SOCKET_ERROR)
	{
		int hr;
		hr = WSAGetLastError();
		g_pLog->WriteLog("设置socket异步失败(%d)\n", hr);
		closesocket(m_Listen);
		WSACleanup();
		return FALSE;
	}

	local.sin_family = AF_INET;
	local.sin_port = htons(g_pSysInfo->m_FilePort);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(m_Listen,(struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		int hr;
		hr = WSAGetLastError();
		g_pLog->WriteLog("监听端口(%d)失败(%d)\n", g_pSysInfo->m_FilePort, hr);
		closesocket(m_Listen);
		WSACleanup();
		return FALSE;
	}
	g_pLog->WriteLog("Connect端口创建成功! 端口:%d\n",g_pSysInfo->m_FilePort);
	WSAAsyncSelect(m_Listen,g_hWnd,WM_LISTEN_SOCKET,FD_ACCEPT|FD_CLOSE);

	int re = listen(m_Listen,50);

	return TRUE;
}

/*
void CManagerConnect::StopFileDown()
{
	WaitForSingleObject( m_VideoSendMutex ,  INFINITE ) ;
	for(int i = 0; i < MAX_CLIENT_SIZE; i++)
	{
		if(m_SocketArray[i] != NULL)
		{
			if(m_SocketArray[i]->m_SendType == FILE_DOWN )
			{
				delete m_SocketArray[i];
				m_SocketArray[i] = NULL;
				break;
			}
		}
	}
	ReleaseMutex( m_VideoSendMutex ) ;
}
*/

//void CManagerConnect::stop_Client(int index)
//{
//	if((index < 0)||(index >= MAX_CLIENT_SIZE))
//	{
//		g_pLog->WriteLog("MC客户端关闭失败，index:%d\n", index);
//		return;
//	}
//
////	WaitForSingleObject( m_VideoSendMutex ,  INFINITE ) ;
//
//	if(m_SocketArray[index] != NULL)
//	{
//		g_pLog->WriteLog("MC客户端关闭，IP:%s, session:%d, SOCKET:%d\n", m_SocketArray[index]->m_csAdd, (int)(m_SocketArray[index]->m_findinfo.session), m_SocketArray[index]->m_Socket);
//		//停止发送数据
//		CChannel *pChannel = g_pManagerDVR->FindChannel(m_SocketArray[index]->m_findinfo.channelid);
//		if(pChannel != NULL)
//		{
//			pChannel->stop_by_Socket(m_SocketArray[index]->m_Socket);
//		}
//		//delete m_SocketArray[index];
//		//m_SocketArray[index] = NULL;
//		//m_clientnum--;
//	}
//
////	ReleaseMutex( m_VideoSendMutex ) ;
//
//	return;
//}

void CManagerConnect::CloseCli(int index)
{
	if((index < 0)||(index >= MAX_CLIENT_SIZE))
	{
		g_pLog->WriteLog("MC客户端关闭失败，index:%d\n", index);
		return;
	}

//	WaitForSingleObject( m_VideoSendMutex ,  INFINITE ) ;

	if(m_SocketArray[index] != NULL)
	{
		g_pLog->WriteLog("MC客户端关闭，IP:%s, session:%d, SOCKET:%d\n", m_SocketArray[index]->m_csAdd, (int)(m_SocketArray[index]->m_findinfo.session), m_SocketArray[index]->m_Socket);
		//停止发送数据
		//CChannel *pChannel = g_pManagerDVR->FindChannel(m_SocketArray[index]->m_findinfo.channelid);
		//if(pChannel != NULL)
		//{
		//	pChannel->stop_by_Socket(m_SocketArray[index]->m_Socket);
		//}
		delete m_SocketArray[index];
		m_SocketArray[index] = NULL;
		m_clientnum--;
	}

//	ReleaseMutex( m_VideoSendMutex ) ;

	return;
}

int CManagerConnect::FindClient(_int64 session)
{
	for(int i = 0; i < MAX_CLIENT_SIZE; i ++)
	{
		if(m_SocketArray[i] != NULL)
		{
			if(m_SocketArray[i]->m_findinfo.session == session)
				return i;
		}
	}
	return -1;
}

void CManagerConnect::ReceiveMsg(int iPlayNum)
{
//	ASSERT(m_SocketArray[iPlayNum] != NULL);

	if(m_SocketArray[iPlayNum] == NULL)
	{
		return;
	}

	int Command;
	int iRece = recv(m_SocketArray[iPlayNum]->m_Socket, (char *)&Command, 4, 0);

	if(iRece == 0)
	{
		//CloseCli(iPlayNum);
	}
	else if(iRece == SOCKET_ERROR)
	{
		//		CloseCli(iPlayNum);
		int hr;
		hr = WSAGetLastError();
		switch(hr)
		{
			////////chenq,此处有BUG，客户端发送数据成功，服务器接收数据阻塞，并且也接不到数据了

		case WSAEWOULDBLOCK:
			{
				g_pLog->WriteLog("MC recv数据阻塞WSAEWOULDBLOCK：IP：%s, SOCKET: %d;num:%d\n", m_SocketArray[iPlayNum]->m_csAdd, m_SocketArray[iPlayNum]->m_Socket, iPlayNum);
				return ;
			}
			break;
		case WSAENOTSOCK :
			{
				g_pLog->WriteLog("MC接收失败，WSAENOTSOCK：IP：%s, SOCKET: %d\n", m_SocketArray[iPlayNum]->m_csAdd, m_SocketArray[iPlayNum]->m_Socket);
			}
			break;
		default:
			{
				g_pLog->WriteLog("MC接收失败，未处理错误(%d)\n", hr);
			}
			break;
		}
		//CloseCli(iPlayNum);
		return;
	}
	else if(iRece != 4)
	{
		g_pLog->WriteLog("MC接收失败，数据接收不完整，抛弃不处理\n");
		return;
	}

	switch(Command)
	{
	case FM_FILEPLAY_REG:
		{
			g_pLog->WriteLog("MC record reg:ip = %s; num:%d\n",m_SocketArray[iPlayNum]->m_csAdd, iPlayNum);
			m_SocketArray[iPlayNum]->SetSendType(FILE_PLAY);

			_int64 session;
			iRece = recv(m_SocketArray[iPlayNum]->m_Socket,(char *)&session,sizeof(_int64),0);
			if(iRece <= 0)
			{
				//CloseCli(iPlayNum);
				return;
			}

			FIND_INFO info;
			if(!g_pManagerRtsp->GetFindInfo(session, info))
			{
				//CloseCli(iPlayNum);
				return;
			}			

			if(!m_SocketArray[iPlayNum]->Init(info))
			{
				//CloseCli(iPlayNum);
				return;
			}

			int commond = FM_FILEPLAY_RE;
			iRece = send(m_SocketArray[iPlayNum]->m_Socket, (char *)&commond, sizeof(int), 0);
			if(iRece <= 0)
			{
				//CloseCli(iPlayNum);
				return;
			}
			g_pLog->WriteLog("MC客户端添加成功，录像，session:%d， socket:%d, num:%d\n", session, m_SocketArray[iPlayNum]->m_Socket, iPlayNum);
			break;
		}
	case FM_CHANNELPLAY_REG:
		{
			g_pLog->WriteLog("MC live reg:ip = %s ;num:%d\n",m_SocketArray[iPlayNum]->m_csAdd, iPlayNum);
			m_SocketArray[iPlayNum]->SetSendType(LIVE_PLAY);

			_int64 session;
			iRece = recv(m_SocketArray[iPlayNum]->m_Socket,(char *)&session,sizeof(_int64),0);
			if(iRece <= 0)
			{
				//CloseCli(iPlayNum);
				return;
			}

			FIND_INFO info;
			if(!g_pManagerRtsp->GetFindInfo(session, info))
			{
				//CloseCli(iPlayNum);
				return;
			}
			if(!m_SocketArray[iPlayNum]->Init(info))
			{
					//CloseCli(iPlayNum);
					return;
			}
			int commond = FM_CHANNELPLAY_REG;
			iRece = send(m_SocketArray[iPlayNum]->m_Socket,(char *)&commond,sizeof(int),0);
			if(iRece <= 0)
			{
				//CloseCli(iPlayNum);
				return;
			}
			g_pLog->WriteLog("MC客户端添加成功，直播，session:%d, socket: %d, num:%d\n", (int)session, m_SocketArray[iPlayNum]->m_Socket, iPlayNum);
		}
		break;

	case FM_FILEDOWN_REG:
		{
			//目前不支持
			break;
		}
	case PAUSE_CMD:
		{
			m_SocketArray[iPlayNum]->PauseNetPlay();
		}
		break;
	case PLAY_CMD:
		{
			m_SocketArray[iPlayNum]->ContinueNetPlay();
		}
		break;
	case FM_CHANNEL_PTZCONTROL_REG:		// 云台控制
		{
			// m_SocketArray[iPlayNum]->ContinueNetPlay();
		}
		break;
	}
}

BOOL CManagerConnect::AddClient(CVideoSend *pCon, int &index)
{

//	WaitForSingleObject( m_VideoSendMutex ,  INFINITE ) ;

	if(m_clientnum >= MAX_CLIENT_SIZE)
	{
		for(int i = 0; i < MAX_CLIENT_SIZE; i++)
		{
			if(m_SocketArray[i] != NULL)
			{
				if(m_SocketArray[i]->m_NetActive == FALSE)
				{
					delete m_SocketArray[i];
					m_SocketArray[i] = NULL;
					m_clientnum--;
				}
			}
		}
	}

//	ReleaseMutex( m_VideoSendMutex ) ;
	if(m_clientnum >= MAX_CLIENT_SIZE)
	{
		return FALSE;
	}

//	WaitForSingleObject( m_VideoSendMutex ,  INFINITE ) ;
	for(int i = 0; i < MAX_CLIENT_SIZE; i++)
	{
		if(m_SocketArray[i] == NULL)
		{
			m_SocketArray[i] = pCon;
			index = i;
			m_clientnum++;
//			ReleaseMutex( m_VideoSendMutex ) ;
			return TRUE;
		}
	}

	m_clientnum = MAX_CLIENT_SIZE;
//	ReleaseMutex( m_VideoSendMutex ) ;

	return FALSE;
}
