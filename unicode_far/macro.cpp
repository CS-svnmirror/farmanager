/*
macro.cpp

�������
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

#include "macro.hpp"
#include "FarGuid.hpp"
#include "cmdline.hpp"
#include "config.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "keyboard.hpp"
#include "lockscrn.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "panel.hpp"
#include "scrbuf.hpp"
#include "syslog.hpp"
#include "macroopcode.hpp"
#include "console.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "flink.hpp"
#include "cddrv.hpp"
#include "fileedit.hpp"
#include "viewer.hpp"
#include "datetime.hpp"
#include "xlat.hpp"
#include "imports.hpp"
#include "plugapi.hpp"
#include "dlgedit.hpp"
#include "clipboard.hpp"
#include "filelist.hpp"
#include "treelist.hpp"
#include "dirmix.hpp"
#include "elevation.hpp"
#include "stddlg.hpp"
#include "vmenu2.hpp"
#include "constitle.hpp"
#include "usermenu.hpp"
#include "filemasks.hpp"
#include "plugins.hpp"
#include "interf.hpp"

#if 0
void print_opcodes()
{
	FILE* fp=fopen("opcodes.tmp", "w");
	if (!fp) return;

	/* ************************************************************************* */
	// �������
	fprintf(fp, "MCODE_F_NOFUNC=0x%X\n", MCODE_F_NOFUNC);
	fprintf(fp, "MCODE_F_ABS=0x%X // N=abs(N)\n", MCODE_F_ABS);
	fprintf(fp, "MCODE_F_AKEY=0x%X // V=akey(Mode[,Type])\n", MCODE_F_AKEY);
	fprintf(fp, "MCODE_F_ASC=0x%X // N=asc(S)\n", MCODE_F_ASC);
	fprintf(fp, "MCODE_F_ATOI=0x%X // N=atoi(S[,radix])\n", MCODE_F_ATOI);
	fprintf(fp, "MCODE_F_CLIP=0x%X // V=clip(N[,V])\n", MCODE_F_CLIP);
	fprintf(fp, "MCODE_F_CHR=0x%X // S=chr(N)\n", MCODE_F_CHR);
	fprintf(fp, "MCODE_F_DATE=0x%X // S=date([S])\n", MCODE_F_DATE);
	fprintf(fp, "MCODE_F_DLG_GETVALUE=0x%X // V=Dlg.GetValue([Pos[,InfoID]])\n", MCODE_F_DLG_GETVALUE);
	fprintf(fp, "MCODE_F_EDITOR_SEL=0x%X // V=Editor.Sel(Action[,Opt])\n", MCODE_F_EDITOR_SEL);
	fprintf(fp, "MCODE_F_EDITOR_SET=0x%X // N=Editor.Set(N,Var)\n", MCODE_F_EDITOR_SET);
	fprintf(fp, "MCODE_F_EDITOR_UNDO=0x%X // V=Editor.Undo(N)\n", MCODE_F_EDITOR_UNDO);
	fprintf(fp, "MCODE_F_EDITOR_POS=0x%X // N=Editor.Pos(Op,What[,Where])\n", MCODE_F_EDITOR_POS);
	fprintf(fp, "MCODE_F_ENVIRON=0x%X // S=Env(S[,Mode[,Value]])\n", MCODE_F_ENVIRON);
	fprintf(fp, "MCODE_F_FATTR=0x%X // N=fattr(S)\n", MCODE_F_FATTR);
	fprintf(fp, "MCODE_F_FEXIST=0x%X // S=fexist(S)\n", MCODE_F_FEXIST);
	fprintf(fp, "MCODE_F_FSPLIT=0x%X // S=fsplit(S,N)\n", MCODE_F_FSPLIT);
	fprintf(fp, "MCODE_F_IIF=0x%X // V=iif(C,V1,V2)\n", MCODE_F_IIF);
	fprintf(fp, "MCODE_F_INDEX=0x%X // S=index(S1,S2[,Mode])\n", MCODE_F_INDEX);
	fprintf(fp, "MCODE_F_INT=0x%X // N=int(V)\n", MCODE_F_INT);
	fprintf(fp, "MCODE_F_ITOA=0x%X // S=itoa(N[,radix])\n", MCODE_F_ITOA);
	fprintf(fp, "MCODE_F_KEY=0x%X // S=key(V)\n", MCODE_F_KEY);
	fprintf(fp, "MCODE_F_LCASE=0x%X // S=lcase(S1)\n", MCODE_F_LCASE);
	fprintf(fp, "MCODE_F_LEN=0x%X // N=len(S)\n", MCODE_F_LEN);
	fprintf(fp, "MCODE_F_MAX=0x%X // N=max(N1,N2)\n", MCODE_F_MAX);
	fprintf(fp, "MCODE_F_MENU_CHECKHOTKEY=0x%X // N=checkhotkey(S[,N])\n", MCODE_F_MENU_CHECKHOTKEY);
	fprintf(fp, "MCODE_F_MENU_GETHOTKEY=0x%X // S=gethotkey([N])\n", MCODE_F_MENU_GETHOTKEY);
	fprintf(fp, "MCODE_F_MENU_SELECT=0x%X // N=Menu.Select(S[,N[,Dir]])\n", MCODE_F_MENU_SELECT);
	fprintf(fp, "MCODE_F_MENU_SHOW=0x%X // S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])\n", MCODE_F_MENU_SHOW);
	fprintf(fp, "MCODE_F_MIN=0x%X // N=min(N1,N2)\n", MCODE_F_MIN);
	fprintf(fp, "MCODE_F_MOD=0x%X // N=mod(a,b) == a %%  b\n", MCODE_F_MOD);
	fprintf(fp, "MCODE_F_MLOAD=0x%X // B=mload(var)\n", MCODE_F_MLOAD);
	fprintf(fp, "MCODE_F_MSAVE=0x%X // B=msave(var)\n", MCODE_F_MSAVE);
	fprintf(fp, "MCODE_F_MSGBOX=0x%X // N=msgbox([\"Title\"[,\"Text\"[,flags]]])\n", MCODE_F_MSGBOX);
	fprintf(fp, "MCODE_F_PANEL_FATTR=0x%X // N=Panel.FAttr(panelType,fileMask)\n", MCODE_F_PANEL_FATTR);
	fprintf(fp, "MCODE_F_PANEL_SETPATH=0x%X // N=panel.SetPath(panelType,pathName[,fileName])\n", MCODE_F_PANEL_SETPATH);
	fprintf(fp, "MCODE_F_PANEL_FEXIST=0x%X // N=Panel.FExist(panelType,fileMask)\n", MCODE_F_PANEL_FEXIST);
	fprintf(fp, "MCODE_F_PANEL_SETPOS=0x%X // N=Panel.SetPos(panelType,fileName)\n", MCODE_F_PANEL_SETPOS);
	fprintf(fp, "MCODE_F_PANEL_SETPOSIDX=0x%X // N=Panel.SetPosIdx(panelType,Idx[,InSelection])\n", MCODE_F_PANEL_SETPOSIDX);
	fprintf(fp, "MCODE_F_PANEL_SELECT=0x%X // V=Panel.Select(panelType,Action[,Mode[,Items]])\n", MCODE_F_PANEL_SELECT);
	fprintf(fp, "MCODE_F_PANELITEM=0x%X // V=PanelItem(Panel,Index,TypeInfo)\n", MCODE_F_PANELITEM);
	fprintf(fp, "MCODE_F_EVAL=0x%X // N=eval(S[,N])\n", MCODE_F_EVAL);
	fprintf(fp, "MCODE_F_RINDEX=0x%X // S=rindex(S1,S2[,Mode])\n", MCODE_F_RINDEX);
	fprintf(fp, "MCODE_F_SLEEP=0x%X // Sleep(N)\n", MCODE_F_SLEEP);
	fprintf(fp, "MCODE_F_STRING=0x%X // S=string(V)\n", MCODE_F_STRING);
	fprintf(fp, "MCODE_F_SUBSTR=0x%X // S=substr(S,start[,length])\n", MCODE_F_SUBSTR);
	fprintf(fp, "MCODE_F_UCASE=0x%X // S=ucase(S1)\n", MCODE_F_UCASE);
	fprintf(fp, "MCODE_F_WAITKEY=0x%X // V=waitkey([N,[T]])\n", MCODE_F_WAITKEY);
	fprintf(fp, "MCODE_F_XLAT=0x%X // S=xlat(S)\n", MCODE_F_XLAT);
	fprintf(fp, "MCODE_F_FLOCK=0x%X // N=FLock(N,N)\n", MCODE_F_FLOCK);
	fprintf(fp, "MCODE_F_CALLPLUGIN=0x%X // V=callplugin(SysID[,param])\n", MCODE_F_CALLPLUGIN);
	fprintf(fp, "MCODE_F_REPLACE=0x%X // S=replace(sS,sF,sR[,Count[,Mode]])\n", MCODE_F_REPLACE);
	fprintf(fp, "MCODE_F_PROMPT=0x%X // S=prompt([\"Title\"[,\"Prompt\"[,flags[, \"Src\"[, \"History\"]]]]])\n", MCODE_F_PROMPT);
	fprintf(fp, "MCODE_F_BM_ADD=0x%X // N=BM.Add()  - �������� ������� ���������� � �������� �����\n", MCODE_F_BM_ADD);
	fprintf(fp, "MCODE_F_BM_CLEAR=0x%X // N=BM.Clear() - �������� ��� ��������\n", MCODE_F_BM_CLEAR);
	fprintf(fp, "MCODE_F_BM_DEL=0x%X // N=BM.Del([Idx]) - ������� �������� � ��������� �������� (x=1...), 0 - ������� ������� ��������\n", MCODE_F_BM_DEL);
	fprintf(fp, "MCODE_F_BM_GET=0x%X // N=BM.Get(Idx,M) - ���������� ���������� ������ (M==0) ��� ������� (M==1) �������� � �������� (Idx=1...)\n", MCODE_F_BM_GET);
	fprintf(fp, "MCODE_F_BM_GOTO=0x%X // N=BM.Goto([n]) - ������� �� �������� � ��������� �������� (0 --> �������)\n", MCODE_F_BM_GOTO);
	fprintf(fp, "MCODE_F_BM_NEXT=0x%X // N=BM.Next() - ������� �� ��������� ��������\n", MCODE_F_BM_NEXT);
	fprintf(fp, "MCODE_F_BM_POP=0x%X // N=BM.Pop() - ������������ ������� ������� �� �������� � ����� ����� � ������� ��������\n", MCODE_F_BM_POP);
	fprintf(fp, "MCODE_F_BM_PREV=0x%X // N=BM.Prev() - ������� �� ���������� ��������\n", MCODE_F_BM_PREV);
	fprintf(fp, "MCODE_F_BM_BACK=0x%X // N=BM.Back() - ������� �� ���������� �������� � ��������� ����������� ������� �������\n", MCODE_F_BM_BACK);
	fprintf(fp, "MCODE_F_BM_PUSH=0x%X // N=BM.Push() - ��������� ������� ������� � ���� �������� � ����� �����\n", MCODE_F_BM_PUSH);
	fprintf(fp, "MCODE_F_BM_STAT=0x%X // N=BM.Stat([M]) - ���������� ���������� � ���������, N=0 - ������� ���������� ��������\n", MCODE_F_BM_STAT);
	fprintf(fp, "MCODE_F_TRIM=0x%X // S=trim(S[,N])\n", MCODE_F_TRIM);
	fprintf(fp, "MCODE_F_FLOAT=0x%X // N=float(V)\n", MCODE_F_FLOAT);
	fprintf(fp, "MCODE_F_TESTFOLDER=0x%X // N=testfolder(S)\n", MCODE_F_TESTFOLDER);
	fprintf(fp, "MCODE_F_PRINT=0x%X // N=Print(Str)\n", MCODE_F_PRINT);
	fprintf(fp, "MCODE_F_MMODE=0x%X // N=MMode(Action[,Value])\n", MCODE_F_MMODE);
	fprintf(fp, "MCODE_F_EDITOR_SETTITLE=0x%X // N=Editor.SetTitle([Title])\n", MCODE_F_EDITOR_SETTITLE);
	fprintf(fp, "MCODE_F_MENU_GETVALUE=0x%X // S=Menu.GetValue([N])\n", MCODE_F_MENU_GETVALUE);
	fprintf(fp, "MCODE_F_MENU_ITEMSTATUS=0x%X // N=Menu.ItemStatus([N])\n", MCODE_F_MENU_ITEMSTATUS);
	fprintf(fp, "MCODE_F_BEEP=0x%X // N=beep([N])\n", MCODE_F_BEEP);
	fprintf(fp, "MCODE_F_KBDLAYOUT=0x%X // N=kbdLayout([N])\n", MCODE_F_KBDLAYOUT);
	fprintf(fp, "MCODE_F_WINDOW_SCROLL=0x%X // N=Window.Scroll(Lines[,Axis])\n", MCODE_F_WINDOW_SCROLL);
	fprintf(fp, "MCODE_F_KEYBAR_SHOW=0x%X // N=KeyBar.Show([N])\n", MCODE_F_KEYBAR_SHOW);
	fprintf(fp, "MCODE_F_HISTORY_DISABLE=0x%X // N=History.Disable([State])\n", MCODE_F_HISTORY_DISABLE);
	fprintf(fp, "MCODE_F_FMATCH=0x%X // N=FMatch(S,Mask)\n", MCODE_F_FMATCH);
	fprintf(fp, "MCODE_F_PLUGIN_MENU=0x%X // N=Plugin.Menu(Guid[,MenuGuid])\n", MCODE_F_PLUGIN_MENU);
	fprintf(fp, "MCODE_F_PLUGIN_CONFIG=0x%X // N=Plugin.Config(Guid[,MenuGuid])\n", MCODE_F_PLUGIN_CONFIG);
	fprintf(fp, "MCODE_F_PLUGIN_CALL=0x%X // N=Plugin.Call(Guid[,Item])\n", MCODE_F_PLUGIN_CALL);
	fprintf(fp, "MCODE_F_PLUGIN_LOAD=0x%X // N=Plugin.Load(DllPath[,ForceLoad])\n", MCODE_F_PLUGIN_LOAD);
	fprintf(fp, "MCODE_F_PLUGIN_COMMAND=0x%X // N=Plugin.Command(Guid[,Command])\n", MCODE_F_PLUGIN_COMMAND);
	fprintf(fp, "MCODE_F_PLUGIN_UNLOAD=0x%X // N=Plugin.UnLoad(DllPath)\n", MCODE_F_PLUGIN_UNLOAD);
	fprintf(fp, "MCODE_F_PLUGIN_EXIST=0x%X // N=Plugin.Exist(Guid)\n", MCODE_F_PLUGIN_EXIST);
	fprintf(fp, "MCODE_F_MENU_FILTER=0x%X // N=Menu.Filter(Action[,Mode])\n", MCODE_F_MENU_FILTER);
	fprintf(fp, "MCODE_F_MENU_FILTERSTR=0x%X // S=Menu.FilterStr([Action[,S]])\n", MCODE_F_MENU_FILTERSTR);
	fprintf(fp, "MCODE_F_DLG_SETFOCUS=0x%X // N=Dlg.SetFocus([ID])\n", MCODE_F_DLG_SETFOCUS);
	fprintf(fp, "MCODE_F_FAR_CFG_GET=0x%X // V=Far.Cfg.Get(Key,Name)\n", MCODE_F_FAR_CFG_GET);
	fprintf(fp, "MCODE_F_SIZE2STR=0x%X // S=Size2Str(N,Flags[,Width])\n", MCODE_F_SIZE2STR);
	fprintf(fp, "MCODE_F_STRWRAP=0x%X // S=StrWrap(Text,Width[,Break[,Flags]])\n", MCODE_F_STRWRAP);
	fprintf(fp, "MCODE_F_MACRO_KEYWORD=0x%X // S=Macro.Keyword(Index[,Type])\n", MCODE_F_MACRO_KEYWORD);
	fprintf(fp, "MCODE_F_MACRO_FUNC=0x%X // S=Macro.Func(Index[,Type])\n", MCODE_F_MACRO_FUNC);
	fprintf(fp, "MCODE_F_MACRO_VAR=0x%X // S=Macro.Var(Index[,Type])\n", MCODE_F_MACRO_VAR);
	fprintf(fp, "MCODE_F_MACRO_CONST=0x%X // S=Macro.Const(Index[,Type])\n", MCODE_F_MACRO_CONST);
	fprintf(fp, "MCODE_F_STRPAD=0x%X // S=StrPad(V,Cnt[,Fill[,Op]])\n", MCODE_F_STRPAD);
	fprintf(fp, "MCODE_F_EDITOR_DELLINE=0x%X // N=Editor.DelLine([Line])\n", MCODE_F_EDITOR_DELLINE);
	fprintf(fp, "MCODE_F_EDITOR_GETSTR=0x%X // S=Editor.GetStr([Line])\n", MCODE_F_EDITOR_GETSTR);
	fprintf(fp, "MCODE_F_EDITOR_INSSTR=0x%X // N=Editor.InsStr([S[,Line]])\n", MCODE_F_EDITOR_INSSTR);
	fprintf(fp, "MCODE_F_EDITOR_SETSTR=0x%X // N=Editor.SetStr([S[,Line]])\n", MCODE_F_EDITOR_SETSTR);
	fprintf(fp, "MCODE_F_POSTNEWMACRO=0x%X // ��������� ������ � ��������� MacroId � Code\n", MCODE_F_POSTNEWMACRO);
	fprintf(fp, "MCODE_F_CHECKALL=0x%X // ��������� ��������������� ������� ���������� �������\n", MCODE_F_CHECKALL);
	fprintf(fp, "MCODE_F_GETOPTIONS=0x%X // �������� �������� ��������� ����� ����\n", MCODE_F_GETOPTIONS);
	fprintf(fp, "MCODE_F_USERMENU=0x%X // ������� ���� ������������\n", MCODE_F_USERMENU);
	fprintf(fp, "MCODE_F_SETCUSTOMSORTMODE=0x%X // ���������� ���������������� ����� ����������\n", MCODE_F_SETCUSTOMSORTMODE);
	fprintf(fp, "MCODE_F_LAST=0x%X // marker\n", MCODE_F_LAST);
	/* ************************************************************************* */
	// ������� ���������� - ��������� ���������
	fprintf(fp, "MCODE_C_AREA_OTHER=0x%X // ����� ����������� ������ � ������, ������������ ����\n", MCODE_C_AREA_OTHER);
	fprintf(fp, "MCODE_C_AREA_SHELL=0x%X // �������� ������\n", MCODE_C_AREA_SHELL);
	fprintf(fp, "MCODE_C_AREA_VIEWER=0x%X // ���������� ��������� ���������\n", MCODE_C_AREA_VIEWER);
	fprintf(fp, "MCODE_C_AREA_EDITOR=0x%X // ��������\n", MCODE_C_AREA_EDITOR);
	fprintf(fp, "MCODE_C_AREA_DIALOG=0x%X // �������\n", MCODE_C_AREA_DIALOG);
	fprintf(fp, "MCODE_C_AREA_SEARCH=0x%X // ������� ����� � �������\n", MCODE_C_AREA_SEARCH);
	fprintf(fp, "MCODE_C_AREA_DISKS=0x%X // ���� ������ ������\n", MCODE_C_AREA_DISKS);
	fprintf(fp, "MCODE_C_AREA_MAINMENU=0x%X // �������� ����\n", MCODE_C_AREA_MAINMENU);
	fprintf(fp, "MCODE_C_AREA_MENU=0x%X // ������ ����\n", MCODE_C_AREA_MENU);
	fprintf(fp, "MCODE_C_AREA_HELP=0x%X // ������� ������\n", MCODE_C_AREA_HELP);
	fprintf(fp, "MCODE_C_AREA_INFOPANEL=0x%X // �������������� ������\n", MCODE_C_AREA_INFOPANEL);
	fprintf(fp, "MCODE_C_AREA_QVIEWPANEL=0x%X // ������ �������� ���������\n", MCODE_C_AREA_QVIEWPANEL);
	fprintf(fp, "MCODE_C_AREA_TREEPANEL=0x%X // ������ ������ �����\n", MCODE_C_AREA_TREEPANEL);
	fprintf(fp, "MCODE_C_AREA_FINDFOLDER=0x%X // ����� �����\n", MCODE_C_AREA_FINDFOLDER);
	fprintf(fp, "MCODE_C_AREA_USERMENU=0x%X // ���� ������������\n", MCODE_C_AREA_USERMENU);
	fprintf(fp, "MCODE_C_AREA_SHELL_AUTOCOMPLETION=0x%X // ������ �������������� � ������� � ���.������\n", MCODE_C_AREA_SHELL_AUTOCOMPLETION);
	fprintf(fp, "MCODE_C_AREA_DIALOG_AUTOCOMPLETION=0x%X // ������ �������������� � �������\n", MCODE_C_AREA_DIALOG_AUTOCOMPLETION);

	fprintf(fp, "MCODE_C_FULLSCREENMODE=0x%X // ������������� �����?\n", MCODE_C_FULLSCREENMODE);
	fprintf(fp, "MCODE_C_ISUSERADMIN=0x%X // Administrator status\n", MCODE_C_ISUSERADMIN);
	fprintf(fp, "MCODE_C_BOF=0x%X // ������ �����/��������� ��������?\n", MCODE_C_BOF);
	fprintf(fp, "MCODE_C_EOF=0x%X // ����� �����/��������� ��������?\n", MCODE_C_EOF);
	fprintf(fp, "MCODE_C_EMPTY=0x%X // ���.������ �����?\n", MCODE_C_EMPTY);
	fprintf(fp, "MCODE_C_SELECTED=0x%X // ���������� ���� ����?\n", MCODE_C_SELECTED);
	fprintf(fp, "MCODE_C_ROOTFOLDER=0x%X // ������ MCODE_C_APANEL_ROOT ��� �������� ������\n", MCODE_C_ROOTFOLDER);

	fprintf(fp, "MCODE_C_APANEL_BOF=0x%X // ������ ���������  ��������?\n", MCODE_C_APANEL_BOF);
	fprintf(fp, "MCODE_C_PPANEL_BOF=0x%X // ������ ���������� ��������?\n", MCODE_C_PPANEL_BOF);
	fprintf(fp, "MCODE_C_APANEL_EOF=0x%X // ����� ���������  ��������?\n", MCODE_C_APANEL_EOF);
	fprintf(fp, "MCODE_C_PPANEL_EOF=0x%X // ����� ���������� ��������?\n", MCODE_C_PPANEL_EOF);
	fprintf(fp, "MCODE_C_APANEL_ISEMPTY=0x%X // �������� ������:  �����?\n", MCODE_C_APANEL_ISEMPTY);
	fprintf(fp, "MCODE_C_PPANEL_ISEMPTY=0x%X // ��������� ������: �����?\n", MCODE_C_PPANEL_ISEMPTY);
	fprintf(fp, "MCODE_C_APANEL_SELECTED=0x%X // �������� ������:  ���������� �������� ����?\n", MCODE_C_APANEL_SELECTED);
	fprintf(fp, "MCODE_C_PPANEL_SELECTED=0x%X // ��������� ������: ���������� �������� ����?\n", MCODE_C_PPANEL_SELECTED);
	fprintf(fp, "MCODE_C_APANEL_ROOT=0x%X // ��� �������� ������� �������� ������?\n", MCODE_C_APANEL_ROOT);
	fprintf(fp, "MCODE_C_PPANEL_ROOT=0x%X // ��� �������� ������� ��������� ������?\n", MCODE_C_PPANEL_ROOT);
	fprintf(fp, "MCODE_C_APANEL_VISIBLE=0x%X // �������� ������:  ������?\n", MCODE_C_APANEL_VISIBLE);
	fprintf(fp, "MCODE_C_PPANEL_VISIBLE=0x%X // ��������� ������: ������?\n", MCODE_C_PPANEL_VISIBLE);
	fprintf(fp, "MCODE_C_APANEL_PLUGIN=0x%X // �������� ������:  ����������?\n", MCODE_C_APANEL_PLUGIN);
	fprintf(fp, "MCODE_C_PPANEL_PLUGIN=0x%X // ��������� ������: ����������?\n", MCODE_C_PPANEL_PLUGIN);
	fprintf(fp, "MCODE_C_APANEL_FILEPANEL=0x%X // �������� ������:  ��������?\n", MCODE_C_APANEL_FILEPANEL);
	fprintf(fp, "MCODE_C_PPANEL_FILEPANEL=0x%X // ��������� ������: ��������?\n", MCODE_C_PPANEL_FILEPANEL);
	fprintf(fp, "MCODE_C_APANEL_FOLDER=0x%X // �������� ������:  ������� ������� �������?\n", MCODE_C_APANEL_FOLDER);
	fprintf(fp, "MCODE_C_PPANEL_FOLDER=0x%X // ��������� ������: ������� ������� �������?\n", MCODE_C_PPANEL_FOLDER);
	fprintf(fp, "MCODE_C_APANEL_LEFT=0x%X // �������� ������ �����?\n", MCODE_C_APANEL_LEFT);
	fprintf(fp, "MCODE_C_PPANEL_LEFT=0x%X // ��������� ������ �����?\n", MCODE_C_PPANEL_LEFT);
	fprintf(fp, "MCODE_C_APANEL_LFN=0x%X // �� �������� ������ ������� �����?\n", MCODE_C_APANEL_LFN);
	fprintf(fp, "MCODE_C_PPANEL_LFN=0x%X // �� ��������� ������ ������� �����?\n", MCODE_C_PPANEL_LFN);
	fprintf(fp, "MCODE_C_APANEL_FILTER=0x%X // �� �������� ������ ������� ������?\n", MCODE_C_APANEL_FILTER);
	fprintf(fp, "MCODE_C_PPANEL_FILTER=0x%X // �� ��������� ������ ������� ������?\n", MCODE_C_PPANEL_FILTER);

	fprintf(fp, "MCODE_C_CMDLINE_BOF=0x%X // ������ � ������ cmd-������ ��������������?\n", MCODE_C_CMDLINE_BOF);
	fprintf(fp, "MCODE_C_CMDLINE_EOF=0x%X // ������ � ����� cmd-������ ��������������?\n", MCODE_C_CMDLINE_EOF);
	fprintf(fp, "MCODE_C_CMDLINE_EMPTY=0x%X // ���.������ �����?\n", MCODE_C_CMDLINE_EMPTY);
	fprintf(fp, "MCODE_C_CMDLINE_SELECTED=0x%X // � ���.������ ���� ��������� �����?\n", MCODE_C_CMDLINE_SELECTED);

	fprintf(fp, "MCODE_C_MSX=0x%X          // Mouse.X\n", MCODE_C_MSX);
	fprintf(fp, "MCODE_C_MSY=0x%X          // Mouse.Y\n", MCODE_C_MSY);
	fprintf(fp, "MCODE_C_MSBUTTON=0x%X     // Mouse.Button\n", MCODE_C_MSBUTTON);
	fprintf(fp, "MCODE_C_MSCTRLSTATE=0x%X  // Mouse.CtrlState\n", MCODE_C_MSCTRLSTATE);
	fprintf(fp, "MCODE_C_MSEVENTFLAGS=0x%X // Mouse.EventFlags\n", MCODE_C_MSEVENTFLAGS);
	fprintf(fp, "MCODE_C_MSLASTCTRLSTATE=0x%X  // Mouse.LastCtrlState\n", MCODE_C_MSLASTCTRLSTATE);

	/* ************************************************************************* */
	// �� ������� ����������
	fprintf(fp, "MCODE_V_FAR_WIDTH=0x%X // Far.Width - ������ ����������� ����\n", MCODE_V_FAR_WIDTH);
	fprintf(fp, "MCODE_V_FAR_HEIGHT=0x%X // Far.Height - ������ ����������� ����\n", MCODE_V_FAR_HEIGHT);
	fprintf(fp, "MCODE_V_FAR_TITLE=0x%X // Far.Title - ������� ��������� ����������� ����\n", MCODE_V_FAR_TITLE);
	fprintf(fp, "MCODE_V_FAR_UPTIME=0x%X // Far.UpTime - ����� ������ Far � �������������\n", MCODE_V_FAR_UPTIME);
	fprintf(fp, "MCODE_V_FAR_PID=0x%X // Far.PID - �������� �� ������� ���������� ����� Far Manager\n", MCODE_V_FAR_PID);
	fprintf(fp, "MCODE_V_MACRO_AREA=0x%X // MacroArea - ��� ������� ������ �������\n", MCODE_V_MACRO_AREA);

	fprintf(fp, "MCODE_V_APANEL_CURRENT=0x%X // APanel.Current - ��� ����� �� �������� ������\n", MCODE_V_APANEL_CURRENT);
	fprintf(fp, "MCODE_V_PPANEL_CURRENT=0x%X // PPanel.Current - ��� ����� �� ��������� ������\n", MCODE_V_PPANEL_CURRENT);
	fprintf(fp, "MCODE_V_APANEL_SELCOUNT=0x%X // APanel.SelCount - �������� ������:  ����� ���������� ���������\n", MCODE_V_APANEL_SELCOUNT);
	fprintf(fp, "MCODE_V_PPANEL_SELCOUNT=0x%X // PPanel.SelCount - ��������� ������: ����� ���������� ���������\n", MCODE_V_PPANEL_SELCOUNT);
	fprintf(fp, "MCODE_V_APANEL_PATH=0x%X // APanel.Path - �������� ������:  ���� �� ������\n", MCODE_V_APANEL_PATH);
	fprintf(fp, "MCODE_V_PPANEL_PATH=0x%X // PPanel.Path - ��������� ������: ���� �� ������\n", MCODE_V_PPANEL_PATH);
	fprintf(fp, "MCODE_V_APANEL_PATH0=0x%X // APanel.Path0 - �������� ������:  ���� �� ������ �� ������ ��������\n", MCODE_V_APANEL_PATH0);
	fprintf(fp, "MCODE_V_PPANEL_PATH0=0x%X // PPanel.Path0 - ��������� ������: ���� �� ������ �� ������ ��������\n", MCODE_V_PPANEL_PATH0);
	fprintf(fp, "MCODE_V_APANEL_UNCPATH=0x%X // APanel.UNCPath - �������� ������:  UNC-���� �� ������\n", MCODE_V_APANEL_UNCPATH);
	fprintf(fp, "MCODE_V_PPANEL_UNCPATH=0x%X // PPanel.UNCPath - ��������� ������: UNC-���� �� ������\n", MCODE_V_PPANEL_UNCPATH);
	fprintf(fp, "MCODE_V_APANEL_WIDTH=0x%X // APanel.Width - �������� ������:  ������ ������\n", MCODE_V_APANEL_WIDTH);
	fprintf(fp, "MCODE_V_PPANEL_WIDTH=0x%X // PPanel.Width - ��������� ������: ������ ������\n", MCODE_V_PPANEL_WIDTH);
	fprintf(fp, "MCODE_V_APANEL_TYPE=0x%X // APanel.Type - ��� �������� ������\n", MCODE_V_APANEL_TYPE);
	fprintf(fp, "MCODE_V_PPANEL_TYPE=0x%X // PPanel.Type - ��� ��������� ������\n", MCODE_V_PPANEL_TYPE);
	fprintf(fp, "MCODE_V_APANEL_ITEMCOUNT=0x%X // APanel.ItemCount - �������� ������:  ����� ���������\n", MCODE_V_APANEL_ITEMCOUNT);
	fprintf(fp, "MCODE_V_PPANEL_ITEMCOUNT=0x%X // PPanel.ItemCount - ��������� ������: ����� ���������\n", MCODE_V_PPANEL_ITEMCOUNT);
	fprintf(fp, "MCODE_V_APANEL_CURPOS=0x%X // APanel.CurPos - �������� ������:  ������� ������\n", MCODE_V_APANEL_CURPOS);
	fprintf(fp, "MCODE_V_PPANEL_CURPOS=0x%X // PPanel.CurPos - ��������� ������: ������� ������\n", MCODE_V_PPANEL_CURPOS);
	fprintf(fp, "MCODE_V_APANEL_OPIFLAGS=0x%X // APanel.OPIFlags - �������� ������: ����� ��������� �������\n", MCODE_V_APANEL_OPIFLAGS);
	fprintf(fp, "MCODE_V_PPANEL_OPIFLAGS=0x%X // PPanel.OPIFlags - ��������� ������: ����� ��������� �������\n", MCODE_V_PPANEL_OPIFLAGS);
	fprintf(fp, "MCODE_V_APANEL_DRIVETYPE=0x%X // APanel.DriveType - �������� ������: ��� �������\n", MCODE_V_APANEL_DRIVETYPE);
	fprintf(fp, "MCODE_V_PPANEL_DRIVETYPE=0x%X // PPanel.DriveType - ��������� ������: ��� �������\n", MCODE_V_PPANEL_DRIVETYPE);
	fprintf(fp, "MCODE_V_APANEL_HEIGHT=0x%X // APanel.Height - �������� ������:  ������ ������\n", MCODE_V_APANEL_HEIGHT);
	fprintf(fp, "MCODE_V_PPANEL_HEIGHT=0x%X // PPanel.Height - ��������� ������: ������ ������\n", MCODE_V_PPANEL_HEIGHT);
	fprintf(fp, "MCODE_V_APANEL_COLUMNCOUNT=0x%X // APanel.ColumnCount - �������� ������:  ���������� �������\n", MCODE_V_APANEL_COLUMNCOUNT);
	fprintf(fp, "MCODE_V_PPANEL_COLUMNCOUNT=0x%X // PPanel.ColumnCount - ��������� ������: ���������� �������\n", MCODE_V_PPANEL_COLUMNCOUNT);
	fprintf(fp, "MCODE_V_APANEL_HOSTFILE=0x%X // APanel.HostFile - �������� ������:  ��� Host-�����\n", MCODE_V_APANEL_HOSTFILE);
	fprintf(fp, "MCODE_V_PPANEL_HOSTFILE=0x%X // PPanel.HostFile - ��������� ������: ��� Host-�����\n", MCODE_V_PPANEL_HOSTFILE);
	fprintf(fp, "MCODE_V_APANEL_PREFIX=0x%X // APanel.Prefix\n", MCODE_V_APANEL_PREFIX);
	fprintf(fp, "MCODE_V_PPANEL_PREFIX=0x%X // PPanel.Prefix\n", MCODE_V_PPANEL_PREFIX);
	fprintf(fp, "MCODE_V_APANEL_FORMAT=0x%X // APanel.Format\n", MCODE_V_APANEL_FORMAT);
	fprintf(fp, "MCODE_V_PPANEL_FORMAT=0x%X // PPanel.Format\n", MCODE_V_PPANEL_FORMAT);

	fprintf(fp, "MCODE_V_ITEMCOUNT=0x%X // ItemCount - ����� ��������� � ������� �������\n", MCODE_V_ITEMCOUNT);
	fprintf(fp, "MCODE_V_CURPOS=0x%X // CurPos - ������� ������ � ������� �������\n", MCODE_V_CURPOS);
	fprintf(fp, "MCODE_V_TITLE=0x%X // Title - ��������� �������� �������\n", MCODE_V_TITLE);
	fprintf(fp, "MCODE_V_HEIGHT=0x%X // Height - ������ �������� �������\n", MCODE_V_HEIGHT);
	fprintf(fp, "MCODE_V_WIDTH=0x%X // Width - ������ �������� �������\n", MCODE_V_WIDTH);

	fprintf(fp, "MCODE_V_EDITORFILENAME=0x%X // Editor.FileName - ��� �������������� �����\n", MCODE_V_EDITORFILENAME);
	fprintf(fp, "MCODE_V_EDITORLINES=0x%X // Editor.Lines - ���������� ����� � ���������\n", MCODE_V_EDITORLINES);
	fprintf(fp, "MCODE_V_EDITORCURLINE=0x%X // Editor.CurLine - ������� ����� � ��������� (� ���������� � Count)\n", MCODE_V_EDITORCURLINE);
	fprintf(fp, "MCODE_V_EDITORCURPOS=0x%X // Editor.CurPos - ������� ���. � ���������\n", MCODE_V_EDITORCURPOS);
	fprintf(fp, "MCODE_V_EDITORREALPOS=0x%X // Editor.RealPos - ������� ���. � ��������� ��� �������� � ������� ���������\n", MCODE_V_EDITORREALPOS);
	fprintf(fp, "MCODE_V_EDITORSTATE=0x%X // Editor.State\n", MCODE_V_EDITORSTATE);
	fprintf(fp, "MCODE_V_EDITORVALUE=0x%X // Editor.Value - ���������� ������� ������\n", MCODE_V_EDITORVALUE);
	fprintf(fp, "MCODE_V_EDITORSELVALUE=0x%X // Editor.SelValue - �������� ���������� ����������� �����\n", MCODE_V_EDITORSELVALUE);

	fprintf(fp, "MCODE_V_DLGITEMTYPE=0x%X // Dlg.ItemType\n", MCODE_V_DLGITEMTYPE);
	fprintf(fp, "MCODE_V_DLGITEMCOUNT=0x%X // Dlg.ItemCount\n", MCODE_V_DLGITEMCOUNT);
	fprintf(fp, "MCODE_V_DLGCURPOS=0x%X // Dlg.CurPos\n", MCODE_V_DLGCURPOS);
	fprintf(fp, "MCODE_V_DLGPREVPOS=0x%X // Dlg.PrevPos\n", MCODE_V_DLGPREVPOS);
	fprintf(fp, "MCODE_V_DLGINFOID=0x%X // Dlg.Info.Id\n", MCODE_V_DLGINFOID);
	fprintf(fp, "MCODE_V_DLGINFOOWNER=0x%X // Dlg.Info.Owner\n", MCODE_V_DLGINFOOWNER);

	fprintf(fp, "MCODE_V_VIEWERFILENAME=0x%X // Viewer.FileName - ��� ���������������� �����\n", MCODE_V_VIEWERFILENAME);
	fprintf(fp, "MCODE_V_VIEWERSTATE=0x%X // Viewer.State\n", MCODE_V_VIEWERSTATE);

	fprintf(fp, "MCODE_V_CMDLINE_ITEMCOUNT=0x%X // CmdLine.ItemCount\n", MCODE_V_CMDLINE_ITEMCOUNT);
	fprintf(fp, "MCODE_V_CMDLINE_CURPOS=0x%X // CmdLine.CurPos\n", MCODE_V_CMDLINE_CURPOS);
	fprintf(fp, "MCODE_V_CMDLINE_VALUE=0x%X // CmdLine.Value\n", MCODE_V_CMDLINE_VALUE);

	fprintf(fp, "MCODE_V_DRVSHOWPOS=0x%X // Drv.ShowPos - ���� ������ ������ ����������: 1=����� (Alt-F1), 2=������ (Alt-F2), 0=\"���� ���\"\n", MCODE_V_DRVSHOWPOS);
	fprintf(fp, "MCODE_V_DRVSHOWMODE=0x%X // Drv.ShowMode - ������ ����������� ���� ������ ������\n", MCODE_V_DRVSHOWMODE);

	fprintf(fp, "MCODE_V_HELPFILENAME=0x%X // Help.FileName\n", MCODE_V_HELPFILENAME);
	fprintf(fp, "MCODE_V_HELPTOPIC=0x%X // Help.Topic\n", MCODE_V_HELPTOPIC);
	fprintf(fp, "MCODE_V_HELPSELTOPIC=0x%X // Help.SelTopic\n", MCODE_V_HELPSELTOPIC);

	fprintf(fp, "MCODE_V_MENU_VALUE=0x%X // Menu.Value\n", MCODE_V_MENU_VALUE);
	fprintf(fp, "MCODE_V_MENUINFOID=0x%X // Menu.Info.Id\n", MCODE_V_MENUINFOID);

	fclose(fp);
}
#endif

// ��� ������� ���������� �������
struct DlgParam
{
	UINT64 Flags;
	DWORD Key;
	FARMACROAREA Mode;
	int Recurse;
	bool Changed;
};

static bool ToDouble(__int64 v, double *d)
{
	if ((v >= 0 && v <= 0x1FFFFFFFFFFFFFLL) || (v < 0 && -v <= 0x1FFFFFFFFFFFFFLL))
	{
		*d = (double)v;
		return true;
	}
	return false;
}

RunningMacro::RunningMacro()
{
	SetBooleanValue(0);
	mp_data.StructSize = sizeof(mp_data);
	mp_data.Count = 0;
	mp_data.Values = mp_values;
	mp_data.Callback = nullptr;
	mp_data.CallbackData = nullptr;

	mp_info.CallType = MCT_MACROSTEP;
	mp_info.Handle = 0;
	mp_info.Data = &mp_data;
}

RunningMacro& RunningMacro::operator= (const RunningMacro& src)
{
	memcpy(mp_values, src.mp_values, sizeof(mp_values));
	mp_data = src.mp_data;
	mp_data.Values = mp_values;
	mp_info = src.mp_info;
	mp_info.Data = &mp_data;
	return *this;
}

MacroRecord::MacroRecord():
	m_flags(0),
	m_key(-1),
	m_macroId(0),
	m_running()
{
}

MacroRecord::MacroRecord(MACROFLAGS_MFLAGS Flags,int MacroId,int Key,const wchar_t* Code,const wchar_t* Description):
	m_flags(Flags),
	m_key(Key),
	m_code(Code),
	m_description(Description),
	m_macroId(MacroId),
	m_running()
{
}

MacroRecord& MacroRecord::operator= (const MacroRecord& src)
{
	if (this != &src)
	{
		m_flags = src.m_flags;
		m_key = src.m_key;
		m_code = src.m_code;
		m_description = src.m_description;
		m_macroId = src.m_macroId;
		m_running = src.m_running;
	}
	return *this;
}

MacroState::MacroState() :
	cRec(),
	Executing(0),
	KeyProcess(0),
	HistoryDisable(0),
	UseInternalClipboard(false)
{
}

void KeyMacro::PushState(bool withClip)
{
	if (withClip)
		m_CurState->UseInternalClipboard=Clipboard::GetUseInternalClipboardState();

	m_StateStack.push(m_CurState);
	m_CurState = new MacroState();
}

void KeyMacro::PopState(bool withClip)
{
	if (!m_StateStack.empty())
	{
		if(!m_CurState->m_MacroQueue.empty())
		{
			MacroState* dst = m_StateStack.top();
			dst->m_MacroQueue.splice(dst->m_MacroQueue.end(), m_CurState->m_MacroQueue);
		}
		delete m_CurState;
		m_CurState = m_StateStack.top();
		m_StateStack.pop();

		if (withClip)
			Clipboard::SetUseInternalClipboardState(m_CurState->UseInternalClipboard);
	}
}

KeyMacro::KeyMacro():
	m_Mode(MACROAREA_SHELL),
	m_Recording(MACROMODE_NOMACRO),
	m_RecMode(MACROAREA_OTHER),
	StartMode(MACROAREA_OTHER),
	m_LockScr(nullptr),
	m_LastErrorLine(0),
	m_InternalInput(0),
	m_MacroPluginIsRunning(0),
	m_DisableNested(0),
	m_WaitKey(0),
	varTextDate()
{
	//print_opcodes();
	m_CurState = new MacroState();
}

KeyMacro::~KeyMacro()
{
	while (!m_StateStack.empty()) PopState(false);
	delete m_CurState;
}

// ������������� ���� ����������
void KeyMacro::InitInternalVars(bool InitedRAM)
{
	if (InitedRAM)
	{
		m_CurState->m_MacroQueue.clear();
		m_CurState->Executing=MACROMODE_NOMACRO;
	}
	m_CurState->HistoryDisable=0;
	m_Recording=MACROMODE_NOMACRO;
	m_InternalInput=0;
}

int KeyMacro::IsExecuting()
{
	MacroRecord* m = GetCurMacro();
	if (m && m->GetHandle())
		return m->Flags()&MFLAGS_NOSENDKEYSTOPLUGINS ? MACROMODE_EXECUTING : MACROMODE_EXECUTING_COMMON;
	else
		return m_StateStack.empty() ? MACROMODE_NOMACRO : MACROMODE_EXECUTING_COMMON;
}

int KeyMacro::IsDisableOutput()
{
	MacroRecord* m = GetCurMacro();
	return m && !(m->Flags()&MFLAGS_ENABLEOUTPUT);
}

DWORD KeyMacro::SetHistoryDisableMask(DWORD Mask)
{
	DWORD OldHistoryDisable=m_CurState->HistoryDisable;
	m_CurState->HistoryDisable=Mask;
	return OldHistoryDisable;
}

DWORD KeyMacro::GetHistoryDisableMask()
{
	return m_CurState->HistoryDisable;
}

bool KeyMacro::IsHistoryDisable(int TypeHistory)
{
	return !m_CurState->m_MacroQueue.empty() && (m_CurState->HistoryDisable & (1 << TypeHistory));
}

bool KeyMacro::LoadMacros(bool InitedRAM,bool LoadAll)
{
	if (Global->Opt->Macro.DisableMacro&MDOL_ALL)
		return false;

	InitInternalVars(InitedRAM);

	FarMacroValue values[1]={{FMVT_BOOLEAN}};
	values[0].Boolean = LoadAll ? 1:0;
	FarMacroCall fmc={sizeof(FarMacroCall),ARRAYSIZE(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_LOADMACROS,0,&fmc};
	return CallMacroPlugin(&info);
}

bool KeyMacro::SaveMacros()
{
	OpenMacroPluginInfo info={MCT_WRITEMACROS,0,nullptr};
	return CallMacroPlugin(&info);
}

static __int64 msValues[constMsLAST];

void KeyMacro::SetMacroConst(int ConstIndex, __int64 Value)
{
	msValues[ConstIndex] = Value;
}

int KeyMacro::GetCurRecord()
{
	return (m_Recording != MACROMODE_NOMACRO) ? m_Recording : IsExecuting();
}

bool KeyMacro::CallMacroPlugin(OpenMacroPluginInfo* Info)
{
	void* ptr;
	MacroRecord* macro = GetCurMacro();

	if (macro)
		Global->ScrBuf->ResetLockCount();

	if (Info->CallType==MCT_MACROINIT || Info->CallType==MCT_MACROSTEP)
	{
		++m_MacroPluginIsRunning;
		PushState(false);
	}
	bool result=Global->CtrlObject->Plugins->CallPlugin(LuamacroGuid,OPEN_LUAMACRO,Info,&ptr) != 0;
	if (Info->CallType==MCT_MACROINIT || Info->CallType==MCT_MACROSTEP)
	{
		PopState(false);
		--m_MacroPluginIsRunning;
	}

	if (result && macro && macro->GetHandle() && !(macro->Flags()&MFLAGS_ENABLEOUTPUT) && Info->CallType==MCT_MACROSTEP)
		Global->ScrBuf->Lock();

	return result && ptr;
}

bool KeyMacro::InitMacroExecution()
{
	//_SHMUEL(SysLog(L"+InitMacroExecution"));
	MacroRecord* macro = GetCurMacro();
	if (macro)
	{
		FarMacroValue values[2] = {{FMVT_DOUBLE,{0}},{FMVT_STRING,{0}}};
		FarMacroCall fmc = {sizeof(FarMacroCall),1,values,nullptr,nullptr};
		OpenMacroPluginInfo info = {MCT_MACROINIT,0,&fmc};

		values[0].Double = macro->m_macroId;
		if (macro->m_macroId == 0)
		{
			fmc.Count = 2;
			values[1].String = macro->Code().data();
		}

		if (CallMacroPlugin(&info))
		{
			macro->SetHandle(info.Ret.ReturnType);
			return true;
		}
		RemoveCurMacro();
		RestoreMacroChar();
	}
	return false;
}

void KeyMacro::RestoreMacroChar(void)
{
	Global->ScrBuf->RestoreMacroChar();

	/*$ 10.08.2000 skv
		If we are in editor mode, and CurEditor defined,
		we need to call this events.
		EE_REDRAW EEREDRAW_ALL    - to notify that whole screen updated
		->Show() to actually update screen.

		This duplication take place since ShowEditor method
		will NOT send this event while screen is locked.
	*/
	if (m_Mode==MACROAREA_EDITOR &&
					Global->CtrlObject->Plugins->GetCurEditor() &&
					Global->CtrlObject->Plugins->GetCurEditor()->IsVisible()
					/* && LockScr*/) // Mantis#0001595
	{
		Global->CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL,Global->CtrlObject->Plugins->GetCurEditor()->GetId());
		Global->CtrlObject->Plugins->GetCurEditor()->Show();
	}
	else if (m_Mode==MACROAREA_VIEWER &&
					Global->CtrlObject->Plugins->GetCurViewer() &&
					Global->CtrlObject->Plugins->GetCurViewer()->IsVisible())
	{
		Global->CtrlObject->Plugins->GetCurViewer()->Show(); // ����� ����� ���� ������������ ������� ����� ������ ������
	}
}

struct GetMacroData
{
	int MacroId;
	FARMACROAREA Area;
	const wchar_t *Code;
	const wchar_t *Description;
	MACROFLAGS_MFLAGS Flags;
	GUID Guid;
	FARMACROCALLBACK Callback;
	void* CallbackId;
};

// ��� ������� ����� ������� ���� ������ �������
bool KeyMacro::LM_GetMacro(GetMacroData* Data, FARMACROAREA Mode, const string& TextKey, bool UseCommon,
	bool CheckOnly)
{
	FarMacroValue InValues[4]={{FMVT_DOUBLE},{FMVT_STRING},{FMVT_BOOLEAN},{FMVT_BOOLEAN}};
	InValues[0].Double=Mode;
	InValues[1].String=TextKey.data();
	InValues[2].Boolean=(UseCommon?1:0);
	InValues[3].Boolean=(CheckOnly?1:0);

	FarMacroCall fmc={sizeof(FarMacroCall),ARRAYSIZE(InValues),InValues,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_GETMACRO,0,&fmc};

	if (CallMacroPlugin(&info))
	{
		size_t Count = info.Ret.Count;
		const FarMacroValue* Values = info.Ret.Values;
		Data->MacroId = (int)Values[0].Double;
		if (Data->MacroId != 0)
		{
			Data->Area        = (FARMACROAREA)(int)Values[1].Double;
			Data->Code        = Values[2].Type==FMVT_STRING ? Values[2].String : L"";
			Data->Description = Values[3].Type==FMVT_STRING ? Values[3].String : L"";
			Data->Flags       = (MACROFLAGS_MFLAGS)Values[4].Double;

			Data->Guid        = (Count>=6 && Values[5].Type==FMVT_BINARY)  ? *(GUID*)Values[5].Binary.Data : FarGuid;
			Data->Callback    = (Count>=7 && Values[6].Type==FMVT_POINTER) ? (FARMACROCALLBACK)Values[6].Pointer : nullptr;
			Data->CallbackId  = (Count>=8 && Values[7].Type==FMVT_POINTER) ? Values[7].Pointer : nullptr;
		}
		return true;
	}
	return false;
}

bool KeyMacro::MacroExists(int Key, FARMACROAREA CheckMode, bool UseCommon)
{
	GetMacroData dummy;
	string strKey;
	return KeyToText(Key,strKey) && LM_GetMacro(&dummy,CheckMode,strKey,UseCommon,true);
}

void KeyMacro::LM_ProcessMacro(FARMACROAREA Mode, const string& TextKey, const string& Code, MACROFLAGS_MFLAGS Flags,
	const string& Description, const GUID* Guid, FARMACROCALLBACK Callback, void* CallbackId)
{
	FarMacroValue values[8]={{FMVT_DOUBLE},{FMVT_STRING},{FMVT_STRING},{FMVT_INTEGER},{FMVT_STRING},{FMVT_BINARY},{FMVT_POINTER},{FMVT_POINTER}};

	values[0].Double=Mode;
	values[1].String=TextKey.data();
	values[2].String=Code.data();
	values[3].Integer=Flags;
	values[4].String=Description.data();
	if (Guid)
	{
		values[5].Binary.Data=const_cast<GUID*>(Guid);
		values[5].Binary.Size=sizeof(GUID);
	}
	else
	{
		values[5].Type=FMVT_BOOLEAN;
		values[5].Boolean=0;
	}
	values[6].Pointer=(void*)Callback;
	values[7].Pointer=CallbackId;

	FarMacroCall fmc={sizeof(FarMacroCall),ARRAYSIZE(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_PROCESSMACRO,0,&fmc};
	CallMacroPlugin(&info);
}

int KeyMacro::ProcessEvent(const FAR_INPUT_RECORD *Rec)
{
	//_SHMUEL(SysLog(L"+KeyMacro::ProcessEvent"));
	if (m_InternalInput || Rec->IntKey==KEY_IDLE || Rec->IntKey==KEY_NONE || !FrameManager->GetCurrentFrame()) //FIXME: ���������� �� Rec->IntKey
		return false;
	//{FILE* log=fopen("c:\\lua.log","at"); if(log) {fprintf(log,"ProcessEvent: %08x\n",Rec->IntKey); fclose(log);}}
	string textKey;
	//if (InputRecordToText(&Rec->Rec,textKey))//FIXME: �� ����� Ctrl ��� ��� �������, � �� �����.
	if (KeyToText(Rec->IntKey,textKey))
	{
		bool ctrldot = Rec->IntKey == Global->Opt->Macro.KeyMacroCtrlDot || Rec->IntKey == Global->Opt->Macro.KeyMacroRCtrlDot;
		bool ctrlshiftdot = Rec->IntKey == Global->Opt->Macro.KeyMacroCtrlShiftDot || Rec->IntKey == Global->Opt->Macro.KeyMacroRCtrlShiftDot;

		if (m_Recording==MACROMODE_NOMACRO)
		{
			if ((ctrldot||ctrlshiftdot) && !IsExecuting())
			{
				// ������� 18
				if (Global->Opt->Policies.DisabledOptions&FFPOL_CREATEMACRO)
					return false;

				if (!Global->CtrlObject->Plugins->FindPlugin(LuamacroGuid))
				{
					Message(MSG_WARNING,1,MSG(MError),
					   MSG(MMacroPluginLuamacroNotLoaded),
					   MSG(MMacroRecordingIsDisabled),
					   MSG(MOk));
					return false;
				}

				UpdateLockScreen(false);

				// ��� ��?
				m_RecMode=(m_Mode==MACROAREA_SHELL&&!Global->WaitInMainLoop)?MACROAREA_OTHER:m_Mode;
				StartMode=m_RecMode;
				// � ����������� �� ����, ��� ������ ������ ������, ��������� ����� ����� (Ctrl-.
				// � ��������� ������� ����) ��� ����������� (Ctrl-Shift-. - ��� �������� ������ �������)
				m_Recording=ctrldot?MACROMODE_RECORDING_COMMON:MACROMODE_RECORDING;

				m_RecCode.clear();
				m_RecDescription.clear();
				Global->ScrBuf->ResetShadow();
				Global->ScrBuf->Flush();
				Global->WaitInFastFind--;
				return true;
			}
			else
			{
				if (!IsExecuting()||(m_CurState->m_MacroQueue.empty()&&!m_DisableNested))
				{
					DWORD key = Rec->IntKey;
					if ((key&0x00FFFFFF) > 0x7F && (key&0x00FFFFFF) < 0xFFFF)
						key=KeyToKeyLayout(key&0x0000FFFF)|(key&~0x0000FFFF);

					if (key<0xFFFF)
						key=Upper(static_cast<wchar_t>(key));

					if (key != Rec->IntKey)
						KeyToText(key,textKey);

					INPUT_RECORD RecCopy = Rec->Rec;
					GetMacroData Data;
					if (LM_GetMacro(&Data, m_Mode, textKey, true, false))
					{
						if (Data.MacroId && PostNewMacro(Data.MacroId, Data.Code, Data.Flags, Rec->IntKey))
						{
							m_CurState->HistoryDisable = 0;
							m_CurState->cRec = RecCopy;
						}
						return true;
					}

					// Mantis 0002307: ��� ������ msgbox �� condition(), ���� �������� msgbox ��������� ������ (�� ���������)
					if (FrameManager)
						FrameManager->SetLastInputRecord(&RecCopy);
				}
			}
		}
		else // m_Recording!=MACROMODE_NOMACRO
		{
			if (ctrldot||ctrlshiftdot) // ������� ����� ������?
			{
				int WaitInMainLoop0=Global->WaitInMainLoop;
				m_InternalInput=1;
				Global->WaitInMainLoop=FALSE;
				// �������� _�������_ �����, � �� _��������� �����������_
				FrameManager->GetCurrentFrame()->Lock(); // ������� ���������� ������
				DWORD MacroKey;
				// ���������� ����� �� ���������.
				UINT64 Flags=MFLAGS_CALLPLUGINENABLEMACRO;
				int AssignRet=AssignMacroKey(MacroKey,Flags);
				FrameManager->ResetLastInputRecord();
				FrameManager->GetCurrentFrame()->Unlock(); // ������ ����� :-)

				if (AssignRet && AssignRet!=2 && !m_RecCode.empty())
				{
					m_RecCode = L"Keys(\"" + m_RecCode + L"\")";
					// ������� �������� �� ��������
					// ���� ������� ��� ��� ������ ������ ���������, �� �� ����� �������� ������ ���������.
					//if (MacroKey != (DWORD)-1 && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
					if (ctrlshiftdot && !GetMacroSettings(MacroKey,Flags))
					{
						AssignRet=0;
					}
				}

				Global->WaitInMainLoop=WaitInMainLoop0;
				m_InternalInput=0;
				if (AssignRet)
				{
					string strKey;
					KeyToText(MacroKey, strKey);
					Flags |= (m_Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);
					LM_ProcessMacro(StartMode,strKey,m_RecCode,Flags,m_RecDescription);
				}

				//{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"%ls\n",m_RecCode.data()); fclose(log);}}
				m_Recording=MACROMODE_NOMACRO;
				m_RecCode.clear();
				m_RecDescription.clear();
				Global->ScrBuf->RestoreMacroChar();
				Global->WaitInFastFind++;

				if (Global->Opt->AutoSaveSetup)
					SaveMacros(); // �������� ������ ���������!

				return true;
			}
			else
			{
				//{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"key: %08x\n",Rec->IntKey); fclose(log);}}
				if (!Global->IsProcessAssignMacroKey)
				{
					if (!m_RecCode.empty()) m_RecCode+=L" ";
					if (textKey==L"\"") textKey=L"\\\"";
					m_RecCode+=textKey;
				}
				return false;
			}
		}
	}
	return false;
}

static void ShowUserMenu(size_t Count, const FarMacroValue *Values)
{
	if (Count==0)
		UserMenu uMenu(false);
	else if (Values[0].Type==FMVT_BOOLEAN)
		UserMenu uMenu(Values[0].Boolean != 0);
	else if (Values[0].Type==FMVT_STRING)
		UserMenu uMenu(string(Values[0].String));
}

int KeyMacro::GetKey()
{
	if (m_InternalInput || !FrameManager->GetCurrentFrame())
	{
		return 0;
	}

	if (m_CurState->m_MacroQueue.empty() && !m_MacroPluginIsRunning)
	{
		if (!m_StateStack.empty())
		{
			PopState(true);
			return 0;
		}

		if (m_Mode==MACROAREA_EDITOR &&
						Global->CtrlObject->Plugins->GetCurEditor() &&
						Global->CtrlObject->Plugins->GetCurEditor()->IsVisible() &&
						Global->ScrBuf->GetLockCount())
		{
			Global->CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL,Global->CtrlObject->Plugins->GetCurEditor()->GetId());
			Global->CtrlObject->Plugins->GetCurEditor()->Show();
		}

		if (m_StateStack.empty())
			Global->ScrBuf->Unlock();

		if (ConsoleTitle::WasTitleModified())
			ConsoleTitle::RestoreTitle();

		Clipboard::SetUseInternalClipboardState(false);
		return 0;
	}

	MacroRecord* macro;
	while ((macro=GetCurMacro()) != nullptr && (macro->GetHandle() || InitMacroExecution()))
	{
		OpenMacroPluginInfo* ompInfo = macro->GetMPInfo();
		MacroPluginReturn* mpr = CallMacroPlugin(ompInfo) ? &ompInfo->Ret : nullptr;
		macro->ResetMPInfo();

		switch (mpr ? mpr->ReturnType : MPRT_ERRORFINISH)
		{
			default:
			case MPRT_NORMALFINISH:
			case MPRT_ERRORFINISH:
			{
				if (mpr == nullptr)
				{
					OpenMacroPluginInfo *info = macro->GetMPInfo();
					info->CallType = MCT_MACROFINAL;
					CallMacroPlugin(info);
				}

				if (!(macro->Flags() & MFLAGS_ENABLEOUTPUT))
					Global->ScrBuf->Unlock();

				RemoveCurMacro();
				if (m_CurState->m_MacroQueue.empty())
				{
					RestoreMacroChar();
					return 0;
				}

				continue;
			}

			case MPRT_KEYS:
			{
				const wchar_t* key = mpr->Values[0].String;

				if (!StrCmpI(key, L"AKey"))
				{
					DWORD aKey=KEY_NONE;
					if (!(macro->Flags()&MFLAGS_POSTFROMPLUGIN))
					{
						INPUT_RECORD *inRec=&m_CurState->cRec;
						if (!inRec->EventType)
							inRec->EventType = KEY_EVENT;
						if(inRec->EventType == MOUSE_EVENT || inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
							aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
					}
					else
						aKey=macro->Key();
					//_SHMUEL(SysLog(L"-KeyMacro::GetKey, returned 0x%X", aKey));
					return aKey;
				}

				if (!StrCmpI(key, L"SelWord"))
					return KEY_OP_SELWORD;

				if (!StrCmpI(key, L"XLat"))
					return KEY_OP_XLAT;

				int iKey = KeyNameToKey(key);
				//_SHMUEL(SysLog(L"-KeyMacro::GetKey, returned 0x%X", iKey==-1 ? KEY_NONE:iKey));
				return iKey==-1 ? KEY_NONE:iKey;
			}

			case MPRT_PRINT:
			{
				varTextDate = mpr->Values[0].String;
				return KEY_OP_PLAINTEXT;
			}

			case MPRT_PLUGINCALL: // V=Plugin.Call(SysID[,param])
			{
				size_t count = mpr->Count;
				macro->SetBooleanValue(0);
				if(count>0 && mpr->Values[0].Type==FMVT_STRING)
				{
					const wchar_t* SysID = mpr->Values[0].String;
					GUID guid;

					if (StrToGuid(SysID,guid) && Global->CtrlObject->Plugins->FindPlugin(guid))
					{
						FarMacroCall* ResultCallPlugin=nullptr;

						FarMacroValue *vParams = count>1 ? mpr->Values+1:nullptr;
						OpenMacroInfo info={sizeof(OpenMacroInfo),count-1,vParams};
						MacroRecord* macro = GetCurMacro();
						bool CallPluginRules = (macro->Flags()&MFLAGS_CALLPLUGINENABLEMACRO) != 0;
						size_t EntryStackSize = m_StateStack.size();

						if (CallPluginRules)
						{
							PushState(true);
							macro->SetBooleanValue(1);
						}
						else
							m_InternalInput++;

						int lockCount = Global->ScrBuf->GetLockCount();
						Global->ScrBuf->SetLockCount(0);

						if (!Global->CtrlObject->Plugins->CallPlugin(guid,OPEN_FROMMACRO,&info,(void**)&ResultCallPlugin))
							ResultCallPlugin = nullptr;

						Global->ScrBuf->SetLockCount(lockCount);

						bool isSynchroCall=true;
						if (CallPluginRules)
						{
							if (m_StateStack.size() > EntryStackSize) // ��� �������� �����, �.�. PopState() ��� ��� ���� ������.
								PopState(true);
							else
								isSynchroCall=false;
						}
						else
							m_InternalInput--;

						if (isSynchroCall)
						{
							//� windows �������������, ��� �� ������ ���������� ������ 0x10000
							if (reinterpret_cast<uintptr_t>(ResultCallPlugin) >= 0x10000 && ResultCallPlugin != INVALID_HANDLE_VALUE)
								macro->SetData(ResultCallPlugin);
							else
								macro->SetBooleanValue(ResultCallPlugin != nullptr);
						}
					}
				}

				break;
			}

			case MPRT_PLUGINMENU:   // N=Plugin.Menu(Guid[,MenuGuid])
			case MPRT_PLUGINCONFIG: // N=Plugin.Config(Guid[,MenuGuid])
			case MPRT_PLUGINCOMMAND: // N=Plugin.Command(Guid[,Command])
			{
				const wchar_t *Arg = nullptr;
				const wchar_t *Guid = nullptr;
				GUID guid, menuGuid;
				CallPluginInfo cpInfo={CPT_CHECKONLY};
				bool ItemFailed=false;

				macro->SetBooleanValue(0);

				if (mpr->Count>0 && mpr->Values[0].Type==FMVT_STRING)
					Guid = mpr->Values[0].String;
				else
					break;

				if (mpr->Count>1 && mpr->Values[1].Type==FMVT_STRING)
					Arg=mpr->Values[1].String;

				switch (mpr->ReturnType)
				{
					default:
					case MPRT_PLUGINMENU:
						cpInfo.CallFlags |= CPT_MENU;
						if (Arg)
						{
							if (StrToGuid(Arg,menuGuid))
								cpInfo.ItemGuid=&menuGuid;
							else
								ItemFailed=true;
						}
						break;
					case MPRT_PLUGINCONFIG:
						cpInfo.CallFlags |= CPT_CONFIGURE;
						if (Arg)
						{
							if (StrToGuid(Arg,menuGuid))
								cpInfo.ItemGuid=&menuGuid;
							else
								ItemFailed=true;
						}
						break;
					case MPRT_PLUGINCOMMAND:
						cpInfo.CallFlags |= CPT_CMDLINE;
						if (Arg)
							cpInfo.Command=Arg;
						else
							cpInfo.Command=L"";
						break;
				}

				if (!ItemFailed && StrToGuid(Guid,guid) && Global->CtrlObject->Plugins->FindPlugin(guid))
				{
					// ����� ������� ��������� "����������" ����� ��������� ������� �������/������
					int Ret=Global->CtrlObject->Plugins->CallPluginItem(guid,&cpInfo);
					if (Ret)
					{
						// ���� ����� ������� - �� ������ ����������
						macro->SetBooleanValue(1);
						cpInfo.CallFlags&=~CPT_CHECKONLY;
						Global->CtrlObject->Plugins->CallPluginItem(guid,&cpInfo);
					}
				}

				// �� �������� � KEY_F11
				FrameManager->RefreshFrame();
				break;
			}

			case MPRT_USERMENU:
				ShowUserMenu(mpr->Count,mpr->Values);
				break;
		}
	}

	//_SHMUEL(SysLog(L"-KeyMacro::GetKey, returned 0"));
	return 0;
}

int KeyMacro::PeekKey()
{
	//_SHMUEL(SysLog(L"+PeekKey"));
	return !m_InternalInput && IsExecuting();
}

bool KeyMacro::GetMacroKeyInfo(const string& strMode, int Pos, string &strKeyName, string &strDescription)
{
	FarMacroValue values[2]={{FMVT_STRING},{FMVT_BOOLEAN}};
	values[0].String = strMode.data();
	values[1].Boolean = Pos?0:1;
	FarMacroCall fmc={sizeof(FarMacroCall),ARRAYSIZE(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_ENUMMACROS,0,&fmc};

	if (CallMacroPlugin(&info) && info.Ret.Count >= 5)
	{
		strKeyName = info.Ret.Values[1].String;
		strDescription = info.Ret.Values[4].String;
		return true;
	}
	return false;
}

void KeyMacro::SendDropProcess()
{//FIXME
}

bool KeyMacro::CheckWaitKeyFunc()
{
	return m_WaitKey != 0;
}

// �������, ����������� ������� ��� ������ ����
void KeyMacro::RunStartMacro()
{
	if ((Global->Opt->Macro.DisableMacro&MDOL_ALL) || (Global->Opt->Macro.DisableMacro&MDOL_AUTOSTART))
		return;

	if (!Global->CtrlObject || !Global->CtrlObject->Cp() || !Global->CtrlObject->Cp()->ActivePanel || !Global->CtrlObject->Plugins->IsPluginsLoaded())
		return;

	static bool IsRunStartMacro=false, IsInside=false;

	if (!IsRunStartMacro && !IsInside)
	{
		IsInside = true;
		OpenMacroPluginInfo info = {MCT_RUNSTARTMACRO,0,nullptr};
		IsRunStartMacro = CallMacroPlugin(&info);
		IsInside = false;
	}
}

int KeyMacro::AddMacro(const wchar_t *PlainText,const wchar_t *Description, FARMACROAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback)
{
	if (Area < 0 || Area >= MACROAREA_LAST)
		return FALSE;

	string strKeyText;
	if (!(InputRecordToText(&AKey, strKeyText) && ParseMacroString(PlainText,true,true)))
		return FALSE;

	LM_ProcessMacro(Area,strKeyText,PlainText,Flags,Description,&PluginId,Callback,Id);
	return TRUE;
}

int KeyMacro::DelMacro(const GUID& PluginId,void* Id)
{
	FarMacroValue values[2]={{FMVT_BINARY},{FMVT_POINTER}};
	values[0].Binary.Data=const_cast<GUID*>(&PluginId);
	values[0].Binary.Size=sizeof(GUID);
	values[1].Pointer=Id;

	FarMacroCall fmc={sizeof(FarMacroCall),ARRAYSIZE(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_DELMACRO,0,&fmc};
	return CallMacroPlugin(&info) ? 1:0;
}

bool KeyMacro::PostNewMacro(int MacroId,const wchar_t* PlainText,UINT64 Flags,DWORD AKey)
{
	if (MacroId != 0 || ParseMacroString(PlainText, false, true))
	{
		MacroRecord* macro=new MacroRecord(Flags, MacroId, AKey, PlainText, L"");

		m_CurState->m_MacroQueue.emplace_back(macro);

		return true;
	}
	return false;
}

static BOOL CheckEditSelected(FARMACROAREA Mode, UINT64 CurFlags)
{
	if (Mode==MACROAREA_EDITOR || Mode==MACROAREA_DIALOG || Mode==MACROAREA_VIEWER || (Mode==MACROAREA_SHELL&&Global->CtrlObject->CmdLine->IsVisible()))
	{
		int NeedType = Mode == MACROAREA_EDITOR?MODALTYPE_EDITOR:(Mode == MACROAREA_VIEWER?MODALTYPE_VIEWER:(Mode == MACROAREA_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS));
		Frame* CurFrame=FrameManager->GetCurrentFrame();

		if (CurFrame && CurFrame->GetType()==NeedType)
		{
			int CurSelected;

			if (Mode==MACROAREA_SHELL && Global->CtrlObject->CmdLine->IsVisible())
				CurSelected=(int)Global->CtrlObject->CmdLine->VMProcess(MCODE_C_SELECTED);
			else
				CurSelected=(int)CurFrame->VMProcess(MCODE_C_SELECTED);

			if (((CurFlags&MFLAGS_EDITSELECTION) && !CurSelected) ||	((CurFlags&MFLAGS_EDITNOSELECTION) && CurSelected))
				return FALSE;
		}
	}

	return TRUE;
}

static BOOL CheckCmdLine(int CmdLength,UINT64 CurFlags)
{
	if (((CurFlags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength) || ((CurFlags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0))
		return FALSE;

	return TRUE;
}

static BOOL CheckPanel(int PanelMode,UINT64 CurFlags,BOOL IsPassivePanel)
{
	if (IsPassivePanel)
	{
		if ((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_PNOPLUGINPANELS)) || (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_PNOFILEPANELS)))
			return FALSE;
	}
	else
	{
		if ((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_NOPLUGINPANELS)) || (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_NOFILEPANELS)))
			return FALSE;
	}

	return TRUE;
}

static BOOL CheckFileFolder(Panel *CheckPanel,UINT64 CurFlags, BOOL IsPassivePanel)
{
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
	CheckPanel->GetFileName(strFileName,CheckPanel->GetCurrentPos(),FileAttr);

	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (IsPassivePanel)
		{
			if (((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFILES)))
				return FALSE;
		}
		else
		{
			if (((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFILES)))
				return FALSE;
		}
	}

	return TRUE;
}

static BOOL CheckAll (FARMACROAREA Mode, UINT64 CurFlags)
{
	/* $TODO:
		����� ������ Check*() ����������� ������� IfCondition()
		��� ���������� �������������� ����.
	*/

	// �������� �� �����/�� ����� � ���.������ (� � ���������? :-)
	if (CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
		if (Global->CtrlObject->CmdLine && !CheckCmdLine(Global->CtrlObject->CmdLine->GetLength(),CurFlags))
			return FALSE;

	FilePanels *Cp=Global->CtrlObject->Cp();

	if (!Cp)
		return FALSE;

	// �������� ������ � ���� �����
	Panel *ActivePanel=Cp->ActivePanel;
	Panel *PassivePanel=Cp->GetAnotherPanel(Cp->ActivePanel);

	if (ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACROAREA_SHELL)
	{
		if (CurFlags&(MFLAGS_NOPLUGINPANELS|MFLAGS_NOFILEPANELS))
			if (!CheckPanel(ActivePanel->GetMode(),CurFlags,FALSE))
				return FALSE;

		if (CurFlags&(MFLAGS_PNOPLUGINPANELS|MFLAGS_PNOFILEPANELS))
			if (!CheckPanel(PassivePanel->GetMode(),CurFlags,TRUE))
				return FALSE;

		if (CurFlags&(MFLAGS_NOFOLDERS|MFLAGS_NOFILES))
			if (!CheckFileFolder(ActivePanel,CurFlags,FALSE))
				return FALSE;

		if (CurFlags&(MFLAGS_PNOFOLDERS|MFLAGS_PNOFILES))
			if (!CheckFileFolder(PassivePanel,CurFlags,TRUE))
				return FALSE;

		if (CurFlags&(MFLAGS_SELECTION|MFLAGS_NOSELECTION|MFLAGS_PSELECTION|MFLAGS_PNOSELECTION))
			if (Mode!=MACROAREA_EDITOR && Mode != MACROAREA_DIALOG && Mode!=MACROAREA_VIEWER)
			{
				size_t SelCount=ActivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_SELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_NOSELECTION) && SelCount >= 1))
					return FALSE;

				SelCount=PassivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_PSELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_PNOSELECTION) && SelCount >= 1))
					return FALSE;
			}
	}

	if (!CheckEditSelected(Mode, CurFlags))
		return FALSE;

	return TRUE;
}

static int Set3State(DWORD Flags,DWORD Chk1,DWORD Chk2)
{
	DWORD Chk12=Chk1|Chk2, FlagsChk12=Flags&Chk12;

	if (FlagsChk12 == Chk12 || !FlagsChk12)
		return (2);
	else
		return (Flags&Chk1?1:0);
}

enum MACROSETTINGSDLG
{
	MS_DOUBLEBOX,
	MS_TEXT_SEQUENCE,
	MS_EDIT_SEQUENCE,
	MS_TEXT_DESCR,
	MS_EDIT_DESCR,
	MS_SEPARATOR1,
	MS_CHECKBOX_OUPUT,
	MS_CHECKBOX_START,
	MS_SEPARATOR2,
	MS_CHECKBOX_A_PANEL,
	MS_CHECKBOX_A_PLUGINPANEL,
	MS_CHECKBOX_A_FOLDERS,
	MS_CHECKBOX_A_SELECTION,
	MS_CHECKBOX_P_PANEL,
	MS_CHECKBOX_P_PLUGINPANEL,
	MS_CHECKBOX_P_FOLDERS,
	MS_CHECKBOX_P_SELECTION,
	MS_SEPARATOR3,
	MS_CHECKBOX_CMDLINE,
	MS_CHECKBOX_SELBLOCK,
	MS_SEPARATOR4,
	MS_BUTTON_OK,
	MS_BUTTON_CANCEL,
};

intptr_t KeyMacro::ParamMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_BTNCLICK:

			if (Param1==MS_CHECKBOX_A_PANEL || Param1==MS_CHECKBOX_P_PANEL)
				for (int i=1; i<=3; i++)
					Dlg->SendMessage(DM_ENABLE,Param1+i,Param2);

			break;
		case DN_CLOSE:

			if (Param1==MS_BUTTON_OK)
			{
				LPCWSTR Sequence=(LPCWSTR)Dlg->SendMessage(DM_GETCONSTTEXTPTR,MS_EDIT_SEQUENCE,0);
				if (*Sequence)
				{
					if (ParseMacroString(Sequence,false,true))
					{
						m_RecCode=Sequence;
						m_RecDescription=(LPCWSTR)Dlg->SendMessage(DM_GETCONSTTEXTPTR,MS_EDIT_DESCR,0);
						return TRUE;
					}
				}

				return FALSE;
			}

			break;

		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

int KeyMacro::GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src,const wchar_t *Descr)
{
	/*
	          1         2         3         4         5         6
	   3456789012345678901234567890123456789012345678901234567890123456789
	 1 �=========== ��������� ������������ ��� 'CtrlP' ==================�
	 2 | ������������������:                                             |
	 3 | _______________________________________________________________ |
	 4 | ��������:                                                       |
	 5 | _______________________________________________________________ |
	 6 |-----------------------------------------------------------------|
	 7 | [ ] ��������� �� ����� ���������� ����� �� �����                |
	 8 | [ ] ��������� ����� ������� FAR                                 |
	 9 |-----------------------------------------------------------------|
	10 | [ ] �������� ������             [ ] ��������� ������            |
	11 |   [?] �� ������ �������           [?] �� ������ �������         |
	12 |   [?] ��������� ��� �����         [?] ��������� ��� �����       |
	13 |   [?] �������� �����              [?] �������� �����            |
	14 |-----------------------------------------------------------------|
	15 | [?] ������ ��������� ������                                     |
	16 | [?] ������� ����                                                |
	17 |-----------------------------------------------------------------|
	18 |               [ ���������� ]  [ �������� ]                      |
	19 L=================================================================+

	*/
	FarDialogItem MacroSettingsDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,69,19,0,nullptr,nullptr,0,L""},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MMacroSequence)},
		{DI_EDIT,5,3,67,3,0,L"MacroSequence",nullptr,DIF_FOCUS|DIF_HISTORY,L""},
		{DI_TEXT,5,4,0,4,0,nullptr,nullptr,0,MSG(MMacroDescription)},
		{DI_EDIT,5,5,67,5,0,L"MacroDescription",nullptr,DIF_HISTORY,L""},

		{DI_TEXT,-1,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(MMacroSettingsEnableOutput)},
		{DI_CHECKBOX,5,8,0,8,0,nullptr,nullptr,0,MSG(MMacroSettingsRunAfterStart)},
		{DI_TEXT,-1,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsActivePanel)},
		{DI_CHECKBOX,7,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,7,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,7,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_CHECKBOX,37,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsPassivePanel)},
		{DI_CHECKBOX,39,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,39,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,39,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_TEXT,-1,14,0,14,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,15,0,15,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsCommandLine)},
		{DI_CHECKBOX,5,16,0,16,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsSelectionBlockPresent)},
		{DI_TEXT,-1,17,0,17,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	auto MacroSettingsDlg = MakeDialogItemsEx(MacroSettingsDlgData);
	string strKeyText;
	KeyToText(Key,strKeyText);
	MacroSettingsDlg[MS_DOUBLEBOX].strData = LangString(MMacroSettingsTitle) << strKeyText;
	//if(!(Key&0x7F000000))
	//MacroSettingsDlg[3].Flags|=DIF_DISABLE;
	MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected=Flags&MFLAGS_ENABLEOUTPUT?1:0;
	MacroSettingsDlg[MS_CHECKBOX_START].Selected=Flags&MFLAGS_RUNAFTERFARSTART?1:0;
	MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_NOFILEPANELS,MFLAGS_NOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected=Set3State(Flags,MFLAGS_NOFILES,MFLAGS_NOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected=Set3State(Flags,MFLAGS_SELECTION,MFLAGS_NOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_PNOFILEPANELS,MFLAGS_PNOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected=Set3State(Flags,MFLAGS_PNOFILES,MFLAGS_PNOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected=Set3State(Flags,MFLAGS_PSELECTION,MFLAGS_PNOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected=Set3State(Flags,MFLAGS_EMPTYCOMMANDLINE,MFLAGS_NOTEMPTYCOMMANDLINE);
	MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected=Set3State(Flags,MFLAGS_EDITSELECTION,MFLAGS_EDITNOSELECTION);
	if (Src && *Src)
	{
		MacroSettingsDlg[MS_EDIT_SEQUENCE].strData=Src;
	}
	else
	{
		MacroSettingsDlg[MS_EDIT_SEQUENCE].strData=m_RecCode;
	}

	MacroSettingsDlg[MS_EDIT_DESCR].strData=(Descr && *Descr)?Descr:m_RecDescription.data();

	DlgParam Param={0, 0, MACROAREA_OTHER, 0, false};
	Dialog Dlg(MacroSettingsDlg, this, &KeyMacro::ParamMacroDlgProc, &Param);
	Dlg.SetPosition(-1,-1,73,21);
	Dlg.SetHelp(L"KeyMacroSetting");
	Frame* BottomFrame = FrameManager->GetBottomFrame();
	if(BottomFrame)
	{
		BottomFrame->Lock(); // ������� ���������� ������
	}
	Dlg.Process();
	if(BottomFrame)
	{
		BottomFrame->Unlock(); // ������ ����� :-)
	}

	if (Dlg.GetExitCode()!=MS_BUTTON_OK)
		return FALSE;

	Flags=MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected?MFLAGS_ENABLEOUTPUT:0;
	Flags|=MacroSettingsDlg[MS_CHECKBOX_START].Selected?MFLAGS_RUNAFTERFARSTART:0;

	if (MacroSettingsDlg[MS_CHECKBOX_A_PANEL].Selected)
	{
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected==0?MFLAGS_NOPLUGINPANELS:MFLAGS_NOFILEPANELS);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected==0?MFLAGS_NOFOLDERS:MFLAGS_NOFILES);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected==0?MFLAGS_NOSELECTION:MFLAGS_SELECTION);
	}

	if (MacroSettingsDlg[MS_CHECKBOX_P_PANEL].Selected)
	{
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected==0?MFLAGS_PNOPLUGINPANELS:MFLAGS_PNOFILEPANELS);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected==0?MFLAGS_PNOFOLDERS:MFLAGS_PNOFILES);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected==0?MFLAGS_PNOSELECTION:MFLAGS_PSELECTION);
	}

	Flags|=MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected==2?0:
	       (MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected==0?MFLAGS_NOTEMPTYCOMMANDLINE:MFLAGS_EMPTYCOMMANDLINE);
	Flags|=MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected==2?0:
	       (MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected==0?MFLAGS_EDITNOSELECTION:MFLAGS_EDITSELECTION);
	return TRUE;
}

bool KeyMacro::ParseMacroString(const wchar_t* Sequence, bool onlyCheck, bool skipFile)
{
	// ������������� ����� ��������� �� ������ �� ������, �.�. ������� Message()
	// �� ����� ����������� ������ � �������� ���������.
	FarMacroValue values[5]={{FMVT_STRING,{0}},{FMVT_BOOLEAN,{0}},{FMVT_BOOLEAN,{0}},{FMVT_STRING,{0}},{FMVT_STRING,{0}}};
	values[0].String=Sequence;
	values[1].Boolean=onlyCheck?1:0;
	values[2].Boolean=skipFile?1:0;
	values[3].String=MSG(MMacroPErrorTitle);
	values[4].String=MSG(MOk);
	FarMacroCall fmc={sizeof(FarMacroCall),ARRAYSIZE(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_MACROPARSE,0,&fmc};

	if (CallMacroPlugin(&info))
	{
		if (info.Ret.ReturnType == MPRT_NORMALFINISH)
		{
			m_LastErrorStr.clear();
			m_LastErrorLine = 0;
			return true;
		}
		if (info.Ret.ReturnType == MPRT_ERRORPARSE)
		{
			m_LastErrorStr = info.Ret.Values[0].String;
			m_LastErrorLine = (int)info.Ret.Values[1].Double;
			if (!onlyCheck)
			{
				RestoreMacroChar();
				FrameManager->RefreshFrame(); // ����� ����� ������ ��������� ��������. ����� ������ �� ����������������.
			}
		}
	}
	return false;
}

bool KeyMacro::ExecuteString(MacroExecuteString *Data)
{
	FarMacroValue values[3]={{FMVT_STRING,{0}},{FMVT_INTEGER,{0}},{FMVT_ARRAY,{0}}};
	values[0].String = Data->SequenceText;
	values[1].Integer = Data->Flags;
	values[2].Array.Count = Data->InCount;
	values[2].Array.Values = Data->InValues;

	FarMacroCall fmc={sizeof(FarMacroCall),ARRAYSIZE(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_EXECSTRING,0,&fmc};

	if (CallMacroPlugin(&info) && info.Ret.ReturnType != MPRT_ERRORFINISH && info.Ret.ReturnType != MPRT_ERRORPARSE)
	{
		Data->OutValues = info.Ret.Values;
		Data->OutCount = info.Ret.Count;
		return true;
	}
	Data->OutCount = 0;
	return false;
}

void KeyMacro::GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc)
{
	*ErrCode = m_LastErrorStr == L"" ? MPEC_SUCCESS : MPEC_ERROR;
	ErrPos->X = 0;
	ErrPos->Y = static_cast<SHORT>(m_LastErrorLine);
	*ErrSrc = m_LastErrorStr;
}

bool KeyMacro::UpdateLockScreen(bool recreate)
{
	bool oldstate = (m_LockScr!=nullptr);
	if (m_LockScr)
	{
		delete m_LockScr;
		m_LockScr=nullptr;
	}
	if (recreate)
		m_LockScr = new LockScreen;
	return oldstate;
}

static bool absFunc(FarMacroCall*);
static bool ascFunc(FarMacroCall*);
static bool atoiFunc(FarMacroCall*);
static bool beepFunc(FarMacroCall*);
static bool chrFunc(FarMacroCall*);
static bool clipFunc(FarMacroCall*);
static bool dateFunc(FarMacroCall*);
static bool dlggetvalueFunc(FarMacroCall*);
static bool dlgsetfocusFunc(FarMacroCall*);
static bool editordellineFunc(FarMacroCall*);
static bool editorinsstrFunc(FarMacroCall*);
static bool editorposFunc(FarMacroCall*);
static bool editorselFunc(FarMacroCall*);
static bool editorsetFunc(FarMacroCall*);
static bool editorsetstrFunc(FarMacroCall*);
static bool editorsettitleFunc(FarMacroCall*);
static bool editorundoFunc(FarMacroCall*);
static bool environFunc(FarMacroCall*);
static bool farcfggetFunc(FarMacroCall*);
static bool fattrFunc(FarMacroCall*);
static bool fexistFunc(FarMacroCall*);
static bool floatFunc(FarMacroCall*);
static bool flockFunc(FarMacroCall*);
static bool fmatchFunc(FarMacroCall*);
static bool fsplitFunc(FarMacroCall*);
static bool indexFunc(FarMacroCall*);
static bool intFunc(FarMacroCall*);
static bool itowFunc(FarMacroCall*);
static bool kbdLayoutFunc(FarMacroCall*);
static bool keybarshowFunc(FarMacroCall*);
static bool keyFunc(FarMacroCall*);
static bool lcaseFunc(FarMacroCall*);
static bool lenFunc(FarMacroCall*);
static bool maxFunc(FarMacroCall*);
static bool menushowFunc(FarMacroCall*);
static bool minFunc(FarMacroCall*);
static bool modFunc(FarMacroCall*);
static bool msgBoxFunc(FarMacroCall*);
static bool panelfattrFunc(FarMacroCall*);
static bool panelfexistFunc(FarMacroCall*);
static bool panelitemFunc(FarMacroCall*);
static bool panelselectFunc(FarMacroCall*);
static bool panelsetpathFunc(FarMacroCall*);
static bool panelsetposFunc(FarMacroCall*);
static bool panelsetposidxFunc(FarMacroCall*);
static bool promptFunc(FarMacroCall*);
static bool replaceFunc(FarMacroCall*);
static bool rindexFunc(FarMacroCall*);
static bool size2strFunc(FarMacroCall*);
static bool sleepFunc(FarMacroCall*);
static bool stringFunc(FarMacroCall*);
static bool strwrapFunc(FarMacroCall*);
static bool strpadFunc(FarMacroCall*);
static bool substrFunc(FarMacroCall*);
static bool testfolderFunc(FarMacroCall*);
static bool trimFunc(FarMacroCall*);
static bool ucaseFunc(FarMacroCall*);
static bool waitkeyFunc(FarMacroCall*);
static bool windowscrollFunc(FarMacroCall*);
static bool xlatFunc(FarMacroCall*);
static bool pluginloadFunc(FarMacroCall*);
static bool pluginunloadFunc(FarMacroCall*);
static bool pluginexistFunc(FarMacroCall*);

static int PassString (const wchar_t *str, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_STRING;
		val.String = str;
		Data->Callback(Data->CallbackData, &val, 1);
	}
	return 1;
}

inline int PassString (const string& str, FarMacroCall* Data)
{
	return PassString(str.data(), Data);
}

static int PassNumber (double dbl, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_DOUBLE;
		val.Double = dbl;
		Data->Callback(Data->CallbackData, &val, 1);
	}
	return 1;
}

static int PassInteger (__int64 Int, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_INTEGER;
		val.Integer = Int;
		Data->Callback(Data->CallbackData, &val, 1);
	}
	return 1;
}

static int PassBoolean (int b, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_BOOLEAN;
		val.Boolean = b;
		Data->Callback(Data->CallbackData, &val, 1);
	}
	return 1;
}

static int PassValue (const TVar* Var, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		double dd;

		if (Var->isDouble())
		{
			val.Type = FMVT_DOUBLE;
			val.Double = Var->d();
		}
		else if (Var->isString())
		{
			val.Type = FMVT_STRING;
			val.String = Var->s();
		}
		else if (ToDouble(Var->i(), &dd))
		{
			val.Type = FMVT_DOUBLE;
			val.Double = dd;
		}
		else
		{
			val.Type = FMVT_INTEGER;
			val.Integer = Var->i();
		}

		Data->Callback(Data->CallbackData, &val, 1);
	}
	return 1;
}

static void __parseParams(size_t Count, TVar* Params, FarMacroCall* Data)
{
	size_t argNum = (Data->Count > Count) ? Count : Data->Count;

	while (argNum < Count)
		Params[--Count].SetType(vtUnknown);

	for (size_t i=0; i<argNum; i++)
	{
		switch (Data->Values[i].Type)
		{
			case FMVT_INTEGER:
				Params[i] = Data->Values[i].Integer; break;
			case FMVT_BOOLEAN:
				Params[i] = Data->Values[i].Boolean; break;
			case FMVT_DOUBLE:
				Params[i] = Data->Values[i].Double; break;
			case FMVT_STRING:
				Params[i] = Data->Values[i].String; break;
			default:
				Params[i].SetType(vtUnknown); break;
		}
	}
}
#define parseParams(c,v,d) TVar v[c]; __parseParams(c,v,d)

class LockOutput
{
		bool dsbl;
	public:
		LockOutput(bool d) : dsbl(d) { if (dsbl) Global->ScrBuf->Lock(); }
		~LockOutput() { if (dsbl) Global->ScrBuf->Unlock(); }
};

intptr_t KeyMacro::CallFar(intptr_t CheckCode, FarMacroCall* Data)
{
	intptr_t ret=0;
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

	MacroRecord* TopMacro = GetTopMacro();
	bool IsOutputDisabled = TopMacro && !(TopMacro->Flags()&MFLAGS_ENABLEOUTPUT);

	// �������� �� �������
	if (CheckCode == 0)
	{
		return PassNumber (Global->WaitInMainLoop ?
			FrameManager->GetCurrentFrame()->GetMacroMode() : GetMode(), Data);
	}

	Panel *ActivePanel=Global->CtrlObject->Cp() ? Global->CtrlObject->Cp()->ActivePanel : nullptr;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	Frame* CurFrame=FrameManager->GetCurrentFrame();

	switch (CheckCode)
	{
		case MCODE_C_MSX:             return PassNumber(msValues[constMsX], Data);
		case MCODE_C_MSY:             return PassNumber(msValues[constMsY], Data);
		case MCODE_C_MSBUTTON:        return PassNumber(msValues[constMsButton], Data);
		case MCODE_C_MSCTRLSTATE:     return PassNumber(msValues[constMsCtrlState], Data);
		case MCODE_C_MSEVENTFLAGS:    return PassNumber(msValues[constMsEventFlags], Data);
		case MCODE_C_MSLASTCTRLSTATE: return PassNumber(msValues[constMsLastCtrlState], Data);

		case MCODE_V_FAR_WIDTH:
			return (ScrX+1);

		case MCODE_V_FAR_HEIGHT:
			return (ScrY+1);

		case MCODE_V_FAR_TITLE:
			Global->Console->GetTitle(strFileName);
			return PassString(strFileName, Data);

		case MCODE_V_FAR_PID:
			return PassNumber(GetCurrentProcessId(), Data);

		case MCODE_V_FAR_UPTIME:
		{
			LARGE_INTEGER Frequency, Counter;
			QueryPerformanceFrequency(&Frequency);
			QueryPerformanceCounter(&Counter);
			return PassNumber(((Counter.QuadPart-Global->FarUpTime().QuadPart)*1000)/Frequency.QuadPart, Data);
		}

		case MCODE_V_MACRO_AREA:
			return PassNumber(GetMode(), Data);

		case MCODE_C_FULLSCREENMODE: // Fullscreen?
			return PassBoolean(IsConsoleFullscreen(), Data);

		case MCODE_C_ISUSERADMIN: // IsUserAdmin?
			return PassBoolean(Global->IsUserAdmin(), Data);

		case MCODE_V_DRVSHOWPOS: // Drv.ShowPos
			return Global->Macro_DskShowPosType;

		case MCODE_V_DRVSHOWMODE: // Drv.ShowMode
			return Global->Opt->ChangeDriveMode;

		case MCODE_C_CMDLINE_BOF:              // CmdLine.Bof - ������ � ������ cmd-������ ��������������?
		case MCODE_C_CMDLINE_EOF:              // CmdLine.Eof - ������ � ������ cmd-������ ��������������?
		case MCODE_C_CMDLINE_EMPTY:            // CmdLine.Empty
		case MCODE_C_CMDLINE_SELECTED:         // CmdLine.Selected
		{
			return PassBoolean(Global->CtrlObject->CmdLine && Global->CtrlObject->CmdLine->VMProcess(CheckCode), Data);
		}

		case MCODE_V_CMDLINE_ITEMCOUNT:        // CmdLine.ItemCount
		case MCODE_V_CMDLINE_CURPOS:           // CmdLine.CurPos
		{
			return Global->CtrlObject->CmdLine?Global->CtrlObject->CmdLine->VMProcess(CheckCode):-1;
		}

		case MCODE_V_CMDLINE_VALUE:            // CmdLine.Value
		{
			if (Global->CtrlObject->CmdLine)
				Global->CtrlObject->CmdLine->GetString(strFileName);
			return PassString(strFileName, Data);
		}

		case MCODE_C_APANEL_ROOT:  // APanel.Root
		case MCODE_C_PPANEL_ROOT:  // PPanel.Root
		{
			Panel *SelPanel=(CheckCode==MCODE_C_APANEL_ROOT)?ActivePanel:PassivePanel;
			return PassBoolean((SelPanel ? SelPanel->VMProcess(MCODE_C_ROOTFOLDER) ? 1:0:0), Data);
		}

		case MCODE_C_APANEL_BOF:
		case MCODE_C_PPANEL_BOF:
		case MCODE_C_APANEL_EOF:
		case MCODE_C_PPANEL_EOF:
		{
			Panel *SelPanel=(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_APANEL_EOF)?ActivePanel:PassivePanel;
			if (SelPanel)
				ret=SelPanel->VMProcess(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_PPANEL_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;
			return PassBoolean(ret, Data);
		}

		case MCODE_C_SELECTED:    // Selected?
		{
			int NeedType = m_Mode == MACROAREA_EDITOR? MODALTYPE_EDITOR : (m_Mode == MACROAREA_VIEWER? MODALTYPE_VIEWER : (m_Mode == MACROAREA_DIALOG? MODALTYPE_DIALOG : MODALTYPE_PANELS));

			if (!(m_Mode == MACROAREA_USERMENU || m_Mode == MACROAREA_MAINMENU || m_Mode == MACROAREA_MENU) && CurFrame && CurFrame->GetType()==NeedType)
			{
				int CurSelected;

				if (m_Mode==MACROAREA_SHELL && Global->CtrlObject->CmdLine->IsVisible())
					CurSelected=(int)Global->CtrlObject->CmdLine->VMProcess(CheckCode);
				else
					CurSelected=(int)CurFrame->VMProcess(CheckCode);

				return PassBoolean(CurSelected, Data);
			}
			else
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
				{
					ret=f->VMProcess(CheckCode);
				}
			}
			return PassBoolean(ret, Data);
		}

		case MCODE_C_EMPTY:   // Empty
		case MCODE_C_BOF:
		case MCODE_C_EOF:
		{
			int CurMMode=GetMode();

			if (!(m_Mode == MACROAREA_USERMENU || m_Mode == MACROAREA_MAINMENU || m_Mode == MACROAREA_MENU) && CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACROAREA_INFOPANEL || CurMMode == MACROAREA_QVIEWPANEL || CurMMode == MACROAREA_TREEPANEL))
			{
				if (CheckCode == MCODE_C_EMPTY)
					ret=Global->CtrlObject->CmdLine->GetLength()?0:1;
				else
					ret=Global->CtrlObject->CmdLine->VMProcess(CheckCode);
			}
			else
			{
				{
					Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

					while (f)
					{
						fo=f;
						f=f->GetTopModal();
					}

					if (!f)
						f=fo;

					if (f)
					{
						ret=f->VMProcess(CheckCode);
					}
				}
			}

			return PassBoolean(ret, Data);
		}

		case MCODE_V_DLGITEMCOUNT: // Dlg.ItemCount
		case MCODE_V_DLGCURPOS:    // Dlg.CurPos
		case MCODE_V_DLGITEMTYPE:  // Dlg.ItemType
		case MCODE_V_DLGPREVPOS:   // Dlg.PrevPos
		case MCODE_V_DLGINFOID:        // Dlg.Info.Id
		case MCODE_V_DLGINFOOWNER:     // Dlg.Info.Owner
		{
			if (CurFrame && CurFrame->GetType()==MODALTYPE_VMENU)
			{
				int idx=FrameManager->IndexOfStack(CurFrame);
				if(idx>0)
					CurFrame=FrameManager->GetModalFrame(idx-1);
			}
			if (!CurFrame || CurFrame->GetType()!=MODALTYPE_DIALOG)
				return (CheckCode==MCODE_V_DLGINFOID || CheckCode==MCODE_V_DLGINFOOWNER) ? PassString(L"", Data) : 0;

			if (CheckCode==MCODE_V_DLGINFOID || CheckCode==MCODE_V_DLGINFOOWNER)
				return PassString(reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(CurFrame->VMProcess(CheckCode))), Data);
			else
				return CurFrame->VMProcess(CheckCode);
		}

		case MCODE_C_APANEL_VISIBLE:  // APanel.Visible
		case MCODE_C_PPANEL_VISIBLE:  // PPanel.Visible
		{
			Panel *SelPanel=CheckCode==MCODE_C_APANEL_VISIBLE?ActivePanel:PassivePanel;
			return PassBoolean(SelPanel && SelPanel->IsVisible(), Data);
		}

		case MCODE_C_APANEL_ISEMPTY: // APanel.Empty
		case MCODE_C_PPANEL_ISEMPTY: // PPanel.Empty
		{
			Panel *SelPanel=CheckCode==MCODE_C_APANEL_ISEMPTY?ActivePanel:PassivePanel;
			if (SelPanel)
			{
				SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);
				size_t GetFileCount=SelPanel->GetFileCount();
				ret=(!GetFileCount || (GetFileCount == 1 && TestParentFolderName(strFileName))) ? 1:0;
			}
			return PassBoolean(ret, Data);
		}

		case MCODE_C_APANEL_FILTER:
		case MCODE_C_PPANEL_FILTER:
		{
			Panel *SelPanel=(CheckCode==MCODE_C_APANEL_FILTER)?ActivePanel:PassivePanel;
			return PassBoolean(SelPanel && SelPanel->VMProcess(MCODE_C_APANEL_FILTER), Data);
		}

		case MCODE_C_APANEL_LFN:
		case MCODE_C_PPANEL_LFN:
		{
			Panel *SelPanel = CheckCode == MCODE_C_APANEL_LFN ? ActivePanel : PassivePanel;
			return PassBoolean(SelPanel && !SelPanel->GetShowShortNamesMode(), Data);
		}

		case MCODE_C_APANEL_LEFT: // APanel.Left
		case MCODE_C_PPANEL_LEFT: // PPanel.Left
		{
			Panel *SelPanel = CheckCode == MCODE_C_APANEL_LEFT ? ActivePanel : PassivePanel;
			return PassBoolean(SelPanel && SelPanel==Global->CtrlObject->Cp()->LeftPanel, Data);
		}

		case MCODE_C_APANEL_FILEPANEL: // APanel.FilePanel
		case MCODE_C_PPANEL_FILEPANEL: // PPanel.FilePanel
		{
			Panel *SelPanel = CheckCode == MCODE_C_APANEL_FILEPANEL ? ActivePanel : PassivePanel;
			return PassBoolean(SelPanel && SelPanel->GetType()==FILE_PANEL, Data);
		}

		case MCODE_C_APANEL_PLUGIN: // APanel.Plugin
		case MCODE_C_PPANEL_PLUGIN: // PPanel.Plugin
		{
			Panel *SelPanel=CheckCode==MCODE_C_APANEL_PLUGIN?ActivePanel:PassivePanel;
			return PassBoolean(SelPanel && SelPanel->GetMode()==PLUGIN_PANEL, Data);
		}

		case MCODE_C_APANEL_FOLDER: // APanel.Folder
		case MCODE_C_PPANEL_FOLDER: // PPanel.Folder
		{
			Panel *SelPanel=CheckCode==MCODE_C_APANEL_FOLDER?ActivePanel:PassivePanel;
			if (SelPanel)
			{
				SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);

				if (FileAttr != INVALID_FILE_ATTRIBUTES)
					ret=(FileAttr&FILE_ATTRIBUTE_DIRECTORY)?1:0;
			}
			return PassBoolean(ret, Data);
		}

		case MCODE_C_APANEL_SELECTED: // APanel.Selected
		case MCODE_C_PPANEL_SELECTED: // PPanel.Selected
		{
			Panel *SelPanel=CheckCode==MCODE_C_APANEL_SELECTED?ActivePanel:PassivePanel;
			return PassBoolean(SelPanel && SelPanel->GetRealSelCount() > 0, Data);
		}

		case MCODE_V_APANEL_CURRENT: // APanel.Current
		case MCODE_V_PPANEL_CURRENT: // PPanel.Current
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURRENT ? ActivePanel : PassivePanel;
			const wchar_t *ptr = L"";
			if (SelPanel )
			{
				SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);
				if (FileAttr != INVALID_FILE_ATTRIBUTES)
					ptr = strFileName.data();
			}
			return PassString(ptr, Data);
		}

		case MCODE_V_APANEL_SELCOUNT: // APanel.SelCount
		case MCODE_V_PPANEL_SELCOUNT: // PPanel.SelCount
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_SELCOUNT ? ActivePanel : PassivePanel;
			return static_cast<int>(SelPanel ? SelPanel->GetRealSelCount() : 0);
		}

		case MCODE_V_APANEL_COLUMNCOUNT:       // APanel.ColumnCount - �������� ������:  ���������� �������
		case MCODE_V_PPANEL_COLUMNCOUNT:       // PPanel.ColumnCount - ��������� ������: ���������� �������
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_COLUMNCOUNT ? ActivePanel : PassivePanel;
			return SelPanel ? SelPanel->GetColumnsCount() : 0;
		}

		case MCODE_V_APANEL_WIDTH: // APanel.Width
		case MCODE_V_PPANEL_WIDTH: // PPanel.Width
		case MCODE_V_APANEL_HEIGHT: // APanel.Height
		case MCODE_V_PPANEL_HEIGHT: // PPanel.Height
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_WIDTH || CheckCode == MCODE_V_APANEL_HEIGHT? ActivePanel : PassivePanel;

			if (SelPanel )
			{
				int X1, Y1, X2, Y2;
				SelPanel->GetPosition(X1,Y1,X2,Y2);

				if (CheckCode == MCODE_V_APANEL_HEIGHT || CheckCode == MCODE_V_PPANEL_HEIGHT)
					ret = Y2-Y1+1;
				else
					ret = X2-X1+1;
			}
			return ret;
		}

		case MCODE_V_APANEL_OPIFLAGS:  // APanel.OPIFlags
		case MCODE_V_PPANEL_OPIFLAGS:  // PPanel.OPIFlags
		case MCODE_V_APANEL_HOSTFILE: // APanel.HostFile
		case MCODE_V_PPANEL_HOSTFILE: // PPanel.HostFile
		case MCODE_V_APANEL_FORMAT:           // APanel.Format
		case MCODE_V_PPANEL_FORMAT:           // PPanel.Format
		{
			Panel *SelPanel =
					CheckCode == MCODE_V_APANEL_OPIFLAGS ||
					CheckCode == MCODE_V_APANEL_HOSTFILE ||
					CheckCode == MCODE_V_APANEL_FORMAT? ActivePanel : PassivePanel;

			const wchar_t* ptr = nullptr;
			if (CheckCode == MCODE_V_APANEL_HOSTFILE || CheckCode == MCODE_V_PPANEL_HOSTFILE ||
				CheckCode == MCODE_V_APANEL_FORMAT || CheckCode == MCODE_V_PPANEL_FORMAT)
				ptr = L"";

			if (SelPanel )
			{
				if (SelPanel->GetMode() == PLUGIN_PANEL)
				{
					OpenPanelInfo Info={};
					Info.StructSize=sizeof(OpenPanelInfo);
					SelPanel->GetOpenPanelInfo(&Info);
					switch (CheckCode)
					{
						case MCODE_V_APANEL_OPIFLAGS:
						case MCODE_V_PPANEL_OPIFLAGS:
							return Info.Flags;
						case MCODE_V_APANEL_HOSTFILE:
						case MCODE_V_PPANEL_HOSTFILE:
							return PassString(Info.HostFile, Data);
						case MCODE_V_APANEL_FORMAT:
						case MCODE_V_PPANEL_FORMAT:
							return PassString(Info.Format, Data);
					}
				}
			}

			return ptr ? PassString(ptr, Data) : 0;
		}

		case MCODE_V_APANEL_PREFIX:           // APanel.Prefix
		case MCODE_V_PPANEL_PREFIX:           // PPanel.Prefix
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_PREFIX ? ActivePanel : PassivePanel;
			const wchar_t *ptr = L"";
			if (SelPanel)
			{
				PluginInfo PInfo = {sizeof(PInfo)};
				if (SelPanel->VMProcess(MCODE_V_APANEL_PREFIX,&PInfo))
					ptr = PInfo.CommandPrefix;
			}
			return PassString(ptr, Data);
		}

		case MCODE_V_APANEL_PATH0:           // APanel.Path0
		case MCODE_V_PPANEL_PATH0:           // PPanel.Path0
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH0 ? ActivePanel : PassivePanel;
			const wchar_t *ptr = L"";
			if (SelPanel )
			{
				if (!SelPanel->VMProcess(CheckCode,&strFileName,0))
					strFileName = SelPanel->GetCurDir();
				ptr = strFileName.data();
			}
			return PassString(ptr, Data);
		}

		case MCODE_V_APANEL_PATH: // APanel.Path
		case MCODE_V_PPANEL_PATH: // PPanel.Path
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH ? ActivePanel : PassivePanel;
			const wchar_t *ptr = L"";
			if (SelPanel)
			{
				if (SelPanel->GetMode() == PLUGIN_PANEL)
				{
					OpenPanelInfo Info={};
					Info.StructSize=sizeof(OpenPanelInfo);
					SelPanel->GetOpenPanelInfo(&Info);
					strFileName = NullToEmpty(Info.CurDir);
				}
				else
					strFileName = SelPanel->GetCurDir();

				DeleteEndSlash(strFileName); // - ����� � ����� ����� ���� C:, ����� ����� ������ ���: APanel.Path + "\\file"
				ptr = strFileName.data();
			}
			return PassString(ptr, Data);
		}

		case MCODE_V_APANEL_UNCPATH: // APanel.UNCPath
		case MCODE_V_PPANEL_UNCPATH: // PPanel.UNCPath
		{
			const wchar_t *ptr = L"";
			if (_MakePath1(CheckCode == MCODE_V_APANEL_UNCPATH?KEY_ALTSHIFTBRACKET:KEY_ALTSHIFTBACKBRACKET,strFileName,L""))
			{
				UnquoteExternal(strFileName);
				DeleteEndSlash(strFileName);
				ptr = strFileName.data();
			}
			return PassString(ptr, Data);
		}

		//FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
		case MCODE_V_APANEL_TYPE: // APanel.Type
		case MCODE_V_PPANEL_TYPE: // PPanel.Type
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_TYPE ? ActivePanel : PassivePanel;
			return SelPanel ? SelPanel->GetType() : 0;
		}

		case MCODE_V_APANEL_DRIVETYPE: // APanel.DriveType - �������� ������: ��� �������
		case MCODE_V_PPANEL_DRIVETYPE: // PPanel.DriveType - ��������� ������: ��� �������
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_DRIVETYPE ? ActivePanel : PassivePanel;
			ret=-1;

			if (SelPanel  && SelPanel->GetMode() != PLUGIN_PANEL)
			{
				strFileName = SelPanel->GetCurDir();
				GetPathRoot(strFileName, strFileName);
				UINT DriveType=FAR_GetDriveType(strFileName, 0);

				// BUGBUG: useless, GetPathRoot expands subst itself

				/*if (ParsePath(strFileName) == PATH_DRIVELETTER)
				{
					string strRemoteName;
					strFileName.SetLength(2);

					if (GetSubstName(DriveType,strFileName,strRemoteName))
						DriveType=DRIVE_SUBSTITUTE;
				}*/

				ret = DriveType;
			}

			return ret;
		}

		case MCODE_V_APANEL_ITEMCOUNT: // APanel.ItemCount
		case MCODE_V_PPANEL_ITEMCOUNT: // PPanel.ItemCount
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_ITEMCOUNT ? ActivePanel : PassivePanel;
			return SelPanel ? SelPanel->GetFileCount() : 0;
		}

		case MCODE_V_APANEL_CURPOS: // APanel.CurPos
		case MCODE_V_PPANEL_CURPOS: // PPanel.CurPos
		{
			Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURPOS ? ActivePanel : PassivePanel;
			return SelPanel ? SelPanel->GetCurrentPos()+(SelPanel->GetFileCount()>0?1:0) : 0;
		}

		case MCODE_V_TITLE: // Title
		{
			Frame *f=FrameManager->GetTopModal();

			if (f)
			{
				if (Global->CtrlObject->Cp() == f)
				{
					ActivePanel->GetTitle(strFileName);
				}
				else
				{
					string strType;

					switch (f->GetTypeAndName(strType,strFileName))
					{
						case MODALTYPE_EDITOR:
						case MODALTYPE_VIEWER:
							f->GetTitle(strFileName);
							break;
					}
				}

				RemoveExternalSpaces(strFileName);
			}

			return PassString(strFileName, Data);
		}

		case MCODE_V_HEIGHT:  // Height - ������ �������� �������
		case MCODE_V_WIDTH:   // Width - ������ �������� �������
		{
			Frame *f=FrameManager->GetTopModal();

			if (f)
			{
				int X1, Y1, X2, Y2;
				f->GetPosition(X1,Y1,X2,Y2);

				if (CheckCode == MCODE_V_HEIGHT)
					ret = Y2-Y1+1;
				else
					ret = X2-X1+1;
			}

			return ret;
		}

		case MCODE_V_MENU_VALUE: // Menu.Value
		case MCODE_V_MENUINFOID: // Menu.Info.Id
		{
			int CurMMode=GetMode();
			const wchar_t* ptr = L"";

			if (CheckCode==MCODE_V_MENUINFOID && CurFrame && CurFrame->GetType()==MODALTYPE_VMENU)
			{
				return PassString(reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(CurFrame->VMProcess(MCODE_V_DLGINFOID))), Data);
			}

			if (IsMenuArea(CurMMode) || CurMMode == MACROAREA_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
				{
					string NewStr;

					switch(CheckCode)
					{
						case MCODE_V_MENU_VALUE:
							if (f->VMProcess(CheckCode,&NewStr))
							{
								HiText2Str(strFileName, NewStr);
								RemoveExternalSpaces(strFileName);
								ptr=strFileName.data();
							}
							break;
						case MCODE_V_MENUINFOID:
							ptr=reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(f->VMProcess(CheckCode)));
							break;
					}
				}
			}

			return PassString(ptr, Data);
		}

		case MCODE_V_ITEMCOUNT: // ItemCount - ����� ��������� � ������� �������
		case MCODE_V_CURPOS: // CurPos - ������� ������ � ������� �������
		{
			Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

			while (f)
			{
				fo=f;
				f=f->GetTopModal();
			}

			if (!f)
				f=fo;

			if (f)
			{
				ret=f->VMProcess(CheckCode);
			}
			return ret;
		}

		case MCODE_V_EDITORCURLINE: // Editor.CurLine - ������� ����� � ��������� (� ���������� � Count)
		case MCODE_V_EDITORSTATE:   // Editor.State
		case MCODE_V_EDITORLINES:   // Editor.Lines
		case MCODE_V_EDITORCURPOS:  // Editor.CurPos
		case MCODE_V_EDITORREALPOS: // Editor.RealPos
		case MCODE_V_EDITORFILENAME: // Editor.FileName
		case MCODE_V_EDITORSELVALUE: // Editor.SelValue
		{
			const wchar_t* ptr = nullptr;
			if (CheckCode == MCODE_V_EDITORSELVALUE)
				ptr=L"";

			if (GetMode()==MACROAREA_EDITOR && Global->CtrlObject->Plugins->GetCurEditor() && Global->CtrlObject->Plugins->GetCurEditor()->IsVisible())
			{
				if (CheckCode == MCODE_V_EDITORFILENAME)
				{
					string strType;
					Global->CtrlObject->Plugins->GetCurEditor()->GetTypeAndName(strType, strFileName);
					ptr=strFileName.data();
				}
				else if (CheckCode == MCODE_V_EDITORSELVALUE)
				{
					Global->CtrlObject->Plugins->GetCurEditor()->VMProcess(CheckCode,&strFileName);
					ptr=strFileName.data();
				}
				else
					return Global->CtrlObject->Plugins->GetCurEditor()->VMProcess(CheckCode);
			}

			return ptr ? PassString(ptr, Data) : 0;
		}

		case MCODE_V_HELPFILENAME:  // Help.FileName
		case MCODE_V_HELPTOPIC:     // Help.Topic
		case MCODE_V_HELPSELTOPIC:  // Help.SelTopic
		{
			const wchar_t* ptr=L"";
			if (GetMode() == MACROAREA_HELP)
			{
				CurFrame->VMProcess(CheckCode,&strFileName,0);
				ptr=strFileName.data();
			}
			return PassString(ptr, Data);
		}

		case MCODE_V_VIEWERFILENAME: // Viewer.FileName
		case MCODE_V_VIEWERSTATE: // Viewer.State
		{
			if ((GetMode()==MACROAREA_VIEWER || GetMode()==MACROAREA_QVIEWPANEL) &&
							Global->CtrlObject->Plugins->GetCurViewer() && Global->CtrlObject->Plugins->GetCurViewer()->IsVisible())
			{
				if (CheckCode == MCODE_V_VIEWERFILENAME)
				{
					Global->CtrlObject->Plugins->GetCurViewer()->GetFileName(strFileName);//GetTypeAndName(nullptr,FileName);
					return PassString(strFileName, Data);
				}
				else
					return PassNumber(Global->CtrlObject->Plugins->GetCurViewer()->VMProcess(MCODE_V_VIEWERSTATE), Data);
			}

			return (CheckCode == MCODE_V_VIEWERFILENAME) ? PassString(L"", Data) : 0;
		}

		// =========================================================================
		// Functions
		// =========================================================================

		case MCODE_F_ABS:             return absFunc(Data);
		case MCODE_F_ASC:             return ascFunc(Data);
		case MCODE_F_ATOI:            return atoiFunc(Data);
		case MCODE_F_BEEP:            return beepFunc(Data);
		case MCODE_F_CHR:             return chrFunc(Data);
		case MCODE_F_CLIP:            return clipFunc(Data);
		case MCODE_F_DATE:            return dateFunc(Data);
		case MCODE_F_DLG_GETVALUE:    return dlggetvalueFunc(Data);
		case MCODE_F_DLG_SETFOCUS:    return dlgsetfocusFunc(Data);
		case MCODE_F_EDITOR_DELLINE:  return editordellineFunc(Data);
		case MCODE_F_EDITOR_INSSTR:   return editorinsstrFunc(Data);
		case MCODE_F_EDITOR_POS:      { LockOutput Lock(IsOutputDisabled); return editorposFunc(Data); }
		case MCODE_F_EDITOR_SEL:      return editorselFunc(Data);
		case MCODE_F_EDITOR_SET:      return editorsetFunc(Data);
		case MCODE_F_EDITOR_SETSTR:   return editorsetstrFunc(Data);
		case MCODE_F_EDITOR_SETTITLE: return editorsettitleFunc(Data);
		case MCODE_F_EDITOR_UNDO:     return editorundoFunc(Data);
		case MCODE_F_ENVIRON:         return environFunc(Data);
		case MCODE_F_FAR_CFG_GET:     return farcfggetFunc(Data);
		case MCODE_F_FATTR:           return fattrFunc(Data);
		case MCODE_F_FEXIST:          return fexistFunc(Data);
		case MCODE_F_FLOAT:           return floatFunc(Data);
		case MCODE_F_FLOCK:           return flockFunc(Data);
		case MCODE_F_FMATCH:          return fmatchFunc(Data);
		case MCODE_F_FSPLIT:          return fsplitFunc(Data);
		case MCODE_F_INDEX:           return indexFunc(Data);
		case MCODE_F_INT:             return intFunc(Data);
		case MCODE_F_ITOA:            return itowFunc(Data);
		case MCODE_F_KBDLAYOUT:       return kbdLayoutFunc(Data);
		case MCODE_F_KEY:             return keyFunc(Data);
		case MCODE_F_KEYBAR_SHOW:     return keybarshowFunc(Data);
		case MCODE_F_LCASE:           return lcaseFunc(Data);
		case MCODE_F_LEN:             return lenFunc(Data);
		case MCODE_F_MAX:             return maxFunc(Data);
		case MCODE_F_MENU_SHOW:       return menushowFunc(Data);
		case MCODE_F_MIN:             return minFunc(Data);
		case MCODE_F_MOD:             return modFunc(Data);
		case MCODE_F_MSGBOX:          return msgBoxFunc(Data);
		case MCODE_F_PANEL_FATTR:     return panelfattrFunc(Data);
		case MCODE_F_PANEL_FEXIST:    return panelfexistFunc(Data);
		case MCODE_F_PANELITEM:       return panelitemFunc(Data);
		case MCODE_F_PANEL_SELECT:    return panelselectFunc(Data);
		case MCODE_F_PANEL_SETPATH:   return panelsetpathFunc(Data);
		case MCODE_F_PANEL_SETPOS:    return panelsetposFunc(Data);
		case MCODE_F_PANEL_SETPOSIDX: return panelsetposidxFunc(Data);
		case MCODE_F_PLUGIN_EXIST:    return pluginexistFunc(Data);
		case MCODE_F_PLUGIN_LOAD:     return pluginloadFunc(Data);
		case MCODE_F_PLUGIN_UNLOAD:   return pluginunloadFunc(Data);
		case MCODE_F_REPLACE:         return replaceFunc(Data);
		case MCODE_F_RINDEX:          return rindexFunc(Data);
		case MCODE_F_SIZE2STR:        return size2strFunc(Data);
		case MCODE_F_SLEEP:           return sleepFunc(Data);
		case MCODE_F_STRING:          return stringFunc(Data);
		case MCODE_F_STRPAD:          return strpadFunc(Data);
		case MCODE_F_STRWRAP:         return strwrapFunc(Data);
		case MCODE_F_SUBSTR:          return substrFunc(Data);
		case MCODE_F_TESTFOLDER:      return testfolderFunc(Data);
		case MCODE_F_TRIM:            return trimFunc(Data);
		case MCODE_F_UCASE:           return ucaseFunc(Data);
		case MCODE_F_WAITKEY:
		{
			LockOutput Lock(IsOutputDisabled);

			++m_DisableNested; ++m_WaitKey;
			bool result=waitkeyFunc(Data);
			--m_DisableNested; --m_WaitKey;

			return result;
		}
		case MCODE_F_WINDOW_SCROLL:   return windowscrollFunc(Data);
		case MCODE_F_XLAT:            return xlatFunc(Data);
		case MCODE_F_PROMPT:          return promptFunc(Data);

		case MCODE_F_POSTNEWMACRO:
		{
			bool Result = false;
			if (Data->Count>=3 && Data->Values[0].Type==FMVT_DOUBLE && Data->Values[2].Type==FMVT_DOUBLE)
			{
				DWORD AKey = 0;
				int macroId = (int)Data->Values[0].Double;
				const wchar_t* code = Data->Values[1].Type==FMVT_STRING ? Data->Values[1].String : L"";
				MACROFLAGS_MFLAGS flags = MFLAGS_POSTFROMPLUGIN | (UINT64)Data->Values[2].Double;
				if (Data->Count >= 4 && Data->Values[3].Type == FMVT_STRING)
				{
					int key = KeyNameToKey(Data->Values[3].String);
					if (key != -1)
						AKey = key;
				}
				Result = PostNewMacro(macroId, code, flags, AKey);
			}
			PassBoolean(Result?1:0, Data);
			return 0;
		}

		case MCODE_F_CHECKALL:
		{
			BOOL Result = 0;
			if (Data->Count >= 2)
			{
				FARMACROAREA Area = (FARMACROAREA)(int)Data->Values[0].Double;
				MACROFLAGS_MFLAGS Flags = (MACROFLAGS_MFLAGS)Data->Values[1].Double;
				FARMACROCALLBACK Callback = (Data->Count>=3 && Data->Values[2].Type==FMVT_POINTER) ?
					(FARMACROCALLBACK)Data->Values[2].Pointer : nullptr;
				void* CallbackId = (Data->Count>=4  && Data->Values[3].Type==FMVT_POINTER) ?
					Data->Values[3].Pointer : nullptr;
				Result = CheckAll(Area, Flags) && (!Callback || Callback(CallbackId,AKMFLAGS_NONE));
			}
			PassBoolean(Result, Data);
			return 0;
		}

		case MCODE_F_GETOPTIONS:
		{
			DWORD Options = Global->Opt->OnlyEditorViewerUsed; // bits 0x1 and 0x2
			if (Global->Opt->Macro.DisableMacro&MDOL_ALL)       Options |= 0x4;
			if (Global->Opt->Macro.DisableMacro&MDOL_AUTOSTART) Options |= 0x8;
			if (Global->Opt->ReadOnlyConfig)                    Options |= 0x10;
			PassNumber(Options, Data);
			break;
		}

		case MCODE_F_USERMENU:
			ShowUserMenu(Data->Count,Data->Values);
			break;

		case MCODE_F_SETCUSTOMSORTMODE:
			if (Data->Count>=3 && Data->Values[0].Type==FMVT_DOUBLE  &&
				Data->Values[1].Type==FMVT_DOUBLE && Data->Values[2].Type==FMVT_BOOLEAN)
			{
				Panel *panel = Global->CtrlObject->Cp()->ActivePanel;
				if (panel && Data->Values[0].Double == 1)
					panel = Global->CtrlObject->Cp()->GetAnotherPanel(panel);

				if (panel)
				{
					int SortMode = (int)Data->Values[1].Double;
					bool InvertByDefault = Data->Values[2].Boolean != 0;
					panel->SetCustomSortMode(SortMode, false, InvertByDefault);
				}
			}
			break;

		case MCODE_F_BM_ADD:              // N=BM.Add()
		case MCODE_F_BM_CLEAR:            // N=BM.Clear()
		case MCODE_F_BM_NEXT:             // N=BM.Next()
		case MCODE_F_BM_PREV:             // N=BM.Prev()
		case MCODE_F_BM_BACK:             // N=BM.Back()
		case MCODE_F_BM_STAT:             // N=BM.Stat([N])
		case MCODE_F_BM_DEL:              // N=BM.Del([Idx]) - ������� �������� � ��������� �������� (x=1...), 0 - ������� ������� ��������
		case MCODE_F_BM_GET:              // N=BM.Get(Idx,M) - ���������� ���������� ������ (M==0) ��� ������� (M==1) �������� � �������� (Idx=1...)
		case MCODE_F_BM_GOTO:             // N=BM.Goto([n]) - ������� �� �������� � ��������� �������� (0 --> �������)
		case MCODE_F_BM_PUSH:             // N=BM.Push() - ��������� ������� ������� � ���� �������� � ����� �����
		case MCODE_F_BM_POP:              // N=BM.Pop() - ������������ ������� ������� �� �������� � ����� ����� � ������� ��������
		{
			parseParams(2,Params,Data);
			TVar& p1(Params[0]);
			TVar& p2(Params[1]);

			__int64 Result=0;
			Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

			while (f)
			{
				fo=f;
				f=f->GetTopModal();
			}

			if (!f)
				f=fo;

			if (f)
			{
				p1.toInteger();
				p2.toInteger();
				Result=f->VMProcess(CheckCode,ToPtr(p2.i()),p1.i());
			}

			return Result;
		}

		case MCODE_F_MENU_ITEMSTATUS:     // N=Menu.ItemStatus([N])
		case MCODE_F_MENU_GETVALUE:       // S=Menu.GetValue([N])
		case MCODE_F_MENU_GETHOTKEY:      // S=gethotkey([N])
		{
			parseParams(1,Params,Data);
			TVar tmpVar=Params[0];

			tmpVar.toInteger();

			int CurMMode=GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACROAREA_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
				{
					__int64 MenuItemPos=tmpVar.i()-1;
					if (CheckCode == MCODE_F_MENU_GETHOTKEY)
					{
						__int64 Result = f->VMProcess(CheckCode,nullptr,MenuItemPos);
						if (Result)
						{

							const wchar_t _value[]={static_cast<wchar_t>(Result),0};
							tmpVar=_value;
						}
						else
							tmpVar=L"";
					}
					else if (CheckCode == MCODE_F_MENU_GETVALUE)
					{
						string NewStr;
						if (f->VMProcess(CheckCode,&NewStr,MenuItemPos))
						{
							HiText2Str(NewStr, NewStr);
							RemoveExternalSpaces(NewStr);
							tmpVar=NewStr;
						}
						else
							tmpVar=L"";
					}
					else if (CheckCode == MCODE_F_MENU_ITEMSTATUS)
					{
						tmpVar=f->VMProcess(CheckCode,nullptr,MenuItemPos);
					}
				}
				else
					tmpVar=L"";
			}
			else
				tmpVar=L"";

			PassValue(&tmpVar,Data);
			return 0;
		}

		case MCODE_F_MENU_SELECT:      // N=Menu.Select(S[,N[,Dir]])
		case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S[,N])
		{
			parseParams(3,Params,Data);
			__int64 Result=-1;
			__int64 tmpMode=0;
			__int64 tmpDir=0;

			if (CheckCode == MCODE_F_MENU_SELECT)
				tmpDir=Params[2].getInteger();

			tmpMode=Params[1].getInteger();

			if (CheckCode == MCODE_F_MENU_SELECT)
				tmpMode |= (tmpDir << 8);
			else
			{
				if (tmpMode > 0)
					tmpMode--;
			}

			int CurMMode=GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACROAREA_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
					Result=f->VMProcess(CheckCode, const_cast<wchar_t*>(Params[0].toString()), tmpMode);
			}

			PassNumber(Result,Data);
			return 0;
		}

		case MCODE_F_MENU_FILTER:      // N=Menu.Filter([Action[,Mode]])
		case MCODE_F_MENU_FILTERSTR:   // S=Menu.FilterStr([Action[,S]])
		{
			parseParams(2,Params,Data);
			bool success=false;
			TVar& tmpAction(Params[0]);

			TVar tmpVar=Params[1];
			if (tmpAction.isUnknown())
				tmpAction=CheckCode == MCODE_F_MENU_FILTER ? 4 : 0;

			int CurMMode=GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACROAREA_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
				{
					if (CheckCode == MCODE_F_MENU_FILTER)
					{
						if (tmpVar.isUnknown())
							tmpVar = -1;
						tmpVar=f->VMProcess(CheckCode,(void*)static_cast<intptr_t>(tmpVar.toInteger()),tmpAction.toInteger());
						success=true;
					}
					else
					{
						string NewStr;
						if (tmpVar.isString())
							NewStr = tmpVar.toString();
						if (f->VMProcess(MCODE_F_MENU_FILTERSTR, (void*)&NewStr, tmpAction.toInteger()))
						{
							tmpVar=NewStr;
							success=true;
						}
					}
				}
			}

			if (!success)
			{
				if (CheckCode == MCODE_F_MENU_FILTER)
					tmpVar = -1;
				else
					tmpVar = L"";
			}

			PassValue(&tmpVar,Data);
			return success;
		}

		case MCODE_F_AKEY:                // V=akey(Mode[,Type])
		{
			parseParams(2,Params,Data);
			int tmpType=(int)Params[1].getInteger();
			int tmpMode=(int)Params[0].getInteger();

			MacroRecord* MR=GetTopMacro();
			if (MR==nullptr)
			{
				PassBoolean(0,Data);
				return 1;
			}

			DWORD aKey=MR->Key();

			if (!tmpType)
			{
				if (!(MR->Flags()&MFLAGS_POSTFROMPLUGIN))
				{
					INPUT_RECORD *inRec = &m_StateStack.top()->cRec;
					if (!inRec->EventType)
						inRec->EventType = KEY_EVENT;
					if(inRec->EventType == MOUSE_EVENT || inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
						aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
				}
				else if (!aKey)
					aKey=KEY_NONE;
			}

			if (!tmpMode)
				return aKey;
			else
			{
				string value;
				KeyToText(aKey,value);
				PassString(value,Data);
				return 1;
			}
		}

		case MCODE_F_HISTORY_DISABLE: // N=History.Disable([State])
		{
			parseParams(1,Params,Data);
			TVar State(Params[0]);

			DWORD oldHistoryDisable=0;
			if (!m_StateStack.empty())
			{
				MacroState* ms = m_StateStack.top();
				oldHistoryDisable=ms->HistoryDisable;

				if (!State.isUnknown())
					ms->HistoryDisable=(DWORD)State.getInteger();
			}
			return oldHistoryDisable;
		}

		case MCODE_F_MMODE:               // N=MMode(Action[,Value])
		{
			parseParams(2,Params,Data);
			__int64 nValue = Params[1].getInteger();
			TVar& Action(Params[0]);

			MacroRecord* MR=GetTopMacro();
			if (MR==nullptr)
			{
				PassBoolean(0,Data);
				return 1;
			}

			__int64 Result=0;

			switch (Action.getInteger())
			{
				case 1: // DisableOutput
				{
					Result = (MR->Flags()&MFLAGS_ENABLEOUTPUT) ? 0:1;
					if (nValue>=0 && nValue<=2 && nValue!=Result)
						MR->m_flags ^= MFLAGS_ENABLEOUTPUT;

					return PassNumber(Result, Data);
				}

				case 2: // Get MacroRecord Flags
				{
					return PassNumber((MR->Flags()<<8) | MACROAREA_COMMON, Data);
				}

				case 3: // CallPlugin Rules
				{
					Result=MR->Flags()&MFLAGS_CALLPLUGINENABLEMACRO?1:0;
					switch (nValue)
					{
						case 0: // ����������� ������� ��� ������ ������� �������� CallPlugin
							MR->m_flags&=~MFLAGS_CALLPLUGINENABLEMACRO;
							break;
						case 1: // ��������� �������
							MR->m_flags|=MFLAGS_CALLPLUGINENABLEMACRO;
							break;
						case 2: // �������� �����
							MR->m_flags^=MFLAGS_CALLPLUGINENABLEMACRO;
							break;
					}
					return PassNumber(Result, Data);
				}
			}
		}
	}

	return 0;
}

/* ------------------------------------------------------------------- */
// S=trim(S[,N])
static bool trimFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	int  mode = (int) Params[1].getInteger();
	wchar_t *p = const_cast<wchar_t*>(Params[0].toString());
	bool Ret=true;

	switch (mode)
	{
		case 0: p=RemoveExternalSpaces(p); break;  // alltrim
		case 1: p=RemoveLeadingSpaces(p); break;   // ltrim
		case 2: p=RemoveTrailingSpaces(p); break;  // rtrim
		default: Ret=false;
	}

	PassString(p, Data);
	return Ret;
}

// S=substr(S,start[,length])
static bool substrFunc(FarMacroCall* Data)
{
	/*
		TODO: http://bugs.farmanager.com/view.php?id=1480
			���� start  >= 0, �� ������� ���������, ������� �� start-������� �� ������ ������.
			���� start  <  0, �� ������� ���������, ������� �� start-������� �� ����� ������.
			���� length >  0, �� ������������ ��������� ����� �������� �������� �� length �������� �������� ������ ������� � start
			���� length <  0, �� � ������������ ��������� ����� ������������� length �������� �� ����� �������� ������, ��� ���, ��� ��� ����� ���������� � ������� start.
								���: length - ����� ����, ��� ����� (���� >=0) ��� ����������� (���� <0).

			������ ������ ������������:
				���� length = 0
				���� ...
	*/
	parseParams(3,Params,Data);
	bool Ret=false;

	int  start     = (int)Params[1].getInteger();
	wchar_t *p = const_cast<wchar_t*>(Params[0].toString());
	int length_str = StrLength(p);
	int length=Params[2].isUnknown()?length_str:(int)Params[2].getInteger();

	if (length)
	{
		if (start < 0)
		{
			start=length_str+start;
			if (start < 0)
				start=0;
		}

		if (start >= length_str)
		{
			length=0;
		}
		else
		{
			if (length > 0)
			{
				if (start+length >= length_str)
					length=length_str-start;
			}
			else
			{
				length=length_str-start+length;

				if (length < 0)
				{
					length=0;
				}
			}
		}
	}

	if (!length)
	{
		PassString(L"", Data);
	}
	else
	{
		p += start;
		p[length] = 0;
		Ret=true;
		PassString(p, Data);
	}

	return Ret;
}

static BOOL SplitFileName(const wchar_t *lpFullName,string &strDest,int nFlags)
{
#define FLAG_DISK   1
#define FLAG_PATH   2
#define FLAG_NAME   4
#define FLAG_EXT    8
	const wchar_t *s = lpFullName; //start of sub-string
	const wchar_t *p = s; //current string pointer
	const wchar_t *es = s+StrLength(s); //end of string
	const wchar_t *e; //end of sub-string

	if (!*p)
		return FALSE;

	if ((*p == L'\\') && (*(p+1) == L'\\'))   //share
	{
		p += 2;
		p = wcschr(p, L'\\');

		if (!p)
			return FALSE; //invalid share (\\server\)

		p = wcschr(p+1, L'\\');

		if (!p)
			p = es;

		if ((nFlags & FLAG_DISK) == FLAG_DISK)
		{
			strDest=s;
			strDest.resize(p-s);
		}
	}
	else
	{
		if (*(p+1) == L':')
		{
			p += 2;

			if ((nFlags & FLAG_DISK) == FLAG_DISK)
			{
				size_t Length=strDest.size()+p-s;
				strDest+=s;
				strDest.resize(Length);
			}
		}
	}

	e = nullptr;
	s = p;

	while (p)
	{
		p = wcschr(p, L'\\');

		if (p)
		{
			e = p;
			p++;
		}
	}

	if (e)
	{
		if ((nFlags & FLAG_PATH))
		{
			size_t Length=strDest.size()+e-s;
			strDest+=s;
			strDest.resize(Length);
		}

		s = e+1;
		p = s;
	}

	if (!p)
		p = s;

	e = nullptr;

	while (p)
	{
		p = wcschr(p+1, L'.');

		if (p)
			e = p;
	}

	if (!e)
		e = es;

	if (!strDest.empty())
		AddEndSlash(strDest);

	if (nFlags & FLAG_NAME)
	{
		const wchar_t *ptr=wcspbrk(s,L":");

		if (ptr)
			s=ptr+1;

		size_t Length=strDest.size()+e-s;
		strDest+=s;
		strDest.resize(Length);
	}

	if (nFlags & FLAG_EXT)
		strDest+=e;

	return TRUE;
}


// S=fsplit(S,N)
static bool fsplitFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	int m = (int)Params[1].getInteger();
	const wchar_t *s = Params[0].toString();
	bool Ret=false;
	string strPath;

	if (!SplitFileName(s,strPath,m))
		strPath.clear();
	else
		Ret=true;

	PassString(strPath, Data);
	return Ret;
}

// N=atoi(S[,radix])
static bool atoiFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	bool Ret=true;
	wchar_t *endptr;
	PassInteger(_wcstoi64(Params[0].toString(),&endptr,(int)Params[1].toInteger()), Data);
	return Ret;
}

// N=Window.Scroll(Lines[,Axis])
static bool windowscrollFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	bool Ret=false;
	int L=0;

	if (Global->Opt->WindowMode)
	{
		int Lines=(int)Params[0].i(), Columns=0;
		if (Params[1].i())
		{
			Columns=Lines;
			Lines=0;
		}

		if (Global->Console->ScrollWindow(Lines, Columns))
		{
			Ret=true;
			L=1;
		}
	}

	PassBoolean(L, Data);
	return Ret;
}

// S=itoa(N[,radix])
static bool itowFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	bool Ret=false;

	if (Params[0].isInteger() || Params[0].isDouble())
	{
		wchar_t value[65];
		int Radix=(int)Params[1].toInteger();

		if (!Radix)
			Radix=10;

		Ret=true;
		Params[0]=TVar(_i64tow(Params[0].toInteger(),value,Radix));
	}

	PassValue(&Params[0], Data);
	return Ret;
}

// N=sleep(N)
static bool sleepFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	long Period=(long)Params[0].getInteger();

	if (Period > 0)
	{
		Sleep((DWORD)Period);
		PassNumber(1, Data);
		return true;
	}

	PassNumber(0, Data);
	return false;
}


// N=KeyBar.Show([N])
static bool keybarshowFunc(FarMacroCall* Data)
{
	/*
	Mode:
		0 - visible?
			ret: 0 - hide, 1 - show, -1 - KeyBar not found
		1 - show
		2 - hide
		3 - swap
		ret: prev mode or -1 - KeyBar not found
    */
	parseParams(1,Params,Data);
	Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

	//f=f->GetTopModal();
	while (f)
	{
		fo=f;
		f=f->GetTopModal();
	}

	if (!f)
		f=fo;

	PassNumber(f?f->VMProcess(MCODE_F_KEYBAR_SHOW,nullptr,Params[0].getInteger())-1:-1, Data);
	return f != nullptr;
}


// S=key(V)
static bool keyFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	string strKeyText;

	if (Params[0].isInteger() || Params[0].isDouble())
	{
		if (Params[0].i())
			KeyToText((int)Params[0].i(),strKeyText);
	}
	else
	{
		// ��������...
		DWORD Key=(DWORD)KeyNameToKey(Params[0].s());

		if (Key != (DWORD)-1)
			strKeyText=Params[0].s();
	}

	PassString(strKeyText, Data);
	return !strKeyText.empty();
}

// V=waitkey([N,[T]])
static bool waitkeyFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	long Type=(long)Params[1].getInteger();
	long Period=(long)Params[0].getInteger();
	DWORD Key=WaitKey((DWORD)-1,Period);

	if (!Type)
	{
		string strKeyText;

		if (Key != KEY_NONE)
			if (!KeyToText(Key,strKeyText))
				strKeyText.clear();

		PassString(strKeyText, Data);
		return !strKeyText.empty();
	}

	if (Key == KEY_NONE)
		Key=-1;

	PassNumber(Key, Data);
	return Key != (DWORD)-1;
}

// n=min(n1,n2)
static bool minFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	PassValue(Params[1] < Params[0] ? &Params[1] : &Params[0], Data);
	return true;
}

// n=max(n1.n2)
static bool maxFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	PassValue(Params[1] > Params[0]  ? &Params[1] : &Params[0], Data);
	return true;
}

// n=mod(n1,n2)
static bool modFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);

	if (!Params[1].i())
	{
		_KEYMACRO(___FILEFUNCLINE___;SysLog(L"Error: Divide (mod) by zero"));
		PassNumber(0, Data);
		return false;
	}

	TVar tmp = Params[0] % Params[1];
	PassValue(&tmp, Data);
	return true;
}

// N=index(S1,S2[,Mode])
static bool indexFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	const wchar_t *s = Params[0].toString();
	const wchar_t *p = Params[1].toString();
	const wchar_t *i = !Params[2].getInteger() ? StrStrI(s,p) : StrStr(s,p);
	bool Ret = i != nullptr;
	PassNumber((i ? i-s : -1), Data);
	return Ret;
}

// S=rindex(S1,S2[,Mode])
static bool rindexFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	const wchar_t *s = Params[0].toString();
	const wchar_t *p = Params[1].toString();
	const wchar_t *i = !Params[2].getInteger() ? RevStrStrI(s,p) : RevStrStr(s,p);
	bool Ret = i != nullptr;
	PassNumber((i ? i-s : -1), Data);
	return Ret;
}

// S=Size2Str(Size,Flags[,Width])
static bool size2strFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	int Width = (int)Params[2].getInteger();

	string strDestStr;
	FileSizeToStr(strDestStr,Params[0].i(), !Width?-1:Width, Params[1].i());

	PassString(strDestStr, Data);
	return true;
}

// S=date([S])
static bool dateFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);

	if (Params[0].isInteger() && !Params[0].i())
		Params[0]=L"";

	const wchar_t *s = Params[0].toString();
	bool Ret=false;
	string strTStr;

	if (MkStrFTime(strTStr,s))
		Ret=true;
	else
		strTStr.clear();

	PassString(strTStr, Data);
	return Ret;
}

// S=xlat(S[,Flags])
/*
  Flags:
  	XLAT_SWITCHKEYBLAYOUT  = 1
		XLAT_SWITCHKEYBBEEP    = 2
		XLAT_USEKEYBLAYOUTNAME = 4
*/
static bool xlatFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	wchar_t *Str = const_cast<wchar_t*>(Params[0].toString());
	bool Ret = Xlat(Str,0,StrLength(Str),Params[1].i()) != nullptr;
	PassString(Str, Data);
	return Ret;
}

// N=beep([N])
static bool beepFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	/*
		MB_ICONASTERISK = 0x00000040
			���� ���������
		MB_ICONEXCLAMATION = 0x00000030
		    ���� �����������
		MB_ICONHAND = 0x00000010
		    ���� ����������� ������
		MB_ICONQUESTION = 0x00000020
		    ���� ������
		MB_OK = 0x0
		    ����������� ����
		SIMPLE_BEEP = 0xffffffff
		    ���������� �������
	*/
	bool Ret=MessageBeep((UINT)Params[0].i()) != FALSE;

	/*
		http://msdn.microsoft.com/en-us/library/dd743680%28VS.85%29.aspx
		BOOL PlaySound(
	    	LPCTSTR pszSound,
	    	HMODULE hmod,
	    	DWORD fdwSound
		);

		http://msdn.microsoft.com/en-us/library/dd798676%28VS.85%29.aspx
		BOOL sndPlaySound(
	    	LPCTSTR lpszSound,
	    	UINT fuSound
		);
	*/

	PassBoolean(Ret?1:0, Data);
	return Ret;
}

/*
Res=kbdLayout([N])

�������� N:
�) ����������: 0x0409 ��� 0x0419 ���...
�) 1 - ��������� ��������� (�� �����)
�) -1 - ���������� ��������� (�� �����)
�) 0 ��� �� ������ - ������� ������� ���������.

���������� ���������� ��������� (��� N=0 �������)
*/
// N=kbdLayout([N])
static bool kbdLayoutFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	DWORD dwLayout = (DWORD)Params[0].getInteger();

	BOOL Ret=TRUE;
	HKL RetLayout=0;

	wchar_t LayoutName[1024]={}; // BUGBUG!!!
	if (Global->ifn->GetConsoleKeyboardLayoutNameW(LayoutName))
	{
		wchar_t *endptr;
		DWORD res=wcstoul(LayoutName, &endptr, 16);
		RetLayout=(HKL)(intptr_t)(HIWORD(res)? res : MAKELONG(res,res));
	}

	HWND hWnd = Global->Console->GetWindow();

	if (hWnd && dwLayout)
	{
		HKL Layout = 0;
		WPARAM wParam;

		if ((long)dwLayout == -1)
		{
			wParam=INPUTLANGCHANGE_BACKWARD;
			Layout=(HKL)HKL_PREV;
		}
		else if (dwLayout == 1)
		{
			wParam=INPUTLANGCHANGE_FORWARD;
			Layout=(HKL)HKL_NEXT;
		}
		else
		{
			wParam=0;
			Layout=(HKL)(intptr_t)(HIWORD(dwLayout)? dwLayout : MAKELONG(dwLayout,dwLayout));
		}

		Ret=PostMessage(hWnd,WM_INPUTLANGCHANGEREQUEST, wParam, (LPARAM)Layout);
	}

	PassNumber(Ret? reinterpret_cast<intptr_t>(RetLayout) : 0, Data);

	return Ret != FALSE;
}

// S=prompt(["Title"[,"Prompt"[,flags[, "Src"[, "History"]]]]])
static bool promptFunc(FarMacroCall* Data)
{
	parseParams(5,Params,Data);
	TVar& ValHistory(Params[4]);
	TVar& ValSrc(Params[3]);
	DWORD Flags = (DWORD)Params[2].getInteger();
	TVar& ValPrompt(Params[1]);
	TVar& ValTitle(Params[0]);
	bool Ret=false;

	const wchar_t *history=nullptr;
	const wchar_t *title=nullptr;

	if (!(ValTitle.isInteger() && !ValTitle.i()))
		title=ValTitle.s();

	if (!(ValHistory.isInteger() && !ValHistory.i()))
		history=ValHistory.s();

	const wchar_t *src=L"";

	if (!(ValSrc.isInteger() && !ValSrc.i()))
		src=ValSrc.s();

	const wchar_t *prompt=L"";

	if (!(ValPrompt.isInteger() && !ValPrompt.i()))
		prompt=ValPrompt.s();

	string strDest;

	DWORD oldHistoryDisable=Global->CtrlObject->Macro.GetHistoryDisableMask();

	if (!(history && *history)) // Mantis#0001743: ����������� ���������� �������
		Global->CtrlObject->Macro.SetHistoryDisableMask(8); // ���� �� ������ history, �� ������������� ��������� ������� ��� ����� prompt()

	if (GetString(title,prompt,history,src,strDest,nullptr,(Flags&~FIB_CHECKBOX)|FIB_ENABLEEMPTY,nullptr,nullptr))
	{
		PassString(strDest,Data);
		Ret=true;
	}
	else
		PassBoolean(0,Data);

	Global->CtrlObject->Macro.SetHistoryDisableMask(oldHistoryDisable);

	return Ret;
}

// N=msgbox(["Title"[,"Text"[,flags]]])
static bool msgBoxFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	DWORD Flags = (DWORD)Params[2].getInteger();
	TVar& ValB(Params[1]);
	TVar& ValT(Params[0]);
	const wchar_t *title = L"";

	if (!(ValT.isInteger() && !ValT.i()))
		title=NullToEmpty(ValT.toString());

	const wchar_t *text  = L"";

	if (!(ValB.isInteger() && !ValB.i()))
		text =NullToEmpty(ValB.toString());

	Flags&=~(FMSG_KEEPBACKGROUND|FMSG_ERRORTYPE);
	Flags|=FMSG_ALLINONE;

	if (!HIWORD(Flags) || HIWORD(Flags) > HIWORD(FMSG_MB_RETRYCANCEL))
		Flags|=FMSG_MB_OK;

	//_KEYMACRO(SysLog(L"title='%s'",title));
	//_KEYMACRO(SysLog(L"text='%s'",text));
	string TempBuf = title;
	TempBuf += L"\n";
	TempBuf += text;
	int Result=pluginapi::apiMessageFn(&FarGuid,&FarGuid,Flags,nullptr,(const wchar_t * const *)UNSAFE_CSTR(TempBuf),0,0)+1;
	/*
	if (Result <= -1) // Break?
		Global->CtrlObject->Macro.SendDropProcess();
	*/
	PassNumber(Result, Data);
	return true;
}

static struct menu_less
{
	bool operator()(const MenuItemEx *el1, const MenuItemEx *el2, const SortItemParam& Param) const
	{
		if (((el1)->Flags & LIF_SEPARATOR) || ((el2)->Flags & LIF_SEPARATOR))
			return false;

		string strName1((el1)->strName);
		string strName2((el2)->strName);
		RemoveChar(strName1,L'&',true);
		RemoveChar(strName2,L'&',true);
		bool Less = NumStrCmpI(strName1.data() + Param.Offset, strName2.data() + Param.Offset) < 0;

		return Param.Reverse? !Less : Less;
	}
} MenuLess;

//S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])
//Flags:
//0x001 - BoxType
//0x002 - BoxType
//0x004 - BoxType
//0x008 - ������������ ��������� - ������ ��� ������
//0x010 - ��������� ������� ���������� �������
//0x020 - ������������� (� ������ ��������)
//0x040 - ������� ������������� ������
//0x080 - ������������� ��������� ������ |= VMENU_AUTOHIGHLIGHT
//0x100 - FindOrFilter - ����� ��� �������������
//0x200 - �������������� ��������� �����
//0x400 - ����������� ���������� ����� ����
//0x800 -
static bool menushowFunc(FarMacroCall* Data)
{
	parseParams(6,Params,Data);
	TVar& VY(Params[5]);
	TVar& VX(Params[4]);
	TVar& VFindOrFilter(Params[3]);
	DWORD Flags = (DWORD)Params[2].getInteger();
	TVar& Title(Params[1]);

	if (Title.isUnknown())
		Title=L"";

	string strTitle=Title.toString();
	string strBottom;
	TVar& Items(Params[0]);
	string strItems = Items.toString();
	ReplaceStrings(strItems,L"\r\n",L"\n");

	if (strItems.back() != L'\n')
		strItems.append(L"\n");

	TVar Result = -1;
	int BoxType = (Flags & 0x7)?(Flags & 0x7)-1:3;
	bool bResultAsIndex = (Flags & 0x08) != 0;
	bool bMultiSelect = (Flags & 0x010) != 0;
	bool bSorting = (Flags & 0x20) != 0;
	bool bPacking = (Flags & 0x40) != 0;
	bool bAutohighlight = (Flags & 0x80) != 0;
	bool bSetMenuFilter = (Flags & 0x100) != 0;
	bool bAutoNumbering = (Flags & 0x200) != 0;
	bool bExitAfterNavigate = (Flags & 0x400) != 0;
	int X = -1;
	int Y = -1;
	unsigned __int64 MenuFlags = VMENU_WRAPMODE;

	int nLeftShift=0;
	if (bAutoNumbering)
	{
		int numlines=0;
		for (const wchar_t* p=strItems.data(); *p; p++)
		{
			if (*p==L'\n') numlines++;
		}
		for (; numlines; numlines/=10)
		{
			nLeftShift++;
		}
		nLeftShift+=3;
	}

	if (!VX.isUnknown())
		X=VX.toInteger();

	if (!VY.isUnknown())
		Y=VY.toInteger();

	if (bAutohighlight)
		MenuFlags |= VMENU_AUTOHIGHLIGHT;

	int SelectedPos=0;
	int LineCount=0;
	size_t CurrentPos=0;
	size_t PosLF;
	ReplaceStrings(strTitle,L"\r\n",L"\n");
	PosLF = strTitle.find(L"\n");
	bool CRFound = PosLF != string::npos;

	if(CRFound)
	{
		strBottom=strTitle.substr(PosLF+1);
		strTitle=strTitle.substr(0,PosLF);
	}
	VMenu2 Menu(strTitle,nullptr,0,ScrY-4);
	Menu.SetBottomTitle(strBottom);
	Menu.SetFlags(MenuFlags);
	Menu.SetPosition(X,Y,0,0);
	Menu.SetBoxType(BoxType);

	PosLF = strItems.find(L"\n");
	CRFound = PosLF != string::npos;

	while(CRFound)
	{
		MenuItemEx NewItem;
		NewItem.Clear();
		size_t SubstrLen=PosLF-CurrentPos;

		if (SubstrLen==0)
			SubstrLen=1;

		NewItem.strName=strItems.substr(CurrentPos,SubstrLen);

		if (NewItem.strName!=L"\n")
		{
		const wchar_t *CurrentChar = NewItem.strName.data();
		bool bContunue=(*CurrentChar<=L'\x4');
		while(*CurrentChar && bContunue)
		{
			switch (*CurrentChar)
			{
				case L'\x1':
					NewItem.Flags|=LIF_SEPARATOR;
					CurrentChar++;
					break;

				case L'\x2':
					NewItem.Flags|=LIF_CHECKED;
					CurrentChar++;
					break;

				case L'\x3':
					NewItem.Flags|=LIF_DISABLE;
					CurrentChar++;
					break;

				case L'\x4':
					NewItem.Flags|=LIF_GRAYED;
					CurrentChar++;
					break;

				default:
				bContunue=false;
				CurrentChar++;
				break;
			}
		}
		NewItem.strName=CurrentChar;
		}
		else
			NewItem.strName.clear();

		if (bAutoNumbering && !(bSorting || bPacking) && !(NewItem.Flags & LIF_SEPARATOR))
		{
			LineCount++;
			NewItem.strName = str_printf(L"%*d - %s", nLeftShift-3, LineCount, NewItem.strName.data());
		}
		Menu.AddItem(&NewItem);
		CurrentPos=PosLF+1;
		PosLF = strItems.find(L"\n",CurrentPos);
		CRFound = PosLF != string::npos;
	}

	if (bSorting)
		Menu.SortItems(MenuLess);

	if (bPacking)
		Menu.Pack();

	if ((bAutoNumbering) && (bSorting || bPacking))
	{
		for (int i = 0; i < Menu.GetShowItemCount(); i++)
		{
			MenuItemEx *Item=Menu.GetItemPtr(i);
			if (!(Item->Flags & LIF_SEPARATOR))
			{
				LineCount++;
				Item->strName = str_printf(L"%*d - %s", nLeftShift-3, LineCount, Item->strName.data());
			}
		}
	}

	if (!VFindOrFilter.isUnknown() && !bSetMenuFilter)
	{
		if (VFindOrFilter.isInteger() || VFindOrFilter.isDouble())
		{
			if (VFindOrFilter.toInteger()-1>=0)
				Menu.SetSelectPos(VFindOrFilter.toInteger()-1,1);
			else
				Menu.SetSelectPos(Menu.GetItemCount()+VFindOrFilter.toInteger(),1);
		}
		else
			if (VFindOrFilter.isString())
				Menu.SetSelectPos(std::max(0,Menu.FindItem(0, VFindOrFilter.toString())),1);
	}

	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()) )
		frame->Lock();

	int PrevSelectedPos=Menu.GetSelectPos();
	DWORD LastKey=0;
	bool CheckFlag;

	Menu.Key(KEY_NONE);
	Menu.Run([&](int Key)->int
	{
		if (bSetMenuFilter && !VFindOrFilter.isUnknown())
		{
			string NewStr=VFindOrFilter.toString();
			Menu.VMProcess(MCODE_F_MENU_FILTERSTR, (void*)&NewStr, 1);
			bSetMenuFilter=0;
		}

		SelectedPos=Menu.GetSelectPos();
		LastKey=Key;
		int KeyProcessed = 1;
		switch (Key)
		{
			case KEY_NUMPAD0:
			case KEY_INS:
				if (bMultiSelect)
					Menu.SetCheck(!Menu.GetCheck(SelectedPos));
				break;

			case KEY_CTRLADD:
			case KEY_CTRLSUBTRACT:
			case KEY_CTRLMULTIPLY:
			case KEY_RCTRLADD:
			case KEY_RCTRLSUBTRACT:
			case KEY_RCTRLMULTIPLY:
				if (bMultiSelect)
				{
					for(int i=0; i<Menu.GetItemCount(); i++)
					{
						if (Menu.GetItemPtr(i)->Flags&MIF_HIDDEN)
							continue;

						if (Key==KEY_CTRLMULTIPLY || Key==KEY_RCTRLMULTIPLY)
						{
							CheckFlag = !Menu.GetCheck(i);
						}
						else
						{
							CheckFlag=(Key==KEY_CTRLADD || Key==KEY_RCTRLADD);
						}
						Menu.SetCheck(CheckFlag, i);
					}
				}
				break;

			case KEY_CTRLA:
			case KEY_RCTRLA:
			{
				int ItemCount=Menu.GetShowItemCount();
				if(ItemCount>ScrY-4)
					ItemCount=ScrY-4;
				Menu.SetMaxHeight(ItemCount);
				break;
			}

			case KEY_BREAK:
				Global->CtrlObject->Macro.SendDropProcess();
				Menu.Close(-1);
				break;

			default:
				KeyProcessed = 0;
		}

		if (bExitAfterNavigate && (PrevSelectedPos!=SelectedPos))
		{
			SelectedPos=Menu.GetSelectPos();
			Menu.Close();
			return KeyProcessed;
		}

		PrevSelectedPos=SelectedPos;
		return KeyProcessed;
	});

	wchar_t temp[65];

	if (Menu.GetExitCode() >= 0)
	{
		SelectedPos=Menu.GetExitCode();
		if (bMultiSelect)
		{
			Result=L"";
			for(int i=0; i<Menu.GetItemCount(); i++)
			{
				if (Menu.GetCheck(i))
				{
					if (bResultAsIndex)
					{
						_i64tow(i+1,temp,10);
						Result+=temp;
					}
					else
						Result+=(*Menu.GetItemPtr(i)).strName.data()+nLeftShift;
					Result+=L"\n";
				}
			}
			if(Result==L"")
			{
				if (bResultAsIndex)
				{
					_i64tow(SelectedPos+1,temp,10);
					Result=temp;
				}
				else
					Result=(*Menu.GetItemPtr(SelectedPos)).strName.data()+nLeftShift;
			}
		}
		else
			if(!bResultAsIndex)
				Result=(*Menu.GetItemPtr(SelectedPos)).strName.data()+nLeftShift;
			else
				Result=SelectedPos+1;
	}
	else
	{
		if (bExitAfterNavigate)
		{
			Result=SelectedPos+1;
			if ((LastKey == KEY_ESC) || (LastKey == KEY_F10) || (LastKey == KEY_BREAK))
				Result=-Result;
		}
		else
		{
			if(bResultAsIndex)
				Result=0;
			else
				Result=L"";
		}
	}

	if (frame )
		frame->Unlock();

	PassValue(&Result, Data);
	return true;
}

// S=Env(S[,Mode[,Value]])
static bool environFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	TVar& Value(Params[2]);
	TVar& Mode(Params[1]);
	TVar& S(Params[0]);
	bool Ret=false;
	string strEnv;


	if (api::GetEnvironmentVariable(S.toString(), strEnv))
		Ret=true;
	else
		strEnv.clear();

	if (Mode.i()) // Mode != 0: Set
	{
		if (Value.isUnknown() || !*Value.s())
			api::DeleteEnvironmentVariable(S.toString());
		else
			api::SetEnvironmentVariable(S.toString(), Value.toString());
	}

	PassString(strEnv, Data);
	return Ret;
}

// V=Panel.Select(panelType,Action[,Mode[,Items]])
static bool panelselectFunc(FarMacroCall* Data)
{
	parseParams(4,Params,Data);
	TVar& ValItems(Params[3]);
	int Mode=(int)Params[2].getInteger();
	DWORD Action=(int)Params[1].getInteger();
	int typePanel=(int)Params[0].getInteger();
	__int64 Result=-1;

	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	Panel *SelPanel = !typePanel ? ActivePanel : (typePanel == 1?PassivePanel:nullptr);

	if (SelPanel)
	{
		__int64 Index=-1;
		if (Mode == 1)
		{
			Index=ValItems.getInteger();
			if (!Index)
				Index=SelPanel->GetCurrentPos();
			else
				Index--;
		}

		if (Mode == 2 || Mode == 3)
		{
			string strStr=ValItems.s();
			ReplaceStrings(strStr,L"\r",L"\n");
			ReplaceStrings(strStr,L"\n\n",L"\n");
			ValItems=strStr;
		}

		MacroPanelSelect mps;
		mps.Action      = Action & 0xF;
		mps.ActionFlags = (Action & (~0xF)) >> 4;
		mps.Mode        = Mode;
		mps.Index       = Index;
		mps.Item        = &ValItems;
		Result=SelPanel->VMProcess(MCODE_F_PANEL_SELECT,&mps,0);
	}

	PassNumber(Result, Data);
	return Result != -1;
}

static bool _fattrFunc(int Type, FarMacroCall* Data)
{
	bool Ret=false;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
	long Pos=-1;

	if (!Type || Type == 2) // �� ������: fattr(0) & fexist(2)
	{
		parseParams(1,Params,Data);
		TVar& Str(Params[0]);
		api::FAR_FIND_DATA FindData;
		api::GetFindDataEx(Str.toString(), FindData);
		FileAttr=FindData.dwFileAttributes;
		Ret=true;
	}
	else // panel.fattr(1) & panel.fexist(3)
	{
		parseParams(2,Params,Data);
		TVar& S(Params[1]);
		int typePanel=(int)Params[0].getInteger();
		const wchar_t *Str = S.toString();
		Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = !typePanel ? ActivePanel : (typePanel == 1?PassivePanel:nullptr);

		if (SelPanel)
		{
			if (wcspbrk(Str,L"*?") )
				Pos=SelPanel->FindFirst(Str);
			else
				Pos = SelPanel->FindFile(Str,wcspbrk(Str,L"\\/:") != nullptr);

			if (Pos >= 0)
			{
				string strFileName;
				SelPanel->GetFileName(strFileName,Pos,FileAttr);
				Ret=true;
			}
		}
	}

	if (Type == 2) // fexist(2)
	{
		PassBoolean(FileAttr!=INVALID_FILE_ATTRIBUTES, Data);
		return 1;
	}
	else if (Type == 3) // panel.fexist(3)
		FileAttr=(DWORD)Pos+1;

	PassNumber((long)FileAttr, Data);
	return Ret;
}

// N=fattr(S)
static bool fattrFunc(FarMacroCall* Data)
{
	return _fattrFunc(0, Data);
}

// N=fexist(S)
static bool fexistFunc(FarMacroCall* Data)
{
	return _fattrFunc(2, Data);
}

// N=panel.fattr(S)
static bool panelfattrFunc(FarMacroCall* Data)
{
	return _fattrFunc(1, Data);
}

// N=panel.fexist(S)
static bool panelfexistFunc(FarMacroCall* Data)
{
	return _fattrFunc(3, Data);
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
static bool flockFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	int Ret = -1;
	int stateFLock=(int)Params[1].getInteger();
	UINT vkKey=(UINT)Params[0].getInteger();

	switch (vkKey)
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

	if (vkKey)
		Ret=SetFLockState(vkKey,stateFLock);

	PassNumber(Ret,Data);
	return Ret != -1;
}

// N=Dlg.SetFocus([ID])
static bool dlgsetfocusFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar Ret(-1);
	unsigned Index=(unsigned)Params[0].getInteger()-1;
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		Ret=CurFrame->VMProcess(MCODE_V_DLGCURPOS);
		if ((int)Index >= 0)
		{
			if(!static_cast<Dialog*>(CurFrame)->SendMessage(DM_SETFOCUS,Index,0))
				Ret=0;
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()!=-1; // ?? <= 0 ??
}

// V=Far.Cfg.Get(Key,Name)
bool farcfggetFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar& Name(Params[1]);
	TVar& Key(Params[0]);

	string strValue;
	bool result = Global->Opt->GetConfigValue(Key.s(),Name.s(), strValue);
	result ? PassString(strValue,Data) : PassBoolean(0,Data);
	return result;
}

// V=Dlg.GetValue([Pos[,InfoID]])
static bool dlggetvalueFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar Ret(-1);
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		Dialog* Dlg = static_cast<Dialog*>(CurFrame);
		TVarType typeIndex=Params[0].type();
		size_t Index=(unsigned)Params[0].getInteger()-1;
		if (typeIndex == vtUnknown || ((typeIndex==vtInteger || typeIndex==vtDouble) && (int)Index < -1))
			Index=Dlg->GetDlgFocusPos();

		TVarType typeInfoID=Params[1].type();
		int InfoID=(int)Params[1].getInteger();
		if (typeInfoID == vtUnknown || (typeInfoID == vtInteger && InfoID < 0))
			InfoID=0;

		FarGetValue fgv={sizeof(FarGetValue),InfoID,FMVT_UNKNOWN};
		auto& DlgItem = Dlg->GetAllItem();
		bool CallDialog=true;

		if (Index == (unsigned)-1)
		{
			SMALL_RECT Rect;

			if (Dlg->SendMessage(DM_GETDLGRECT,0,&Rect))
			{
				switch (InfoID)
				{
					case 0: Ret=(__int64)DlgItem.size(); break;
					case 2: Ret=Rect.Left; break;
					case 3: Ret=Rect.Top; break;
					case 4: Ret=Rect.Right; break;
					case 5: Ret=Rect.Bottom; break;
					case 6: Ret=(__int64)Dlg->GetDlgFocusPos()+1; break;
					default: Ret=0; Ret.SetType(vtUnknown); break;
				}
			}
		}
		else if (Index < DlgItem.size() && !DlgItem.empty())
		{
			const DialogItemEx& Item=DlgItem[Index];
			FARDIALOGITEMTYPES ItemType=Item.Type;
			FARDIALOGITEMFLAGS ItemFlags=Item.Flags;

			if (!InfoID)
			{
				if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
				{
					InfoID=7;
				}
				else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
				{
					FarListGetItem ListItem={sizeof(FarListGetItem)};
					ListItem.ItemIndex=Item.ListPtr->GetSelectPos();

					if (Dlg->SendMessage(DM_LISTGETITEM,Index,&ListItem))
					{
						Ret=ListItem.Item.Text;
					}
					else
					{
						Ret=L"";
					}

					InfoID=-1;
				}
				else
				{
					InfoID=10;
				}
			}

			switch (InfoID)
			{
				case 1: Ret=ItemType;    break;
				case 2: Ret=Item.X1;    break;
				case 3: Ret=Item.Y1;    break;
				case 4: Ret=Item.X2;    break;
				case 5: Ret=Item.Y2;    break;
				case 6: Ret=(Item.Flags&DIF_FOCUS)!=0; break;
				case 7:
				{
					if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
					{
						Ret=Item.Selected;
					}
					else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=Item.ListPtr->GetSelectPos()+1;
					}
					else
					{
						Ret=0ll;
						/*
						int Item->Selected;
						const char *Item->History;
						const char *Item->Mask;
						FarList *Item->ListItems;
						int  Item->ListPos;
						FAR_CHAR_INFO *Item->VBuf;
						*/
					}

					break;
				}
				case 8: Ret=(__int64)ItemFlags; break;
				case 9: Ret=(Item.Flags&DIF_DEFAULTBUTTON)!=0; break;
				case 10:
				{
					Ret=Item.strData;

					if (IsEdit(ItemType))
					{
						DlgEdit *EditPtr;

						if ((EditPtr = (DlgEdit *)(Item.ObjPtr)) )
							Ret=EditPtr->GetStringAddr();
					}

					break;
				}
				case 11:
				{
					if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=Item.ListPtr->GetItemCount();
					}
					break;
				}
			}
		}
		else if (Index >= DlgItem.size())
		{
			Ret=(__int64)InfoID;
		}
		else
			CallDialog=false;

		if (CallDialog)
		{
			fgv.Value.Type=(FARMACROVARTYPE)Ret.type();
			switch (Ret.type())
			{
				case vtUnknown:
				case vtInteger:
					fgv.Value.Integer=Ret.i();
					break;
				case vtString:
					fgv.Value.String=Ret.s();
					break;
				case vtDouble:
					fgv.Value.Double=Ret.d();
					break;
			}

			if (Dlg->SendMessage(DN_GETVALUE,Index,&fgv))
			{
				switch (fgv.Value.Type)
				{
					case FMVT_UNKNOWN:
						Ret=0;
						break;
					case FMVT_INTEGER:
						Ret=fgv.Value.Integer;
						break;
					case FMVT_DOUBLE:
						Ret=fgv.Value.Double;
						break;
					case FMVT_STRING:
						Ret=fgv.Value.String;
						break;
					default:
						Ret=-1;
						break;
				}
			}
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()!=-1;
}

// N=Editor.Pos(Op,What[,Where])
// Op: 0 - get, 1 - set
static bool editorposFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	TVar Ret(-1);
	int Where = (int)Params[2].getInteger();
	int What  = (int)Params[1].getInteger();
	int Op    = (int)Params[0].getInteger();

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_EDITOR && Global->CtrlObject->Plugins->GetCurEditor() && Global->CtrlObject->Plugins->GetCurEditor()->IsVisible())
	{
		EditorInfo ei={sizeof(EditorInfo)};
		Global->CtrlObject->Plugins->GetCurEditor()->EditorControl(ECTL_GETINFO,0,&ei);

		switch (Op)
		{
			case 0: // get
			{
				switch (What)
				{
					case 1: // CurLine
						Ret=ei.CurLine+1;
						break;
					case 2: // CurPos
						Ret=ei.CurPos+1;
						break;
					case 3: // CurTabPos
						Ret=ei.CurTabPos+1;
						break;
					case 4: // TopScreenLine
						Ret=ei.TopScreenLine+1;
						break;
					case 5: // LeftPos
						Ret=ei.LeftPos+1;
						break;
					case 6: // Overtype
						Ret=ei.Overtype;
						break;
				}

				break;
			}
			case 1: // set
			{
				EditorSetPosition esp={sizeof(EditorSetPosition)};
				esp.CurLine=-1;
				esp.CurPos=-1;
				esp.CurTabPos=-1;
				esp.TopScreenLine=-1;
				esp.LeftPos=-1;
				esp.Overtype=-1;

				switch (What)
				{
					case 1: // CurLine
						esp.CurLine=Where-1;

						if (esp.CurLine < 0)
							esp.CurLine=-1;

						break;
					case 2: // CurPos
						esp.CurPos=Where-1;

						if (esp.CurPos < 0)
							esp.CurPos=-1;

						break;
					case 3: // CurTabPos
						esp.CurTabPos=Where-1;

						if (esp.CurTabPos < 0)
							esp.CurTabPos=-1;

						break;
					case 4: // TopScreenLine
						esp.TopScreenLine=Where-1;

						if (esp.TopScreenLine < 0)
							esp.TopScreenLine=-1;

						break;
					case 5: // LeftPos
					{
						int Delta=Where-1-ei.LeftPos;
						esp.LeftPos=Where-1;

						if (esp.LeftPos < 0)
							esp.LeftPos=-1;

						esp.CurPos=ei.CurPos+Delta;
						break;
					}
					case 6: // Overtype
						esp.Overtype=Where;
						break;
				}

				int Result=Global->CtrlObject->Plugins->GetCurEditor()->EditorControl(ECTL_SETPOSITION,0,&esp);

				if (Result)
					Global->CtrlObject->Plugins->GetCurEditor()->EditorControl(ECTL_REDRAW,0,nullptr);

				Ret=Result;
				break;
			}
		}
	}

	PassValue(&Ret, Data);
	return Ret.i() != -1;
}

// OldVar=Editor.Set(Idx,Value)
static bool editorsetFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar Ret(-1);
	TVar& Value(Params[1]);
	int Index=(int)Params[0].getInteger();

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_EDITOR && Global->CtrlObject->Plugins->GetCurEditor() && Global->CtrlObject->Plugins->GetCurEditor()->IsVisible())
	{
		long longState=-1L;

		if (Index != 12)
			longState=(long)Value.toInteger();
		else
		{
			if (Value.isString() || Value.i() != -1)
				longState=0;
		}

		Options::EditorOptions EdOpt;
		Global->CtrlObject->Plugins->GetCurEditor()->GetEditorOptions(EdOpt);

		switch (Index)
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
			case 5:  // AutoDetectCodePage;
				Ret=(__int64)EdOpt.AutoDetectCodePage; break;
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
			case 15: // ShowScrollBar;
				Ret=(__int64)EdOpt.ShowScrollBar; break;
			case 16: // EditOpenedForWrite;
				Ret=(__int64)EdOpt.EditOpenedForWrite; break;
			case 17: // SearchSelFound;
				Ret=(__int64)EdOpt.SearchSelFound; break;
			case 18: // SearchRegexp;
				Ret=(__int64)EdOpt.SearchRegexp; break;
			case 19: // SearchPickUpWord;
				Ret=(__int64)EdOpt.SearchPickUpWord; break;
			case 20: // ShowWhiteSpace;
				Ret=static_cast<INT64>(EdOpt.ShowWhiteSpace); break;
			default:
				Ret=(__int64)-1L;
		}

		if (longState != -1)
		{
			switch (Index)
			{
				case 0:  // TabSize;
					EdOpt.TabSize=longState; break;
				case 1:  // ExpandTabs;
					EdOpt.ExpandTabs=longState; break;
				case 2:  // PersistentBlocks;
					EdOpt.PersistentBlocks=longState != 0; break;
				case 3:  // DelRemovesBlocks;
					EdOpt.DelRemovesBlocks=longState != 0; break;
				case 4:  // AutoIndent;
					EdOpt.AutoIndent=longState != 0; break;
				case 5:  // AutoDetectCodePage;
					EdOpt.AutoDetectCodePage=longState != 0; break;
				case 7:  // CursorBeyondEOL;
					EdOpt.CursorBeyondEOL=longState != 0; break;
				case 8:  // BSLikeDel;
					EdOpt.BSLikeDel=(longState != 0); break;
				case 9:  // CharCodeBase;
					EdOpt.CharCodeBase=longState; break;
				case 10: // SavePos;
					EdOpt.SavePos=(longState != 0); break;
				case 11: // SaveShortPos;
					EdOpt.SaveShortPos=(longState != 0); break;
				case 12: // char WordDiv[256];
					EdOpt.strWordDiv = Value.toString(); break;
				case 13: // F7Rules;
					EdOpt.F7Rules=longState != 0; break;
				case 14: // AllowEmptySpaceAfterEof;
					EdOpt.AllowEmptySpaceAfterEof=longState != 0; break;
				case 15: // ShowScrollBar;
					EdOpt.ShowScrollBar=longState != 0; break;
				case 16: // EditOpenedForWrite;
					EdOpt.EditOpenedForWrite=longState != 0; break;
				case 17: // SearchSelFound;
					EdOpt.SearchSelFound=longState != 0; break;
				case 18: // SearchRegexp;
					EdOpt.SearchRegexp=longState != 0; break;
				case 19: // SearchPickUpWord;
					EdOpt.SearchPickUpWord=longState != 0; break;
				case 20: // ShowWhiteSpace;
					EdOpt.ShowWhiteSpace=longState; break;
				default:
					Ret=-1;
					break;
			}

			Global->CtrlObject->Plugins->GetCurEditor()->SetEditorOptions(EdOpt);
			Global->CtrlObject->Plugins->GetCurEditor()->ShowStatus();
			if (Index == 0 || Index == 12 || Index == 14 || Index == 15 || Index == 20)
				Global->CtrlObject->Plugins->GetCurEditor()->Show();
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()==-1;
}

// V=Clip(N[,V])
static bool clipFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar& Val(Params[1]);
	int cmdType=(int)Params[0].getInteger();

	// ������������� ������ �������� ������ AS string
	if (cmdType != 5 && Val.isInteger() && !Val.i())
	{
		Val=L"";
		Val.toString();
	}

	int Ret=0;

	switch (cmdType)
	{
		case 0: // Get from Clipboard, "S" - ignore
		{
			string ClipText;
			if (GetClipboard(ClipText))
			{
				PassString(ClipText, Data);
				return true;
			}

			break;
		}
		case 1: // Put "S" into Clipboard
		{
			if (!*Val.s())
			{
				Clipboard clip;
				if (clip.Open())
				{
					Ret=1;
					clip.Clear();
				}
			}
			else
			{
				Ret=SetClipboard(Val.s());
			}

			PassNumber(Ret, Data); // 0!  ???
			return Ret != 0;
		}
		case 2: // Add "S" into Clipboard
		{
			TVar varClip(Val.s());
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				string CopyData;
				if (clip.Get(CopyData))
				{
					varClip = CopyData + Val.s();
				}

				Ret=clip.Set(varClip.s());
			}

			PassNumber(Ret, Data); // 0!  ???
			return Ret != 0;
		}
		case 3: // Copy Win to internal, "S" - ignore
		case 4: // Copy internal to Win, "S" - ignore
		{
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				Ret=clip.InternalCopy(cmdType == 4);
			}

			PassNumber(Ret, Data); // 0!  ???
			return Ret != 0;
		}
		case 5: // ClipMode
		{
			// 0 - flip, 1 - �������� �����, 2 - ����������, -1 - ��� ������?
			int Action=(int)Val.getInteger();
			bool mode=Clipboard::GetUseInternalClipboardState();
			if (Action >= 0)
			{
				switch (Action)
				{
					case 0: mode=!mode; break;
					case 1: mode=false; break;
					case 2: mode=true;  break;
				}
				mode=Clipboard::SetUseInternalClipboardState(mode);
			}
			PassNumber((mode?2:1), Data); // 0!  ???
			return Ret != 0;
		}
	}

	return Ret != 0;
}


// N=Panel.SetPosIdx(panelType,Idx[,InSelection])
/*
*/
static bool panelsetposidxFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	int InSelection=(int)Params[2].getInteger();
	long idxItem=(long)Params[1].getInteger();
	int typePanel=(int)Params[0].getInteger();
	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;
	__int64 Ret=0;

	if (SelPanel)
	{
		int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
		{
			size_t EndPos=SelPanel->GetFileCount();
			long idxFoundItem=0;

			if (idxItem) // < 0 || > 0
			{
				EndPos--;
				if ( EndPos > 0 )
				{
					size_t StartPos;
					long Direct=idxItem < 0?-1:1;

					if( Direct < 0 )
						idxItem=-idxItem;
					idxItem--;

					if( Direct < 0 )
					{
						StartPos=EndPos;
						EndPos=0;//InSelection?0:idxItem;
					}
					else
						StartPos=0;//!InSelection?0:idxItem;

					bool found=false;

					for (intptr_t I=StartPos ; ; I+=Direct ) // �����: ���������� I ������ ���� signed!
					{
						if (Direct > 0)
						{
							if(I > (intptr_t)EndPos)
								break;
						}
						else
						{
							if(I < (intptr_t)EndPos)
								break;
						}

						if ( (!InSelection || (InSelection && SelPanel->IsSelected(I))) && SelPanel->FileInFilter(I) )
						{
							if (idxFoundItem == idxItem)
							{
								idxItem = static_cast<long>(I);
								if (SelPanel->FilterIsEnabled())
									idxItem--;
								found=true;
								break;
							}
							idxFoundItem++;
						}
					}

					if (!found)
						idxItem=-1;

					if (idxItem != -1 && SelPanel->GoToFile(idxItem))
					{
						SelPanel->Show();
						// <Mantis#0000289> - ������, �� �� ������ :-)
						//ShellUpdatePanels(SelPanel);
						//SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
						//FrameManager->RefreshFrame(FrameManager->GetTopModal());
						// </Mantis#0000289>

						if ( !InSelection )
							Ret=(__int64)(SelPanel->GetCurrentPos()+1);
						else
							Ret=(__int64)(idxFoundItem+1);
					}
				}
			}
			else // = 0 - ������ ������� �������
			{
				if ( !InSelection )
					Ret=(__int64)(SelPanel->GetCurrentPos()+1);
				else
				{
					long CurPos=SelPanel->GetCurrentPos();
					for (size_t I=0 ; I < EndPos ; I++ )
					{
						if ( SelPanel->IsSelected(I) && SelPanel->FileInFilter(I) )
						{
							if (I == static_cast<size_t>(CurPos))
							{
								Ret=(__int64)(idxFoundItem+1);
								break;
							}
							idxFoundItem++;
						}
					}
				}
			}
		}
	}

	PassNumber(Ret, Data);
	return Ret != 0;
}

// N=panel.SetPath(panelType,pathName[,fileName])
static bool panelsetpathFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	TVar& ValFileName(Params[2]);
	TVar& Val(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	__int64 Ret=0;

	if (!(Val.isInteger() && !Val.i()))
	{
		const wchar_t *pathName=Val.s();
		const wchar_t *fileName=L"";

		if (!ValFileName.isInteger())
			fileName=ValFileName.s();

		Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

		if (SelPanel)
		{
			if (SelPanel->SetCurDir(pathName,SelPanel->GetMode()==PLUGIN_PANEL && IsAbsolutePath(pathName),FrameManager->GetCurrentFrame()->GetType() == MODALTYPE_PANELS))
			{
				ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
				PassivePanel=ActivePanel?Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel):nullptr;
				SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

				//����������� ������� ����� �� �������� ������.
				if (ActivePanel)
					ActivePanel->SetCurPath();
				// Need PointToName()?
				if (SelPanel)
				{
					SelPanel->GoToFile(fileName); // ����� ��� ��������, �.�. �������� fileName ��� ������������
					//SelPanel->Show();
					// <Mantis#0000289> - ������, �� �� ������ :-)
					//ShellUpdatePanels(SelPanel);
					SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
				}
				FrameManager->RefreshFrame(FrameManager->GetTopModal());
				// </Mantis#0000289>
				Ret=1;
			}
		}
	}

	PassBoolean(Ret, Data);
	return Ret != 0;
}

// N=Panel.SetPos(panelType,fileName)
static bool panelsetposFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar& Val(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	const wchar_t *fileName=Val.s();

	if (!fileName || !*fileName)
		fileName=L"";

	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;
	__int64 Ret=0;

	if (SelPanel)
	{
		int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
		{
			// Need PointToName()?
			if (SelPanel->GoToFile(fileName))
			{
				//SelPanel->Show();
				// <Mantis#0000289> - ������, �� �� ������ :-)
				//ShellUpdatePanels(SelPanel);
				SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
				FrameManager->RefreshFrame(FrameManager->GetTopModal());
				// </Mantis#0000289>
				Ret=(__int64)(SelPanel->GetCurrentPos()+1);
			}
		}
	}

	PassNumber(Ret, Data);
	return Ret != 0;
}

// Result=replace(Str,Find,Replace[,Cnt[,Mode]])
/*
Find=="" - return Str
Cnt==0 - return Str
Replace=="" - return Str (� ��������� ���� �������� Find)
Str=="" return ""

Mode:
      0 - case insensitive
      1 - case sensitive

*/
static bool replaceFunc(FarMacroCall* Data)
{
	parseParams(5,Params,Data);
	int Mode=(int)Params[4].getInteger();
	TVar& Count(Params[3]);
	TVar& Repl(Params[2]);
	TVar& Find(Params[1]);
	TVar& Src(Params[0]);
	__int64 Ret=1;
	// TODO: ����� ����� ��������� � ������������ � ��������!
	string strStr;
	//int lenS=(int)StrLength(Src.s());
	int lenF = StrLength(Find.s());
	//int lenR=(int)StrLength(Repl.s());
	int cnt=0;

	if( lenF )
	{
		const wchar_t *Ptr=Src.s();
		if( !Mode )
		{
			while ((Ptr=StrStrI(Ptr,Find.s())) )
			{
				cnt++;
				Ptr+=lenF;
			}
		}
		else
		{
			while ((Ptr=StrStr(Ptr,Find.s())) )
			{
				cnt++;
				Ptr+=lenF;
			}
		}
	}

	if (cnt)
	{
		//if (lenR > lenF)
		//	lenS+=cnt*(lenR-lenF+1); //???

		strStr=Src.s();
		cnt=(int)Count.i();

		if (cnt <= 0)
			cnt=-1;

		ReplaceStrings(strStr,Find.s(),Repl.s(),cnt,!Mode);
		PassString(strStr, Data);
	}
	else
		PassValue(&Src, Data);

	return Ret != 0;
}

// V=Panel.Item(typePanel,Index,TypeInfo)
static bool panelitemFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	TVar& P2(Params[2]);
	TVar& P1(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	TVar Ret(0ll);
	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

	if (!SelPanel)
	{
		PassValue(&Ret, Data);
		return false;
	}

	int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

	if (!(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL))
	{
		PassValue(&Ret, Data);
		return false;
	}

	int Index=(int)(P1.toInteger())-1;
	int TypeInfo=(int)P2.toInteger();

	if (TypePanel == TREE_PANEL)
	{
		auto treeItem = SelPanel->GetItem(Index);
		if (treeItem && !TypeInfo)
		{
			PassString(treeItem->strName, Data);
			return true;
		}
	}
	else
	{
		string strDate, strTime;

		if (TypeInfo == 11)
			SelPanel->ReadDiz();

		const FileListItem* filelistItem = static_cast<FileList*>(SelPanel)->GetItem(Index);

		if (!filelistItem)
			TypeInfo=-1;

		switch (TypeInfo)
		{
			case 0:  // Name
				Ret=TVar(filelistItem->strName);
				break;
			case 1:  // ShortName
				Ret=TVar(filelistItem->strShortName);
				break;
			case 2:  // FileAttr
				PassNumber((long)filelistItem->FileAttr, Data);
				return false;
			case 3:  // CreationTime
				ConvertDate(filelistItem->CreationTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate);
				break;
			case 4:  // AccessTime
				ConvertDate(filelistItem->AccessTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate);
				break;
			case 5:  // WriteTime
				ConvertDate(filelistItem->WriteTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate);
				break;
			case 6:  // FileSize
				PassInteger(filelistItem->FileSize, Data);
				return false;
			case 7:  // AllocationSize
				PassInteger(filelistItem->AllocationSize, Data);
				return false;
			case 8:  // Selected
				PassBoolean(filelistItem->Selected, Data);
				return false;
			case 9:  // NumberOfLinks
				PassNumber(filelistItem->NumberOfLinks, Data);
				return false;
			case 10:  // SortGroup
				PassBoolean(filelistItem->SortGroup, Data);
				return false;
			case 11:  // DizText
				Ret=TVar((const wchar_t *)filelistItem->DizText);
				break;
			case 12:  // Owner
				Ret=TVar(filelistItem->strOwner);
				break;
			case 13:  // CRC32
				PassNumber(filelistItem->CRC32, Data);
				return false;
			case 14:  // Position
				PassNumber(filelistItem->Position, Data);
				return false;
			case 15:  // CreationTime (FILETIME)
				PassInteger(FileTimeToUI64(&filelistItem->CreationTime), Data);
				return false;
			case 16:  // AccessTime (FILETIME)
				PassInteger(FileTimeToUI64(&filelistItem->AccessTime), Data);
				return false;
			case 17:  // WriteTime (FILETIME)
				PassInteger(FileTimeToUI64(&filelistItem->WriteTime), Data);
				return false;
			case 18: // NumberOfStreams
				PassNumber(filelistItem->NumberOfStreams, Data);
				return false;
			case 19: // StreamsSize
				PassInteger(filelistItem->StreamsSize, Data);
				return false;
			case 20:  // ChangeTime
				ConvertDate(filelistItem->ChangeTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate);
				break;
			case 21:  // ChangeTime (FILETIME)
				PassInteger(FileTimeToUI64(&filelistItem->ChangeTime), Data);
				return false;
			case 22:  // CustomData
				Ret=TVar(filelistItem->strCustomData);
				break;
			case 23:  // ReparseTag
			{
				PassNumber(filelistItem->ReparseTag, Data);
				return false;
			}
		}
	}

	PassValue(&Ret, Data);
	return false;
}

// N=len(V)
static bool lenFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	PassNumber(StrLength(Params[0].toString()), Data);
	return true;
}

static bool ucaseFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& Val(Params[0]);
	StrUpper(const_cast<wchar_t*>(Val.toString()));
	PassValue(&Val, Data);
	return true;
}

static bool lcaseFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& Val(Params[0]);
	StrLower(const_cast<wchar_t*>(Val.toString()));
	PassValue(&Val, Data);
	return true;
}

static bool stringFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& Val(Params[0]);
	Val.toString();
	PassValue(&Val, Data);
	return true;
}

// S=StrPad(Src,Cnt[,Fill[,Op]])
static bool strpadFunc(FarMacroCall* Data)
{
	string strDest;
	parseParams(4,Params,Data);
	TVar& Src(Params[0]);
	if (Src.isUnknown())
	{
		Src=L"";
		Src.toString();
	}
	int Cnt=(int)Params[1].getInteger();
	TVar& Fill(Params[2]);
	if (Fill.isUnknown())
		Fill=L" ";
	DWORD Op=(DWORD)Params[3].getInteger();

	strDest=Src.s();
	int LengthFill = StrLength(Fill.s());
	if (Cnt > 0 && LengthFill > 0)
	{
		int LengthSrc  = StrLength(Src.s());
		int FineLength = Cnt-LengthSrc;

		if (FineLength > 0)
		{
			wchar_t_ptr NewFill(FineLength + 1);
			if (NewFill)
			{
				const wchar_t *pFill=Fill.s();

				for (int I=0; I < FineLength; ++I)
					NewFill[I]=pFill[I%LengthFill];
				NewFill[FineLength]=0;

				int CntL=0, CntR=0;
				switch (Op)
				{
					case 0: // right
						CntR=FineLength;
						break;
					case 1: // left
						CntL=FineLength;
						break;
					case 2: // center
						if (LengthSrc > 0)
						{
							CntL=FineLength / 2;
							CntR=FineLength-CntL;
						}
						else
							CntL=FineLength;
						break;
				}

				string strPad(NewFill.get());
				strPad.resize(CntL);
				strPad+=strDest;
				strPad.append(NewFill.get(), CntR);
				strDest=strPad;
			}
		}
	}

	PassString(strDest, Data);
	return true;
}

// S=StrWrap(Text,Width[,Break[,Flags]])
static bool strwrapFunc(FarMacroCall* Data)
{
	parseParams(4,Params,Data);
	DWORD Flags=(DWORD)Params[3].getInteger();
	TVar& Break(Params[2]);
	int Width=(int)Params[1].getInteger();
	TVar& Text(Params[0]);

	if (Break.isInteger() && !Break.i())
	{
		Break=L"";
		Break.toString();
	}

	string strDest;
	FarFormatText(Text.s(),Width,strDest,*Break.s()?Break.s():nullptr,Flags);
	PassString(strDest, Data);
	return true;
}

static bool intFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& Val(Params[0]);
	Val.toInteger();
	PassInteger(Val.i(), Data);
	return true;
}

static bool floatFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& Val(Params[0]);
	Val.toDouble();
	PassValue(&Val, Data);
	return true;
}

static bool absFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& tmpVar(Params[0]);

	if (tmpVar < 0ll)
		tmpVar=-tmpVar;

	PassValue(&tmpVar, Data);
	return true;
}

static bool ascFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& tmpVar(Params[0]);

	if (tmpVar.isString())
		PassNumber((DWORD)(WORD)*tmpVar.toString(), Data);
	else
		PassValue(&tmpVar, Data);

	return true;
}

static bool chrFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar tmpVar(Params[0]);

	if (tmpVar.isNumber())
	{
		const wchar_t tmp[]={static_cast<wchar_t>(tmpVar.i()), L'\0'};
		tmpVar = tmp;
		tmpVar.toString();
	}

	PassValue(&tmpVar, Data);
	return true;
}

// N=FMatch(S,Mask)
static bool fmatchFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar& Mask(Params[1]);
	TVar& S(Params[0]);
	filemasks FileMask;

	if (FileMask.Set(Mask.toString(), FMF_SILENT))
		PassNumber(FileMask.Compare(S.toString()), Data);
	else
		PassNumber(-1, Data);
	return true;
}

// V=Editor.Sel(Action[,Opt])
static bool editorselFunc(FarMacroCall* Data)
{
	/*
	 MCODE_F_EDITOR_SEL
	  Action: 0 = Get Param
	              Opt:  0 = return FirstLine
	                    1 = return FirstPos
	                    2 = return LastLine
	                    3 = return LastPos
	                    4 = return block type (0=nothing 1=stream, 2=column)
	              return: 0 = failure, 1... request value

	          1 = Set Pos
	              Opt:  0 = begin block (FirstLine & FirstPos)
	                    1 = end block (LastLine & LastPos)
	              return: 0 = failure, 1 = success

	          2 = Set Stream Selection Edge
	              Opt:  0 = selection start
	                    1 = selection finish
	              return: 0 = failure, 1 = success

	          3 = Set Column Selection Edge
	              Opt:  0 = selection start
	                    1 = selection finish
	              return: 0 = failure, 1 = success
	          4 = Unmark selected block
	              Opt: ignore
	              return 1
	*/
	parseParams(2,Params,Data);
	TVar Ret(0ll);
	TVar& Opts(Params[1]);
	TVar& Action(Params[0]);

	FARMACROAREA Mode=Global->CtrlObject->Macro.GetMode();
	int NeedType = Mode == MACROAREA_EDITOR?MODALTYPE_EDITOR:(Mode == MACROAREA_VIEWER?MODALTYPE_VIEWER:(Mode == MACROAREA_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS)); // MACROAREA_SHELL?
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CurFrame && CurFrame->GetType()==NeedType)
	{
		if (Mode==MACROAREA_SHELL && Global->CtrlObject->CmdLine->IsVisible())
			Ret=Global->CtrlObject->CmdLine->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.i());
		else
			Ret=CurFrame->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.i());
	}

	PassValue(&Ret, Data);
	return Ret.i() == 1;
}

// V=Editor.Undo(Action)
static bool editorundoFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar Ret(0ll);
	TVar& Action(Params[0]);

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_EDITOR && Global->CtrlObject->Plugins->GetCurEditor() && Global->CtrlObject->Plugins->GetCurEditor()->IsVisible())
	{
		EditorUndoRedo eur={sizeof(EditorUndoRedo)};
		eur.Command=static_cast<EDITOR_UNDOREDO_COMMANDS>(Action.toInteger());
		Ret = Global->CtrlObject->Plugins->GetCurEditor()->EditorControl(ECTL_UNDOREDO,0,&eur);
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Editor.SetTitle([Title])
static bool editorsettitleFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar Ret(0ll);
	TVar& Title(Params[0]);

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_EDITOR && Global->CtrlObject->Plugins->GetCurEditor() && Global->CtrlObject->Plugins->GetCurEditor()->IsVisible())
	{
		if (Title.isInteger() && !Title.i())
		{
			Title=L"";
			Title.toString();
		}
		Ret = Global->CtrlObject->Plugins->GetCurEditor()->EditorControl(ECTL_SETTITLE, 0, const_cast<wchar_t*>(Title.s()));
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Editor.DelLine([Line])
static bool editordellineFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar Ret(0ll);
	TVar& Line(Params[0]);

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_EDITOR && Global->CtrlObject->Plugins->GetCurEditor() && Global->CtrlObject->Plugins->GetCurEditor()->IsVisible())
	{
		if (Line.isNumber())
		{
			Ret = Global->CtrlObject->Plugins->GetCurEditor()->VMProcess(MCODE_F_EDITOR_DELLINE, nullptr, Line.getInteger()-1);
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Editor.InsStr([S[,Line]])
static bool editorinsstrFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar Ret(0ll);
	TVar& S(Params[0]);
	TVar& Line(Params[1]);

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_EDITOR && Global->CtrlObject->Plugins->GetCurEditor() && Global->CtrlObject->Plugins->GetCurEditor()->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S=L"";
				S.toString();
			}

			Ret = Global->CtrlObject->Plugins->GetCurEditor()->VMProcess(MCODE_F_EDITOR_INSSTR, const_cast<wchar_t*>(S.s()), Line.getInteger()-1);
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Editor.SetStr([S[,Line]])
static bool editorsetstrFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar Ret(0ll);
	TVar& S(Params[0]);
	TVar& Line(Params[1]);

	if (Global->CtrlObject->Macro.GetMode()==MACROAREA_EDITOR && Global->CtrlObject->Plugins->GetCurEditor() && Global->CtrlObject->Plugins->GetCurEditor()->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S=L"";
				S.toString();
			}

			Ret = Global->CtrlObject->Plugins->GetCurEditor()->VMProcess(MCODE_F_EDITOR_SETSTR, const_cast<wchar_t*>(S.s()), Line.getInteger()-1);
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Plugin.Exist(Guid)
static bool pluginexistFunc(FarMacroCall* Data)
{
	int Ret=0;
	if (Data->Count>0 && Data->Values[0].Type==FMVT_STRING)
	{
		GUID guid;
		Ret=(StrToGuid(Data->Values[0].String,guid) && Global->CtrlObject->Plugins->FindPlugin(guid))?1:0;
	}
	PassBoolean(Ret, Data);
	return Ret != 0;
}

// N=Plugin.Load(DllPath[,ForceLoad])
static bool pluginloadFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar Ret(0ll);
	TVar& ForceLoad(Params[1]);
	TVar& DllPath(Params[0]);
	if (DllPath.s())
		Ret = pluginapi::apiPluginsControl(nullptr, !ForceLoad.i()?PCTL_LOADPLUGIN:PCTL_FORCEDLOADPLUGIN, 0, const_cast<wchar_t*>(DllPath.s()));
	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Plugin.UnLoad(DllPath)
static bool pluginunloadFunc(FarMacroCall* Data)
{
	int Ret=0;
	if (Data->Count>0 && Data->Values[0].Type==FMVT_STRING)
	{
		Plugin* p = Global->CtrlObject->Plugins->GetPlugin(Data->Values[0].String);
		if(p)
		{
			Ret=(int)pluginapi::apiPluginsControl(p, PCTL_UNLOADPLUGIN, 0, nullptr);
		}
	}
	PassNumber(Ret, Data);
	return Ret!=0;
}

// N=testfolder(S)
/*
���������� ���� ��������� ������������ ��������:

TSTFLD_NOTFOUND   (2) - ��� ������
TSTFLD_NOTEMPTY   (1) - �� �����
TSTFLD_EMPTY      (0) - �����
TSTFLD_NOTACCESS (-1) - ��� �������
TSTFLD_ERROR     (-2) - ������ (������ ��������� ��� ��������� ������ ��� ��������� ������������� �������)
*/
static bool testfolderFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& tmpVar(Params[0]);
	__int64 Ret=TSTFLD_ERROR;

	if (tmpVar.isString())
	{
		DisableElevation de;
		Ret=(__int64)TestFolder(tmpVar.s());
	}

	PassNumber(Ret, Data);
	return Ret != 0;
}

// ���������� ����������� ���� ���������� �������
intptr_t KeyMacro::AssignMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	string strKeyText;
	static int LastKey=0;
	static DlgParam *KMParam=nullptr;
	const INPUT_RECORD* record=nullptr;
	int key=0;

	if (Msg == DN_CONTROLINPUT)
	{
		record=(const INPUT_RECORD *)Param2;
		if (record->EventType==KEY_EVENT)
		{
			key = InputRecordToKey((const INPUT_RECORD *)Param2);
			if (key&KEY_RCTRL) key = (key&~KEY_RCTRL)|KEY_CTRL;
			if (key&KEY_RALT) key = (key&~KEY_RALT)|KEY_ALT;
		}
	}

	//_SVS(SysLog(L"LastKey=%d Msg=%s",LastKey,_DLGMSG_ToName(Msg)));
	if (Msg == DN_INITDIALOG)
	{
		KMParam=reinterpret_cast<DlgParam*>(Param2);
		LastKey=0;
		// <�������, ������� �� ������� � ������� ����������>
		static const DWORD PreDefKeyMain[]=
		{
			//KEY_CTRLDOWN,KEY_RCTRLDOWN,KEY_ENTER,KEY_NUMENTER,KEY_ESC,KEY_F1,KEY_CTRLF5,KEY_RCTRLF5,
			KEY_CTRLDOWN,KEY_ENTER,KEY_NUMENTER,KEY_ESC,KEY_F1,KEY_CTRLF5,
		};

		std::for_each(CONST_RANGE(PreDefKeyMain, i)
		{
			KeyToText(i, strKeyText);
			Dlg->SendMessage(DM_LISTADDSTR, 2, UNSAFE_CSTR(strKeyText));
		});

		static const DWORD PreDefKey[]=
		{
			KEY_MSWHEEL_UP,KEY_MSWHEEL_DOWN,KEY_MSWHEEL_LEFT,KEY_MSWHEEL_RIGHT,
			KEY_MSLCLICK,KEY_MSRCLICK,KEY_MSM1CLICK,KEY_MSM2CLICK,KEY_MSM3CLICK,
#if 0
			KEY_MSLDBLCLICK,KEY_MSRDBLCLICK,KEY_MSM1DBLCLICK,KEY_MSM2DBLCLICK,KEY_MSM3DBLCLICK,
#endif
		};
		static const DWORD PreDefModKey[]=
		{
			//0,KEY_CTRL,KEY_RCTRL,KEY_SHIFT,KEY_ALT,KEY_RALT,KEY_CTRLSHIFT,KEY_RCTRLSHIFT,KEY_CTRLALT,KEY_RCTRLRALT,KEY_CTRLRALT,KEY_RCTRLALT,KEY_ALTSHIFT,KEY_RALTSHIFT,
			0,KEY_CTRL,KEY_SHIFT,KEY_ALT,KEY_CTRLSHIFT,KEY_CTRLALT,KEY_ALTSHIFT,
		};

		std::for_each(CONST_RANGE(PreDefKey, i)
		{
			Dlg->SendMessage(DM_LISTADDSTR, 2, const_cast<wchar_t*>(L"\1"));

			std::for_each(CONST_RANGE(PreDefModKey, j)
			{
				KeyToText(i | j, strKeyText);
				Dlg->SendMessage(DM_LISTADDSTR, 2, UNSAFE_CSTR(strKeyText));
			});
		});

		Dlg->SendMessage(DM_SETTEXTPTR,2,nullptr);
		// </�������, ������� �� ������� � ������� ����������>
	}
	else if (Param1 == 2 && Msg == DN_EDITCHANGE)
	{
		LastKey=0;
		_SVS(SysLog(L"[%d] ((FarDialogItem*)Param2)->PtrData='%s'",__LINE__,((FarDialogItem*)Param2)->Data));
		key=KeyNameToKey(((FarDialogItem*)Param2)->Data);

		if (key != -1 && !KMParam->Recurse)
			goto M1;
	}
	else if (Msg == DN_CONTROLINPUT && record->EventType==KEY_EVENT && (((key&KEY_END_SKEY) < KEY_END_FKEY) ||
	                           (((key&KEY_END_SKEY) > INTERNAL_KEY_BASE) && (key&KEY_END_SKEY) < INTERNAL_KEY_BASE_2)))
	{
		//if((key&0x00FFFFFF) >= 'A' && (key&0x00FFFFFF) <= 'Z' && ShiftPressed)
		//key|=KEY_SHIFT;

		//_SVS(SysLog(L"Macro: Key=%s",_FARKEY_ToName(key)));
		// <��������� ������ ������: F1 & Enter>
		// Esc & (Enter � ���������� Enter) - �� ������������
		if (key == KEY_ESC ||
		        ((key == KEY_ENTER||key == KEY_NUMENTER) && (LastKey == KEY_ENTER||LastKey == KEY_NUMENTER)) ||
		        key == KEY_CTRLDOWN || key == KEY_RCTRLDOWN ||
		        key == KEY_F1)
		{
			return FALSE;
		}

		/*
		// F1 - ������ ������ - ����� ���� 2 ����
		// ������ ��� ����� ������� ����,
		// � ������ ��� - ������ ��� ��� ����������
		if(key == KEY_F1 && LastKey!=KEY_F1)
		{
		  LastKey=KEY_F1;
		  return FALSE;
		}
		*/
		// ���� ���-�� ��� ������ � Enter`�� ������������
		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),(LastKey?_FARKEY_ToName(LastKey):L"")));

		if ((key == KEY_ENTER||key == KEY_NUMENTER) && LastKey && !(LastKey == KEY_ENTER||LastKey == KEY_NUMENTER))
			return FALSE;

		// </��������� ������ ������: F1 & Enter>
M1:
		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),LastKey?_FARKEY_ToName(LastKey):L""));

		if ((key&0x00FFFFFF) > 0x7F && (key&0x00FFFFFF) < 0xFFFF)
			key=KeyToKeyLayout(key&0x0000FFFF)|(key&~0x0000FFFF);

		if (key<0xFFFF)
		{
			key=Upper(static_cast<wchar_t>(key));
		}

		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),LastKey?_FARKEY_ToName(LastKey):L""));
		KMParam->Key=(DWORD)key;
		KeyToText(key, strKeyText);

		// ���� ��� ���� ����� ������...
		GetMacroData Data;
		if (LM_GetMacro(&Data,KMParam->Mode,strKeyText,true,true) && Data.MacroId)
		{
			// ����� ������� ��������� ������ ��� ��������.
			if (m_RecCode.empty() || Data.Area!=MACROAREA_COMMON_INTERNAL)
			{
				string strBufKey=Data.Code;
				InsertQuote(strBufKey);

				bool SetChange = m_RecCode.empty();
				LangString strBuf;
				if (Data.Area==MACROAREA_COMMON_INTERNAL)
				{
					strBuf = SetChange ? MMacroCommonDeleteKey : MMacroCommonReDefinedKey;
					//"����� ������������ '%1'   ����� ������� : ��� ����������."
				}
				else
				{
					strBuf = SetChange ? MMacroDeleteKey : MMacroReDefinedKey;
					//"������������ '%1'   ����� ������� : ��� ����������."
				}
				strBuf << strKeyText;

				int Result=0;
				{
					const wchar_t* NoKey=MSG(MNo);
					Result=Message(MSG_WARNING,SetChange?3:2,MSG(MWarning),
					          strBuf.data(),
					          MSG(MMacroSequence),
					          strBufKey.data(),
					          MSG(SetChange?MMacroDeleteKey2:MMacroReDefinedKey2),
					          MSG(MYes),
					          (SetChange?MSG(MMacroEditKey):NoKey),
					          (!SetChange?nullptr:NoKey));
				}

				if (!Result)
				{
					// � ����� ������ - ������������
					Dlg->SendMessage(DM_CLOSE,1,0);
					return TRUE;
				}
				else if (SetChange && Result == 1)
				{
					string strDescription;

					if ( *Data.Code )
						strBufKey=Data.Code;

					if ( *Data.Description )
						strDescription=Data.Description;

					if (GetMacroSettings(key,Data.Flags,strBufKey.data(),strDescription.data()))
					{
						KMParam->Flags = Data.Flags;
						KMParam->Changed = true;
						// � ����� ������ - ������������
						Dlg->SendMessage(DM_CLOSE,1,0);
						return TRUE;
					}
				}

				// ����� - ����� �� �������� "���", �� � �� ��� � ���� ���
				//  � ������ ������� ���� �����.
				strKeyText.clear();
			}
		}

		KMParam->Recurse++;
		Dlg->SendMessage(DM_SETTEXTPTR,2, UNSAFE_CSTR(strKeyText));
		KMParam->Recurse--;
		//if(key == KEY_F1 && LastKey == KEY_F1)
		//LastKey=-1;
		//else
		LastKey = key;
		return TRUE;
	}
	return Dlg->DefProc(Msg,Param1,Param2);
}

int KeyMacro::AssignMacroKey(DWORD &MacroKey, UINT64 &Flags)
{
	/*
	  +------ Define macro ------+
	  | Press the desired key    |
	  | ________________________ |
	  +--------------------------+
	*/
	FarDialogItem MacroAssignDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,30,4,0,nullptr,nullptr,0,MSG(MDefineMacroTitle)},
		{DI_TEXT,-1,2,0,2,0,nullptr,nullptr,0,MSG(MDefineMacro)},
		{DI_COMBOBOX,5,3,28,3,0,nullptr,nullptr,DIF_FOCUS|DIF_DEFAULTBUTTON,L""},
	};
	auto MacroAssignDlg = MakeDialogItemsEx(MacroAssignDlgData);
	DlgParam Param={Flags, 0, StartMode, 0, false};
	//_SVS(SysLog(L"StartMode=%d",StartMode));
	Global->IsProcessAssignMacroKey++;
	Dialog Dlg(MacroAssignDlg, this, &KeyMacro::AssignMacroDlgProc, &Param);
	Dlg.SetPosition(-1,-1,34,6);
	Dlg.SetHelp(L"KeyMacro");
	Dlg.Process();
	Global->IsProcessAssignMacroKey--;

	if (Dlg.GetExitCode() == -1)
		return 0;

	MacroKey = Param.Key;
	Flags = Param.Flags;
	return Param.Changed ? 2 : 1;
}
