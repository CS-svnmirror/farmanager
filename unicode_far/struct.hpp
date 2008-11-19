#ifndef __FARSTRUCT_HPP__
#define __FARSTRUCT_HPP__
/*
struct.hpp

��� ����������� ��������� (������� �������� ������ ������� ����)
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
#include "UnicodeString.hpp"

struct PanelOptions
{
  int Type;
  int Visible;
  int Focus;
  int ViewMode;
  int SortMode;
  int SortOrder;
  int SortGroups;
  int ShowShortNames;
  int NumericSort;
};

struct Confirmation
{
  int Copy;
  int Move;
  int Drag;
  int Delete;
  int DeleteFolder;
  int Exit;
  int Esc;  // ��� CheckForEsc
  /* $ 12.03.2002 VVM
    + Opt.EscTwiceToInterrupt
      ���������� ��������� ��� ���������� ���������� ��������
      0 - ������ ESC ���������� ��������
      1 - ������ ESC ��������� �������� */
  int EscTwiceToInterrupt;
  int RemoveConnection;
  /* $ 23.05.2001
    +  Opt.Confirmation.AllowReedit - ����, ������� �������� ��������� ��������
      ����� �� �������������� ����, ������ ���� ��� �������������. �� ��������� - 1
      0 - ���� ��� �������� ���� �� ��� �������, �� ���������� ������� � ��������� ���������
          ��� �������������� ��������. ���� ���� ��� �������, �� �������� ������, � � ������
          ���� ������� ������� Reload, �� ����������� ����� ����� �����, ��� ���� ���������
          ��������� ��������.
      1 - ��� ��� ���� ������. �������� ������ � ���������� ������� ���� ��� � ��������� �����
          ���� ����������� ����� ������ ���������.
      */
  int AllowReedit;
  int HistoryClear;
  int RemoveSUBST;
  int RemoveHotPlug;
};

struct DizOptions
{
  string strListNames;
  int ROUpdate;
  int UpdateMode;
  int SetHidden;
  int StartPos;

  /*
  void Clear()
  {
    strListNames=L"";
    ROUpdate=0;
    UpdateMode=0;
    SetHidden=0;
    StartPos=0;
  }
  */
};

struct CodeXLAT
{
  DWORD Flags;       // �������������� �����

  int XLatEditorKey;
  int XLatCmdLineKey;
  int XLatDialogKey;
  int XLatFastFindKey;

  int XLatAltEditorKey;
  int XLatAltCmdLineKey;
  int XLatAltDialogKey;
  int XLatAltFastFindKey;

  /* $ 25.11.2000 IS
     �������������� ���� �� ������� ��� ������� Xlat
  */
  string strWordDivForXlat;

  // ������ ���� - ������ �������
  BYTE Table[2][81]; // [0] non-english �����, [1] english �����
  BYTE Rules[3][81]; // 3 �� 40 ������:
                    //  [0] "���� ���������� ������ ���������"
                    //  [1] "���� ���������� ������ ����������� ������"
                    //  [2] "���� ���������� ������ �� ���/lat"

  /*
  void Clear()
  {
    Flags=0;
    XLatEditorKey=0;
    XLatCmdLineKey=0;
    XLatDialogKey=0;
    XLatFastFindKey=0;
    XLatAltEditorKey=0;
    XLatAltCmdLineKey=0;
    XLatAltDialogKey=0;
    XLatAltFastFindKey=0;
    strWordDivForXlat=L"";
    memset(Table,0,sizeof(Table));
    memset(Rules,0,sizeof(Rules));
  }
  */
};

struct EditorOptions
{
	int TabSize;
	int ExpandTabs;
	int PersistentBlocks;
	int DelRemovesBlocks;
	int AutoIndent;
	int AutoDetectTable;
	int AnsiTableForNewFile;
	int AnsiTableAsDefault;
	int CursorBeyondEOL;
	int BSLikeDel;
	int CharCodeBase;
	int SavePos;
	int SaveShortPos;
	int F7Rules; // $ 28.11.2000 SVS - ������� �� ���� ������ � ���������
	int AllowEmptySpaceAfterEof; // $ 21.06.2005 SKV - ��������� ���������� ������ ������������ ����� ��������� ������ �������������� �����.
	int ReadOnlyLock; // $ 29.11.2000 SVS - ������ ���� ��� �������� � ���������, ���� �� ����� �������� R|S|H
	int UndoSize; // $ 03.12.2001 IS - ������ ������ undo � ���������
	int UseExternalEditor;
	DWORD FileSizeLimitLo;
	DWORD FileSizeLimitHi;
	int ShowKeyBar;
	int ShowTitleBar;

	string strWordDiv;

	void Clear ()
	{
		TabSize = 0;
		ExpandTabs = 0;
		PersistentBlocks = 0;
		DelRemovesBlocks = 0;
		AutoIndent = 0;
		AutoDetectTable = 0;
		AnsiTableForNewFile = 0;
		AnsiTableAsDefault = 0;
		CursorBeyondEOL = 0;
		BSLikeDel = 0;
		CharCodeBase = 0;
		SavePos = 0;
		SaveShortPos = 0;
		F7Rules = 0;
		AllowEmptySpaceAfterEof = 0;
		ReadOnlyLock = 0;
		UndoSize = 0;
		UseExternalEditor = 0;
		ShowKeyBar = 0;
		ShowTitleBar = 0;

		FileSizeLimitLo = 0;
		FileSizeLimitHi = 0;

		strWordDiv = L"";
	}

	void CopyTo (EditorOptions &dest)
	{
		dest.TabSize = TabSize;
		dest.ExpandTabs = ExpandTabs;
		dest.PersistentBlocks = PersistentBlocks;
		dest.DelRemovesBlocks = DelRemovesBlocks;
		dest.AutoIndent = AutoIndent;
		dest.AutoDetectTable = AutoDetectTable;
		dest.AnsiTableForNewFile = AnsiTableForNewFile;
		dest.AnsiTableAsDefault = AnsiTableAsDefault;
		dest.CursorBeyondEOL = CursorBeyondEOL;
		dest.BSLikeDel = BSLikeDel;
		dest.CharCodeBase = CharCodeBase;
		dest.SavePos = SavePos;
		dest.SaveShortPos = SaveShortPos;
		dest.F7Rules = F7Rules;
		dest.AllowEmptySpaceAfterEof = AllowEmptySpaceAfterEof;
		dest.ReadOnlyLock = ReadOnlyLock;
		dest.UndoSize = UndoSize;
		dest.UseExternalEditor = UseExternalEditor;
		dest.ShowKeyBar = ShowKeyBar;
		dest.ShowTitleBar = ShowTitleBar;
		dest.strWordDiv = strWordDiv;

		dest.FileSizeLimitLo = FileSizeLimitLo;
		dest.FileSizeLimitHi = FileSizeLimitHi;
	}
};

/* $ 29.03.2001 IS
     ��� ������� ������� "���������" ��������� ��� ��������� ���������
*/
struct ViewerOptions
{
  int TabSize;
  int AutoDetectTable;
  int ShowScrollbar;     // $ 18.07.2000 tran ���� �������� ��� viewer
  int ShowArrows;
  int PersistentBlocks; // $ 14.05.2002 VVM ���������� ����� �� ������
  int ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
  int ViewerWrap; // Wrap=0|WordWarp=1
  int SaveViewerPos;
  int SaveViewerShortPos;
  int UseExternalViewer;
  int ShowKeyBar; // $ 15.07.2000 tran + ShowKeyBar
  int AnsiTableAsDefault;
  int ShowTitleBar;
};

// "�������"
struct PoliciesOptions {
  int DisabledOptions;  // ������������� ���� ������������
  int ShowHiddenDrives; // ���������� ������� ���������� �����
};

struct DialogsOptions{
  int   EditBlock;          // ���������� ����� � ������� �����
  int   EditHistory;        // ��������� � �������?
  int   AutoComplete;       // ��������� ��������������?
  int   EULBsClear;         // = 1 - BS � �������� ��� UnChanged ������ ������� ����� ������ �����, ��� � Del
  int   SelectFromHistory;  // = 0 then (ctrl-down � ������ � �������� ������ �������������� �� ����� ������� ������)
  DWORD EditLine;           // ����� ���������� � ������ ����� (������ ��� ����... ��������� ��������� ����������)
  int   MouseButton;        // ���������� ���������� ������/����� ������ ���� ��� ������ �������� ���� �������
  int   DelRemovesBlocks;
  int   CBoxMaxHeight;      // ������������ ������ ������������ ������ (�� ���������=8)
};

struct NowellOptions{
  int MoveRO;               // ����� ��������� Move ������� R/S/H ��������, ����� �������� - ���������� �������
};

// ��������� ���������� ������ character table
struct FindCharTable
{
  int AllTables;
  int AnsiTable;
  int UnicodeTable;
  int TableNum;
};

struct ScreenSizes{
  COORD DeltaXY;            // �� ������� ���. �������� ������� ��� ������������ ������
#if defined(DETECT_ALT_ENTER)
  /*
    Opt.WScreenSize - Windowed/Full Screen Size
       COORD[0].X - Windowed Width  mode 1
       COORD[0].Y - Windowed Height mode 1
       COORD[1].X - Windowed Width  mode 2
       COORD[1].Y - Windowed Height mode 2

       COORD[2].X - FullScreen Width  mode 1
       COORD[2].Y - FullScreen Height mode 1
       COORD[3].X - FullScreen Width  mode 2
       COORD[3].Y - FullScreen Height mode 2
  */
  int WScreenSizeSet;
  COORD WScreenSize[4];
#endif
};

struct LoadPluginsOptions
{
//  DWORD TypeLoadPlugins;       // see TYPELOADPLUGINSOPTIONS
  int MainPluginDir; // TRUE - ������������ ����������� ���� � �������� ��������
  int PluginsCacheOnly; // seting by '/co' switch, not saved in registry
  int PluginsPersonal;

  string strCustomPluginsPath;  // ���� ��� ������ ��������, ��������� � /p
  string strPersonalPluginsPath;
  int SilentLoadPlugin; // ��� �������� ������� � ������...

  /*
  void Clear()
  {
    MainPluginDir=0;
    PluginsCacheOnly=0;
    PluginsPersonal=0;

    strCustomPluginsPath=L"";
    strPersonalPluginsPath=L"";
    SilentLoadPlugin=0;
  }
  */
};

struct FindFileOptions
{
  int FindFolders;
  int CollectFiles;
  int FileSearchMode;
  int SearchInFirst;
  int UseFilter;
  string strSearchInFirstSize;

  /*
  void Clear()
  {
    FindFolders=0;
    CollectFiles=0;
    FileSearchMode=0;
    SearchInFirst=0;
    strSearchInFirstSize=L"";
  }
  */
};

struct TreeOptions{
  int LocalDisk;         // ������� ���� ��������� ����� ��� ��������� ������
  int NetDisk;           // ������� ���� ��������� ����� ��� ������� ������
  int NetPath;           // ������� ���� ��������� ����� ��� ������� �����
  int RemovableDisk;     // ������� ���� ��������� ����� ��� ������� ������
  int MinTreeCount;      // ����������� ���������� ����� ��� ���������� ������ � �����.
  int AutoChangeFolder;  // ��������� ����� ��� ����������� �� ������
  DWORD TreeFileAttr;    // �������� �������� ��� ������-�������
};

struct CopyMoveOptions{
  int UseSystemCopy;         // ������������ ��������� ������� �����������
  int CopyOpened;            // ���������� �������� �� ������ �����
  int CopyShowTotal;         // �������� ����� ��������� �����������
  int MultiCopy;             // "��������� �����������������/�����������/�������� ������"
  DWORD CopySecurityOptions; // ��� �������� Move - ��� ������ � ������ "Copy access rights"
  int CopyTimeRule;          // $ 30.01.2001 VVM  ���������� ����� �����������,���������� ����� � ������� ��������
};

struct Options
{
  int Clock;
  int Mouse;
  int ShowKeyBar;
  int ScreenSaver;
  int ScreenSaverTime;
  int UsePromptFormat;
  string strPromptFormat;
  int AltGr;
  int UseVk_oem_x;
  int InactivityExit;
  int InactivityExitTime;
  int ShowHidden;
  int Highlight;

  string strLeftFolder;
  string strRightFolder;

  string strLeftCurFile;
  string strRightCurFile;

  int RightSelectedFirst;
  int LeftSelectedFirst;
  int SelectFolders;
  int ReverseSort;
  int ClearReadOnly;
  int SortFolderExt;
  int DeleteToRecycleBin;         // ������� � �������?
  int DeleteToRecycleBinKillLink; // ����� ��������� ����� � ������� ������� ��������� ��������.
  int WipeSymbol; // ������ ����������� ��� "ZAP-��������"

  struct CopyMoveOptions CMOpt;

  int MultiMakeDir; // ����� �������� ���������� ��������� �� ���� �����

  int CreateUppercaseFolders;
  int UseRegisteredTypes;

  int ViewerEditorClock;
  int OnlyEditorViewerUsed; // =1, ���� ����� ��� /e ��� /v
  int SaveViewHistory;
  int ViewHistoryCount;

  string strExternalEditor;
  struct EditorOptions EdOpt;
  string strExternalViewer;
  struct ViewerOptions ViOpt;


  string strWordDiv; // $ 03.08.2000 SVS �������������� ���� �� �������
  string strQuotedSymbols;
  DWORD QuotedName;
  int AutoSaveSetup;
  int SetupArgv; // ���������� ��������� � ���������� ����
  int ChangeDriveMode;
  int ChangeDriveDisconnetMode;

  int SaveHistory;
  int HistoryCount;
  int SaveFoldersHistory;
  int SavePluginFoldersHistory;
  int FoldersHistoryCount;
  int DialogsHistoryCount;

  struct FindFileOptions FindOpt;

  string strTempPath;
  int HeightDecrement;
  int WidthDecrement;

  string strPassiveFolder;

  int ShowColumnTitles;
  int ShowPanelStatus;
  int ShowPanelTotals;
  int ShowPanelFree;
  int ShowPanelScrollbar;
  int ShowMenuScrollbar; // $ 29.06.2000 SVS �������� ������� ������ Scroll Bar � ����.
  int ShowScreensNumber;
  int ShowSortMode;
  int ShowMenuBar;
  int FormatNumberSeparators;
  int CleanAscii;
  int NoGraphics;
  string strFolderInfoFiles;

  struct Confirmation Confirm;
  struct DizOptions Diz;

  int ShellRightLeftArrowsRule;
  struct PanelOptions LeftPanel;
  struct PanelOptions RightPanel;

  DWORD  AutoUpdateLimit; // ���� ����� ���������� ������������� �� ��������� ������.
  int AutoUpdateRemoteDrive;

  string strLanguage;
  int SmallIcon;
  string strRegRoot;
  int PanelRightClickRule; // ������ ��������� ������ ������� ����
  int PanelMiddleClickRule; // ��������� ������� ������ ���� � �������
  int PanelCtrlAltShiftRule; // ������ ��������� Ctrl-Alt-Shift ��� �������.
  // Panel/CtrlFRule � ������� - ������ ��������� Ctrl-F. ���� = 0, �� ���������� ���� ��� ����, ����� - � ������ ����������� �� ������
  int PanelCtrlFRule;
  /*
    ������� �����, ������ ��������� Ctrl-Alt-Shift
     ��� ���������� - ������� ��������:
     0 - Panel
     1 - Edit
     2 - View
     3 - Help
     4 - Dialog
  */
  int AllCtrlAltShiftRule;

  int CASRule; // 18.12.2003 - ������� ��������� ����� � ������ CAS (������� #1).
  /*
    ������ ��������� Esc ��� ��������� ������:
      =1 - �� �������� ��������� � History, ���� ����� Ctrl-E/Ctrl/-X
           ������ ESC (��������� - ��� VC).
      =0 - ��������� ��� � ���� - �������� ��������� � History
  */
  int CmdHistoryRule;

  DWORD ExcludeCmdHistory;
  int SubstPluginPrefix; // 1 = ��������������� ������� ������� (��� Ctrl-[ � ��� ��������)
  int MaxPositionCache; // ���������� ������� � ���� ����������
  int SetAttrFolderRules; // ������� �� ���� ��������� ��������� �� ��������
  int ExceptRules; // ������� �� ���� ������ ����������
  int ExceptCallDebugger; // �������� �������� ��� ����������
  int HotkeyRules; // ������� �� ���� ������ ��������� �������
  /*
   + Opt.ShiftsKeyRules - ������� �� ���� ������ ��������� ����������
     Alt-����� ��� ����������� ������� � �������� "`-=[]\;',./" �
     �������������� Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt-
  */
  int ShiftsKeyRules;
  int MacroReuseRules; // ������� �� ���� �������� ������������� ����������� ������
  DWORD DisableMacro; // ��������� /m ��� /ma ��� /m....
  int IgnoreErrorBadPathName;

  DWORD KeyMacroCtrlDot; // ��� KEY_CTRLDOT
  DWORD KeyMacroCtrlShiftDot; // ��� KEY_CTRLSHIFTDOT
  int CursorSize[4];   // ������ ������� ����

  struct CodeXLAT XLat;

  int ConsoleDetachKey; // ���������� ������ ��� ������ Far'������ ������� �� ����������� ���������������� �������� � ��� �����������.

  int UsePrintManager;

  string strHelpLanguage;
  int FullScreenHelp;
  int HelpTabSize;

  int HelpURLRules; // =0 ��������� ����������� ������� URL-����������

  // ���������� ���������� ����� � �� ���������� ������ ���. ��� �������������� "����������" "�������" ������.
  int RememberLogicalDrives;
  /*
    ����� ������ ��:
        ���������� ������ � ������� � ������ ���������
        ���������� LastPositions � ��������� � ������
  */
  int FlagPosixSemantics;

  int MsWheelDelta; // ������ �������� ��� ���������
  int MsWheelDeltaView;
  int MsWheelDeltaEdit;
  int MsWheelDeltaHelp;
  // �������������� ���������
  int MsHWheelDelta;
  int MsHWheelDeltaView;
  int MsHWheelDeltaEdit;

  /* $ 28.04.2001 VVM
    + Opt.SubstNameRule ������� �����:
      0 - ���� ����������, �� ���������� ������� ����� ��� GetSubstName()
      1 - ���� ����������, �� ���������� ��� ��������� ��� GetSubstName() */
  int SubstNameRule;

  /* $ 23.05.2001 AltF9
    + Opt.AltF9 ���� ��������� ������� ��������  ������ ���������� Alt-F9
         (��������� ������� ������) � ������� ������. �� ��������� - 1.
      0 - ������������ ��������, ����������� � FAR ������ 1.70 beta 3 �
         ����, �.�. ������������ 25/50 �����.
      1 - ������������ ������������������� �������� - ���� FAR Manager
         ����� ������������� � ����������� �� ����������� ��������� ������
         ����������� ���� � �������.*/
  int AltF9;

  int PgUpChangeDisk;
  int ShowCheckingFile;
  int CloseConsoleRule;
  int CloseCDGate;       // ���������������� CD

  DWORD LCIDSort;
  int RestoreCPAfterExecute;
  int ExecuteShowErrorMessage;
  int ExecuteUseAppPath;
  int ExecuteFullTitle;
  string strExecuteBatchType;

  DWORD PluginMaxReadData;
  int UseNumPad;
  int ScanJunction;

  DWORD ShowTimeoutDelFiles; // ������ � �������� �������� (� ms)
  DWORD ShowTimeoutDACLFiles;
  int DelThreadPriority; // ��������� �������� ��������, �� ��������� = THREAD_PRIORITY_NORMAL

  //int CPAJHefuayor; // ����������� �� "Close Plugin And Jump:
                  // Highly experimental feature, use at your own risk"

  string strDateFormat; // ��� $Date
  struct LoadPluginsOptions LoadPlug;

  struct DialogsOptions Dialogs;
  struct PoliciesOptions Policies;
  struct NowellOptions Nowell;
  struct ScreenSizes ScrSize;

  struct FindCharTable CharTable; // ��������� ��� ����������� ���������� ������ �������� � ������

  struct TreeOptions Tree;

  /*
  void Clear()
  {
    Clock=0;
    Mouse=0;
    ShowKeyBar=0;
    ScreenSaver=0;
    ScreenSaverTime=0;
    UsePromptFormat=0;
    strPromptFormat=L"";
    AltGr=0;
    UseVk_oem_x=0;
    InactivityExit=0;
    InactivityExitTime=0;
    ShowHidden=0;
    Highlight=0;

    strLeftFolder=L"";
    strRightFolder=L"";

    strLeftCurFile=L"";
    strRightCurFile=L"";

    RightSelectedFirst=0;
    LeftSelectedFirst=0;
    SelectFolders=0;
    ReverseSort=0;
    ClearReadOnly=0;
    SortFolderExt=0;
    DeleteToRecycleBin=0;
    WipeSymbol=0;

    memset(&CMOpt,0,sizeof(CMOpt));

    MultiMakeDir=0;
    CreateUppercaseFolders=0;
    UseRegisteredTypes=0;

    ViewerEditorClock=0;
    OnlyEditorViewerUsed=0;
    SaveViewHistory=0;
    ViewHistoryCount=0;

    strExternalEditor=L"";
    EdOpt.Clear();
    strExternalViewer=L"";
    memset(&ViOpt,0,sizeof(ViOpt));


    strWordDiv=L"";
    strQuotedSymbols=L"";
    QuotedName=0;
    AutoSaveSetup=0;
    SetupArgv=0;
    ChangeDriveMode=0;
    ChangeDriveDisconnetMode=0;

    SaveHistory=0;
    HistoryCount=0;
    SaveFoldersHistory=0;
    SavePluginFoldersHistory=0;
    FoldersHistoryCount=0;
    DialogsHistoryCount=0;

    FindOpt.Clear();

    strTempPath=L"";
    HeightDecrement=0;
    WidthDecrement=0;

    strPassiveFolder=L"";

    ShowColumnTitles=0;
    ShowPanelStatus=0;
    ShowPanelTotals=0;
    ShowPanelFree=0;
    ShowPanelScrollbar=0;
    ShowMenuScrollbar=0;
    ShowScreensNumber=0;
    ShowSortMode=0;
    ShowMenuBar=0;

    CleanAscii=0;
    NoGraphics=0;
    strFolderInfoFiles=L"";

    memset(&Confirm,0,sizeof(Confirm));
    Diz.Clear();
    memset(&LeftPanel,0,sizeof(LeftPanel));
    memset(&RightPanel,0,sizeof(RightPanel));

    AutoUpdateLimit=0;
    AutoUpdateRemoteDrive=0;

    strLanguage=L"";
    SmallIcon=0;
    strRegRoot=L"";
    PanelRightClickRule=0;
    PanelMiddleClickRule=0;
    PanelCtrlAltShiftRule=0;
    PanelCtrlFRule=0;
    AllCtrlAltShiftRule=0;
    CASRule=0;
    CmdHistoryRule=0;
    ExcludeCmdHistory=0;
    SubstPluginPrefix=0;
    MaxPositionCache=0;
    SetAttrFolderRules=0;
    ExceptRules=0;
    ExceptCallDebugger=0;
    HotkeyRules=0;
    ShiftsKeyRules=0;
    MacroReuseRules=0;
    IgnoreErrorBadPathName=0;

    KeyMacroCtrlDot=0;
    KeyMacroCtrlShiftDot=0;
    memset(CursorSize,0,sizeof(CursorSize));
    XLat.Clear();
    ConsoleDetachKey=0;

    UsePrintManager=0;

    strHelpLanguage=L"";
    FullScreenHelp=0;
    HelpTabSize=0;
    HelpURLRules=0;

    RememberLogicalDrives=0;
    FlagPosixSemantics=0;
    MsWheelDelta=0;
    MsWheelDeltaView=0;
    MsWheelDeltaEdit=0;
    MsWheelDeltaHelp=0;
    SubstNameRule=0;

    AltF9=0;
    PgUpChangeDisk=0;
    ShowCheckingFile=0;
    CloseConsoleRule=0;
    CloseCDGate=0;

    LCIDSort=0;
    RestoreCPAfterExecute=0;
    ExecuteShowErrorMessage=0;
    ExecuteUseAppPath=0;
    ExecuteFullTitle=0;
    strExecuteBatchType=L"";

    PluginMaxReadData=0;
    UseNumPad=0;
    ScanJunction=0;

    ShowTimeoutDelFiles=0;
    ShowTimeoutDACLFiles=0;
    DelThreadPriority=0;

    strDateFormat=L"";
    LoadPlug.Clear();

    memset(&Dialogs,0,sizeof(Dialogs));
    memset(&Policies,0,sizeof(Policies));
    memset(&Nowell,0,sizeof(Nowell));
    memset(&ScrSize,0,sizeof(ScrSize));
    memset(&CharTable,0,sizeof(CharTable));
    memset(&Tree,0,sizeof(Tree));
  }
  */
};

struct PluginHandle
{
  HANDLE hPlugin;
  class Plugin *pPlugin;
};

// for class Edit
struct ColorItem
{
  int StartPos;
  int EndPos;
  int Color;
};

enum enumHighlightDataColor
{
  HIGHLIGHTCOLOR_NORMAL = 0,
  HIGHLIGHTCOLOR_SELECTED,
  HIGHLIGHTCOLOR_UNDERCURSOR,
  HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR,

  HIGHLIGHTCOLORTYPE_FILE = 0,
  HIGHLIGHTCOLORTYPE_MARKCHAR = 1,
};

struct HighlightDataColor
{
  WORD Color[2][4]; // [0=file, 1=mark][0=normal,1=selected,2=undercursor,3=selectedundercursor]; if HIBYTE == 0xFF then transparent
  DWORD MarkChar;
};

struct FAR_FIND_DATA_EX
{
	DWORD    dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	unsigned __int64 nFileSize;

	union {
		unsigned __int64 nPackSize; //same as reserved
		struct {
			DWORD dwReserved0;
			DWORD dwReserved1;
		};
	};

	string   strFileName;
	string   strAlternateFileName;

	void Clear()
	{
		dwFileAttributes=0;
		memset(&ftCreationTime,0,sizeof(ftCreationTime));
		memset(&ftLastAccessTime,0,sizeof(ftLastAccessTime));
		memset(&ftLastWriteTime,0,sizeof(ftLastWriteTime));
		nFileSize=_ui64(0);
		nPackSize=_ui64(0);
		strFileName=L"";
		strAlternateFileName=L"";
	}

	FAR_FIND_DATA_EX& operator=(const FAR_FIND_DATA_EX &ffdexCopy)
	{
		dwFileAttributes=ffdexCopy.dwFileAttributes;
		memcpy(&ftCreationTime,&ffdexCopy.ftCreationTime,sizeof(ftCreationTime));
		memcpy(&ftLastAccessTime,&ffdexCopy.ftLastAccessTime,sizeof(ftLastAccessTime));
		memcpy(&ftLastWriteTime,&ffdexCopy.ftLastWriteTime,sizeof(ftLastWriteTime));
		nFileSize=ffdexCopy.nFileSize;
		nPackSize=ffdexCopy.nPackSize;
		strFileName=ffdexCopy.strFileName;
		strAlternateFileName=ffdexCopy.strAlternateFileName;
		return *this;
	}
};


#endif // __FARSTRUCT_HPP__
