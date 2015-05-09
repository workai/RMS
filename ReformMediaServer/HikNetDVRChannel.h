#pragma once
#include "channel.h"


// 海康dvr设备
class CHikNetDVRChannel : public CChannel
{
//public:
//	CHikNetDVRChannel(void);
//	~CHikNetDVRChannel(void);
//
//	// 启动预览并设置回调数据流
//	BOOL Init(LONG lLoginID, int nChannelID);
//	BOOL	InitMultiCast();
//
//	BOOL	StartTrans();
//	BOOL	PostStopTrans();
//	BOOL	StopTrans();
//	BOOL	SetImageParam(IMAGEPARAM_INFO info);
//	BOOL	GetPic(char* sFileName);
//
	static const map<int,int> m_mapYTCmd;
	BOOL	_PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop);// 云台控制
//
//	unsigned char* icon_data_;
//	DWORD icon_len_;
//	void save_icon(char *pbuffer, DWORD length);
public:
	CHikNetDVRChannel(LONG lLoginID, const int nChannelID, const int tStreamType);
	~CHikNetDVRChannel();
	BOOL connect();
	BOOL disconnect();
};

