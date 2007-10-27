/*
manager.cpp

������������ ����� ����������� file panels, viewers, editors, dialogs
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

#include "manager.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "frame.hpp"
#include "vmenu.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"

Manager *FrameManager;

Manager::Manager()
{
  FrameList=NULL;
  FrameCount=FrameListSize=0;
  FramePos=-1;
  ModalStack=NULL;
  FrameList=(Frame **)xf_realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));

  ModalStack=NULL;
  ModalStackSize = ModalStackCount = 0;
  EndLoop = FALSE;
  RefreshedFrame=NULL;

  CurrentFrame  = NULL;
  InsertedFrame = NULL;
  DeletedFrame  = NULL;
  ActivatedFrame= NULL;
  DeactivatedFrame=NULL;
  ModalizedFrame=NULL;
  UnmodalizedFrame=NULL;
  ExecutedFrame=NULL;
  //SemiModalBackFrames=NULL; //������ ��� ������
  //SemiModalBackFramesCount=0;
  //SemiModalBackFramesSize=0;
  ModalEVCount=0;
  StartManager=FALSE;
}

Manager::~Manager()
{
  if (FrameList)
    xf_free(FrameList);
  if (ModalStack)
    xf_free (ModalStack);
  /*if (SemiModalBackFrames)
    xf_free(SemiModalBackFrames);*/
}


/* $ 29.12.2000 IS
  ������ CloseAll, �� ��������� ����������� ����������� ������ � ����,
  ���� ������������ ��������� ������������� ����.
  ���������� TRUE, ���� ��� ������� � ����� �������� �� ����.
*/
BOOL Manager::ExitAll()
{
  int i;
  for (i=this->ModalStackCount-1; i>=0; i--){
    Frame *iFrame=this->ModalStack[i];
    if (!iFrame->GetCanLoseFocus(TRUE)){
      int PrevFrameCount=ModalStackCount;
      iFrame->ProcessKey(KEY_ESC);
      Commit();
      if (PrevFrameCount==ModalStackCount){
        return FALSE;
      }
    }
  }
  for (i=FrameCount-1; i>=0; i--){
    Frame *iFrame=FrameList[i];
    if (!iFrame->GetCanLoseFocus(TRUE)){
      ActivateFrame(iFrame);
      Commit();
      int PrevFrameCount=FrameCount;
      iFrame->ProcessKey(KEY_ESC);
      Commit();
      if (PrevFrameCount==FrameCount){
        return FALSE;
      }
    }
  }
  return TRUE;
}

void Manager::CloseAll()
{
  int i;
  Frame *iFrame;
  for (i=ModalStackCount-1;i>=0;i--){
    iFrame=ModalStack[i];
    DeleteFrame(iFrame);
    DeleteCommit();
    DeletedFrame=NULL;
  }
  for (i=FrameCount-1;i>=0;i--){
    iFrame=(*this)[i];
    DeleteFrame(iFrame);
    DeleteCommit();
    DeletedFrame=NULL;
  }
  xf_free(FrameList);
  FrameList=NULL;
  FrameCount=FramePos=0;
}

BOOL Manager::IsAnyFrameModified(int Activate)
{
  for (int I=0;I<FrameCount;I++)
    if (FrameList[I]->IsFileModified())
    {
      if (Activate)
      {
        ActivateFrame(I);
        Commit();
      }
      return(TRUE);
    }

  return(FALSE);
}

void Manager::InsertFrame(Frame *Inserted, int Index)
{
  _OT(SysLog(L"InsertFrame(), Inserted=%p, Index=%i",Inserted, Index));
  if (Index==-1)
    Index=FramePos;
  InsertedFrame=Inserted;
}

void Manager::DeleteFrame(Frame *Deleted)
{
  _OT(SysLog(L"DeleteFrame(), Deleted=%p",Deleted));
  for (int i=0;i<FrameCount;i++){
    Frame *iFrame=FrameList[i];
    if(iFrame->RemoveModal(Deleted)){
      return;
    }
  }
  if (!Deleted){
    DeletedFrame=CurrentFrame;
  } else {
    DeletedFrame=Deleted;
  }
}

void Manager::DeleteFrame(int Index)
{
  _OT(SysLog(L"DeleteFrame(), Index=%i",Index));
  DeleteFrame(this->operator[](Index));
}


void Manager::ModalizeFrame (Frame *Modalized, int Mode)
{
  _OT(SysLog(L"ModalizeFrame(), Modalized=%p",Modalized));
  ModalizedFrame=Modalized;
  ModalizeCommit();
}

void Manager::UnmodalizeFrame (Frame *Unmodalized)
{
  UnmodalizedFrame=Unmodalized;
  UnmodalizeCommit();
}

void Manager::ExecuteNonModal ()
{
  _OT(SysLog(L"ExecuteNonModal(), ExecutedFrame=%p, InsertedFrame=%p, DeletedFrame=%p",ExecutedFrame, InsertedFrame, DeletedFrame));
  Frame *NonModal=InsertedFrame?InsertedFrame:(ExecutedFrame?ExecutedFrame:ActivatedFrame);
  if (!NonModal) {
    return;
  }
  /* $ 14.05.2002 SKV
    ������� ������� ����� � ������ "���������" ������������� �������
  */
  //Frame *SaveFrame=CurrentFrame;
  //AddSemiModalBackFrame(SaveFrame);
  int NonModalIndex=IndexOf(NonModal);
  if (-1==NonModalIndex){
    InsertedFrame=NonModal;
    ExecutedFrame=NULL;
    InsertCommit();
    InsertedFrame=NULL;
  } else {
    ActivateFrame(NonModalIndex);
  }

  //Frame* ModalStartLevel=NonModal;
  while (1){
    Commit();
    if (CurrentFrame!=NonModal){
      break;
    }
    ProcessMainLoop();
  }

  //ExecuteModal(NonModal);
  /* $ 14.05.2002 SKV
    ... � ����� ��� ��.
  */
  //RemoveSemiModalBackFrame(SaveFrame);
}

void Manager::ExecuteModal (Frame *Executed)
{
  _OT(SysLog(L"ExecuteModal(), Executed=%p, ExecutedFrame=%p",Executed,ExecutedFrame));
  if (!Executed && !ExecutedFrame){
    return;
  }
  if (Executed){
    if (ExecutedFrame) {
      _OT(SysLog(L"������� � ����� ����� ��������� � ��������� ������ ��� ������. Executed=%p, ExecitedFrame=%p",Executed, ExecutedFrame));
      return;// NULL; //?? ����������, ����� �������� ��������� ���������� � ���� ������
    } else {
      ExecutedFrame=Executed;
    }
  }

  int ModalStartLevel=ModalStackCount;
  int OriginalStartManager=StartManager;
  StartManager=TRUE;
  while (1){
    Commit();
    if (ModalStackCount<=ModalStartLevel){
      break;
    }
    ProcessMainLoop();
  }
  StartManager=OriginalStartManager;
  return;// GetModalExitCode();
}

int Manager::GetModalExitCode()      {
  return ModalExitCode;
}

/* $ 11.10.2001 IS
   ���������� ���������� ������� � ��������� ������.
*/
int Manager::CountFramesWithName(const wchar_t *Name, BOOL IgnoreCase)
{
   int Counter=0;
   typedef int (__cdecl *cmpfunc_t)(const wchar_t *s1, const wchar_t *s2);
   cmpfunc_t cmpfunc=IgnoreCase ? StrCmpI : StrCmp;
   string strType, strCurName;
   for (int I=0;I<FrameCount;I++)
   {
     FrameList[I]->GetTypeAndName(strType, strCurName);
     if(!cmpfunc(Name, strCurName)) ++Counter;
   }
   return Counter;
}

/*!
  \return ���������� NULL ���� ����� "�����" ��� ���� ����� ������� �����.
  ������� �������, ���� ����������� ����� �� ���������.
  ���� �� ����� ���������, �� ����� ������� ������ ����������
  ��������� �� ���������� �����.
*/
Frame *Manager::FrameMenu()
{
  /* $ 28.04.2002 KM
      ���� ��� ����������� ����, ��� ���� ������������
      ������� ��� ������������.
  */
  static int AlreadyShown=FALSE;

  if (AlreadyShown)
    return NULL;

  int ExitCode, CheckCanLoseFocus=CurrentFrame->GetCanLoseFocus();
  {
    MenuItemEx ModalMenuItem;

    VMenu ModalMenu(UMSG(MScreensTitle),NULL,0,ScrY-4);
    ModalMenu.SetHelp(L"ScrSwitch");
    ModalMenu.SetFlags(VMENU_WRAPMODE);
    ModalMenu.SetPosition(-1,-1,0,0);

    if (!CheckCanLoseFocus)
      ModalMenuItem.SetDisable(TRUE);

    for (int I=0;I<FrameCount;I++)
    {
      string strType, strName, strNumText;
      FrameList[I]->GetTypeAndName(strType, strName);

      ModalMenuItem.Clear ();

      if (I<10)
        strNumText.Format (L"&%d. ",I);
      else if (I<36)
        strNumText.Format (L"&%c. ",I+55); // 55='A'-10
      else
        strNumText = L"&   ";

      TruncPathStr(strName,ScrX-24);
      ReplaceStrings(strName,L"&",L"&&",-1);
      /*  ����������� "*" ���� ���� ������� */
      ModalMenuItem.strName.Format (L"%s%-10.10s %c %s", (const wchar_t*)strNumText, (const wchar_t*)strType,(FrameList[I]->IsFileModified()?L'*':L' '), (const wchar_t*)strName);
      ModalMenuItem.SetSelect(I==FramePos);
      ModalMenu.AddItem(&ModalMenuItem);
    }
    AlreadyShown=TRUE;
    ModalMenu.Process();
    AlreadyShown=FALSE;
    ExitCode=ModalMenu.Modal::GetExitCode();
  }

  if(CheckCanLoseFocus)
  {
    if (ExitCode>=0)
    {
      ActivateFrame (ExitCode);
      return (ActivatedFrame==CurrentFrame || !CurrentFrame->GetCanLoseFocus()?NULL:CurrentFrame);
    }
    return (ActivatedFrame==CurrentFrame?NULL:CurrentFrame);
  }
  return NULL;
}


int Manager::GetFrameCountByType(int Type)
{
  int ret=0;
  for (int I=0;I<FrameCount;I++)
  {
    /* $ 10.05.2001 DJ
       �� ��������� �����, ������� ���������� �������
    */
    if (FrameList[I] == DeletedFrame || FrameList [I]->GetExitCode() == XC_QUIT)
      continue;
    if (FrameList[I]->GetType()==Type)
      ret++;
  }
  return ret;
}

void Manager::SetFramePos(int NewPos)
{
  _OT(SysLog(L"Manager::SetFramePos(), NewPos=%i",NewPos));
  FramePos=NewPos;
}

/*$ 11.05.2001 OT ������ ����� ������ ���� �� ������ �� ������� �����, �� � �������� - ����, �������� ��� */
int  Manager::FindFrameByFile(int ModalType,const wchar_t *FileName, const wchar_t *Dir)
{
  string strBufFileName;
  string strFullFileName = FileName;
  if (Dir)
  {
    strBufFileName = Dir;
    AddEndSlash(strBufFileName);
    strBufFileName += FileName;
    strFullFileName = strBufFileName;
  }

  for (int I=0;I<FrameCount;I++)
  {
    string strType, strName;
    if (FrameList[I]->GetTypeAndName(strType, strName)==ModalType)
      if (StrCmpI(strName, strFullFileName)==0)
        return(I);
  }
  return(-1);
}

BOOL Manager::ShowBackground()
{
  CtrlObject->CmdLine->ShowBackground();
  return TRUE;
}


void Manager::ActivateFrame(Frame *Activated)
{
  _OT(SysLog(L"ActivateFrame(), Activated=%i",Activated));
  if(IndexOf(Activated)==-1 && IndexOfStack(Activated)==-1)
    return;

  if (!ActivatedFrame)
  {
    ActivatedFrame=Activated;
  }
}

void Manager::ActivateFrame(int Index)
{
  _OT(SysLog(L"ActivateFrame(), Index=%i",Index));
  ActivateFrame((*this)[Index]);
}

void Manager::DeactivateFrame (Frame *Deactivated,int Direction)
{
  _OT(SysLog(L"DeactivateFrame(), Deactivated=%p",Deactivated));
  if (Direction) {
    FramePos+=Direction;
    if (Direction>0){
      if (FramePos>=FrameCount){
        FramePos=0;
      }
    } else {
      if (FramePos<0) {
        FramePos=FrameCount-1;
      }
    }
    ActivateFrame(FramePos);
  } else {
    // Direction==0
    // Direct access from menu or (in future) from plugin
  }
  DeactivatedFrame=Deactivated;
}

void Manager::SwapTwoFrame (int Direction)
{
  if (Direction)
  {
    int OldFramePos=FramePos;
    FramePos+=Direction;
    if (Direction>0)
    {
      if (FramePos>=FrameCount)
      {
        FramePos=0;
      }
    }
    else
    {
      if (FramePos<0)
      {
        FramePos=FrameCount-1;
      }
    }

    Frame *TmpFrame=FrameList[OldFramePos];
    FrameList[OldFramePos]=FrameList[FramePos];
    FrameList[FramePos]=TmpFrame;
    ActivateFrame(OldFramePos);
  }
  DeactivatedFrame=CurrentFrame;
}

void Manager::RefreshFrame(Frame *Refreshed)
{
  _OT(SysLog(L"RefreshFrame(), Refreshed=%p",Refreshed));

  if (ActivatedFrame)
    return;

  if (Refreshed)
  {
    RefreshedFrame=Refreshed;
  }
  else
  {
    RefreshedFrame=CurrentFrame;
  }

  if(IndexOf(Refreshed)==-1 && IndexOfStack(Refreshed)==-1)
    return;

  /* $ 13.04.2002 KM
    - �������� �������������� Commit() ��� ������ �������� �����
      NextModal, ��� �������� ��� �������� ������ ��������
      VMenu, � ������ Commit() ��� �� ����� ������ ����� ��������
      �� �������.
      ��������� ��� ���� ������ �������������, ����� ���� ���
      ������ ��������� ��������� �������� VMenu. ������:
      ��������� ������. ������ AltF9 � ������� ���������
      ������ ��������� �������������� ����.
  */
  if (RefreshedFrame && RefreshedFrame->NextModal)
    Commit();
}

void Manager::RefreshFrame(int Index)
{
  RefreshFrame((*this)[Index]);
}

void Manager::ExecuteFrame(Frame *Executed)
{
  _OT(SysLog(L"ExecuteFrame(), Executed=%p",Executed));
  ExecutedFrame=Executed;
}


/* $ 10.05.2001 DJ
   ������������� �� ������ (����� � ������� 0)
*/

void Manager::SwitchToPanels()
{
  ActivateFrame (0);
}


int Manager::HaveAnyFrame()
{
    if ( FrameCount || InsertedFrame || DeletedFrame || ActivatedFrame || RefreshedFrame ||
         ModalizedFrame || DeactivatedFrame || ExecutedFrame || CurrentFrame)
        return 1;
    return 0;
}

void Manager::EnterMainLoop()
{
  WaitInFastFind=0;
  StartManager=TRUE;
  while (1)
  {
    Commit();
    if (EndLoop || !HaveAnyFrame()) {
      break;
    }
    ProcessMainLoop();
  }
}

void Manager::ProcessMainLoop()
{

                                  // Mantis#0000073: �� �������� ������������ � QView
  WaitInMainLoop=IsPanelsActive() && ((FilePanels*)CurrentFrame)->ActivePanel->GetType()!=QVIEW_PANEL;

  //WaitInFastFind++;
  int Key=GetInputRecord(&LastInputRecord);
  //WaitInFastFind--;
  WaitInMainLoop=FALSE;
  if (EndLoop)
    return;
  if (LastInputRecord.EventType==MOUSE_EVENT)
    ProcessMouse(&LastInputRecord.Event.MouseEvent);
  else
    ProcessKey(Key);
}

void Manager::ExitMainLoop(int Ask)
{
  if (CloseFAR)
  {
    CloseFAR=FALSE;
    CloseFARMenu=TRUE;
  };
  if (!Ask || !Opt.Confirm.Exit || Message(0,2,UMSG(MQuit),UMSG(MAskQuit),UMSG(MYes),UMSG(MNo))==0)
   /* $ 29.12.2000 IS
      + ���������, ��������� �� ��� ���������� �����. ���� ���, �� �� �������
        �� ����.
   */
   if(ExitAll())
   {
   //TODO: ��� �������� �� x ����� ������ ������������� �����. ����� ����� ����
   //      �����, ��������, ��� ������������
     FilePanels *cp;
     if ( (cp = CtrlObject->Cp()) == NULL
        || (!cp->LeftPanel->ProcessPluginEvent(FE_CLOSE,NULL) && !cp->RightPanel->ProcessPluginEvent(FE_CLOSE,NULL)) )
       EndLoop=TRUE;
   } else {
     CloseFARMenu=FALSE;
   }
}

#if defined(FAR_ALPHA_VERSION)
#include <float.h>
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4717)
#ifdef _WIN64
#ifdef __cplusplus
extern "C"
#endif
          void _cdecl exec_ud2_cmd(void);
#endif
#endif
static void Test_EXCEPTION_STACK_OVERFLOW(char* target)
{
   char Buffer[1024]; /* ����� ������� ������� */
   strcpy( Buffer, "zzzz" );
   Test_EXCEPTION_STACK_OVERFLOW( Buffer );
}
#if defined(_MSC_VER)
#pragma warning( pop )
#endif
#endif


int  Manager::ProcessKey(DWORD Key)
{
  int ret=FALSE;

  if ( CurrentFrame)
  {
    int i=0;

    DWORD KeyM=(Key&(~KEY_CTRLMASK));
    if(!(/*KeyM >= KEY_MACRO_BASE && KeyM <= KEY_MACRO_ENDBASE ||*/ KeyM >= KEY_OP_BASE && KeyM <= KEY_OP_ENDBASE)) // ��������� �����-����
    {
      switch(CurrentFrame->GetType())
      {
        case MODALTYPE_PANELS:
        {
          _ALGO(CleverSysLog clv(L"Manager::ProcessKey()"));
          _ALGO(SysLog(L"Key=%u (0x%08X)",Key,Key));
          if(CtrlObject->Cp()->ActivePanel->SendKeyToPlugin(Key,TRUE))
            return TRUE;
          break;
        }
        case MODALTYPE_VIEWER:
          //if(((FileViewer*)CurrentFrame)->ProcessViewerInput(FrameManager->GetLastInputRecord()))
          //  return TRUE;
          break;
        case MODALTYPE_EDITOR:
          //if(((FileEditor*)CurrentFrame)->ProcessEditorInput(FrameManager->GetLastInputRecord()))
          //  return TRUE;
          break;
        case MODALTYPE_DIALOG:
          //((Dialog*)CurrentFrame)->CallDlgProc(DN_KEY,((Dialog*)CurrentFrame)->GetDlgFocusPos(),Key);
          break;
        case MODALTYPE_VMENU:
        case MODALTYPE_HELP:
        case MODALTYPE_COMBOBOX:
        case MODALTYPE_USER:
        case MODALTYPE_FINDFOLDER:
        default:
          break;
      }
    }

#if defined(FAR_ALPHA_VERSION)
// ��� ��� ��� �������� ����������, ������� �� ������� :-)
    if(Key == (KEY_APPS|KEY_CTRL|KEY_ALT) && GetRegKey(L"System\\Exception",L"Used",0))
    {
      struct __ECODE {
        DWORD Code;
        const wchar_t *Name;
      } ECode[]={
        {EXCEPTION_ACCESS_VIOLATION,L"Access Violation (Read)"},
        {EXCEPTION_ACCESS_VIOLATION,L"Access Violation (Write)"},
        {EXCEPTION_INT_DIVIDE_BY_ZERO,L"Divide by zero"},
        {EXCEPTION_ILLEGAL_INSTRUCTION,L"Illegal instruction"},
        {EXCEPTION_STACK_OVERFLOW,L"Stack Overflow"},
        {EXCEPTION_FLT_DIVIDE_BY_ZERO,L"Floating-point divide by zero"},
/*
        {EXCEPTION_FLT_OVERFLOW,"EXCEPTION_FLT_OVERFLOW"},
        {EXCEPTION_DATATYPE_MISALIGNMENT,"EXCEPTION_DATATYPE_MISALIGNMENT",},
        {EXCEPTION_BREAKPOINT,"EXCEPTION_BREAKPOINT",},
        {EXCEPTION_SINGLE_STEP,"EXCEPTION_SINGLE_STEP",},
        {EXCEPTION_ARRAY_BOUNDS_EXCEEDED,"EXCEPTION_ARRAY_BOUNDS_EXCEEDED",},
        {EXCEPTION_FLT_DENORMAL_OPERAND,"EXCEPTION_FLT_DENORMAL_OPERAND",},
        {EXCEPTION_FLT_INEXACT_RESULT,"EXCEPTION_FLT_INEXACT_RESULT",},
        {EXCEPTION_FLT_INVALID_OPERATION,"EXCEPTION_FLT_INVALID_OPERATION",},
        {EXCEPTION_FLT_STACK_CHECK,"EXCEPTION_FLT_STACK_CHECK",},
        {EXCEPTION_FLT_UNDERFLOW,"EXCEPTION_FLT_UNDERFLOW",},
        {EXCEPTION_INT_OVERFLOW,"EXCEPTION_INT_OVERFLOW",0},
        {EXCEPTION_PRIV_INSTRUCTION,"EXCEPTION_PRIV_INSTRUCTION",0},
        {EXCEPTION_IN_PAGE_ERROR,"EXCEPTION_IN_PAGE_ERROR",0},
        {EXCEPTION_NONCONTINUABLE_EXCEPTION,"EXCEPTION_NONCONTINUABLE_EXCEPTION",0},
        {EXCEPTION_INVALID_DISPOSITION,"EXCEPTION_INVALID_DISPOSITION",0},
        {EXCEPTION_GUARD_PAGE,"EXCEPTION_GUARD_PAGE",0},
        {EXCEPTION_INVALID_HANDLE,"EXCEPTION_INVALID_HANDLE",0},
*/
      };

      static union {
        int     i;
        int     *iptr;
        double  d;
      }zero_const, refers;

      MenuItemEx ModalMenuItem;

      ModalMenuItem.Clear ();
      VMenu ModalMenu(L"Test Exceptions",NULL,0,ScrY-4);
      ModalMenu.SetFlags(VMENU_WRAPMODE);
      ModalMenu.SetPosition(-1,-1,0,0);

      for (int I=0;I<sizeof(ECode)/sizeof(ECode[0]);I++)
      {
        ModalMenuItem.strName = ECode[I].Name;
        ModalMenu.AddItem(&ModalMenuItem);
      }

      ModalMenu.Process();
      int ExitCode=ModalMenu.Modal::GetExitCode();

      switch(ExitCode)
      {
        case 0:
          return *zero_const.iptr;
        case 1:
          *zero_const.iptr = 0;
          break;
        case 2:
          return i / zero_const.i;
        case 3:
#ifdef _WIN64
          exec_ud2_cmd();
#elif defined(__GNUC__)
          asm("ud2");
#elif defined(_MSC_VER)
          _asm _emit 0xF
          _asm _emit 0xB
#elif defined(__BORLANDC__)
          __emit__(0xF, 0xB);
#else
#error "Unsupported compiler"
#endif
          return 0;
        case 4:
          Test_EXCEPTION_STACK_OVERFLOW(NULL);
          return 0;
        case 5:
          refers.d = 1 / zero_const.d;
          return 0;
      }
      return TRUE;
    }
#endif

    /*** ���� ����������������� ������ ! ***/
    /***   ������� ������ �����������    ***/
    switch(Key)
    {
      case (KEY_ALT|KEY_NUMPAD0):
      case (KEY_ALT|KEY_INS):
      {
        RunGraber();
        return TRUE;
      }

      case KEY_CONSOLE_BUFFER_RESIZE:
        Sleep(1);
        ResizeAllFrame();
        return TRUE;
    }

    /*** � ��� ����� - ��� ���������! ***/
    if(!IsProcessAssignMacroKey || IsProcessVE_FindFile)
       // � ����� ������ ���� ����-�� ������� ��� ������� ���
    {
      /* ** ��� ������� ��������� ��� ������ �������/���������
            �� ����� ������ ����� �� ������ ������ ** */
      switch(Key)
      {
        case KEY_CTRLW:
          ShowProcessList();
          return(TRUE);

        case KEY_F11:
          PluginsMenu();
          FrameManager->RefreshFrame();
          _OT(SysLog(-1));
          return TRUE;

        case KEY_ALTF9:
        {
          //_SVS(SysLog(1,"Manager::ProcessKey, KEY_ALTF9 pressed..."));
          Sleep(1);
          SetVideoMode(FarAltEnter(-2));
          Sleep(1);

          /* � �������� ���������� Alt-F9 (� ���������� ������) � �������
             ������� �������� WINDOW_BUFFER_SIZE_EVENT, ����������� �
             ChangeVideoMode().
             � ������ ���������� �������� ��� �� ���������� �� ������ ��������
             ��������.
          */
          if(CtrlObject->Macro.IsExecuting())
          {
            int PScrX=ScrX;
            int PScrY=ScrY;
            Sleep(1);
            GetVideoMode(CurScreenBufferInfo);
            if (PScrX+1 == CurScreenBufferInfo.dwSize.X &&
                PScrY+1 == CurScreenBufferInfo.dwSize.Y)
            {
              //_SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_NONE"));
              return TRUE;
            }
            else
            {
              PrevScrX=PScrX;
              PrevScrY=PScrY;
              //_SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_CONSOLE_BUFFER_RESIZE"));
              Sleep(1);

              return ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
            }
          }
          //_SVS(SysLog(-1));
          return TRUE;
        }

        case KEY_F12:
        {
          int TypeFrame=FrameManager->GetCurrentFrame()->GetType();
          if(TypeFrame != MODALTYPE_HELP && TypeFrame != MODALTYPE_DIALOG)
          {
            DeactivateFrame(FrameMenu(),0);
            _OT(SysLog(-1));
            return TRUE;
          }
          break; // ������� F12 ������ �� �������
        }
      }

      // � ����� ��, ��� ����� ���� ��������� ����� :-)
      if(!IsProcessVE_FindFile)
      {
        switch(Key)
        {
          case KEY_CTRLALTSHIFTPRESS:
          case KEY_RCTRLALTSHIFTPRESS:
          {
            if(!(Opt.CASRule&1) && Key == KEY_CTRLALTSHIFTPRESS)
              break;
            if(!(Opt.CASRule&2) && Key == KEY_RCTRLALTSHIFTPRESS)
              break;
            if(!NotUseCAS)
            {
              if (CurrentFrame->FastHide())
              {
                int isPanelFocus=CurrentFrame->GetType() == MODALTYPE_PANELS;
                if(isPanelFocus)
                {
                  int LeftVisible=CtrlObject->Cp()->LeftPanel->IsVisible();
                  int RightVisible=CtrlObject->Cp()->RightPanel->IsVisible();
                  int CmdLineVisible=CtrlObject->CmdLine->IsVisible();
                  int KeyBarVisible=CtrlObject->Cp()->MainKeyBar.IsVisible();
                  CtrlObject->CmdLine->ShowBackground();

                  CtrlObject->Cp()->LeftPanel->Hide0();
                  CtrlObject->Cp()->RightPanel->Hide0();
                  switch(Opt.PanelCtrlAltShiftRule)
                  {
                    case 0:
                      CtrlObject->CmdLine->Show();
                      CtrlObject->Cp()->MainKeyBar.Show();
                      break;
                    case 1:
                      CtrlObject->Cp()->MainKeyBar.Show();
                      break;
                  }

                  WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:KEY_RCTRLALTSHIFTRELEASE);

                  if (LeftVisible)      CtrlObject->Cp()->LeftPanel->Show();
                  if (RightVisible)     CtrlObject->Cp()->RightPanel->Show();
                  if (CmdLineVisible)   CtrlObject->CmdLine->Show();
                  if (KeyBarVisible)    CtrlObject->Cp()->MainKeyBar.Show();
                }
                else
                {
                  ImmediateHide();
                  WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:KEY_RCTRLALTSHIFTRELEASE);
                }
                FrameManager->RefreshFrame();
              }
              return TRUE;
            }
            break;
          }

          case KEY_CTRLTAB:
          case KEY_CTRLSHIFTTAB:
            if (CurrentFrame->GetCanLoseFocus()){
              DeactivateFrame(CurrentFrame,Key==KEY_CTRLTAB?1:-1);
            }
            _OT(SysLog(-1));
            return TRUE;
        }
      }
    }
    CurrentFrame->UpdateKeyBar();
    CurrentFrame->ProcessKey(Key);
  }
  _OT(SysLog(-1));
  return ret;
}

int  Manager::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
    // ��� ����������� ���� ������ ���������� ��������� �������
//    if (ScreenObject::CaptureMouseObject)
//      return ScreenObject::CaptureMouseObject->ProcessMouse(MouseEvent);

    int ret=FALSE;
//    _D(SysLog(1,"Manager::ProcessMouse()"));
    if ( CurrentFrame)
        ret=CurrentFrame->ProcessMouse(MouseEvent);
//    _D(SysLog(L"Manager::ProcessMouse() ret=%i",ret));
    _OT(SysLog(-1));
    return ret;
}

void Manager::PluginsMenu()
{
  _OT(SysLog(1));
  int curType = CurrentFrame->GetType();
  if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER)
  {
    /* 02.01.2002 IS
       ! ����� ���������� ������ �� Shift-F1 � ���� �������� � ���������/������
       ! ���� �� ������ QVIEW ��� INFO ������ ����, �� �������, ��� ���
         ����������� ����� � ��������� � ��������������� ���������� �������
    */
    if(curType==MODALTYPE_PANELS)
    {
      int pType=CtrlObject->Cp()->ActivePanel->GetType();
      if(pType==QVIEW_PANEL || pType==INFO_PANEL)
      {
         string strType, strCurFileName;
         CtrlObject->Cp()->GetTypeAndName(strType, strCurFileName);
         if( !strCurFileName.IsEmpty () )
         {
           DWORD Attr=GetFileAttributesW(strCurFileName);
           // ���������� ������ ������� �����
           if(Attr!=0xFFFFFFFF && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
             curType=MODALTYPE_VIEWER;
         }
      }
    }

    // � ��������� ��� ������ ������� ���� ������ �� Shift-F1
    const wchar_t *Topic=curType==MODALTYPE_EDITOR?L"Editor":
      curType==MODALTYPE_VIEWER?L"Viewer":NULL;
    CtrlObject->Plugins.CommandsMenu(curType,0,Topic);
  }
  _OT(SysLog(-1));
}

BOOL Manager::IsPanelsActive()
{
  if (FramePos>=0) {
    return CurrentFrame?CurrentFrame->GetType() == MODALTYPE_PANELS:FALSE;
  } else {
    return FALSE;
  }
}

Frame *Manager::operator[](int Index)
{
  if (Index<0||Index>=FrameCount ||FrameList==0){
    return NULL;
  }
  return FrameList[Index];
}

int Manager::IndexOfStack(Frame *Frame)
{
  int Result=-1;
  for (int i=0;i<ModalStackCount;i++)
  {
    if (Frame==ModalStack[i])
    {
      Result=i;
      break;
    }
  }
  return Result;
}

int Manager::IndexOf(Frame *Frame)
{
  int Result=-1;
  for (int i=0;i<FrameCount;i++)
  {
    if (Frame==FrameList[i])
    {
      Result=i;
      break;
    }
  }
  return Result;
}

BOOL Manager::Commit()
{
  _OT(SysLog(1));
  int Result = false;
  if (DeletedFrame && (InsertedFrame||ExecutedFrame)){
    UpdateCommit();
    DeletedFrame = NULL;
    InsertedFrame = NULL;
    ExecutedFrame=NULL;
    Result=true;
  } else if (ExecutedFrame) {
    ExecuteCommit();
    ExecutedFrame=NULL;
    Result=true;
  } else if (DeletedFrame){
    DeleteCommit();
    DeletedFrame = NULL;
    Result=true;
  } else if (InsertedFrame){
    InsertCommit();
    InsertedFrame = NULL;
    Result=true;
  } else if(DeactivatedFrame){
    DeactivateCommit();
    DeactivatedFrame=NULL;
    Result=true;
  } else if(ActivatedFrame){
    ActivateCommit();
    ActivatedFrame=NULL;
    Result=true;
  } else if (RefreshedFrame){
    RefreshCommit();
    RefreshedFrame=NULL;
    Result=true;
  } else if (ModalizedFrame){
    ModalizeCommit();
//    ModalizedFrame=NULL;
    Result=true;
  } else if (UnmodalizedFrame){
    UnmodalizeCommit();
//    UnmodalizedFrame=NULL;
    Result=true;
  }
  if (Result){
    Result=Commit();
  }
  _OT(SysLog(-1));
  return Result;
}

void Manager::DeactivateCommit()
{
  _OT(SysLog(L"DeactivateCommit(), DeactivatedFrame=%p",DeactivatedFrame));
  /*$ 18.04.2002 skv
    ���� ������ ������������, �� � �����-�� �� ���� � ��������������.
  */
  if (!DeactivatedFrame || !ActivatedFrame)
  {
    return;
  }

  if (!ActivatedFrame)
  {
    _OT("WARNING!!!!!!!!");
  }

  if (DeactivatedFrame)
  {
    DeactivatedFrame->OnChangeFocus(0);
  }

  int modalIndex=IndexOfStack(DeactivatedFrame);
  if (-1 != modalIndex && modalIndex== ModalStackCount-1)
  {
    /*if (IsSemiModalBackFrame(ActivatedFrame))
    { // �������� �� "���������" �������������� ������?
      ModalStackCount--;
    }
    else
    {*/
      if(IndexOfStack(ActivatedFrame)==-1)
      {
        ModalStack[ModalStackCount-1]=ActivatedFrame;
      }
      else
      {
        ModalStackCount--;
      }
//    }
  }
}


void Manager::ActivateCommit()
{
  _OT(SysLog(L"ActivateCommit(), ActivatedFrame=%p",ActivatedFrame));
  if (CurrentFrame==ActivatedFrame)
  {
    RefreshedFrame=ActivatedFrame;
    return;
  }

  int FrameIndex=IndexOf(ActivatedFrame);

  if (-1!=FrameIndex)
  {
    FramePos=FrameIndex;
  }
  /* 14.05.2002 SKV
    ���� �� �������� ������������ ������������� �����,
    �� ���� ��� ������� �� ���� ����� �������.
  */

  for(int I=0;I<ModalStackCount;I++)
  {
    if(ModalStack[I]==ActivatedFrame)
    {
      Frame *tmp=ModalStack[I];
      ModalStack[I]=ModalStack[ModalStackCount-1];
      ModalStack[ModalStackCount-1]=tmp;
      break;
    }
  }

  RefreshedFrame=CurrentFrame=ActivatedFrame;
}

void Manager::UpdateCommit()
{
  _OT(SysLog(L"UpdateCommit(), DeletedFrame=%p, InsertedFrame=%p, ExecutedFrame=%p",DeletedFrame,InsertedFrame, ExecutedFrame));
  if (ExecutedFrame){
    DeleteCommit();
    ExecuteCommit();
    return;
  }
  int FrameIndex=IndexOf(DeletedFrame);
  if (-1!=FrameIndex){
    ActivateFrame(FrameList[FrameIndex] = InsertedFrame);
    ActivatedFrame->FrameToBack=CurrentFrame;
    DeleteCommit();
  } else {
    _OT(SysLog(L"UpdateCommit(). ������ �� ������ ��������� �����"));
  }
}

//! ������� DeletedFrame ��� ���� ��������!
//! ��������� ��������� ��������, (������ �� ����� �������������)
//! �� ������ � ��� ������, ���� �������� ����� ��� �� �������� �������.
void Manager::DeleteCommit()
{
  _OT(SysLog(L"DeleteCommit(), DeletedFrame=%p",DeletedFrame));
  if (!DeletedFrame)
  {
    return;
  }

  // <ifDoubleInstance>
  //BOOL ifDoubI=ifDoubleInstance(DeletedFrame);
  // </ifDoubleInstance>
  int ModalIndex=IndexOfStack(DeletedFrame);
  if (ModalIndex!=-1)
  {
    /* $ 14.05.2002 SKV
      ������� ����� � ������� ������ ��, ���
      �����, � �� ������ �������.
    */
    for(int i=0;i<ModalStackCount;i++)
    {
      if(ModalStack[i]==DeletedFrame)
      {
        for(int j=i+1;j<ModalStackCount;j++)
        {
          ModalStack[j-1]=ModalStack[j];
        }
        ModalStackCount--;
        break;
      }
    }
    if (ModalStackCount)
    {
      ActivateFrame(ModalStack[ModalStackCount-1]);
    }
  }

  for (int i=0;i<FrameCount;i++)
  {
    if (FrameList[i]->FrameToBack==DeletedFrame)
    {
      FrameList[i]->FrameToBack=CtrlObject->Cp();
    }
  }

  int FrameIndex=IndexOf(DeletedFrame);
  if (-1!=FrameIndex)
  {
    DeletedFrame->DestroyAllModal();
    for (int j=FrameIndex; j<FrameCount-1; j++ ){
      FrameList[j]=FrameList[j+1];
    }
    FrameCount--;
    if ( FramePos >= FrameCount ) {
      FramePos=0;
    }
    if (DeletedFrame->FrameToBack==CtrlObject->Cp()){
      ActivateFrame(FrameList[FramePos]);
    } else {
      ActivateFrame(DeletedFrame->FrameToBack);
    }
  }

  /* $ 14.05.2002 SKV
    ����� �� ��� ������, ����� �� �� ���� ��� ��� ���.
    �� ����� ��� �����.
    SVS> ����� ����������� - � ��������� ������ �������������� ����� ����
         ���������� �������� <ifDoubleInstance>

  if (ifDoubI && IsSemiModalBackFrame(ActivatedFrame)){
    for(int i=0;i<ModalStackCount;i++)
    {
      if(ModalStack[i]==ActivatedFrame)
      {
        break;
      }
    }

    if(i==ModalStackCount)
    {
      if (ModalStackCount == ModalStackSize){
        ModalStack = (Frame **) xf_realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
      }
      ModalStack[ModalStackCount++]=ActivatedFrame;
    }
  }
  */


  DeletedFrame->OnDestroy();
  if (DeletedFrame->GetDynamicallyBorn())
  {
    _tran(SysLog(L"delete DeletedFrame %p, CurrentFrame=%p",DeletedFrame,CurrentFrame));
    if ( CurrentFrame==DeletedFrame )
      CurrentFrame=0;
    /* $ 14.05.2002 SKV
      ��� ��� � ����������� ������ ������ ����� ����
      ������ commit, �� ���� ���������������.
    */
    Frame *tmp=DeletedFrame;
    DeletedFrame=NULL;
    delete tmp;
  }

  // ���������� �� ��, ��� � ActevateFrame �� ����� ��������� ���
  // �����������  ActivatedFrame
  if (ModalStackCount){
    ActivateFrame(ModalStack[ModalStackCount-1]);
  } else {
    ActivateFrame(FramePos);
  }
}

void Manager::InsertCommit()
{
  _OT(SysLog(L"InsertCommit(), InsertedFrame=%p",InsertedFrame));
  if (InsertedFrame){
    if (FrameListSize <= FrameCount)
    {
      FrameList=(Frame **)xf_realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));
      FrameListSize++;
    }
    InsertedFrame->FrameToBack=CurrentFrame;
    FrameList[FrameCount]=InsertedFrame;
    if (!ActivatedFrame){
      ActivatedFrame=InsertedFrame;
    }
    FrameCount++;
  }
}

void Manager::RefreshCommit()
{
  _OT(SysLog(L"RefreshCommit(), RefreshedFrame=%p,Refreshable()=%i",RefreshedFrame,RefreshedFrame->Refreshable()));
  if (!RefreshedFrame)
    return;

  if(IndexOf(RefreshedFrame)==-1 && IndexOfStack(RefreshedFrame)==-1)
    return;

  if ( !RefreshedFrame->Locked() )
  {
    if (!IsRedrawFramesInProcess)
      RefreshedFrame->ShowConsoleTitle();
    RefreshedFrame->Refresh();
    if (!RefreshedFrame)
      return;
    CtrlObject->Macro.SetMode(RefreshedFrame->GetMacroMode());
  }
  if (Opt.ViewerEditorClock &&
      (RefreshedFrame->GetType() == MODALTYPE_EDITOR ||
      RefreshedFrame->GetType() == MODALTYPE_VIEWER)
      || WaitInMainLoop && Opt.Clock)
    ShowTime(1);
}

void Manager::ExecuteCommit()
{
  _OT(SysLog(L"ExecuteCommit(), ExecutedFrame=%p",ExecutedFrame));
  if (!ExecutedFrame) {
    return;
  }
  if (ModalStackCount == ModalStackSize){
    ModalStack = (Frame **) xf_realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
  }
  ModalStack [ModalStackCount++] = ExecutedFrame;
  ActivatedFrame=ExecutedFrame;
}

/*$ 26.06.2001 SKV
  ��� ������ �� �������� ����������� ACTL_COMMIT
*/
BOOL Manager::PluginCommit()
{
  return Commit();
}

/* $ ������� ��� ���� CtrlAltShift OT */
void Manager::ImmediateHide()
{
  if (FramePos<0)
    return;

  // ������� ���������, ���� �� � ������������ ������ SaveScreen
  if (CurrentFrame->HasSaveScreen())
  {
    CurrentFrame->Hide();
    return;
  }

  // ������ ����������������, ������ ��� ������
  // �� ���������� ��������� �������, ����� �� �������.
  if (ModalStackCount>0)
  {
    /* $ 28.04.2002 KM
        ��������, � �� ��������� �� �������� ��� ������ �� �������
        ���������� �����? � ���� ��, ������� User screen.
    */
    if (ModalStack[ModalStackCount-1]->GetType()==MODALTYPE_EDITOR ||
        ModalStack[ModalStackCount-1]->GetType()==MODALTYPE_VIEWER)
    {
      CtrlObject->CmdLine->ShowBackground();
    }
    else
    {
      int UnlockCount=0;
      IsRedrawFramesInProcess++;

      while ((*this)[FramePos]->Locked())
      {
        (*this)[FramePos]->Unlock();
        UnlockCount++;
      }
      RefreshFrame((*this)[FramePos]);

      Commit();
      for (int i=0;i<UnlockCount;i++)
      {
        (*this)[FramePos]->Lock();
      }

      if (ModalStackCount>1)
      {
        for (int i=0;i<ModalStackCount-1;i++)
        {
          if (!(ModalStack[i]->FastHide() & CASR_HELP))
          {
            RefreshFrame(ModalStack[i]);
            Commit();
          }
          else
          {
            break;
          }
        }
      }
      /* $ 04.04.2002 KM
         ���������� ��������� ������ � ��������� ������.
         ���� �� ������������� ��������� ��������� �������
         ��� ����������� ���� �������.
      */
      IsRedrawFramesInProcess--;
      CurrentFrame->ShowConsoleTitle();
    }
  }
  else
  {
    CtrlObject->CmdLine->ShowBackground();
  }
}

void Manager::ModalizeCommit()
{
  CurrentFrame->Push(ModalizedFrame);
  ModalizedFrame=NULL;
}

void Manager::UnmodalizeCommit()
{
  int i;
  Frame *iFrame;
  for (i=0;i<FrameCount;i++)
  {
    iFrame=FrameList[i];
    if(iFrame->RemoveModal(UnmodalizedFrame))
    {
      break;
    }
  }

  for (i=0;i<ModalStackCount;i++)
  {
    iFrame=ModalStack[i];
    if(iFrame->RemoveModal(UnmodalizedFrame))
    {
      break;
    }
  }
  UnmodalizedFrame=NULL;
}

BOOL Manager::ifDoubleInstance(Frame *frame)
{
  // <ifDoubleInstance>
/*
  if (ModalStackCount<=0)
    return FALSE;
  if(IndexOfStack(frame)==-1)
    return FALSE;
  if(IndexOf(frame)!=-1)
    return TRUE;
*/
  // </ifDoubleInstance>
  return FALSE;
}

/*  ����� ResizeConsole ��� ���� NextModal �
    ���������� ������. KM
*/
void Manager::ResizeAllModal(Frame *ModalFrame)
{
  if (!ModalFrame->NextModal)
    return;

  Frame *iModal=ModalFrame->NextModal;
  while (iModal)
  {
    iModal->ResizeConsole();
    iModal=iModal->NextModal;
  }
}

void Manager::ResizeAllFrame()
{
  int I;
  for (I=0; I < FrameCount; I++)
  {
    FrameList[I]->ResizeConsole();
  }

  for (I=0; I < ModalStackCount; I++)
  {
    ModalStack[I]->ResizeConsole();
    /* $ 13.04.2002 KM
      - � ������ ����������� ��� NextModal...
    */
    ResizeAllModal(ModalStack[I]);
  }
  ImmediateHide();
  FrameManager->RefreshFrame();
  //RefreshFrame();
}

void Manager::InitKeyBar(void)
{
  for (int I=0;I < FrameCount;I++)
    FrameList[I]->InitKeyBar();
}

/*void Manager::AddSemiModalBackFrame(Frame* frame)
{
  if(SemiModalBackFramesCount>=SemiModalBackFramesSize)
  {
    SemiModalBackFramesSize+=4;
    SemiModalBackFrames=
      (Frame**)xf_realloc(SemiModalBackFrames,sizeof(Frame*)*SemiModalBackFramesSize);

  }
  SemiModalBackFrames[SemiModalBackFramesCount]=frame;
  SemiModalBackFramesCount++;
}

BOOL Manager::IsSemiModalBackFrame(Frame *frame)
{
  if(!SemiModalBackFrames)return FALSE;
  for(int i=0;i<SemiModalBackFramesCount;i++)
  {
    if(SemiModalBackFrames[i]==frame)return TRUE;
  }
  return FALSE;
}

void Manager::RemoveSemiModalBackFrame(Frame* frame)
{
  if(!SemiModalBackFrames)return;
  for(int i=0;i<SemiModalBackFramesCount;i++)
  {
    if(SemiModalBackFrames[i]==frame)
    {
      for(int j=i+1;j<SemiModalBackFramesCount;j++)
      {
        SemiModalBackFrames[j-1]=SemiModalBackFrames[j];
      }
      SemiModalBackFramesCount--;
      return;
    }
  }
}
*/

    // ���������� top-����� ��� ��� �����, ���� � ������ ���� �������
Frame* Manager::GetTopModal()
{
  Frame *f=CurrentFrame, *fo=NULL;
  while(f)
  {
    fo=f;
    f=f->GetTopModal();
  }
  if(!f)
    f=fo;
  return f;
}
