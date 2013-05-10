/*
global.cpp

���������� ����������
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

#include "imports.hpp"
#include "console.hpp"
#include "scrbuf.hpp"
#include "TaskBar.hpp"
#include "format.hpp"
#include "TPreRedrawFunc.hpp"
#include "window.hpp"
#include "config.hpp"
#include "language.hpp"
#include "elevation.hpp"
#include "treelist.hpp"
#include "interf.hpp"
#include "PluginSynchro.hpp"
#include "codepage.hpp"
#include "configdb.hpp"
#include "ctrlobj.hpp"
#include "edit.hpp"

global::global():
	m_MainThreadId(GetCurrentThreadId()),
	ifn(nullptr),
	Console(nullptr),
	ScrBuf(nullptr),
	TBC(nullptr),
	ConsoleIcons(nullptr),
	//FS(nullptr),
	PreRedraw(nullptr),
	Window(nullptr),
	Opt(nullptr),
	Lang(nullptr),
	OldLang(nullptr),
	Elevation(nullptr),
	TreeCache(nullptr),
	tempTreeCache(nullptr),
	PluginSynchroManager(nullptr),
	CodePages(nullptr),
	Db(nullptr),
	CtrlObject(nullptr)
{
	Global = this;

	QueryPerformanceCounter(&m_FarUpTime);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &m_MainThreadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);

	// BUGBUG

	// ���� ������� ���������� ������� � �������?
	IsProcessAssignMacroKey=FALSE;
	// ��� ������� ����������� ���� �������
	IsRedrawFramesInProcess=FALSE;
	PluginPanelsCount = 0;
	// ���� ������� �������� ������ � �������?
	WaitInFastFind=FALSE;
	// �� �������� � �������� �����?
	WaitInMainLoop=FALSE;
	StartIdleTime=0;
	GlobalSearchCase=false;
	GlobalSearchWholeWords=false; // �������� "Whole words" ��� ������
	GlobalSearchHex=false;     // �������� "Search for hex" ��� ������
	GlobalSearchReverse=false;
	ScreenSaverActive=FALSE;
	CloseFAR=FALSE;
	CloseFARMenu=FALSE;
	AllowCancelExit=TRUE;
	DisablePluginsOutput=FALSE;
	ProcessException=FALSE;
	ProcessShowClock=FALSE;
	HelpFileMask=L"*.hlf";
	HelpFormatLinkModule=L"<%s>%s";
#if defined(SYSLOG)
	StartSysLog=0;
#endif
#ifdef DIRECT_RT
	DirectRT = false;
#endif
	GlobalSaveScrPtr=nullptr;
	CriticalInternalError=FALSE;
	KeepUserScreen = 0;
	Macro_DskShowPosType=0; // ��� ����� ������ �������� ���� ������ ������ (0 - ������� �� ��������, 1 - ����� (AltF1), 2 - ������ (AltF2))
	ErrorMode = SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX;
#ifndef NO_WRAPPER
	strRegRoot = L"Software\\Far Manager";
#endif // NO_WRAPPER

	// BUGBUG end

	ifn = new ImportedFunctions;
	Console = console::CreateInstance(true);
	ScrBuf = new ScreenBuf;
	TBC = new TaskBarCore;
	ConsoleIcons = new consoleicons;
	//FS = new FormatScreen;
	PreRedraw = new TPreRedrawFunc;
	Window = new WindowHandler;
	Opt = new Options;
	Lang = new Language;
	OldLang = new Language;
	Elevation = new elevation;
	TreeCache = new TreeListCache;
	tempTreeCache = new TreeListCache;
	PluginSynchroManager = new PluginSynchro;
	CodePages = new codepages;
	Sets = new sets;
}

global::~global()
{
	delete CtrlObject;
	CtrlObject = nullptr;
	delete Db;
	Db = nullptr;
	delete Sets;
	Sets = nullptr;
	delete CodePages;
	CodePages = nullptr;
	delete PluginSynchroManager;
	PluginSynchroManager = nullptr;
	delete tempTreeCache;
	tempTreeCache = nullptr;
	delete TreeCache;
	TreeCache = nullptr;
	delete Elevation;
	Elevation = nullptr;
	delete OldLang;
	OldLang = nullptr;
	delete Lang;
	Lang = nullptr;
	delete Opt;
	Opt = nullptr;
	delete Window;
	Window = nullptr;
	delete PreRedraw;
	PreRedraw = nullptr;
	//delete FS;
	//FS = nullptr;
	delete ConsoleIcons;
	ConsoleIcons = nullptr;
	delete TBC;
	TBC = nullptr;
	delete ScrBuf;
	ScrBuf = nullptr;
	delete Console;
	Console = nullptr;
	delete ifn;
	ifn = nullptr;

	CloseHandle(m_MainThreadHandle);

	Global = nullptr;
}

bool global::IsUserAdmin() const
{
	static bool Checked = false;
	static bool Result = false;

	if(!Checked)
	{
		SID_IDENTIFIER_AUTHORITY NtAuthority=SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup;
		if(AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup))
		{
			BOOL IsMember = FALSE;
			if(CheckTokenMembership(nullptr, AdministratorsGroup, &IsMember) && IsMember)
			{
				Result = true;
			}
			FreeSid(AdministratorsGroup);
		}
		Checked = true;
	}
	return Result;
}

const OSVERSIONINFO& global::WinVer() const
{
	static OSVERSIONINFO Info;
	static bool Checked = false;
	if(!Checked)
	{
		Info.dwOSVersionInfoSize = sizeof(Info);
		GetVersionEx(&Info);
		Checked = true;
	}
	return Info;
}

bool global::IsPtr(const void* Address) const
{
	static SYSTEM_INFO info;
	static bool Checked = false;
	if(!Checked)
	{
		GetSystemInfo(&info);
		Checked = true;
	}
	return reinterpret_cast<uintptr_t>(Address) >= reinterpret_cast<uintptr_t>(info.lpMinimumApplicationAddress) &&
		reinterpret_cast<uintptr_t>(Address) <= reinterpret_cast<uintptr_t>(info.lpMaximumApplicationAddress);
}

#include "bootstrap/copyright.inc"
