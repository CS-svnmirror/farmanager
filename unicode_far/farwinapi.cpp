/*
farwinapi.cpp

������� ������ ��������� WinAPI �������
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "flink.hpp"
#include "imports.hpp"

class NTPath
{
	string Str;
public:
	NTPath(LPCWSTR Src)
	{
		ConvertNameToFull(Src,Str);
		if(!PathPrefix(Str))
		{
			if(IsNetworkPath(Str))
				Str=string(L"\\\\?\\UNC\\")+&Str[2];
			else
				Str=string(L"\\\\?\\")+Str;
		}
	}
	operator LPCWSTR() const
	{
		return Str;
	}
};

BOOL apiDeleteFile (const wchar_t *lpwszFileName)
{
	return DeleteFile(NTPath(lpwszFileName));
}


BOOL apiRemoveDirectory (const wchar_t *DirName)
{
	return RemoveDirectoryW (NTPath(DirName));
}

HANDLE apiCreateFile (
		const wchar_t *lpwszFileName,     // pointer to name of the file
		DWORD dwDesiredAccess,  // access (read-write) mode
		DWORD dwShareMode,      // share mode
		LPSECURITY_ATTRIBUTES lpSecurityAttributes, // pointer to security attributes
		DWORD dwCreationDistribution, // how to create
		DWORD dwFlagsAndAttributes,   // file attributes
		HANDLE hTemplateFile          // handle to file with attributes to copy
		)
{
	return CreateFileW (
			NTPath(lpwszFileName),
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDistribution,
			dwFlagsAndAttributes,
			hTemplateFile
			);
}

BOOL apiCopyFileEx (
		const wchar_t *lpwszExistingFileName,
		const wchar_t *lpwszNewFileName,
		LPPROGRESS_ROUTINE lpProgressRoutine,
		LPVOID lpData,
		LPBOOL pbCancel,
		DWORD dwCopyFlags
		)
{
		return CopyFileExW(
				NTPath(lpwszExistingFileName),
				NTPath(lpwszNewFileName),
				lpProgressRoutine,
				lpData,
				pbCancel,
				dwCopyFlags
				);
}


BOOL apiMoveFile (
		const wchar_t *lpwszExistingFileName, // address of name of the existing file
		const wchar_t *lpwszNewFileName   // address of new name for the file
		)
{
	return MoveFileW (NTPath(lpwszExistingFileName),NTPath(lpwszNewFileName));
}

BOOL apiMoveFileEx (
		const wchar_t *lpwszExistingFileName, // address of name of the existing file
		const wchar_t *lpwszNewFileName,   // address of new name for the file
		DWORD dwFlags   // flag to determine how to move file
		)
{
	return MoveFileExW (NTPath(lpwszExistingFileName),NTPath(lpwszNewFileName),dwFlags);
}


BOOL apiMoveFileThroughTemp(const wchar_t *Src, const wchar_t *Dest)
{
  string strTemp;
  BOOL rc = FALSE;

  if ( FarMkTempEx(strTemp, NULL, FALSE) )
  {
      if ( apiMoveFile (Src, strTemp) )
          rc = apiMoveFile (strTemp, Dest);
  }

  return rc;
}

DWORD apiGetEnvironmentVariable (const wchar_t *lpwszName, string &strBuffer)
{
	int nSize = GetEnvironmentVariableW (lpwszName, NULL, 0);

	if ( nSize )
	{
		wchar_t *lpwszBuffer = strBuffer.GetBuffer (nSize);

		nSize = GetEnvironmentVariableW (lpwszName, lpwszBuffer, nSize);

		strBuffer.ReleaseBuffer ();
	}

	return nSize;
}

DWORD apiGetCurrentDirectory (string &strCurDir)
{
	DWORD dwSize = GetCurrentDirectoryW (0, NULL);
	wchar_t *lpwszCurDir = strCurDir.GetBuffer (dwSize);
	GetCurrentDirectoryW (dwSize, lpwszCurDir);
	strCurDir.ReleaseBuffer ();

	if(IsLocalVolumeRootPath(strCurDir))
		AddEndSlash(strCurDir);

	return (DWORD)strCurDir.GetLength();
}

DWORD apiGetTempPath (string &strBuffer)
{
	DWORD dwSize = GetTempPathW (0, NULL);

	wchar_t *lpwszBuffer = strBuffer.GetBuffer (dwSize);

	dwSize = GetTempPathW (dwSize, lpwszBuffer);

	strBuffer.ReleaseBuffer ();

	return dwSize;
};


DWORD apiGetModuleFileName (HMODULE hModule, string &strFileName)
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszFileName = NULL;

	do {
		dwBufferSize <<= 1;

		lpwszFileName = (wchar_t*)xf_realloc (lpwszFileName, dwBufferSize*sizeof (wchar_t));

		dwSize = GetModuleFileNameW (hModule, lpwszFileName, dwBufferSize);
	} while ( dwSize && (dwSize >= dwBufferSize) );

	if ( dwSize )
		strFileName = lpwszFileName;

	xf_free (lpwszFileName);

	return dwSize;
}

DWORD apiExpandEnvironmentStrings (const wchar_t *src, string &strDest)
{
	string strSrc = src;

	DWORD length = ExpandEnvironmentStringsW(strSrc, NULL, 0);

	if ( length )
	{
		wchar_t *lpwszDest = strDest.GetBuffer (length);

		ExpandEnvironmentStringsW(strSrc, lpwszDest, length);

		strDest.ReleaseBuffer ();

		length = (DWORD)strDest.GetLength ();
	}

	return length;
}


DWORD apiGetConsoleTitle (string &strConsoleTitle)
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszTitle = NULL;

	do {
		dwBufferSize <<= 1;

		lpwszTitle = (wchar_t*)xf_realloc (lpwszTitle, dwBufferSize*sizeof (wchar_t));

		dwSize = GetConsoleTitleW (lpwszTitle, dwBufferSize);

	} while ( !dwSize && GetLastError() == ERROR_SUCCESS );

	if ( dwSize )
		strConsoleTitle = lpwszTitle;

	xf_free (lpwszTitle);

	return dwSize;
}


DWORD apiWNetGetConnection (const wchar_t *lpwszLocalName, string &strRemoteName)
{
	DWORD dwRemoteNameSize = 0;
	DWORD dwResult = WNetGetConnectionW(lpwszLocalName, NULL, &dwRemoteNameSize);

	if ( dwResult == ERROR_SUCCESS )
	{
		wchar_t *lpwszRemoteName = strRemoteName.GetBuffer (dwRemoteNameSize);

		dwResult = WNetGetConnectionW (lpwszLocalName, lpwszRemoteName, &dwRemoteNameSize);

		strRemoteName.ReleaseBuffer ();
	}

	return dwResult;
}

BOOL apiGetVolumeInformation (
        const wchar_t *lpwszRootPathName,
        string *pVolumeName,
        LPDWORD lpVolumeSerialNumber,
        LPDWORD lpMaximumComponentLength,
        LPDWORD lpFileSystemFlags,
        string *pFileSystemName
        )
{
    wchar_t *lpwszVolumeName = pVolumeName?pVolumeName->GetBuffer (MAX_PATH+1):NULL; //MSDN!
    wchar_t *lpwszFileSystemName = pFileSystemName?pFileSystemName->GetBuffer (MAX_PATH+1):NULL;

    BOOL bResult = GetVolumeInformationW (
            lpwszRootPathName,
            lpwszVolumeName,
            lpwszVolumeName?MAX_PATH:0,
            lpVolumeSerialNumber,
            lpMaximumComponentLength,
            lpFileSystemFlags,
            lpwszFileSystemName,
            lpwszFileSystemName?MAX_PATH:0
            );

    if ( lpwszVolumeName )
        pVolumeName->ReleaseBuffer ();

    if ( lpwszFileSystemName )
        pFileSystemName->ReleaseBuffer ();

    return bResult;
}

HANDLE apiFindFirstFile (
        const wchar_t *lpwszFileName,
        FAR_FIND_DATA_EX *pFindFileData,
        bool ScanSymLink
        )
{
    WIN32_FIND_DATAW fdata;

		HANDLE hResult = FindFirstFileW (NTPath(lpwszFileName), &fdata);

		if(hResult==INVALID_HANDLE_VALUE && ScanSymLink)
		{
			string strRealName;
			ConvertNameToReal(lpwszFileName,strRealName);
			hResult=FindFirstFileW(NTPath(strRealName),&fdata);
		}

    if ( hResult != INVALID_HANDLE_VALUE )
    {
        pFindFileData->dwFileAttributes = fdata.dwFileAttributes;
        pFindFileData->ftCreationTime = fdata.ftCreationTime;
        pFindFileData->ftLastAccessTime = fdata.ftLastAccessTime;
        pFindFileData->ftLastWriteTime = fdata.ftLastWriteTime;
        pFindFileData->nFileSize = fdata.nFileSizeHigh*_ui64(0x100000000)+fdata.nFileSizeLow;
        pFindFileData->dwReserved0 = fdata.dwReserved0;
        pFindFileData->dwReserved1 = fdata.dwReserved1;
        pFindFileData->strFileName = fdata.cFileName;
        pFindFileData->strAlternateFileName = fdata.cAlternateFileName;
    }

    return hResult;
}

BOOL apiFindNextFile (HANDLE hFindFile, FAR_FIND_DATA_EX *pFindFileData)
{
    WIN32_FIND_DATAW fdata;

    BOOL bResult = FindNextFileW (hFindFile, &fdata);

    if ( bResult )
    {
        pFindFileData->dwFileAttributes = fdata.dwFileAttributes;
        pFindFileData->ftCreationTime = fdata.ftCreationTime;
        pFindFileData->ftLastAccessTime = fdata.ftLastAccessTime;
        pFindFileData->ftLastWriteTime = fdata.ftLastWriteTime;
        pFindFileData->nFileSize = fdata.nFileSizeHigh*_ui64(0x100000000)+fdata.nFileSizeLow;
        pFindFileData->dwReserved0 = fdata.dwReserved0;
        pFindFileData->dwReserved1 = fdata.dwReserved1;
        pFindFileData->strFileName = fdata.cFileName;
        pFindFileData->strAlternateFileName = fdata.cAlternateFileName;
    }

    return bResult;
}

BOOL apiFindClose(HANDLE hFindFile)
{
	return FindClose(hFindFile);
}

void apiFindDataToDataEx (const FAR_FIND_DATA *pSrc, FAR_FIND_DATA_EX *pDest)
{
    pDest->dwFileAttributes = pSrc->dwFileAttributes;
    pDest->ftCreationTime = pSrc->ftCreationTime;
    pDest->ftLastAccessTime = pSrc->ftLastAccessTime;
    pDest->ftLastWriteTime = pSrc->ftLastWriteTime;
    pDest->nFileSize = pSrc->nFileSize;
    pDest->nPackSize = pSrc->nPackSize;
    pDest->strFileName = pSrc->lpwszFileName;
    pDest->strAlternateFileName = pSrc->lpwszAlternateFileName;
}

void apiFindDataExToData (const FAR_FIND_DATA_EX *pSrc, FAR_FIND_DATA *pDest)
{
    pDest->dwFileAttributes = pSrc->dwFileAttributes;
    pDest->ftCreationTime = pSrc->ftCreationTime;
    pDest->ftLastAccessTime = pSrc->ftLastAccessTime;
    pDest->ftLastWriteTime = pSrc->ftLastWriteTime;
    pDest->nFileSize = pSrc->nFileSize;
    pDest->nPackSize = pSrc->nPackSize;
    pDest->lpwszFileName = xf_wcsdup (pSrc->strFileName);
    pDest->lpwszAlternateFileName = xf_wcsdup (pSrc->strAlternateFileName);
}

void apiFreeFindData (FAR_FIND_DATA *pData)
{
    xf_free (pData->lpwszFileName);
    xf_free (pData->lpwszAlternateFileName);
}

BOOL apiGetFindDataEx (const wchar_t *lpwszFileName, FAR_FIND_DATA_EX *pFindData,bool ScanSymLink)
{
    HANDLE hSearch = apiFindFirstFile (lpwszFileName, pFindData,ScanSymLink);

    if ( hSearch != INVALID_HANDLE_VALUE )
    {
        apiFindClose (hSearch);
        return TRUE;
    }
    else
	{
		DWORD dwAttr=apiGetFileAttributes(lpwszFileName);
		if(dwAttr!=INVALID_FILE_ATTRIBUTES)
		{
			// ���, ������ ���� ���� ����. �������� ��������� �������.
			if(pFindData)
			{
				pFindData->Clear();
				pFindData->dwFileAttributes=dwAttr;
				HANDLE hFile=apiCreateFile(lpwszFileName,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,(pFindData->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
				if(hFile!=INVALID_HANDLE_VALUE)
				{
					GetFileTime(hFile,&pFindData->ftCreationTime,&pFindData->ftLastAccessTime,&pFindData->ftLastWriteTime);
					apiGetFileSize (hFile,&pFindData->nFileSize);
					CloseHandle(hFile);
				}
				if(pFindData->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
				{
					string strTmp;
					GetReparsePointInfo(lpwszFileName,strTmp,&pFindData->dwReserved0); //MSDN
				}
				else
				{
					pFindData->dwReserved0=0;
				}
				pFindData->dwReserved1=0;
				pFindData->strFileName=PointToName(lpwszFileName);
				ConvertNameToShort(lpwszFileName,pFindData->strAlternateFileName);
				return TRUE;
			}
		}
	}
	if(pFindData)
	{
		pFindData->Clear();
		pFindData->dwFileAttributes=INVALID_FILE_ATTRIBUTES; //BUGBUG
	}
    return FALSE;
}

BOOL apiGetFileSize (HANDLE hFile, unsigned __int64 *pSize)
{
	DWORD dwHi, dwLo;

	dwLo = GetFileSize (hFile, &dwHi);

	int nError = GetLastError();
	SetLastError (nError);

	if ( (dwLo == INVALID_FILE_SIZE) && (nError != NO_ERROR) )
		return FALSE;
	else
	{
		if ( pSize )
			*pSize = dwHi*_ui64(0x100000000)+dwLo;

		return TRUE;
	}
}

int apiRegEnumKeyEx(HKEY hKey,DWORD dwIndex,string &strName,PFILETIME lpftLastWriteTime)
{
	int ExitCode=ERROR_MORE_DATA;
	for(DWORD Size=512;ExitCode==ERROR_MORE_DATA;Size<<=1)
	{
		wchar_t *Name=strName.GetBuffer(Size);
		ExitCode=RegEnumKeyExW(hKey,dwIndex,Name,&Size,NULL,NULL,NULL,lpftLastWriteTime);
		strName.ReleaseBuffer();
	}
	return ExitCode;
}

BOOL apiIsDiskInDrive(const wchar_t *Root)
{
	string strVolName;
	string strDrive;
	DWORD  MaxComSize;
	DWORD  Flags;
	string strFS;

	strDrive = Root;

	AddEndSlash(strDrive);
	//UINT ErrMode = SetErrorMode ( SEM_FAILCRITICALERRORS );
	//���� �� ������� SetErrorMode - �������� ����������� ������ "Drive Not Ready"
	BOOL Res = apiGetVolumeInformation (strDrive, &strVolName, NULL, &MaxComSize, &Flags, &strFS);
	//SetErrorMode(ErrMode);
	return Res;
}

int apiGetFileTypeByName(const wchar_t *Name)
{
	HANDLE hFile=apiCreateFile(NTPath(Name),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0);

	if (hFile==INVALID_HANDLE_VALUE)
		return FILE_TYPE_UNKNOWN;

	int Type=GetFileType(hFile);

	CloseHandle(hFile);

	return Type;
}

BOOL apiGetDiskSize(const wchar_t *Root,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree)
{
	int ExitCode=0;

	unsigned __int64 uiTotalSize,uiTotalFree,uiUserFree;
	uiUserFree=_i64(0);
	uiTotalSize=_i64(0);
	uiTotalFree=_i64(0);

	ExitCode=GetDiskFreeSpaceExW(Root,(PULARGE_INTEGER)&uiUserFree,(PULARGE_INTEGER)&uiTotalSize,(PULARGE_INTEGER)&uiTotalFree);

	if ( TotalSize )
		*TotalSize = uiTotalSize;
	if ( TotalFree )
		*TotalFree = uiTotalFree;
	if ( UserFree )
		*UserFree = uiUserFree;

	return ExitCode;
}

BOOL apiGetConsoleKeyboardLayoutName (string &strDest)
{
	BOOL ret=FALSE;
	strDest.SetLength(0);
	if ( ifn.pfnGetConsoleKeyboardLayoutName )
	{
		wchar_t *p = strDest.GetBuffer(KL_NAMELENGTH+1);
		if ( p && ifn.pfnGetConsoleKeyboardLayoutName(p) )
			ret=TRUE;
		strDest.ReleaseBuffer();
	}
	return ret;
}

HANDLE apiFindFirstFileName(LPCWSTR lpFileName,DWORD dwFlags,string& strLinkName)
{
	HANDLE hRet=INVALID_HANDLE_VALUE;
	DWORD StringLength=0;
	if(ifn.pfnFindFirstFileNameW(NTPath(lpFileName),0,&StringLength,NULL)==INVALID_HANDLE_VALUE && GetLastError()==ERROR_MORE_DATA)
	{
		hRet=ifn.pfnFindFirstFileNameW(NTPath(lpFileName),0,&StringLength,strLinkName.GetBuffer(StringLength));
		strLinkName.ReleaseBuffer();
	}
	return hRet;
}
	
BOOL apiFindNextFileName(HANDLE hFindStream,string& strLinkName)
{
	BOOL Ret=FALSE;
	DWORD StringLength=0;
	if(!ifn.pfnFindNextFileNameW(hFindStream,&StringLength,NULL) && GetLastError()==ERROR_MORE_DATA)
	{
		Ret=ifn.pfnFindNextFileNameW(hFindStream,&StringLength,strLinkName.GetBuffer(StringLength));
		strLinkName.ReleaseBuffer();
	}
	return Ret;
}

BOOL apiCreateDirectory(LPCWSTR lpPathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return CreateDirectory(NTPath(lpPathName),lpSecurityAttributes);
}

DWORD apiGetFileAttributes(LPCWSTR lpFileName)
{
	return GetFileAttributes(NTPath(lpFileName));
}

BOOL apiSetFileAttributes(LPCWSTR lpFileName,DWORD dwFileAttributes)
{
	return SetFileAttributes(NTPath(lpFileName),dwFileAttributes);
}

BOOL apiSetCurrentDirectory(LPCWSTR lpPathName)
{
	string strDir=lpPathName;
	AddEndSlash(strDir);
	BOOL Ret=SetCurrentDirectory(strDir);
	if(!Ret)
	{
		strDir=NTPath(lpPathName);
		AddEndSlash(strDir);
		Ret=SetCurrentDirectory(strDir);
	}
	return Ret;
}

BOOL apiCreateSymbolicLink(LPCWSTR lpSymlinkFileName,LPCWSTR lpTargetFileName,DWORD dwFlags)
{
	BOOL Ret=ifn.pfnCreateSymbolicLink(lpSymlinkFileName,lpTargetFileName,dwFlags);
	if(!Ret)
		Ret=ifn.pfnCreateSymbolicLink(NTPath(lpSymlinkFileName),NTPath(lpTargetFileName),dwFlags);
	return Ret;
}

DWORD apiGetCompressedFileSize(LPCWSTR lpFileName,LPDWORD lpFileSizeHigh)
{
	return GetCompressedFileSize(NTPath(lpFileName),lpFileSizeHigh);
}

BOOL apiCreateHardLink(LPCWSTR lpFileName,LPCWSTR lpExistingFileName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return CreateHardLink(NTPath(lpFileName),NTPath(lpExistingFileName),lpSecurityAttributes);
}

DWORD apiGetFullPathName(LPCWSTR lpFileName,string &strFullPathName)
{
	// ��� ���, �������������� ���������
	LPCWSTR SrcPtr=PointToName(lpFileName);
	int NameLength=0;
	string strFileName=lpFileName;
	if(StrCmp(SrcPtr,L"..") && StrCmp(SrcPtr,L"."))
		NameLength=StrLength(SrcPtr);
	else
		AddEndSlash(strFileName);

	//Mantis#459 - ����� +3 ��� ��� � Win2K SP4 ���� ���� � GetFullPathNameW
	int nLength = GetFullPathNameW(strFileName,0,NULL,NULL)+1+3;
	wchar_t *Buffer=strFullPathName.GetBuffer(nLength+NameLength);
	GetFullPathNameW(strFileName,nLength,Buffer,NULL);
	
	// ��� ���, �������������� ���������
	LPWSTR DstPtr=(LPWSTR)PointToName(Buffer);
	wcsncpy(DstPtr,SrcPtr,NameLength);

	strFullPathName.ReleaseBuffer();
	return (DWORD)strFullPathName.GetLength();
}

BOOL apiSetFilePointerEx(HANDLE hFile,INT64 DistanceToMove,PINT64 NewFilePointer,DWORD dwMoveMethod)
{
	return SetFilePointerEx(hFile,*((PLARGE_INTEGER)&DistanceToMove),(PLARGE_INTEGER)NewFilePointer,dwMoveMethod);
}


