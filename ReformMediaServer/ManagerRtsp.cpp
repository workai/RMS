#include "StdHeader.h"
#include "ManagerRtsp.h"

#include "ManagerConnect.h"

#include "VideoSend.h"
//#include "FileServerDlg.h"

#include "Log.h"
#include "SysInfo.h"
#include "Channel.h"
#include "managerDVR.h"
#include "DVR.h"
#include "DHDVRChannel.h"
#include "HikNetDVRChannel.h"
#include "Packet.h"
#include "WinBase.h"
#include "shlobj.h"
#include "PushServer.h"

CManagerRtsp::CManagerRtsp(void)
{
    for(int i = 0; i < MAX_CLIENT_SIZE; i++)
    {
        m_RtspClientArray[i].sock = INVALID_SOCKET;
		memset(&(m_RtspClientArray[i].findinfo), 0, sizeof(m_RtspClientArray[i].findinfo));
    }
    m_ClientNum = 0;

    m_SessionIndex = 1;
//	m_ArrayMutex		= CreateMutex( NULL , FALSE , NULL ) ;
}

CManagerRtsp::~CManagerRtsp(void)
{
    for(int i = 0; i < MAX_CLIENT_SIZE; i++)
    {
        if(m_RtspClientArray[i].sock != INVALID_SOCKET)
        {
            closesocket(m_RtspClientArray[i].sock);
            m_RtspClientArray[i].sock = INVALID_SOCKET;
        }
    }
}

BOOL CManagerRtsp::GetFindInfo(_int64 session,FIND_INFO &findinfo)
{
    int index = -1;
    for(int i = 0; i < MAX_CLIENT_SIZE; i++)
    {
    	if(m_RtspClientArray[i].sock != INVALID_SOCKET)
    	{
    		if(session == m_RtspClientArray[i].findinfo.session)
    		{
    			index = i;
    			memcpy(&findinfo, &m_RtspClientArray[i].findinfo,sizeof(FIND_INFO));
    			break;
    		}
    	}
    }

    if(index == -1)
    	return FALSE;

    return TRUE;
}

BOOL CManagerRtsp::Init()
{
	// 创建socket	
	m_SockRTSPlisten = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);		// 可以考虑RTSP用TCP连接
	if (m_SockRTSPlisten == INVALID_SOCKET) 
	{
//		wsprintf(szError,TEXT("Create socket failed! Error: %d"),WSAGetLastError());
//		MessageBox(szError);
		DWORD err = WSAGetLastError();
		g_pLog->WriteLog("创建socket失败(%d)!\n", err);
		return FALSE;
	}

	unsigned long ul = 1;
	int nRet = ioctlsocket(m_SockRTSPlisten, FIONBIO, (unsigned long *)&ul);
	if(nRet == SOCKET_ERROR)
	{
		DWORD err = WSAGetLastError();
		g_pLog->WriteLog("设置socket异步失败(%d)!\n", err);
		return FALSE;
	}

	// Bind
 	struct sockaddr_in m_ServerRTSPAddr;						// 服务器的RTSP地址信息    
	m_ServerRTSPAddr.sin_family = AF_INET;
	m_ServerRTSPAddr.sin_port = htons(g_pSysInfo->m_RtspPort);	// RTSP的端口
	m_ServerRTSPAddr.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(m_SockRTSPlisten,(struct sockaddr *)&m_ServerRTSPAddr, sizeof(m_ServerRTSPAddr)) == SOCKET_ERROR)
	{
		DWORD err = WSAGetLastError();
		g_pLog->WriteLog("将听端口(%d)失败(%d)!\n", g_pSysInfo->m_RtspPort, err);
		return FALSE;
	}

	g_pLog->WriteLog( "rtspserver init completed! port:%d\n", g_pSysInfo->m_RtspPort );
	if(WSAAsyncSelect( m_SockRTSPlisten, g_hWnd2, WM_LISTEN_COMMANDSOCKET, FD_ACCEPT | FD_CLOSE ) !=0)
	{
		 g_pLog->WriteLog("RTSP监听失败,%d",WSAGetLastError());
		 return FALSE;
	}

    // Listen
	if( listen(m_SockRTSPlisten, 50) == SOCKET_ERROR )
	{
//		wsprintf(szError,TEXT("Listen failed! Error: %d"),WSAGetLastError());
//		MessageBox(szError);
		return	FALSE;
	}

	return TRUE;
}

// 如果存在可用的客户端连接资源，则保存与客户端的连接，否则直接返回
BOOL CManagerRtsp::OnAccept()
{
    if(m_ClientNum == MAX_CLIENT_SIZE)
        return FALSE;

    int index = -1;
    for(int i = 0; i < MAX_CLIENT_SIZE; i++)
    {// 判断是否存在尚未使用的客户端连接
        if(m_RtspClientArray[i].sock == INVALID_SOCKET)
        {
            index = i;
            break;
        }
    }

    if(index == -1)
    {// 超出最大可用连接数，直接返回
        m_ClientNum = MAX_CLIENT_SIZE;
        return FALSE;
    }

    // 获得连接
    m_RtspClientArray[index].sock = accept(m_SockRTSPlisten, NULL, 0);	
    WSAAsyncSelect(m_RtspClientArray[index].sock, g_hWnd2, WM_LISTEN_COMMANDSOCKET + index + 1, FD_READ | FD_CLOSE );

	g_pLog->WriteLog("RTSP accept %d, index=%d\n", m_RtspClientArray[index].sock, index);

    m_ClientNum++;
    return TRUE;
}

void CManagerRtsp::ReceiveMsg(int index)
{
	char buf[1024] = {0};
	char szMessage[1024] = {0};

	int re = recv(m_RtspClientArray[index].sock, buf, 1024, 0);
	if(re == 0)
	{
		//CloseCli(index);
		g_pLog->WriteLog("RTSP close socket\n");
	}
	else if(re == SOCKET_ERROR)
	{
		DWORD err = GetLastError();
		g_pLog->WriteLog("ERROR: RTSP recv fail(%d) index:%d, socket:%d\n", err, index, m_RtspClientArray[index].sock);
	}
	else
	{
		char *p = buf;

		if(strstr(p, "OPTIONS"))
		{
			//g_pLog->WriteLog("rtspserv receive msg from %d, index=%d, cmd OPTIONS\n", m_RtspClientArray[index].sock, index);
			OptionAnswer(p, index);
		}
		else if(strstr(p, "SETUP"))
		{
			//g_pLog->WriteLog("rtspserv receive msg from %d, index=%d, cmd SETUP\n", m_RtspClientArray[index].sock, index);
			SetupAnswer(p, index);
		}
		else if(strstr(p, "PLAY"))
		{
			//g_pLog->WriteLog("rtspserv receive msg from %d, index=%d, cmd PLAY\n", m_RtspClientArray[index].sock, index);
			PlayAnswer(p, index);
		}
		else if(strstr(p, "PAUSE"))
		{
			//g_pLog->WriteLog("rtspserv receive msg from %d, index=%d, cmd PAUSE\n", m_RtspClientArray[index].sock, index);
			PauseAnswer(p, index);
		}
		else if(strstr(p, "TEARDOWN"))
		{
			g_pLog->WriteLog("rtspserv receive msg from %d, index=%d, cmd TEARDOWN\n", m_RtspClientArray[index].sock, index);
			TeardownAnswer(p, index);
		}
		else if(strstr(p, "PTZCONTROL"))
		{
			//g_pLog->WriteLog("rtspserv receive msg from %d, index=%d, cmd PTZCONTROL\n", m_RtspClientArray[index].sock, index);
			//sprintf_s(szMessage, "rtspserv recv ptzcontrol command：\n[%s]\n", buf);
			//g_pLog->WriteLog(szMessage);

			PTZCommand(p, index);
		}
		else if(strstr(p, "GETPTZSTATE"))
		{
			//sprintf_s(szMessage, "rtspserv recv ptzquery command：\n[%s]\n", buf);
			//g_pLog->WriteLog(szMessage);

			//g_pLog->WriteLog("rtspserv receive msg from %d, index=%d, cmd GETPTZSTATE\n", m_RtspClientArray[index].sock, index);
			PTZStateQuery(p, index);
		}
		else if(strstr(p,"SERVERCAPPIC"))
		{
			//CapturePic(p, index);
		}
		else if(strstr(p,"PTZOBTAIN"))
		{
			PTZObtain(p, index);
		}
		else if(strstr(p,"PTZRELEASE"))
		{
			PTZRelease(p, index);
		}
		else
		{
			g_pLog->WriteLog("rtspserv receive msg from %d, index=%d, cmd UNKNOWN\n", m_RtspClientArray[index].sock, index);
		}
	}
}

// 判断云台是否可以控制
BOOL CManagerRtsp::IsPtzUsed(char *recvbuf, int index)
{
	ItPtzStateMap iterState = m_mapPtzState.find(m_RtspClientArray[index].findinfo.channelid);
	if(iterState != m_mapPtzState.end())
	{
		if(iterState->second->bUsing && iterState->second->m_SessionIndex != index)
		{
			double dt = difftime(time(NULL), iterState->second->lasttime);
			if(dt < PTZCTRL_TIMEOUT)
				return TRUE;
		}
	}

	return FALSE;
}

// 查询云台控制状态
BOOL CManagerRtsp::PTZStateQuery(char *recvbuf, int index)
{
	 if(index < 0 || index >= MAX_CLIENT_SIZE 
		 || m_RtspClientArray[index].sock == INVALID_SOCKET)
        return FALSE;

	 char szMessage[1024] = {0};

    BOOL res = g_pManagerDVR->PTZStateQuery(m_RtspClientArray[index].findinfo.channelid);
	switch(res)
	{
	case 2:
		{
			string username="";
			if(_ptz_user.find(m_RtspClientArray[index].findinfo.channelid)==_ptz_user.end())
				g_pLog->WriteLog("ERROR:ptzcontrol username not find\n");
			else
				username = _ptz_user[m_RtspClientArray[index].findinfo.channelid];
			sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:succeed\nMessage:BUSY\nUser:%s\nFinished:true",username.c_str());
		}
		break;
	case 1:
		{
			sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:succeed\nMessage:READY\nFinished:true");
		}
		break;
	case 0:
		{
			sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:UNAVAILABLE\nFinished:true");
		}
		break;
	case -1:
		g_pLog->WriteLog("rtspserv wrong number of channelid \n");
	default:
		{
			sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:NOT FOUND\nFinished:true");
		}
		break;
	}
	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);

	//if(IsPtzUsed(recvbuf, index))
	//{
	//	g_pLog->WriteLog("设备已被占用(%d)!\n", index);
	//	sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:device in use，please try again later！Finished:true");
	//	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
	//	return FALSE;
	//}
	//else
	//{
	//	g_pLog->WriteLog("device is not used！\n");
	//	sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:succeed\nMessage:device is not used！Finished:true");
	//	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
	//	return TRUE;
	//}

	return TRUE;
}

//BOOL CManagerRtsp::CapturePic(char *recvbuf, int index)
//{
//	BOOL bRet = FALSE;
//	if(index < 0 || index >= MAX_CLIENT_SIZE 
//		|| m_RtspClientArray[index].sock == INVALID_SOCKET)
//		return TRUE;
//
//	char szMessage[1024] = {0};
//
//	char *pPicFile = strstr(recvbuf, "PICFILE:");
//	char *pFinished = strstr(recvbuf, "Finished:true");
//
//	if(pPicFile == NULL || pFinished == NULL)
//	{
//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nCapturePic:failed\nMessage: wrong number of command parameters ！Finished:true");
//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//		return FALSE;
//	}
//
//	//获取抓图文件名
//	char szPicFile[128]={0};
//	strncpy(szPicFile, pPicFile + 8, pFinished - pPicFile - 8);
//	char *picFileName=szPicFile;
//	if(strstr(picFileName,"\\")) //包含了非法字符
//	{
//		g_pLog->WriteLog("rtspserv wrong fileName \n");
//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nCapturePic:failed\nMessage:wrong fileName！Finished:true");
//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//		return FALSE;
//	}
//
//	CChannel *pChannel = g_pManagerDVR->FindChannel(m_RtspClientArray[index].findinfo.channelid);
//	if(pChannel == NULL)
//	{
//		g_pLog->WriteLog("rtspserv wrong number of channelid \n");
//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nCapturePic:failed\nMessage:wrong number of channelid！Finished:true");
//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//		return FALSE;
//	}
//	string sRecordPath=pChannel->m_strRecordPath+"Images\\";
//	//modify by hexy 2014.3.29
//	bRet= SHCreateDirectoryEx(NULL,sRecordPath.c_str(),NULL);
//	if(!bRet) return bRet;
//
//	const char *sTempPath=sRecordPath.c_str();
//	char sTemp[sizeof(sTempPath)];
//	strcpy(sTemp,sTempPath);
//		char *sFileName=lstrcatA(lstrcatA(sTemp,"\\"), picFileName);
//
//	if(pChannel->m_pDVR->m_type == HIK_DVRDH_CHANNEL )		// 大华
//	{
//		CDHDVRChannel *pDHChannel = (CDHDVRChannel *)pChannel;
//		
//		bRet=pDHChannel->GetPic(sFileName);
//
//		g_pLog->WriteLog("rtspserv DH exec ：%s\n", bRet ? "successfully!" : "failed!");
//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nCapturePic:%s\nMessage:%s！Finished:true", 
//			bRet ? "succeed" : "failed", bRet ? "exec successfully!" : "exec failed !");
//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//	}
//	else if(pChannel->m_pDVR->m_type == HIK_DVRHK_CHANNEL)
//	{
//
//		CHikNetDVRChannel *pHIKChannel = (CHikNetDVRChannel *)pChannel;
//		//bRet = pHIKChannel->PTZControl(HIK_PTZ_COMMAND[nCommand], atoi(szParam1), atoi(szParam2), atoi(szParam3), atoi(szStop));
//
//		g_pLog->WriteLog("rtspserv HIK exec ：%s\n", bRet ? "successfully！" : "failed!");
//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nCapturePic:%s\nMessage:%s！Finished:true", 
//			bRet ? "succeed" : "failed", bRet ? "exec successfully!" : "exec failed !");
//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//	}
//	else
//	{
//		//str.Format("RTSP/1.0 200 OK\nPTZContResult:%s", bRet ? "Succeed" : "Failed");
//		//strcpy(buf, str.GetBuffer(str.GetLength()));
//		//SendRtspMessage(m_RtspClientArray[index].sock, buf, str.GetLength(), 0);
//		//str.ReleaseBuffer();
//		//return bRet;
//	}
//	return bRet;
//}

// 云台控制, 增加权限控制，采用排队的策略
//BOOL CManagerRtsp::PTZCommand(char *recvbuf, int index)
//{
//    BOOL bRet = FALSE;
//    if(index < 0 || index >= MAX_CLIENT_SIZE 
//		|| m_RtspClientArray[index].sock == INVALID_SOCKET)
//        return FALSE;
//
//	char szMessage[1024] = {0};
//	char buf[1024] = {0};
//    char szCommand[128] = {0};
//	char szParam1[128] = {0};
//	char szParam2[128] = {0};
//	char szParam3[128] = {0};
//    char szStop[128] = {0};
//
//    // 获取command参数
//    char *pComamnd = strstr(recvbuf, "PTZCommand:");
//	char *pParam1 = strstr(recvbuf, "Param1:");
//	char *pParam2 = strstr(recvbuf, "Param2:");
//	char *pParam3 = strstr(recvbuf, "Param3:");
//    char *pStop = strstr(recvbuf, "Stop:");
//    char *pFinished = strstr(recvbuf, "Finished:true");
//	char *pUser = strstr(recvbuf, "User:");
//    if(pComamnd == NULL || pParam1 == NULL ||
//		pParam2 == NULL || pParam3 == NULL ||
//		pStop == NULL || pFinished == NULL)
//    {
//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZContResult:failed\nMessage: wrong number of command parameters ！Finished:true");
//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//        return FALSE;
//    }
//
//	// "PTZCONTROL * RTSP/1.0\n CSeq:1\n PTZCommand:%d\n Param1:%d\n Param2:%d\n Param3:%d\n Stop:%d\n Finished:true"
//    strncpy(szCommand, pComamnd + 11, pParam1 - pComamnd - 11);
//    strncpy(szParam1, pParam1 + 7, pParam2 - pParam1 - 7);
//	strncpy(szParam2, pParam2 + 7, pParam3 - pParam2 - 7);
//	strncpy(szParam3, pParam3 + 7, pStop - pParam3 - 7);
//	if(pUser ==NULL)
//		strncpy(szStop, pStop + 5, pFinished - pStop - 5);
//	else
//		strncpy(szStop, pStop + 5, pUser - pStop - 5);
//
//	int nCommand = atoi(szCommand);
//	if(nCommand < 0 || nCommand >= PTZ_MAX_COMMAND)
//	{
//		g_pLog->WriteLog("ptz command exec failed：invalide command value%d\n", nCommand);
//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:invalide command value！%d！Finished:true", nCommand);
//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//		return FALSE;
//	}
//	
//	map<string ,int>::iterator iter = _ptzStatus.find(m_RtspClientArray[index].findinfo.channelid);
//	if(iter == _ptzStatus.end())
//	{
//		BOOL dwStop = atoi(szStop);
//		bRet = g_pManagerDVR->PTZControl(m_RtspClientArray[index].findinfo.channelid, nCommand, atoi(szParam1), atoi(szParam2), atoi(szParam3), dwStop );
//		if(bRet)
//		{
//			string username;
//			if(pUser != NULL)
//			{
//				int len = pFinished - pUser - 7;
//				if(len > 0 && len < 128)
//				{
//					char pname[128] = {'\0'};
//					strncpy(pname, pUser + 5, len);
//					username = pname;
//				}
//				else
//				{
//					username = "INVALID-STR";
//				}
//			}
//			else
//			{
//				char *pParam4 = strstr(recvbuf, "Param4:");
//				if(pParam4!=NULL && strstr(pParam4, "HIGH\n")!=NULL)
//				{
//					username = "ScheduledTask";
//					_ptzStatus[m_RtspClientArray[index].findinfo.channelid] = index;	
//				}
//				else
//					username = "LowVersionUser";
//			}
//			_ptz_user[m_RtspClientArray[index].findinfo.channelid] = username;
//			if(dwStop)
//				g_pLog->WriteLog("云台停止执行,%s\n", username.c_str(), m_RtspClientArray[index].findinfo.channelid);
//			else
//				g_pLog->WriteLog("云台开始执行,%s\n", username.c_str(), m_RtspClientArray[index].findinfo.channelid);
//		}
//		else
//		{
//			_ptz_user[m_RtspClientArray[index].findinfo.channelid] = "ControlFail";
//		}
//	}
//	else if(iter->second == index)
//	{
//		BOOL dwStop = atoi(szStop);
//		bRet = g_pManagerDVR->PTZControl(m_RtspClientArray[index].findinfo.channelid, nCommand, atoi(szParam1), atoi(szParam2), atoi(szParam3), dwStop);
//		if(dwStop)
//		{
//			_ptzStatus.erase(iter);
//			g_pLog->WriteLog("计划任务执行完成,%s\n", m_RtspClientArray[index].findinfo.channelid);
//		}
//	}
//	else
//		g_pLog->WriteLog("云台正在执行计划任务\n");
//	if(!bRet)
//		g_pLog->WriteLog("云台执行失败\n");
//    // 调用云台控制接口
// //   CChannel *pChannel = g_pManagerDVR->FindChannel(m_RtspClientArray[index].findinfo.channelid);
// //   if(pChannel == NULL)
// //   {
// //       g_pLog->WriteLog("rtspserv wrong number of channelid \n");
//	//	sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:wrong number of channelid！Finished:true");
// //       SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
// //       return FALSE;
// //   }
//	//
//	//char *pParam4 = strstr(recvbuf, "Param4:");
//	//if(pParam4!=NULL && strstr(pParam4, "HIGH\n")!=NULL)
//	//	pChannel->m_YTStatus=1;
//	//else if(pChannel->m_YTStatus==1)
//	//{
//	//	 g_pLog->WriteLog("channel is not allowed to control\n");
//	//	sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:channel is not allowed to control! Finished:true");
// //       SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
// //       return FALSE;
//	//}
//
//	//pChannel->PTZControlIndex=index;
//
//
//	//bRet = pChannel->PTZControl( nCommand, atoi(szParam1), atoi(szParam2), atoi(szParam3), atoi(szStop) );
//
//	//g_pLog->WriteLog("rtspserv %s exec ：%s\n", pChannel->m_type.c_str(), bRet ? "successfully!" : "failed!");
//	sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:%s\nMessage:%s！Finished:true", 
//		bRet ? "succeed" : "failed", bRet ? "exec successfully!" : "exec failed !");
//	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//
// //   if(pChannel->m_pDVR->m_type == HIK_DVRDH_CHANNEL )		// 大华
// //   {
// //       // 转换云台控制指令
//	//	int nCommand = atoi(szCommand);
//	//	if(nCommand < 0 || nCommand >= PTZ_MAX_COMMAND)
//	//	{
//	//		g_pLog->WriteLog("ptz command exec failed：invalide command value%d\n", nCommand);
//	//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:invalide command value！%d！Finished:true", nCommand);
//	//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//	//		return FALSE;
//	//	}
//
// //       CDHDVRChannel *pDHChannel = (CDHDVRChannel *)pChannel;
////        bRet = pDHChannel->PTZControl( DH_PTZ_COMMAND[nCommand], atoi(szParam1), atoi(szParam2), atoi(szParam3), atoi(szStop) );
//
// //       g_pLog->WriteLog("rtspserv DH exec ：%s\n", bRet ? "successfully!" : "failed!");
//	//	sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:%s\nMessage:%s！Finished:true", 
//	//		bRet ? "succeed" : "failed", bRet ? "exec successfully!" : "exec failed !");
//	//	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
// //   }
//	//else if(pChannel->m_pDVR->m_type == HIK_DVRHK_CHANNEL)
//	//{
// //       // 转换云台控制指令
//	//	int nCommand = atoi(szCommand);
//	//	if(nCommand < 0 || nCommand >= PTZ_MAX_COMMAND)
//	//	{
//	//		g_pLog->WriteLog("ptz command exec failed：invalide command value%d\n", nCommand);
//	//		sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:invalide command value！%d！Finished:true", nCommand);
//	//		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//	//		return FALSE;
//	//	}
//
// //       CHikNetDVRChannel *pHIKChannel = (CHikNetDVRChannel *)pChannel;
//	//	bRet = pHIKChannel->PTZControl(HIK_PTZ_COMMAND[nCommand], atoi(szParam1), atoi(szParam2), atoi(szParam3), atoi(szStop));
//
// //       g_pLog->WriteLog("rtspserv HIK exec ：%s\n", bRet ? "successfully！" : "failed!");
//	//	sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:%s\nMessage:%s！Finished:true", 
//	//		bRet ? "succeed" : "failed", bRet ? "exec successfully!" : "exec failed !");
//	//	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
//	//}
//	//else
//	//{
//	//	//str.Format("RTSP/1.0 200 OK\nPTZContResult:%s", bRet ? "Succeed" : "Failed");
//	//	//strcpy(buf, str.GetBuffer(str.GetLength()));
//	//	//SendRtspMessage(m_RtspClientArray[index].sock, buf, str.GetLength(), 0);
//	//	//str.ReleaseBuffer();
//	//	//return bRet;
//	//}
//
//	// 更新云台控制列表, 
//	// 部分本就不支持云台控制的设备将被忽略
//	//if(bRet)
//	//{
//	//	ItPtzStateMap iterState = m_mapPtzState.find(m_RtspClientArray[index].findinfo.channelid);
//	//	if(iterState != m_mapPtzState.end())
//	//	{
//	//		LpPtzState lpPtzState			= iterState->second;
//	//		lpPtzState->bUsing				= true;
//	//		lpPtzState->lasttime			= time(NULL);
//	//	}
//	//	else
//	//	{
//	//		LpPtzState lpPtzState			= new PTZState;
//	//		lpPtzState->bUsing				= true;
//	//		lpPtzState->channelid			= m_RtspClientArray[index].findinfo.channelid;
//	//		lpPtzState->lasttime			= time(NULL);
//	//		lpPtzState->m_SessionIndex		= index;
//	//		m_mapPtzState.insert(VtPtzStateMap(m_RtspClientArray[index].findinfo.channelid, lpPtzState));
//	//	}
//	//}
//
//    return bRet;
//}

BOOL CManagerRtsp::PTZCommand(char *recvbuf, int index)
{
    BOOL bRet = FALSE;
    if(index < 0 || index >= MAX_CLIENT_SIZE 
		|| m_RtspClientArray[index].sock == INVALID_SOCKET)
        return FALSE;

	char szMessage[1024] = {0};
	char buf[1024] = {0};

	string cameraid = m_RtspClientArray[index].findinfo.channelid;
	map<string ,PTZPush>::iterator iter_ctr = _ptzPushMap.find(cameraid);
	if(iter_ctr == _ptzPushMap.end() || iter_ctr->second.session != index || !CPushServer::isReady(iter_ctr->second.session))
	{
		g_pLog->WriteLog("云台操作未先获取资源\n");
		sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:PTZObtain\nFinished:true");
		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
		return TRUE;
	}

    char szCommand[128] = {0};
	char szParam1[128] = {0};
	char szParam2[128] = {0};
	char szParam3[128] = {0};
    char szStop[128] = {0};

    // 获取command参数
    char *pComamnd = strstr(recvbuf, "PTZCommand:");
	char *pParam1 = strstr(recvbuf, "Param1:");
	char *pParam2 = strstr(recvbuf, "Param2:");
	char *pParam3 = strstr(recvbuf, "Param3:");
    char *pStop = strstr(recvbuf, "Stop:");
    char *pFinished = strstr(recvbuf, "Finished:true");
    if(pComamnd == NULL || pParam1 == NULL ||
		pParam2 == NULL || pParam3 == NULL ||
		pStop == NULL || pFinished == NULL)
    {
		sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZContResult:failed\nMessage: wrong number of command parameters ！Finished:true");
		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
        return FALSE;
    }

	// "PTZCONTROL * RTSP/1.0\n CSeq:1\n PTZCommand:%d\n Param1:%d\n Param2:%d\n Param3:%d\n Stop:%d\n Finished:true"
    strncpy(szCommand, pComamnd + 11, pParam1 - pComamnd - 11);
    strncpy(szParam1, pParam1 + 7, pParam2 - pParam1 - 7);
	strncpy(szParam2, pParam2 + 7, pParam3 - pParam2 - 7);
	strncpy(szParam3, pParam3 + 7, pStop - pParam3 - 7);
	strncpy(szStop, pStop + 5, pFinished - pStop - 5);

	int nCommand = atoi(szCommand);
	if(nCommand < 0 || nCommand >= PTZ_MAX_COMMAND)
	{
		g_pLog->WriteLog("ptz command exec failed：invalide command value%d\n", nCommand);
		sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:failed\nMessage:invalide command value！%d！Finished:true", nCommand);
		SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);
		return FALSE;
	}

	BOOL dwStop = atoi(szStop);
	bRet = g_pManagerDVR->PTZControl(m_RtspClientArray[index].findinfo.channelid, nCommand, atoi(szParam1), atoi(szParam2), atoi(szParam3), dwStop );

	if(!bRet)
		g_pLog->WriteLog("云台执行失败\n");

	sprintf_s(szMessage, "RTSP/1.0 200 OK \nPTZCtrlResult:%s\nMessage:%s！Finished:true", 
		bRet ? "succeed" : "failed", bRet ? "exec successfully!" : "exec failed !");
	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);

    return bRet;
}

void CManagerRtsp::SendRtspMessage(int clientSocket, char * buf, int bufLength, int flag)
{
    int re = send(clientSocket, buf, bufLength, flag);
}

// 测试时候发现通道号不正确的时候，程序崩溃，需要修改下面的代码
BOOL CManagerRtsp::GetChannelInfo(char* channelname, CHANNEL_INFO& channelInfo)
{
	BOOL ret= g_pManagerDVR->FindChannel(channelname, channelInfo);
	return ret;
}

// 获取设备类型(大华的DVR,还是海思或者海康的，，，)
int GetChannelType(char *channelname)
{
   CPacket inPacket;
   DOMElement* AccNode = NULL;
   DOMElement* AccNode1 = NULL;
   DOMElement* AccNode2 = NULL;

   if(inPacket.BuiltTree(g_pSysInfo->m_XMLFilePath) == -1)
   {
       return -1;
   }
//   AccNode = inPacket.SearchElement("/所有摄像头/Domains/Domain/Cameras/Camera");
    AccNode = inPacket.SearchElement("/所有摄像头");
    AccNode = inPacket.SearchElement("Domains");
    AccNode = inPacket.SearchElement("Domain");
    AccNode = inPacket.SearchElement("Cameras");
    AccNode = inPacket.SearchElement("Camera");
   if(!AccNode)
   {
       return -1;
   }
   inPacket.SetCurrentElement(AccNode);
   while(AccNode)
   {
        AccNode1 = inPacket.SearchElement("strCameraID");
        CString str;
        if(strcmp(channelname,AccNode1->getTextContent()) != 0)
        {
            AccNode = AccNode->GetNextElement();
            inPacket.SetCurrentElement(AccNode);
            continue;
        }

        AccNode2 = inPacket.SearchElement("strProvider"); 
        if(AccNode2)
        {
            if(strcmp(AccNode2->getTextContent(),"海康") == 0)
            {
                return HIK_DVRHK_CHANNEL;
            }else if(strcmp(AccNode2->getTextContent(),"大华") == 0)
            {
                return HIK_DVRDH_CHANNEL;
            }else if(strcmp(AccNode2->getTextContent(),"松下") == 0)
            {
                return HIK_JPG_CHANNEL;
            }else if(strcmp(AccNode2->getTextContent(),"Axis") == 0)
            {
                return HIK_JPG_CHANNEL;
            }
        }
        AccNode = AccNode->GetNextElement();
        inPacket.SetCurrentElement(AccNode);
   }  
   return -1;
}

void CManagerRtsp::CloseCli(int index)
{
    if(index < 0)
        return ;

    if(index >= MAX_CLIENT_SIZE)
        return ;

    if (m_RtspClientArray[index].sock != INVALID_SOCKET)
    {
		//CChannel *pChannel = g_pManagerDVR->FindChannel(m_RtspClientArray[index].findinfo.channelid);
		//if(pChannel != NULL && pChannel->PTZControlIndex == index)
		//{
		//	pChannel->PTZControl( 0, 0, 0, 0, 1);
		//}

		g_pLog->WriteLog("客户端RTSP关闭%d，session:%d;index:%d\n", m_RtspClientArray[index].sock, (int)m_RtspClientArray[index].findinfo.session, index);
		m_RtspClientArray[index].findinfo.session = 0;
        closesocket(m_RtspClientArray[index].sock);
        m_RtspClientArray[index].sock = INVALID_SOCKET;
    }

    m_ClientNum--;
    if(m_ClientNum == 0)
    {
        m_ClientNum = 0;
    }
}

BOOL CManagerRtsp::OptionAnswer(char *recvbuf, int index)
{
    if(index < 0)
	{
		// g_pLog->WriteLog("receive msg from %d, session=%d, cmd PTZCONTROL\n", m_RtspClientArray[index].sock, index);
        return FALSE;
	}

    if(index >= MAX_CLIENT_SIZE)
        return FALSE;

    if (m_RtspClientArray[index].sock != INVALID_SOCKET)
    {
        char *p = strstr(recvbuf,"file-play");
        if(p == NULL)
        {
            p = strstr(recvbuf,"live-play");
            if(p == NULL)
            {
                //CloseCli(index);
                return FALSE;
            }
            else
            {
                m_RtspClientArray[index].findinfo.type = LIVE_PLAY;
            }
        }
        else
        {
            m_RtspClientArray[index].findinfo.type = FILE_PLAY;
        }

        p = strstr(recvbuf,"Token:");
        if(p == NULL)
        {
            //CloseCli(index);
            return FALSE;
        }

        p = p + 4;
        if(!CheckPsw(p))
        {
            //CloseCli(index);
            return FALSE;
        }

        char buf[1024];
        CString str;
        str.Format("RTSP/1.0 200 OK\nCSeq:1\nPublic:SETUP,TEARDOWN,PLAY,PAUSE");
        strcpy(buf, str.GetBuffer(str.GetLength()));
        str.ReleaseBuffer();

        int re = send(m_RtspClientArray[index].sock, buf, str.GetLength(),0);
        if(re <= 0)
        {
            //CloseCli(index);
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL CManagerRtsp::SetupAnswer(char *recvbuf,int index)
{
    if(index < 0)
        return FALSE;
    if(index >= MAX_CLIENT_SIZE)
        return FALSE;

    if (m_RtspClientArray[index].sock != INVALID_SOCKET)
    {
        char *p = strstr(recvbuf,"rtsp://mediaserver/");
        if(p == NULL)
        {
            //CloseCli(index);
            return FALSE;
        }

        p = p +19;
        char ch[128];
        ch[0] ='\0'; 
        int i;
        for(i = 0; i < 126; i++)
        {
            if(*p == ' ')
            {
                break;
            }else{
                ch[i] = *p;
                ch[i+1] = '\0'; 
                p++;
            }
        }
        if(ch[0] =='\0')
        {
            //CloseCli(index);
            return FALSE;
        }
        strcpy(m_RtspClientArray[index].findinfo.channelid, ch);


		CHANNEL_INFO channelInfo={};
		channelInfo.ChannelType = -1;

        if(m_RtspClientArray[index].findinfo.type == FILE_PLAY)
        {
			char file_name[128]={};
			_splitpath(ch, 0, 0, file_name, 0);
			if(file_name[0]=='D' && file_name[1] =='H')
				channelInfo.ChannelType = HIK_DVRDH_CHANNEL;
			else if(file_name[0]=='H' && file_name[1] =='K')
				channelInfo.ChannelType = HIK_DVRHK_CHANNEL;
			else
				g_pLog->WriteLog("unknown file type:%s!\n", file_name);

            _int64 starttime, endtime;

            p = strstr(recvbuf,"Starttime:");
            if(p == NULL)
            {
                //CloseCli(index);
                return FALSE;
            }
            p = p+10;

            ch[0] ='\0'; 
            for( i = 0; i < 126; i++)
            {
                if(*p != '\n')
                {
                    ch[i] = *p;
                    ch[i+1] = '\0'; 
                    p++;
                }else{
                    break;
                }
            }
            if(ch[0] =='\0')
            {
                //CloseCli(index);
                return FALSE;
            }
            starttime = _atoi64(ch);

            p = strstr(recvbuf,"Endtime:");
            if(p == NULL)
            {
                //CloseCli(index);
                return FALSE;
            }
            p = p + 8;
            endtime = _atoi64(p);
            m_RtspClientArray[index].findinfo.starttime = starttime;
            m_RtspClientArray[index].findinfo.endtime = endtime;
		}
		else
		{
			GetChannelInfo(m_RtspClientArray[index].findinfo.channelid, channelInfo);
		}
//		CTime curtime = CTime::GetCurrentTime();
        m_RtspClientArray[index].findinfo.session = m_SessionIndex;
        m_SessionIndex++;

		char buf[1024]={};
        CString str;

        if(channelInfo.ChannelType == -1)
        {
			g_pLog->WriteLog("error channel id(%s) or channel type(%d)!\n", m_RtspClientArray[index].findinfo.channelid, channelInfo.ChannelType);
            str.Format("RTSP/1.0 400 Bad\nServer:GentekServer\nCseq:1\nSession:-1 \nTransport:0;server_port=0");
			strcpy(buf, str.GetBuffer(str.GetLength()));
			str.ReleaseBuffer();
			int re = send(m_RtspClientArray[index].sock, buf, str.GetLength(), 0);
            // CloseCli(index);
            return FALSE;
        }
        
        m_RtspClientArray[index].findinfo.encodetype =  channelInfo.ChannelType;

        if( channelInfo.ChannelType == HIK_DVRHK_CHANNEL)
        {
            if(channelInfo.isMulticast)
            {
                str.Format("RTSP/1.0 200 OK\nServer:GentekServer\nCseq:1\nSession:%I64d \nTransport:hk\nCast:MultiCast;servermulticast_ip=%s\nservermulticast_port=%d",
                    m_RtspClientArray[index].findinfo.session, channelInfo.MultiCastIP,channelInfo.MultiPort);
            }
            else{
                str.Format("RTSP/1.0 200 OK\nServer:GentekServer\nCseq:1\nSession:%I64d \nTransport:hk;server_port=%d",
                    m_RtspClientArray[index].findinfo.session, g_pSysInfo->m_FilePort);
            }
        }
        else if( channelInfo.ChannelType == HIK_DVRDH_CHANNEL)
        {
            if(channelInfo.isMulticast)
            {
                str.Format("RTSP/1.0 200 OK\nServer:GentekServer\nCseq:1\nSession:%I64d \nTransport:dh\nCast:MultiCast;servermulticast_ip=%s\nservermulticast_port=%d",
                    m_RtspClientArray[index].findinfo.session, channelInfo.MultiCastIP,channelInfo.MultiPort);
            }
            else{
				str.Format("RTSP/1.0 200 OK\nServer:GentekServer\nCseq:1\nSession:%I64d \nTransport:dh;server_port=%d",
                m_RtspClientArray[index].findinfo.session, g_pSysInfo->m_FilePort);}
        }
        else if( channelInfo.ChannelType == HIK_JPG_CHANNEL)
        {
            if(channelInfo.isMulticast)
                {str.Format("RTSP/1.0 200 OK\nServer:GentekServer\nCseq:1\nSession:%I64d\nTransport:dh\nCast:MultiCast;servermulticast_ip=%s\nservermulticast_port=%d",
                    m_RtspClientArray[index].findinfo.session, channelInfo.MultiCastIP,channelInfo.MultiPort);}
            else{
                str.Format("RTSP/1.0 200 OK\nServer:GentekServer\nCseq:1\nSession:%I64d\nTransport:jpg;server_port=%d",
                m_RtspClientArray[index].findinfo.session, g_pSysInfo->m_FilePort);
            }
        }

        strcpy(buf, str.GetBuffer(str.GetLength()));
        str.ReleaseBuffer();
        g_pLog->WriteLog("SETUP session=%d;index=%d;channel=%s;socket=%d;NETWORK=%d\n", (int)m_RtspClientArray[index].findinfo.session, index, channelInfo.ChannelName, m_RtspClientArray[index].sock, channelInfo.isMulticast);

        int re = send(m_RtspClientArray[index].sock, buf, str.GetLength(), 0);
        if(re <= 0)
        {
            //CloseCli(index);
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL CManagerRtsp::PlayAnswer(char *recvbuf,int index)
{
    if(index < 0)
        return FALSE;

    if(index >= MAX_CLIENT_SIZE)
        return FALSE;

    if (m_RtspClientArray[index].sock != INVALID_SOCKET)
    {
        if(m_RtspClientArray[index].findinfo.type == FILE_PLAY)
        {
            int speed;
            char *p = strstr(recvbuf, "Speed:");

            if(p == NULL)
            {
                //CloseCli(index);
                return FALSE;
            }

            p = p+6;
            char ch[128];
            ch[0] ='\0'; 

            int i;

            for(i = 0; i < 126; i++)
            {
                if(*p == ' ')
                {
                    break;
                }else
                {
                    ch[i] = *p;
                    ch[i+1] = '\0'; 
                    p++;
                }
            }
            if(ch[0] == '\0')
            {
                //CloseCli(index);
                return FALSE;
            }
            speed = atoi(ch);

            p = strstr(recvbuf, "Range:");
            if(p == NULL)
            {
                //CloseCli(index);
                return FALSE;
            }

            p = p + 6;
            int startpos = atoi(p);

            i = g_pManagerConnect->FindClient(m_RtspClientArray[index].findinfo.session);
            if(i == -1)
            {
                //CloseCli(index);
                return FALSE;
            }

            g_pManagerConnect->m_SocketArray[i]->m_Speed = speed;
            if(startpos == -1)
                g_pManagerConnect->m_SocketArray[i]->ContinueNetPlay();
            else
             {
				 //g_pManagerConnect->m_SocketArray[i]->StartNetPlay((_int64)startpos);
				g_pManagerConnect->m_SocketArray[i]->StartNetPlayOne();
			}

            g_pLog->WriteLog("fileplay:channelid = %s\n",m_RtspClientArray[index].findinfo.channelid);

            char buf[1024];
            CString str;

            str.Format("RTSP/1.0 200 OK\nCSeq:2\nRange:%d",startpos );
            strcpy(buf, str.GetBuffer(str.GetLength()));

            str.ReleaseBuffer();
            int re = send(m_RtspClientArray[index].sock, buf, str.GetLength(),0);

            if(re <= 0)
            {
                //CloseCli(index);
                return FALSE;
            }
        }
        else if(m_RtspClientArray[index].findinfo.type == LIVE_PLAY)
        {
            int i = g_pManagerConnect->FindClient(m_RtspClientArray[index].findinfo.session);
            if(i == -1)
            {
                //CloseCli(index);
                return FALSE;
            }
			g_pManagerDVR->addClient(m_RtspClientArray[index].findinfo.channelid,g_pManagerConnect->m_SocketArray[i]->m_Socket);
    //        CChannel *pChannel = g_pManagerDVR->FindChannel(m_RtspClientArray[index].findinfo.channelid);
    //        if(pChannel == NULL)
    //        {
    //            //CloseCli(index);
    //            g_pLog->WriteLog("未找到该通道\n");
    //            return FALSE;
    //        }

    //        if(pChannel->m_pDVR->m_type == HIK_DVRDH_CHANNEL )
    //        {
    //            if(!pChannel->m_ChannelInfo.isMulticast)
    //            {          
    //                pChannel->AddTrans(g_pManagerConnect->m_SocketArray[i]->m_Socket);

				//	 g_pLog->WriteLog("PLAY: DH live session=%d;index=%d;channelid=%s;socket=%d\n", (int)m_RtspClientArray[index].findinfo.session, index, m_RtspClientArray[index].findinfo.channelid, g_pManagerConnect->m_SocketArray[i]->m_Socket);
    //            }
    //        }
    //        else if(pChannel->m_pDVR->m_type ==HIK_DVRHK_CHANNEL )
    //        {
    //            pChannel->AddTrans(g_pManagerConnect->m_SocketArray[i]->m_Socket);

				//g_pLog->WriteLog("PLAY: HK live session=%d;index=%d;channelid=%s;socket=%d\n", (int)m_RtspClientArray[index].findinfo.session, index, m_RtspClientArray[index].findinfo.channelid, g_pManagerConnect->m_SocketArray[i]->m_Socket);
    //        }
    //        else
    //        {
    //            //CloseCli(index);
    //            return FALSE;
    //        }

            char buf[1024];
            CString str;

            str.Format("RTSP/1.0 200 OK\nCSeq:2");
            strcpy(buf, str.GetBuffer(str.GetLength()));
            str.ReleaseBuffer();

            int re = send(m_RtspClientArray[index].sock, buf, str.GetLength(),0);
            if(re <= 0)
            {
                //CloseCli(index);
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

BOOL CManagerRtsp::PauseAnswer(char *recvbuf,int index)
{
    if(index < 0)
        return FALSE;

    if(index >= MAX_CLIENT_SIZE)
        return FALSE;

    if (m_RtspClientArray[index].sock != INVALID_SOCKET)
    {
        if(m_RtspClientArray[index].findinfo.type == FILE_PLAY)
        {
            int i = g_pManagerConnect->FindClient(m_RtspClientArray[index].findinfo.session);
            if(i == -1)
            {
                //CloseCli(index);
                return FALSE;
            }
            g_pManagerConnect->m_SocketArray[i]->PauseNetPlay();
        }

        char buf[1024];
        CString str;

        str.Format("RTSP/1.0 200 OK\nCSeq:3");
        strcpy(buf, str.GetBuffer(str.GetLength()));

        str.ReleaseBuffer();
        int re = send(m_RtspClientArray[index].sock, buf, str.GetLength(), 0);

        if(re <= 0)
        {
            //CloseCli(index);
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL CManagerRtsp::TeardownAnswer(char *recvbuf,int index)
{
    if(index < 0)
        return FALSE;
    if(index >= MAX_CLIENT_SIZE)
        return FALSE;

    if (m_RtspClientArray[index].sock != INVALID_SOCKET)
    {
        char buf[1024];
        CString str;
        str.Format("RTSP/1.0 200 OK\nCSeq:3");
        strcpy(buf, str.GetBuffer(str.GetLength()));
        str.ReleaseBuffer();
        int re = send(m_RtspClientArray[index].sock, buf, str.GetLength(),0);
        
        int i = g_pManagerConnect->FindClient(m_RtspClientArray[index].findinfo.session);
        if(i == -1)
        {
            //CloseCli(index);
            return FALSE;
        }

        //g_pManagerConnect->stop_Client(i);

        //CloseCli(index);
        return TRUE;
    }
    return FALSE;
}

BOOL CManagerRtsp::CheckPsw(char *psw)
{
    return TRUE;
}

/*
*接收字符："PTZOBTAIN * RTSP/1.0\n CSeq:%d\n User:%s\n Priority:%s\n Finished:true"
*发送字符："RTSP/1.0 200 OK\n PTZOBTAIN Result:%s\n Port:%d\n Push:%s\n Finished:true"
*/
BOOL CManagerRtsp::PTZObtain(char *recvbuf,int index)
{
	if(index < 0 || index >= MAX_CLIENT_SIZE || m_RtspClientArray[index].sock == INVALID_SOCKET)
	{
		return  FALSE;
	}
		
	BOOL bPush = TRUE;
	int port=0;
	bPush = CPushServer::getPort(port);

	string ptUser="";
	char *pUser = strstr(recvbuf, "User:");
	char *pPry = strstr(recvbuf, "Priority");
	if(pUser!=NULL && pPry!=NULL)
	{
		int len = pPry - pUser - 7;
		if(len >0 && len < 128)
		{
			char pname[128] = {0};
			strncpy(pname, pUser + 5, len);
			ptUser = pname;
		}
		if(strstr(recvbuf, "Priority:HIGH") == NULL)
			ptUser = "USER:" + ptUser;
		else
			ptUser = "ADMIN:" + ptUser;
	}

	BOOL bRet = TRUE;
	string cameraid = m_RtspClientArray[index].findinfo.channelid;
	map<string ,PTZPush>::iterator iter = _ptzPushMap.find(cameraid);
	if(iter != _ptzPushMap.end() && iter->second.msg.c_str()[0] == 'A')
	{
		bRet = FALSE;
	}
	else
	{
		if(iter != _ptzPushMap.end())
		{
			CPushServer::pushMSG(iter->second.session, ptUser.c_str(), strlen(ptUser.c_str()));
		}

		PTZPush ptzpush;
		ptzpush.session = index;
		ptzpush.msg = ptUser;
		_ptzPushMap[cameraid]=ptzpush;
	}
	char szMessage[1024] = {0};
	sprintf_s(szMessage, "RTSP/1.0 200 OK\n PTZOBTAIN Result:%s\n Port:%d\n Push:%d\n Finished:true", bRet ? "succeed" : "failed",port, index);
	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);

	return TRUE;
}

/*
*不返回信息
*/
BOOL CManagerRtsp::PTZRelease(char *recvbuf,int index)
{
	//BOOL bRet = FALSE;
	string cameraid = m_RtspClientArray[index].findinfo.channelid;
	map<string ,PTZPush>::iterator iter = _ptzPushMap.find(cameraid);
	if(iter != _ptzPushMap.end() && iter->second.session == index)
	{
		_ptzPushMap.erase(iter);
		//bRet = TRUE;
	}
	/*char szMessage[1024] = {0};
	sprintf_s(szMessage, "RTSP/1.0 200 OK\n PTZRELEASE Result:%s\n Finished:true", bRet ? "succeed" : "failed");
	SendRtspMessage(m_RtspClientArray[index].sock, szMessage, strlen(szMessage), 0);*/

	return TRUE;
}

BOOL CManagerRtsp::PTZGetDirection(char *recvbuf,int index)
{
	return FALSE;
}
