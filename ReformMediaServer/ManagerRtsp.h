#pragma once

//#include "RTSP.h"
#include "ReformMediaServerDef.h"
#include <string>
#include <map>

using namespace std;

#define PTZCTRL_TIMEOUT			180	// 云台控制超时时间 3 * 60

// 云台控制状态
typedef struct _PTZState
{
	string		channelid;			// 通道号 （是否唯一编号？）
	time_t		lasttime;			// 最后控制时间
	bool		bUsing;				// 是否被使用中
	int			m_SessionIndex;		// 控制者
}PTZState, *LpPtzState;

typedef map<string, LpPtzState> MapPtzState;
typedef MapPtzState::iterator ItPtzStateMap;
typedef MapPtzState::value_type VtPtzStateMap;

class CManagerRtsp
{
public:
	CManagerRtsp(void);
public:
	~CManagerRtsp(void);

	SOCKET	m_SockRTSPlisten;	  // 服务器端RTSP的监听Sock
	int		m_SessionIndex;
	
	BOOL Init();
	BOOL OnAccept();
	void ReceiveMsg(int index);
	void CloseCli(int index);
	void SendRtspMessage(int clientSocket, char * buf, int bufLength, int flag);		// 发送rtsp回应

	RTSPCLIENT_INFO m_RtspClientArray[MAX_CLIENT_SIZE];
	int m_ClientNum;

	BOOL OptionAnswer(char *recvbuf, int index);
	BOOL SetupAnswer(char *recvbuf,int index);
	BOOL PlayAnswer(char *recvbuf,int index);
	BOOL PauseAnswer(char *recvbuf,int index);
	BOOL TeardownAnswer(char *recvbuf,int index);

	BOOL PTZCommand(char *recvbuf,int index);		// 增加云台控制功能
	BOOL PTZStateQuery(char *recvbuf, int index);	// 云台控制状态查询
	BOOL PTZObtain(char *recvbuf,int index);			// 占有云台控制权限
	BOOL PTZRelease(char *recvbuf,int index);			// 释放云台控制权限
	BOOL PTZGetDirection(char *recvbuf,int index);	// 获取云台当前旋转角度(阻塞操作)

	BOOL GetFindInfo(_int64 session, FIND_INFO &findinfo);
	BOOL CapturePic(char *recvbuf, int index); //服务器抓图
private:
	BOOL IsPtzUsed(char *recvbuf, int index);	// 云台控制状态查询
	map<string, string> _ptz_user;
	map<string , int> _ptzStatus;
	
	struct PTZPush
	{
		int session;
		string msg;
	};
	map<string, PTZPush> _ptzPushMap;
	int ptzPushsession;

// 维护一些已经连上RTSP，但是未连上STREAM的客户端连接，当长时间（100秒以上）没连上STREAM，关闭该RTSP连接
/*	CArray<RTSPCLIENT_INFO , RTSPCLIENT_INFO> m_RtspArray;
	HANDLE		m_ArrayMutex;
	BOOL RemoveArray(_int64 session);
*/
	BOOL GetChannelInfo(char* channelname, CHANNEL_INFO& channelInfo);
public:
	BOOL CheckPsw(char *psw);

private:
	MapPtzState m_mapPtzState;
};
/*
100 Continue all
200 OK all
201 Created RECORD
250 Low on Storage Space RECORD
300 Multiple Choices all
301 Moved Permanently all
302 Moved Temporarily all
303 See Other all
305 Use Proxy all
400 Bad Request all
401 Unauthorized all
402 Payment Required all
403 Forbidden all
404 Not Found all
405 Method Not Allowed all
406 Not Acceptable all
407 Proxy Authentication Required all
408 Request Timeout all
410 Gone all
411 Length Required all
412 Precondition Failed DESCRIBE, SETUP
413 Request Entity Too Large all
414 Request-URI Too Long all
415 Unsupported Media Type all
451 Invalid parameter SETUP
452 Illegal Conference Identifier SETUP
453 Not Enough Bandwidth SETUP
454 Session Not Found all
455 Method Not Valid In This State all
456 Header Field Not Valid all
457 Invalid Range PLAY
458 Parameter Is Read-Only SET_PARAMETER
459 Aggregate Operation Not Allowed all
460 Only Aggregate Operation Allowed all
461 Unsupported Transport all
462 Destination Unreachable all
500 Internal Server Error all
501 Not Implemented all
502 Bad Gateway all
503 Service Unavailable all
504 Gateway Timeout all
505 RTSP Version Not Supported all
551 Option not support all
*/