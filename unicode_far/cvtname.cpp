/*
cvtname.cpp

������� ��� �������������� ���� ������/�����.
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
#include "syslog.hpp"

int ConvertNameToFull (
        const wchar_t *lpwszSrc,
        string &strDest
        )
{
	string strSrc = lpwszSrc; //����������� � ������ ���������� �� ������ dest == src
	lpwszSrc = strSrc;

	const wchar_t *lpwszName = PointToName(lpwszSrc);

	if ( (lpwszName == lpwszSrc) &&
				!TestParentFolderName(lpwszName) && !TestCurrentFolderName(lpwszName))
	{
		apiGetCurrentDirectory(strDest);
		AddEndSlash(strDest);

		strDest += lpwszSrc;

		return (int)strDest.GetLength ();
	}

	if ( PathMayBeAbsolute(lpwszSrc) )
	{
		if ( *lpwszName &&
				(*lpwszName != L'.' || (lpwszName[1] != 0 && (lpwszName[1] != L'.' || lpwszName[2] != 0)) ) &&
				(wcsstr (lpwszSrc, L"\\..\\") == NULL && wcsstr (lpwszSrc, L"\\.\\") == NULL) )
		{
			strDest = lpwszSrc;

			return (int)strDest.GetLength ();
		}
	}
	return (int)apiGetFullPathName(lpwszSrc,strDest);
}

/*
  ����������� Src � ������ �������� ���� � ������ reparse point
*/
int ConvertNameToReal (const wchar_t *Src, string &strDest, bool Internal)
{
  string strTempDest;
  BOOL IsAddEndSlash=FALSE; // =TRUE, ���� ���� ��������� ��������������
                            // � ����� �� ��� ����... ������.

  // ������� ������� ������ ���� �� ������� ������� ��������
  int Ret=ConvertNameToFull(Src, strTempDest);
  //RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));
  _SVS(SysLog(L"ConvertNameToFull('%s') -> '%s'",Src,(const wchar_t*)strTempDest));

  wchar_t *TempDest;
  /* $ 14.06.2003 IS
     ��� ����������� ������ ���� � �� �������� ������������� ��������
  */
  // ����� ������ �� ������ ��� ����������� ������, �.�. ��� ��� ���������� ������
  // ���������� ���������� ��� ������, �� ������� ��������� ������� (�.�. ����������
  // "������������ �������")
  if (IsLocalDrive(strTempDest))
  {
    DWORD FileAttr;

		if((FileAttr=apiGetFileAttributes(strTempDest)) != INVALID_FILE_ATTRIBUTES)
    {
      AddEndSlash(strTempDest);
      IsAddEndSlash=TRUE;
    }

    TempDest = strTempDest.GetBuffer();
    wchar_t *Ptr, Chr;

    Ptr = TempDest + strTempDest.GetLength();

    const wchar_t *CtrlChar = TempDest;

    if (strTempDest.GetLength() > 2 && TempDest[0]==L'\\' && TempDest[1]==L'\\')
      CtrlChar = wcschr(TempDest+2, L'\\');

    // ������� ���� ������� ����� �� �����
    while(CtrlChar)
    {
			while(Ptr > TempDest && !IsSlash(*Ptr))
        --Ptr;
      /* $ 07.01.2003 IS
         - ������: �����-�� ������������ ���� "�����:" - �� �����
           �������� �������� �� ����� "�����", ��� ����� �
           ��������������� �����������
      */
      // ���� ��� UNC, �� �������� �� ����� �������, �� ������...
      if(*Ptr != L'\\' || Ptr == CtrlChar
        // ���� ����� �� "�����:", �� ���� �����������
        || *(Ptr-1)==L':')
        break;
      /* IS $ */

      Chr=*Ptr;
      *Ptr=0;
			FileAttr=apiGetFileAttributes(TempDest);
      // �! ��� ��� ������ - ���� �� "���������" ���� - �������
      if(FileAttr != INVALID_FILE_ATTRIBUTES && (FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
      {
        string strTempDest2;
        // ������� ���� ��������
        if(GetReparsePointInfo(TempDest, strTempDest2))
        {
          // ������� \\??\ �� ���� ��������
          if(!StrCmpN(strTempDest2,L"\\??\\",4))
            strTempDest2.LShift(4);
          // ��� ������ �������������� ����� (�� �������� �����)...
          if(!StrCmpNI(strTempDest2, L"Volume{", 7))
          {
            string strJuncRoot;
            // ������� ���� ����� �����, ����...
            GetPathRootOne(strTempDest2, strJuncRoot);
            // ...�� � ����� ������ ����� ���������.
            // (�������� - ���� ����� �� �������� - ����� ����� ������������)
            strTempDest2 = (strJuncRoot.At(1)==L':'||!Internal)?strJuncRoot:TempDest;
          }
          DeleteEndSlash(strTempDest2);
          // ����� ���� ��������
          size_t temp2Length = strTempDest2.GetLength();
          // ����� ��������
          wchar_t* TempDest2 = strTempDest2.GetBuffer();
          // �������� ����� ����� � ������ ������ ����
          size_t leftLength = StrLength(TempDest);
          size_t rightLength = StrLength(Ptr + 1); // �������� ����� ���� ������� �� ���������� ������� ����� �������
          // ����������� ������
          *Ptr=Chr;
          // ���� ���� �������� ������ ����� ����� ����, ����������� �����
          if (leftLength < temp2Length)
          {
            // �������� ����� �����
            TempDest = strTempDest.GetBuffer(strTempDest.GetLength() + temp2Length - leftLength + (IsAddEndSlash?2:1));
          }
          // ��� ��� �� ����������� ����������� � ����� ������ ���� �������� ��������� ��
          // ������� ������� ������� � ����
          Ptr = TempDest + temp2Length - 1;
          // ���������� ������ ����� ���� �� ������ �����, ������ ���� ����� ���� ���������� ��
          // ������� �� ���� ��������
          if (leftLength != temp2Length)
          {
            // ���������� ����� �������� ��� �����, ��������� '/', �������� '/' (���� �� ����) � '\0'
            wmemmove(TempDest + temp2Length, TempDest + leftLength, rightLength + (IsAddEndSlash ? 3 : 2));
          }
          // �������� ���� � �������� ������� ����
          wmemcpy(TempDest, TempDest2, temp2Length);
          // ��������� ������ �� ������ ���������� ����������� �� ����
          CtrlChar = TempDest;
          if (StrLength(TempDest) > 2 && TempDest[0] == L'\\' && TempDest[1] == L'\\')
          {
            CtrlChar = wcschr(TempDest + 2, L'\\');
						if(!CtrlChar)
							CtrlChar=wcschr(TempDest + 2, L'/');
          }
          // ������������� ����� ������������ ������
          Ret = StrLength(TempDest);
          // ������� ����� ��� ������������ ���������� ����� ������. ���� ����� �� ������, �� ���
          // ���������� ������ ����� ������������� �� ��� ������
          strTempDest.ReleaseBuffer(Ret);
          // ��������� � ���������� ����
          continue;
        }
      }
      *Ptr=Chr;
      --Ptr;
    }
  }

  // ���� �� ������� - ������.
  if(IsAddEndSlash)
  {
    if (DeleteEndSlash(strTempDest))
    {
      --Ret;
    }
  }

  strDest = strTempDest;

  return Ret;
}

void ConvertNameToShort(const wchar_t *Src, string &strDest)
{
	string strCopy = Src;

	int nSize = GetShortPathNameW (strCopy, NULL, 0);

	if ( nSize )
	{
		wchar_t *lpwszDest = strDest.GetBuffer (nSize);

		GetShortPathNameW (strCopy, lpwszDest, nSize);

		strDest.ReleaseBuffer ();
	}
	else
		strDest = strCopy;

	strDest.Upper ();
}

void ConvertNameToLong(const wchar_t *Src, string &strDest)
{
	string strCopy = Src;

	int nSize = GetLongPathNameW (strCopy, NULL, 0);

	if ( nSize )
	{
		wchar_t *lpwszDest = strDest.GetBuffer (nSize);

		GetLongPathNameW (strCopy, lpwszDest, nSize);

		strDest.ReleaseBuffer ();
	}
	else
		strDest = strCopy;
}

string &DriveLocalToRemoteName(int DriveType,wchar_t Letter,string &strDest)
{
  int NetPathShown=FALSE, IsOK=FALSE;
  wchar_t LocalName[8]=L" :\0\0\0";
  string strRemoteName;

  *LocalName=Letter;
  strDest=L"";

  if(DriveType == DRIVE_UNKNOWN)
  {
    LocalName[2]=L'\\';
    DriveType = FAR_GetDriveType(LocalName);
    LocalName[2]=0;
  }

  if (DriveType==DRIVE_REMOTE)
  {
    DWORD res = apiWNetGetConnection(LocalName,strRemoteName);
    if (res == NO_ERROR || res == ERROR_CONNECTION_UNAVAIL)
    {
      NetPathShown=TRUE;
      IsOK=TRUE;
    }
  }

  if (!NetPathShown)
    if (GetSubstName(DriveType,LocalName,strRemoteName))
      IsOK=TRUE;

  if(IsOK)
    strDest = strRemoteName;

  return strDest;
}

void ConvertNameToUNC(string &strFileName)
{
	ConvertNameToFull(strFileName,strFileName);
	// ��������� �� ��� �������� �������
	string strFileSystemName;
	string strTemp;
	GetPathRoot(strFileName,strTemp);

	if(!apiGetVolumeInformation (strTemp,NULL,NULL,NULL,NULL,&strFileSystemName))
		strFileSystemName=L"";

	DWORD uniSize = 1024;
	UNIVERSAL_NAME_INFOW *uni=(UNIVERSAL_NAME_INFOW*)xf_malloc(uniSize);

	// ��������� WNetGetUniversalName ��� ���� ������, ������ �� ��� Novell`�
	if (StrCmpI(strFileSystemName,L"NWFS"))
	{
		DWORD dwRet=WNetGetUniversalNameW(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize);
		switch(dwRet)
		{
		case NO_ERROR:
			strFileName = uni->lpUniversalName;
			break;
		case ERROR_MORE_DATA:
			uni=(UNIVERSAL_NAME_INFOW*)xf_realloc(uni,uniSize);
			if(WNetGetUniversalNameW(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize)==NO_ERROR)
				strFileName = uni->lpUniversalName;
			break;
		}
	}
	else if(strFileName.At(1) == L':')
	{
		// BugZ#449 - �������� ������ CtrlAltF � ��������� Novell DS
		// �����, ���� �� ���������� �������� UniversalName � ���� ���
		// ��������� ���� - �������� ��� ��� ���� ������ ������

		if(!DriveLocalToRemoteName(DRIVE_UNKNOWN,strFileName.At(0),strTemp).IsEmpty())
		{
			const wchar_t *NamePtr;
			if((NamePtr=wcschr(strFileName, L'/')) == NULL)
				NamePtr=wcschr(strFileName, L'\\');
			if(NamePtr != NULL)
			{
				AddEndSlash(strTemp);
				strTemp += &NamePtr[1];
			}
			strFileName = strTemp;
		}
	}
	xf_free(uni);
	ConvertNameToReal(strFileName,strFileName);
}
