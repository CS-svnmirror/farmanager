/*
plugapi.cpp

API, ��������� �������� (�������, ����, ...)
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

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "struct.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "scantree.hpp"
#include "rdrwdsk.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "plugins.hpp"
#include "savescr.hpp"
#include "flink.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "frame.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"
#include "lockscrn.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"


void ScanPluginDir();


/* $ 07.12.2001 IS
   ������� ������ GetString ��� �������� - � ������� �����������������.
   ������� ��� ����, ����� �� ����������� ��� GetString.
*/
int WINAPI FarInputBox (
        const wchar_t *Title,
        const wchar_t *Prompt,
        const wchar_t *HistoryName,
        const wchar_t *SrcText,
        wchar_t *DestText,
        int DestLength,
        const wchar_t *HelpTopic,
        DWORD Flags
        )
{
  if (FrameManager->ManagerIsDown())
    return FALSE;

  string strDest;

  int nResult = GetString(Title,Prompt,HistoryName,SrcText,strDest,DestLength,
     HelpTopic,Flags&~FIB_CHECKBOX,NULL,NULL);

  xwcsncpy (DestText, strDest, DestLength);

  return nResult;
}

/* ������� ������ ������ */
BOOL WINAPI FarShowHelp (
		const wchar_t *ModuleName,
		const wchar_t *HelpTopic,
		DWORD Flags
		)
{
  if (FrameManager->ManagerIsDown())
    return FALSE;
  if (!HelpTopic)
    HelpTopic=L"Contents";

  DWORD OFlags=Flags;
  Flags&=~(FHELP_NOSHOWERROR|FHELP_USECONTENTS);
  string strPath, strTopic;
  string strMask;

  // ��������� � ������ ������ ���� �� ������������ � � ��� ������,
  // ���� ����� FHELP_FARHELP...
  if((Flags&FHELP_FARHELP) || *HelpTopic==L':')
    strTopic = HelpTopic+((*HelpTopic == L':')?1:0);
  else
  {
    if(ModuleName)
    {
      // FHELP_SELFHELP=0 - ���������� ������ ���-� ��� Info.ModuleName
      //                   � �������� ����� �� ����� ���������� �������
      /* $ 17.11.2000 SVS
         � �������� FHELP_SELFHELP ����� ����? ��������� - 0
         � ����� ����� ��������� ����, ��� ������� �� �������� :-(
      */
      if(Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE|FHELP_CUSTOMPATH)))
      {
        strPath = ModuleName;
        if(Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE)))
        {
          if(Flags&FHELP_CUSTOMFILE)
            strMask=PointToName(strPath);
          else
            strMask = L"";
          CutToSlash(strPath);
        }
      }
      else
        return FALSE;

      strTopic.Format (HelpFormatLink,(const wchar_t*)strPath,HelpTopic);
    }
    else
      return FALSE;
  }
  {
    Help Hlp (strTopic,strMask,OFlags);
    if(Hlp.GetError())
      return FALSE;
  }
  return TRUE;
}

/* $ 05.07.2000 IS
  �������, ������� ����� ����������� � � ���������, � � �������, �...
*/
INT_PTR WINAPI FarAdvControl(INT_PTR ModuleNumber, int Command, void *Param)
{
  struct Opt2Flags{
    int *Opt;
    DWORD Flags;
  };

  switch(Command)
  {
    case ACTL_GETFARVERSION:
    case ACTL_GETSYSWORDDIV:
    case ACTL_GETCOLOR:
    case ACTL_GETARRAYCOLOR:
    case ACTL_GETFARHWND:
    case ACTL_GETSYSTEMSETTINGS:
    case ACTL_GETPANELSETTINGS:
    case ACTL_GETINTERFACESETTINGS:
    case ACTL_GETCONFIRMATIONS:
    case ACTL_GETDESCSETTINGS:
    case ACTL_GETPOLICIES:
    case ACTL_GETPLUGINMAXREADDATA:
    case ACTL_GETMEDIATYPE:
      break;
    default:
     if (FrameManager && FrameManager->ManagerIsDown())
       return 0;
  }

  switch(Command)
  {
    case ACTL_GETFARVERSION:
    {
      if(Param)
        *(DWORD*)Param=FAR_VERSION;
      return FAR_VERSION;
    }

    /* $ 25.07.2000 SVS
       + ���������� ������������ FulScreen <-> Windowed (ACTL_CONSOLEMODE)
       mode = -2 - �������� ������� ���������
              -1 - ��� ������
               0 - Windowed
               1 - FulScreen
       Return
               0 - Windowed
               1 - FulScreen
    */
    case ACTL_CONSOLEMODE:
    {
      return FarAltEnter((int)(INT_PTR)Param);
    }

    case ACTL_GETPLUGINMAXREADDATA:
    {
      return Opt.PluginMaxReadData;
    }

    case ACTL_GETWCHARMODE:
    {
		//BUGBUG!!!
      return 0;//Opt.UseUnicodeConsole;
    }

    /* $ 03.08.2000 SVS
       ��������� ������ � ������������� ����
       ���������� ������ ���������� ������ ��� '\0'
       ������������ ������ ��������� ������ = 80 � �������������� '\0'
       ������ ���������� �� �� �������, � �� Opt.
    */
    case ACTL_GETSYSWORDDIV:
    {
      int LenWordDiv=(int)Opt.strWordDiv.GetLength ();
      /* $ 09.08.2000 tran
       + if param==NULL, plugin ����� ������ ������ ����� ������  */
      if (Param && !IsBadWritePtr(Param,LenWordDiv+1))
        wcscpy((wchar_t *)Param,Opt.strWordDiv);
      return LenWordDiv;
    }

    /* $ 24.08.2000 SVS
       ������� ������������ (��� �����) �������
       (int)Param - ���������� ��� �������, ������� �������, ��� -1
       ���� ��� ����� ����� ������� �����.
       ���������� 0;
    */
    case ACTL_WAITKEY:
    {
      return WaitKey(Param?(DWORD)(DWORD_PTR)Param:(DWORD)-1);
    }

    /* $ 04.12.2000 SVS
      ACTL_GETCOLOR - �������� ������������ ���� �� ������, �������������
       � farcolor.hpp
      (int)Param - ������.
      Return - �������� ����� ��� -1 ���� ������ �������.
    */
    case ACTL_GETCOLOR:
    {
      if((int)(INT_PTR)Param < SizeArrayPalette && (int)(INT_PTR)Param >= 0)
        return (int)((unsigned int)Palette[(int)(INT_PTR)Param]);
      return -1;
    }

    /* $ 04.12.2000 SVS
      ACTL_GETARRAYCOLOR - �������� ���� ������ ������
      Param - ��������� �� ������ ��� NULL - ����� �������� ������ ������
      Return - ������ �������.
    */
    case ACTL_GETARRAYCOLOR:
    {
      if(Param && !IsBadWritePtr(Param,SizeArrayPalette))
        memmove(Param,Palette,SizeArrayPalette);
      return SizeArrayPalette;
    }

    /*
      Param=struct FARColor{
        DWORD Flags;
        int StartIndex;
        int ColorItem;
        LPBYTE Colors;
      };
    */
    case ACTL_SETARRAYCOLOR:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(struct FarSetColors)))
      {
        struct FarSetColors *Pal=(struct FarSetColors*)Param;
        if(Pal->Colors &&
           Pal->StartIndex >= 0 &&
           Pal->StartIndex+Pal->ColorCount <= SizeArrayPalette &&
           !IsBadReadPtr(Pal->Colors,Pal->ColorCount)
          )
        {
          memmove(Palette+Pal->StartIndex,Pal->Colors,Pal->ColorCount);
          if(Pal->Flags&FCLR_REDRAW)
          {
            ScrBuf.Lock(); // �������� ������ ����������
            FrameManager->ResizeAllFrame();
            FrameManager->PluginCommit(); // ��������.
            ScrBuf.Unlock(); // ��������� ����������
          }
          return TRUE;
        }
      }
      return FALSE;
    }

    /* $ 14.12.2000 SVS
      ACTL_EJECTMEDIA - ������� ���� �� �������� ����������
      Param - ��������� �� ��������� ActlEjectMedia
      Return - TRUE - �������� ����������, FALSE - ������.
    */
    case ACTL_EJECTMEDIA:
    {
      return Param?EjectVolume((wchar_t)((ActlEjectMedia*)Param)->Letter,
                               ((ActlEjectMedia*)Param)->Flags):FALSE;
/*
      if(Param)
      {
        struct ActlEjectMedia *aem=(struct ActlEjectMedia *)Param;
        char DiskLetter[4]=" :\\";
        DiskLetter[0]=(char)aem->Letter;
        int DriveType = FAR_GetDriveType(DiskLetter,NULL,FALSE); // ����� �� ���������� ��� CD

        if(DriveType == DRIVE_USBDRIVE && RemoveUSBDrive((char)aem->Letter,aem->Flags))
          return TRUE;
        if(DriveType == DRIVE_SUBSTITUTE && DelSubstDrive(DiskLetter))
          return TRUE;
        if(IsDriveTypeCDROM(DriveType) && EjectVolume((char)aem->Letter,aem->Flags))
          return TRUE;

      }
      return FALSE;
*/
    }
/*
    case ACTL_GETMEDIATYPE:
    {
      struct ActlMediaType *amt=(struct ActlMediaType *)Param;
      char DiskLetter[4]=" :\\";
      DiskLetter[0]=(amt)?(char)amt->Letter:0;
      return FAR_GetDriveType(DiskLetter,NULL,(amt && !(amt->Flags&MEDIATYPE_NODETECTCDROM)?TRUE:FALSE));
    }
*/
    /* $ 21.12.2000 SVS
       Macro API
    */
    case ACTL_KEYMACRO:
    {
      if(CtrlObject && Param) // ��� ������� �� ���� ������.
      {
        KeyMacro& Macro=CtrlObject->Macro; //??
        struct ActlKeyMacro *KeyMacro=(struct ActlKeyMacro*)Param;
        switch(KeyMacro->Command)
        {
          case MCMD_LOADALL: // �� ������� � ������ ��� � ���������� �����������
          {
            if(Macro.IsRecording())
              return FALSE;
            return Macro.LoadMacros(!Macro.IsExecuting());
          }

          case MCMD_SAVEALL: // �� ������ ���� � �������
          {
            if(Macro.IsRecording()) // || Macro.IsExecuting())
              return FALSE;
            Macro.SaveMacros();
            return TRUE;
          }

          case MCMD_POSTMACROSTRING:
          {
            return Macro.PostNewMacro(KeyMacro->Param.PlainText.SequenceText,KeyMacro->Param.PlainText.Flags<<8);
          }

          case MCMD_CHECKMACRO:  // �������� �������
          {
            struct MacroRecord CurMacro={0};
            int Ret=Macro.ParseMacroString(&CurMacro,KeyMacro->Param.PlainText.SequenceText);
            if(Ret)
            {
              if(CurMacro.BufferSize > 1)
                xf_free(CurMacro.Buffer);
            }
            else
            {
              static string ErrMsg[3];
              Macro.GetMacroParseError(&ErrMsg[0],&ErrMsg[1],&ErrMsg[2]);
              KeyMacro->Param.MacroResult.ErrMsg1=ErrMsg[0];
              KeyMacro->Param.MacroResult.ErrMsg2=ErrMsg[1];
              KeyMacro->Param.MacroResult.ErrMsg3=ErrMsg[2];
            }
            return Ret;
          }

          case MCMD_GETSTATE:
          {
            return Macro.GetCurRecord(NULL,NULL);
          }

#if 0
          case MCMD_COMPILEMACRO:
          {
            struct MacroRecord CurMacro={0};
            int Ret=Macro.ParseMacroString(&CurMacro,KeyMacro->Param.PlainText.SequenceText);
            if(Ret)
            {
              //KeyMacro->Params.Compile.Flags=CurMacro.Flags;
              KeyMacro->Param.Compile.Sequence=CurMacro.Buffer;
              KeyMacro->Param.Compile.Count=CurMacro.BufferSize;
            }
            return Ret;
          }
#endif
        }
      }
      return FALSE;
    }

    case ACTL_POSTKEYSEQUENCE:
    {
      if(CtrlObject && Param && ((struct KeySequence*)Param)->Count > 0)
      {
        struct MacroRecord MRec;
        memset(&MRec,0,sizeof(struct MacroRecord));
        MRec.Flags=(((struct KeySequence*)Param)->Flags)<<8;
        MRec.BufferSize=((struct KeySequence*)Param)->Count;
        if(MRec.BufferSize == 1)
          MRec.Buffer=(DWORD *)(DWORD_PTR)((struct KeySequence*)Param)->Sequence[0];
        else
          MRec.Buffer=((struct KeySequence*)Param)->Sequence;
        return CtrlObject->Macro.PostNewMacro(&MRec,TRUE,TRUE);
#if 0
        // ���� ����� - ��� ���������� �������������
        {
          //CtrlObject->Macro.PostNewMacro(&MRec);
          for(int I=0; I < MRec.BufferSize; ++I)
          {
            int Key=MRec.Buffer[I];
            if(CtrlObject->Macro.ProcessKey(Key))
            {
              while((Key=CtrlObject->Macro.GetKey()) != 0)
              {
                FrameManager->ProcessKey(Key);
              }
            }
            else
              FrameManager->ProcessKey(Key);
            FrameManager->PluginCommit();
          }
          return TRUE;
        }
#endif
      }
      return FALSE;
    }

    /* $ 05.06.2001 tran
       ����� ACTL_ ��� ������ � �������� */
    case ACTL_GETWINDOWINFO:
    /* $ 12.04.2005 AY
         thread safe window info */
    case ACTL_GETSHORTWINDOWINFO:
    {
      if(FrameManager && Param && !IsBadWritePtr(Param,sizeof(WindowInfo)))
      {
        string strType, strName;
        WindowInfo *wi=(WindowInfo*)Param;
        Frame *f;
        /* $ 22.12.2001 VVM
          + ���� Pos == -1 �� ����� ������� ����� */
        if (wi->Pos == -1)
          f=FrameManager->GetCurrentFrame();
        else
          f=FrameManager->operator[](wi->Pos);
        if ( f==NULL )
          return FALSE;
        if (Command==ACTL_GETWINDOWINFO)
        {
          f->GetTypeAndName(strType, strName);
          wi->TypeName=xf_wcsdup(strType);
          wi->Name=xf_wcsdup(strName);
        }
        else
        {
          wi->TypeName=NULL;
          wi->Name=NULL;
        }
        wi->Pos=FrameManager->IndexOf(f);
        wi->Type=f->GetType();
        wi->Modified=f->IsFileModified();
        wi->Current=f==FrameManager->GetCurrentFrame();
        return TRUE;
      }
      return FALSE;
    }
    case ACTL_FREEWINDOWINFO:
    {
      if(!Param)
        return FALSE;
      WindowInfo *wi=(WindowInfo*)Param;
      xf_free((void *)wi->TypeName);
      xf_free((void *)wi->Name);
      return TRUE;
    }

    case ACTL_GETWINDOWCOUNT:
    {
      return FrameManager?FrameManager->GetFrameCount():0;
    }

    case ACTL_SETCURRENTWINDOW:
    {
      // �������� ������������ �������, ���� ��������� � ��������� ���������/������.
      if (FrameManager && !FrameManager->InModalEV() && FrameManager->operator[]((int)(INT_PTR)Param)!=NULL )
      {
        int TypeFrame=FrameManager->GetCurrentFrame()->GetType();
        // �������� ������������ �������, ���� ��������� � ����� ��� ������� (���� ���������)
        if(TypeFrame != MODALTYPE_HELP && TypeFrame != MODALTYPE_DIALOG)
        {
          Frame* PrevFrame = FrameManager->GetCurrentFrame();
          FrameManager->ActivateFrame((int)(INT_PTR)Param);
          FrameManager->DeactivateFrame(PrevFrame, 0);
          return TRUE;
        }
      }
      return FALSE;
    }

    /*$ 26.06.2001 SKV
      ��� ����������� ������ � ACTL_SETCURRENTWINDOW
      (� ����� ��� ��� ���� � �������)
    */
    case ACTL_COMMIT:
    {
      return FrameManager?FrameManager->PluginCommit():FALSE;
    }
    /* $ 15.09.2001 tran
       ���������� �������� */
    case ACTL_GETFARHWND:
    {
      if(!hFarWnd)
        InitDetectWindowedMode();
      return (INT_PTR)hFarWnd;
    }

    case ACTL_GETDIALOGSETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags ODlg[]={
        {&Opt.Dialogs.EditHistory,FDIS_HISTORYINDIALOGEDITCONTROLS},
        {&Opt.Dialogs.EditBlock,FDIS_PERSISTENTBLOCKSINEDITCONTROLS},
        {&Opt.Dialogs.AutoComplete,FDIS_AUTOCOMPLETEININPUTLINES},
        {&Opt.Dialogs.EULBsClear,FDIS_BSDELETEUNCHANGEDTEXT},
        {&Opt.Dialogs.DelRemovesBlocks,FDIS_DELREMOVESBLOCKS},
        {&Opt.Dialogs.MouseButton,FDIS_MOUSECLICKOUTSIDECLOSESDIALOG},
      };
      for(size_t I=0; I < countof(ODlg); ++I)
        if(*ODlg[I].Opt)
          Options|=ODlg[I].Flags;
      return Options;
    }
    /* $ 24.11.2001 IS
       ��������� � ����������� ����������, ������, ����������, �������������
    */
    case ACTL_GETSYSTEMSETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.ClearReadOnly,FSS_CLEARROATTRIBUTE},
        {&Opt.DeleteToRecycleBin,FSS_DELETETORECYCLEBIN},
        {&Opt.CMOpt.UseSystemCopy,FSS_USESYSTEMCOPYROUTINE},
        {&Opt.CMOpt.CopyOpened,FSS_COPYFILESOPENEDFORWRITING},
        {&Opt.ScanJunction,FSS_SCANSYMLINK},
        {&Opt.CreateUppercaseFolders,FSS_CREATEFOLDERSINUPPERCASE},
        {&Opt.SaveHistory,FSS_SAVECOMMANDSHISTORY},
        {&Opt.SaveFoldersHistory,FSS_SAVEFOLDERSHISTORY},
        {&Opt.SaveViewHistory,FSS_SAVEVIEWANDEDITHISTORY},
        {&Opt.UseRegisteredTypes,FSS_USEWINDOWSREGISTEREDTYPES},
        {&Opt.AutoSaveSetup,FSS_AUTOSAVESETUP},
      };
      for(size_t I=0; I < countof(OSys); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETPANELSETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.ShowHidden,FPS_SHOWHIDDENANDSYSTEMFILES},
        {&Opt.Highlight,FPS_HIGHLIGHTFILES},
        {&Opt.Tree.AutoChangeFolder,FPS_AUTOCHANGEFOLDER},
        {&Opt.SelectFolders,FPS_SELECTFOLDERS},
        {&Opt.ReverseSort,FPS_ALLOWREVERSESORTMODES},
        {&Opt.ShowColumnTitles,FPS_SHOWCOLUMNTITLES},
        {&Opt.ShowPanelStatus,FPS_SHOWSTATUSLINE},
        {&Opt.ShowPanelTotals,FPS_SHOWFILESTOTALINFORMATION},
        {&Opt.ShowPanelFree,FPS_SHOWFREESIZE},
        {&Opt.ShowPanelScrollbar,FPS_SHOWSCROLLBAR},
        {&Opt.ShowScreensNumber,FPS_SHOWBACKGROUNDSCREENSNUMBER},
        {&Opt.ShowSortMode,FPS_SHOWSORTMODELETTER},
      };
      for(size_t I=0; I < countof(OSys); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETINTERFACESETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.Clock,FIS_CLOCKINPANELS},
        {&Opt.ViewerEditorClock,FIS_CLOCKINVIEWERANDEDITOR},
        {&Opt.Mouse,FIS_MOUSE},
        {&Opt.ShowKeyBar,FIS_SHOWKEYBAR},
        {&Opt.ShowMenuBar,FIS_ALWAYSSHOWMENUBAR},
        {&Opt.AltGr,FIS_USERIGHTALTASALTGR},
        {&Opt.CMOpt.CopyShowTotal,FIS_SHOWTOTALCOPYPROGRESSINDICATOR},
        {&Opt.CMOpt.CopyTimeRule,FIS_SHOWCOPYINGTIMEINFO},
        {&Opt.PgUpChangeDisk,FIS_USECTRLPGUPTOCHANGEDRIVE},
      };
      for(size_t I=0; I < countof(OSys); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETCONFIRMATIONS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.Confirm.Copy,FCS_COPYOVERWRITE},
        {&Opt.Confirm.Move,FCS_MOVEOVERWRITE},
        {&Opt.Confirm.Drag,FCS_DRAGANDDROP},
        {&Opt.Confirm.Delete,FCS_DELETE},
        {&Opt.Confirm.DeleteFolder,FCS_DELETENONEMPTYFOLDERS},
        {&Opt.Confirm.Esc,FCS_INTERRUPTOPERATION},
        {&Opt.Confirm.RemoveConnection,FCS_DISCONNECTNETWORKDRIVE},
        {&Opt.Confirm.AllowReedit,FCS_RELOADEDITEDFILE},
        {&Opt.Confirm.HistoryClear,FCS_CLEARHISTORYLIST},
        {&Opt.Confirm.Exit,FCS_EXIT},
      };
      for(size_t I=0; I < countof(OSys); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETDESCSETTINGS:
    {
      // ����� ���� - � �������� �� ��������������
      DWORD Options=0;
      if (Opt.Diz.UpdateMode == DIZ_UPDATE_IF_DISPLAYED)
        Options |= FDS_UPDATEIFDISPLAYED;
      else if (Opt.Diz.UpdateMode == DIZ_UPDATE_ALWAYS)
        Options |= FDS_UPDATEALWAYS;
      if (Opt.Diz.SetHidden)
        Options |= FDS_SETHIDDEN;
      if (Opt.Diz.ROUpdate)
        Options |= FDS_UPDATEREADONLY;
      return Options;
    }

    case ACTL_GETPOLICIES:
    {
      return Opt.Policies.DisabledOptions|(Opt.Policies.ShowHiddenDrives?FFPOL_SHOWHIDDENDRIVES:0);
    }

    case ACTL_REDRAWALL:
      return FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);

  }
  return FALSE;
}

int WINAPI FarMenuFn (
		INT_PTR PluginNumber,
		int X,
		int Y,
		int MaxHeight,
		DWORD Flags,
		const wchar_t *Title,
		const wchar_t *Bottom,
		const wchar_t *HelpTopic,
		const int *BreakKeys,
		int *BreakCode,
		const FarMenuItem *Item,
		int ItemsNumber
		)
{
  int I;

  if (FrameManager->ManagerIsDown())
    return -1;

  if (DisablePluginsOutput)
    return(-1);

  int ExitCode;
  {
    VMenu FarMenu(Title,NULL,0,MaxHeight);
    CtrlObject->Macro.SetMode(MACRO_MENU);
    FarMenu.SetPosition(X,Y,0,0);
    if (BreakCode!=NULL)
      *BreakCode=-1;

    {
      string strTopic;
      if(Help::MkTopic(PluginNumber,HelpTopic,strTopic))
        FarMenu.SetHelp(strTopic);
    }

    if (Bottom!=NULL)
      FarMenu.SetBottomTitle(Bottom);

    // ����� ����� ����
    DWORD MenuFlags=0;
    if (Flags & FMENU_SHOWAMPERSAND)
      MenuFlags|=VMENU_SHOWAMPERSAND;
    if (Flags & FMENU_WRAPMODE)
      MenuFlags|=VMENU_WRAPMODE;
    if (Flags & FMENU_CHANGECONSOLETITLE)
      MenuFlags|=VMENU_CHANGECONSOLETITLE;
    if (Flags & FMENU_TRUNCPATH)
      MenuFlags|=VMENU_TRUNCPATH;
    if (Flags & FMENU_TRUNCSTR)
      MenuFlags|=VMENU_TRUNCSTR;
    if (Flags & FMENU_TRUNCSTREND)
      MenuFlags|=VMENU_TRUNCSTREND;
    FarMenu.SetFlags(MenuFlags);

    MenuItemEx CurItem;

    CurItem.Clear ();
    int Selected=0;

    if(Flags&FMENU_USEEXT)
    {
      struct FarMenuItemEx *ItemEx=(struct FarMenuItemEx*)Item;
      for (I=0; I < ItemsNumber; I++, ++ItemEx)
      {
        CurItem.Flags=ItemEx->Flags;
        CurItem.strName=L"";

        // ��������� MultiSelected, �.�. � ��� ������ ������ � ����� �� ������������, ��������� ������ ������
        DWORD SelCurItem=CurItem.Flags&LIF_SELECTED;
        CurItem.Flags&=~LIF_SELECTED;
        if(!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
        {
          CurItem.Flags|=SelCurItem;
          Selected++;
        }

        CurItem.strName=ItemEx->Text;

        CurItem.AccelKey=(CurItem.Flags&LIF_SEPARATOR)?0:ItemEx->AccelKey;
        FarMenu.AddItem(&CurItem);
      }
    }
    else
    {
      for (I=0;I<ItemsNumber;I++)
      {
        CurItem.Flags=Item[I].Checked?(LIF_CHECKED|(Item[I].Checked&0xFFFF)):0;
        CurItem.Flags|=Item[I].Selected?LIF_SELECTED:0;
        CurItem.Flags|=Item[I].Separator?LIF_SEPARATOR:0;
        if(Item[I].Separator)
          CurItem.strName=L"";
        else
          CurItem.strName = Item[I].Text;

        DWORD SelCurItem=CurItem.Flags&LIF_SELECTED;
        CurItem.Flags&=~LIF_SELECTED;
        if(!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
        {
          CurItem.Flags|=SelCurItem;
          Selected++;
        }
        FarMenu.AddItem(&CurItem);
      }
    }

    if (!Selected)
      FarMenu.SetSelectPos(0,1);

    // ����� ����, � ������� ���������
    if (Flags & FMENU_AUTOHIGHLIGHT)
      FarMenu.AssignHighlights(FALSE);
    if (Flags & FMENU_REVERSEAUTOHIGHLIGHT)
      FarMenu.AssignHighlights(TRUE);

    FarMenu.SetTitle(Title);

    FarMenu.Show();
    while (!FarMenu.Done() && !CloseFARMenu)
    {
      INPUT_RECORD ReadRec;
      int ReadKey=GetInputRecord(&ReadRec);
      if (ReadKey==KEY_CONSOLE_BUFFER_RESIZE)
      {
        LockScreen LckScr;
        FarMenu.Hide();
        FarMenu.Show();
      }
      else if (ReadRec.EventType==MOUSE_EVENT)
      {
        FarMenu.ProcessMouse(&ReadRec.Event.MouseEvent);
      }
      else if (ReadKey!=KEY_NONE)
      {
        if (BreakKeys!=NULL)
        {
          for (int I=0;BreakKeys[I]!=0;I++)
          {
            if (CtrlObject->Macro.IsExecuting())
            {
              int VirtKey,ControlState;
              TranslateKeyToVK(ReadKey,VirtKey,ControlState,&ReadRec);
            }
            if (ReadRec.Event.KeyEvent.wVirtualKeyCode==(BreakKeys[I] & 0xffff))
            {
              DWORD Flags=BreakKeys[I]>>16;

              DWORD RealFlags=ReadRec.Event.KeyEvent.dwControlKeyState &
                    (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|
                    LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED|SHIFT_PRESSED);

              DWORD f = 0;
              if (RealFlags & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))
                f |= PKF_CONTROL;
              if (RealFlags & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
                f |= PKF_ALT;
              if (RealFlags & SHIFT_PRESSED)
                f |= PKF_SHIFT;

              if (f == Flags)
              {
                if (BreakCode!=NULL)
                  *BreakCode=I;
                FarMenu.Hide();
//                  CheckScreenLock();
                return FarMenu.GetSelectPos();
              }
            }
          }
        }
        FarMenu.ProcessKey(ReadKey);
      }
    }
    ExitCode=FarMenu.Modal::GetExitCode();
  }
//  CheckScreenLock();
  return(ExitCode);
}

// ������� FarDefDlgProc ��������� ������� �� ���������
LONG_PTR WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  if(hDlg)  // ��������� ������ ����� ��� hDlg=0
    return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
  return 0;
}

// ������� ��������� �������
LONG_PTR WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  if(hDlg) // ��������� ������ ����� ��� hDlg=0
    return Dialog::SendDlgMessage(hDlg,Msg,Param1,Param2);
  return 0;
}

#if !defined(__GNUC__)
/* ���� ������ ������� - ��������� ���� Flags - ������� ����, ���
   �� ����� ��� �� � �������
*/
static int Except_FarDialogEx()
{
  if(CtrlObject)
    CtrlObject->Plugins.Flags.Set(PSIF_DIALOG);

  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
  {
    //while(!frame->Refreshable()) // � ����� ��� ���� �����???
      frame->Unlock(); // ������ ����� :-)
  }
//  CheckScreenLock();
  FrameManager->RefreshFrame(); //??

  return EXCEPTION_CONTINUE_SEARCH; // ��������� ���������� ������� ����������!
}
#endif

static int FarDialogExSehed(Dialog *FarDialog)
{
  TRY
  {
    FarDialog->Process();
    return FarDialog->GetExitCode();
  }
  EXCEPT (Except_FarDialogEx())
  {
    return -1;
  }
}

HANDLE WINAPI FarDialogInit(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2,
                       const wchar_t *HelpTopic, struct FarDialogItem *Item,
                       unsigned int ItemsNumber, DWORD Reserved, DWORD Flags,
                       FARWINDOWPROC DlgProc, LONG_PTR Param)
{
  HANDLE hDlg=INVALID_HANDLE_VALUE;

  if (FrameManager->ManagerIsDown())
    return hDlg;

  if (DisablePluginsOutput ||
      ItemsNumber <= 0 ||
      !Item ||
      IsBadReadPtr(Item,sizeof(struct FarDialogItem)*ItemsNumber))
    return hDlg;

  // ����! ������ ��������� ������������� X2 � Y2
  if(X2 < 0 || Y2 < 0)
    return hDlg;

  {
  	Dialog *FarDialog = new Dialog(Item,ItemsNumber,DlgProc,Param);
    if (!FarDialog)
      return hDlg;

    if (!FarDialog->InitOK())
    {
    	delete FarDialog;
    	return hDlg;
    }

    hDlg = (HANDLE)FarDialog;

    FarDialog->SetPosition(X1,Y1,X2,Y2);

    if(Flags & FDLG_WARNING)
      FarDialog->SetDialogMode(DMODE_WARNINGSTYLE);
    if(Flags & FDLG_SMALLDIALOG)
      FarDialog->SetDialogMode(DMODE_SMALLDIALOG);
    if(Flags & FDLG_NODRAWSHADOW)
      FarDialog->SetDialogMode(DMODE_NODRAWSHADOW);
    if(Flags & FDLG_NODRAWPANEL)
      FarDialog->SetDialogMode(DMODE_NODRAWPANEL);
    if(Flags & FDLG_NONMODAL)
      FarDialog->SetCanLoseFocus(TRUE);
    FarDialog->SetHelp(HelpTopic);

    /* $ 29.08.2000 SVS
       �������� ����� ������� - ������ � �������� ��� ������������ HelpTopic
    */
    FarDialog->SetPluginNumber(PluginNumber);
  }

  return hDlg;
}

int WINAPI FarDialogRun(HANDLE hDlg)
{
	if (FrameManager->ManagerIsDown())
		return -1;

	if(hDlg==INVALID_HANDLE_VALUE) return -1;

	Frame *frame;
	if((frame=FrameManager->GetBottomFrame()) != NULL)
		frame->Lock(); // ������� ���������� ������

	int ExitCode=-1;

	Dialog *FarDialog = (Dialog *)hDlg;

	if(Opt.ExceptRules)
	{
		CtrlObject->Plugins.Flags.Clear(PSIF_DIALOG);
		ExitCode=FarDialogExSehed(FarDialog);
	}
	else
	{
		FarDialog->Process();
		ExitCode=FarDialog->GetExitCode();
	}

	/* $ 15.05.2002 SKV
		������ ����������� ����� ����� ��, ��� ��������.
	*/
	if(frame != NULL)
		frame->Unlock(); // ������ ����� :-)
	//CheckScreenLock();
	FrameManager->RefreshFrame(); //?? - //AY - ��� ����� ���� ��������� ������ ����� ������ �� �������

	return(ExitCode);
}

void WINAPI FarDialogFree(HANDLE hDlg)
{
	if(hDlg!=INVALID_HANDLE_VALUE)
	{
		Dialog *FarDialog = (Dialog *)hDlg;
		delete FarDialog;
	}
}

const wchar_t* WINAPI FarGetMsgFn(INT_PTR PluginHandle,int MsgId)
{
	//BUGBUG, ���� ���������, ��� PluginHandle - ������

	Plugin *pPlugin = (Plugin*)PluginHandle;

	string strPath = pPlugin->GetModuleName();

	CutToSlash(strPath);

	if ( pPlugin->InitLang(strPath) )
		return pPlugin->GetMsg(MsgId);

	return L"";
}

int WINAPI FarMessageFn(INT_PTR PluginNumber,DWORD Flags,const wchar_t *HelpTopic,
                        const wchar_t * const *Items,int ItemsNumber,
                        int ButtonsNumber)
{
  if (FrameManager->ManagerIsDown())
    return -1;

  if (DisablePluginsOutput)
    return(-1);

  if ((!(Flags&(FMSG_ALLINONE|FMSG_ERRORTYPE)) && ItemsNumber<2) || !Items)
    return(-1);

  wchar_t *SingleItems=NULL;
  wchar_t *Msg;
  int I;

  // ������ ���������� ����� ��� FMSG_ALLINONE
  if(Flags&FMSG_ALLINONE)
  {
    ItemsNumber=0;
    I=StrLength((wchar_t *)Items)+2;
    if((SingleItems=(wchar_t *)xf_malloc(I*sizeof (wchar_t))) == NULL)
      return -1;

    Msg=wcscpy(SingleItems,(wchar_t *)Items);
    while ((Msg = wcschr(Msg, L'\n')) != NULL)
    {
//      *Msg='\0';

      if(*++Msg == L'\0')
        break;
      ++ItemsNumber;
    }
    ItemsNumber++; //??
  }

  const wchar_t **MsgItems=(const wchar_t **)xf_malloc(sizeof(wchar_t*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));
  if(!MsgItems)
  {
    xf_free(SingleItems);
    return(-1);
  }

  memset(MsgItems,0,sizeof(wchar_t*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));

  if(Flags&FMSG_ALLINONE)
  {
    I=0;
    Msg=SingleItems;

    // ������ ���������� ����� � �������� �� ������
    wchar_t *MsgTemp;
    while ((MsgTemp = wcschr(Msg, L'\n')) != NULL)
    {
      *MsgTemp=L'\0';
      MsgItems[I]=Msg;
      Msg+=StrLength(Msg)+1;

      if(*Msg == L'\0')
        break;
      ++I;
    }
    if(*Msg)
    {
      MsgItems[I]=Msg;
    }
  }
  else
  {
    for (I=0; I < ItemsNumber; I++)
      MsgItems[I]=Items[I];
  }

  // ����������� �� ������
  if(ItemsNumber > ScrY-2)
  {
    ItemsNumber=ScrY-2-(Flags&0x000F0000?1:0);
  }

  /* $ 22.03.2001 tran
     ItemsNumber++ -> ++ItemsNumber
     �������� ��������� ������� */
  switch(Flags&0x000F0000)
  {
    case FMSG_MB_OK:
      ButtonsNumber=1;
      MsgItems[ItemsNumber++]=MSG(MOk);
      break;
    case FMSG_MB_OKCANCEL:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=MSG(MOk);
      MsgItems[ItemsNumber++]=MSG(MCancel);
      break;
    case FMSG_MB_ABORTRETRYIGNORE:
      ButtonsNumber=3;
      MsgItems[ItemsNumber++]=MSG(MAbort);
      MsgItems[ItemsNumber++]=MSG(MRetry);
      MsgItems[ItemsNumber++]=MSG(MIgnore);
      break;
    case FMSG_MB_YESNO:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=MSG(MYes);
      MsgItems[ItemsNumber++]=MSG(MNo);
      break;
    case FMSG_MB_YESNOCANCEL:
      ButtonsNumber=3;
      MsgItems[ItemsNumber++]=MSG(MYes);
      MsgItems[ItemsNumber++]=MSG(MNo);
      MsgItems[ItemsNumber++]=MSG(MCancel);
      break;
    case FMSG_MB_RETRYCANCEL:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=MSG(MRetry);
      MsgItems[ItemsNumber++]=MSG(MCancel);
      break;
  }

  // ���������� �����
  if(PluginNumber != -1)
  {
    string strTopic;
    if(Help::MkTopic(PluginNumber,HelpTopic,strTopic))
      SetMessageHelp(strTopic);
  }

  // ���������������... �����
  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->Lock(); // ������� ���������� ������
  int MsgCode=Message(Flags,ButtonsNumber,MsgItems[0],MsgItems+1,ItemsNumber-1,PluginNumber);
  /* $ 15.05.2002 SKV
    ������ ����������� ���� ����� ��, ��� ��������.
  */
  if(frame != NULL)
    frame->Unlock(); // ������ ����� :-)
  //CheckScreenLock();

  if(SingleItems)
    xf_free(SingleItems);
  xf_free(MsgItems);
  return(MsgCode);
}

int WINAPI FarControl(HANDLE hPlugin,int Command,void *Param)
{
  _FCTLLOG(CleverSysLog CSL(L"Control"));
  _FCTLLOG(SysLog(L"(hPlugin=0x%08X, Command=%s, Param=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param,Param));
  _ALGO(CleverSysLog clv(L"FarControl"));
  _ALGO(SysLog(L"(hPlugin=0x%08X, Command=%s, Param=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param,Param));

  if(Command == FCTL_CHECKPANELSEXIST)
    return CmdMode == FALSE?TRUE:FALSE;

  if (CmdMode || !CtrlObject || !FrameManager || FrameManager->ManagerIsDown())
    return 0;

  FilePanels *FPanels=CtrlObject->Cp();
  CommandLine *CmdLine=CtrlObject->CmdLine;

  switch(Command)
  {
    case FCTL_CLOSEPLUGIN:
      g_strDirToSet = (wchar_t *)Param;

    case FCTL_GETPANELINFO:
    case FCTL_GETPANELSHORTINFO:
    case FCTL_FREEPANELINFO:
    case FCTL_UPDATEPANEL:
    case FCTL_REDRAWPANEL:
    case FCTL_SETPANELDIR:
    case FCTL_SETSELECTION:
    case FCTL_SETVIEWMODE:
    case FCTL_SETSORTMODE:
    case FCTL_SETSORTORDER:
    case FCTL_SETNUMERICSORT:
    {
      if (!FPanels)
        return FALSE;

      if ( (hPlugin == PANEL_ACTIVE) || (hPlugin == PANEL_PASSIVE) )
      {
        Panel *pPanel = (hPlugin == PANEL_ACTIVE)?FPanels->ActivePanel:FPanels->GetAnotherPanel (FPanels->ActivePanel);

        if ( pPanel )
        {
         	pPanel->SetPluginCommand (Command, Param);
         	return TRUE;
        }

        return FALSE; //???
      }

      HANDLE hInternal;
      Panel *LeftPanel=FPanels->LeftPanel;
      Panel *RightPanel=FPanels->RightPanel;
      int Processed=FALSE;
      struct PluginHandle *PlHandle;

      if (LeftPanel && LeftPanel->GetMode()==PLUGIN_PANEL)
      {
        PlHandle=(struct PluginHandle *)LeftPanel->GetPluginHandle();
        if (PlHandle && !IsBadReadPtr(PlHandle,sizeof(struct PluginHandle)))
        {
          hInternal=PlHandle->hPlugin;
          if (hPlugin==hInternal)
          {
            LeftPanel->SetPluginCommand(Command,Param);
            Processed=TRUE;
          }
        }
      }

      if (RightPanel && RightPanel->GetMode()==PLUGIN_PANEL)
      {
        PlHandle=(struct PluginHandle *)RightPanel->GetPluginHandle();
        if (PlHandle && !IsBadReadPtr(PlHandle,sizeof(struct PluginHandle)))
        {
          hInternal=PlHandle->hPlugin;
          if (hPlugin==hInternal)
          {
            RightPanel->SetPluginCommand(Command,Param);
            Processed=TRUE;
          }
        }
      }

      return(Processed);
    }

    case FCTL_SETUSERSCREEN:
    {
      if (!FPanels || !FPanels->LeftPanel || !FPanels->RightPanel)
        return(FALSE);

      KeepUserScreen++;
      FPanels->LeftPanel->ProcessingPluginCommand++;
      FPanels->RightPanel->ProcessingPluginCommand++;
      ScrBuf.FillBuf();
      ScrollScreen(1);
      SaveScreen SaveScr;
      {
        RedrawDesktop Redraw;
        CmdLine->Hide();
        SaveScr.RestoreArea(FALSE);
      }
      KeepUserScreen--;
      FPanels->LeftPanel->ProcessingPluginCommand--;
      FPanels->RightPanel->ProcessingPluginCommand--;
      return(TRUE);
    }

    case FCTL_GETUSERSCREEN:
    {
      FrameManager->ShowBackground();
      int Lock=ScrBuf.GetLockCount();
      ScrBuf.SetLockCount(0);
      MoveCursor(0,ScrY-1);
      SetInitialCursorType();
      ScrBuf.Flush();
      ScrBuf.SetLockCount(Lock);
      return TRUE;
    }

		case FCTL_GETCMDLINE:
		case FCTL_GETCMDLINESELECTEDTEXT:
		{
			string strParam;
			if(Command==FCTL_GETCMDLINE)
				CmdLine->GetString(strParam);
			else
				CmdLine->GetSelString(strParam);
			if(Param)
				wcscpy((wchar_t*)Param, strParam);
      return (int)strParam.GetLength();
		}

    case FCTL_SETCMDLINE:
    case FCTL_INSERTCMDLINE:
    {
      if (Command==FCTL_SETCMDLINE)
        CmdLine->SetString((const wchar_t*)Param);
      else
        CmdLine->InsertString((const wchar_t*)Param);

      CmdLine->Redraw();
      return(TRUE);
    }

    case FCTL_SETCMDLINEPOS:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(int)))
      {
        CmdLine->SetCurPos(*(int *)Param);
        CmdLine->Redraw();
        return TRUE;
      }
      return FALSE;
    }

    case FCTL_GETCMDLINEPOS:
    {
      if(Param && !IsBadWritePtr(Param,sizeof(int)))
      {
        *(int *)Param=CmdLine->GetCurPos();
        return TRUE;
      }
      return FALSE;
    }

    case FCTL_GETCMDLINESELECTION:
    {
      CmdLineSelect *sel=(CmdLineSelect*)Param;
      if(sel && !IsBadWritePtr(sel,sizeof(struct CmdLineSelect)))
      {
        CmdLine->GetSelection(sel->SelStart,sel->SelEnd);
        return TRUE;
      }
      return FALSE;
    }

    case FCTL_SETCMDLINESELECTION:
    {
      CmdLineSelect *sel=(CmdLineSelect*)Param;
      if(sel && !IsBadReadPtr(sel,sizeof(struct CmdLineSelect)))
      {
        CmdLine->Select(sel->SelStart,sel->SelEnd);
        CmdLine->Redraw();
        return TRUE;
      }
      return FALSE;
    }

    case FCTL_ISACTIVEPANEL:
    {
    	if ( hPlugin == PANEL_ACTIVE )
    		return TRUE;

    	Panel *pPanel = FPanels->ActivePanel;
		PluginHandle *PlHandle;

		if ( pPanel && (pPanel->GetMode() == PLUGIN_PANEL) )
		{
			PlHandle = (PluginHandle *)pPanel->GetPluginHandle();

			if ( PlHandle && !IsBadReadPtr(PlHandle, sizeof(PluginHandle)) )
			{
				if ( PlHandle->hPlugin == hPlugin )
					return TRUE;
			}
		}

		return FALSE;
    }
  }
  return(FALSE);
}


HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2)
{
  if (FrameManager->ManagerIsDown())
    return NULL;

  if (X2==-1)
    X2=ScrX;
  if (Y2==-1)
    Y2=ScrY;

  return((HANDLE)(new SaveScreen(X1,Y1,X2,Y2,FALSE)));
}


void WINAPI FarRestoreScreen(HANDLE hScreen)
{
  if (FrameManager->ManagerIsDown())
    return;

  if (hScreen==NULL)
    ScrBuf.FillBuf();
  if (hScreen)
    delete (SaveScreen *)hScreen;
}


static void PR_FarGetDirListMsg(void)
{
  Message(MSG_DOWN,0,L"",MSG(MPreparingList));
}

int WINAPI FarGetDirList(const wchar_t *Dir,FAR_FIND_DATA **pPanelItem,int *pItemsNumber)
{
  if (FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
    return FALSE;

  string strDirName;

  ConvertNameToFull(Dir, strDirName);

  {
    TPreRedrawFuncGuard preRedrawFuncGuard(PR_FarGetDirListMsg);
    SaveScreen SaveScr;
    clock_t StartTime=clock();
    int MsgOut=0;

    *pItemsNumber=0;
    *pPanelItem=NULL;

    FAR_FIND_DATA_EX FindData;
    string strFullName;
    ScanTree ScTree(FALSE);

    ScTree.SetFindPath(strDirName,L"*.*");

    CutToSlash(strDirName); //BUGBUG
    FAR_FIND_DATA *ItemsList=NULL;
    int ItemsNumber=0;
    while (ScTree.GetNextName(&FindData,strFullName))
    {
      if ((ItemsNumber & 31)==0)
      {
        if (CheckForEsc())
        {
          if(ItemsList)
            xf_free(ItemsList);
          return FALSE;
        }

        if (!MsgOut && clock()-StartTime > 500)
        {
          SetCursorType(FALSE,0);
          PR_FarGetDirListMsg();
          MsgOut=1;
        }

        ItemsList=(FAR_FIND_DATA*)xf_realloc(ItemsList,sizeof(*ItemsList)*(ItemsNumber+32+1));
        if (ItemsList==NULL)
        {
          *pItemsNumber=0;
          return FALSE;
        }
      }

      ItemsList[ItemsNumber].dwFileAttributes = FindData.dwFileAttributes;
      ItemsList[ItemsNumber].nFileSize = FindData.nFileSize;
      ItemsList[ItemsNumber].nPackSize = FindData.nPackSize;
      ItemsList[ItemsNumber].ftCreationTime = FindData.ftCreationTime;
      ItemsList[ItemsNumber].ftLastAccessTime = FindData.ftLastAccessTime;
      ItemsList[ItemsNumber].ftLastWriteTime = FindData.ftLastWriteTime;
      ItemsList[ItemsNumber].lpwszFileName = xf_wcsdup (FindData.strFileName);
      ItemsList[ItemsNumber].lpwszAlternateFileName = xf_wcsdup (FindData.strAlternateFileName);

      ItemsNumber++;
    }

    *pPanelItem=ItemsList;
    *pItemsNumber=ItemsNumber;
  }
  return TRUE;
}


static struct PluginPanelItem *PluginDirList;
static int DirListItemsNumber;
static string strPluginSearchPath;
static int StopSearch;
static HANDLE hDirListPlugin;
static int PluginSearchMsgOut;
/*static struct
{
  PluginPanelItem *Addr;
  int ItemsNumber;
} DirListNumbers[16];*/

static void FarGetPluginDirListMsg(const wchar_t *Name,DWORD Flags)
{
  Message(Flags,0,L"",MSG(MPreparingList),Name);
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  preRedrawItem.Param.Flags=Flags;
  preRedrawItem.Param.Param1=(void*)Name;
  PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_FarGetPluginDirListMsg(void)
{
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  FarGetPluginDirListMsg((const wchar_t *)preRedrawItem.Param.Param1,preRedrawItem.Param.Flags&(~MSG_KEEPBACKGROUND));
}

int WINAPI FarGetPluginDirList(INT_PTR PluginNumber,
                               HANDLE hPlugin,
                               const wchar_t *Dir,
                               struct PluginPanelItem **pPanelItem,
                               int *pItemsNumber)
{
  if (FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
    return FALSE;

  {
    if (StrCmp(Dir,L".")==0 || TestParentFolderName(Dir))
      return FALSE;

    static struct PluginHandle DirListPlugin;

    // � �� ����� �� ������ ���������� �� ������� ������?
    if (hPlugin==INVALID_HANDLE_VALUE)
    {
      /* $ 30.11.2001 DJ
         � ���������� �� ��� ������?
      */
      HANDLE Handle = CtrlObject->Cp()->ActivePanel->GetPluginHandle();
      if (!Handle || Handle == INVALID_HANDLE_VALUE)
        return FALSE;

      DirListPlugin=*((struct PluginHandle *)Handle);
    }
    else
    {
      DirListPlugin.pPlugin=(Plugin*)PluginNumber;
      DirListPlugin.hPlugin=hPlugin;
    }

    {
      SaveScreen SaveScr;
      TPreRedrawFuncGuard preRedrawFuncGuard(PR_FarGetPluginDirListMsg);

      {
        string strDirName;
        strDirName = Dir;
        TruncStr(strDirName,30);
        CenterStr(strDirName,strDirName,30);
        SetCursorType(FALSE,0);

        FarGetPluginDirListMsg(strDirName,0);
        PluginSearchMsgOut=FALSE;

        hDirListPlugin=(HANDLE)&DirListPlugin;
        StopSearch=FALSE;
        *pItemsNumber=DirListItemsNumber=0;
        *pPanelItem=PluginDirList=NULL;

        struct OpenPluginInfo Info;
        CtrlObject->Plugins.GetOpenPluginInfo(hDirListPlugin,&Info);

        string strPrevDir = Info.CurDir;

        if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,Dir,OPM_FIND))
        {
          strPluginSearchPath = Dir;
          strPluginSearchPath += L"\x1";

          ScanPluginDir();

          *pPanelItem=PluginDirList;
          *pItemsNumber=DirListItemsNumber;
          CtrlObject->Plugins.SetDirectory(hDirListPlugin,L"..",OPM_FIND);
          PluginPanelItem *PanelData=NULL;

          int ItemCount=0;
          if (CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
            CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);

          struct OpenPluginInfo NewInfo;
          CtrlObject->Plugins.GetOpenPluginInfo(hDirListPlugin,&NewInfo);

          if ( StrCmpI (strPrevDir, NewInfo.CurDir) !=0 )
            CtrlObject->Plugins.SetDirectory(hDirListPlugin,strPrevDir,OPM_FIND);
        }
      }
    }
  }

  /*if (!StopSearch)
    for (int I=0;I<sizeof(DirListNumbers)/sizeof(DirListNumbers[0]);I++)
      if (DirListNumbers[I].Addr==NULL)
      {
        DirListNumbers[I].Addr=*pPanelItem;
        DirListNumbers[I].ItemsNumber=*pItemsNumber;
        break;
      }*/
  return(!StopSearch);
}

/* $ 30.11.2001 DJ
   ������� � ������� ����� ��� ��� ����������� ������ � ScanPluginDir()
*/

static void CopyPluginDirItem (PluginPanelItem *CurPanelItem)
{
  string strFullName;

  strFullName = strPluginSearchPath;
  strFullName += CurPanelItem->FindData.lpwszFileName;

  wchar_t *lpwszFullName = strFullName.GetBuffer ();

  for (int I=0;lpwszFullName[I]!=0;I++)
    if (lpwszFullName[I]==L'\x1')
      lpwszFullName[I]=L'\\';

  strFullName.ReleaseBuffer ();

  PluginPanelItem *DestItem=PluginDirList+DirListItemsNumber;
  *DestItem=*CurPanelItem;
  if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
  {
    DWORD Size=*(DWORD *)CurPanelItem->UserData;
    DestItem->UserData=(DWORD_PTR)xf_malloc(Size);
    memcpy((void *)DestItem->UserData,(void *)CurPanelItem->UserData,Size);
  }

  DestItem->FindData.lpwszFileName = xf_wcsdup (strFullName);
  DestItem->FindData.lpwszAlternateFileName=NULL;
  DirListItemsNumber++;
}

void ScanPluginDir()
{
	int I;
	PluginPanelItem *PanelData=NULL;
	int ItemCount=0;
	int AbortOp=FALSE;

	string strDirName;
	strDirName = strPluginSearchPath;

	wchar_t *lpwszDirName = strDirName.GetBuffer();

	for (I=0;lpwszDirName[I]!=0;I++)
		if (lpwszDirName[I]=='\x1')
			lpwszDirName[I]=lpwszDirName[I+1]==0 ? 0:L'\\';

	strDirName.ReleaseBuffer();

	TruncStr(strDirName,30);
	CenterStr(strDirName,strDirName,30);

	if (CheckForEscSilent())
	{
		if (Opt.Confirm.Esc) // ����� ���������� ������?
			AbortOp=TRUE;

		if (ConfirmAbortOp())
			StopSearch=TRUE;
	}

	FarGetPluginDirListMsg(strDirName,AbortOp?0:MSG_KEEPBACKGROUND);

	if (StopSearch || !CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
		return;
	struct PluginPanelItem *NewList=(struct PluginPanelItem *)xf_realloc(PluginDirList,1+sizeof(*PluginDirList)*(DirListItemsNumber+ItemCount));
	if (NewList==NULL)
	{
		StopSearch=TRUE;
		return;
	}
	PluginDirList=NewList;
	for (I=0;I<ItemCount && !StopSearch;I++)
	{
		PluginPanelItem *CurPanelItem=PanelData+I;
		if ((CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
			CopyPluginDirItem (CurPanelItem);
	}
	for (I=0;I<ItemCount && !StopSearch;I++)
	{
		PluginPanelItem *CurPanelItem=PanelData+I;
		if ((CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
				StrCmp(CurPanelItem->FindData.lpwszFileName,L".")!=0 &&
				!TestParentFolderName(CurPanelItem->FindData.lpwszFileName))

		{
			struct PluginPanelItem *NewList=(struct PluginPanelItem *)xf_realloc(PluginDirList,sizeof(*PluginDirList)*(DirListItemsNumber+1));
			if (NewList==NULL)
			{
				StopSearch=TRUE;
				return;
			}
			PluginDirList=NewList;
			/* $ 30.11.2001 DJ
					���������� ����� ������� ��� ����������� FindData (�� ��������
					���������� PPIF_USERDATA)
			*/
			CopyPluginDirItem (CurPanelItem);

			string strFileName = CurPanelItem->FindData.lpwszFileName;

			if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,strFileName,OPM_FIND))
			{
				strPluginSearchPath += CurPanelItem->FindData.lpwszFileName;
				strPluginSearchPath += L"\x1";
				ScanPluginDir();
				size_t pos = (size_t)-1;
				strPluginSearchPath.RPos(pos,L'\x1');
				strPluginSearchPath.SetLength(pos);
				if (strPluginSearchPath.RPos(pos,L'\x1'))
					strPluginSearchPath.SetLength(pos+1);
				else
					strPluginSearchPath.SetLength(0);

				if (!CtrlObject->Plugins.SetDirectory(hDirListPlugin,L"..",OPM_FIND))
				{
					StopSearch=TRUE;
					break;
				}
			}
		}
	}
	CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);
}


void WINAPI FarFreeDirList(FAR_FIND_DATA *PanelItem, int nItemsNumber)
{
  for (int I=0;I<nItemsNumber;I++)
  {
    FAR_FIND_DATA *CurPanelItem=PanelItem+I;

    apiFreeFindData (CurPanelItem);
  }

  xf_free (PanelItem);
}


void WINAPI FarFreePluginDirList(PluginPanelItem *PanelItem, int ItemsNumber)
{
  if (PanelItem==NULL)
    return;

  for (int I=0;I<ItemsNumber;I++)
  {
    PluginPanelItem *CurPanelItem=PanelItem+I;

    if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
    {
      xf_free((void *)CurPanelItem->UserData);
    }

    apiFreeFindData (&CurPanelItem->FindData);
  }
  xf_free((void*)PanelItem);
}

int WINAPI FarViewer(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2, int Y2,DWORD Flags)
{
  if (FrameManager->ManagerIsDown())
    return FALSE;


  class ConsoleTitle ct;

  int DisableHistory=(Flags & VF_DISABLEHISTORY)?TRUE:FALSE;

  // $ 15.05.2002 SKV - �������� ����� ������������ ��������� ������ �� ����������.
  if( FrameManager->InModalEV())
  {
    Flags&=~VF_NONMODAL;
  }

  if (Flags & VF_NONMODAL)
  {
    /* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
    FileViewer *Viewer=new FileViewer(FileName,TRUE,DisableHistory,Title,X1,Y1,X2,Y2);
    if(!Viewer)
      return FALSE;
    /* $ 14.06.2002 IS
       ��������� VF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
       ��������� �� ��������� � VF_DELETEONCLOSE
    */
    if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
      Viewer->SetTempViewName(FileName,(Flags&VF_DELETEONCLOSE)?TRUE:FALSE);
    Viewer->SetEnableF6 ((Flags & VF_ENABLE_F6) != 0);

    /* $ 21.05.2002 SKV
      ��������� ���� ���� ������ ���� �� ��� ������ ����.
    */
    if(!(Flags&VF_IMMEDIATERETURN))
    {
      FrameManager->ExecuteNonModal();
    }
    else
    {
      if(GlobalSaveScrPtr)
        GlobalSaveScrPtr->Discard();
      FrameManager->PluginCommit();
    }
  }
  else
  {
    /* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
    FileViewer Viewer (FileName,FALSE,DisableHistory,Title,X1,Y1,X2,Y2);
    /* $ 28.05.2001 �� ��������� �����, ������� ����� ����� ������� ��������� ���� */
    Viewer.SetDynamicallyBorn(false);
    FrameManager->EnterModalEV();
    FrameManager->ExecuteModal();
    FrameManager->ExitModalEV();
    /* $ 14.06.2002 IS
       ��������� VF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
       ��������� �� ��������� � VF_DELETEONCLOSE
    */
    if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
      Viewer.SetTempViewName(FileName,(Flags&VF_DELETEONCLOSE)?TRUE:FALSE);
    Viewer.SetEnableF6 ((Flags & VF_ENABLE_F6) != 0);
    if (!Viewer.GetExitCode()){
      return FALSE;
    }
  }
  return(TRUE);
}


int WINAPI FarEditor(
		const wchar_t *FileName,
		const wchar_t *Title,
		int X1,
		int Y1,
		int X2,
		int Y2,
		DWORD Flags,
		int StartLine,
		int StartChar
		)
{
  if (FrameManager->ManagerIsDown())
    return EEC_OPEN_ERROR;

  ConsoleTitle ct;

  /* $ 12.07.2000 IS
   �������� ������ ��������� (������ ��� ��������������) � ��������
   ������������ ���������, ���� ���� ��������������� ����
  */
  int CreateNew = (Flags & EF_CREATENEW)?TRUE:FALSE;
  int Locked=(Flags & EF_LOCKED)?TRUE:FALSE;
  int DisableHistory=(Flags & EF_DISABLEHISTORY)?TRUE:FALSE;

  /* $ 14.06.2002 IS
     ��������� EF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
     ��������� �� ��������� � EF_DELETEONCLOSE
  */
  int DeleteOnClose = 0;
  if(Flags & EF_DELETEONCLOSE)
    DeleteOnClose = 1;
  else if(Flags & EF_DELETEONLYFILEONCLOSE)
    DeleteOnClose = 2;

  int OpMode=FEOPMODE_QUERY;

  if((Flags&EF_OPENMODE_MASK) != 0)
    OpMode=Flags&EF_OPENMODE_MASK;

  /*$ 15.05.2002 SKV
    �������� ����� ������������ ���������, ���� ��������� � ���������
    ��������� ��� ������.
  */
  if (FrameManager->InModalEV())
  {
    Flags&=~EF_NONMODAL;
  }

  int editorExitCode;

  int ExitCode=EEC_OPEN_ERROR;
  if (Flags & EF_NONMODAL)
  {
    /* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
    FileEditor *Editor=new FileEditor(FileName,CP_AUTODETECT,(CreateNew?FFILEEDIT_CANNEWFILE:0)|FFILEEDIT_ENABLEF6|(DisableHistory?FFILEEDIT_DISABLEHISTORY:0)|(Locked?FFILEEDIT_LOCKED:0),
                                      StartLine,StartChar,Title,
                                      X1,Y1,X2,Y2,
                                      DeleteOnClose,OpMode);
    if (Editor)
    {
      editorExitCode=Editor->GetExitCode();

      // ��������� - �������� ���� �������� (������ ��������� XC_OPEN_ERROR - ��. ��� FileEditor::Init())
      if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
      {
        delete Editor;
        Editor=NULL;
        return editorExitCode;
      }

      Editor->SetEnableF6 ((Flags & EF_ENABLE_F6) != 0);
      Editor->SetPluginTitle(Title);

      /* $ 21.05.2002 SKV - ��������� ���� ����, ������ ���� �� ��� ������ ����. */
      if(!(Flags&EF_IMMEDIATERETURN))
      {
        FrameManager->ExecuteNonModal();
      }
      else
      {
        if(GlobalSaveScrPtr)
          GlobalSaveScrPtr->Discard();
        FrameManager->PluginCommit();
      }

      ExitCode=XC_MODIFIED;
    }

  }
  else
  {
    /* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
    FileEditor Editor(FileName,CP_AUTODETECT,(CreateNew?FFILEEDIT_CANNEWFILE:0)|(DisableHistory?FFILEEDIT_DISABLEHISTORY:0)|(Locked?FFILEEDIT_LOCKED:0),
                      StartLine,StartChar,Title,
                      X1,Y1,X2,Y2,
                      DeleteOnClose,OpMode);

    editorExitCode=Editor.GetExitCode();

    // �������� ������������ (������ ������ ����� ����)
    if(editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
      ExitCode=editorExitCode;
    else
    {
      Editor.SetDynamicallyBorn(false);
      Editor.SetEnableF6 ((Flags & EF_ENABLE_F6) != 0);
      Editor.SetPluginTitle(Title);
      /* $ 15.05.2002 SKV
        ����������� ���� � ����� �/�� ���������� ���������.
      */
      FrameManager->EnterModalEV();
      FrameManager->ExecuteModal();
      FrameManager->ExitModalEV();

      ExitCode = Editor.GetExitCode();

      if (ExitCode)
      {
#if 0
         if(OpMode==FEOPMODE_BREAKIFOPEN && ExitCode==XC_QUIT)
           ExitCode = XC_OPEN_ERROR;
         else
#endif
           ExitCode = Editor.IsFileChanged()?XC_MODIFIED:XC_NOT_MODIFIED;
      }
    }
  }
  return ExitCode;
}

int WINAPI FarCmpName(const wchar_t *pattern,const wchar_t *string,int skippath)
{
  return(CmpName(pattern,string,skippath));
}


int WINAPI FarCharTable(int Command,char *Buffer,int BufferSize)
{
  if (FrameManager->ManagerIsDown())
    return -1;

  struct CharTableSet TableSet;

  if (Command==FCT_DETECT)
  {
    string strDataFileName;
    FILE *DataFile;
    /* $ 19.06.2001
       - ���: �� �������� ���������������.
         ��, ����, ����� �� �� return -1 ������������� � 268??
    */
    if (!FarMkTempEx(strDataFileName) || (DataFile=_wfopen(strDataFileName,L"w+b"))==NULL)
      return(-1);
    fwrite(Buffer,1,BufferSize,DataFile);
    fseek(DataFile,0,SEEK_SET);
    int TableNum;
    int DetectCode=DetectTable(DataFile,&TableSet,TableNum);
    fclose(DataFile);
    _wremove(strDataFileName);
    return(DetectCode ? TableNum-1:-1);
  }

  if (BufferSize > (int)sizeof(struct CharTableSet))
    return(-1);

  /* $ 07.08.2001 IS
       ��� ������� �������� ��������� ������� ��� OEM
  */
  memcpy(&TableSet,Buffer,Min((int)sizeof(CharTableSet),BufferSize));
  /* $ 17.03.2002 IS �� ����������� ���������� �������� TableName */
  if (!PrepareTable(&TableSet,Command,TRUE))
  {
    for(unsigned int i=0;i<256;++i)
    {
      TableSet.EncodeTable[i]=TableSet.DecodeTable[i]=i;
      TableSet.UpperTable[i]=LocalUpper(i);
      TableSet.LowerTable[i]=LocalLower(i);
    }

    string strTableName = MSG(MGetTableNormalText);

    UnicodeToAnsi (strTableName, TableSet.TableName, sizeof(TableSet.TableName)); //BUGBUG
    Command=-1;
  }
  memcpy(Buffer,&TableSet,BufferSize);
  return(Command);
}


void WINAPI FarText(int X,int Y,int Color,const wchar_t *Str)
{
  if (FrameManager->ManagerIsDown())
    return;
  if (Str==NULL)
  {
    int PrevLockCount=ScrBuf.GetLockCount();
    ScrBuf.SetLockCount(0);
    ScrBuf.Flush();
    ScrBuf.SetLockCount(PrevLockCount);
  }
  else
    Text(X,Y,Color,Str);
}


int WINAPI FarEditorControl(int Command,void *Param)
{
  if (FrameManager->ManagerIsDown() || !CtrlObject->Plugins.CurEditor)
    return(0);
  return(CtrlObject->Plugins.CurEditor->EditorControl(Command,Param));
}

int WINAPI FarViewerControl(int Command,void *Param)
{
  if (FrameManager->ManagerIsDown() || !CtrlObject->Plugins.CurViewer)
    return(0);
  return(CtrlObject->Plugins.CurViewer->ViewerControl(Command,Param));
}


void __stdcall farUpperBuf(wchar_t *Buf, int Length)
{
	return UpperBuf(Buf, Length);
}

void __stdcall farLowerBuf(wchar_t *Buf, int Length)
{
	return LowerBuf(Buf, Length);
}

void __stdcall farStrUpper(wchar_t *s1)
{
	return StrUpper(s1);
}

void __stdcall farStrLower(wchar_t *s1)
{
	return StrLower(s1);
}

wchar_t __stdcall farUpper(wchar_t Ch)
{
	return Upper(Ch);
}

wchar_t __stdcall farLower(wchar_t Ch)
{
	return Lower(Ch);
}

int __stdcall farStrCmpNI(const wchar_t *s1, const wchar_t *s2, int n)
{
	return StrCmpNI(s1, s2, n);
}

int __stdcall farStrCmpI(const wchar_t *s1, const wchar_t *s2)
{
	return StrCmpI(s1, s2);
}

int __stdcall farIsLower(wchar_t Ch)
{
	return IsLower(Ch);
}

int __stdcall farIsUpper(wchar_t Ch)
{
	return IsUpper(Ch);
}

int __stdcall farIsAlpha(wchar_t Ch)
{
	return IsAlpha(Ch);
}

int __stdcall farIsAlphaNum(wchar_t Ch)
{
	return IsAlphaNum(Ch);
}

int WINAPI farGetFileOwner(const wchar_t *Computer,const wchar_t *Name, wchar_t *Owner)
{
	string strOwner;
	int Ret=GetFileOwner(Computer,Name,strOwner);
	wcsncpy(Owner,strOwner,NM);
	return Ret;
}

int WINAPI farPluginsControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
{
	switch (Command)
	{
		case PCTL_LOADPLUGIN:
		case PCTL_UNLOADPLUGIN:
		{
			if (Param1 == PLT_PATH)
			{
				if (Param2 != 0)
				{
					string strPath;
					ConvertNameToFull((const wchar_t *)Param2, strPath);
					if (Command == PCTL_LOADPLUGIN)
						return CtrlObject->Plugins.LoadPlugin(strPath, true);
					else
						return CtrlObject->Plugins.UnloadPluginExternal(strPath);
				}
			}
			break;
	  }
	}

	return 0;
}

int WINAPI farFileFilterControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
{
	FileFilter *Filter=NULL;

	if (Command != FFCTL_CREATEFILEFILTER)
	{
		if (hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		Filter = (FileFilter *)hHandle;
	}

	switch (Command)
	{
		case FFCTL_CREATEFILEFILTER:
		{
			if (Param2 == 0)
				break;

			*((HANDLE *)Param2) = INVALID_HANDLE_VALUE;

			if (hHandle != PANEL_ACTIVE && hHandle != PANEL_PASSIVE)
				break;

			switch (Param1)
			{
				case FFT_PANEL:
				case FFT_FINDFILE:
				case FFT_COPY:
				case FFT_SELECT:
					break;

				default:
					return FALSE;

			}

			Filter = new FileFilter((Panel *)hHandle, (FAR_FILE_FILTER_TYPE)Param1);

			if (Filter)
			{
				*((HANDLE *)Param2) = (HANDLE)Filter;
				return TRUE;
			}

			break;
		}

		case FFCTL_FREEFILEFILTER:
		{
			delete Filter;
			return TRUE;
		}

		case FFCTL_OPENFILTERSMENU:
		{
			return Filter->FilterEdit() ? TRUE : FALSE;
		}

		case FFCTL_STARTINGTOFILTER:
		{
			Filter->UpdateCurrentTime();
			return TRUE;
		}

		case FFCTL_ISFILEINFILTER:
		{
			if (Param2 == 0)
				break;

			return Filter->FileInFilter((const FAR_FIND_DATA *)Param2) ? TRUE : FALSE;
		}
	}

	return FALSE;
}
