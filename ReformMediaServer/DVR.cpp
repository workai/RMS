// DVR.cpp: implementation of the CDVR class.
//
//////////////////////////////////////////////////////////////////////
#include "DVR.h"
#include "StdHeader.h"
#include "Channel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVR::CDVR(const char* const sDVRIP,const WORD wDVRPort, const char* const sUserName, const char* const sPassword)
	:m_DVRIP(sDVRIP),
	m_DVRPort(wDVRPort)
{
	m_CnlNum = 0;
	//memcpy(m_DVRIP,sDVRIP,16);
	//m_DVRPort= wDVRPort;
	memcpy(m_UserName, sUserName, 50);
	memcpy(m_Password, sPassword, 50);
}

CDVR::~CDVR()
{
	//clearChannels();
	//disconnect();
}

CChannel* CDVR::tryStream(const int CnlID, const int type)
{
	CChannel* ret=NULL;

	if(connect())
	{
		ret = stream(CnlID, type);
	}

	return ret;
}

void CDVR::clearChannels()
{
	for(map<int, CChannel*>::iterator iter = _ChannelList.begin();iter!=_ChannelList.end();)
	{
		iter->second->disconnect();
		delete iter->second;
		iter = _ChannelList.erase(iter);
	}
}

void CDVR::freeChannels()
{
	for(map<int, CChannel*>::iterator iter = _ChannelList.begin();iter!=_ChannelList.end();)
	{
		if(iter->second->idle())
		{
			iter->second->disconnect();
			delete iter->second;
			iter = _ChannelList.erase(iter);
		}
		else
			iter++;
	}
}

BOOL CDVR::Online()
{
	BOOL ret = TRUE;
	for(map<int, CChannel*>::iterator iter = _ChannelList.begin();iter!=_ChannelList.end();iter++)
	{
		if(iter->second->idle()==FALSE && iter->second->cmdTimeOut()==FALSE)
		{
			ret = FALSE;
		}
	}
	return ret;
}