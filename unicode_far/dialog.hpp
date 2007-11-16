#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

����� ������� Dialog.

������������ ��� ����������� ��������� ��������.
�������� ����������� �� ������ Frame.
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

#include "frame.hpp"
#include "plugin.hpp"
#include "vmenu.hpp"
#include "bitflags.hpp"
#include "CriticalSections.hpp"
#include "UnicodeString.hpp"

// ����� �������� ������ �������
#define DMODE_INITOBJECTS   0x00000001 // �������� ����������������?
#define DMODE_CREATEOBJECTS 0x00000002 // ������� (Edit,...) �������?
#define DMODE_WARNINGSTYLE  0x00000004 // Warning Dialog Style?
#define DMODE_DRAGGED       0x00000008 // ������ ���������?
#define DMODE_ISCANMOVE     0x00000010 // ����� �� ������� ������?
#define DMODE_ALTDRAGGED    0x00000020 // ������ ��������� �� Alt-�������?
#define DMODE_SMALLDIALOG   0x00000040 // "�������� ������"
#define DMODE_DRAWING       0x00001000 // ������ ��������?
#define DMODE_KEY           0x00002000 // ���� ������� ������?
#define DMODE_SHOW          0x00004000 // ������ �����?
#define DMODE_MOUSEEVENT    0x00008000 // ����� �������� MouseMove � ����������?
#define DMODE_RESIZED       0x00010000 //
#define DMODE_ENDLOOP       0x00020000 // ����� ����� ��������� �������?
#define DMODE_BEGINLOOP     0x00040000 // ������ ����� ��������� �������?
//#define DMODE_OWNSITEMS     0x00080000 // ���� TRUE, Dialog ����������� ������ Item � �����������
#define DMODE_NODRAWSHADOW  0x00100000 // �� �������� ����?
#define DMODE_NODRAWPANEL   0x00200000 // �� �������� ��������?
#define DMODE_CLICKOUTSIDE  0x20000000 // ���� ������� ���� ��� �������?
#define DMODE_MSGINTERNAL   0x40000000 // ���������� Message?
#define DMODE_OLDSTYLE      0x80000000 // ������ � ������ (�� 1.70) �����

#define DIMODE_REDRAW       0x00000001 // ��������� �������������� ���������� �����?

// ����� ��� ������� ConvertItem
#define CVTITEM_TOPLUGIN    0
#define CVTITEM_FROMPLUGIN  1

enum DLGEDITLINEFLAGS {
  DLGEDITLINE_CLEARSELONKILLFOCUS = 0x00000001, // ��������� ���������� ����� ��� ������ ������ �����
  DLGEDITLINE_SELALLGOTFOCUS      = 0x00000002, // ��������� ���������� ����� ��� ��������� ������ �����
  DLGEDITLINE_NOTSELONGOTFOCUS    = 0x00000004, // �� ��������������� ��������� ������ �������������� ��� ��������� ������ �����
  DLGEDITLINE_NEWSELONGOTFOCUS    = 0x00000008, // ��������� ��������� ��������� ����� ��� ��������� ������
  DLGEDITLINE_GOTOEOLGOTFOCUS     = 0x00000010, // ��� ��������� ������ ����� ����������� ������ � ����� ������
  DLGEDITLINE_PERSISTBLOCK        = 0x00000020, // ���������� ����� � ������� �����
  DLGEDITLINE_AUTOCOMPLETE        = 0x00000040, // �������������� � ������� �����
  DLGEDITLINE_AUTOCOMPLETECTRLEND = 0x00000040, // ��� �������������� ������������ ����������� Ctrl-End
  DLGEDITLINE_HISTORY             = 0x00000100, // ������� � ������� ����� ��������
};


enum DLGITEMINTERNALFLAGS {
  DLGIIF_LISTREACTIONFOCUS        = 0x00000001, // MouseReaction ��� ��������� ��������
  DLGIIF_LISTREACTIONNOFOCUS      = 0x00000002, // MouseReaction ��� �� ��������� ��������
  DLGIIF_EDITPATH                 = 0x00000004, // ����� Ctrl-End � ������ �������������� ����� �������� �� ���� �������������� ������������ ����� � ���������� � ������ �� �������
  DLGIIF_COMBOBOXNOREDRAWEDIT     = 0x00000008, // �� ������������� ������ �������������� ��� ���������� � �����
  DLGIIF_COMBOBOXEVENTKEY         = 0x00000010, // �������� ������� ���������� � ���������� ����. ��� ��������� ����������
  DLGIIF_COMBOBOXEVENTMOUSE       = 0x00000020, // �������� ������� ���� � ���������� ����. ��� ��������� ����������
};


#define MakeDialogItemsEx(Data,Item) \
  struct DialogItemEx Item[sizeof(Data)/sizeof(Data[0])]; \
  Dialog::DataToItemEx(Data,Item,sizeof(Data)/sizeof(Data[0]));

// ���������, ����������� ������������� ��� DIF_AUTOMATION
// �� ������ ����� - ����������� - ����������� ������ � ��������� ��� CheckBox
struct DialogItemAutomation{
  WORD ID;                    // ��� ����� ��������...
  DWORD Flags[3][2];          // ...��������� ��� ��� �����
                              // [0] - Unchecked, [1] - Checked, [2] - 3Checked
                              // [][0] - Set, [][1] - Skip
};

// ������ ��� DI_USERCONTROL
class DlgUserControl{
  public:
    COORD CursorPos;
    int   CursorVisible,CursorSize;

  public:
    DlgUserControl(){CursorSize=CursorPos.X=CursorPos.Y=-1;CursorVisible=0;}
   ~DlgUserControl(){};
};

/*
��������� ���� ������� ������� - ��������� �������������.
��� �������� ��� FarDialogItem (�� ����������� ObjPtr)
*/
struct DialogItemEx
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  union
  {
  	DWORD_PTR Reserved;
    int Selected;
    const wchar_t *History;
    const wchar_t *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  int DefaultButton;

  string strData;
  size_t nMaxLength;

  WORD ID;
  BitFlags IFlags;
  int AutoCount;   // �������������
  struct DialogItemAutomation* AutoPtr;
  DWORD_PTR UserData; // ��������������� ������

  // ������
  void *ObjPtr;
  VMenu *ListPtr;
  DlgUserControl *UCData;

  int SelStart;
  int SelEnd;

  void Clear()
  {
    Type=0;
    X1=0;
    Y1=0;
    X2=0;
    Y2=0;
    Focus=0;
    History=NULL;
    Flags=0;
    DefaultButton=0;

    strData=L"";
    nMaxLength=0;

    ID=0;
    IFlags.ClearAll();
    AutoCount=0;
    AutoPtr=NULL;
    UserData=0;

    ObjPtr=NULL;
    ListPtr=NULL;
    UCData=NULL;

    SelStart=0;
    SelEnd=0;
  }
};

/*
��������� ���� ������� ������� - ��� ���������� �������
��������� ����������� ��������� InitDialogItem (��. "Far PlugRinG
Russian Help Encyclopedia of Developer")
*/

struct DialogDataEx
{
  WORD  Type;
  short X1,Y1,X2,Y2;
  BYTE  Focus;
  union {
    DWORD_PTR Reserved;
    unsigned int Selected;
    const wchar_t *History;
    const wchar_t *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  BYTE  DefaultButton;

  const wchar_t *Data;
};


struct FarDialogMessage{
  HANDLE   hDlg;
  int      Msg;
  int      Param1;
  LONG_PTR Param2;
};

class DlgEdit;
class ConsoleTitle;

typedef LONG_PTR (WINAPI *SENDDLGMESSAGE) (HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

class Dialog: public Frame
{
  friend class FindFiles;

  private:
    bool bInitOK;               // ������ ��� ������� ���������������
    INT_PTR PluginNumber;       // ����� �������, ��� ������������ HelpTopic
    unsigned FocusPos;               // ������ �������� ����� ������� � ������
    unsigned PrevFocusPos;           // ������ �������� ����� ������� ��� � ������
    int IsEnableRedraw;         // ��������� ����������� �������? ( 0 - ���������)
    BitFlags DialogMode;        // ����� �������� ������ �������

    LONG_PTR DataDialog;        // ������, ������������� ��� ����������� ���������� ������� (������������� ����� ��������, ���������� � �����������)

    struct DialogItemEx **Item; // ������ ��������� �������
    struct DialogItemEx *pSaveItemEx; // ���������������� ������ ��������� �������

    unsigned ItemCount;         // ���������� ��������� �������

    ConsoleTitle *OldTitle;     // ���������� ���������
    int DialogTooLong;          //
    int PrevMacroMode;          // ���������� ����� �����

    FARWINDOWPROC DlgProc;      // ������� ��������� �������

    // ���������� ��� ����������� �������
    int OldX1,OldX2,OldY1,OldY2;

    wchar_t *HelpTopic;

    volatile int DropDownOpened;// �������� ������ ���������� � �������: TRUE - ������, FALSE - ������.

    CriticalSection CS;

    int RealWidth, RealHeight;

  private:
    void Init(FARWINDOWPROC DlgProc,LONG_PTR InitParam);
    virtual void DisplayObject();
    void DeleteDialogObjects();
    int  LenStrItem(int ID, const wchar_t *lpwszStr = NULL);

    void ShowDialog(int ID=-1);  //    ID=-1 - ���������� ���� ������

    LONG_PTR CtlColorDlgItem(int ItemPos,int Type,int Focus,DWORD Flags);
    /* $ 28.07.2000 SVS
       + �������� ����� ����� ����� ����� ����������.
         ������� �������� ��� ����, ����� ���������� DMSG_KILLFOCUS & DMSG_SETFOCUS
    */
    int ChangeFocus2(unsigned KillFocusPos,unsigned SetFocusPos);

    int ChangeFocus(unsigned FocusPos,int Step,int SkipGroup);
    BOOL SelectFromEditHistory(struct DialogItemEx *CurItem,DlgEdit *EditLine,const wchar_t *HistoryName,string &strStr,int MaxLen);
    int SelectFromComboBox(struct DialogItemEx *CurItem,DlgEdit*EditLine,VMenu *List,int MaxLen);
    int FindInEditForAC(int TypeFind, const wchar_t *HistoryName, string &strFindStr);
    int AddToEditHistory(const wchar_t *AddStr,const wchar_t *HistoryName);

    void ProcessLastHistory (struct DialogItemEx *CurItem, int MsgIndex); // ��������� DIF_USELASTHISTORY

    int ProcessHighlighting(int Key,int FocusPos,int Translate);
    BOOL CheckHighlights(WORD Chr);

    void SelectOnEntry(int Pos,BOOL Selected);

    void CheckDialogCoord(void);
    BOOL GetItemRect(int I,RECT& Rect);

    // ���������� ��������� ������� (����� ������� ������ ��� ������)
    const wchar_t *GetDialogTitle();

    BOOL SetItemRect(int ID,SMALL_RECT *Rect);

    /* $ 23.06.2001 KM
       + ������� ������������ ��������/�������� ���������� � �������
         � ��������� ������� ����������/���������� ���������� � �������.
    */
    volatile void SetDropDownOpened(int Status){ DropDownOpened=Status; }
    volatile int GetDropDownOpened(){ return DropDownOpened; }

    void ProcessCenterGroup(void);
    int ProcessRadioButton(int);

    int InitDialogObjects(int ID=-1);

    int ProcessOpenComboBox(int Type,struct DialogItemEx *CurItem,int CurFocusPos);
    int ProcessMoveDialog(DWORD Key);

    int Do_ProcessTab(int Next);
    int Do_ProcessNextCtrl(int Next,BOOL IsRedraw=TRUE);
    int Do_ProcessFirstCtrl();
    int Do_ProcessSpace();

    LONG_PTR CallDlgProc (int nMsg, int nParam1, LONG_PTR nParam2);

  public:
    Dialog(struct DialogItemEx *SrcItem, unsigned SrcItemCount,
           FARWINDOWPROC DlgProc=NULL,LONG_PTR InitParam=0);
    Dialog(struct FarDialogItem *SrcItem, unsigned SrcItemCount,
           FARWINDOWPROC DlgProc=NULL,LONG_PTR InitParam=0);
    bool InitOK() {return bInitOK;}
    virtual ~Dialog();

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);
    virtual void Show();
    virtual void Hide();
    void FastShow() {ShowDialog();}

    void GetDialogObjectsData();

    void SetDialogMode(DWORD Flags){ DialogMode.Set(Flags); }

    // �������������� �� ����������� ������������� � FarDialogItem � �������
    static bool ConvertItemEx (int FromPlugin, struct FarDialogItem *Item,
                               struct DialogItemEx *Data, unsigned Count);

    static void DataToItemEx(struct DialogDataEx *Data,struct DialogItemEx *Item,
                           int Count);

    static int IsKeyHighlighted(const wchar_t *Str,int Key,int Translate,int AmpPos=-1);

    // ����� ��� ����������� �������
    void AdjustEditPos(int dx,int dy);

    int IsMoving() {return DialogMode.Check(DMODE_DRAGGED);}
    void SetModeMoving(int IsMoving) { DialogMode.Change(DMODE_ISCANMOVE,IsMoving);}
    int  GetModeMoving(void) {return DialogMode.Check(DMODE_ISCANMOVE);}
    void SetDialogData(LONG_PTR NewDataDialog);
    LONG_PTR GetDialogData(void) {return DataDialog;};

    void InitDialog(void);
    void Process();
    void SetPluginNumber(INT_PTR NewPluginNumber){PluginNumber=NewPluginNumber;}

    void SetHelp(const wchar_t *Topic);
    void ShowHelp();
    int Done() { return DialogMode.Check(DMODE_ENDLOOP); }
    void ClearDone();
    virtual void SetExitCode (int Code);

    void CloseDialog();

    virtual int GetTypeAndName(string &strType, string &strName);
    virtual int GetType() { return MODALTYPE_DIALOG; }
    virtual const wchar_t *GetTypeName() {return L"[Dialog]";};

    virtual int GetMacroMode();

    /* $ ������� ��� ���� CtrlAltShift OT */
    virtual int FastHide();
    virtual void ResizeConsole();
//    virtual void OnDestroy();

    // For MACRO
    const struct DialogItemEx **GetAllItem(){return (const DialogItemEx**)Item;};
    int GetAllItemCount(){return ItemCount;};              // ���������� ��������� �������
    int GetDlgFocusPos(){return FocusPos;};


    int SetAutomation(WORD IDParent,WORD id,
                        DWORD UncheckedSet,DWORD UncheckedSkip,
                        DWORD CheckedSet,DWORD CheckedSkip,
                        DWORD Checked3Set=0,DWORD Checked3Skip=0);

    /* $ 23.07.2000 SVS: ������� ��������� ������� (�� ���������) */
    static LONG_PTR WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
    /* $ 28.07.2000 SVS: ������� ������� ��������� ������� */
    static LONG_PTR WINAPI SendDlgMessage(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

    virtual void SetPosition(int X1,int Y1,int X2,int Y2);
};

#endif // __DIALOG_HPP__
