#include "DaHuaDVR.h"
#include "StdHeader.h"
#include "dhnetsdk.h"
#include "Log.h"
#include "Config.h"

const char* const CDaHuaDVR::m_loginerr[10]={"密码不正确","用户名不存在","登录超时","帐号已登录","帐号已被锁定 ","帐号被列为黑名单","资源不足，系统忙","子连接失败","主连接失败","超过最大用户连接数"};

CDaHuaDVR::CDaHuaDVR(const char * sDVRIP,const WORD wDVRPort, const char * sUserName, const char* sPassword)
	:CDVR(sDVRIP, wDVRPort, sUserName, sPassword)
{
	m_InvalidID = 0;
	m_lLoginID = m_InvalidID;
}

CDaHuaDVR::~CDaHuaDVR(void)
{
	clearChannels();
	disconnect();
}

BOOL CDaHuaDVR::connect()
{
	BOOL ret = TRUE;
	if(m_lLoginID==0)
	{
		char ip[16];
		strcpy_s(ip,m_DVRIP.c_str());
		int err = 0;
		NET_DEVICEINFO info;// 用于返回设备信息
		memset(&info, 0 ,sizeof(NET_DEVICEINFO));
		m_lLoginID = CLIENT_Login(ip, m_DVRPort, m_UserName, m_Password, &info, &err);	// 登录并返回设备信息
		if(m_lLoginID == 0)
		{
			ret = FALSE;
			//log error
			if(err<=10 && err>=1)
				g_pLog->WriteLog("调用大华SDK登入失败:%s, %s:%d\n",m_loginerr[err-1],ip,m_DVRPort);
			else
				g_pLog->WriteLog("调用大华SDK登入失败:错误类型不识别code=%d, %s:%d\n",err,ip,m_DVRPort);
		}
		else
		{
			//CLIENT_GetDevConfig
			m_CnlNum = info.byChanNum;
			m_StartCnl = 0;	

			g_pLog->WriteLog("DVR大华登入成功,loginID:%ld,%s:%d\n", m_lLoginID, ip,m_DVRPort);
		}
	}
	return ret;
}

BOOL CDaHuaDVR::disconnect()
{
	BOOL ret = TRUE;
	if(m_lLoginID > 0)
	{
		clearChannels();
		ret = CLIENT_Logout(m_lLoginID);
		m_lLoginID=0;
		g_pLog->WriteLog("调用大华SDK登出,%s:%d\n",m_DVRIP.c_str(), m_DVRPort);
		if(ret == FALSE)
		{
			g_pLog->WriteLog("调用大华SDK登出失败,code=%d\n",CLIENT_GetLastError());
		}
	}
	return ret;
}
