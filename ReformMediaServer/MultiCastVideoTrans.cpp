#include "StdHeader.h"
#include "MultiCastVideoTrans.h"
#include "log.h"
#include "ReformMediaServerDef.h"

DWORD WINAPI CMultiCastVideoTrans::TransThread( LPVOID lpParameter)
{
	CMultiCastVideoTrans* pOwner = reinterpret_cast<CMultiCastVideoTrans*>(lpParameter);
	if(pOwner == NULL)
	{
		//g_pLog->WriteLog("通道为空\n");
		return 1;
	}

	HANDLE hMulEvents[2];
	hMulEvents[0] = pOwner->m_hTransKill;
	hMulEvents[1] = pOwner->m_DataEvent;

	//g_pLog->WriteLog("chu shi chang du %d\n",pOwner->m_Buflength);

	try
	{
		while(1)
		{
			switch(WaitForMultipleObjects(2, hMulEvents, FALSE, INFINITE))
			{
			case WAIT_OBJECT_0:		// 此处退出，写退出函数
				{
					return 1;
				}
				break;
			case WAIT_OBJECT_0 + 1:
				{
					if(pOwner!= NULL)
					{
						if(!pOwner->OnTransVideo())
						{
							if(pOwner!= NULL)
							{
								pOwner->End();
							}
							////g_pLog->WriteLog("播放失败\n");
							//g_pLog->WriteLog("播放失败,或完毕！\n");
							return 1;
						}
					}else{
						return 1;
					}
					break;
				}
			default:// 异常，写退出函数
				{
					// g_pLog->WriteLog("NetPlayThread返回异常事件");
					Sleep(50);
					break;
				}
			}
//			OnPlay();
		}
	}
	/*catch(CException *pEx)
	{
		pEx->Delete();
		pEx = NULL;
		if(pOwner != NULL)
		{
			pOwner->End();
		}
	}*/
	catch(...)
	{
		if(pOwner != NULL)
		{
			pOwner->End();
		}
	}
	return 1;
}

CMultiCastVideoTrans::CMultiCastVideoTrans(void)
{
	m_Membuf = new char[MEMBUF_LENGTH];
	m_Socket = INVALID_SOCKET;

	m_Buflength = 0;
	m_BufMutex = CreateMutex( NULL , FALSE , NULL ) ;
	m_bTrans = FALSE;
}

CMultiCastVideoTrans::~CMultiCastVideoTrans(void)
{
	g_pLog->WriteLog("注销CMultiCastVideoTrans\n");

	End();

	if(m_hTransThread)
	{
		if(m_hTransKill != NULL)
		{
			SetEvent(m_hTransKill);
			DWORD dwStatus;
			for(int i=0;i<5;i++)
			{
				if(!::GetExitCodeThread(m_hTransThread, &dwStatus)||i==4)
				{
					TerminateThread(m_hTransThread,0);
					break;
				}
				else
				{
					if(dwStatus==STILL_ACTIVE)
					{
						SetEvent(m_hTransKill);
						Sleep(30);
					}
					else
						break;
				}
			}
		}
		CloseHandle(m_hTransThread);
		m_hTransThread = NULL;
	}

	if(m_hTransKill != NULL)
	{
		CloseHandle(m_hTransKill);
		m_hTransKill=NULL;
	}
	delete [] m_Membuf;
}

BOOL CMultiCastVideoTrans::End()
{
	g_pLog->WriteLog("停止发送\n");
	m_bTrans = FALSE;

	if(m_hTransKill != NULL)
	{
		SetEvent(m_hTransKill);
	}

	Sleep(100);

	if (m_Socket != INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	return TRUE;
}

BOOL CMultiCastVideoTrans::Init(SOCKET sock,sockaddr_in remote)
{
	m_Socket = sock;
	m_Remote = remote;
	m_Buflength = 0;

	DWORD dw;
    m_DataEvent		= CreateEvent( NULL , TRUE , FALSE , NULL ) ;
	m_hTransKill	= CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hTransThread	= CreateThread( NULL, 0, LPTHREAD_START_ROUTINE(TransThread), this, 0, &dw);

	if(!(m_DataEvent && m_hTransKill && m_hTransThread))
	{
		g_pLog->WriteLog("创建线程失败\n");

		return FALSE;
	}
	m_bTrans = TRUE;
	return TRUE;
}

BOOL CMultiCastVideoTrans::AddBuffer(char *pbuf, int length)
{
	if(m_bTrans == FALSE)
		return FALSE;

    WaitForSingleObject( m_BufMutex ,  INFINITE );
	if((length + m_Buflength) > MEMBUF_LENGTH)
	{
		//g_pLog->WriteLog("数据大于缓冲区, %d,%d\n",length ,m_Buflength);
		m_Buflength = 0;
	}
	
	memcpy( m_Membuf + m_Buflength, pbuf , length ) ;
	m_Buflength = m_Buflength + length;

	ReleaseMutex( m_BufMutex ) ;
	SetEvent(m_DataEvent);

	return TRUE;
}

BOOL CMultiCastVideoTrans::OnTransVideo()
{
	WaitForSingleObject( m_BufMutex ,  INFINITE ) ;
	// 发送的长度，每次最多发送15 * 1024
	int sendlength = BLOCKSIZE_UDP;
	if(sendlength > m_Buflength)
		sendlength = m_Buflength;

	// 发送
	// int re = send(m_Socket, m_Membuf, sendlength, 0);
	int re = sendto( m_Socket, m_Membuf, sendlength, 0, (struct sockaddr*)&m_Remote, sizeof(m_Remote) );
	if( re <= 0 )
	{
		int hr;
		hr = WSAGetLastError();
		switch(hr)
		{
////////chenq,此处有BUG，客户端发送数据成功，服务器接收数据阻塞，并且也接不到数据了
		  case WSAEWOULDBLOCK:
			{
//				  g_pLog->WriteLog("ReceiveMsg数据阻塞WSAEWOULDBLOCK：\n");
				ReleaseMutex( m_BufMutex ) ;
				Sleep(30);
				return TRUE;
			}
		  break;
		  case WSAENOTSOCK :
			{
				g_pLog->WriteLog("WSAENOTSOCK：\n");
			}
		  break;
		  case 0:
			{
				ReleaseMutex( m_BufMutex ) ;
				Sleep(30);
				return TRUE;
			}
		  break;
		  default:
			  g_pLog->WriteLog("dont know\n");
		  break;
		}

		ReleaseMutex( m_BufMutex ) ;
		g_pLog->WriteLog("异常关闭socket:%d, error:%d\n", m_Socket, hr );
		return FALSE;
	}

	m_Buflength = m_Buflength - re;
//	g_pLog->WriteLog("发送数据：%d,实际发送%d\n",sendlength, re);

	// 把已经发送的信息擦除
	if(m_Buflength > 0)
		memmove( m_Membuf, m_Membuf + re , m_Buflength ) ;

	ReleaseMutex( m_BufMutex ) ;

	if(m_Buflength == 0)
	{
        ResetEvent( m_DataEvent ) ;   // 给出数据信号
	} 
	Sleep(30);

	return TRUE;
}
