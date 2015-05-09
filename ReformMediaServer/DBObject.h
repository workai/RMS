#pragma once
#include "ReformMediaServerDef.H"
#include "StdHeader.h"
#include "RFProtocol.h"

// 数据库管理类
class CDBObject
{
public:
	CDBObject(void);
	~CDBObject(void);

public:
	void Init();
	void CheckDataBase();

	// 设备管理
	BOOL UpdateDevice(list<string> sqlList);
	BOOL UpdateData(LPDeviceUnitData lpDeviceData);
	void GetDeviceList(MapRFCamera& mapRfCamera);

	// 录像检索、记录入库
	BOOL QueryRecord( MapQueryRecordResult& mapRecord, string time = "", string timeEnd = "", string strkey = "");		// 按时间、名称检索
	int InsertReocord(const char* pProvider, const char* pDVRName, string pId,	int pChannelId,
		const char* pDVRAddr, int state, int type, const char* path, const char* file, const char* startTime, const char* endTime = NULL);
	BOOL UpdateRecord(int nId);
	BOOL DeleteRecord(int nId);

	int getFirstFile(char* name);
	BOOL isFileInRecord(const char* file);

private:
	void ClearData();
	void GetDevice();
	void GetVideoRecords(string time, string timeEnd, string strkey);

	char g_szFile[MAX_PATH];
private:

	MapRFCamera m_mapRFCamera;
	MapQueryRecordResult m_mapRecord;

	CRITICAL_SECTION m_CriticalSectionDevice;
	CRITICAL_SECTION m_CriticalSectionRecord;
};

extern CDBObject g_dbobj;