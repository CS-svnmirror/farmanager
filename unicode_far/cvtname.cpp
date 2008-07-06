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
				(*lpwszName != L'.' || (lpwszName[1] != 0 && (lpwszName[1] != L'.' || lpwszName[2] != 0)) ) &&
				(wcsstr (lpwszSrc, L"\\..\\") == NULL && wcsstr (lpwszSrc, L"\\.\\") == NULL) )
		{
			strDest = lpwszSrc;

			return (int)strDest.GetLength ();
		}
	}

	int nLength = GetFullPathNameW (lpwszSrc, 0, NULL, NULL);
	wchar_t *lpwszDest = strDest.GetBuffer (nLength);
	GetFullPathNameW (lpwszSrc, nLength, lpwszDest, NULL);

	// ��� ����� ����� � ������ cd //host/share
	// � ������ ����� �� ���� c:\\host\share

	if ( lpwszSrc[0] == L'/' &&
				lpwszSrc[1] == L'/' &&
				lpwszDest[1] == L':' &&
				lpwszDest[3] == L'\\' )
				wmemmove (lpwszDest, lpwszDest+2, wcslen (lpwszDest+2)+1);

	strDest.ReleaseBuffer ();

	return (int)strDest.GetLength ();
}

/*
  ����������� Src � ������ �������� ���� � ������ reparse point � Win2K
  ���� OS ����, �� ���������� ������� ConvertNameToFull()
*/
int WINAPI ConvertNameToReal (const wchar_t *Src, string &strDest, bool Internal)
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

    if((FileAttr=GetFileAttributesW(strTempDest)) != INVALID_FILE_ATTRIBUTES && (FileAttr&FILE_ATTRIBUTE_DIRECTORY))
    {
      AddEndSlash(strTempDest);
      IsAddEndSlash=TRUE;
    }

    TempDest = strTempDest.GetBuffer();
    wchar_t *Ptr, Chr;

    Ptr = TempDest+StrLength(TempDest);

    const wchar_t *CtrlChar = TempDest;

    if (StrLength(TempDest) > 2 && TempDest[0]==L'\\' && TempDest[1]==L'\\')
      CtrlChar = wcschr(TempDest+2, L'\\');

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
      if(FileAttr != INVALID_FILE_ATTRIBUTES && (FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
      {
        string strTempDest2;
        // ������� ���� ��������
        if(GetJunctionPointInfo(TempDest, strTempDest2))
        {
          // ������� \\??\ �� ���� ��������
          strTempDest2.LShift(4);
          // ��� ������ �������������� ����� (�� �������� �����)...
          if(!wcsncmp(strTempDest2, L"Volume{", 7))
          {
            string strJuncRoot;
            // ������� ���� ����� �����, ����...
            GetPathRootOne(strTempDest2, strJuncRoot);
            // ...�� � ����� ������ ����� ���������.
            // (�������� - ���� ����� �� �������� - ����� ����� ������������)
            strTempDest2 = (strJuncRoot.At(1)==L':'||!Internal)?strJuncRoot:TempDest;
          }
          DeleteEndSlash(strTempDest2);
          // ����� ��������
          wchar_t* TempDest2 = strTempDest2.GetBuffer();
          // ����� ���� ��������
          size_t tempLength = StrLength(TempDest2);
          // �������� ����� ����� � ������ ������ ����
          size_t leftLength = StrLength(TempDest);
          size_t rightLength = StrLength(Ptr + 1); // �������� ����� ���� ������� �� ���������� ������� ����� �������
          // ����������� ������
          *Ptr=Chr;
          // ���� ���� �������� ������ ����� ����� ����, ����������� �����
          if (leftLength < tempLength)
          {
            // �������� ����� �����
            TempDest = strTempDest.GetBuffer((int)(strTempDest.GetLength() + tempLength - leftLength + (IsAddEndSlash?2:1)));
          }
          // ��� ��� �� ����������� ����������� � ����� ������ ���� �������� ��������� ��
          // ������� ������� ������� � ����
          Ptr = TempDest + tempLength - 1;
          // ���������� ������ ����� ���� �� ������ �����, ������ ���� ����� ���� ���������� ��
          // ������� �� ���� ��������
          if (leftLength != tempLength)
          {
            // ���������� ����� �������� ��� �����, ��������� '/', �������� '/' (���� �� ����) � '\0'
            wmemmove(TempDest + tempLength, TempDest + leftLength, rightLength + (IsAddEndSlash ? 3 : 2));
          }
          // �������� ���� � �������� ������� ����
          wmemcpy(TempDest, TempDest2, tempLength);
          // ��������� ������ �� ������ ���������� ����������� �� ����
          CtrlChar = TempDest;
          if (StrLength(TempDest) > 2 && TempDest[0] == L'\\' && TempDest[1] == L'\\')
          {
            CtrlChar = wcschr(TempDest + 2, L'\\');
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

int WINAPI OldConvertNameToReal(const wchar_t *Src, string &strDest)
{
	return ConvertNameToReal(Src,strDest,false);
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
