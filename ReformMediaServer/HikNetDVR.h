#pragma once
#include "dvr.h"
#include "HikNetDVRChannel.h"

class CHikNetDVR : public CDVR
{
public:
	CHikNetDVR(const char* const sDVRIP, const WORD wDVRPort, const char* const sUserName, const char* const sPassword);
	~CHikNetDVR(void);

	BOOL connect();
	BOOL disconnect();

private:
	CChannel* stream(const int CnlID, const int type)
	{
		if(m_lLoginID < 0)
			return NULL;
		return streaming<CHikNetDVRChannel>(CnlID,type);
	}

};

