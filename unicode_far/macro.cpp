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

#ifdef FAR_LUA
#include "macro.hpp"
#include "FarGuid.hpp"
#include "cmdline.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "frame.hpp" //???
#include "keyboard.hpp"
#include "keys.hpp" //FIXME
#include "lockscrn.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "panel.hpp"
#include "scrbuf.hpp"
#include "strmix.hpp"
#include "syslog.hpp"

#include "macroopcode.hpp"
#include "interf.hpp"
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

static BOOL CheckAll(MACROMODEAREA Mode, UINT64 CurFlags);

#if 0
void print_opcodes()
{
	FILE* fp=fopen("opcodes.tmp", "w");
	if (!fp) return;

	fprintf(fp, "MCODE_OP_EXIT=0x%X // ������������� ��������� ���������� �����������������������\n", MCODE_OP_EXIT);

	fprintf(fp, "MCODE_OP_JMP=0x%X // Jumps..\n", MCODE_OP_JMP);
	fprintf(fp, "MCODE_OP_JZ=0x%X\n", MCODE_OP_JZ);
	fprintf(fp, "MCODE_OP_JNZ=0x%X\n", MCODE_OP_JNZ);
	fprintf(fp, "MCODE_OP_JLT=0x%X\n", MCODE_OP_JLT);
	fprintf(fp, "MCODE_OP_JLE=0x%X\n", MCODE_OP_JLE);
	fprintf(fp, "MCODE_OP_JGT=0x%X\n", MCODE_OP_JGT);
	fprintf(fp, "MCODE_OP_JGE=0x%X\n", MCODE_OP_JGE);

	fprintf(fp, "MCODE_OP_NOP=0x%X // ��� ��������\n", MCODE_OP_NOP);

	fprintf(fp, "MCODE_OP_SAVE=0x%X // ������������ ����������. ��� ���������� ��������� DWORD (��� � $Text).\n", MCODE_OP_SAVE);
	fprintf(fp, "MCODE_OP_SAVEREPCOUNT=0x%X\n", MCODE_OP_SAVEREPCOUNT);
	fprintf(fp, "MCODE_OP_PUSHUNKNOWN=0x%X // ������������������� �������� (���������� ��������� �������)\n", MCODE_OP_PUSHUNKNOWN);
	fprintf(fp, "MCODE_OP_PUSHINT=0x%X // �������� �������� �� ����. ����\n", MCODE_OP_PUSHINT);
	fprintf(fp, "MCODE_OP_PUSHFLOAT=0x%X // �������� �������� �� ����. double\n", MCODE_OP_PUSHFLOAT);
	fprintf(fp, "MCODE_OP_PUSHSTR=0x%X // �������� - ��������� DWORD\n", MCODE_OP_PUSHSTR);
	fprintf(fp, "MCODE_OP_PUSHVAR=0x%X // ��� ��������� ������� (��� � $Text)\n", MCODE_OP_PUSHVAR);
	fprintf(fp, "MCODE_OP_PUSHCONST=0x%X // � ���� �������� ���������\n", MCODE_OP_PUSHCONST);

	fprintf(fp, "MCODE_OP_REP=0x%X // $rep - ������� ������ �����\n", MCODE_OP_REP);
	fprintf(fp, "MCODE_OP_END=0x%X // $end - ������� ����� �����/�������\n", MCODE_OP_END);

	// ����������� ��������
	fprintf(fp, "MCODE_OP_PREINC=0x%X // ++a\n", MCODE_OP_PREINC);
	fprintf(fp, "MCODE_OP_PREDEC=0x%X // --a\n", MCODE_OP_PREDEC);
	fprintf(fp, "MCODE_OP_POSTINC=0x%X // a++\n", MCODE_OP_POSTINC);
	fprintf(fp, "MCODE_OP_POSTDEC=0x%X // a--\n", MCODE_OP_POSTDEC);

	fprintf(fp, "MCODE_OP_UPLUS=0x%X // +a\n", MCODE_OP_UPLUS);
	fprintf(fp, "MCODE_OP_NEGATE=0x%X // -a\n", MCODE_OP_NEGATE);
	fprintf(fp, "MCODE_OP_NOT=0x%X // !a\n", MCODE_OP_NOT);
	fprintf(fp, "MCODE_OP_BITNOT=0x%X // ~a\n", MCODE_OP_BITNOT);

	// ���������� ��������
	fprintf(fp, "MCODE_OP_MUL=0x%X // a *  b\n", MCODE_OP_MUL);
	fprintf(fp, "MCODE_OP_DIV=0x%X // a /  b\n", MCODE_OP_DIV);

	fprintf(fp, "MCODE_OP_ADD=0x%X // a +  b\n", MCODE_OP_ADD);
	fprintf(fp, "MCODE_OP_SUB=0x%X // a -  b\n", MCODE_OP_SUB);

	fprintf(fp, "MCODE_OP_BITSHR=0x%X // a >> b\n", MCODE_OP_BITSHR);
	fprintf(fp, "MCODE_OP_BITSHL=0x%X // a << b\n", MCODE_OP_BITSHL);

	fprintf(fp, "MCODE_OP_LT=0x%X // a <  b\n", MCODE_OP_LT);
	fprintf(fp, "MCODE_OP_LE=0x%X // a <= b\n", MCODE_OP_LE);
	fprintf(fp, "MCODE_OP_GT=0x%X // a >  b\n", MCODE_OP_GT);
	fprintf(fp, "MCODE_OP_GE=0x%X // a >= b\n", MCODE_OP_GE);

	fprintf(fp, "MCODE_OP_EQ=0x%X // a == b\n", MCODE_OP_EQ);
	fprintf(fp, "MCODE_OP_NE=0x%X // a != b\n", MCODE_OP_NE);

	fprintf(fp, "MCODE_OP_BITAND=0x%X // a &  b\n", MCODE_OP_BITAND);

	fprintf(fp, "MCODE_OP_BITXOR=0x%X // a ^  b\n", MCODE_OP_BITXOR);

	fprintf(fp, "MCODE_OP_BITOR=0x%X // a |  b\n", MCODE_OP_BITOR);

	fprintf(fp, "MCODE_OP_AND=0x%X // a && b\n", MCODE_OP_AND);

	fprintf(fp, "MCODE_OP_XOR=0x%X // a ^^ b\n", MCODE_OP_XOR);

	fprintf(fp, "MCODE_OP_OR=0x%X // a || b\n", MCODE_OP_OR);

	fprintf(fp, "MCODE_OP_ADDEQ=0x%X // a +=  b\n", MCODE_OP_ADDEQ);
	fprintf(fp, "MCODE_OP_SUBEQ=0x%X // a -=  b\n", MCODE_OP_SUBEQ);
	fprintf(fp, "MCODE_OP_MULEQ=0x%X // a *=  b\n", MCODE_OP_MULEQ);
	fprintf(fp, "MCODE_OP_DIVEQ=0x%X // a /=  b\n", MCODE_OP_DIVEQ);
	fprintf(fp, "MCODE_OP_BITSHREQ=0x%X // a >>= b\n", MCODE_OP_BITSHREQ);
	fprintf(fp, "MCODE_OP_BITSHLEQ=0x%X // a <<= b\n", MCODE_OP_BITSHLEQ);
	fprintf(fp, "MCODE_OP_BITANDEQ=0x%X // a &=  b\n", MCODE_OP_BITANDEQ);
	fprintf(fp, "MCODE_OP_BITXOREQ=0x%X // a ^=  b\n", MCODE_OP_BITXOREQ);
	fprintf(fp, "MCODE_OP_BITOREQ=0x%X // a |=  b\n", MCODE_OP_BITOREQ);

	fprintf(fp, "MCODE_OP_DISCARD=0x%X // ������ �������� � ������� �����\n", MCODE_OP_DISCARD);
	fprintf(fp, "MCODE_OP_DUP=0x%X // �������������� ������� �������� � �����\n", MCODE_OP_DUP);
	fprintf(fp, "MCODE_OP_SWAP=0x%X // �������� ������� ��� �������� � ������� �����\n", MCODE_OP_SWAP);
	fprintf(fp, "MCODE_OP_POP=0x%X // ��������� �������� ���������� � ������ �� ������� �����\n", MCODE_OP_POP);
	fprintf(fp, "MCODE_OP_COPY=0x%X // %%a=%%d, ���� �� ������������\n", MCODE_OP_COPY);

	fprintf(fp, "MCODE_OP_KEYS=0x%X // �� ���� ����� ������� ������ ���� ������\n", MCODE_OP_KEYS);
	fprintf(fp, "MCODE_OP_ENDKEYS=0x%X // ������ ���� �����������.\n", MCODE_OP_ENDKEYS);

	/* ************************************************************************* */
	fprintf(fp, "MCODE_OP_IF=0x%X // ������-�� ��� ������ � �������\n", MCODE_OP_IF);
	fprintf(fp, "MCODE_OP_ELSE=0x%X // �� ������� ������� :)\n", MCODE_OP_ELSE);
	fprintf(fp, "MCODE_OP_WHILE=0x%X\n", MCODE_OP_WHILE);
	fprintf(fp, "MCODE_OP_CONTINUE=0x%X // $continue\n", MCODE_OP_CONTINUE);
	fprintf(fp, "MCODE_OP_BREAK=0x%X // $break\n", MCODE_OP_BREAK);
	/* ************************************************************************* */

	fprintf(fp, "MCODE_OP_XLAT=0x%X\n", MCODE_OP_XLAT);
	fprintf(fp, "MCODE_OP_PLAINTEXT=0x%X\n", MCODE_OP_PLAINTEXT);

	fprintf(fp, "MCODE_OP_AKEY=0x%X // $AKey - �������, ������� ������� ������\n", MCODE_OP_AKEY);
	fprintf(fp, "MCODE_OP_SELWORD=0x%X // $SelWord - �������� \"�����\"\n", MCODE_OP_SELWORD);


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
	fprintf(fp, "MCODE_F_HISTIORY_DISABLE=0x%X // N=History.Disable([State])\n", MCODE_F_HISTIORY_DISABLE);
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

void SZLOG (const char *fmt, ...)
{
	FILE* log=_wfopen(L"c:\\lua.log",L"at");
	if (log)
	{
		va_list argp;
		fprintf(log, "FAR: ");
		va_start(argp, fmt);
		vfprintf(log, fmt, argp);
		va_end(argp);
		fprintf(log, "\n");
		fclose(log);
	}
}

static TVar __varTextDate;

// ��� ������� ���������� �������
struct DlgParam
{
	UINT64 Flags;
	KeyMacro *Handle;
	DWORD Key;
	int Mode;
	int Recurse;
	bool Changed;
};

struct TMacroKeywords
{
	const wchar_t *Name;   // ������������
	DWORD Value;           // ��������
};

TMacroKeywords MKeywordsArea[] =
{
	{L"Other",                    MACRO_OTHER},
	{L"Shell",                    MACRO_SHELL},
	{L"Viewer",                   MACRO_VIEWER},
	{L"Editor",                   MACRO_EDITOR},
	{L"Dialog",                   MACRO_DIALOG},
	{L"Search",                   MACRO_SEARCH},
	{L"Disks",                    MACRO_DISKS},
	{L"MainMenu",                 MACRO_MAINMENU},
	{L"Menu",                     MACRO_MENU},
	{L"Help",                     MACRO_HELP},
	{L"Info",                     MACRO_INFOPANEL},
	{L"QView",                    MACRO_QVIEWPANEL},
	{L"Tree",                     MACRO_TREEPANEL},
	{L"FindFolder",               MACRO_FINDFOLDER},
	{L"UserMenu",                 MACRO_USERMENU},
	{L"ShellAutoCompletion",      MACRO_SHELLAUTOCOMPLETION},
	{L"DialogAutoCompletion",     MACRO_DIALOGAUTOCOMPLETION},
	{L"Common",                   MACRO_COMMON},
};
TMacroKeywords MKeywordsFlags[] =
{
	// �����
	{L"DisableOutput",      MFLAGS_DISABLEOUTPUT},
	{L"RunAfterFARStart",   MFLAGS_RUNAFTERFARSTART},
	{L"EmptyCommandLine",   MFLAGS_EMPTYCOMMANDLINE},
	{L"NotEmptyCommandLine",MFLAGS_NOTEMPTYCOMMANDLINE},
	{L"EVSelection",        MFLAGS_EDITSELECTION},
	{L"NoEVSelection",      MFLAGS_EDITNOSELECTION},

	{L"NoFilePanels",       MFLAGS_NOFILEPANELS},
	{L"NoPluginPanels",     MFLAGS_NOPLUGINPANELS},
	{L"NoFolders",          MFLAGS_NOFOLDERS},
	{L"NoFiles",            MFLAGS_NOFILES},
	{L"Selection",          MFLAGS_SELECTION},
	{L"NoSelection",        MFLAGS_NOSELECTION},

	{L"NoFilePPanels",      MFLAGS_PNOFILEPANELS},
	{L"NoPluginPPanels",    MFLAGS_PNOPLUGINPANELS},
	{L"NoPFolders",         MFLAGS_PNOFOLDERS},
	{L"NoPFiles",           MFLAGS_PNOFILES},
	{L"PSelection",         MFLAGS_PSELECTION},
	{L"NoPSelection",       MFLAGS_PNOSELECTION},

	{L"NoSendKeysToPlugins",MFLAGS_NOSENDKEYSTOPLUGINS},
};
const wchar_t* GetAreaName(DWORD AreaValue) {return GetNameOfValue(AreaValue, MKeywordsArea);}

const string FlagsToString(FARKEYMACROFLAGS Flags)
{
	return FlagsToString(Flags, MKeywordsFlags);
}

FARKEYMACROFLAGS StringToFlags(const string& strFlags)
{
	return StringToFlags(strFlags, MKeywordsFlags);
}

MacroRecord::MacroRecord():
	m_area(MACRO_COMMON),
	m_flags(0),
	m_key(-1),
	m_guid(FarGuid),
	m_id(nullptr),
	m_callback(nullptr),
	m_handle(nullptr)
{
}

MacroRecord::MacroRecord(MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,int Key,string Name,string Code,string Description):
	m_area(Area),
	m_flags(Flags),
	m_key(Key),
	m_name(Name),
	m_code(Code),
	m_description(Description),
	m_guid(FarGuid),
	m_id(nullptr),
	m_callback(nullptr),
	m_handle(nullptr)
{
}

MacroRecord& MacroRecord::operator= (const MacroRecord& src)
{
	if (this != &src)
	{
		m_area = src.m_area;
		m_flags = src.m_flags;
		m_key = src.m_key;
		m_name = src.m_name;
		m_code = src.m_code;
		m_description = src.m_description;
		m_guid = src.m_guid;
		m_id = src.m_id;
		m_callback = src.m_callback;
		m_handle = src.m_handle;
	}
	return *this;
}

MacroState::MacroState() :
	Executing(0),
	KeyProcess(0),
	HistoryDisable(0),
	UseInternalClipboard(false)
{
}

MacroState& MacroState::operator= (const MacroState& src)
{
	cRec=src.cRec;
	Executing=src.Executing;

	//m_MacroQueue=src.m_MacroQueue; // ��� ������ (��� ����-������������)
	m_MacroQueue.Clear();
	MacroState* ms = const_cast<MacroState*>(&src); // ������� ��������
	MacroRecord* curr = ms->m_MacroQueue.First();
	while (curr)
	{
		m_MacroQueue.Push(curr);
		curr = ms->m_MacroQueue.Next(curr);
	}

	KeyProcess=src.KeyProcess;
	HistoryDisable=src.HistoryDisable;
	UseInternalClipboard=src.UseInternalClipboard;
	return *this;
}

void KeyMacro::PushState()
{
	m_CurState.UseInternalClipboard=Clipboard::GetUseInternalClipboardState();
	m_StateStack.Push(m_CurState);
	m_CurState = MacroState();
}

void KeyMacro::PopState()
{
	if (!m_StateStack.empty())
	{
		m_StateStack.Pop(m_CurState);
		Clipboard::SetUseInternalClipboardState(m_CurState.UseInternalClipboard);
	}
}

KeyMacro::KeyMacro():
	m_Mode(MACRO_SHELL),
	m_Recording(MACROMODE_NOMACRO),
	m_RecMode(MACRO_OTHER),
	m_LockScr(nullptr),
	m_PluginIsRunning(0),
	m_InternalInput(0)
{
	//print_opcodes();
}

KeyMacro::~KeyMacro()
{
}

// ������������� ���� ����������
void KeyMacro::InitInternalVars(bool InitedRAM)
{
	//InitInternalLIBVars();

	//if (LockScr)
	//{
	//	delete LockScr;
	//	LockScr=nullptr;
	//}

	if (InitedRAM)
	{
		m_CurState.m_MacroQueue.Clear();
		m_CurState.Executing=MACROMODE_NOMACRO;
	}

	m_CurState.HistoryDisable=0;
	m_RecCode.Clear();
	m_RecDescription.Clear();

	m_Recording=MACROMODE_NOMACRO;
	m_StateStack.Free();
	m_InternalInput=0;
	//VMStack.Free();
}

int KeyMacro::IsRecording()
{
	return m_Recording;
}

int KeyMacro::IsExecuting()
{
	MacroRecord* m = GetCurMacro();
	return m ? (m->Flags()&MFLAGS_NOSENDKEYSTOPLUGINS) ?
		MACROMODE_EXECUTING : MACROMODE_EXECUTING_COMMON : MACROMODE_NOMACRO;
}

int KeyMacro::IsExecutingLastKey()
{
	return false;
}

int KeyMacro::IsDsableOutput()
{
	MacroRecord* m = GetCurMacro();
	return m && (m->Flags()&MFLAGS_DISABLEOUTPUT);
}

DWORD KeyMacro::SetHistoryDisableMask(DWORD Mask)
{
	DWORD OldHistoryDisable=m_CurState.HistoryDisable;
	m_CurState.HistoryDisable=Mask;
	return OldHistoryDisable;
}

DWORD KeyMacro::GetHistoryDisableMask()
{
	return m_CurState.HistoryDisable;
}

bool KeyMacro::IsHistoryDisable(int TypeHistory)
{
	return !m_CurState.m_MacroQueue.Empty() && (m_CurState.HistoryDisable & (1 << TypeHistory));
}

void KeyMacro::SetMode(int Mode) //FIXME: int->MACROMODEAREA
{
	m_Mode=(MACROMODEAREA)Mode;
}

MACROMODEAREA KeyMacro::GetMode(void)
{
	return m_Mode;
}

bool KeyMacro::LoadMacros(bool InitedRAM,bool LoadAll)
{
	//SZLOG("+KeyMacro::LoadMacros, InitedRAM=%s, LoadALL=%s", InitedRAM?"true":"false", LoadAll?"true":"false");
	int ErrCount=0;
	InitInternalVars(InitedRAM);

	for (int k=0; k<MACRO_LAST; k++)
	{
		m_Macros[k].Free();
	}

	if (Opt.Macro.DisableMacro&MDOL_ALL)
		return false;

	int Areas[MACRO_LAST];

	for (int ii=MACRO_OTHER;ii<MACRO_LAST;++ii)
	{
		Areas[ii]=ii;
	}

	if (!LoadAll)
	{
		// "������� �� �����" �������� ������� - ����� ����������� ������ ��, ��� �� ����� �������� MACRO_LAST
		Areas[MACRO_SHELL]=
			Areas[MACRO_SEARCH]=
			Areas[MACRO_DISKS]=
			Areas[MACRO_MAINMENU]=
			Areas[MACRO_INFOPANEL]=
			Areas[MACRO_QVIEWPANEL]=
			Areas[MACRO_TREEPANEL]=
			Areas[MACRO_USERMENU]= // <-- Mantis#0001594
			Areas[MACRO_SHELLAUTOCOMPLETION]=
			Areas[MACRO_FINDFOLDER]=MACRO_LAST;
	}

	for (int ii=MACRO_OTHER;ii<MACRO_LAST;++ii)
	{
		if (Areas[ii]==MACRO_LAST) continue;
		if (!ReadKeyMacro((MACROMODEAREA)ii))
		{
			ErrCount++;
		}
	}

/*
	for(size_t ii=0;ii<MACRO_LAST;++ii)
	{
		{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"count: %d,%d\n",m_Macros[ii].getSize(),ii); fclose(log);}}
		for(size_t jj=0;jj<m_Macros[ii].getSize();++jj)
		{
			{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"%ls\n",m_Macros[ii].getItem(jj)->Code().CPtr()); fclose(log);}}
		}
	}
*/
	return ErrCount?false:true;
}

void KeyMacro::SaveMacros()
{
	WriteMacro();
}

static __int64 msValues[constMsLAST];

void KeyMacro::SetMacroConst(int ConstIndex, __int64 Value)
{
	msValues[ConstIndex] = Value;
}

int KeyMacro::GetCurRecord(MacroRecord* RBuf,int *KeyPos)
{
	return (m_Recording != MACROMODE_NOMACRO) ? m_Recording : IsExecuting();
}

void* KeyMacro::CallMacroPlugin(unsigned Type,void* Data)
{
	//SZLOG("+KeyMacro::CallMacroPlugin, Type=%s", Type==OPEN_MACROINIT?"OPEN_MACROINIT":	Type==OPEN_MACROSTEP?"OPEN_MACROSTEP": Type==OPEN_MACROPARSE ? "OPEN_MACROPARSE": "UNKNOWN");
	void* ptr;
	m_PluginIsRunning++;

	MacroRecord* macro = GetCurMacro();
	if (macro)
		ScrBuf.SetLockCount(0);

	bool result=CtrlObject->Plugins->CallPlugin(LuamacroGuid,Type,Data,&ptr) != 0;

	if (macro && macro->m_handle && (macro->Flags()&MFLAGS_DISABLEOUTPUT))
		ScrBuf.Lock();

	m_PluginIsRunning--;
	//SZLOG("-KeyMacro::CallMacroPlugin, return=%p", result?ptr:nullptr);
	return result?ptr:nullptr;
}

bool KeyMacro::InitMacroExecution()
{
	//SZLOG("+InitMacroExecution");
	MacroRecord* macro = GetCurMacro();
	if (macro)
	{
		FarMacroValue values[2]={{FMVT_INTEGER,{0}},{FMVT_STRING,{0}}};
		values[0].Integer=MCT_MACROINIT;
		values[1].String=macro->Code();
		OpenMacroInfo info={sizeof(OpenMacroInfo),ARRAYSIZE(values),values};
		macro->m_handle=CallMacroPlugin(OPEN_LUAMACRO,&info);
		if (macro->m_handle)
		{
			m_LastKey = L"first_key";
			return true;
		}
		RemoveCurMacro();
	}
	return false;
}

int KeyMacro::ProcessEvent(const struct FAR_INPUT_RECORD *Rec)
{
	//SZLOG("+KeyMacro::ProcessEvent");
	if (m_InternalInput || Rec->IntKey==KEY_IDLE || Rec->IntKey==KEY_NONE || !FrameManager->GetCurrentFrame()) //FIXME: ���������� �� Rec->IntKey
		return false;
	//{FILE* log=fopen("c:\\lua.log","at"); if(log) {fprintf(log,"ProcessEvent: %08x\n",Rec->IntKey); fclose(log);}}
	string textKey;
	//if (InputRecordToText(&Rec->Rec,textKey))//FIXME: �� ����� Ctrl ��� ��� �������, � �� �����.
	if (KeyToText(Rec->IntKey,textKey))
	{
		bool ctrldot=(0==StrCmpI(textKey,L"Ctrl.")||0==StrCmpI(textKey,L"RCtrl."));
		bool ctrlshiftdot=(0==StrCmpI(textKey,L"CtrlShift.")||0==StrCmpI(textKey,L"RCtrlShift."));

		if (m_Recording==MACROMODE_NOMACRO)
		{
			if (ctrldot||ctrlshiftdot)
			{
				// ������� 18
				if (Opt.Policies.DisabledOptions&FFPOL_CREATEMACRO)
					return false;

				UpdateLockScreen(false);

				// ��� ��?
				m_RecMode=(m_Mode==MACRO_SHELL&&!WaitInMainLoop)?MACRO_OTHER:m_Mode;
				StartMode=m_RecMode;
				// � ����������� �� ����, ��� ������ ������ ������, ��������� ����� ����� (Ctrl-.
				// � ��������� ������� ����) ��� ����������� (Ctrl-Shift-. - ��� �������� ������ �������)
				m_Recording=ctrldot?MACROMODE_RECORDING_COMMON:MACROMODE_RECORDING;

				m_RecCode.Clear();
				m_RecDescription.Clear();
				ScrBuf.ResetShadow();
				ScrBuf.Flush();
				WaitInFastFind--;
				return true;
			}
			else
			{
				if (m_CurState.m_MacroQueue.Empty())
				{
					int Key = Rec->IntKey;
					if ((Key&(~KEY_CTRLMASK)) > 0x01 && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN) // 0xFFFF ??
					{
						if ((Key&(~KEY_CTRLMASK)) > 0x7F && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN)
							Key=KeyToKeyLayout(Key&0x0000FFFF)|(Key&(~0x0000FFFF));

						if ((DWORD)Key < KEY_FKEY_BEGIN)
							Key=Upper(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
					}

					int Area, Index;
					Index = GetIndex(&Area,Key,textKey,m_Mode,true,true);
					if (Index != -1)
					{
						MacroRecord* macro = m_Macros[Area].getItem(Index);
						if (CheckAll(macro->Area(), macro->Flags()) && (!macro->m_callback||macro->m_callback(macro->m_id,AKMFLAGS_NONE)))
						{
							int ret = PostNewMacro(macro->Code(),macro->Flags(),Rec->IntKey,false);
							if (ret)
							{
								m_CurState.HistoryDisable=0;
								m_CurState.cRec=Rec->Rec;
								//IsRedrawEditor=CtrlObject->Plugins->CheckFlags(PSIF_ENTERTOOPENPLUGIN)?FALSE:TRUE; //FIXME
							}
							return ret;
						}
					}
				}
			}
		}
		else // m_Recording!=MACROMODE_NOMACRO
		{
			if (ctrldot||ctrlshiftdot) // ������� ����� ������?
			{
				int WaitInMainLoop0=WaitInMainLoop;
				m_InternalInput=1;
				WaitInMainLoop=FALSE;
				// �������� _�������_ �����, � �� _��������� �����������_
				FrameManager->GetCurrentFrame()->Lock(); // ������� ���������� ������
				DWORD MacroKey;
				// ���������� ����� �� ���������.
				UINT64 Flags=MFLAGS_DISABLEOUTPUT|MFLAGS_CALLPLUGINENABLEMACRO; // ???
				int AssignRet=AssignMacroKey(MacroKey,Flags);
				FrameManager->ResetLastInputRecord();
				FrameManager->GetCurrentFrame()->Unlock(); // ������ ����� :-)

				if (AssignRet && AssignRet!=2 && !m_RecCode.IsEmpty())
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

				WaitInMainLoop=WaitInMainLoop0;
				m_InternalInput=0;
				if (AssignRet)
				{
					int Area, Pos;
					string strKey;
					KeyToText(MacroKey, strKey);

					// � ������� common ����� ������ ������ ��� ��������
					Pos = GetIndex(&Area,MacroKey,strKey,StartMode,m_RecCode.IsEmpty(),true);
					Flags |= MFLAGS_NEEDSAVEMACRO|(m_Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);

					if (Pos == -1)
					{
						if (!m_RecCode.IsEmpty())
						{
							m_Macros[StartMode].addItem(MacroRecord(StartMode,Flags,MacroKey,strKey,m_RecCode,m_RecDescription));
						}
					}
					else
					{
						MacroRecord* macro = m_Macros[Area].getItem(Pos);
						if (!m_RecCode.IsEmpty())
						{
							macro->m_flags = Flags;
							macro->m_code = m_RecCode;
							macro->m_description = m_RecDescription;
						}
						else
						{
							macro->m_flags = MFLAGS_NEEDSAVEMACRO|MFLAGS_DISABLEMACRO;
							macro->m_code.Clear();
						}
					}
				}

				//{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"%ls\n",m_RecCode.CPtr()); fclose(log);}}
				m_Recording=MACROMODE_NOMACRO;
				m_RecCode.Clear();
				m_RecDescription.Clear();
				ScrBuf.RestoreMacroChar();
				WaitInFastFind++;

				if (Opt.AutoSaveSetup)
					WriteMacro(); // �������� ������ ���������!

				return true;
			}
			else
			{
				//{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"key: %08x\n",Rec->IntKey); fclose(log);}}
				if (!IsProcessAssignMacroKey)
				{
					if(!m_RecCode.IsEmpty()) m_RecCode+=L" ";
					m_RecCode+=textKey;
				}
				return false;
			}
		}
	}
	return false;
}

int KeyMacro::GetKey()
{
	//SZLOG("+KeyMacro::GetKey, m_PluginIsRunning=%d", m_PluginIsRunning);
	if (m_PluginIsRunning || m_InternalInput || !FrameManager->GetCurrentFrame())
	{
		return 0;
	}

	MacroRecord* macro;
	FarMacroValue mp_values[3]={{FMVT_INTEGER,{0}},{FMVT_DOUBLE,{0}},{FMVT_DOUBLE,{0}}};
	mp_values[0].Integer=MCT_MACROSTEP;
	OpenMacroInfo mp_info={sizeof(OpenMacroInfo), 2, mp_values};

	while ((macro=GetCurMacro()) != nullptr && (macro->m_handle || InitMacroExecution()))
	{
		mp_values[1].Double=(intptr_t)macro->m_handle;

		MacroPluginReturn* mpr = (MacroPluginReturn*)CallMacroPlugin(OPEN_LUAMACRO,&mp_info);
		mp_info.Count=2;
		if (mpr == nullptr)
			break;

		switch (mpr->ReturnType)
		{
			case MPRT_NORMALFINISH:
			case MPRT_ERRORFINISH:
			{
				if (macro->Flags() & MFLAGS_DISABLEOUTPUT)
					ScrBuf.Unlock();

				RemoveCurMacro();
				if (m_CurState.m_MacroQueue.Empty())
				{
					ScrBuf.RestoreMacroChar();
					return 0;
				}

				continue;
			}

			case MPRT_KEYS:
			{
				const wchar_t* key = mpr->Args[0].String;
				m_LastKey = key;

				if ((macro->Flags()&MFLAGS_DISABLEOUTPUT) && ScrBuf.GetLockCount()==0)
					ScrBuf.Lock();

				if (!StrCmpI(key, L"AKey"))
				{
					DWORD aKey=KEY_NONE;
					if (!(macro->Flags()&MFLAGS_POSTFROMPLUGIN))
					{
						INPUT_RECORD *inRec=&m_CurState.cRec;
						if (!inRec->EventType)
							inRec->EventType = KEY_EVENT;
						if(inRec->EventType == MOUSE_EVENT || inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
							aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
					}
					else
						aKey=macro->Key();
					//SZLOG("-KeyMacro::GetKey, returned 0x%X", aKey);
					return aKey;
				}

				if (!StrCmpI(key, L"SelWord"))
					return KEY_OP_SELWORD;

				if (!StrCmpI(key, L"XLat"))
					return KEY_OP_XLAT;

				int iKey = KeyNameToKey(key);
				//SZLOG("-KeyMacro::GetKey, returned 0x%X", iKey==-1 ? KEY_NONE:iKey);
				return iKey==-1 ? KEY_NONE:iKey;
			}

			case MPRT_PRINT:
			{
				if ((macro->Flags()&MFLAGS_DISABLEOUTPUT) && ScrBuf.GetLockCount()==0)
					ScrBuf.Lock();

				__varTextDate = mpr->Args[0].String;
				return KEY_OP_PLAINTEXT;
			}

			case MPRT_PLUGINCALL: // V=Plugin.Call(SysID[,param])
			{
				int Ret=0;
				size_t count = mpr->ArgNum;
				if(count>0 && mpr->Args[0].Type==FMVT_STRING)
				{
					TVar SysID = mpr->Args[0].String;
					GUID guid;

					if (StrToGuid(SysID.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))
					{
						FarMacroValue *vParams = count>1 ? mpr->Args+1:nullptr;
						OpenMacroInfo info={sizeof(OpenMacroInfo),count-1,vParams};
						MacroRecord* macro = GetCurMacro();
						bool CallPluginRules = (macro->Flags()&MFLAGS_CALLPLUGINENABLEMACRO) != 0;
						if (CallPluginRules)
							PushState();
						else
							m_InternalInput++;

						void* ResultCallPlugin=nullptr;

						if (CtrlObject->Plugins->CallPlugin(guid,OPEN_FROMMACRO,&info,&ResultCallPlugin))
							Ret=(intptr_t)ResultCallPlugin;

						mp_values[2].Double=Ret;
						mp_info.Count=3;

						//if (MR != Work.MacroWORK) // ??? Mantis#0002094 ???
							//MR=Work.MacroWORK;

						if (CallPluginRules)
							PopState();
						else
						{
							//VMStack.Push(Ret);
							m_InternalInput--;
						}
					}
					//else
						//VMStack.Push(Ret);

					//if (Work.Executing == MACROMODE_NOMACRO)
						//goto return_func;
				}
				//return Ret;
			}
		}
	}

	//SZLOG("-KeyMacro::GetKey, returned 0");
	return 0;
}

int KeyMacro::PeekKey()
{
	//SZLOG("+PeekKey");
	int key=0;
	if (!m_InternalInput && IsExecuting())
	{
		if (!StrCmp(m_LastKey, L"first_key")) //FIXME
			return KEY_NONE;
		if ((key=KeyNameToKey(m_LastKey)) == -1)
			key=0;
	}
	return key;
}

// �������� ��� ���� �� �����
int KeyMacro::GetAreaCode(const wchar_t *AreaName)
{
	for (int i=MACRO_OTHER; i < MACRO_LAST; i++)
		if (!StrCmpI(MKeywordsArea[i].Name,AreaName))
			return i;

	return -4; //FIXME: MACRO_FUNCS-1;
}

int KeyMacro::GetMacroKeyInfo(bool FromDB, int Mode, int Pos, string &strKeyName, string &strDescription)
{
	const int MACRO_FUNCS = -3;
	if (Mode >= MACRO_FUNCS && Mode < MACRO_LAST)
	{
		if (FromDB)
		{
			if (Mode >= MACRO_OTHER)
			{
				// TODO
				return Pos+1;
			}
			else if (Mode == MACRO_FUNCS)
			{
				// TODO: MACRO_FUNCS
				return -1;
			}
			else
			{
				// TODO
				return Pos+1;
			}
		}
		else
		{
			if (Mode >= MACRO_OTHER)
			{
				size_t Len=CtrlObject->Macro.m_Macros[Mode].getSize();
				if (Len && (size_t)Pos < Len)
				{
					MacroRecord *MPtr=CtrlObject->Macro.m_Macros[Mode].getItem(Pos);
					strKeyName=MPtr->Name();
					strDescription=NullToEmpty(MPtr->Description());
					return Pos+1;
				}
			}
		}
	}

	return -1;
}

void KeyMacro::SendDropProcess()
{//FIXME
}

bool KeyMacro::CheckWaitKeyFunc()
{//FIXME
	return false;
}

// ������� ��������� ������� ������� ������� � �������
// Ret=-1 - �� ������ �������.
// ���� CheckMode=-1 - ������ ������ � ����� ������, �.�. ������ ����������
// StrictKeys=true - �� �������� ��������� ����� Ctrl/Alt ������ (���� ����� �� �����)
// FIXME: parameter StrictKeys.
int KeyMacro::GetIndex(int* area, int Key, string& strKey, int CheckMode, bool UseCommon, bool StrictKeys)
{
	//SZLOG("GetIndex: %08x,%ls",Key,strKey.CPtr());
	int loops = UseCommon && CheckMode!=-1 && CheckMode!=MACRO_COMMON ? 2:1;
	for (int k=0; k<loops; k++)
	{
		int startArea = (CheckMode==-1) ? 0:CheckMode;
		int endArea = (CheckMode==-1) ? MACRO_LAST:CheckMode+1;
		for (int i=startArea; i<endArea; i++)
		{
			for (unsigned j=0; j<m_Macros[i].getSize(); j++)
			{
				MacroRecord* MPtr = m_Macros[i].getItem(j);
				bool found = (Key != -1 && Key != 0) ?
					!((MPtr->Key() ^ Key) & ~0xFFFF) &&
							Upper(static_cast<WCHAR>(MPtr->Key()))==Upper(static_cast<WCHAR>(Key)) :
					!strKey.IsEmpty() && !StrCmpI(strKey,MPtr->Name());

				if (found && !(MPtr->Flags()&MFLAGS_DISABLEMACRO))
						//&& (!MPtr->m_callback || MPtr->m_callback(MPtr->m_id,AKMFLAGS_NONE)))
				{
					*area = i; return j;
				}
			}
		}
		CheckMode=MACRO_COMMON;
	}
	*area = -1;
	return -1;
}

// �������, ����������� ������� ��� ������ ����
void KeyMacro::RunStartMacro()
{
	if ((Opt.Macro.DisableMacro&MDOL_ALL) || (Opt.Macro.DisableMacro&MDOL_AUTOSTART) || Opt.OnlyEditorViewerUsed)
		return;

	if (!CtrlObject || !CtrlObject->Cp() || !CtrlObject->Cp()->ActivePanel || !CtrlObject->Plugins->IsPluginsLoaded())
		return;

	static int IsRunStartMacro=FALSE;
	if (IsRunStartMacro)
		return;

	for (unsigned j=0; j<m_Macros[MACRO_SHELL].getSize(); j++)
	{
		MacroRecord* macro = m_Macros[MACRO_SHELL].getItem(j);
		MACROFLAGS_MFLAGS flags = macro->Flags();
		if (!(flags&MFLAGS_DISABLEMACRO) && (flags&MFLAGS_RUNAFTERFARSTART) && CheckAll(macro->Area(), flags))
		{
			PostNewMacro(macro->Code(), flags&~MFLAGS_DISABLEOUTPUT); //FIXME
		}
	}
	IsRunStartMacro=TRUE;
}

int KeyMacro::AddMacro(const wchar_t *PlainText,const wchar_t *Description,enum MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback)
{
	if (Area < 0 || Area >= MACRO_LAST)
		return FALSE;

	string strKeyText;
	if (!(InputRecordToText(&AKey, strKeyText) && ParseMacroString(PlainText,true)))
		return FALSE;

	int Key=InputRecordToKey(&AKey);
	MacroRecord macro(Area,Flags,Key,strKeyText,PlainText,Description);
	macro.m_guid = PluginId;
	macro.m_id = Id;
	macro.m_callback = Callback;
	m_Macros[Area].addItem(macro);
	return TRUE;
}

int KeyMacro::DelMacro(const GUID& PluginId,void* Id)
{
	for (int i=0; i<MACRO_LAST; i++)
	{
		for (unsigned j=0; j<m_Macros[i].getSize(); j++)
		{
			MacroRecord* macro = m_Macros[i].getItem(j);
			if (!(macro->m_flags&MFLAGS_DISABLEMACRO) && macro->m_id==Id && IsEqualGUID(macro->m_guid,PluginId))
			{
				macro->m_flags = MFLAGS_DISABLEMACRO;
				macro->m_name.Clear();
				macro->m_code.Clear();
				macro->m_description.Clear();
				return TRUE;
			}
		}
	}
	return FALSE;
}

bool KeyMacro::PostNewMacro(const wchar_t *PlainText,UINT64 Flags,DWORD AKey,bool onlyCheck)
{
	if (!m_InternalInput && ParseMacroString(PlainText, onlyCheck))
	{
		string strKeyText;
		KeyToText(AKey,strKeyText);
		MacroRecord macro(MACRO_COMMON, Flags, AKey, strKeyText, PlainText, L"");
		m_CurState.m_MacroQueue.Push(&macro);
		return true;
	}
	return false;
}

bool KeyMacro::ReadKeyMacro(MACROMODEAREA Area)
{
	unsigned __int64 MFlags=0;
	string strKey,strArea,strMFlags;
	string strSequence, strDescription;
	string strGUID;
	int ErrorCount=0;

	strArea=GetAreaName(static_cast<MACROMODEAREA>(Area));

	while(MacroCfg->EnumKeyMacros(strArea, strKey, strMFlags, strSequence, strDescription))
	{
		RemoveExternalSpaces(strKey);
		RemoveExternalSpaces(strSequence);
		RemoveExternalSpaces(strDescription);

		if (strSequence.IsEmpty())
		{
			//ErrorCount++; // �������������, ���� �� ����������� ������ "Sequence"
			continue;
		}
		if (!ParseMacroString(strSequence))
		{
			ErrorCount++;
			continue;
		}

		MFlags=StringToFlags(strMFlags);

		if (Area == MACRO_EDITOR || Area == MACRO_DIALOG || Area == MACRO_VIEWER)
		{
			if (MFlags&MFLAGS_SELECTION)
			{
				MFlags&=~MFLAGS_SELECTION;
				MFlags|=MFLAGS_EDITSELECTION;
			}

			if (MFlags&MFLAGS_NOSELECTION)
			{
				MFlags&=~MFLAGS_NOSELECTION;
				MFlags|=MFLAGS_EDITNOSELECTION;
			}
		}
		int Key=KeyNameToKey(strKey);
		m_Macros[Area].addItem(MacroRecord(Area,MFlags,Key,strKey,strSequence,strDescription));
	}

	return ErrorCount?false:true;
}

void KeyMacro::WriteMacro(void)
{
	MacroCfg->BeginTransaction();
	for(size_t ii=MACRO_OTHER;ii<MACRO_LAST;++ii)
	{
		for(size_t jj=0;jj<m_Macros[ii].getSize();++jj)
		{
			MacroRecord& rec=*m_Macros[ii].getItem(jj);
			if (rec.IsSave())
			{
				rec.ClearSave();
				string Code = rec.Code();
				RemoveExternalSpaces(Code);
				if (Code.IsEmpty())
					MacroCfg->DeleteKeyMacro(GetAreaName(rec.Area()), rec.Name());
				else
					MacroCfg->SetKeyMacro(GetAreaName(rec.Area()),rec.Name(),FlagsToString(rec.Flags()),Code,rec.Description());
			}
		}
	}
	MacroCfg->EndTransaction();
}

const wchar_t *eStackAsString(int)
{
	const wchar_t *s=__varTextDate.toString();
	return !s?L"":s;
}

static BOOL CheckEditSelected(MACROMODEAREA Mode, UINT64 CurFlags)
{
	if (Mode==MACRO_EDITOR || Mode==MACRO_DIALOG || Mode==MACRO_VIEWER || (Mode==MACRO_SHELL&&CtrlObject->CmdLine->IsVisible()))
	{
		int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS));
		Frame* CurFrame=FrameManager->GetCurrentFrame();

		if (CurFrame && CurFrame->GetType()==NeedType)
		{
			int CurSelected;

			if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
			{
				intptr_t SelStart,SelEnd;
				CtrlObject->CmdLine->GetSelection(SelStart,SelEnd);
				CurSelected = (SelStart != -1 && SelStart < SelEnd);
			}
			else
				CurSelected = FALSE;

			if (((CurFlags&MFLAGS_EDITSELECTION) && !CurSelected) ||	((CurFlags&MFLAGS_EDITNOSELECTION) && CurSelected))
				return FALSE;
		}
	}

	return TRUE;
}

static BOOL CheckInsidePlugin(UINT64 CurFlags)
{
	if (CtrlObject && CtrlObject->Plugins->CurPluginItem && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS)) // ?????
		//if(CtrlObject && CtrlObject->Plugins->CurEditor && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS))
		return FALSE;

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

static BOOL CheckAll (MACROMODEAREA Mode, UINT64 CurFlags)
{
	/* $TODO:
		����� ������ Check*() ����������� ������� IfCondition()
		��� ���������� �������������� ����.
	*/
	if (!CheckInsidePlugin(CurFlags))
		return FALSE;

	// �������� �� �����/�� ����� � ���.������ (� � ���������? :-)
	if (CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
		if (CtrlObject->CmdLine && !CheckCmdLine(CtrlObject->CmdLine->GetLength(),CurFlags))
			return FALSE;

	FilePanels *Cp=CtrlObject->Cp();

	if (!Cp)
		return FALSE;

	// �������� ������ � ���� �����
	Panel *ActivePanel=Cp->ActivePanel;
	Panel *PassivePanel=Cp->GetAnotherPanel(Cp->ActivePanel);

	if (ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACRO_SHELL)
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
			if (Mode!=MACRO_EDITOR && Mode != MACRO_DIALOG && Mode!=MACRO_VIEWER)
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

intptr_t WINAPI KeyMacro::ParamMacroDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	static DlgParam *KMParam=nullptr;

	switch (Msg)
	{
		case DN_INITDIALOG:
			KMParam=(DlgParam *)Param2;
			break;
		case DN_BTNCLICK:

			if (Param1==MS_CHECKBOX_A_PANEL || Param1==MS_CHECKBOX_P_PANEL)
				for (int i=1; i<=3; i++)
					SendDlgMessage(hDlg,DM_ENABLE,Param1+i,Param2);

			break;
		case DN_CLOSE:

			if (Param1==MS_BUTTON_OK)
			{
				LPCWSTR Sequence=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_SEQUENCE,0);
				if (*Sequence)
				{
					KeyMacro *Macro=KMParam->Handle;
					if (Macro->ParseMacroString(Sequence))
					{
						Macro->m_RecCode=Sequence;
						Macro->m_RecDescription=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_DESCR,0);
						return TRUE;
					}
				}

				return FALSE;
			}

			break;

		default:
			break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
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

		{DI_TEXT,3,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(MMacroSettingsEnableOutput)},
		{DI_CHECKBOX,5,8,0,8,0,nullptr,nullptr,0,MSG(MMacroSettingsRunAfterStart)},
		{DI_TEXT,3,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsActivePanel)},
		{DI_CHECKBOX,7,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,7,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,7,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_CHECKBOX,37,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsPassivePanel)},
		{DI_CHECKBOX,39,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,39,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,39,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_TEXT,3,14,0,14,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,15,0,15,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsCommandLine)},
		{DI_CHECKBOX,5,16,0,16,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsSelectionBlockPresent)},
		{DI_TEXT,3,17,0,17,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(MacroSettingsDlgData,MacroSettingsDlg);
	string strKeyText;
	KeyToText(Key,strKeyText);
	MacroSettingsDlg[MS_DOUBLEBOX].strData = LangString(MMacroSettingsTitle) << strKeyText;
	//if(!(Key&0x7F000000))
	//MacroSettingsDlg[3].Flags|=DIF_DISABLE;
	MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected=Flags&MFLAGS_DISABLEOUTPUT?0:1;
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

	MacroSettingsDlg[MS_EDIT_DESCR].strData=(Descr && *Descr)?Descr:(const wchar_t*)m_RecDescription;

	DlgParam Param={0, this, 0, 0, 0, false};
	Dialog Dlg(MacroSettingsDlg,ARRAYSIZE(MacroSettingsDlg),ParamMacroDlgProc,&Param);
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

	Flags=MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected?0:MFLAGS_DISABLEOUTPUT;
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

bool KeyMacro::ParseMacroString(const wchar_t *Sequence, bool onlyCheck)
{
	// ������������� ����� ��������� �� ������ �� ������, �.�. ������� Message()
	// �� ����� ����������� ������ � �������� ���������.
	FarMacroValue values[5]={{FMVT_INTEGER,{0}},{FMVT_STRING,{0}},{FMVT_BOOLEAN,{0}},{FMVT_STRING,{0}},{FMVT_STRING,{0}}};
	values[0].Integer=MCT_MACROPARSE;
	values[1].String=Sequence;
	values[2].Integer=onlyCheck?1:0;
	values[3].String=MSG(MMacroPErrorTitle);
	values[4].String=MSG(MOk);
	OpenMacroInfo info={sizeof(OpenMacroInfo),ARRAYSIZE(values),values};

	MacroPluginReturn* mpr = (MacroPluginReturn*)CallMacroPlugin(OPEN_LUAMACRO,&info);
	bool IsOK = mpr && mpr->ReturnType==MPRT_NORMALFINISH;
	if (mpr && !IsOK && !onlyCheck)
	{
		FrameManager->RefreshFrame(); // ����� ����� ������ ��������� ��������. ����� ������ �� ����������������.
	}
	return IsOK;
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
static bool editorgetstrFunc(FarMacroCall*);
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
static bool ReadVarsConsts(FarMacroCall*);

int PassString (const wchar_t* str, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_STRING;
		val.String = str;
		Data->Callback(Data->CallbackData, &val);
	}
	return 1;
}

int PassNumber (double dbl, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_DOUBLE;
		val.Double = dbl;
		Data->Callback(Data->CallbackData, &val);
	}
	return 1;
}

int PassInteger (__int64 Int, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_INTEGER;
		val.Integer = Int;
		Data->Callback(Data->CallbackData, &val);
	}
	return 1;
}

int PassBoolean (int b, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_BOOLEAN;
		val.Integer = b;
		Data->Callback(Data->CallbackData, &val);
	}
	return 1;
}

int PassValue (TVar* Var, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;

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
		else
		{
			val.Type = FMVT_INTEGER;
			val.Integer = Var->i();
		}

		Data->Callback(Data->CallbackData, &val);
	}
	return 1;
}

static void __parseParams(int Count, TVar* Params, FarMacroCall* Data)
{
	int argNum = (Data->ArgNum > Count) ? Count : Data->ArgNum;

	while (argNum < Count)
		Params[--Count].SetType(vtUnknown);

	for (int i=0; i<argNum; i++)
	{
		switch (Data->Args[i].Type)
		{
			case FMVT_INTEGER:
			case FMVT_BOOLEAN:
				Params[i] = Data->Args[i].Integer; break;
			case FMVT_DOUBLE:
				Params[i] = Data->Args[i].Double; break;
			case FMVT_STRING:
				Params[i] = Data->Args[i].String; break;
			default:
				Params[i].SetType(vtUnknown); break;
		}
	}
}
#define parseParams(c,v,d) TVar v[c]; __parseParams(c,v,d)

intptr_t KeyMacro::CallFar(intptr_t CheckCode, FarMacroCall* Data)
{
	intptr_t ret=0;
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

	// �������� �� �������
	if (CheckCode >= MACRO_OTHER && CheckCode < MACRO_LAST) //FIXME: CheckCode range
	{
		return PassBoolean (WaitInMainLoop ?
			CheckCode == FrameManager->GetCurrentFrame()->GetMacroMode() :
		  CheckCode == CtrlObject->Macro.GetMode(), Data);
	}

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	Frame* CurFrame=FrameManager->GetCurrentFrame();

	switch (CheckCode)
	{
		case MCODE_V_FAR_WIDTH:
			return (ScrX+1);

		case MCODE_V_FAR_HEIGHT:
			return (ScrY+1);

		case MCODE_V_FAR_TITLE:
			Console.GetTitle(strFileName);
			return PassString(strFileName, Data);

		case MCODE_V_FAR_PID:
			return PassNumber(GetCurrentProcessId(), Data);

		case MCODE_V_FAR_UPTIME:
		{
			LARGE_INTEGER Frequency, Counter;
			QueryPerformanceFrequency(&Frequency);
			QueryPerformanceCounter(&Counter);
			return PassNumber(((Counter.QuadPart-FarUpTime.QuadPart)*1000)/Frequency.QuadPart, Data);
		}

		case MCODE_V_MACRO_AREA:
			return PassString(GetAreaName(CtrlObject->Macro.GetMode()), Data);

		case MCODE_C_FULLSCREENMODE: // Fullscreen?
			return PassBoolean(IsConsoleFullscreen(), Data);

		case MCODE_C_ISUSERADMIN: // IsUserAdmin?
			return PassBoolean(Opt.IsUserAdmin, Data);

		case MCODE_V_DRVSHOWPOS: // Drv.ShowPos
			return Macro_DskShowPosType;

		case MCODE_V_DRVSHOWMODE: // Drv.ShowMode
			return Opt.ChangeDriveMode;

		case MCODE_C_CMDLINE_BOF:              // CmdLine.Bof - ������ � ������ cmd-������ ��������������?
		case MCODE_C_CMDLINE_EOF:              // CmdLine.Eof - ������ � ������ cmd-������ ��������������?
		case MCODE_C_CMDLINE_EMPTY:            // CmdLine.Empty
		case MCODE_C_CMDLINE_SELECTED:         // CmdLine.Selected
		{
			return PassBoolean(CtrlObject->CmdLine && CtrlObject->CmdLine->VMProcess(CheckCode), Data);
		}

		case MCODE_V_CMDLINE_ITEMCOUNT:        // CmdLine.ItemCount
		case MCODE_V_CMDLINE_CURPOS:           // CmdLine.CurPos
		{
			return CtrlObject->CmdLine?CtrlObject->CmdLine->VMProcess(CheckCode):-1;
		}

		case MCODE_V_CMDLINE_VALUE:            // CmdLine.Value
		{
			if (CtrlObject->CmdLine)
				CtrlObject->CmdLine->GetString(strFileName);
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
			int NeedType = m_Mode == MACRO_EDITOR? MODALTYPE_EDITOR : (m_Mode == MACRO_VIEWER? MODALTYPE_VIEWER : (m_Mode == MACRO_DIALOG? MODALTYPE_DIALOG : MODALTYPE_PANELS));

			if (!(m_Mode == MACRO_USERMENU || m_Mode == MACRO_MAINMENU || m_Mode == MACRO_MENU) && CurFrame && CurFrame->GetType()==NeedType)
			{
				int CurSelected;

				if (m_Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
					CurSelected=(int)CtrlObject->CmdLine->VMProcess(CheckCode);
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
			int CurMMode=CtrlObject->Macro.GetMode();

			if (!(m_Mode == MACRO_USERMENU || m_Mode == MACRO_MAINMENU || m_Mode == MACRO_MENU) && CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACRO_INFOPANEL || CurMMode == MACRO_QVIEWPANEL || CurMMode == MACRO_TREEPANEL))
			{
				if (CheckCode == MCODE_C_EMPTY)
					ret=CtrlObject->CmdLine->GetLength()?0:1;
				else
					ret=CtrlObject->CmdLine->VMProcess(CheckCode);
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
		{
			if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? m_Mode == MACRO_DIALOG ??
				return CurFrame->VMProcess(CheckCode);
			return 0;
		}

		case MCODE_V_DLGINFOID:        // Dlg.Info.Id
		case MCODE_V_DLGINFOOWNER:     // Dlg.Info.Owner
		{
			if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
			{
				return PassString(reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(CurFrame->VMProcess(CheckCode))), Data);
			}
			return PassString(L"", Data);
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
				int GetFileCount=SelPanel->GetFileCount();
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
			return PassBoolean(SelPanel && SelPanel==CtrlObject->Cp()->LeftPanel, Data);
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
					ptr = strFileName.CPtr();
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
					SelPanel->GetCurDir(strFileName);
				ptr = strFileName.CPtr();
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
					strFileName = Info.CurDir;
				}
				else
					SelPanel->GetCurDir(strFileName);

				DeleteEndSlash(strFileName); // - ����� � ����� ����� ���� C:, ����� ����� ������ ���: APanel.Path + "\\file"
				ptr = strFileName.CPtr();
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
				ptr = strFileName.CPtr();
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
				SelPanel->GetCurDir(strFileName);
				GetPathRoot(strFileName, strFileName);
				UINT DriveType=FAR_GetDriveType(strFileName,nullptr,0);

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
				if (CtrlObject->Cp() == f)
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

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
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
								ptr=strFileName.CPtr();
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
		case MCODE_V_EDITORVALUE:   // Editor.Value
		case MCODE_V_EDITORSELVALUE: // Editor.SelValue
		{
			const wchar_t* ptr = nullptr;
			if (CheckCode == MCODE_V_EDITORVALUE || CheckCode == MCODE_V_EDITORSELVALUE)
				ptr=L"";

			if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
			{
				if (CheckCode == MCODE_V_EDITORFILENAME)
				{
					string strType;
					CtrlObject->Plugins->CurEditor->GetTypeAndName(strType, strFileName);
					ptr=strFileName.CPtr();
				}
				else if (CheckCode == MCODE_V_EDITORVALUE)
				{
					EditorGetString egs={sizeof(EditorGetString)};
					egs.StringNumber=-1;
					CtrlObject->Plugins->CurEditor->EditorControl(ECTL_GETSTRING,0,&egs);
					ptr=egs.StringText;
				}
				else if (CheckCode == MCODE_V_EDITORSELVALUE)
				{
					CtrlObject->Plugins->CurEditor->VMProcess(CheckCode,&strFileName);
					ptr=strFileName.CPtr();
				}
				else
					return CtrlObject->Plugins->CurEditor->VMProcess(CheckCode);
			}

			return ptr ? PassString(ptr, Data) : 0;
		}

		case MCODE_V_HELPFILENAME:  // Help.FileName
		case MCODE_V_HELPTOPIC:     // Help.Topic
		case MCODE_V_HELPSELTOPIC:  // Help.SelTopic
		{
			const wchar_t* ptr=L"";
			if (CtrlObject->Macro.GetMode() == MACRO_HELP)
			{
				CurFrame->VMProcess(CheckCode,&strFileName,0);
				ptr=strFileName.CPtr();
			}
			return PassString(ptr, Data);
		}

		case MCODE_V_VIEWERFILENAME: // Viewer.FileName
		case MCODE_V_VIEWERSTATE: // Viewer.State
		{
			if ((CtrlObject->Macro.GetMode()==MACRO_VIEWER || CtrlObject->Macro.GetMode()==MACRO_QVIEWPANEL) &&
							CtrlObject->Plugins->CurViewer && CtrlObject->Plugins->CurViewer->IsVisible())
			{
				if (CheckCode == MCODE_V_VIEWERFILENAME)
				{
					CtrlObject->Plugins->CurViewer->GetFileName(strFileName);//GetTypeAndName(nullptr,FileName);
					return PassString(strFileName, Data);
				}
				else
					return PassNumber(CtrlObject->Plugins->CurViewer->VMProcess(MCODE_V_VIEWERSTATE), Data);
			}

			return (CheckCode == MCODE_V_VIEWERFILENAME) ? PassString(L"", Data) : 0;
		}

		case MCODE_OP_JMP:  return PassNumber(msValues[constMsX], Data);
		case MCODE_OP_JZ:   return PassNumber(msValues[constMsY], Data);
		case MCODE_OP_JNZ:  return PassNumber(msValues[constMsButton], Data);
		case MCODE_OP_JLT:  return PassNumber(msValues[constMsCtrlState], Data);
		case MCODE_OP_JLE:  return PassNumber(msValues[constMsEventFlags], Data);

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
		case MCODE_F_EDITOR_GETSTR:   return editorgetstrFunc(Data);
		case MCODE_F_EDITOR_INSSTR:   return editorinsstrFunc(Data);
		case MCODE_F_EDITOR_POS:      return editorposFunc(Data);
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
		case MCODE_F_WAITKEY:         return waitkeyFunc(Data);
		case MCODE_F_WINDOW_SCROLL:   return windowscrollFunc(Data);
		case MCODE_F_XLAT:            return xlatFunc(Data);
		case MCODE_F_PROMPT:          return promptFunc(Data);
		case MCODE_OP_JGE:            return ReadVarsConsts(Data);

		case MCODE_OP_JGT: // ��������� ���� ������� ��� Eval(S,2).
		{
			parseParams(1,Params,Data);
			TVar& Val(Params[0]);

			if (!(Val.isInteger() && !Val.i())) // ��������� ������ ���������� ���������� ������ ����������
			{
				// ����������� ����� �������, ����������� �� ���������������
				/*
					 ��� �����:
					 �) ������ �������� ������� ���������� � 2
					 �) ������ ���������� ������� ������ � ������� "Area/Key"
							�����:
								"Area" - �������, �� ������� ����� ������� ������
								"/" - �����������
								"Key" - �������� �������
							"Area/" ����� �� ���������, � ���� ������ ����� "Key" ����� ������� � ������� �������� ������������,
								 ���� � ������� ������� "Key" �� ������, �� ����� ����������� � ������� Common.
								 ��� �� ��������� ����� � ������� Common (����������� ������ "����" ��������),
								 ���������� � �������� "Area" ������� �����.

					 ��� ������ 2 ������� ������
						 -1 - ������
						 -2 - ��� �������, ��������� ���������������� (��� ������ ������������)
							0 - Ok
				*/
				int _Mode;
				bool UseCommon=true;
				string strVal=Val.toString();
				strVal=RemoveExternalSpaces(strVal);

				wchar_t *lpwszVal = strVal.GetBuffer();
				wchar_t *p=wcsrchr(lpwszVal,L'/');

				if (p  && p[1])
				{
					*p++=0;
					if ((_Mode = GetAreaCode(lpwszVal)) < 0)
					{
						_Mode=GetMode();
						if (lpwszVal[0] == L'.' && !lpwszVal[1]) // ������� "./Key" �� ������������� ����� � Common`�
							UseCommon=false;
					}
					else
						UseCommon=false;
				}
				else
				{
					p=lpwszVal;
					_Mode=GetMode();
				}

				string strKeyName=p;
				int Area;
				int I=GetIndex(&Area,0,strKeyName,_Mode,UseCommon);
				if (I != -1)
				{
					MacroRecord* macro = m_Macros[Area].getItem(I);
					if (!(macro->Flags() & MFLAGS_DISABLEMACRO))
					{
						PassString(macro->Code(), Data);
						return 1;
					}
				}
			}
			PassBoolean(0, Data);
			return 0;
		}

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

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				__int64 Result=0;

				if (f)
				{
					__int64 MenuItemPos=tmpVar.i()-1;
					if (CheckCode == MCODE_F_MENU_GETHOTKEY)
					{
						if ((Result=f->VMProcess(CheckCode,nullptr,MenuItemPos)) )
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
							tmpVar=NewStr.CPtr();
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

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
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
					Result=f->VMProcess(CheckCode,(void*)Params[0].toString(),tmpMode);
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

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
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
						if (f->VMProcess(CheckCode,(void*)&NewStr,tmpAction.toInteger()))
						{
							tmpVar=NewStr.CPtr();
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

		case MCODE_F_PLUGIN_MENU:   // N=Plugin.Menu(Guid[,MenuGuid])
		case MCODE_F_PLUGIN_CONFIG: // N=Plugin.Config(Guid[,MenuGuid])
		case MCODE_F_PLUGIN_COMMAND: // N=Plugin.Command(Guid[,Command])
		{
			int Ret=0;
			parseParams(2,Params,Data);
			TVar& Arg = (Params[1]);
			TVar& Guid = (Params[0]);
			GUID guid, menuGuid;
			CallPluginInfo cpInfo={CPT_CHECKONLY};
			wchar_t EmptyStr[1]={};
			bool ItemFailed=false;


			switch (CheckCode)
			{
				case MCODE_F_PLUGIN_MENU:
					cpInfo.CallFlags |= CPT_MENU;
					if (!Arg.isUnknown())
					{
						if (StrToGuid(Arg.s(),menuGuid))
							cpInfo.ItemGuid=&menuGuid;
						else
							ItemFailed=true;
					}
					break;
				case MCODE_F_PLUGIN_CONFIG:
					cpInfo.CallFlags |= CPT_CONFIGURE;
					if (!Arg.isUnknown())
					{
						if (StrToGuid(Arg.s(),menuGuid))
							cpInfo.ItemGuid=&menuGuid;
						else
							ItemFailed=true;
					}
					break;
				case MCODE_F_PLUGIN_COMMAND:
					cpInfo.CallFlags |= CPT_CMDLINE;
					if (Arg.isString())
						cpInfo.Command=Arg.s();
					else
						cpInfo.Command=EmptyStr;
					break;
			}

			if (!ItemFailed && StrToGuid(Guid.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))
			{
				// ����� ������� ��������� "����������" ����� ��������� ������� �������/������
				Ret=CtrlObject->Plugins->CallPluginItem(guid,&cpInfo);
				PassBoolean(Ret!=0,Data);

				if (Ret)
				{
					// ���� ����� ������� - �� ������ ����������
					cpInfo.CallFlags&=~CPT_CHECKONLY;
					CtrlObject->Plugins->CallPluginItem(guid,&cpInfo);
#if 0 //FIXME
					if (MR != Work.MacroWORK)
						MR=Work.MacroWORK;
#endif
				}
			}
			else
			{
				PassBoolean(0,Data);
			}

			// �� �������� � KEY_F11
			FrameManager->RefreshFrame();

#if 0 //FIXME
			if (Work.Executing == MACROMODE_NOMACRO)
				goto return_func;

			goto begin;
#else
			return 0;
#endif
		}

		case MCODE_F_AKEY:                // V=akey(Mode[,Type])
		{
			parseParams(2,Params,Data);
			int tmpType=(int)Params[1].getInteger();
			int tmpMode=(int)Params[0].getInteger();

			MacroRecord* MR=GetCurMacro();
			DWORD aKey=MR->Key();

			if (!tmpType)
			{
				if (!(MR->Flags()&MFLAGS_POSTFROMPLUGIN))
				{
					INPUT_RECORD *inRec=&m_CurState.cRec;
					if (!inRec->EventType)
						inRec->EventType = KEY_EVENT;
					if(inRec->EventType == MOUSE_EVENT || inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
						aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
				}
				else if (!aKey)
					aKey=KEY_NONE;
			}

			if (!tmpMode)
				return (__int64)aKey;
			else
			{
				string value;
				KeyToText(aKey,value);
				PassString(value,Data);
				return 1;
			}
		}

		case MCODE_F_HISTIORY_DISABLE: // N=History.Disable([State])
		{
			parseParams(1,Params,Data);
			TVar State(Params[0]);

			DWORD oldHistoryDisable=m_CurState.HistoryDisable;

			if (!State.isUnknown())
				m_CurState.HistoryDisable=(DWORD)State.getInteger();

			return (__int64)oldHistoryDisable;
		}

		case MCODE_F_MMODE:               // N=MMode(Action[,Value])
		{
			parseParams(2,Params,Data);
			__int64 nValue = Params[1].getInteger();
			TVar& Action(Params[0]);

			MacroRecord* MR=GetCurMacro();
			__int64 Result=0;

			switch (Action.getInteger())
			{
				case 1: // DisableOutput
				{
					Result = (MR->Flags()&MFLAGS_DISABLEOUTPUT) ? 1:0;

					if (Result && (nValue==0 || nValue==2))
					{
						ScrBuf.Unlock();
						ScrBuf.Flush();
						MR->m_flags&=~MFLAGS_DISABLEOUTPUT;
					}
					else if (!Result && (nValue==1 || nValue==2))
					{
						ScrBuf.Lock();
						MR->m_flags|=MFLAGS_DISABLEOUTPUT;
					}

					return PassNumber(Result, Data);
				}

				case 2: // Get MacroRecord Flags
				{
					Result = MR->Flags();
					Result = (Result<<8) | (MR->Area()==MACRO_COMMON ? 0xFF : MR->Area());
					return PassNumber(Result, Data);
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
	wchar_t *p = (wchar_t *)Params[0].toString();
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
	wchar_t *p = (wchar_t *)Params[0].toString();
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
			strDest.SetLength(p-s);
		}
	}
	else
	{
		if (*(p+1) == L':')
		{
			p += 2;

			if ((nFlags & FLAG_DISK) == FLAG_DISK)
			{
				size_t Length=strDest.GetLength()+p-s;
				strDest+=s;
				strDest.SetLength(Length);
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
			size_t Length=strDest.GetLength()+e-s;
			strDest+=s;
			strDest.SetLength(Length);
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

	if (!strDest.IsEmpty())
		AddEndSlash(strDest);

	if (nFlags & FLAG_NAME)
	{
		const wchar_t *ptr=wcspbrk(s,L":");

		if (ptr)
			s=ptr+1;

		size_t Length=strDest.GetLength()+e-s;
		strDest+=s;
		strDest.SetLength(Length);
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
		strPath.Clear();
	else
		Ret=true;

	PassString(strPath.CPtr(), Data);
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

	if (Opt.WindowMode)
	{
		int Lines=(int)Params[0].i(), Columns=0;
		if (Params[1].i())
		{
			Columns=Lines;
			Lines=0;
		}

		if (Console.ScrollWindow(Lines, Columns))
		{
			Ret=true;
			L=1;
		}
	}

	PassNumber(L, Data);
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
	return f?true:false;
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

	PassString(strKeyText.CPtr(), Data);
	return !strKeyText.IsEmpty()?true:false;
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
				strKeyText.Clear();

		PassString(strKeyText.CPtr(), Data);
		return !strKeyText.IsEmpty()?true:false;
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
	bool Ret= i ? true : false;
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
	bool Ret= i ? true : false;
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
		strTStr.Clear();

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
	wchar_t *Str = (wchar_t *)Params[0].toString();
	bool Ret=::Xlat(Str,0,StrLength(Str),Params[1].i())?true:false;
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
	bool Ret=MessageBeep((UINT)Params[0].i())?true:false;

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

	PassNumber(Ret?1:0, Data);
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
	HKL  Layout=0, RetLayout=0;

	wchar_t LayoutName[1024]={}; // BUGBUG!!!
	if (ifn.GetConsoleKeyboardLayoutNameW(LayoutName))
	{
		wchar_t *endptr;
		DWORD res=wcstoul(LayoutName, &endptr, 16);
		RetLayout=(HKL)(intptr_t)(HIWORD(res)? res : MAKELONG(res,res));
	}

	HWND hWnd = Console.GetWindow();

	if (hWnd && dwLayout)
	{
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

	PassNumber(Ret?static_cast<INT64>(reinterpret_cast<intptr_t>(RetLayout)):0, Data);

	return Ret?true:false;
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

	DWORD oldHistoryDisable=CtrlObject->Macro.GetHistoryDisableMask();

	if (!(history && *history)) // Mantis#0001743: ����������� ���������� �������
		CtrlObject->Macro.SetHistoryDisableMask(8); // ���� �� ������ history, �� ������������� ��������� ������� ��� ����� prompt()

	if (GetString(title,prompt,history,src,strDest,nullptr,(Flags&~FIB_CHECKBOX)|FIB_ENABLEEMPTY,nullptr,nullptr))
	{
		PassString(strDest,Data);
		Ret=true;
	}
	else
		PassBoolean(0,Data);

	CtrlObject->Macro.SetHistoryDisableMask(oldHistoryDisable);

	return Ret;
}

// N=msgbox(["Title"[,"Text"[,flags]]])
static bool msgBoxFunc(FarMacroCall* Data)
{
//FIXME: has flags IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT
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
	int Result=pluginapi::apiMessageFn(&FarGuid,&FarGuid,Flags,nullptr,(const wchar_t * const *)TempBuf.CPtr(),0,0)+1;
	/*
	if (Result <= -1) // Break?
		CtrlObject->Macro.SendDropProcess();
	*/
	PassNumber(Result, Data);
	return true;
}


static intptr_t WINAPI CompareItems(const MenuItemEx **el1, const MenuItemEx **el2, const SortItemParam *Param)
{
	if (((*el1)->Flags & LIF_SEPARATOR) || ((*el2)->Flags & LIF_SEPARATOR))
		return 0;

	string strName1((*el1)->strName);
	string strName2((*el2)->strName);
	RemoveChar(strName1,L'&',true);
	RemoveChar(strName2,L'&',true);
	int Res = NumStrCmpI(strName1.CPtr()+Param->Offset,strName2.CPtr()+Param->Offset);
	return (Param->Direction?(Res<0?1:(Res>0?-1:0)):Res);
}

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

	if (!strItems.IsSubStrAt(strItems.GetLength()-1,L"\n"))
		strItems.Append(L"\n");

	TVar Result = -1;
	int BoxType = (Flags & 0x7)?(Flags & 0x7)-1:3;
	bool bResultAsIndex = (Flags & 0x08)?true:false;
	bool bMultiSelect = (Flags & 0x010)?true:false;
	bool bSorting = (Flags & 0x20)?true:false;
	bool bPacking = (Flags & 0x40)?true:false;
	bool bAutohighlight = (Flags & 0x80)?true:false;
	bool bSetMenuFilter = (Flags & 0x100)?true:false;
	bool bAutoNumbering = (Flags & 0x200)?true:false;
	bool bExitAfterNavigate = (Flags & 0x400)?true:false;
	int nLeftShift=bAutoNumbering?9:0;
	int X = -1;
	int Y = -1;
	unsigned __int64 MenuFlags = VMENU_WRAPMODE;

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
	size_t SubstrLen;
	ReplaceStrings(strTitle,L"\r\n",L"\n");
	bool CRFound=strTitle.Pos(PosLF, L"\n");

	if(CRFound)
	{
		strBottom=strTitle.SubStr(PosLF+1);
		strTitle=strTitle.SubStr(0,PosLF);
	}
	VMenu Menu(strTitle.CPtr(),nullptr,0,ScrY-4);
	Menu.SetBottomTitle(strBottom.CPtr());
	Menu.SetFlags(MenuFlags);
	Menu.SetPosition(X,Y,0,0);
	Menu.SetBoxType(BoxType);

	CRFound=strItems.Pos(PosLF, L"\n");
	while(CRFound)
	{
		MenuItemEx NewItem;
		NewItem.Clear();
		SubstrLen=PosLF-CurrentPos;

		if (SubstrLen==0)
			SubstrLen=1;

		NewItem.strName=strItems.SubStr(CurrentPos,SubstrLen);

		if (NewItem.strName!=L"\n")
		{
		wchar_t *CurrentChar=(wchar_t *)NewItem.strName.CPtr();
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
			NewItem.strName.Clear();

		if (bAutoNumbering && !(bSorting || bPacking) && !(NewItem.Flags & LIF_SEPARATOR))
		{
			LineCount++;
			NewItem.strName.Format(L"%6d - %s", LineCount, NewItem.strName.CPtr());
		}
		Menu.AddItem(&NewItem);
		CurrentPos=PosLF+1;
		CRFound=strItems.Pos(PosLF, L"\n",CurrentPos);
	}

	if (bSorting)
		Menu.SortItems(reinterpret_cast<TMENUITEMEXCMPFUNC>(CompareItems));

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
				Item->strName.Format(L"%6d - %s", LineCount, Item->strName.CPtr());
			}
		}
	}

	if (!VFindOrFilter.isUnknown())
	{
		if (bSetMenuFilter)
		{
			Menu.SetFilterEnabled(true);
			Menu.SetFilterString(VFindOrFilter.toString());
			Menu.FilterStringUpdated(true);
			Menu.Show();
		}
		else
		{
			if (VFindOrFilter.isInteger())
			{
				if (VFindOrFilter.toInteger()-1>=0)
					Menu.SetSelectPos(VFindOrFilter.toInteger()-1,1);
				else
					Menu.SetSelectPos(Menu.GetItemCount()+VFindOrFilter.toInteger(),1);
			}
			else
				if (VFindOrFilter.isString())
					Menu.SetSelectPos(Max(0,Menu.FindItem(0, VFindOrFilter.toString())),1);
		}
	}

	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()) )
		frame->Lock();

	Menu.Show();
	int PrevSelectedPos=Menu.GetSelectPos();
	DWORD Key=0;
	int RealPos;
	bool CheckFlag;
	int X1, Y1, X2, Y2, NewY2;
	while (!Menu.Done() && !CloseFARMenu)
	{
		SelectedPos=Menu.GetSelectPos();
		Key=Menu.ReadInput();
		switch (Key)
		{
			case KEY_NUMPAD0:
			case KEY_INS:
				if (bMultiSelect)
				{
					Menu.SetCheck(!Menu.GetCheck(SelectedPos));
					Menu.Show();
				}
				break;

			case KEY_CTRLADD:
			case KEY_CTRLSUBTRACT:
			case KEY_CTRLMULTIPLY:
			case KEY_RCTRLADD:
			case KEY_RCTRLSUBTRACT:
			case KEY_RCTRLMULTIPLY:
				if (bMultiSelect)
				{
					for(int i=0; i<Menu.GetShowItemCount(); i++)
					{
						RealPos=Menu.VisualPosToReal(i);
						if (Key==KEY_CTRLMULTIPLY || Key==KEY_RCTRLMULTIPLY)
						{
							CheckFlag=Menu.GetCheck(RealPos)?false:true;
						}
						else
						{
							CheckFlag=(Key==KEY_CTRLADD || Key==KEY_RCTRLADD);
						}
						Menu.SetCheck(CheckFlag, RealPos);
					}
					Menu.Show();
				}
				break;

			case KEY_CTRLA:
			case KEY_RCTRLA:
			{
				Menu.GetPosition(X1, Y1, X2, Y2);
				NewY2=Y1+Menu.GetShowItemCount()+1;

				if (NewY2>ScrY-2)
					NewY2=ScrY-2;

				Menu.SetPosition(X1,Y1,X2,NewY2);
				Menu.Show();
				break;
			}

			case KEY_BREAK:
				CtrlObject->Macro.SendDropProcess();
				Menu.SetExitCode(-1);
				break;

			default:
				Menu.ProcessInput();
				break;
		}

		if (bExitAfterNavigate && (PrevSelectedPos!=SelectedPos))
		{
			SelectedPos=Menu.GetSelectPos();
			break;
		}

		PrevSelectedPos=SelectedPos;
	}

	wchar_t temp[65];

	if (Menu.Modal::GetExitCode() >= 0)
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
						Result+=(*Menu.GetItemPtr(i)).strName.CPtr()+nLeftShift;
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
					Result=(*Menu.GetItemPtr(SelectedPos)).strName.CPtr()+nLeftShift;
			}
		}
		else
			if(!bResultAsIndex)
				Result=(*Menu.GetItemPtr(SelectedPos)).strName.CPtr()+nLeftShift;
			else
				Result=SelectedPos+1;
		Menu.Hide();
	}
	else
	{
		Menu.Hide();
		if (bExitAfterNavigate)
		{
			Result=SelectedPos+1;
			if ((Key == KEY_ESC) || (Key == KEY_F10) || (Key == KEY_BREAK))
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


	if (apiGetEnvironmentVariable(S.toString(), strEnv))
		Ret=true;
	else
		strEnv.Clear();

	if (Mode.i()) // Mode != 0: Set
	{
		SetEnvironmentVariable(S.toString(),Value.isUnknown() || !*Value.s()?nullptr:Value.toString());
	}

	PassString(strEnv.CPtr(), Data);
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

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

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
			ValItems=strStr.CPtr();
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
	return Result==-1?false:true;
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
		FAR_FIND_DATA_EX FindData;
		apiGetFindDataEx(Str.toString(), FindData);
		FileAttr=FindData.dwFileAttributes;
		Ret=true;
	}
	else // panel.fattr(1) & panel.fexist(3)
	{
		parseParams(2,Params,Data);
		TVar& S(Params[1]);
		int typePanel=(int)Params[0].getInteger();
		const wchar_t *Str = S.toString();
		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = !typePanel ? ActivePanel : (typePanel == 1?PassivePanel:nullptr);

		if (SelPanel)
		{
			if (wcspbrk(Str,L"*?") )
				Pos=SelPanel->FindFirst(Str);
			else
				Pos=SelPanel->FindFile(Str,wcspbrk(Str,L"\\/:")?FALSE:TRUE);

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
	__int64 Ret = -1;
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

	return Ret != 0;
}

// N=Dlg.SetFocus([ID])
static bool dlgsetfocusFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar Ret(-1);
	unsigned Index=(unsigned)Params[0].getInteger()-1;
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		Ret=(__int64)CurFrame->VMProcess(MCODE_V_DLGCURPOS);
		if ((int)Index >= 0)
		{
			if(!SendDlgMessage((HANDLE)CurFrame,DM_SETFOCUS,Index,0))
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
	bool result = GetConfigValue(Key.s(),Name.s(), strValue);
	result ? PassString(strValue,Data) : PassBoolean(0,Data);
	return result;
}

// V=Dlg.GetValue([Pos[,InfoID]])
static bool dlggetvalueFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar Ret(-1);
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		TVarType typeIndex=Params[0].type();
		unsigned Index=(unsigned)Params[0].getInteger()-1;
		if (typeIndex == vtUnknown || ((typeIndex==vtInteger || typeIndex==vtDouble) && (int)Index < -1))
			Index=((Dialog*)CurFrame)->GetDlgFocusPos();

		TVarType typeInfoID=Params[1].type();
		int InfoID=(int)Params[1].getInteger();
		if (typeInfoID == vtUnknown || (typeInfoID == vtInteger && InfoID < 0))
			InfoID=0;

		FarGetValue fgv={sizeof(FarGetValue),InfoID,FMVT_UNKNOWN};
		unsigned DlgItemCount=((Dialog*)CurFrame)->GetAllItemCount();
		const DialogItemEx **DlgItem=((Dialog*)CurFrame)->GetAllItem();
		bool CallDialog=true;

		if (Index == (unsigned)-1)
		{
			SMALL_RECT Rect;

			if (SendDlgMessage((HANDLE)CurFrame,DM_GETDLGRECT,0,&Rect))
			{
				switch (InfoID)
				{
					case 0: Ret=(__int64)DlgItemCount; break;
					case 2: Ret=(__int64)Rect.Left; break;
					case 3: Ret=(__int64)Rect.Top; break;
					case 4: Ret=(__int64)Rect.Right; break;
					case 5: Ret=(__int64)Rect.Bottom; break;
					case 6: Ret=(__int64)(((Dialog*)CurFrame)->GetDlgFocusPos()+1); break;
					default: Ret=0; Ret.SetType(vtUnknown); break;
				}
			}
		}
		else if (Index < DlgItemCount && DlgItem)
		{
			const DialogItemEx *Item=DlgItem[Index];
			FARDIALOGITEMTYPES ItemType=Item->Type;
			FARDIALOGITEMFLAGS ItemFlags=Item->Flags;

			if (!InfoID)
			{
				if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
				{
					InfoID=7;
				}
				else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
				{
					FarListGetItem ListItem={sizeof(FarListGetItem)};
					ListItem.ItemIndex=Item->ListPtr->GetSelectPos();

					if (SendDlgMessage(CurFrame,DM_LISTGETITEM,Index,&ListItem))
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
				case 1: Ret=(__int64)ItemType;    break;
				case 2: Ret=(__int64)Item->X1;    break;
				case 3: Ret=(__int64)Item->Y1;    break;
				case 4: Ret=(__int64)Item->X2;    break;
				case 5: Ret=(__int64)Item->Y2;    break;
				case 6: Ret=(__int64)((Item->Flags&DIF_FOCUS)!=0); break;
				case 7:
				{
					if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
					{
						Ret=(__int64)Item->Selected;
					}
					else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=(__int64)(Item->ListPtr->GetSelectPos()+1);
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
				case 9: Ret=(__int64)((Item->Flags&DIF_DEFAULTBUTTON)!=0); break;
				case 10:
				{
					Ret=Item->strData.CPtr();

					if (IsEdit(ItemType))
					{
						DlgEdit *EditPtr;

						if ((EditPtr = (DlgEdit *)(Item->ObjPtr)) )
							Ret=EditPtr->GetStringAddr();
					}

					break;
				}
				case 11:
				{
					if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=(__int64)(Item->ListPtr->GetItemCount());
					}
					break;
				}
			}
		}
		else if (Index >= DlgItemCount)
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

			if (SendDlgMessage((HANDLE)CurFrame,DN_GETVALUE,Index,&fgv))
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

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		EditorInfo ei={sizeof(EditorInfo)};
		CtrlObject->Plugins->CurEditor->EditorControl(ECTL_GETINFO,0,&ei);

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

				int Result=CtrlObject->Plugins->CurEditor->EditorControl(ECTL_SETPOSITION,0,&esp);

				if (Result)
					CtrlObject->Plugins->CurEditor->EditorControl(ECTL_REDRAW,0,nullptr);

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

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		long longState=-1L;

		if (Index != 12)
			longState=(long)Value.toInteger();
		else
		{
			if (Value.isString() || Value.i() != -1)
				longState=0;
		}

		EditorOptions EdOpt;
		CtrlObject->Plugins->CurEditor->GetEditorOptions(EdOpt);

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
			case 6:  // AnsiCodePageForNewFile;
				Ret=(__int64)EdOpt.AnsiCodePageForNewFile; break;
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
				case 6:  // AnsiCodePageForNewFile;
					EdOpt.AnsiCodePageForNewFile=longState != 0; break;
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

			CtrlObject->Plugins->CurEditor->SetEditorOptions(EdOpt);
			CtrlObject->Plugins->CurEditor->ShowStatus();
			if (Index == 0 || Index == 12 || Index == 14 || Index == 15 || Index == 20)
				CtrlObject->Plugins->CurEditor->Show();
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
			wchar_t *ClipText=PasteFromClipboard();

			if (ClipText)
			{
				PassString(ClipText, Data);
				xf_free(ClipText);
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
					clip.Empty();
				}
				clip.Close();
			}
			else
			{
				Ret=CopyToClipboard(Val.s());
			}

			PassNumber(Ret, Data); // 0!  ???
			return Ret?true:false;
		}
		case 2: // Add "S" into Clipboard
		{
			TVar varClip(Val.s());
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				wchar_t *CopyData=clip.Paste();

				if (CopyData)
				{
					size_t DataSize=StrLength(CopyData);
					wchar_t *NewPtr=(wchar_t *)xf_realloc(CopyData,(DataSize+StrLength(Val.s())+2)*sizeof(wchar_t));

					if (NewPtr)
					{
						CopyData=NewPtr;
						wcscpy(CopyData+DataSize,Val.s());
						varClip=CopyData;
						xf_free(CopyData);
					}
					else
					{
						xf_free(CopyData);
					}
				}

				Ret=clip.Copy(varClip.s());

				clip.Close();
			}

			PassNumber(Ret, Data); // 0!  ???
			return Ret?true:false;
		}
		case 3: // Copy Win to internal, "S" - ignore
		case 4: // Copy internal to Win, "S" - ignore
		{
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				Ret=clip.InternalCopy((cmdType-3)?true:false)?1:0;
				clip.Close();
			}

			PassNumber(Ret, Data); // 0!  ???
			return Ret?true:false;
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
			return Ret?true:false;
		}
	}

	return Ret?true:false;
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
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;
	__int64 Ret=0;

	if (SelPanel)
	{
		int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
		{
			long EndPos=SelPanel->GetFileCount();
			long I;
			long idxFoundItem=0;

			if (idxItem) // < 0 || > 0
			{
				EndPos--;
				if ( EndPos > 0 )
				{
					long StartPos;
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

					for ( I=StartPos ; ; I+=Direct )
					{
						if (Direct > 0)
						{
							if(I > EndPos)
								break;
						}
						else
						{
							if(I < EndPos)
								break;
						}

						if ( (!InSelection || (InSelection && SelPanel->IsSelected(I))) && SelPanel->FileInFilter(I) )
						{
							if (idxFoundItem == idxItem)
							{
								idxItem=I;
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
						//SelPanel->Show();
						// <Mantis#0000289> - ������, �� �� ������ :-)
						//ShellUpdatePanels(SelPanel);
						SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
						FrameManager->RefreshFrame(FrameManager->GetTopModal());
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
					for ( I=0 ; I < EndPos ; I++ )
					{
						if ( SelPanel->IsSelected(I) && SelPanel->FileInFilter(I) )
						{
							if (I == CurPos)
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
	return Ret?true:false;
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

		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

		if (SelPanel)
		{
			if (SelPanel->SetCurDir(pathName,SelPanel->GetMode()==PLUGIN_PANEL && IsAbsolutePath(pathName),FrameManager->GetCurrentFrame()->GetType() == MODALTYPE_PANELS))
			{
				ActivePanel=CtrlObject->Cp()->ActivePanel;
				PassivePanel=ActivePanel?CtrlObject->Cp()->GetAnotherPanel(ActivePanel):nullptr;
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

	PassNumber(Ret, Data);
	return Ret?true:false;
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

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

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
	return Ret?true:false;
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
	int lenF=(int)StrLength(Find.s());
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
		PassString(strStr.CPtr(), Data);
	}
	else
		PassValue(&Src, Data);

	return Ret?true:false;
}

// V=Panel.Item(typePanel,Index,TypeInfo)
static bool panelitemFunc(FarMacroCall* Data)
{
	parseParams(3,Params,Data);
	TVar& P2(Params[2]);
	TVar& P1(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	TVar Ret(0ll);
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

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
	FileListItem filelistItem;

	if (TypePanel == TREE_PANEL)
	{
		TreeItem treeItem;

		if (SelPanel->GetItem(Index,&treeItem) && !TypeInfo)
		{
			PassString(treeItem.strName, Data);
			return true;
		}
	}
	else
	{
		string strDate, strTime;

		if (TypeInfo == 11)
			SelPanel->ReadDiz();

		if (!SelPanel->GetItem(Index,&filelistItem))
			TypeInfo=-1;

		switch (TypeInfo)
		{
			case 0:  // Name
				Ret=TVar(filelistItem.strName);
				break;
			case 1:  // ShortName
				Ret=TVar(filelistItem.strShortName);
				break;
			case 2:  // FileAttr
				PassNumber((long)filelistItem.FileAttr, Data);
				return false;
			case 3:  // CreationTime
				ConvertDate(filelistItem.CreationTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 4:  // AccessTime
				ConvertDate(filelistItem.AccessTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 5:  // WriteTime
				ConvertDate(filelistItem.WriteTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 6:  // FileSize
				Ret=TVar((__int64)filelistItem.FileSize);
				break;
			case 7:  // AllocationSize
				Ret=TVar((__int64)filelistItem.AllocationSize);
				break;
			case 8:  // Selected
				PassNumber((DWORD)filelistItem.Selected, Data);
				return false;
			case 9:  // NumberOfLinks
				PassNumber(filelistItem.NumberOfLinks, Data);
				return false;
			case 10:  // SortGroup
				PassNumber(filelistItem.SortGroup, Data);
				return false;
			case 11:  // DizText
				Ret=TVar((const wchar_t *)filelistItem.DizText);
				break;
			case 12:  // Owner
				Ret=TVar(filelistItem.strOwner);
				break;
			case 13:  // CRC32
				PassNumber(filelistItem.CRC32, Data);
				return false;
			case 14:  // Position
				PassNumber(filelistItem.Position, Data);
				return false;
			case 15:  // CreationTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.CreationTime));
				break;
			case 16:  // AccessTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.AccessTime));
				break;
			case 17:  // WriteTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.WriteTime));
				break;
			case 18: // NumberOfStreams
				PassNumber(filelistItem.NumberOfStreams, Data);
				return false;
			case 19: // StreamsSize
				Ret=TVar((__int64)filelistItem.StreamsSize);
				break;
			case 20:  // ChangeTime
				ConvertDate(filelistItem.ChangeTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 21:  // ChangeTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.ChangeTime));
				break;
			case 22:  // CustomData
				Ret=TVar(filelistItem.strCustomData);
				break;
			case 23:  // ReparseTag
			{
				PassNumber(filelistItem.ReparseTag, Data);
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
	StrUpper((wchar_t *)Val.toString());
	PassValue(&Val, Data);
	return true;
}

static bool lcaseFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& Val(Params[0]);
	StrLower((wchar_t *)Val.toString());
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
			wchar_t *NewFill=new wchar_t[FineLength+1];
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

				string strPad=NewFill;
				strPad.SetLength(CntL);
				strPad+=strDest;
				strPad.Append(NewFill, CntR);
				strDest=strPad;

				delete[] NewFill;
			}
		}
	}

	PassString(strDest.CPtr(), Data);
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
	PassString(strDest.CPtr(), Data);
	return true;
}

static bool intFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar& Val(Params[0]);
	Val.toInteger();
	PassValue(&Val, Data);
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
	CFileMask FileMask;

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

	int Mode=CtrlObject->Macro.GetMode();
	int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS)); // MACRO_SHELL?
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CurFrame && CurFrame->GetType()==NeedType)
	{
		if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
			Ret=CtrlObject->CmdLine->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.i());
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

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		EditorUndoRedo eur={sizeof(EditorUndoRedo)};
		eur.Command=static_cast<EDITOR_UNDOREDO_COMMANDS>(Action.toInteger());
		Ret=(__int64)CtrlObject->Plugins->CurEditor->EditorControl(ECTL_UNDOREDO,0,&eur);
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

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Title.isInteger() && !Title.i())
		{
			Title=L"";
			Title.toString();
		}
		Ret=(__int64)CtrlObject->Plugins->CurEditor->EditorControl(ECTL_SETTITLE,0,(void*)Title.s());
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

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_DELLINE, nullptr, Line.getInteger()-1);
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// S=Editor.GetStr([Line])
static bool editorgetstrFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	__int64 Ret=0;
	TVar Res(L"");
	TVar& Line(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			string strRes;
			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_GETSTR, &strRes, Line.getInteger()-1);
			Res=strRes.CPtr();
		}
	}

	PassValue(&Res, Data);
	return Ret!=0;
}

// N=Editor.InsStr([S[,Line]])
static bool editorinsstrFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar Ret(0ll);
	TVar& S(Params[0]);
	TVar& Line(Params[1]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S=L"";
				S.toString();
			}

			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_INSSTR, (wchar_t *)S.s(), Line.getInteger()-1);
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

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S=L"";
				S.toString();
			}

			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_SETSTR, (wchar_t *)S.s(), Line.getInteger()-1);
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Plugin.Exist(Guid)
static bool pluginexistFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar Ret(0ll);
	TVar& pGuid(Params[0]);

	if (pGuid.s())
	{
		GUID guid;
		Ret=(StrToGuid(pGuid.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))?1:0;
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Plugin.Load(DllPath[,ForceLoad])
static bool pluginloadFunc(FarMacroCall* Data)
{
	parseParams(2,Params,Data);
	TVar Ret(0ll);
	TVar& ForceLoad(Params[1]);
	TVar& DllPath(Params[0]);
	if (DllPath.s())
		Ret=(__int64)pluginapi::apiPluginsControl(nullptr, !ForceLoad.i()?PCTL_LOADPLUGIN:PCTL_FORCEDLOADPLUGIN, 0, (void*)DllPath.s());
	PassValue(&Ret, Data);
	return Ret.i()!=0;
}

// N=Plugin.UnLoad(DllPath)
static bool pluginunloadFunc(FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	TVar Ret(0ll);
	TVar& DllPath(Params[0]);
	if (DllPath.s())
	{
		Plugin* p = CtrlObject->Plugins->GetPlugin(DllPath.s());
		if(p)
		{
			Ret=(__int64)pluginapi::apiPluginsControl(p, PCTL_UNLOADPLUGIN, 0, nullptr);
		}
	}

	PassValue(&Ret, Data);
	return Ret.i()!=0;
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
	return Ret?true:false;
}

static bool ReadVarsConsts (FarMacroCall* Data)
{
	parseParams(1,Params,Data);
	string strName;
	string Value, strType;
	bool received = false;

	if (!StrCmp(Params[0].s(), L"consts"))
		received = MacroCfg->EnumConsts(strName, Value, strType);
	else if (!StrCmp(Params[0].s(), L"vars"))
		received = MacroCfg->EnumVars(strName, Value, strType);

	if (received)
	{
		PassString(strName,Data);
		PassString(Value,Data);
		PassString(strType,Data);
	}
	else
		PassBoolean(0, Data);

	return true;
}

#else
#include "macro.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
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
#include "TStack.hpp"
#include "syslog.hpp"
#include "plugapi.hpp"
#include "plugin.hpp"
#include "plugins.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "grabber.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "datetime.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "constitle.hpp"
#include "dirmix.hpp"
#include "console.hpp"
#include "imports.hpp"
#include "CFileMask.hpp"
#include "vmenu.hpp"
#include "elevation.hpp"
#include "FarGuid.hpp"
#include "configdb.hpp"

// ��� ������� ���������� �������
struct DlgParam
{
	UINT64 Flags;
	KeyMacro *Handle;
	DWORD Key;
	int Mode;
	int Recurse;
	bool Changed;
};

TMacroKeywords MKeywords[] =
{
	{0,  L"Other",                    MCODE_C_AREA_OTHER,0},
	{0,  L"Shell",                    MCODE_C_AREA_SHELL,0},
	{0,  L"Viewer",                   MCODE_C_AREA_VIEWER,0},
	{0,  L"Editor",                   MCODE_C_AREA_EDITOR,0},
	{0,  L"Dialog",                   MCODE_C_AREA_DIALOG,0},
	{0,  L"Search",                   MCODE_C_AREA_SEARCH,0},
	{0,  L"Disks",                    MCODE_C_AREA_DISKS,0},
	{0,  L"MainMenu",                 MCODE_C_AREA_MAINMENU,0},
	{0,  L"Menu",                     MCODE_C_AREA_MENU,0},
	{0,  L"Help",                     MCODE_C_AREA_HELP,0},
	{0,  L"Info",                     MCODE_C_AREA_INFOPANEL,0},
	{0,  L"QView",                    MCODE_C_AREA_QVIEWPANEL,0},
	{0,  L"Tree",                     MCODE_C_AREA_TREEPANEL,0},
	{0,  L"FindFolder",               MCODE_C_AREA_FINDFOLDER,0},
	{0,  L"UserMenu",                 MCODE_C_AREA_USERMENU,0},
	{0,  L"Shell.AutoCompletion",     MCODE_C_AREA_SHELL_AUTOCOMPLETION,0},
	{0,  L"Dialog.AutoCompletion",    MCODE_C_AREA_DIALOG_AUTOCOMPLETION,0},

	// ������
	{2,  L"Bof",                MCODE_C_BOF,0},
	{2,  L"Eof",                MCODE_C_EOF,0},
	{2,  L"Empty",              MCODE_C_EMPTY,0},
	{2,  L"Selected",           MCODE_C_SELECTED,0},

	{2,  L"Far.Width",          MCODE_V_FAR_WIDTH,0},
	{2,  L"Far.Height",         MCODE_V_FAR_HEIGHT,0},
	{2,  L"Far.Title",          MCODE_V_FAR_TITLE,0},
	{2,  L"Far.UpTime",         MCODE_V_FAR_UPTIME,0},
	{2,  L"Far.PID",            MCODE_V_FAR_PID,0},
	{2,  L"Macro.Area",         MCODE_V_MACRO_AREA,0},

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
	{2,  L"APanel.Filter",      MCODE_C_APANEL_FILTER,0},
	{2,  L"PPanel.Filter",      MCODE_C_PPANEL_FILTER,0},

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
	{2,  L"APanel.Path0",       MCODE_V_APANEL_PATH0,0},
	{2,  L"PPanel.Path0",       MCODE_V_PPANEL_PATH0,0},
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
	{2,  L"APanel.HostFile",    MCODE_V_APANEL_HOSTFILE,0},
	{2,  L"PPanel.HostFile",    MCODE_V_PPANEL_HOSTFILE,0},
	{2,  L"APanel.Prefix",      MCODE_V_APANEL_PREFIX,0},
	{2,  L"PPanel.Prefix",      MCODE_V_PPANEL_PREFIX,0},
	{2,  L"APanel.Format",      MCODE_V_APANEL_FORMAT,0},
	{2,  L"PPanel.Format",      MCODE_V_PPANEL_FORMAT,0},

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
	{2,  L"Editor.SelValue",    MCODE_V_EDITORSELVALUE,0},

	{2,  L"Dlg.ItemType",       MCODE_V_DLGITEMTYPE,0},
	{2,  L"Dlg.ItemCount",      MCODE_V_DLGITEMCOUNT,0},
	{2,  L"Dlg.CurPos",         MCODE_V_DLGCURPOS,0},
	{2,  L"Dlg.PrevPos",        MCODE_V_DLGPREVPOS,0},
	{2,  L"Dlg.Info.Id",        MCODE_V_DLGINFOID,0},
	{2,  L"Dlg.Info.Owner",     MCODE_V_DLGINFOOWNER,0},

	{2,  L"Help.FileName",      MCODE_V_HELPFILENAME, 0},
	{2,  L"Help.Topic",         MCODE_V_HELPTOPIC, 0},
	{2,  L"Help.SelTopic",      MCODE_V_HELPSELTOPIC, 0},

	{2,  L"Drv.ShowPos",        MCODE_V_DRVSHOWPOS,0},
	{2,  L"Drv.ShowMode",       MCODE_V_DRVSHOWMODE,0},

	{2,  L"Viewer.FileName",    MCODE_V_VIEWERFILENAME,0},
	{2,  L"Viewer.State",       MCODE_V_VIEWERSTATE,0},

	{2,  L"Menu.Value",         MCODE_V_MENU_VALUE,0},
	{2,  L"Menu.Info.Id",       MCODE_V_MENUINFOID,0},

	{2,  L"Fullscreen",         MCODE_C_FULLSCREENMODE,0},
	{2,  L"IsUserAdmin",        MCODE_C_ISUSERADMIN,0},
};

TMacroKeywords MKeywordsArea[] =
{
	{0,  L"Funcs",             (DWORD)MACRO_FUNCS,0},
	{0,  L"Consts",            (DWORD)MACRO_CONSTS,0},
	{0,  L"Vars",              (DWORD)MACRO_VARS,0},
	{0,  L"Other",                    MACRO_OTHER,0},
	{0,  L"Shell",                    MACRO_SHELL,0},
	{0,  L"Viewer",                   MACRO_VIEWER,0},
	{0,  L"Editor",                   MACRO_EDITOR,0},
	{0,  L"Dialog",                   MACRO_DIALOG,0},
	{0,  L"Search",                   MACRO_SEARCH,0},
	{0,  L"Disks",                    MACRO_DISKS,0},
	{0,  L"MainMenu",                 MACRO_MAINMENU,0},
	{0,  L"Menu",                     MACRO_MENU,0},
	{0,  L"Help",                     MACRO_HELP,0},
	{0,  L"Info",                     MACRO_INFOPANEL,0},
	{0,  L"QView",                    MACRO_QVIEWPANEL,0},
	{0,  L"Tree",                     MACRO_TREEPANEL,0},
	{0,  L"FindFolder",               MACRO_FINDFOLDER,0},
	{0,  L"UserMenu",                 MACRO_USERMENU,0},
	{0,  L"Shell.AutoCompletion",     MACRO_SHELLAUTOCOMPLETION,0},
	{0,  L"Dialog.AutoCompletion",    MACRO_DIALOGAUTOCOMPLETION,0},
	{0,  L"Common",                   MACRO_COMMON,0},
};

TMacroKeywords MKeywordsVarType[] =
{
	{3,  L"unknown",   vtUnknown, 0},
	{3,  L"integer",   vtInteger, 0},
	{3,  L"text",      vtString,  0},
	{3,  L"real",      vtDouble,  0},
};

TMacroKeywords MKeywordsFlags[] =
{
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

	{1,  L"NoSendKeysToPlugins",MFLAGS_NOSENDKEYSTOPLUGINS,0},
};

const wchar_t* GetAreaName(DWORD AreaValue) {return GetNameOfValue(AreaValue, MKeywordsArea);}
DWORD GetAreaValue(const wchar_t* AreaName) {return GetValueOfVame(AreaName, MKeywordsArea);}

const wchar_t* GetVarTypeName(DWORD ValueType) {return GetNameOfValue(ValueType, MKeywordsVarType);}
DWORD GetVarTypeValue(const wchar_t* ValueName) {return GetValueOfVame(ValueName, MKeywordsVarType);}

const string FlagsToString(FARKEYMACROFLAGS Flags)
{
	return FlagsToString(Flags, MKeywordsFlags);
}

FARKEYMACROFLAGS StringToFlags(const string& strFlags)
{
	return StringToFlags(strFlags, MKeywordsFlags);
}

// ������������� ������� - ��� <-> ��� ������������
static struct TKeyCodeName
{
	int Key;
	int Len;
	const wchar_t *Name;
} KeyMacroCodes[]=
{
	{ MCODE_OP_AKEY,                 5, L"$AKey"      }, // �������, ������� ������� ������
	{ MCODE_OP_BREAK,                6, L"$Break"     },
	{ MCODE_OP_CONTINUE,             9, L"$Continue"  },
	{ MCODE_OP_ELSE,                 5, L"$Else"      },
	{ MCODE_OP_END,                  4, L"$End"       },
	{ MCODE_OP_EXIT,                 5, L"$Exit"      },
	{ MCODE_OP_IF,                   3, L"$If"        },
	{ MCODE_OP_REP,                  4, L"$Rep"       },
	{ MCODE_OP_SELWORD,              8, L"$SelWord"   },
	//{ MCODE_OP_PLAINTEXT,            5, L"$Text"      }, // $Text "Plain Text"
	{ MCODE_OP_WHILE,                6, L"$While"     },
	{ MCODE_OP_XLAT,                 5, L"$XLat"      },
};

static bool absFunc(const TMacroFunction*);
static bool ascFunc(const TMacroFunction*);
static bool atoiFunc(const TMacroFunction*);
static bool beepFunc(const TMacroFunction*);
static bool chrFunc(const TMacroFunction*);
static bool clipFunc(const TMacroFunction*);
static bool dateFunc(const TMacroFunction*);
static bool dlggetvalueFunc(const TMacroFunction*);
static bool dlgsetfocusFunc(const TMacroFunction*);
static bool editordellineFunc(const TMacroFunction*);
static bool editorgetstrFunc(const TMacroFunction*);
static bool editorinsstrFunc(const TMacroFunction*);
static bool editorposFunc(const TMacroFunction*);
static bool editorselFunc(const TMacroFunction*);
static bool editorsetFunc(const TMacroFunction*);
static bool editorsetstrFunc(const TMacroFunction*);
static bool editorsettitleFunc(const TMacroFunction*);
static bool editorundoFunc(const TMacroFunction*);
static bool environFunc(const TMacroFunction*);
static bool farcfggetFunc(const TMacroFunction*);
static bool fattrFunc(const TMacroFunction*);
static bool fexistFunc(const TMacroFunction*);
static bool floatFunc(const TMacroFunction*);
static bool flockFunc(const TMacroFunction*);
static bool fmatchFunc(const TMacroFunction*);
static bool fsplitFunc(const TMacroFunction*);
static bool iifFunc(const TMacroFunction*);
static bool indexFunc(const TMacroFunction*);
static bool intFunc(const TMacroFunction*);
static bool itowFunc(const TMacroFunction*);
static bool kbdLayoutFunc(const TMacroFunction*);
static bool keybarshowFunc(const TMacroFunction*);
static bool keyFunc(const TMacroFunction*);
static bool lcaseFunc(const TMacroFunction*);
static bool lenFunc(const TMacroFunction*);
static bool macroenumkwdFunc(const TMacroFunction*);
static bool macroenumfuncFunc(const TMacroFunction*);
static bool macroenumvarFunc(const TMacroFunction*);
static bool macroenumConstFunc(const TMacroFunction*);
static bool maxFunc(const TMacroFunction*);
static bool menushowFunc(const TMacroFunction*);
static bool minFunc(const TMacroFunction*);
static bool mloadFunc(const TMacroFunction*);
static bool modFunc(const TMacroFunction*);
static bool msaveFunc(const TMacroFunction*);
static bool msgBoxFunc(const TMacroFunction*);
static bool panelfattrFunc(const TMacroFunction*);
static bool panelfexistFunc(const TMacroFunction*);
static bool panelitemFunc(const TMacroFunction*);
static bool panelselectFunc(const TMacroFunction*);
static bool panelsetpathFunc(const TMacroFunction*);
static bool panelsetposFunc(const TMacroFunction*);
static bool panelsetposidxFunc(const TMacroFunction*);
static bool pluginsFunc(const TMacroFunction*);
static bool promptFunc(const TMacroFunction*);
static bool replaceFunc(const TMacroFunction*);
static bool rindexFunc(const TMacroFunction*);
static bool size2strFunc(const TMacroFunction*);
static bool sleepFunc(const TMacroFunction*);
static bool stringFunc(const TMacroFunction*);
static bool strwrapFunc(const TMacroFunction*);
static bool strpadFunc(const TMacroFunction*);
static bool substrFunc(const TMacroFunction*);
static bool testfolderFunc(const TMacroFunction*);
static bool trimFunc(const TMacroFunction*);
static bool ucaseFunc(const TMacroFunction*);
static bool usersFunc(const TMacroFunction*);
static bool waitkeyFunc(const TMacroFunction*);
static bool windowscrollFunc(const TMacroFunction*);
static bool xlatFunc(const TMacroFunction*);
static bool pluginloadFunc(const TMacroFunction*);
static bool pluginunloadFunc(const TMacroFunction*);
static bool pluginexistFunc(const TMacroFunction*);

static TMacroFunction intMacroFunction[]=
{
	//Name                fnGUID   Syntax                                                        Func                Buffer BufferSize IntFlags                          Code
	{L"Abs",              nullptr, L"N=Abs(N)",                                                  absFunc,            nullptr, 0, 0,                                      MCODE_F_ABS,             },
	{L"Akey",             nullptr, L"V=Akey(Mode[,Type])",                                       usersFunc,          nullptr, 0, 0,                                      MCODE_F_AKEY,            },
	{L"Asc",              nullptr, L"N=Asc(N)",                                                  ascFunc,            nullptr, 0, 0,                                      MCODE_F_ASC,             },
	{L"Atoi",             nullptr, L"N=Atoi(S[,Radix])",                                         atoiFunc,           nullptr, 0, 0,                                      MCODE_F_ATOI,            },
	{L"Beep",             nullptr, L"N=Beep([N])",                                               beepFunc,           nullptr, 0, 0,                                      MCODE_F_BEEP,            },
	{L"BM.Add",           nullptr, L"N=BM.Add()",                                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_ADD,          },
	{L"BM.Clear",         nullptr, L"N=BM.Clear()",                                              usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_CLEAR,        },
	{L"BM.Del",           nullptr, L"N=BM.Del([Idx])",                                           usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_DEL,          },
	{L"BM.Get",           nullptr, L"N=BM.Get(Idx,M)",                                           usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_GET,          },
	{L"BM.Goto",          nullptr, L"N=BM.Goto([N])",                                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_GOTO,         },
	{L"BM.Next",          nullptr, L"N=BM.Next()",                                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_NEXT,         },
	{L"BM.Pop",           nullptr, L"N=BM.Pop()",                                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_POP,          },
	{L"BM.Prev",          nullptr, L"N=BM.Prev()",                                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_PREV,         },
	{L"BM.Back",          nullptr, L"N=BM.Back()",                                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_BACK,         },
	{L"BM.Push",          nullptr, L"N=BM.Push()",                                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_PUSH,         },
	{L"BM.Stat",          nullptr, L"N=BM.Stat([N])",                                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_STAT,         },
	{L"CallPlugin",       nullptr, L"V=CallPlugin(SysID[,param])",                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_CALLPLUGIN,      },
	{L"CheckHotkey",      nullptr, L"N=CheckHotkey(S[,N])",                                      usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_CHECKHOTKEY,},
	{L"Chr",              nullptr, L"S=Chr(N)",                                                  chrFunc,            nullptr, 0, 0,                                      MCODE_F_CHR,             },
	{L"Clip",             nullptr, L"V=Clip(N[,V])",                                             clipFunc,           nullptr, 0, 0,                                      MCODE_F_CLIP,            },
	{L"Date",             nullptr, L"S=Date([S])",                                               dateFunc,           nullptr, 0, 0,                                      MCODE_F_DATE,            },
	{L"Dlg.GetValue",     nullptr, L"V=Dlg.GetValue([Pos[,InfoID]])",                            dlggetvalueFunc,    nullptr, 0, 0,                                      MCODE_F_DLG_GETVALUE,    },
	{L"Dlg.SetFocus",     nullptr, L"N=Dlg.SetFocus([ID])",                                      dlgsetfocusFunc,    nullptr, 0, 0,                                      MCODE_F_DLG_SETFOCUS,    },
	{L"Editor.DelLine",   nullptr, L"N=Editor.DelLine([Line])",                                  editordellineFunc,  nullptr, 0, 0,                                      MCODE_F_EDITOR_DELLINE,  },
	{L"Editor.GetStr",    nullptr, L"S=Editor.GetStr([Line])",                                   editorgetstrFunc,   nullptr, 0, 0,                                      MCODE_F_EDITOR_GETSTR,   },
	{L"Editor.InsStr",    nullptr, L"N=Editor.InsStr([S[,Line]])",                               editorinsstrFunc,   nullptr, 0, 0,                                      MCODE_F_EDITOR_INSSTR,   },
	{L"Editor.Pos",       nullptr, L"N=Editor.Pos(Op,What[,Where])",                             editorposFunc,      nullptr, 0, 0,                                      MCODE_F_EDITOR_POS,      },
	{L"Editor.Sel",       nullptr, L"V=Editor.Sel(Action[,Opt])",                                editorselFunc,      nullptr, 0, 0,                                      MCODE_F_EDITOR_SEL,      },
	{L"Editor.Set",       nullptr, L"N=Editor.Set(N,Var)",                                       editorsetFunc,      nullptr, 0, 0,                                      MCODE_F_EDITOR_SET,      },
	{L"Editor.SetStr",    nullptr, L"N=Editor.SetStr([S[,Line]])",                               editorsetstrFunc,   nullptr, 0, 0,                                      MCODE_F_EDITOR_SETSTR,   },
	{L"Editor.Settitle",  nullptr, L"N=Editor.SetTitle([Title])",                                editorsettitleFunc, nullptr, 0, 0,                                      MCODE_F_EDITOR_SETTITLE, },
	{L"Editor.Undo",      nullptr, L"V=Editor.Undo(N)",                                          editorundoFunc,     nullptr, 0, 0,                                      MCODE_F_EDITOR_UNDO,     },
	{L"Env",              nullptr, L"S=Env(S[,Mode[,Value]])",                                   environFunc,        nullptr, 0, 0,                                      MCODE_F_ENVIRON,         },
	{L"Eval",             nullptr, L"N=Eval(S[,N])",                                             usersFunc,          nullptr, 0, 0,                                      MCODE_F_EVAL,            },
	{L"Far.Cfg.Get",      nullptr, L"V=Far.Cfg.Get(Key,Name)",                                   farcfggetFunc,      nullptr, 0, 0,                                      MCODE_F_FAR_CFG_GET,     },
	{L"FAttr",            nullptr, L"N=FAttr(S)",                                                fattrFunc,          nullptr, 0, 0,                                      MCODE_F_FATTR,           },
	{L"FExist",           nullptr, L"N=FExist(S)",                                               fexistFunc,         nullptr, 0, 0,                                      MCODE_F_FEXIST,          },
	{L"Float",            nullptr, L"N=Float(V)",                                                floatFunc,          nullptr, 0, 0,                                      MCODE_F_FLOAT,           },
	{L"FLock",            nullptr, L"N=FLock(N,N)",                                              flockFunc,          nullptr, 0, 0,                                      MCODE_F_FLOCK,           },
	{L"FMatch",           nullptr, L"N=FMatch(S,Mask)",                                          fmatchFunc,         nullptr, 0, 0,                                      MCODE_F_FMATCH,          },
	{L"FSplit",           nullptr, L"S=FSplit(S,N)",                                             fsplitFunc,         nullptr, 0, 0,                                      MCODE_F_FSPLIT,          },
	{L"GetHotkey",        nullptr, L"S=GetHotkey([N])",                                          usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_GETHOTKEY,  },
	{L"History.Disable",  nullptr, L"N=History.Disable([State])",                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_HISTIORY_DISABLE,},
	{L"Iif",              nullptr, L"V=Iif(Condition,V1,V2)",                                    iifFunc,            nullptr, 0, 0,                                      MCODE_F_IIF,             },
	{L"Index",            nullptr, L"S=Index(S1,S2[,Mode])",                                     indexFunc,          nullptr, 0, 0,                                      MCODE_F_INDEX,           },
	{L"Int",              nullptr, L"N=Int(V)",                                                  intFunc,            nullptr, 0, 0,                                      MCODE_F_INT,             },
	{L"Itoa",             nullptr, L"S=Itoa(N[,radix])",                                         itowFunc,           nullptr, 0, 0,                                      MCODE_F_ITOA,            },
	{L"KbdLayout",        nullptr, L"N=kbdLayout([N])",                                          kbdLayoutFunc,      nullptr, 0, 0,                                      MCODE_F_KBDLAYOUT,       },
	{L"Key",              nullptr, L"S=Key(V)",                                                  keyFunc,            nullptr, 0, 0,                                      MCODE_F_KEY,             },
	{L"KeyBar.Show",      nullptr, L"N=KeyBar.Show([N])",                                        keybarshowFunc,     nullptr, 0, 0,                                      MCODE_F_KEYBAR_SHOW,     },
	{L"LCase",            nullptr, L"S=LCase(S1)",                                               lcaseFunc,          nullptr, 0, 0,                                      MCODE_F_LCASE,           },
	{L"Len",              nullptr, L"N=Len(S)",                                                  lenFunc,            nullptr, 0, 0,                                      MCODE_F_LEN,             },
	{L"Macro.Const",      nullptr, L"S=Macro.Const(Index[,Type])",                               macroenumConstFunc, nullptr, 0, 0,                                      MCODE_F_MACRO_CONST,     },
	{L"Macro.Func",       nullptr, L"S=Macro.Func(Index[,Type])",                                macroenumfuncFunc,  nullptr, 0, 0,                                      MCODE_F_MACRO_FUNC,      },
	{L"Macro.Keyword",    nullptr, L"S=Macro.Keyword(Index[,Type])",                             macroenumkwdFunc,   nullptr, 0, 0,                                      MCODE_F_MACRO_KEYWORD,   },
	{L"Macro.Var",        nullptr, L"S=Macro.Var(Index[,Type])",                                 macroenumvarFunc,   nullptr, 0, 0,                                      MCODE_F_MACRO_VAR,       },
	{L"Max",              nullptr, L"N=Max(N1,N2)",                                              maxFunc,            nullptr, 0, 0,                                      MCODE_F_MAX,             },
	{L"Menu.Filter",      nullptr, L"N=Menu.Filter([Action[,Mode]])",                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_FILTER,     },
	{L"Menu.FilterStr",   nullptr, L"N=Menu.FilterStr([Action[,S]])",                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_FILTERSTR,  },
	{L"Menu.GetValue",    nullptr, L"S=Menu.GetValue([N])",                                      usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_GETVALUE,   },
	{L"Menu.ItemStatus",  nullptr, L"N=Menu.ItemStatus([N])",                                    usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_ITEMSTATUS, },
	{L"Menu.Select",      nullptr, L"N=Menu.Select(S[,N[,Dir]])",                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_SELECT,     },
	{L"Menu.Show",        nullptr, L"S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])", menushowFunc,       nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_MENU_SHOW,       },
	{L"Min",              nullptr, L"N=Min(N1,N2)",                                              minFunc,            nullptr, 0, 0,                                      MCODE_F_MIN,             },
	{L"MLoad",            nullptr, L"N=MLoad(S)",                                                mloadFunc,          nullptr, 0, 0,                                      MCODE_F_MLOAD,           },
	{L"MMode",            nullptr, L"N=MMode(Action[,Value])",                                   usersFunc,          nullptr, 0, 0,                                      MCODE_F_MMODE,           },
	{L"Mod",              nullptr, L"N=Mod(a,b)",                                                modFunc,            nullptr, 0, 0,                                      MCODE_F_MOD,             },
	{L"MSave",            nullptr, L"N=MSave(S)",                                                msaveFunc,          nullptr, 0, 0,                                      MCODE_F_MSAVE,           },
	{L"MsgBox",           nullptr, L"N=MsgBox([Title[,Text[,flags]]])",                          msgBoxFunc,         nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_MSGBOX,          },
	{L"Panel.FAttr",      nullptr, L"N=Panel.FAttr(panelType,fileMask)",                         panelfattrFunc,     nullptr, 0, 0,                                      MCODE_F_PANEL_FATTR,     },
	{L"Panel.FExist",     nullptr, L"N=Panel.FExist(panelType,fileMask)",                        panelfexistFunc,    nullptr, 0, 0,                                      MCODE_F_PANEL_FEXIST,    },
	{L"Panel.Item",       nullptr, L"V=Panel.Item(Panel,Index,TypeInfo)",                        panelitemFunc,      nullptr, 0, 0,                                      MCODE_F_PANELITEM,       },
	{L"Panel.Select",     nullptr, L"V=Panel.Select(panelType,Action[,Mode[,Items]])",           panelselectFunc,    nullptr, 0, 0,                                      MCODE_F_PANEL_SELECT,    },
	{L"Panel.SetPath",    nullptr, L"N=panel.SetPath(panelType,pathName[,fileName])",            panelsetpathFunc,   nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_PANEL_SETPATH,   },
	{L"Panel.SetPos",     nullptr, L"N=panel.SetPos(panelType,fileName)",                        panelsetposFunc,    nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_PANEL_SETPOS,    },
	{L"Panel.SetPosIdx",  nullptr, L"N=Panel.SetPosIdx(panelType,Idx[,InSelection])",            panelsetposidxFunc, nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_PANEL_SETPOSIDX, },
	{L"Plugin.Call",      nullptr, L"N=Plugin.Call(Guid[,Item])",                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_PLUGIN_CALL,     },
	{L"Plugin.Command",   nullptr, L"N=Plugin.Command(Guid[,Command])",                          usersFunc,          nullptr, 0, 0,                                      MCODE_F_PLUGIN_COMMAND,  },
	{L"Plugin.Config",    nullptr, L"N=Plugin.Config(Guid[,MenuGuid])",                          usersFunc,          nullptr, 0, 0,                                      MCODE_F_PLUGIN_CONFIG,   },
	{L"Plugin.Exist",     nullptr, L"N=Plugin.Exist(Guid)",                                      pluginexistFunc,    nullptr, 0, 0,                                      MCODE_F_PLUGIN_EXIST,    },
	{L"Plugin.Load",      nullptr, L"N=Plugin.Load(DllPath[,ForceLoad])",                        pluginloadFunc,     nullptr, 0, 0,                                      MCODE_F_PLUGIN_LOAD,     },
	{L"Plugin.Menu",      nullptr, L"N=Plugin.Menu(Guid[,MenuGuid])",                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_PLUGIN_MENU,     },
	{L"Plugin.UnLoad",    nullptr, L"N=Plugin.UnLoad(DllPath)",                                  pluginunloadFunc,   nullptr, 0, 0,                                      MCODE_F_PLUGIN_UNLOAD,   },
	{L"Print",            nullptr, L"N=Print(Str)",                                              usersFunc,          nullptr, 0, 0,                                      MCODE_F_PRINT,           },
	{L"Prompt",           nullptr, L"S=Prompt([Title[,Prompt[,flags[,Src[,History]]]]])",        promptFunc,         nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_PROMPT,          },
	{L"Replace",          nullptr, L"S=Replace(Str,Find,Replace[,Cnt[,Mode]])",                  replaceFunc,        nullptr, 0, 0,                                      MCODE_F_REPLACE,         },
	{L"Rindex",           nullptr, L"S=RIndex(S1,S2[,Mode])",                                    rindexFunc,         nullptr, 0, 0,                                      MCODE_F_RINDEX,          },
 	{L"Size2Str",         nullptr, L"S=Size2Str(N,Flags[,Width])",                               size2strFunc,       nullptr, 0, 0,                                      MCODE_F_SIZE2STR,        },
	{L"Sleep",            nullptr, L"N=Sleep(N)",                                                sleepFunc,          nullptr, 0, 0,                                      MCODE_F_SLEEP,           },
	{L"String",           nullptr, L"S=String(V)",                                               stringFunc,         nullptr, 0, 0,                                      MCODE_F_STRING,          },
	{L"StrPad",           nullptr, L"S=StrPad(Src,Cnt[,Fill[,Op]])",                             strpadFunc,         nullptr, 0, 0,                                      MCODE_F_STRPAD,          },
	{L"StrWrap",          nullptr, L"S=StrWrap(Text,Width[,Break[,Flags]])",                     strwrapFunc,        nullptr, 0, 0,                                      MCODE_F_STRWRAP,         },
	{L"SubStr",           nullptr, L"S=SubStr(S,start[,length])",                                substrFunc,         nullptr, 0, 0,                                      MCODE_F_SUBSTR,          },
	{L"TestFolder",       nullptr, L"N=TestFolder(S)",                                           testfolderFunc,     nullptr, 0, 0,                                      MCODE_F_TESTFOLDER,      },
	{L"Trim",             nullptr, L"S=Trim(S[,N])",                                             trimFunc,           nullptr, 0, 0,                                      MCODE_F_TRIM,            },
	{L"UCase",            nullptr, L"S=UCase(S1)",                                               ucaseFunc,          nullptr, 0, 0,                                      MCODE_F_UCASE,           },
	{L"WaitKey",          nullptr, L"V=Waitkey([N,[T]])",                                        waitkeyFunc,        nullptr, 0, 0,                                      MCODE_F_WAITKEY,         },
	{L"Window.Scroll",    nullptr, L"N=Window.Scroll(Lines[,Axis])",                             windowscrollFunc,   nullptr, 0, 0,                                      MCODE_F_WINDOW_SCROLL,   },
	{L"Xlat",             nullptr, L"S=Xlat(S[,Flags])",                                         xlatFunc,           nullptr, 0, 0,                                      MCODE_F_XLAT,            },
};

static_assert(MCODE_F_LAST - KEY_MACRO_F_BASE == ARRAYSIZE(intMacroFunction), "intMacroFunction size != MCODE_F_* count");

int MKeywordsSize = ARRAYSIZE(MKeywords);
int MKeywordsFlagsSize = ARRAYSIZE(MKeywordsFlags);

DWORD KeyMacro::LastOpCodeUF=KEY_MACRO_U_BASE;
size_t KeyMacro::CMacroFunction=0;
size_t KeyMacro::AllocatedFuncCount=0;
TMacroFunction *KeyMacro::AMacroFunction=nullptr;

TVarTable glbVarTable;
TVarTable glbConstTable;
TVMStack VMStack;

static TVar __varTextDate;

bool __CheckCondForSkip(const TVar& Cond,DWORD Op)
{
	if (Cond.isString() && *Cond.s())
		return false;

	__int64 res=Cond.getInteger();
	switch(Op)
	{
		case MCODE_OP_JZ:
			return !res?true:false;
		case MCODE_OP_JNZ:
			return res?true:false;
		case MCODE_OP_JLT:
			return res < 0?true:false;
		case MCODE_OP_JLE:
			return res <= 0?true:false;
		case MCODE_OP_JGT:
			return res > 0?true:false;
		case MCODE_OP_JGE:
			return res >= 0?true:false;
	}
	return false;
}

// ������� �������������� ���� ������������ � �����
BOOL KeyMacroToText(int Key,string &strKeyText0)
{
	string strKeyText;

	for (int I=0; I<int(ARRAYSIZE(KeyMacroCodes)); I++)
	{
		if (Key==KeyMacroCodes[I].Key)
		{
			strKeyText = KeyMacroCodes[I].Name;
			break;
		}
	}

	if (strKeyText.IsEmpty())
	{
		strKeyText0.Clear();
		return FALSE;
	}

	strKeyText0 = strKeyText;
	return TRUE;
}

// ������� �������������� �������� � ��� ������������
// ������ -1, ���� ��� �����������!
int KeyNameMacroToKey(const wchar_t *Name)
{
	// ��������� �� ���� �������������
	for (int I=0; I < int(ARRAYSIZE(KeyMacroCodes)); ++I)
		if (!StrCmpI(Name,KeyMacroCodes[I].Name))
			return KeyMacroCodes[I].Key;

	return -1;
}

#if 0
static bool checkMacroFarIntConst(string &strValueName)
{
	return
		strValueName==constMsX ||
		strValueName==constMsY ||
		strValueName==constMsButton ||
		strValueName==constMsCtrlState ||
		strValueName==constMsEventFlags ||
		strValueName==constRCounter ||
		strValueName==constFarCfgErr;
}
#endif

static void initMacroFarIntConst()
{
	INT64 TempValue=0;
	KeyMacro::SetMacroConst(constMsX,TempValue);
	KeyMacro::SetMacroConst(constMsY,TempValue);
	KeyMacro::SetMacroConst(constMsButton,TempValue);
	KeyMacro::SetMacroConst(constMsCtrlState,TempValue);
	KeyMacro::SetMacroConst(constMsEventFlags,TempValue);
	KeyMacro::SetMacroConst(constRCounter,TempValue);
	KeyMacro::SetMacroConst(constFarCfgErr,TempValue);
}

const TVar& TVMStack::Pop()
{
	static TVar temp; //���� ����� ���� ������� �� ��������.

	if (TStack<TVar>::Pop(temp))
		return temp;

	return Error;
};

void TVMStack::Swap()
{
	TStack<TVar>::Swap();
}

TVar& TVMStack::Pop(TVar &dest)
{
	if (!TStack<TVar>::Pop(dest))
		dest=Error;

	return dest;
};

const TVar& TVMStack::Peek()
{
	TVar *var = TStack<TVar>::Peek();

	if (var)
		return *var;

	return Error;
};

KeyMacro::KeyMacro():
	MacroVersion(2),
	Recording(MACROMODE_NOMACRO),
	InternalInput(0),
	IsRedrawEditor(TRUE),
	Mode(MACRO_SHELL),
	StartMode(MACRO_SHELL),
	CurPCStack(-1),
	StopMacro(false),
	MacroLIBCount(0),
	MacroLIB(nullptr),
	RecBufferSize(0),
	RecBuffer(nullptr),
	RecSrc(nullptr),
	RecDescription(nullptr),
	LockScr(nullptr)
{
	Work.Init(nullptr);
	ClearArray(IndexMode);
}

KeyMacro::~KeyMacro()
{
	InitInternalVars();

	if (Work.AllocVarTable && Work.locVarTable)
		xf_free(Work.locVarTable);

	DestroyMacroLib();

	UnregMacroFunction(-1);
}

void KeyMacro::DestroyMacroLib()
{
	if (MacroLIB)
	{
		while(MacroLIBCount) DelMacro(MacroLIBCount-1);
		xf_free(MacroLIB);
		MacroLIB=nullptr;
	}
}

void KeyMacro::InitInternalLIBVars()
{
	if (MacroLIB)
	{
		for (int ii=0;ii<MacroLIBCount;)
		{
			if (IsEqualGUID(FarGuid,MacroLIB[ii].Guid))
				DelMacro(ii);
			else
				++ii;
		}
		if (0==MacroLIBCount)
		{
			xf_free(MacroLIB);
				MacroLIB=nullptr;
		}
	}
	else
	{
		MacroLIBCount=0;
	}

	if (RecBuffer)
		xf_free(RecBuffer);
	RecBuffer=nullptr;
	RecBufferSize=0;

	ClearArray(IndexMode);
 	//MacroLIBCount=0;
 	//MacroLIB=nullptr;
	//LastOpCodeUF=KEY_MACRO_U_BASE;
}

// ������������� ���� ����������
void KeyMacro::InitInternalVars(BOOL InitedRAM)
{
	InitInternalLIBVars();

	if (LockScr)
	{
		delete LockScr;
		LockScr=nullptr;
	}

	if (InitedRAM)
	{
		ReleaseWORKBuffer(TRUE);
		Work.Executing=MACROMODE_NOMACRO;
	}

	Work.HistoryDisable=0;
	RecBuffer=nullptr;
	RecBufferSize=0;
	RecSrc=nullptr;
	RecDescription=nullptr;
	Recording=MACROMODE_NOMACRO;
	InternalInput=FALSE;
	VMStack.Free();
	CurPCStack=-1;
}

// �������� ���������� ������, ���� �� ���������� �����������
// (����������� - ������ � PlayMacros �������� ������.
void KeyMacro::ReleaseWORKBuffer(BOOL All)
{
	if (Work.MacroWORK)
	{
		if (All || Work.MacroWORKCount <= 1)
		{
			for (int I=0; I<Work.MacroWORKCount; I++)
			{
				if (Work.MacroWORK[I].BufferSize > 1 && Work.MacroWORK[I].Buffer)
					xf_free(Work.MacroWORK[I].Buffer);

				if (Work.MacroWORK[I].Src)
					xf_free(Work.MacroWORK[I].Src);

				if (Work.MacroWORK[I].Name)
					xf_free(Work.MacroWORK[I].Name);

				if (Work.MacroWORK[I].Description)
					xf_free(Work.MacroWORK[I].Description);
			}

			xf_free(Work.MacroWORK);

			if (Work.AllocVarTable)
			{
				deleteVTable(*Work.locVarTable);
				//xf_free(Work.locVarTable);
				//Work.locVarTable=nullptr;
				//Work.AllocVarTable=false;
			}

			Work.MacroWORK=nullptr;
			Work.MacroWORKCount=0;
		}
		else
		{
			if (Work.MacroWORK[0].BufferSize > 1 && Work.MacroWORK[0].Buffer)
				xf_free(Work.MacroWORK[0].Buffer);

			if (Work.MacroWORK[0].Src)
				xf_free(Work.MacroWORK[0].Src);

			if (Work.MacroWORK[0].Name)
				xf_free(Work.MacroWORK[0].Name);

			if (Work.MacroWORK[0].Description)
				xf_free(Work.MacroWORK[0].Description);

			if (Work.AllocVarTable)
			{
				deleteVTable(*Work.locVarTable);
				//xf_free(Work.locVarTable);
				//Work.locVarTable=nullptr;
				//Work.AllocVarTable=false;
			}

			Work.MacroWORKCount--;
			memmove(Work.MacroWORK,((BYTE*)Work.MacroWORK)+sizeof(MacroRecord),sizeof(MacroRecord)*Work.MacroWORKCount);
			Work.MacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*Work.MacroWORKCount);
		}
	}
}

// �������� ���� �������� �� �������
int KeyMacro::LoadMacros(BOOL InitedRAM,BOOL LoadAll)
{
	int ErrCount=0;
	InitInternalVars(InitedRAM);

	if (Opt.Macro.DisableMacro&MDOL_ALL)
		return FALSE;

	string strBuffer;
	ReadVarsConsts();
	ReadPluginFunctions();

	int Areas[MACRO_LAST];

	for (int i=MACRO_OTHER; i < MACRO_LAST; i++)
	{
		Areas[i]=i;
	}

	if (!LoadAll)
	{
		// "������� �� �����" �������� ������� - ����� ����������� ������ ��, ��� �� ����� �������� MACRO_LAST
		Areas[MACRO_SHELL]=
			Areas[MACRO_SEARCH]=
			Areas[MACRO_DISKS]=
			Areas[MACRO_MAINMENU]=
			Areas[MACRO_INFOPANEL]=
			Areas[MACRO_QVIEWPANEL]=
			Areas[MACRO_TREEPANEL]=
			Areas[MACRO_USERMENU]= // <-- Mantis#0001594
			Areas[MACRO_SHELLAUTOCOMPLETION]=
			Areas[MACRO_FINDFOLDER]=MACRO_LAST;
	}

	for (int i=MACRO_OTHER; i < MACRO_LAST; i++)
	{
		if (Areas[i] == MACRO_LAST)
			continue;

		if (!ReadKeyMacro(i))
		{
			ErrCount++;
		}
	}

	KeyMacro::Sort();

	return ErrCount?FALSE:TRUE;
}

int KeyMacro::ProcessEvent(const struct FAR_INPUT_RECORD *Rec)
{
	string strKey;
	int Key=Rec->IntKey;

	if (InternalInput || Key==KEY_IDLE || Key==KEY_NONE || !FrameManager->GetCurrentFrame())
		return FALSE;

	if (Recording) // ���� ������?
	{
		// ������� ����� ������?
		if (Key==Opt.Macro.KeyMacroCtrlDot || Key==Opt.Macro.KeyMacroRCtrlDot
			|| Key==Opt.Macro.KeyMacroCtrlShiftDot || Key==Opt.Macro.KeyMacroRCtrlShiftDot)
		{
			_KEYMACRO(CleverSysLog Clev(L"MACRO End record..."));
			int WaitInMainLoop0=WaitInMainLoop;
			InternalInput=TRUE;
			WaitInMainLoop=FALSE;
			// �������� _�������_ �����, � �� _��������� �����������_
			FrameManager->GetCurrentFrame()->Lock(); // ������� ���������� ������
			DWORD MacroKey;
			// ���������� ����� �� ���������.
			UINT64 Flags=MFLAGS_DISABLEOUTPUT|MFLAGS_CALLPLUGINENABLEMACRO; // ???
			int AssignRet=AssignMacroKey(MacroKey,Flags);
			FrameManager->ResetLastInputRecord();
			FrameManager->GetCurrentFrame()->Unlock(); // ������ ����� :-)
			// ������� �������� �� ��������
			// ���� ������� ��� ��� ������ ������ ���������, �� �� ����� �������� ������ ���������.
			//if (MacroKey != (DWORD)-1 && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
			if (AssignRet && AssignRet!=2 && RecBufferSize
				&& (Key==Opt.Macro.KeyMacroCtrlShiftDot || Key==Opt.Macro.KeyMacroRCtrlShiftDot))
			{
				if (!GetMacroSettings(MacroKey,Flags))
					AssignRet=0;
			}

			WaitInMainLoop=WaitInMainLoop0;
			InternalInput=FALSE;

			if (!AssignRet)
			{
				if (RecBuffer)
				{
					xf_free(RecBuffer);
					RecBuffer=nullptr;
					RecBufferSize=0;
				}
			}
			else
			{
				// � ������� common ����� ������ ������ ��� ��������
				int Pos=GetIndex(MacroKey,strKey,StartMode,!(RecBuffer && RecBufferSize),true);

				if (Pos == -1)
				{
					Pos=MacroLIBCount;

					if (RecBufferSize > 0)
					{
						MacroRecord *NewMacroLIB=(MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));

						if (!NewMacroLIB)
						{
							WaitInFastFind++;
							return FALSE;
						}

						MacroLIB=NewMacroLIB;
						MacroLIBCount++;
					}
				}
				else
				{
					if (MacroLIB[Pos].BufferSize > 1 && MacroLIB[Pos].Buffer)
						xf_free(MacroLIB[Pos].Buffer);

					if (MacroLIB[Pos].Src)
						xf_free(MacroLIB[Pos].Src);

					if (MacroLIB[Pos].Name)
						xf_free(MacroLIB[Pos].Name);

					if (MacroLIB[Pos].Description)
						xf_free(MacroLIB[Pos].Description);

					MacroLIB[Pos].Buffer=nullptr;
					MacroLIB[Pos].Src=nullptr;
					MacroLIB[Pos].Name=nullptr;
					MacroLIB[Pos].Callback=nullptr;
					MacroLIB[Pos].Description=nullptr;
				}

				if (Pos < MacroLIBCount)
				{
					MacroRecord Macro = {0};
					Macro.Key=MacroKey;

					if (RecBufferSize > 0 && !RecSrc)
						RecBuffer[RecBufferSize++]=MCODE_OP_ENDKEYS;

					if (RecBufferSize > 1)
						Macro.Buffer=RecBuffer;
					else if (RecBuffer && RecBufferSize > 0)
						Macro.Buffer=reinterpret_cast<DWORD*>((intptr_t)(*RecBuffer));
					else if (!RecBufferSize)
						Macro.Buffer=nullptr;

					Macro.BufferSize=RecBufferSize;
					Macro.Src=RecSrc?RecSrc:MkTextSequence(Macro.Buffer,Macro.BufferSize);
					Macro.Description=RecDescription;

					// ���� ������� ������ - ������������� StartMode,
					// ����� ������ �� common ������� �� �������, � ������� ��� ������ �������.
					if (!Macro.BufferSize||!Macro.Src)
						StartMode=MacroLIB[Pos].Flags&MFLAGS_MODEMASK;

					Macro.Flags=Flags|(StartMode&MFLAGS_MODEMASK)|MFLAGS_NEEDSAVEMACRO|(Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);
					Macro.Guid=FarGuid;
					Macro.Id=nullptr;
					Macro.Callback=nullptr;

					string strKeyText;
					if (KeyToText(MacroKey, strKeyText))
						Macro.Name=xf_wcsdup(strKeyText);
					else
						Macro.Name=nullptr;

					MacroLIB[Pos]=Macro;
				}
			}

			Recording=MACROMODE_NOMACRO;
			RecBuffer=nullptr;
			RecBufferSize=0;
			RecSrc=nullptr;
			RecDescription=nullptr;
			ScrBuf.RestoreMacroChar();
			WaitInFastFind++;
			KeyMacro::Sort();

			if (Opt.AutoSaveSetup)
				WriteMacroRecords(); // �������� ������ ���������!

			return TRUE;
		}
		else // ������� ������ ������������.
		{
			if ((unsigned int)Key>=KEY_NONE && (unsigned int)Key<=KEY_END_SKEY) // ����������� ������� ��������
				return FALSE;

			RecBuffer=(DWORD *)xf_realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+3));

			if (!RecBuffer)
			{
				RecBufferSize=0;
				return FALSE;
			}

			if (IntKeyState.ReturnAltValue) // "����������" ������ ;-)
				Key|=KEY_ALTDIGIT;

			if (!RecBufferSize)
				RecBuffer[RecBufferSize++]=MCODE_OP_KEYS;

			RecBuffer[RecBufferSize++]=Key;
			return FALSE;
		}
	}
	// ������ ������?
	else if (Key==Opt.Macro.KeyMacroCtrlDot || Key==Opt.Macro.KeyMacroRCtrlDot
			|| Key==Opt.Macro.KeyMacroCtrlShiftDot || Key==Opt.Macro.KeyMacroRCtrlShiftDot)
	{
		_KEYMACRO(CleverSysLog Clev(L"MACRO Begin record..."));

		// ������� 18
		if (Opt.Policies.DisabledOptions&FFPOL_CREATEMACRO)
			return FALSE;

		//if(CtrlObject->Plugins->CheckFlags(PSIF_ENTERTOOPENPLUGIN))
		//	return FALSE;

		if (LockScr)
			delete LockScr;
		LockScr=nullptr;

		// ��� ��?
		StartMode=(Mode==MACRO_SHELL && !WaitInMainLoop)?MACRO_OTHER:Mode;
		// ��� ������ - � ������� ������� �������� ���...
		// � ����������� �� ����, ��� ������ ������ ������, ��������� ����� ����� (Ctrl-.
		// � ��������� ������� ����) ��� ����������� (Ctrl-Shift-. - ��� �������� ������ �������)
		Recording=(Key==Opt.Macro.KeyMacroCtrlDot || Key==Opt.Macro.KeyMacroRCtrlDot) ? MACROMODE_RECORDING_COMMON:MACROMODE_RECORDING;

		if (RecBuffer)
			xf_free(RecBuffer);
		RecBuffer=nullptr;
		RecBufferSize=0;

		RecSrc=nullptr;
		RecDescription=nullptr;
		ScrBuf.ResetShadow();
		ScrBuf.Flush();
		WaitInFastFind--;
		return TRUE;
	}
	else
	{
		if (Work.Executing == MACROMODE_NOMACRO) // ��� ��� �� ����� ����������?
		{
			//_KEYMACRO(CleverSysLog Clev(L"MACRO find..."));
			//_KEYMACRO(SysLog(L"Param Key=%s",_FARKEY_ToName(Key)));
			UINT64 CurFlags;

			StopMacro=false;

			if ((Key&(~KEY_CTRLMASK)) > 0x01 && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN) // 0xFFFF ??
			{
				//Key=KeyToKeyLayout(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
				//Key=Upper(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
				//_KEYMACRO(SysLog(L"Upper(Key)=%s",_FARKEY_ToName(Key)));

				if ((Key&(~KEY_CTRLMASK)) > 0x7F && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN)
					Key=KeyToKeyLayout(Key&0x0000FFFF)|(Key&(~0x0000FFFF));

				if ((DWORD)Key < KEY_FKEY_BEGIN)
					Key=Upper(Key&0x0000FFFF)|(Key&(~0x0000FFFF));

			}

			int I=GetIndex(Key,strKey,(Mode==MACRO_SHELL && !WaitInMainLoop) ? MACRO_OTHER:Mode);

			if (I != -1 && !((CurFlags=MacroLIB[I].Flags)&MFLAGS_DISABLEMACRO) && CtrlObject)
			{
				_KEYMACRO(SysLog(L"[%d] Found KeyMacro (I=%d Key=%s,%s)",__LINE__,I,_FARKEY_ToName(Key),_FARKEY_ToName(MacroLIB[I].Key)));

				if (!CheckAll(Mode,CurFlags))
					return FALSE;

				// ��������� ������� ���������� � MacroWORK
				//PostNewMacro(MacroLIB+I);
				// ��������� �����?
				if (CurFlags&MFLAGS_DISABLEOUTPUT)
				{
					if (LockScr)
						delete LockScr;

					LockScr=new LockScreen;
				}

				// ��������� ����� ����� (� ��������� ������� ����) ��� ����������� (��� �������� ������ �������)
				Work.HistoryDisable=0;
				Work.ExecLIBPos=0;
				PostNewMacro(MacroLIB+I);
				//Work.cRec=*FrameManager->GetLastInputRecord();
				Work.cRec=Rec->Rec;
				_SVS(FarSysLog_INPUT_RECORD_Dump(L"Macro",&Work.cRec));
				Work.MacroPC=I;
				IsRedrawEditor=CtrlObject->Plugins->CheckFlags(PSIF_ENTERTOOPENPLUGIN)?FALSE:TRUE;
				_KEYMACRO(SysLog(L"**** Start Of Execute Macro ****"));
				_KEYMACRO(SysLog(1));
				return TRUE;
			}
		}

		return FALSE;
	}
}

bool KeyMacro::GetPlainText(string& strDest)
{
	strDest.Clear();

	if (!Work.MacroWORK)
		return false;

	MacroRecord *MR=Work.MacroWORK;
	int LenTextBuf=(int)(StrLength((wchar_t*)&MR->Buffer[Work.ExecLIBPos])+1)*sizeof(wchar_t);

	if (LenTextBuf && MR->Buffer[Work.ExecLIBPos])
	{
		strDest=(const wchar_t *)&MR->Buffer[Work.ExecLIBPos];
		_SVS(SysLog(L"strDest='%s'",strDest.CPtr()));
		_SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
		size_t nSize = LenTextBuf/sizeof(DWORD);
		if (LenTextBuf == sizeof(wchar_t) || (LenTextBuf % sizeof(DWORD)) )    // ���������� �� sizeof(DWORD) ������.
			nSize++;
		Work.ExecLIBPos+=static_cast<int>(nSize);
		_SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
		return true;
	}
	else
	{
		Work.ExecLIBPos++;
	}

	return false;
}

int KeyMacro::GetPlainTextSize()
{
	if (!Work.MacroWORK)
		return 0;

	MacroRecord *MR=Work.MacroWORK;
	return StrLength((wchar_t*)&MR->Buffer[Work.ExecLIBPos]);
}

TVar KeyMacro::FARPseudoVariable(UINT64 Flags,DWORD CheckCode,DWORD& Err)
{
	_KEYMACRO(CleverSysLog Clev(L"KeyMacro::FARPseudoVariable()"));
	size_t I;
	TVar Cond(0ll);
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

	// ������ ������ ������� ��������
	for (I=0 ; I < ARRAYSIZE(MKeywords) ; ++I)
		if (MKeywords[I].Value == CheckCode)
			break;

	if (I == ARRAYSIZE(MKeywords))
	{
		Err=1;
		_KEYMACRO(SysLog(L"return; Err=%d",Err));
		return Cond; // ����� TRUE �����������, ����� ���������� ���������� �������, ��� ��� �� ���������.
	}

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

	// ������ ������� ����������� ��������
	switch (MKeywords[I].Type)
	{
		case 0: // �������� �� �������
		{
			if (WaitInMainLoop) // ����� ���� ������ ��� ����� WaitInMainLoop, ���� ���� � ���������!!!
				Cond=int(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == FrameManager->GetCurrentFrame()->GetMacroMode()?1:0;
			else
				Cond=int(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == CtrlObject->Macro.GetMode()?1:0;

			break;
		}
		case 2:
		{
			Panel *PassivePanel=nullptr;

			if (ActivePanel)
				PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

			Frame* CurFrame=FrameManager->GetCurrentFrame();

			switch (CheckCode)
			{
				case MCODE_V_FAR_WIDTH:
					Cond=(__int64)(ScrX+1);
					break;
				case MCODE_V_FAR_HEIGHT:
					Cond=(__int64)(ScrY+1);
					break;
				case MCODE_V_FAR_TITLE:
					Console.GetTitle(strFileName);
					Cond=strFileName.CPtr();
					break;
				case MCODE_V_FAR_PID:
					Cond=(__int64)GetCurrentProcessId();
					break;
				case MCODE_V_FAR_UPTIME:
				{
					LARGE_INTEGER Frequency, Counter;
					QueryPerformanceFrequency(&Frequency);
					QueryPerformanceCounter(&Counter);
					Cond=((Counter.QuadPart-FarUpTime.QuadPart)*1000)/Frequency.QuadPart;
					break;
				}
				case MCODE_V_MACRO_AREA:
					Cond=GetAreaName(CtrlObject->Macro.GetMode());
					break;
				case MCODE_C_FULLSCREENMODE: // Fullscreen?
					Cond=IsConsoleFullscreen()?1:0;
					break;
				case MCODE_C_ISUSERADMIN: // IsUserAdmin?
					Cond=(__int64)Opt.IsUserAdmin;
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
					Cond=CtrlObject->CmdLine?CtrlObject->CmdLine->VMProcess(CheckCode):-1;
					break;
				}
				case MCODE_V_CMDLINE_VALUE:            // CmdLine.Value
				{
					if (CtrlObject->CmdLine)
						CtrlObject->CmdLine->GetString(strFileName);
					Cond=strFileName.CPtr();
					break;
				}
				case MCODE_C_APANEL_ROOT:  // APanel.Root
				case MCODE_C_PPANEL_ROOT:  // PPanel.Root
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_ROOT)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(MCODE_C_ROOTFOLDER)?1:0;

					break;
				}
				case MCODE_C_APANEL_BOF:
				case MCODE_C_PPANEL_BOF:
				case MCODE_C_APANEL_EOF:
				case MCODE_C_PPANEL_EOF:
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_APANEL_EOF)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_PPANEL_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;

					break;
				}
				case MCODE_C_SELECTED:    // Selected?
				{
					int NeedType = Mode == MACRO_EDITOR? MODALTYPE_EDITOR : (Mode == MACRO_VIEWER? MODALTYPE_VIEWER : (Mode == MACRO_DIALOG? MODALTYPE_DIALOG : MODALTYPE_PANELS));

					if (!(Mode == MACRO_USERMENU || Mode == MACRO_MAINMENU || Mode == MACRO_MENU) && CurFrame && CurFrame->GetType()==NeedType)
					{
						int CurSelected;

						if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
							CurSelected=(int)CtrlObject->CmdLine->VMProcess(CheckCode);
						else
							CurSelected=(int)CurFrame->VMProcess(CheckCode);

						Cond=CurSelected?1:0;
					}
					else
					{
					#if 1
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
						while (f)
						{
							fo=f;
							f=f->GetTopModal();
						}

						if (!f)
							f=fo;

						if (f)
						{
							Cond=f->VMProcess(CheckCode);
						}
					#else

						Frame *f=FrameManager->GetTopModal();

						if (f)
							Cond=(__int64)f->VMProcess(CheckCode);
					#endif
					}
					break;
				}
				case MCODE_C_EMPTY:   // Empty
				case MCODE_C_BOF:
				case MCODE_C_EOF:
				{
					int CurMMode=CtrlObject->Macro.GetMode();

					if (!(Mode == MACRO_USERMENU || Mode == MACRO_MAINMENU || Mode == MACRO_MENU) && CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACRO_INFOPANEL || CurMMode == MACRO_QVIEWPANEL || CurMMode == MACRO_TREEPANEL))
					{
						if (CheckCode == MCODE_C_EMPTY)
							Cond=CtrlObject->CmdLine->GetLength()?0:1;
						else
							Cond=CtrlObject->CmdLine->VMProcess(CheckCode);
					}
					else
					{
						//if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
						{
							Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

							//f=f->GetTopModal();
							while (f)
							{
								fo=f;
								f=f->GetTopModal();
							}

							if (!f)
								f=fo;

							if (f)
							{
								Cond=f->VMProcess(CheckCode);
							}
						}
					}

					break;
				}
				case MCODE_V_DLGITEMCOUNT: // Dlg.ItemCount
				case MCODE_V_DLGCURPOS:    // Dlg.CurPos
				case MCODE_V_DLGITEMTYPE:  // Dlg.ItemType
				case MCODE_V_DLGPREVPOS:   // Dlg.PrevPos
				{
					if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
						Cond=(__int64)CurFrame->VMProcess(CheckCode);

					break;
				}
				case MCODE_V_DLGINFOID:        // Dlg.Info.Id
				{
					if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
					{
						Cond=reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(CurFrame->VMProcess(CheckCode)));
					}

					break;
				}
				case MCODE_V_DLGINFOOWNER:        // Dlg.Info.Owner
				{
					if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
					{
						Cond=reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(CurFrame->VMProcess(CheckCode)));
					}

					break;
				}
				case MCODE_C_APANEL_VISIBLE:  // APanel.Visible
				case MCODE_C_PPANEL_VISIBLE:  // PPanel.Visible
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_VISIBLE?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond = SelPanel->IsVisible()?1:0;

					break;
				}
				case MCODE_C_APANEL_ISEMPTY: // APanel.Empty
				case MCODE_C_PPANEL_ISEMPTY: // PPanel.Empty
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_ISEMPTY?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);
						int GetFileCount=SelPanel->GetFileCount();
						Cond=(!GetFileCount ||
						      (GetFileCount == 1 && TestParentFolderName(strFileName)))
						     ?1:0;
					}

					break;
				}
				case MCODE_C_APANEL_FILTER:
				case MCODE_C_PPANEL_FILTER:
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_FILTER)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(MCODE_C_APANEL_FILTER)?1:0;

					break;
				}
				case MCODE_C_APANEL_LFN:
				case MCODE_C_PPANEL_LFN:
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_LFN ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = SelPanel->GetShowShortNamesMode()?0:1;

					break;
				}
				case MCODE_C_APANEL_LEFT: // APanel.Left
				case MCODE_C_PPANEL_LEFT: // PPanel.Left
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_LEFT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = SelPanel == CtrlObject->Cp()->LeftPanel ? 1 : 0;

					break;
				}
				case MCODE_C_APANEL_FILEPANEL: // APanel.FilePanel
				case MCODE_C_PPANEL_FILEPANEL: // PPanel.FilePanel
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_FILEPANEL ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=SelPanel->GetType() == FILE_PANEL;

					break;
				}
				case MCODE_C_APANEL_PLUGIN: // APanel.Plugin
				case MCODE_C_PPANEL_PLUGIN: // PPanel.Plugin
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_PLUGIN?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->GetMode() == PLUGIN_PANEL;

					break;
				}
				case MCODE_C_APANEL_FOLDER: // APanel.Folder
				case MCODE_C_PPANEL_FOLDER: // PPanel.Folder
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_FOLDER?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);

						if (FileAttr != INVALID_FILE_ATTRIBUTES)
							Cond=(FileAttr&FILE_ATTRIBUTE_DIRECTORY)?1:0;
					}

					break;
				}
				case MCODE_C_APANEL_SELECTED: // APanel.Selected
				case MCODE_C_PPANEL_SELECTED: // PPanel.Selected
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_SELECTED?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						Cond = SelPanel->GetRealSelCount() > 0; //??
					}

					break;
				}
				case MCODE_V_APANEL_CURRENT: // APanel.Current
				case MCODE_V_PPANEL_CURRENT: // PPanel.Current
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURRENT ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);

						if (FileAttr != INVALID_FILE_ATTRIBUTES)
							Cond = strFileName.CPtr();
					}

					break;
				}
				case MCODE_V_APANEL_SELCOUNT: // APanel.SelCount
				case MCODE_V_PPANEL_SELCOUNT: // PPanel.SelCount
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_SELCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = (__int64)SelPanel->GetRealSelCount();

					break;
				}
				case MCODE_V_APANEL_COLUMNCOUNT:       // APanel.ColumnCount - �������� ������:  ���������� �������
				case MCODE_V_PPANEL_COLUMNCOUNT:       // PPanel.ColumnCount - ��������� ������: ���������� �������
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_COLUMNCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = (__int64)SelPanel->GetColumnsCount();

					break;
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
							Cond = (__int64)(Y2-Y1+1);
						else
							Cond = (__int64)(X2-X1+1);
					}

					break;
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

					if (CheckCode == MCODE_V_APANEL_HOSTFILE || CheckCode == MCODE_V_PPANEL_HOSTFILE ||
						CheckCode == MCODE_V_APANEL_FORMAT || CheckCode == MCODE_V_PPANEL_FORMAT)
						Cond = L"";

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
								Cond = (__int64)Info.Flags;
									break;
								case MCODE_V_APANEL_HOSTFILE:
								case MCODE_V_PPANEL_HOSTFILE:
								Cond = Info.HostFile;
									break;
								case MCODE_V_APANEL_FORMAT:
								case MCODE_V_PPANEL_FORMAT:
									Cond = Info.Format;
									break;
						}
					}
					}

					break;
				}

				case MCODE_V_APANEL_PREFIX:           // APanel.Prefix
				case MCODE_V_PPANEL_PREFIX:           // PPanel.Prefix
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PREFIX ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						PluginInfo PInfo = {sizeof(PInfo)};
						if (SelPanel->VMProcess(MCODE_V_APANEL_PREFIX,&PInfo))
							Cond = PInfo.CommandPrefix;
					}

					break;
				}

				case MCODE_V_APANEL_PATH0:           // APanel.Path0
				case MCODE_V_PPANEL_PATH0:           // PPanel.Path0
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH0 ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						if (!SelPanel->VMProcess(CheckCode,&strFileName,0))
							SelPanel->GetCurDir(strFileName);
						Cond = strFileName.CPtr();
					}

					break;
				}

				case MCODE_V_APANEL_PATH: // APanel.Path
				case MCODE_V_PPANEL_PATH: // PPanel.Path
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						if (SelPanel->GetMode() == PLUGIN_PANEL)
						{
							OpenPanelInfo Info={};
							Info.StructSize=sizeof(OpenPanelInfo);
							SelPanel->GetOpenPanelInfo(&Info);
							strFileName = Info.CurDir;
						}
						else
							SelPanel->GetCurDir(strFileName);
						DeleteEndSlash(strFileName); // - ����� � ����� ����� ���� C:, ����� ����� ������ ���: APanel.Path + "\\file"
						Cond = strFileName.CPtr();
					}

					break;
				}

				case MCODE_V_APANEL_UNCPATH: // APanel.UNCPath
				case MCODE_V_PPANEL_UNCPATH: // PPanel.UNCPath
				{
					Cond = L"";

					if (_MakePath1(CheckCode == MCODE_V_APANEL_UNCPATH?KEY_ALTSHIFTBRACKET:KEY_ALTSHIFTBACKBRACKET,strFileName,L""))
					{
						UnquoteExternal(strFileName);
						DeleteEndSlash(strFileName);
						Cond = strFileName.CPtr();
					}

					break;
				}
				//FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
				case MCODE_V_APANEL_TYPE: // APanel.Type
				case MCODE_V_PPANEL_TYPE: // PPanel.Type
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_TYPE ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=(__int64)SelPanel->GetType();

					break;
				}
				case MCODE_V_APANEL_DRIVETYPE: // APanel.DriveType - �������� ������: ��� �������
				case MCODE_V_PPANEL_DRIVETYPE: // PPanel.DriveType - ��������� ������: ��� �������
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_DRIVETYPE ? ActivePanel : PassivePanel;
					Cond=-1;

					if (SelPanel  && SelPanel->GetMode() != PLUGIN_PANEL)
					{
						SelPanel->GetCurDir(strFileName);
						GetPathRoot(strFileName, strFileName);
						UINT DriveType=FAR_GetDriveType(strFileName,nullptr,0);

						// BUGBUG: useless, GetPathRoot expands subst itself

						/*if (ParsePath(strFileName) == PATH_DRIVELETTER)
						{
							string strRemoteName;
							strFileName.SetLength(2);

							if (GetSubstName(DriveType,strFileName,strRemoteName))
								DriveType=DRIVE_SUBSTITUTE;
						}*/

						Cond=TVar((__int64)DriveType);
					}

					break;
				}
				case MCODE_V_APANEL_ITEMCOUNT: // APanel.ItemCount
				case MCODE_V_PPANEL_ITEMCOUNT: // PPanel.ItemCount
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_ITEMCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=(__int64)SelPanel->GetFileCount();

					break;
				}
				case MCODE_V_APANEL_CURPOS: // APanel.CurPos
				case MCODE_V_PPANEL_CURPOS: // PPanel.CurPos
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURPOS ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=SelPanel->GetCurrentPos()+(SelPanel->GetFileCount()>0?1:0);

					break;
				}
				case MCODE_V_TITLE: // Title
				{
					Frame *f=FrameManager->GetTopModal();

					if (f)
					{
						if (CtrlObject->Cp() == f)
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

					Cond=strFileName.CPtr();
					break;
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
							Cond = (__int64)(Y2-Y1+1);
						else
							Cond = (__int64)(X2-X1+1);
					}

					break;
				}
				case MCODE_V_MENU_VALUE: // Menu.Value
				case MCODE_V_MENUINFOID: // Menu.Info.Id
				{
					int CurMMode=GetMode();
					Cond=L"";

					if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
					{
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
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
										Cond=strFileName.CPtr();
									}
									break;
								case MCODE_V_MENUINFOID:
									Cond=reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(f->VMProcess(CheckCode)));
									break;
							}
						}
					}

					break;
				}
				case MCODE_V_ITEMCOUNT: // ItemCount - ����� ��������� � ������� �������
				case MCODE_V_CURPOS: // CurPos - ������� ������ � ������� �������
				{
					#if 1
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
						while (f)
						{
							fo=f;
							f=f->GetTopModal();
						}

						if (!f)
							f=fo;

						if (f)
						{
							Cond=f->VMProcess(CheckCode);
						}
					#else

						Frame *f=FrameManager->GetTopModal();

						if (f)
							Cond=(__int64)f->VMProcess(CheckCode);
					#endif
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
				case MCODE_V_EDITORSELVALUE: // Editor.SelValue
				{
					if (CheckCode == MCODE_V_EDITORVALUE || CheckCode == MCODE_V_EDITORSELVALUE)
						Cond=L"";

					if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
					{
						if (CheckCode == MCODE_V_EDITORFILENAME)
						{
							string strType;
							CtrlObject->Plugins->CurEditor->GetTypeAndName(strType, strFileName);
							Cond=strFileName.CPtr();
						}
						else if (CheckCode == MCODE_V_EDITORVALUE)
						{
							EditorGetString egs={sizeof(EditorGetString)};
							egs.StringNumber=-1;
							CtrlObject->Plugins->CurEditor->EditorControl(ECTL_GETSTRING,0,&egs);
							Cond=egs.StringText;
						}
						else if (CheckCode == MCODE_V_EDITORSELVALUE)
						{
							CtrlObject->Plugins->CurEditor->VMProcess(CheckCode,&strFileName);
							Cond=strFileName.CPtr();
						}
						else
							Cond=CtrlObject->Plugins->CurEditor->VMProcess(CheckCode);
					}

					break;
				}
				case MCODE_V_HELPFILENAME:  // Help.FileName
				case MCODE_V_HELPTOPIC:     // Help.Topic
				case MCODE_V_HELPSELTOPIC:  // Help.SelTopic
				{
					Cond=L"";

					if (CtrlObject->Macro.GetMode() == MACRO_HELP)
					{
						CurFrame->VMProcess(CheckCode,&strFileName,0);
						Cond=strFileName.CPtr();
					}

					break;
				}
				case MCODE_V_VIEWERFILENAME: // Viewer.FileName
				case MCODE_V_VIEWERSTATE: // Viewer.State
				{
					if (CheckCode == MCODE_V_VIEWERFILENAME)
						Cond=L"";

					if ((CtrlObject->Macro.GetMode()==MACRO_VIEWER || CtrlObject->Macro.GetMode()==MACRO_QVIEWPANEL) &&
					        CtrlObject->Plugins->CurViewer && CtrlObject->Plugins->CurViewer->IsVisible())
					{
						if (CheckCode == MCODE_V_VIEWERFILENAME)
						{
							CtrlObject->Plugins->CurViewer->GetFileName(strFileName);//GetTypeAndName(nullptr,FileName);
							Cond=strFileName.CPtr();
						}
						else
							Cond=CtrlObject->Plugins->CurViewer->VMProcess(MCODE_V_VIEWERSTATE);
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

//HERE
static void __parseParams(int Count,TVar* Params)
{
	int stackCount=VMStack.Pop().getInteger();
	while(stackCount>Count)
	{
		VMStack.Pop();
		--stackCount;
	}
	while(stackCount<Count)
	{
		Params[--Count].SetType(vtUnknown);
	}
	for(int ii=stackCount-1;ii>=0;--ii)
	{
		Params[ii]=VMStack.Pop();
	}
}
#define parseParams(c,v) TVar v[c]; __parseParams(c,v)

/* ------------------------------------------------------------------- */
// S=trim(S[,N])
static bool trimFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	int  mode = (int) Params[1].getInteger();
	wchar_t *p = (wchar_t *)Params[0].toString();
	bool Ret=true;

	switch (mode)
	{
		case 0: p=RemoveExternalSpaces(p); break;  // alltrim
		case 1: p=RemoveLeadingSpaces(p); break;   // ltrim
		case 2: p=RemoveTrailingSpaces(p); break;  // rtrim
		default: Ret=false;
	}

	VMStack.Push(p);
	return Ret;
}

// S=substr(S,start[,length])
static bool substrFunc(const TMacroFunction*)
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
	parseParams(3,Params);
	bool Ret=false;

	int  start     = (int)Params[1].getInteger();
	wchar_t *p = (wchar_t *)Params[0].toString();
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
		VMStack.Push(L"");
	}
	else
	{
		p += start;
		p[length] = 0;
		Ret=true;
		VMStack.Push(p);
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
			strDest.SetLength(p-s);
		}
	}
	else
	{
		if (*(p+1) == L':')
		{
			p += 2;

			if ((nFlags & FLAG_DISK) == FLAG_DISK)
			{
				size_t Length=strDest.GetLength()+p-s;
				strDest+=s;
				strDest.SetLength(Length);
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
			size_t Length=strDest.GetLength()+e-s;
			strDest+=s;
			strDest.SetLength(Length);
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

	if (!strDest.IsEmpty())
		AddEndSlash(strDest);

	if (nFlags & FLAG_NAME)
	{
		const wchar_t *ptr=wcspbrk(s,L":");

		if (ptr)
			s=ptr+1;

		size_t Length=strDest.GetLength()+e-s;
		strDest+=s;
		strDest.SetLength(Length);
	}

	if (nFlags & FLAG_EXT)
		strDest+=e;

	return TRUE;
}


// S=fsplit(S,N)
static bool fsplitFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	int m = (int)Params[1].getInteger();
	const wchar_t *s = Params[0].toString();
	bool Ret=false;
	string strPath;

	if (!SplitFileName(s,strPath,m))
		strPath.Clear();
	else
		Ret=true;

	VMStack.Push(strPath.CPtr());
	return Ret;
}

#if 0
// S=Meta("!.!") - � �������� ����� ������ �����������
static bool metaFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	const wchar_t *s = Params[0].toString();

	if (s && *s)
	{
		char SubstText[512];
		char Name[NM],ShortName[NM];
		xstrncpy(SubstText,s,sizeof(SubstText));
		SubstFileName(SubstText,sizeof(SubstText),Name,ShortName,nullptr,nullptr,TRUE);
		return TVar(SubstText);
	}

	return TVar(L"");
}
#endif


// N=atoi(S[,radix])
static bool atoiFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	bool Ret=true;
	wchar_t *endptr;
	VMStack.Push(TVar(_wcstoi64(Params[0].toString(),&endptr,(int)Params[1].toInteger())));
	return Ret;
}


// N=Window.Scroll(Lines[,Axis])
static bool windowscrollFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	bool Ret=false;
	TVar L=Params[0];

	if (Opt.WindowMode)
	{
		int Lines=(int)Params[0].i(), Columns=0;
		L=0;
		if (Params[1].i())
		{
			Columns=Lines;
			Lines=0;
		}

		if (Console.ScrollWindow(Lines, Columns))
		{
			Ret=true;
			L=1;
		}
	}
	else
		L=0;

	VMStack.Push(L);
	return Ret;
}

// S=itoa(N[,radix])
static bool itowFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	bool Ret=false;

	if (Params[0].isInteger())
	{
		wchar_t value[65];
		int Radix=(int)Params[1].toInteger();

		if (!Radix)
			Radix=10;

		Ret=true;
		Params[0]=TVar(_i64tow(Params[0].toInteger(),value,Radix));
	}

	VMStack.Push(Params[0]);
	return Ret;
}

// N=sleep(N)
static bool sleepFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	long Period=(long)Params[0].getInteger();

	if (Period > 0)
	{
		Sleep((DWORD)Period);
		VMStack.Push(1);
		return true;
	}

	VMStack.Push(0ll);
	return false;
}


// N=KeyBar.Show([N])
static bool keybarshowFunc(const TMacroFunction*)
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
	parseParams(1,Params);
	Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

	//f=f->GetTopModal();
	while (f)
	{
		fo=f;
		f=f->GetTopModal();
	}

	if (!f)
		f=fo;

	VMStack.Push(f?f->VMProcess(MCODE_F_KEYBAR_SHOW,nullptr,Params[0].getInteger())-1:-1);
	return f?true:false;
}


// S=key(V)
static bool keyFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	string strKeyText;

	if (Params[0].isInteger())
	{
		if (Params[0].i())
			KeyToText((int)Params[0].i(),strKeyText);
	}
	else
	{
		// ��������...
		DWORD Key=(DWORD)KeyNameToKey(Params[0].s());

		if (Key != (DWORD)-1 && Key==(DWORD)Params[0].i())
			strKeyText=Params[0].s();
	}

	VMStack.Push(strKeyText.CPtr());
	return !strKeyText.IsEmpty()?true:false;
}

// V=waitkey([N,[T]])
static bool waitkeyFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	long Type=(long)Params[1].getInteger();
	long Period=(long)Params[0].getInteger();
	DWORD Key=WaitKey((DWORD)-1,Period);

	if (!Type)
	{
		string strKeyText;

		if (Key != KEY_NONE)
			if (!KeyToText(Key,strKeyText))
				strKeyText.Clear();

		VMStack.Push(strKeyText.CPtr());
		return !strKeyText.IsEmpty()?true:false;
	}

	if (Key == KEY_NONE)
		Key=-1;

	VMStack.Push((__int64)Key);
	return Key != (DWORD)-1;
}

// n=min(n1,n2)
static bool minFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	VMStack.Push(Params[1] < Params[0] ? Params[1] : Params[0]);
	return true;
}

// n=max(n1.n2)
static bool maxFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	VMStack.Push(Params[1] > Params[0]  ? Params[1] : Params[0]);
	return true;
}

// n=mod(n1,n2)
static bool modFunc(const TMacroFunction*)
{
	parseParams(2,Params);

	if (!Params[1].i())
	{
		_KEYMACRO(___FILEFUNCLINE___;SysLog(L"Error: Divide (mod) by zero"));
		VMStack.Push(0ll);
		return false;
	}

	VMStack.Push(Params[0] % Params[1]);
	return true;
}

// n=iif(expression,n1,n2)
static bool iifFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	VMStack.Push(__CheckCondForSkip(Params[0],MCODE_OP_JZ) ? Params[2] : Params[1]);
	return true;
}

// N=index(S1,S2[,Mode])
static bool indexFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	const wchar_t *s = Params[0].toString();
	const wchar_t *p = Params[1].toString();
	const wchar_t *i = !Params[2].getInteger() ? StrStrI(s,p) : StrStr(s,p);
	bool Ret= i ? true : false;
	VMStack.Push(TVar((__int64)(i ? i-s : -1)));
	return Ret;
}

// S=rindex(S1,S2[,Mode])
static bool rindexFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	const wchar_t *s = Params[0].toString();
	const wchar_t *p = Params[1].toString();
	const wchar_t *i = !Params[2].getInteger() ? RevStrStrI(s,p) : RevStrStr(s,p);
	bool Ret= i ? true : false;
	VMStack.Push(TVar((__int64)(i ? i-s : -1)));
	return Ret;
}

// S=Size2Str(Size,Flags[,Width])
static bool size2strFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	int Width = (int)Params[2].getInteger();

	string strDestStr;
	FileSizeToStr(strDestStr,Params[0].i(), !Width?-1:Width, Params[1].i());

	VMStack.Push(TVar(strDestStr.CPtr()));
	return true;
}

// S=date([S])
static bool dateFunc(const TMacroFunction*)
{
	parseParams(1,Params);

	if (Params[0].isInteger() && !Params[0].i())
		Params[0]=L"";

	const wchar_t *s = Params[0].toString();
	bool Ret=false;
	string strTStr;

	if (MkStrFTime(strTStr,s))
		Ret=true;
	else
		strTStr.Clear();

	VMStack.Push(TVar(strTStr.CPtr()));
	return Ret;
}

// S=xlat(S[,Flags])
/*
  Flags:
  	XLAT_SWITCHKEYBLAYOUT  = 1
	XLAT_SWITCHKEYBBEEP    = 2
	XLAT_USEKEYBLAYOUTNAME = 4
*/
static bool xlatFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	wchar_t *Str = (wchar_t *)Params[0].toString();
	bool Ret=::Xlat(Str,0,StrLength(Str),Params[1].i())?true:false;
	VMStack.Push(TVar(Str));
	return Ret;
}

// N=beep([N])
static bool beepFunc(const TMacroFunction*)
{
	parseParams(1,Params);
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
	bool Ret=MessageBeep((UINT)Params[0].i())?true:false;

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

	VMStack.Push(Ret?1:0);
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
static bool kbdLayoutFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	DWORD dwLayout = (DWORD)Params[0].getInteger();

	BOOL Ret=TRUE;
	HKL  Layout=0, RetLayout=0;

	wchar_t LayoutName[1024]={}; // BUGBUG!!!
	if (ifn.GetConsoleKeyboardLayoutNameW(LayoutName))
	{
		wchar_t *endptr;
		DWORD res=wcstoul(LayoutName, &endptr, 16);
		RetLayout=(HKL)(intptr_t)(HIWORD(res)? res : MAKELONG(res,res));
	}

	HWND hWnd = Console.GetWindow();

	if (hWnd && dwLayout)
	{
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

	VMStack.Push(Ret?TVar(static_cast<INT64>(reinterpret_cast<intptr_t>(RetLayout))):0);

	return Ret?true:false;
}

// S=prompt(["Title"[,"Prompt"[,flags[, "Src"[, "History"]]]]])
static bool promptFunc(const TMacroFunction*)
{
	parseParams(5,Params);
	TVar& ValHistory(Params[4]);
	TVar& ValSrc(Params[3]);
	DWORD Flags = (DWORD)Params[2].getInteger();
	TVar& ValPrompt(Params[1]);
	TVar& ValTitle(Params[0]);
	TVar Result(L"");
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

	DWORD oldHistoryDisable=CtrlObject->Macro.GetHistoryDisableMask();

	if (!(history && *history)) // Mantis#0001743: ����������� ���������� �������
		CtrlObject->Macro.SetHistoryDisableMask(8); // ���� �� ������ history, �� ������������� ��������� ������� ��� ����� prompt()

	if (GetString(title,prompt,history,src,strDest,nullptr,(Flags&~FIB_CHECKBOX)|FIB_ENABLEEMPTY,nullptr,nullptr))
	{
		Result=strDest.CPtr();
		Result.toString();
		Ret=true;
	}
	else
		Result=0;

	CtrlObject->Macro.SetHistoryDisableMask(oldHistoryDisable);

	VMStack.Push(Result);
	return Ret;
}

// N=msgbox(["Title"[,"Text"[,flags]]])
static bool msgBoxFunc(const TMacroFunction*)
{
	parseParams(3,Params);
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
	int Result=pluginapi::apiMessageFn(&FarGuid,&FarGuid,Flags,nullptr,(const wchar_t * const *)TempBuf.CPtr(),0,0)+1;
	/*
	if (Result <= -1) // Break?
		CtrlObject->Macro.SendDropProcess();
	*/
	VMStack.Push((__int64)Result);
	return true;
}


static intptr_t WINAPI CompareItems(const MenuItemEx **el1, const MenuItemEx **el2, const SortItemParam *Param)
{
	if (((*el1)->Flags & LIF_SEPARATOR) || ((*el2)->Flags & LIF_SEPARATOR))
		return 0;

	string strName1((*el1)->strName);
	string strName2((*el2)->strName);
	RemoveChar(strName1,L'&',true);
	RemoveChar(strName2,L'&',true);
	int Res = NumStrCmpI(strName1.CPtr()+Param->Offset,strName2.CPtr()+Param->Offset);
	return (Param->Direction?(Res<0?1:(Res>0?-1:0)):Res);
}

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
static bool menushowFunc(const TMacroFunction*)
{
	parseParams(6,Params);
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

	if (!strItems.IsSubStrAt(strItems.GetLength()-1,L"\n"))
		strItems.Append(L"\n");

	TVar Result = -1;
	int BoxType = (Flags & 0x7)?(Flags & 0x7)-1:3;
	bool bResultAsIndex = (Flags & 0x08)?true:false;
	bool bMultiSelect = (Flags & 0x010)?true:false;
	bool bSorting = (Flags & 0x20)?true:false;
	bool bPacking = (Flags & 0x40)?true:false;
	bool bAutohighlight = (Flags & 0x80)?true:false;
	bool bSetMenuFilter = (Flags & 0x100)?true:false;
	bool bAutoNumbering = (Flags & 0x200)?true:false;
	bool bExitAfterNavigate = (Flags & 0x400)?true:false;
	int nLeftShift=bAutoNumbering?9:0;
	int X = -1;
	int Y = -1;
	unsigned __int64 MenuFlags = VMENU_WRAPMODE;

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
	size_t SubstrLen;
	ReplaceStrings(strTitle,L"\r\n",L"\n");
	bool CRFound=strTitle.Pos(PosLF, L"\n");

	if(CRFound)
	{
		strBottom=strTitle.SubStr(PosLF+1);
		strTitle=strTitle.SubStr(0,PosLF);
	}
	VMenu Menu(strTitle.CPtr(),nullptr,0,ScrY-4);
	Menu.SetBottomTitle(strBottom.CPtr());
	Menu.SetFlags(MenuFlags);
	Menu.SetPosition(X,Y,0,0);
	Menu.SetBoxType(BoxType);

	CRFound=strItems.Pos(PosLF, L"\n");
	while(CRFound)
	{
		MenuItemEx NewItem;
		NewItem.Clear();
		SubstrLen=PosLF-CurrentPos;

		if (SubstrLen==0)
			SubstrLen=1;

		NewItem.strName=strItems.SubStr(CurrentPos,SubstrLen);

		if (NewItem.strName!=L"\n")
		{
		wchar_t *CurrentChar=(wchar_t *)NewItem.strName.CPtr();
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
			NewItem.strName.Clear();

		if (bAutoNumbering && !(bSorting || bPacking) && !(NewItem.Flags & LIF_SEPARATOR))
		{
			LineCount++;
			NewItem.strName.Format(L"%6d - %s", LineCount, NewItem.strName.CPtr());
		}
		Menu.AddItem(&NewItem);
		CurrentPos=PosLF+1;
		CRFound=strItems.Pos(PosLF, L"\n",CurrentPos);
	}

	if (bSorting)
		Menu.SortItems(reinterpret_cast<TMENUITEMEXCMPFUNC>(CompareItems));

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
				Item->strName.Format(L"%6d - %s", LineCount, Item->strName.CPtr());
			}
		}
	}

	if (!VFindOrFilter.isUnknown())
	{
		if (bSetMenuFilter)
		{
			Menu.SetFilterEnabled(true);
			Menu.SetFilterString(VFindOrFilter.toString());
			Menu.FilterStringUpdated(true);
			Menu.Show();
		}
		else
		{
			if (VFindOrFilter.isInteger())
			{
				if (VFindOrFilter.toInteger()-1>=0)
					Menu.SetSelectPos(VFindOrFilter.toInteger()-1,1);
				else
					Menu.SetSelectPos(Menu.GetItemCount()+VFindOrFilter.toInteger(),1);
			}
			else
				if (VFindOrFilter.isString())
					Menu.SetSelectPos(Max(0,Menu.FindItem(0, VFindOrFilter.toString())),1);
		}
	}

	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()) )
		frame->Lock();

	Menu.Show();
	int PrevSelectedPos=Menu.GetSelectPos();
	DWORD Key=0;
	int RealPos;
	bool CheckFlag;
	int X1, Y1, X2, Y2, NewY2;
	while (!Menu.Done() && !CloseFARMenu)
	{
		SelectedPos=Menu.GetSelectPos();
		Key=Menu.ReadInput();
		switch (Key)
		{
			case KEY_NUMPAD0:
			case KEY_INS:
				if (bMultiSelect)
				{
					Menu.SetCheck(!Menu.GetCheck(SelectedPos));
					Menu.Show();
				}
				break;

			case KEY_CTRLADD:
			case KEY_CTRLSUBTRACT:
			case KEY_CTRLMULTIPLY:
			case KEY_RCTRLADD:
			case KEY_RCTRLSUBTRACT:
			case KEY_RCTRLMULTIPLY:
				if (bMultiSelect)
				{
					for(int i=0; i<Menu.GetShowItemCount(); i++)
					{
						RealPos=Menu.VisualPosToReal(i);
						if (Key==KEY_CTRLMULTIPLY || Key==KEY_RCTRLMULTIPLY)
						{
							CheckFlag=Menu.GetCheck(RealPos)?false:true;
						}
						else
						{
							CheckFlag=(Key==KEY_CTRLADD || Key==KEY_RCTRLADD);
						}
						Menu.SetCheck(CheckFlag, RealPos);
					}
					Menu.Show();
				}
				break;

			case KEY_CTRLA:
			case KEY_RCTRLA:
			{
				Menu.GetPosition(X1, Y1, X2, Y2);
				NewY2=Y1+Menu.GetShowItemCount()+1;

				if (NewY2>ScrY-2)
					NewY2=ScrY-2;

				Menu.SetPosition(X1,Y1,X2,NewY2);
				Menu.Show();
				break;
			}

			case KEY_BREAK:
				CtrlObject->Macro.SendDropProcess();
				Menu.SetExitCode(-1);
				break;

			default:
				Menu.ProcessInput();
				break;
		}

		if (bExitAfterNavigate && (PrevSelectedPos!=SelectedPos))
		{
			SelectedPos=Menu.GetSelectPos();
			break;
		}

		PrevSelectedPos=SelectedPos;
	}

	wchar_t temp[65];

	if (Menu.Modal::GetExitCode() >= 0)
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
						Result+=(*Menu.GetItemPtr(i)).strName.CPtr()+nLeftShift;
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
					Result=(*Menu.GetItemPtr(SelectedPos)).strName.CPtr()+nLeftShift;
			}
		}
		else
			if(!bResultAsIndex)
				Result=(*Menu.GetItemPtr(SelectedPos)).strName.CPtr()+nLeftShift;
			else
				Result=SelectedPos+1;
		Menu.Hide();
	}
	else
	{
		Menu.Hide();
		if (bExitAfterNavigate)
		{
			Result=SelectedPos+1;
			if ((Key == KEY_ESC) || (Key == KEY_F10) || (Key == KEY_BREAK))
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

	VMStack.Push(Result);
	return true;
}

// S=Env(S[,Mode[,Value]])
static bool environFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	TVar& Value(Params[2]);
	TVar& Mode(Params[1]);
	TVar& S(Params[0]);
	bool Ret=false;
	string strEnv;


	if (apiGetEnvironmentVariable(S.toString(), strEnv))
		Ret=true;
	else
		strEnv.Clear();

	if (Mode.i()) // Mode != 0: Set
	{
		SetEnvironmentVariable(S.toString(),Value.isUnknown() || !*Value.s()?nullptr:Value.toString());
	}

	VMStack.Push(strEnv.CPtr());
	return Ret;
}

// V=Panel.Select(panelType,Action[,Mode[,Items]])
static bool panelselectFunc(const TMacroFunction*)
{
	parseParams(4,Params);
	TVar& ValItems(Params[3]);
	int Mode=(int)Params[2].getInteger();
	DWORD Action=(int)Params[1].getInteger();
	int typePanel=(int)Params[0].getInteger();
	__int64 Result=-1;

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

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
			ValItems=strStr.CPtr();
		}

		MacroPanelSelect mps;
		mps.Action      = Action & 0xF;
		mps.ActionFlags = (Action & (~0xF)) >> 4;
		mps.Mode        = Mode;
		mps.Index       = Index;
		mps.Item        = &ValItems;
		Result=SelPanel->VMProcess(MCODE_F_PANEL_SELECT,&mps,0);
	}

	VMStack.Push(Result);
	return Result==-1?false:true;
}

static bool _fattrFunc(int Type)
{
	bool Ret=false;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
	long Pos=-1;

	if (!Type || Type == 2) // �� ������: fattr(0) & fexist(2)
	{
		parseParams(1,Params);
		TVar& Str(Params[0]);
		FAR_FIND_DATA_EX FindData;
		apiGetFindDataEx(Str.toString(), FindData);
		FileAttr=FindData.dwFileAttributes;
		Ret=true;
	}
	else // panel.fattr(1) & panel.fexist(3)
	{
		parseParams(2,Params);
		TVar& S(Params[1]);
		int typePanel=(int)Params[0].getInteger();
		const wchar_t *Str = S.toString();
		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = !typePanel ? ActivePanel : (typePanel == 1?PassivePanel:nullptr);

		if (SelPanel)
		{
			if (wcspbrk(Str,L"*?") )
				Pos=SelPanel->FindFirst(Str);
			else
				Pos=SelPanel->FindFile(Str,wcspbrk(Str,L"\\/:")?FALSE:TRUE);

			if (Pos >= 0)
			{
				string strFileName;
				SelPanel->GetFileName(strFileName,Pos,FileAttr);
				Ret=true;
			}
		}
	}

	if (Type == 2) // fexist(2)
		FileAttr=(FileAttr!=INVALID_FILE_ATTRIBUTES)?1:0;
	else if (Type == 3) // panel.fexist(3)
		FileAttr=(DWORD)Pos+1;

	VMStack.Push(TVar((__int64)(long)FileAttr));
	return Ret;
}

// N=fattr(S)
static bool fattrFunc(const TMacroFunction*)
{
	return _fattrFunc(0);
}

// N=fexist(S)
static bool fexistFunc(const TMacroFunction*)
{
	return _fattrFunc(2);
}

// N=panel.fattr(S)
static bool panelfattrFunc(const TMacroFunction*)
{
	return _fattrFunc(1);
}

// N=panel.fexist(S)
static bool panelfexistFunc(const TMacroFunction*)
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
static bool flockFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(-1);
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
		Ret=(__int64)SetFLockState(vkKey,stateFLock);

	VMStack.Push(Ret);
	return Ret.i()!=-1;
}

// N=Dlg.SetFocus([ID])
static bool dlgsetfocusFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(-1);
	unsigned Index=(unsigned)Params[0].getInteger()-1;
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		Ret=(__int64)CurFrame->VMProcess(MCODE_V_DLGCURPOS);
		if ((int)Index >= 0)
		{
			if(!SendDlgMessage((HANDLE)CurFrame,DM_SETFOCUS,Index,0))
				Ret=0;
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=-1; // ?? <= 0 ??
}

// V=Far.Cfg.Get(Key,Name)
bool farcfggetFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar& Name(Params[1]);
	TVar& Key(Params[0]);

	string strValue;
	bool resultGetCfg = GetConfigValue(Key.s(),Name.s(), strValue);
	KeyMacro::SetMacroConst(constFarCfgErr,resultGetCfg?0:1);
	VMStack.Push(resultGetCfg?strValue.CPtr():L"");

	return resultGetCfg;
}

// V=Dlg.GetValue([Pos[,InfoID]])
static bool dlggetvalueFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(-1);
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		TVarType typeIndex=Params[0].type();
		unsigned Index=(unsigned)Params[0].getInteger()-1;
		if (typeIndex == vtUnknown || (typeIndex == vtInteger && (int)Index < -1))
			Index=((Dialog*)CurFrame)->GetDlgFocusPos();

		TVarType typeInfoID=Params[1].type();
		int InfoID=(int)Params[1].getInteger();
		if (typeInfoID == vtUnknown || (typeInfoID == vtInteger && InfoID < 0))
			InfoID=0;

		FarGetValue fgv={sizeof(FarGetValue),InfoID,FMVT_UNKNOWN};
		unsigned DlgItemCount=((Dialog*)CurFrame)->GetAllItemCount();
		const DialogItemEx **DlgItem=((Dialog*)CurFrame)->GetAllItem();
		bool CallDialog=true;

		if (Index == (unsigned)-1)
		{
			SMALL_RECT Rect;

			if (SendDlgMessage((HANDLE)CurFrame,DM_GETDLGRECT,0,&Rect))
			{
				switch (InfoID)
				{
					case 0: Ret=(__int64)DlgItemCount; break;
					case 2: Ret=(__int64)Rect.Left; break;
					case 3: Ret=(__int64)Rect.Top; break;
					case 4: Ret=(__int64)Rect.Right; break;
					case 5: Ret=(__int64)Rect.Bottom; break;
					case 6: Ret=(__int64)(((Dialog*)CurFrame)->GetDlgFocusPos()+1); break;
					default: Ret=0; Ret.SetType(vtUnknown); break;
				}
			}
		}
		else if (Index < DlgItemCount && DlgItem)
		{
			const DialogItemEx *Item=DlgItem[Index];
			FARDIALOGITEMTYPES ItemType=Item->Type;
			FARDIALOGITEMFLAGS ItemFlags=Item->Flags;

			if (!InfoID)
			{
				if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
				{
					InfoID=7;
				}
				else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
				{
					FarListGetItem ListItem={sizeof(FarListGetItem)};
					ListItem.ItemIndex=Item->ListPtr->GetSelectPos();

					if (SendDlgMessage(CurFrame,DM_LISTGETITEM,Index,&ListItem))
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
				case 1: Ret=(__int64)ItemType;    break;
				case 2: Ret=(__int64)Item->X1;    break;
				case 3: Ret=(__int64)Item->Y1;    break;
				case 4: Ret=(__int64)Item->X2;    break;
				case 5: Ret=(__int64)Item->Y2;    break;
				case 6: Ret=(__int64)((Item->Flags&DIF_FOCUS)!=0); break;
				case 7:
				{
					if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
					{
						Ret=(__int64)Item->Selected;
					}
					else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=(__int64)(Item->ListPtr->GetSelectPos()+1);
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
				case 9: Ret=(__int64)((Item->Flags&DIF_DEFAULTBUTTON)!=0); break;
				case 10:
				{
					Ret=Item->strData.CPtr();

					if (IsEdit(ItemType))
					{
						DlgEdit *EditPtr;

						if ((EditPtr = (DlgEdit *)(Item->ObjPtr)) )
							Ret=EditPtr->GetStringAddr();
					}

					break;
				}
				case 11:
				{
					if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=(__int64)(Item->ListPtr->GetItemCount());
					}
					break;
				}
			}
		}
		else if (Index >= DlgItemCount)
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

			if (SendDlgMessage((HANDLE)CurFrame,DN_GETVALUE,Index,&fgv))
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

	VMStack.Push(Ret);
	return Ret.i()!=-1;
}

// N=Editor.Pos(Op,What[,Where])
// Op: 0 - get, 1 - set
static bool editorposFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	TVar Ret(-1);
	int Where = (int)Params[2].getInteger();
	int What  = (int)Params[1].getInteger();
	int Op    = (int)Params[0].getInteger();

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		EditorInfo ei={sizeof(EditorInfo)};
		CtrlObject->Plugins->CurEditor->EditorControl(ECTL_GETINFO,0,&ei);

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

				int Result=CtrlObject->Plugins->CurEditor->EditorControl(ECTL_SETPOSITION,0,&esp);

				if (Result)
					CtrlObject->Plugins->CurEditor->EditorControl(ECTL_REDRAW,0,nullptr);

				Ret=Result;
				break;
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.i() != -1;
}

// OldVar=Editor.Set(Idx,Value)
static bool editorsetFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(-1);
	TVar& Value(Params[1]);
	int Index=(int)Params[0].getInteger();

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		long longState=-1L;

		if (Index != 12)
			longState=(long)Value.toInteger();
		else
		{
			if (Value.isString() || Value.i() != -1)
				longState=0;
		}

		EditorOptions EdOpt;
		CtrlObject->Plugins->CurEditor->GetEditorOptions(EdOpt);

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
			case 6:  // AnsiCodePageForNewFile;
				Ret=(__int64)EdOpt.AnsiCodePageForNewFile; break;
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
				case 6:  // AnsiCodePageForNewFile;
					EdOpt.AnsiCodePageForNewFile=longState != 0; break;
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

			CtrlObject->Plugins->CurEditor->SetEditorOptions(EdOpt);
			CtrlObject->Plugins->CurEditor->ShowStatus();
			if (Index == 0 || Index == 12 || Index == 14 || Index == 15 || Index == 20)
				CtrlObject->Plugins->CurEditor->Show();
		}
	}

	VMStack.Push(Ret);
	return Ret.i()==-1;
}

// b=mload(var)
bool mloadFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	TVar TempVar;
	const wchar_t *Name=Val.s();

	if (!Name || *Name!= L'%')
	{
		VMStack.Push(0ll);
		return false;
	}

	__int64 Ret=CtrlObject->Macro.LoadVarFromDB(Name, TempVar);

	if(Ret)
		varInsert(glbVarTable, Name+1)->value = TempVar.s();

	VMStack.Push(Ret);
	return Ret != 0;
}

// b=msave(var)
bool msaveFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	TVarTable *t = &glbVarTable;
	const wchar_t *Name=Val.s();

	if (!Name || *Name!= L'%')
	{
		VMStack.Push(0ll);
		return false;
	}

	TVarSet *tmpVarSet=varLook(*t, Name+1);

	if (!tmpVarSet)
	{
		VMStack.Push(0ll);
		return false;
	}

	string strValueName = Val.s();

	__int64 Ret=CtrlObject->Macro.SaveVarToDB(strValueName, tmpVarSet->value);

	VMStack.Push(Ret);
	return Ret != 0;
}

// V=Clip(N[,V])
static bool clipFunc(const TMacroFunction*)
{
	parseParams(2,Params);
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
			wchar_t *ClipText=PasteFromClipboard();

			if (ClipText)
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
			if (!*Val.s())
			{
				Clipboard clip;
				if (clip.Open())
				{
					Ret=1;
					clip.Empty();
				}
				clip.Close();
			}
			else
			{
				Ret=CopyToClipboard(Val.s());
			}

			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
		}
		case 2: // Add "S" into Clipboard
		{
			TVar varClip(Val.s());
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				wchar_t *CopyData=clip.Paste();

				if (CopyData)
				{
					size_t DataSize=StrLength(CopyData);
					wchar_t *NewPtr=(wchar_t *)xf_realloc(CopyData,(DataSize+StrLength(Val.s())+2)*sizeof(wchar_t));

					if (NewPtr)
					{
						CopyData=NewPtr;
						wcscpy(CopyData+DataSize,Val.s());
						varClip=CopyData;
						xf_free(CopyData);
					}
					else
					{
						xf_free(CopyData);
					}
				}

				Ret=clip.Copy(varClip.s());

				clip.Close();
			}

			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
		}
		case 3: // Copy Win to internal, "S" - ignore
		case 4: // Copy internal to Win, "S" - ignore
		{
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				Ret=clip.InternalCopy((cmdType-3)?true:false)?1:0;
				clip.Close();
			}

			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
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
			VMStack.Push((__int64)(mode?2:1)); // 0!  ???
			return Ret?true:false;
		}
	}

	return Ret?true:false;
}


// N=Panel.SetPosIdx(panelType,Idx[,InSelection])
/*
*/
static bool panelsetposidxFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	int InSelection=(int)Params[2].getInteger();
	long idxItem=(long)Params[1].getInteger();
	int typePanel=(int)Params[0].getInteger();
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;
	__int64 Ret=0;

	if (SelPanel)
	{
		int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
		{
			long EndPos=SelPanel->GetFileCount();
			long I;
			long idxFoundItem=0;

			if (idxItem) // < 0 || > 0
			{
				EndPos--;
				if ( EndPos > 0 )
				{
					long StartPos;
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

					for ( I=StartPos ; ; I+=Direct )
					{
						if (Direct > 0)
						{
							if(I > EndPos)
								break;
						}
						else
						{
							if(I < EndPos)
								break;
						}

						if ( (!InSelection || (InSelection && SelPanel->IsSelected(I))) && SelPanel->FileInFilter(I) )
						{
							if (idxFoundItem == idxItem)
							{
								idxItem=I;
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
						//SelPanel->Show();
						// <Mantis#0000289> - ������, �� �� ������ :-)
						//ShellUpdatePanels(SelPanel);
						SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
						FrameManager->RefreshFrame(FrameManager->GetTopModal());
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
					for ( I=0 ; I < EndPos ; I++ )
					{
						if ( SelPanel->IsSelected(I) && SelPanel->FileInFilter(I) )
						{
							if (I == CurPos)
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

	VMStack.Push(Ret);
	return Ret?true:false;
}

// N=panel.SetPath(panelType,pathName[,fileName])
static bool panelsetpathFunc(const TMacroFunction*)
{
	parseParams(3,Params);
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

		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

		if (SelPanel)
		{
			if (SelPanel->SetCurDir(pathName,SelPanel->GetMode()==PLUGIN_PANEL && IsAbsolutePath(pathName),FrameManager->GetCurrentFrame()->GetType() == MODALTYPE_PANELS))
			{
				ActivePanel=CtrlObject->Cp()->ActivePanel;
				PassivePanel=ActivePanel?CtrlObject->Cp()->GetAnotherPanel(ActivePanel):nullptr;
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

	VMStack.Push(Ret);
	return Ret?true:false;
}

// N=Panel.SetPos(panelType,fileName)
static bool panelsetposFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar& Val(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	const wchar_t *fileName=Val.s();

	if (!fileName || !*fileName)
		fileName=L"";

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

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

	VMStack.Push(Ret);
	return Ret?true:false;
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
static bool replaceFunc(const TMacroFunction*)
{
	parseParams(5,Params);
	int Mode=(int)Params[4].getInteger();
	TVar& Count(Params[3]);
	TVar& Repl(Params[2]);
	TVar& Find(Params[1]);
	TVar& Src(Params[0]);
	__int64 Ret=1;
	// TODO: ����� ����� ��������� � ������������ � ��������!
	string strStr;
	//int lenS=(int)StrLength(Src.s());
	int lenF=(int)StrLength(Find.s());
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
		VMStack.Push(strStr.CPtr());
	}
	else
		VMStack.Push(Src);

	return Ret?true:false;
}

// V=Panel.Item(typePanel,Index,TypeInfo)
static bool panelitemFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	TVar& P2(Params[2]);
	TVar& P1(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	TVar Ret(0ll);
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

	if (!SelPanel)
	{
		VMStack.Push(Ret);
		return false;
	}

	int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

	if (!(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL))
	{
		VMStack.Push(Ret);
		return false;
	}

	int Index=(int)(P1.toInteger())-1;
	int TypeInfo=(int)P2.toInteger();
	FileListItem filelistItem;

	if (TypePanel == TREE_PANEL)
	{
		TreeItem treeItem;

		if (SelPanel->GetItem(Index,&treeItem) && !TypeInfo)
		{
			VMStack.Push(TVar(treeItem.strName));
			return true;
		}
	}
	else
	{
		string strDate, strTime;

		if (TypeInfo == 11)
			SelPanel->ReadDiz();

		if (!SelPanel->GetItem(Index,&filelistItem))
			TypeInfo=-1;

		switch (TypeInfo)
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
				Ret=TVar(strDate.CPtr());
				break;
			case 4:  // AccessTime
				ConvertDate(filelistItem.AccessTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 5:  // WriteTime
				ConvertDate(filelistItem.WriteTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 6:  // FileSize
				Ret=TVar((__int64)filelistItem.FileSize);
				break;
			case 7:  // AllocationSize
				Ret=TVar((__int64)filelistItem.AllocationSize);
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
				Ret=TVar((const wchar_t *)filelistItem.DizText);
				break;
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
			case 18: // NumberOfStreams
				Ret=TVar((__int64)filelistItem.NumberOfStreams);
				break;
			case 19: // StreamsSize
				Ret=TVar((__int64)filelistItem.StreamsSize);
				break;
			case 20:  // ChangeTime
				ConvertDate(filelistItem.ChangeTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 21:  // ChangeTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.ChangeTime));
				break;
			case 22:  // CustomData
				Ret=TVar(filelistItem.strCustomData);
				break;
			case 23:  // ReparseTag
			{
				Ret=TVar((__int64)filelistItem.ReparseTag);
				break;
			}
		}
	}

	VMStack.Push(Ret);
	return false;
}

// N=len(V)
static bool lenFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	VMStack.Push(TVar(StrLength(Params[0].toString())));
	return true;
}

static bool ucaseFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	StrUpper((wchar_t *)Val.toString());
	VMStack.Push(Val);
	return true;
}

static bool lcaseFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	StrLower((wchar_t *)Val.toString());
	VMStack.Push(Val);
	return true;
}

static bool stringFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	Val.toString();
	VMStack.Push(Val);
	return true;
}

// S=StrPad(Src,Cnt[,Fill[,Op]])
static bool strpadFunc(const TMacroFunction*)
{
	string strDest;
	parseParams(4,Params);
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
			wchar_t *NewFill=new wchar_t[FineLength+1];
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

				string strPad=NewFill;
				strPad.SetLength(CntL);
				strPad+=strDest;
				strPad.Append(NewFill, CntR);
				strDest=strPad;

				delete[] NewFill;
			}
		}
	}

	VMStack.Push(strDest.CPtr());
	return true;
}

// S=StrWrap(Text,Width[,Break[,Flags]])
static bool strwrapFunc(const TMacroFunction*)
{
	parseParams(4,Params);
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
	VMStack.Push(strDest.CPtr());
	return true;
}

static bool intFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	Val.toInteger();
	VMStack.Push(Val);
	return true;
}

static bool floatFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	//Val.toDouble();
	VMStack.Push(Val);
	return true;
}

static bool absFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& tmpVar(Params[0]);

	if (tmpVar < 0ll)
		tmpVar=-tmpVar;

	VMStack.Push(tmpVar);
	return true;
}

static bool ascFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& tmpVar(Params[0]);

	if (tmpVar.isString())
	{
		tmpVar = (__int64)((DWORD)((WORD)*tmpVar.toString()));
		tmpVar.toInteger();
	}

	VMStack.Push(tmpVar);
	return true;
}

static bool chrFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& tmpVar(Params[0]);

	if (tmpVar.isInteger())
	{
		const wchar_t tmp[]={static_cast<wchar_t>(tmpVar.i()), L'\0'};
		tmpVar = tmp;
		tmpVar.toString();
	}

	VMStack.Push(tmpVar);
	return true;
}

// N=FMatch(S,Mask)
static bool fmatchFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar& Mask(Params[1]);
	TVar& S(Params[0]);
	CFileMask FileMask;

	if (FileMask.Set(Mask.toString(), FMF_SILENT))
		VMStack.Push(FileMask.Compare(S.toString()));
	else
		VMStack.Push(-1);
	return true;
}

// V=Editor.Sel(Action[,Opt])
static bool editorselFunc(const TMacroFunction*)
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
	parseParams(2,Params);
	TVar Ret(0ll);
	TVar& Opts(Params[1]);
	TVar& Action(Params[0]);

	int Mode=CtrlObject->Macro.GetMode();
	int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS)); // MACRO_SHELL?
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CurFrame && CurFrame->GetType()==NeedType)
	{
		if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
			Ret=CtrlObject->CmdLine->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.i());
		else
			Ret=CurFrame->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.i());
	}

	VMStack.Push(Ret);
	return Ret.i() == 1;
}

// V=Editor.Undo(Action)
static bool editorundoFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& Action(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		EditorUndoRedo eur={sizeof(EditorUndoRedo)};
		eur.Command=static_cast<EDITOR_UNDOREDO_COMMANDS>(Action.toInteger());
		Ret=(__int64)CtrlObject->Plugins->CurEditor->EditorControl(ECTL_UNDOREDO,0,&eur);
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Editor.SetTitle([Title])
static bool editorsettitleFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& Title(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Title.isInteger() && !Title.i())
		{
			Title=L"";
			Title.toString();
		}
		Ret=(__int64)CtrlObject->Plugins->CurEditor->EditorControl(ECTL_SETTITLE,0,(void*)Title.s());
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Editor.DelLine([Line])
static bool editordellineFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& Line(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_DELLINE, nullptr, Line.getInteger()-1);
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// S=Editor.GetStr([Line])
static bool editorgetstrFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	__int64 Ret=0;
	TVar Res(L"");
	TVar& Line(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			string strRes;
			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_GETSTR, &strRes, Line.getInteger()-1);
			Res=strRes.CPtr();
		}
	}

	VMStack.Push(Res);
	return Ret!=0;
}

// N=Editor.InsStr([S[,Line]])
static bool editorinsstrFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(0ll);
	TVar& S(Params[0]);
	TVar& Line(Params[1]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S=L"";
				S.toString();
			}

			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_INSSTR, (wchar_t *)S.s(), Line.getInteger()-1);
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Editor.SetStr([S[,Line]])
static bool editorsetstrFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(0ll);
	TVar& S(Params[0]);
	TVar& Line(Params[1]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S=L"";
				S.toString();
			}

			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_SETSTR, (wchar_t *)S.s(), Line.getInteger()-1);
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Plugin.Exist(Guid)
static bool pluginexistFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& pGuid(Params[0]);

	if (pGuid.s())
	{
		GUID guid;
		Ret=(StrToGuid(pGuid.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))?1:0;
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Plugin.Load(DllPath[,ForceLoad])
static bool pluginloadFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(0ll);
	TVar& ForceLoad(Params[1]);
	TVar& DllPath(Params[0]);
	if (DllPath.s())
		Ret=(__int64)pluginapi::apiPluginsControl(nullptr, !ForceLoad.i()?PCTL_LOADPLUGIN:PCTL_FORCEDLOADPLUGIN, 0, (void*)DllPath.s());
	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Plugin.UnLoad(DllPath)
static bool pluginunloadFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& DllPath(Params[0]);
	if (DllPath.s())
	{
		Plugin* p = CtrlObject->Plugins->GetPlugin(DllPath.s());
		if(p)
		{
			Ret=(__int64)pluginapi::apiPluginsControl(p, PCTL_UNLOADPLUGIN, 0, nullptr);
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// S=Macro.Keyword(Index[,Type])
static bool macroenumkwdFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(L"");
	TVar& Index(Params[0]);
	TVar& Type(Params[1]);

	if (Index.isInteger())
	{
		size_t I=Index.toInteger()-1;

		if ((int)I < 0)
		{
			size_t CountsDefs[]={ARRAYSIZE(MKeywords),ARRAYSIZE(MKeywordsArea)-3,ARRAYSIZE(MKeywordsFlags),ARRAYSIZE(KeyMacroCodes),ARRAYSIZE(MKeywordsVarType)};
			int iType = Type.toInteger();
			Ret=(int)(((unsigned)iType < ARRAYSIZE(CountsDefs))?CountsDefs[iType]:-1);
		}
		else
		{
			switch (Type.toInteger())
			{
				case 0: // Far Keywords
				{
					if (I < ARRAYSIZE(MKeywords))
						Ret=MKeywords[I].Name;
					break;
				}
				case 1: // Area
				{
					I+=3;
					if (I < ARRAYSIZE(MKeywordsArea))
						Ret=MKeywordsArea[I].Name;
					break;
				}
				case 2: // Macro Flags
				{
					if (I < ARRAYSIZE(MKeywordsFlags))
						Ret=MKeywordsFlags[I].Name;
					break;
				}
				case 3: // Macro Operation
				{
					if (I < ARRAYSIZE(KeyMacroCodes))
						Ret=KeyMacroCodes[I].Name;
					break;
				}
				case 4: // type name
				{
					if (I < ARRAYSIZE(MKeywordsVarType))
						Ret=MKeywordsVarType[I].Name;
					break;
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.isString()?(*Ret.s()!=0):(Ret.i() != -1);
}

// S=Macro.Func(Index[,Type])
static bool macroenumfuncFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(L"");
	TVar& Index(Params[0]);
	TVar& Type(Params[1]);

	if (Index.isInteger())
	{
		size_t I=Index.toInteger()-1;

		if ((int)I < 0)
			Ret=(int)KeyMacro::GetCountMacroFunction();
		else
		{
			const TMacroFunction *MFunc=KeyMacro::GetMacroFunction(I);
			if (MFunc)
			{
				switch (Type.toInteger())
				{
					case 0: // Name
						Ret=(const wchar_t*)MFunc->Name;
						break;
					case 1: // Syntax
						Ret=(const wchar_t*)MFunc->Syntax;
						break;
					case 2: // GUID Host
						Ret=(const wchar_t*)MFunc->fnGUID;
						break;
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.isString()?(*Ret.s()!=0):(Ret.i() != -1);
}

static bool _MacroEnumWords(int TypeTable,const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(0);
	TVar& Index(Params[0]);
	TVar& Type(Params[1]);

	Ret.SetType(vtUnknown);
	if (Index.isInteger())
	{
		int I=static_cast<int>(Index.toInteger()-1);

		//TVarTable *t = KeyMacro::GetLocalVarTable();
		TVarTable *t=(TypeTable==MACRO_VARS)?&glbVarTable:&glbConstTable;

		if (I < 0)
		{
			for (I=0; varEnum(*t,I); ++I)
				;
			Ret=I;
		}
		else
		{
			TVarSet *v=varEnum(*t,I);

			if (v)
			{
				switch (Type.toInteger())
				{
					case 0: // Name
						Ret=(const wchar_t*)v->str;
						break;
					case 1: // Value
						Ret=v->value;
						break;
					case 2: // Type (�����)
						Ret=v->value.type();
						break;
					case 3: // TypeName
						Ret=(const wchar_t*)MKeywordsVarType[v->value.type()].Name;
						break;
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.isUnknown()?false:true;
}

// S=Macro.Var(Index[,Type])
static bool macroenumvarFunc(const TMacroFunction *mf)
{
	return _MacroEnumWords(MACRO_VARS,mf);
}

// S=Macro.Const(Index[,Type])
static bool macroenumConstFunc(const TMacroFunction *mf)
{
	return _MacroEnumWords(MACRO_CONSTS,mf);
}

static void VarToFarMacroValue(const TVar& From,FarMacroValue& To)
{
	To.Type=(FARMACROVARTYPE)From.type();
	switch(From.type())
	{
		case vtUnknown:
		case vtInteger:
			To.Integer=From.i();
			break;
		case vtString:
			To.String=xf_wcsdup(From.s());
			break;
		case vtDouble:
			To.Double=From.d();
			break;
		//case vtUnknown:
		//	break;
	}
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
static bool testfolderFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& tmpVar(Params[0]);
	__int64 Ret=TSTFLD_ERROR;

	if (tmpVar.isString())
	{
		DisableElevation de;
		Ret=(__int64)TestFolder(tmpVar.s());
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}

// ����� ���������� �������
static bool pluginsFunc(const TMacroFunction *thisFunc)
{
	TVar V;
	bool Ret=false;
	int nParam=VMStack.Pop().getInteger();
#if defined(MANTIS_0000466)
/*
enum FARMACROVARTYPE
{
	FMVT_UNKNOWN                = 0,
	FMVT_INTEGER                = 1,
	FMVT_STRING                 = 2,
	FMVT_DOUBLE                 = 3,
};

struct FarMacroValue
{
	enum FARMACROVARTYPE Type;
	union
	{
		__int64  Integer;
		double   Double;
		const wchar_t *String;
	}
	Value
	;
};

struct ProcessMacroFuncInfo
{
	size_t StructSize;
	const wchar_t *Name;
	const FarMacroValue *Params;
	int nParams;
	struct FarMacroValue *Results;
	int nResults;
};
*/
	GUID guid;
	if (StrToGuid(thisFunc->fnGUID,guid) && CtrlObject->Plugins->FindPlugin(guid))
	{
		FarMacroValue *vParams=new FarMacroValue[nParam];
		if (vParams)
		{
			int I;
			memset(vParams,0,sizeof(FarMacroValue) * nParam);

			for (I=nParam-1; I >= 0; --I)
			{
				VMStack.Pop(V);
				VarToFarMacroValue(V,*(vParams+I));
			}

			ProcessMacroInfo Info={sizeof(Info),FMIT_PROCESSFUNC};
			Info.Func.StructSize=sizeof(ProcessMacroFuncInfo);
			Info.Func.Name=thisFunc->Name;
			Info.Func.Params=vParams;
			Info.Func.nParams=nParam;

			if (CtrlObject->Plugins->ProcessMacro(guid,&Info))
			{
				if (Info.Func.Results)
				{
					for (I=0; I < Info.Func.nResults; ++I)
					//for (I=nResults-1; I >= 0; --I)
					{
						//V.type()=(TVarType)(Results+I)->Type;
						switch((Info.Func.Results+I)->Type)
						{
							case FMVT_INTEGER:
								V=(Info.Func.Results+I)->Integer;
								break;
							case FMVT_STRING:
								V=(Info.Func.Results+I)->String;
								break;
							case FMVT_DOUBLE:
								V=(Info.Func.Results+I)->Double;
								break;
							case FMVT_UNKNOWN:
								V=0;
								break;
						}
						VMStack.Push(V);
					}
				}
			}

			for (I=0; I < nParam; ++I)
				if((vParams+I)->Type == FMVT_STRING && (vParams+I)->String)
					xf_free((void*)(vParams+I)->String);

			delete[] vParams;
		}
		else
			VMStack.Push(0);
	}
	else
	{
		while(--nParam >= 0)
			VMStack.Pop(V);

		VMStack.Push(0);
	}
#else
	/* �������� */ while(--nParam >= 0) VMStack.Pop(V);
#endif
	return Ret;
}

// ����� ���������������� �������
static bool usersFunc(const TMacroFunction *thisFunc)
{
	TVar V;
	bool Ret=false;

	int nParam=VMStack.Pop().getInteger();
	/* �������� */ while(--nParam >= 0) VMStack.Pop(V);

	VMStack.Push(0);
	return Ret;
}


const wchar_t *eStackAsString(int)
{
	const wchar_t *s=__varTextDate.toString();
	return !s?L"":s;
}

int KeyMacro::GetKey()
{
	MacroRecord *MR;
	TVar tmpVar;
	TVarSet *tmpVarSet=nullptr;

	//_SVS(SysLog(L">KeyMacro::GetKey() InternalInput=%d Executing=%d (%p)",InternalInput,Work.Executing,FrameManager->GetCurrentFrame()));
	if (InternalInput || !FrameManager->GetCurrentFrame())
	{
		//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
		return 0;
	}

	int RetKey=0;  // ������� ������ ������� 0 - ������ � ���, ��� ����������������������� ���

	if (Work.Executing == MACROMODE_NOMACRO)
	{
		if (!Work.MacroWORK)
		{
			if (CurPCStack >= 0)
			{
				//_KEYMACRO(SysLog(L"[%d] if(CurPCStack >= 0)",__LINE__));
				PopState();
				return RetKey;
			}

			if (Mode==MACRO_EDITOR &&
			        IsRedrawEditor &&
			        CtrlObject->Plugins->CurEditor &&
			        CtrlObject->Plugins->CurEditor->IsVisible() &&
			        LockScr)
			{
				CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL,CtrlObject->Plugins->CurEditor->GetId());
				CtrlObject->Plugins->CurEditor->Show();
			}

			if (CurPCStack < 0)
			{
				if (LockScr)
					delete LockScr;

				LockScr=nullptr;
			}

			if (ConsoleTitle::WasTitleModified())
				ConsoleTitle::SetFarTitle(nullptr);

			Clipboard::SetUseInternalClipboardState(false); //??
			//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
			return RetKey;
		}

		/*
		else if(Work.ExecLIBPos>=MR->BufferSize)
		{
			ReleaseWORKBuffer();
			Work.Executing=MACROMODE_NOMACRO;
			return FALSE;
		}
		else
		*/
		//if(Work.MacroWORK)
		{
			Work.Executing=Work.MacroWORK[0].Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
			Work.ExecLIBPos=0; //?????????????????????????????????
		}
		//else
		//	return FALSE;
	}

initial:

	if (!(MR=Work.MacroWORK) || !MR->Buffer)
	{
		//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
		return 0; // RetKey; ?????
	}

	//_SVS(SysLog(L"KeyMacro::GetKey() initial: Work.ExecLIBPos=%d (%d) %p",Work.ExecLIBPos,MR->BufferSize,Work.MacroWORK));

	// ��������! �������� �����!
	if (!Work.ExecLIBPos && !LockScr && (MR->Flags&MFLAGS_DISABLEOUTPUT))
		LockScr=new LockScreen;

begin:

	if (!MR || Work.ExecLIBPos>=MR->BufferSize || !MR->Buffer)
	{
done:

		/*$ 10.08.2000 skv
			If we are in editor mode, and CurEditor defined,
			we need to call this events.
			EE_REDRAW EEREDRAW_ALL    - to notify that whole screen updated
			->Show() to actually update screen.

			This duplication take place since ShowEditor method
			will NOT send this event while screen is locked.
		*/
		if (Mode==MACRO_EDITOR &&
		        IsRedrawEditor &&
		        CtrlObject->Plugins->CurEditor &&
		        CtrlObject->Plugins->CurEditor->IsVisible()
		        /* && LockScr*/) // Mantis#0001595
		{
			CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL,CtrlObject->Plugins->CurEditor->GetId());
			CtrlObject->Plugins->CurEditor->Show();
		}

		if (CurPCStack < 0 && (Work.MacroWORKCount-1) <= 0) // mantis#351
		{
			if (LockScr) delete LockScr;
			LockScr=nullptr;
			if (MR)
				MR->Flags&=~MFLAGS_DISABLEOUTPUT; // ????
		}

		Clipboard::SetUseInternalClipboardState(false); //??
		Work.Executing=MACROMODE_NOMACRO;
		ReleaseWORKBuffer();

		// �������� - "� ���� �� � ��������� ����� ��� �������"?
		if (Work.MacroWORKCount > 0)
		{
			// �������, �������� ��������� �� �����
			Work.ExecLIBPos=0;
		}

		if (ConsoleTitle::WasTitleModified())
			ConsoleTitle::SetFarTitle(nullptr); // �������� ������ ��������� �� ���������� �������

		//FrameManager->RefreshFrame();
		//FrameManager->PluginCommit();
		_KEYMACRO(SysLog(-1); SysLog(L"[%d] **** End Of Execute Macro ****",__LINE__));
		if (--Work.KeyProcess < 0)
			Work.KeyProcess=0;
		_KEYMACRO(SysLog(L"Work.KeyProcess=%d",Work.KeyProcess));

		if (Work.MacroWORKCount <= 0 && CurPCStack >= 0)
		{
			PopState();
			goto initial;
		}

		ScrBuf.RestoreMacroChar();
		Work.HistoryDisable=0;

		StopMacro=false;

		return KEY_NONE; // ����� ������!
	}

	if (!Work.ExecLIBPos)
		Work.Executing=Work.MacroWORK[0].Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;

	// Mantis#0000581: �������� ����������� �������� ���������� �������
	{
		INPUT_RECORD rec;

		//if (PeekInputRecord(&rec) && rec.EventType==KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
		if (StopMacro)
		{
			GetInputRecord(&rec,true);  // ������� �� ������� ��� "�������"...
			Work.KeyProcess=0;
			VMStack.Pop();              // Mantis#0000841 - (TODO: �������� ����� ����� Pop`�� �� ��������, ����� ���������!)
			goto done;                  // ...� ��������� ������.
		}
	}

	DWORD Key=!MR?MCODE_OP_EXIT:GetOpCode(MR,Work.ExecLIBPos++);

	string value;
	_KEYMACRO(SysLog(L"[%d] IP=%d Op=%08X ==> %s or %s",__LINE__,Work.ExecLIBPos-1,Key,_MCODE_ToName(Key),_FARKEY_ToName(Key)));

	if (Work.KeyProcess && Key != MCODE_OP_ENDKEYS)
	{
		_KEYMACRO(SysLog(L"[%d] IP=%d  %s (Work.KeyProcess (%d) && Key != MCODE_OP_ENDKEYS)",__LINE__,Work.ExecLIBPos-1,_FARKEY_ToName(Key),Work.KeyProcess));
		goto return_func;
	}

	switch (Key)
	{
		case MCODE_OP_NOP:
			goto begin;
		case MCODE_OP_KEYS:                    // �� ���� ����� ������� ������ ���� ������
		{
			_KEYMACRO(SysLog(L"MCODE_OP_KEYS (Work.KeyProcess=%d)",Work.KeyProcess));
			Work.KeyProcess++;
			goto begin;
		}
		case MCODE_OP_ENDKEYS:                 // ������ ���� �����������.
		{
			_KEYMACRO(SysLog(L"MCODE_OP_ENDKEYS (Work.KeyProcess=%d)",Work.KeyProcess));
			Work.KeyProcess--;
			goto begin;
		}
		case KEY_ALTINS:
		case KEY_RALTINS:
		{
			if (RunGraber())
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
		case MCODE_F_PRINT:               // N=Print(Str)
		case MCODE_OP_PLAINTEXT:          // $Text "Text"
		{
			if (VMStack.empty())
				return KEY_NONE;

			if (Key == MCODE_F_PRINT)
			{
				parseParams(1,Params);
				__varTextDate=Params[0];
				VMStack.Push(1);
			}
			else
			{
				VMStack.Pop(__varTextDate);
			}
			return KEY_OP_PLAINTEXT;
		}
		case MCODE_OP_EXIT:               // $Exit
		{
			goto done;
		}

		case MCODE_OP_AKEY:               // $AKey
		{
			DWORD aKey=KEY_NONE;
			if (!(MR->Flags&MFLAGS_POSTFROMPLUGIN))
			{
				INPUT_RECORD *inRec=&Work.cRec;
				if (!inRec->EventType)
					inRec->EventType = KEY_EVENT;
				if(inRec->EventType == MOUSE_EVENT || inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
					aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
			}
			else
				aKey=MR->Key;
			return aKey;
		}

		case MCODE_F_AKEY:                // V=akey(Mode[,Type])
		{
			parseParams(2,Params);
			int tmpType=(int)Params[1].getInteger();
			int tmpMode=(int)Params[0].getInteger();

			DWORD aKey=MR->Key;

			if (!tmpType)
			{
				if (!(MR->Flags&MFLAGS_POSTFROMPLUGIN))
				{
					INPUT_RECORD *inRec=&Work.cRec;
					if (!inRec->EventType)
						inRec->EventType = KEY_EVENT;
					if(inRec->EventType == MOUSE_EVENT || inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
						aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
				}
				else if (!aKey)
					aKey=KEY_NONE;
			}

			if (!tmpMode)
				tmpVar=(__int64)aKey;
			else
			{
				KeyToText(aKey,value);
				tmpVar=value.CPtr();
				tmpVar.toString();
			}
			VMStack.Push(tmpVar);
			goto begin;
		}

		case MCODE_F_HISTIORY_DISABLE: // N=History.Disable([State])
		{
		    parseParams(1,Params);
			TVar& State(Params[0]);

			DWORD oldHistoryDisable=Work.HistoryDisable;

			if (!State.isUnknown())
				Work.HistoryDisable=(DWORD)State.getInteger();

			VMStack.Push((__int64)oldHistoryDisable);
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
			LARGE_INTEGER Counter;

			if ((Counter.QuadPart=VMStack.Pop().getInteger()) < 0)
				Counter.QuadPart=0;

			SetOpCode(MR,Work.ExecLIBPos+1,Counter.u.HighPart);
			SetOpCode(MR,Work.ExecLIBPos+2,Counter.u.LowPart);
			SetMacroConst(constRCounter,Counter.QuadPart);
			goto begin;
		}
		case MCODE_OP_REP:
		{
			// ������� ������� �������� ��������
			LARGE_INTEGER Counter ={GetOpCode(MR,Work.ExecLIBPos+1), (LONG)GetOpCode(MR,Work.ExecLIBPos)};
			// � ������� ��� �� ������� �����
			VMStack.Push(Counter.QuadPart);
			SetMacroConst(constRCounter,Counter.QuadPart);
			// �������� ��� � ������ �� MCODE_OP_JZ
			Counter.QuadPart--;
			SetOpCode(MR,Work.ExecLIBPos++,Counter.u.HighPart);
			SetOpCode(MR,Work.ExecLIBPos++,Counter.u.LowPart);
			goto begin;
		}
		case MCODE_OP_END:
			// ������ ��������� ���� �������� ���������� :)
			goto begin;
		case MCODE_F_MMODE:               // N=MMode(Action[,Value])
		{
			parseParams(2,Params);
			__int64 nValue = (__int64)Params[1].getInteger();
			TVar& Action(Params[0]);

			__int64 Result=0;

			switch (Action.getInteger())
			{
				case 1: // DisableOutput
				{
					Result=LockScr?1:0;

					if (nValue == 2) // �������� ����� ����������� ("DisableOutput").
					{
						if (MR->Flags&MFLAGS_DISABLEOUTPUT)
							nValue=0;
						else
							nValue=1;
					}

					switch (nValue)
					{
						case 0: // DisableOutput=0, ��������� �����
							if (LockScr)
							{
								delete LockScr;
								LockScr=nullptr;
							}
							MR->Flags&=~MFLAGS_DISABLEOUTPUT;
							break;
						case 1: // DisableOutput=1, �������� �����
							if (!LockScr)
								LockScr=new LockScreen;
							MR->Flags|=MFLAGS_DISABLEOUTPUT;
							break;
					}

					break;
				}

				case 2: // Get MacroRecord Flags
				{
					Result=(__int64)MR->Flags;
					if ((Result&MFLAGS_MODEMASK) == MACRO_COMMON)
						Result|=0x00FF; // ...��� �� Common ��� ������ ���������.
					break;
				}

				case 3: // CallPlugin Rules
				{
					Result=MR->Flags&MFLAGS_CALLPLUGINENABLEMACRO?1:0;

					if (nValue == 2) // �������� �����
					{
						if (MR->Flags&MFLAGS_CALLPLUGINENABLEMACRO)
							nValue=0;
						else
							nValue=1;
					}

					switch (nValue)
					{
						case 0: // ����������� ������� ��� ������ ������� �������� CallPlugin
							MR->Flags&=~MFLAGS_CALLPLUGINENABLEMACRO;
							break;
						case 1: // ��������� �������
							MR->Flags|=MFLAGS_CALLPLUGINENABLEMACRO;
							break;
					}

					break;
				}
			}

			VMStack.Push(Result);
			break;
		}

		case MCODE_OP_DUP:        // �������������� ������� �������� � �����
			tmpVar=VMStack.Peek();
			VMStack.Push(tmpVar);
			goto begin;

		case MCODE_OP_SWAP:
		{
			VMStack.Swap();
			goto begin;
		}

		case MCODE_OP_DISCARD:    // ������ �������� � ������� �����
			VMStack.Pop();
			goto begin;

		/*
		case MCODE_OP_POP:        // 0: pop 1: varname -> ��������� �������� ���������� � ������ �� ������� �����
		{
			VMStack.Pop(tmpVar);
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);

			if (tmpVarSet)
				tmpVarSet->value=tmpVar;

			goto begin;
		}
        */
		case MCODE_OP_SAVE:
		{
			TVar Val0;
			VMStack.Pop(Val0);    // TODO: �������� �� "Val0=VMStack.Peek();", ��� �������� �� ����� ���� MCODE_OP_DISCARD
			GetPlainText(value);

			// ����� �������� �����, �.�. ���������� ������� ������ �������, ��� ���������� ����������
			if (!value.IsEmpty())
			{
				TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
				varInsert(*t, value)->value = Val0;
			}

			goto begin;
		}
		/*                               ������
			0: MCODE_OP_COPY                 0:   MCODE_OP_PUSHVAR
			1: szVarDest                     1:   VarSrc
			...                              ...
			N: szVarSrc                      N:   MCODE_OP_SAVE
			...                            N+1:   VarDest
			                               N+2:
			                                 ...
		*/
		case MCODE_OP_COPY:       // 0: Copy 1: VarDest 2: VarSrc ==>  %a=%d
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value,true);

			if (tmpVarSet)
				tmpVar=tmpVarSet->value;

			GetPlainText(value);
			t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value,true);

			if (tmpVarSet)
				tmpVar=tmpVarSet->value;

			goto begin;
		}
		case MCODE_OP_PUSHFLOAT:
		{
			union { struct { DWORD l, h; }; double d; } u = {GetOpCode(MR,Work.ExecLIBPos+1), GetOpCode(MR,Work.ExecLIBPos)};
			Work.ExecLIBPos+=2;
			VMStack.Push(u.d);
			goto begin;
		}
		case MCODE_OP_PUSHUNKNOWN:
		case MCODE_OP_PUSHINT: // �������� ����� �������� �� ����.
		{
			LARGE_INTEGER i64 = {GetOpCode(MR,Work.ExecLIBPos+1), (LONG)GetOpCode(MR,Work.ExecLIBPos)};
			Work.ExecLIBPos+=2;
			TVar *ptrVar=VMStack.Push(i64.QuadPart);
			if (Key == MCODE_OP_PUSHUNKNOWN)
				ptrVar->SetType(vtUnknown);
			goto begin;
		}
		case MCODE_OP_PUSHCONST:  // �������� �� ���� ���������.
		{
			GetPlainText(value);
			tmpVarSet=varLook(glbConstTable, value,true);

			if (tmpVarSet)
				VMStack.Push(tmpVarSet->value);
			else
				VMStack.Push(0ll);

			goto begin;
		}
		case MCODE_OP_PUSHVAR: // �������� �� ���� ����������.
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			// %%name - ���������� ����������
			tmpVarSet=varLook(*t, value,true);

			if (tmpVarSet)
				VMStack.Push(tmpVarSet->value);
			else
				VMStack.Push(0ll);

			goto begin;
		}
		case MCODE_OP_PUSHSTR: // �������� �� ���� ������-���������.
		{
			GetPlainText(value);
			VMStack.Push(TVar(value.CPtr()));
			goto begin;
		}
		// ��������
		case MCODE_OP_JMP:
			Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
			goto begin;

		case MCODE_OP_JZ:
		case MCODE_OP_JNZ:
		case MCODE_OP_JLT:
		case MCODE_OP_JLE:
		case MCODE_OP_JGT:
		case MCODE_OP_JGE:
			if(__CheckCondForSkip(VMStack.Pop(),Key))
				Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
			else
				Work.ExecLIBPos++;

			goto begin;

			// ��������
		case MCODE_OP_UPLUS:  /*VMStack.Pop(tmpVar); VMStack.Push(-tmpVar); */ goto begin;
		case MCODE_OP_NEGATE: VMStack.Pop(tmpVar); VMStack.Push(-tmpVar); goto begin;
		case MCODE_OP_NOT:    VMStack.Pop(tmpVar); VMStack.Push(!tmpVar); goto begin;

		case MCODE_OP_LT:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() <  tmpVar); goto begin;
		case MCODE_OP_LE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() <= tmpVar); goto begin;
		case MCODE_OP_GT:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >  tmpVar); goto begin;
		case MCODE_OP_GE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >= tmpVar); goto begin;
		case MCODE_OP_EQ:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() == tmpVar); goto begin;
		case MCODE_OP_NE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() != tmpVar); goto begin;

		case MCODE_OP_ADD:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() +  tmpVar); goto begin;
		case MCODE_OP_SUB:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() -  tmpVar); goto begin;
		case MCODE_OP_MUL:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() *  tmpVar); goto begin;
		case MCODE_OP_DIV:
		{
			if (VMStack.Peek()==0ll)
			{
				_KEYMACRO(SysLog(L"[%d] IP=%d/0x%08X Error: Divide by zero",__LINE__,Work.ExecLIBPos,Work.ExecLIBPos));
				goto done;
			}

			VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() /  tmpVar);
			goto begin;
		}
		case MCODE_OP_PREINC:                  // ++var_a
		case MCODE_OP_PREDEC:                  // --var_a
		case MCODE_OP_POSTINC:                 // var_a++
		case MCODE_OP_POSTDEC:                 // var_a--
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value,true);
			switch (Key)
			{
				case MCODE_OP_PREINC:                  // ++var_a
					++tmpVarSet->value;
					tmpVar=tmpVarSet->value;
					break;
				case MCODE_OP_PREDEC:                  // --var_a
					--tmpVarSet->value;
					tmpVar=tmpVarSet->value;
					break;
				case MCODE_OP_POSTINC:                 // var_a++
					tmpVar=tmpVarSet->value;
					tmpVarSet->value++;
					break;
				case MCODE_OP_POSTDEC:                 // var_a--
					tmpVar=tmpVarSet->value;
					tmpVarSet->value--;
					break;
			}
			VMStack.Push(tmpVar);
			goto begin;
		}

			// Logical
		case MCODE_OP_AND:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() && tmpVar); goto begin;
		case MCODE_OP_OR:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() || tmpVar); goto begin;
		case MCODE_OP_XOR:    VMStack.Pop(tmpVar); VMStack.Push(xor_op(VMStack.Pop(),tmpVar)); goto begin;
			// Bit Op
		case MCODE_OP_BITAND: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() &  tmpVar); goto begin;
		case MCODE_OP_BITOR:  VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() |  tmpVar); goto begin;
		case MCODE_OP_BITXOR: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() ^  tmpVar); goto begin;
		case MCODE_OP_BITSHR: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >> tmpVar); goto begin;
		case MCODE_OP_BITSHL: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() << tmpVar); goto begin;
		case MCODE_OP_BITNOT: VMStack.Pop(tmpVar); VMStack.Push(~tmpVar); goto begin;

		case MCODE_OP_ADDEQ:                   // var_a +=  exp_b
		case MCODE_OP_SUBEQ:                   // var_a -=  exp_b
		case MCODE_OP_MULEQ:                   // var_a *=  exp_b
		case MCODE_OP_DIVEQ:                   // var_a /=  exp_b
		case MCODE_OP_BITSHREQ:                // var_a >>= exp_b
		case MCODE_OP_BITSHLEQ:                // var_a <<= exp_b
		case MCODE_OP_BITANDEQ:                // var_a &=  exp_b
		case MCODE_OP_BITXOREQ:                // var_a ^=  exp_b
		case MCODE_OP_BITOREQ:                 // var_a |=  exp_b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value,true);
			VMStack.Pop(tmpVar);
			switch (Key)
			{
				case MCODE_OP_ADDEQ:    tmpVarSet->value  += tmpVar; break;
				case MCODE_OP_SUBEQ:    tmpVarSet->value  -= tmpVar; break;
				case MCODE_OP_MULEQ:    tmpVarSet->value  *= tmpVar; break;
				case MCODE_OP_BITSHREQ: tmpVarSet->value >>= tmpVar; break;
				case MCODE_OP_BITSHLEQ: tmpVarSet->value <<= tmpVar; break;
				case MCODE_OP_BITANDEQ: tmpVarSet->value  &= tmpVar; break;
				case MCODE_OP_BITXOREQ: tmpVarSet->value  ^= tmpVar; break;
				case MCODE_OP_BITOREQ:  tmpVarSet->value  |= tmpVar; break;
				case MCODE_OP_DIVEQ:
				{
					if (tmpVar == 0ll)
						goto done;
					tmpVarSet->value /= tmpVar;
					break;
				}
			}
			VMStack.Push(tmpVarSet->value);
			goto begin;
		}
			// Function
		case MCODE_F_EVAL: // N=eval(S[,N])
		{
			parseParams(2,Params);
			DWORD Cmd=(DWORD)Params[1].getInteger();
			TVar& Val(Params[0]);
			MacroRecord RBuf={};
			int KeyPos;

			if (!(Val.isInteger() && !Val.i())) // ��������� ������ ���������� ���������� ������ ����������
			{
				int Ret=-1;

				switch (Cmd)
				{
					case 0:
					{
						GetCurRecord(&RBuf,&KeyPos);
						PushState(true);

						if (!(MR->Flags&MFLAGS_DISABLEOUTPUT))
							RBuf.Flags &= ~MFLAGS_DISABLEOUTPUT;

						if (!PostNewMacro(Val.toString(),RBuf.Flags,RBuf.Key))
							PopState();
						else
							Ret=1;
						VMStack.Push((__int64)__getMacroErrorCode());
						break;
					}

					case 1: // ������ ��������? (� ������� ���� ������)
					{
						PostNewMacro(Val.toString(),0,0,TRUE);
						VMStack.Push((__int64)__getMacroErrorCode());
						break;
					}
					case 3: // ������ ��������? (� ������� ������ ������)
					{
						string strResult;
						PostNewMacro(Val.toString(),0,0,TRUE);
						if (__getMacroErrorCode() != err_Success)
						{
							string ErrMsg[4];
							GetMacroParseError(&ErrMsg[0],&ErrMsg[1],&ErrMsg[2],&ErrMsg[3]);
							strResult=ErrMsg[3]+L"\n"+ErrMsg[0]+L"\n"+ErrMsg[1]+L"\n"+ErrMsg[2];
						}
						VMStack.Push(strResult.CPtr());
						break;
					}


					case 2: // ����������� ����� �������, ����������� �� ���������������
					{
						/*
						   ��� �����:
						   �) ������ �������� ������� ���������� � 2
						   �) ������ ���������� ������� ������ � ������� "Area/Key"
						      �����:
						        "Area" - �������, �� ������� ����� ������� ������
						        "/" - �����������
						        "Key" - �������� �������
						      "Area/" ����� �� ���������, � ���� ������ ����� "Key" ����� ������� � ������� �������� ������������,
						         ���� � ������� ������� "Key" �� ������, �� ����� ����������� � ������� Common.
						         ��� �� ��������� ����� � ������� Common (����������� ������ "����" ��������),
						         ���������� � �������� "Area" ������� �����.

						   ��� ������ 2 ������� ������
						     -1 - ������
						     -2 - ��� �������, ��������� ���������������� (��� ������ ������������)
						      0 - Ok
						*/
						int _Mode;
						bool UseCommon=true;
						string strKeyName;
						string strVal=Val.toString();
						strVal=RemoveExternalSpaces(strVal);

						wchar_t *lpwszVal = strVal.GetBuffer();
						wchar_t *p=wcsrchr(lpwszVal,L'/');

						if (p  && p[1])
						{
							*p++=0;
							if ((_Mode = GetAreaCode(lpwszVal)) < MACRO_FUNCS)
							{
								_Mode=GetMode();
								if (lpwszVal[0] == L'.' && !lpwszVal[1]) // ������� "./Key" �� ������������� ����� � Common`�
									UseCommon=false;
							}
							else
								UseCommon=false;
						}
						else
						{
							p=lpwszVal;
							_Mode=GetMode();
						}

						strKeyName=(const wchar_t*)p;
						DWORD KeyCode = KeyNameToKey(p);
						strVal.ReleaseBuffer();

						int I=GetIndex(KeyCode,strKeyName,_Mode,UseCommon);

						if (I != -1 && !(MacroLIB[I].Flags&MFLAGS_DISABLEMACRO)) // && CtrlObject)
						{
							PushState(true);
							// __setMacroErrorCode(err_Success); // ???
							PostNewMacro(MacroLIB+I);
							VMStack.Push((__int64)__getMacroErrorCode()); // ???
							Ret=1;
						}
						else
						{
							VMStack.Push(-2);
						}
						break;
					}
				}

				if (Ret > 0)
					goto initial; // �.�.
			}
			else
				VMStack.Push(-1);
			goto begin;
		}

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
			parseParams(2,Params);
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
				Result=f->VMProcess(Key,ToPtr(p2.i()),p1.i());

			VMStack.Push(Result);
			goto begin;
		}

		case MCODE_F_MENU_ITEMSTATUS:     // N=Menu.ItemStatus([N])
		case MCODE_F_MENU_GETVALUE:       // S=Menu.GetValue([N])
		case MCODE_F_MENU_GETHOTKEY:      // S=gethotkey([N])
		{
			parseParams(1,Params);
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_GETHOTKEY?L"MCODE_F_MENU_GETHOTKEY":(Key == MCODE_F_MENU_ITEMSTATUS?L"MCODE_F_MENU_ITEMSTATUS":L"MCODE_F_MENU_GETVALUE")));
			tmpVar=Params[0];

			if (!tmpVar.isInteger())
				tmpVar=0ll;

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				__int64 Result;

				if (f)
				{
					__int64 MenuItemPos=tmpVar.i()-1;
					if (Key == MCODE_F_MENU_GETHOTKEY)
					{
						if ((Result=f->VMProcess(Key,nullptr,MenuItemPos)) )
						{

							const wchar_t _value[]={static_cast<wchar_t>(Result),0};
							tmpVar=_value;
						}
						else
							tmpVar=L"";
					}
					else if (Key == MCODE_F_MENU_GETVALUE)
					{
						string NewStr;
						if (f->VMProcess(Key,&NewStr,MenuItemPos))
						{
							HiText2Str(NewStr, NewStr);
							RemoveExternalSpaces(NewStr);
							tmpVar=NewStr.CPtr();
						}
						else
							tmpVar=L"";
					}
					else if (Key == MCODE_F_MENU_ITEMSTATUS)
					{
						tmpVar=f->VMProcess(Key,nullptr,MenuItemPos);
					}
				}
				else
					tmpVar=L"";
			}
			else
				tmpVar=L"";

			VMStack.Push(tmpVar);
			goto begin;
		}

		case MCODE_F_MENU_SELECT:      // N=Menu.Select(S[,N[,Dir]])
		case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S[,N])
		{
			parseParams(3,Params);
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_CHECKHOTKEY? L"MCODE_F_MENU_CHECKHOTKEY":L"MCODE_F_MENU_SELECT"));
			__int64 Result=-1;
			__int64 tmpMode=0;
			__int64 tmpDir=0;

			if (Key == MCODE_F_MENU_SELECT)
				tmpDir=Params[2].getInteger();

			tmpMode=Params[1].getInteger();

			if (Key == MCODE_F_MENU_SELECT)
				tmpMode |= (tmpDir << 8);
			else
			{
				if (tmpMode > 0)
					tmpMode--;
			}

			//const wchar_t *checkStr=Params[0].toString();
			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
					Result=f->VMProcess(Key,(void*)Params[0].toString(),tmpMode);
			}

			VMStack.Push(Result);
			goto begin;
		}

		case MCODE_F_MENU_FILTER:      // N=Menu.Filter([Action[,Mode]])
		case MCODE_F_MENU_FILTERSTR:   // S=Menu.FilterStr([Action[,S]])
		{
			parseParams(2,Params);
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_FILTER? L"MCODE_F_MENU_FILTER":L"MCODE_F_MENU_FILTERSTR"));
			bool succees=false;
			TVar& tmpAction(Params[0]);

			tmpVar=Params[1];
			if (tmpAction.isUnknown())
				tmpAction=Key == MCODE_F_MENU_FILTER ? 4 : 0;

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
				{
					if (Key == MCODE_F_MENU_FILTER)
					{
						if (tmpVar.isUnknown())
							tmpVar = -1;
						tmpVar=f->VMProcess(Key,(void*)static_cast<intptr_t>(tmpVar.toInteger()),tmpAction.toInteger());
						succees=true;
					}
					else
					{
						string NewStr;
						if (tmpVar.isString())
							NewStr = tmpVar.toString();
						if (f->VMProcess(Key,(void*)&NewStr,tmpAction.toInteger()))
						{
							tmpVar=NewStr.CPtr();
							succees=true;
						}
					}
				}
			}

			if (!succees)
			{
				if (Key == MCODE_F_MENU_FILTER)
					tmpVar = -1;
				else
					tmpVar = L"";
			}

			VMStack.Push(tmpVar);
			goto begin;
		}

		case MCODE_F_CALLPLUGIN: // V=callplugin(SysID[,param])
		// ����� CallPlugin, ��� ��������
		case MCODE_F_PLUGIN_CALL: // V=Plugin.Call(SysID[,param])
		{
			__int64 Ret=0;
			int count=VMStack.Pop().getInteger();
			if(count-->0)
			{
				FarMacroValue *vParams=nullptr;
				if(count>0)
				{
					vParams=new FarMacroValue[count];
					memset(vParams,0,sizeof(FarMacroValue)*count);
					TVar value;
					for(int ii=count-1;ii>=0;--ii)
					{
						VMStack.Pop(value);
						VarToFarMacroValue(value,*(vParams+ii));
					}
				}

				TVar SysID; VMStack.Pop(SysID);
				GUID guid;

				if (StrToGuid(SysID.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))
				{
					OpenMacroInfo info={sizeof(OpenMacroInfo),(size_t)count,vParams};

					int CallPluginRules=GetCurrentCallPluginMode();

					if( CallPluginRules == 1)
					{
						PushState(true);
						VMStack.Push(1);
					}
					else
						InternalInput++;

					int ResultCallPlugin=0;

					if (CtrlObject->Plugins->CallPlugin(guid,OPEN_FROMMACRO,&info,&ResultCallPlugin))
						Ret=(__int64)ResultCallPlugin;

					if (MR != Work.MacroWORK) // ??? Mantis#0002094 ???
						MR=Work.MacroWORK;

					if( CallPluginRules == 1 )
						PopState();
					else
					{
						VMStack.Push(Ret);
						InternalInput--;
					}
				}
				else
					VMStack.Push(Ret);

				if(vParams)
				{
					for(int ii=0;ii<count;++ii)
					{
						if(vParams[ii].Type == FMVT_STRING && vParams[ii].String)
							xf_free((void*)vParams[ii].String);
					}
				}

				if (Work.Executing == MACROMODE_NOMACRO)
					goto return_func;
			}
			else
				VMStack.Push(Ret);
			goto begin;
		}

		case MCODE_F_PLUGIN_MENU:   // N=Plugin.Menu(Guid[,MenuGuid])
		case MCODE_F_PLUGIN_CONFIG: // N=Plugin.Config(Guid[,MenuGuid])
		case MCODE_F_PLUGIN_COMMAND: // N=Plugin.Command(Guid[,Command])
		{
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_PLUGIN_MENU?L"Plugin.Menu()":(Key == MCODE_F_PLUGIN_CONFIG?L"Plugin.Config()":L"Plugin.Command()")));
			__int64 Ret=0;
			parseParams(2,Params);
			TVar& Arg = (Params[1]);
			TVar& Guid = (Params[0]);
			GUID guid, menuGuid;
			CallPluginInfo Data={CPT_CHECKONLY};
			wchar_t EmptyStr[1]={};
			bool ItemFailed=false;


			switch (Key)
			{
				case MCODE_F_PLUGIN_MENU:
					Data.CallFlags |= CPT_MENU;
					if (!Arg.isUnknown())
					{
						if (StrToGuid(Arg.s(),menuGuid))
							Data.ItemGuid=&menuGuid;
						else
							ItemFailed=true;
					}
					break;
				case MCODE_F_PLUGIN_CONFIG:
					Data.CallFlags |= CPT_CONFIGURE;
					if (!Arg.isUnknown())
					{
						if (StrToGuid(Arg.s(),menuGuid))
							Data.ItemGuid=&menuGuid;
						else
							ItemFailed=true;
					}
					break;
				case MCODE_F_PLUGIN_COMMAND:
					Data.CallFlags |= CPT_CMDLINE;
					if (Arg.isString())
						Data.Command=Arg.s();
					else
						Data.Command=EmptyStr;
					break;
			}

			if (!ItemFailed && StrToGuid(Guid.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))
			{
				// ����� ������� ��������� "����������" ����� ��������� ������� �������/������
				Ret=(__int64)CtrlObject->Plugins->CallPluginItem(guid,&Data);
				VMStack.Push(Ret);

				if (Ret)
				{
					// ���� ����� ������� - �� ������ ����������
					Data.CallFlags&=~CPT_CHECKONLY;
					CtrlObject->Plugins->CallPluginItem(guid,&Data);
					if (MR != Work.MacroWORK)
						MR=Work.MacroWORK;
				}
			}
			else
			{
				VMStack.Push(Ret);
			}

			// �� �������� � KEY_F11
			FrameManager->RefreshFrame();

			if (Work.Executing == MACROMODE_NOMACRO)
				goto return_func;

			goto begin;
		}

		default:
		{
			size_t J;

			for (J=0; J < CMacroFunction; ++J)
			{
				const TMacroFunction *MFunc = KeyMacro::GetMacroFunction(J);
				if (MFunc->Code == (TMacroOpCode)Key && MFunc->Func)
				{
					UINT64 Flags=MR->Flags;

					if (MFunc->IntFlags&IMFF_UNLOCKSCREEN)
					{
						if (Flags&MFLAGS_DISABLEOUTPUT) // ���� ��� - ������
						{
							if (LockScr) delete LockScr;

							LockScr=nullptr;
						}
					}

					if ((MFunc->IntFlags&IMFF_DISABLEINTINPUT))
						InternalInput++;

					MFunc->Func(MFunc);

					if ((MFunc->IntFlags&IMFF_DISABLEINTINPUT))
						InternalInput--;

					if (MFunc->IntFlags&IMFF_UNLOCKSCREEN)
					{
						if (Flags&MFLAGS_DISABLEOUTPUT) // ���� ���� - �������
						{
							if (LockScr) delete LockScr;

							LockScr=new LockScreen;
						}
					}

					break;
				}
			}

			if (J >= CMacroFunction)
			{
				DWORD Err=0;
				tmpVar=FARPseudoVariable(MR->Flags, Key, Err);

				if (!Err)
					VMStack.Push(tmpVar);
				else
				{
					if (Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE)
					{
						// ��� �� �������, � ������������ OpCode, ��������� ���������� �������
						goto done;
					}
					break; // ������� ����� ����������
				}
			}

			goto begin;
		} // END default
	} // END: switch(Key)

return_func:

	if (Work.KeyProcess != 0 && (Key&KEY_ALTDIGIT)) // "����������" ������ ;-)
	{
		Key&=~KEY_ALTDIGIT;
		IntKeyState.ReturnAltValue=1;
	}

#if 0

	if (MR==Work.MacroWORK &&
	        (Work.ExecLIBPos>=MR->BufferSize || Work.ExecLIBPos+1==MR->BufferSize && MR->Buffer[Work.ExecLIBPos]==KEY_NONE) &&
	        Mode==MACRO_DIALOG
	   )
	{
		RetKey=Key;
		goto done;
	}

#else

	if (MR && MR==Work.MacroWORK && Work.ExecLIBPos>=MR->BufferSize)
	{
		_KEYMACRO(SysLog(-1); SysLog(L"[%d] **** End Of Execute Macro ****",__LINE__));
		if (--Work.KeyProcess < 0)
			Work.KeyProcess=0;
		_KEYMACRO(SysLog(L"Work.KeyProcess=%d",Work.KeyProcess));
		ReleaseWORKBuffer();
		Work.Executing=MACROMODE_NOMACRO;

		if (ConsoleTitle::WasTitleModified())
			ConsoleTitle::SetFarTitle(nullptr);
	}

#endif
	return(Key);
}

// ��������� - ���� �� ��� �������?
int KeyMacro::PeekKey()
{
	if (InternalInput || !Work.MacroWORK)
		return 0;

	MacroRecord *MR=Work.MacroWORK;

	if ((Work.Executing == MACROMODE_NOMACRO && !Work.MacroWORK) || Work.ExecLIBPos >= MR->BufferSize)
		return FALSE;

	DWORD OpCode=GetOpCode(MR,Work.ExecLIBPos);
	return OpCode;
}

UINT64 KeyMacro::SwitchFlags(UINT64& Flags,UINT64 Value)
{
	if (Flags&Value) Flags&=~Value;
	else Flags|=Value;

	return Flags;
}


/*
  ����� ������ ���� ������� ����� ������� ������!!!
  ������� ���������� ������ ������� ������������������, �.�.... �������
  � ��������� ������ ���������� Src
*/
wchar_t *KeyMacro::MkTextSequence(DWORD *Buffer,int BufferSize,const wchar_t *Src)
{
	string strMacroKeyText;
	string strTextBuffer;

	if (!Buffer)
		return nullptr;

#if 0

	if (BufferSize == 1)
	{
		if (
		    (((DWORD)(intptr_t)Buffer)&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (((DWORD)(intptr_t)Buffer)&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
		    (((DWORD)(intptr_t)Buffer)&KEY_OP_ENDBASE) >= KEY_OP_BASE && (((DWORD)(intptr_t)Buffer)&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE
		)
		{
			return Src?xf_wcsdup(Src):nullptr;
		}

		if (KeyToText((DWORD)(intptr_t)Buffer,strMacroKeyText))
			return xf_wcsdup(strMacroKeyText.CPtr());

		return nullptr;
	}

#endif
	strTextBuffer.Clear();

	if (Buffer[0] == MCODE_OP_KEYS)
		for (int J=1; J < BufferSize; J++)
		{
			int Key=Buffer[J];

			if (Key == MCODE_OP_ENDKEYS || Key == MCODE_OP_KEYS)
				continue;

			if (/*
				(Key&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (Key&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
				(Key&KEY_OP_ENDBASE) >= KEY_OP_BASE && (Key&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE ||
				*/
			    !KeyToText(Key,strMacroKeyText)
			)
			{
				return Src?xf_wcsdup(Src):nullptr;
			}

			if (J > 1)
				strTextBuffer += L" ";

			strTextBuffer += strMacroKeyText;
		}

	if (!strTextBuffer.IsEmpty())
		return xf_wcsdup(strTextBuffer.CPtr());

	return nullptr;
}

bool KeyMacro::LoadVarFromDB(const wchar_t *Name, TVar &Value)
{
	bool Ret;
	string TempValue, strType;

	Ret=MacroCfg->GetVarValue(Name, TempValue, strType);

	if(Ret)
	{
		Value=TempValue.CPtr();
		switch (GetVarTypeValue(strType))
		{
			case vtUnknown:
				Value.toInteger();
				Value.SetType(vtUnknown);
				break;
			case vtInteger:
				Value.toInteger();
				break;
			case vtDouble:
				Value.toDouble();
				break;
			case vtString:
				break;
			default:
				Value.toString();
				break;
		}

	}

	return Ret;
}

bool KeyMacro::SaveVarToDB(const wchar_t *Name, TVar Value)
{
	bool Ret;

	MacroCfg->BeginTransaction();
	TVarType type=Value.type();
	Ret = MacroCfg->SetVarValue(Name, Value.s(), GetVarTypeName((DWORD)type)) != 0;
	MacroCfg->EndTransaction();

	return Ret;
}

void KeyMacro::WriteVarsConsts()
{
#if 0
	string strValueName;
	TVarTable *t;

	// Consts
	t= &glbConstTable;

	MacroCfg->BeginTransaction();
	for (int I=0; ; I++)
	{
		TVarSet *var=varEnum(*t,I);

		if (!var)
			break;

		strValueName = var->str;
		if(checkMacroFarIntConst(strValueName))
			continue;
		TVarType type=var->value.type();
		MacroCfg->SetConstValue(strValueName, var->value.s(), GetVarTypeName((DWORD)type));
	}
	MacroCfg->EndTransaction();

	// Vars
	t = &glbVarTable;

	MacroCfg->BeginTransaction();
	for (int I=0; ; I++)
	{
		TVarSet *var=varEnum(*t,I);

		if (!var)
			break;

		strValueName = var->str;
		strValueName = L"%"+strValueName;
		TVarType type=var->value.type();
		MacroCfg->SetVarValue(strValueName, var->value.s(), GetVarTypeName((DWORD)type));
	}
	MacroCfg->EndTransaction();
#endif
}

void KeyMacro::SavePluginFunctionToDB(const TMacroFunction *MF)
{
	// ����������������� ��� ����� ������ ���������� �������
	//MacroCfg->SetFunction(MF->Syntax, MF->Name, MF->IntFlags, MF->Name, MF->Name, MF->Name);

	// ���������������� ��� ����� ������ ���������� �������
	if(MF->fnGUID && MF->Name)
		MacroCfg->SetFunction(MF->fnGUID, MF->Name, FlagsToString(MF->IntFlags), nullptr, MF->Syntax, nullptr);
}

void KeyMacro::WritePluginFunctions()
{
	const TMacroFunction *Func;
	MacroCfg->BeginTransaction();
		for (size_t I=0; I < CMacroFunction; ++I)
		{
			Func=GetMacroFunction(I);
			if(Func)
			{
				SavePluginFunctionToDB(Func);
			}
		}
	MacroCfg->EndTransaction();
}

void KeyMacro::SaveMacroRecordToDB(const MacroRecord *MR)
{
	int Area;
	DWORD Flags;

	Flags=MR->Flags;

	Area=Flags & MFLAGS_MODEMASK;
	Flags &= ~(MFLAGS_MODEMASK|MFLAGS_NEEDSAVEMACRO);
	string strKeyName;
	if (MR->Key != -1)
		KeyToText(MR->Key, strKeyName);
	else
		strKeyName=(const wchar_t*)MR->Name;

	MacroCfg->SetKeyMacro(GetAreaName(Area), strKeyName, FlagsToString(Flags), MR->Src, MR->Description);
}

void KeyMacro::WriteMacroRecords()
{
	MacroCfg->BeginTransaction();
	for (int I=0; I<MacroLIBCount; I++)
	{
		if (!MacroLIB[I].BufferSize || !MacroLIB[I].Src)
		{
			string strKeyName;

			if (MacroLIB[I].Key != -1)
				KeyToText(MacroLIB[I].Key, strKeyName);
			else
				strKeyName=(const wchar_t*)MacroLIB[I].Name;

			MacroCfg->DeleteKeyMacro(GetAreaName(MacroLIB[I].Flags & MFLAGS_MODEMASK), strKeyName);
			continue;
		}

		if (!(MacroLIB[I].Flags&MFLAGS_NEEDSAVEMACRO))
			continue;

		SaveMacroRecordToDB(&MacroLIB[I]);
		MacroLIB[I].Flags &= ~MFLAGS_NEEDSAVEMACRO;
	}
	MacroCfg->EndTransaction();
}

// ���������� ���� ��������
void KeyMacro::SaveMacros()
{
	WritePluginFunctions();
	WriteMacroRecords();
}

void KeyMacro::ReadVarsConsts()
{
	string strName;
	string Value, strType;

	while (MacroCfg->EnumConsts(strName, Value, strType))
	{
		TVarSet *NewSet=varInsert(glbConstTable, strName);
		NewSet->value = Value.CPtr();
		switch (GetVarTypeValue(strType))
		{
			case vtUnknown:
				NewSet->value.toInteger();
				NewSet->value.SetType(vtUnknown);
				break;
			case vtInteger:
				NewSet->value.toInteger();
				break;
			case vtDouble:
				NewSet->value.toDouble();
				break;
			case vtString:
				break;
			default:
				NewSet->value.toString();
				break;
		}
	}

	while (MacroCfg->EnumVars(strName, Value, strType))
	{
		TVarSet *NewSet=varInsert(glbVarTable, strName.CPtr()+1);
		NewSet->value = Value.CPtr();
		switch (GetVarTypeValue(strType))
		{
			case vtUnknown:
				NewSet->value.toInteger();
				NewSet->value.SetType(vtUnknown);
				break;
			case vtInteger:
				NewSet->value.toInteger();
				break;
			case vtDouble:
				NewSet->value.toDouble();
				break;
			case vtString:
				break;
			default:
				NewSet->value.toString();
				break;
		}
	}

	initMacroFarIntConst();
}

void KeyMacro::SetMacroConst(const wchar_t *ConstName, const TVar& Value)
{
	varLook(glbConstTable, ConstName,1)->value = Value;
}

/*
   KeyMacros\Function
*/
void KeyMacro::ReadPluginFunctions()
{
	/*
	 � ������� ������� ������ "KeyMacros\Funcs" - ���������� ������������, �������������� ��������� (ProcessMacroW)
     ��� ���������� - ��� ��� "�������"
     �������� � ������� ����������:
       Syntax:reg_sz - ��������� ������� (�� ������� - � �������� ���������)
       Params:reg_dword - ���������� ���������� � �������
       OParams:reg_dword - �������������� ��������� �������
       Sequence:reg_sz - ���� �������
       Flags:reg_dword - �����
       GUID:reg_sz - GUID ��� ���� � ������� � �������� PluginsCache (������� �� Flags)
       Description:reg_sz - �������������� ��������

     Flags - ����� �����
       0: � GUID ���� � �������, ��� � PluginsCache ����� GUID
       1: ������������ Sequence ������ �������; ��� �� ����� �������, ���� GUID ����
       2: ...


     ��������� � ����� �������, ��� � ������� abs, mix, len, etc.
     ���� Plugin �� ����, Sequence ������������.
     Plugin - ��� ���������� �� ����� PluginsCache

	[HKEY_CURRENT_USER\Software\Far2\KeyMacros\Funcs\math.sin]
	"Syntax"="d=sin(V)"
	"nParams"=dword:1
	"oParams"=dword:0
	"Sequence"=""
	"Flags"=dword:0
	"GUID"="C:/Program Files/Far2/Plugins/Calc/bin/calc.dll"
	"Description"="���������� �������� ������ � ������� �����"

	plugin_guid TEXT NOT NULL,
	function_name TEXT NOT NULL,
	nparam INTEGER NOT NULL,
	oparam INTEGER NOT NULL,
	flags TEXT, sequence TEXT,
	syntax TEXT NOT NULL,
	description TEXT

	Flags:
		����:
			0: � GUID ���� � �������, ��� � PluginsCache ����� GUID
			1: ������������ Sequence ������ �������; ��� �� ����� �������, ���� GUID ����
			2:

	$1, $2, $3 - ���������
	*/
#if 1
	string strPluginGUID;
	string strFunctionName;
	unsigned __int64 Flags;
	string strSequence, strFlags;
	string strSyntax;
	string strDescription;

	while (MacroCfg->EnumFunctions(strPluginGUID, strFunctionName, strFlags, strSequence, strSyntax, strDescription))
	{
		RemoveExternalSpaces(strPluginGUID);
		RemoveExternalSpaces(strFunctionName);
		RemoveExternalSpaces(strSequence);
		RemoveExternalSpaces(strSyntax);
		RemoveExternalSpaces(strDescription); //���� �� �������������

		MacroRecord mr={};
		bool UsePluginFunc=true;
		if (!strSequence.IsEmpty())
		{
			if (!ParseMacroString(&mr,strSequence.CPtr()))
				mr.Buffer=0;
		}

		Flags=StringToFlags(strFlags);
		// ������������ Sequence ������ �������; ��� �� ����� �������, ���� GUID ����
		if ((Flags & 2) && (mr.Buffer || strPluginGUID.IsEmpty()))
		{
			UsePluginFunc=false;
		}

		// ���������������� �������
		TMacroFunction MFunc={
			strFunctionName.CPtr(),
			strPluginGUID.CPtr(),
			strSyntax.CPtr(),
			(UsePluginFunc?pluginsFunc:usersFunc),
			mr.Buffer,
			mr.BufferSize,
			0,
			MCODE_F_NOFUNC,
		};

		RegisterMacroFunction(&MFunc);

		if (mr.Buffer)
			xf_free(mr.Buffer);

	}

#endif
}

void KeyMacro::RegisterMacroIntFunction()
{
	static bool InitedInternalFuncs=false;

	if (!InitedInternalFuncs)
	{
		for(size_t I=0; I < ARRAYSIZE(intMacroFunction); ++I)
			KeyMacro::RegisterMacroFunction(intMacroFunction+I);

		InitedInternalFuncs=true;
	}
}

TMacroFunction *KeyMacro::RegisterMacroFunction(const TMacroFunction *tmfunc)
{
	if (!tmfunc->Name || !tmfunc->Name[0])
		return nullptr;

	TMacroOpCode Code = tmfunc->Code;
	if ( !Code || Code == MCODE_F_NOFUNC) // �������� ��������� OpCode ������������ KEY_MACRO_U_BASE
		Code=(TMacroOpCode)GetNewOpCode();

	TMacroFunction *pTemp;

	if (CMacroFunction >= AllocatedFuncCount)
	{
		AllocatedFuncCount=AllocatedFuncCount+64;

		if (!(pTemp=(TMacroFunction *)xf_realloc(AMacroFunction,AllocatedFuncCount*sizeof(TMacroFunction))))
			return nullptr;

		AMacroFunction=pTemp;
	}

	pTemp=AMacroFunction+CMacroFunction;

	pTemp->Name=xf_wcsdup(tmfunc->Name);
	pTemp->fnGUID=tmfunc->fnGUID?xf_wcsdup(tmfunc->fnGUID):nullptr;
	pTemp->Syntax=tmfunc->Syntax?xf_wcsdup(tmfunc->Syntax):nullptr;
	//pTemp->Src=tmfunc->Src?xf_wcsdup(tmfunc->Src):nullptr;
	//pTemp->Description=tmfunc->Description?xf_wcsdup(tmfunc->Description):nullptr;
	pTemp->Code=Code;
	pTemp->BufferSize=tmfunc->BufferSize;

	if (tmfunc->BufferSize > 0)
	{
		pTemp->Buffer=(DWORD *)xf_malloc(sizeof(DWORD)*tmfunc->BufferSize);
		if (pTemp->Buffer)
			memmove(pTemp->Buffer,tmfunc->Buffer,sizeof(DWORD)*tmfunc->BufferSize);
	}
	else
		pTemp->Buffer=nullptr;
	pTemp->IntFlags=tmfunc->IntFlags;
	pTemp->Func=tmfunc->Func;

	CMacroFunction++;
	return pTemp;
}

bool KeyMacro::UnregMacroFunction(size_t Index)
{
	if (static_cast<int>(Index) == -1)
	{
		if (AMacroFunction)
		{
			TMacroFunction *pTemp;
			for (size_t I=0; I < CMacroFunction; ++I)
			{
				pTemp=AMacroFunction+I;
				if (pTemp->Name)        xf_free((void*)pTemp->Name);        pTemp->Name=nullptr;
				if (pTemp->fnGUID)      xf_free((void*)pTemp->fnGUID);      pTemp->fnGUID=nullptr;
				if (pTemp->Syntax)      xf_free((void*)pTemp->Syntax);      pTemp->Syntax=nullptr;
				if (pTemp->Buffer)      xf_free((void*)pTemp->Buffer);      pTemp->Buffer=nullptr;
				//if (pTemp->Src)         xf_free((void*)pTemp->Src);         pTemp->Src=nullptr;
				//if (pTemp->Description) xf_free((void*)pTemp->Description); pTemp->Description=nullptr;
			}
			CMacroFunction=0;
			AllocatedFuncCount=0;
			xf_free(AMacroFunction);
			AMacroFunction=nullptr;
		}
	}
	else
	{
		if (AMacroFunction && Index < CMacroFunction)
			AMacroFunction[Index].Code=MCODE_F_NOFUNC;
		else
			return false;
	}

	return true;
}

const TMacroFunction *KeyMacro::GetMacroFunction(size_t Index)
{
	if (AMacroFunction && Index < CMacroFunction)
		return AMacroFunction+Index;

	return nullptr;
}

size_t KeyMacro::GetCountMacroFunction()
{
	return CMacroFunction;
}

DWORD KeyMacro::GetNewOpCode()
{
	return LastOpCodeUF++;
}

int KeyMacro::ReadKeyMacro(int Area)
{
	MacroRecord CurMacro={};
	int Key;
	unsigned __int64 MFlags=0;
	string strKey,strArea,strMFlags;
	string strSequence, strDescription;
	string strGUID;
	int ErrorCount=0;

	strArea=GetAreaName(static_cast<MACROMODEAREA>(Area));

	while(MacroCfg->EnumKeyMacros(strArea, strKey, strMFlags, strSequence, strDescription))
	{
		RemoveExternalSpaces(strKey);
		Key=KeyNameToKey(strKey);
		//if (Key==-1)
		//	continue;

		RemoveExternalSpaces(strSequence);
		RemoveExternalSpaces(strDescription);

		if (strSequence.IsEmpty())
		{
			//ErrorCount++; // �������������, ���� �� ����������� ������ "Sequence"
			continue;
		}

		MFlags=StringToFlags(strMFlags);

		CurMacro.Key=Key;
		CurMacro.Buffer=nullptr;
		CurMacro.Src=nullptr;
		CurMacro.Description=nullptr;
		CurMacro.BufferSize=0;
		CurMacro.Flags=MFlags|(Area&MFLAGS_MODEMASK);

		if (Area == MACRO_EDITOR || Area == MACRO_DIALOG || Area == MACRO_VIEWER)
		{
			if (CurMacro.Flags&MFLAGS_SELECTION)
			{
				CurMacro.Flags&=~MFLAGS_SELECTION;
				CurMacro.Flags|=MFLAGS_EDITSELECTION;
			}

			if (CurMacro.Flags&MFLAGS_NOSELECTION)
			{
				CurMacro.Flags&=~MFLAGS_NOSELECTION;
				CurMacro.Flags|=MFLAGS_EDITNOSELECTION;
			}
		}

		if (!ParseMacroString(&CurMacro,strSequence))
		{
			ErrorCount++;
			continue;
		}

		MacroRecord *NewMacros=(MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));

		if (!NewMacros)
		{
			return FALSE;
		}

		MacroLIB=NewMacros;
		CurMacro.Src=xf_wcsdup(strSequence);
		if (!strDescription.IsEmpty())
		{
			CurMacro.Description=xf_wcsdup(strDescription);
		}
		CurMacro.Name=xf_wcsdup(strKey);

		GUID Guid=FarGuid;
		// BUGBUG!
		/*if (GetRegKey(strRegKeyName,L"GUID",strGUID,L"",&regType))
		{
			if(!StrToGuid(strGUID,Guid))
				Guid=FarGuid;
		}*/
		CurMacro.Guid=Guid;

		MacroLIB[MacroLIBCount]=CurMacro;
		MacroLIBCount++;
	}

	return ErrorCount?FALSE:TRUE;
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
	if (Opt.Macro.DisableMacro&MDOL_ALL)
		return;

	if (Opt.Macro.DisableMacro&MDOL_AUTOSTART)
		return;

	// �������� ������� ������ �������
#if 1

	if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !Opt.OnlyEditorViewerUsed && CtrlObject->Plugins->IsPluginsLoaded()))
		return;

	static int IsRunStartMacro=FALSE;

	if (IsRunStartMacro)
		return;

	if (!IndexMode[MACRO_SHELL][1])
		return;

	MacroRecord *MR=MacroLIB+IndexMode[MACRO_SHELL][0];

	for (int I=0; I < IndexMode[MACRO_SHELL][1]; ++I)
	{
		UINT64 CurFlags;

		if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==MACRO_SHELL &&
		        MR[I].BufferSize>0 &&
		        // ��������� �� ������������� �������
		        !(CurFlags&MFLAGS_DISABLEMACRO) &&
		        (CurFlags&MFLAGS_RUNAFTERFARSTART) && CtrlObject)
		{
			if (CheckAll(MACRO_SHELL,CurFlags))
				PostNewMacro(MR+I);
		}
	}

	IsRunStartMacro=TRUE;

#else
	static int AutoRunMacroStarted=FALSE;

	if (AutoRunMacroStarted || !MacroLIB || !IndexMode[Mode][1])
		return;

	//if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !Opt.OnlyEditorViewerUsed && CtrlObject->Plugins->IsPluginsLoaded()))
	if (!(CtrlObject && CtrlObject->Plugins->IsPluginsLoaded()))
		return;

	MacroRecord *MR=MacroLIB+IndexMode[Mode][0];

	for (int I=0; I < IndexMode[Mode][1]; ++I)
	{
		DWORD CurFlags;

		if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==Mode &&   // ���� ������ �� ���� �����?
		        MR[I].BufferSize > 0 &&                             // ���-�� ������ ����
		        !(CurFlags&MFLAGS_DISABLEMACRO) &&                  // ��������� �� ������������� �������
		        (CurFlags&MFLAGS_RUNAFTERFARSTART) &&               // � ���� ��, ��� ������ ����������
		        !(CurFlags&MFLAGS_RUNAFTERFARSTARTED)      // � ��� �����, ������� ��� �� ����������
		   )
		{
			if (CheckAll(Mode,CurFlags)) // ������ ��� ��������� - �������� �����
			{
				PostNewMacro(MR+I);
				MR[I].Flags|=MFLAGS_RUNAFTERFARSTARTED; // ���� ������ ������� �������� �� �����
			}
		}
	}

	// ��������� ���������� ���������� �������������� ��������
	int CntStart=0;

	for (int I=0; I < MacroLIBCount; ++I)
		if ((MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTART) && !(MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTARTED))
			CntStart++;

	if (!CntStart) // ������ ����� �������, ��� ��� ���������� � � ������� RunStartMacro() ������ ������
		AutoRunMacroStarted=TRUE;

#endif

	if (Work.Executing == MACROMODE_NOMACRO)
		Work.ExecLIBPos=0;  // � ���� ��?
}
#endif

// ���������� ����������� ���� ���������� �������
intptr_t WINAPI KeyMacro::AssignMacroDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
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
		}
	}

	//_SVS(SysLog(L"LastKey=%d Msg=%s",LastKey,_DLGMSG_ToName(Msg)));
	if (Msg == DN_INITDIALOG)
	{
		KMParam=reinterpret_cast<DlgParam*>(Param2);
		LastKey=0;
		// <�������, ������� �� ������� � ������� ����������>
		DWORD PreDefKeyMain[]=
		{
			KEY_CTRLDOWN,KEY_RCTRLDOWN,KEY_ENTER,KEY_NUMENTER,KEY_ESC,KEY_F1,KEY_CTRLF5,KEY_RCTRLF5,
		};

		for (size_t i=0; i<ARRAYSIZE(PreDefKeyMain); i++)
		{
			KeyToText(PreDefKeyMain[i],strKeyText);
			SendDlgMessage(hDlg,DM_LISTADDSTR,2,const_cast<wchar_t*>(strKeyText.CPtr()));
		}

		DWORD PreDefKey[]=
		{
			KEY_MSWHEEL_UP,KEY_MSWHEEL_DOWN,KEY_MSWHEEL_LEFT,KEY_MSWHEEL_RIGHT,
			KEY_MSLCLICK,KEY_MSRCLICK,KEY_MSM1CLICK,KEY_MSM2CLICK,KEY_MSM3CLICK,
#if 0
			KEY_MSLDBLCLICK,KEY_MSRDBLCLICK,KEY_MSM1DBLCLICK,KEY_MSM2DBLCLICK,KEY_MSM3DBLCLICK,
#endif
		};
		DWORD PreDefModKey[]=
		{
			0,KEY_CTRL,KEY_RCTRL,KEY_SHIFT,KEY_ALT,KEY_RALT,KEY_CTRLSHIFT,KEY_RCTRLSHIFT,KEY_CTRLALT,KEY_RCTRLRALT,KEY_CTRLRALT,KEY_RCTRLALT,KEY_ALTSHIFT,KEY_RALTSHIFT,
		};

		for (size_t i=0; i<ARRAYSIZE(PreDefKey); i++)
		{
			SendDlgMessage(hDlg,DM_LISTADDSTR,2,const_cast<wchar_t*>(L"\1"));

			for (size_t j=0; j<ARRAYSIZE(PreDefModKey); j++)
			{
				KeyToText(PreDefKey[i]|PreDefModKey[j],strKeyText);
				SendDlgMessage(hDlg,DM_LISTADDSTR,2,const_cast<wchar_t*>(strKeyText.CPtr()));
			}
		}

		/*
		int KeySize=GetRegKeySize("KeyMacros","DlgKeys");
		char *KeyStr;
		if(KeySize &&
			(KeyStr=(char*)xf_malloc(KeySize+1))  &&
			GetRegKey("KeyMacros","DlgKeys",KeyStr,"",KeySize)
		)
		{
			UserDefinedList KeybList;
			if(KeybList.Set(KeyStr))
			{
				KeybList.Start();
				const char *OneKey;
				*KeyText=0;
				while(nullptr!=(OneKey=KeybList.GetNext()))
				{
					xstrncpy(KeyText, OneKey, sizeof(KeyText));
					SendDlgMessage(hDlg,DM_LISTADDSTR,2,(long)KeyText);
				}
			}
			xf_free(KeyStr);
		}
		*/
		SendDlgMessage(hDlg,DM_SETTEXTPTR,2,nullptr);
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
		KeyMacro *MacroDlg=KMParam->Handle;

		if ((key&0x00FFFFFF) > 0x7F && (key&0x00FFFFFF) < 0xFFFF)
			key=KeyToKeyLayout((int)(key&0x0000FFFF))|(DWORD)(key&(~0x0000FFFF));

		if (key<0xFFFF)
		{
			key=Upper(static_cast<wchar_t>(key));
		}

		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),LastKey?_FARKEY_ToName(LastKey):L""));
		KMParam->Key=(DWORD)key;
		KeyToText((int)key,strKeyText);
		string strKey;

#ifdef FAR_LUA
		// ���� ��� ���� ����� ������...
		int Area,Index;
		if ((Index=MacroDlg->GetIndex(&Area,(int)key,strKeyText,KMParam->Mode,true,true)) != -1)
		{
			MacroRecord *Mac = MacroDlg->m_Macros[Area].getItem(Index);

			// ����� ������� ��������� ������ ��� ��������.
			if (MacroDlg->m_RecCode.IsEmpty() || Area!=MACRO_COMMON)
			{
				string strBufKey=Mac->Code();
				InsertQuote(strBufKey);

				bool DisFlags = (Mac->Flags()&MFLAGS_DISABLEMACRO) != 0;
				bool SetChange = MacroDlg->m_RecCode.IsEmpty();
				LangString strBuf;
				if (Area==MACRO_COMMON)
				{
					strBuf = SetChange ? (DisFlags?MMacroCommonDeleteAssign:MMacroCommonDeleteKey) : MMacroCommonReDefinedKey;
					//"����� ������������ '%1'     �� �������              : ����� �������         : ��� ����������."
				}
				else
				{
					strBuf = SetChange ? (DisFlags?MMacroDeleteAssign:MMacroDeleteKey) : MMacroReDefinedKey;
					//"������������ '%1'           �� �������        : ����� �������   : ��� ����������."
				}
				strBuf << strKeyText;

				// �������� "� �� ��������� �� ��?"
				int Result=0;
				if (DisFlags || MacroDlg->m_RecCode != Mac->Code())
				{
					const wchar_t* NoKey=MSG(DisFlags && !SetChange?MMacroDisAnotherKey:MNo);
					Result=Message(MSG_WARNING,SetChange?3:2,MSG(MWarning),
					          strBuf,
					          MSG(MMacroSequence),
					          strBufKey,
					          MSG(SetChange?MMacroDeleteKey2:
					              (DisFlags?MMacroDisDisabledKey:MMacroReDefinedKey2)),
					          MSG(DisFlags && !SetChange?MMacroDisOverwrite:MYes),
					          (SetChange?MSG(MMacroEditKey):NoKey),
					          (!SetChange?nullptr:NoKey));
				}

				if (!Result)
				{
					if (DisFlags)
					{
						// ������� �� DB ������ ���� ������� ��������
						if (Opt.AutoSaveSetup)
						{
							MacroCfg->BeginTransaction();
							// ������ ������ ������ �� DB
							MacroCfg->DeleteKeyMacro(GetAreaName(Area), Mac->Name());
							MacroCfg->EndTransaction();
						}
						// �����������
						Mac->m_flags&=~(MFLAGS_DISABLEMACRO|MFLAGS_NEEDSAVEMACRO);
					}

					// � ����� ������ - ������������
					SendDlgMessage(hDlg,DM_CLOSE,1,0);
					return TRUE;
				}
				else if (SetChange && Result == 1)
				{
					string strDescription;

					if ( !Mac->Code().IsEmpty() )
						strBufKey=Mac->Code();

					if ( !Mac->Description().IsEmpty() )
						strDescription=Mac->Description();

					if (MacroDlg->GetMacroSettings(key,Mac->m_flags,strBufKey,strDescription))
					{
						KMParam->Flags = Mac->m_flags;
						KMParam->Changed = true;
						// � ����� ������ - ������������
						SendDlgMessage(hDlg,DM_CLOSE,1,0);
						return TRUE;
					}
				}

				// ����� - ����� �� �������� "���", �� � �� ��� � ���� ���
				//  � ������ ������� ���� �����.
				strKeyText.Clear();
			}
		}
#else
		// ���� ��� ���� ����� ������...
		int Index;
		if ((Index=MacroDlg->GetIndex((int)key,strKey,KMParam->Mode,true,true)) != -1)
		{
			MacroRecord *Mac=MacroDlg->MacroLIB+Index;

			// ����� ������� ��������� ������ ��� ��������.
			if (!MacroDlg->RecBuffer || !MacroDlg->RecBufferSize || (Mac->Flags&0xFF)!=MACRO_COMMON)
			{
				string strBufKey;
				if (Mac->Src )
				{
					strBufKey=Mac->Src;
					InsertQuote(strBufKey);
				}

				DWORD DisFlags=Mac->Flags&MFLAGS_DISABLEMACRO;
				LangString strBuf;
				if ((Mac->Flags&MFLAGS_MODEMASK)==MACRO_COMMON)
				{
					strBuf = !MacroDlg->RecBufferSize? (DisFlags? MMacroCommonDeleteAssign : MMacroCommonDeleteKey) : MMacroCommonReDefinedKey;
				}
				else
				{
					strBuf = !MacroDlg->RecBufferSize? (DisFlags?MMacroDeleteAssign : MMacroDeleteKey) : MMacroReDefinedKey;
				}
				strBuf << strKeyText;

				// �������� "� �� ��������� �� ��?"
				int Result=0;
				bool SetChange=!MacroDlg->RecBufferSize;
				if (!(!DisFlags &&
				        Mac->Buffer && MacroDlg->RecBuffer &&
				        Mac->BufferSize == MacroDlg->RecBufferSize &&
				        (
				            (Mac->BufferSize >  1 && !memcmp(Mac->Buffer,MacroDlg->RecBuffer,MacroDlg->RecBufferSize*sizeof(DWORD))) ||
				            (Mac->BufferSize == 1 && (DWORD)(intptr_t)Mac->Buffer == (DWORD)(intptr_t)MacroDlg->RecBuffer)
				        )
				   ))
				{
					const wchar_t* NoKey=MSG(DisFlags && !SetChange?MMacroDisAnotherKey:MNo);
					Result=Message(MSG_WARNING,SetChange?3:2,MSG(MWarning),
					          strBuf,
					          MSG(MMacroSequence),
					          strBufKey,
					          MSG(SetChange?MMacroDeleteKey2:
					              (DisFlags?MMacroDisDisabledKey:MMacroReDefinedKey2)),
					          MSG(DisFlags && !SetChange?MMacroDisOverwrite:MYes),
					          (SetChange?MSG(MMacroEditKey):NoKey),
					          (!SetChange?nullptr:NoKey));
				}

				if (!Result)
				{
					if (DisFlags)
					{
						// ������� �� DB ������ ���� ������� ��������
						if (Opt.AutoSaveSetup)
						{
							MacroCfg->BeginTransaction();
							// ������ ������ ������ �� DB
							string strKeyName;
							KeyToText(Mac->Key, strKeyName);
							MacroCfg->DeleteKeyMacro(GetAreaName(Mac->Flags&MFLAGS_MODEMASK), strKeyName);
							MacroCfg->EndTransaction();
						}
						// �����������
						Mac->Flags&=~(MFLAGS_DISABLEMACRO|MFLAGS_NEEDSAVEMACRO);
					}

					// � ����� ������ - ������������
					SendDlgMessage(hDlg,DM_CLOSE,1,0);
					return TRUE;
				}
				else if (SetChange && Result == 1)
				{
					string strDescription;

					if ( Mac->Src )
						strBufKey=Mac->Src;

					if ( Mac->Description )
						strDescription=Mac->Description;

					if (MacroDlg->GetMacroSettings(key,Mac->Flags,strBufKey,strDescription))
					{
						KMParam->Flags = Mac->Flags;
						KMParam->Changed = true;
						// � ����� ������ - ������������
						SendDlgMessage(hDlg,DM_CLOSE,1,0);
						return TRUE;
					}
				}

				// ����� - ����� �� �������� "���", �� � �� ��� � ���� ���
				//  � ������ ������� ���� �����.
				strKeyText.Clear();
			}
		}
#endif

		KMParam->Recurse++;
		SendDlgMessage(hDlg,DM_SETTEXTPTR,2,const_cast<wchar_t*>(strKeyText.CPtr()));
		KMParam->Recurse--;
		//if(key == KEY_F1 && LastKey == KEY_F1)
		//LastKey=-1;
		//else
		LastKey=(int)key;
		return TRUE;
	}
	return DefDlgProc(hDlg,Msg,Param1,Param2);
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
	MakeDialogItemsEx(MacroAssignDlgData,MacroAssignDlg);
	DlgParam Param={Flags, this, 0, StartMode, 0, false};
	//_SVS(SysLog(L"StartMode=%d",StartMode));
	IsProcessAssignMacroKey++;
	Dialog Dlg(MacroAssignDlg,ARRAYSIZE(MacroAssignDlg),AssignMacroDlgProc,&Param);
	Dlg.SetPosition(-1,-1,34,6);
	Dlg.SetHelp(L"KeyMacro");
	Dlg.Process();
	IsProcessAssignMacroKey--;

	if (Dlg.GetExitCode() == -1)
		return 0;

	MacroKey = Param.Key;
	Flags = Param.Flags;
	return Param.Changed ? 2 : 1;
}

#ifdef FAR_LUA
#else
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

intptr_t WINAPI KeyMacro::ParamMacroDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	static DlgParam *KMParam=nullptr;

	switch (Msg)
	{
		case DN_INITDIALOG:
			KMParam=(DlgParam *)Param2;
			break;
		case DN_BTNCLICK:

			if (Param1==MS_CHECKBOX_A_PANEL || Param1==MS_CHECKBOX_P_PANEL)
				for (int i=1; i<=3; i++)
					SendDlgMessage(hDlg,DM_ENABLE,Param1+i,Param2);

			break;
		case DN_CLOSE:

			if (Param1==MS_BUTTON_OK)
			{
				MacroRecord mr={};
				KeyMacro *Macro=KMParam->Handle;
				LPCWSTR Sequence=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_SEQUENCE,0);

				if (*Sequence)
				{
					if (Macro->ParseMacroString(&mr,Sequence))
					{
						xf_free(Macro->RecBuffer);
						Macro->RecBufferSize=mr.BufferSize;
						Macro->RecBuffer=mr.Buffer;
						Macro->RecSrc=xf_wcsdup(Sequence);
						LPCWSTR Description=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_DESCR,0);
						Macro->RecDescription=xf_wcsdup(Description);
						return TRUE;
					}
				}

				return FALSE;
			}

			break;

		default:
			break;
	}

#if 0
	else if (Msg==DN_KEY && Param2==KEY_ALTF4)
	{
		KeyMacro *MacroDlg=KMParam->Handle;
		(*FrameManager)[0]->UnlockRefresh();
		FILE *MacroFile;
		char MacroFileName[NM];

		if (!FarMkTempEx(MacroFileName) || !(MacroFile=fopen(MacroFileName,"wb")))
			return TRUE;

		char *TextBuffer;
		DWORD Buf[1];
		Buf[0]=MacroDlg->RecBuffer[0];

		if ((TextBuffer=MacroDlg->MkTextSequence((MacroDlg->RecBufferSize==1?Buf:MacroDlg->RecBuffer),MacroDlg->RecBufferSize)) )
		{
			fwrite(TextBuffer,strlen(TextBuffer),1,MacroFile);
			fclose(MacroFile);
			xf_free(TextBuffer);
			{
				//ConsoleTitle *OldTitle=new ConsoleTitle;
				FileEditor ShellEditor(MacroFileName,-1,FFILEEDIT_DISABLEHISTORY,-1,-1,nullptr);
				//delete OldTitle;
				ShellEditor.SetDynamicallyBorn(false);
				FrameManager->EnterModalEV();
				FrameManager->ExecuteModal();
				FrameManager->ExitModalEV();

				if (!ShellEditor.IsFileChanged() || !(MacroFile=fopen(MacroFileName,"rb")))
					;
				else
				{
					MacroRecord NewMacroWORK2={};
					long FileSize=filelen(MacroFile);
					TextBuffer=(char*)xf_malloc(FileSize);

					if (TextBuffer)
					{
						fread(TextBuffer,FileSize,1,MacroFile);

						if (!MacroDlg->ParseMacroString(&NewMacroWORK2,TextBuffer))
						{
							if (NewMacroWORK2.BufferSize > 1)
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
	return DefDlgProc(hDlg,Msg,Param1,Param2);
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

		{DI_TEXT,3,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(MMacroSettingsEnableOutput)},
		{DI_CHECKBOX,5,8,0,8,0,nullptr,nullptr,0,MSG(MMacroSettingsRunAfterStart)},
		{DI_TEXT,3,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsActivePanel)},
		{DI_CHECKBOX,7,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,7,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,7,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_CHECKBOX,37,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsPassivePanel)},
		{DI_CHECKBOX,39,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,39,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,39,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_TEXT,3,14,0,14,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,15,0,15,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsCommandLine)},
		{DI_CHECKBOX,5,16,0,16,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsSelectionBlockPresent)},
		{DI_TEXT,3,17,0,17,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(MacroSettingsDlgData,MacroSettingsDlg);
	string strKeyText;
	KeyToText(Key,strKeyText);
	MacroSettingsDlg[MS_DOUBLEBOX].strData = LangString(MMacroSettingsTitle) << strKeyText;
	//if(!(Key&0x7F000000))
	//MacroSettingsDlg[3].Flags|=DIF_DISABLE;
	MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected=Flags&MFLAGS_DISABLEOUTPUT?0:1;
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
		LPWSTR Sequence=MkTextSequence(RecBuffer,RecBufferSize);
		MacroSettingsDlg[MS_EDIT_SEQUENCE].strData=Sequence;
		xf_free(Sequence);
	}

	MacroSettingsDlg[MS_EDIT_DESCR].strData=(Descr && *Descr)?Descr:RecDescription;

	DlgParam Param={0, this, 0, 0, 0, false};
	Dialog Dlg(MacroSettingsDlg,ARRAYSIZE(MacroSettingsDlg),ParamMacroDlgProc,&Param);
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

	Flags=MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected?0:MFLAGS_DISABLEOUTPUT;
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

int KeyMacro::PostNewMacro(const wchar_t *PlainText,UINT64 Flags,DWORD AKey,bool onlyCheck)
{
	MacroRecord NewMacroWORK2={};
	wchar_t *Buffer=(wchar_t *)PlainText;
	bool allocBuffer=false;

	// ������� ������� �� ������
	BOOL parsResult=ParseMacroString(&NewMacroWORK2,Buffer,onlyCheck);

	if (allocBuffer && Buffer)
		xf_free(Buffer);

	if (!parsResult)
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return FALSE;
	}

	if (onlyCheck)
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return TRUE;
	}

	NewMacroWORK2.Flags=Flags;
	NewMacroWORK2.Key=AKey;
	// ������ ��������� �������� ������� ������ ������
	MacroRecord *NewMacroWORK;

	if (!(NewMacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))))
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return FALSE;
	}

	// ������ ������� � ���� "�������" ����� ������
	Work.MacroWORK=NewMacroWORK;
	NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
	*NewMacroWORK=NewMacroWORK2;
	Work.MacroWORKCount++;

	//Work.Executing=Work.MacroWORK[0].Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
	if (Work.ExecLIBPos > Work.MacroWORK[0].BufferSize)
		Work.ExecLIBPos=0;

	return TRUE;
}

int KeyMacro::PostNewMacro(MacroRecord *MRec,BOOL NeedAddSendFlag,bool IsPluginSend)
{
	if (!MRec)
		return FALSE;

	MacroRecord NewMacroWORK2=*MRec;
	NewMacroWORK2.Src=nullptr;
	NewMacroWORK2.Name=nullptr; //???
	NewMacroWORK2.Description=nullptr;
	//if(MRec->BufferSize > 1)
	{
		if (!(NewMacroWORK2.Buffer=(DWORD*)xf_malloc((MRec->BufferSize+3)*sizeof(DWORD))))
		{
			return FALSE;
		}
	}
	// ������ ��������� �������� ������� ������ ������
	MacroRecord *NewMacroWORK;

	if (!(NewMacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))))
	{
		//if(MRec->BufferSize > 1)
		xf_free(NewMacroWORK2.Buffer);
		return FALSE;
	}

	// ������ ������� � ���� "�������" ����� ������
	if (IsPluginSend)
		NewMacroWORK2.Buffer[0]=MCODE_OP_KEYS;

	if ((MRec->BufferSize+1) > 2)
		memcpy(&NewMacroWORK2.Buffer[IsPluginSend?1:0],MRec->Buffer,sizeof(DWORD)*MRec->BufferSize);
	else if (MRec->Buffer)
		NewMacroWORK2.Buffer[IsPluginSend?1:0]=(DWORD)(intptr_t)MRec->Buffer;

	if (IsPluginSend)
		NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize+1]=MCODE_OP_ENDKEYS;

	//NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize]=MCODE_OP_NOP; // ���.�������/��������

	if (IsPluginSend)
		NewMacroWORK2.BufferSize+=2;

	Work.MacroWORK=NewMacroWORK;
	NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
	*NewMacroWORK=NewMacroWORK2;
	Work.MacroWORKCount++;

	//Work.Executing=Work.MacroWORK[0].Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
	if (Work.ExecLIBPos > Work.MacroWORK[0].BufferSize)
		Work.ExecLIBPos=0;

	return TRUE;
}

int KeyMacro::ParseMacroString(MacroRecord *CurMacro,const wchar_t *BufPtr,bool onlyCheck)
{
	BOOL Result=FALSE;

	if (CurMacro)
	{
		Result=__parseMacroString(CurMacro->Buffer, CurMacro->BufferSize, BufPtr);

		if (!Result && !onlyCheck)
		{
			// TODO: ���� ����� ������ ������������ ����������� ������ SILENT!
			bool scrLocks=LockScr!=nullptr;
			string ErrMsg[4];

			if (scrLocks) // ���� ��� - ������
			{
				if (LockScr) delete LockScr;

				LockScr=nullptr;
			}

			InternalInput++; // InternalInput - ������������ ����, ����� ������ �� ��������� ���� ����������
			GetMacroParseError(&ErrMsg[0],&ErrMsg[1],&ErrMsg[2],&ErrMsg[3]);
			//if(...)
			string strTitle=MSG(MMacroPErrorTitle);
			if(CurMacro->Key)
			{
				strTitle+=L" ";
				string strKey;
				KeyToText(CurMacro->Key,strKey);
				strTitle.Append(GetAreaName(CurMacro->Flags&MFLAGS_MODEMASK)).Append(L"\\").Append(strKey);
			}
			Message(MSG_WARNING|MSG_LEFTALIGN,1,strTitle,ErrMsg[3]+L":",ErrMsg[0],L"\x1",ErrMsg[1],ErrMsg[2],L"\x1",MSG(MOk));
			//else
			// ������� ����������� � ����
			InternalInput--;

			if (scrLocks) // ���� ���� - �������
			{
				if (LockScr) delete LockScr;

				LockScr=new LockScreen;
			}
		}
	}

	return Result;
}


void MacroState::Init(TVarTable *tbl)
{
	KeyProcess=Executing=MacroPC=ExecLIBPos=MacroWORKCount=0;
	HistoryDisable=0;
	MacroWORK=nullptr;

	if (!tbl)
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
	_KEYMACRO(CleverSysLog Clev(L"KeyMacro::PushState()"));
	if (CurPCStack+1 >= STACKLEVEL)
		return FALSE;

	++CurPCStack;
	Work.UseInternalClipboard=Clipboard::GetUseInternalClipboardState();
	PCStack[CurPCStack]=Work;
	Work.Init(CopyLocalVars?PCStack[CurPCStack].locVarTable:nullptr);
	_KEYMACRO(SysLog(L"CurPCStack=%d",CurPCStack));
	return TRUE;
}

int KeyMacro::PopState()
{
	_KEYMACRO(CleverSysLog Clev(L"KeyMacro::PopState()"));
	if (CurPCStack < 0)
		return FALSE;

	Work=PCStack[CurPCStack];
	Clipboard::SetUseInternalClipboardState(Work.UseInternalClipboard);
	CurPCStack--;
	_KEYMACRO(SysLog(L"CurPCStack=%d",CurPCStack));
	return TRUE;
}

// ������� ��������� ������� ������� ������� � �������
// Ret=-1 - �� ������ �������.
// ���� CheckMode=-1 - ������ ������ � ����� ������, �.�. ������ ����������
// StrictKeys=true - �� �������� ��������� ����� Ctrl/Alt ������ (���� ����� �� �����)
int KeyMacro::GetIndex(int Key, string& strKey, int CheckMode, bool UseCommon, bool StrictKeys)
{
	if (MacroLIB)
	{
		int KeyParam=Key;
		for (int I=0; I < 2; ++I)
		{
			int Len;
			MacroRecord *MPtr=nullptr;
			Key=KeyParam;

			if (CheckMode == -1)
			{
				Len=MacroLIBCount;
				MPtr=MacroLIB;
			}
			else if (CheckMode >= 0 && CheckMode < MACRO_LAST)
			{
				Len=IndexMode[CheckMode][1];

				if (Len)
					MPtr=MacroLIB+IndexMode[CheckMode][0];

				//_SVS(SysLog(L"CheckMode=%d (%d,%d)",CheckMode,IndexMode[CheckMode][0],IndexMode[CheckMode][1]));
			}
			else
			{
				Len=0;
			}

			if (Len)
			{
				int ctrl = 0;
				if (Key != -1)
					ctrl =(!StrictKeys && (Key&(KEY_RCTRL|KEY_RALT)) && !(Key&(KEY_CTRL|KEY_ALT))) ? 0 : 1;
				MacroRecord *MPtrSave=MPtr;

				for (; ctrl < 2; ctrl++)
				{
					for (int Pos=0; Pos < Len; ++Pos, ++MPtr)
					{
						if (Key != -1)
						{
							if (!((MPtr->Key ^ Key) & ~0xFFFF) &&
							        (Upper(static_cast<WCHAR>(MPtr->Key))==Upper(static_cast<WCHAR>(Key))) &&
							        (MPtr->BufferSize > 0))
							{
								//        && (CheckMode == -1 || (MPtr->Flags&MFLAGS_MODEMASK) == CheckMode))
								//_SVS(SysLog(L"GetIndex: Pos=%d MPtr->Key=0x%08X", Pos,MPtr->Key));
								if (!(MPtr->Flags&MFLAGS_DISABLEMACRO))
								{
								    if(!MPtr->Callback||MPtr->Callback(MPtr->Id,AKMFLAGS_NONE))
								    	return Pos+((CheckMode >= 0)?IndexMode[CheckMode][0]:0);
								}
							}
						}
						else if (!strKey.IsEmpty() && !StrCmpI(strKey,MPtr->Name))
						{
							if (MPtr->BufferSize > 0)
							{
								if (!(MPtr->Flags&MFLAGS_DISABLEMACRO))
								{
								    if(!MPtr->Callback||MPtr->Callback(MPtr->Id,AKMFLAGS_NONE))
								    	return Pos+((CheckMode >= 0)?IndexMode[CheckMode][0]:0);
								}
							}
						}
					}

					if (Key != -1)
					{
						if (!ctrl)
						{
							if (Key & KEY_RCTRL)
							{
								Key &= ~KEY_RCTRL;
								Key |= KEY_CTRL;
							}
							if (Key & KEY_RALT)
							{
								Key &= ~KEY_RALT;
								Key |= KEY_ALT;
							}
							MPtr = MPtrSave;
						}
					}
					else
						MPtr = MPtrSave;
				}
			}

			// ����� ������� �� MACRO_COMMON
			if (CheckMode != -1 && !I && UseCommon)
				CheckMode=MACRO_COMMON;
			else
				break;
		}
	}

	return -1;
}

#if 0
// ��������� �������, ����������� ��������� ��������
// Ret= 0 - �� ������ �������.
// ���� CheckMode=-1 - ������ ������ � ����� ������, �.�. ������ ����������
int KeyMacro::GetRecordSize(int Key, int CheckMode)
{
	//BUGBUG: StrictKeys?
	string strKey;
	int Pos=GetIndex(Key,strKey,CheckMode);

	if (Pos == -1)
		return 0;

	return sizeof(MacroRecord)+MacroLIB[Pos].BufferSize;
}
#endif

// �������� �������� ���� �� ����
const wchar_t* KeyMacro::GetAreaName(int AreaCode)
{
	return (AreaCode >= MACRO_FUNCS && AreaCode < MACRO_LAST)?MKeywordsArea[AreaCode+3].Name:L"";
}

// �������� ��� ���� �� �����
int KeyMacro::GetAreaCode(const wchar_t *AreaName)
{
	for (int i=MACRO_FUNCS; i < MACRO_LAST; i++)
		if (!StrCmpI(MKeywordsArea[i+3].Name,AreaName))
			return i;

	return MACRO_FUNCS-1;
}

int KeyMacro::GetMacroKeyInfo(bool FromDB, int Mode, int Pos, string &strKeyName, string &strDescription)
{
	if (Mode >= MACRO_FUNCS && Mode < MACRO_LAST)
	{
		if (FromDB)
		{
			if (Mode >= MACRO_OTHER)
			{
				// TODO
				return Pos+1;
			}
			else if (Mode == MACRO_FUNCS)
			{
				// TODO: MACRO_FUNCS
				return -1;
			}
			else
			{
				// TODO
				return Pos+1;
			}
		}
		else
		{
			if (Mode >= MACRO_OTHER)
			{
				int Len=CtrlObject->Macro.IndexMode[Mode][1];

				if (Len && Pos < Len)
				{
					MacroRecord *MPtr=CtrlObject->Macro.MacroLIB+CtrlObject->Macro.IndexMode[Mode][0]+Pos;
					if (MPtr->Key != -1)
						::KeyToText(MPtr->Key,strKeyName);
					else
						strKeyName=MPtr->Name;
					strDescription=NullToEmpty(MPtr->Description);
					return Pos+1;
				}
			}
			else if (Mode == MACRO_FUNCS)
			{
				// TODO: MACRO_FUNCS
				return -1;
			}
			else
			{
				TVarSet *var=varEnum((Mode==MACRO_VARS)?glbVarTable:glbConstTable,Pos);

				if (!var)
					return -1;

				strKeyName = var->str;
				strKeyName = (Mode==MACRO_VARS?L"%":L"")+strKeyName;

				switch (var->value.type())
				{
					case vtInteger:
					{
						__int64 IData64=var->value.i();
						strDescription.Format(L"%I64d (0x%I64X)", IData64, IData64);
						break;
					}
					case vtDouble:
					{
						double FData=var->value.d();
						strDescription.Format(L"%g", FData);
						break;
					}
					case vtString:
						strDescription.Format(L"\"%s\"", var->value.s());
						break;
					default:
						break;
				}

				return Pos+1;
			}
		}
	}

	return -1;
}

BOOL KeyMacro::CheckEditSelected(UINT64 CurFlags)
{
	if (Mode==MACRO_EDITOR || Mode==MACRO_DIALOG || Mode==MACRO_VIEWER || (Mode==MACRO_SHELL&&CtrlObject->CmdLine->IsVisible()))
	{
		int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS));
		Frame* CurFrame=FrameManager->GetCurrentFrame();

		if (CurFrame && CurFrame->GetType()==NeedType)
		{
			int CurSelected;

			if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
				CurSelected=(int)CtrlObject->CmdLine->VMProcess(MCODE_C_SELECTED);
			else
				CurSelected=(int)CurFrame->VMProcess(MCODE_C_SELECTED);

			if (((CurFlags&MFLAGS_EDITSELECTION) && !CurSelected) ||	((CurFlags&MFLAGS_EDITNOSELECTION) && CurSelected))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL KeyMacro::CheckInsidePlugin(UINT64 CurFlags)
{
	if (CtrlObject && CtrlObject->Plugins->CurPluginItem && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS)) // ?????
		//if(CtrlObject && CtrlObject->Plugins->CurEditor && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS))
		return FALSE;

	return TRUE;
}

BOOL KeyMacro::CheckCmdLine(int CmdLength,UINT64 CurFlags)
{
	if (((CurFlags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength) || ((CurFlags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0))
		return FALSE;

	return TRUE;
}

BOOL KeyMacro::CheckPanel(int PanelMode,UINT64 CurFlags,BOOL IsPassivePanel)
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

BOOL KeyMacro::CheckFileFolder(Panel *CheckPanel,UINT64 CurFlags, BOOL IsPassivePanel)
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

BOOL KeyMacro::CheckAll(int /*CheckMode*/,UINT64 CurFlags)
{
	/* $TODO:
		����� ������ Check*() ����������� ������� IfCondition()
		��� ���������� �������������� ����.
	*/
	if (!CheckInsidePlugin(CurFlags))
		return FALSE;

	// �������� �� �����/�� ����� � ���.������ (� � ���������? :-)
	if (CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
		if (CtrlObject->CmdLine && !CheckCmdLine(CtrlObject->CmdLine->GetLength(),CurFlags))
			return FALSE;

	FilePanels *Cp=CtrlObject->Cp();

	if (!Cp)
		return FALSE;

	// �������� ������ � ���� �����
	Panel *ActivePanel=Cp->ActivePanel;
	Panel *PassivePanel=Cp->GetAnotherPanel(Cp->ActivePanel);

	if (ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACRO_SHELL)
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
			if (Mode!=MACRO_EDITOR && Mode != MACRO_DIALOG && Mode!=MACRO_VIEWER)
			{
				size_t SelCount=ActivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_SELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_NOSELECTION) && SelCount >= 1))
					return FALSE;

				SelCount=PassivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_PSELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_PNOSELECTION) && SelCount >= 1))
					return FALSE;
			}
	}

	if (!CheckEditSelected(CurFlags))
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
	if (Work.Executing && Work.MacroWORK)
	{
		return (Work.MacroWORK[0].Flags&Flags)?TRUE:FALSE;
	}

	return FALSE;
}

/*
  Return: 0 - �� � ������ �����, 1 - Executing, 2 - Executing common, 3 - Recording, 4 - Recording common
  See MacroRecordAndExecuteType
*/
int KeyMacro::GetCurRecord(MacroRecord* RBuf,int *KeyPos)
{
	if (KeyPos && RBuf)
	{
		*KeyPos=Work.Executing?Work.ExecLIBPos:0;
		ClearStruct(*RBuf);

		if (Recording == MACROMODE_NOMACRO)
		{
			if (Work.Executing)
			{
				*RBuf=*Work.MacroWORK;   //MacroLIB[Work.MacroPC]; //????
				return Work.Executing;
			}

			ClearStruct(*RBuf);
			return MACROMODE_NOMACRO;
		}

		RBuf->BufferSize=RecBufferSize;
		RBuf->Buffer=RecBuffer;

		return Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON;
	}

	return Recording?(Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON):(Work.Executing?Work.Executing:MACROMODE_NOMACRO);
}

DWORD KeyMacro::SetHistoryDisableMask(DWORD Mask)
{
	DWORD OldHistoryDisable=Work.HistoryDisable;
	Work.HistoryDisable=Mask;
	return OldHistoryDisable;
}

DWORD KeyMacro::GetHistoryDisableMask()
{
	return Work.HistoryDisable;
}

bool KeyMacro::IsHistoryDisable(int TypeHistory)
{
	return (Work.HistoryDisable & (1 << TypeHistory))? true : false;
}

static int __cdecl SortMacros(const MacroRecord *el1,const MacroRecord *el2)
{
	int result=(el1->Flags&MFLAGS_MODEMASK)-(el2->Flags&MFLAGS_MODEMASK);
	if (result==0)
	{
		result=memcmp(&el1->Guid,&el2->Guid,sizeof(GUID));
		if (result==0)
		{
			result=static_cast<int>(static_cast<char*>(el1->Id)-static_cast<char*>(el2->Id));
}
	}
	return result;
}

// ���������� ��������� ������
void KeyMacro::Sort()
{
	typedef int (__cdecl *qsort_fn)(const void*,const void*);
	// ���������
	far_qsort(MacroLIB,MacroLIBCount,sizeof(MacroRecord),(qsort_fn)SortMacros);
	// ������������� ������ �����
	int CurMode=MACRO_OTHER;
	ClearArray(IndexMode);

	for (int I=0; I<MacroLIBCount; I++)
	{
		int J=MacroLIB[I].Flags&MFLAGS_MODEMASK;

		if (CurMode != J)
		{
			IndexMode[J][0]=I;
			CurMode=J;
		}

		IndexMode[J][1]++;
	}

	//_SVS(for(I=0; I < ARRAYSIZE(IndexMode); ++I)SysLog(L"IndexMode[%02d.%s]=%d,%d",I,GetAreaName(I),IndexMode[I][0],IndexMode[I][1]));
}

DWORD KeyMacro::GetOpCode(MacroRecord *MR,int PC)
{
	DWORD OpCode=(MR->BufferSize > 1)?MR->Buffer[PC]:(DWORD)(intptr_t)MR->Buffer;
	return OpCode;
}

// function for Mantis#0000968
bool KeyMacro::CheckWaitKeyFunc()
{
	if (InternalInput || !Work.MacroWORK || Work.Executing == MACROMODE_NOMACRO)
		return false;

	MacroRecord *MR=Work.MacroWORK;

	if (Work.ExecLIBPos >= MR->BufferSize || Work.ExecLIBPos <= 0)
		return false;

	return (GetOpCode(MR,Work.ExecLIBPos-1) == MCODE_F_WAITKEY)?true:false;
}

// ������ OpCode � �����. ���������� ���������� ��������
DWORD KeyMacro::SetOpCode(MacroRecord *MR,int PC,DWORD OpCode)
{
	DWORD OldOpCode;

	if (MR->BufferSize > 1)
	{
		OldOpCode=MR->Buffer[PC];
		MR->Buffer[PC]=OpCode;
	}
	else
	{
		OldOpCode=(DWORD)(intptr_t)MR->Buffer;
		MR->Buffer=(DWORD*)(intptr_t)OpCode;
	}

	return OldOpCode;
}

// ��� ��� ����� ��� ���:
// BugZ#873 - ACTL_POSTKEYSEQUENCE � ��������� ����
int KeyMacro::IsExecutingLastKey()
{
	if (Work.Executing && Work.MacroWORK)
	{
		return (Work.ExecLIBPos == Work.MacroWORK[0].BufferSize-1);
	}

	return FALSE;
}


void KeyMacro::SendDropProcess()
{
	if (Work.Executing)
		StopMacro=true;
}

void KeyMacro::DropProcess()
{
	if (Work.Executing)
	{
		if (LockScr) delete LockScr;

		LockScr=nullptr;
		Clipboard::SetUseInternalClipboardState(false); //??
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
	if (global)
	{
		initVTable(glbVarTable);
		initVTable(glbConstTable); //???
	}
}

void doneMacroVarTable(int global)
{
	if (global)
	{
		deleteVTable(glbVarTable);
		deleteVTable(glbConstTable); //???
	}
}

BOOL KeyMacro::GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc)
{
	return __getMacroParseError(ErrCode,ErrPos,ErrSrc);
}

BOOL KeyMacro::GetMacroParseError(string *Err1, string *Err2, string *Err3, string *Err4)
{
	return __getMacroParseError(Err1, Err2, Err3, Err4);
}

// ��� OpCode (�� ����������� MCODE_OP_ENDKEYS)?
bool KeyMacro::IsOpCode(DWORD p)
{
	return (!(p&KEY_MACRO_BASE) || p == MCODE_OP_ENDKEYS)?false:true;
}

int KeyMacro::AddMacro(const wchar_t *PlainText,const wchar_t *Description,enum MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback)
{
	if (Area < MACRO_OTHER || Area > MACRO_COMMON)
		return FALSE;

	MacroRecord CurMacro={};
	CurMacro.Flags=Area;
	CurMacro.Flags|=Flags;
	CurMacro.Key=InputRecordToKey(&AKey);
	string strKeyText;
	if (KeyToText(CurMacro.Key, strKeyText))
		CurMacro.Name=xf_wcsdup(strKeyText);
	else
		CurMacro.Name=nullptr;
	CurMacro.Src=xf_wcsdup(PlainText);
	CurMacro.Description=xf_wcsdup(Description);
	CurMacro.Guid=PluginId;
	CurMacro.Id=Id;
	CurMacro.Callback=Callback;
	if (ParseMacroString(&CurMacro,PlainText,false))
	{
		MacroRecord *NewMacroLIB=(MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));
		if (NewMacroLIB)
		{
			MacroLIB=NewMacroLIB;
			MacroLIB[MacroLIBCount]=CurMacro;
			++MacroLIBCount;
			KeyMacro::Sort();
			return TRUE;
		}
	}
	return FALSE;
}

int KeyMacro::DelMacro(const GUID& PluginId,void* Id)
{
	if (MacroLIB)
	{
		for (int Area=MACRO_OTHER; Area < MACRO_LAST; ++Area)
		{
			size_t size=IndexMode[Area][0]+IndexMode[Area][1];
			for(size_t ii=IndexMode[Area][0];ii<size;++ii)
			{
				if(MacroLIB[ii].Id==Id && IsEqualGUID(MacroLIB[ii].Guid,PluginId))
				{
					DelMacro(ii);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void KeyMacro::DelMacro(size_t Index)
{
	if (MacroLIB[Index].BufferSize > 1 && MacroLIB[Index].Buffer)
		xf_free(MacroLIB[Index].Buffer);
	if (MacroLIB[Index].Src)
		xf_free(MacroLIB[Index].Src);
	if (MacroLIB[Index].Name)
		xf_free(MacroLIB[Index].Name);
	if (MacroLIB[Index].Description)
		xf_free(MacroLIB[Index].Description);
	memcpy(MacroLIB+Index,MacroLIB+Index+1,(MacroLIBCount-Index-1)*sizeof(MacroLIB[0]));
	--MacroLIBCount;
	KeyMacro::Sort();
}

int KeyMacro::GetCurrentCallPluginMode()
{
	int Ret=-1;
	MacroRecord *MR=Work.MacroWORK;
	if (MR)
	{
		Ret=MR->Flags&MFLAGS_CALLPLUGINENABLEMACRO?1:0;
	}
	return Ret;
}

TVarTable *KeyMacro::GetLocalVarTable()
{
	if (CtrlObject)
		return CtrlObject->Macro.Work.locVarTable;

	return nullptr;
}
#endif
