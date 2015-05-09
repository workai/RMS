#include "CBufferCmd.h"
#include "DBObject.h"
#include "Log.h"
#include "Config.h"

DWORD CBufferCmd::num = 0;

CBufferCmd::CBufferCmd()
	:CmdID(num++)
{
}

CRecordCmd::CRecordCmd(const CAMERA_INFO &info)
	:m_type(info.strProvider),
	m_strDVRName(info.strName),
	m_strCameraId(info.strCameraID),
	m_DVRChannelID(atoi(info.strCnlID.c_str())),
	m_strServIp(info.strIP),
	_fp(NULL)
{
}

BOOL CRecordCmd::handleFunc(const BYTE *const pBuffer, const DWORD dwBufSize)
{
	SYSTEMTIME loctime;
	GetLocalTime(&loctime);
	char szTime[128]={0};
	sprintf_s(szTime, "%.4d%.2d%.2d%.2d%.2d%.2d", loctime.wYear, loctime.wMonth, loctime.wDay, loctime.wHour, loctime.wMinute, loctime.wSecond);

	if(_fp != NULL)
	{
		union {
			FILETIME ft;
			LONGLONG val;
		}now,last;

		SystemTimeToFileTime(&loctime, &now.ft);
		SystemTimeToFileTime(&m_tLastSaveTime, &last.ft);
		LONGLONG sec = (now.val - last.val)/10000000;
		if( sec > RECORD_INTERVAL)
		{
			fclose(_fp);
			_fp=NULL;
			char szLastTime[128]={0};
			sprintf_s(szLastTime, "%.4d%.2d%.2d%.2d%.2d%.2d", m_tLastSaveTime.wYear, m_tLastSaveTime.wMonth, m_tLastSaveTime.wDay, m_tLastSaveTime.wHour, m_tLastSaveTime.wMinute, m_tLastSaveTime.wSecond);
			g_dbobj.InsertReocord(m_type.c_str(), m_strDVRName.c_str(), m_strCameraId.c_str() ,
				m_DVRChannelID ,m_strServIp.c_str(), 0, 1, m_strRecordPath.c_str(), m_strRecordName.c_str(), szLastTime, szTime);
			g_pLog->WriteLog("录制文件完成,%s\n", m_strRecordName.c_str());
		}
	}

	if(_fp == NULL)
	{
		char szPath[MAX_PATH] = {0};
		char szFilePath[MAX_PATH] = {0};
		char szFileName[MAX_PATH] = {0};
		if(g_xmlConfig.get_record_path(m_strCameraId , szFilePath))
		{
			if(strstr(m_type.c_str(),"大华")!=NULL)
			{
				sprintf_s( szFileName, "DH_%s_%s_%d_%s.rd",  m_strDVRName.c_str(), m_strCameraId.c_str(), m_DVRChannelID, szTime);
			}
			else if(strstr(m_type.c_str(),"海康")!=NULL)
			{
				sprintf_s( szFileName, "HK_%s_%s_%d_%s.rd",  m_strDVRName.c_str(), m_strCameraId.c_str(), m_DVRChannelID, szTime);
			}
			else
			{
				g_pLog->WriteLog( "ERROR:不支持设备类型\n");
				return FALSE;
			}
			sprintf_s(szPath, MAX_PATH, "%s/%s", szFilePath, szFileName);

			m_strRecordPath = szFilePath;
			m_strRecordName = szFileName;
			m_tLastSaveTime = loctime;
			_fp = fopen(szPath, "wb");
			if(_fp != NULL)
			{			
				g_pLog->WriteLog("开始录制文件,%s\n", szPath);
			}
			else
			{
				g_pLog->WriteLog("打开文件失败(%s),%s\n", strerror(errno), m_strRecordName.c_str());
			}
		}
		else
		{
			g_pLog->WriteLog( "ERROR:获取录像存储路径失败\n");
		}
	}

	if(_fp!= NULL)
	{
		fwrite(pBuffer, 1, dwBufSize, _fp);
		return TRUE;
	}
	return FALSE;
}

CSendCmd::CSendCmd(SOCKET s)
	:_socket(s), _FailNum(0)
{
}

BOOL CSendCmd::handleFunc(const BYTE *const pBuffer, const DWORD dwBufSize)
{
	BOOL ret = TRUE;
	int re = send(_socket, (char*)pBuffer, dwBufSize, 0);
	if(re == SOCKET_ERROR)
	{
		int err = GetLastError();
		if(err != WSAEWOULDBLOCK)
		{
			ret = FALSE;
			g_pLog->WriteLog("发送视频数据失败,SOCKET:%d,ERROR:%d\n", _socket,err);
		}
		else if(++_FailNum > FailTimes)
		{
			ret = FALSE;
		}
	}
	else if(_FailNum >0)
	{
		--_FailNum;
	}
	return ret;
}