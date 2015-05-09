//FILESERVERDEF.H

#pragma once

#include "StdHeader.h"

#include <map>
#include <string>
#include <vector>
#include <list>
#include <algorithm> 
//#include <functional>
using namespace std;

class CLog;
class CManagerConnect;
class CSysInfo;
class CManagerRtsp;
class CManagerDVR;

extern HWND			g_hWnd;
extern WNDPROC		g_lpOriginalProc;           // 消息响应窗体原始地址
extern HWND			g_hWnd2;
extern WNDPROC		g_lpOriginalProc2;           // 消息响应窗体原始地址

extern CLog				*g_pLog;
extern CManagerConnect	*g_pManagerConnect;
extern CManagerRtsp		*g_pManagerRtsp;
extern CSysInfo			*g_pSysInfo;
extern CManagerDVR		*g_pManagerDVR;

#define WM_DVRALARMCOMMAND			2410	//消息,DVR异常上传消息
#define WM_LISTEN_SOCKET			4000	//消息,异步SOCK处理消息,流
#define WM_LISTEN_COMMANDSOCKET		3000	//消息,异步SOCK处理消息,RTSP
#define WM_STOPTRANS_COMMANDSOCKET	3500	//消息,停止转发

#define   MAX_CLIENT_SIZE			512

#define   MAX_SERVER_SIZE			64

#define   MAX_CHANNEL_SIZE			64

#define   HEAD_SIZE					40


#define   MEMBUF_LENGTH					16*1024*1024
///通道类型
#define  HIK_DVRHK_CHANNEL			1//海康DVR
#define  HIK_DVRDH_CHANNEL			2//大华DVR
#define  HIK_JPG_CHANNEL			3//大华DVR
#define  HIK_MJPG_CHANNEL			4//大华DVR


///视频格式
#define   VIDEO_QCIF					1
#define   VIDEO_CIF						2
#define   VIDEO_4CIF					4

/************************************************************************/
/*服务器与客户端间命令接口！                              */
/************************************************************************/
#define FM_COMMAND_MESSAGE 0X5000
//远程文件播放
#define FM_FILEPLAY_REG		FM_COMMAND_MESSAGE + 0
#define FM_FILEPLAY_RE		FM_COMMAND_MESSAGE + 1
#define FM_FILEDOWN_REG		FM_COMMAND_MESSAGE + 2

#define FM_NETPLAY_PAUSE		FM_COMMAND_MESSAGE + 3
#define FM_NETPLAY_CONTINUE		FM_COMMAND_MESSAGE + 4

#define FM_CHANNELPLAY_REG		FM_COMMAND_MESSAGE + 5
#define FM_CHANNELPLAY_RE		FM_COMMAND_MESSAGE + 6

// 云台控制
#define FM_CHANNEL_PTZCONTROL_REG		FM_COMMAND_MESSAGE + 7
#define FM_CHANNEL_PTZCONTROL_RE		FM_COMMAND_MESSAGE + 8

//服务器和客户端之间视频文件传输类型
#define FILE_PLAY			1
#define FILE_DOWN			2
#define LIVE_PLAY			3

#define BUF_SIZE			1024

#define 	OPTIONS_CMD	1
#define		PLAY_CMD 	2
#define		SETUP_CMD	3		
#define		STOP_CMD	4
#define		PAUSE_CMD	5


typedef struct 
{
	char FileName[MAX_PATH];
	int startpos;
	int endpos;//-1表示到文件结尾处
}FILE_INFO;//查询出来的录象文件信息

//typedef struct
//{
//	int channelType;
//	bool multicast;
//	char multicastAddress[20];
//	char multicastPort[10];
//}

typedef struct 
{
	int startblock;//代表开始文件块号
	int endblock;//代表结束文件块号
	int BlockFileNum;//代表录像段数

}IDXFILEHEAD_INFO;//.IDX文件的头信息

typedef struct 
{
	_int64 starttime;//代表开始shijian
	_int64 endtime;//代表结束shijian 
	int startblock;//代表开始文件块号
	int endblock;//代表结束文件块号
	int stampnum;//代表时间戳数量
}IDXFILEBLOCK_INFO;//.IDX文件段信息

typedef struct 
{
	int blockindex;//代表所在文件块号
	int pos;//代表该时间戳在文件块的位置
	_int64 time;//代表时间戳时间戳的时间点
}IDXSTAMP_INFO;//.IDX文件的时间戳信息

typedef struct 
{
	char filename[150];//代表所在文件块号
	int pos;//代表该时间戳在文件块的位置
	_int64 time;//代表时间戳时间戳的时间点
}FILESTAMP_INFO;//.IDX文件的时间戳信息

typedef struct 
{
	char channelid[MAX_PATH];
	_int64 starttime;//代表开始shijian
	_int64 endtime;//代表结束shijian 
	_int64 session;
	int		type;	//类型
	int		encodetype;//编码格式;
}FIND_INFO;//.IDX文件的时间戳信息

typedef struct 
{
	FIND_INFO findinfo;
//	_int64 time;//代表信息请求的时间，SETUP
	SOCKET sock;
//	int index;//所在数组位置

}RTSPCLIENT_INFO;//.IDX文件的时间戳信息


typedef struct 
{
	char	UserName[32];
	char	Password[32];
}USER_NAME_INFO;                                                                                                                                                                                                                                                                                                              
typedef struct 
{
	char	ServerIP[16];
	char	DVRName[20];		// DVR名称
	int		port;
	int		ChannelID;			// 通道ID
	char	ChannelName[32];	// 通道名称
	int		ChannelType;		// 通道类型mgqja625450
	int		Format;
	int		Quality;			//（1-10)
	char	username[32];
	char	psw[32];
	bool	isMulticast;
	char	MultiCastIP[16];
	int		MultiPort;
}CHANNEL_INFO;

typedef struct 
{
	string	strCameraID;	
	string	eCameraType;
	string	strName;	
	string	strModel;
	string	strProvider;
	string	strCodec;
	string	strLogin;
	string	strPassword;
	string	strIP;
	string	strPort;
	string	strCnlID;
	string	boolMultiCast;
	string	strMultiCastAddress;
	string	strMultiCastPort;
	string	eRecordType;
}CAMERA_INFO;

typedef struct 
{
	int Brightness;//亮度值（0-255,DVR是1-10）
	int Contrast; //对比度（0-127，DVR是1-10）
	int Saturation;//饱和度(0-127，DVR是1-10）
	int Hue;//色调0-255，DVR是1-10）
}IMAGEPARAM_INFO;

// 时间包
typedef struct _TimePack
{
	unsigned short    nYear;
	unsigned char     nMonth;
	unsigned char     nDay;
	unsigned char     nSer;
	unsigned char     nHour;
	unsigned char     nMinute;
	unsigned char     nSecond;
} TimePack, * LPTimePack;


// ------------------------------
// IPC设备
typedef struct _RFCamera
{
	int			nId;					// 数据库中的自动编号
	string		strCameraId;			// 系统生成的摄像头设备编号
	string		strCameraType;			// 摄像头类型
	string		strCameraName;			// 摄像头名称
	string		strMode;				// 摄像头类型(DVR、NVR、IPC，，，)
	string		strProvider;			// 设备厂家(大华、海康、海思，，，)
	string		strCodec;				// 编码格式
	string		strLogin;				// 连接到设备时登录的用户名
	string		strPassword;			// 登录密码
	string		strIp;					// 设备的IP地址
	string		strPort;				// 设备连接端口号
	string		strCnlId;				// 设备连接通道号

	string		strMultiCast;			// 是否广播数据
	string		strMultiCastAddress;	// 广播地址
	string		strMultiCastPort;		// 广播端口
	string		strRecordType;			// 
	
	_RFCamera()
	{
		nId						= -1;
		strCameraId				= "";
		strCameraType			= "";
		strCameraName			= "";
		strMode					= "DVR";
		strProvider				= "大华";
		strCodec				= "H264";
		strLogin				= "admin";
		strPassword				= "admin";
		strIp					= "";
		strPort					= "37776";
		strCnlId				= "1";

		strMultiCast			= "true";
		strMultiCastAddress		= "";
		strMultiCastPort		= "5152";
		strRecordType			= "";
	};
} RFCamera, *LPRFCamera;

// IPC设备列表
typedef map<int, LPRFCamera> MapRFCamera,*LPMapRFCamera; 
typedef MapRFCamera::value_type ValuetRFCamera;
typedef MapRFCamera::iterator IterRFCamera;

// 录像存储类型
enum RF_RECORD_TYPE
{
	RECORD_FORBIT = 0,				// 不存储
	RECORD_BYMONTH,					// 按月的指定日期存储
	RECORD_BYWEEK,					// 指定一周的某天存储
	RECORD_EVERYDAY,				// 按设定的绝对日期时间存储
	RECORD_AWAYS					// 总是存储
};

// 时间类型
typedef struct _RFTime
{
	int			Hour;
	int			Minute;
	int			Second;
} RFTime;

// 开始时间到结束时间
typedef struct _RFRecordTime
{
	RFTime		tStart;
	RFTime		tStop;
}RFRecordTime;

// ------------------------------
// 录像存储的策略
typedef struct _RFRecordSetting
{
	int				nRecordId;			// 对应配置文件的录像编号
	string			strCameraId;		// 针对哪路IPC进行存储
	string			strExName;			// 给录像添加的名称标识
	string			strRecordPath;		// 存储的目标路径（暂时保存在当前路径下,后面根据情况扩展）
	RF_RECORD_TYPE	nRecordType;		// 录像类型
	int				nRecordMask;		// 标识存储的日期( 01111111111111111111111111111111 )
	bool			bRecordAnyTime;		// 录像的时间点
	list<RFRecordTime>	listRecordTime;	// 录像记录的时间
	string			strChangeFileReasion;// 录像时按设定时间录像、或者按大小录像
	int				nChangeFileValue;	// 多少秒或者多少字节
	string			strDiskFullDo;		// 磁盘空间不够时：停止录像、删除最以前的记录、切换到系统中磁盘空间最大的盘

	_RFRecordSetting()
	{
		nRecordId				= -1;
		strCameraId				= "";
		strExName				= "";
		strRecordPath			= "";
		nRecordType				= RECORD_FORBIT;
		nRecordMask				= 0;
		bRecordAnyTime			= false;
		strChangeFileReasion	= "bytime";// bysize
		nChangeFileValue		= 500;
		strDiskFullDo			= "";

	}
} RFRecordSetting, *LPRFRecordSetting;

// 设定的录像策略列表
typedef map<string, LPRFRecordSetting> MapRecordSettings;
typedef MapRecordSettings::iterator IterMapRecordSettings;
typedef MapRecordSettings::value_type ValueMapRecordSettings;


// 录像状态
typedef struct _RFRecordState
{
	int			nRecordId;			// 录像编号
	int			nRecordIndex;		// 如果有多个录像时间，此处记录第几个时间段
	bool		bIsRecording;		// 是否处于下载状态
	string		strFile;			// 当前录像的名称，因为单个文件不能超过4G，所以当录像时间很长时，需要切分文件
	int			nRecordSize;		// 当前录像文件的大小(单位：字节)
	int			nRecordTime;		// 当前录像的时长(单位：秒)

	_RFRecordState()
	{
		nRecordId		= -1;
		bIsRecording	= false;
		nRecordIndex	= -1;
		strFile			= "";
		nRecordSize		= 0;
		nRecordTime		= 0;
	}			
} RFRecordState, * LpRFRecordState;

// 录像中的录像状态信息
typedef map<int, LpRFRecordState> MapRecordState;
typedef MapRecordState::iterator IterMapRecordState;
typedef MapRecordState::value_type ValueMapRecordState;

// 查询录像时，录像在本地的信息
typedef struct _QueryRecordResult
{
	int			nRecordId;				// 录像记录入库时的id标识
	int			nDeviceid;				// id标识
	string		strRecordName;			// 录像文件的名称
	string		strFolder;				// 录像文件所在的文件夹路径
	int			nFileSize;				// 录像文件大小
	bool		bIsRecording;			// 录像文件是否还在录像中（如果录像还在进行，此时读取该录像文件的数据会失败）
	string		tStart;					// 录像开始时间
	string		tEnd;					// 录像结束时间
	string		strDVRName;				// 录像文件所在DVR的名称标识
	string		strDVRAddr;				// 录像文件所在DVR的IP地址
	string		strDVRPort;				// 录像文件所在DVR的端口号
	string		strDVRChannel;			// 录像文件所在DVR的通道号
	string		strProvider;
}QueryRecordResult, *LpQueryRecordResult;

// 查询时返回的录像列表
typedef map<int, LpQueryRecordResult> MapQueryRecordResult;
typedef MapQueryRecordResult::iterator IterMapQueryRecordResult;
typedef MapQueryRecordResult::value_type ValueMapQueryRecordResult;


//---------------------------------------
// 云台控制策略的设定
typedef struct _PTZControlPolicy
{
	int			nUserId;			// 客户端的标识
	TimePack	timeLastCtrl;		// 最后控制时间
	vector<int>	vetWaitingUsers;	// 等待列表(是否实现排队通知策略?)

} PTZControlPolicy, *LPPTZControlPolicy;

//---------------------------------------
// 设备连接信息
typedef struct _ConnectionInfo
{
	string	strRtspUrl;		// rtsp连接字符串
	string	strIPAddr;		// IP
	int		nPort;			// 端口
	int		nCnlId;			// 设备通道

	int		nPlayerId;		// 客户标识编号
	string	strProvider;	// 厂家


	_ConnectionInfo()
	{
		strRtspUrl = "";
		strIPAddr = "";
		nPort = 37776;
		nCnlId = 1;
		nPlayerId = -1;
		strProvider = "DH";
	};
} ConnectionInfo, *LPConnectionInfo;


BOOL BuildWindow();
LRESULT WINAPI WndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //窗体消息响应
LRESULT WINAPI WndSubClassProc2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //窗体消息响应

void Ticks2Time(_int64 dwtime, CTime &time);
_int64 Time2Ticks(CTime time);

#define BLOCKSIZE_LIVE 8*1024
#define BLOCKSIZE_FILE 8*1024
#define BLOCKSIZE_UDP 15*1024

#define TIME_LOOP 30 


/*
	PicFormat_BMP = 0,
	PicFormat_JPEG,
	PicFormat_JPEG_70,
	PicFormat_JPEG_50,
	PicFormat_JPEG_30,
	PicFormat_JPEG_10,
	*/
#define ICON_FORMAT 1
#define ICON_EXTENSION "jpeg"
#define ICON_WIDTH 256
#define ICON_HEIGHT 256

#define RECORD_INTERVAL 30*60

#define ICON_HK_BUFFER_LEN 1024*1024

BOOL check_IP(const char* str);

#define VT_SEND_TIMEOUT 10000

void terminate_thread(HANDLE& thread, HANDLE hevent = NULL);

CString GetAppPath();

// 力方科技云台控制命令
enum PTZCommand
{
    PTZ_UP = 0,						// 向上
    PTZ_DOWN,						// 向下
    PTZ_LEFT,						// 向左
    PTZ_RIGHT,						// 向右
    PTZ_ZOOM_ADD,					// 变倍+
    PTZ_ZOOM_DEC,					// 变倍-
    PTZ_FOCUS_ADD,					// 调焦+
    PTZ_FOCUS_DEC,					// 调焦-
    PTZ_APERTURE_ADD,				// 光圈+
    PTZ_APERTURE_DEC,				// 光圈-
    PTZ_POINT_MOVE,					// 转至预置点
    PTZ_POINT_SET,					// 设置预置点
    PTZ_LAMP_OPEN,					// 打开灯光
    PTZ_LAMP_CLOSE,					// 关闭灯光
    PTZ_WIPER_OPEN,					// 打开雨刷
    PTZ_WIPER_CLOSE,				// 关闭雨刷
    PTZ_THROUGHFOG_OPEN,			// 打开透雾
    PTZ_THROUGHFOG_CLOSE,			// 关闭透雾
    PTZ_DEV_OPEN,					// 打开辅助设备
    PTZ_DEV_CLOSE,					// 关闭辅助设备
    PTZ_LEFTUP,						// 左上
    PTZ_RIGHTUP,					// 右上
    PTZ_LEFTDOWN,					// 左下
    PTZ_RIGHTDOWN,					// 右下
	PTZ_MAX_COMMAND					// 最大值
};

