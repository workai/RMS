#include "Config.h"

#include "Log.h"
#include "strutil.h"
#include <direct.h>
#include <io.h>

CConfig g_xmlConfig;

CConfig::CConfig(void)
{
	m_bExit			= false;// 是否退出程序
	m_bRecordLog	= true;	// 是否记录录像日志
	m_bCameraLog	= true;	// 是否记录设备操作日志
	m_strFile		= "";	// 录像配置文件路径

	m_hExitEvent	= NULL;
	m_nMaxId		= 1;
}

CConfig::~CConfig(void)
{
	SetEvent(m_hExitEvent);
	Sleep(0);

	if(m_hExitEvent)
	{
		CloseHandle(m_hExitEvent);
		m_hExitEvent = NULL;
	}	
}

DWORD WINAPI CConfig::RecordSettingThread( LPVOID lpParameter)
{
	//CConfig* pThis = reinterpret_cast<CConfig*>(lpParameter);

	//try
	//{
	//	while(!pThis->m_bExit)
	//	{
	//		if(pThis == NULL)
	//			return 1;

	//	}
	//}
	//catch(...)
	//{

	//}

	return 1;
}

// 加载摄像头对象列表
bool CConfig::LoadCameras()
{
	string fullPath = GetAppPath() + "\\" +CAMERA_SETTINGS_FILE_NAME;
	TiXmlDocument doc(fullPath.c_str());		// CameraCfg.xml
	bool loadOkay = doc.LoadFile();
	if( !loadOkay )
	{
		g_pLog->WriteLog("load device settings failed！\n");
		return false;
	}
	
	// 根节点
	TiXmlElement* pRootElement = doc.RootElement();
	string strText = pRootElement->Value();

	// records节点
	TiXmlElement* pDomainsElement = pRootElement->FirstChildElement();
	strText = pDomainsElement->Value();

	TiXmlElement* pDomainElement = pDomainsElement->FirstChildElement();
	strText = pDomainElement->Value();

	GetChildText(pDomainElement,"RecordPath",m_RecordPath);

	TiXmlElement* pCamerasElement = pDomainElement->FirstChildElement("Cameras");
	strText = pCamerasElement->Value();

	bool bRet = false;
	LPRFCamera lpCamera = NULL;

	// 遍历record节点
	TiXmlElement* pCameraElement = pCamerasElement->FirstChildElement();
	for(pCameraElement; pCameraElement; pCameraElement = pCameraElement->NextSiblingElement())
	{
		lpCamera = new RFCamera();

		if( !GetChildText( pCameraElement, "strCameraID"		, lpCamera->strCameraId			)	||
			!GetChildText( pCameraElement, "eCameraType"		, lpCamera->strCameraType		)	||
			!GetChildText( pCameraElement, "strName"			, lpCamera->strCameraName		)	|| 
			!GetChildText( pCameraElement, "strModel"			, lpCamera->strMode				)	||
			!GetChildText( pCameraElement, "strProvider"		, lpCamera->strProvider			)	||
			!GetChildText( pCameraElement, "strCodec"			, lpCamera->strCodec			)	||
			!GetChildText( pCameraElement, "strLogin"			, lpCamera->strLogin			)	||
			!GetChildText( pCameraElement, "strPassword"		, lpCamera->strPassword			)	||
			!GetChildText( pCameraElement, "strIP"				, lpCamera->strIp				)	||
			!GetChildText( pCameraElement, "strPort"			, lpCamera->strPort				)	||
			!GetChildText( pCameraElement, "strCnlID"			, lpCamera->strCnlId			)	||
			!GetChildText( pCameraElement, "boolMultiCast"		, lpCamera->strMultiCast		)	||
			!GetChildText( pCameraElement, "strMultiCastAddress", lpCamera->strMultiCastAddress )	||
			!GetChildText( pCameraElement, "strMultiCastPort"	, lpCamera->strMultiCastPort	)	||
			!GetChildText( pCameraElement, "eRecordType"		, lpCamera->strRecordType		) )
		{
			delete lpCamera;
			return false;
		}

		IterRFCamera iter = m_mapCameras.find(	atoi( lpCamera->strCameraId.c_str() ) );
		if( iter != m_mapCameras.end() )
		{
			iter->second = lpCamera;
			//::MessageBox( NULL, "录像策略所在的配置文件设置有误,重复的recordid !将被重复的配置覆盖", "配置文件设置错误", MB_OK );
		}
		else
		{
			m_mapCameras.insert( ValuetRFCamera( atoi( lpCamera->strCameraId.c_str() ), lpCamera ) );
		}
	}

	return true;
}

// 加载摄像头对象列表
bool CConfig::LoadPTZControlRuler()
{
	return true;
}

// 加载录像策略
bool CConfig::LoadRecordSettings()
{
	string fullPath = GetAppPath() + "\\" +RECORD_SETTINGS_FILE_NAME;
	TiXmlDocument doc(fullPath.c_str());		// CameraCfg.xml
	bool loadOkay = doc.LoadFile();
	if( !loadOkay )
	{
		g_pLog->WriteLog("load record settings failed！\n");
		return false;
	}
	
	// 根节点
	TiXmlElement* pRootElement = doc.RootElement();
	string strText = pRootElement->Value();

	// records节点
	TiXmlElement* pRecordsElement = pRootElement->FirstChildElement();
	strText = pRecordsElement->Value();

	// 是否记录日志
	bool bWriteLog = false;
	pRecordsElement->QueryBoolAttribute("WriteLog", &bWriteLog);

	bool bRet = false;
	LPRFRecordSetting lpRecord = NULL;
	TiXmlElement* pRecordElement = pRecordsElement->FirstChildElement();

	// 遍历record节点
	for(pRecordElement; pRecordElement; pRecordElement = pRecordElement->NextSiblingElement())
	{
		lpRecord = new RFRecordSetting();

		if( !GetChildText	( pRecordElement, "RecordId"			, lpRecord->nRecordId				) ||
			!GetChildText	( pRecordElement, "strCameraId"			, lpRecord->strCameraId				) ||
			!GetRecordType	( pRecordElement, "RecordType"			, lpRecord->nRecordType				) ||
			!GetChildText	( pRecordElement, "RecordDate"			, lpRecord->nRecordMask				) ||
			!GetChildText	( pRecordElement, "RecordPath"			, lpRecord->strRecordPath			) ||
			!GetChildText	( pRecordElement, "ChangeFileReasion"	, lpRecord->strChangeFileReasion	) ||
			!GetChildText	( pRecordElement, "ChangeFileValue"		, lpRecord->nChangeFileValue		) ||
			!GetChildText	( pRecordElement, "DiskFullError"		, lpRecord->strDiskFullDo			) )
		{
			delete lpRecord;
			return false;
		}

		TiXmlElement* pTimeElement = pRecordElement->FirstChildElement("RecordTime")->FirstChildElement("time");
		for( pTimeElement; pTimeElement; pTimeElement = pTimeElement->NextSiblingElement() )
		{
			RFRecordTime recordTime;
			string strStart = pTimeElement->Attribute("start");
			string strStop = pTimeElement->Attribute("stop");

			if( !GetRecordTime( strStart, strStop,  recordTime) )
			{
				delete lpRecord;
				return false;
			}

			lpRecord->listRecordTime.push_back(recordTime);
		}

		IterMapRecordSettings iter = m_mapRecordSettings.find( lpRecord->strCameraId );
		if( iter != m_mapRecordSettings.end() )
		{
			iter->second = lpRecord;
			//::MessageBox( NULL, "录像策略所在的配置文件设置有误,重复的recordid !将被重复的配置覆盖", "配置文件设置错误", MB_OK );
		}
		else
		{
			m_mapRecordSettings.insert(ValueMapRecordSettings(lpRecord->strCameraId, lpRecord));
		}
	}

	return true;
}

// 保存录像策略
bool CConfig::SaveRecordSettings()
{
	if(m_mapRecordSettings.empty())
		return true;

	TiXmlDocument doc;

	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "gb2312", "" );
    doc.LinkEndChild( decl );

	TiXmlElement * pElementRfServ = new TiXmlElement( "rfServ" ); // 创建元素element1

	TiXmlElement * pElementRecords = new TiXmlElement( "Records" );//创建元素element1
	pElementRecords->SetAttribute( "WriteLog", m_bRecordLog ? "true" : "false" );

	char szIndex[10] = {0};
	IterMapRecordSettings iterRecord;

	for( iterRecord = m_mapRecordSettings.begin(); iterRecord != m_mapRecordSettings.end(); iterRecord++ )
	{
		TiXmlElement * pElementRecord = new TiXmlElement( "Record" );
		pElementRecord->SetAttribute( "desic", "同一个摄像头不支持多条记录，单条记录可以设多个时间段" );
		pElementRecords->LinkEndChild( pElementRecord );

		sprintf_s( szIndex, "%d", iterRecord->second->nRecordId );
		TiXmlElement * pElementRecordId = new TiXmlElement( "RecordId" );
		// pElementRecordId->SetValue( szIndex );
		//TiXmlText * pText = new TiXmlText( szIndex );
		pElementRecordId->LinkEndChild( new TiXmlText( szIndex ) );
		pElementRecord->LinkEndChild( pElementRecordId );

		TiXmlElement * pElementCameraId = new TiXmlElement( "strCameraId" );
		//pElementCameraId->SetValue( iterRecord->second->strCameraId.c_str() );
		pElementCameraId->LinkEndChild( new TiXmlText( iterRecord->second->strCameraId.c_str() ) );
		pElementRecord->LinkEndChild( pElementCameraId );

		string strRecordType = "recordForbit";
		GetRecordType( iterRecord->second->nRecordType, strRecordType);

		TiXmlElement * pElementRecordType = new TiXmlElement( "RecordType" );
		//pElementRecordType->SetValue( strRecordType.c_str() );
		pElementRecordType->SetAttribute( "desic", "类型：recordForbit, everyMonth、everyWeek、everyDay, recordAways" );
		pElementRecordType->LinkEndChild( new TiXmlText( strRecordType.c_str() ) );
		pElementRecord->LinkEndChild( pElementRecordType );

		sprintf_s( szIndex, "%d", iterRecord->second->nRecordMask );
		TiXmlElement * pElementRecordDate = new TiXmlElement( "RecordDate" );
		pElementRecordDate->SetAttribute( "desic", "录像日期根据录像类型而定" );
		//pElementRecordDate->SetValue( szIndex );
		pElementRecordDate->LinkEndChild( new TiXmlText( szIndex ) );
		pElementRecord->LinkEndChild( pElementRecordDate );
		
		// RecordTime 
		TiXmlElement * pElementRecordTime = new TiXmlElement( "RecordTime" );
		pElementRecordTime->SetAttribute( "desic", "录像时间可以分多个时间段!" );
		
		char szTime[20] = {0};
		list<RFRecordTime>::iterator iterTime;
		for( iterTime = iterRecord->second->listRecordTime.begin();
			iterTime != iterRecord->second->listRecordTime.end();
			iterTime++ )
		{
			TiXmlElement * pElementTime = new TiXmlElement( "time" );
			
			// 开始时间
			sprintf_s( szTime, "%.2d:%.2d:%.2d", iterTime->tStart.Hour, iterTime->tStart.Minute, iterTime->tStart.Second);
			pElementTime->SetAttribute( "start", szTime);

			// 结束时间
			sprintf_s( szTime, "%.2d:%.2d:%.2d", iterTime->tStop.Hour, iterTime->tStop.Minute, iterTime->tStop.Second);
			pElementTime->SetAttribute( "stop", szTime);
			
			pElementRecordTime->LinkEndChild(pElementTime);
		}

		pElementRecord->LinkEndChild( pElementRecordTime );

		TiXmlElement * pElementRecordPath = new TiXmlElement( "RecordPath" );
		//pElementRecordPath->SetValue( iterRecord->second->strRecordPath.c_str() );
		pElementRecordPath->LinkEndChild( new TiXmlText( iterRecord->second->strRecordPath.c_str() ) );
		pElementRecord->LinkEndChild( pElementRecordPath );
		
		TiXmlElement * pElementChangeFileReasion = new TiXmlElement( "ChangeFileReasion" );
		//pElementChangeFileReasion->SetValue( iterRecord->second->strChangeFileReasion.c_str() );
		pElementChangeFileReasion->LinkEndChild( new TiXmlText( iterRecord->second->strChangeFileReasion.c_str() ) );
		pElementRecord->LinkEndChild( pElementChangeFileReasion );
		
		char szValue[20] = {0};
		sprintf_s( szValue, "%d", iterRecord->second->nChangeFileValue);
		TiXmlElement * pElementChangeFileValue = new TiXmlElement( "ChangeFileValue" );
		//pElementChangeFileValue->SetValue( szValue );
		pElementChangeFileValue->LinkEndChild( new TiXmlText( szValue ) );
		pElementRecord->LinkEndChild( pElementChangeFileValue );
		
		TiXmlElement * pElementDiskFullError = new TiXmlElement( "DiskFullError" );
		//pElementDiskFullError->SetValue( iterRecord->second->strDiskFullDo.c_str() );
		pElementDiskFullError->LinkEndChild( new TiXmlText( iterRecord->second->strDiskFullDo.c_str() ) );
		pElementRecord->LinkEndChild( pElementDiskFullError );

	}
	
	pElementRfServ->LinkEndChild( pElementRecords );	
	doc.LinkEndChild( pElementRfServ );
    doc.SaveFile( RECORD_SETTINGS_FILE_NAME );

	return true;
}

// 获取录像类型
bool CConfig::GetRecordType(RF_RECORD_TYPE type, string& strType)
{
	switch(type)
	{
	case RECORD_FORBIT:
		strType = "recordForbit";
		break;
	case RECORD_BYMONTH:
		strType = "everyMonth";
		break;
	case RECORD_BYWEEK:
		strType = "everyWeek";
		break;
	case RECORD_EVERYDAY:
		strType = "everyDay";
		break;
	case RECORD_AWAYS:
		strType = "recordAways";
		break;
	default:
		strType = "recordForbit";
		break;
	}

	return true;
}
				
// 保存设备列表
bool CConfig::SaveCameras()
{
	return true;
}

// 保存云台控制策略
bool CConfig::SavePTZControlRuler()
{
	return true;
}

// 读取整形值
bool CConfig::GetChildText( TiXmlElement* pElement ,string strElementName, int& value )
{
	if(pElement == NULL)
		return false;

	TiXmlElement* pChildElement = pElement->FirstChildElement(strElementName.c_str());
	if(pChildElement == NULL)
		return false;

	value = atoi(pChildElement->GetText());

	return true;
}

// 获取时间值
bool CConfig::GetRecordTime( string strStart,  string strEnd, RFRecordTime& value )
{
	if(strStart.empty() || strEnd.empty())
		return false;

	vector<string> vecStart = strutil::split( strStart, ":");
	vector<string> vecEnd = strutil::split( strEnd, ":");

	if( vecStart.empty() || vecEnd.empty() || 3 != vecStart.size() || 3 != vecEnd.size() )
		return false;

	value.tStart.Hour		= atoi( ( (string)(vecStart[0]) ).c_str() );
	value.tStart.Minute		= atoi( ( (string)(vecStart[1]) ).c_str() );
	value.tStart.Second		= atoi( ( (string)(vecStart[2]) ).c_str() );

	value.tStop.Hour		= atoi( ( (string)(vecEnd[0]) ).c_str() );
	value.tStop.Minute		= atoi( ( (string)(vecEnd[1]) ).c_str() );
	value.tStop.Second		= atoi( ( (string)(vecEnd[2]) ).c_str() );
	
	return true;
}

// 读取字符串
bool CConfig::GetChildText( TiXmlElement* pElement ,string strElementName, string& value )
{
	if(pElement == NULL)
		return false;

	TiXmlElement* pChildElement = pElement->FirstChildElement(strElementName.c_str());
	if(pChildElement == NULL)
		return false;

	value = pChildElement->GetText();
	return true;
}

// 读取属性字符串
bool CConfig::GetChildAttribute( TiXmlElement* pElement , string strElementName, string strAttribute, string& value )
{
	if(pElement == NULL)
		return false;

	TiXmlElement* pChildElement = pElement->FirstChildElement(strElementName.c_str());
	if(pChildElement == NULL)
		return false;

	value = pChildElement->Attribute(strAttribute.c_str());
	return true;
}

// 读取属性字符串
bool CConfig::GetChildAttribute( TiXmlElement* pElement , string strElementName, string strAttribute, int& value )
{
	if(pElement == NULL)
		return false;

	TiXmlElement* pChildElement = pElement->FirstChildElement(strElementName.c_str());
	if(pChildElement == NULL)
		return false;

	pChildElement->QueryIntAttribute(strAttribute.c_str(), &value);
	return true;
}

// 获取录像存储类型
bool CConfig::GetRecordType( TiXmlElement* pElement , string strElementName, RF_RECORD_TYPE& value )
{
	string strRecordType = "";
	if( !GetChildText( pElement, strElementName, strRecordType ) )
		return false;

	if(strutil::equalsIgnoreCase(strRecordType, "everyMonth"))
	{
		value = RECORD_BYMONTH;
	}
	else if(strutil::equalsIgnoreCase(strRecordType, "everyWeek"))
	{
		value = RECORD_BYWEEK;
	}
	else if(strutil::equalsIgnoreCase(strRecordType, "everyDay"))
	{
		value = RECORD_EVERYDAY;
	}
	else if(strutil::equalsIgnoreCase(strRecordType, "recordAways"))
	{
		value = RECORD_AWAYS;
	}
	else
	{
		value = RECORD_FORBIT;
	}

	return true;
}

// 初始化，加载配置数据
bool CConfig::Init()
{
	// m_strFile = strFile;

	// 加载摄像头设备列表
	if( !LoadCameras() )
		return false;

	// 加载录像策略配置
	if( !LoadRecordSettings() )
		return false;

	// 加载云台控制策略配置
	if( !LoadPTZControlRuler() )
		return false;

	// 启动服务线程
	m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	DWORD dw;
	CreateThread( NULL, 0, LPTHREAD_START_ROUTINE(RecordSettingThread), this, 0, &dw);

	return true;
}

// 保存数据到文件
bool CConfig::Save()
{
	return true;
}

// 录像处理 // 插入录像策略
bool CConfig::AddRecord(LPRFRecordSetting lpRecord)
{
	return true;
}

// 更新录像策略
bool CConfig::UpdateRecord(LPRFRecordSetting lpRecord)
{
	return true;
}

// 删除录像策略
bool CConfig::DeleteRecord(LPRFRecordSetting lpRecord)
{
	return true;
}

// 获取所有策略
bool CConfig::QueryRecords(MapRecordSettings & recordsettings)
{
	return true;
}

// 查询某个通道是否录像和存储路径
bool CConfig::GetRecordSetting(string pId, char * path)
{
	//if( m_mapRecordSettings.empty() )
	//	return false;

	IterMapRecordSettings iter = m_mapRecordSettings.find( pId );
	if( iter != m_mapRecordSettings.end() )
	{
		LPRFRecordSetting precord = iter->second;
		//判断是否在录像时间段以内
		SYSTEMTIME loctime;
		GetLocalTime(&loctime);
		list<RFRecordTime>::iterator iter_period=precord->listRecordTime.begin();
		for(;iter_period!= precord->listRecordTime.end();iter_period++)
		{
			RFRecordTime period = *iter_period;
			if((period.tStart.Hour < loctime.wHour && period.tStart.Minute < loctime.wMinute && period.tStart.Second < loctime.wSecond )&&(period.tStop.Hour > loctime.wHour && period.tStop.Minute > loctime.wMinute && period.tStop.Second > loctime.wSecond))
				break;
		}
		if(iter_period == precord->listRecordTime.end())
			return false;

		if( !iter->second->strRecordPath.empty() )
		{
			//遍历所有目录
			char apath[1024] = {};
			strcpy(apath, iter->second->strRecordPath.c_str());
			char* epath = strtok(apath,";");
			while(epath!=NULL)
			{
				//判断目录可写
				if(_access(epath, 0) ==-1)
					_mkdir(epath);
				//判断存储空间
				ULARGE_INTEGER avaliblespace;
				memset(&avaliblespace, 0, sizeof(avaliblespace));
				DWORD err;
				if(GetDiskFreeSpaceEx(epath,&avaliblespace,0,0) == 0)
					err= GetLastError();
				if(avaliblespace.QuadPart > 1073741824)
				{
					//返回存储目录
					strcpy(path, epath);
					return true;
				}
				epath = strtok(NULL,";");
			}
		}
	}

	return false;
}

// 修改channel中的录像状态位，以控制录像过程,并且更新数据库中的录像日志
bool CConfig::SetRecordState(bool bRecord, string strFileName)
{
	return true;
}

// 更新录像状态
bool CConfig::UpdateRecordState(int recordId, string strFile)
{
	return true;
}

// 查询录像返回的结果集
bool CConfig::QueryRecordByName(string strQueryName, MapQueryRecordResult &queryResult)
{
	return true;
}

// 查询指定名字的录像是否已经结束，只有结束的录像才能查看，下载等
bool CConfig::IsRecordFinished(string strRecordName)
{
	return true;
}

// 摄像头处理
// 添加摄像头(更新内存、保存到数据库、通知所有客户端重新加载)
bool CConfig::AddCamera( LPRFCamera lpCamera )
{
	return true;
}

// 更新摄像头信息(更新内存、保存到数据库、通知所有客户端重新加载)
bool CConfig::UpdateCamera( LPRFCamera lpCamera )
{
	return true;
}

// 删除摄像头(更新内存、保存到数据库、通知所有客户端重新加载)
bool CConfig::DeleteCamera(LPRFCamera lpCamera)
{
	return true;
}

// 查询所有摄像头
bool CConfig::QueryCameras(MapRFCamera &cameras)
{
	if(m_mapCameras.empty())
		return false;

	cameras = m_mapCameras;
	return true;
}

bool CConfig::get_record_status(string pId)
{
	IterMapRecordSettings iter = m_mapRecordSettings.find( pId );
	if( iter != m_mapRecordSettings.end() )
	{
		LPRFRecordSetting precord = iter->second;
		//判断是否在录像时间段以内
		SYSTEMTIME loctime;
		GetLocalTime(&loctime);
		int now_time = loctime.wHour*3600+ loctime.wMinute*60+loctime.wSecond;
		list<RFRecordTime>::iterator iter_period=precord->listRecordTime.begin();
		for(;iter_period!= precord->listRecordTime.end();iter_period++)
		{
			RFRecordTime period = *iter_period;
			int start_time = period.tStart.Hour*3600 + period.tStart.Minute*60 + period.tStart.Second;
			int stop_time = period.tStop.Hour*3600 + period.tStop.Minute*60 + period.tStop.Second;
			if(now_time>= start_time && now_time<= stop_time)
				break;
		}
		if(iter_period != precord->listRecordTime.end())
			return true;
	}
	return false;
}

bool CConfig::get_record_path(string pId, char * path)
{
	char apath[1024] = {};
	if(!m_RecordPath.empty())
	{
		strcpy(apath, m_RecordPath.c_str());
	}
	else
	{
		IterMapRecordSettings iter = m_mapRecordSettings.find( pId );
		if( iter != m_mapRecordSettings.end() )
		{
			if( !iter->second->strRecordPath.empty() )
			{
				//遍历所有目录
				strcpy(apath, iter->second->strRecordPath.c_str());
			}
		}
	}
	char* epath = strtok(apath,";");
	while(epath!=NULL)
	{
		//判断目录可写
		if(_access(epath, 0) ==-1)
			_mkdir(epath);
		//判断存储空间
		ULARGE_INTEGER avaliblespace;
		memset(&avaliblespace, 0, sizeof(avaliblespace));
		DWORD err;
		if(GetDiskFreeSpaceEx(epath,&avaliblespace,0,0) == 0)
			err= GetLastError();
		if(avaliblespace.QuadPart > 1073741824*20)
		{
			//返回存储目录
			strcpy(path, epath);
			return true;
		}
		g_pLog->WriteLog("ERROR: 路径%s磁盘空间(%I64u)不足\n",epath,avaliblespace.QuadPart);
		epath = strtok(NULL,";");
	}

	return false;
}

bool CConfig::get_record_path(int size, string pId, char * path)
{
	char apath[1024] = {};
	if(!m_RecordPath.empty())
	{
		strcpy(apath, m_RecordPath.c_str());
	}
	else
	{
		return true;
	}
	char* epath = strtok(apath,";");
	while(epath!=NULL)
	{
		//判断目录可写
		if(_access(epath, 0) ==-1)
			_mkdir(epath);
		//判断存储空间
		ULARGE_INTEGER avaliblespace;
		memset(&avaliblespace, 0, sizeof(avaliblespace));
		DWORD err;
		if(GetDiskFreeSpaceEx(epath,&avaliblespace,0,0) == 0)
			err= GetLastError();
		if(avaliblespace.QuadPart/1073741824 > size)
		{
			//返回存储目录
			//g_pLog->WriteLog("ERROR: 路径%s磁盘空间(%I64u, %d)\n",epath,avaliblespace.QuadPart/1073741824, size);
			strcpy(path, epath);
			return true;
		}
		g_pLog->WriteLog("ERROR: 路径%s磁盘空间(%I64u, %d)不足\n",epath,avaliblespace.QuadPart/1073741824, size);
		epath = strtok(NULL,";");
	}

	return false;
}

