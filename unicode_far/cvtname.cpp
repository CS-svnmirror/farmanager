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

#include "plugin.hpp"
#include "lang.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"


int ConvertNameToFull (
        const wchar_t *lpwszSrc,
        string &strDest
        )
{
	string strSrc = lpwszSrc; //����������� � ������ ���������� �� ������ dest == src

	lpwszSrc = strSrc;

	int Result = (int)wcslen (lpwszSrc);

	const wchar_t *lpwszName = PointToName(lpwszSrc);

	if ( (lpwszName == lpwszSrc) &&
				(lpwszName[0] != L'.' || lpwszName[1] != 0) )
	{
		FarGetCurDir(strDest);
		AddEndSlash(strDest);

		strDest += lpwszSrc;

		return (int)strDest.GetLength ();
	}

	if ( PathMayBeAbsolute(lpwszSrc) )
	{
		if ( *lpwszName &&
				(*lpwszName != L'.' || lpwszName[1] != 0 && (lpwszName[1] != L'.' || lpwszName[2] != 0) ) &&
				(wcsstr (lpwszSrc, L"\\..\\") == NULL && wcsstr (lpwszSrc, L"\\.\\") == NULL) )
		{
			strDest = lpwszSrc;

			return (int)strDest.GetLength ();
		}
	}

	int nLength = GetFullPathNameW (lpwszSrc, 0, NULL, NULL)+1;

	wchar_t *lpwszDest = strDest.GetBuffer (nLength);
	GetFullPathNameW (lpwszSrc, nLength, lpwszDest, NULL);


	// ��� ����� ����� � ������ cd //host/share
	// � ������ ����� �� ���� c:\\host\share

	if ( lpwszSrc[0] == L'/' &&
				lpwszSrc[1] == L'/' &&
				lpwszDest[1] == L':' &&
				lpwszDest[3] == L'\\' )
				memmove (lpwszDest, lpwszDest+2, (wcslen (lpwszDest+2)+1)*sizeof (wchar_t));

	strDest.ReleaseBuffer (nLength);

	return (int)strDest.GetLength ();
}

/*
  ����������� Src � ������ �������� ���� � ������ reparse point � Win2K
  ���� OS ����, �� ���������� ������� ConvertNameToFull()
*/
int WINAPI ConvertNameToReal (const wchar_t *Src, string &strDest)
{
  string strTempDest;
  BOOL IsAddEndSlash=FALSE; // =TRUE, ���� ���� ��������� ��������������
                            // � ����� �� ��� ����... ������.

  // ������� ������� ������ ���� �� ������� ������� ��������
  int Ret=ConvertNameToFull(Src, strTempDest);
  //RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));
  _SVS(SysLog(L"ConvertNameToFull('%S') -> '%S'",Src,(const wchar_t*)strTempDest));

  wchar_t *TempDest;
  /* $ 14.06.2003 IS
     ��� ����������� ������ ���� � �� �������� ������������� ��������
  */
  // ��������� �������� Win2K, �.�. � ������ ���� ������ ���� ���������
  // �������, ����������� ������ �������� ��� �����.
  // ����� ������ �� ������ ��� ����������� ������, �.�. ��� ��� ���������� ������
  // ���������� ���������� ��� ������, �� ������� ��������� ������� (�.�. ����������
  // "������������ �������")
  if (IsLocalDrive(strTempDest) &&
      WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
  {
    DWORD FileAttr;

    if((FileAttr=GetFileAttributesW(strTempDest)) != -1 && (FileAttr&FILE_ATTRIBUTE_DIRECTORY))
    {
      AddEndSlash(strTempDest);
      IsAddEndSlash=TRUE;
    }

    TempDest = strTempDest.GetBuffer (2048); //BUGBUGBUG!!!!
    wchar_t *Ptr, Chr;

    Ptr = TempDest+StrLength(TempDest);

    const wchar_t *CtrlChar = TempDest;

    if (StrLength(TempDest) > 2 && TempDest[0]==L'\\' && TempDest[1]==L'\\')
      CtrlChar= wcschr(TempDest+2, L'\\');

    // ������� ���� ������� ����� �� �����
    while(CtrlChar)
    {
      while(Ptr > TempDest && *Ptr != L'\\')
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
      FileAttr=GetFileAttributesW(TempDest);
      // �! ��� ��� ������ - ���� �� "���������" ���� - �������
      if(FileAttr != (DWORD)-1 && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
      {
        string strTempDest2;

//        if(CheckParseJunction(TempDest,sizeof(TempDest)))
        {
          // ������� ���� ��������
          if(GetJunctionPointInfo(TempDest, strTempDest2))
          {
            strTempDest.LShift (4); //???
            // ��� ������ �������������� ����� (�� �������� �����)...
            if(!wcsncmp(strTempDest2,L"Volume{",7))
            {
              string strJuncRoot;
              // ������� ���� ����� �����, ����...
              GetPathRootOne(strTempDest2, strJuncRoot);
              // ...�� � ����� ������ ����� ���������.
              strTempDest2 = strJuncRoot;
            }

            *Ptr=Chr; // ����������� ������
            DeleteEndSlash(strTempDest2);
            strTempDest2 = Ptr;
            wcscpy(TempDest,strTempDest2); //BUGBUG
            Ret=StrLength(TempDest);
            // ���. �������� ���� � ��� � �������...
            break;
          }
        }
      }
      *Ptr=Chr;
      --Ptr;
    }

    strTempDest.ReleaseBuffer ();
  }

  TempDest = strTempDest.GetBuffer ();

  if(IsAddEndSlash) // ���� �� ������� - ������.
    TempDest[StrLength(TempDest)-1]=0;

  strTempDest.ReleaseBuffer ();

  strDest = strTempDest;

  return Ret;
}


void ConvertNameToShort(const wchar_t *Src, string &strDest)
{
	string strCopy = NullToEmpty(Src);

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
	string strCopy = NullToEmpty(Src);

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
