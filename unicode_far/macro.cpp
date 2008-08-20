/*
macro.cpp

�������
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

#include "fn.hpp"
#include "macro.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "plugin.hpp"
#include "lockscrn.hpp"
#include "viewer.hpp"
#include "fileedit.hpp"
#include "fileview.hpp"
#include "dialog.hpp"
#include "dlgedit.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "udlist.hpp"
#include "filelist.hpp"
#include "treelist.hpp"
#include "flink.hpp"
#include "TVMStack.hpp"

// ��� ������� ���������� �������
struct DlgParam{
  KeyMacro *Handle;
  DWORD Key;
  int Mode;
};

struct TMacroKeywords MKeywords[] ={
  {0,  L"Other",              MCODE_C_AREA_OTHER,0},
  {0,  L"Shell",              MCODE_C_AREA_SHELL,0},
  {0,  L"Viewer",             MCODE_C_AREA_VIEWER,0},
  {0,  L"Editor",             MCODE_C_AREA_EDITOR,0},
  {0,  L"Dialog",             MCODE_C_AREA_DIALOG,0},
  {0,  L"Search",             MCODE_C_AREA_SEARCH,0},
  {0,  L"Disks",              MCODE_C_AREA_DISKS,0},
  {0,  L"MainMenu",           MCODE_C_AREA_MAINMENU,0},
  {0,  L"Menu",               MCODE_C_AREA_MENU,0},
  {0,  L"Help",               MCODE_C_AREA_HELP,0},
  {0,  L"Info",               MCODE_C_AREA_INFOPANEL,0},
  {0,  L"QView",              MCODE_C_AREA_QVIEWPANEL,0},
  {0,  L"Tree",               MCODE_C_AREA_TREEPANEL,0},
  {0,  L"FindFolder",         MCODE_C_AREA_FINDFOLDER,0},
  {0,  L"UserMenu",           MCODE_C_AREA_USERMENU,0},

  // ������
  {2,  L"Bof",                MCODE_C_BOF,0},
  {2,  L"Eof",                MCODE_C_EOF,0},
  {2,  L"Empty",              MCODE_C_EMPTY,0},
  {2,  L"DisableOutput",      MCODE_C_DISABLEOUTPUT,0},
  {2,  L"Selected",           MCODE_C_SELECTED,0},
  {2,  L"IClip",              MCODE_C_ICLIP,0},

  {2,  L"Far.Width",          MCODE_V_FAR_WIDTH,0},
  {2,  L"Far.Height",         MCODE_V_FAR_HEIGHT,0},
  {2,  L"Far.Title",          MCODE_V_FAR_TITLE,0},
  {2,  L"MacroArea",          MCODE_V_MACROAREA,0},

  {2,  L"ItemCount",          MCODE_V_ITEMCOUNT,0},  // ItemCount - ����� ��������� � ������� �������
  {2,  L"CurPos",             MCODE_V_CURPOS,0},    // CurPos - ������� ������ � ������� �������
  {2,  L"Title",              MCODE_V_TITLE,0},
  {2,  L"Height",             MCODE_V_HEIGHT,0},
  {2,  L"Width",              MCODE_V_WIDTH,0},

  {2,  L"APanel.Empty",       MCODE_C_APANEL_ISEMPTY,0},
  {2,  L"PPanel.Empty",       MCODE_C_PPANEL_ISEMPTY,0},
  {2,  L"APanel.Bof",         MCODE_C_APANEL_BOF,0},
  {2,  L"PPanel.Bof",         MCODE_C_PPANEL_BOF,0},
  {2,  L"APanel.Eof",         MCODE_C_APANEL_EOF,0},
  {2,  L"PPanel.Eof",         MCODE_C_PPANEL_EOF,0},
  {2,  L"APanel.Root",        MCODE_C_APANEL_ROOT,0},
  {2,  L"PPanel.Root",        MCODE_C_PPANEL_ROOT,0},
  {2,  L"APanel.Visible",     MCODE_C_APANEL_VISIBLE,0},
  {2,  L"PPanel.Visible",     MCODE_C_PPANEL_VISIBLE,0},
  {2,  L"APanel.Plugin",      MCODE_C_APANEL_PLUGIN,0},
  {2,  L"PPanel.Plugin",      MCODE_C_PPANEL_PLUGIN,0},
  {2,  L"APanel.FilePanel",   MCODE_C_APANEL_FILEPANEL,0},
  {2,  L"PPanel.FilePanel",   MCODE_C_PPANEL_FILEPANEL,0},
  {2,  L"APanel.Folder",      MCODE_C_APANEL_FOLDER,0},
  {2,  L"PPanel.Folder",      MCODE_C_PPANEL_FOLDER,0},
  {2,  L"APanel.Selected",    MCODE_C_APANEL_SELECTED,0},
  {2,  L"PPanel.Selected",    MCODE_C_PPANEL_SELECTED,0},
  {2,  L"APanel.Left",        MCODE_C_APANEL_LEFT,0},
  {2,  L"PPanel.Left",        MCODE_C_PPANEL_LEFT,0},
  {2,  L"APanel.LFN",         MCODE_C_APANEL_LFN,0},
  {2,  L"PPanel.LFN",         MCODE_C_PPANEL_LFN,0},

  {2,  L"APanel.Type",        MCODE_V_APANEL_TYPE,0},
  {2,  L"PPanel.Type",        MCODE_V_PPANEL_TYPE,0},
  {2,  L"APanel.ItemCount",   MCODE_V_APANEL_ITEMCOUNT,0},
  {2,  L"PPanel.ItemCount",   MCODE_V_PPANEL_ITEMCOUNT,0},
  {2,  L"APanel.CurPos",      MCODE_V_APANEL_CURPOS,0},
  {2,  L"PPanel.CurPos",      MCODE_V_PPANEL_CURPOS,0},
  {2,  L"APanel.Current",     MCODE_V_APANEL_CURRENT,0},
  {2,  L"PPanel.Current",     MCODE_V_PPANEL_CURRENT,0},
  {2,  L"APanel.SelCount",    MCODE_V_APANEL_SELCOUNT,0},
  {2,  L"PPanel.SelCount",    MCODE_V_PPANEL_SELCOUNT,0},
  {2,  L"APanel.Path",        MCODE_V_APANEL_PATH,0},
  {2,  L"PPanel.Path",        MCODE_V_PPANEL_PATH,0},
  {2,  L"APanel.UNCPath",     MCODE_V_APANEL_UNCPATH,0},
  {2,  L"PPanel.UNCPath",     MCODE_V_PPANEL_UNCPATH,0},
  {2,  L"APanel.Height",      MCODE_V_APANEL_HEIGHT,0},
  {2,  L"PPanel.Height",      MCODE_V_PPANEL_HEIGHT,0},
  {2,  L"APanel.Width",       MCODE_V_APANEL_WIDTH,0},
  {2,  L"PPanel.Width",       MCODE_V_PPANEL_WIDTH,0},
  {2,  L"APanel.OPIFlags",    MCODE_V_APANEL_OPIFLAGS,0},
  {2,  L"PPanel.OPIFlags",    MCODE_V_PPANEL_OPIFLAGS,0},
  {2,  L"APanel.DriveType",   MCODE_V_APANEL_DRIVETYPE,0}, // APanel.DriveType - �������� ������: ��� �������
  {2,  L"PPanel.DriveType",   MCODE_V_PPANEL_DRIVETYPE,0}, // PPanel.DriveType - ��������� ������: ��� �������
  {2,  L"APanel.ColumnCount", MCODE_V_APANEL_COLUMNCOUNT,0}, // APanel.ColumnCount - �������� ������:  ���������� �������
  {2,  L"PPanel.ColumnCount", MCODE_V_PPANEL_COLUMNCOUNT,0}, // PPanel.ColumnCount - ��������� ������: ���������� �������

  {2,  L"CmdLine.Bof",        MCODE_C_CMDLINE_BOF,0}, // ������ � ������ cmd-������ ��������������?
  {2,  L"CmdLine.Eof",        MCODE_C_CMDLINE_EOF,0}, // ������ � ������ cmd-������ ��������������?
  {2,  L"CmdLine.Empty",      MCODE_C_CMDLINE_EMPTY,0},
  {2,  L"CmdLine.Selected",   MCODE_C_CMDLINE_SELECTED,0},
  {2,  L"CmdLine.ItemCount",  MCODE_V_CMDLINE_ITEMCOUNT,0},
  {2,  L"CmdLine.CurPos",     MCODE_V_CMDLINE_CURPOS,0},
  {2,  L"CmdLine.Value",      MCODE_V_CMDLINE_VALUE,0},

  {2,  L"Editor.FileName",    MCODE_V_EDITORFILENAME,0},
  {2,  L"Editor.CurLine",     MCODE_V_EDITORCURLINE,0},  // ������� ����� � ��������� (� ���������� � Count)
  {2,  L"Editor.Lines",       MCODE_V_EDITORLINES,0},
  {2,  L"Editor.CurPos",      MCODE_V_EDITORCURPOS,0},
  {2,  L"Editor.RealPos",     MCODE_V_EDITORREALPOS,0},
  {2,  L"Editor.State",       MCODE_V_EDITORSTATE,0},
  {2,  L"Editor.Value",       MCODE_V_EDITORVALUE,0},

  {2,  L"Dlg.ItemType",       MCODE_V_DLGITEMTYPE,0},
  {2,  L"Dlg.ItemCount",      MCODE_V_DLGITEMCOUNT,0},
  {2,  L"Dlg.CurPos",         MCODE_V_DLGCURPOS,0},

  {2,  L"Help.FileName",      MCODE_V_HELPFILENAME, 0},
  {2,  L"Help.Topic",         MCODE_V_HELPTOPIC, 0},
  {2,  L"Help.SelTopic",      MCODE_V_HELPSELTOPIC, 0},

  {2,  L"Drv.ShowPos",        MCODE_V_DRVSHOWPOS,0},
  {2,  L"Drv.ShowMode",       MCODE_V_DRVSHOWMODE,0},

  {2,  L"Viewer.FileName",    MCODE_V_VIEWERFILENAME,0},
  {2,  L"Viewer.State",       MCODE_V_VIEWERSTATE,0},

  {2,  L"Windowed",           MCODE_C_WINDOWEDMODE,0},
};

struct TMacroKeywords MKeywordsArea[] ={
  {0,  L"Other",              MACRO_OTHER,0},
  {0,  L"Shell",              MACRO_SHELL,0},
  {0,  L"Viewer",             MACRO_VIEWER,0},
  {0,  L"Editor",             MACRO_EDITOR,0},
  {0,  L"Dialog",             MACRO_DIALOG,0},
  {0,  L"Search",             MACRO_SEARCH,0},
  {0,  L"Disks",              MACRO_DISKS,0},
  {0,  L"MainMenu",           MACRO_MAINMENU,0},
  {0,  L"Menu",               MACRO_MENU,0},
  {0,  L"Help",               MACRO_HELP,0},
  {0,  L"Info",               MACRO_INFOPANEL,0},
  {0,  L"QView",              MACRO_QVIEWPANEL,0},
  {0,  L"Tree",               MACRO_TREEPANEL,0},
  {0,  L"FindFolder",         MACRO_FINDFOLDER,0},
  {0,  L"UserMenu",           MACRO_USERMENU,0},
  {0,  L"Common",             MACRO_COMMON,0},
};

struct TMacroKeywords MKeywordsFlags[] ={
  // �����
  {1,  L"DisableOutput",      MFLAGS_DISABLEOUTPUT,0},
  {1,  L"RunAfterFARStart",   MFLAGS_RUNAFTERFARSTART,0},
  {1,  L"EmptyCommandLine",   MFLAGS_EMPTYCOMMANDLINE,0},
  {1,  L"NotEmptyCommandLine",MFLAGS_NOTEMPTYCOMMANDLINE,0},
  {1,  L"EVSelection",        MFLAGS_EDITSELECTION,0},
  {1,  L"NoEVSelection",      MFLAGS_EDITNOSELECTION,0},

  {1,  L"NoFilePanels",       MFLAGS_NOFILEPANELS,0},
  {1,  L"NoPluginPanels",     MFLAGS_NOPLUGINPANELS,0},
  {1,  L"NoFolders",          MFLAGS_NOFOLDERS,0},
  {1,  L"NoFiles",            MFLAGS_NOFILES,0},
  {1,  L"Selection",          MFLAGS_SELECTION,0},
  {1,  L"NoSelection",        MFLAGS_NOSELECTION,0},

  {1,  L"NoFilePPanels",      MFLAGS_PNOFILEPANELS,0},
  {1,  L"NoPluginPPanels",    MFLAGS_PNOPLUGINPANELS,0},
  {1,  L"NoPFolders",         MFLAGS_PNOFOLDERS,0},
  {1,  L"NoPFiles",           MFLAGS_PNOFILES,0},
  {1,  L"PSelection",         MFLAGS_PSELECTION,0},
  {1,  L"NoPSelection",       MFLAGS_PNOSELECTION,0},

  {1,  L"ReuseMacro",         MFLAGS_REUSEMACRO,0},
  {1,  L"NoSendKeysToPlugins",MFLAGS_NOSENDKEYSTOPLUGINS,0},
};

int MKeywordsSize = sizeof(MKeywords)/sizeof(*MKeywords);
int MKeywordsFlagsSize = sizeof(MKeywordsFlags)/sizeof(*MKeywordsFlags);

// ������������� ������� - ��� <-> ��� ������������
static struct TKeyCodeName{
  int Key;
  int Len;
  const wchar_t *Name;
} KeyMacroCodes[]={
   { MCODE_OP_AKEY,                 5, L"$AKey"    }, // �������, ������� ������� ������
   { MCODE_OP_DATE,                 5, L"$Date"    }, // $Date "%d-%a-%Y"
   { MCODE_OP_ELSE,                 5, L"$Else"    },
   { MCODE_OP_END,                  4, L"$End"     },
   { MCODE_OP_EXIT,                 5, L"$Exit"    },
   { MCODE_OP_ICLIP,                6, L"$IClip"   },
   { MCODE_OP_IF,                   3, L"$If"      },
   { MCODE_OP_SWITCHKBD,           10, L"$KbdSwitch"},
   { MCODE_OP_MACROMODE,            6, L"$MMode"   },
   { MCODE_OP_REP,                  4, L"$Rep"     },
   { MCODE_OP_SELWORD,              8, L"$SelWord" },
   { MCODE_OP_PLAINTEXT,            5, L"$Text"    }, // $Text "Plain Text"
   { MCODE_OP_WHILE,                6, L"$While"   },
   { MCODE_OP_XLAT,                 5, L"$XLat"    },
};


TVarTable glbVarTable;
TVarTable glbConstTable;

static TVar __varTextDate;
TVMStack VMStack;

// ������� �������������� ���� ������������ � �����
BOOL WINAPI KeyMacroToText(int Key,string &strKeyText0)
{
  string strKeyText;

  for (int I=0;I<int(sizeof(KeyMacroCodes)/sizeof(KeyMacroCodes[0]));I++)
    if (Key==KeyMacroCodes[I].Key)
    {
      strKeyText = KeyMacroCodes[I].Name;
      break;
    }

  if( strKeyText.IsEmpty ())
  {
    strKeyText0=L"";
    return FALSE;
  }

  strKeyText0 = strKeyText;

  return TRUE;
}

// ������� �������������� �������� � ��� ������������
// ������ -1, ���� ��� �����������!
int WINAPI KeyNameMacroToKey(const wchar_t *Name)
{
  // ��������� �� ���� �������������
  for(int I=0; I < int(sizeof(KeyMacroCodes)/sizeof(KeyMacroCodes[0])); ++I)
    if(!StrCmpNI (Name,KeyMacroCodes[I].Name,KeyMacroCodes[I].Len))
      return KeyMacroCodes[I].Key;
  return -1;
}



KeyMacro::KeyMacro()
{
  MacroVersion=GetRegKey(L"KeyMacros",L"MacroVersion",0);
  CurPCStack=-1;
  Work.Init(NULL);
  LockScr=NULL;
  MacroLIB=NULL;
  RecBuffer=NULL;
  Mode=MACRO_SHELL;
  LoadMacros();
}

KeyMacro::~KeyMacro()
{
  InitInternalVars();
  if(Work.AllocVarTable && Work.locVarTable)
    xf_free(Work.locVarTable);
}

void KeyMacro::InitInternalLIBVars()
{
  if(MacroLIB)
  {
    for (int I=0;I<MacroLIBCount;I++)
    {
      if(MacroLIB[I].BufferSize > 1 && MacroLIB[I].Buffer)
        xf_free(MacroLIB[I].Buffer);
      if(MacroLIB[I].Src)
        xf_free(MacroLIB[I].Src);
    }
    xf_free(MacroLIB);
  }
  if(RecBuffer) xf_free(RecBuffer);
  MacroLIBCount=0;
  MacroLIB=NULL;
}

// ������������� ���� ����������
void KeyMacro::InitInternalVars(BOOL InitedRAM)
{
  InitInternalLIBVars();

  if(LockScr)
  {
    delete LockScr;
    LockScr=NULL;
  }

  if(InitedRAM)
  {
    ReleaseWORKBuffer(TRUE);
    Work.Executing=MACROMODE_NOMACRO;
  }

  RecBuffer=NULL;
  RecBufferSize=0;

  Recording=MACROMODE_NOMACRO;
  InternalInput=FALSE;
}

// �������� ���������� ������, ���� �� ���������� �����������
// (����������� - ������ � PlayMacros �������� ������.
void KeyMacro::ReleaseWORKBuffer(BOOL All)
{
  if(Work.MacroWORK)
  {
    if(All || Work.MacroWORKCount <= 1)
    {
      for (int I=0;I<Work.MacroWORKCount;I++)
      {
        if(Work.MacroWORK[I].BufferSize > 1 && Work.MacroWORK[I].Buffer)
          xf_free(Work.MacroWORK[I].Buffer);
        if(Work.MacroWORK[I].Src)
          xf_free(Work.MacroWORK[I].Src);
      }
      xf_free(Work.MacroWORK);
      if(Work.AllocVarTable)
      {
        deleteVTable(*Work.locVarTable);
        //xf_free(Work.locVarTable);
        //Work.locVarTable=NULL;
        //Work.AllocVarTable=false;
      }
      Work.MacroWORK=NULL;
      Work.MacroWORKCount=0;
    }
    else
    {
      if(Work.MacroWORK->BufferSize > 1 && Work.MacroWORK->Buffer)
        xf_free(Work.MacroWORK->Buffer);
      if(Work.MacroWORK->Src)
        xf_free(Work.MacroWORK->Src);
      if(Work.AllocVarTable)
      {
        deleteVTable(*Work.locVarTable);
        //xf_free(Work.locVarTable);
        //Work.locVarTable=NULL;
        //Work.AllocVarTable=false;
      }
      Work.MacroWORKCount--;

      memmove(Work.MacroWORK,((BYTE*)Work.MacroWORK)+sizeof(struct MacroRecord),sizeof(struct MacroRecord)*Work.MacroWORKCount);
      Work.MacroWORK=(struct MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(struct MacroRecord)*Work.MacroWORKCount);
    }
  }
}

// �������� ���� �������� �� �������
int KeyMacro::LoadMacros(BOOL InitedRAM)
{
  int Ret=FALSE;
  InitInternalVars(InitedRAM);

  if(Opt.DisableMacro&MDOL_ALL)
    return Ret;

  string strBuffer;

    int I;
    ReadVarsConst(MACRO_VARS,strBuffer);
    ReadVarsConst(MACRO_CONSTS,strBuffer);
    for(I=MACRO_OTHER; I < MACRO_LAST; ++I)
      if(!ReadMacros(I,strBuffer))
        break;
    // �������� ��� �������� - ���� �� ��� ��� �����������, ��
    // ����� FALSE
    Ret=(I == MACRO_LAST)?TRUE:FALSE;
    if(Ret)
      KeyMacro::Sort();

  return Ret;
}

int KeyMacro::ProcessKey(int Key)
{
  if (InternalInput || Key==KEY_IDLE || Key==KEY_NONE || !FrameManager->GetCurrentFrame())
    return(FALSE);

  if (Recording) // ���� ������?
  {
    if ((unsigned int)Key==Opt.KeyMacroCtrlDot || (unsigned int)Key==Opt.KeyMacroCtrlShiftDot) // ������� ����� ������?
    {
      _KEYMACRO(CleverSysLog Clev(L"MACRO End record..."));
      DWORD MacroKey;
      int WaitInMainLoop0=WaitInMainLoop;
      InternalInput=TRUE;
      WaitInMainLoop=FALSE;

      // �������� _�������_ �����, � �� _��������� �����������_
      FrameManager->GetCurrentFrame()->Lock(); // ������� ���������� ������
      MacroKey=AssignMacroKey();
      FrameManager->ResetLastInputRecord();
      FrameManager->GetCurrentFrame()->Unlock(); // ������ ����� :-)

      // ���������� ����� �� ���������.
      DWORD Flags=MFLAGS_DISABLEOUTPUT; // ???

      // ������� �������� �� ��������
      // ���� �������, �� �� ����� �������� ������ ���������.
      //if (MacroKey != (DWORD)-1 && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
      if (MacroKey != (DWORD)-1 && (unsigned int)Key==Opt.KeyMacroCtrlShiftDot && RecBufferSize)
      {
        if (!GetMacroSettings(MacroKey,Flags))
          MacroKey=(DWORD)-1;
      }
      WaitInMainLoop=WaitInMainLoop0;
      InternalInput=FALSE;

      if (MacroKey==(DWORD)-1)
      {
        if(RecBuffer)  xf_free(RecBuffer);
        RecBuffer=NULL;
      }
      else
      {
        // � ������� common ����� ������ ������ ��� ��������
        int Pos=GetIndex(MacroKey,StartMode,!(RecBuffer && RecBufferSize));
        if (Pos == -1)
        {
          Pos=MacroLIBCount;
          if(RecBufferSize > 0)
          {
            struct MacroRecord *NewMacroLIB=(struct MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));
            if (NewMacroLIB==NULL)
            {
              WaitInFastFind++;
              return(FALSE);
            }
            MacroLIB=NewMacroLIB;
            MacroLIBCount++;
          }
        }
        else
        {
          if(MacroLIB[Pos].BufferSize > 1 && MacroLIB[Pos].Buffer)
            xf_free(MacroLIB[Pos].Buffer);
          if(MacroLIB[Pos].Src)
            xf_free(MacroLIB[Pos].Src);
          MacroLIB[Pos].Buffer=NULL;
          MacroLIB[Pos].Src=NULL;
        }

        if(Pos < MacroLIBCount)
        {
          MacroLIB[Pos].Key=MacroKey;

          if(RecBufferSize > 0)
            RecBuffer[RecBufferSize++]=MCODE_OP_ENDKEYS;

          if(RecBufferSize > 1)
            MacroLIB[Pos].Buffer=RecBuffer;
          else if(RecBuffer && RecBufferSize > 0)
            MacroLIB[Pos].Buffer=reinterpret_cast<DWORD*>((DWORD_PTR)(*RecBuffer));
          else if(!RecBufferSize)
            MacroLIB[Pos].Buffer=NULL;

          MacroLIB[Pos].BufferSize=RecBufferSize;
          MacroLIB[Pos].Src=MkTextSequence(MacroLIB[Pos].Buffer,MacroLIB[Pos].BufferSize);

          // ���� ������� ������ - ������������� StartMode,
          // ����� ������ �� common ������� �� �������, � ������� ��� ������ �������.
          if(!MacroLIB[Pos].BufferSize||!MacroLIB[Pos].Src)
            StartMode=MacroLIB[Pos].Flags&MFLAGS_MODEMASK;

          MacroLIB[Pos].Flags=Flags|(StartMode&MFLAGS_MODEMASK)|MFLAGS_NEEDSAVEMACRO|(Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);
        }
      }

      Recording=MACROMODE_NOMACRO;
      RecBuffer=NULL;
      RecBufferSize=0;
      ScrBuf.RestoreMacroChar();
      WaitInFastFind++;
      KeyMacro::Sort();

      if (Opt.AutoSaveSetup)
        SaveMacros(FALSE); // �������� ������ ���������!

      return(TRUE);
    }
    else // ������� ������ ������������.
    {
      if ((unsigned int)Key>=KEY_NONE && (unsigned int)Key<=KEY_END_SKEY) // ����������� ������� ��������
        return(FALSE);

      RecBuffer=(DWORD *)xf_realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+3));
      if (RecBuffer==NULL)
        return(FALSE);

      if(ReturnAltValue) // "����������" ������ ;-)
        Key|=KEY_ALTDIGIT;

      if(!RecBufferSize)
        RecBuffer[RecBufferSize++]=MCODE_OP_KEYS;

      RecBuffer[RecBufferSize++]=Key;
      return(FALSE);
    }
  }
  else if ((unsigned int)Key==Opt.KeyMacroCtrlDot || (unsigned int)Key==Opt.KeyMacroCtrlShiftDot) // ������ ������?
  {
    _KEYMACRO(CleverSysLog Clev(L"MACRO Begin record..."));
    // ������� 18
    if(Opt.Policies.DisabledOptions&FFPOL_CREATEMACRO)
      return FALSE;
//    if(CtrlObject->Plugins.CheckFlags(PSIF_ENTERTOOPENPLUGIN))
//      return FALSE;

    if(LockScr) delete LockScr;
    LockScr=NULL;

    // ��� ��?
    StartMode=(Mode==MACRO_SHELL && !WaitInMainLoop)?MACRO_OTHER:Mode;
    // ��� ������ - � ������� ������� �������� ���...
    // � ����������� �� ����, ��� ������ ������ ������, ��������� ����� ����� (Ctrl-.
    // � ��������� ������� ����) ��� ����������� (Ctrl-Shift-. - ��� �������� ������ �������)
    Recording=((unsigned int)Key==Opt.KeyMacroCtrlDot) ? MACROMODE_RECORDING_COMMON:MACROMODE_RECORDING;

    if(RecBuffer)
      xf_free(RecBuffer);

    RecBuffer=NULL;
    RecBufferSize=0;
    ScrBuf.ResetShadow();
    ScrBuf.Flush();
    WaitInFastFind--;
    return(TRUE);
  }
  else
  {
    if (Work.Executing == MACROMODE_NOMACRO) // ��� ��� �� ����� ����������?
    {
      //_KEYMACRO(CleverSysLog Clev(L"MACRO find..."));
      //_KEYMACRO(SysLog(L"Param Key=%s",_FARKEY_ToName(Key)));
      DWORD CurFlags;
      if((Key&(~KEY_CTRLMASK)) > 0x01 && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN) // 0xFFFF ??
      {
//        Key=LocalKeyToKey(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
        Key=Upper(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
        //_KEYMACRO(SysLog(L"Upper(Key)=%s",_FARKEY_ToName(Key)));

        if((Key&(~KEY_CTRLMASK)) > 0x7F && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN)
          Key=LocalKeyToKey(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
      }

      int I=GetIndex(Key,(Mode==MACRO_SHELL && !WaitInMainLoop) ? MACRO_OTHER:Mode);
      if(I != -1 && !((CurFlags=MacroLIB[I].Flags)&MFLAGS_DISABLEMACRO) && CtrlObject)
      {
        _KEYMACRO(SysLog(L"[%d] Found KeyMacro (I=%d Key=%s,%s)",__LINE__,I,_FARKEY_ToName(Key),_FARKEY_ToName(MacroLIB[I].Key)));
        if(!CheckAll(Mode,CurFlags))
          return FALSE;

        // ��������� ������� ���������� � MacroWORK
        //PostNewMacro(MacroLIB+I);
        // ��������� �����?
        if (CurFlags&MFLAGS_DISABLEOUTPUT)
        {
          if(LockScr) delete LockScr;
          LockScr=new LockScreen;
        }

        // ��������� ����� ����� (� ��������� ������� ����) ��� ����������� (��� �������� ������ �������)
        Work.ExecLIBPos=0;
        PostNewMacro(MacroLIB+I);
        Work.MacroPC=I;

        IsRedrawEditor=CtrlObject->Plugins.CheckFlags(PSIF_ENTERTOOPENPLUGIN)?FALSE:TRUE;

        _KEYMACRO(SysLog(L"**** Start Of Execute Macro ****"));
        _KEYMACRO(SysLog(1));
        return(TRUE);
      }
    }
    return(FALSE);
  }
}

bool KeyMacro::GetPlainText(string& strDest)
{
  if(!Work.MacroWORK)
    return false;

  struct MacroRecord *MR=Work.MacroWORK;

  int LenTextBuf=(int)(StrLength((wchar_t*)&MR->Buffer[Work.ExecLIBPos]))*sizeof(wchar_t);
  if(LenTextBuf && MR->Buffer[Work.ExecLIBPos])
  {
    strDest=L"";
    strDest=(const wchar_t *)&MR->Buffer[Work.ExecLIBPos];
    _SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
    Work.ExecLIBPos+=(LenTextBuf+sizeof(wchar_t))/sizeof(DWORD);
    _SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
    if(((LenTextBuf+sizeof(wchar_t))%sizeof(DWORD)) != 0)
      ++Work.ExecLIBPos;
    _SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
    return true;
  }
  else
    Work.ExecLIBPos++;
  return false;
}

int KeyMacro::GetPlainTextSize()
{
  if(!Work.MacroWORK)
    return 0;
  struct MacroRecord *MR=Work.MacroWORK;
  return StrLength((wchar_t*)&MR->Buffer[Work.ExecLIBPos]);
}

TVar KeyMacro::FARPseudoVariable(DWORD Flags,DWORD CheckCode,DWORD& Err)
{
  _KEYMACRO(CleverSysLog Clev(L"KeyMacro::FARPseudoVariable()"));
  size_t I;
  TVar Cond(_i64(0));

  string strFileName;

  DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

  // ������ ������ ������� ��������
  for ( I=0 ; I < countof(MKeywords) ; ++I )
    if ( MKeywords[I].Value == CheckCode )
      break;
  if ( I == countof(MKeywords) )
  {
    Err=1;
    _KEYMACRO(SysLog(L"return; Err=%d",Err));
    return Cond; // ����� TRUE �����������, ����� ���������� ����������
                 // �������, ��� ��� �� ���������.
  }

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

  // ������ ������� ����������� ��������
  switch(MKeywords[I].Type)
  {
    case 0: // �������� �� �������
    {
      if(WaitInMainLoop) // ����� ���� ������ ��� ����� WaitInMainLoop, ���� ���� � ���������!!!
       Cond=int(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == FrameManager->GetCurrentFrame()->GetMacroMode()?1:0;
     else
       Cond=int(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == CtrlObject->Macro.GetMode()?1:0;
      break;
    }

    case 2:
    {
      Panel *PassivePanel=NULL;
      if(ActivePanel!=NULL)
        PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
      Frame* CurFrame=FrameManager->GetCurrentFrame();

      switch(CheckCode)
      {
        case MCODE_V_FAR_WIDTH:
          Cond=(__int64)(ScrX+1);
          break;

        case MCODE_V_FAR_HEIGHT:
          Cond=(__int64)(ScrY+1);
          break;

        case MCODE_V_FAR_TITLE:
          apiGetConsoleTitle(strFileName);
          Cond=(const wchar_t*)strFileName;
          break;

        case MCODE_V_MACROAREA:
          Cond=GetSubKey(CtrlObject->Macro.GetMode());
          break;

        case MCODE_C_DISABLEOUTPUT: // DisableOutput?
          Cond=LockScr?1:0;
          break;

        case MCODE_C_WINDOWEDMODE: // Windowed?
          Cond=FarAltEnter(-2)==0?1:0;
          break;

        case MCODE_C_ICLIP:
          Cond=(__int64)UsedInternalClipboard;
          break;

        case MCODE_V_DRVSHOWPOS: // Drv.ShowPos
          Cond=(__int64)Macro_DskShowPosType;
          break;

        case MCODE_V_DRVSHOWMODE: // Drv.ShowMode
          Cond=(__int64)Opt.ChangeDriveMode;
          break;

        case MCODE_C_CMDLINE_BOF:              // CmdLine.Bof - ������ � ������ cmd-������ ��������������?
        case MCODE_C_CMDLINE_EOF:              // CmdLine.Eof - ������ � ������ cmd-������ ��������������?
        case MCODE_C_CMDLINE_EMPTY:            // CmdLine.Empty
        case MCODE_C_CMDLINE_SELECTED:         // CmdLine.Selected
        case MCODE_V_CMDLINE_ITEMCOUNT:        // CmdLine.ItemCount
        case MCODE_V_CMDLINE_CURPOS:           // CmdLine.CurPos
        {
          Cond=CtrlObject->CmdLine->VMProcess(CheckCode);
          break;
        }

        case MCODE_V_CMDLINE_VALUE:            // CmdLine.Value
        {
          CtrlObject->CmdLine->GetString(strFileName);
          Cond=(const wchar_t*)strFileName;
          break;
        }

        case MCODE_C_APANEL_ROOT:  // APanel.Root
        case MCODE_C_PPANEL_ROOT:  // PPanel.Root
        {
          Panel *SelPanel=(CheckCode==MCODE_C_APANEL_ROOT)?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond=SelPanel->VMProcess(MCODE_C_ROOTFOLDER)?1:0;
          break;
        }

        case MCODE_C_APANEL_BOF:
        case MCODE_C_PPANEL_BOF:
        case MCODE_C_APANEL_EOF:
        case MCODE_C_PPANEL_EOF:
        {
          Panel *SelPanel=(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_APANEL_EOF)?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond=SelPanel->VMProcess(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_PPANEL_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;
          break;
        }

        case MCODE_C_SELECTED:    // Selected?
        {
#if 1
          int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS));
          if (CurFrame && CurFrame->GetType()==NeedType)
          {
            int CurSelected;
            if(Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
              CurSelected=(int)CtrlObject->CmdLine->VMProcess(MCODE_C_SELECTED);
            else
              CurSelected=(int)CurFrame->VMProcess(MCODE_C_SELECTED);
            Cond=CurSelected?1:0;
          }
#else
          Frame *f=FrameManager->GetTopModal();
          if(f)
            Cond=(__int64)f->VMProcess(CheckCode);
#endif
          break;
        }

        case MCODE_C_EMPTY:   // Empty
        {
#if 1
          int CurMMode=CtrlObject->Macro.GetMode();
          if(CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACRO_INFOPANEL || CurMMode == MACRO_QVIEWPANEL || CurMMode == MACRO_TREEPANEL))
            Cond=CtrlObject->CmdLine->GetLength()==0?1:0;
          else
          {
            Frame *f=FrameManager->GetTopModal();
            if(f)
              Cond=f->VMProcess(CheckCode);
          }
#else
          Frame *f=FrameManager->GetTopModal();
          if(f)
            Cond=f->VMProcess(CheckCode);
#endif
          break;
        }


        case MCODE_C_BOF:
        case MCODE_C_EOF:
        {
#if 1
          int CurMMode=CtrlObject->Macro.GetMode();
          if(CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACRO_INFOPANEL || CurMMode == MACRO_QVIEWPANEL || CurMMode == MACRO_TREEPANEL))
            Cond=CtrlObject->CmdLine->VMProcess(CheckCode);
          else
          {
            Frame *f=FrameManager->GetTopModal();
            if(f)
              Cond=f->VMProcess(CheckCode);
          }
#else
          Frame *f=FrameManager->GetTopModal();
          if(f)
            Cond=f->VMProcess(CheckCode);
#endif
          break;
        }

        case MCODE_V_DLGITEMCOUNT: // Dlg.ItemCount
        case MCODE_V_DLGCURPOS:    // Dlg.CurPos
        case MCODE_V_DLGITEMTYPE:  // Dlg.ItemType
        {
          if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
          {
            Cond=(__int64)CurFrame->VMProcess(CheckCode);
          }
          break;
        }

        case MCODE_C_APANEL_VISIBLE:  // APanel.Visible
        case MCODE_C_PPANEL_VISIBLE:  // PPanel.Visible
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_VISIBLE?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond = SelPanel->IsVisible()?1:0;
          break;
        }

        case MCODE_C_APANEL_ISEMPTY: // APanel.Empty
        case MCODE_C_PPANEL_ISEMPTY: // PPanel.Empty
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_ISEMPTY?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
          {
            SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);
            int GetFileCount=SelPanel->GetFileCount();
            Cond=(GetFileCount == 0 ||
                 (GetFileCount == 1 && TestParentFolderName(strFileName)))
                 ?1:0;
          }
          break;
        }

        case MCODE_C_APANEL_LFN:
        case MCODE_C_PPANEL_LFN:
        {
          Panel *SelPanel = CheckCode == MCODE_C_APANEL_LFN ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond = SelPanel->GetShowShortNamesMode()?0:1;
          break;
        }

        case MCODE_C_APANEL_LEFT: // APanel.Left
        case MCODE_C_PPANEL_LEFT: // PPanel.Left
        {
          Panel *SelPanel = CheckCode == MCODE_C_APANEL_LEFT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond = SelPanel == CtrlObject->Cp()->LeftPanel ? 1 : 0;
          break;
        }

        case MCODE_C_APANEL_FILEPANEL: // APanel.FilePanel
        case MCODE_C_PPANEL_FILEPANEL: // PPanel.FilePanel
        {
          Panel *SelPanel = CheckCode == MCODE_C_APANEL_FILEPANEL ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond=SelPanel->GetType() == FILE_PANEL;
          break;
        }

        case MCODE_C_APANEL_PLUGIN: // APanel.Plugin
        case MCODE_C_PPANEL_PLUGIN: // PPanel.Plugin
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_PLUGIN?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
            Cond=SelPanel->GetMode() == PLUGIN_PANEL;
          break;
        }

        case MCODE_C_APANEL_FOLDER: // APanel.Folder
        case MCODE_C_PPANEL_FOLDER: // PPanel.Folder
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_FOLDER?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
          {
            SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);
            if(FileAttr != INVALID_FILE_ATTRIBUTES)
              Cond=(FileAttr&FILE_ATTRIBUTE_DIRECTORY)?1:0;
          }
          break;
        }

        case MCODE_C_APANEL_SELECTED: // APanel.Selected
        case MCODE_C_PPANEL_SELECTED: // PPanel.Selected
        {
          Panel *SelPanel=CheckCode==MCODE_C_APANEL_SELECTED?ActivePanel:PassivePanel;
          if(SelPanel!=NULL)
          {
            int SelCount=SelPanel->GetRealSelCount();
            Cond=SelCount >= 1?1:0; //??
          }
          break;
        }

        //- AN -----------------------------------------------------
        //  ���� ����� ���������������� �� ����������� ���� �� �����
        //- AN -----------------------------------------------------
        case MCODE_V_APANEL_CURRENT: // APanel.Current
        case MCODE_V_PPANEL_CURRENT: // PPanel.Current
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURRENT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
          {
            SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);
            if ( FileAttr != INVALID_FILE_ATTRIBUTES )
              Cond = (const wchar_t*)strFileName;
          }
          break;
        }

        case MCODE_V_APANEL_SELCOUNT: // APanel.SelCount
        case MCODE_V_PPANEL_SELCOUNT: // PPanel.SelCount
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_SELCOUNT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond = (__int64)SelPanel->GetRealSelCount();
          break;
        }

        case MCODE_V_APANEL_COLUMNCOUNT:       // APanel.ColumnCount - �������� ������:  ���������� �������
        case MCODE_V_PPANEL_COLUMNCOUNT:       // PPanel.ColumnCount - ��������� ������: ���������� �������
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_COLUMNCOUNT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond = (__int64)SelPanel->GetColumnsCount();
          break;
        }

        case MCODE_V_APANEL_WIDTH: // APanel.Width
        case MCODE_V_PPANEL_WIDTH: // PPanel.Width
        case MCODE_V_APANEL_HEIGHT: // APanel.Height
        case MCODE_V_PPANEL_HEIGHT: // PPanel.Height
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_WIDTH || CheckCode == MCODE_V_APANEL_HEIGHT? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
          {
            int X1, Y1, X2, Y2;
            SelPanel->GetPosition(X1,Y1,X2,Y2);
            if(CheckCode == MCODE_V_APANEL_HEIGHT || CheckCode == MCODE_V_PPANEL_HEIGHT)
              Cond = (__int64)(Y2-Y1+1);
            else
              Cond = (__int64)(X2-X1+1);
          }
          break;
        }

        case MCODE_V_APANEL_OPIFLAGS:  // APanel.OPIFlags
        case MCODE_V_PPANEL_OPIFLAGS:  // PPanel.OPIFlags
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_OPIFLAGS ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
          {
            struct OpenPluginInfo Info;
            SelPanel->GetOpenPluginInfo(&Info);
            Cond = (__int64)Info.Flags;
          }
          break;
        }


        case MCODE_V_APANEL_PATH: // APanel.Path
        case MCODE_V_PPANEL_PATH: // PPanel.Path
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
          {
            SelPanel->GetCurDir(strFileName);
            DeleteEndSlash(strFileName); // - ����� � ����� ����� ���� C:, ����� ����� ������ ���: APanel.Path + "\\file"
            Cond = (const wchar_t*)strFileName;
          }
          break;
        }

        case MCODE_V_APANEL_UNCPATH: // APanel.UNCPath
        case MCODE_V_PPANEL_UNCPATH: // PPanel.UNCPath
        {
          if(_MakePath1(CheckCode == MCODE_V_APANEL_UNCPATH?KEY_ALTSHIFTBRACKET:KEY_ALTSHIFTBACKBRACKET,strFileName,L""))
          {
            UnquoteExternal(strFileName);
            DeleteEndSlash(strFileName);
            Cond = (const wchar_t*)strFileName;
          }
          break;
        }

        //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
        case MCODE_V_APANEL_TYPE: // APanel.Type
        case MCODE_V_PPANEL_TYPE: // PPanel.Type
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_TYPE ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond=(__int64)SelPanel->GetType();
          break;
        }

        case MCODE_V_APANEL_DRIVETYPE: // APanel.DriveType - �������� ������: ��� �������
        case MCODE_V_PPANEL_DRIVETYPE: // PPanel.DriveType - ��������� ������: ��� �������
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_DRIVETYPE ? ActivePanel : PassivePanel;
          Cond=_i64(-1);
          if ( SelPanel != NULL && SelPanel->GetMode() != PLUGIN_PANEL)
          {
            SelPanel->GetCurDir(strFileName);
            GetPathRoot(strFileName, strFileName);
            UINT DriveType=FAR_GetDriveType(strFileName,NULL,0);
            if(IsLocalPath(strFileName))
            {
              string strRemoteName;
              strFileName.SetLength(2);
              if(GetSubstName(DriveType,strFileName,strRemoteName))
                DriveType=DRIVE_SUBSTITUTE;
            }
            Cond=TVar((__int64)DriveType);
          }

          break;
        }

        case MCODE_V_APANEL_ITEMCOUNT: // APanel.ItemCount
        case MCODE_V_PPANEL_ITEMCOUNT: // PPanel.ItemCount
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_ITEMCOUNT ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond=(__int64)SelPanel->GetFileCount();
          break;
        }

        case MCODE_V_APANEL_CURPOS: // APanel.CurPos
        case MCODE_V_PPANEL_CURPOS: // PPanel.CurPos
        {
          Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURPOS ? ActivePanel : PassivePanel;
          if ( SelPanel != NULL )
            Cond=SelPanel->GetCurrentPos()+(SelPanel->GetFileCount()>0?1:0);
          break;
        }

        case MCODE_V_TITLE: // Title
        {
          Frame *f=FrameManager->GetTopModal();
          if(f)
          {
            if(CtrlObject->Cp() == f)
            {
              ActivePanel->GetTitle(strFileName);
            }
            else
            {
              string strType;
              switch(f->GetTypeAndName(strType,strFileName))
              {
                case MODALTYPE_EDITOR:
                case MODALTYPE_VIEWER:
                  f->GetTitle(strFileName);
                  break;
              }
            }
            RemoveExternalSpaces(strFileName);
          }
          Cond=(const wchar_t*)strFileName;
          break;
        }

        case MCODE_V_HEIGHT:  // Height - ������ �������� �������
        case MCODE_V_WIDTH:   // Width - ������ �������� �������
        {
          Frame *f=FrameManager->GetTopModal();
          if(f)
          {
            int X1, Y1, X2, Y2;
            f->GetPosition(X1,Y1,X2,Y2);
            if(CheckCode == MCODE_V_HEIGHT)
              Cond = (__int64)(Y2-Y1+1);
            else
              Cond = (__int64)(X2-X1+1);
          }
          break;
        }

        case MCODE_V_ITEMCOUNT: // ItemCount - ����� ��������� � ������� �������
        case MCODE_V_CURPOS: // CurPos - ������� ������ � ������� �������
        {
          Frame *f=FrameManager->GetTopModal();
          if(f)
          {
/*
            if(f->GetType() == MODALTYPE_VIEWER)
            {
              if(CheckCode == MCODE_V_ITEMCOUNT)
                Cond=(__int64)((FileViewer*)f)->GetViewFileSize();
              else if(CheckCode == MCODE_V_CURPOS)
                Cond=(__int64)((FileViewer*)f)->GetViewFilePos()+1;
            }
            else
              Cond=(__int64)f->VMProcess(CheckCode);
*/
            Cond=f->VMProcess(CheckCode);
          }
          break;
        }
        // *****************

        case MCODE_V_EDITORCURLINE: // Editor.CurLine - ������� ����� � ��������� (� ���������� � Count)
        case MCODE_V_EDITORSTATE:   // Editor.State
        case MCODE_V_EDITORLINES:   // Editor.Lines
        case MCODE_V_EDITORCURPOS:  // Editor.CurPos
        case MCODE_V_EDITORREALPOS: // Editor.RealPos
        case MCODE_V_EDITORFILENAME: // Editor.FileName
        case MCODE_V_EDITORVALUE:   // Editor.Value
        {
          if(CtrlObject->Macro.GetMode()==MACRO_EDITOR &&
             CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
          {
            if(CheckCode == MCODE_V_EDITORFILENAME)
            {
              string strType;
              CtrlObject->Plugins.CurEditor->GetTypeAndName(strType, strFileName);

              Cond=(const wchar_t*)strFileName;
            }
            else if(CheckCode == MCODE_V_EDITORVALUE)
            {
              struct EditorGetString egs;
              egs.StringNumber=-1;
              CtrlObject->Plugins.CurEditor->EditorControl(ECTL_GETSTRING,&egs);
              Cond=egs.StringText;
            }
            else
              Cond=CtrlObject->Plugins.CurEditor->VMProcess(CheckCode);
          }
          break;
        }

        case MCODE_V_HELPFILENAME:  // Help.FileName
        case MCODE_V_HELPTOPIC:     // Help.Topic
        case MCODE_V_HELPSELTOPIC:  // Help.SelTopic
        {
          if(CtrlObject->Macro.GetMode() == MACRO_HELP)
          {
            CurFrame->VMProcess(CheckCode,&strFileName,0);
            Cond=(const wchar_t*)strFileName;
          }
          break;
        }

        case MCODE_V_VIEWERFILENAME: // Viewer.FileName
        case MCODE_V_VIEWERSTATE: // Viewer.State
        {
          if((CtrlObject->Macro.GetMode()==MACRO_VIEWER || CtrlObject->Macro.GetMode()==MACRO_QVIEWPANEL) &&
             CtrlObject->Plugins.CurViewer && CtrlObject->Plugins.CurViewer->IsVisible())
          {
            if(CheckCode == MCODE_V_VIEWERFILENAME)
            {
              CtrlObject->Plugins.CurViewer->GetFileName(strFileName);//GetTypeAndName(NULL,FileName);
              Cond=(const wchar_t*)strFileName;
            }
            else
              Cond=CtrlObject->Plugins.CurViewer->VMProcess(MCODE_V_VIEWERSTATE);
          }
          break;
        }

      }
      break;

    }

    default:
    {
      Err=1;
      break;
    }
  }

  _KEYMACRO(SysLog(L"return; Err=%d",Err));
  return Cond;
}


// S=trim(S[,N])
static bool trimFunc()
{
  int  mode = (int) VMStack.Pop().toInteger();
  TVar Val= VMStack.Pop();

  wchar_t *p = (wchar_t *)Val.toString();
  bool Ret=true;

  switch(mode)
  {
    case 0: p=RemoveExternalSpaces(p); break;  // alltrim
    case 1: p=RemoveLeadingSpaces(p); break;   // ltrim
    case 2: p=RemoveTrailingSpaces(p); break;  // rtrim
    default: Ret=false;
  }

  VMStack.Push((const wchar_t*)p);
  return Ret;
}

// S=substr(S,N1[,N2])
static bool substrFunc()
{
  int  p2 = (int)VMStack.Pop().toInteger();
  int  p1 = (int)VMStack.Pop().toInteger();
  TVar Val= VMStack.Pop();

  wchar_t *p = (wchar_t *)Val.toString();
  bool Ret=false;

  int len = StrLength(p);
  if ( p2 != 0 && p1 >= 0 &&  p1 < len )
  {
    if(p1 > 0)
      p += p1;
    len = StrLength(p);
    if ( ( p2 > 0 ) && ( p2 < len ) )
      p[p2] = 0;
    Ret=true;
    VMStack.Push((const wchar_t*)p);
  }
  else
    VMStack.Push(L"");

  return Ret;
}

#define FLAG_DISK   1
#define FLAG_PATH   2
#define FLAG_NAME   4
#define FLAG_EXT    8

static BOOL SplitFileName (const wchar_t *lpFullName,wchar_t *lpDest,int nFlags)
{
  wchar_t *s = (wchar_t*)lpFullName; //start of sub-string
  wchar_t *p = s; //current string pointer

  wchar_t *es = s+StrLength(s); //end of string
  wchar_t *e; //end of sub-string

  if ( !*p )
      return FALSE;

  if ( (*p == L'\\') && (*(p+1) == L'\\') ) //share
  {
    p += 2;

    p = wcschr(p, L'\\');

    if ( !p )
      return FALSE; //invalid share (\\server\)

    p = wcschr (p+1, L'\\');

    if ( !p )
      p = es;

    if ( (nFlags & FLAG_DISK) == FLAG_DISK )
      xwcsncpy (lpDest, s, p-s);
  }
  else
  {
    if ( *(p+1) == L':' )
    {
      if ( !IsAlpha (*p) )
        return FALSE; // 1:\ is not a valid disk

      p += 2;

      if ( (nFlags & FLAG_DISK) == FLAG_DISK )
        wcsncat (lpDest, s, p-s);
    }
  }

  e = NULL;
  s = p;

  while ( p )
  {
    p = wcschr (p, L'\\');

    if ( p )
    {
      e = p;
      p++;
    }
  };

  if ( e )
  {
    if ( (nFlags & FLAG_PATH) )
      wcsncat (lpDest, s, e-s);

    s = e+1;
    p = s;
  }

  if ( !p )
    p = s;

  e = NULL;

  while ( p )
  {
    p = wcschr (p+1, L'.');

    if ( p )
       e = p;
  }

  if ( !e )
      e = es;

  //FSF.AddEndSlash replacement

  if ( *lpDest && (lpDest[StrLength(lpDest)] != L'\\') ) //hack, just in case of "disk+name" etc.
      wcscat (lpDest, L"\\");

  if ( nFlags & FLAG_NAME )
  {
    wchar_t *ptr=wcspbrk(s,L":");
    if(ptr)
      s=ptr+1;
    wcsncat (lpDest, s, e-s);
  }

  if ( nFlags & FLAG_EXT )
      wcscat (lpDest, e);

  return TRUE;
}


// S=fsplit(S,N)
static bool fsplitFunc()
{
  wchar_t path[NM*2];
  int m = (int)VMStack.Pop().toInteger();
  TVar Val= VMStack.Pop();
  const wchar_t *s = Val.toString();
  bool Ret=false;
  *path=0;
  if(!SplitFileName(s,path,m))
    *path=0;
  else
    Ret=true;
  VMStack.Push(path);
  return Ret;
}

#if 0
// S=Meta("!.!") - � �������� ����� ������ �����������
static bool metaFunc()
{
  TVar Val= VMStack.Pop();
  const wchar_t *s = Val.toString();

  if(s && *s)
  {
    char SubstText[512];
    char Name[NM],ShortName[NM];
    xstrncpy(SubstText,s,sizeof(SubstText)-1);
    SubstFileName(SubstText,sizeof (SubstText),Name,ShortName,NULL,NULL,TRUE);
    return TVar(SubstText);
  }
  return TVar("");
}
#endif


// S=itoa(N[,radix])
static bool itowFunc()
{
  bool Ret=false;

  TVar R = VMStack.Pop();
  TVar N = VMStack.Pop();
  if(N.isInteger())
  {
    wchar_t value[NM];
    int Radix=(int)R.toInteger();
    if(Radix == 0)
      Radix=10;
    Ret=true;
    N=TVar(_i64tow((int)N.toInteger(),value,Radix));
  }
  VMStack.Push(N);

  return Ret;
}

// N=sleep(N)
static bool sleepFunc()
{
  long Period=(long)VMStack.Pop().toInteger();
  if(Period > 0)
  {
    Sleep((DWORD)Period);
    VMStack.Push(_i64(1));
    return true;
  }
  VMStack.Push(_i64(0));
  return false;
}

// N=eval(S)
static bool evalFunc()
{
  bool Ret=true;
  TVar Val= VMStack.Pop();

  struct MacroRecord RBuf;
  int KeyPos;

  CtrlObject->Macro.GetCurRecord(&RBuf,&KeyPos);
  CtrlObject->Macro.PushState(true);
  if(!CtrlObject->Macro.PostNewMacro(Val.toString(),RBuf.Flags&(~MFLAGS_REG_MULTI_SZ),RBuf.Key))
  {
    CtrlObject->Macro.PopState();
    Ret=false;
  }
  VMStack.Push((__int64)__getMacroErrorCode());

  return Ret;
}

// S=waitkey([N])
static bool waitkeyFunc()
{
  long Period=(long)VMStack.Pop().toInteger();
  DWORD Key=WaitKey((DWORD)-1,Period);
  string strKeyText;
  if(Key != KEY_NONE)
   if(!KeyToText(Key,strKeyText))
     strKeyText = L"";
  VMStack.Push((const wchar_t *)strKeyText);
  return !strKeyText.IsEmpty()?true:false;
}

// n=min(n1,n2)
static bool minFunc()
{
  TVar V2 = VMStack.Pop();
  TVar V1 = VMStack.Pop();
  VMStack.Push( V2 < V1  ? V2 : V1);
  return true;
}

// n=max(n1.n2)
static bool maxFunc()
{
  TVar V2 = VMStack.Pop();
  TVar V1 = VMStack.Pop();
  VMStack.Push( V2 > V1  ? V2 : V1);
  return true;
}

// n=modFunc(n1,n2)
static bool modFunc()
{
  TVar V2 = VMStack.Pop();
  TVar V1 = VMStack.Pop();
  if(!V2.i())
  {
    _KEYMACRO(SysLog(L"[%d] modFunc() Error: Divide (mod) by zero",__LINE__));
    VMStack.Push(_i64(0));
    return false;
  }
  VMStack.Push( V1 % V2 );
  return true;
}

// n=iif(expression,n1,n2)
static bool iifFunc()
{
  TVar V2 = VMStack.Pop();
  TVar V1 = VMStack.Pop();
  TVar E  = VMStack.Pop();
  VMStack.Push( E.toInteger() ? V1 : V2 );
  return true;
}

// N=index(S1,S2)
static bool indexFunc()
{
  TVar S2 = VMStack.Pop();
  TVar S1 = VMStack.Pop();
  const wchar_t *s = S1.toString();
  const wchar_t *p = S2.toString();
  const wchar_t *i = StrStrI(s,p);
  bool Ret= i ? true : false;
  VMStack.Push(TVar((__int64)(i ? i-s : -1)));
  return Ret;
}

// S=rindex(S1,S2)
static bool rindexFunc()
{
  TVar S2 = VMStack.Pop();
  TVar S1 = VMStack.Pop();
  const wchar_t *s = S1.toString();
  const wchar_t *p = S2.toString();
  const wchar_t *i = RevStrStrI(s,p);
  bool Ret= i ? true : false;
  VMStack.Push(TVar((__int64)(i ? i-s : -1)));
  return Ret;
}

// S=date(S)
static bool dateFunc()
{
  TVar Val = VMStack.Pop();
  const wchar_t *s = Val.toString();
  bool Ret=false;
  string strTStr;
  if(MkStrFTime(strTStr,s))
    Ret=true;
  else
    strTStr=L"";
  VMStack.Push(TVar((const wchar_t*)strTStr));
  return Ret;
}

// S=xlat(S)
static bool xlatFunc()
{
  TVar Val = VMStack.Pop();
  wchar_t *Str = (wchar_t *)Val.toString();
  bool Ret=::Xlat(Str,0,StrLength(Str),NULL,Opt.XLat.Flags) == NULL?false:true;
  VMStack.Push(TVar((const wchar_t*)Str));
  return Ret;
}

// S=prompt("Title"[,"Prompt"[,flags[, "Src"[, "History"]]]])
static bool promptFunc()
{
  TVar ValHistory = VMStack.Pop();
  TVar ValSrc = VMStack.Pop();
  DWORD Flags = (DWORD)VMStack.Pop().toInteger();
  TVar ValPrompt = VMStack.Pop();
  TVar ValTitle = VMStack.Pop();
  TVar Result(L"");
  bool Ret=false;

  if(!(ValTitle.isInteger() && !ValTitle.i()))
  {
    const wchar_t *history=NULL;
    if(!(ValHistory.isInteger() && !ValHistory.i()))
      history=ValHistory.s();

    const wchar_t *src=L"";
    if(!(ValSrc.isInteger() && !ValSrc.i()))
       src=ValSrc.s();

    const wchar_t *prompt=L"";
    if(!(ValPrompt.isInteger() && !ValPrompt.i()))
       prompt=ValPrompt.s();

    const wchar_t *title=NullToEmpty(ValTitle.toString());

    string strDest;

    if(GetString(title,prompt,history,src,strDest,sizeof(wchar_t)*2048+1,NULL,Flags&~FIB_CHECKBOX,NULL,NULL))
    {
      Result=(const wchar_t *)strDest;
      Result.toString();
      Ret=true;
    }
  }
  VMStack.Push(Result);
  return Ret;
}

// N=msgbox(["Title"[,"Text"[,flags]]])
static bool msgBoxFunc()
{
  DWORD Flags = (DWORD)VMStack.Pop().toInteger();
  TVar ValB = VMStack.Pop();
  TVar ValT = VMStack.Pop();

  const wchar_t *title = L"";
  if(!(ValT.isInteger() && !ValT.i()))
    title=NullToEmpty(ValT.toString());

  const wchar_t *text  = L"";
  if(!(ValB.isInteger() && !ValB.i()))
    text =NullToEmpty(ValB.toString());

  Flags&=~(FMSG_KEEPBACKGROUND|FMSG_ERRORTYPE);
  Flags|=FMSG_ALLINONE;
  if(HIWORD(Flags) == 0 || HIWORD(Flags) > HIWORD(FMSG_MB_RETRYCANCEL))
    Flags|=FMSG_MB_OK;

  //_KEYMACRO(SysLog(L"title='%s'",title));
  //_KEYMACRO(SysLog(L"text='%s'",text));
  string TempBuf = title;
  TempBuf += L"\n";
  TempBuf += text;
  int Result=FarMessageFn(-1,Flags,NULL,(const wchar_t * const *)((const wchar_t *)TempBuf),0,0)+1;
  VMStack.Push((__int64)Result);
  return true;
}


// S=env(S)
static bool environFunc()
{
  TVar S = VMStack.Pop();
  bool Ret=false;
  string strEnv;

  if ( apiGetEnvironmentVariable(S.toString(), strEnv) )
    Ret=true;
  else
    strEnv=L"";

  VMStack.Push((const wchar_t*)strEnv);

  return Ret;
}

static bool _fattrFunc(int Type)
{
  bool Ret=false;
  DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

  if(Type == 0 || Type == 2) // �� ������
  {
    TVar Str = VMStack.Pop();
    UINT  PrevErrMode;
    // ���� �� ���������� ������ ������, ���� ���� �������.
    PrevErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    FileAttr=GetFileAttributesW((wchar_t *)Str.toString());
    SetErrorMode(PrevErrMode);
    Ret=true;
  }
  else
  {
    TVar S = VMStack.Pop();
    int typePanel=(int)VMStack.Pop().toInteger();
    wchar_t *Str = (wchar_t *)S.toString();

    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    Panel *PassivePanel=NULL;
    if(ActivePanel!=NULL)
      PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
    //Frame* CurFrame=FrameManager->GetCurrentFrame();

    Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
    if(SelPanel)
    {
      long Pos=-1;

      if(wcspbrk(Str,L"*?") != NULL)
        Pos=SelPanel->FindFirst(Str);
      else
        Pos=SelPanel->FindFile(Str,wcspbrk(Str,L"\\/:")?FALSE:TRUE);

      if(Pos >= 0)
      {
        string strFileName;
        SelPanel->GetFileName(strFileName,Pos,FileAttr);
        Ret=true;
      }
    }
  }

  if(Type == 2 || Type == 3)
    FileAttr=(FileAttr!=INVALID_FILE_ATTRIBUTES)?1:0;

  VMStack.Push(TVar((__int64)(long)FileAttr));

  return Ret;
}

// N=fattr(S)
static bool fattrFunc()
{
  return _fattrFunc(0);
}

// N=fexist(S)
static bool fexistFunc()
{
  return _fattrFunc(2);
}

// N=panel.fattr(S)
static bool panelfattrFunc()
{
  return _fattrFunc(1);
}

// N=panel.fexist(S)
static bool panelfexistFunc()
{
  return _fattrFunc(3);
}

// N=FLock(Nkey,NState)
/*
  Nkey:
     0 - NumLock
     1 - CapsLock
     2 - ScrollLock

  State:
    -1 get state
     0 off
     1 on
     2 flip
*/
static bool flockFunc()
{
  TVar Ret(-1);

  int stateFLock=(int)VMStack.Pop().toInteger();
  UINT vkKey=(UINT)VMStack.Pop().toInteger();

  switch(vkKey)
  {
    case 0:
      vkKey=VK_NUMLOCK;
      break;
    case 1:
      vkKey=VK_CAPITAL;
      break;
    case 2:
      vkKey=VK_SCROLL;
      break;
    default:
      vkKey=0;
      break;
  }

  if(vkKey)
    Ret=(__int64)SetFLockState(vkKey,stateFLock);

  VMStack.Push(Ret);
  return Ret.i() != _i64(-1);
}

// V=Dlg.GetValue(ID,N)
static bool dlggetvalueFunc()
{
  TVar Ret(-1);

  int TypeInf=(int)VMStack.Pop().toInteger();
  unsigned Index=(unsigned)VMStack.Pop().toInteger()-1;

  Frame* CurFrame=FrameManager->GetCurrentFrame();

  if(CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
  {
    unsigned DlgItemCount=((Dialog*)CurFrame)->GetAllItemCount();
    const struct DialogItemEx **DlgItem=((Dialog*)CurFrame)->GetAllItem();
    if(Index == (unsigned)-1)
    {
      SMALL_RECT Rect;
      if(((Dialog*)CurFrame)->SendDlgMessage((HANDLE)CurFrame,DM_GETDLGRECT,0,(LONG_PTR)&Rect))
      {
        switch(TypeInf)
        {
          case 0: Ret=(__int64)DlgItemCount; break;
          case 2: Ret=(__int64)Rect.Left; break;
          case 3: Ret=(__int64)Rect.Top; break;
          case 4: Ret=(__int64)Rect.Right; break;
          case 5: Ret=(__int64)Rect.Bottom; break;
          case 6: Ret=(__int64)(((Dialog*)CurFrame)->GetDlgFocusPos()+1); break;
        }
      }
    }
    else if(Index < DlgItemCount && DlgItem)
    {
      const struct DialogItemEx *Item=DlgItem[Index];
      int ItemType=Item->Type;
      DWORD ItemFlags=Item->Flags;

      if(TypeInf == 0)
      {
        if(ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
          TypeInf=7;
        else if(ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
        {
          struct FarListGetItem ListItem;
          ListItem.ItemIndex=Item->ListPtr->GetSelectPos();
          if(((Dialog*)CurFrame)->SendDlgMessage((HANDLE)CurFrame,DM_LISTGETITEM,0,(LONG_PTR)&ListItem))
          {
            Ret=(wchar_t *)ListItem.Item.Text;
          }
          else
            Ret=L"";
          TypeInf=-1;
        }
        else
          TypeInf=10;
      }

      switch(TypeInf)
      {
        case 1: Ret=(__int64)ItemType;    break;
        case 2: Ret=(__int64)Item->X1;    break;
        case 3: Ret=(__int64)Item->Y1;    break;
        case 4: Ret=(__int64)Item->X2;    break;
        case 5: Ret=(__int64)Item->Y2;    break;
        case 6: Ret=(__int64)Item->Focus; break;
        case 7:
        {
          if(ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
            Ret=(__int64)Item->Selected;
          else if(ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
          {
            Ret=(__int64)(Item->ListPtr->GetSelectPos()+1);
          }
          else
          {
            Ret=_i64(0);
/*
    int Item->Selected;
    const char *Item->History;
    const char *Item->Mask;
    struct FarList *Item->ListItems;
    int  Item->ListPos;
    CHAR_INFO *Item->VBuf;
*/
          }
          break;
        }
        case 8: Ret=(__int64)ItemFlags; break;
        case 9: Ret=(__int64)Item->DefaultButton; break;
        case 10:
        {
          wchar_t Str[512]=L"";
          // <TODO: ��� UNICODE ����������� �������������� � ����������>
          // if((ItemType == DI_COMBOBOX || ItemType == DI_EDIT) && (ItemFlags&DIF_VAREDIT))
          // ��������: error C2662: 'UnicodeString::GetCharString' : cannot convert 'this' pointer from 'const UnicodeString' to 'UnicodeString &'
          //                    Conversion loses qualifiers
          //Item->strData.GetCharString(Str,sizeof(Str)-1); // UNICODE! BUG!!!
          Ret=(wchar_t *)Str;
          break;
        }
      }
    }
  }

  VMStack.Push(Ret);

  return Ret.i() != _i64(-1);
}

// OldVar=Editor.Set(Idx,Var)
static bool editorsetFunc()
{
  TVar Ret(-1);

  TVar _longState=VMStack.Pop();
  int Index=(int)VMStack.Pop().toInteger();

  if(CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
  {
    long longState=-1L;

    if(Index != 12)
      longState=(long)_longState.toInteger();

    struct EditorOptions EdOpt;
    CtrlObject->Plugins.CurEditor->GetEditorOptions(EdOpt);

    switch(Index)
    {
      case 0:  // TabSize;
        Ret=(__int64)EdOpt.TabSize; break;
      case 1:  // ExpandTabs;
        Ret=(__int64)EdOpt.ExpandTabs; break;
      case 2:  // PersistentBlocks;
        Ret=(__int64)EdOpt.PersistentBlocks; break;
      case 3:  // DelRemovesBlocks;
        Ret=(__int64)EdOpt.DelRemovesBlocks; break;
      case 4:  // AutoIndent;
        Ret=(__int64)EdOpt.AutoIndent; break;
      case 5:  // AutoDetectTable;
        Ret=(__int64)EdOpt.AutoDetectTable; break;
      case 6:  // AnsiTableForNewFile;
        Ret=(__int64)EdOpt.AnsiTableForNewFile; break;
      case 7:  // CursorBeyondEOL;
        Ret=(__int64)EdOpt.CursorBeyondEOL; break;
      case 8:  // BSLikeDel;
        Ret=(__int64)EdOpt.BSLikeDel; break;
      case 9:  // CharCodeBase;
        Ret=(__int64)EdOpt.CharCodeBase; break;
      case 10: // SavePos;
        Ret=(__int64)EdOpt.SavePos; break;
      case 11: // SaveShortPos;
        Ret=(__int64)EdOpt.SaveShortPos; break;
      case 12: // char WordDiv[256];
        Ret=TVar(EdOpt.strWordDiv); break;
      case 13: // F7Rules;
        Ret=(__int64)EdOpt.F7Rules; break;
      case 14: // AllowEmptySpaceAfterEof;
        Ret=(__int64)EdOpt.AllowEmptySpaceAfterEof; break;
      default:
        Ret=-1L;
    }

    if((Index != 12 && longState != -1) || (Index == 12 && _longState.i() == -1))
    {
      switch(Index)
      {
        case 0:  // TabSize;
          EdOpt.TabSize=longState; break;
        case 1:  // ExpandTabs;
          EdOpt.ExpandTabs=longState; break;
        case 2:  // PersistentBlocks;
          EdOpt.PersistentBlocks=longState; break;
        case 3:  // DelRemovesBlocks;
          EdOpt.DelRemovesBlocks=longState; break;
        case 4:  // AutoIndent;
          EdOpt.AutoIndent=longState; break;
        case 5:  // AutoDetectTable;
          EdOpt.AutoDetectTable=longState; break;
        case 6:  // AnsiTableForNewFile;
          EdOpt.AnsiTableForNewFile=longState; break;
        case 7:  // CursorBeyondEOL;
          EdOpt.CursorBeyondEOL=longState; break;
        case 8:  // BSLikeDel;
          EdOpt.BSLikeDel=longState; break;
        case 9:  // CharCodeBase;
          EdOpt.CharCodeBase=longState; break;
        case 10: // SavePos;
          EdOpt.SavePos=longState; break;
        case 11: // SaveShortPos;
          EdOpt.SaveShortPos=longState; break;
        case 12: // char WordDiv[256];
          EdOpt.strWordDiv = _longState.toString(); break;
        case 13: // F7Rules;
          EdOpt.F7Rules=longState; break;
        case 14: // AllowEmptySpaceAfterEof;
          EdOpt.AllowEmptySpaceAfterEof=longState; break;
        default:
          Ret=-1;
          break;
      }
      CtrlObject->Plugins.CurEditor->SetEditorOptions(EdOpt);
      CtrlObject->Plugins.CurEditor->ShowStatus();
    }

  }

  VMStack.Push(Ret);

  return Ret.i() == _i64(-1);
}

// b=msave(var)
static bool msaveFunc()
{
  TVar Val=VMStack.Pop();

  TVarTable *t = &glbVarTable;
  const wchar_t *Name=Val.s();
  if(!Name || *Name!= L'%')
  {
    VMStack.Push(_i64(0));
    return false;
  }

  TVarSet *tmpVarSet=varLook(*t, Name+1);
  if(!tmpVarSet)
  {
    VMStack.Push(_i64(0));
    return false;
  }

  TVar Result=tmpVarSet->value;

  DWORD Ret=(DWORD)-1;
  string strValueName = Val.s();
  switch(Result.type())
  {
    case vtInteger:
    {
      __int64 rrr=Result.toInteger();
      Ret=SetRegKey64(L"KeyMacros\\Vars",strValueName,rrr);
      break;
    }
    case vtString:
    {
      Ret=SetRegKey(L"KeyMacros\\Vars",strValueName,Result.toString());
      break;
    }
  }
  VMStack.Push(TVar(Ret==ERROR_SUCCESS?1:0));
  return Ret==ERROR_SUCCESS;
}

// V=Clip(N[,S])
static bool clipFunc()
{
  TVar Val=VMStack.Pop();
  int cmdType=(int)VMStack.Pop().toInteger();

  // ������������� ������ �������� ������ AS string
  if(Val.isInteger() && Val.i() == 0)
  {
    Val=(const wchar_t *)L"";
    Val.toString();
  }

  int Ret=0;

  switch(cmdType)
  {
    case 0: // Get from Clipboard, "S" - ignore
    {
      wchar_t *ClipText=InternalPasteFromClipboard(0); // 0!  ???
      if(ClipText)
      {
        TVar varClip(ClipText);
        xf_free(ClipText);
        VMStack.Push(varClip);
        return true;
      }
      break;
    }
    case 1: // Put "S" into Clipboard
    {
      Ret=InternalCopyToClipboard(Val.s(),0); // 0!  ???
      VMStack.Push(TVar((__int64)Ret)); // 0!  ???
      return Ret?true:false;
    }
    case 2: // Add "S" into Clipboard
    {
      TVar varClip(Val.s());
      wchar_t *CopyData=InternalPasteFromClipboard(0); // 0!  ???
      if(CopyData)
      {
        size_t DataSize=StrLength(CopyData);
        wchar_t *NewPtr=(wchar_t *)xf_realloc(CopyData,(DataSize+StrLength(Val.s())+2)*sizeof (wchar_t));
        if (NewPtr)
        {
          CopyData=NewPtr;
          wcscpy(CopyData+DataSize,Val.s());
          varClip=CopyData;
          xf_free(CopyData);
        }
        else
          xf_free(CopyData);
      }
      Ret=InternalCopyToClipboard(varClip.s(),0);
      VMStack.Push(TVar((__int64)Ret)); // 0!  ???
      return Ret?true:false;
    }
    case 3: // Copy Win to internal, "S" - ignore
    case 4: // Copy internal to Win, "S" - ignore
    {
      int _UsedInternalClipboard=UsedInternalClipboard;

      {
        TVar varClip(L"");
        UsedInternalClipboard=cmdType-3;
        wchar_t *ClipText=InternalPasteFromClipboard(0); // 0!  ???
        if(ClipText)
        {
          varClip=ClipText;
          xf_free(ClipText);
        }
        UsedInternalClipboard=UsedInternalClipboard==0?1:0;
        Ret=InternalCopyToClipboard(varClip.s(),0); // 0!  ???
      }

      UsedInternalClipboard=_UsedInternalClipboard;
      VMStack.Push(TVar((__int64)Ret)); // 0!  ???
      return Ret?true:false;
    }

  }

  return Ret?true:false;
}


// N=Panel.SetPosIdx(panelType,Idx)
static bool panelsetposidxFunc()
{
  long idxItem=(long)VMStack.Pop().toInteger();
  int typePanel=(int)VMStack.Pop().toInteger();
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *PassivePanel=NULL;
  if(ActivePanel!=NULL)
    PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
  //Frame* CurFrame=FrameManager->GetCurrentFrame();

  Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
  __int64 Ret=0;

  if(SelPanel)
  {
    int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
    if(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
    {
      if(SelPanel->GoToFile(idxItem-1))
      {
        //SelPanel->Show();
        // <Mantis#0000289> - ������, �� �� ������ :-)
        ShellUpdatePanels(SelPanel);
        FrameManager->RefreshFrame(FrameManager->GetTopModal());
        // </Mantis#0000289>
        Ret=(__int64)(SelPanel->GetCurrentPos()+1);
      }
    }
  }
  VMStack.Push(Ret);
  return Ret?true:false;
}

// N=panel.SetPath(panelType,pathName[,fileName])
static bool panelsetpathFunc()
{
  TVar ValFileName=VMStack.Pop();
  TVar Val=VMStack.Pop();
  int typePanel=(int)VMStack.Pop().toInteger();
  __int64 Ret=_i64(0);

  if(!(Val.isInteger() && !Val.i()))
  {
    const wchar_t *pathName=Val.s();

    const wchar_t *fileName=L"";
    if(!ValFileName.isInteger())
      fileName=ValFileName.s();

    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    Panel *PassivePanel=NULL;
    if(ActivePanel!=NULL)
      PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
    //Frame* CurFrame=FrameManager->GetCurrentFrame();

    Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);

    if(SelPanel)
    {
      if(SelPanel->SetCurDir(pathName,TRUE))
      {
                       // Need PointToName()?
        SelPanel->GoToFile(fileName); // ����� ��� ��������, �.�. �������� fileName ��� ������������

        //SelPanel->Show();
        // <Mantis#0000289> - ������, �� �� ������ :-)
        ShellUpdatePanels(SelPanel);
        FrameManager->RefreshFrame(FrameManager->GetTopModal());
        // </Mantis#0000289>
        Ret=_i64(1);
      }
    }
  }

  VMStack.Push(Ret);
  return Ret?true:false;
}

// N=Panel.SetPos(panelType,fileName)
static bool panelsetposFunc()
{
  TVar Val=VMStack.Pop();
  int typePanel=(int)VMStack.Pop().toInteger();
  const wchar_t *fileName=Val.s();
  if(!fileName || !*fileName)
    fileName=L"";

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *PassivePanel=NULL;
  if(ActivePanel!=NULL)
    PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
  //Frame* CurFrame=FrameManager->GetCurrentFrame();

  Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
  __int64 Ret=_i64(0);

  if(SelPanel)
  {
    int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
    if(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
    {
                       // Need PointToName()?
      if(SelPanel->GoToFile(fileName))
      {
        //SelPanel->Show();
        // <Mantis#0000289> - ������, �� �� ������ :-)
        ShellUpdatePanels(SelPanel);
        FrameManager->RefreshFrame(FrameManager->GetTopModal());
        // </Mantis#0000289>
        Ret=(__int64)(SelPanel->GetCurrentPos()+1);
      }
    }
  }

  VMStack.Push(Ret);
  return Ret?true:false;
}

// Result=replace(Str,Find,Replace[,Cnt])
static bool replaceFunc()
{
  TVar Count= VMStack.Pop();
  TVar Repl = VMStack.Pop();
  TVar Find = VMStack.Pop();
  TVar Src  = VMStack.Pop();

  __int64 Ret=_i64(1);

  // TODO: ����� ����� ��������� � ������������ � ��������!
  string strStr;

  int lenS=(int)StrLength(Src.s());
  int lenF=(int)StrLength(Find.s());
  int lenR=(int)StrLength(Repl.s());

  int cnt=0;
  const wchar_t *Ptr=Src.s();
  while((Ptr=StrStrI(Ptr,Find.s())) != NULL)
  {
    cnt++;
    Ptr+=lenF;
  }

  if(cnt)
  {
    if(lenR > lenF)
      lenS+=cnt*(lenR-lenF+1); //???

    strStr=Src.s();
    cnt=(int)Count.i();
    if(cnt <= 0)
      cnt=-1;
    ReplaceStrings(strStr,Find.s(),Repl.s(),cnt,TRUE);
    VMStack.Push((const wchar_t *)strStr);
  }
  else
    VMStack.Push(Src);

  return Ret?true:false;
}

// V=PanelItem(typePanel,Index,TypeInfo)
static bool panelitemFunc()
{
  TVar P2=VMStack.Pop();
  TVar P1=VMStack.Pop();
  int typePanel=(int)VMStack.Pop().toInteger();

  TVar Ret(_i64(0));

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *PassivePanel=NULL;
  if(ActivePanel!=NULL)
    PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
  //Frame* CurFrame=FrameManager->GetCurrentFrame();

  Panel *SelPanel = typePanel == 0 ? ActivePanel : (typePanel == 1?PassivePanel:NULL);
  if(!SelPanel)
  {
    VMStack.Push(Ret);
    return false;
  }

  int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
  if(!(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL))
  {
    VMStack.Push(Ret);
    return false;
  }

  int Index=(int)(P1.toInteger())-1;
  int TypeInfo=(int)P2.toInteger();

  struct FileListItem filelistItem;
  if(TypePanel == TREE_PANEL)
  {
    struct TreeItem treeItem;

    if(SelPanel->GetItem(Index,&treeItem) && !TypeInfo)
    {
      VMStack.Push(TVar(treeItem.strName));
      return true;
    }
  }
  else
  {
    string strDate, strTime;

    if(!SelPanel->GetItem(Index,&filelistItem))
      TypeInfo=-1;
    switch(TypeInfo)
    {
      case 0:  // Name
        Ret=TVar(filelistItem.strName);
        break;
      case 1:  // ShortName
        Ret=TVar(filelistItem.strShortName);
        break;
      case 2:  // FileAttr
        Ret=TVar((__int64)(long)filelistItem.FileAttr);
        break;
      case 3:  // CreationTime
        ConvertDate(filelistItem.CreationTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
        strDate += L" ";
        strDate += strTime;
        Ret=TVar((const wchar_t*)strDate);
        break;
      case 4:  // AccessTime
        ConvertDate(filelistItem.AccessTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
        strDate += L" ";
        strDate += strTime;
        Ret=TVar((const wchar_t*)strDate);
        break;
      case 5:  // WriteTime
        ConvertDate(filelistItem.WriteTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
        strDate += L" ";
        strDate += strTime;
        Ret=TVar((const wchar_t*)strDate);
        break;
      case 6:  // UnpSize
        Ret=TVar(filelistItem.UnpSize);
        break;
      case 7:  // PackSize
        Ret=TVar(filelistItem.PackSize);
        break;
      case 8:  // Selected
        Ret=TVar((__int64)((DWORD)filelistItem.Selected));
        break;
      case 9:  // NumberOfLinks
        Ret=TVar((__int64)filelistItem.NumberOfLinks);
        break;
      case 10:  // SortGroup
        Ret=TVar((__int64)filelistItem.SortGroup);
        break;
      case 11:  // DizText
      {
        const wchar_t *LPtr=filelistItem.DizText;
        Ret=TVar(LPtr);
        break;
      }
      case 12:  // Owner
        Ret=TVar(filelistItem.strOwner);
        break;
      case 13:  // CRC32
        Ret=TVar((__int64)filelistItem.CRC32);
        break;
      case 14:  // Position
        Ret=TVar((__int64)filelistItem.Position);
        break;

      case 15:  // CreationTime (FILETIME)
        Ret=TVar((__int64)FileTimeToUI64(&filelistItem.CreationTime));
        break;
      case 16:  // AccessTime (FILETIME)
        Ret=TVar((__int64)FileTimeToUI64(&filelistItem.AccessTime));
        break;
      case 17:  // WriteTime (FILETIME)
        Ret=TVar((__int64)FileTimeToUI64(&filelistItem.WriteTime));
        break;
    }
  }

  VMStack.Push(Ret);
  return false;
}

static bool lenFunc()
{
  VMStack.Push(TVar(StrLength(VMStack.Pop().toString())));
  return true;
}

static bool ucaseFunc()
{
  TVar Val=VMStack.Pop();
  StrUpper((wchar_t *)Val.toString());
  VMStack.Push(Val);
  return true;
}

static bool lcaseFunc()
{
  TVar Val=VMStack.Pop();
  StrLower((wchar_t *)Val.toString());
  VMStack.Push(Val);
  return true;
}

static bool stringFunc()
{
  VMStack.Push(VMStack.Pop().toString());
  return true;
}

static bool intFunc()
{
  VMStack.Push(VMStack.Pop().toInteger());
  return true;
}

static bool absFunc()
{
  TVar tmpVar=VMStack.Pop();
  if ( tmpVar.isInteger() )
  {
    __int64 v=tmpVar.i();
    if(v < 0)
      v=-v;
    tmpVar = v;
  }
  VMStack.Push(tmpVar);
  return true;
}

static bool ascFunc()
{
  TVar tmpVar=VMStack.Pop();
  if ( tmpVar.isString() )
  {
    tmpVar = (__int64)((DWORD)((WORD)*tmpVar.toString()));
    tmpVar.toInteger();
  }
  VMStack.Push(tmpVar);
  return true;
}

static bool chrFunc()
{
  TVar tmpVar=VMStack.Pop();
  if ( tmpVar.isInteger() )
  {
    __int64 val=tmpVar.i();
    wchar_t tmp[2]={0,0};
    tmp[0]=(wchar_t)(val&0xFFFF); //????
    tmpVar = (const wchar_t *)tmp;
    tmpVar.toString();
  }
  VMStack.Push(tmpVar);
  return true;
}


const wchar_t *eStackAsString(int)
{
  const wchar_t *s=__varTextDate.toString();
  return !s?L"":s;
}

int KeyMacro::GetKey()
{
  struct MacroRecord *MR;
  TVar tmpVar;
  TVarSet *tmpVarSet=NULL;

  //_SVS(SysLog(L">KeyMacro::GetKey() InternalInput=%d Executing=%d (%p)",InternalInput,Work.Executing,FrameManager->GetCurrentFrame()));
  if (InternalInput || !FrameManager->GetCurrentFrame())
  {
    //_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
    return 0;
  }

  int RetKey=0;  // ������� ������ ������� 0 - ������ � ���, ��� ����������������������� ���

  if(Work.Executing == MACROMODE_NOMACRO)
  {
    if(!Work.MacroWORK)
    {
      if(CurPCStack >= 0)
      {
        //_KEYMACRO(SysLog(L"[%d] if(CurPCStack >= 0)",__LINE__));
        PopState();
        return RetKey;
      }
      if(Mode==MACRO_EDITOR &&
         IsRedrawEditor &&
         CtrlObject->Plugins.CurEditor &&
         CtrlObject->Plugins.CurEditor->IsVisible() &&
         LockScr)
      {
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
        CtrlObject->Plugins.CurEditor->Show();
      }
      if(CurPCStack < 0)
      {
        if(LockScr) delete LockScr;
        LockScr=NULL;
      }
      if(TitleModified) SetFarTitle(NULL);
      UsedInternalClipboard=0; //??
      //_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
      return RetKey;
    }
/*
    else if(Work.ExecLIBPos>=MR->BufferSize)
    {
      ReleaseWORKBuffer();
      Work.Executing=MACROMODE_NOMACRO;
      return(FALSE);
    }
    else
*/
    //if(Work.MacroWORK)
    {
      Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
      Work.ExecLIBPos=0; //?????????????????????????????????
    }
    //else
    //  return FALSE;
  }

initial:
  if((MR=Work.MacroWORK) == NULL || !MR->Buffer)
  {
    //_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
    return 0; // RetKey; ?????
  }
//_SVS(SysLog(L"KeyMacro::GetKey() initial: Work.ExecLIBPos=%d (%d) %p",Work.ExecLIBPos,MR->BufferSize,Work.MacroWORK));

  // ��������! �������� �����!
  if(!Work.ExecLIBPos && !LockScr && (MR->Flags&MFLAGS_DISABLEOUTPUT))
    LockScr=new LockScreen;

begin:
  if (Work.ExecLIBPos>=MR->BufferSize || MR->Buffer==NULL)
  {
done:
    /*$ 10.08.2000 skv
      If we are in editor mode, and CurEditor defined,
      we need to call this events.
      EE_REDRAW 2 - to notify that text changed.
      EE_REDRAW 0 - to notify that whole screen updated
      ->Show() to actually update screen.

      This duplication take place since ShowEditor method
      will NOT send this event while screen is locked.
    */
    if(Mode==MACRO_EDITOR &&
       IsRedrawEditor &&
       CtrlObject->Plugins.CurEditor &&
       CtrlObject->Plugins.CurEditor->IsVisible() &&
       LockScr)
    {
      CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
      CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
      CtrlObject->Plugins.CurEditor->Show();
    }

    if(CurPCStack < 0 && (Work.MacroWORKCount-1) <= 0) // mantis#351
    {
      if(LockScr) delete LockScr;
      LockScr=NULL;
    }
    UsedInternalClipboard=0; //??
    Work.Executing=MACROMODE_NOMACRO;
    ReleaseWORKBuffer();
    // �������� - "� ���� �� � ��������� ����� ��� �������"?
    if(Work.MacroWORKCount > 0)
    {
      // �������, �������� ��������� �� �����
      Work.ExecLIBPos=0;
    }
    if(TitleModified) SetFarTitle(NULL); // �������� ������ ��������� �� ���������� �������
    //FrameManager->RefreshFrame();
    //FrameManager->PluginCommit();
    _KEYMACRO(SysLog(-1);SysLog(L"[%d] **** End Of Execute Macro ****",__LINE__));
    if(CurPCStack >= 0)
    {
      PopState();
      goto initial;
    }
    return KEY_NONE; // ����� ������!
  }

  if(Work.ExecLIBPos==0)
    Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;


  // Mantis#0000581: �������� ����������� �������� ���������� �������
  {
    INPUT_RECORD rec;
    if(PeekInputRecord(&rec) && rec.EventType==KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
    {
      GetInputRecord(&rec,true);  // ������� �� ������� ��� "�������"...
      Work.KeyProcess=0;
      goto done;                  // ...� ��������� ������.
    }
  }

  DWORD Key=GetOpCode(MR,Work.ExecLIBPos++);

  string value;

  _KEYMACRO(SysLog(L"[%d] IP=%d Op=%08X ==> %s or %s",__LINE__,Work.ExecLIBPos-1,Key,_MCODE_ToName(Key),_FARKEY_ToName(Key)));

  if(Work.KeyProcess && Key != MCODE_OP_ENDKEYS)
  {
    _KEYMACRO(SysLog(L"[%d] IP=%d  %s (Work.KeyProcess && Key != MCODE_OP_ENDKEYS)",__LINE__,Work.ExecLIBPos-1,_FARKEY_ToName(Key)));
    goto return_func;
  }

  switch(Key)
  {
    case MCODE_OP_NOP:
      goto begin;

    case MCODE_OP_KEYS:                    // �� ���� ����� ������� ������ ���� ������
    {
      _KEYMACRO(SysLog(L"MCODE_OP_KEYS"));
      Work.KeyProcess++;
      goto begin;
    }

    case MCODE_OP_ENDKEYS:                 // ������ ���� �����������.
    {
      _KEYMACRO(SysLog(L"MCODE_OP_ENDKEYS"));
      Work.KeyProcess--;
      goto begin;
    }

    case KEY_ALTINS:
    {
      if(RunGraber())
        return KEY_NONE;
      break;
    }

    case MCODE_OP_XLAT:               // $XLat
    {
      return KEY_OP_XLAT;
    }

    case MCODE_OP_SELWORD:            // $SelWord
    {
      return KEY_OP_SELWORD;
    }

    case MCODE_OP_DATE:               // $Date ["format"]
    {
      __varTextDate=VMStack.Pop();
      return KEY_OP_DATE;
    }

    case MCODE_OP_PLAINTEXT:          // $Text "Text"
    {
      if(VMStack.isEmpty())
        return KEY_NONE;
      __varTextDate=VMStack.Pop();
      return KEY_OP_PLAINTEXT;
    }

    case MCODE_OP_EXIT:               // $Exit
      goto done;

    case MCODE_OP_AKEY:               // $AKey
    {
      return MR->Key;
    }

    /* $IClip
       0: MCODE_OP_ICLIP
    */
    case MCODE_OP_ICLIP:              // $IClip
    {
      UsedInternalClipboard=UsedInternalClipboard==0?1:0;
      goto begin;
    }

    case MCODE_OP_SWITCHKBD:          // $KbdSwitch
    {
      if(!hFarWnd)
        InitDetectWindowedMode();
      if(hFarWnd)
      {
        PostMessageW(hFarWnd,WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_FORWARD, 0);
        //if(Flags & XLAT_SWITCHKEYBBEEP)
        //  MessageBeep(0);
      }
      goto begin;
    }

// $Rep (expr) ... $End
// -------------------------------------
//            <expr>
//            MCODE_OP_SAVEREPCOUNT       1
// +--------> MCODE_OP_REP                2   p1=*
// |          <counter>                   3
// |          <counter>                   4
// |          MCODE_OP_JZ  ------------+  5   p2=*+2
// |          ...                      |
// +--------- MCODE_OP_JMP             |
//            MCODE_OP_END <-----------+
    case MCODE_OP_SAVEREPCOUNT:
    {
      // ������� ������������ �������� ��������
      // �� ����� � ������� ��� � ������� �����
      FARINT64 Counter;
      if((Counter.i64=VMStack.Pop().toInteger()) < 0)
        Counter.i64=0;
      SetOpCode(MR,Work.ExecLIBPos+1,Counter.Part.HighPart);
      SetOpCode(MR,Work.ExecLIBPos+2,Counter.Part.LowPart);
      goto begin;
    }

    case MCODE_OP_REP:
    {
      // ������� ������� �������� ��������
      FARINT64 Counter;
      Counter.Part.HighPart=GetOpCode(MR,Work.ExecLIBPos);
      Counter.Part.LowPart=GetOpCode(MR,Work.ExecLIBPos+1);
      // � ������� ��� �� ������� �����
      VMStack.Push(Counter.i64);
      // �������� ��� � ������ �� MCODE_OP_JZ
      Counter.i64--;
      SetOpCode(MR,Work.ExecLIBPos++,Counter.Part.HighPart);
      SetOpCode(MR,Work.ExecLIBPos++,Counter.Part.LowPart);
      goto begin;
    }

    case MCODE_OP_END:
      // ������ ��������� ���� �������� ���������� :)
      goto begin;

    case MCODE_OP_SAVE:
    {
      TVar Val0=VMStack.Pop();
      GetPlainText(value);
      // ����� �������� �����, �.�. ���������� ������� ������ �������, ��� ���������� ����������
      if(!value.IsEmpty())
      {
        TVarTable *t = ( value.At(0) == L'%' ) ? &glbVarTable : Work.locVarTable;
        varInsert(*t, value)->value = Val0;
      }
      goto begin;
    }

    /* $MMode 1
       0: MCODE_OP_MACROMODE   - � ����� ��������� �����
    */
    case MCODE_OP_MACROMODE:          // $MMode 1
      if (Work.ExecLIBPos<MR->BufferSize)
      {
        if(VMStack.Pop().toInteger() == _i64(1)) // �������� ����� ����������� ("DisableOutput").
        {
          DWORD Flags=MR->Flags;
          if(Flags&MFLAGS_DISABLEOUTPUT) // ���� ��� - ������
          {
            if(LockScr) delete LockScr;
            LockScr=NULL;
          }

          SwitchFlags(MR->Flags,MFLAGS_DISABLEOUTPUT);

          if(MR->Flags&MFLAGS_DISABLEOUTPUT) // ���� ���� - �������
          {
            if(LockScr) delete LockScr;
            LockScr=new LockScreen;
          }
        }
        goto begin;
      }
      break;


    case MCODE_OP_DISCARD:    // ������ �������� � ������� �����
      VMStack.Pop();
      goto begin;

    case MCODE_OP_POP:        // 0: pop 1: varname -> ��������� �������� ���������� � ������ �� ������� �����
    {
      tmpVar=VMStack.Pop();
      GetPlainText(value);
      TVarTable *t = ( value.At(0) == L'%' ) ? &glbVarTable : Work.locVarTable;
      tmpVarSet=varLook(*t, value);
      if(tmpVarSet)
        tmpVarSet->value=tmpVar;
      goto begin;
    }

    /*                               ������
        0: MCODE_OP_COPY                 0:   MCODE_OP_PUSHVAR
        1: VarDest                       1:   VarSrc
        ...                              ...
        N: VarSrc                        N:   MCODE_OP_DOIT
        ...                            N+1:   MCODE_OP_SAVE
                                       N+2:   VarDest
                                         ...

    */
    case MCODE_OP_COPY:       // 0: Copy 1: VarDest 2: VarSrc ==>  %a=%d
    {
      GetPlainText(value);
      TVarTable *t = ( value.At(0) == L'%' ) ? &glbVarTable : Work.locVarTable;
      tmpVarSet=varLook(*t, value);
      if(tmpVarSet)
         tmpVar=tmpVarSet->value;
      GetPlainText(value);
      t = ( value.At(0) == L'%' ) ? &glbVarTable : Work.locVarTable;
      tmpVarSet=varLook(*t, value);
      if(tmpVarSet)
         tmpVar=tmpVarSet->value;
      goto begin;
    }

    case MCODE_OP_PUSHINT: // �������� ����� �������� �� ����.
    {
      FARINT64 i64;
      i64.Part.HighPart=GetOpCode(MR,Work.ExecLIBPos++);   //???
      i64.Part.LowPart=GetOpCode(MR,Work.ExecLIBPos++);    //???
      VMStack.Push(i64.i64);
      goto begin;
    }

    case MCODE_OP_PUSHCONST:  // �������� �� ���� ���������.
    {
      GetPlainText(value);
      tmpVarSet=varLook(glbConstTable, value);
      if(tmpVarSet)
        VMStack.Push(tmpVarSet->value);
      else
        VMStack.Push(_i64(0));
      goto begin;
    }

    case MCODE_OP_PUSHVAR: // �������� �� ���� ����������.
    {
      GetPlainText(value);
      TVarTable *t = ( value.At(0) == L'%' ) ? &glbVarTable : Work.locVarTable;
      // %%name - ���������� ����������
      tmpVarSet=varLook(*t, value);
      if(tmpVarSet)
        VMStack.Push(tmpVarSet->value);
      else
        VMStack.Push(_i64(0));
      goto begin;
    }
    case MCODE_OP_PUSHSTR: // �������� �� ���� ������-���������.
      GetPlainText(value);
      VMStack.Push(TVar((const wchar_t*)value));
      goto begin;

    // ��������
    case MCODE_OP_JMP:
      Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      goto begin;

    case MCODE_OP_JZ:
      if ( VMStack.Pop().toInteger() == 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JNZ:
      if ( VMStack.Pop().toInteger() != 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JLT:
      if ( VMStack.Pop().toInteger() < 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JLE:
      if ( VMStack.Pop().toInteger() <= 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JGT:
      if ( VMStack.Pop().toInteger() > 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    case MCODE_OP_JGE:
      if ( VMStack.Pop().toInteger() >= 0 )
        Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
      else
        Work.ExecLIBPos++;
      goto begin;

    // ��������
    case MCODE_OP_NEGATE: VMStack.Push(-VMStack.Pop()); goto begin;
    case MCODE_OP_NOT:    VMStack.Push(!VMStack.Pop()); goto begin;

    case MCODE_OP_LT:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() <  tmpVar); goto begin;
    case MCODE_OP_LE:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() <= tmpVar); goto begin;
    case MCODE_OP_GT:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() >  tmpVar); goto begin;
    case MCODE_OP_GE:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() >= tmpVar); goto begin;
    case MCODE_OP_EQ:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() == tmpVar); goto begin;
    case MCODE_OP_NE:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() != tmpVar); goto begin;

    case MCODE_OP_ADD:    tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() +  tmpVar); goto begin;
    case MCODE_OP_SUB:    tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() -  tmpVar); goto begin;
    case MCODE_OP_MUL:    tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() *  tmpVar); goto begin;
    case MCODE_OP_DIV:
      if(VMStack.Peek() == _i64(0)) //???
      {
        _KEYMACRO(SysLog(L"[%d] IP=%d/0x%08X Error: Divide by zero",__LINE__,Work.ExecLIBPos,Work.ExecLIBPos));
        goto done;
      }
      tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() /  tmpVar);
      goto begin;

    // Logical
    case MCODE_OP_AND:    tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() && tmpVar); goto begin;
    case MCODE_OP_OR:     tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() || tmpVar); goto begin;

    // Bit Op
    case MCODE_OP_BITAND: tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() &  tmpVar); goto begin;
    case MCODE_OP_BITOR:  tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() |  tmpVar); goto begin;
    case MCODE_OP_BITXOR: tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() ^  tmpVar); goto begin;
    case MCODE_OP_BITSHR: tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() >> tmpVar); goto begin;
    case MCODE_OP_BITSHL: tmpVar=VMStack.Pop(); VMStack.Push(VMStack.Pop() << tmpVar); goto begin;

    case MCODE_OP_BITNOT: VMStack.Push(~VMStack.Pop()); goto begin;

    // Function
    case MCODE_F_EVAL: // N=eval(S)
    {
      if(evalFunc())
        goto initial; // �.�.
      goto begin;
    }

    case MCODE_F_AKEY: // V=akey(N)
    {
      TVar tmpVar=VMStack.Pop();
      if(tmpVar.i() == 0)
         tmpVar=(__int64)MR->Key;
      else
      {
         KeyToText(MR->Key,value);
         tmpVar=(const wchar_t*)value;
         tmpVar.toString();
      }
      VMStack.Push(tmpVar);
      goto begin;
    }

    case MCODE_F_BM_ADD:              // N=BM.Add()
    case MCODE_F_BM_CLEAR:            // N=BM.Clear()
    case MCODE_F_BM_NEXT:             // N=BM.Next()
    case MCODE_F_BM_PREV:             // N=BM.Prev()
    case MCODE_F_BM_STAT:             // N=BM.Stat([N])
    case MCODE_F_BM_DEL:              // N=BM.Del([Idx]) - ������� �������� � ��������� �������� (x=0...), -1 - ������� ������� ��������
    case MCODE_F_BM_GET:              // N=BM.Get(Idx,M) - ���������� ���������� ������ (M==0) ��� ������� (M==1) �������� � �������� (Idx=0...)
    {
       TVar p1, p2;
       if(Key == MCODE_F_BM_GET)
         p2=VMStack.Pop();
       if(Key == MCODE_F_BM_GET || Key == MCODE_F_BM_DEL || Key == MCODE_F_BM_GET)
         p1=VMStack.Pop();

       __int64 Result=_i64(0);
       Frame *f=FrameManager->GetCurrentFrame(), *fo=NULL;
       while(f)
       {
         fo=f;
         f=f->GetTopModal();
       }
       if(!f)
         f=fo;

       if(f)
         Result=f->VMProcess(Key,(void*)(LONG_PTR)p2.i(),p1.i());

       VMStack.Push(Result);
       goto begin;
    }

    case MCODE_F_MENU_GETHOTKEY:      // S=gethotkey([N])
    {
       _KEYMACRO(CleverSysLog Clev(L"MCODE_F_MENU_GETHOTKEY"));
       tmpVar=VMStack.Pop();
       if(!tmpVar.isInteger())
         tmpVar=_i64(0);
       int CurMMode=CtrlObject->Macro.GetMode();
       if(CurMMode == MACRO_MAINMENU || CurMMode == MACRO_MENU || CurMMode == MACRO_DISKS || CurMMode == MACRO_USERMENU)
       {
         Frame *f=FrameManager->GetCurrentFrame(), *fo=NULL;
         //f=f->GetTopModal();
         while(f)
         {
           fo=f;
           f=f->GetTopModal();
         }
         if(!f)
           f=fo;

         __int64 Result;

         if(f && (Result=f->VMProcess(MCODE_F_MENU_GETHOTKEY,NULL,tmpVar.i()-1)) != 0)
         {
           wchar_t _value[2];
           _value[0]=_value[1]=0;
           if(Result)
             _value[0]=(wchar_t)Result;
           tmpVar=(const wchar_t*)_value;
         }
         else
           tmpVar=(const wchar_t*)L"";
       }
       else
         tmpVar=(const wchar_t*)L"";

       VMStack.Push(tmpVar);
       goto begin;
    }

    case MCODE_F_MENU_SELECT:      // N=Menu.Select(S[,N])
    case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S)
    {
       _KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_CHECKHOTKEY? L"MCODE_F_MENU_CHECKHOTKEY":L"MCODE_F_MENU_SELECT"));
       __int64 Result=_i64(0);
       __int64 tmpMode=_i64(0);
       if(Key == MCODE_F_MENU_SELECT)
       {
         tmpMode=VMStack.Pop().toInteger();
       }
       tmpVar=VMStack.Pop();
       //const wchar_t *checkStr=tmpVar.toString();
       int CurMMode=CtrlObject->Macro.GetMode();
       if(CurMMode == MACRO_MAINMENU || CurMMode == MACRO_MENU || CurMMode == MACRO_DISKS || CurMMode == MACRO_USERMENU)
       {
         Frame *f=FrameManager->GetCurrentFrame(), *fo=NULL;
         //f=f->GetTopModal();
         while(f)
         {
           fo=f;
           f=f->GetTopModal();
         }
         if(!f)
           f=fo;

         if(f)
           Result=f->VMProcess(Key,(void*)tmpVar.toString());
       }
       VMStack.Push(Result);
       goto begin;
    }

    case MCODE_F_PROMPT:  // S=prompt("Title"[,"Prompt"[,flags[, "Src"[, "History"]]]])
    case MCODE_F_MSGBOX:  // N=msgbox("Title","Text",flags)
    {
        _KEYMACRO(CleverSysLog Clev(Key == MCODE_F_PROMPT?L"MCODE_F_PROMPT":L"MCODE_F_MSGBOX"));
        DWORD Flags=MR->Flags;
        if(Flags&MFLAGS_DISABLEOUTPUT) // ���� ��� - ������
        {
          if(LockScr) delete LockScr;
          LockScr=NULL;
        }
        InternalInput++; // InternalInput - ������������ ����, ����� ������ �� ��������� ���� ����������
        if(Key == MCODE_F_PROMPT)
          promptFunc();
        else
          msgBoxFunc();
        InternalInput--;
        if(Flags&MFLAGS_DISABLEOUTPUT) // ���� ���� - �������
        {
          if(LockScr) delete LockScr;
          LockScr=new LockScreen;
        }
        goto begin;
    }

    default:
    {
      static struct TMCode2Func{
        DWORD Op;
        bool (*Func)();
      } MCode2Func[]={
        {MCODE_F_WAITKEY,waitkeyFunc},  // S=waitkey([N])
        {MCODE_F_ITOA,itowFunc}, // S=itoa(N,radix)
        {MCODE_F_MIN,minFunc},  // N=min(N1,N2)
        {MCODE_F_MOD,modFunc},  // N=mod(N1,N2)
        {MCODE_F_MAX,maxFunc},  // N=max(N1,N2)
        {MCODE_F_IIF,iifFunc},  // V=iif(Condition,V1,V2)
        {MCODE_F_SUBSTR,substrFunc}, // S=substr(S,N1,N2)
        {MCODE_F_TRIM,trimFunc}, // S=trim(S[,N])
        {MCODE_F_RINDEX,rindexFunc}, // S=rindex(S1,S2)
        {MCODE_F_INDEX,indexFunc}, // S=index(S1,S2)
        {MCODE_F_PANELITEM,panelitemFunc},  // V=panelitem(Panel,Index,TypeInfo)
        {MCODE_F_PANEL_SETPOS,panelsetposFunc}, // N=Panel.SetPos(panelType,fileName)
        {MCODE_F_PANEL_SETPATH,panelsetpathFunc}, // N=panel.SetPath(panelType,pathName,fileName)
        {MCODE_F_PANEL_SETPOSIDX,panelsetposidxFunc}, // N=Panel.SetPosIdx(panelType,Idx)
        {MCODE_F_PANEL_FATTR,panelfattrFunc},         // N=Panel.FAttr(panelType,fileMask)
        {MCODE_F_PANEL_FEXIST,panelfexistFunc},        // N=Panel.FExist(panelType,fileMask)
        {MCODE_F_SLEEP,sleepFunc}, // N=Sleep(N)
        {MCODE_F_ENVIRON,environFunc}, // S=env(S)
        {MCODE_F_LEN,lenFunc},  // N=len(S)
        {MCODE_F_UCASE,ucaseFunc}, // S=ucase(S1)
        {MCODE_F_LCASE,lcaseFunc}, // S=lcase(S1)
        {MCODE_F_FEXIST,fexistFunc},  // S=fexist(S)
        {MCODE_F_FLOCK,flockFunc},  // N=FLock(N,N)
        {MCODE_F_FSPLIT,fsplitFunc},  // S=fsplit(S,N)
        {MCODE_F_FATTR,fattrFunc},   // N=fattr(S)
        {MCODE_F_MSAVE,msaveFunc},   // N=msave(S)
        {MCODE_F_DLG_GETVALUE,dlggetvalueFunc},        // V=Dlg.GetValue(ID,N)
        {MCODE_F_EDITOR_SET,editorsetFunc}, // N=Editor.Set(N,Var)
        {MCODE_F_STRING,stringFunc},  // S=string(V)
        {MCODE_F_CLIP,clipFunc}, // V=Clip(N[,S])
        {MCODE_F_INT,intFunc}, // N=int(V)
        {MCODE_F_DATE,dateFunc},  // // S=date(S)
        {MCODE_F_XLAT,xlatFunc}, // S=xlat(S)
        {MCODE_F_ABS,absFunc}, // N=abs(N)
        {MCODE_F_ASC,ascFunc}, // N=asc(S)
        {MCODE_F_CHR,chrFunc}, // S=chr(N)
        {MCODE_F_REPLACE,replaceFunc}, // S=replace(sS,sF,sR)
      };
      int J;
      for(J=0; J < int(sizeof(MCode2Func)/sizeof(MCode2Func[0])); ++J)
        if(MCode2Func[J].Op == Key)
        {
          MCode2Func[J].Func();
          break;
        }

      if(J >= int(sizeof(MCode2Func)/sizeof(MCode2Func[0])))
      {
        DWORD Err=0;
        tmpVar=FARPseudoVariable(MR->Flags, Key, Err);
        if(!Err)
          VMStack.Push(tmpVar);
        else
        {
          break; // ������� ����� ����������
        }
      }
      goto begin;

    } // END default


  } // END: switch(Key)

return_func:
  if(Work.KeyProcess && (Key&KEY_ALTDIGIT)) // "����������" ������ ;-)
  {
    Key&=~KEY_ALTDIGIT;
    ReturnAltValue=1;
  }

#if 0
  if(MR==Work.MacroWORK &&
      ( Work.ExecLIBPos>=MR->BufferSize || Work.ExecLIBPos+1==MR->BufferSize && MR->Buffer[Work.ExecLIBPos]==KEY_NONE) &&
      Mode==MACRO_DIALOG
    )
  {
    RetKey=Key;
    goto done;
  }
#else
  if(MR==Work.MacroWORK && Work.ExecLIBPos>=MR->BufferSize)
  {
    _KEYMACRO(SysLog(-1);SysLog(L"[%d] **** End Of Execute Macro ****",__LINE__));
    ReleaseWORKBuffer();
    Work.Executing=MACROMODE_NOMACRO;
    if(TitleModified)
      SetFarTitle(NULL);

  }
#endif

  return(Key);
}

// ��������� - ���� �� ��� �������?
int KeyMacro::PeekKey()
{
  if (InternalInput || !Work.MacroWORK)
    return(0);

  struct MacroRecord *MR=Work.MacroWORK;
  if((Work.Executing == MACROMODE_NOMACRO && !Work.MacroWORK) || Work.ExecLIBPos >= MR->BufferSize)
    return(FALSE);

  DWORD OpCode=GetOpCode(MR,Work.ExecLIBPos);
  return OpCode;
}

DWORD KeyMacro::SwitchFlags(DWORD& Flags,DWORD Value)
{
  if(Flags&Value) Flags&=~Value;
  else Flags|=Value;
  return Flags;
}


string &KeyMacro::MkRegKeyName(int IdxMacro, string &strRegKeyName)
{
  string strKeyText;
  KeyToText(MacroLIB[IdxMacro].Key, strKeyText);

  strRegKeyName.Format (L"KeyMacros\\%s\\%s%s",
       GetSubKey(MacroLIB[IdxMacro].Flags&MFLAGS_MODEMASK),  (MacroLIB[IdxMacro].Flags&MFLAGS_DISABLEMACRO?L"~":L""),  (const wchar_t*)strKeyText); //BUGBUG
  return strRegKeyName;
}

/*
  ����� ������ ���� ������� ����� ������� ������!!!
  ������� ���������� ������ ������� ������������������, �.�.... �������
  � ��������� ������ ���������� Src
*/
wchar_t *KeyMacro::MkTextSequence(DWORD *Buffer,int BufferSize,const wchar_t *Src)
{
  int J, Key;
  string strMacroKeyText;
  string strTextBuffer;

  if(!Buffer)
    return NULL;

#if 0
  if(BufferSize == 1)
  {
    if(
        (((DWORD)(DWORD_PTR)Buffer)&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (((DWORD)(DWORD_PTR)Buffer)&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
        (((DWORD)(DWORD_PTR)Buffer)&KEY_OP_ENDBASE) >= KEY_OP_BASE && (((DWORD)(DWORD_PTR)Buffer)&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE
      )
    {
      return Src?xf_wcsdup(Src):NULL;
    }

    if(KeyToText((DWORD)(DWORD_PTR)Buffer,strMacroKeyText))
      return xf_wcsdup((const wchar_t*)strMacroKeyText);
    return NULL;
  }
#endif

  strTextBuffer=L"";
  if(Buffer[0] == MCODE_OP_KEYS)
    for (J=1; J < BufferSize; J++)
    {
      Key=Buffer[J];

      if(Key == MCODE_OP_ENDKEYS)
        continue;

      if(/*
          (Key&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (Key&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
          (Key&KEY_OP_ENDBASE) >= KEY_OP_BASE && (Key&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE ||
        */
          !KeyToText(Key,strMacroKeyText)
        )
      {
        return Src?xf_wcsdup(Src):NULL;
      }
      if(J > 1)
        strTextBuffer += L" ";
      strTextBuffer += strMacroKeyText;
    }

  if(!strTextBuffer.IsEmpty())
    return xf_wcsdup((const wchar_t*)strTextBuffer);
  return NULL;
}

// ���������� ���� ��������
void KeyMacro::SaveMacros(BOOL AllSaved)
{
  string strRegKeyName;

  //WriteVarsConst(MACRO_VARS);
  //WriteVarsConst(MACRO_CONSTS);

  for (int I=0;I<MacroLIBCount;I++)
  {
    if(!AllSaved  && !(MacroLIB[I].Flags&MFLAGS_NEEDSAVEMACRO))
      continue;

    MkRegKeyName(I, strRegKeyName);

    if (MacroLIB[I].BufferSize==0 || !MacroLIB[I].Src)
    {
      DeleteRegKey(strRegKeyName);
      continue;
    }
#if 0
    if((TextBuffer=MkTextSequence(MacroLIB[I].Buffer,MacroLIB[I].BufferSize,MacroLIB[I].Src)) == NULL)
      continue;

    SetRegKey(RegKeyName,"Sequence",TextBuffer);
    //_SVS(SysLog(L"%3d) %s|Sequence='%s'",I,RegKeyName,TextBuffer));
    if(TextBuffer)
      xf_free(TextBuffer);
#endif
    BOOL Ok=TRUE;
    if(MacroLIB[I].Flags&MFLAGS_REG_MULTI_SZ)
    {
      int Len=StrLength(MacroLIB[I].Src)+2;
      wchar_t *ptrSrc=(wchar_t *)xf_malloc(Len*sizeof(wchar_t));
      if(ptrSrc)
      {
        wcscpy(ptrSrc,MacroLIB[I].Src);
        for(int J=0; ptrSrc[J]; ++J)
          if(ptrSrc[J] == L'\n')
            ptrSrc[J]=0;
        ptrSrc[Len-1]=0;
        SetRegKey(strRegKeyName,L"Sequence",ptrSrc,Len*sizeof(wchar_t),REG_MULTI_SZ);
        xf_free(ptrSrc);
        Ok=FALSE;
      }
    }

    if(Ok)
      SetRegKey(strRegKeyName,L"Sequence",MacroLIB[I].Src);

    // ����������� ����...
    for(int J=0; J < int(sizeof(MKeywordsFlags)/sizeof(MKeywordsFlags[0])); ++J)
    {
      if (MacroLIB[I].Flags & MKeywordsFlags[J].Value)
        SetRegKey(strRegKeyName,MKeywordsFlags[J].Name,1);
      else
        DeleteRegValue(strRegKeyName,MKeywordsFlags[J].Name);
    }
  }
}


int KeyMacro::WriteVarsConst(int WriteMode)
{
  string strUpKeyName;
  string strValueName;

  strUpKeyName.Format (L"KeyMacros\\%s",(WriteMode==MACRO_VARS?L"Vars":L"Consts"));

  TVarTable *t = (WriteMode==MACRO_VARS)?&glbVarTable:&glbConstTable;

  for (int I=0;I < V_TABLE_SIZE;I++)
    for(int J=0;;++J)
    {
      TVarSet *var=varEnum(*t,I,J);
      if(!var)
        break;
      strValueName = var->str;
      strValueName = (WriteMode==MACRO_VARS?L"%":L"")+strValueName;
      switch(var->value.type())
      {
        case vtInteger:
          SetRegKey64(strUpKeyName,strValueName,var->value.i());
          break;
        case vtString:
          SetRegKey(strUpKeyName,strValueName,var->value.s());
          break;
      }

    }
  return TRUE;
}

/*
   KeyMacros\\Vars
     "StringName":REG_SZ
     "IntName":REG_DWORD
*/

int KeyMacro::ReadVarsConst(int ReadMode, string &strSData)
{
  int I;
  string strUpKeyName;
  string strValueName;
  long IData;
  __int64 IData64;

  strUpKeyName.Format (L"KeyMacros\\%s",(ReadMode==MACRO_VARS?L"Vars":L"Consts"));
  TVarTable *t = (ReadMode==MACRO_VARS)?&glbVarTable:&glbConstTable;

  for (I=0;;I++)
  {
    IData=0;
    strValueName=L"";
    strSData=L"";

    int Type=EnumRegValueEx(strUpKeyName,I,strValueName,strSData,(LPDWORD)&IData,(__int64*)&IData64);

    if (Type == REG_NONE)
      break;

    if( ReadMode == MACRO_VARS &&  ! ( strValueName.At(0) == L'%' && strValueName.At(1) == L'%' ) )
      continue;

    if (Type == REG_SZ)
      varInsert(*t, (const wchar_t*)strValueName+1)->value = (const wchar_t*)strSData;
    else if (Type == REG_DWORD)
      varInsert(*t, (const wchar_t*)strValueName+1)->value = (__int64)IData;
    else if (Type == REG_QWORD)
      varInsert(*t, (const wchar_t*)strValueName+1)->value = IData64;
  }
  return TRUE;
}

/*
   KeyMacros\Function
*/
int KeyMacro::ReadMacroFunction(int ReadMode, string &strBuffer)
{
  if(ReadMode != MACRO_FUNC) // ���� ��� :-)
    return FALSE;
  return TRUE;
}

int KeyMacro::ReadMacros(int ReadMode, string &strBuffer)
{
  int I, J;
  struct MacroRecord CurMacro;
  memset(&CurMacro,0,sizeof(CurMacro));

  string strUpKeyName;
  string strRegKeyName, strKeyText;

  strUpKeyName.Format (L"KeyMacros\\%s", GetSubKey(ReadMode));

  for (I=0;;I++)
  {
    DWORD MFlags=0;

    if (!EnumRegKey(strUpKeyName,I,strRegKeyName))
      break;

    const wchar_t *KeyNamePtr=wcsrchr(strRegKeyName, L'\\');
    if (KeyNamePtr!=NULL)
    {
      strKeyText = KeyNamePtr+1;
      // ������! ��� �������� �������, ������������ �� ������ ~ - ���
      // ������������� ������!!!
      if( strKeyText.At (0) == L'~' && strKeyText.At(1) )
      {
          wchar_t *KeyText=strKeyText.GetBuffer();

          wchar_t *Ptr = KeyText+1;

          while(*Ptr && *Ptr == L'~')// && IsSpace(KeyText[1]))
             ++Ptr;
          wmemmove(KeyText,Ptr,StrLength(Ptr)+1);

          strKeyText.ReleaseBuffer ();

          MFlags|=MFLAGS_DISABLEMACRO;
      }
    }
    else
      strKeyText= L"";

    int KeyCode=KeyNameToKey(strKeyText);
    if (KeyCode==-1)
      continue;

    DWORD regType=0;
    if(GetRegKey(strRegKeyName,L"Sequence",strBuffer,L"",&regType) && regType == REG_MULTI_SZ)
    {
      // ��������� ��� �� REG_MULTI_SZ
      wchar_t *ptrBuffer = strBuffer.GetBuffer ();
      while(1)
      {
        ptrBuffer+=StrLength(ptrBuffer);
        if(!ptrBuffer[0] && !ptrBuffer[1])
          break;
        *ptrBuffer=L'\n';
      }
      strBuffer.ReleaseBuffer ();
    }

    RemoveExternalSpaces(strBuffer);

    if( strBuffer.IsEmpty() )
      continue;

    CurMacro.Key=KeyCode;
    CurMacro.Buffer=NULL;
    CurMacro.Src=NULL;
    CurMacro.BufferSize=0;
    CurMacro.Flags=MFlags|(ReadMode&MFLAGS_MODEMASK)|(regType == REG_MULTI_SZ?MFLAGS_REG_MULTI_SZ:0);

    for(J=0; J < int(sizeof(MKeywordsFlags)/sizeof(MKeywordsFlags[0])); ++J)
      CurMacro.Flags|=GetRegKey(strRegKeyName,MKeywordsFlags[J].Name,0)?MKeywordsFlags[J].Value:0;

    if(ReadMode == MACRO_EDITOR || ReadMode == MACRO_DIALOG || ReadMode == MACRO_VIEWER)
    {
      if(CurMacro.Flags&MFLAGS_SELECTION)
      {
        CurMacro.Flags&=~MFLAGS_SELECTION;
        CurMacro.Flags|=MFLAGS_EDITSELECTION;
      }
      if(CurMacro.Flags&MFLAGS_NOSELECTION)
      {
        CurMacro.Flags&=~MFLAGS_NOSELECTION;
        CurMacro.Flags|=MFLAGS_EDITNOSELECTION;
      }
    }

    if(!ParseMacroString(&CurMacro,strBuffer))
      continue;

    struct MacroRecord *NewMacros=(struct MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));
    if (NewMacros==NULL)
    {
      return FALSE;
    }
    MacroLIB=NewMacros;
    CurMacro.Src=xf_wcsdup(strBuffer);
    memcpy(MacroLIB+MacroLIBCount,&CurMacro,sizeof(CurMacro));
    MacroLIBCount++;
  }
  return TRUE;
}

// ��� ������� ����� ���������� �� ��� �������, ������� ����� ���������� ��������
void KeyMacro::RestartAutoMacro(int /*Mode*/)
{
#if 0
/*
�������      �������
-------------------------------------------------------
Other         0
Shell         1 ���, ��� ������� ����
Viewer        ��� ������ ����� ����� �������
Editor        ��� ������ ����� ����� ��������
Dialog        0
Search        0
Disks         0
MainMenu      0
Menu          0
Help          0
Info          1 ���, ��� ������� ���� � ����������� ����� ������
QView         1 ���, ��� ������� ���� � ����������� ����� ������
Tree          1 ���, ��� ������� ���� � ����������� ����� ������
Common        0
*/
#endif
}

// �������, ����������� ������� ��� ������ ����
// ���� �� ��������� �������������� � �������������� ���������
// �������� ��������, �� ������ ����!
void KeyMacro::RunStartMacro()
{
  if(Opt.DisableMacro&MDOL_ALL)
    return;

  if(Opt.DisableMacro&MDOL_AUTOSTART)
    return;

// �������� ������� ������ �������
#if 1
  if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !CmdMode && CtrlObject->Plugins.IsPluginsLoaded()))
    return;

  static int IsRunStartMacro=FALSE;
  if(IsRunStartMacro)
    return;
  IsRunStartMacro=TRUE;

  if(!IndexMode[MACRO_SHELL][1])
    return;

  struct MacroRecord *MR=MacroLIB+IndexMode[MACRO_SHELL][0];
  for(int I=0; I < IndexMode[MACRO_SHELL][1]; ++I)
  {
    DWORD CurFlags;
    if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==MACRO_SHELL &&
        MR[I].BufferSize>0 &&
        // ��������� �� ������������� �������
        !(CurFlags&MFLAGS_DISABLEMACRO) &&
        (CurFlags&MFLAGS_RUNAFTERFARSTART) && CtrlObject)
    {
      if(CheckAll(MACRO_SHELL,CurFlags))
        PostNewMacro(MR+I);
    }
  }
#else
  static int AutoRunMacroStarted=FALSE;
  if(AutoRunMacroStarted || !MacroLIB || !IndexMode[Mode][1])
    return;

  //if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !CmdMode && CtrlObject->Plugins.IsPluginsLoaded()))
  if (!(CtrlObject && CtrlObject->Plugins.IsPluginsLoaded()))
    return;

  int I;
  struct MacroRecord *MR=MacroLIB+IndexMode[Mode][0];
  for(I=0; I < IndexMode[Mode][1]; ++I)
  {
    DWORD CurFlags;
    if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==Mode &&   // ���� ������ �� ���� �����?
        MR[I].BufferSize > 0 &&                             // ���-�� ������ ����
        !(CurFlags&MFLAGS_DISABLEMACRO) &&                  // ��������� �� ������������� �������
        (CurFlags&MFLAGS_RUNAFTERFARSTART) &&               // � ���� ��, ��� ������ ����������
        !(CurFlags&MFLAGS_RUNAFTERFARSTARTED)      // � ��� �����, ������� ��� �� ����������
       )
    {
      if(CheckAll(Mode,CurFlags)) // ������ ��� ��������� - �������� �����
      {
        PostNewMacro(MR+I);
        MR[I].Flags|=MFLAGS_RUNAFTERFARSTARTED; // ���� ������ ������� �������� �� �����
      }
    }
  }

  // ��������� ���������� ���������� �������������� ��������
  int CntStart=0;
  for(I=0; I < MacroLIBCount; ++I)
    if((MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTART) && !(MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTARTED))
      CntStart++;

  if(!CntStart) // ������ ����� �������, ��� ��� ���������� � � ������� RunStartMacro() ������ ������
    AutoRunMacroStarted=TRUE;

#endif
  Work.ExecLIBPos=0;
}

// ���������� ����������� ���� ���������� �������
LONG_PTR WINAPI KeyMacro::AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  string strKeyText;
  static int LastKey=0;
  static struct DlgParam *KMParam=NULL;
  int Index, I;
  //_SVS(SysLog(L"LastKey=%d Msg=%s",LastKey,_DLGMSG_ToName(Msg)));
  if(Msg == DN_INITDIALOG)
  {
    KMParam=(struct DlgParam *)Param2;
    LastKey=0;

    // <�������, ������� �� ������� � ������� ����������>
    static const wchar_t * const PreDefKeyName[]={
      L"CtrlDown", L"Enter", L"Esc", L"F1", L"CtrlF5",
      L"CtrlMsWheelUp",L"ShiftMsWheelUp",L"AltMsWheelUp",L"CtrlShiftMsWheelUp",L"CtrlAltMsWheelUp",L"AltShiftMsWheelUp",
      L"CtrlMsWheelDown",L"ShiftMsWheelDown",L"AltMsWheelDown",L"CtrlShiftMsWheelDown",L"CtrlAltMsWheelDown",L"AltShiftMsWheelDown",
      L"CtrlMsWheelLeft",L"ShiftMsWheelLeft",L"AltMsWheelLeft",L"CtrlShiftMsWheelLeft",L"CtrlAltMsWheelLeft",L"AltShiftMsWheelLeft",
      L"CtrlMsWheelRight",L"ShiftMsWheelRight",L"AltMsWheelRight",L"CtrlShiftMsWheelRight",L"CtrlAltMsWheelRight",L"AltShiftMsWheelRight"
    };
    for(I=0; I < int(sizeof(PreDefKeyName)/sizeof(PreDefKeyName[0])); ++I)
      Dialog::SendDlgMessage(hDlg,DM_LISTADDSTR,2,(LONG_PTR)PreDefKeyName[I]);
/*
    int KeySize=GetRegKeySize("KeyMacros","DlgKeys");
    char *KeyStr;
    if(KeySize &&
       (KeyStr=(char*)xf_malloc(KeySize+1)) != NULL &&
       GetRegKey("KeyMacros","DlgKeys",KeyStr,"",KeySize)
      )
    {
      UserDefinedList KeybList;
      if(KeybList.Set(KeyStr))
      {
        KeybList.Start();
        const char *OneKey;
        *KeyText=0;
        while(NULL!=(OneKey=KeybList.GetNext()))
        {
          xstrncpy(KeyText, OneKey, sizeof(KeyText)-1);
          Dialog::SendDlgMessage(hDlg,DM_LISTADDSTR,2,(long)KeyText);
        }
      }
      xf_free(KeyStr);
    }
*/
    Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)L"");
    // </�������, ������� �� ������� � ������� ����������>

  }
  else if(Param1 == 2 && Msg == DN_EDITCHANGE)
  {
    LastKey=0;

      _SVS(SysLog(L"[%d] ((FarDialogItem*)Param2)->PtrData='%s'",__LINE__,((FarDialogItem*)Param2)->PtrData));
      Param2=KeyNameToKey(((FarDialogItem*)Param2)->PtrData);

    if(Param2 != -1)
      goto M1;
  }
  else if ( Msg == DN_KEY && (((Param2&KEY_END_SKEY) < KEY_END_FKEY) ||
   (((Param2&KEY_END_SKEY) > INTERNAL_KEY_BASE) && (Param2&KEY_END_SKEY) < INTERNAL_KEY_BASE_2)) )
  {
//    if((Param2&0x00FFFFFF) >= 'A' && (Param2&0x00FFFFFF) <= 'Z' && ShiftPressed)
//      Param2|=KEY_SHIFT;

//_SVS(SysLog(L"Macro: Key=%s",_FARKEY_ToName(Param2)));
    // <��������� ������ ������: F1 & Enter>
    // Esc & (Enter � ���������� Enter) - �� ������������
    if(Param2 == KEY_ESC ||
       ((Param2 == KEY_ENTER||Param2 == KEY_NUMENTER) && (LastKey == KEY_ENTER||LastKey == KEY_NUMENTER)) ||
       Param2 == KEY_CTRLDOWN ||
       Param2 == KEY_F1
      )
    {
      return FALSE;
    }
/*
    // F1 - ������ ������ - ����� ���� 2 ����
    // ������ ��� ����� ������� ����,
    // � ������ ��� - ������ ��� ��� ����������
    if(Param2 == KEY_F1 && LastKey!=KEY_F1)
    {
      LastKey=KEY_F1;
      return FALSE;
    }
*/
    // ���� ���-�� ��� ������ � Enter`�� ������������
    _SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)Param2),(LastKey?_FARKEY_ToName(LastKey):L"")));
    if((Param2 == KEY_ENTER||Param2 == KEY_NUMENTER) && LastKey && !(LastKey == KEY_ENTER||LastKey == KEY_NUMENTER))
      return FALSE;
    // </��������� ������ ������: F1 & Enter>
M1:
    _SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)Param2),LastKey?_FARKEY_ToName(LastKey):L""));
    KeyMacro *MacroDlg=KMParam->Handle;

    // -- if((Param2&0x00FFFFFF) > 0x7F && (Param2&0x00FFFFFF) < 0xFF)
    if((Param2&0x00FFFFFF) > 0 && (Param2&0x00FFFFFF) < 0xFFFF)
      Param2=LocalKeyToKey((int)(Param2&0x0000FFFF))|(DWORD)(Param2&(~0x0000FFFF));
    _SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)Param2),LastKey?_FARKEY_ToName(LastKey):L""));
    KMParam->Key=(DWORD)Param2;
    KeyToText((int)Param2,strKeyText);

    // ���� ��� ���� ����� ������...
    if((Index=MacroDlg->GetIndex((int)Param2,KMParam->Mode)) != -1)
    {
      struct MacroRecord *Mac=MacroDlg->MacroLIB+Index;
      // ����� ������� ��������� ������ ��� ��������.
      if(!MacroDlg->RecBuffer || !MacroDlg->RecBufferSize || (Mac->Flags&0xFF)!=MACRO_COMMON)
      {
        DWORD DisFlags=Mac->Flags&MFLAGS_DISABLEMACRO;
        string strBuf;
        string strBufKey;
        string strRegKeyName;
        string strTextBuffer;

        MacroDlg->MkRegKeyName(Index, strRegKeyName);

        if(Mac->Src != NULL)
        {
          strBufKey=Mac->Src;
          InsertQuote(strBufKey);
        }
        else
          strBufKey=L"";

        if((Mac->Flags&0xFF)==MACRO_COMMON)
            strBuf.Format (UMSG(!MacroDlg->RecBufferSize?
               (DisFlags?MMacroCommonDeleteAssign:MMacroCommonDeleteKey):
               MMacroCommonReDefinedKey), (const wchar_t*)strKeyText);
        else
          strBuf.Format (UMSG(!MacroDlg->RecBufferSize?
               (DisFlags?MMacroDeleteAssign:MMacroDeleteKey):
               MMacroReDefinedKey), (const wchar_t*)strKeyText);

        // �������� "� �� ��������� �� ��?"
        if(!DisFlags &&
           Mac->Buffer && MacroDlg->RecBuffer &&
           Mac->BufferSize == MacroDlg->RecBufferSize &&
           (
             (Mac->BufferSize >  1 && !memcmp(Mac->Buffer,MacroDlg->RecBuffer,MacroDlg->RecBufferSize*sizeof(DWORD))) ||
             (Mac->BufferSize == 1 && (DWORD)(DWORD_PTR)Mac->Buffer == (DWORD)(DWORD_PTR)MacroDlg->RecBuffer)
           )
          )
          I=0;
        else
          I=Message(MSG_WARNING,2,UMSG(MWarning),
              strBuf,
              UMSG(MMacroSequence),
              strBufKey,
              UMSG(!MacroDlg->RecBufferSize?MMacroDeleteKey2:
                    (DisFlags?MMacroDisDisabledKey:MMacroReDefinedKey2)),
              UMSG(DisFlags && MacroDlg->RecBufferSize?MMacroDisOverwrite:MYes),
              UMSG(DisFlags && MacroDlg->RecBufferSize?MMacroDisAnotherKey:MNo));

        if(!I)
        {
          if(DisFlags)
          {
            if (Opt.AutoSaveSetup) // ������� �� ������� ������ � ������
            {                      // ����� ������� ��������
              // ������ ������ ������ �� �������
              DeleteRegKey(strRegKeyName);
            }
            // �����������
            Mac->Flags&=~MFLAGS_DISABLEMACRO;
          }
          // � ����� ������ - ������������
          Dialog::SendDlgMessage(hDlg,DM_CLOSE,1,0);
          return TRUE;
        }
        // ����� - ����� �� �������� "���", �� � �� ��� � ���� ���
        //  � ������ ������� ���� �����.
        strKeyText = L"";
      }
    }
    Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)(const wchar_t*)strKeyText);
//    if(Param2 == KEY_F1 && LastKey == KEY_F1)
//      LastKey=-1;
//    else
      LastKey=(int)Param2;
    return TRUE;
  }
  else if(Msg == DN_CTLCOLORDLGITEM) // ������� Unchanged
  {
    Param2&=0xFF00FFFFU;      // Unchanged � ��� ����� � ������� ����� �������� �����
    Param2|=(Param2&0xFF)<<16;
    return Param2;
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

DWORD KeyMacro::AssignMacroKey()
{
/*
  +------ Define macro ------+
  | Press the desired key    |
  | ________________________ |
  +--------------------------+
*/
  static struct DialogDataEx MacroAssignDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,30,4,0,0,0,0,(const wchar_t *)MDefineMacroTitle,
  /* 01 */ DI_TEXT,-1,2,0,2,0,0,DIF_BOXCOLOR|DIF_READONLY,0,(const wchar_t *)MDefineMacro,
  /* 02 */ DI_COMBOBOX,5,3,28,3,1,0,0,1,L"",
  };
  MakeDialogItemsEx(MacroAssignDlgData,MacroAssignDlg);
  struct DlgParam Param={this,0,StartMode};
//_SVS(SysLog(L"StartMode=%d",StartMode));

  IsProcessAssignMacroKey++;
  Dialog Dlg(MacroAssignDlg,sizeof(MacroAssignDlg)/sizeof(MacroAssignDlg[0]),AssignMacroDlgProc,(LONG_PTR)&Param);
  Dlg.SetPosition(-1,-1,34,6);
  Dlg.SetHelp(L"KeyMacro");
  Dlg.Process();
  IsProcessAssignMacroKey--;

  if(Dlg.GetExitCode() == -1)
    return (DWORD)-1;
  return Param.Key;
}

static int Set3State(DWORD Flags,DWORD Chk1,DWORD Chk2)
{
  DWORD Chk12=Chk1|Chk2, FlagsChk12=Flags&Chk12;
  if(FlagsChk12 == Chk12 || FlagsChk12 == 0)
    return (2);
  else
    return (Flags&Chk1?1:0);
}


LONG_PTR WINAPI KeyMacro::ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	static struct DlgParam *KMParam=NULL;

	if(Msg == DN_INITDIALOG)
	{
		KMParam=(struct DlgParam *)Param2;
	}
	else if(Msg == DN_BTNCLICK && (Param1 == 4 || Param1 == 8))
	{
		for(int I=1; I <= 3; ++I)
			Dialog::SendDlgMessage(hDlg,DM_ENABLE,Param1+I,Param2);
	}
#if 0
  else if(Msg==DN_KEY && Param2==KEY_ALTF4)
  {
		KeyMacro *MacroDlg=KMParam->Handle;
		(*FrameManager)[0]->UnlockRefresh();
		FILE *MacroFile;
		char MacroFileName[NM];
		if (!FarMkTempEx(MacroFileName) || (MacroFile=fopen(MacroFileName,"wb"))==NULL)
			return TRUE;

		char *TextBuffer;
		DWORD Buf[1];
		Buf[0]=MacroDlg->RecBuffer[0];
		if((TextBuffer=MacroDlg->MkTextSequence((MacroDlg->RecBufferSize==1?Buf:MacroDlg->RecBuffer),MacroDlg->RecBufferSize)) != NULL)
		{
			fwrite(TextBuffer,strlen(TextBuffer),1,MacroFile);
			fclose(MacroFile);
			xf_free(TextBuffer);
			{
				//ConsoleTitle *OldTitle=new ConsoleTitle;
				FileEditor ShellEditor(MacroFileName,-1,FFILEEDIT_DISABLEHISTORY,-1,-1,NULL);
				//delete OldTitle;
				ShellEditor.SetDynamicallyBorn(false);
				FrameManager->EnterModalEV();
				FrameManager->ExecuteModal();
				FrameManager->ExitModalEV();
				if (!ShellEditor.IsFileChanged() || (MacroFile=fopen(MacroFileName,"rb"))==NULL)
					;
				else
				{
					struct MacroRecord NewMacroWORK2={0};
					long FileSize=filelen(MacroFile);
					TextBuffer=(char*)xf_malloc(FileSize);
					if(TextBuffer)
					{
						fread(TextBuffer,FileSize,1,MacroFile);
						if(!MacroDlg->ParseMacroString(&NewMacroWORK2,TextBuffer))
						{
							if(NewMacroWORK2.BufferSize > 1)
								xf_free(NewMacroWORK2.Buffer);
						}
						else
						{
							MacroDlg->RecBuffer=NewMacroWORK2.Buffer;
							MacroDlg->RecBufferSize=NewMacroWORK2.BufferSize;
						}
					}
					fclose(MacroFile);
				}
			}
			FrameManager->ResizeAllFrame();
			FrameManager->PluginCommit();
		}
		else
			fclose(MacroFile);
		remove(MacroFileName);
		return TRUE;
	}
#endif
	return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int KeyMacro::GetMacroSettings(int Key,DWORD &Flags)
{
/*
           1         2         3         4         5         6
    3456789012345678901234567890123456789012345678901234567890123456789
  1 �=========== ��������� ������������ ��� 'CtrlP' ==================�
  2 � [ ] ��������� �� ����� ���������� ����� �� �����                �
  3 � [ ] ��������� ����� ������� FAR                                 �
  4 �-----------------------------------------------------------------�
  5 � [ ] �������� ������             [ ] ��������� ������            �
  6 �   [?] �� ������ �������           [?] �� ������ �������         �
  7 �   [?] ��������� ��� �����         [?] ��������� ��� �����       �
  8 �   [?] �������� �����              [?] �������� �����            �
  9 �-----------------------------------------------------------------�
 10   [?] ������ ��������� ������
 11 � [?] ������� ����                                                �
 12 �-----------------------------------------------------------------�
 13 �               [ ���������� ]  [ �������� ]                      �
 14 L=================================================================+

*/
  static struct DialogDataEx MacroSettingsDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,69,14,0,0,0,0,L"",
  /* 01 */DI_CHECKBOX,5,2,0,2,1,0,0,0,(const wchar_t *)MMacroSettingsEnableOutput,
  /* 02 */DI_CHECKBOX,5,3,0,3,0,0,0,0,(const wchar_t *)MMacroSettingsRunAfterStart,
  /* 03 */DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 04 */DI_CHECKBOX,5,5,0,5,0,0,0,0,(const wchar_t *)MMacroSettingsActivePanel,
  /* 05 */DI_CHECKBOX,7,6,0,6,0,2,DIF_3STATE|DIF_DISABLE,0,(const wchar_t *)MMacroSettingsPluginPanel,
  /* 06 */DI_CHECKBOX,7,7,0,7,0,2,DIF_3STATE|DIF_DISABLE,0,(const wchar_t *)MMacroSettingsFolders,
  /* 07 */DI_CHECKBOX,7,8,0,8,0,2,DIF_3STATE|DIF_DISABLE,0,(const wchar_t *)MMacroSettingsSelectionPresent,
  /* 08 */DI_CHECKBOX,37,5,0,5,0,0,0,0,(const wchar_t *)MMacroSettingsPassivePanel,
  /* 09 */DI_CHECKBOX,39,6,0,6,0,2,DIF_3STATE|DIF_DISABLE,0,(const wchar_t *)MMacroSettingsPluginPanel,
  /* 10 */DI_CHECKBOX,39,7,0,7,0,2,DIF_3STATE|DIF_DISABLE,0,(const wchar_t *)MMacroSettingsFolders,
  /* 11 */DI_CHECKBOX,39,8,0,8,0,2,DIF_3STATE|DIF_DISABLE,0,(const wchar_t *)MMacroSettingsSelectionPresent,
  /* 12 */DI_TEXT,3,9,0,9,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 13 */DI_CHECKBOX,5,10,0,10,0,2,DIF_3STATE,0,(const wchar_t *)MMacroSettingsCommandLine,
  /* 14 */DI_CHECKBOX,5,11,0,11,0,2,DIF_3STATE,0,(const wchar_t *)MMacroSettingsSelectionBlockPresent,
  /* 15 */DI_TEXT,3,12,0,12,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 16 */DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 17 */DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(MacroSettingsDlgData,MacroSettingsDlg);

  string strKeyText;
  KeyToText(Key,strKeyText);

  MacroSettingsDlg[0].strData.Format (UMSG(MMacroSettingsTitle), (const wchar_t*)strKeyText);
//  if(!(Key&0x7F000000))
//    MacroSettingsDlg[3].Flags|=DIF_DISABLE;

  MacroSettingsDlg[1].Selected=Flags&MFLAGS_DISABLEOUTPUT?0:1;
  MacroSettingsDlg[2].Selected=Flags&MFLAGS_RUNAFTERFARSTART?1:0;

  MacroSettingsDlg[5].Selected=Set3State(Flags,MFLAGS_NOFILEPANELS,MFLAGS_NOPLUGINPANELS);
  MacroSettingsDlg[6].Selected=Set3State(Flags,MFLAGS_NOFILES,MFLAGS_NOFOLDERS);
  MacroSettingsDlg[7].Selected=Set3State(Flags,MFLAGS_SELECTION,MFLAGS_NOSELECTION);

  MacroSettingsDlg[9].Selected=Set3State(Flags,MFLAGS_PNOFILEPANELS,MFLAGS_PNOPLUGINPANELS);
  MacroSettingsDlg[10].Selected=Set3State(Flags,MFLAGS_PNOFILES,MFLAGS_PNOFOLDERS);
  MacroSettingsDlg[11].Selected=Set3State(Flags,MFLAGS_PSELECTION,MFLAGS_PNOSELECTION);

  MacroSettingsDlg[13].Selected=Set3State(Flags,MFLAGS_EMPTYCOMMANDLINE,MFLAGS_NOTEMPTYCOMMANDLINE);
  MacroSettingsDlg[14].Selected=Set3State(Flags,MFLAGS_EDITSELECTION,MFLAGS_EDITNOSELECTION);

  struct DlgParam Param={this,0,0};
  Dialog Dlg(MacroSettingsDlg,sizeof(MacroSettingsDlg)/sizeof(MacroSettingsDlg[0]),ParamMacroDlgProc,(LONG_PTR)&Param);
  Dlg.SetPosition(-1,-1,73,16);
  Dlg.SetHelp(L"KeyMacroSetting");
  FrameManager->GetBottomFrame()->Lock(); // ������� ���������� ������
  Dlg.Process();
  FrameManager->GetBottomFrame()->Unlock(); // ������ ����� :-)
  if (Dlg.GetExitCode()!=16)
    return(FALSE);

  Flags=MacroSettingsDlg[1].Selected?0:MFLAGS_DISABLEOUTPUT;
  Flags|=MacroSettingsDlg[2].Selected?MFLAGS_RUNAFTERFARSTART:0;
  if(MacroSettingsDlg[4].Selected)
  {
    Flags|=MacroSettingsDlg[5].Selected==2?0:
            (MacroSettingsDlg[5].Selected==0?MFLAGS_NOPLUGINPANELS:MFLAGS_NOFILEPANELS);
    Flags|=MacroSettingsDlg[6].Selected==2?0:
            (MacroSettingsDlg[6].Selected==0?MFLAGS_NOFOLDERS:MFLAGS_NOFILES);
    Flags|=MacroSettingsDlg[7].Selected==2?0:
            (MacroSettingsDlg[7].Selected==0?MFLAGS_NOSELECTION:MFLAGS_SELECTION);
  }
  if(MacroSettingsDlg[8].Selected)
  {
    Flags|=MacroSettingsDlg[9].Selected==2?0:
            (MacroSettingsDlg[9].Selected==0?MFLAGS_PNOPLUGINPANELS:MFLAGS_PNOFILEPANELS);
    Flags|=MacroSettingsDlg[10].Selected==2?0:
            (MacroSettingsDlg[10].Selected==0?MFLAGS_PNOFOLDERS:MFLAGS_PNOFILES);
    Flags|=MacroSettingsDlg[11].Selected==2?0:
            (MacroSettingsDlg[11].Selected==0?MFLAGS_PNOSELECTION:MFLAGS_PSELECTION);
  }
  Flags|=MacroSettingsDlg[13].Selected==2?0:
          (MacroSettingsDlg[13].Selected==0?MFLAGS_NOTEMPTYCOMMANDLINE:MFLAGS_EMPTYCOMMANDLINE);
  Flags|=MacroSettingsDlg[14].Selected==2?0:
          (MacroSettingsDlg[14].Selected==0?MFLAGS_EDITNOSELECTION:MFLAGS_EDITSELECTION);

  return(TRUE);
}

int KeyMacro::PostNewMacro(const wchar_t *PlainText,DWORD Flags,DWORD AKey)
{
  struct MacroRecord NewMacroWORK2={0};

  wchar_t *Buffer=(wchar_t *)PlainText;
  bool allocBuffer=false;

  if(Flags&MFLAGS_REG_MULTI_SZ) // ��������� ��� �� REG_MULTI_SZ
  {
    int lenPlainText=0;
    while(1)
    {
      if(!PlainText[lenPlainText] && !PlainText[lenPlainText+1])
      {
        lenPlainText+=2;
        break;
      }
      lenPlainText++;
    }
    //lenPlainText++;

    Buffer=(wchar_t*)xf_malloc((lenPlainText+1)*(int)sizeof(wchar_t));
    if(Buffer)
    {
      allocBuffer=true;
      wmemmove(Buffer,PlainText,lenPlainText);
      Buffer[lenPlainText]=0; // +1
      wchar_t *ptrBuffer=Buffer;
      while(1)
      {
        ptrBuffer+=StrLength(ptrBuffer);
        if(!ptrBuffer[0] && !ptrBuffer[1])
          break;
        *ptrBuffer=L'\n';
      }
    }
    else
      return FALSE;
  }

  // ������� ������� �� ������
  BOOL parsResult=ParseMacroString(&NewMacroWORK2,Buffer);
  if(allocBuffer && Buffer)
    xf_free(Buffer);

  if(!parsResult)
  {
    if(NewMacroWORK2.BufferSize > 1)
      xf_free(NewMacroWORK2.Buffer);
    return FALSE;
  }
  NewMacroWORK2.Flags=Flags;
  NewMacroWORK2.Key=AKey;

  // ������ ��������� �������� ������� ������ ������
  struct MacroRecord *NewMacroWORK;
  if((NewMacroWORK=(struct MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))) == NULL)
  {
    if(NewMacroWORK2.BufferSize > 1)
      xf_free(NewMacroWORK2.Buffer);
    return FALSE;
  }

  // ������ ������� � ���� "�������" ����� ������
  Work.MacroWORK=NewMacroWORK;
  NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
  memcpy(NewMacroWORK,&NewMacroWORK2,sizeof(struct MacroRecord));
  Work.MacroWORKCount++;

//  Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
  if(Work.ExecLIBPos == Work.MacroWORK->BufferSize)
    Work.ExecLIBPos=0;
  return TRUE;
}

int KeyMacro::PostNewMacro(struct MacroRecord *MRec,BOOL NeedAddSendFlag,BOOL IsPluginSend)
{
  if(!MRec)
    return FALSE;

  struct MacroRecord NewMacroWORK2={0};
  memcpy(&NewMacroWORK2,MRec,sizeof(struct MacroRecord));
  NewMacroWORK2.Src=NULL;

//  if(MRec->BufferSize > 1)
  {
    if((NewMacroWORK2.Buffer=(DWORD*)xf_malloc((MRec->BufferSize+3)*sizeof(DWORD))) == NULL)
    {
      return FALSE;
    }
  }

  // ������ ��������� �������� ������� ������ ������
  struct MacroRecord *NewMacroWORK;
  if((NewMacroWORK=(struct MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))) == NULL)
  {
//    if(MRec->BufferSize > 1)
      xf_free(NewMacroWORK2.Buffer);
    return FALSE;
  }

  // ������ ������� � ���� "�������" ����� ������
  if(IsPluginSend)
    NewMacroWORK2.Buffer[0]=MCODE_OP_KEYS;
  if((MRec->BufferSize+1) > 2)
    memcpy(&NewMacroWORK2.Buffer[IsPluginSend?1:0],MRec->Buffer,sizeof(DWORD)*MRec->BufferSize);
  else if(MRec->Buffer)
    NewMacroWORK2.Buffer[IsPluginSend?1:0]=(DWORD)(DWORD_PTR)MRec->Buffer;
  if(IsPluginSend)
    NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize+1]=MCODE_OP_ENDKEYS;
  //NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize]=MCODE_OP_NOP; // ���.�������/��������

  if(IsPluginSend)
    NewMacroWORK2.BufferSize+=2;

  Work.MacroWORK=NewMacroWORK;
  NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
  memcpy(NewMacroWORK,&NewMacroWORK2,sizeof(struct MacroRecord));
  Work.MacroWORKCount++;

//  Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
  if(Work.ExecLIBPos == Work.MacroWORK->BufferSize)
    Work.ExecLIBPos=0;
  return TRUE;
}

int KeyMacro::ParseMacroString(struct MacroRecord *CurMacro,const wchar_t *BufPtr)
{
  if ( CurMacro )
    return __parseMacroString(CurMacro->Buffer, CurMacro->BufferSize, BufPtr);
  return FALSE;
}


void MacroState::Init(TVarTable *tbl)
{
  KeyProcess=Executing=MacroPC=ExecLIBPos=MacroWORKCount=0;
  MacroWORK=NULL;
  if(!tbl)
  {
    AllocVarTable=true;
    locVarTable=(TVarTable*)xf_malloc(sizeof(TVarTable));
    initVTable(*locVarTable);
  }
  else
  {
    AllocVarTable=false;
    locVarTable=tbl;
  }
}

int KeyMacro::PushState(bool CopyLocalVars)
{
  if(CurPCStack+1 >= STACKLEVEL)
    return FALSE;
  ++CurPCStack;
  Work.UsedInternalClipboard=::UsedInternalClipboard;
  memcpy(PCStack+CurPCStack,&Work,sizeof(struct MacroState));
  Work.Init(CopyLocalVars?PCStack[CurPCStack].locVarTable:NULL);
  return TRUE;
}

int KeyMacro::PopState()
{
  if(CurPCStack < 0)
    return FALSE;
  memcpy(&Work,PCStack+CurPCStack,sizeof(struct MacroState));
  ::UsedInternalClipboard=Work.UsedInternalClipboard;
  CurPCStack--;
  return TRUE;
}

// ������� ��������� ������� ������� ������� � �������
// Ret=-1 - �� ������ �������.
// ���� CheckMode=-1 - ������ ������ � ����� ������, �.�. ������ ����������
int KeyMacro::GetIndex(int Key, int ChechMode, bool UseCommon)
{
  if(MacroLIB)
  {
    for(int I=0; I < 2; ++I)
    {
      int Pos,Len;
      struct MacroRecord *MPtr;
      if(ChechMode == -1)
      {
        Len=MacroLIBCount;
        MPtr=MacroLIB;
      }
      else
      {
        Len=IndexMode[ChechMode][1];
        if(Len)
          MPtr=MacroLIB+IndexMode[ChechMode][0];
  //_SVS(SysLog(L"ChechMode=%d (%d,%d)",ChechMode,IndexMode[ChechMode][0],IndexMode[ChechMode][1]));
      }

      if(Len)
      {
        for(Pos=0; Pos < Len; ++Pos, ++MPtr)
        {
          if ( !((MPtr->Key ^ Key) & ~0xFFFF) &&
                (Upper(MPtr->Key)==Upper(Key)) &&
                (MPtr->BufferSize > 0) )
          {
    //        && (ChechMode == -1 || (MPtr->Flags&MFLAGS_MODEMASK) == ChechMode))
    //_SVS(SysLog(L"GetIndex: Pos=%d MPtr->Key=0x%08X", Pos,MPtr->Key));
            if(!(MPtr->Flags&MFLAGS_DISABLEMACRO))
              return Pos+((ChechMode >= 0)?IndexMode[ChechMode][0]:0);
          }
        }
      }
      // ����� ������� �� MACRO_COMMON
      if(ChechMode != -1 && !I && UseCommon)
        ChechMode=MACRO_COMMON;
      else
        break;
    }
  }
  return -1;
}

// ��������� �������, ����������� ��������� ��������
// Ret= 0 - �� ������ �������.
// ���� CheckMode=-1 - ������ ������ � ����� ������, �.�. ������ ����������
int KeyMacro::GetRecordSize(int Key, int CheckMode)
{
  int Pos=GetIndex(Key,CheckMode);
  if(Pos == -1)
    return 0;
  return sizeof(struct MacroRecord)+MacroLIB[Pos].BufferSize;
}

// �������� �������� ���� �� ����
const wchar_t* KeyMacro::GetSubKey(int Mode)
{
  return (const wchar_t *)((Mode >= MACRO_OTHER && Mode < MACRO_LAST)?MKeywordsArea[Mode].Name:L"");
}

// �������� ��� ���� �� �����
int KeyMacro::GetSubKey(const wchar_t *Mode)
{
  int I;
  for(I=MACRO_OTHER; I < MACRO_LAST; ++I)
    if(!StrCmpI(MKeywordsArea[I].Name,Mode))
      return I;
  return -1;
}

int KeyMacro::GetMacroKeyInfo(int Mode,int Pos,const wchar_t *KeyName, string &strDescription)
{
  if(Mode >= MACRO_OTHER && Mode < MACRO_LAST)
  {
    string strUpKeyName;
    string strRegKeyName, strKeyText;
    strUpKeyName.Format (L"KeyMacros\\%s",GetSubKey(Mode));

    if (!EnumRegKey(strUpKeyName,Pos,strRegKeyName))
      return -1;

    //const wchar_t *KeyNamePtr=wcsrchr(strRegKeyName,L'\\');
    //if (KeyNamePtr!=NULL) BUGBUG
      //strKeyName = KeyNamePtr+1;
    GetRegKey(strRegKeyName,L"Description",strDescription,L"");
    return Pos+1;
  }
  return -1;
}

BOOL KeyMacro::CheckEditSelected(DWORD CurFlags)
{
  if(Mode==MACRO_EDITOR || Mode==MACRO_DIALOG || Mode==MACRO_VIEWER || (Mode==MACRO_SHELL&&CtrlObject->CmdLine->IsVisible()))
  {
    int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS));
    Frame* CurFrame=FrameManager->GetCurrentFrame();
    if (CurFrame && CurFrame->GetType()==NeedType)
    {
      int CurSelected;
      if(Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
        CurSelected=(int)CtrlObject->CmdLine->VMProcess(MCODE_C_SELECTED);
      else
        CurSelected=(int)CurFrame->VMProcess(MCODE_C_SELECTED);

      if(((CurFlags&MFLAGS_EDITSELECTION) && !CurSelected) ||
         ((CurFlags&MFLAGS_EDITNOSELECTION) && CurSelected))
          return FALSE;
    }
  }
  return TRUE;
}

BOOL KeyMacro::CheckInsidePlugin(DWORD CurFlags)
{
  if(CtrlObject && CtrlObject->Plugins.CurPluginItem && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS)) // ?????
  //if(CtrlObject && CtrlObject->Plugins.CurEditor && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS))
    return FALSE;
  return TRUE;
}

BOOL KeyMacro::CheckCmdLine(int CmdLength,DWORD CurFlags)
{
 if (((CurFlags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength!=0) ||
     ((CurFlags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0))
      return FALSE;
  return TRUE;
}

BOOL KeyMacro::CheckPanel(int PanelMode,DWORD CurFlags,BOOL IsPassivePanel)
{
  if(IsPassivePanel)
  {
    if((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_PNOPLUGINPANELS)) ||
       (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_PNOFILEPANELS)))
      return FALSE;
  }
  else
  {
    if((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_NOPLUGINPANELS)) ||
       (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_NOFILEPANELS)))
      return FALSE;
  }
  return TRUE;
}

BOOL KeyMacro::CheckFileFolder(Panel *CheckPanel,DWORD CurFlags, BOOL IsPassivePanel)
{
  string strFileName;
  DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
  CheckPanel->GetFileName(strFileName,CheckPanel->GetCurrentPos(),FileAttr);
  if(FileAttr != INVALID_FILE_ATTRIBUTES)
  {
    if(IsPassivePanel)
    {
      if(((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFILES)))
        return FALSE;
    }
    else
    {
      if(((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFILES)))
        return FALSE;
    }
  }
  return TRUE;
}

BOOL KeyMacro::CheckAll(int /*CheckMode*/,DWORD CurFlags)
{
/* $TODO:
     ����� ������ Check*() ����������� ������� IfCondition()
     ��� ���������� �������������� ����.
*/
  if(!CheckInsidePlugin(CurFlags))
    return FALSE;

  // �������� �� �����/�� ����� � ���.������ (� � ���������? :-)
  if(CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
    if(!CheckCmdLine(CtrlObject->CmdLine->GetLength(),CurFlags))
      return FALSE;

  FilePanels *Cp=CtrlObject->Cp();
  if(!Cp)
    return FALSE;

  // �������� ������ � ���� �����
  Panel *ActivePanel=Cp->ActivePanel;
  Panel *PassivePanel=Cp->GetAnotherPanel(Cp->ActivePanel);
  if(ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACRO_SHELL)
  {

    if(CurFlags&(MFLAGS_NOPLUGINPANELS|MFLAGS_NOFILEPANELS))
      if(!CheckPanel(ActivePanel->GetMode(),CurFlags,FALSE))
        return FALSE;

    if(CurFlags&(MFLAGS_PNOPLUGINPANELS|MFLAGS_PNOFILEPANELS))
      if(!CheckPanel(PassivePanel->GetMode(),CurFlags,TRUE))
        return FALSE;

    if(CurFlags&(MFLAGS_NOFOLDERS|MFLAGS_NOFILES))
      if(!CheckFileFolder(ActivePanel,CurFlags,FALSE))
        return FALSE;

    if(CurFlags&(MFLAGS_PNOFOLDERS|MFLAGS_PNOFILES))
      if(!CheckFileFolder(PassivePanel,CurFlags,TRUE))
        return FALSE;

    if(CurFlags&(MFLAGS_SELECTION|MFLAGS_NOSELECTION|MFLAGS_PSELECTION|MFLAGS_PNOSELECTION))
      if(Mode!=MACRO_EDITOR && Mode != MACRO_DIALOG && Mode!=MACRO_VIEWER)
      {
        int SelCount=ActivePanel->GetRealSelCount();
        if(((CurFlags&MFLAGS_SELECTION) && SelCount < 1) ||
           ((CurFlags&MFLAGS_NOSELECTION) && SelCount >= 1))
          return FALSE;

        SelCount=PassivePanel->GetRealSelCount();
        if(((CurFlags&MFLAGS_PSELECTION) && SelCount < 1) ||
           ((CurFlags&MFLAGS_PNOSELECTION) && SelCount >= 1))
          return FALSE;
      }
  }

  if(!CheckEditSelected(CurFlags))
    return FALSE;

  return TRUE;
}

/*
  Return: FALSE - ���� ����������� MFLAGS_* �� ���������� ���
                  ��� �� ����� ���������� �������!
          TRUE  - ����� ����(�) ����������(�)
*/
BOOL KeyMacro::CheckCurMacroFlags(DWORD Flags)
{
  if(Work.Executing && Work.MacroWORK)
  {
    return (Work.MacroWORK->Flags&Flags)?TRUE:FALSE;
  }
  return(FALSE);

}

/*
  Return: 0 - �� � ������ �����, 1 - Executing, 2 - Executing common, 3 - Recording, 4 - Recording common
  See farconst.hpp::MacroRecordAndExecuteType
*/
int KeyMacro::GetCurRecord(struct MacroRecord* RBuf,int *KeyPos)
{
  if(KeyPos && RBuf)
  {
    *KeyPos=Work.Executing?Work.ExecLIBPos:0;
    memset(RBuf,0,sizeof(struct MacroRecord));
    if(Recording == MACROMODE_NOMACRO)
    {
      if(Work.Executing)
      {
        memcpy(RBuf,MacroLIB+Work.MacroPC,sizeof(struct MacroRecord)); //????
        return Work.Executing;
      }
      memset(RBuf,0,sizeof(struct MacroRecord));
      return MACROMODE_NOMACRO;
    }
    RBuf->BufferSize=RecBufferSize;
    RBuf->Buffer=RecBuffer;
    return Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON;
  }
  return Recording?(Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON):(Work.Executing?Work.Executing:MACROMODE_NOMACRO);
}

static int __cdecl SortMacros(const struct MacroRecord *el1,
                           const struct MacroRecord *el2)
{
  int Mode1, Mode2;
  if((Mode1=(el1->Flags&MFLAGS_MODEMASK)) == (Mode2=(el2->Flags&MFLAGS_MODEMASK)))
    return 0;
  if(Mode1 < Mode2)
    return -1;
  return 1;
}

// ���������� ��������� ������
void KeyMacro::Sort(void)
{
  typedef int (__cdecl *qsort_fn)(const void*,const void*);
  // ���������
  far_qsort(MacroLIB,
        MacroLIBCount,
        sizeof(struct MacroRecord),
        (qsort_fn)SortMacros);
  // ������������� ������ �����
  struct MacroRecord *MPtr;
  int I,J;
  int CurMode=MACRO_OTHER;
  memset(IndexMode,0,sizeof(IndexMode));
  for(MPtr=MacroLIB,I=0; I < MacroLIBCount; ++I,++MPtr)
  {
    J=MPtr->Flags&MFLAGS_MODEMASK;
    if(CurMode != J)
    {
      IndexMode[J][0]=I;
      CurMode=J;
    }
    IndexMode[J][1]++;
  }

//_SVS(for(I=0; I < sizeof(IndexMode)/sizeof(IndexMode[0]); ++I)SysLog(L"IndexMode[%02d.%s]=%d,%d",I,GetSubKey(I),IndexMode[I][0],IndexMode[I][1]));
}

DWORD KeyMacro::GetOpCode(struct MacroRecord *MR,int PC)
{
  DWORD OpCode=(MR->BufferSize > 1)?MR->Buffer[PC]:(DWORD)(DWORD_PTR)MR->Buffer;
  return OpCode;
}

// ������ OpCode � �����. ���������� ���������� ��������
DWORD KeyMacro::SetOpCode(struct MacroRecord *MR,int PC,DWORD OpCode)
{
  DWORD OldOpCode;
  if(MR->BufferSize > 1)
  {
    OldOpCode=MR->Buffer[PC];
    MR->Buffer[PC]=OpCode;
  }
  else
  {
    OldOpCode=(DWORD)(DWORD_PTR)MR->Buffer;
    MR->Buffer=(DWORD*)(DWORD_PTR)OpCode;
  }
  return OldOpCode;
}

// ��� ��� ����� ��� ���:
// BugZ#873 - ACTL_POSTKEYSEQUENCE � ��������� ����
int KeyMacro::IsExecutingLastKey()
{
  if(Work.Executing && Work.MacroWORK)
  {
    return (Work.ExecLIBPos == Work.MacroWORK->BufferSize-1);
  }
  return FALSE;
}

void KeyMacro::DropProcess()
{
  if(Work.Executing)
  {
    if(LockScr) delete LockScr;
    LockScr=NULL;
    UsedInternalClipboard=0; //??
    Work.Executing=MACROMODE_NOMACRO;
    ReleaseWORKBuffer();
  }
}

bool checkMacroConst(const wchar_t *name)
{
   return !varLook(glbConstTable, name)?false:true;
}

void initMacroVarTable(int global)
{
  if(global)
  {
    initVTable(glbVarTable);
    initVTable(glbConstTable); //???
  }
}

void doneMacroVarTable(int global)
{
  if(global)
  {
    deleteVTable(glbVarTable);
    deleteVTable(glbConstTable); //???
  }
}

BOOL KeyMacro::GetMacroParseError(string *ErrMsg1,string *ErrMsg2,string *ErrMsg3)
{
  return __getMacroParseError(ErrMsg1,ErrMsg2,ErrMsg3);
}

// ��� OpCode (�� ����������� MCODE_OP_ENDKEYS)?
bool KeyMacro::IsOpCode(DWORD p)
{
  return (!(p&KEY_MACRO_BASE) || p == MCODE_OP_ENDKEYS)?false:true;
}
