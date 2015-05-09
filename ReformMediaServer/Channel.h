// Channel.h: interface for the CChannel class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "ReformMediaServerDef.h"
#include <string>
#include "CLock.h"
#include "CBufferCmd.h"

using namespace std;

//#define RF_VIDEO_RECORD_FOLDER "reformvideorecord"

////class CVideoRecord;
//class CDVR  ;
//class CVideoTrans  ;
//class CMultiCastVideoTrans;

class CChannel  
{
//public:
//	CChannel();
//	virtual ~CChannel();
//
//	virtual BOOL	Init(int ChannelID, int index, int DVRChannelID, CDVR *pDVR);	// 初始化
//	virtual BOOL	InitMultiCast();												// 初始化多播
//	virtual BOOL	SetImageParam(IMAGEPARAM_INFO info);
//	BOOL	SendBuffer(char *pbuffer, DWORD length, BOOL bhead);			// 给客户端发送数据
//	virtual BOOL	GetPic(char* sFileName);
//	virtual BOOL	StartTrans()=0;
//	BOOL	AddTrans(SOCKET sock);											// 增加数据源
//	BOOL	CloseTrans(int index);											// 关闭数据源
//	virtual BOOL	PostStopTrans();
//	virtual BOOL	StopTrans() = 0;
//
//	virtual BOOL	PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop);		// 云台控制
//	//virtual BOOL	StartRecord(BOOL bRecord, char * pPath);						// 启动录像
//
//	CDVR			*m_pDVR;				// 本通道所属的DVR设备的指针
//	string			m_strDVRName;			// 所属dvr名称
//	int				m_DVRChannelID;			// 通道在DVR中的顺序号
//
//	CHANNEL_INFO	m_ChannelInfo;			// 通道的连接信息
//	IMAGEPARAM_INFO m_ImageInfo;			// 视频图像的参数信息
//
//	int				m_index;				// 所在数组
//	string			m_strCameraId;			// 摄像头编号
//	int				m_Frames;
//	int				m_subFrames;
//	int				m_Playhandle;
//
//	int				m_SocketNum;
//
//	BOOL			m_bRecord;				// 是否录像
//	string			m_strRecordPath;		// 录像路径
//	string			m_strRecordName;		// 录像名称（录像名字 = dvr名称 + dvr通道名称 + 录像开始时间）
//
//	time_t			m_tLastSaveTime;		// 录像保存时间
//	string			m_strLastSaveTime;		// 录像保存时间
//	//FILE			*fp;					// 文件指针
//	int				m_recordId;				// 录像文件在数据库中的编号
//
//	int				m_FileLength;			// 文件字节长度
//	BOOL			m_bTrans;
//	BOOL			m_bMotion;
//	BOOL			m_bCurMotiom;
//
//	CVideoTrans	*m_VideoTransArray[MAX_CLIENT_SIZE];			// 收到数据流之后，转发方式有两种(组播，和针对列表内的用户发送)
//	CMultiCastVideoTrans *m_MultiCastVedio;						// 
//
//	string m_type_ID;//设备类型英文缩写
//	string m_type;//设备类型中文
//	FILE			*_fp;					// 文件指针
//	void save_record(char *pbuffer, DWORD length);
//	void add_record_to_db();
//	bool get_record_file_path(char* path, char* file_path, char* file_name);
//
//	long icon_port;
//	bool is_icon_init_;
//	bool is_ready_icon_;
//	char icon_path[MAX_PATH];
//	void init_icon(char* name);
//	virtual void save_icon(char *pbuffer, DWORD length) = 0;
//
//	int m_work_status;
//
//	time_t m_disconnect_time;
//
//	virtual BOOL stop_by_Socket(SOCKET s);
//
//	//释放硬盘空间
//	//成功删除录像文件，返回1
//	//只删除了数据库记录但未能删除文件，返回0，重试
//	//查询第一条数据库记录失败，返回-1
//	int freeDiskspace();
//
//	int PTZControlIndex;
//	BOOL m_bControl;
//
//	int m_YTStatus;
//	map<int,int> m_mapYTCmd;
//	int m_lastCmd;
//
//	static volatile DWORD m_RecordStatus;
//
//	virtual BOOL Init(LONG lLoginID, int nChannelID){return FALSE;}

public:
	CChannel(const LONG lLoginID, const int nChannelID, const int tStreamType);
	virtual ~CChannel();

	const LONG _LoginID;
	const int _ChannelID;
	const int _StreamType;

	//enum{
	//	CH_Run=0,
	//	CH_Pend,
	//	CH_Stop
	//}_ConnectStatus;
	LONG _Playhandle;
	virtual BOOL connect()=0;//{return FALSE;};
	virtual BOOL disconnect() =0;//{return TRUE;};

	BOOL addCmd(CBufferCmd* cmd);
	BOOL delCmd(CBufferCmd* cmd);
	void VedioFunc(const BYTE* pBuffer, const DWORD dwBufSize);

	int m_ptzcount;
	static const int m_ptznum = 16;
	static const int m_ptzdiff = 30;
	time_t m_ptzbegintime;
	BOOL m_bControl;
	int m_lastCmd;
	BOOL	PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop);		// 云台控制
	
	CTry _Atry;

	BOOL idle();
	
	/*
	*与idle()结合使用，当idle()返回TRUE时，
	*比较上次任务执行时间与当前时间：
	*如果时差超过1小时，返回FALSE;
	*否则，返回TRUE
	*/
	BOOL cmdTimeOut();
private:
	CLock _lock;
	time_t m_lastcmdtime;

	map<DWORD, CBufferCmd*> _CmdList;
	virtual BOOL	_PTZControl(DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop){return FALSE;}		// 云台控制
};
