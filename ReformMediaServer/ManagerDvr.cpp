// ManagerDvr.cpp: implementation of the CManagerDVR class.
//
//////////////////////////////////////////////////////////////////////

#include "StdHeader.h"
#include "ManagerDvr.h"
//#include "hikvideodef.h"
#include "SysInfo.h"
#include "DVR.h"
#include "channel.h"
#include "Packet.h"
#include "ReformMediaServerDef.H"


//#include "HCstruct.h"
//#include "PlayMpeg4.h"
//#include "DBWork.h"

#include "dhnetsdk.h"
#include "HCNetSDK.h"

#include "DaHuaDVR.h"
#include "HikNetDVR.h"
#include "Log.h"
#include "CBufferCmd.h"
#include "CGetPicture.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

void CALLBACK DisConnectCallBack(LONG lLoginID, char *pchDVRIP, LONG nDVRPort, DWORD dwUser)
{
	int i = 0;
	//for(; i < MAX_SERVER_SIZE; i ++)
	//{
	//	if(g_pManagerDVR->m_pDVRArray[i]!=NULL)
	//	{

	//		if((strcmp(pchDVRIP, g_pManagerDVR->m_pDVRArray[i]->m_DVRIP) == 0)&&(g_pManagerDVR->m_pDVRArray[i]->m_DVRPort == nDVRPort))
	//		{
	//			g_pManagerDVR->m_pDVRArray[i]->m_work_status = 0;
	//			g_pManagerDVR->m_pDVRArray[i]->m_disconnent_time = time(NULL);
	//			for(int k=0;k<MAX_CHANNEL_SIZE;k++)
	//			{
	//				if( g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]!=NULL)
	//				{
	//					g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]->m_work_status = 0;
	//					g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]->m_disconnect_time = g_pManagerDVR->m_pDVRArray[i]->m_disconnent_time;
	//				}
	//			}
	//			g_pLog->WriteLog("大华连接断开,注销(%s,%d)\n", pchDVRIP, nDVRPort);
	//			break;
	//		}
	//	}
	//}
	//if(i == MAX_SERVER_SIZE)
	//	g_pLog->WriteLog("大华连接断开,注销失败(%s,%d)\n", pchDVRIP, nDVRPort);
	g_pLog->WriteLog("视频%s:%d连接断开,大华\n", pchDVRIP, nDVRPort);
	return;
	//g_pLog->WriteLog("无法初始化大华SDK");
}
void CALLBACK AutoConnectFunc(LONG lLoginID,char *pchDVRIP,LONG nDVRPort,DWORD dwUser)
{
	int i = 0;
	//for(; i < MAX_SERVER_SIZE; i ++)
	//{
	//	if(g_pManagerDVR->m_pDVRArray[i]!=NULL)
	//	{

	//		if((strcmp(pchDVRIP, g_pManagerDVR->m_pDVRArray[i]->m_DVRIP) == 0)&&(g_pManagerDVR->m_pDVRArray[i]->m_DVRPort == nDVRPort))
	//		{
	//			g_pManagerDVR->m_pDVRArray[i]->m_work_status = 1;
	//			for(int k=0;k<MAX_CHANNEL_SIZE;k++)
	//			{
	//				if( g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]!=NULL)
	//				{
	//					g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]->m_work_status = 1;
	//				}
	//			}
	//			g_pLog->WriteLog("大华重新连接成功(%s,%d)\n", pchDVRIP, nDVRPort);
	//			break;
	//		}
	//	}
	//}
	//if(i == MAX_SERVER_SIZE)
	//	g_pLog->WriteLog("大华重新连接成功,添加失败(%s,%d)\n", pchDVRIP, nDVRPort);
	g_pLog->WriteLog("视频%s:%d重新连接成功,大华\n", pchDVRIP, nDVRPort);
	return;
}

// 接收异常消息的回调函数的外部实现
void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	char tempbuf[256];
	ZeroMemory(tempbuf,256);

	switch(dwType) 
	{
	case EXCEPTION_AUDIOEXCHANGE:	// 语音对讲时网络异常
		sprintf(tempbuf,"语音对讲时网络异常!!!\n");
		g_pLog->WriteLog("%s",tempbuf);
		// TODO: 关闭语音对讲
		break;
	case EXCEPTION_ALARM:			// 报警上传时网络异常
		sprintf(tempbuf,"报警上传时网络异常!!!\n");
		g_pLog->WriteLog("%s",tempbuf);
		// TODO: 关闭报警上传
		break;
	case EXCEPTION_SERIAL:			// 透明通道传输时异常
		sprintf(tempbuf,"ID%d透明通道传输时网络异常!!!\n", lUserID);
		g_pLog->WriteLog("%s", tempbuf);
		// TODO: 关闭透明通道
		break;
	case EXCEPTION_PREVIEW:			// 网络预览时异常
		sprintf(tempbuf,"ID%d网络预览时网络异常!!!\n", lUserID);
		g_pLog->WriteLog("%s",tempbuf);	
		// TODO: 关闭网络预览
	case EXCEPTION_RECONNECT:		// 预览时重连
		{
			int i = 0;
			/*for(; i < MAX_SERVER_SIZE; i ++)
			{
				if(g_pManagerDVR->m_pDVRArray[i]!=NULL)
				{
					if(g_pManagerDVR->m_pDVRArray[i]->m_UserID == lUserID)
					{
						if(g_pManagerDVR->m_pDVRArray[i]->m_work_status == 1)
						{
							g_pManagerDVR->m_pDVRArray[i]->m_work_status = 0;
							g_pManagerDVR->m_pDVRArray[i]->m_disconnent_time = time(NULL);
							for(int k=0;k<MAX_CHANNEL_SIZE;k++)
							{
								if( g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]!=NULL)
								{
									g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]->m_work_status = 0;
									g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]->m_disconnect_time = g_pManagerDVR->m_pDVRArray[i]->m_disconnent_time;
								}
							}
							g_pLog->WriteLog("海康连接断开,注销(userid:%d)\n", lUserID);
							break;
						}
						else
						{
							g_pLog->WriteLog("WARN:未工作海康设备连接断开(userid:%d)\n", lUserID);
						}
					}
				}
			}
			if(i == MAX_SERVER_SIZE)
				g_pLog->WriteLog("海康连接断开,注销失败(userid:%d)\n", lUserID);*/
		}
		g_pLog->WriteLog("海康连接断开,(userid:%d)\n", lUserID);
		break;
	case PREVIEW_RECONNECTSUCCESS:
		{
			int i = 0;
			/*for(; i < MAX_SERVER_SIZE; i ++)
			{
				if(g_pManagerDVR->m_pDVRArray[i]!=NULL)
				{
					if(g_pManagerDVR->m_pDVRArray[i]->m_UserID == lUserID)
					{
						if(g_pManagerDVR->m_pDVRArray[i]->m_work_status == 1)
							g_pLog->WriteLog("WARN:未标记海康设备连接成功(userid:%d)\n", lUserID);
						g_pManagerDVR->m_pDVRArray[i]->m_work_status = 1;
						for(int k=0;k<MAX_CHANNEL_SIZE;k++)
						{
							if( g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]!=NULL)
							{
								g_pManagerDVR->m_pDVRArray[i]->m_pChannelArray[k]->m_work_status = 1;
							}
						}
						g_pLog->WriteLog("海康重新连接成功(userid:%d)\n", lUserID);
						break;
					}
				}
			}
			if(i == MAX_SERVER_SIZE)
				g_pLog->WriteLog("ERROR:海康重新连接成功,添加失败(userid:%d)\n", lUserID);*/
		}
		g_pLog->WriteLog("海康重新连接成功,(userid:%d)\n", lUserID);
		break;
	default:
		g_pLog->WriteLog("WARN:未处理异常, 海康IPC(userid:%d), %d\n", lUserID, dwType);
		break;
	}
}

CManagerDVR::CManagerDVR()
{
	// 初始化大华SDK
	BOOL B = CLIENT_Init(DisConnectCallBack, (DWORD)this);		// 初始化sdk，设置断线回调函数
	CLIENT_SetAutoReconnect(AutoConnectFunc, 0);				// 设置断线重连成功的回调函数。不调用此接口，sdk将不进行断线重连。
	CLIENT_SetConnectTime(8000, NULL);

	// 初始化海康SDK
	NET_DVR_Init();
	NET_DVR_SetConnectTime(8000);
	NET_DVR_SetReconnect();
	NET_DVR_SetExceptionCallBack_V30( WM_NULL, NULL, g_ExceptionCallBack, NULL );

	// 这里可以添加其他厂家的SDK初始化

	m_ServerNum = 0;
	for( int i = 0; i < MAX_SERVER_SIZE; i++ )
	{
		m_pDVRArray[i] = NULL;
	}

	for( int i = 0; i < MAX_SERVER_SIZE*MAX_CHANNEL_SIZE; i++ )
	{
		m_pChannelArray[i] = NULL;
	}
}

CManagerDVR::~CManagerDVR()
{
	int i = 0;
	//for( i = 0; i < MAX_SERVER_SIZE; i++ )
	//{
	//	if(m_pDVRArray[i]!=NULL)
	//	{
	//		if(m_pDVRArray[i]->m_type == HIK_DVRDH_CHANNEL )
	//		{
	//			delete (CDaHuaDVR *)m_pDVRArray[i];
	//			m_pDVRArray[i] = NULL;
	//		}
	//		else if(m_pDVRArray[i]->m_type == HIK_DVRHK_CHANNEL )
	//		{				
	//			//其他扩展
	//		}
	//		else
	//		{
	//			delete m_pDVRArray[i];
	//			m_pDVRArray[i] = NULL;
	//		}
	//	}
	//}

	// 大华
	CLIENT_Cleanup();

	// 海康
	NET_DVR_Cleanup();

	//这里可以添加其他厂家摄像头的登出方式
}

BOOL CManagerDVR::Init()
{
	CPacket inPacket;
	DOMElement* AccNode = NULL;
	DOMElement* AccNode1 = NULL;
	DOMElement* AccNode2 = NULL;

	if(inPacket.BuiltTree(g_pSysInfo->m_XMLFilePath) == -1)
	{
		g_pLog->WriteLog("无法打开摄像头配置文件!\n");
		return FALSE;
	}
	AccNode = inPacket.SearchElement("/所有摄像头");
	AccNode = inPacket.SearchElement("Domains");
	AccNode = inPacket.SearchElement("Domain");
	AccNode = inPacket.SearchElement("Cameras");
	AccNode = inPacket.SearchElement("Camera");
	if(!AccNode)
	{
		return FALSE;
	}
	inPacket.SetCurrentElement(AccNode);
	const char* const elements[]={"strCameraID","eCameraType","strName","strModel",
		"strProvider","strCodec","strLogin","strPassword",
		"strIP","strPort","strCnlID",
		"boolMultiCast","strMultiCastAddress","strMultiCastPort","eRecordType"};
	while(AccNode)
	{
		bool isadd=true;
		string records[15]={""};
		for(int i=0;i<15;i++)
		{
			AccNode1 = inPacket.SearchElement(elements[i]);	// 通道编号
			if(AccNode1)
			{
				records[i] = AccNode1->getTextContent();
			}
			else
			{
				isadd=false;
				break;
			}
		}
		if(isadd)
		{
			CAMERA_INFO info = {""};
			info.strCameraID=records[0];
			info.eCameraType=records[1];
			info.strName=records[2];
			info.strModel=records[3];
			info.strProvider=records[4];
			info.strCodec=records[5];
			info.strLogin=records[6];
			info.strPassword=records[7];
			info.strIP=records[8];
			info.strPort=records[9];
			info.strCnlID=records[10];
			info.boolMultiCast=records[11];
			info.strMultiCastAddress=records[12];
			info.strMultiCastPort=records[13];
			info.eRecordType=records[14];

			m_CameraList[info.strCameraID]=info;
		}
		AccNode = AccNode->GetNextElement();
		if(AccNode)
		{
			inPacket.SetCurrentElement(AccNode);
		}
		else
		{
			break;
		}
	}  

	// 登录dvr
	for(map<string, CAMERA_INFO>::iterator iter = m_CameraList.begin();iter!=m_CameraList.end();iter++)
	{
		if(strlen(iter->second.strIP.c_str()) > 16)
		{
			g_pLog->WriteLog("IP格式不正确,%s\n", iter->second.strIP.c_str());
		}
		else
		{
			string key_dvr=iter->second.strIP+":"+iter->second.strPort;
			map<string, CDVR*>::iterator ptdvr=m_DVRList.find(key_dvr);
			if(ptdvr==m_DVRList.end())
			{
				CDVR *pDVR = NULL;
				int port = atoi(iter->second.strPort.c_str());
				if(strstr(iter->second.strProvider.c_str(),"大华")!=NULL)
				{
					pDVR = new CDaHuaDVR(iter->second.strIP.c_str(), port,iter->second.strLogin.c_str(),iter->second.strPassword.c_str());
				}
				else if(strstr(iter->second.strProvider.c_str(),"海康")!=NULL)
				{
					pDVR = new CHikNetDVR(iter->second.strIP.c_str(), port,iter->second.strLogin.c_str(),iter->second.strPassword.c_str());		
				}

				if(pDVR!=NULL)
					m_DVRList[key_dvr] = pDVR;
			}

			if(strcmp(iter->second.eRecordType.c_str(), "RECORD") == 0)
			{
				map<string, CDVR*>::iterator pt_dvr=m_DVRList.find(key_dvr);
				if(pt_dvr != m_DVRList.end())
				{
					//m_FailRecordList[iter->second] = 0;
					string rkey=key_dvr+":"+iter->second.strCnlID;
					m_RecordList[rkey]=iter->second;
				}
			}
		}
	}

#ifdef _DEBUG_GETPIC
		char* pic2;
	int size2 =CGetIconFormFile::Instance()->getIcon(&pic2, "D:\\record\\DH_高桥监测站_0_20140721160002343.rd", "大华");
	FILE* tf2 = fopen("D:\\record\\z.jpeg", "wb+");
	fwrite(pic2,size2,1,tf2);
	fclose(tf2);
	delete [] pic2;

	char* pic0;
	int size0 =CGetIconFormFile::Instance()->getIcon(&pic0, "D:\\record\\HK_出入口2_1_20140721102107089.rd", "海康");
	FILE* tf0 = fopen("D:\\record\\0.jpeg", "wb+");
	fwrite(pic0,size0,1,tf0);
	fclose(tf0);
	delete [] pic0;

	char* pic1;
	int size1 =CGetIconFormFile::Instance()->getIcon(&pic1, "D:\\record\\HK_出入口3_1_20140721162200734.rd", "海康");
	FILE* tf1 = fopen("D:\\record\\1.jpeg", "wb+");
	fwrite(pic1,size1,1,tf1);
	fclose(tf1);
	delete [] pic1;

	char* pic;
	int size =CGetIconFormFile::Instance()->getIcon(&pic, "D:\\record\\HK_出入口1_1_20140721162200531.rd", "海康");
	FILE* tf = fopen("D:\\record\\z.jpeg", "wb+");
	fwrite(pic,size,1,tf);
	fclose(tf);
	delete [] pic;

#endif

	unsigned int theId = 0;
	(HANDLE)_beginthreadex( NULL, 0,  reconnectFunc, (void*)this, 0, &theId );

	return TRUE;
}

//// 登录dvr设备
//int CManagerDVR::AddServer(const CHANNEL_INFO& channel)
//{
//	// 是否已经在dvr设备列表中？(判断依据：ip + port)
//	for(int i = 0; i < MAX_SERVER_SIZE; i++)
//	{
//		if(m_pDVRArray[i] != NULL)
//		{
//			// IP可能相同，只是端口不同
//			if((strcmp(channel.ServerIP, m_pDVRArray[i]->m_strServIp) == 0) && (m_pDVRArray[i]->m_ServerPort == channel.port))
//				return 0;	// 已经登陆
//		}
//	}
//
//	// 添加到数组中
//	for(int i = 0; i < MAX_SERVER_SIZE; i++ )
//	{
//		if(m_pDVRArray[i] == NULL)
//		{
//			CDVR *pDVR = NULL;
//			if(channel.ChannelType == HIK_DVRDH_CHANNEL )
//			{// 大华DVR
//				pDVR = new CDaHuaDVR();
//			}
//			else if(channel.ChannelType == HIK_DVRHK_CHANNEL)
//			{// 海康DVR
//				pDVR = new CHikNetDVR();
//			}
//			else
//			{
//				// 其他扩展
//			}
//
//			// 没有支持的厂家库
//			if( pDVR == NULL )
//			{
//				g_pLog->WriteLog("不支持此厂家的设备：IP=%s,Port=%d,Type=%d\n", channel.ServerIP, channel.port, channel.ChannelType);
//				return -1;
//			}
//			
//			m_pDVRArray[i] = pDVR;
//			
//			pDVR->m_is_multicast = channel.isMulticast;
//			pDVR->m_multicst_port = channel.MultiPort;
//			memcpy(pDVR->m_multicast_ip, channel.MultiCastIP, 16);
//
//			//if(!pDVR->Init(strIP.GetBuffer(strIP.GetLength()),strCnlName.GetBuffer(strCnlName.GetLength()), port, info.UserName, info.Password, i, type))
//			if(!pDVR->Init(i,channel))
//			{
//				m_unconnect_list.push_back(pDVR);
//				return -1;
//			}
//			else
//			{
//				//pDVR->Init(strIP.GetBuffer(strIP.GetLength()),strCnlName.GetBuffer(strCnlName.GetLength()), port, info.UserName, info.Password, i, type);		
//				m_ServerNum++;
//				return 1;
//			}
//		}
//	}
//
//	return -1;
//}

//int CManagerDVR::AddServer(CString strIP, int port, USER_NAME_INFO info, int type,CString strCnlName, bool mis, char* mip, int mport)
//{
//	// 是否已经在dvr设备列表中？(判断依据：ip + port)
//	for(int i = 0; i < MAX_SERVER_SIZE; i++)
//	{
//		if(m_pDVRArray[i] != NULL)
//		{
//			// IP可能相同，只是端口不同
//			if((strIP.Compare(m_pDVRArray[i]->m_strServIp) == 0) && (m_pDVRArray[i]->m_ServerPort == port))
//				return 0;	// 已经登陆
//		}
//	}
//
//	// 添加到数组中
//	for(int i = 0; i < MAX_SERVER_SIZE; i++ )
//	{
//		if(m_pDVRArray[i] == NULL)
//		{
//			CDVR *pDVR = NULL;
//			if(type == HIK_DVRDH_CHANNEL )
//			{// 大华DVR
//				pDVR = new CDaHuaDVR();
//			}
//			else if(type == HIK_DVRHK_CHANNEL)
//			{// 海康DVR
//				pDVR = new CHikNetDVR();
//			}
//			else
//			{
//				// 其他扩展
//			}
//
//			// 没有支持的厂家库
//			if( pDVR == NULL )
//			{
//				g_pLog->WriteLog("不支持此厂家的设备：IP=%s,Port=%d,Type=%d\n", strIP, port, type);
//				return -1;
//			}
//			
//			m_pDVRArray[i] = pDVR;
//			
//			pDVR->m_is_multicast = mis;
//			pDVR->m_multicst_port = mport;
//			memcpy(pDVR->m_multicast_ip, mip, 16);
//
//			if(!pDVR->Init(strIP.GetBuffer(strIP.GetLength()),strCnlName.GetBuffer(strCnlName.GetLength()), port, info.UserName, info.Password, i, type))
//			{
//				m_unconnect_list.push_back(pDVR);
//				return -1;
//			}
//			else
//			{
//				//pDVR->Init(strIP.GetBuffer(strIP.GetLength()),strCnlName.GetBuffer(strCnlName.GetLength()), port, info.UserName, info.Password, i, type);		
//				m_ServerNum++;
//				return 1;
//			}
//		}
//	}
//
//	return -1;
//}

//BOOL CManagerDVR::CloseServer(CString strIP,int port)
//{
//	for(int i = 0; i < MAX_SERVER_SIZE; i ++)
//	{
//		if(m_pDVRArray[i]!=NULL)
//		{
//
//			if((strIP.Compare(m_pDVRArray[i]->m_strServIp) == 0)&&(m_pDVRArray[i]->m_ServerPort == port))
//			{
//				delete m_pDVRArray[i];
//				m_pDVRArray[i] = NULL;
//				m_ServerNum--;
//				return TRUE;
//			}
//		}
//	}
//	return FALSE;
//}

//CChannel *CManagerDVR::FindChannel(CString serverip,int port, int channelid, int type)
//{
//	CChannel *pchannel = NULL;
//	for(int i = 0; i < MAX_SERVER_SIZE; i ++)
//	{
//		if(m_pDVRArray[i]!=NULL)
//		{
//			if( ( serverip.Compare( m_pDVRArray[i]->m_strServIp) == 0 ) &&(m_pDVRArray[i]->m_ServerPort==port) && ( m_pDVRArray[i]->m_type = type ) )
//			{
//				for(int k=0;k<MAX_CHANNEL_SIZE;k++)
//				{
//					if(m_pDVRArray[i]->m_pChannelArray[k]==NULL) continue;
//					if((m_pDVRArray[i]->m_pChannelArray[k]->m_DVRChannelID)==channelid)
//					{
//						pchannel = m_pDVRArray[i]->m_pChannelArray[k];
//						return pchannel;
//					}
//				}
//			}
//		}
//	}
//	return NULL;
//}

//CChannel *CManagerDVR::FindChannel(char *channelname)
//{
//	CChannel *pchannel = NULL;
//	for(int i = 0; i < MAX_SERVER_SIZE*MAX_CHANNEL_SIZE; i ++)
//	{
//		if(m_pChannelArray[i]!=NULL && m_pChannelArray[i]->m_work_status==1)
//		{
//
//			if(strcmp(m_pChannelArray[i]->m_ChannelInfo.ChannelName,channelname) == 0)
//			{
//				pchannel = m_pChannelArray[i];
//				return pchannel;
//			}
//		}
//	}
//	return NULL;
//}



// 根据ip地址和端口号来获取dvr设备
//CDVR *CManagerDVR::FindServer(CString serverip, int port)
//{
//	for(int i = 0; i < MAX_SERVER_SIZE; i ++)
//	{
//		if(m_pDVRArray[i] != NULL)
//		{
//			// 比较ip地址和端口号
//			if((serverip.Compare(m_pDVRArray[i]->m_strServIp) == 0) && (m_pDVRArray[i]->m_ServerPort == port))
//			{
//				return m_pDVRArray[i];
//			}
//		}
//	}
//	return NULL;
//}

//DWORD CManagerDVR::thread_func_connect_dvr(void* p)
//{
//	CManagerDVR* lp_dvr = (CManagerDVR*)p;
//	lp_dvr->m_isRunning = true;
//	//CDVR* channelinfo;
//	while(lp_dvr->m_isRunning)
//	{
//		//登陆连接
//		for(vector<CDVR*>::iterator iter = lp_dvr->m_unconnect_list.begin(); iter!= lp_dvr->m_unconnect_list.end(); )
//		{
//			CDVR *pDVR = *iter;
//			if(pDVR != NULL && pDVR->connect_DVR(false))
//			{ 
//				lp_dvr->m_ServerNum++;
//				iter = lp_dvr->m_unconnect_list.erase(iter);
//				// 把通道添加到通道列表
//				CChannel *pchannel = lp_dvr->FindChannel(pDVR->m_strServIp, pDVR->m_ServerPort, pDVR->m_dwChannelNum, pDVR->m_type);
//				if(pchannel != NULL)
//				{
//					memcpy(pchannel->m_ChannelInfo.ChannelName,	pDVR->m_strCameraId.c_str(),32);// 通道名称
//					pchannel->m_strDVRName = pDVR->m_strDVRName;							// dvr名称
//					pchannel->m_ChannelInfo.port = pDVR->m_ServerPort;						// 通道端口号
//					memcpy(pchannel->m_ChannelInfo.ServerIP, pDVR->m_strServIp,16);		// dvr设备ip地址
//					memcpy(pchannel->m_ChannelInfo.username, pDVR->m_UserInfo.UserName,32);		// dvr登录帐号
//					memcpy(pchannel->m_ChannelInfo.psw, pDVR->m_UserInfo.Password,32);					// dvr登录密码
//					pchannel->m_ChannelInfo.isMulticast= pDVR->m_is_multicast;			// 接收到数据之后是否以多播的方式传送
//					memcpy(pchannel->m_ChannelInfo.MultiCastIP, pDVR->m_multicast_ip,16);	// 多播的ip地址
//					pchannel->m_ChannelInfo.MultiPort= pDVR->m_multicst_port;				// 多播端口号
//					pchannel->m_strCameraId = pDVR->m_strCameraId;						// 通道名称
//
//					pchannel->InitMultiCast();				// 初始化多播模块
//					int i = 0;
//					for(; i < MAX_SERVER_SIZE*MAX_CHANNEL_SIZE; i ++)
//					{
//						if(lp_dvr->m_pChannelArray[i] == NULL)
//						{
//							lp_dvr->m_pChannelArray[i] = pchannel;			// 添加到通道列表
//							break;
//						}
//					}
//					if(i == MAX_SERVER_SIZE*MAX_CHANNEL_SIZE)
//						g_pLog->WriteLog("通道已满！\n");
//				}
//				continue;
//			}
//			iter++;
//		}
//
//		//重连超时，则重新登陆连接
//		for(int i = 0; i < MAX_SERVER_SIZE; i ++)
//		{
//			if(lp_dvr->m_pDVRArray[i]!=NULL)
//			{
//				if(lp_dvr->m_pDVRArray[i]->m_work_status == 0)
//				{
//					double dt = difftime(time(NULL), lp_dvr->m_pDVRArray[i]->m_disconnent_time);
//					if(dt > 60 )
//					{
//						lp_dvr->m_unconnect_list.push_back(lp_dvr->m_pDVRArray[i]);
//						lp_dvr->m_pDVRArray[i]->reset();
//						g_pLog->WriteLog("IPC(%s)重连超时，断开\n", lp_dvr->m_pDVRArray[i]->m_strCameraId.c_str());
//					}
//				}
//			}
//		}
//
//		Sleep(1500);
//	}
//	
//	return TRUE;
//}

BOOL CManagerDVR::addClient(string cameraID, SOCKET client)
{
	BOOL ret = FALSE;
	map<string, CAMERA_INFO>::iterator iter_camera=m_CameraList.find(cameraID);
	if(iter_camera != m_CameraList.end())
	{
		string key=iter_camera->second.strIP+":"+iter_camera->second.strPort;
		map<string, CDVR*>::iterator iter_dvr=m_DVRList.find(key);
		if(iter_dvr != m_DVRList.end())
		{
			int type = 0;
			if(strcmp(iter_camera->second.eCameraType.c_str(), "SD") == 0)
			{
				type = 1;
			}
			CChannel* channel=iter_dvr->second->stream(atoi(iter_camera->second.strCnlID.c_str()), type);
			if(channel!=NULL)
			{
				CSendCmd* cmd = new CSendCmd(client);
				ret = channel->addCmd(cmd);
				g_pLog->WriteLog("视频接入%s:%s \n", ret?"成功":"失败", cameraID.c_str());
			}
		}
	}

	return ret;
}

BOOL CManagerDVR::FindChannel(char *channelname, CHANNEL_INFO& channelInfo)
{
	map<string, CAMERA_INFO>::iterator iter=m_CameraList.find(channelname);
	if(iter != m_CameraList.end())
	{
		//channelInfo = iter->second;
		memcpy(channelInfo.ChannelName, iter->second.strCameraID.c_str(), 32);
		if(strstr(iter->second.strProvider.c_str(),"大华") != NULL)
		{
			channelInfo.ChannelType =HIK_DVRDH_CHANNEL;
		}
		else if(strstr(iter->second.strProvider.c_str(),"海康")!=NULL)
		{
			channelInfo.ChannelType =HIK_DVRHK_CHANNEL;
		}
		channelInfo.isMulticast = false;
		return TRUE;
	}
	return FALSE;
}

void CManagerDVR::Release()
{
	for(map<string, CDVR*>::iterator iter=m_DVRList.begin();iter!=m_DVRList.end();)
	{
		delete iter->second;
		iter = m_DVRList.erase(iter);
	}
	// 大华
	CLIENT_Cleanup();
	// 海康
	NET_DVR_Cleanup();
}

#ifdef DEBUG
void CManagerDVR::test()
{
	for(map<string, CDVR*>::iterator iter=m_DVRList.begin();iter!=m_DVRList.end();iter++)
	{
		iter->second->disconnect();
	}
}
#endif

BOOL	CManagerDVR::PTZControl(string cameraID, DWORD dwPTZCommand, DWORD param1, DWORD param2, DWORD param3, BOOL dwStop)
{
	BOOL ret = FALSE;
	map<string, CAMERA_INFO>::iterator iter_camera=m_CameraList.find(cameraID);
	if(iter_camera != m_CameraList.end())
	{
		string key=iter_camera->second.strIP+":"+iter_camera->second.strPort;
		map<string, CDVR*>::iterator iter_dvr=m_DVRList.find(key);
		if(iter_dvr != m_DVRList.end())
		{
			int type = 0;
			if(strcmp(iter_camera->second.eCameraType.c_str(), "SD") == 0)
			{
				type = 1;
			}
			CChannel* channel=iter_dvr->second->stream(atoi(iter_camera->second.strCnlID.c_str()), type);
			if(channel!=NULL)
			{
				ret = channel->PTZControl(dwPTZCommand, param1, param2, param3, dwStop);
			}
		}
	}

	return ret;
}

BOOL CManagerDVR::PTZStateQuery(const char* cid)
{
	BOOL ret = FALSE;
	map<string, CAMERA_INFO>::iterator iter_camera=m_CameraList.find(cid);
	if(iter_camera != m_CameraList.end())
	{
		string key=iter_camera->second.strIP+":"+iter_camera->second.strPort;
		map<string, CDVR*>::iterator iter_dvr=m_DVRList.find(key);
		if(iter_dvr != m_DVRList.end())
		{
			int type = 0;
			if(strcmp(iter_camera->second.eCameraType.c_str(), "SD") == 0)
			{
				type = 1;
			}
			CChannel* channel=iter_dvr->second->stream(atoi(iter_camera->second.strCnlID.c_str()), type);
			if(channel!=NULL)
			{
				if(!channel->m_bControl)
					ret = TRUE;
				else
					ret = 2;
			}
			else
				ret = FALSE;
		}
		else
			ret = -1;
	}

	return ret;
}

unsigned int WINAPI CManagerDVR::reconnectFunc(LPVOID inThread)
{
	CManagerDVR* pDVR = (CManagerDVR*)inThread;

	while(true)
	{
		//连接所有DVR
		for(map<string, CDVR*>::iterator ptdvr=pDVR->m_DVRList.begin();ptdvr!=pDVR->m_DVRList.end();ptdvr++)
		{
			ptdvr->second->connect();
		}
		////添加录像任务
		//for(vector<CAMERA_INFO>::iterator iter = pDVR->m_FailRecordList.begin();iter!=pDVR->m_FailRecordList.end();)
		//{
		//	string key_dvr=iter->strIP+":"+iter->strPort;
		//	map<string, CDVR*>::iterator ptdvr=pDVR->m_DVRList.find(key_dvr);
		//	if(ptdvr!=pDVR->m_DVRList.end())
		//	{
		//		map<string, CDVR*>::iterator pt_dvr=pDVR->m_DVRList.find(key_dvr);
		//		if(pt_dvr != pDVR->m_DVRList.end())
		//		{
		//			int type = 0;
		//			if(strcmp(iter->eCameraType.c_str(), "SD") == 0)
		//			{
		//				type = 1;
		//			}
		//			CChannel* channel=pt_dvr->second->stream(atoi(iter->strCnlID.c_str()), type);
		//			if(channel!=NULL)
		//			{
		//				CRecordCmd* cmd = new CRecordCmd(*iter);
		//				channel->addCmd(cmd);
		//				iter = pDVR->m_FailRecordList.erase(iter);
		//				continue;
		//			}
		//		}
		//	}
		//	iter++;
		//}
		
		for(map<string, CAMERA_INFO>::iterator iter_r=pDVR->m_RecordList.begin();iter_r!=pDVR->m_RecordList.end();)
		{
			bool success = false;
			for(map<string, CDVR*>::iterator pt_dvr=pDVR->m_DVRList.begin();pt_dvr!=pDVR->m_DVRList.end();pt_dvr++)
			{
				if(strstr(iter_r->first.c_str(), pt_dvr->first.c_str()) != NULL)
				{
					int type = 0;
					if(strcmp(iter_r->second.eCameraType.c_str(), "SD") == 0)
					{
						type = 1;
					}
					CChannel* channel=pt_dvr->second->tryStream(atoi(iter_r->second.strCnlID.c_str()), type);
					if(channel!=NULL)
					{
						CRecordCmd* cmd = new CRecordCmd(iter_r->second);
						channel->addCmd(cmd);
						success = true;
						break;
					}
				}
			}
			if(success)
				iter_r = pDVR->m_RecordList.erase(iter_r);
			else
				iter_r++;
		}
		//关闭没有任务的chanel
		for(map<string, CDVR*>::iterator iter_dvr=pDVR->m_DVRList.begin(); iter_dvr!=pDVR->m_DVRList.end();iter_dvr++)
		{
			if(!iter_dvr->second->Online())
			{
				iter_dvr->second->disconnect();
				for(map<string, CAMERA_INFO>::iterator iter = pDVR->m_CameraList.begin();iter!=pDVR->m_CameraList.end();iter++)
				{
					if(strcmp(iter->second.strIP.c_str(), iter_dvr->second->m_DVRIP.c_str()) == 0 && atoi(iter->second.strPort.c_str())==iter_dvr->second->m_DVRPort)
					{
						if(strcmp(iter->second.eRecordType.c_str(), "RECORD") == 0)
						{
							string rkey = iter->second.strIP+":"+iter->second.strPort+":"+iter->second.strCnlID;
							pDVR->m_RecordList[rkey]=iter->second;
						}
					}
				}
			}
			else
				iter_dvr->second->freeChannels();
		}

		//if(ready)
		//	break;

		Sleep(5000);
	}
	return 0;
}