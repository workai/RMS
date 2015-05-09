///////////////////////////////////////////////////////////////////
//               FileManip.h
//
// CFileManip, an API wrapping class that is developed to make file
// manipulation simpler and easier. 
//
// The class itself basically offers "DOS command like" methods which
// are very similar to related DOS commands in both names and 
// functionalities, I hope they bring you back to the good old time
// of DOS age.:-) Being able to using functions such as "xcopy" and
// "deltree" in my C++ code is what I always wanted to.
//
// Progress windows are provided automatically during lengthy tasks
// by operating system. This class does not require MFC, so it can be
// used in any Win32 applications. 
//
// Copyright 2002, Bin Liu
// All rights reserved.
//
// Version history:
//
// 7/28/2002 - Initial release. 
// 7/30/2002 - Use strncpy to duplicate strings instead of strcpy.
//             Suggested by jbarton.
// 2/08/2003 - The class has been made unicode-compliant.
//
///////////////////////////////////////////////////////////////////

#ifndef __FILEMANIP_H__
#define __FILEMANIP_H__

#include <tchar.h>
#include <windows.h> 
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <shellapi.h>

#include <shlobj.h>

// User difined error code, it will be returned by ::GetLastError()
// if a "file-only" operation attempts to modify a directory.
#define ERR_DIRECTORY	0x10000000

class CFileManip
{
public:

	// File-only operations
	static BOOL Copy(LPCTSTR lpSource, LPCTSTR lpDestination, BOOL bHidePrompt = TRUE);
	static BOOL Del(LPCTSTR lpSource, BOOL bHidePrompt = TRUE);
	static BOOL Ren(LPCTSTR lpSource, LPCTSTR lpDestination, BOOL bHidePrompt = TRUE);

	// File/directory operations
	static BOOL XCopy(LPCTSTR lpSource, LPCTSTR lpDestination, BOOL bHidePrompt = TRUE);
	static BOOL DelTree(LPCTSTR lpSource, BOOL bHidePrompt = TRUE);
	static BOOL Move(LPCTSTR lpSource, LPCTSTR lpDestination, BOOL bHidePrompt = TRUE);
	
	// Directory-only operations
	static BOOL MkDir(LPCTSTR lpDirectory);
	static BOOL RmDir(LPCTSTR lpDirectory);

	// File/directory attributes access
	static BOOL SetAttribute(LPCTSTR lpSource, DWORD dwNewAttr);
	static DWORD GetAttribute(LPCTSTR lpSource);

	// File/directory existence check
	enum { FM_NOTEXIST = 0, FM_DIRECTORY, FM_FILE};
	static int Existence(LPCTSTR lpSource);

	// Directory travelling and checking
	static BOOL CdDotDot(LPTSTR lpCurDirectory = NULL);
	static TCHAR IsRoot(LPCTSTR lpDirectory);
	static BOOL IsParentDirectory(LPCTSTR lpParent, LPCTSTR lpSubDirectory);

	/*special directory abtain*/
	//this is add by digua
	//fuc: get "<user name>\Application Data " directory
	static BOOL GetAppDirectory(LPTSTR lpAppDir);
	static BOOL GetExcutableDir(LPTSTR lpExcDir);
	static BOOL GetWorkingDir(LPTSTR lpWorDir);
};

#endif