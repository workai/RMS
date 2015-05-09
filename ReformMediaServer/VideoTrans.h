#pragma once
//#include "ReformMediaServerDef.h"

/*
发送从ipc（网络摄像机）获取的数据
*/
class CVideoTrans
{
public:
	CVideoTrans(SOCKET sock);
public:
	~CVideoTrans(void);

	BOOL AddBuffer(char *pbuf, int length);
	BOOL OnTransVideo();

	static DWORD WINAPI TransThread( LPVOID lpParameter);

	SOCKET	m_Socket;
	BOOL	m_bTrans;				//当前对象是否活着！

	HANDLE 		m_hTransThread;		//
	HANDLE		m_hTransKill;		//
	HANDLE		m_DataEvent ;		//
	HANDLE		m_BufMutex;

	long m_bufsize;
	char*		m_Membuf;
	int			m_Buflength;

	char	ChannelName[32];

};
