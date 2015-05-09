#include "ReformMediaServerDef.h"
#include "StdHeader.h"
#include "DHDVRChannel.h"
#include "dhnetsdk.h"
#include "Log.h"
#include "managerDvr.h"
#include "dvr.h"
#include "DaHuaDVR.h"
#include "VideoTrans.h"
#include "MultiCastVideoTrans.h"
#include "Config.h"
#include "DBObject.h"
#include "dhplay.h"
#include <iostream>
//#include "ManagerRtsp.h"

//void CALLBACK RealDataCallBackDH(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LONG param, DWORD dwUser)
//{
//	CDHDVRChannel * pThis = (CDHDVRChannel *)dwUser;
//	if( pThis != NULL && dwBufSize > 0)
//	{
//		pThis->SendBuffer((char *)pBuffer, dwBufSize, FALSE);
//		pThis->save_record((char *)pBuffer, dwBufSize);
//	}
//}
//
//CDHDVRChannel::CDHDVRChannel()
//{
//	is_ready_icon_ = false;
//	is_icon_init_ = false;
//
//	m_type_ID = "DH";
//	m_type = "大华";
//	m_pDVR = NULL;
//	m_Playhandle = -1;
//
//	m_bTrans = FALSE;
//	m_SocketNum = 0;	
//
//	icon_port = -1;
//	//init
//	is_icon_init_ = false;
//	if(!PLAY_GetFreePort(&icon_port))
//	{
//		g_pLog->WriteLog("初始化大华控件失败,-1\n");
//	}
//	else if(!PLAY_SetStreamOpenMode(icon_port, STREAME_REALTIME))
//	{
//		g_pLog->WriteLog("初始化大华控件失败,0\n");
//	}
//	else if(!PLAY_OpenStream(icon_port, NULL, 0, 1024 * 1024))
//	{
//		g_pLog->WriteLog("初始化大华控件失败,1\n");
//	}
//	else if(!PLAY_Play(icon_port, 0))
//	{
//		g_pLog->WriteLog("初始化大华控件失败,2\n");
//	}
//	else
//		is_icon_init_ = true;
//
//	m_mapYTCmd[PTZ_UP]=DH_PTZ_UP_CONTROL;
//	m_mapYTCmd[PTZ_DOWN]=DH_PTZ_DOWN_CONTROL;
//	m_mapYTCmd[PTZ_LEFT]=DH_PTZ_LEFT_CONTROL;
//	m_mapYTCmd[PTZ_RIGHT]=DH_PTZ_RIGHT_CONTROL;
//	m_mapYTCmd[PTZ_ZOOM_ADD]=DH_PTZ_ZOOM_ADD_CONTROL;
//	m_mapYTCmd[PTZ_ZOOM_DEC]=DH_PTZ_ZOOM_DEC_CONTROL;
//	m_mapYTCmd[PTZ_FOCUS_ADD]=DH_PTZ_FOCUS_ADD_CONTROL;
//	m_mapYTCmd[PTZ_FOCUS_DEC]=DH_PTZ_FOCUS_DEC_CONTROL;
//	m_mapYTCmd[PTZ_APERTURE_ADD]=DH_PTZ_APERTURE_ADD_CONTROL;
//	m_mapYTCmd[PTZ_APERTURE_DEC]=DH_PTZ_APERTURE_DEC_CONTROL;
//
//	//m_mapYTCmd[10]=DH_PTZ_POINT_MOVE_CONTROL;
//	//m_mapYTCmd[11]=DH_PTZ_POINT_SET_CONTROL;
//	
//	m_mapYTCmd[PTZ_LEFTUP]=DH_EXTPTZ_LEFTTOP;
//	m_mapYTCmd[PTZ_RIGHTUP]=DH_EXTPTZ_RIGHTTOP;
//	m_mapYTCmd[PTZ_LEFTDOWN]=DH_EXTPTZ_LEFTDOWN;
//	m_mapYTCmd[PTZ_RIGHTDOWN]=DH_EXTPTZ_RIGHTDOWN;
//}
//
//CDHDVRChannel::~CDHDVRChannel()
//{
//	StopTrans();
//	if(icon_port > 0)
//	{
//		PLAY_ReleasePort(icon_port);
//		PLAY_Stop(icon_port);
//		PLAY_CloseStream(icon_port);
//		icon_port = -1;
//	}
//}
//
//BOOL CDHDVRChannel::InitMultiCast()
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
//// 开启视频请求
//BOOL CDHDVRChannel::StartTrans()
//{
//	//当IPC设置的码流很低时，采用主码流方式获取不到数据
//	m_Playhandle = CLIENT_RealPlay(((CDaHuaDVR *)m_pDVR)->m_UserID , m_ChannelInfo.ChannelID, 0);
//	//子码流
//	//m_Playhandle = CLIENT_RealPlayEx(((CDaHuaDVR *)m_pDVR)->m_UserID , m_ChannelInfo.ChannelID, 0, DH_RType_Realplay_1);
//	if(m_Playhandle == 0)
//	{
//		g_pLog->WriteLog("大华 CLIENT_RealPlay 失败\n");
//		return FALSE;
//	}
//
//	//m_bTrans = CLIENT_SetRealDataCallBackEx(m_Playhandle,RealDataCallBackDH, (DWORD)m_index,0x00000001);
//
//	//设置回调函数
//	m_bTrans = CLIENT_SetRealDataCallBackEx(m_Playhandle, RealDataCallBackDH, (DWORD)this/*用户数据*/, 0x00000001/*等同原来的原始数据*/);
//
//	if(!m_bTrans)
//		g_pLog->WriteLog("大华 CLIENT_SetRealDataCallBackEx 失败\n");
//	else
//	{
//		g_pLog->WriteLog("%s大华开始转发\t%s:%d,通道:%d\n", m_ChannelInfo.ChannelName, m_ChannelInfo.ServerIP,m_pDVR->m_ServerPort, m_ChannelInfo.ChannelID);		
//	}
//
//	return m_bTrans;
//}
//
//BOOL CDHDVRChannel::PostStopTrans()
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
//BOOL CDHDVRChannel::StopTrans()
//{
//	//CLIENT_SetRealDataCallBackEx(m_Playhandle,NULL, (DWORD)m_index,0x00000001);
//	// 清除回调函数
//	CLIENT_SetRealDataCallBackEx(m_Playhandle, NULL, (DWORD)m_index, 0x00000001);
//
//	m_bTrans = FALSE;
//
//	// 停止获取录像
//	CLIENT_StopRealPlay(m_Playhandle);
//
//	return TRUE;
//}
//
//BOOL CDHDVRChannel::SetImageParam(IMAGEPARAM_INFO info)
//{
//	return TRUE;
//}
//
//BOOL CDHDVRChannel::GetPic(char* sFileName)
//{
//	int nType=1;
//	BOOL bRet= FALSE;//PLAY_CatchPicEx((LONG)m_DVRChannelID,sFileName,(tPicFormats)nType);
//	return bRet;
//}
//
// 云台控制
BOOL CDHDVRChannel::_PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop)
{
	//DHDEV_COMM_CFG cf={0};
	//DWORD rsize=0;
	//BOOL tret = CLIENT_GetDevConfig(_LoginID, DH_DEV_COMMCFG, _ChannelID, &cf, sizeof(DHDEV_COMM_CFG), rsize);
	//DWORD terr;
	//if(!tret)
	//{
	//	terr = CLIENT_GetLastError();
	//}
	//else
	//{
	//	CLIENT_GetPtzOptAttr(_LoginID, cf.DecProName,);
	//}

	if(_Playhandle < 0)
		return FALSE;

	if(m_bControl)
	{
		CLIENT_DHPTZControlEx(_LoginID, _ChannelID, m_lastCmd, param1, param2, param3, TRUE);
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
			bRet=CLIENT_DHPTZControlEx(_LoginID, _ChannelID, DH_PTZ_POINT_MOVE_CONTROL, 0, 120, 0, FALSE);
		}
		break;
	case PTZ_POINT_SET:
		{
			bRet=CLIENT_DHPTZControlEx(_LoginID, _ChannelID, DH_PTZ_POINT_SET_CONTROL, 0, 120, 0, FALSE);
		}
		break;
	case PTZ_WIPER_OPEN:
		{
			bRet=CLIENT_DHPTZControlEx(_LoginID, _ChannelID, DH_PTZ_LAMP_CONTROL, 1, 0, 0, FALSE);
		}
		break;
	case PTZ_WIPER_CLOSE:
		{
			bRet=CLIENT_DHPTZControlEx(_LoginID, _ChannelID, DH_PTZ_LAMP_CONTROL, 0, 0, 0, FALSE);
		}
		break;
	case PTZ_THROUGHFOG_OPEN:
		{
		}
		break;
	case PTZ_THROUGHFOG_CLOSE:
		{
		}
		break;
	default:
		{
			map<int, int>::const_iterator iter = m_mapYTCmd.find(dwPTZCommand);
			if(iter != m_mapYTCmd.end())
			{
				int CMD = iter->second;
				bRet=CLIENT_DHPTZControlEx(_LoginID, _ChannelID, dwPTZCommand, param1, param2, param3, FALSE);
				if(bRet)
				{
					m_bControl=TRUE;
					m_lastCmd = CMD;
				}
			}
			else
			{
				g_pLog->WriteLog("不识别的大华云台控制命令\n");
			}
		}
		break;
	}

	if(!bRet)
	{
		DWORD err = CLIENT_GetLastError();
		g_pLog->WriteLog("执行大华云台控制命令失败:%d！\n",err);
		return FALSE;
	}
	else
	{
		g_pLog->WriteLog("大华云台控制id=%d, %u;;%u;;%u;;%u;;%u\n", _LoginID, dwPTZCommand, param1, param2, param3, dwStop);
	}
	return TRUE;
}
//
//void CDHDVRChannel::save_icon(char *pbuffer, DWORD length)
//{
//	if(!is_icon_init_)
//		return;
//	if(!is_ready_icon_)
//	{
//		if(!PLAY_InputData(icon_port, (unsigned char*)pbuffer, length))
//		{
//			g_pLog->WriteLog("WARN:抓图数据输入失败,%s,3\n", m_type.c_str());
//			//return;
//		}
//
//		tPicFormats f;
//		switch(ICON_FORMAT)
//		{
//		case 0:
//			f = PicFormat_BMP;
//			break;
//		case 1:
//			f = PicFormat_JPEG;
//			break;
//		case 2:
//			f = PicFormat_JPEG_70;
//			break;
//		case 3:
//			f = PicFormat_JPEG_50;
//			break;
//		case 4:
//			f = PicFormat_JPEG_30;
//			break;
//		case 5:
//			f = PicFormat_JPEG_10;
//			break;
//		default:
//			f = PicFormat_BMP;
//		}
//
//		if(PLAY_CatchResizePic(icon_port, icon_path, ICON_WIDTH, ICON_HEIGHT, f))
//		{
//			PLAY_ResetSourceBuffer(icon_port);
//			//PLAY_Stop(icon_port);
//			//PLAY_CloseStream(icon_port);
//			//PLAY_ReleasePort(icon_port);
//			//icon_port = -1;
//
//			is_ready_icon_ = true;
//			g_pLog->WriteLog("抓图成功,%s, %s\n", m_type.c_str(), icon_path);
//		}
//		else
//		{
//			g_pLog->WriteLog("正在抓图,%s, %s ,w:%d,h:%d, id:%d\n", m_type.c_str(), icon_path, ICON_WIDTH, ICON_HEIGHT, icon_port);
//		}
//	}
//}
//
//BOOL CDHDVRChannel::Init(LONG lLoginID, int nChannelID)
//{
//	BOOL ret=TRUE;
//	//当IPC设置的码流很低时，采用主码流方式获取不到数据
//	//子码流
//	//m_Playhandle = CLIENT_RealPlayEx(((CDaHuaDVR *)m_pDVR)->m_UserID , m_ChannelInfo.ChannelID, 0, DH_RType_Realplay_1);
//	if((m_Playhandle = CLIENT_RealPlay(lLoginID , nChannelID, 0)) == 0)
//	{
//		DWORD err = CLIENT_GetLastError();
//		g_pLog->WriteLog("大华 CLIENT_RealPlay 失败%lu\n",err);
//		ret = FALSE;
//	}
//	else if(!(m_bTrans = CLIENT_SetRealDataCallBackEx(m_Playhandle, RealDataCallBackDH, (DWORD)this, 0x00000001)))
//	{
//		DWORD err = CLIENT_GetLastError();
//		g_pLog->WriteLog("大华 CLIENT_SetRealDataCallBackEx 失败%lu\n", err);
//		ret = FALSE;
//	}
//
//	return ret;
//}

void CALLBACK RealDataCallBackDH(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LONG param, DWORD dwUser)
{
	CDHDVRChannel * pThis = (CDHDVRChannel *)dwUser;
	if( pThis != NULL && dwBufSize > 0)
	{
		pThis->VedioFunc(pBuffer, dwBufSize);
	}
}

//	m_mapYTCmd[PTZ_UP]=DH_PTZ_UP_CONTROL;
//	m_mapYTCmd[PTZ_DOWN]=DH_PTZ_DOWN_CONTROL;
//	m_mapYTCmd[PTZ_LEFT]=DH_PTZ_LEFT_CONTROL;
//	m_mapYTCmd[PTZ_RIGHT]=DH_PTZ_RIGHT_CONTROL;

//	m_mapYTCmd[PTZ_ZOOM_ADD]=DH_PTZ_ZOOM_ADD_CONTROL;
//	m_mapYTCmd[PTZ_ZOOM_DEC]=DH_PTZ_ZOOM_DEC_CONTROL;
//	m_mapYTCmd[PTZ_FOCUS_ADD]=DH_PTZ_FOCUS_ADD_CONTROL;
//	m_mapYTCmd[PTZ_FOCUS_DEC]=DH_PTZ_FOCUS_DEC_CONTROL;
//	m_mapYTCmd[PTZ_APERTURE_ADD]=DH_PTZ_APERTURE_ADD_CONTROL;
//	m_mapYTCmd[PTZ_APERTURE_DEC]=DH_PTZ_APERTURE_DEC_CONTROL;
//
//	//m_mapYTCmd[10]=DH_PTZ_POINT_MOVE_CONTROL;
//	//m_mapYTCmd[11]=DH_PTZ_POINT_SET_CONTROL;
//	
//	m_mapYTCmd[PTZ_LEFTUP]=DH_EXTPTZ_LEFTTOP;
//	m_mapYTCmd[PTZ_RIGHTUP]=DH_EXTPTZ_RIGHTTOP;
//	m_mapYTCmd[PTZ_LEFTDOWN]=DH_EXTPTZ_LEFTDOWN;
//	m_mapYTCmd[PTZ_RIGHTDOWN]=DH_EXTPTZ_RIGHTDOWN;
static const map<int,int>::value_type init_value[]=
{
	map<int,int>::value_type(PTZ_UP, DH_PTZ_UP_CONTROL),
	map<int,int>::value_type(PTZ_DOWN, DH_PTZ_DOWN_CONTROL),
	map<int,int>::value_type(PTZ_LEFT,DH_PTZ_LEFT_CONTROL),
	map<int,int>::value_type(PTZ_RIGHT,DH_PTZ_RIGHT_CONTROL),

	map<int,int>::value_type(PTZ_ZOOM_ADD, DH_PTZ_ZOOM_ADD_CONTROL),
	map<int,int>::value_type(PTZ_ZOOM_DEC, DH_PTZ_ZOOM_DEC_CONTROL),
	map<int,int>::value_type(PTZ_FOCUS_ADD, DH_PTZ_FOCUS_ADD_CONTROL),
	map<int,int>::value_type(PTZ_FOCUS_DEC, DH_PTZ_FOCUS_DEC_CONTROL),
	map<int,int>::value_type(PTZ_APERTURE_ADD, DH_PTZ_APERTURE_ADD_CONTROL),
	map<int,int>::value_type(PTZ_APERTURE_DEC, DH_PTZ_APERTURE_DEC_CONTROL),

	map<int,int>::value_type(PTZ_LEFTUP, DH_EXTPTZ_LEFTTOP),
	map<int,int>::value_type(PTZ_RIGHTUP, DH_EXTPTZ_RIGHTTOP),
	map<int,int>::value_type(PTZ_LEFTDOWN, DH_EXTPTZ_LEFTDOWN),
	map<int,int>::value_type(PTZ_RIGHTDOWN, DH_EXTPTZ_RIGHTDOWN),
};
const map<int,int> CDHDVRChannel::m_mapYTCmd(init_value,init_value+sizeof(init_value));

CDHDVRChannel::CDHDVRChannel(LONG lLoginID, const int nChannelID, const int tStreamType)
	:CChannel(lLoginID, nChannelID, tStreamType)
{
	_Playhandle = 0;
}

CDHDVRChannel::~CDHDVRChannel()
{
	disconnect();
}

BOOL CDHDVRChannel::connect()
{
	//当IPC设置的码流很低时，采用主码流方式获取不到数据
	//_Playhandle = CLIENT_RealPlay(_LoginID , _ChannelID, 0);
	//子码流
	BOOL ret = TRUE;
	if(_Playhandle == 0&&_Atry.have_a_try())
	{
		_Playhandle = CLIENT_RealPlayEx(_LoginID , _ChannelID, 0, (DH_RealPlayType)_StreamType/*DH_RType_Realplay_1*/);
		if(_Playhandle == 0)
		{
			ret = FALSE;
			g_pLog->WriteLog("大华 CLIENT_RealPlay 失败,%lu\n",CLIENT_GetLastError());	
		}
		else if(CLIENT_SetRealDataCallBackEx(_Playhandle, RealDataCallBackDH, (DWORD)this/*用户数据*/, 0x00000001/*等同原来的原始数据*/) == FALSE)
		{
			ret = FALSE;
			g_pLog->WriteLog("大华 CLIENT_SetRealDataCallBackEx 失败%lu\n",CLIENT_GetLastError());
		}	
		_Atry.finish();
	}
	if(ret)
		g_pLog->WriteLog("连接通道成功,大华,%ld/%ld\n", _LoginID, _Playhandle);
	return ret;
}

BOOL CDHDVRChannel::disconnect()
{
	if(_Playhandle != 0 && _Atry.have_a_try())
	{
		if(CLIENT_SetRealDataCallBackEx(_Playhandle, NULL, NULL, 0x00000001) == FALSE)// 清除回调函数
		{
			g_pLog->WriteLog("大华 CLIENT_SetRealDataCallBackEx(NULL)失败%lu\n",CLIENT_GetLastError());
		}
		else if(CLIENT_StopRealPlay(_Playhandle) == FALSE)// 停止获取录像
		{
			g_pLog->WriteLog("大华CLIENT_StopRealPlay失败%lu\n",CLIENT_GetLastError());
		}
		_Playhandle =0;
		_Atry.finish();
	}
	return TRUE;
}

