
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#include "ReformMediaServer.h"
#include "ReformMediaServerDef.H"
#include <stdlib.h>
#include "ManagerRtsp.h"
#include "ManagerDVR.h"
#include "ManagerConnect.h"
#include "StdHeader.h"
#include "MediaServer.h"
#include "DBObject.h"
#include "Config.h"
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>
#include "getopt.h"
#include "ManageVedioFiles.h"
#include "PushServer.h"

using namespace std;

// CSysInfo *g_pSysInfo = NULL;


#pragma comment (lib, "Dbghelp")

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	char path[MAX_PATH];         
	GetModuleFileName( NULL, path, MAX_PATH);
	(_tcsrchr(path, _T('\\')))[1] = 0;
	CString curpath;
	curpath.Format("%s", path);
	curpath = curpath + "\\DumpFile.dmp";

	HANDLE lhDumpFile = CreateFile(curpath,GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ClientPointers = TRUE;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

	CloseHandle(lhDumpFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

static void InstallService(char* inServiceName);
static void RemoveService(char *inServiceName);
static void RunAsService(char* inServiceName);
static SERVICE_STATUS_HANDLE sServiceStatusHandle = 0;
void __stdcall ServiceMain(DWORD /*argc*/, LPTSTR *argv);
void RunServer();

int main(int argc, char * argv[]) 
{
	//使用minidump进行程序异常调试
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	bool IsService=false;

	int ch;
	while ((ch = getopt(argc,argv, "vdxp:o:c:irsS:I")) != EOF)
	{
		switch(ch)
		{
		case 'i':
			printf("Installing the ReformMediaServer service...\n");
			::InstallService("ReformMediaServer");
			printf("Starting the ReformMediaServer...\n");
			::RunAsService("ReformMediaServer");
			::exit(0);
			break;
		case 'r':
			printf("Removing the ReformMediaServer service...\n");
			::RemoveService("ReformMediaServer");
			::exit(0);
		case 's':
			printf("Starting the ReformMediaServer service...\n");
			::RunAsService("ReformMediaServer");
			::exit(0);
		case 'd':
			IsService=true;
			break;
		default:
			break;
		}
	}

	if(IsService)
	{
		///BOOL ret =SetProcessShutdownParameters(0x3ff, SHUTDOWN_NORETRY);
		//BOOL ret =SetConsoleCtrlHandler( (PHANDLER_ROUTINE) ctrlhandler, TRUE );
		RunServer();
		::exit(0);
	}
	else
	{
		SERVICE_TABLE_ENTRY dispatchTable[] =
		{
			{ "", ServiceMain },
			{ NULL, NULL }
		};

		printf("Darwin Streaming Server must either be started from the DOS Console\n");
		printf("using the -d command-line option, or using the Service Control Manager\n\n");
		printf("Waiting for the Service Control Manager to start Darwin Streaming Server...\n");
		BOOL theErr = ::StartServiceCtrlDispatcher(dispatchTable);
		if (!theErr)
		{
			printf("Fatal Error: Couldn't start Service\n");
			::exit(-1);
		}
	}
	return 0;
}

void RunAsService(char* inServiceName)
{
	SC_HANDLE   theService;
	SC_HANDLE   theSCManager;

	theSCManager = ::OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
		);
	if (!theSCManager)
		return;

	theService = ::OpenService(
		theSCManager,               // SCManager database
		inServiceName,               // name of service
		SERVICE_ALL_ACCESS );

	SERVICE_STATUS lpServiceStatus;

	if (theService)
	{   const signed long kNotRunning = 1062;
	unsigned short stopped = ::ControlService(theService, SERVICE_CONTROL_STOP, &lpServiceStatus);
	if(!stopped && ( (signed long) ::GetLastError() != kNotRunning) )
		printf("Stopping Service Error: %d\n", ::GetLastError());

	unsigned short started = ::StartService(theService, 0, NULL);
	if(!started)
		printf("Starting Service Error: %d\n", ::GetLastError());

	::CloseServiceHandle(theService);
	}

	::CloseServiceHandle(theSCManager);
}


void InstallService(char* inServiceName)
{
	SC_HANDLE   theService;
	SC_HANDLE   theSCManager;

	TCHAR thePath[512];
	TCHAR theQuotedPath[522];

	BOOL theErr = ::GetModuleFileName( NULL, thePath, 512 );
	if (!theErr)
		return;

	sprintf_s(theQuotedPath, "\"%s\"", thePath);

	theSCManager = ::OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
		);
	if (!theSCManager)
	{
		printf("Failed to install Darwin Streaming Server Service\n");
		return;
	}

	theService = CreateService(
		theSCManager,               // SCManager database
		inServiceName,               // name of service
		inServiceName,               // name to display
		SERVICE_ALL_ACCESS,         // desired access
		SERVICE_WIN32_OWN_PROCESS,  // service type
		SERVICE_AUTO_START,       // start type
		SERVICE_ERROR_NORMAL,       // error control type
		theQuotedPath,               // service's binary
		NULL,                       // no load ordering group
		NULL,                       // no tag identifier
		NULL,       // dependencies
		NULL,                       // LocalSystem account
		NULL);                      // no password

	if (theService)
	{
		::CloseServiceHandle(theService);
		printf("Installed Darwin Streaming Server Service\n");
	}
	else
		printf("Failed to install Darwin Streaming Server Service\n");

	::CloseServiceHandle(theSCManager);
}

void RemoveService(char *inServiceName)
{
	SC_HANDLE   theSCManager;
	SC_HANDLE   theService;

	theSCManager = ::OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
		);
	if (!theSCManager)
	{
		std::printf("Failed to remove Darwin Streaming Server Service\n");
		return;
	}

	theService = ::OpenService(theSCManager, inServiceName, SERVICE_ALL_ACCESS);
	if (theService != NULL)
	{
		unsigned short stopped = ::ControlService(theService, SERVICE_CONTROL_STOP, NULL);
		if(!stopped)
			printf("Stopping Service Error: %d\n", ::GetLastError());

		(void)::DeleteService(theService);
		::CloseServiceHandle(theService);
		printf("Removed Darwin Streaming Server Service\n");
	}
	else
		printf("Failed to remove Darwin Streaming Server Service\n");

	::CloseServiceHandle(theSCManager);  
}

void ReportStatus(DWORD inCurrentState, DWORD inExitCode)
{
    static unsigned short sFirstTime = 1;
    static unsigned long sCheckpoint = 0;
    static SERVICE_STATUS sStatus;
    
    if(sFirstTime)
    {
        sFirstTime = false;
        
        //
        // Setup the status structure
        sStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        sStatus.dwCurrentState = SERVICE_START_PENDING;
        //sStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
        sStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        sStatus.dwWin32ExitCode = 0;
        sStatus.dwServiceSpecificExitCode = 0;
        sStatus.dwCheckPoint = 0;
        sStatus.dwWaitHint = 0;
    }

    if (sStatus.dwCurrentState == SERVICE_START_PENDING)
        sStatus.dwCheckPoint = ++sCheckpoint;
    else
        sStatus.dwCheckPoint = 0;
    
    sStatus.dwCurrentState = inCurrentState;
    sStatus.dwServiceSpecificExitCode = inExitCode;
    BOOL theErr = SetServiceStatus(sServiceStatusHandle, &sStatus);
    if (theErr == 0)
    {
        DWORD theerrvalue = ::GetLastError();
    }
}

static DWORD _threadIDMain=NULL;
void WINAPI ServiceControl(DWORD inControlCode)
{
	DWORD theStatusReport = SERVICE_START_PENDING;
	switch(inControlCode)
	{
		// Stop the service.
		//
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		 theStatusReport = SERVICE_STOP_PENDING;
		 PostThreadMessage(_threadIDMain,WM_QUIT,NULL,NULL);
		 break;
	default:
		theStatusReport = SERVICE_RUNNING;
		break;
	}
	::ReportStatus(theStatusReport, NO_ERROR);
}

void __stdcall ServiceMain(DWORD /*argc*/, LPTSTR *argv)
{    
#ifdef _DEBUG
	Sleep(10000);
#endif

	char* theServerName = argv[0];
    sServiceStatusHandle = ::RegisterServiceCtrlHandler( theServerName, &ServiceControl);
    if (sServiceStatusHandle == 0)
    {
        printf("Failure registering service handler");
        return;
    }

	::ReportStatus( SERVICE_RUNNING, NO_ERROR );
	HRESULT hr= CoInitialize(NULL);
	_threadIDMain = GetCurrentThreadId();
	RunServer();
	CoUninitialize();
	::ReportStatus( SERVICE_STOPPED, NO_ERROR );
}

void RunServer()
{
	// 进入系统
	g_pSysInfo = new CSysInfo();
	g_pLog = new CLog();

	g_pLog->WriteLog("start media server...\n");

	if(!BuildWindow())
	{
		g_pLog->WriteLog("start failed!!!\n");
		return;
	}

	WSADATA wsd;
	if(WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		g_pLog->WriteLog("start fail : binding port error\n");
		return ;
	}

	// 加载数据库数据
	g_dbobj.Init();

	// 测试 xml文件的读写
	//g_xmlConfig.LoadRecordSettings();
	//g_xmlConfig.SaveRecordSettings();
	g_xmlConfig.Init();

	CManageVedioFiles::Instance();

	// rtsp客户端管理类
	g_pManagerRtsp = new CManagerRtsp();
	if(g_pManagerRtsp->Init() == FALSE)
	{
		g_pLog->WriteLog("服务开启失败，退出...\n");
		Sleep(5000);
		return;
	}

	// 录像文件下载管理类
	g_pManagerConnect = new CManagerConnect();
	if(g_pManagerConnect->InitListenSocket() == FALSE)
	{
		g_pLog->WriteLog("服务开启失败，退出...\n");
		Sleep(5000);
		return;
	}

	// 视频数据获取类
	//g_pLog->WriteLog("正在登陆DVR......\n");
	g_pManagerDVR = new CManagerDVR();
	g_pManagerDVR->Init();

	CPushServer::init(g_pSysInfo->m_PushPort);

	g_pLog->WriteLog("reform media server start successfully!\n");

		// 建立即时配置网络服务
	if(g_servObj.Start() == FALSE)
	{
		g_pLog->WriteLog("服务开启失败，退出...\n");
		Sleep(5000);
		return;
	}

	BOOL bRet;
	MSG msg; 
	try
	{
		while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
		{ 
			if (bRet == -1)
			{
				// handle the error and possibly exit
			}
			else
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			}
		}
	}
	catch(...)
	{
	}
	char a[20]; 
	std::cin>>a;
}