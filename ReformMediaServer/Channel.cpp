// Channel.cpp: implementation of the CChannel class.
//
//////////////////////////////////////////////////////////////////////
//#include "ReformMediaServerDef.h"
#include "StdHeader.h"
#include "Channel.h"
#include "Config.h"
#include "Log.h"
#include "DBObject.h"
#include "dvr.h"
#include "VideoTrans.h"
#include "MultiCastVideoTrans.h"
//#include <iostream>

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//volatile DWORD CChannel::m_RecordStatus=0;
//
//CChannel::CChannel()
//{
//	m_work_status = 0;
//
//	m_type_ID= "UNKNOWN";
//	m_type = "未初始化";
//	_fp = NULL;
//	m_pDVR = NULL;
//	
//	m_strRecordName = "";
//	m_strRecordPath = "";
//
//	for(int i = 0; i < MAX_CLIENT_SIZE; i++)
//	{
//		m_VideoTransArray[i] = NULL;
//	}
//
//	PTZControlIndex = -1;
//	m_bControl=FALSE;
//	m_YTStatus=0;
//}
//
//CChannel::~CChannel()
//{
//	//StopTrans();
//	for(int i = 0; i < MAX_CLIENT_SIZE; i++)
//	{
//		if(m_VideoTransArray[i] != NULL)
//		{
//			delete m_VideoTransArray[i];
//			m_VideoTransArray[i] = NULL;
//		}
//	}
//	if(_fp)
//		fclose(_fp);
//}
//
//BOOL CChannel::Init(int ChannelID, int index,int DVRChannelID,CDVR *pDVR)
//{
//	return TRUE;
//}
//
//BOOL CChannel::InitMultiCast()
//{
//	return TRUE;
//}
//
///*
//BOOL CChannel::StartPlay(int hwndindex,HWND hwnd)
//{
//return TRUE;
//}
//
//BOOL CChannel::StopPlay()
//{
//return TRUE;
//}
//
//BOOL CChannel::StartRecord(int type,int Intertime)
//{
//return TRUE;
//}
//
//BOOL CChannel::StopRecord(int type)
//{
//return TRUE;
//}
//*/
//
//BOOL CChannel::SetImageParam(IMAGEPARAM_INFO info)
//{
//	return TRUE;
//}
//
//BOOL CChannel::SendBuffer(char *pbuffer, DWORD length, BOOL bhead)
//{
//	if(this->m_ChannelInfo.isMulticast)
//	{
//		// 组播发送数据
//		m_MultiCastVedio->AddBuffer(pbuffer, length);
//	}
//	else
//	{
//		// 逐个客户端发送数据
//		for(int i = 0; i < MAX_CLIENT_SIZE; i++)
//		{
//			if(m_VideoTransArray[i] != NULL)
//			{
//				if(!m_VideoTransArray[i]->AddBuffer(pbuffer, length))
//				{
//					CloseTrans(i);
//				}
//			}
//		}
//	}
//
//	return TRUE;
//}
//
//BOOL CChannel::GetPic(char* sFileName)
//{
//	return TRUE;
//}
//BOOL CChannel::AddTrans(SOCKET sock)
//{
//	if(sock == INVALID_SOCKET)
//		return FALSE;
//
//	if(!m_bTrans)
//	{
//		StartTrans();
//	}
//
//	if(!m_bTrans)
//	{
//		return FALSE;
//	}	
//
//	for(int i = 0; i < MAX_CLIENT_SIZE; i++)
//	{
//		if(m_VideoTransArray[i] == NULL)
//		{
//			CVideoTrans* vt = new CVideoTrans(sock);
//			if(vt->m_bTrans == FALSE)
//			{
//				delete vt;
//				return FALSE;
//			}
//			memcpy(vt->ChannelName, m_ChannelInfo.ChannelName, 32);
//			m_VideoTransArray[i] = vt;
//			g_pLog->WriteLog("(%s)%s 开始往socket:(%d)发送数据\n", m_ChannelInfo.ChannelName, m_type.c_str(), sock);
//			m_SocketNum++;
//			return TRUE;
//		}
//	}
//
//	return FALSE;
//}
//BOOL CChannel::CloseTrans(int index)
//{
//	if(m_VideoTransArray[index] != NULL)
//	{
//		delete m_VideoTransArray[index];
//		m_VideoTransArray[index] = NULL;
//		m_SocketNum-- ;
//	}
//
//	// 没有客户端连接的时候，停止数据传输
//	if(m_SocketNum <= 0)
//	{
//		m_SocketNum = 0;
//		PostStopTrans();
//	}
//	return TRUE;
//}
//BOOL CChannel::PostStopTrans()
//{
//	return TRUE;
//}
//
//BOOL CChannel::PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop)
//{
//	return TRUE;
//}
//
//// 启动录像
////BOOL CChannel::StartRecord(BOOL bRecord, char * pPath)
////{
////	return TRUE;
////}
//
//void CChannel::save_record(char *pbuffer, DWORD length)
//{
//	SYSTEMTIME loctime;
//	GetLocalTime(&loctime);
//	char szTime[128]={0};
//	sprintf_s(szTime, "%.4d%.2d%.2d%.2d%.2d%.2d", loctime.wYear, loctime.wMonth, loctime.wDay, loctime.wHour, loctime.wMinute, loctime.wSecond);
//
//	bool is_open_file = false;
//	if(_fp == NULL)
//	{
//		char szPath[MAX_PATH] = {0};
//		char szFilePath[MAX_PATH] = {0};
//		char szFileName[MAX_PATH] = {0};
//		if(get_record_file_path(szPath, szFilePath, szFileName))
//		{
//			m_strRecordPath = szFilePath;
//			m_strRecordName = szFileName;
//			m_tLastSaveTime = time(NULL);
//			m_strLastSaveTime = szTime;
//			_fp = fopen(szPath, "wb");
//			if(_fp != NULL)
//			{
//				is_open_file = true;
//				init_icon(szPath);
//				g_pLog->WriteLog("开始录制文件,%s\n", szPath);
//			}
//			else
//			{
//				g_pLog->WriteLog("打开文件失败(%s),%s\n", strerror(errno), m_strRecordName.c_str());
//			}
//		}
//	}
//	else
//	{
//		is_open_file = true;
//		// 判断是否需要分文件
//		double dt = difftime(time(NULL), m_tLastSaveTime);
//		if(dt > RECORD_INTERVAL )
//		{
//			is_open_file = false;
//			fclose(_fp);
//			m_recordId = g_dbobj.InsertReocord(m_type.c_str(), m_strDVRName.c_str(), m_strCameraId.c_str() ,
//				m_DVRChannelID ,m_pDVR->m_strServIp, 0, 1, m_strRecordPath.c_str(), m_strRecordName.c_str(), m_strLastSaveTime.c_str(), szTime);
//			g_pLog->WriteLog("录制文件完成,%s\n", m_strRecordName.c_str());
//
//			char szPath[MAX_PATH] = {0};
//			char szFilePath[MAX_PATH] = {0};
//			char szFileName[MAX_PATH] = {0};
//			if(get_record_file_path(szPath, szFilePath, szFileName))
//			{
//				m_strRecordPath = szFilePath;
//				m_strRecordName = szFileName;
//				m_tLastSaveTime = time(NULL);
//				m_strLastSaveTime = szTime;
//				_fp = fopen(szPath, "wb");
//				if(_fp != NULL)
//				{
//					is_open_file = true;
//					init_icon(szPath);
//					g_pLog->WriteLog("开始录制文件,%s\n", szPath);
//				}
//				else
//				{
//					g_pLog->WriteLog("打开文件失败(%s),%s\n", strerror(errno), m_strRecordName.c_str());
//				}
//			}
//			else
//			{
//				g_pLog->WriteLog( "ERROR:获取录像存储路径失败\n");
//			}
//		}
//	}
//
//	if(is_open_file)
//	{
//		fwrite(pbuffer, 1, length, _fp);
//		save_icon(pbuffer, length);
//	}
//
//}
//
//bool CChannel::get_record_file_path(char* path, char* file_path, char* file_name)
//{
//	bool ret = false;
//	char szPath[MAX_PATH] = {0};
//	bool bgetposition = g_xmlConfig.get_record_path(m_strCameraId , szPath );
//	bool bwhile = !bgetposition;
//	while(bwhile)
//	{
//		int err = freeDiskspace();
//		switch(err)
//		{
//		case 0:
//			//continue
//			Sleep(500);
//			break;
//		case -1:
//			{
//				bwhile = false;
//				g_pLog->WriteLog("ERROR: 获取录像存储位置失败\n");
//			}
//			break;
//		case 1:
//			{
//				bgetposition = g_xmlConfig.get_record_path(m_strCameraId , szPath );
//				bwhile = !bgetposition;
//			}
//			break;
//		default:
//			bwhile = false;
//			g_pLog->WriteLog("ERROR: 获取录像存储位置失败，未处理的返回值%d\n", err);
//			break;
//		}
//	}
//
//	if(bgetposition)
//	{
//		char szName[128] = {0};
//		SYSTEMTIME loctime;
//		GetLocalTime(&loctime);
//		char szTime[18]={0};
//		sprintf_s(szTime, "%.4d%.2d%.2d%.2d%.2d%.2d%.3d", loctime.wYear, loctime.wMonth, loctime.wDay, loctime.wHour, loctime.wMinute, loctime.wSecond, loctime.wMilliseconds );
//		sprintf_s( szName, "%s_%s_%d_%s.rd",  m_type_ID.c_str(), m_strDVRName.c_str(), m_DVRChannelID, szTime);
//		sprintf_s(path, MAX_PATH, "%s/%s", szPath, szName);
//		sprintf_s(file_path, MAX_PATH, "%s", szPath);
//		sprintf_s(file_name, MAX_PATH, "%s", szName);
//		ret = true;
//	}
//	return ret;
//}
//
//void CChannel::init_icon(char* name)
//{
//	//is_icon_init_ = false;
//	is_ready_icon_ = false;
//	memset(icon_path, 0, MAX_PATH);
//	sprintf_s(icon_path, "%s.%s", name, ICON_EXTENSION);
//}
//
//BOOL CChannel::stop_by_Socket(SOCKET s)
//{
//	//for(int i = 0; i < MAX_CLIENT_SIZE; i++)
//	//{
//	//	if(m_VideoTransArray[i] != NULL && m_VideoTransArray[i]->m_Socket == s)
//	//	{
//	//		m_VideoTransArray[i]->m_Socket = INVALID_SOCKET;
//	//		m_VideoTransArray[i]->End();
//	//		return TRUE;
//	//	}
//	//}
//	return FALSE;
//}
//
//int CChannel::freeDiskspace()
//{
//	int ret = -1;
//	if(InterlockedCompareExchange(&m_RecordStatus, 1, 0) == 0)
//	{
//		//MapQueryRecordResult mapRecord;
//		//g_dbobj.QueryRecord(mapRecord, "", "", "");
//		//int count = mapRecord.count/4;
//
//		char file_name[MAX_PATH]={0};
//		int qid = g_dbobj.getFirstFile(file_name);
//		if(qid >= 0)
//		{
//			g_dbobj.DeleteRecord(qid);
//			int err = remove(file_name);
//			if(err == 0)
//			{
//				ret = 1;
//				g_pLog->WriteLog("删除录像文件%s\n", file_name);
//			}
//			else
//			{
//				ret = 0;
//				if(errno == EACCES)
//				{
//					g_pLog->WriteLog("ERROR: 删除录像文件失败 %s 文件为只读或正在使用\n", file_name);
//				}
//				else
//				{
//					//ENOENT 
//					g_pLog->WriteLog("WARN: 删除录像文件失败 %s 文件不存在, errno:%d\n", file_name, errno); 
//				}
//			}
//		}
//		else
//		{
//			g_pLog->WriteLog("WARN:无法释放硬盘空间，数据库为空\n");
//		}
//		m_RecordStatus=0;
//	}
//	else
//		ret = 0;
//	return ret;
//}

//const int CChannel::FailTimes = 50;

CChannel::CChannel(LONG lLoginID, const int nChannelID, const int tStreamType)
	:_LoginID(lLoginID), _ChannelID(nChannelID), _StreamType(tStreamType), _lock(), m_ptzcount(0), m_bControl(FALSE)
{
}

CChannel::~CChannel()
{
	//disconnect();
	for(map<DWORD, CBufferCmd*>::iterator iter=_CmdList.begin();iter!=_CmdList.end();)
	{
		delete iter->second;
		iter = _CmdList.erase(iter);
	}
}

BOOL CChannel::addCmd(CBufferCmd* cmd)
{
	m_lastcmdtime = time(NULL);

	BOOL ret = FALSE;
	if(_lock.lock())
	{
		_CmdList[cmd->CmdID]=cmd;
		_lock.unlock();
		ret = TRUE;
	}
	return ret;
}

BOOL CChannel::delCmd(CBufferCmd* cmd)
{
	BOOL ret = FALSE;
	if(_lock.lock())
	{
		map<DWORD, CBufferCmd*>::iterator iter = _CmdList.find(cmd->CmdID);
		if(iter!= _CmdList.end())
			_CmdList.erase(iter);
		_lock.unlock();
		ret = TRUE;
	}
	return ret;
}

void CChannel::VedioFunc(const BYTE *pBuffer, const DWORD dwBufSize)
{
	m_lastcmdtime = time(NULL);
	if(_lock.lock())
	{
		for(map<DWORD, CBufferCmd*>::iterator iter=_CmdList.begin();iter!=_CmdList.end();)
		{
			BOOL ret=iter->second->handleFunc(pBuffer, dwBufSize);
			if(ret == FALSE)
			{
				delete iter->second;
				iter = _CmdList.erase(iter);
			}
			else
				iter++;
		}
		_lock.unlock();
	}
}

BOOL	CChannel::PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop)
{
	BOOL ret = FALSE;
	if(m_ptzcount > m_ptznum)
	{
		if(difftime(time(NULL), m_ptzbegintime) < m_ptzdiff)
		{
			//在30秒内执行命令次数超过15，则不执行
			g_pLog->WriteLog("云台命令频繁\n");
			return ret;
		}
		else
			m_ptzcount = 0;
	}

	ret = _PTZControl(dwPTZCommand, param1, param2, param3, dwStop);
	if(ret == TRUE)
	{
		if(m_ptzcount == 0)
			m_ptzbegintime = time(NULL);
		m_ptzcount++;
	}
	return ret;
}

BOOL CChannel::idle()
{
	BOOL ret = FALSE;
	if(_lock.lock())
	{
		if(_CmdList.empty())
			ret = TRUE;
		_lock.unlock();
	}
	return ret;
}

BOOL CChannel::cmdTimeOut()
{
	double diff = difftime(time(NULL), m_lastcmdtime);
	if(diff > 60*60)
		return FALSE;
	return TRUE;
}
