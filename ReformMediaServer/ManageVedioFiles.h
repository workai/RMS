#ifndef _MANAGEVEDIOFILES_H_
#define _MANAGEVEDIOFILES_H_
#include <list>

using namespace std;

class CManageVedioFiles
{
private:
	typedef void* LPVOID;
	typedef void* HANDLE;

private:
	CManageVedioFiles();
	static CManageVedioFiles* _instance;
	HANDLE          fThreadID;

	static unsigned int __stdcall _Entry(LPVOID inThread);
public:
	//非线程安全
	static CManageVedioFiles* Instance()
	{
		if(!_instance)
			_instance = new CManageVedioFiles();
		return _instance;
	}

	bool getDiskStatus();
	int delRecords();
	void clearPaths();
	list<string> getFileNames();
	void getDirFiles(const char* dir, list<string>& files);
};



#endif