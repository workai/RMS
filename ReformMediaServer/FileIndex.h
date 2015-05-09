#pragma once

#include "StdHeader.h"
#include "ReformMediaServerDef.h"

class CFileIndex
{
public:
	CFileIndex(void);
public:
	~CFileIndex(void);
	
	BOOL Init(_int64 starttimepos, _int64 endtimepos, char *channelid);

	// CArray<FILE_INFO , FILE_INFO> m_FileArray;
	ATL::CSimpleArray<FILE_INFO , FILE_INFO> m_FileArray;

	BOOL ReadIdxFileInfo(HANDLE hfile,IDXFILEHEAD_INFO &info);
	BOOL ReadNextBlockInfo(HANDLE hfile,IDXFILEBLOCK_INFO &info);
	BOOL ReadNextTimeStampInfo(HANDLE hfile,IDXSTAMP_INFO &info);

	//CArray<FILESTAMP_INFO,FILESTAMP_INFO> m_StampArray;
	ATL::CSimpleArray<FILESTAMP_INFO,FILESTAMP_INFO> m_StampArray;

};
