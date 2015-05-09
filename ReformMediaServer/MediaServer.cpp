#include "SysInfo.h"
#include "MediaServer.h"
#include "Log.h"
#include "RFProtocol.h"
#include "DBObject.h"
#include "Config.h"
#include "media_protocal.h"
#include "CGetPicture.h"

#include <string>
#include <vector>
#include <list>
#include <io.h>

using namespace std;


int read_file(const char* name, char** buf);

MediaServer g_servObj;
#ifdef _DEBUG
extern int scount;
extern int lcount;
#endif

hash_map<SOCKET, AsyncSocketClient> MediaServer::_client_map;
SOCKET MediaServer::_bind_socket;

MediaServer::MediaServer(void)
{
	m_bExit			= false;
	m_hExitEvent	= NULL;
	m_strServAddr	= "127.0.0.1";

	_bind_socket = INVALID_SOCKET;

	InitializeCriticalSection(&m_CriticalSection);
}

MediaServer::~MediaServer(void)
{
	m_bExit = true;
	SetEvent(m_hExitEvent);
	Sleep(0);
	DeleteCriticalSection(&m_CriticalSection);
}

int MediaServer::GetServerIPAddress()
{
	int sock = -1;
	struct in_addr testAddr;
	int newSocket = -1;

	// We need to find our source address
	struct sockaddr_in fromAddr;
	fromAddr.sin_addr.s_addr = INADDR_ANY;

	// Get our address by sending a (0-TTL) multicast packet,
	// receiving it, and looking at the source address used.
	// (This is kinda bogus, but it provides the best guarantee
	// that other nodes will think our address is the same as we do.)
	do
	{
		testAddr.s_addr = inet_addr("228.67.43.91");		// arbitrary
		int port = 15947; // ditto

		newSocket = WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0, WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF |	WSA_FLAG_OVERLAPPED);
		if( newSocket == INVALID_SOCKET)
			break;

		int reuseFlag = 1;
		if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseFlag, sizeof reuseFlag) < 0) 
			break;

		struct sockaddr_in name;
		name.sin_family = AF_INET;
		name.sin_addr.s_addr = INADDR_ANY;
		name.sin_port = htons(port);
		if (bind(newSocket, (struct sockaddr*)&name, sizeof(sockaddr_in)) != 0)
			break;

		fromAddr.sin_family = AF_INET;
		fromAddr.sin_port = htons(port);
		fromAddr.sin_addr.s_addr = testAddr.s_addr;
		if(( sock = WSAJoinLeaf(newSocket, (SOCKADDR*)&fromAddr, sizeof(fromAddr), NULL, NULL, NULL, NULL, JL_BOTH)) == INVALID_SOCKET)
			break;

		unsigned char testString[] = "rftest...";
		unsigned testStringLength = sizeof testString;

		int ttl = 0;
		struct sockaddr_in dest;
		dest.sin_family = AF_INET;
		dest.sin_addr.s_addr = testAddr.s_addr;
		dest.sin_port = htons(port);
		int bytesSent = sendto(newSocket, (const char*)testString, testStringLength, 0, (struct sockaddr*)&dest, sizeof dest);
		if (bytesSent != testStringLength) 
			break;

		// Block until the socket is readable (with a 5-second timeout):
		fd_set rd_set;
		FD_ZERO(&rd_set);
		FD_SET((unsigned)newSocket, &rd_set);
		const unsigned numFds = newSocket + 1;
		struct timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		int result = select(numFds, &rd_set, NULL, NULL, &timeout);
		if (result <= 0)
			break;

		unsigned char readBuffer[20];
		int addressSize = sizeof sockaddr_in;
		int bytesRead = recvfrom(newSocket, (char*)readBuffer, sizeof(readBuffer), 0, (struct sockaddr*)&fromAddr, &addressSize);
		if (bytesRead != (int)testStringLength ) 
			break;
	}
	while (0);

	if (newSocket >= 0)
		closesocket(newSocket);

	if(sock >= 0)
		closesocket(sock);

	m_strServAddr = inet_ntoa(fromAddr.sin_addr);
	return 0;
}

int MediaServer::Start(void)
{
	if(init_socket_server() == FALSE)
		return FALSE;
	//m_hPrepareEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//ResetEvent(m_hExitEvent);
	//GetServerIPAddress();

	m_qSend.Init("发送队列");
	m_qRecv.Init("接收队列");

	DWORD dw;
	//CreateThread( NULL, 0, LPTHREAD_START_ROUTINE(ThreadListe), this, 0, &dw);
	CreateThread( NULL, 0, LPTHREAD_START_ROUTINE(threadNetWork), this, 0, &dw);
	CreateThread( NULL, 0, LPTHREAD_START_ROUTINE(threadWrite), this, 0, &dw);
	//CreateThread( NULL, 0, LPTHREAD_START_ROUTINE(ThreadSend), this, 0, &dw);
	CreateThread( NULL, 0, LPTHREAD_START_ROUTINE(ThreadProcessData), this, 0, &dw);
	
	return TRUE;
}

// 分析指令数据
int MediaServer::AnalyseMsg(char *buff, int len, int sock)
{
	g_pLog->WriteLog("Process a cmd...\n");

	if (buff == NULL)
		return -1;

	LpRFAppComand pAppCommand = (LpRFAppComand)buff;
	if (pAppCommand->nPacketID != A_PROTOCOL_HEAD)
		return -1;

	if (pAppCommand->nDataSize > PACKET_MAX_SIZE - sizeof(RFAppComand))
		return -1;

	//itSvrSocket itsvrsock = m_Socket.m_mapSvrSocket.find(sock);
	//if (itsvrsock == m_Socket.m_mapSvrSocket.end())
	//	return -1;

	//LPSvrSocket pSvrSocket = (LPSvrSocket)itsvrsock->second;
	//if (pSvrSocket == NULL)
	//	return -1;

	switch (pAppCommand->nCommand)
	{

	case REGISTER_COMMAND:				// 注册
		OnRegist(buff, sock);
		break;

	case HANDSHAKE_COMMAND:				// 握手
		OnHandshake(buff, sock);
		break;

	case UPDATE_DEVICE_COMMAND:			// 即时配置
		OnUpdateDevice(buff, sock);
		break;

	case UPDATE_CONFIG_COMMAND:			// 即时更新配置
		OnUpdateConfig(buff, sock);
		break;

	case DEVICE_REQUEST_COMMAND:		// 客户端请求设备列表
		OnRequestDevices(buff, sock);
		break;

	case VIDEO_RECORD_REQUEST_COMMAND:	// 客户端请求录像文件列表
		//OnRequestVideoRecords(buff, sock);
		RequestVRResource(buff, sock);
		break;
	case VIDEO_RECORD_REQUEST_WITH_ICON_COMMAND:
		RequestVRResourceWithIcon(buff, sock);
		break;

	default: 
		return -1;
	}
	g_pLog->WriteLog("Finish a cmd(%d)\n", pAppCommand->nCommand);
	return 0;
}

// 注册
BOOL MediaServer::OnRegist(char *recvBuff, int sock)
{
	LpRFAppComand pAppHead		= (LpRFAppComand)recvBuff;
	LPRegister pRegister		= (LPRegister)(pAppHead + 1);
	string strUser				= pRegister->szUser;
	string strPass				= pRegister->szPass;

	// 用户验证(需要修改成查询数据库）
	if(strUser == "admin" && strPass == "12345")
		return FALSE;

	char buff[sizeof(RFAppComand) + sizeof(RegisterAck)] = {0};
	LpRFAppComand pAppHead_RegAck	= (LpRFAppComand)buff;
	LPRegisterAck pRegisterAck		= (LPRegisterAck)(pAppHead_RegAck + 1);

	pAppHead_RegAck->nPacketID		= A_PROTOCOL_HEAD;
	pAppHead_RegAck->nPacketNo		= pAppHead->nPacketNo;
	pAppHead_RegAck->nCommand		= RESP_REGISTER_COMMAND;
	pAppHead_RegAck->nDataSize		= sizeof(RegisterAck);

	pRegisterAck->nResult			= 0;
	pRegisterAck->nDeviceAddr		= pRegister->nDeviceAddr;

	QueueList			qList;
	qList.sock			= sock;
	qList.nLen			= pAppHead->GetPacketTotalSize();
	memcpy(qList.buffer, buff, pAppHead->GetPacketTotalSize()); 
	m_qSend.PutBack(qList);

	return TRUE;
}

// 握手
void MediaServer::OnHandshake(char *recvBuff, int sock)
{

}

// 数据入库
BOOL MediaServer::CreateUpdateDeviceSql(char* buff, list<string>& sqlList)
{
	LPDeviceUnitData pUnitData = (LPDeviceUnitData)buff;
	if(pUnitData->nUpdateType == INSERT_DATA)
	{
		char szComm[1024] = {0};
		sprintf_s(szComm, "INSERT INTO Camera VALUES(NULL, '%s', 'DIGITAL', '%s', '%s','%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 'INVAILID_RECORD')", 
			pUnitData->szCameraId, pUnitData->szCameraName, pUnitData->szMode, pUnitData->szProvider, pUnitData->szCodec, pUnitData->szLogin, pUnitData->szPassword,
			pUnitData->szIp, pUnitData->szPort, pUnitData->szCnlId, pUnitData->szMultiCast, pUnitData->szMultiCastAddress, pUnitData->szMultiCastPort);
		sqlList.push_back(szComm);
	}
	else if(pUnitData->nUpdateType == UPDATE_DATA)
	{
		char szComm[1024] = {0};
		sprintf_s(szComm,
			"UPDATE Camera SET CmeraId			= '%s',			\
								CameraType		= 'DIGITAL',	\
								CameraName		= '%s',			\
								CameraMode		= '%s',			\
								Provider		= '%s',			\
								Codec			= '%s',			\
								Login			= '%s',			\
								Password		= '%s',			\
								IP				= '%s',			\
								Port			= '%s',			\
								ChannelId		= '%s',			\
								IsMultiCast		= '%s',			\
								MultiCastAddr	= '%s',			\
								MultiCastPort	= '%s',			\
								RecordType		= 'INVAILID_RECORD') WHERE Id = %d", 
			pUnitData->szCameraId, pUnitData->szCameraName, pUnitData->szMode, pUnitData->szProvider, pUnitData->szCodec, pUnitData->szLogin, pUnitData->szPassword,
			pUnitData->szIp, pUnitData->szPort, pUnitData->szCnlId, pUnitData->szMultiCast, pUnitData->szMultiCastAddress, pUnitData->szMultiCastPort, pUnitData->nId);
		sqlList.push_back(szComm);
	}
	else if(pUnitData->nUpdateType == DELETE_DATA)
	{
		char szComm[1024] = {0};
		sprintf_s(szComm, "DELETE FROM Camera WHERE Id = %d", pUnitData->nId);
		sqlList.push_back(szComm);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

// 更新设备信息(支持多条记录同时更新)
void MediaServer::OnUpdateDevice(char *recvBuff, int sock)
{
	BOOL bRet = FALSE;

	if (recvBuff == NULL)
		return;

	LpRFAppComand pAppHead = (LpRFAppComand)recvBuff;
	LPDevicePack pDevice = (LPDevicePack)(pAppHead + 1);

	do
	{
		if(pDevice->byCount <= 0)
			break;

		list<string> sqlList;
		LPDeviceUnitData pUnitData = NULL;

		// 更新数据库数据
		for(int i=0; i<pDevice->byCount; i++)
		{
			pUnitData = (LPDeviceUnitData)(pDevice + i);
			CreateUpdateDeviceSql((char*)pUnitData, sqlList);
		}

		bRet = g_dbobj.UpdateDevice(sqlList);
		if(!bRet)
			break;

		// 更新内存数据(全部数据一起更新)
		g_dbobj.UpdateData(pUnitData);
		/*for(int i=0; i<pDevice->byCount; i++)
		{
			pUnitData = (LPDeviceUnitData)(pDevice + i);	// 单条记录更新
			g_dbobj.UpdateData(pUnitData);
		}*/

		// 通知各个客户端进行数据更新
		MapSvrSocket mapSvrSocket;
		m_Socket.GetSvrSockMap(mapSvrSocket);
		itSvrSocket itsocket;

		for(itsocket = mapSvrSocket.begin(); itsocket != mapSvrSocket.end(); ++itsocket)
		{
			char buff[sizeof(RFAppComand) + sizeof(DeviceUpdate)] = {0};
			LpRFAppComand pAppHead_RegAck = (LpRFAppComand)buff;
			pAppHead_RegAck->nCommand = DEVICE_UPDATE_COMMAND;
			pAppHead_RegAck->nPacketID = A_PROTOCOL_HEAD;
			pAppHead_RegAck->nPacketNo = 9999;
			pAppHead_RegAck->nDataSize = sizeof(DeviceUpdate);
			LpDeviceUpdate pRegisterAck = (LpDeviceUpdate)(pAppHead_RegAck + 1);
			pRegisterAck->flag = 1;

			QueueList qList;
			qList.sock  = itsocket->second->sock;
			qList.nLen	= pAppHead_RegAck->GetPacketTotalSize();
			memcpy(qList.buffer, buff, pAppHead_RegAck->GetPacketTotalSize()); 
			m_qSend.PutBack(qList);
		}
	}
	while(0);

	char buff[sizeof(RFAppComand) + sizeof(CommonPackAck)] = {0};
	LpRFAppComand pAppHead_RegAck = (LpRFAppComand)buff;
	pAppHead_RegAck->nPacketID = A_PROTOCOL_HEAD;
	pAppHead_RegAck->nCommand = RESP_UPDATE_DEVICE_COMMAND;
	pAppHead_RegAck->nDataSize = sizeof(CommonPackAck);
	pAppHead_RegAck->nPacketNo = pAppHead->nPacketNo;

	LPCommonPackAck pRegisterAck = (LPCommonPackAck)(pAppHead_RegAck + 1);
	pRegisterAck->nResult = bRet ? 0 : 1;
	
	QueueList qList;
	qList.sock  = sock;
	qList.nLen	= pAppHead_RegAck->GetPacketTotalSize();
	memcpy(qList.buffer, buff, pAppHead_RegAck->GetPacketTotalSize()); 
	m_qSend.PutBack(qList);
}

// 即时配置命令
void MediaServer::OnUpdateConfig(char *recvBuff, int sock)
{
	BOOL bRet = FALSE;

	if (recvBuff == NULL)
		return;

	// 暂时搞不清楚即时配置需要配置什么内容？
}

void MediaServer::OnRequestDevices(char *recvBuff, int sock)
{
	BOOL bRet = FALSE;
	if (recvBuff == NULL)
		return;

	MapRFCamera mapCamera;	
	g_xmlConfig.QueryCameras(mapCamera);

	g_pLog->WriteLog("查询到%d条设备信息\n", mapCamera.size());

	QueueList qList;
	if(mapCamera.empty())
	{
		char szSendBuff[1024] = {'\0'};
		LpRFAppComand lpAck = (LpRFAppComand)szSendBuff;
		lpAck->nPacketID = A_PROTOCOL_HEAD;
		lpAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
		lpAck->nCommand = RESP_DEVICE_REQUEST_COMMAND;
		lpAck->nDataSize = sizeof(QueryVideoRecordCommandAck);//消息长度赋值
		LpGetDeviceListCommandAck lpAckRecord = (LpGetDeviceListCommandAck)(lpAck + 1);
		lpAckRecord->nTotalCount = 0;
		lpAckRecord->nCurrCount = -1;
		qList.nLen = lpAck->GetPacketTotalSize();
		qList.sock = sock;
		qList.buffer = new char[qList.nLen];
		if(qList.buffer ==NULL)
		{
			g_pLog->WriteLog("申请内存失败, %d\n", qList.nLen);
			return;
		}
		//拷贝消息头
		memcpy(qList.buffer, szSendBuff, sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck));
		//加入发送列表m_qSend
		m_qSend.PutBack(qList);
	}
	else
	{
		LpRFAppComand pAppCommandAck = (LpRFAppComand)recvBuff;
		int count = 0;
		int size = ( PACKET_MAX_SIZE - sizeof(GetDeviceListCommandAck) - sizeof(RFAppComand) ) / sizeof(DeviceObject);
		DeviceObject lpDevice[50] = {'\0'};
		BOOL hasSend = FALSE;
		for(IterRFCamera iterDevice = mapCamera.begin(); iterDevice != mapCamera.end(); iterDevice++)
		{
			LPRFCamera lpCamera=iterDevice->second;
			sprintf_s(lpDevice[count].szRtspString, ":%d/%s", g_pSysInfo->m_RtspPort, lpCamera->strCameraId.c_str());
			sprintf_s(lpDevice[count].szDVRName, "%s", lpCamera->strCameraName.c_str());
			sprintf_s(lpDevice[count].szChannelName, "%s", lpCamera->strCnlId.c_str());
			sprintf_s(lpDevice[count].szInnerName, "%s", lpCamera->strCameraId.c_str());
			sprintf_s(lpDevice[count].szProvider,"%s",lpCamera->strProvider.c_str());
			hasSend = FALSE;
			if((++count)>=size)
			{
				char szSendBuff[1024]={'\0'};
				LpRFAppComand lpAck=(LpRFAppComand)szSendBuff;
				lpAck->nPacketID=A_PROTOCOL_HEAD;
				lpAck->nPacketNo=((LpRFAppComand)recvBuff)->nPacketNo;
				lpAck->nCommand=RESP_DEVICE_REQUEST_COMMAND;
				lpAck->nDataSize=sizeof(GetDeviceListCommandAck) + (count * sizeof(DeviceObject));

				LpGetDeviceListCommandAck lpAckGetDevice=(LpGetDeviceListCommandAck)(lpAck+1);
				lpAckGetDevice->nTotalCount=mapCamera.size();
				lpAckGetDevice->nCurrCount=count;

				qList.nLen=lpAck->GetPacketTotalSize();
				qList.sock=sock;
				qList.buffer = new char[qList.nLen];
				if(qList.buffer == NULL)
				{
					g_pLog->WriteLog("申请内存失败, %d\n", qList.nLen);
					return;
				}
				memcpy(qList.buffer,szSendBuff,sizeof(RFAppComand) + sizeof(GetDeviceListCommandAck));
				memcpy(qList.buffer+sizeof(RFAppComand)+sizeof(GetDeviceListCommandAck),(char*)lpDevice,count*sizeof(DeviceObject));
				m_qSend.PutBack(qList);
				count=0;
				hasSend = TRUE;
			}
		}

		if(count>=0 && !hasSend)
		{
			char szSendBuff[1024]={'\0'};
			LpRFAppComand lpAck=(LpRFAppComand)szSendBuff;
			lpAck->nPacketID=A_PROTOCOL_HEAD;
			lpAck->nPacketNo=((LpRFAppComand)recvBuff)->nPacketNo;
			lpAck->nCommand=RESP_DEVICE_REQUEST_COMMAND;
			lpAck->nDataSize=sizeof(GetDeviceListCommandAck) + (count * sizeof(DeviceObject));

			LpGetDeviceListCommandAck lpAckGetDevice=(LpGetDeviceListCommandAck)(lpAck+1);
			lpAckGetDevice->nTotalCount=mapCamera.size();
			lpAckGetDevice->nCurrCount=count;

			qList.nLen=lpAck->GetPacketTotalSize();
			qList.sock=sock;
			qList.buffer = new char[qList.nLen];
			if(qList.buffer == NULL)
			{
				g_pLog->WriteLog("申请内存失败, %d\n", qList.nLen);
				return;
			}
			memcpy(qList.buffer,szSendBuff,sizeof(RFAppComand) + sizeof(GetDeviceListCommandAck));
			memcpy(qList.buffer+sizeof(RFAppComand)+sizeof(GetDeviceListCommandAck),(char*)lpDevice,count*sizeof(DeviceObject));
			m_qSend.PutBack(qList);
		}
	}
}

// 客户端请求设备列表
//void MediaServer::OnRequestDevices(char *recvBuff, int sock)
//{
//	BOOL bRet = FALSE;
//	if (recvBuff == NULL)
//		return;
//
//	MapRFCamera mapCamera;
//	// g_dbobj.GetDeviceList(mapCamera);
//	g_xmlConfig.QueryCameras(mapCamera);
//
//	char buff[PACKET_MAX_SIZE] = {0};
//	LpRFAppComand pAppCommandAck = (LpRFAppComand)buff;
//	LpGetDeviceListCommandAck pDeviceAck = (LpGetDeviceListCommandAck)(pAppCommandAck + 1);// LpDeviceObject
//
//	pAppCommandAck->nPacketID = A_PROTOCOL_HEAD;
//	pAppCommandAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
//	pAppCommandAck->nCommand = RESP_DEVICE_REQUEST_COMMAND;
//
//	pDeviceAck->nTotalCount = mapCamera.size();
//	pDeviceAck->nCurrCount = 0;
//
//	int size = ( PACKET_MAX_SIZE - sizeof(GetDeviceListCommandAck) - sizeof(RFAppComand) ) / sizeof(DeviceObject);
//	pAppCommandAck->nDataSize = sizeof(GetDeviceListCommandAck);
//	
//	int count = 0;
//	LpDeviceObject lpDevice = (LpDeviceObject)(pDeviceAck + 1);
//
//	for(IterRFCamera iterDevice = mapCamera.begin(); iterDevice != mapCamera.end(); ++iterDevice)
//	{
//		sprintf_s(lpDevice->szRtspString, "rtsp://%s/%s", m_strServAddr.c_str(), iterDevice->second->strCameraId.c_str());
//		sprintf_s(lpDevice->szDVRName, "%s", iterDevice->second->strCameraName.c_str());
//		sprintf_s(lpDevice->szChannelName, "%s", iterDevice->second->strCnlId.c_str());
//		sprintf_s(lpDevice->szInnerName, "%s", iterDevice->second->strCameraId.c_str());
//		sprintf_s(lpDevice->szProvider,"%s",iterDevice->second->strProvider.c_str());
//
//		count++;
//		lpDevice++;
//		pDeviceAck->nCurrCount++;
//		pAppCommandAck->nDataSize += sizeof(DeviceObject);
//
//		if(count == size )
//		{//当执行到这里循环尚未结束时下一次循环报错
//			QueueList qList;
//			qList.sock  = sock;
//			qList.nLen	= pAppCommandAck->GetPacketTotalSize();
//			memcpy(qList.buffer, buff, pAppCommandAck->GetPacketTotalSize());
//			memset( buff + sizeof(RFAppComand) + sizeof(GetDeviceListCommandAck), 0, PACKET_MAX_SIZE - sizeof(RFAppComand) + sizeof(GetDeviceListCommandAck));
//			m_qSend.PutBack(qList);
//
//			count = 0;
//			lpDevice = (LpDeviceObject)(pDeviceAck + 1);
//			pAppCommandAck->nDataSize = sizeof(GetDeviceListCommandAck);
//		}
//	}
//
//	QueueList qList;
//	qList.sock  = sock;
//	qList.nLen	= pAppCommandAck->GetPacketTotalSize();
//	memcpy(qList.buffer, buff, pAppCommandAck->GetPacketTotalSize()); 
//	m_qSend.PutBack(qList);
//}

// 客户端请求录像列表
//void MediaServer::OnRequestVideoRecords(char *recvBuff, int sock)
//{
//	BOOL bRet = FALSE;
//	if (recvBuff == NULL)
//		return;
//	try
//	{
//	LpRFAppComand lpCommand = (LpRFAppComand)recvBuff;
//	LpQueryVideoReocrdCommand lpRecordCommand = (LpQueryVideoReocrdCommand)(lpCommand + 1);
//
//	MapQueryRecordResult mapRecord;
//	g_dbobj.QueryRecord(mapRecord, lpRecordCommand->szStartTime, lpRecordCommand->szEndTime, lpRecordCommand->szKey );
//
//	char buff[PACKET_MAX_SIZE]={0};
//	LpRFAppComand lpAppCommandAck=(LpRFAppComand)buff;
//	lpAppCommandAck->nPacketID = A_PROTOCOL_HEAD;
//	lpAppCommandAck->nPacketNo = lpCommand->nPacketNo;
//	lpAppCommandAck->nCommand = RESP_VIDEO_RECORD_REQUEST_COMMAND;
//
//	LpQueryVideoRecordCommandAck lpAckRecord = (LpQueryVideoRecordCommandAck)(lpAppCommandAck + 1);
//	lpAckRecord->nTotalCount=mapRecord.size();
//	lpAckRecord->nCurrCount=0;
//	int size = ( PACKET_MAX_SIZE - sizeof(QueryVideoRecordCommandAck) - sizeof(RFAppComand) ) / sizeof(QueryVideoRecord);
//	lpAppCommandAck->nDataSize=sizeof(QueryVideoRecordCommandAck);
//
//	int count = 0;
//	LpQueryVideoRecord lpRecord=(LpQueryVideoRecord)(lpAckRecord+1);
//
//	for(IterMapQueryRecordResult iterRecord = mapRecord.begin(); iterRecord != mapRecord.end(); ++iterRecord)
//	{
//		LpQueryRecordResult lpQueryRecordResult = (LpQueryRecordResult)iterRecord->second;
//
//		lpRecord->Id = lpQueryRecordResult->nRecordId;
//		lpRecord->nDeviceId = lpQueryRecordResult->nDeviceid;
//		sprintf_s(lpRecord->szDVRName, "%s", lpQueryRecordResult->strDVRName.c_str());
//		sprintf_s(lpRecord->szChannelName, "%s", lpQueryRecordResult->strDVRChannel.c_str());
//		sprintf_s(lpRecord->szDVRAddr, "%s", lpQueryRecordResult->strDVRAddr.c_str());
//		sprintf_s(lpRecord->szPath, "%s", lpQueryRecordResult->strFolder.c_str());
//		sprintf_s(lpRecord->szFileName, "%s", lpQueryRecordResult->strRecordName.c_str());
//		sprintf_s(lpRecord->szStartTime, "%s", lpQueryRecordResult->tStart.c_str());
//		sprintf_s(lpRecord->szEndTime, "%s", lpQueryRecordResult->tEnd.c_str());		
//
//		count++;
//		lpRecord++;
//		lpAckRecord->nCurrCount++;
//		lpAppCommandAck->nDataSize+=sizeof(QueryVideoRecord);
//
//		if(count==size)
//		{//当执行到这里循环尚未结束时下一次循环报错
//			QueueList qList;
//			qList.sock=sock;
//			qList.nLen=lpAppCommandAck->GetPacketTotalSize();
//			memcpy(qList.buffer,buff,lpAppCommandAck->GetPacketTotalSize());
//			memset(buff+sizeof(RFAppComand)+sizeof(QueryVideoRecordCommandAck),0,PACKET_MAX_SIZE-sizeof(RFAppComand)+sizeof(QueryVideoRecordCommandAck));
//			m_qSend.PutBack(qList);
//
//			count=0;
//			lpRecord=(LpQueryVideoRecord)(lpAckRecord+1);
//			lpAppCommandAck->nDataSize=sizeof(QueryVideoRecordCommandAck);
//		}		
//	}	
//
//	QueueList qList;
//	qList.sock=sock;
//	qList.nLen=lpAppCommandAck->GetPacketTotalSize();
//	memcpy(qList.buffer,buff,lpAppCommandAck->GetPacketTotalSize());
//	m_qSend.PutBack((qList));
//	}
//	catch (...)
//	{
//	}
//
//}

// 客户端请求录像列表
void MediaServer::OnRequestVideoRecords(char *recvBuff, int sock)
{
	BOOL bRet = FALSE;
	if (recvBuff == NULL)
		return;

	LpRFAppComand lpCommand = (LpRFAppComand)recvBuff;
	LpQueryVideoReocrdCommand lpRecordCommand = (LpQueryVideoReocrdCommand)(lpCommand + 1);
	//LpQueryVideoReocrdCommand lpRecordCommand = (LpQueryVideoReocrdCommand)recvBuff;

	MapQueryRecordResult mapRecord;
	g_dbobj.QueryRecord(mapRecord, lpRecordCommand->szStartTime, lpRecordCommand->szEndTime, lpRecordCommand->szKey );
		
	QueueList qList;
	int count = 0;
	int oneSize = sizeof(QueryVideoRecord);
	int size = ( PACKET_MAX_SIZE - sizeof(QueryVideoRecordCommandAck) - sizeof(RFAppComand) ) / sizeof(QueryVideoRecord);

	//char buff[PACKET_MAX_SIZE] = {'\0'};
	//QueryVideoRecord* lpRecord = (QueryVideoRecord *)(buff);
	QueryVideoRecord lpRecord[PACKET_MAX_SIZE] = {'\0'};
	BOOL hasSend = FALSE;
	int sendTotal=0;
	for(IterMapQueryRecordResult iterRecord = mapRecord.begin(); iterRecord != mapRecord.end(); iterRecord++)
	{
		LpQueryRecordResult lpQueryRecordResult = (LpQueryRecordResult)iterRecord->second;

		lpRecord[count].Id = lpQueryRecordResult->nRecordId;
		lpRecord[count].nDeviceId = lpQueryRecordResult->nDeviceid;
		sprintf_s(lpRecord[count].szDVRName, "%s", lpQueryRecordResult->strDVRName.c_str());

		sprintf_s(lpRecord[count].szChannelName, "%s", lpQueryRecordResult->strDVRChannel.c_str());
		sprintf_s(lpRecord[count].szDVRAddr, "%s", lpQueryRecordResult->strDVRAddr.c_str());
		sprintf_s(lpRecord[count].szPath, "%s", lpQueryRecordResult->strFolder.c_str());
		sprintf_s(lpRecord[count].szFileName, "%s", lpQueryRecordResult->strRecordName.c_str());
		sprintf_s(lpRecord[count].szStartTime, "%s", lpQueryRecordResult->tStart.c_str());
		sprintf_s(lpRecord[count].szEndTime, "%s", lpQueryRecordResult->tEnd.c_str());		
		/*BOOL */hasSend = FALSE;
		if( ( ++count ) >= size)
		{
			char szSendBuff[1024] = {'\0'};
			LpRFAppComand lpAck = (LpRFAppComand)szSendBuff;
			lpAck->nPacketID = A_PROTOCOL_HEAD;
			lpAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
			lpAck->nCommand = RESP_VIDEO_RECORD_REQUEST_COMMAND;
			lpAck->nDataSize = sizeof(QueryVideoRecordCommandAck) + ( count * sizeof(QueryVideoRecord) );

			LpQueryVideoRecordCommandAck lpAckRecord = (LpQueryVideoRecordCommandAck)(lpAck + 1);
			lpAckRecord->nTotalCount = mapRecord.size();
			lpAckRecord->nCurrCount = count;
			
			memcpy(qList.buffer, szSendBuff, sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck));
			memcpy( qList.buffer + sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck), (char*)lpRecord, count * sizeof(QueryVideoRecord));
			qList.nLen = lpAck->GetPacketTotalSize();
			qList.sock = sock;
			m_qSend.PutBack(qList);
			
			count = 0;
			/*BOOL */hasSend = TRUE;
#ifdef _DEBUG
			scount++;
#endif
		}
	}	

	if(count >= 0 && !hasSend)
	{
		char szSendBuff[1024] = {'\0'};
		LpRFAppComand lpAck = (LpRFAppComand)szSendBuff;
		lpAck->nPacketID = A_PROTOCOL_HEAD;
		lpAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
		lpAck->nCommand = RESP_VIDEO_RECORD_REQUEST_COMMAND;
		lpAck->nDataSize = sizeof(QueryVideoRecordCommandAck) + ( count * sizeof(QueryVideoRecord) );

		LpQueryVideoRecordCommandAck lpAckRecord = (LpQueryVideoRecordCommandAck)(lpAck + 1);
		lpAckRecord->nTotalCount = mapRecord.size();
		lpAckRecord->nCurrCount = count;

		memcpy(qList.buffer, szSendBuff, sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck));
		memcpy( qList.buffer + sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck), (char*)lpRecord, count * sizeof(QueryVideoRecord));
		qList.nLen = lpAck->GetPacketTotalSize();
		qList.sock = sock;
		m_qSend.PutBack(qList);
		
	}
	//g_pLog->WriteLog("已发送录像记录总数:%d\n",sendTotal);
}

//// 监听线程
//int MediaServer::ThreadListe(LPVOID p_pJobParam)
//{
//	MediaServer* pThis = (MediaServer*)p_pJobParam;
//
//	g_pLog->WriteLog( "start RFMediaServ，port:%d\n", RF_STREAM_SERVER_PORT );
//
//	while (!pThis->m_bExit)
//	{
//		int ret = pThis->m_Socket.InitAndListen( RF_STREAM_SERVER_PORT );
//
//		if (ret == 0)
//		{
//			g_pLog->WriteLog( "client connect succeeded\n");
//
//			DWORD dw;
//			CreateThread( NULL, 0, LPTHREAD_START_ROUTINE(ThreadRecv), pThis, 0, &dw);
//
//			// 保证接收线程启动
//			//WaitForSingleObject(pThis->m_hPrepareEvent, INFINITE);
//		}
//		else if (ret == -1)
//		{
//			g_pLog->WriteLog("client connect failed\n", RF_STREAM_SERVER_PORT);
//			return 0;
//		}
//		else if (ret == -2)
//		{
//			g_pLog->WriteLog("listen failed!port:%d \n", RF_STREAM_SERVER_PORT);
//			return 0;
//		}
//		else if (ret == -3)
//		{
//			g_pLog->WriteLog("RFMediaServ accept failed，port:%d\n", RF_STREAM_SERVER_PORT);
//		}
//		else if (ret == -4)
//		{
//			g_pLog->WriteLog("client connect failed，port：%d, insufficient space!\n", RF_STREAM_SERVER_PORT);
//		}
//	}
//
//	return 0;
//}
//
//// 接收线程
//int MediaServer::ThreadRecv(LPVOID p_pJobParam)
//{
//	MediaServer* pThis = (MediaServer*)p_pJobParam;
//	SOCKET sock = pThis->m_Socket.m_hClientSocketLast;
//
//	itSvrSocket itsvrsock = pThis->m_Socket.m_mapSvrSocket.find(sock);
//	if (itsvrsock == pThis->m_Socket.m_mapSvrSocket.end())
//		return 1;
//
//	LPSvrSocket pSvrSocket = (LPSvrSocket)itsvrsock->second;
//	int nPort = pSvrSocket->port;
//
//	while (!pThis->m_bExit)
//	{
//		if (pThis->m_Socket.CheckSocketReadable(sock) != 0)
//		{
//			Sleep(5);
//			continue;
//		}
//
//		char buffer[256] = {'\0'};
//		int  nResult = pThis->m_Socket.ReceiveMsg(buffer, sock);
//		if( nResult > 0 )
//		{
//			// 压入接收队列里
//			LpRFAppComand pAppCommand = (LpRFAppComand)(buffer);
//			QueueList qList;
//			qList.sock  = (int)sock;
//			qList.nLen	= pAppCommand->GetPacketTotalSize();
//			memcpy(qList.buffer, buffer, pAppCommand->GetPacketTotalSize());
//			pThis->m_qRecv.PutBack(qList);
//		}
//		else if (nResult == 0)
//		{
//			CString strMsg;
//			strMsg.Format("recv timeout ip:%s, port:%d, errorno:%d\n", inet_ntoa(pSvrSocket->ip), nPort, nResult);
//			g_pLog->WriteLog( strMsg.GetBuffer() );
//			continue;
//		}
//		else if (nResult < 0) 
//		{
//			pThis->m_Socket.CloseClientSocket(sock);
//
//			CString strMsg;
//			strMsg.Format("connnect closed ip:%s, port:%d\n", inet_ntoa(pSvrSocket->ip), nPort, nResult);
//			g_pLog->WriteLog( strMsg.GetBuffer() );
//
//			return 0;
//		}
//	}
//	return 0;
//}
//
//// 发送线程
//int MediaServer::ThreadSend(LPVOID p_pJobParam)
//{
//	MediaServer * pThis = (MediaServer *)p_pJobParam;
//
//	HANDLE hSem = pThis->m_qSend.GetSemphore();
//	HANDLE hEvent[2] = { hSem, pThis->m_hExitEvent };
//
//	char szContent[1024] = {'\0'};
//
//	while (!pThis->m_bExit)
//	{    
//		DWORD dwRet = WaitForMultipleObjects(2, hEvent, FALSE, 100/*WSA_INFINITE*/);
//
//		switch (dwRet)
//		{
//		case WAIT_OBJECT_0:
//			{
//				QueueList qList;
//				if (pThis->m_qSend.GetHead(&qList) == 0)
//				{
//					MapSvrSocket mapSvrSocket;
//					pThis->m_Socket.GetSvrSockMap(mapSvrSocket);
//
//					if (qList.sock != 0)
//					{
//						//if (pThis->m_Socket.CheckSocketWriteable(pSvrSocket->sock) != 0)
//						//{
//						//	lcount++;
//						//	continue;
//						//}
//
//						if (pThis->m_Socket.SendMsg((char*)qList.buffer, qList.nLen, qList.sock/*pSvrSocket->sock*/) < 0)
//						{
//							g_pLog->WriteLog("RFMediaServ send data failed, client IP:%s\n"/*, inet_ntoa(pSvrSocket->ip)*/);
//							continue;
//						}
//					}
//				}
//				break;
//			}
//		case WAIT_OBJECT_0 + 1:
//			//pThis->m_qSend.CleanData();		// 是否需要这边调用清理函数？
//			return 0;
//		case WAIT_TIMEOUT:
//			break;
//		}
//	}
//
//	return 0;
//}

// 数据处理线程
int MediaServer::ThreadProcessData(LPVOID p_pJobParam)
{
	MediaServer * pThis = (MediaServer *)p_pJobParam;
	HANDLE hSem = pThis->m_qRecv.GetSemphore();
	HANDLE hEvent[2] = { hSem, pThis->m_hExitEvent };

	while (!pThis->m_bExit)
	{    
		DWORD dwRet = WaitForMultipleObjects(2, hEvent, FALSE, 100);

		switch (dwRet)
		{
		case WAIT_OBJECT_0:
			{
				QueueList qList;
				if (pThis->m_qRecv.GetHead(&qList) == 0)
				{
					pThis->AnalyseMsg(qList.buffer, qList.nLen, qList.sock);

					//itSvrSocket itsocket = pThis->m_Socket.m_mapSvrSocket.find(qList.sock);
					//if (itsocket == pThis->m_Socket.m_mapSvrSocket.end())
					//	break;

					//LPSvrSocket pSvrSocket = (LPSvrSocket)itsocket->second;
				}
				break;
			}
		case WAIT_OBJECT_0 + 1:
			// 退出线程
			return 0;
		case WAIT_TIMEOUT:
			break;
		}
	}

	return 0;
}

int MediaServer::init_socket_server()
{
	SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(server_socket == INVALID_SOCKET)
	{
		g_pLog->WriteLog("Init MediaServer Fail:%d\n", GetLastError());
		return FALSE;
	}
	unsigned long ul = 1;
	int nRet = ioctlsocket(server_socket, FIONBIO, (unsigned long *)&ul);
	if(nRet == SOCKET_ERROR)
	{
		g_pLog->WriteLog("rtspserv initialization failure(%d)!\n", GetLastError());
		closesocket(server_socket);
		return FALSE;
	}
	struct sockaddr_in server_sockaddr;						// 服务器的RTSP地址信息    
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(RF_STREAM_SERVER_PORT);	// RTSP的端口
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(server_socket,(struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == SOCKET_ERROR)
	{
		DWORD err = GetLastError();
		closesocket(server_socket);
		g_pLog->WriteLog("Init MediaServer Fail:bind[%d](%d)\n", RF_STREAM_SERVER_PORT, err);
		return FALSE;
	}
	if(listen(server_socket, MAX_NUM_CLIENT_MS) == SOCKET_ERROR)
	{	
		g_pLog->WriteLog("Init MediaServer Fail:listen(%d)\n", GetLastError());
		closesocket(server_socket);
		return FALSE;
	}
	
	_bind_socket = server_socket;
	return TRUE;
}
// 监听线程
int MediaServer::threadNetWork(LPVOID p_pJobParam)
{
	MediaServer* pThis = (MediaServer*)p_pJobParam;

	//int sinsize;
	//struct sockaddr clientaddr;		
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(_bind_socket, &rfds);

	int err=0;
	int ret = 0;
	while(true)
	{
		//
		ret = select(_bind_socket+1, &rfds, 0, 0, 0);
		if( ret == SOCKET_ERROR)
		{
			if(GetLastError() == WSAENOTSOCK)
			{
				FD_ZERO(&rfds);
				FD_SET(_bind_socket, &rfds);
				for(hash_map<SOCKET, AsyncSocketClient>::iterator iter = _client_map.begin();iter!=_client_map.end();)
				{
					if(iter->second.getSocket() != INVALID_SOCKET)
					{
						FD_SET(iter->second.getSocket(), &rfds);
						iter++;
					}
					else
					{
						iter = _client_map.erase(iter);
					}
				}
				continue;
			}
			else
			{
				closesocket(_bind_socket);
				_bind_socket = INVALID_SOCKET;
				g_pLog->WriteLog("RecvThread Exit:%d\n", GetLastError());
				break;
			}
		}
		else if(ret == 0)
			continue;

		//new connect
		if(FD_ISSET(_bind_socket,&rfds))
		{
			int s = accept(_bind_socket, 0, 0);
			if(s == INVALID_SOCKET)
				err = WSAGetLastError();
			else
			{
				_client_map[s] = AsyncSocketClient(s);
			}
		}
		else
		{
			//receive from client
			for(hash_map<SOCKET, AsyncSocketClient>::iterator iter = _client_map.begin();iter!=_client_map.end();)
			{
				if(FD_ISSET(iter->second.getSocket(),&rfds))
				{
					char* buffer;
					int recv_ret = iter->second.recvMSG(&buffer);
					if(recv_ret == TRUE)
					{
						// 压入接收队列里
						LpRFAppComand pAppCommand = (LpRFAppComand)(buffer);
						QueueList qList;
						qList.sock  = iter->second.getSocket();
						qList.nLen	= pAppCommand->GetPacketTotalSize();
						qList.buffer = new char[qList.nLen];
						memcpy(qList.buffer, buffer, pAppCommand->GetPacketTotalSize());
						pThis->m_qRecv.PutBack(qList);
						g_pLog->WriteLog("Receive a cmd(%d)\n", pAppCommand->nCommand);
					}

					if(recv_ret == SOCKET_ERROR)
					{
						g_pLog->WriteLog("RecvThread Client read ERROR, socket:%d\n",  iter->second.getSocket());
						iter = _client_map.erase(iter);
					}
					else
						iter++;
				}
				else
					iter++;
			}
		}

		FD_ZERO(&rfds);
		FD_SET(_bind_socket, &rfds);
		for(hash_map<SOCKET, AsyncSocketClient>::iterator iter = _client_map.begin();iter!=_client_map.end();)
		{
			if(iter->second.getSocket() != INVALID_SOCKET)
			{
				FD_SET(iter->second.getSocket(), &rfds);
				iter++;
			}
			else
			{
				iter = _client_map.erase(iter);
			}
		}

	}

	return 0;
}

int MediaServer::threadWrite(LPVOID p_pJobParam)
{
	MediaServer * pThis = (MediaServer *)p_pJobParam;
	HANDLE hSem = pThis->m_qSend.GetSemphore();
	HANDLE hEvent[2] = { hSem, pThis->m_hExitEvent };

	char szContent[1024] = {'\0'};
	DWORD dwRet;
	
	hash_map<SOCKET, SOCKET> wait_list;
	fd_set wfds;
	timeval time_out;
	time_out.tv_sec = 0;
	time_out.tv_usec = 30;

	while (!pThis->m_bExit)
	{    
		if(wait_list.size() != 0)
		{
			//发送未完成的数据
			FD_ZERO(&wfds);
			for(hash_map<SOCKET, SOCKET>::iterator iter = wait_list.begin();iter!=wait_list.end();iter++)
				FD_SET(iter->second, &wfds);
			int ret=select(_bind_socket, 0, &wfds, 0, &time_out);
			if(ret > 0)
			{
				for(hash_map<SOCKET, SOCKET>::iterator iter = wait_list.begin();iter!=wait_list.end();)
				{
					if(FD_ISSET(iter->second, &wfds))
					{
						int fret = _client_map[iter->second].flushSendData();
						 if(fret == TRUE)
						 {
							 g_pLog->WriteLog("RFMediaServ send data finish, socket :%d\n", iter->second);
							 iter = wait_list.erase(iter);
						 }
						 else if(fret == -1)
						 {
							 iter = wait_list.erase(iter);
						 }
						 else
							 iter++;
					}
					else
						iter++;
				}
			}
			else if(ret == SOCKET_ERROR)
			{
				for(hash_map<SOCKET, SOCKET>::iterator iter = wait_list.begin();iter!=wait_list.end();)
				{
					if(iter->second == INVALID_SOCKET)
						iter = wait_list.erase(iter);
					else
						iter++;
				}

				g_pLog->WriteLog("RFMediaServ send ERROR:%d\n",GetLastError());
			}
		}
		else
			dwRet = WaitForMultipleObjects(2, hEvent, FALSE, 100/*WSA_INFINITE*/);

		switch (dwRet)
		{
		case WAIT_OBJECT_0:
			{
				QueueList qList;
				if(pThis->m_qSend.GetHead(&qList) == 0)
				{
					if (qList.sock != 0)
					{
						int ret = _client_map[qList.sock].sendMSG(qList.buffer, qList.nLen);
						delete [] qList.buffer;
						if(ret == TRUE)
						{
							g_pLog->WriteLog("RFMediaServ send data finish, socket :%d, len:%d\n", qList.sock, qList.nLen);
						}
						else if ( ret == FALSE)
						{
							wait_list[qList.sock] = qList.sock;
							g_pLog->WriteLog("RFMediaServ send data delay, socket :%d\n", qList.sock);
						}
						else
						{
							g_pLog->WriteLog("ERROR: RFMediaServ send fail, socket :%d\n", qList.sock);
						}
					}
				}
				break;
			}
		case WAIT_OBJECT_0 + 1:
			//pThis->m_qSend.CleanData();		// 是否需要这边调用清理函数？
			return 0;
		case WAIT_TIMEOUT:
			break;
		default:
			break;
		}
	}

	return 0;
}

void MediaServer::RequestVRResource(char *recvBuff, int sock)
{
	BOOL bRet = FALSE;
	if (recvBuff == NULL)
		return;

	LpRFAppComand lpCommand = (LpRFAppComand)recvBuff;
	LpQueryVideoReocrdCommand lpRecordCommand = (LpQueryVideoReocrdCommand)(lpCommand + 1);
	MapQueryRecordResult mapRecord;
	g_dbobj.QueryRecord(mapRecord, lpRecordCommand->szStartTime, lpRecordCommand->szEndTime, lpRecordCommand->szKey );

	QueueList qList;
	if(mapRecord.empty())
	{
		char szSendBuff[1024] = {'\0'};
		LpRFAppComand lpAck = (LpRFAppComand)szSendBuff;
		lpAck->nPacketID = A_PROTOCOL_HEAD;
		lpAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
		lpAck->nCommand = RESP_VIDEO_RECORD_REQUEST_COMMAND;
		lpAck->nDataSize = sizeof(QueryVideoRecordCommandAck);//消息长度赋值
		LpQueryVideoRecordCommandAck lpAckRecord = (LpQueryVideoRecordCommandAck)(lpAck + 1);
		lpAckRecord->nTotalCount = 0;
		lpAckRecord->nCurrCount = -1;
		qList.nLen = lpAck->GetPacketTotalSize();
		qList.sock = sock;
		qList.buffer = new char[qList.nLen];
		if(qList.buffer ==NULL)
		{
			g_pLog->WriteLog("申请内存失败, %d\n", qList.nLen);
			return;
		}
		//拷贝消息头
		memcpy(qList.buffer, szSendBuff, sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck));
		//加入发送列表m_qSend
		m_qSend.PutBack(qList);
	}
	else
	{
		int count = 0;
		int oneSize = sizeof(QueryVideoRecord);
		int size = ( PACKET_MAX_SIZE - sizeof(QueryVideoRecordCommandAck) - sizeof(RFAppComand) ) / sizeof(QueryVideoRecord);

		QueryVRResource lpRecord[PACKET_MAX_SIZE] = {'\0'};
		BOOL hasSend = FALSE;
		int sendTotal=0;
		for(IterMapQueryRecordResult iterRecord = mapRecord.begin(); iterRecord != mapRecord.end(); iterRecord++)
		{
			LpQueryRecordResult lpQueryRecordResult = (LpQueryRecordResult)iterRecord->second;

			lpRecord[count].file_size = lpQueryRecordResult->nFileSize;
			sprintf_s(lpRecord[count].url, ":%d/%s/%s", g_pSysInfo->m_RtspPort, lpQueryRecordResult->strFolder.c_str(), lpQueryRecordResult->strRecordName.c_str());
			sprintf_s(lpRecord[count].file_name, "%s", lpQueryRecordResult->strRecordName.c_str());
			sprintf_s(lpRecord[count].start_time, "%s", lpQueryRecordResult->tStart.c_str());
			sprintf_s(lpRecord[count].end_time, "%s", lpQueryRecordResult->tEnd.c_str());		
			hasSend = FALSE;
			if( ( ++count ) >= size)
			{
				char szSendBuff[1024] = {'\0'};
				LpRFAppComand lpAck = (LpRFAppComand)szSendBuff;
				lpAck->nPacketID = A_PROTOCOL_HEAD;
				lpAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
				lpAck->nCommand = RESP_VIDEO_RECORD_REQUEST_COMMAND;
				lpAck->nDataSize = sizeof(QueryVideoRecordCommandAck) + ( count * sizeof(QueryVRResource) );

				LpQueryVideoRecordCommandAck lpAckRecord = (LpQueryVideoRecordCommandAck)(lpAck + 1);
				lpAckRecord->nTotalCount = mapRecord.size();
				lpAckRecord->nCurrCount = count;

				qList.nLen=lpAck->GetPacketTotalSize();
				qList.sock=sock;
				qList.buffer = new char[qList.nLen];
				if(qList.buffer == NULL)
				{
					g_pLog->WriteLog("申请内存失败, %d\n", qList.nLen);
					return;
				}
				memcpy(qList.buffer, szSendBuff, sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck));
				memcpy( qList.buffer + sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck), (char*)lpRecord, count * sizeof(QueryVRResource));
				m_qSend.PutBack(qList);

				count = 0;
				hasSend = TRUE;
#ifdef _DEBUG
				scount++;
#endif
			}
		}	

		if(count >= 0 && !hasSend)
		{
			char szSendBuff[1024] = {'\0'};
			LpRFAppComand lpAck = (LpRFAppComand)szSendBuff;
			lpAck->nPacketID = A_PROTOCOL_HEAD;
			lpAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
			lpAck->nCommand = RESP_VIDEO_RECORD_REQUEST_COMMAND;
			lpAck->nDataSize = sizeof(QueryVideoRecordCommandAck) + ( count * sizeof(QueryVRResource) );

			LpQueryVideoRecordCommandAck lpAckRecord = (LpQueryVideoRecordCommandAck)(lpAck + 1);
			lpAckRecord->nTotalCount = mapRecord.size();
			lpAckRecord->nCurrCount = count;

			qList.nLen=lpAck->GetPacketTotalSize();
			qList.sock=sock;
			qList.buffer = new char[qList.nLen];
			if(qList.buffer == NULL)
			{
				g_pLog->WriteLog("申请内存失败, %d\n", qList.nLen);
				return;
			}
			memcpy(qList.buffer, szSendBuff, sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck));
			memcpy( qList.buffer + sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck), (char*)lpRecord, count * sizeof(QueryVRResource));
			m_qSend.PutBack(qList);

		}
	}
	//g_pLog->WriteLog("已发送录像记录总数:%d\n",sendTotal);
}

//查询录像信息，附有ICON
void MediaServer::RequestVRResourceWithIcon(char *recvBuff, int sock)
{
	BOOL bRet = FALSE;
	if (recvBuff == NULL)
		return;

	//解析接收消息
	LpRFAppComand lpCommand = (LpRFAppComand)recvBuff;
	LpQueryVideoReocrdCommand lpRecordCommand = (LpQueryVideoReocrdCommand)(lpCommand + 1);

	//查询数据
	MapQueryRecordResult mapRecord;
	g_dbobj.QueryRecord(mapRecord, lpRecordCommand->szStartTime, lpRecordCommand->szEndTime, lpRecordCommand->szKey );
	
	for(IterMapQueryRecordResult iterRecord = mapRecord.begin(); iterRecord != mapRecord.end(); )
	{
		LpQueryRecordResult lpQueryRecordResult = (LpQueryRecordResult)iterRecord->second;
		char file_path[MAX_PATH]={0};
		sprintf_s(file_path, "%s/%s", lpQueryRecordResult->strFolder.c_str(), lpQueryRecordResult->strRecordName.c_str());
		if(_access(file_path,4)!=0)
			iterRecord = mapRecord.erase(iterRecord);
		else
			iterRecord++;
	}

	//组装发送消息并加入发送列表m_qSend
	QueueList qList;
	if(mapRecord.empty())
	{
		char szSendBuff[1024] = {'\0'};
		LpRFAppComand lpAck = (LpRFAppComand)szSendBuff;
		lpAck->nPacketID = A_PROTOCOL_HEAD;
		lpAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
		lpAck->nCommand = RESP_VIDEO_RECORD_REQUEST_COMMAND;
		lpAck->nDataSize = sizeof(QueryVideoRecordCommandAck);//消息长度赋值
		LpQueryVideoRecordCommandAck lpAckRecord = (LpQueryVideoRecordCommandAck)(lpAck + 1);
		lpAckRecord->nTotalCount = 0;
		lpAckRecord->nCurrCount = -1;
		qList.nLen = lpAck->GetPacketTotalSize();
		qList.sock = sock;
		qList.buffer = new char[qList.nLen];
		if(qList.buffer ==NULL)
		{
			g_pLog->WriteLog("申请内存失败, %d\n", qList.nLen);
			return;
		}
		//拷贝消息头
		memcpy(qList.buffer, szSendBuff, qList.nLen);
		//加入发送列表m_qSend
		m_qSend.PutBack(qList);
	}
	else
	{
		map<string,bool> iconlist;
		QueryVRResourceWithIcon lpRecord = {0};
		int i = 1;
		for(IterMapQueryRecordResult iterRecord = mapRecord.begin(); iterRecord != mapRecord.end(); iterRecord++,i++)
		{
			LpQueryRecordResult lpQueryRecordResult = (LpQueryRecordResult)iterRecord->second;

			lpRecord.file_size = lpQueryRecordResult->nFileSize;
			sprintf_s(lpRecord.url, ":%d/%s/%s", g_pSysInfo->m_RtspPort, lpQueryRecordResult->strFolder.c_str(), lpQueryRecordResult->strRecordName.c_str());
			sprintf_s(lpRecord.file_name, "%s", lpQueryRecordResult->strRecordName.c_str());
			sprintf_s(lpRecord.start_time, "%s", lpQueryRecordResult->tStart.c_str());
			sprintf_s(lpRecord.end_time, "%s", lpQueryRecordResult->tEnd.c_str());
			if(iconlist.find(lpQueryRecordResult->strDVRName) == iconlist.end())
			{
				//char icon_file[MAX_PATH] = {0};
				//sprintf_s(icon_file, "%s/%s.%s", lpQueryRecordResult->strFolder.c_str(), lpQueryRecordResult->strRecordName.c_str(), ICON_EXTENSION);
				//lpRecord.icon_size = read_file(icon_file, &(lpRecord.icon_data));
				//if(lpRecord.icon_size >0)
				//{
				//	iconlist[lpQueryRecordResult->strDVRName]=true;
				//}
				char vfile[MAX_PATH]={'\0'};
				sprintf_s(vfile, "%s/%s", lpQueryRecordResult->strFolder.c_str(), lpQueryRecordResult->strRecordName.c_str());
				lpRecord.icon_size = CGetIconFormFile::Instance()->getIcon(&(lpRecord.icon_data),vfile,lpQueryRecordResult->strProvider.c_str());
				if(lpRecord.icon_size >0)
				{
					iconlist[lpQueryRecordResult->strDVRName]=true;
				}
			}
			else
			{
				lpRecord.icon_size = 0;
				lpRecord.icon_data=NULL;
			}

			//组装消息头
			char szSendBuff[1024] = {'\0'};
			LpRFAppComand lpAck = (LpRFAppComand)szSendBuff;
			lpAck->nPacketID = A_PROTOCOL_HEAD;
			lpAck->nPacketNo = ((LpRFAppComand)recvBuff)->nPacketNo;
			lpAck->nCommand = RESP_VIDEO_RECORD_REQUEST_COMMAND;
			lpAck->nDataSize = sizeof(QueryVideoRecordCommandAck) + sizeof(QueryVRResourceWithIcon)  + lpRecord.icon_size;//消息长度赋值
			LpQueryVideoRecordCommandAck lpAckRecord = (LpQueryVideoRecordCommandAck)(lpAck + 1);
			lpAckRecord->nTotalCount = mapRecord.size();
			lpAckRecord->nCurrCount = i;

			//拷贝消息到发送缓存
			qList.nLen = lpAck->GetPacketTotalSize();
			qList.sock = sock;
			qList.buffer = new char[qList.nLen];
			if(qList.buffer ==NULL)
			{
				g_pLog->WriteLog("申请内存失败, %d\n", qList.nLen);
				return;
			}
			//拷贝消息头
			memcpy(qList.buffer, szSendBuff, sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck));
			//拷贝消息体
			memcpy( qList.buffer + sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck), &lpRecord, sizeof(QueryVRResourceWithIcon));
			//拷贝ICON
			if(lpRecord.icon_size > 0)
			{
				memcpy(qList.buffer + sizeof(RFAppComand) + sizeof(QueryVideoRecordCommandAck) + sizeof(QueryVRResourceWithIcon), lpRecord.icon_data, lpRecord.icon_size);
				delete [] lpRecord.icon_data;
			}
			//加入发送列表m_qSend
			m_qSend.PutBack(qList);
		}	
	}
	//g_pLog->WriteLog("已发送录像记录总数:%d\n",sendTotal);
}

//注意手动释放内存
int read_file(const char* name, char** buf)
{
	int size = 0;
	FILE* file = fopen(name, "rb");
	if(file != NULL)
	{
		fseek(file, 0L, SEEK_END);
		size = ftell(file);
		if(size > 0)
		{
			fseek(file, 0L, SEEK_SET);
			*buf = new char[size];
			if(*buf == NULL)
			{
				g_pLog->WriteLog("申请内存失败, %d\n", size);
				size = 0;
			}
			else if(fread(*buf, 1, size, file) != size)
			{
				delete [] *buf;
				g_pLog->WriteLog("读文件失败, %s(%d)\n", name, size);
				size = 0;
			}
		}
		fclose(file);
	}
	else
	{
		g_pLog->WriteLog("打开%s文件失败\n", name);
	}
	return size;
}
