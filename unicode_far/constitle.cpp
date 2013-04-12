/*
constitle.cpp

��������� �������
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

#include "headers.hpp"
#pragma hdrstop

#include "constitle.hpp"
#include "language.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "ctrlobj.hpp"
#include "synchro.hpp"
#include "console.hpp"
#include "farversion.hpp"
#include "scrbuf.hpp"

static const string& GetFarTitleAddons()
{
	// " - Far%Ver%Admin"
	/*
		%Ver      - 2.0
		%Build    - 1259
		%Platform - x86
		%Admin    - MFarTitleAddonsAdmin
		%PID      - current PID
    */
	static FormatString strVer, strBuild, strPID;
	static bool bFirstRun = true;
	static string strTitleAddons;

	strTitleAddons.Copy(L" - Far ",7);
	strTitleAddons += Global->Opt->strTitleAddons.Get();

	if (bFirstRun)
	{
		bFirstRun = false;
		strVer<<FAR_VERSION.Major<<L"."<<FAR_VERSION.Minor;
		strBuild<<FAR_VERSION.Build;
		strPID<<GetCurrentProcessId();
	}

	ReplaceStrings(strTitleAddons,L"%PID",strPID,-1,true);
	ReplaceStrings(strTitleAddons,L"%Ver",strVer,-1,true);
	ReplaceStrings(strTitleAddons,L"%Build",strBuild,-1,true);
	ReplaceStrings(strTitleAddons,L"%Platform",
#ifdef _WIN64
#ifdef _M_IA64
	L"IA64",
#else
	L"x64",
#endif
#else
	L"x86",
#endif
	-1,true);
	ReplaceStrings(strTitleAddons,L"%Admin",Global->IsUserAdmin()?MSG(MFarTitleAddonsAdmin):L"",-1,true);
	RemoveTrailingSpaces(strTitleAddons);

	return strTitleAddons;
}

bool ConsoleTitle::TitleModified = false;
DWORD ConsoleTitle::ShowTime = 0;

CriticalSection TitleCS;

ConsoleTitle::ConsoleTitle()
{
	CriticalSectionLock Lock(TitleCS);
	Global->Console->GetTitle(strOldTitle);
}

ConsoleTitle::ConsoleTitle(const string& title)
{
	CriticalSectionLock Lock(TitleCS);
	Global->Console->GetTitle(strOldTitle);
	SetFarTitle(title);
}

ConsoleTitle::~ConsoleTitle()
{
	CriticalSectionLock Lock(TitleCS);
	const string &strTitleAddons = GetFarTitleAddons();
	size_t OldLen = strOldTitle.GetLength();
	size_t AddonsLen = strTitleAddons.GetLength();

	if (AddonsLen <= OldLen)
	{
		if (!StrCmpI(strOldTitle.CPtr()+OldLen-AddonsLen, strTitleAddons.CPtr()))
			strOldTitle.SetLength(OldLen-AddonsLen);
	}

	SetFarTitle(strOldTitle);
}

BaseFormat& ConsoleTitle::Flush()
{
	SetFarTitle(*this);
	Clear();
	return *this;
}

static string& FarTitle()
{
	static string strFarTitle;
	return strFarTitle;
}
void ConsoleTitle::SetFarTitle(const string& Title)
{//MessageBox(0,0,Title.CPtr(),0);
	CriticalSectionLock Lock(TitleCS);
	string strOldFarTitle;

	Global->Console->GetTitle(strOldFarTitle);
	FarTitle() = Title;
	FarTitle() += GetFarTitleAddons();
	TitleModified=true;

	if (strOldFarTitle != FarTitle() && !Global->ScrBuf->GetLockCount())
	{
		Global->Console->SetTitle(FarTitle());
		TitleModified=true;
	}
}

void ConsoleTitle::RestoreTitle()
{
	if(Global->ScrBuf->GetLockCount()==0)
	{
		/*
			RestoreTitle() ��� ������, ����� ����� ��������� ����.���������
			�� ��� ����!
			���� ����� ����� ����� ������ ������ �����-������!
		*/
		Global->Console->SetTitle(FarTitle());
		TitleModified=false;
		//_SVS(SysLog(L"  (nullptr)FarTitle='%s'",FarTitle));
	}
}
