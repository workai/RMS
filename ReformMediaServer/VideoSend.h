// VideoSend.h: interface for the CVideoSend class.
//
//////////////////////////////////////////////////////////////////////


#pragma once
//#include <afxtempl.h>
//#include "VideoSend.h"
#include "ReformMediaServerDef.H"

class CFileIndex;

class CVideoSend
{
public:// 共同部分
	CVideoSend();
	virtual ~CVideoSend();

	BOOL	SendVideo(char *pDate, int length);
	BOOL	AcceptSock(SOCKET s, int num);
	BOOL	InitSock();
	BOOL	GetPeerName(ATL::CString& rPeerAddress, UINT& rPeerPort);
	BOOL	SetSendType(int type);
	
	BOOL	OnSendFile();
	BOOL	OnSendJpgFile();
	BOOL	End();
	static	DWORD WINAPI NetThread( LPVOID lpParameter);

//public:// 网络播放部分：
	BOOL	Init(FIND_INFO info);
	BOOL	StartNetPlay(_int64 startpos);//iStartTime为录象的具体时间！！！
	BOOL	PauseNetPlay();
	BOOL	ContinueNetPlay();

	CFileIndex *m_pFileIndex;
	int			m_FileIndex;
	int			m_StampIndex;

	FIND_INFO	m_findinfo;

    //char	m_csAdd[20];				// 客户端IP地址
	CString				m_csAdd;		// 客户端IP地址
	unsigned int 		m_Port;			// 客户端端口号
//	int		TokenID;

	SOCKET				m_Socket;
	int					m_sockid;		// 在manager队列中的位置
	int					m_SendType;		// 发送类型！！！

	HANDLE 		m_hNetThread;			// 负责网络播放、下载处理的线程句柄
	HANDLE		m_hNetKill;				// 触发网络播放、下载处理线程结束的事件
	HANDLE		m_DataEvent ;			// 触发网络播放、下载处理线程暂停启动的事件
	ATL::CString  m_Filepath;
	HANDLE		m_NetFile;
	int			m_ReadFilePos;
	DWORD			m_FileLength;
	int			m_curFileLength;
	BOOL		m_NetActive;

	char	m_Head[1024];
	int		m_HeadLength;
	int		m_Speed;
	char	m_JpgSendMem[512*1024];
	int		m_JpgSendMemLength;
	
	DWORD _left_length;
	char _readbuf[BLOCKSIZE_FILE];
	DWORD _buf_size;
	DWORD _buf_left;
	BOOL StartNetPlayOne();
	BOOL OnSendOneFile();
	bool onerun;
	static	DWORD WINAPI NetOneThread( LPVOID lpParameter);
};

