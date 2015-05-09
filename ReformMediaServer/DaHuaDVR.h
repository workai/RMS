#pragma once
#include "ReformMediaServerDef.H"
#include "dvr.h"
#include "DHDVRChannel.h"

class CDaHuaDVR :public CDVR
{
private:
	const static char* const m_loginerr[10];
public:
	CDaHuaDVR(const char *  sDVRIP,const WORD wDVRPort, const char *  sUserName, const char* sPassword);
	~CDaHuaDVR(void);

	BOOL connect();
	BOOL disconnect();
	
private:
	CChannel* stream(const int CnlID, const int type)
	{
		if(m_lLoginID==0)
			return NULL;
		return streaming<CDHDVRChannel>(CnlID,type);
	}

};
