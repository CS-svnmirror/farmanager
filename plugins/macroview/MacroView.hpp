#ifndef MACRO_INCLUDED

#include <tchar.h>
#include "plugin.hpp"
#include "strclass.hpp"
#include "regclass.hpp"

#define GROUPNAMELEN 128
#define CAPTIONLEN 128
#define TITLELEN 64


//----
HINSTANCE hInstance;
PluginStartupInfo Info;
FarStandardFunctions FSF;
OSVERSIONINFO vi;


//----
TCHAR PluginRootKey[128];
TCHAR FarKey[128];               // default "\\Software\\Far"
TCHAR FarUsersKey[128];          // default "\\Software\\Far\\Users"
TCHAR KeyMacros[128];            // default "\\Software\\Far\\KeyMacros"
TCHAR FarUserName[MAX_PATH_LEN];
TCHAR FarFullName[MAX_PATH_LEN]; // default "C:\\Program Files\\Far\\Far.exe"


//----
int OpenFrom;
static int AltPressed=FALSE,CtrlPressed=FALSE,ShiftPressed=FALSE;


//----
const TCHAR *HKCU=_T("HKEY_CURRENT_USER");
const TCHAR *KeyMacros_KEY=_T("KeyMacros");
const TCHAR *Module_KEY=_T("MacroView");
const TCHAR *Plugins_KEY=_T("\\Plugins");
const TCHAR *Default_KEY=_T("\\Software\\Far");
const TCHAR *Users_KEY=_T("\\Software\\Far\\Users");


//----
TCHAR *MacroGroupShort[]=
{
  _T("Dialog"),_T("Disks"),_T("Editor"),_T("Help"),_T("Info"),_T("MainMenu"),
  _T("Menu"),_T("QView"),_T("Search"),_T("Shell"),_T("Tree"),_T("Viewer"),
  _T("Other"),_T("Common"),_T("FindFolder"),_T("UserMenu"),
};


//----
// ������� � ���� ������� �������� ����� ������� � �������
// MacroGroupShort, ���� �������� �������� ������ ��� �������
// ������
int GroupIndex[]=
{
  -1,9,-1,-1,-1,2,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};


//----
enum GroupNameConvert
{
  GRP_TOLONGNAME,
  GRP_TOSHORTNAME,
};

enum
{
  DM_NONE,
  DM_DELETED,
  DM_DEACTIVATED,
};

enum
{
  KB_COMMON,
  KB_ALT,
  KB_CTRL,
  KB_SHIFT,
  KB_DIALOG,
  KB_SHIFTDIALOG,
};

enum
{
  MAC_MENUACTIVE,
  MAC_EDITACTIVE,
  MAC_DELETEACTIVE,
  MAC_EXPORTACTIVE,
  MAC_COPYACTIVE,
  MAC_ERRORACTIVE,
};

enum EditMode
{
  EM_NONE,
  EM_INSERT,
  EM_EDIT,
};


//----
struct InitDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  DWORD_PTR Selected;
  int Flags;
  int DefaultButton;
  TCHAR *Data;
};


struct Config
{
  int AddDescription;
  int AutomaticSave;
  int ViewShell;
  int ViewViewer;
  int ViewEditor;
  int UseHighlight;
  int StartDependentSort;
  int LongGroupNames;
  int MenuCycle;
  int DblClick;
  int GroupDivider;
  int SaveOnStart;
};


struct MenuData
{
  TCHAR Group[16];
  TCHAR Key[32];
};


//----
BOOL InterceptDllCall(HMODULE hLocalModule,const char* c_szDllName,const char* c_szApiName,
                      PVOID pApiNew,PVOID* p_pApiOrg);

typedef BOOL (WINAPI *TReadConsoleInput)(HANDLE hConsole,INPUT_RECORD *ir,DWORD nNumber,LPDWORD nNumberOfRead);
typedef BOOL (WINAPI *TPeekConsoleInput)(HANDLE hConsole,INPUT_RECORD *ir,DWORD nNumber,LPDWORD nNumberOfRead);
TReadConsoleInput p_fnReadConsoleInputOrgA;
TReadConsoleInput p_fnReadConsoleInputOrgW;
TPeekConsoleInput p_fnPeekConsoleInputOrgA;
TPeekConsoleInput p_fnPeekConsoleInputOrgW;

LONG_PTR WINAPI MacroDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
LONG_PTR WINAPI MenuDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
LONG_PTR WINAPI DefKeyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
LONG_PTR WINAPI CopyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
//BOOL WINAPI ProcessKey(PINPUT_RECORD ir);
BOOL __fastcall ProcessPeekKey(PINPUT_RECORD ir);


//----
TCHAR *__fastcall AllTrim(TCHAR *S);
TCHAR *__fastcall UnQuoteText(TCHAR *S);
TCHAR *__fastcall QuoteText(TCHAR *S,BOOL Force=FALSE);
TCHAR *__fastcall GetMsg(int MsgId);
TCHAR *__fastcall CheckFirstBackSlash(TCHAR *S,BOOL mustbe);
TCHAR *__fastcall CheckLen(TCHAR *S,unsigned ln,BOOL AddDots=TRUE);
TCHAR *__fastcall CheckRLen(TCHAR *S,unsigned ln,BOOL AddDots=TRUE);
int   __fastcall CmpStr(const TCHAR *String1,const TCHAR *String2,int ln1=-1,int ln2=-1);


//----
class TMacroView
{
  friend LONG_PTR WINAPI MacroDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
  friend LONG_PTR WINAPI MenuDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
  friend LONG_PTR WINAPI DefKeyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
  friend LONG_PTR WINAPI CopyDialogProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);
  friend BOOL WINAPI myReadConsoleInputA(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead);
  friend BOOL WINAPI myReadConsoleInputW(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead);
  friend BOOL WINAPI myPeekConsoleInputA(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead);
  friend BOOL WINAPI myPeekConsoleInputW(HANDLE hConsole,PINPUT_RECORD ir,DWORD nNumber,LPDWORD nNumberOfRead);
//  friend BOOL WINAPI ProcessKey(PINPUT_RECORD ir);
  friend BOOL __fastcall ProcessPeekKey(PINPUT_RECORD ir);
  friend void __fastcall FlushInputBuffer();

  private:
    const TCHAR  *MacroText,
                 *MacroCmdHistory,
                 *MacroKeyHistory,
                 *MacroDescrHistory,
                 *MacroExpHistory,
                 *MacroCopyHistory;

    Config        Conf;
    FarDialogItem EditDialog[32];
    FarDialogItem MenuDialog[2];
    FarDialogItem DefKeyDialog[2];
    FarListItem   GroupItems[16],*ConfItems;
    FarList       GroupList,ConfList;

    BOOL          CtrlDotPressed,
                  WaitForKeyToMacro,
                  AltInsPressed,
                  HelpInvoked,
                  HelpActivated,
                  EditInMove,
          MultiLine;
    HANDLE        hand,
                  EditDlg,
                  MenuDlg,
                  DefDlg,
                  SaveScr,
                  /*SaveBar,*/
                  hOut,
                  hIn;
    TStrList     *NameList,
                 *MacNameList,
                 *DescrList,
                 *ValueList,
                 *MenuList;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    WIN32_FIND_DATA fData;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    TCHAR         S[MAX_PATH_LEN],
                  Str[MAX_PATH_LEN],
                  TempPath[MAX_PATH_LEN],
                  TempFileName[MAX_PATH_LEN],
                  Group[MAX_KEY_LEN],
                  Key[MAX_KEY_LEN];

    TCHAR         OldCaption[CAPTIONLEN],
                  NewCaption[CAPTIONLEN],
                  MenuTitle[TITLELEN],
                  MenuBottom[TITLELEN];

    TCHAR         *MacroData;

    int           Deactivated;
    int           ActiveMode;
    int           EditMode;
    int           MenuItemsNumber;
    int           MacroGroupsSize;
    int           KeyWidth;
    int           GroupKeyLen;
    int           MaxMenuItemLen;
    int           SelectPos;
    int           TopPos;
    int           GroupPos;
    int           UserConfPos;
    int           LastFocus;
    int           MenuX,MenuY,MenuH,MenuW;
    int           EditX1,EditY1,EditX2,EditY2;
#ifdef UNICODE
    // for EditDialog
    wchar_t       _Button[/*BUTTONLEN*/64];
    wchar_t       _Group[MAX_KEY_LEN]; //������� �������� �������� ������� �������
    wchar_t       _Data[MAX_PATH_LEN];
    wchar_t       _Descr[MAX_PATH_LEN];
#endif

  public:
    TMacroView();
    ~TMacroView();
    BOOL          __fastcall Configure();
    void          __fastcall ReadConfig();
    int           MacroList();

  private:
    void          __fastcall InitData();
    void          __fastcall InitMacroAreas();
    void          __fastcall InitDialogs();
//    void          __fastcall ParseMenuItem(FarListGetItem *List);
    void          __fastcall WriteKeyBar(int kbType);
    BOOL          __fastcall CreateDirs(TCHAR *Dir);
    TCHAR         *ConvertGroupName(TCHAR *Group,int nWhere);
    void          InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber);
    void          __fastcall InsertMacroToEditor(BOOL AllMacros);
    void          __fastcall ExportMacroToFile(BOOL AllMacros=FALSE);
    void          SwitchOver(const TCHAR *Group,const TCHAR *Key);
    int           DeletingMacro(TCHAR **Items,int ItemsSize,TCHAR *HelpTopic);
    BOOL          __fastcall CopyMoveMacro(int Op);
    void          MoveTildeInKey(TStrList *&NameList,BOOL doit=FALSE);
    void          PrepareDependentSort(TStrList *&NameList,BOOL doit=FALSE);
    void          __fastcall FillMenu(HANDLE hDlg,int RebuildList=TRUE);
    void          WriteRegValues(FarDialogItem *DialogItems);
    BOOL          __fastcall CopyMacro(int vKey);
    void          __fastcall ExportMacro(BOOL AllMacros=FALSE);
    BOOL          __fastcall DeleteMacro();
    void          __fastcall SetFocus(int Focus);
    int           __fastcall ShowEdit();
    BOOL          __fastcall InsertMacro();
    BOOL          __fastcall EditMacro();
};

TReg *Reg=NULL;
TMacroView *Macro=NULL;

#define MACRO_INCLUDED
#endif
