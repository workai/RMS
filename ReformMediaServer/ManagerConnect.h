// ManagerConnect.h: interface for the CManagerConnet class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// 客户端视频文件发送管理类

#include "ReformMediaServerDef.h"

class CVideoSend;

// 
class CManagerConnect  
{
public:
	CManagerConnect();
	virtual ~CManagerConnect();

	BOOL OnAccept();
	BOOL InitListenSocket();

	void ReceiveMsg(int iPlayNum);
	void CloseCli(int index);
	//void stop_Client(int index);
//	void	StopFileDown();
	int FindClient(_int64 session);

	SOCKET m_Listen;
    sockaddr_in local;

	int		m_clientnum;

	CVideoSend *m_SocketArray[MAX_CLIENT_SIZE];        // socket列表   

//	HANDLE				m_VideoSendMutex ;

////RTSP
 //  int m_RtspStatus;	 //用于表示RTSP自动机的状态

private:
	BOOL AddClient(CVideoSend *pCon, int &index);	

};
