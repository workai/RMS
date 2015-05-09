// ManagerDvr.h: interface for the CManagerDVR class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "ReformMediaServerDef.h"

class CDVR;  
class CChannel;  

class CManagerDVR  
{
public:
	CManagerDVR();
	virtual		~CManagerDVR();

	BOOL		Init();			// 加载本地配置的dvr文件

	int AddServer(const CHANNEL_INFO& channel);
	int AddServer(CString strIP, int port, USER_NAME_INFO info, int type,CString strCnlName, bool mis, char* mip, int mport);
	BOOL CloseServer(CString strIP, int port);

	CDVR *FindServer(CString serverip, int port);
	CChannel *FindChannel(char *channelname);
	CChannel *FindChannel(CString serverip,int port, int channelid, int type);

	vector<CDVR*> m_unconnect_list;
	bool m_isRunning;
	static DWORD thread_func_connect_dvr(void* p);

public:
	int			m_ServerNum;						// 管理的dvr数量
	CDVR		*m_pDVRArray[MAX_SERVER_SIZE];		// DVR列表
	CChannel	*m_pChannelArray[MAX_SERVER_SIZE * MAX_CHANNEL_SIZE];

	//map<string, CChannel*> m_StreamList;
	map<string, CAMERA_INFO> m_CameraList;
	map<string, CDVR*> m_DVRList;//放到DVR中，static

	BOOL addClient(string cameraID, SOCKET client);
	BOOL controlCamera(string cameraID);
	BOOL FindChannel(char *channelname, CHANNEL_INFO& channelInfo);

	BOOL	PTZControl(string cameraID, DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop);
	BOOL PTZStateQuery(const char* cid);
	void Release();

	map<string, CAMERA_INFO> m_RecordList;
	//vector<CAMERA_INFO> m_FailRecordList;
	static unsigned int WINAPI reconnectFunc(LPVOID inThread);

#ifdef DEBUG
	void test();
#endif
};

