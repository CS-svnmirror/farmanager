/*
fileattr.cpp

������ � ���������� ������
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

#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "flink.hpp"
#include "imports.hpp"

static int SetFileEncryption(const wchar_t *Name,int State);
static int SetFileCompression(const wchar_t *Name,int State);


int ESetFileAttributes(const wchar_t *Name,DWORD Attr,int SkipMode)
{
//_SVS(SysLog(L"Attr=0x%08X",Attr));
  while (!SetFileAttributesW(Name,Attr))
  {
    int Code;
    if(SkipMode!=-1)
      Code=SkipMode;
    else
      Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,UMSG(MError),
             UMSG(MSetAttrCannotFor),Name,UMSG(MHRetry),UMSG(MHSkip),UMSG(MHSkipAll),UMSG(MHCancel));
    switch(Code)
    {
    case -2:
    case -1:
    case 1:
      return SETATTR_RET_SKIP;
    case 2:
      return SETATTR_RET_SKIPALL;
    case 3:
      return SETATTR_RET_ERROR;
    }
  }
  return SETATTR_RET_OK;
}

static int SetFileCompression(const wchar_t *Name,int State)
{
  HANDLE hFile=apiCreateFile(Name,FILE_READ_DATA|FILE_WRITE_DATA,
                 FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                 FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(FALSE);
  USHORT NewState=State ? COMPRESSION_FORMAT_DEFAULT:COMPRESSION_FORMAT_NONE;
  DWORD Result;
  int RetCode=DeviceIoControl(hFile,FSCTL_SET_COMPRESSION,&NewState,
                              sizeof(NewState),NULL,0,&Result,NULL);
  CloseHandle(hFile);
  return(RetCode);
}


int ESetFileCompression(const wchar_t *Name,int State,DWORD FileAttr,int SkipMode)
{
  if (((FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0) == State)
    return SETATTR_RET_OK;

  int Ret=SETATTR_RET_OK;
  if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributesW(Name,FileAttr & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

  // Drop Encryption
  if ((FileAttr & FILE_ATTRIBUTE_ENCRYPTED) && State)
    SetFileEncryption(Name,0);

  while (!SetFileCompression(Name,State))
  {
    if (GetLastError()==ERROR_INVALID_FUNCTION)
    {
      Ret=SETATTR_RET_OK;
      break;
    }
    int Code;
    if(SkipMode!=-1)
      Code=SkipMode;
    else
      Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,UMSG(MError),
                UMSG(MSetAttrCompressedCannotFor),Name,UMSG(MHRetry),
                UMSG(MHSkip),UMSG(MHSkipAll),UMSG(MHCancel));
    if (Code==1 || Code<0)
    {
      Ret=SETATTR_RET_SKIP;
      break;
    }
    else if (Code==2)
    {
      Ret=SETATTR_RET_SKIPALL;
      break;
    }
    else if (Code==3)
    {
      Ret=SETATTR_RET_ERROR;
      break;
    }
  }
  // Set ReadOnly
  if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributesW(Name,FileAttr);
  return(Ret);
}


static int SetFileEncryption(const wchar_t *Name,int State)
{
	if ( ifn.bEncryptFunctions )
	{
		if ( State )
			return ifn.pfnEncryptFile(Name);
		else
			return ifn.pfnDecryptFile(Name, 0);
	}

	return FALSE;
}


int ESetFileEncryption(const wchar_t *Name,int State,DWORD FileAttr,int SkipMode,int Silent)
{
  if (((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0) == State)
    return SETATTR_RET_OK;

  if(!IsCryptFileASupport)
    return SETATTR_RET_OK;

  int Ret=SETATTR_RET_OK;

  // Drop ReadOnly
  if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributesW(Name,FileAttr & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));

  while (!SetFileEncryption(Name,State))
  {
    if (GetLastError()==ERROR_INVALID_FUNCTION)
      break;

    if(Silent)
    {
      Ret=SETATTR_RET_ERROR;
      break;
    }
    int Code;
    if(SkipMode!=-1)
      Code=SkipMode;
    else            
      Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,UMSG(MError),
                UMSG(MSetAttrEncryptedCannotFor),Name,UMSG(MHRetry), //BUGBUG
                UMSG(MHSkip),UMSG(MHSkipAll),UMSG(MHCancel));
    if (Code==1 || Code<0)
    {
      Ret=SETATTR_RET_SKIP;
      break;
    }
    if (Code==2)
    {
      Ret=SETATTR_RET_SKIPALL;
      break;
    }
    if (Code==3)
    {
      Ret=SETATTR_RET_ERROR;
      break;
    }
  }

  // Set ReadOnly
  if (FileAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
    SetFileAttributesW(Name,FileAttr);

  return(Ret);
}


int ESetFileTime(const wchar_t *Name,FILETIME *LastWriteTime,FILETIME *CreationTime,
                  FILETIME *LastAccessTime,DWORD FileAttr,int SkipMode)
{
  if ((LastWriteTime==NULL && CreationTime==NULL && LastAccessTime==NULL) ||
      ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT))
    return SETATTR_RET_OK;

  while (1)
  {
    if (FileAttr & FILE_ATTRIBUTE_READONLY)
      SetFileAttributesW(Name,FileAttr & ~FILE_ATTRIBUTE_READONLY);

    HANDLE hFile=apiCreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
                 NULL,OPEN_EXISTING,
                 (FileAttr & FILE_ATTRIBUTE_DIRECTORY) ? FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
    int SetTime;
    DWORD LastError=0;
    if (hFile==INVALID_HANDLE_VALUE)
    {
      SetTime=FALSE;
      LastError=GetLastError();
    }
    else
    {
      SetTime=SetFileTime(hFile,CreationTime,LastAccessTime,LastWriteTime);
      LastError=GetLastError();
      CloseHandle(hFile);

      if ( (FileAttr & FILE_ATTRIBUTE_DIRECTORY) && LastError==ERROR_NOT_SUPPORTED ) // FIX: Mantis#223
      {
        string strDriveRoot;
        GetPathRoot (Name, strDriveRoot);
        if ( GetDriveTypeW (strDriveRoot)==DRIVE_REMOTE ) break;
      }
    }

    if (FileAttr & FILE_ATTRIBUTE_READONLY)
      SetFileAttributesW(Name,FileAttr);
    SetLastError(LastError);

    if (SetTime)
      break;
    int Code;
    if(SkipMode!=-1)
      Code=SkipMode;
    else
      Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,UMSG(MError),
                UMSG(MSetAttrTimeCannotFor),Name,UMSG(MHRetry), //BUGBUG
                UMSG(MHSkip),UMSG(MHSkipAll),UMSG(MHCancel));
    switch(Code)
    {
    case -2:
    case -1:
    case 3:
      return SETATTR_RET_ERROR;
    case 2:
      return SETATTR_RET_SKIPALL;
    case 1:
      return SETATTR_RET_SKIP;
    }
  }
  return SETATTR_RET_OK;
}
