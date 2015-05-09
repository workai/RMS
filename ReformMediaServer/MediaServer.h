#pragma once

//#include "ReformMediaServerDef.h"
#include "ServerSocket.h"
#include "queuelist.h"
//#include "device.h"
#include <string>
#include <WinSock2.h>
#include <hash_map>
#include "async_socket_client.h"

using namespace std;

#define RF_STREAM_SERVER_PORT 9000
#define MAX_NUM_CLIENT_MS 100
//---------------------------------------
// 即时通信数据队列
typedef struct _QueueList
{
	int sock;			// Socket(仅对于服务端有意义)
	unsigned int nLen;	// 数据包长度
	char* buffer;	// 数据包

	_QueueList()
	{
		sock	= 0;
		nLen	= 0;
		//memset(buffer, 0, sizeof(buffer));
		buffer = NULL;
	}
} QueueList, *LPQueueList;

////---------------------------------------
//// client信息
//typedef struct _RFMediaClient
//{
//	int clientId;		// 生成的编号
//	int sock;			// Socket(仅对于服务端有意义)
//
//	_RFMediaClient()
//	{
//
//	}
//} RFMediaClient, *LPRFMediaClient;

// 即时配置服务器
class MediaServer
{
public:
	MediaServer(void);
	~MediaServer(void);

	// 初始化网络服务
	int Start();

private:
	

	void RequestVRResource(char *recvBuff, int sock);
	void RequestVRResourceWithIcon(char *recvBuff, int sock);

	static SOCKET _bind_socket;
	static int threadWrite(LPVOID p_pJobParam);

	static hash_map<SOCKET, AsyncSocketClient> _client_map;
	static int threadNetWork(LPVOID p_pJobParam);

	//static int ThreadListe(LPVOID p_pJobParam);
	//static int ThreadRecv(LPVOID p_pJobParam);
	//static int ThreadSend(LPVOID p_pJobParam);
	static int ThreadProcessData(LPVOID p_pJobParam);

	int AnalyseMsg(char *buff, int len, int sock);

	BOOL OnRegist(char *recvBuff, int sock);
	void OnHandshake(char *recvBuff, int sock);
	void OnUpdateDevice(char *recvBuff, int sock);
	BOOL CreateUpdateDeviceSql(char* buff, list<string>& sqlList);
	void OnUpdateConfig(char *recvBuff, int sock);
	void OnRequestDevices(char *recvBuff, int sock);
	void OnRequestVideoRecords(char *recvBuff, int sock);
	int GetServerIPAddress();
private:

	BOOL	m_bExit;			// 退出标识
	HANDLE	m_hExitEvent;		// 退出信号
	//HANDLE  m_hPrepareEvent;	// 启动接收线程信号
	
	string m_strServAddr;

	CServerSocket m_Socket;
	CRITICAL_SECTION m_CriticalSection;

	CQBuffList<QueueList> m_qSend;
	CQBuffList<QueueList> m_qRecv;


	int init_socket_server();
};

extern MediaServer g_servObj;
