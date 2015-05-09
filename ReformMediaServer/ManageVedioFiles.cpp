#include "ManageVedioFiles.h"
#include "ReformMediaServerDef.h"
#include "Log.h"
#include "DBObject.h"
#include "Config.h"
#include <process.h>

CManageVedioFiles* CManageVedioFiles::_instance= NULL;

CManageVedioFiles::CManageVedioFiles()
{
	unsigned int theId = 0;
	fThreadID = (HANDLE)_beginthreadex( NULL,   // Inherit security
		0,      // Inherit stack size
		_Entry, // Entry function
		(void*)this,    // Entry arg
		0,      // Begin executing immediately
		&theId );
}

unsigned int WINAPI CManageVedioFiles::_Entry(LPVOID inThread)
{
	string id = "";
	char path[MAX_PATH] = {0};
	CManageVedioFiles* manager = (CManageVedioFiles*)inThread;
	manager->clearPaths();
	while(true)
	{
		if(!g_xmlConfig.get_record_path(30, id , path))
		{
			manager->delRecords();
		}
		else
			Sleep(2000);
	}
}

int CManageVedioFiles::delRecords()
{
	int ret = 1;
	MapQueryRecordResult mapRecord;
	char file_name[MAX_PATH]={0};
	int qid = g_dbobj.getFirstFile(file_name);
	if(qid >= 0)
	{
		g_dbobj.DeleteRecord(qid);
		int err = remove(file_name);
		if(err == 0)
		{
			g_pLog->WriteLog("删除录像文件%s\n", file_name);
		}
		else
		{
			ret = 0;
			if(errno == EACCES)
			{
				g_pLog->WriteLog("ERROR: 删除录像文件失败 %s 文件为只读或正在使用\n", file_name);
			}
			else
			{
				//ENOENT 
				g_pLog->WriteLog("WARN: 删除录像文件失败 %s 文件不存在, errno:%d\n", file_name, errno); 
			}
		}
	}
	else
	{
		ret = -1;
		g_pLog->WriteLog("WARN:无法释放硬盘空间，数据库为空\n");
	}
	return ret;
}

void CManageVedioFiles::getDirFiles(const char* dir, list<string>& files)
{
	char szFind[MAX_PATH]={0};
	WIN32_FIND_DATA FindFileData;
	strcpy_s(szFind, dir);
	strcat_s(szFind,"\\*.*");
	HANDLE hFind=::FindFirstFile(szFind,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)   
	{
		return;
	}

	do
	{
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(strcmp(FindFileData.cFileName,".")!=0 && strcmp(FindFileData.cFileName, "..")!=0)
			{
				//发现子目录，递归之
				char szFile[MAX_PATH] = {0};
				strcpy(szFile,dir);
				strcat(szFile,"\\");
				strcat(szFile,FindFileData.cFileName);
				getDirFiles(szFile, files);
			}
		}
		else
		{
			//找到文件，处理之
			char tf[MAX_PATH]={0};
			sprintf_s(tf,"%s\\%s",dir, FindFileData.cFileName);
			string filename(tf);
			files.push_back(filename);
		}
	}while(::FindNextFile(hFind,&FindFileData));

	::FindClose(hFind);
}

list<string> CManageVedioFiles::getFileNames()
{
	list<string> files;
	char apath[1024] = {};
	strcpy(apath, g_xmlConfig.getRecordPaths().c_str());
	char* epath = strtok(apath,";");
	while(epath!=NULL)
	{
		getDirFiles(epath, files);
		epath = strtok(NULL,";");
	}
	return files;
}
void CManageVedioFiles::clearPaths()
{
	list<string> files = getFileNames();
	for(list<string>::iterator iter=files.begin();iter!=files.end();iter++)
	{
		char* file;
		char name[MAX_PATH];
		strcpy_s(name, (*iter).c_str());
		file=strrchr(name,'\\');
		if(!g_dbobj.isFileInRecord(file+1))
		{
			int err = remove((*iter).c_str());
			if(err != 0)
			{
				g_pLog->WriteLog("删除录像文件失败%s\n", *iter);
			}
		}
	}
}