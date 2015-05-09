//FILESERVERDEF.CPP

#include "stdHeader.h"
#include "ReformMediaServerDef.h"
#include "ManagerConnect.h"
#include "SysInfo.h"
#include "ManagerRtsp.h"
#include "ManagerDVR.h"
#include "dvr.h"
#include "channel.h"
#include "dhdvrchannel.h"
#include "HikNetDVRChannel.h"
#include <tchar.h>
#include "Log.h"

#define MAX_STR 100

HWND				g_hWnd = NULL;
WNDPROC				g_lpOriginalProc = NULL;	// 消息响应窗体原始地址
HWND				g_hWnd2 = NULL;
WNDPROC				g_lpOriginalProc2 = NULL;   // 消息响应窗体原始地址

CManagerConnect		*g_pManagerConnect = NULL;
CManagerRtsp		*g_pManagerRtsp = NULL;
CLog				*g_pLog = NULL;

CSysInfo			*g_pSysInfo = NULL;
CManagerDVR			*g_pManagerDVR = NULL;

#ifdef _DEBUG
int scount=0;
int ocount=0;
int lcount=0;
#endif

void Ticks2Time(_int64 dwtime, CTime &time)
{
	_int64 dw = dwtime/10000000;

    int  days[13] = { 0, 31 , 29 , 031 , 30 , 31 , 30 , 31 , 31 , 30 , 31 , 30 , 31 } ;
    int  i ;
    int second,minute,hour,year,month,date;
    second = dw % 60 ;
    dw          = dw / 60 ;        /*dat is minute*/
    
    minute = dw % 60 ;
    dw          = dw / 60 ;        /*dat is hour  */
    
    hour   = dw % 24 ;
    dw          = dw / 24 ;        /*dat is day   */
        
    i            = 2001    ;

	dw = dw - (365*3+366)*500 + 15;

    while( dw >= 365 )
    {
        if( ( i % 4 ) == 0 )
            dw -= 366 ;
        else
            dw -= 365 ;
        i++ ;		
    }
    
    year  = i ;
    if( i % 4 ) 
        days[2] = 28 ;
    
    for( i = 1 ; i < 13 ; i++ )
    {
        if( dw < days[i] )
            break;
        dw -= days[i] ;    
    }
    month  =  i  ;
    date    = (int)dw + 1 ;
	time = CTime::CTime(year,month,date,hour,minute,second);
}

_int64 Time2Ticks(CTime time)
{
	_int64 dwtime;

    int  days[13] = { 0, 31 , 29 , 31 , 30 , 31 , 30 , 31 , 31 , 30 , 31 , 30 , 31 } ;
    int  day  , i ;
    _int64  seconds ;
    
    seconds =    0 ;
    day     =    0 ;
    
    if( ( time.GetYear()-2000 )%4 != 0 )
        days[2] = 28 ;
    
    for( i = 0 ; i < ( time.GetYear()-2000 ) ; i++ )    
    {
        if( i%4 == 0 )
            day += 366 ;
        else
            day += 365 ;
    }    
    for( i = 1 ; i < time.GetMonth() ; i++ ) 
    {
        day += days[i] ;
    }       
    day += ( time.GetDay() - 1 ); 
	seconds =(365*3+366)*499 - 15;
	seconds = seconds*24*3600;
	seconds =seconds + 365*3*24*3600;
	seconds =seconds +( day * 24 + time.GetHour() )*3600 + time.GetMinute() * 60 + time.GetSecond();
    
//    seconds = ( day * 24 + time.GetHour() )*3600 + time.GetMinute() * 60 + time.GetSecond() + ((365*3+366)*499)*24*3600 + 365*3*24*3600;
	dwtime = seconds*10000000;
//    return seconds ;  
	
	return dwtime;
}

LRESULT CALLBACK BuildWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

//处理rtsp消息
LRESULT WINAPI WndSubClassProc2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) //窗体消息响应
{
	/* CWinThread *p = AfxGetThread();
	TRACE("2,Current Thread ID = 0x%X\n", p->m_nThreadID); */
	if(uMsg == WM_DESTROY)
	{
		return CallWindowProc(g_lpOriginalProc2,hWnd,uMsg,wParam,lParam);
	}
	if(uMsg == WM_LISTEN_COMMANDSOCKET)
	{
		if(WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			g_pManagerRtsp->OnAccept();
		}
		else if(WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
//			g_pLog->WriteLog("g_pManagerConnect监听关闭，CloseCli:\n");
//			g_pManagerConnect->OnAccept();
		}

		return CallWindowProc(g_lpOriginalProc2,hWnd,uMsg,wParam,lParam);
	}

//	int length = g_pManagerConnect->m_SocketArray.GetSize();
	if((uMsg > WM_LISTEN_COMMANDSOCKET)&&(uMsg <= WM_LISTEN_COMMANDSOCKET + MAX_CLIENT_SIZE * 4))
	{
		if(WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
//			printdebug("客户端关闭，CloseCli:FD_CLOSE\n");
			g_pLog->WriteLog("RTSP客户端主动关闭(%d)\n", (int)(uMsg - WM_LISTEN_COMMANDSOCKET - 1));
			g_pManagerRtsp->CloseCli(uMsg - WM_LISTEN_COMMANDSOCKET - 1);
		}

		if(WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			g_pManagerRtsp->ReceiveMsg(uMsg - WM_LISTEN_COMMANDSOCKET - 1);
		}

		return CallWindowProc(g_lpOriginalProc2, hWnd, uMsg, wParam, lParam);
	}

	return CallWindowProc(g_lpOriginalProc2, hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI WndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) //窗体消息响应
{
	//CWinThread *p = AfxGetThread();
	/*ATL::AtlCreateThread();
	TRACE("2,Current Thread ID = 0x%X\n", p->m_nThreadID);*/
	if(uMsg == WM_DESTROY)
	{
		return CallWindowProc(g_lpOriginalProc,hWnd,uMsg,wParam,lParam);
	}
	// 消息,异步SOCK处理消息,流
	if(uMsg == WM_LISTEN_SOCKET)
	{
		if(WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			g_pManagerConnect->OnAccept();
		}
		else if(WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
//			g_pLog->WriteLog("g_pManagerConnect监听关闭，CloseCli:\n");
//			g_pManagerConnect->OnAccept();
		}
		return CallWindowProc(g_lpOriginalProc,hWnd,uMsg,wParam,lParam);
	}

//	int length = g_pManagerConnect->m_SocketArray.GetSize();
	if((uMsg > WM_LISTEN_SOCKET)&&(uMsg <= WM_LISTEN_SOCKET + MAX_CLIENT_SIZE*4))
	{
		if(WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
//			printdebug("客户端关闭，CloseCli:FD_CLOSE\n");
			g_pLog->WriteLog("MC客户端主动关闭(%d)\n", (int)(uMsg - WM_LISTEN_SOCKET - 1));
			g_pManagerConnect->CloseCli(uMsg - WM_LISTEN_SOCKET - 1);
		}
		if(WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			g_pManagerConnect->ReceiveMsg(uMsg - WM_LISTEN_SOCKET - 1);
		}
		return CallWindowProc(g_lpOriginalProc,hWnd,uMsg,wParam,lParam);
	}

	// 消息,DVR异常上传消息
	if(uMsg == WM_DVRALARMCOMMAND)
	{
		char tempbuf[256];
		DWORD dwType = wParam; //异常类型
		LONG lIndex = lParam; //出现异常的相应类型的句柄
		ZeroMemory(tempbuf, 256);

		switch(dwType)
		{
		case 0x8001: //语音对讲时网络异常
			sprintf(tempbuf,"语音对讲时网络异常!!!");
			//TRACE("%s",tempbuf);
			//TODO: 关闭语音对讲
			break;
		case 0x8002: //报警上传时网络异常
			sprintf(tempbuf,"报警上传时网络异常!!!");
			//TRACE("%s",tempbuf);
			//TODO: 关闭报警上传
			break;
		case 0x8003: //网络预览时异常
			sprintf(tempbuf,"网络预览时网络异常!!!");
			//TRACE("%s",tempbuf);
			//TODO: 关闭网络预览
			break;
		case 0x8004: //透明通道传输时异常
			sprintf(tempbuf,"透明通道传输时网络异常!!!");
			//TRACE("%s",tempbuf);
			//TODO: 关闭透明通道
			break;
		case 0x8005: //预览时重连
			break;
		default:
			break;
		}
	}

	// 消息,停止转发
	if(uMsg == WM_STOPTRANS_COMMANDSOCKET)
	{
		int dvrindex = wParam;
		int channelindex = lParam;

		//if(g_pManagerDVR->m_pDVRArray[dvrindex]!= NULL)
		//{
		//	if(g_pManagerDVR->m_pDVRArray[dvrindex]->m_pChannelArray[channelindex] != NULL)
		//	{
		//		if(g_pManagerDVR->m_pDVRArray[dvrindex]->m_type == HIK_DVRDH_CHANNEL/*大华DVR*/ )
		//		{
		//			CDHDVRChannel *pChannel = (CDHDVRChannel *)g_pManagerDVR->m_pDVRArray[dvrindex]->m_pChannelArray[channelindex];
		//			pChannel->StopTrans();
		//		}
		//		else if(g_pManagerDVR->m_pDVRArray[dvrindex]->m_type == HIK_DVRHK_CHANNEL/*海康DVR*/ )
		//		{
		//			CHikNetDVRChannel *pChannel = (CHikNetDVRChannel *)g_pManagerDVR->m_pDVRArray[dvrindex]->m_pChannelArray[channelindex];
		//			pChannel->StopTrans();
		//		}
		//	}
		//}
	}
	return CallWindowProc(g_lpOriginalProc, hWnd, uMsg, wParam, lParam);
}

//创建两个窗口，用来处理异步的RTSP和SOCKET
BOOL BuildWindow()
{
	HINSTANCE hInstance = NULL;
    hInstance = GetModuleHandle(NULL);	
	char lpszClassName[]="ReformMediaServerWIN";//窗口类名

	WNDCLASS wndClass;
	wndClass.style =0; // 窗口类型为缺省类型
	wndClass.lpfnWndProc=BuildWndProc; //定义窗口处理函数
	//wndClass.lpfnWndProc=WndSubClassProc; //定义窗口处理函数
	wndClass.cbClsExtra=0; //窗口类无扩展
	wndClass.cbWndExtra=0; //窗口实例无扩展
	wndClass.hInstance=hInstance; //当前实例句柄
	wndClass.hIcon=0;//窗口的最小化图标为缺省图标
	wndClass.hCursor=0; // 窗口采用箭头光标
	wndClass.hbrBackground=0; //窗口背景为白色
	wndClass.lpszMenuName=NULL; //窗口无菜单
	wndClass.lpszClassName=lpszClassName; //窗口类名为“窗口”
         
    if(!RegisterClass( &wndClass ))
	{
		//DWORD re = GetLastError();
		g_pLog->WriteErr("无法注册窗口类，可能是操作系统不支持该程序。");
		return FALSE;
	}

	//LPCTSTR lpszClass = AfxRegisterWndClass(NULL);
    g_hWnd=::CreateWindow(lpszClassName,             // windows class name
					"ReformMediaServerWin1",        // window caption
                    WS_OVERLAPPEDWINDOW,            // window style
                    CW_USEDEFAULT, CW_USEDEFAULT,   // position and dimensions
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    NULL,						    // owner window handle--NULL is Desktop
					NULL,					    	// for popup and overlapped windows: window menu handle
					hInstance,   	// handle to application instance
					NULL				            // pointer to window-creation data
					);
	if(g_hWnd == NULL)
	{
		//DWORD re = GetLastError();
		//g_pLog->WriteLog("无法生成窗口1。错误代码:%d\n",re);
		g_pLog->WriteErr("无法生成窗口1!\n");
		return FALSE;
	}

	// change the window procdure address
	g_lpOriginalProc = (WNDPROC) SetWindowLong(g_hWnd, GWL_WNDPROC, (LONG)WndSubClassProc);
	//g_lpOriginalProc=(WNDPROC)WndSubClassProc;
	
    g_hWnd2 = ::CreateWindow(lpszClassName,         // windows class name
					"ReformMediaServerWin2",        // window caption
                    WS_OVERLAPPEDWINDOW,            // window style
                    CW_USEDEFAULT, CW_USEDEFAULT,   // position and dimensions
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    NULL,						    // owner window handle--NULL is Desktop
					NULL,					    	// for popup and overlapped windows: window menu handle
					hInstance,	    				// handle to application instance
					NULL				            // pointer to window-creation data
					);
	if(g_hWnd2 == NULL)
	{
		//DWORD re = GetLastError();
		//g_pLog->WriteLog("无法生成窗口2.错误代码:%d\n",re);
		g_pLog->WriteErr("无法生成窗口2!\n");
		return FALSE;
	}

	// rtsp socket
	g_lpOriginalProc2 = (WNDPROC) SetWindowLong(g_hWnd2, GWL_WNDPROC, (LONG)WndSubClassProc2);
	return TRUE;
}

BOOL check_IP(const char* str)
{
	if(strlen(str) > 16)
		return FALSE;
	char tip[16]={0};
	memcpy(tip, str, 16);
	char* token=NULL;
	char* dots[4]={0};
	int ilen = 0;
	token = strtok(tip, ".");
	while(token !=NULL)
	{
		dots[ilen] = token;
		ilen++;
		token = strtok(NULL, ".");
	}
	if(ilen != 4)
		return FALSE;
	else
	{
		for(int i=0;i!=4;i++)
		{
			int e = atoi(dots[i]);
			if(e>255 || e <0 )
				return FALSE;
		}
	}
	return TRUE;
}

void terminate_thread(HANDLE& thread, HANDLE hevent)
{
	if(thread)
	{
		DWORD dwStatus;
		for(int i=0;;i++)
		{
			if(hevent != NULL)
				SetEvent(hevent);
			ResumeThread(thread);	
			Sleep(30);
			BOOL ret = ::GetExitCodeThread(thread, &dwStatus);
			if(ret== FALSE)
			{
				g_pLog->WriteLog("ERROR:VS关闭线程失败(err:%d tid:%d)\n", GetLastError(), thread);
				break;
			}
			else
			{
				if( dwStatus==STILL_ACTIVE)
				{
					if(i==100)
						g_pLog->WriteLog("WARN:VS未能关闭线程(%d)\n", thread);
				}
				else
				{
					g_pLog->WriteLog("VS关闭线程%d\n", thread);
					break;
				}		
			}
		}
		CloseHandle(thread);
		thread=NULL;
	}
}

CString GetAppPath()
{
	//获取应用程序根目录
    char modulePath[MAX_PATH];
    GetModuleFileName(NULL, modulePath, MAX_PATH);
    CString strModulePath(modulePath);
    strModulePath = strModulePath.Left(strModulePath.ReverseFind(_T('\\')));
    return strModulePath;
}
