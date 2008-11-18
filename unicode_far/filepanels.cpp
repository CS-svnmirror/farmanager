/*
filepanels.cpp

�������� ������
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

#include "filepanels.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "lang.hpp"
#include "plugin.hpp"
#include "ctrlobj.hpp"
#include "filelist.hpp"
#include "rdrwdsk.hpp"
#include "cmdline.hpp"
#include "treelist.hpp"
#include "qview.hpp"
#include "infolist.hpp"
#include "help.hpp"
#include "filefilter.hpp"
#include "findfile.hpp"
#include "savescr.hpp"
#include "manager.hpp"

FilePanels::FilePanels()
{
  _OT(SysLog(L"[%p] FilePanels::FilePanels()", this));
  LeftPanel=CreatePanel(Opt.LeftPanel.Type);
  RightPanel=CreatePanel(Opt.RightPanel.Type);
//  CmdLine=0;
  ActivePanel=0;
  LastLeftType=0;
  LastRightType=0;
//  HideState=0;
  LeftStateBeforeHide=0;
  RightStateBeforeHide=0;
  LastLeftFilePanel=0;
  LastRightFilePanel=0;
  MacroMode = MACRO_SHELL;
  KeyBarVisible = Opt.ShowKeyBar;
//  SetKeyBar(&MainKeyBar);
//  _D(SysLog(L"MainKeyBar=0x%p",&MainKeyBar));
}

static void PrepareOptFolderW(string &strSrc, int IsLocalPath_FarPath)
{
  if ( strSrc.IsEmpty() )
  {
    strSrc = g_strFarPath;
    DeleteEndSlash(strSrc);
  }
  else
    apiExpandEnvironmentStrings(strSrc, strSrc);

  if(!StrCmp(strSrc,L"/"))
  {
    strSrc = g_strFarPath;

    wchar_t *lpwszSrc = strSrc.GetBuffer ();

    if(IsLocalPath_FarPath)
    {
      lpwszSrc[2]='\\';
      lpwszSrc[3]=0;
    }

    strSrc.ReleaseBuffer ();
  }
  else
    CheckShortcutFolder(&strSrc,FALSE,TRUE);
}

void FilePanels::Init()
{
  SetPanelPositions(FileList::IsModeFullScreen(Opt.LeftPanel.ViewMode),
                    FileList::IsModeFullScreen(Opt.RightPanel.ViewMode));
  LeftPanel->SetViewMode(Opt.LeftPanel.ViewMode);
  RightPanel->SetViewMode(Opt.RightPanel.ViewMode);
  LeftPanel->SetSortMode(Opt.LeftPanel.SortMode);
  RightPanel->SetSortMode(Opt.RightPanel.SortMode);
  LeftPanel->SetNumericSort(Opt.LeftPanel.NumericSort);
  RightPanel->SetNumericSort(Opt.RightPanel.NumericSort);
  LeftPanel->SetSortOrder(Opt.LeftPanel.SortOrder);
  RightPanel->SetSortOrder(Opt.RightPanel.SortOrder);
  LeftPanel->SetSortGroups(Opt.LeftPanel.SortGroups);
  RightPanel->SetSortGroups(Opt.RightPanel.SortGroups);
  LeftPanel->SetShowShortNamesMode(Opt.LeftPanel.ShowShortNames);
  RightPanel->SetShowShortNamesMode(Opt.RightPanel.ShowShortNames);
  LeftPanel->SetSelectedFirstMode(Opt.LeftSelectedFirst);
  RightPanel->SetSelectedFirstMode(Opt.RightSelectedFirst);
  SetCanLoseFocus(TRUE);

  Panel *PassivePanel=NULL;
  int PassiveIsLeftFlag=TRUE;

  if (Opt.LeftPanel.Focus)
  {
    ActivePanel=LeftPanel;
    PassivePanel=RightPanel;
    PassiveIsLeftFlag=FALSE;
  }
  else
  {
    ActivePanel=RightPanel;
    PassivePanel=LeftPanel;
    PassiveIsLeftFlag=TRUE;
  }
  ActivePanel->SetFocus();

  // �������� ��������� �� ��������� ��� �������
  int IsLocalPath_FarPath=IsLocalPath(g_strFarPath);
  PrepareOptFolderW(Opt.strLeftFolder,IsLocalPath_FarPath);
  PrepareOptFolderW(Opt.strRightFolder,IsLocalPath_FarPath);
  PrepareOptFolderW(Opt.strPassiveFolder,IsLocalPath_FarPath);

  if (Opt.AutoSaveSetup || !Opt.SetupArgv)
  {
    if (GetFileAttributesW(Opt.strLeftFolder)!=INVALID_FILE_ATTRIBUTES)
      LeftPanel->InitCurDir(Opt.strLeftFolder);
    if (GetFileAttributesW(Opt.strRightFolder)!=INVALID_FILE_ATTRIBUTES)
      RightPanel->InitCurDir(Opt.strRightFolder);
  }

  if (!Opt.AutoSaveSetup)
  {
    if(Opt.SetupArgv >= 1)
    {
      if(ActivePanel==RightPanel)
      {
        if (GetFileAttributesW(Opt.strRightFolder)!=INVALID_FILE_ATTRIBUTES)
          RightPanel->InitCurDir(Opt.strRightFolder);
      }
      else
      {
        if (GetFileAttributesW(Opt.strLeftFolder)!=INVALID_FILE_ATTRIBUTES)
          LeftPanel->InitCurDir(Opt.strLeftFolder);
      }
      if(Opt.SetupArgv == 2)
      {
        if(ActivePanel==LeftPanel)
        {
          if (GetFileAttributesW(Opt.strRightFolder)!=INVALID_FILE_ATTRIBUTES)
            RightPanel->InitCurDir(Opt.strRightFolder);
        }
        else
        {
          if (GetFileAttributesW(Opt.strLeftFolder)!=INVALID_FILE_ATTRIBUTES)
            LeftPanel->InitCurDir(Opt.strLeftFolder);
        }
      }
    }
    if (Opt.SetupArgv < 2 && !Opt.strPassiveFolder.IsEmpty() && (GetFileAttributesW(Opt.strPassiveFolder)!=INVALID_FILE_ATTRIBUTES))
    {
      PassivePanel->InitCurDir(Opt.strPassiveFolder);
    }
  }
#if 1
  //! ������� "����������" ��������� ������
  if(PassiveIsLeftFlag)
  {
    if (Opt.LeftPanel.Visible)
    {
      LeftPanel->Show();
    }
    if (Opt.RightPanel.Visible)
    {
      RightPanel->Show();
    }
  }
  else
  {
    if (Opt.RightPanel.Visible)
    {
      RightPanel->Show();
    }
    if (Opt.LeftPanel.Visible)
    {
      LeftPanel->Show();
    }
  }
#endif
  // ��� ���������� ������� �� ������ �� ��������� ��������� ������� � CmdLine
  if (!Opt.RightPanel.Visible && !Opt.LeftPanel.Visible)
  {
    CtrlObject->CmdLine->SetCurDir(PassiveIsLeftFlag?Opt.strRightFolder:Opt.strLeftFolder);
  }

  SetKeyBar(&MainKeyBar);
  MainKeyBar.SetOwner(this);
}

FilePanels::~FilePanels()
{
  _OT(SysLog(L"[%p] FilePanels::~FilePanels()", this));
  if (LastLeftFilePanel!=LeftPanel && LastLeftFilePanel!=RightPanel)
    DeletePanel(LastLeftFilePanel);
  if (LastRightFilePanel!=LeftPanel && LastRightFilePanel!=RightPanel)
    DeletePanel(LastRightFilePanel);
  DeletePanel(LeftPanel);
  LeftPanel=NULL;
  DeletePanel(RightPanel);
  RightPanel=NULL;
}

void FilePanels::SetPanelPositions(int LeftFullScreen,int RightFullScreen)
{
  if (Opt.WidthDecrement < -(ScrX/2-10))
    Opt.WidthDecrement=-(ScrX/2-10);
  if (Opt.WidthDecrement > (ScrX/2-10))
    Opt.WidthDecrement=(ScrX/2-10);
  if (Opt.HeightDecrement>ScrY-7)
    Opt.HeightDecrement=ScrY-7;
  if (Opt.HeightDecrement<0)
    Opt.HeightDecrement=0;
  if (LeftFullScreen){
    LeftPanel->SetPosition(0,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
    LeftPanel->ViewSettings.FullScreen=1;
  } else {
    LeftPanel->SetPosition(0,Opt.ShowMenuBar,ScrX/2-Opt.WidthDecrement,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  }
  if (RightFullScreen) {
    RightPanel->SetPosition(0,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
    RightPanel->ViewSettings.FullScreen=1;
  } else {
    RightPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Opt.ShowMenuBar,ScrX,ScrY-1-(Opt.ShowKeyBar!=0)-Opt.HeightDecrement);
  }
}

void FilePanels::SetScreenPosition()
{
  _OT(SysLog(L"[%p] FilePanels::SetScreenPosition() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
//  RedrawDesktop Redraw;
  CtrlObject->CmdLine->SetPosition(0,ScrY-(Opt.ShowKeyBar!=0),ScrX,ScrY-(Opt.ShowKeyBar!=0));
  TopMenuBar.SetPosition(0,0,ScrX,0);
  MainKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
  SetPanelPositions(LeftPanel->IsFullScreen(),RightPanel->IsFullScreen());
  SetPosition(0,0,ScrX,ScrY);

}

void FilePanels::RedrawKeyBar()
{
  ActivePanel->UpdateKeyBar();
  MainKeyBar.Redraw();
}


Panel* FilePanels::CreatePanel(int Type)
{
  Panel *pResult = NULL;

  switch (Type)
  {
    case FILE_PANEL:
      pResult = new FileList;
      break;

    case TREE_PANEL:
      pResult = new TreeList;
      break;

    case QVIEW_PANEL:
      pResult = new QuickView;
      break;

    case INFO_PANEL:
      pResult = new InfoList;
      break;
  }

  if ( pResult )
    pResult->SetOwner (this);

  return pResult;
}


void FilePanels::DeletePanel(Panel *Deleted)
{
  if (Deleted==NULL)
    return;
  if (Deleted==LastLeftFilePanel)
    LastLeftFilePanel=NULL;
  if (Deleted==LastRightFilePanel)
    LastRightFilePanel=NULL;
  delete Deleted;
}

int FilePanels::SetAnhoterPanelFocus(void)
{
  int Ret=FALSE;
  if (ActivePanel==LeftPanel)
  {
    if (RightPanel->IsVisible())
    {
      RightPanel->SetFocus();
      Ret=TRUE;
    }
  }
  else
  {
    if (LeftPanel->IsVisible())
    {
      LeftPanel->SetFocus();
      Ret=TRUE;
    }
  }
  return Ret;
}


int FilePanels::SwapPanels(void)
{
  int Ret=FALSE; // ��� ������ �� ���� �� ������� �� �����

  if (LeftPanel->IsVisible() || RightPanel->IsVisible())
  {
    int XL1,YL1,XL2,YL2;
    int XR1,YR1,XR2,YR2;

    LeftPanel->GetPosition(XL1,YL1,XL2,YL2);
    RightPanel->GetPosition(XR1,YR1,XR2,YR2);
    if (!LeftPanel->ViewSettings.FullScreen || !RightPanel->ViewSettings.FullScreen)
    {
      Opt.WidthDecrement=-Opt.WidthDecrement;
      if (!LeftPanel->ViewSettings.FullScreen){
        LeftPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
        if (LastLeftFilePanel)
          LastLeftFilePanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,YR1,ScrX,YR2);
      }
      if(!RightPanel->ViewSettings.FullScreen){
        RightPanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);
        if (LastRightFilePanel)
          LastRightFilePanel->SetPosition(0,YL1,ScrX/2-Opt.WidthDecrement,YL2);
      }
    }

    Panel *Swap;
    int SwapType;

    Swap=LeftPanel;
    LeftPanel=RightPanel;
    RightPanel=Swap;
    Swap=LastLeftFilePanel;
    LastLeftFilePanel=LastRightFilePanel;
    LastRightFilePanel=Swap;
    SwapType=LastLeftType;
    LastLeftType=LastRightType;
    LastRightType=SwapType;
    FileFilter::SwapFilter();

    Ret=TRUE;
  }
  FrameManager->RefreshFrame();
  return Ret;
}

__int64 FilePanels::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
  return ActivePanel->VMProcess(OpCode,vParam,iParam);
}

int FilePanels::ProcessKey(int Key)
{
  if (!Key)
    return(TRUE);

  if ((Key==KEY_CTRLLEFT || Key==KEY_CTRLRIGHT || Key==KEY_CTRLNUMPAD4 || Key==KEY_CTRLNUMPAD6
      /* || Key==KEY_CTRLUP   || Key==KEY_CTRLDOWN || Key==KEY_CTRLNUMPAD8 || Key==KEY_CTRLNUMPAD2 */) &&
      (CtrlObject->CmdLine->GetLength()>0 ||
      (!LeftPanel->IsVisible() && !RightPanel->IsVisible())))
  {
    CtrlObject->CmdLine->ProcessKey(Key);
    return(TRUE);
  }

  switch(Key)
  {
    case KEY_F1:
    {
      if (!ActivePanel->ProcessKey(KEY_F1))
      {
        Help Hlp (L"Contents");
      }
      return(TRUE);
    }

    case KEY_TAB:
    {
      SetAnhoterPanelFocus();
      break;
    }

    case KEY_CTRLF1:
    {
      if (LeftPanel->IsVisible())
      {
        LeftPanel->Hide();
        if (RightPanel->IsVisible())
          RightPanel->SetFocus();
      }
      else
      {
        if (!RightPanel->IsVisible())
          LeftPanel->SetFocus();
        LeftPanel->Show();
      }
      Redraw();
      break;
    }

    case KEY_CTRLF2:
    {
      if (RightPanel->IsVisible())
      {
        RightPanel->Hide();
        if (LeftPanel->IsVisible())
          LeftPanel->SetFocus();
      }
      else
      {
        if (!LeftPanel->IsVisible())
          RightPanel->SetFocus();
        RightPanel->Show();
      }
      Redraw();
      break;
    }

    case KEY_CTRLB:
    {
      Opt.ShowKeyBar=!Opt.ShowKeyBar;
      KeyBarVisible = Opt.ShowKeyBar;
      if(!KeyBarVisible)
        MainKeyBar.Hide();
      SetScreenPosition();
      FrameManager->RefreshFrame();
      break;
    }

    case KEY_CTRLL:
    case KEY_CTRLQ:
    case KEY_CTRLT:
    {
      if (ActivePanel->IsVisible())
      {
        Panel *AnotherPanel=GetAnotherPanel(ActivePanel);
        int NewType;
        if (Key==KEY_CTRLL)
          NewType=INFO_PANEL;
        else
          if (Key==KEY_CTRLQ)
            NewType=QVIEW_PANEL;
          else
            NewType=TREE_PANEL;

        if (ActivePanel->GetType()==NewType)
          AnotherPanel=ActivePanel;

        if (!AnotherPanel->ProcessPluginEvent(FE_CLOSE,NULL))
        {
          if (AnotherPanel->GetType()==NewType)
          /* $ 19.09.2000 IS
            ��������� ������� �� ctrl-l|q|t ������ �������� �������� ������
          */
            AnotherPanel=ChangePanel(AnotherPanel,FILE_PANEL,FALSE,FALSE);
          else
            AnotherPanel=ChangePanel(AnotherPanel,NewType,FALSE,FALSE);

          /* $ 07.09.2001 VVM
            ! ��� �������� �� CTRL+Q, CTRL+L ����������� �������, ���� �������� ������ - ������. */
          if (ActivePanel->GetType() == TREE_PANEL)
          {
            string strCurDir;
            ActivePanel->GetCurDir(strCurDir);
            AnotherPanel->SetCurDir(strCurDir, TRUE);
            AnotherPanel->Update(0);
          }
          else
            AnotherPanel->Update(UPDATE_KEEP_SELECTION);
          AnotherPanel->Show();
        }
        ActivePanel->SetFocus();
      }
      break;
    }

    case KEY_CTRLO:
    {
      {
        int LeftVisible=LeftPanel->IsVisible();
        int RightVisible=RightPanel->IsVisible();
        int HideState=!LeftVisible && !RightVisible;
        if (!HideState)
        {
          LeftStateBeforeHide=LeftVisible;
          RightStateBeforeHide=RightVisible;
          LeftPanel->Hide();
          RightPanel->Hide();
          FrameManager->RefreshFrame();
        }
        else
        {
          if (!LeftStateBeforeHide && !RightStateBeforeHide)
            LeftStateBeforeHide=RightStateBeforeHide=TRUE;
          if (LeftStateBeforeHide)
            LeftPanel->Show();
          if (RightStateBeforeHide)
            RightPanel->Show();
          if (!ActivePanel->IsVisible())
          {
            if(ActivePanel == RightPanel)
              LeftPanel->SetFocus();
            else
              RightPanel->SetFocus();
          }
        }
      }
      break;
    }

    case KEY_CTRLP:
    {
      if (ActivePanel->IsVisible())
      {
        Panel *AnotherPanel=GetAnotherPanel(ActivePanel);
        if (AnotherPanel->IsVisible())
          AnotherPanel->Hide();
        else
          AnotherPanel->Show();
        CtrlObject->CmdLine->Redraw();
      }
      FrameManager->RefreshFrame();
      break;
    }

    case KEY_CTRLI:
    {
      ActivePanel->EditFilter();
      return(TRUE);
    }

    case KEY_CTRLU:
    {
      SwapPanels();
      break;
    }

    /* $ 08.04.2002 IS
       ��� ����� ����� ��������� ������������� ������� ������� �� ��������
       ������, �.�. ������� �� ����� ������ � ���, ��� � ���� ��� ������, �
       ������� ��� ������� ����� ����� ����� ����� ���� ������� � �� ���������
       ������
    */
    case KEY_ALTF1:
    {
      LeftPanel->ChangeDisk();
      if(ActivePanel!=LeftPanel)
        ActivePanel->SetCurPath();
      break;
    }

    case KEY_ALTF2:
    {
      RightPanel->ChangeDisk();
      if(ActivePanel!=RightPanel)
        ActivePanel->SetCurPath();
      break;
    }

    case KEY_ALTF7:
    {
      {
        FindFiles FindFiles;
      }
      break;
    }

    case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
    {
      if (Opt.HeightDecrement<ScrY-7)
      {
        Opt.HeightDecrement++;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    }

    case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
    {
      if (Opt.HeightDecrement>0)
      {
        Opt.HeightDecrement--;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    }

    case KEY_CTRLLEFT: case KEY_CTRLNUMPAD4:
    {
      if (Opt.WidthDecrement<ScrX/2-10)
      {
        Opt.WidthDecrement++;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    }

    case KEY_CTRLRIGHT: case KEY_CTRLNUMPAD6:
    {
      if (Opt.WidthDecrement>-(ScrX/2-10))
      {
        Opt.WidthDecrement--;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    }

    case KEY_CTRLCLEAR:
    {
      if (Opt.WidthDecrement!=0)
      {
        Opt.WidthDecrement=0;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    }

    case KEY_CTRLSHIFTCLEAR:
    {
      if (Opt.HeightDecrement!=0)
      {
        Opt.HeightDecrement=0;
        SetScreenPosition();
        FrameManager->RefreshFrame();
      }
      break;
    }

    case KEY_F9:
    {
      ShellOptions(0,NULL);
      return(TRUE);
    }

    case KEY_SHIFTF10:
    {
      ShellOptions(1,NULL);
      return(TRUE);
    }

    default:
    {
      if(Key >= KEY_CTRL0 && Key <= KEY_CTRL9)
        ChangePanelViewMode(ActivePanel,Key-KEY_CTRL0,TRUE);
      else if (!ActivePanel->ProcessKey(Key))
        CtrlObject->CmdLine->ProcessKey(Key);
      break;
    }
  }

  return(TRUE);
}

int FilePanels::ChangePanelViewMode(Panel *Current,int Mode,BOOL RefreshFrame)
{
  if(Current && Mode >= VIEW_0 && Mode <= VIEW_9)
  {
    Current->SetViewMode(Mode);
    Current=ChangePanelToFilled(Current,FILE_PANEL);
    Current->SetViewMode(Mode);
    // ��������! �������! �� ��������!
    SetScreenPosition();
    if(RefreshFrame)
      FrameManager->RefreshFrame();

    return TRUE;
  }
  return FALSE;
}

Panel* FilePanels::ChangePanelToFilled(Panel *Current,int NewType)
{
  if (Current->GetType()!=NewType && !Current->ProcessPluginEvent(FE_CLOSE,NULL))
  {
    Current->Hide();
    Current=ChangePanel(Current,NewType,FALSE,FALSE);
    Current->Update(0);
    Current->Show();
    if (!GetAnotherPanel(Current)->GetFocus())
      Current->SetFocus();
  }
  return(Current);
}

Panel* FilePanels::GetAnotherPanel(Panel *Current)
{
  if (Current==LeftPanel)
    return(RightPanel);
  else
    return(LeftPanel);
}


Panel* FilePanels::ChangePanel(Panel *Current,int NewType,int CreateNew,int Force)
{
  Panel *NewPanel;
  SaveScreen *SaveScr=NULL;
  // OldType �� �����������������...
  int OldType=Current->GetType(),X1,Y1,X2,Y2;
  int OldViewMode,OldSortMode,OldSortOrder,OldSortGroups,OldSelectedFirst;
  int OldShowShortNames,OldPanelMode,LeftPosition,ChangePosition,OldNumericSort;
  int OldFullScreen,OldFocus,UseLastPanel=0;

  OldPanelMode=Current->GetMode();
  if (!Force && NewType==OldType && OldPanelMode==NORMAL_PANEL)
    return(Current);
  OldViewMode=Current->GetPrevViewMode();
  OldFullScreen=Current->IsFullScreen();

  OldSortMode=Current->GetPrevSortMode();
  OldSortOrder=Current->GetPrevSortOrder();
  OldNumericSort=Current->GetPrevNumericSort();
  OldSortGroups=Current->GetSortGroups();
  OldShowShortNames=Current->GetShowShortNamesMode();
  OldFocus=Current->GetFocus();

  OldSelectedFirst=Current->GetSelectedFirstMode();

  LeftPosition=(Current==LeftPanel);
  Panel *(&LastFilePanel)=LeftPosition ? LastLeftFilePanel:LastRightFilePanel;

  Current->GetPosition(X1,Y1,X2,Y2);

  ChangePosition=((OldType==FILE_PANEL && NewType!=FILE_PANEL &&
             OldFullScreen) || (NewType==FILE_PANEL &&
             ((OldFullScreen && !FileList::IsModeFullScreen(OldViewMode)) ||
             (!OldFullScreen && FileList::IsModeFullScreen(OldViewMode)))));

  if (!ChangePosition)
  {
    SaveScr=Current->SaveScr;
    Current->SaveScr=NULL;
  }

  if (OldType==FILE_PANEL && NewType!=FILE_PANEL)
  {
    delete Current->SaveScr;
    Current->SaveScr=NULL;
    if (LastFilePanel!=Current)
    {
      DeletePanel(LastFilePanel);
      LastFilePanel=Current;
    }
    LastFilePanel->Hide();
    if (LastFilePanel->SaveScr)
    {
      LastFilePanel->SaveScr->Discard();
      delete LastFilePanel->SaveScr;
      LastFilePanel->SaveScr=NULL;
    }
  }
  else
  {
    Current->Hide();
    DeletePanel(Current);
    if (OldType==FILE_PANEL && NewType==FILE_PANEL)
    {
      DeletePanel(LastFilePanel);
      LastFilePanel=NULL;
    }
  }

  if (!CreateNew && NewType==FILE_PANEL && LastFilePanel!=NULL)
  {
    int LastX1,LastY1,LastX2,LastY2;
    LastFilePanel->GetPosition(LastX1,LastY1,LastX2,LastY2);
    if (LastFilePanel->IsFullScreen())
      LastFilePanel->SetPosition(LastX1,Y1,LastX2,Y2);
    else
      LastFilePanel->SetPosition(X1,Y1,X2,Y2);
    NewPanel=LastFilePanel;
    if (!ChangePosition)
    {
      if ((NewPanel->IsFullScreen() && !OldFullScreen) ||
          (!NewPanel->IsFullScreen() && OldFullScreen))
      {
        Panel *AnotherPanel=GetAnotherPanel(Current);
        if (SaveScr!=NULL && AnotherPanel->IsVisible() &&
            AnotherPanel->GetType()==FILE_PANEL && AnotherPanel->IsFullScreen())
          SaveScr->Discard();
        delete SaveScr;
      }
      else
        NewPanel->SaveScr=SaveScr;
    }
    if (!OldFocus && NewPanel->GetFocus())
      NewPanel->KillFocus();
    UseLastPanel=TRUE;
  }
  else
    NewPanel=CreatePanel(NewType);

  if (Current==ActivePanel)
    ActivePanel=NewPanel;
  if (LeftPosition)
  {
    LeftPanel=NewPanel;
    LastLeftType=OldType;
  }
  else
  {
    RightPanel=NewPanel;
    LastRightType=OldType;
  }
  if (!UseLastPanel)
  {
    if (ChangePosition)
    {
      if (LeftPosition)
      {
        NewPanel->SetPosition(0,Y1,ScrX/2-Opt.WidthDecrement,Y2);
        RightPanel->Redraw();
      }
      else
      {
        NewPanel->SetPosition(ScrX/2+1-Opt.WidthDecrement,Y1,ScrX,Y2);
        LeftPanel->Redraw();
      }
    }
    else
    {
      NewPanel->SaveScr=SaveScr;
      NewPanel->SetPosition(X1,Y1,X2,Y2);
    }

    NewPanel->SetSortMode(OldSortMode);
    NewPanel->SetSortOrder(OldSortOrder);
    NewPanel->SetNumericSort(OldNumericSort);
    NewPanel->SetSortGroups(OldSortGroups);
    NewPanel->SetShowShortNamesMode(OldShowShortNames);
    NewPanel->SetPrevViewMode(OldViewMode);
    NewPanel->SetViewMode(OldViewMode);
    NewPanel->SetSelectedFirstMode(OldSelectedFirst);
  }
  return(NewPanel);
}

int  FilePanels::GetTypeAndName(string &strType, string &strName)
{
  strType = MSG(MScreensPanels);

  string strFullName, strShortName;

  switch (ActivePanel->GetType())
  {
    case TREE_PANEL:
    case QVIEW_PANEL:
    case FILE_PANEL:
    case INFO_PANEL:
        ActivePanel->GetCurName(strFullName, strShortName);
        ConvertNameToFull(strFullName, strFullName);
        break;
  }

  strName = strFullName;

  return(MODALTYPE_PANELS);
}

void FilePanels::OnChangeFocus(int f)
{
  _OT(SysLog(L"FilePanels::OnChangeFocus(%i)",f));
  /* $ 20.06.2001 tran
     ��� � ���������� ��� ����������� � ��������
     �� ���������� LockRefreshCount */
  if ( f ) {
    /*$ 22.06.2001 SKV
      + update ������� ��� ��������� ������
    */
    CtrlObject->Cp()->GetAnotherPanel(ActivePanel)->UpdateIfChanged(UIC_UPDATE_FORCE_NOTIFICATION);
    ActivePanel->UpdateIfChanged(UIC_UPDATE_FORCE_NOTIFICATION);
    /* $ 13.04.2002 KM
      ! ??? � �� ����� ����� ����� Redraw, ����
        Redraw ���������� ������ �� Frame::OnChangeFocus.
    */
//    Redraw();
    Frame::OnChangeFocus(1);
  }
}

void FilePanels::DisplayObject ()
{
//  if ( Focus==0 )
//      return;
  _OT(SysLog(L"[%p] FilePanels::Redraw() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
  CtrlObject->CmdLine->ShowBackground();

  if (Opt.ShowMenuBar)
    CtrlObject->TopMenuBar->Show();
  CtrlObject->CmdLine->Show();

  if (Opt.ShowKeyBar)
    MainKeyBar.Show();
  else
    if(MainKeyBar.IsVisible())
      MainKeyBar.Hide();
  KeyBarVisible=Opt.ShowKeyBar;
#if 1
  if (LeftPanel->IsVisible())
      LeftPanel->Show();
  if (RightPanel->IsVisible())
      RightPanel->Show();
#else
  Panel *PassivePanel=NULL;
  int PassiveIsLeftFlag=TRUE;

  if (Opt.LeftPanel.Focus)
  {
    ActivePanel=LeftPanel;
    PassivePanel=RightPanel;
    PassiveIsLeftFlag=FALSE;
  }
  else
  {
    ActivePanel=RightPanel;
    PassivePanel=LeftPanel;
    PassiveIsLeftFlag=TRUE;
  }

  //! ������� "����������" ��������� ������
  if(PassiveIsLeftFlag)
  {
    if (Opt.LeftPanel.Visible)
    {
      LeftPanel->Show();
    }
    if (Opt.RightPanel.Visible)
    {
      RightPanel->Show();
    }
  }
  else
  {
    if (Opt.RightPanel.Visible)
    {
      RightPanel->Show();
    }
    if (Opt.LeftPanel.Visible)
    {
      LeftPanel->Show();
    }
  }

#endif
}

int  FilePanels::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (!ActivePanel->ProcessMouse(MouseEvent))
    if (!GetAnotherPanel(ActivePanel)->ProcessMouse(MouseEvent))
      if (!MainKeyBar.ProcessMouse(MouseEvent))
        CtrlObject->CmdLine->ProcessMouse(MouseEvent);
  return(TRUE);
}

void FilePanels::ShowConsoleTitle()
{
  if (ActivePanel)
    ActivePanel->SetTitle();
}

void FilePanels::ResizeConsole()
{
  Frame::ResizeConsole();
  CtrlObject->CmdLine->ResizeConsole();
  MainKeyBar.ResizeConsole();
  TopMenuBar.ResizeConsole();
  SetScreenPosition();
  _OT(SysLog(L"[%p] FilePanels::ResizeConsole() {%d, %d - %d, %d}", this,X1,Y1,X2,Y2));
}

int FilePanels::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_PANEL;
}

void FilePanels::Refresh()
{
  /*$ 31.07.2001 SKV
    ������� ���, � �� Frame::OnChangeFocus,
    ������� �� ����� � ��������.
  */
  //Frame::OnChangeFocus(1);
  OnChangeFocus(1);
}

void FilePanels::GoToFile(const wchar_t *FileName)
{
  if(wcschr(FileName,'\\') || wcschr(FileName,'/'))
  {
    string ADir,PDir;
    Panel *PassivePanel = GetAnotherPanel(ActivePanel);
    int PassiveMode = PassivePanel->GetMode();
    if (PassiveMode == NORMAL_PANEL)
    {
      PassivePanel->GetCurDir(PDir);
      AddEndSlash(PDir);
    }

    int ActiveMode = ActivePanel->GetMode();
    if(ActiveMode==NORMAL_PANEL)
    {
      ActivePanel->GetCurDir(ADir);
      AddEndSlash(ADir);
    }

    string strNameFile = PointToName(FileName);
    string strNameDir = FileName;

    CutToSlash(strNameDir);

    /* $ 10.04.2001 IS
         �� ������ SetCurDir, ���� ������ ���� ��� ���� �� ��������
         �������, ��� ����� ���������� ����, ��� ��������� � ���������
         ������� �� ������������.
    */
    BOOL AExist=(ActiveMode==NORMAL_PANEL) && (StrCmpI(ADir,strNameDir)==0);
    BOOL PExist=(PassiveMode==NORMAL_PANEL) && (StrCmpI(PDir,strNameDir)==0);
    // ���� ������ ���� ���� �� ��������� ������
    if (!AExist && PExist)
      ProcessKey(KEY_TAB);
    if (!AExist && !PExist)
      ActivePanel->SetCurDir(strNameDir,TRUE);
    ActivePanel->GoToFile(strNameFile);
    // ������ ������� ��������� ������, ����� ���� �������� �����, ���
    // Ctrl-F10 ���������
    ActivePanel->SetTitle();
  }
}


int  FilePanels::GetMacroMode()
{
  switch (ActivePanel->GetType())
  {
    case TREE_PANEL:
      return MACRO_TREEPANEL;
    case QVIEW_PANEL:
      return MACRO_QVIEWPANEL;
    case INFO_PANEL:
      return MACRO_INFOPANEL;
    default:
      return MACRO_SHELL;
  }
}
