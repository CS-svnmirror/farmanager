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
#include "synchro.hpp"
#include "codepage.hpp"

global::global()
{
	Global = this;

	#include "bootstrap/copyright.inc"

	WinVer.dwOSVersionInfoSize = sizeof(WinVer);

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

	WidthNameForMessage=0;

	ProcessException=FALSE;
	ProcessShowClock=FALSE;

	HelpFileMask=L"*.hlf";
	HelpFormatLinkModule=L"<%s>%s";

#if defined(SYSLOG)
	StartSysLog=0;
	CallNewDelete=0;
	CallMallocFree=0;
#endif

#ifdef DIRECT_RT
	DirectRT = false;
#endif

	GlobalSaveScrPtr=nullptr;

	CriticalInternalError=FALSE;

	KeepUserScreen = 0;

	Macro_DskShowPosType=0; // ��� ����� ������ �������� ���� ������ ������ (0 - ������� �� ��������, 1 - ����� (AltF1), 2 - ������ (AltF2))

	MainThreadId = GetCurrentThreadId();
	ErrorMode = SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX;


	ifn = new ImportedFunctions;
	Console = new console;
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
}

global::~global()
{
	delete CodePages;
	delete PluginSynchroManager;
	delete tempTreeCache;
	delete TreeCache;
	delete Elevation;
	delete OldLang;
	delete Lang;
	delete Opt;
	delete Window;
	delete PreRedraw;
	//delete FS;
	delete ConsoleIcons;
	delete TBC;
	delete ScrBuf;
	delete Console;
	delete ifn;
}