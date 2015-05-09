// VideoSend.cpp: implementation of the CVideoSend class.
//
//////////////////////////////////////////////////////////////////////

#include "StdHeader.h"
#include "VideoSend.h"
#include "ManagerConnect.h"
#include "Log.h"
#include "FileIndex.h"
#include "ReformMediaServerDef.H"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWORD WINAPI CVideoSend::NetThread( LPVOID lpParameter)
{
	CVideoSend* pOwner = reinterpret_cast<CVideoSend*>(lpParameter);

	HANDLE hMulEvents[2];
	hMulEvents[0]=pOwner->m_hNetKill;
	hMulEvents[1]=pOwner->m_DataEvent;

	try
	{
		DWORD dwEvent;
		while(1)
		{
			dwEvent = WaitForMultipleObjects(2,hMulEvents,FALSE,INFINITE);
			switch(dwEvent)
			{
			case WAIT_OBJECT_0://此处退出，写退出函数
				{
//					pOwner->End();
					return 1;
				}
				break;
			case WAIT_OBJECT_0 + 1:
				{
					if(pOwner!= NULL)
					{
						if(!pOwner->OnSendFile())
						{
							if(pOwner!= NULL)
							{
								pOwner->End();
								g_pLog->WriteLog("通道TUICHUC DealData\n");
							}
							return 1;
						}
					}else{
						return 1;
					}
					break;
				}
			case WAIT_TIMEOUT:
				{
					if(pOwner!= NULL)
					{
						pOwner->End();
						g_pLog->WriteLog("通道TUICHUC DealData WAIT_TIMEOUT\n");
					}
					return 1;
				}
			default://异常，写退出函数
				{
/*					if(pOwner!= NULL)
					{
						pOwner->End();
					}
					return 1;*/
					g_pLog->WriteLog("ERROR:VideoThread，WaitForMultipleObjects返回异常事件!(event:%u error:%u)\n", dwEvent, GetLastError());
					return 1;
				}
			}
		}
	}
	/*catch(CException *pEx)
	{
		pEx->Delete();
		pEx = NULL;
		if(pOwner!= NULL)
		{
			pOwner->End();
		}
		g_pLog->WriteLog("通道TUICHUC DealData CException\n");
	}*/
	catch(...)
	{		
		if(pOwner!= NULL)
		{
			pOwner->End();
		}
		g_pLog->WriteLog("通道TUICHUC DealData CException\n");
	}
	return 1;
}

CVideoSend::CVideoSend()
{
	m_sockid = -1;
	m_Socket = INVALID_SOCKET;
	m_SendType = -1;

	m_NetFile  = INVALID_HANDLE_VALUE;
	m_ReadFilePos = 0;
	m_FileLength = 0;
	m_NetActive = FALSE;


	m_hNetThread = NULL;
	m_hNetKill=NULL;
	m_DataEvent = NULL;

	m_pFileIndex = NULL;
	m_FileIndex = -1;

	m_Speed = 1;

	m_curFileLength = 0;
	m_JpgSendMemLength = 0;
}

CVideoSend::~CVideoSend()
{
	onerun = false;
	terminate_thread(m_hNetThread);
	if(m_hNetKill != NULL)
	{
		CloseHandle(m_hNetKill);
		m_hNetKill=NULL;
	}
	if(m_NetFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_NetFile);
		m_NetFile = INVALID_HANDLE_VALUE;
	}
	if(m_pFileIndex != NULL)
	{
		delete m_pFileIndex;
		m_pFileIndex = NULL;
	}
}

BOOL CVideoSend::End()
{
	m_NetActive = FALSE;

	if (m_Socket != INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	if(m_hNetKill != NULL)
	{
		SetEvent(m_hNetKill);
	}

	if(m_NetFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_NetFile);
		m_NetFile = INVALID_HANDLE_VALUE;
	}

	if(m_pFileIndex != NULL)
	{
		delete m_pFileIndex;
		m_pFileIndex = NULL;
	}
	return TRUE;
}

BOOL CVideoSend::InitSock()
{
	if((m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED) ) == INVALID_SOCKET)
	{
		int hr;
		hr = WSAGetLastError();
		WSACleanup();
		return FALSE;
	}
	return TRUE;
}

BOOL CVideoSend::AcceptSock(SOCKET s, int num)
{
	m_Socket = s;
	if (m_Socket == INVALID_SOCKET)
	{
		return FALSE;
	}

	int timeout = 10000;
	if(setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(int)) == SOCKET_ERROR)
	{
		int hr;
		hr = WSAGetLastError();
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
//		WSACleanup();
		return FALSE;
	}

	int size = 1024*256 ; //SO_MAX_MSG_SIZE;
	if(setsockopt(m_Socket,SOL_SOCKET,SO_SNDBUF,(char*)&size,sizeof(int)) == SOCKET_ERROR)
	{
		int hr;
		hr = WSAGetLastError();
		WSACleanup();
		return FALSE;
	}

	WSAAsyncSelect(m_Socket,g_hWnd,WM_LISTEN_SOCKET + num+1,FD_READ|FD_CLOSE);
	m_sockid = num;
	
	return TRUE;
}

BOOL CVideoSend::SendVideo(char *pDate, int length )
{
	if(m_Socket != INVALID_SOCKET)
	{
		if(send(m_Socket, (char *)pDate, length,0) == SOCKET_ERROR)
		{
			int hr;
			hr = WSAGetLastError();
			switch(hr)
			{
			case WSANOTINITIALISED:
//			  printdebug("WSANOTINITIALISED");
			  break;

			  case WSAENETDOWN:
//			  printdebug("WSAENETDOWN");
			  break;

			  case WSAEACCES:
//			  printdebug("WSAEACCES");
			  break;

			  case WSAEINPROGRESS:
//			  printdebug("WSAEINPROGRESS");
			  break;

			  case WSAEFAULT:
//			  printdebug("WSAEFAULT");
			  break;
			
			  case WSAENETRESET:
//			  printdebug("WSAENETRESET");
			  break;

			  case WSAENOBUFS:
//			  printdebug("WSAENOBUFS");
			  break;

			  case WSAENOTCONN:
//			  printdebug("WSAENOTCONN");
			  break;

			  case WSAENOTSOCK:
//			  printdebug("WSAENOTSOCK");
			  break;

			  case WSAESHUTDOWN:
//			  printdebug("WSAESHUTDOWN");
			  break;

			  case WSAEOPNOTSUPP:
//			  printdebug("WSAEOPNOTSUPP");
			  break;

			  case WSAECONNABORTED:
//			  printdebug("WSAECONNABORTED");
			  break;

			  case WSAEWOULDBLOCK:
				  {
					  g_pLog->WriteLog("数据阻塞WSAEWOULDBLOCK：IP：%s, PORT: %d\n", m_csAdd, m_Port);
						return TRUE;
				  }
			  break;

			  case WSAEMSGSIZE:
//			  printdebug("WSAEMSGSIZE");
			  break;

			   case WSAECONNRESET:
//			  printdebug("WSAECONNRESET");
			  break;

			   case WSAEADDRNOTAVAIL:
//			  printdebug("WSAEADDRNOTAVAIL");
			  break;

			   case WSAEAFNOSUPPORT:
//			  printdebug("WSAEAFNOSUPPORT");
			  break;

			   case WSAEDESTADDRREQ:
//			  printdebug("WSAEDESTADDRREQ");
			  break;

			  case WSAENETUNREACH:
//			  printdebug("WSAENETUNREACH");
			  break;

			  case WSAEINVAL:
//			  printdebug("WSAEINVAL");
			  break;
			  default:
//			  printdebug("dont know");
			  break;
			}

//			printdebug("\n");
		}else
		{
//			printdebug("client send TRUE:sockid = %d, IP = %s, port = %d\n",
//				m_sockid,m_csAdd,m_Port);
			return TRUE;
		}
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	return FALSE ;
}

BOOL CVideoSend::GetPeerName(ATL::CString& rPeerAddress, UINT& rPeerPort)
{
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	if (!bResult)
	{
		rPeerPort = ntohs(sockAddr.sin_port);
		rPeerAddress = inet_ntoa(sockAddr.sin_addr);
	}
	return bResult;
}

BOOL CVideoSend::SetSendType(int type)
{
	m_SendType = type;
	return TRUE;
}

/*BOOL CVideoSend::AddFile(RECORDFILE_FIND_RESULT info)
{
	memcpy(&m_Fileinfo, &info, sizeof(RECORDFILE_FIND_RESULT));

	return TRUE;
}*/

BOOL CVideoSend::Init(FIND_INFO info)
{
	memcpy(&m_findinfo,&info, sizeof(FIND_INFO));
	return TRUE;
}

BOOL CVideoSend::StartNetPlay(_int64 startpos)//ipos为开始点的百分比
{
//	m_Filepath = "e:\\2.mp4";
	m_Speed = 1;

	if(startpos > (m_findinfo.endtime - m_findinfo.starttime)/10000000)
	{
		startpos = 0;
	}

	m_HeadLength = 40;
	m_NetActive = TRUE;
	m_pFileIndex = new CFileIndex();
/*	CTime startt = CTime::CTime(starttime);
	CTime endt = CTime::CTime(endtime);

	_int64 starttimepos = Time2Ticks(startt);
	_int64 endtimepos = Time2Ticks(endt);*/
	//m_pFileIndex->Init(m_findinfo.starttime + startpos*10000000, m_findinfo.endtime, m_findinfo.channelid);
	FILE_INFO vfile;
	strcpy(vfile.FileName, m_findinfo.channelid);
	vfile.startpos=0;
	m_pFileIndex->m_FileArray.Add(vfile);

	int length = m_pFileIndex->m_FileArray.GetSize();
	int i;
	for( i = 0; i < length; i ++)
	{
		ATL::CString str;
		SECURITY_ATTRIBUTES sa;
		sa.nLength              = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle       = 1;

		str.Format("%s", m_pFileIndex->m_FileArray[i].FileName);

		m_NetFile = ::CreateFile(str, 
			GENERIC_READ,
			FILE_SHARE_READ,
			&sa,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if(m_NetFile == INVALID_HANDLE_VALUE)
		{
			g_pLog->WriteLog("建立下载文件失败：%s\n",m_pFileIndex->m_FileArray[i].FileName);
			continue;
		}
		m_FileLength = GetFileSize(m_NetFile, NULL);
		m_curFileLength = m_FileLength;

		if(m_HeadLength > m_FileLength)
		{
			g_pLog->WriteLog("文件过小：%s\n",m_pFileIndex->m_FileArray[i].FileName);
			continue;
		}

		ULONG readlength;
		if(!ReadFile(m_NetFile,m_Head,m_HeadLength,&readlength,NULL))
		{
			g_pLog->WriteLog("读头失败：%s\n",m_pFileIndex->m_FileArray[i].FileName);
			continue;
		}
		if(m_pFileIndex->m_FileArray[i].startpos == 0)
			m_pFileIndex->m_FileArray[i].startpos = 40;
		SetFilePointer(m_NetFile, m_pFileIndex->m_FileArray[i].startpos,NULL, FILE_BEGIN);
		m_FileIndex = i;
		m_FileLength = m_FileLength - m_pFileIndex->m_FileArray[i].startpos;

		break;

	}
	if(m_NetFile == INVALID_HANDLE_VALUE)
	{
		End();
		return FALSE;
	}

	_int64 filelength = m_FileLength;
	for(i = m_FileIndex+1; i < length; i ++)
	{
		ATL::CString str;
		SECURITY_ATTRIBUTES sa;
		sa.nLength              = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle       = 1;

		str.Format("%s", m_pFileIndex->m_FileArray[i].FileName);
		HANDLE hfile = ::CreateFile(str, 
			GENERIC_READ,
			FILE_SHARE_READ,
			&sa,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if(hfile == INVALID_HANDLE_VALUE)
		{
			continue;
		}
		filelength = filelength + GetFileSize(hfile, NULL) - 40;
		::CloseHandle(hfile);
		hfile = INVALID_HANDLE_VALUE;
	}

	if(m_findinfo.encodetype == HIK_JPG_CHANNEL)
	{
		int stampsize = m_pFileIndex->m_StampArray.GetSize();
		if(stampsize == 0)
		{
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			return FALSE;
		}

		filelength = filelength+stampsize*20;

	}
	m_StampIndex = 0;
	
	int ret = send( m_Socket,(char *)&filelength,8,0);
	if(ret == SOCKET_ERROR)
	{
		g_pLog->WriteLog("文件发送失败！\n");
		int hr;
		hr = WSAGetLastError();
		g_pLog->WriteLog("文件发送失败,失败原因：%d！",hr);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}

	if((m_findinfo.encodetype == HIK_DVRHK_CHANNEL)||(m_findinfo.encodetype == HIK_DVRDH_CHANNEL))
	{
		ret = send( m_Socket,m_Head,m_HeadLength,0);
		if(ret == SOCKET_ERROR)
		{
			g_pLog->WriteLog("文件发送失败！\n");
			int hr;
			hr = WSAGetLastError();
			g_pLog->WriteLog("文件发送失败,失败原因：%d！\n",hr);
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			return FALSE;
		}
	}

	g_pLog->WriteLog("\n开始文件发送！  name = %s\n", m_Filepath );
	
	DWORD dw;
    m_DataEvent		 =  CreateEvent( NULL , TRUE , FALSE , NULL ) ;
	m_hNetKill	 =	CreateEvent(NULL,FALSE,FALSE,NULL);

//	ResetEvent( m_hNetKill ) ;
	m_hNetThread =	CreateThread(NULL,0, LPTHREAD_START_ROUTINE(NetThread),this,0,&dw);
	if(!(m_hNetThread&&m_hNetKill))
	{
		g_pLog->WriteLog("创建线程失败\n");
		return FALSE;
	}
	SetEvent( m_DataEvent ) ;

	return TRUE;
}
BOOL CVideoSend::OnSendJpgFile()
{
	if(m_JpgSendMemLength > 0)
	{
		int ret = send( m_Socket,m_JpgSendMem,m_JpgSendMemLength,0);
		if(ret == SOCKET_ERROR)
		{
			g_pLog->WriteLog("文件发送失败！\n");
			int hr;
			hr = WSAGetLastError();

			switch(hr)
			{
			  case WSAEWOULDBLOCK:
				  {
					  g_pLog->WriteLog("数据阻塞WSAEWOULDBLOCK：IP：%s, PORT: %d\n", m_csAdd, m_Port);
					  PauseNetPlay();
						return TRUE;
				  }
			  break;
			  case WSAENOTSOCK :
				  {
					  break;
				  }
			  default:
			  break;
			}

			g_pLog->WriteLog("文件发送失败,失败原因：%d！\n",hr);
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			return FALSE;
		}
		m_JpgSendMemLength = m_JpgSendMemLength - ret;
		if(m_JpgSendMemLength > 0)
		{
			memmove(m_JpgSendMem, m_JpgSendMem + ret, m_JpgSendMemLength);
			return TRUE;
		}
	}

	int stampsize = m_pFileIndex->m_StampArray.GetSize();

	if(m_StampIndex < stampsize)
	{
		if(strcmp(m_pFileIndex->m_FileArray[m_FileIndex].FileName, m_pFileIndex->m_StampArray[m_StampIndex].filename) != 0)
		{
			m_FileIndex++;
			int length = m_pFileIndex->m_FileArray.GetSize();
			if(m_FileIndex >=length)
			{
				g_pLog->WriteLog("\n文件发送完毕！m_FileIndex = %d, length = %d\n", m_FileIndex,length );
				return FALSE;
			}
			ATL::CString str;
			SECURITY_ATTRIBUTES sa;
			sa.nLength              = sizeof(sa);
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle       = 1;
			str.Format("%s", m_pFileIndex->m_FileArray[m_FileIndex].FileName);
			m_NetFile = ::CreateFile(str, 
				GENERIC_READ,
				FILE_SHARE_READ,
				&sa,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if(m_NetFile == INVALID_HANDLE_VALUE)
			{
				g_pLog->WriteLog("建立下载文件失败：%s\n",m_pFileIndex->m_FileArray[m_FileIndex]);
				return TRUE;
			}
			g_pLog->WriteLog("\n开始文件发送！m_FileIndex = %d,    name = %s\n", m_FileIndex,m_pFileIndex->m_FileArray[m_FileIndex].FileName );
			
			m_FileLength = GetFileSize(m_NetFile, NULL);
			m_curFileLength = m_FileLength;
			SetFilePointer(m_NetFile, 0,NULL, FILE_BEGIN);
		}
		if(m_StampIndex+1 == stampsize)
		{
			return FALSE;
		}
		unsigned int StampLength;
		if(strcmp(m_pFileIndex->m_StampArray[m_StampIndex+1].filename, m_pFileIndex->m_StampArray[m_StampIndex].filename) != 0)
		{
			StampLength = m_curFileLength - m_pFileIndex->m_StampArray[m_StampIndex].pos;
			if(StampLength <= 0)
			{
				return FALSE;
			}
		}else{
			StampLength = m_pFileIndex->m_StampArray[m_StampIndex+1].pos - m_pFileIndex->m_StampArray[m_StampIndex].pos;
			if(StampLength <= 0)
			{
				return FALSE;
			}
		}

		SetFilePointer(m_NetFile, m_pFileIndex->m_StampArray[m_StampIndex].pos,NULL, FILE_BEGIN);

		ULONG readlength;
		char buf[256*1024];
		
		if(!ReadFile(m_NetFile,buf+20,StampLength,&readlength,NULL))//读到具体的一个图片
		{
			g_pLog->WriteLog("读文件失败：%s,startpos:%d,StampLength:%d\n",m_pFileIndex->m_StampArray[m_StampIndex].filename,
				m_pFileIndex->m_StampArray[m_StampIndex].pos, StampLength);
			return FALSE;
		}
		if(readlength < StampLength)
		{
			m_FileLength = 0;
			return TRUE;
		}
		m_FileLength = m_FileLength - StampLength;

		DWORD date = 0;
		memcpy(buf,&date,4);
		memcpy(buf+4,&date,4);
		memcpy(buf+8,&StampLength,4);
		memcpy(buf+12,&m_pFileIndex->m_StampArray[m_StampIndex].time,8);

		m_JpgSendMemLength = StampLength + 20;
		memcpy(m_JpgSendMem,buf,StampLength + 20);

		g_pLog->WriteLog("第%d个图片，图片长度%d,时间：%I64d\n",m_StampIndex,StampLength,m_pFileIndex->m_StampArray[m_StampIndex].time );

		m_StampIndex++;

		int ret = send( m_Socket,m_JpgSendMem,m_JpgSendMemLength,0);
		if(ret == SOCKET_ERROR)
		{
			g_pLog->WriteLog("文件发送失败！\n");
			int hr;
			hr = WSAGetLastError();
			switch(hr)
			{
			  case WSAEWOULDBLOCK:
				  {
					  g_pLog->WriteLog("数据阻塞WSAEWOULDBLOCK：IP：%s, PORT: %d\n", m_csAdd, m_Port);

					  PauseNetPlay();
						return TRUE;
				  }
			  break;
			  case WSAENOTSOCK :
				  {

					  break;
				  }
			  default:
			  break;
			}
			g_pLog->WriteLog("文件发送失败,失败原因：%d！\n",hr);
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			return FALSE;
		}
		m_JpgSendMemLength = m_JpgSendMemLength - ret;
		if(m_JpgSendMemLength > 0)
		{
			memmove(m_JpgSendMem, m_JpgSendMem + ret, m_JpgSendMemLength);
		}

		Sleep(30);
		return TRUE;
	}
	return FALSE;
}

BOOL CVideoSend::OnSendFile()
{
	if(m_findinfo.encodetype == HIK_JPG_CHANNEL)
	{
		return OnSendJpgFile();
	}

	if(m_FileLength <= 0)
	{
		if(m_NetFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(m_NetFile);
			m_NetFile = INVALID_HANDLE_VALUE;
		}

		m_FileIndex++;
		int length = m_pFileIndex->m_FileArray.GetSize();
		if(m_FileIndex >=length)
		{
			g_pLog->WriteLog("\n文件发送完毕！m_FileIndex = %d, length = %d\n", m_FileIndex,length );
			return FALSE;
		}

		ATL::CString str;
		SECURITY_ATTRIBUTES sa;
		sa.nLength              = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle       = 1;
		str.Format("%s", m_pFileIndex->m_FileArray[m_FileIndex].FileName);
		m_NetFile = ::CreateFile(str, 
			GENERIC_READ,
			FILE_SHARE_READ,
			&sa,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if(m_NetFile == INVALID_HANDLE_VALUE)
		{
			g_pLog->WriteLog("建立下载文件失败：%s\n",m_pFileIndex->m_FileArray[m_FileIndex]);
			return TRUE;
		}
		g_pLog->WriteLog("\n开始文件发送！m_FileIndex = %d,    name = %s\n", m_FileIndex,m_pFileIndex->m_FileArray[m_FileIndex].FileName );
		
		m_FileLength = GetFileSize(m_NetFile, NULL);
//		m_FileLength = m_FileLength - 40;
		SetFilePointer(m_NetFile, 0,NULL, FILE_BEGIN);

	}

//////////////////////////////
	ULONG readlength;
	char buf[BLOCKSIZE_LIVE];
	if((m_findinfo.encodetype == HIK_DVRHK_CHANNEL)||(m_findinfo.encodetype == HIK_DVRDH_CHANNEL))
	{
		if(!ReadFile(m_NetFile, buf, BLOCKSIZE_FILE, &readlength, NULL))
		{
			return FALSE;
		}
		if(readlength == 0)
		{
			m_FileLength = 0;
			return TRUE;
		}
	}

	int ret = send( m_Socket,buf,readlength,0);
	if(ret == SOCKET_ERROR)
	{
		g_pLog->WriteLog("文件发送失败！\n");
		int hr;
		hr = WSAGetLastError();
		switch(hr)
		{
		  case WSAEWOULDBLOCK:
			  {
				  g_pLog->WriteLog("数据阻塞WSAEWOULDBLOCK：IP：%s, PORT: %d\n", m_csAdd, m_Port);
				  long fpback = readlength;
				  SetFilePointer(m_NetFile, -fpback, NULL, FILE_CURRENT);
				  //PauseNetPlay();
					return TRUE;
			  }
		  break;
		  case WSAENOTSOCK :
			  {
//				  g_pLog->WriteLog("数据阻塞WSAENOTSOCK：IP：%s, PORT: %d\n", m_csAdd, m_Port);
//				  SetFilePointer(m_NetFile, -readlength, NULL, FILE_CURRENT);
//				  PauseNetPlay();
				  break;
			  }
		  default:
		  break;
		}
		g_pLog->WriteLog("文件发送失败,失败原因：%d！\n",hr);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}
	else if((unsigned int)ret < readlength)
	{
		SetFilePointer(m_NetFile, ret-readlength, NULL, FILE_CURRENT);
	}
	m_FileLength = m_FileLength - ret;
	Sleep(TIME_LOOP);
	return TRUE;
}

BOOL CVideoSend::PauseNetPlay()
{
	ResetEvent( m_DataEvent ) ; 
	g_pLog->WriteLog("\nPauseNetPlay\n");
	return TRUE;
}

BOOL CVideoSend::ContinueNetPlay()
{
	SetEvent( m_DataEvent ) ;
	g_pLog->WriteLog("\nContinueNetPlay\n");
	return TRUE;
}

BOOL CVideoSend::StartNetPlayOne()
{
	if(m_findinfo.starttime!=0 && m_findinfo.endtime!=0 && m_findinfo.starttime > m_findinfo.endtime)
	{
		g_pLog->WriteLog("播放时间段不对，可以将起始和终止时间设为空\n", );
		return FALSE;
	}

	_buf_left = 0;
	_buf_size = 0;

	ATL::CString str;
	SECURITY_ATTRIBUTES sa;
	sa.nLength              = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle       = 1;

	str.Format("%s", m_findinfo.channelid);

	m_NetFile = ::CreateFile(str, 
		GENERIC_READ,
		FILE_SHARE_READ,
		&sa,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(m_NetFile == INVALID_HANDLE_VALUE)
	{
		g_pLog->WriteLog("建立下载文件失败：%s ERROR:%lu\n",m_findinfo.channelid, GetLastError());
		return FALSE;
	}
	m_FileLength = GetFileSize(m_NetFile, NULL);

	if(1024*4 > m_FileLength || m_FileLength/1024/1024/1024 > 4)
	{
		g_pLog->WriteLog("文件长度不支持:%ld :%s\n", m_FileLength, m_findinfo.channelid);
		::CloseHandle(m_NetFile);
		m_NetFile = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	_left_length = m_FileLength;
	if(m_findinfo.starttime!=0 || m_findinfo.endtime!=0)
	{
		//设置开始发送位置
		char file_name[128]={};
		_splitpath(m_findinfo.channelid, 0, 0, file_name, 0);
		//char time_str[21];
		//strcpy(time_str, file_name + strlen(file_name) - 14);
		char* time_str = _tcsrchr(file_name, '_');
		if(time_str == NULL || strlen(time_str) < 15)
		{
			::CloseHandle(m_NetFile);
			m_NetFile = INVALID_HANDLE_VALUE;
			g_pLog->WriteLog("字符串格式不对\n", );
			return FALSE;
		}
		time_str +=1;
		char ys[4] = {};
		memcpy(ys, time_str , sizeof(ys));
		int year = atoi(ys);
		char ms[2] = {};
		memcpy(ms, time_str+4, sizeof(ms));
		int month = atoi(ms);
		char ds[2] = {};
		memcpy(ds, time_str+6, sizeof(ms));
		int day = atoi(ds);
		char hs[2] = {};
		memcpy(hs, time_str+8, sizeof(hs));
		int hour = atoi(hs);
		char mis[2] = {};
		memcpy(mis, time_str+10, sizeof(mis));
		int min = atoi(mis);
		char ss[2] = {};
		memcpy(ss, time_str+12, sizeof(ss));
		int sec = atoi(ss);
		//判断时间是否有效
		if(year>3000 || year<1970 || month>12 || month<1 || day>31 || day<1 || hour>23 || hour<0 || min>59 || min<0 || sec>59 || sec<0)
		{
			::CloseHandle(m_NetFile);
			m_NetFile = INVALID_HANDLE_VALUE;
			g_pLog->WriteLog("时间解析无效\n", );
			return FALSE;
		}
		CTime statt(year, month, day, hour, min, sec);
		long long sdiff = m_findinfo.starttime - statt.GetTime();
		if(sdiff >= 1800)
		{
			::CloseHandle(m_NetFile);
			m_NetFile = INVALID_HANDLE_VALUE;
			g_pLog->WriteLog("开始时间大于该文件停止录制时间\n", );
			return FALSE;
		}
		sdiff = (sdiff > 0)?sdiff:0;
		long long ediff = m_findinfo.endtime - statt.GetTime();
		if(ediff < 0)
		{
			::CloseHandle(m_NetFile);
			m_NetFile = INVALID_HANDLE_VALUE;
			g_pLog->WriteLog("停止时间小于该文件开始录制时间\n", );
			return FALSE;
		}
		ediff = (ediff<1800)?ediff:1800;
		_left_length = m_FileLength/1800*(ediff - sdiff);
		if(_left_length <= 1024*4)
		{
			::CloseHandle(m_NetFile);
			m_NetFile = INVALID_HANDLE_VALUE;
			g_pLog->WriteLog("播放时间段太短，请提前开始时间或延后停止时间\n", );
			return FALSE;
		}
		if(sdiff > 0)
		{
			int spos = m_FileLength/1800*sdiff;
			SetFilePointer(m_NetFile, spos, NULL, FILE_CURRENT);
		}
	}
	//CTime startt(time_str);
	//CTime endt = startt. + ;

	//int ret = send( m_Socket,(char *)&m_FileLength,sizeof(m_FileLength),0);
	//if(ret == SOCKET_ERROR)
	//{
	//	int hr;
	//	hr = WSAGetLastError();
	//	g_pLog->WriteLog("文件发送失败,失败原因：%d！",hr);
	//	closesocket(m_Socket);
	//	m_Socket = INVALID_SOCKET;
	//	return FALSE;
	//}

	g_pLog->WriteLog("\n开始文件发送！  name = %s\n", m_findinfo.channelid );

	onerun = true;

	DWORD dw;
	m_hNetThread =	CreateThread(NULL,0, LPTHREAD_START_ROUTINE(NetOneThread),this,0,&dw);
	if(!m_hNetThread)
	{
		g_pLog->WriteLog("创建线程失败\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CVideoSend::OnSendOneFile()
{
	if(_left_length == 0)
	{
		if(m_NetFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(m_NetFile);
			m_NetFile = INVALID_HANDLE_VALUE;
		}
		g_pLog->WriteLog("\n文件发送完毕, %s\n", m_findinfo.channelid);
		return FALSE;
	}

	if(_buf_left == 0)
	{
		if(!ReadFile(m_NetFile, _readbuf, BLOCKSIZE_FILE, &_buf_size, NULL))
		{
			int hr = WSAGetLastError();
			g_pLog->WriteLog("文件发送失败,读取文件失败(%d)\n",hr);
			return FALSE;
		}
		if(_buf_size == 0)
		{
			_left_length = 0;
			return TRUE;
		}
		_buf_left = _buf_size;
	}

	int ret = send( m_Socket, _readbuf + (_buf_size - _buf_left), _buf_left,0);
	if(ret == SOCKET_ERROR)
	{
		int hr = WSAGetLastError();
		switch(hr)
		{
		case WSAEWOULDBLOCK:
			{
				g_pLog->WriteLog("数据阻塞WSAEWOULDBLOCK：IP：%s, PORT: %d\n", m_csAdd, m_Port);
				Sleep(2000);
				return TRUE;
			}
			break;
		default:
			{
				g_pLog->WriteLog("文件发送失败,失败原因：%d！\n",hr);
				//closesocket(m_Socket);
				//m_Socket = INVALID_SOCKET;
				return FALSE;
			}
			break;
		}
	}
	_buf_left = (_buf_size>ret)?(_buf_size - ret):0;
	_left_length = (_left_length>ret)?(_left_length - ret):0;
	
	return TRUE;
}

DWORD WINAPI CVideoSend::NetOneThread( LPVOID lpParameter)
{
	try
	{
		CVideoSend* pOwner = reinterpret_cast<CVideoSend*>(lpParameter);
		HANDLE id = pOwner->m_hNetThread;
		while(pOwner->onerun)
		{
			if(!pOwner->OnSendOneFile())
			{
				g_pLog->WriteLog("ERROR: 文件发送失败(%d)\n", id);
				return 1;
			}
			Sleep(TIME_LOOP);
		}
		g_pLog->WriteLog("VS退出线程(%d)\n", id);
	}
	catch(...)
	{
		g_pLog->WriteLog("ERROR: 文件发送失败 CException\n");
	}
	return 1;
}
