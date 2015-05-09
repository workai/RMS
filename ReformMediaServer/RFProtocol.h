#ifndef __RF_PROCOTOL_H__
#define __RF_PROCOTOL_H__


#define PACKET_MAX_SIZE				1024	// 单个数据包的最大长度

#define A_PROTOCOL_HEAD				0xABCD	// 协议头标志
#define REGISTER_COMMAND			0x9001	// 注册
#define RESP_REGISTER_COMMAND		0x9002	// 注册回应
#define UPDATE_DEVICE_COMMAND		0x9003	// 更新设备
#define RESP_UPDATE_DEVICE_COMMAND	0x9004	// 更新设备回应
#define UPDATE_CONFIG_COMMAND		0x9005	// 更新配置
#define RESP_UPDATE_CONFIG_COMMAND	0x9006	// 更新配置回应
#define HANDSHAKE_COMMAND			0x9007	// 握手
#define RESP_HANDSHAKE_COMMAND		0x9008	// 握手回应

#define DEVICE_UPDATE_COMMAND		0x9009	// 数据更新指令

#define DEVICE_REQUEST_COMMAND		0x9010	// 数据请求指令
#define RESP_DEVICE_REQUEST_COMMAND	0x9011	// 数据更新回应指令

#define VIDEO_RECORD_REQUEST_COMMAND		0x9012	// 数据请求指令
#define RESP_VIDEO_RECORD_REQUEST_COMMAND	0x9013	// 数据更新回应指令

#define VIDEO_RECORD_REQUEST_WITH_ICON_COMMAND		0x9014	// 获取附有图标的录像信息

// 应用层包头
#pragma pack(1)
typedef struct tagRFAppComand
{
	unsigned short  nPacketID;		// (0xAB,0xCD)
	unsigned short  nPacketNo;		// 包序号    
	unsigned short  nCommand;		// 指令
	unsigned int  nDataSize;		// 数据长度

	tagRFAppComand()
	{
		*((unsigned char *)&nPacketID)		= 0xAB;
		*((unsigned char *)&nPacketID + 1)	= 0xCD;
	}

	int GetPacketTotalSize()
	{
		return sizeof(tagRFAppComand) + nDataSize;
	}
} RFAppComand, *LpRFAppComand;
#pragma pack()

// 请求设备指令回应格式
typedef struct tagGetDeviceListCommandAck
{
	int			nTotalCount;	// 列表总数
	int			nCurrCount;		// 设备列表数量
} GetDeviceListCommandAck, *LpGetDeviceListCommandAck;

typedef struct tagQueryVideoReocrdCommand
{
	char		szStartTime[20];
	char		szEndTime[20];
	char		szKey[20];
} QueryVideoReocrdCommand, *LpQueryVideoReocrdCommand ;

// 请求录像列表指令回应格式
typedef struct tagQueryVideoRecordCommandAck
{
	int			nTotalCount;	// 列表总数
	int			nCurrCount;		// 设备列表数量
} QueryVideoRecordCommandAck, *LpQueryVideoRecordCommandAck;

// 查询设备列表
typedef struct tagDeviceObject
{
	char	szRtspString[300];
	char	szDVRName[20];
	char	szChannelName[20];
	char	szInnerName[20];
	char	szProvider[20];

} DeviceObject, *LpDeviceObject;

// 查询录像文件列表
typedef struct tagQueryVideoRecord
{
	// 编号，录像文件名称，完全路径，开始时间，结束时间，dvr名称，dvr的ip地址， 通道号，摄像头id， 文件大小
	char		szFileName[50];
	char		szPath[200];
	char		szStartTime[20];
	char		szEndTime[20];
	char		szDVRName[20];
	char		szDVRAddr[20];
	char		szChannelName[20];
	int			nDeviceId;
	int			Id;
	int			nFileSize;
} QueryVideoRecord, *LpQueryVideoRecord;

typedef struct _QuerayVRResource
{
	char file_name[50];
	char url[200];
	char start_time[20];
	char end_time[20];
	int file_size;
}QueryVRResource, *LPQueryVRResource;

typedef struct _QuerayVRResourceWithIcon
{
	char file_name[50];
	char url[200];
	char start_time[20];
	char end_time[20];
	int file_size;
	int icon_size;
	char* icon_data;
}QueryVRResourceWithIcon, *LPQueryVRResourceWithIcon;

// 通用回应指令格式
typedef struct _CommonPackAck
{
	unsigned short nResult;	// 结果 0 成功 其他见错误码
} CommonPackAck, * LPCommonPackAck;

// 注册包
typedef struct _Register
{
	unsigned short nDeviceType;		// 设备类型
	unsigned int   nDeviceAddr;		// 设备地址
	unsigned int   nSoftVer;		// 软件版本
	char szUser[20];
	char szPass[20];
} Register, * LPRegister;

// 注册包回应
typedef struct _RegisterAck
{
	unsigned short nResult;			// 注册结果
	unsigned int   nDeviceAddr;		// 分配的设备地址
} RegisterAck, * LPRegisterAck;

// 握手回应
typedef struct _HandShakeAck
{
	unsigned short nStatus;	// 状态 0 正常 1 异常
} HandShakeAck, * LPHandShakeAck;

// 数据更新指令
typedef struct _DeviceUpdate
{
	int flag;
}DeviceUpdate, *LpDeviceUpdate;

// 设备数据包
typedef struct _DevicePack
{
	unsigned char byCount;		// 数量
} DevicePack, *LPDevicePack;

// 数据操作类型
typedef enum _UpdateType
{
	SELECT_DATA = 0,		// 读取
	INSERT_DATA = 1,		// 插入
	UPDATE_DATA = 2,		// 修改
	DELETE_DATA = 3			// 删除
} UpdateType;

// 应用程序类型
typedef enum _AppType
{
    APP_CENTER_SERVER,
    APP_DEVICE_SERVER,
    APP_RECORD_SERVER,
    APP_STREAM_SERVER,
    APP_ALARM_SERVER,
    APP_CLIENT_CLIENT,
    APP_MANAGER_CLIENT,
    APP_MATRIX_CLIENT,
    APP_ACTIVEX_CLIENT,
} AppType;

// 设备数据
typedef struct _DeviceUnitData
{
	int			nId;					// 数据库对应的编号
	int			nUpdateType;			// 数据操作类型(插入=0、更新=1、删除=2,,,)
	char		szCameraId[20];			// 系统生成的摄像头设备编号
	char		szCameraType[20];		// 摄像头类型
	char		szCameraName[20];		// 摄像头名称
	char		szMode[20];				// 摄像头类型(DVR、NVR、IPC，，，)
	char		szProvider[20];			// 设备厂家(大华、海康、海思，，，)
	char		szCodec[20];			// 编码格式
	char		szLogin[20];			// 连接到设备时登录的用户名
	char		szPassword[20];			// 登录密码
	char		szIp[20];				// 设备的IP地址
	char		szPort[20];				// 设备连接端口号
	char		szCnlId[20];			// 设备连接通道号

	char		szMultiCast[20];		// 是否广播数据
	char		szMultiCastAddress[20];	// 广播地址
	char		szMultiCastPort[20];	// 广播端口
	char		szRecordType[20];		// 
} DeviceUnitData, *LPDeviceUnitData;

// 时间包
typedef struct _RecordTime
{
	unsigned short    nYear;
	unsigned char     nMonth;
	unsigned char     nDay;
	unsigned char     nSer;
	unsigned char     nHour;
	unsigned char     nMinute;
	unsigned char     nSecond;
} RecordTime, * LPRecordTime;

// 录像在数据库中的字段
typedef struct _RecordData
{
	int		nId;					// 数据库中的编号
	int		nDeviceId;				// 设备编号
	char	szDVRName[20];			// DVR名称
	char	szChannelName[20];		// 通道号
	char	szInnerName[20];		// 内部名称
	char	szIp[20];				// 设备的IP地址
	char	szFileName[50];			// 文件名字
	char	szPath[256];			// 文件完整路径
	int		nRecordType;			// 录像类型(告警、日常、)
	int		nState;					// 录像文件状态(录像中、录完、有错误)
	RecordTime	tStart;				// 录像开始时间
	RecordTime	tEnd;				// 录像结束时间	

} RecordData, *LpRecordData;

#endif