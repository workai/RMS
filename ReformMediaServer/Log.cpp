// Log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////
#include "ReformMediaServerDef.H"
#include "StdHeader.h"
#include "Log.h"
#include "SysInfo.h"
#include <iostream>

#include <string>

using namespace std;

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog()
{
	__fStdOut = NULL;
	__fStdErr=NULL;
	//__hStdOut = NULL;
//	return ;
	//startConsoleWin("fileserver");
}

CLog::~CLog()
{
//	return ;
	CloseWin();
}

void CLog::startConsoleWin(char* fLogName,char* fErrName)
{
	// 对应FileServer.ini配置文件
	if(g_pSysInfo == NULL)
	{
		return ;
	}

	// 是否记录日志
	if(g_pSysInfo->m_bLog != 1)
	{
		return ;
	}

	if(fLogName)
		__fStdOut = fopen(fLogName, "a+");
	if(fErrName)
		__fStdErr = fopen(fErrName, "a+");
}

void CLog::CloseWin()
{
	if(g_pSysInfo == NULL)
	{
		return ;
	}

	if(g_pSysInfo->m_bLog != 1)
	{
		return ;
	}

	/*FreeConsole();
	CloseHandle(__hStdOut);*/
//	::PostMessage(HWND_BROADCAST,WM_DESTROY,NULL,NULL);
	if(__fStdOut)
		fclose(__fStdOut);
	if(__fStdErr)
		fclose(__fStdErr);
}

int CLog::WriteErr(char *fmt, ...)
{
	if(g_pSysInfo == NULL)
	{// FileServer.ini配置文件
		return 0;
	}

	if(g_pSysInfo->m_bLog != 1)
	{// 不记录日志
		return 0;
	}

	// 类似于printf的函数，向Console写入文本
	char s[2048];
	va_list argptr;
	int cnt;	

	//DWORD cCharsWritten;

	// 2002/11/30日屏蔽时间显示
	DWORD errnum = GetLastError();
	char errstr[50];
	sprintf(errstr, " 错误代码: ( %d )", errnum);

	char infotime[30];
	SYSTEMTIME lotime;
	GetLocalTime(&lotime);
	sprintf(infotime,"[ %04.04d-%02.02d-%02.02d %02.02d:%02.02d:%02.02d ] ", lotime.wYear, lotime.wMonth, lotime.wDay, lotime.wHour, lotime.wMinute, lotime.wSecond);

	va_start(argptr, fmt);
	cnt = vsprintf(s, fmt, argptr);
	//va_start(argptr, infotime);
	//cnt = vsprintf(s, infotime, argptr);
	strcat(s, infotime);	
	va_end(argptr);
	
	string strMsg = infotime;
	strMsg += errstr;
	strMsg += s;

	std::cout<< strMsg;

//	char infotime[256];
//	SYSTEMTIME lotime;
//	GetLocalTime(&lotime);

	// 按日期生成文件名
	char Cur_Time[9];
	sprintf(Cur_Time, "%04.04d%02.02d%02.02d", lotime.wYear, lotime.wMonth, lotime.wDay);

	if(strcmp(Cur_Time, Pre_Time)==0)
	{
		if(__fStdErr)
		{
			fprintf(__fStdErr, s);
			fflush(__fStdErr);
			return(cnt);
		}
		else
		{
			sprintf(infotime, "%s%s.txt", "err-", Cur_Time);
			__fStdErr = fopen(infotime, "a+");

			if(__fStdErr)
				fprintf(__fStdErr, s);
			fflush(__fStdErr);
			return(cnt);
		}
	}
	else
	{
		sprintf(Pre_Time, Cur_Time);
		if(__fStdErr)
		{
			fclose(__fStdErr);
			__fStdErr=NULL;
		}

		sprintf(infotime, "%s%s.txt", "err-", Cur_Time);
		__fStdErr = fopen(infotime, "a+");

		if(__fStdErr)
			fprintf(__fStdErr, s);
		fflush(__fStdErr);
		return(cnt);
	}	
	return 0;
}

int CLog::WriteLogf(char* fmt1, long fmt2, char *fmt, ...)//写日志函数。
{
	//取消日志，因为写日志出错
//#ifndef _DEBUG
//	return 0;
//#endif

	if(g_pSysInfo == NULL)
	{
		return 0;
	}

	if(g_pSysInfo->m_bLog == 1)
	{// 不记录日志
		return 0;
	}

	// 类似于printf的函数，向Console写入文本
	char s[2048];
	va_list argptr;
	int cnt;
	
	//DWORD cCharsWritten;
	//2002/11/30日屏蔽时间显示
	char infotime[MAX_PATH];
	SYSTEMTIME lotime;
	GetLocalTime(&lotime);
	sprintf_s(infotime, sizeof(infotime),"[ %04.04d-%02.02d-%02.02d %02.02d:%02.02d:%02.02d ] ", lotime.wYear, lotime.wMonth, lotime.wDay, lotime.wHour, lotime.wMinute, lotime.wSecond);

	va_start(argptr, fmt);
	cnt = vsprintf(s, fmt, argptr);
	va_end(argptr);
			
	string strMsg = infotime;
	char s0[MAX_PATH];
	sprintf_s(s0, sizeof(s0), "%s:%d ", fmt1, fmt2);
	strMsg += s0;
	strMsg += s;

	std::cout<< strMsg;

//	char infotime[256];
//	SYSTEMTIME lotime;
//	GetLocalTime(&lotime);

	// 生成文件名
	char Cur_Time[9] = {0};
	sprintf(Cur_Time,"%04.04d%02.02d%02.02d", lotime.wYear, lotime.wMonth, lotime.wDay);

	if(strcmp(Cur_Time, Pre_Time)==0)
	{
		if(__fStdOut)
		{
			fprintf(__fStdOut, strMsg.c_str());
			fflush(__fStdOut);
			return(cnt);
		}
		else
		{
			sprintf(infotime, "%s//log-%s.txt", GetAppPath(), Cur_Time);
			__fStdOut = fopen(infotime, "a+");

			if(__fStdOut)
				fprintf(__fStdOut, s);
			fflush(__fStdOut);
			return(cnt);
		}
	}
	else
	{
		sprintf(Pre_Time,Cur_Time);
		if(__fStdOut)
		{
			fclose(__fStdOut);
			__fStdOut=NULL;
		}

		sprintf(infotime,"%s//log-%s.txt", GetAppPath(), Cur_Time);
		__fStdOut = fopen(infotime, "a+");

		if(__fStdOut)
			fprintf(__fStdOut, s);
		fflush(__fStdOut);
		return(cnt);
	}	
	return 0;
}