#pragma once
#include "StdHeader.h"

// 用于获取和写入FileServer.ini配置文件
class CSysInfo
{
public:
	CSysInfo(void);
public:
	~CSysInfo(void);

	int m_FilePort;
	int m_RtspPort;
	int m_ListenDvrPort;
	int m_bLog;
	int m_PushPort;

	char m_XMLFilePath[MAX_PATH];

	BOOL WriteLoacl();//将系统参数信息写入文件中保存
	BOOL ReadLoacl();//从文件读取系统参数信息

	BOOL GetDeviceObjects();
};
