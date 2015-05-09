#pragma once
//#include "ReformMediaServerDef.h"

class CMultiCastVideoTrans
{
public:
	CMultiCastVideoTrans(void);
	~CMultiCastVideoTrans(void);

	BOOL Init(SOCKET sock, sockaddr_in remote);
	BOOL AddBuffer(char *pbuf, int length);

	BOOL OnTransVideo();
	BOOL End();

	static DWORD WINAPI TransThread( LPVOID lpParameter);

	SOCKET	m_Socket;
	sockaddr_in m_Remote;
	BOOL	m_bTrans;				// 当前对象是否活着！
	
	HANDLE 		m_hTransThread;		//
	HANDLE		m_hTransKill;		//
	HANDLE		m_DataEvent ;		//
	HANDLE		m_BufMutex;

	char*		m_Membuf;
	int			m_Buflength;
};
