#pragma once

/*
datetime.hpp

������� ��� ������ � ����� � ��������
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
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

DWORD ConvertYearToFull(DWORD ShortYear);
int GetDateFormat();
wchar_t GetDateSeparator();
wchar_t GetTimeSeparator();

void OnIntlSettingsChange();

void GetFileDateAndTime(const string& Src,LPWORD Dst,size_t Count,int Separator);
void StrToDateTime(const string& CDate, const string& CTime, FILETIME &ft, int DateFormat, wchar_t DateSeparator, wchar_t TimeSeparator, bool bRelative=false);
void ConvertDate(const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength, int Brief=FALSE,int TextMonth=FALSE,int FullYear=0);
void ConvertRelativeDate(const FILETIME &ft,string &strDaysText,string &strTimeText);

void PrepareStrFTime();
size_t StrFTime(string &strDest, const wchar_t *Format,const tm *t);
size_t MkStrFTime(string &strDest, const wchar_t *Fmt=nullptr);

inline uint64_t FileTimeToUI64(const FILETIME& ft)
{
	ULARGE_INTEGER t = {ft.dwLowDateTime, ft.dwHighDateTime};
	return t.QuadPart;
}

inline FILETIME UI64ToFileTime(uint64_t time)
{
	ULARGE_INTEGER i;
	i.QuadPart = time;
	FILETIME ft;
	ft.dwLowDateTime = i.LowPart;
	ft.dwHighDateTime = i.HighPart;
	return ft;
}

inline int CompareFileTime(const FILETIME& a, const FILETIME& b)
{
	__int64 Result = FileTimeToUI64(a) - FileTimeToUI64(b);
	return Result ? (Result > 0 ? 1 : -1) : 0;
}

inline bool operator==(const FILETIME& a, const FILETIME& b)
{
	return a.dwLowDateTime == b.dwLowDateTime && a.dwHighDateTime == b.dwHighDateTime;
}

inline bool operator!=(const FILETIME& a, const FILETIME& b)
{
	return !(a == b);
}

inline bool operator<(const FILETIME& a, const FILETIME& b)
{
	return CompareFileTime(a, b) < 0;
}

inline uint64_t GetCurrentUTCTimeInUI64()
{
	FILETIME Timestamp;
	GetSystemTimeAsFileTime(&Timestamp); // in UTC
	return FileTimeToUI64(Timestamp);
}

bool Utc2Local(const FILETIME &ft, SYSTEMTIME &lst);
bool Local2Utc(const FILETIME &lft, SYSTEMTIME &st);
bool Utc2Local(const SYSTEMTIME &st, FILETIME &lft);
bool Local2Utc(const SYSTEMTIME &lst, FILETIME &ft);
