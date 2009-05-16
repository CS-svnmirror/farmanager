/*
global.cpp

���������� ����������
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

/* $ 29.06.2000 tran
  ����� char *CopyRight �� inc ����� */
#include "copyright.inc"

/* $ 07.12.2000 SVS
   + ������ ������� �� ����� farversion.inc
*/
#include "farversion.inc"

OSVERSIONINFOW WinVer={0};

struct Options Opt;// BUG !! ={0};

// �������� ���� ��������?
bool LanguageLoaded=false;

// ���� �� ������ ������ Ctrl-Alt-Shift
BOOL NotUseCAS=FALSE;

// ���� ������� ���������� ������� � �������?
BOOL IsProcessAssignMacroKey=FALSE;

// ���� ������� "������/��������" �� ����� ������ ������?
BOOL IsProcessVE_FindFile=FALSE;

// ��� ������� ����������� ���� �������
BOOL IsRedrawFramesInProcess=FALSE;

// ���� ������� �������� ������ � �������?
int WaitInFastFind=FALSE;

// �� �������� � �������� �����?
int WaitInMainLoop=FALSE;


CONSOLE_SCREEN_BUFFER_INFO InitScreenBufferInfo={0};
CONSOLE_SCREEN_BUFFER_INFO CurScreenBufferInfo={0};
int ScrX=0,ScrY=0;
int PrevScrX=-1,PrevScrY=-1;
HANDLE hConOut=NULL,hConInp=NULL;

clock_t StartIdleTime=0;

DWORD InitialConsoleMode=0;

clock_t StartExecTime=0;

string g_strFarPath;

string strLastFarTitle;
int  TitleModified=FALSE;
wchar_t RegColorsHighlight[]=L"Colors\\Highlight";


string strGlobalSearchString;
int GlobalSearchCase=FALSE;
int GlobalSearchWholeWords=FALSE; // �������� "Whole words" ��� ������
int GlobalSearchSelFound=FALSE; // �������� "Select found" ��� ������
int GlobalSearchHex=FALSE;     // �������� "Search for hex" ��� ������
int GlobalSearchReverse=FALSE;

int ScreenSaverActive=FALSE;

FileEditor *CurrentEditor=NULL;
int CloseFAR=FALSE,CloseFARMenu=FALSE;

int CmpNameSearchMode=FALSE;
int DisablePluginsOutput=FALSE;
int CmdMode=FALSE;

int WidthNameForMessage=0;

const wchar_t DOS_EOL_fmt[]  = L"\r\n";
const wchar_t UNIX_EOL_fmt[] = L"\n";
const wchar_t MAC_EOL_fmt[]  = L"\r";
const wchar_t WIN_EOL_fmt[]  = L"\r\r\n";

const char DOS_EOL_fmtA[]  = "\r\n";
const char UNIX_EOL_fmtA[] = "\n";
const char MAC_EOL_fmtA[]  = "\r";
const char WIN_EOL_fmtA[]  = "\r\r\n";


BOOL ProcessException=FALSE;
BOOL ProcessShowClock=FALSE;

const wchar_t *FarTitleAddons=L" - Far";

const wchar_t FAR_VerticalBlock[]= L"FAR_VerticalBlock";
const wchar_t FAR_VerticalBlock_Unicode[]= L"FAR_VerticalBlock_Unicode";

int InGrabber=FALSE;

const wchar_t *HelpFileMask=L"*.hlf";
const wchar_t *HelpFormatLinkModule=L"<%s>%s";

#if defined(SYSLOG)
BOOL StartSysLog=0;
long CallNewDelete=0;
long CallMallocFree=0;
#endif

class SaveScreen;
SaveScreen *GlobalSaveScrPtr=NULL;

int CriticalInternalError=FALSE;

int UsedInternalClipboard=0;

#if defined(DETECT_ALT_ENTER)
int PrevFarAltEnterMode=-1;
#endif

WCHAR BoxSymbols[64];

int _localLastError=0;

const wchar_t *ReservedFilenameSymbols=L"<>|";

int KeepUserScreen;
string g_strDirToSet;

int Macro_DskShowPosType=0; // ��� ����� ������ �������� ���� ������ ������ (0 - ������� �� ��������, 1 - ����� (AltF1), 2 - ������ (AltF2))

const wchar_t *FavoriteCodePagesKey=L"CodePages\\Favorites";

// Macro Const
const wchar_t constMsX[]=L"MsX";
const wchar_t constMsY[]=L"MsY";
const wchar_t constMsButton[]=L"MsButton";
const wchar_t constMsCtrlState[]=L"MsCtrlState";
