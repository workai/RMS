#include "HikNetDVRChannel.h"
#include "StdHeader.h"
#include "HCNetSDK.h"
#include "Log.h"
#include "managerDvr.h"
#include "ReformMediaServerDef.h"
#include "dvr.h"
#include "HikNetDVR.h"
#include "VideoTrans.h"
#include "MultiCastVideoTrans.h"
//#include "ManagerRtsp.h"
#include "Config.h"
#include "DBObject.h"
#include "PlayM4.h"
//#include "plaympeg4.h"
#include <iostream>

//CHikNetDVRChannel::CHikNetDVRChannel(void)
//{
//	m_type_ID = "HK";
//	m_type = "海康";
//	m_pDVR = NULL;
//	m_Playhandle = -1;
//
//	m_bTrans = FALSE;
//	m_SocketNum = 0;
//
//	icon_data_ = new unsigned char[ICON_HK_BUFFER_LEN];
//
//	icon_port = -1;
//	//init
//	is_icon_init_= false;
//	BYTE pHeader[40]={73,77,75,72,1,1,0,0,2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//	BYTE* header=pHeader;
//	DWORD ret = 0;
//	if(!PlayM4_GetPort(&icon_port))
//	{
//		g_pLog->WriteLog("抓图失败,%s,获取端口号失败\n", m_type.c_str());
//	}
//	else if(!PlayM4_SetStreamOpenMode(icon_port, STREAME_REALTIME))
//	{
//		ret = PlayM4_GetLastError(icon_port);
//		g_pLog->WriteLog("抓图失败,%s,SETMODE(%d)\n", m_type.c_str(), ret);
//	}
//	else if(!PlayM4_OpenStream(icon_port, header, 40, 2*1024 * 1024))
//	{
//		ret = PlayM4_GetLastError(icon_port);
//		g_pLog->WriteLog("抓图失败,%s,OPEN(%d)\n", m_type.c_str(), ret);
//	}
//	else if(!PlayM4_Play( icon_port, 0))
//	{
//		ret = PlayM4_GetLastError(icon_port);
//		g_pLog->WriteLog("抓图失败,%s,PLAY(%d)\n", m_type.c_str(), ret);
//	}
//	else
//		is_icon_init_ = true;
//
//	m_mapYTCmd[PTZ_UP]=TILT_UP;
//	m_mapYTCmd[PTZ_DOWN]=TILT_DOWN;
//	m_mapYTCmd[PTZ_LEFT]=PAN_LEFT;
//	m_mapYTCmd[PTZ_RIGHT]=PAN_RIGHT;
//	m_mapYTCmd[PTZ_ZOOM_ADD]=ZOOM_IN;
//	m_mapYTCmd[PTZ_ZOOM_DEC]=ZOOM_OUT;
//	m_mapYTCmd[PTZ_FOCUS_ADD]=FOCUS_NEAR;
//	m_mapYTCmd[PTZ_FOCUS_DEC]=FOCUS_FAR;
//	m_mapYTCmd[PTZ_APERTURE_ADD]=IRIS_OPEN;
//	m_mapYTCmd[PTZ_APERTURE_DEC]=IRIS_CLOSE;
//
//	//m_mapYTCmd[10]=GOTO_PRESET;
//	//m_mapYTCmd[11]=SET_PRESET;
//	
//	m_mapYTCmd[PTZ_LEFTUP]=UP_LEFT;
//	m_mapYTCmd[PTZ_RIGHTUP]=UP_RIGHT;
//	m_mapYTCmd[PTZ_LEFTDOWN]=DOWN_LEFT;
//	m_mapYTCmd[PTZ_RIGHTDOWN]=DOWN_RIGHT;
//}
//
//CHikNetDVRChannel::~CHikNetDVRChannel(void)
//{
//	StopTrans();
//	delete [] icon_data_;
//
//	if(icon_port >0)
//	{
//		PlayM4_Stop(icon_port);
//		PlayM4_CloseStream(icon_port);
//		PlayM4_FreePort(icon_port);
//		icon_port = -1;
//	}
//
//}
//
//void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, DWORD dwUser)
//{	
//	switch(dwDataType)
//	{
//		case NET_DVR_SYSHEAD:
//		case NET_DVR_STREAMDATA:   //码流数据
//		if (dwBufSize > 0 )
//		{
//			CHikNetDVRChannel* pThis = (CHikNetDVRChannel*)dwUser;
//
//			// 把获取的数据发送出去
//			if(pThis != NULL && dwBufSize >0)
//			{
//				pThis->SendBuffer((char *)pBuffer, dwBufSize, FALSE);
//				//pThis->SaveRecord((char *)pBuffer, dwBufSize);
//				pThis->save_record((char *)pBuffer, dwBufSize);
//			}
//		}
//		break;
//		default:break;
//	}
//}
//
//BOOL CHikNetDVRChannel::InitMultiCast()
//{
//
//	// 多播方式
//	if(m_ChannelInfo.isMulticast)
//	{
//		WSADATA wsd;
//		struct sockaddr_in remote;
//		SOCKET sock,sockM;
//
//		//char sendbuf[BUFSIZE];
//		int len = sizeof( struct sockaddr_in);
//
//		//初始化WinSock2.2
//		if( WSAStartup( MAKEWORD(2,2),&wsd) != 0 )
//		{
//			g_pLog->WriteErr("WSAStartup() failed\n");
//			return -1;
//		}
//
//		if((sock = WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0, WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF |	WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
//		{
//			//printf("socket failed with:%d\n",WSAGetLastError());
//			g_pLog->WriteErr("组播socket打开错误");
//			WSACleanup();
//			return FALSE;
//		}
//
//		BOOL bReuseaddr = TRUE;
//		int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReuseaddr, sizeof(BOOL));
//		if(0 != ret)
//		{
//			g_pLog->WriteErr("setsockopt() error");
//			return FALSE;
//		}
//
//		// 加入多播组
//		remote.sin_family = AF_INET;
//		//remote.sin_port = htons(MCASTPORT);
//		//remote.sin_port=htons(m_ChannelInfo.MultiPort);
//		remote.sin_port = m_ChannelInfo.MultiPort;
//		remote.sin_addr.s_addr = inet_addr(m_ChannelInfo.MultiCastIP);
//
//		if(( sockM = WSAJoinLeaf(sock, (SOCKADDR*)&remote, sizeof(remote), NULL, NULL, NULL, NULL, JL_BOTH)) == INVALID_SOCKET)
//		{
//			g_pLog->WriteErr("添加到组播错误");
//			closesocket(sock);
//			WSACleanup();
//			return FALSE;
//		}
//
//		// 打开组播
//		m_MultiCastVedio = new CMultiCastVideoTrans();
//		m_MultiCastVedio->Init(sockM, remote);
//	}
//	StartTrans();
//
//	return TRUE;
//}
//
//// 初始化通道，并请求视频数据
//BOOL CHikNetDVRChannel::Init(int ChannelID, int index, int DVRChannelID, CDVR *pDVR)
//{	
//	m_pDVR			= pDVR;					// DVR设备指针
//	m_DVRChannelID	= DVRChannelID;			// 通道在DVR中的顺序号
//	m_index			= index;				// 
//	m_ChannelInfo.ChannelID = ChannelID;	// 通道在DVR中的顺序号
//	m_strCameraId	=  pDVR->m_strCameraId;			// 摄像头编号
//
//	sprintf(m_ChannelInfo.ChannelName, "通道%d", ChannelID);
//
//	m_ChannelInfo.ChannelType = HIK_DVRHK_CHANNEL;
//	strcpy_s(m_ChannelInfo.ServerIP, m_pDVR->m_strServIp);
//
//	m_work_status = 1;
//	//StartTrans();
//	return TRUE;
//}
//
//// 启动录像
////BOOL CHikNetDVRChannel::StartRecord(BOOL bRecord, char * pPath)
////{
////	if(bRecord)
////	{
////		if(fp)
////		{
////			fclose(fp);
////			
////			// 数据入库
////			char szTime[128] = {0};
////			SYSTEMTIME loctime;
////			GetLocalTime(&loctime);	
////			sprintf_s(szTime, "%.4d%.2d%.2d%.2d%.2d%.2d", loctime.wYear, loctime.wMonth, loctime.wDay, loctime.wHour, loctime.wMinute, loctime.wSecond );
////			m_recordId = g_dbobj.InsertReocord("大华", m_strDVRName.c_str(), m_strCameraId.c_str() ,
////				m_DVRChannelID + 1,m_pDVR->m_strServIp.c_str(), 0, 1, m_strRecordPath.c_str(), m_strRecordName.c_str(), m_strLastSaveTime.c_str(), szTime);
////		}
////
////		//m_strRecordPath = pPath;
////		fp = fopen(pPath, "wb");
////		if(fp)
////		{
////
////			char szTime[128] = {0};
////
////			SYSTEMTIME loctime;
////			GetLocalTime(&loctime);	
////
////			sprintf_s(szTime, "%.4d%.2d%.2d%.2d%.2d%.2d", loctime.wYear, loctime.wMonth, loctime.wDay, loctime.wHour, loctime.wMinute, loctime.wSecond );
////			m_strLastSaveTime = szTime;
////
////		}
////
////		g_pLog->WriteLog( "海康开始录像成功！\t%s:%d,通道%d\n",m_ChannelInfo.ServerIP,m_pDVR->m_ServerPort, m_ChannelInfo.ChannelID);
////	}
////	else
////	{
////		m_strRecordPath = "";
////		fclose(fp);
////		g_dbobj.UpdateRecord(m_recordId);
////		g_pLog->WriteLog( "海康停止录像成功！\t%s:%d,通道%d\n",m_ChannelInfo.ServerIP,m_pDVR->m_ServerPort, m_ChannelInfo.ChannelID);
////	}
////
////	m_bRecord = bRecord;
////	return TRUE;
////
////}
//
//// 开启视频请求
//BOOL CHikNetDVRChannel::StartTrans()
//{
//	 // 启动预览并设置回调数据流
//	//LONG lRealPlayHandle;
//
//	NET_DVR_PREVIEWINFO struPlayInfo = {0};
//	struPlayInfo.hPlayWnd		= 0;		// 需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
//	struPlayInfo.dwStreamType	= 0;		// 0-主码流，1-子码流，2-码流3，3-码流4，以此类推
//	struPlayInfo.dwLinkMode		= 0;		// 0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
//	struPlayInfo.lChannel		= m_ChannelInfo.ChannelID;		// 预览通道号
//
//	/*NET_DVR_CLIENTINFO struPlayInfo={0};
//	struPlayInfo.hPlayWnd=0;
//	struPlayInfo.lChannel=m_ChannelInfo.ChannelID;
//	struPlayInfo.lLinkMode=0;
//	struPlayInfo.sMultiCastIP=NULL;
//	m_Playhandle = NET_DVR_RealPlay_V30(((CHikNetDVR *)m_pDVR)->m_UserID, &struPlayInfo, NULL, this);*/
//
//	m_Playhandle = NET_DVR_RealPlay_V40(m_pDVR->m_UserID, &struPlayInfo, NULL,this);
//	if (m_Playhandle < 0)
//	{
//		g_pLog->WriteLog("海康 NET_DVR_RealPlay_V40 失败\n");
//		return FALSE;
//	}
//	m_bTrans= NET_DVR_SetRealDataCallBack(m_Playhandle,g_RealDataCallBack_V30,(DWORD)this);
//	if(!m_bTrans)
//		g_pLog->WriteLog("海康 NET_DVR_SetRealDataCallBack 失败\n");
//	else
//	{
//		g_pLog->WriteLog("%s海康开始转发\t%s:%d,通道%d\n", m_ChannelInfo.ChannelName, m_ChannelInfo.ServerIP,m_pDVR->m_ServerPort, m_ChannelInfo.ChannelID);
//	}
//	return m_bTrans;
//}
//
//BOOL CHikNetDVRChannel::PostStopTrans()
//{
//	return TRUE;
//
//	::PostMessage(NULL, WM_STOPTRANS_COMMANDSOCKET, m_pDVR->m_serverindex, m_ChannelInfo.ChannelID);
///*
//	CLIENT_SetRealDataCallBackEx(m_Playhandle,NULL, (DWORD)m_index,0x00000001);
//	m_bTrans = FALSE;
//	CLIENT_StopRealPlay(m_Playhandle);*/
//	return TRUE;
//}
//
//BOOL CHikNetDVRChannel::StopTrans()
//{
//	// 清除回调函数
//	if (!NET_DVR_SetRealDataCallBack(m_Playhandle, NULL, (DWORD)m_index))
//	{
//		g_pLog->WriteLog("NET_DVR_SetRealDataCallBack error\n");
//	}
//
//	m_bTrans = FALSE;
//
//	// 停止获取录像
//	NET_DVR_StopRealPlay(m_Playhandle);
//
//	return TRUE;
//}
//
//BOOL CHikNetDVRChannel::SetImageParam(IMAGEPARAM_INFO info)
//{
//	return TRUE;
//}
//
//BOOL CHikNetDVRChannel::GetPic(char* sFileName)
//{
//	return TRUE;
//}
//
// 云台控制
BOOL CHikNetDVRChannel::_PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop)
{	
	//DWORD terr;
	//DWORD tsize = sizeof(NET_DVR_PTZPOS);
	//NET_DVR_PTZPOS tbuf={0};
	///*DWORD tsize = 1024;
	//PBYTE tbuf=new BYTE[tsize];*/
	//DWORD rsize=0;
	//BOOL test = NET_DVR_GetDVRConfig(_LoginID,NET_DVR_GET_PTZPOS, _ChannelID, (&tbuf), tsize, &rsize);
	//if(test == FALSE)
	//{
	//	terr = NET_DVR_GetLastError();
	//}

	if(_Playhandle < 0)
		return FALSE;
	if(m_bControl)
	{
		NET_DVR_PTZControlWithSpeed(_Playhandle,m_lastCmd, 1,param2);
		m_bControl=FALSE;
	}
	if(dwStop)
	{
		return TRUE;
	}

	BOOL bRet = FALSE;
	switch(dwPTZCommand)
	{
	case PTZ_POINT_MOVE:
		{
			bRet = NET_DVR_PTZPreset(_Playhandle,GOTO_PRESET,120);
		}
		break;
	case PTZ_POINT_SET:
		{
			bRet = NET_DVR_PTZPreset(_Playhandle,SET_PRESET,120);
		}
		break;
	case PTZ_WIPER_OPEN:
		{
			bRet = NET_DVR_PTZControlWithSpeed(_Playhandle,WIPER_PWRON,0,1);
		}
		break;
	case PTZ_WIPER_CLOSE:
		{
			bRet = NET_DVR_PTZControlWithSpeed(_Playhandle,WIPER_PWRON,1,1);
		}
		break;
	case PTZ_THROUGHFOG_OPEN:
		{
			NET_DVR_CAMERA_DEHAZE_CFG lpBuffer;
			lpBuffer.byDehazeMode=2;
			bRet=NET_DVR_SetDVRConfig(_LoginID,NET_DVR_SET_CAMERA_DEHAZE_CFG,_ChannelID,&lpBuffer,sizeof(lpBuffer));
		}
		break;
	case PTZ_THROUGHFOG_CLOSE:
		{
			NET_DVR_CAMERA_DEHAZE_CFG lpBuffer;
			lpBuffer.byDehazeMode=0;
			bRet=NET_DVR_SetDVRConfig(_LoginID,NET_DVR_SET_CAMERA_DEHAZE_CFG,_ChannelID,&lpBuffer,sizeof(lpBuffer));
		}
		break;
	default:
		{
			map<int, int>::const_iterator iter = m_mapYTCmd.find(dwPTZCommand);
			if(iter != m_mapYTCmd.end())
			{
				int CMD = iter->second;
				bRet = NET_DVR_PTZControlWithSpeed(_Playhandle,CMD,0,param2);
				if(bRet)
				{
					m_bControl=TRUE;
					m_lastCmd = CMD;
				}
			}
			else
			{
				g_pLog->WriteLog("不识别的海康云台控制命令\n");
			}
		}
		break;
	}

	if(!bRet)
	{
		g_pLog->WriteLog("执行海康云台控制命令失败:%s！\n", NET_DVR_GetErrorMsg());
		return FALSE;
	}
	else
	{
		g_pLog->WriteLog("海康云台控制id=%d, %u;;%u;;%u;;%u;;%u\n", _Playhandle, dwPTZCommand, param1, param2, param3, dwStop);
	}
	return TRUE;
}
//
//void CHikNetDVRChannel::save_icon(char *pbuffer, DWORD length)
//{
//	if(!is_icon_init_)
//		return;
//	if(!is_ready_icon_)
//	{
//		PlayM4_InputData(icon_port, (unsigned char*)pbuffer, length);
//
//		//PlayM4_GetCropJPEG(icon_port, );
//		if(PlayM4_GetJPEG(icon_port, icon_data_, ICON_HK_BUFFER_LEN, &icon_len_))
//		{
//			FILE* file = fopen(icon_path, "wb+");
//			if(fwrite(icon_data_, 1, icon_len_, file) == icon_len_)
//			{
//				is_ready_icon_ = true;
//				//PlayM4_Stop(icon_port);
//				//PlayM4_CloseStream(icon_port);
//				//PlayM4_FreePort(icon_port);
//				//icon_port = -1;
//
//				g_pLog->WriteLog("抓图成功,%s, %s\n", m_type.c_str(), icon_path);
//			}
//			fclose(file);
//		}
//		else
//		{
//			g_pLog->WriteLog("正在抓图,%s, %s ,w:%d,h:%d, id:%d\n", m_type.c_str(), icon_path, ICON_WIDTH, ICON_HEIGHT, icon_port);
//		}
//	}
//}
//
//BOOL CHikNetDVRChannel::Init(LONG lLoginID, int nChannelID)
//{
//	BOOL ret = TRUE;
//	NET_DVR_PREVIEWINFO struPlayInfo = {0};
//	struPlayInfo.hPlayWnd		= 0;		// 需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
//	struPlayInfo.dwStreamType	= 0;		// 0-主码流，1-子码流，2-码流3，3-码流4，以此类推
//	struPlayInfo.dwLinkMode		= 0;		// 0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
//	struPlayInfo.lChannel		= nChannelID;		// 预览通道号
//
//	m_Playhandle = NET_DVR_RealPlay_V40(lLoginID, &struPlayInfo, NULL,this);
//	if (m_Playhandle < 0)
//	{
//		g_pLog->WriteLog("海康 NET_DVR_RealPlay_V40 失败\n");
//		ret = FALSE;
//	}
//	else if(!(m_bTrans = NET_DVR_SetRealDataCallBack(m_Playhandle,g_RealDataCallBack_V30,(DWORD)this)))
//		g_pLog->WriteLog("海康 NET_DVR_SetRealDataCallBack 失败\n");
//	else
//	{
//		g_pLog->WriteLog("%s海康开始转发\t%s:%d,通道%d\n", m_ChannelInfo.ChannelName, m_ChannelInfo.ServerIP,m_pDVR->m_ServerPort, m_ChannelInfo.ChannelID);
//	}
//	return ret;
//}

void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, DWORD dwUser)
{	
	//在这里调用关闭realplay将导致未定义行为
	switch(dwDataType)
	{
	case NET_DVR_SYSHEAD:
	case NET_DVR_STREAMDATA:   //码流数据
		if (dwBufSize > 0 )
		{
			CHikNetDVRChannel* pThis = (CHikNetDVRChannel*)dwUser;
			// 把获取的数据发送出去
			if(pThis != NULL)
			{
				if(dwBufSize >0)
				{
					pThis->VedioFunc(pBuffer, dwBufSize);
				}
			}
			else
			{
				g_pLog->WriteLog("ERROR:channel 对象为NULL,海康\n");
				exit(0);
			}
		}
		break;
	default:
		break;
	}
}

static const map<int,int>::value_type init_value[]=
{
	map<int,int>::value_type(PTZ_UP,TILT_UP),
	map<int,int>::value_type(PTZ_DOWN,TILT_DOWN),
	map<int,int>::value_type(PTZ_LEFT,PAN_LEFT),
	map<int,int>::value_type(PTZ_RIGHT,PAN_RIGHT),

	map<int,int>::value_type(PTZ_ZOOM_ADD,ZOOM_IN),
	map<int,int>::value_type(PTZ_ZOOM_DEC,ZOOM_OUT),
	map<int,int>::value_type(PTZ_FOCUS_ADD,FOCUS_NEAR),
	map<int,int>::value_type(PTZ_FOCUS_DEC,FOCUS_FAR),
	map<int,int>::value_type(PTZ_APERTURE_ADD,IRIS_OPEN),
	map<int,int>::value_type(PTZ_APERTURE_DEC,IRIS_CLOSE),

	map<int,int>::value_type(PTZ_LEFTUP,UP_LEFT),
	map<int,int>::value_type(PTZ_RIGHTUP,UP_RIGHT),
	map<int,int>::value_type(PTZ_LEFTDOWN,DOWN_LEFT),
	map<int,int>::value_type(PTZ_RIGHTDOWN,DOWN_RIGHT),
};
const map<int,int> CHikNetDVRChannel::m_mapYTCmd(init_value,init_value+sizeof(init_value));
//CHikNetDVRChannel::m_mapYTCmd[PTZ_UP]=TILT_UP;
//CHikNetDVRChannel::m_mapYTCmd[PTZ_DOWN]=TILT_DOWN;
//m_mapYTCmd[PTZ_LEFT]=PAN_LEFT;
//m_mapYTCmd[PTZ_RIGHT]=PAN_RIGHT;
//m_mapYTCmd[PTZ_ZOOM_ADD]=ZOOM_IN;
//m_mapYTCmd[PTZ_ZOOM_DEC]=ZOOM_OUT;
//m_mapYTCmd[PTZ_FOCUS_ADD]=FOCUS_NEAR;
//m_mapYTCmd[PTZ_FOCUS_DEC]=FOCUS_FAR;
//m_mapYTCmd[PTZ_APERTURE_ADD]=IRIS_OPEN;
//m_mapYTCmd[PTZ_APERTURE_DEC]=IRIS_CLOSE;
//
////m_mapYTCmd[10]=GOTO_PRESET;
////m_mapYTCmd[11]=SET_PRESET;
//
//m_mapYTCmd[PTZ_LEFTUP]=UP_LEFT;
//m_mapYTCmd[PTZ_RIGHTUP]=UP_RIGHT;
//m_mapYTCmd[PTZ_LEFTDOWN]=DOWN_LEFT;
//m_mapYTCmd[PTZ_RIGHTDOWN]=DOWN_RIGHT;

CHikNetDVRChannel::CHikNetDVRChannel(LONG lLoginID, const int nChannelID, const int tStreamType)
	:CChannel(lLoginID, nChannelID, tStreamType)
{
	_Playhandle= -1;
}

CHikNetDVRChannel::~CHikNetDVRChannel()
{
	disconnect();
}

BOOL CHikNetDVRChannel::connect()
{
	BOOL ret = TRUE;
	if(_Playhandle == -1&&_Atry.have_a_try())
	{
		NET_DVR_PREVIEWINFO struPlayInfo = {0};
		struPlayInfo.hPlayWnd		= 0;		// 需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
		struPlayInfo.dwStreamType	= _StreamType;		// 0-主码流，1-子码流，2-码流3，3-码流4，以此类推
		struPlayInfo.dwLinkMode		= 0;		// 0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
		struPlayInfo.lChannel		= _ChannelID;		// 预览通道号

		_Playhandle = NET_DVR_RealPlay_V40(_LoginID, &struPlayInfo, NULL,this);
		if (_Playhandle == -1)
		{
			g_pLog->WriteLog("海康 NET_DVR_RealPlay_V40 失败,%s\n", NET_DVR_GetErrorMsg());
			ret = FALSE;
		}
		else if(!NET_DVR_SetRealDataCallBack(_Playhandle,g_RealDataCallBack_V30,(DWORD)this))
		{
			NET_DVR_StopRealPlay(_Playhandle);
			_Playhandle=-1;
			g_pLog->WriteLog("海康 NET_DVR_SetRealDataCallBack 失败,%s\n",NET_DVR_GetErrorMsg());
			ret = FALSE;
		}
		_Atry.finish();
	}
	if(ret)
		g_pLog->WriteLog("连接通道成功,海康,%ld/%ld\n", _LoginID, _Playhandle);
	return ret;
}

BOOL CHikNetDVRChannel::disconnect()
{
	if (_Playhandle != -1 && _Atry.have_a_try())
	{
		if (NET_DVR_SetRealDataCallBack(_Playhandle, NULL, NULL) == FALSE)// 清除回调函数
		{
			g_pLog->WriteLog("NET_DVR_SetRealDataCallBack(NULL) error:%s\n", NET_DVR_GetErrorMsg());
		}
		if(NET_DVR_StopRealPlay(_Playhandle) == FALSE)// 停止获取录像
		{
			g_pLog->WriteLog("NET_DVR_StopRealPlay error:%s\n", NET_DVR_GetErrorMsg());
		}
		_Playhandle = -1;
		_Atry.finish();
	}
	return TRUE;
}
