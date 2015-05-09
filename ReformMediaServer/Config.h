#pragma once

#include "RFProtocol.h"
#include "ReformMediaServerDef.H"
#include "xml\tinyxml.h"
#include <string>

#define RECORD_SETTINGS_FILE_NAME		"RecordCfg.xml"
#define CAMERA_SETTINGS_FILE_NAME		"CameraCfg.xml"
#define PTZCONTROL_SETTINGS_FILE_NAME	"PtzControlCfg.xml"

using namespace std;

// 配置类
class CConfig
{
public:
	CConfig(void);
	~CConfig(void);

	bool Init();											// 初始化，加载配置数据
	bool Save();											// 保存数据到文件

	bool get_record_status(string pId);
	bool get_record_path(string pId, char * path);
	bool get_record_path(int size, string pId, char * path);
	// 录像处理
	bool AddRecord(LPRFRecordSetting lpRecord);					// 插入录像策略
	bool UpdateRecord(LPRFRecordSetting lpRecord);					// 更新录像策略
	bool DeleteRecord(LPRFRecordSetting lpRecord);					// 删除录像策略
	bool QueryRecords(MapRecordSettings & recordsettings);	// 获取所有策略	
	bool GetRecordSetting(string pId, char * path);			// 查询某个通道是否录像和存储路径

	bool SetRecordState(bool bRecord, string strFileName);	// 修改channel中的录像状态位，以控制录像过程,并且更新数据库中的录像日志
	bool UpdateRecordState(int recordId, string strFile);	// 更新录像状态
	bool QueryRecordByName(string strQueryName, MapQueryRecordResult &queryResult);		// 查询录像返回的结果集
	bool IsRecordFinished(string strRecordName);			// 查询指定名字的录像是否已经结束，只有结束的录像才能查看，下载等

	// 摄像头处理
	bool AddCamera(	LPRFCamera lpCamera);			// 添加摄像头(更新内存、保存到数据库、通知所有客户端重新加载)
	bool UpdateCamera(LPRFCamera lpCamera);			// 更新摄像头信息(更新内存、保存到数据库、通知所有客户端重新加载)
	bool DeleteCamera(LPRFCamera lpCamera);			// 删除摄像头(更新内存、保存到数据库、通知所有客户端重新加载)
	bool QueryCameras(MapRFCamera &cameras);		// 查询所有摄像头
	
//private:
public:

	// 更新录像状态列表线程
	static DWORD WINAPI RecordSettingThread( LPVOID lpParameter);

	bool LoadCameras();								// 加载摄像头对象列表
	bool LoadRecordSettings();						// 加载录像策略
	bool LoadPTZControlRuler();						// 加载云台控制策略
	bool SaveRecordSettings();						// 保存录像策略
	bool SaveCameras();								// 保存设备列表
	bool SavePTZControlRuler();						// 保存云台控制策略

	bool GetRecordType(RF_RECORD_TYPE type, string& strType);													// 获取录像类型字符串
	bool GetRecordTime( string strStart,  string strEnd, RFRecordTime& value );									// 获取时间值
	bool GetChildText( TiXmlElement* pElement ,string strElementName, int& value );								// 获取子节点整形值
	bool GetChildText( TiXmlElement* pElement ,string strElementName, string& value );							// 获取子节点文本值
	bool GetChildAttribute( TiXmlElement* pElement ,string strElementName, string strAttribute, int& value );	// 获取子节点属性值
	bool GetChildAttribute( TiXmlElement* pElement ,string strElementName, string strAttribute, string& value );// 获取子节点属性值
	bool GetRecordType( TiXmlElement* pElement ,string strElementName, RF_RECORD_TYPE& value );					// 获取录像类型

	string getRecordPaths()
	{
		return m_RecordPath;
	}

private:

	bool					m_bExit;				// 是否退出程序
	HANDLE					m_hExitEvent;			// 退出事件
	HANDLE					m_hSettingChangeEvent;	// 配置更改事件
	bool					m_bRecordLog;			// 是否记录录像日志
	bool					m_bCameraLog;			// 是否记录设备操作日志

	// 摄像头处理
	MapRFCamera				m_mapCameras;	// 摄像头列表

	// 录像处理
	string					m_strFile;				// 录像配置文件路径
	MapRecordSettings		m_mapRecordSettings;	// 所有录像配置

	int						m_nMaxId;				// 最大的id编号
	string m_RecordPath;
};

extern CConfig g_xmlConfig;