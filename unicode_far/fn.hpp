#ifndef __FARFUNC_HPP__
#define __FARFUNC_HPP__
/*
fn.hpp

�������� �������
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

#include "farconst.hpp"
#include "global.hpp"
#include "plugin.hpp"
#include "filefilter.hpp"

#define countof(a) (sizeof(a)/sizeof(a[0]))

char *UnicodeToAnsi (const wchar_t *lpwszUnicodeString, int nMaxLength = -1);
void UnicodeToAnsi (const wchar_t *lpwszUnicodeString, char *lpDest, int nMaxLength = -1); //BUGBUG

void SetHighlighting();
void _export StartFAR();
void Box(int x1,int y1,int x2,int y2,int Color,int Type);
/*$ 14.02.2001 SKV
  ������� �� ������� default ����������.
  �� ��������� - ��.
  � 0 ������������ ��� ConsoleDetach.
*/
void InitConsole(int FirstInit=TRUE);
void InitRecodeOutTable(UINT cp=0);
void CloseConsole();
void SetFarConsoleMode(BOOL SetsActiveBuffer=FALSE);
void ChangeVideoMode(int NumLines,int NumColumns);
void ChangeVideoMode(int Maximized);
void SetVideoMode(int ConsoleMode);
void GetVideoMode(CONSOLE_SCREEN_BUFFER_INFO &csbi);
void GotoXY(int X,int Y);
int WhereX();
int WhereY();
void MoveCursor(int X,int Y);
void GetCursorPos(int& X,int& Y);
void SetCursorType(int Visible,int Size);
void SetInitialCursorType();
void GetCursorType(int &Visible,int &Size);
void MoveRealCursor(int X,int Y);
void GetRealCursorPos(int& X,int& Y);
void SetRealCursorType(int Visible,int Size);
void GetRealCursorType(int &Visible,int &Size);

void Text(int X, int Y, int Color, const WCHAR *Str);
void Text(const WCHAR *Str);
void Text(int MsgId);
void VText(const WCHAR *Str);
void HiText(const WCHAR *Str,int HiColor,int isVertText=0);
#define HiVText(Str,HiColor) HiText(Str,HiColor,1)

void DrawLine(int Length,int Type, const wchar_t* UserSep=NULL);
#define ShowSeparator(Length,Type) DrawLine(Length,Type)
#define ShowUserSeparator(Length,Type,UserSep) DrawLine(Length,Type,UserSep)

WCHAR* MakeSeparator(int Length,WCHAR *DestStr,int Type=1, const wchar_t* UserSep=NULL);
void SetScreen(int X1,int Y1,int X2,int Y2,wchar_t Ch,int Color);
void MakeShadow(int X1,int Y1,int X2,int Y2);
void ChangeBlockColor(int X1,int Y1,int X2,int Y2,int Color);
void SetColor(int Color);
void SetRealColor(int Color);
void ClearScreen(int Color);
int  GetColor();
void GetText(int X1,int Y1,int X2,int Y2,void *Dest,int DestSize);
void PutText(int X1,int Y1,int X2,int Y2,const void *Src);
void GetRealText(int X1,int Y1,int X2,int Y2,void *Dest);
void PutRealText(int X1,int Y1,int X2,int Y2,const void *Src);
void _GetRealText(HANDLE hConsoleOutput,int X1,int Y1,int X2,int Y2,const void *Src,int BufX,int BufY);
void _PutRealText(HANDLE hConsoleOutput,int X1,int Y1,int X2,int Y2,const void *Src,int BufX,int BufY);

void mprintf(const WCHAR *fmt,...);
void vmprintf(const WCHAR *fmt,...);

inline WORD GetVidChar(CHAR_INFO CI)
{
  return CI.Char.UnicodeChar;
}

inline void SetVidChar(CHAR_INFO& CI,wchar_t Chr)
{
	extern wchar_t Oem2Unicode[];
	CI.Char.UnicodeChar = (Chr<L'\x20'||Chr==L'\x7f')?Oem2Unicode[Chr]:Chr;
}

void ShowTime(int ShowAlways);
int GetDateFormat();
int GetDateSeparator();
int GetTimeSeparator();

const wchar_t *GetShellAction(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& Error);
void ScrollScreen(int Count);
int ScreenSaver(int EnableExit);

string &FormatNumber(const wchar_t *Src, string &strDest, int NumDigits=0);
string &InsertCommas(unsigned __int64 li, string &strDest);

void DeleteDirTree(const wchar_t *Dir);

void InitDetectWindowedMode();
void DetectWindowedMode();
int IsWindowed();
void RestoreIcons();
void Log(char *fmt,...);
void BoxText(WORD Chr);
void BoxText(const wchar_t *Str,int IsVert=0);
int FarColorToReal(int FarColor);
void ConvertCurrentPalette();
void ReopenConsole();

string &RemoveChar(string &strStr,wchar_t Target,BOOL Dup=TRUE);

wchar_t *InsertString(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize=0);

int ReplaceStrings(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count=-1,BOOL IgnoreCase=FALSE);

#define RemoveHighlights(Str) RemoveChar(Str,L'&')

BOOL IsCaseMixed(const string &strStr);
BOOL IsCaseLower(const string &strStr);

int DeleteFileWithFolder(const wchar_t *FileName);

const wchar_t* GetLanguageString (int nID);

#define MSG(ID) GetLanguageString(ID)

int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2=NULL,const wchar_t *Str3=NULL,const wchar_t *Str4=NULL,
            INT_PTR PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6=NULL,const wchar_t *Str7=NULL,
            INT_PTR PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9=NULL,const wchar_t *Str10=NULL,
            INT_PTR PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9,const wchar_t *Str10,
            const wchar_t *Str11,const wchar_t *Str12=NULL,const wchar_t *Str13=NULL,
            const wchar_t *Str14=NULL, INT_PTR PluginNumber=-1);

//int __cdecl MessageW (DWORD Flags,int Buttons,const char *Title, INT_PTR PluginNumber, ...);

int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t * const *Items,
            int ItemsNumber,INT_PTR PluginNumber=-1);

/* $ 12.03.2002 VVM
  ����� ������� - ������������ ��������� �������� ��������.
  ������� ������.
  ����������:
   FALSE - ���������� ��������
   TRUE  - �������� ��������
*/
int AbortMessage();

void SetMessageHelp(const wchar_t *Topic);
void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2);
int ToPercent(unsigned long N1,unsigned long N2);
int ToPercent64(unsigned __int64 N1,unsigned __int64 N2);
// ����������: 1 - LeftPressed, 2 - Right Pressed, 3 - Middle Pressed, 0 - none
int IsMouseButtonPressed();
int CmpName(const wchar_t *pattern,const wchar_t *str,int skippath=TRUE);
// ���������� ��� �����: �������� � ������, �������, ������������� �� �����
int WINAPI ProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);

wchar_t* WINAPI QuoteSpace(wchar_t *Str);
string &QuoteSpace(string &strStr);


wchar_t* WINAPI InsertQuote(wchar_t *Str);
string& InsertQuote(string& strStr);

int ProcessGlobalFileTypes(const wchar_t *Name,int AlwaysWaitFinish);
int ProcessLocalFileTypes(const wchar_t *Name,const wchar_t *ShortName,int Mode,int AlwaysWaitFinish);
void ProcessExternal(const wchar_t *Command,const wchar_t *Name,const wchar_t *ShortName,int AlwaysWaitFinish);

int SubstFileName(string &strStr, const wchar_t *Name, const wchar_t *ShortName,
                  string *strListName=NULL,
                  string *strAnotherListName = NULL,
                  string *strShortListName=NULL,
                  string *strAnotherShortListName=NULL,
                  int IgnoreInput=FALSE,const wchar_t *CmdLineDir=NULL);
BOOL ExtractIfExistCommand(string &strCommandText);
void EditFileTypes();
void ProcessUserMenu(int EditMenu);

int ConvertNameToFull(const wchar_t *lpwszSrc, string &strDest);
int WINAPI OldConvertNameToReal(const wchar_t *Src, string &strDest);
int WINAPI ConvertNameToReal(const wchar_t *Src, string &strDest, bool Internal=true);
void ConvertNameToShort(const wchar_t *Src, string &strDest); //BUGBUG, int
void ConvertNameToLong(const wchar_t *Src, string &strDest); //BUGBUG, int

void ChangeConsoleMode(int Mode);
void FlushInputBuffer();
void SystemSettings();
void PanelSettings();
void InterfaceSettings();
void DialogSettings();
void SetConfirmations();
void SetDizConfig();
int  IsLocalDrive(const wchar_t *Path);
void ViewerConfig(struct ViewerOptions &ViOpt,int Local=0);
void EditorConfig(struct EditorOptions &EdOpt,int Local=0);
void SetFolderInfoFiles();
void ReadConfig();
void SaveConfig(int Ask);
void SetColors();
int GetColorDialog(unsigned int &Color,bool bCentered=false,bool bAddTransparent=false);
int HiStrlen(const wchar_t *Str);
int HiFindRealPos(const wchar_t *Str, int Pos, BOOL ShowAmp);
int GetErrorString (string &strErrStr);
// �������� �� "��������������" ������������� ��... ��������, �������� ����� � ������� �������!
BOOL CheckErrorForProcessed(DWORD Err);
void ShowProcessList();

wchar_t* PasteFormatFromClipboard(const wchar_t *Format);
int CopyFormatToClipboard(const wchar_t *Format,const wchar_t *Data);
wchar_t* PasteFormatFromClipboard(const wchar_t *Format);
wchar_t* WINAPI PasteFromClipboardEx(int max);
BOOL WINAPI FAR_EmptyClipboard(VOID);

bool GetShellType(const wchar_t *Ext, string &strType);

int GetFileTypeByName(const wchar_t *Name);

bool CutToSlash(string &strStr, bool bInclude = false);
string &CutToNameUNC(string &strPath);
string &CutToFolderNameIfFolder(string &strPath);
const wchar_t *PointToNameUNC(const wchar_t *lpwszPath);

void SetFarTitle(const wchar_t *Title);
void LocalUpperInit();
void InitLCIDSort();
void InitKeysArray();
int WINAPI LocalIslower(unsigned Ch);
int WINAPI LocalIsupper(unsigned Ch);
int WINAPI LocalIsalpha(unsigned Ch);
int WINAPI LocalIsalphanum(unsigned Ch);

unsigned WINAPI LocalUpper(unsigned LowerChar);
void WINAPI LocalUpperBuf(char *Buf,int Length);
void WINAPI LocalLowerBuf(char *Buf,int Length);
unsigned WINAPI LocalLower(unsigned UpperChar);
void WINAPI LocalStrupr(char *s1);
void WINAPI LocalStrlwr(char *s1);
int WINAPI LStricmp(const char *s1,const char *s2);
int WINAPI LStrnicmp(const char *s1,const char *s2,int n);
const char * __cdecl LocalStrstri(const char *str1, const char *str2);
const char * __cdecl LocalRevStrstri(const char *str1, const char *str2);

int __cdecl StrLength(const wchar_t *str);

const wchar_t * __cdecl StrStrI(const wchar_t *str1, const wchar_t *str2);
const wchar_t * __cdecl RevStrStrI(const wchar_t *str1, const wchar_t *str2);

void __cdecl UpperBuf(wchar_t *Buf, int Length);
void __cdecl LowerBuf(wchar_t *Buf, int Length);
void __cdecl StrUpper(wchar_t *s1);
void __cdecl StrLower(wchar_t *s1);

wchar_t __cdecl Upper(wchar_t Ch);
wchar_t __cdecl Lower(wchar_t Ch);
int __cdecl StrCmpNI(const wchar_t *s1, const wchar_t *s2, int n);
int __cdecl StrCmpI(const wchar_t *s1, const wchar_t *s2);
int __cdecl IsLower(wchar_t Ch);
int __cdecl IsUpper(wchar_t Ch);
int __cdecl IsAlpha(wchar_t Ch);
int __cdecl IsAlphaNum(wchar_t Ch);

int __cdecl StrCmp(const wchar_t *s1, const wchar_t *s2);
int __cdecl StrCmpN(const wchar_t *s1, const wchar_t *s2, int n);
int __cdecl NumStrCmp(const wchar_t *s1, const wchar_t *s2);
int __cdecl NumStrCmpI(const wchar_t *s1, const wchar_t *s2);

int LocalKeyToKey(int Key);
int GetShortcutFolder(int Key,string *pDestFolder, string *pPluginModule=NULL,
                      string *pPluginFile=NULL,string *pPluginData=NULL);
int SaveFolderShortcut(int Key,string *pSrcFolder,string *pPluginModule=NULL,
                       string *pPluginFile=NULL,string *pPluginData=NULL);
int GetShortcutFolderSize(int Key);
void ShowFolderShortcut();
void ShowFilter();

inline bool IsUnicodeCP(UINT CP){return(CP==CP_UNICODE)||(CP==CP_UTF8)||(CP==CP_UTF7)||(CP==CP_REVERSEBOM);}
UINT GetTableEx (UINT nCurrent,bool bShowUnicode, bool bShowUTF);

#define NullToEmpty(s) (s?s:L"")

string& CenterStr(const wchar_t *Src, string &strDest,int Length);

const wchar_t *GetCommaWord(const wchar_t *Src,string &strWord,wchar_t Separator=L',');

void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total);

int WINAPI GetFileOwner(const wchar_t *Computer,const wchar_t *Name, string &strOwner);

void SIDCacheFlush(void);

void TransformA(unsigned char *Buffer,int &BufLen,const char *ConvStr,char TransformType);
void Transform(string &strBuffer,const wchar_t *ConvStr,wchar_t TransformType);

void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator);
void StrToDateTime(const wchar_t *CDate, const wchar_t *CTime, FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator, bool bRelative=false);

bool CheckFileSizeStringFormat(const wchar_t *FileSizeStr);
unsigned __int64 ConvertFileSizeString(const wchar_t *FileSizeStr);

void ConvertDate(const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
        int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE,int DynInit=FALSE);
void ConvertRelativeDate(const FILETIME &ft,string &strDaysText,string &strTimeText);

void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent);

int CheckFolder(const wchar_t *Name);
int CheckShortcutFolder(string *pTestPath,int IsHostFile, BOOL Silent=FALSE);

#if defined(__FARCONST_HPP__) && (defined(_INC_WINDOWS) || defined(_WINDOWS_) || defined(_WINDOWS_H))
DWORD NTTimeToDos(FILETIME *ft);
int Execute(const wchar_t *CmdStr,int AlwaysWaitFinish,int SeparateWindow=FALSE,int DirectRun=FALSE,int FolderRun=FALSE);
#endif

class Panel;
void ShellMakeDir(Panel *SrcPanel);
void ShellDelete(Panel *SrcPanel,int Wipe);
int  ShellSetFileAttributes(Panel *SrcPanel);
void PrintFiles(Panel *SrcPanel);
void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir=FALSE);
int  CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName);

BOOL GetDiskSize(const wchar_t *Root,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree);

int GetDirInfo(const wchar_t *Title,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,unsigned __int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               FileFilter *Filter,
               DWORD Flags=GETDIRINFO_SCANSYMLINKDEF);
int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize);

int DetectTable(FILE *SrcFile,struct CharTableSet *TableSet,int &TableNum);

#ifdef __PLUGIN_HPP__
/* $ 17.03.2002 IS
   �������� UseTableName - � �������� ����� ������� ������������ �� ��� �����
   �������, � ��������������� ����������.
   �� ��������� - FALSE (������������ ��� �����).
*/
int PrepareTable(struct CharTableSet *TableSet,int TableNum,BOOL UseTableName=FALSE);
#endif


#ifdef __PLUGIN_HPP__

//----------- PLUGIN API/FSF ---------------------------------------------------
//��� ��� �������, �� ���������� sprintf/sscanf ����� ��� ������ __stdcall

void __stdcall farUpperBuf(wchar_t *Buf, int Length);
void __stdcall farLowerBuf(wchar_t *Buf, int Length);
void __stdcall farStrUpper(wchar_t *s1);
void __stdcall farStrLower(wchar_t *s1);
wchar_t __stdcall farUpper(wchar_t Ch);
wchar_t __stdcall farLower(wchar_t Ch);
int __stdcall farStrCmpNI(const wchar_t *s1, const wchar_t *s2, int n);
int __stdcall farStrCmpI(const wchar_t *s1, const wchar_t *s2);
int __stdcall farIsLower(wchar_t Ch);
int __stdcall farIsUpper(wchar_t Ch);
int __stdcall farIsAlpha(wchar_t Ch);
int __stdcall farIsAlphaNum(wchar_t Ch);

int WINAPI farGetFileOwner(const wchar_t *Computer,const wchar_t *Name, wchar_t *Owner);

int WINAPI FarGetPluginDirList(INT_PTR PluginNumber,HANDLE hPlugin,
                  const wchar_t *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber);
void WINAPI FarFreePluginDirList(PluginPanelItem *PanelItem, int ItemsNumber);

int WINAPI FarMenuFn(INT_PTR PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,const wchar_t *Title,const wchar_t *Bottom,
           const wchar_t *HelpTopic,const int *BreakKeys,int *BreakCode,
           const struct FarMenuItem *Item, int ItemsNumber);
const wchar_t* WINAPI FarGetMsgFn(INT_PTR PluginHandle,int MsgId);
int WINAPI FarMessageFn(INT_PTR PluginNumber,DWORD Flags,
           const wchar_t *HelpTopic,const wchar_t * const *Items,int ItemsNumber,
           int ButtonsNumber);
int WINAPI FarControl(HANDLE hPlugin,int Command,void *Param);
HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2);
void WINAPI FarRestoreScreen(HANDLE hScreen);

int WINAPI FarGetDirList(const wchar_t *Dir, FAR_FIND_DATA **pPanelItem, int *pItemsNumber);
void WINAPI FarFreeDirList(FAR_FIND_DATA *PanelItem, int nItemsNumber);

int WINAPI FarViewer(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2,int Y2,DWORD Flags, UINT CodePage);
int WINAPI FarEditor(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2, int Y2,DWORD Flags,
                     int StartLine,int StartChar, UINT CodePage);
int WINAPI FarCmpName(const wchar_t *pattern,const wchar_t *string,int skippath);
void WINAPI FarText(int X,int Y,int Color,const wchar_t *Str);
int WINAPI TextToCharInfo(const char *Text,WORD Attr, CHAR_INFO *CharInfo, int Length, DWORD Reserved);
int WINAPI FarEditorControl(int Command,void *Param);

int WINAPI FarViewerControl(int Command,void *Param);

/* ������� ������ ������ */
BOOL WINAPI FarShowHelp(const wchar_t *ModuleName,
                        const wchar_t *HelpTopic,DWORD Flags);

/* ������� ������ GetString ��� �������� - � ������� �����������������.
   ������� ��� ����, ����� �� ����������� ��� GetString.*/

int WINAPI FarInputBox(const wchar_t *Title,const wchar_t *Prompt,
                       const wchar_t *HistoryName,const wchar_t *SrcText,
                       wchar_t *DestText,int DestLength,
                       const wchar_t *HelpTopic,DWORD Flags);
/* �������, ������� ����� ����������� � � ���������, � � �������, �... */
INT_PTR WINAPI FarAdvControl(INT_PTR ModuleNumber, int Command, void *Param);
//  ������� ������������ �������
HANDLE WINAPI FarDialogInit(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2,
                       const wchar_t *HelpTopic, struct FarDialogItem *Item,
                       unsigned int ItemsNumber, DWORD Reserved, DWORD Flags,
                       FARWINDOWPROC Proc, LONG_PTR Param);
int WINAPI FarDialogRun(HANDLE hDlg);
void WINAPI FarDialogFree(HANDLE hDlg);
//  ������� ��������� ������� �� ���������
LONG_PTR WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
// ������� ��������� �������
LONG_PTR WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1, LONG_PTR Param2);

int WINAPI farPluginsControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);

int WINAPI farFileFilterControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
#endif


BOOL UnExpandEnvString(const char *Path, const char *EnvVar, char* Dest, int DestSize);
BOOL PathUnExpandEnvStr(const char *Path, char* Dest, int DestSize);

void WINAPI Unquote(string &strStr);
void WINAPI Unquote(wchar_t *Str);

void UnquoteExternal(string &strStr);

wchar_t* WINAPI RemoveLeadingSpaces(wchar_t *Str);
string& WINAPI RemoveLeadingSpaces(string &strStr);

char* WINAPI RemoveTrailingSpacesA(char *Str);
wchar_t *WINAPI RemoveTrailingSpaces(wchar_t *Str);
string& WINAPI RemoveTrailingSpaces(string &strStr);

wchar_t* WINAPI RemoveExternalSpaces(wchar_t *Str);
string & WINAPI RemoveExternalSpaces(string &strStr);
string & WINAPI RemoveUnprintableCharacters(string &strStr);

wchar_t* __stdcall TruncStr(wchar_t *Str,int MaxLength);
string& __stdcall TruncStr(string &strStr,int MaxLength);

wchar_t* WINAPI TruncStrFromEnd(wchar_t *Str,int MaxLength);
string& __stdcall TruncStrFromEnd(string &strStr, int MaxLength);

wchar_t* __stdcall TruncPathStr(wchar_t *Str, int MaxLength);
string& __stdcall TruncPathStr(string &strStr, int MaxLength);

wchar_t* WINAPI QuoteSpaceOnly(wchar_t *Str);
string& WINAPI QuoteSpaceOnly(string &strStr);
BOOL IsWordDiv(const wchar_t *WordDiv, wchar_t Chr);

const wchar_t* __stdcall PointToName(const wchar_t *lpwszPath);
const wchar_t* __stdcall PointToFolderNameIfFolder(const wchar_t *lpwszPath);
const wchar_t* PointToExt(const wchar_t *lpwszPath);

BOOL  TestParentFolderName(const wchar_t *Name);

BOOL  AddEndSlash(string &strPath, wchar_t TypeSlash);
BOOL  AddEndSlash(string &strPath);

BOOL  AddEndSlash(wchar_t *Path, wchar_t TypeSlash);
BOOL  WINAPI AddEndSlash(wchar_t *Path);

BOOL  WINAPI DeleteEndSlash(string &strPath,bool allendslash=false);

string& ReplaceSlashToBSlash(string& strStr);

int __digit_cnt_0(const wchar_t* s, const wchar_t** beg);
wchar_t *WINAPI FarItoa(int value, wchar_t *string, int radix);
__int64 WINAPI FarAtoi64(const wchar_t *s);
wchar_t *WINAPI FarItoa64(__int64 value, wchar_t *string, int radix);
int WINAPI FarAtoi(const wchar_t *s);
void WINAPI FarQsort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
void WINAPI FarQsortEx(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *),void*);

int WINAPI CopyToClipboard(const wchar_t *Data);
wchar_t* WINAPI PasteFromClipboard(void);


wchar_t* InternalPasteFromClipboard(int AnsiMode);
wchar_t* InternalPasteFromClipboardEx(int max,int AnsiMode);
int InternalCopyToClipboard(const wchar_t *Data,int AnsiMode);


int __stdcall GetString(
		const wchar_t *Title,
		const wchar_t *SubTitle,
		const wchar_t *HistoryName,
		const wchar_t *SrcText,
		string &strDestText,
		int DestLength,
		const wchar_t *HelpTopic = NULL,
		DWORD Flags = 0,
		int *CheckBoxValue = NULL,
		const wchar_t *CheckBoxText = NULL
		);

int WINAPI GetNameAndPassword(const wchar_t *Title,string &strUserName, string &strPassword, const wchar_t *HelpTopic,DWORD Flags);

/* ���������� ������������ FulScreen <-> Windowed
   (� ������ "Vasily V. Moshninov" <vmoshninov@newmail.ru>)
   mode = -2 - GetMode
          -1 - ��� ������
           0 - Windowed
           1 - FulScreen
   Return
           0 - Windowed
           1 - FulScreen
*/
int FarAltEnter(int mode);


wchar_t* WINAPI Xlat(wchar_t *Line,
                    int StartPos,
                    int EndPos,
                    DWORD Flags);

#ifdef __cplusplus
extern "C" {
#endif

void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));

typedef int  (WINAPI *FRSUSERFUNCW)(const FAR_FIND_DATA *FData,const wchar_t *FullName,void *param);
void WINAPI FarRecursiveSearch(const wchar_t *initdir,const wchar_t *mask,FRSUSERFUNCW func,DWORD flags,void *param);

wchar_t* __stdcall FarMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
string& FarMkTempEx(string &strDest, const wchar_t *Prefix=NULL, BOOL WithPath=TRUE);

void CreatePath(string &strPath);

/* $ 15.02.2002 IS
   ��������� ������� ����� � �������� � ������������ ��������������� ����������
   ���������. � ������ ������ ������������ �� ����.
   ���� ChangeDir==FALSE, �� �� ������ �������  ����, � ������ �������������
   ���������� ���������.
*/
BOOL FarChDir(const wchar_t *NewDir,BOOL ChangeDir=TRUE);

// ������� ������ ������� ��������� �������� ����.
// ��� ���������� ���� ������ ����� ����� � uppercase
DWORD FarGetCurDir(string &strBuffer);

void WINAPI DeleteBuffer(void* Buffer);

#ifdef __cplusplus
};
#endif

BOOL EjectVolume(wchar_t Letter,DWORD Flags);
BOOL RemoveUSBDrive(char Letter,DWORD Flags);
BOOL IsEjectableMedia(wchar_t Letter,UINT DriveType=DRIVE_NOT_INIT,BOOL ForceCDROM=FALSE);
BOOL IsDriveUsb(wchar_t DriveName,void *pDevInst);
int  ProcessRemoveHotplugDevice (wchar_t Drive, DWORD Flags);

bool InitializeSetupAPI ();
bool CheckInitSetupAPI ();
void FinalizeSetupAPI ();
void ShowHotplugDevice ();

int ESetFileAttributes(const wchar_t *Name,DWORD Attr,int SkipMode=-1);
int ESetFileCompression(const wchar_t *Name,int State,DWORD FileAttr,int SkipMode=-1);
int ESetFileEncryption(const wchar_t *Name,int State,DWORD FileAttr,int SkipMode=-1,int Silent=0);
#define ESetFileEncryptionSilent(Name,State,FileAttr,SkipMode) ESetFileEncryptionW(Name,State,FileAttr,SkipMode,1)
int ESetFileTime(const wchar_t *Name,FILETIME *LastWriteTime,
                  FILETIME *CreationTime,FILETIME *LastAccessTime,
                  DWORD FileAttr,int SkipMode=-1);

//int ConvertWildcards(const char *Src,char *Dest, int SelectedFolderNameLength);
int ConvertWildcards(const wchar_t *SrcName,string &strDest, int SelectedFolderNameLength);

const wchar_t* WINAPI PrepareOSIfExist(const wchar_t *CmdLine);
BOOL IsBatchExtType(const wchar_t *ExtPtr);

int WINAPI GetSearchReplaceString (
         int IsReplaceMode,
         string *pSearchStr,
         string *pReplaceStr,
         const wchar_t *TextHistoryName,
         const wchar_t *ReplaceHistoryName,
         int *Case,
         int *WholeWords,
         int *Reverse,
         int *SelectFound);


BOOL WINAPI KeyMacroToText(int Key,string &strKeyText0);
int WINAPI KeyNameMacroToKey(const wchar_t *Name);
int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *rec=NULL);
int WINAPI KeyNameToKey(const wchar_t *Name);
BOOL WINAPI KeyToText (int Key, string &strKeyText);
int WINAPI InputRecordToKey(const INPUT_RECORD *Rec);
DWORD GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=false);
DWORD PeekInputRecord(INPUT_RECORD *rec);
DWORD CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros=NULL);
DWORD WaitKey(DWORD KeyWait=(DWORD)-1,DWORD delayMS=0);
int SetFLockState(UINT vkKey, int State);
BOOL FARGetKeybLayoutNameW (string &strDest);
int WriteInput(int Key,DWORD Flags=0);
int IsNavKey(DWORD Key);
int IsShiftKey(DWORD Key);
int CheckForEsc();
int CheckForEscSilent();
int ConfirmAbortOp();

// �������� �� ����� ����� RemoteName
string &DriveLocalToRemoteName(int DriveType,wchar_t Letter,string &strDest);

void __PrepareKMGTbStr(void);
string& __stdcall FileSizeToStr(string &strDestStr, unsigned __int64 Size, int Width=-1, int ViewFlags=COLUMN_COMMAS);


DWORD WINAPI FarGetLogicalDrives(void);

string &Add_PATHEXT(string &strDest);

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl qsortex(char *base, size_t nel, size_t width,
            int (__cdecl *comp_fp)(const void *, const void *,void*), void *user);

char * __cdecl farmktemp(char *temp);
char * __cdecl xstrncat (char * dest,const char * src,size_t maxlen);
wchar_t * __cdecl xwcsncat (wchar_t * dest,const wchar_t * src,size_t maxlen);
char * __cdecl xstrncpy (char * dest,const char * src,size_t maxlen);
wchar_t * __cdecl xwcsncpy (wchar_t * dest,const wchar_t * src,size_t maxlen);
char * __cdecl xf_strdup (const char * string);
wchar_t * __cdecl xf_wcsdup (const wchar_t * string);
void __cdecl far_qsort (
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
    );

void  __cdecl xf_free(void *__block);
void *__cdecl xf_malloc(size_t __size);
void *__cdecl xf_realloc(void *__block, size_t __size);

#ifdef __cplusplus
}
#endif

void GenerateWINDOW_BUFFER_SIZE_EVENT(int Sx=-1, int Sy=-1);

void PrepareStrFTime();
size_t WINAPI StrFTime(string &strDest, const wchar_t *Format,const tm *t);
size_t MkStrFTime(string &strDest, const wchar_t *Fmt=NULL);

BOOL WINAPI GetMenuHotKey(string &strHotKey,int LenHotKey,
                          const wchar_t *DlgHotKeyTitle,
                          const wchar_t *DlgHotKeyText,
                          const wchar_t *DlgPluginTitle,  // ���������
                          const wchar_t *HelpTopic,
                          const wchar_t *RegKey,
                          const wchar_t *RegValueName);

string& WINAPI FarFormatText(const wchar_t *SrcText, int Width, string &strDestText, const wchar_t* Break, DWORD Flags);


int PathMayBeAbsolute(const wchar_t *Src);

string& PrepareDiskPath(string &strPath, BOOL CheckFullPath=TRUE);

//   WordDiv  - ����� ������������ ����� � ��������� OEM
// ���������� ��������� �� ������ �����
const wchar_t * const CalcWordFromString(const wchar_t *Str,int CurPos,int *Start,int *End,const wchar_t *WordDiv);

long filelen(FILE *FPtr);
__int64 filelen64(FILE *FPtr);
__int64 ftell64(FILE *fp);
int fseek64 (FILE *fp, __int64 offset, int whence);

BOOL IsDiskInDrive(const wchar_t *Drive);

CDROM_DeviceCaps GetCDDeviceCaps(HANDLE hDevice);
UINT GetCDDeviceTypeByCaps(CDROM_DeviceCaps caps);
BOOL IsDriveTypeCDROM(UINT DriveType);
UINT FAR_GetDriveType(const wchar_t *RootDir,CDROM_DeviceCaps *caps=NULL,DWORD Detect=0);

bool PathPrefix(const wchar_t *Path);
BOOL IsNetworkPath(const wchar_t *Path);
BOOL IsLocalPath(const wchar_t *Path);
BOOL IsLocalRootPath(const wchar_t *Path);
BOOL IsLocalPrefixPath(const wchar_t *Path);
BOOL IsLocalVolumePath(const wchar_t *Path);
BOOL IsLocalVolumeRootPath(const wchar_t *Path);

BOOL RunGraber(void);

BOOL ProcessOSAliases(string &strStr);

int PartCmdLine(const wchar_t *CmdStr, string &strNewCmdStr, string &strNewCmdPar);

void initMacroVarTable(int global);
void doneMacroVarTable(int global);
bool checkMacroConst(const wchar_t *name);
const wchar_t *eStackAsString(int Pos=0);

int __parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const wchar_t *BufPtr);
BOOL __getMacroParseError(string *strErrMessage1, string *strErrMessage2,string *strErrMessage3);
int  __getMacroErrorCode(int *nErr=NULL);

int _MakePath1(DWORD Key,string &strPathName, const wchar_t *Param2,int ShortNameAsIs=TRUE);

string &CurPath2ComputerName(const wchar_t *CurDir, string &strComputerName);
void ConvertNameToUNC(string &strFileName);
int CheckDisksProps(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType);

bool GetFileFormat (FILE *file, UINT &nCodePage, bool *pSignatureFound = NULL);

string& HiText2Str(string& strDest, const wchar_t *Str);

__int64 FileTimeDifference(const FILETIME *a, const FILETIME* b);
unsigned __int64 FileTimeToUI64(const FILETIME *ft);

wchar_t *ReadString (FILE *file, wchar_t *lpwszDest, int nDestLength, int nCodePage);

#endif  // __FARFUNC_HPP__
