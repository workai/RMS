#pragma once
#include "channel.h"

#define MCASTADDR "233.0.0.1"		// 本例使用的多播组地址。
#define MCASTPORT 5150				// 本地端口号。

// 大华dvr设备
class CDHDVRChannel : public CChannel
{
//public:
//	CDHDVRChannel();
//public:
//	~CDHDVRChannel(void);
//
//	// 初始化通道，并请求视频数据
//	BOOL Init(LONG lLoginID, int nChannelID);
//	BOOL	InitMultiCast();
//
//	BOOL	StartTrans();
//	BOOL	PostStopTrans();
//	BOOL	StopTrans();
//	BOOL	SetImageParam(IMAGEPARAM_INFO info);
//	BOOL	GetPic(char* sFileName);
	static const map<int,int> m_mapYTCmd;
	BOOL	_PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop);// 云台控制
//	//BOOL	StartRecord(BOOL bRecord, char * pPath);			// 启动录像
//
//	void save_icon(char *pbuffer, DWORD length);
//
//private:
	//void	SaveRecord(char *pbuffer, DWORD length);
public:
	CDHDVRChannel(LONG lLoginID, const int nChannelID, const int tStreamType);
	~CDHDVRChannel();
	BOOL connect();
	BOOL disconnect();
};
