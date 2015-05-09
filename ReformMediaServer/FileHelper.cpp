///////////////////////////////////////////////////////////////////
//               FileManip.cpp
//
// Implementation of CFileManip class.
//
// Copyright 2002, Bin Liu
// All rights reserved.
//
// Version history:
//
// 7/28/2002 - Initial release. 
// 7/30/2002 - Use strncpy to duplicate strings instead of ::_tcscpy.
//             Suggested by jbarton.
// 2/08/2003 - The class has been made unicode-compliant.
//
///////////////////////////////////////////////////////////////////

//#include "stdafx.h" // Only include if is used in MFC projects
#include "FileHelper.h"

#ifndef	MAX_PATH
	#define MAX_PATH	255
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////
// File-level operations
/////////////////////////////////////////////////////////////////
BOOL CFileManip::Copy(LPCTSTR lpSource, LPCTSTR lpDestination, BOOL bHidePrompt)
{
	// Enlightened by jbarton on Codeproject.com:
	// SHFileOperation expects character strings with "double-null terminators",
	// thus LPCTSTR's that are passed in as parameters cannot be directly used
	// by SHFileOperation. We need to duplicate the strings in order to make sure
	// that they are "double-null terminated".
	TCHAR szSource[MAX_PATH + 2] = _T("");
	TCHAR szDestination[MAX_PATH + 2] = _T("");
	::_tcsncpy(szSource, lpSource, MAX_PATH);
	::_tcsncpy(szDestination, lpDestination, MAX_PATH);

	// Copy should not be able to affect any directories, just like
	// how "copy" works in DOS. If need to copy directories, use 
	// XCopy instead.
	if (Existence(szSource) == FM_DIRECTORY)
	{
		::SetLastError(ERR_DIRECTORY);
		return FALSE;
	}

	SHFILEOPSTRUCT fs;
	::memset(&fs, 0, sizeof(SHFILEOPSTRUCT));

	fs.pFrom = szSource;
	fs.pTo = szDestination;
	fs.wFunc = FO_COPY;
	fs.fFlags = FOF_FILESONLY;

	if (bHidePrompt)
		fs.fFlags |= (FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR);

	// This does the real job...
	return (::SHFileOperation(&fs) == 0);
}

BOOL CFileManip::Del(LPCTSTR lpSource, BOOL bHidePrompt)
{
	TCHAR szSource[MAX_PATH + 2] = _T("");
	::_tcsncpy(szSource, lpSource, MAX_PATH);

	// Del should not be able to affect any directories, just like
	// how "del" works in DOS. If need to delete directories, use 
	// DelTree instead.
	if (Existence(szSource) == FM_DIRECTORY)
	{
		::SetLastError(ERR_DIRECTORY);
		return FALSE;
	}

	SHFILEOPSTRUCT fs;
	::memset(&fs, 0, sizeof(SHFILEOPSTRUCT));

	fs.pFrom = szSource;
	fs.wFunc = FO_DELETE;
	fs.fFlags = FOF_FILESONLY;

	if (bHidePrompt)
		fs.fFlags |= (FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR);

	return (::SHFileOperation(&fs) == 0);
}

BOOL CFileManip::Ren(LPCTSTR lpSource, LPCTSTR lpDestination, BOOL bHidePrompt)
{
	TCHAR szSource[MAX_PATH + 2] = _T("");
	TCHAR szDestination[MAX_PATH + 2] = _T("");
	::_tcsncpy(szSource, lpSource, MAX_PATH);
	::_tcsncpy(szDestination, lpDestination, MAX_PATH);

	// Ren should not be able to affect any directories, just like
	// how "ren" works in DOS. If need to rename directories, use 
	// Move instead.
	if (Existence(szSource) == FM_DIRECTORY)
	{
		::SetLastError(ERR_DIRECTORY);
		return FALSE;
	}

	SHFILEOPSTRUCT fs;
	::memset(&fs, 0, sizeof(SHFILEOPSTRUCT));

	fs.pFrom = szSource;
	fs.pTo = szDestination;
	fs.wFunc = FO_RENAME;
	fs.fFlags = FOF_FILESONLY;
	
	if (bHidePrompt)
		fs.fFlags |= (FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR);

	return (::SHFileOperation(&fs) == 0);
}


/////////////////////////////////////////////////////////////////
// Subdirectory affected operations
/////////////////////////////////////////////////////////////////
BOOL CFileManip::XCopy(LPCTSTR lpSource, LPCTSTR lpDestination, BOOL bHidePrompt)
{
	TCHAR szSource[MAX_PATH + 2] = _T("");
	TCHAR szDestination[MAX_PATH + 2] = _T("");
	::_tcsncpy(szSource, lpSource, MAX_PATH);
	::_tcsncpy(szDestination, lpDestination, MAX_PATH);

	SHFILEOPSTRUCT fs;
	::memset(&fs, 0, sizeof(SHFILEOPSTRUCT));

	fs.pFrom = szSource;
	fs.pTo = szDestination;
	fs.wFunc = FO_COPY;
	
	if (bHidePrompt)
		fs.fFlags |= (FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR);

	return ::SHFileOperation(&fs) == 0;
}

BOOL CFileManip::DelTree(LPCTSTR lpSource, BOOL bHidePrompt)
{
	TCHAR szSource[MAX_PATH + 2] = _T("");
	::_tcsncpy(szSource, lpSource, MAX_PATH);

	SHFILEOPSTRUCT fs;
	::memset(&fs, 0, sizeof(SHFILEOPSTRUCT));

	fs.pFrom = szSource;
	fs.wFunc = FO_DELETE;

	if (bHidePrompt)
		fs.fFlags |= (FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR);

	// If lpSource or any of its subdirectories is the current
	// directory, we need to browse backward until the current
	// directory is parent of lpSource, otherwise operation will
	// fail.
	TCHAR szCurDir[MAX_PATH + 1] = _T("");
	::GetCurrentDirectory(MAX_PATH, szCurDir);

	// Keeps travelling backward until current directory becomes parent
	// or root directory is reached.
	BOOL bNotRoot = TRUE;
	while (bNotRoot && !IsParentDirectory(szCurDir, szSource))
	{
		bNotRoot = CdDotDot(szCurDir);
	}
	
	return (::SHFileOperation(&fs) == 0);
}

BOOL CFileManip::Move(LPCTSTR lpSource, LPCTSTR lpDestination, BOOL bHidePrompt)
{
	// Move files or directories from lpSource to lpDestination,
	// it may also be used to rename a directory.
	TCHAR szSource[MAX_PATH + 2] = _T("");
	TCHAR szDestination[MAX_PATH + 2] = _T("");
	::_tcsncpy(szSource, lpSource, MAX_PATH);
	::_tcsncpy(szDestination, lpDestination, MAX_PATH);

	SHFILEOPSTRUCT fs;
	::memset(&fs, 0, sizeof(SHFILEOPSTRUCT));

	fs.pFrom = szSource;
	fs.pTo = szDestination;
	fs.wFunc = FO_MOVE;

	if (bHidePrompt)
		fs.fFlags |= (FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR);

	// If lpSource or any of its subdirectories is the current
	// directory, we need to browse backward until the current
	// directory is parent of lpSource, otherwise operation will
	// fail.
	TCHAR szCurDir[MAX_PATH + 1] = _T("");
	::GetCurrentDirectory(MAX_PATH, szCurDir);

	// Keeps travelling backward until current directory becomes parent
	// or root directory is reached.
	BOOL bNotRoot = TRUE;
	while (bNotRoot && !IsParentDirectory(szCurDir, szSource))
	{
		bNotRoot = CdDotDot();
		::GetCurrentDirectory(MAX_PATH, szCurDir);
	}

	return (::SHFileOperation(&fs) == 0);
}

// Create a directory using specified name
BOOL CFileManip::MkDir(LPCTSTR lpDirectory)
{
	return ::CreateDirectory(lpDirectory, NULL);
}

BOOL CFileManip::RmDir(LPCTSTR lpDirectory)
{
	return ::RemoveDirectory(lpDirectory);
}


/////////////////////////////////////////////////////////////////
// Attributes access -- Set and get file/directory attributes
/////////////////////////////////////////////////////////////////
BOOL CFileManip::SetAttribute(LPCTSTR lpSource, DWORD dwNewAttr)
{
	return ::SetFileAttributes(lpSource, dwNewAttr);
}

DWORD CFileManip::GetAttribute(LPCTSTR lpSource)
{
	return ::GetFileAttributes(lpSource);
}


/////////////////////////////////////////////////////////////////
// Helper functions
/////////////////////////////////////////////////////////////////

// check for file/directory existences
int CFileManip::Existence(LPCTSTR lpSource)
{
	WIN32_FIND_DATA fd;
	::memset(&fd, 0, sizeof(WIN32_FIND_DATA));

	HANDLE hFile = ::FindFirstFile(lpSource, &fd);

	// Not exists.
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FM_NOTEXIST;
	}

	// Exists, but file or directory?
	
	if (fd.dwFileAttributes == (fd.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY))
	{
		while (::FindNextFile(hFile, &fd))
		{
			// There is at least a file with this name.
			if (fd.dwFileAttributes != (fd.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY))
			{
				::FindClose(hFile);
				return FM_FILE;
			}
		}

		// Exists but there's no file with this name, so it's a directory.
		::FindClose(hFile);
		return FM_DIRECTORY;
	}
	else
	{
		// Exists and is not a directory, so it's a file.
		::FindClose(hFile);
		return FM_FILE;
	}
}

// Travel backward in directory structure
BOOL CFileManip::CdDotDot(LPTSTR lpCurDirectory)
{
	// Travel backward one directory level, same as DOS
	// command "cd.." does.
	TCHAR szCurDir[MAX_PATH + 1] = _T("");
	::GetCurrentDirectory(MAX_PATH, szCurDir);
	if (IsRoot(szCurDir) != 0)
		return FALSE; // root reached
	
	int nLen = ::_tcslen(szCurDir);
	for (int i = nLen - 1; i >= 0; i--)
	{
		if (szCurDir[i] == _T('\\'))
		{
			szCurDir[i] = _T('\0');
			if (lpCurDirectory != NULL)
				::_tcscpy(lpCurDirectory, szCurDir);

			if (IsRoot(szCurDir) != 0)
				return FALSE; // root reached

			::SetCurrentDirectory(szCurDir);
			return TRUE; // Moved backward successfully
		}
	}
	
	if (lpCurDirectory != NULL)
		::_tcscpy(lpCurDirectory, szCurDir);
	return FALSE; // Already at root directory
}

// check if one directory is parent of the other
BOOL CFileManip::IsParentDirectory(LPCTSTR lpParent, LPCTSTR lpSubDirectory)
{
	UINT nLenParent = ::_tcslen(lpParent);
	UINT nLenSubDirectory = ::_tcslen(lpSubDirectory);
	if (nLenParent >= nLenSubDirectory - 1)
		return FALSE;

	// compare two strings and check if one is parent directory of the other
	UINT i;
	for (i = 0; i < nLenParent; i++)
	{
		if (toupper(lpParent[i]) != toupper(lpSubDirectory[i]))
			return FALSE;
	}

	return lpSubDirectory[i] == _T('\\');	
}

TCHAR CFileManip::IsRoot(LPCTSTR lpDirectory)
{
	// Root directory must be "<driver-letter>:" or "<driver-letter>:\\"
	int nLen = ::_tcslen(lpDirectory);
	if (nLen < 2 || nLen > 3)
		return 0;

	if (toupper(lpDirectory[0]) < _T('A') || toupper(lpDirectory[0]) > _T('Z'))
		return 0;

	if (lpDirectory[1] != _T(':'))
		return 0;

	if (nLen == 3 && lpDirectory[2] != _T('\\'))
		return 0;

	// If is root, return driver letter.
	return lpDirectory[0];
}

BOOL CFileManip::GetAppDirectory(LPTSTR lpAppDir)
{
	if( NULL != lpAppDir )
		delete lpAppDir;

	lpAppDir = new TCHAR[MAX_PATH + 2];
	memset(lpAppDir, 0, sizeof(TCHAR)*(MAX_PATH+2) );

	// Application Data is where user settings go
	if( !SHGetSpecialFolderPath(NULL, lpAppDir, CSIDL_APPDATA, TRUE) )
	{
		delete lpAppDir;
		lpAppDir = NULL;
		return FALSE;
	}
	return TRUE;
}

BOOL CFileManip::GetExcutableDir(LPTSTR lpExcDir)
{
	if( NULL != lpExcDir)
		delete lpExcDir;

	lpExcDir = new TCHAR[MAX_PATH + 2];
	memset(lpExcDir, 0, sizeof(TCHAR)*(MAX_PATH+2) );

	DWORD dwret = GetModuleFileName(NULL, lpExcDir, MAX_PATH+2);
	if((0 == dwret)||(MAX_PATH+2 == dwret))
	{
		delete lpExcDir;
		lpExcDir = NULL;
		return FALSE;
	}
	LPTSTR pdest = ::_tcschr(lpExcDir, _T('\\'));
	if(NULL != pdest)
	{
		*(pdest + 1) = 0;
		return TRUE;
	}
	delete lpExcDir;
	lpExcDir = NULL;
	return FALSE;
}

BOOL CFileManip::GetWorkingDir(char* lpWorDir)
{
	if( NULL != lpWorDir)
		delete lpWorDir;

	lpWorDir = new char[MAX_PATH + 2];
	memset(lpWorDir, 0, sizeof(TCHAR)*(MAX_PATH+2) );

	DWORD dwret = GetCurrentDirectory(MAX_PATH + 2, lpWorDir);

	if( 0 == dwret )
	{
		delete lpWorDir;
		lpWorDir = NULL;
		return FALSE;
	}
	if( dwret > MAX_PATH +2)
	{
		delete lpWorDir;

		lpWorDir = new char[dwret];

		if(NULL == lpWorDir)
			return FALSE;
		dwret = GetCurrentDirectory(dwret, lpWorDir);
		if(dwret >0)
		{
			return TRUE;
		}
		else
		{
			delete lpWorDir;
			return FALSE;
		}
	}

	return TRUE;
}