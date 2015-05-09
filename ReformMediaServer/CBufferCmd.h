#ifndef CBUFFERCMD_H
#define CBUFFERCMD_H

#include "ReformMediaServerDef.h"

class CBufferCmd
{
private:
	static DWORD num;
public:
	DWORD CmdID;
	CBufferCmd();
	virtual BOOL handleFunc(const BYTE *const pBuffer, const DWORD dwBufSize)=0;
};

class CRecordCmd :public CBufferCmd
{
private:
	const string m_type;
	const string m_strDVRName;
	const string m_strCameraId;
	const int m_DVRChannelID;
	const string m_strServIp;

	string m_strRecordPath;
	string m_strRecordName;
	FILE *_fp;	
	SYSTEMTIME			m_tLastSaveTime;		// Â¼Ïñ±£´æÊ±¼ä

	void init();
public:
	CRecordCmd(const CAMERA_INFO &info);
	BOOL handleFunc(const BYTE *const pBuffer, const DWORD dwBufSize);
};

class CSendCmd :public CBufferCmd
{
private:
	const SOCKET _socket;
	int _FailNum;

	static const int FailTimes = 200;
public:
	CSendCmd(SOCKET s);
	BOOL handleFunc(const BYTE *const pBuffer, const DWORD dwBufSize);
};

#endif