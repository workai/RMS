// DVR.h: interface for the CDVR class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "ReformMediaServerDef.h"
#include <string>

using namespace std;

class CChannel;  

class CDVR  
{
private:
	map<int, CChannel*> _ChannelList;
	
public:
	CDVR(const char* const sDVRIP,const WORD wDVRPort, const char* constsUserName, const char* const sPassword);
	virtual ~CDVR();

	LONG m_InvalidID;
	LONG m_lLoginID;
	int m_CnlNum;
	int m_StartCnl;

	const string m_DVRIP;
	//char				m_DVRIP[16];
	const WORD			m_DVRPort;
	char			m_UserName[50];
	char			m_Password[50];
	
	time_t m_disconnent_time;
	virtual BOOL connect() =0;//{return FALSE;}
	virtual BOOL disconnect() =0;//{return TRUE;}

	virtual CChannel* stream(const int CnlID,const int type/*=0*/) =0;
	/*
	*自动管理
	*如果CnlID已经连接，返回现有连接
	*/
	CChannel* tryStream(const int CnlID, const int type);

	///*
	//*使用完毕调用，需要手动delete释放
	//*/
	//CChannel* handStream(const int CnlID);

	/*删除所有channel*/
	void clearChannels();
	/*
	释放无任务的channel
	*/
	void freeChannels();
	BOOL Online();

	template<typename T>
	T* streaming(const int CnlID, const int type)
	{
		T* channel = NULL;
		map<int, CChannel*>::iterator iter = _ChannelList.find(CnlID);
		if(iter != _ChannelList.end())
		{
			channel = dynamic_cast<T*>(iter->second);
		}
		else
		{
			channel  = new T(m_lLoginID,CnlID,type);
			_ChannelList[CnlID] = channel;
		}
		if(channel->connect())
			return channel;
		return NULL;
	}

};
