/*
findfile.cpp

����� (Alt-F7)
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

#include "findfile.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "editor.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "namelist.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
#include "filefilter.hpp"
#include "farexcpt.hpp"

#define DLG_HEIGHT 23
#define DLG_WIDTH 74
#define PARTIAL_DLG_STR_LEN 30
#define CHAR_TABLE_SIZE 5

#define LIST_DELTA  64
static DWORD LIST_INDEX_NONE = (DWORD)-1;

// ������ ��������� ������. ������ �� ������ �������� � ����.
static LPFINDLIST  *FindList;
static DWORD       FindListCapacity;
static DWORD       FindListCount;
// ������ �������. ���� ���� ������ � ������, �� FindList->ArcIndex ��������� ����.
static LPARCLIST   *ArcList;
static DWORD       ArcListCapacity;
static DWORD       ArcListCount;

static CriticalSection ffCS; // ����� �� ��������� ������ ��������� ������, ���� �� �� ���� ������
static CriticalSection statusCS; // ����� �� ������ � FindMessage/FindFileCount/FindDirCount ���� �� ������ ������


static DWORD FindFileArcIndex;
// ������������ ��� �������� ������ �� ��������� ������.
// ������ �������� �������� � ������ � ���� ��� ��������.
static DWORD FindExitIndex;
enum {FIND_EXIT_NONE, FIND_EXIT_SEARCHAGAIN, FIND_EXIT_GOTO, FIND_EXIT_PANEL};
static int FindExitCode;
//static char FindFileArcName[NM];

static string strFindMask, strFindStr;
static int SearchMode,CmpCase,WholeWords,SearchInArchives,SearchHex;

static int FindFoldersChanged;
static int SearchFromChanged;
static int DlgWidth,DlgHeight;
static volatile int StopSearch,PauseSearch,SearchDone,LastFoundNumber,FindFileCount,FindDirCount,WriteDataUsed,PrepareFilesListUsed;
static string strFindMessage;
static string strLastDirName;
static int FindMessageReady,FindCountReady,FindPositionChanged;
static string strPluginSearchPath;
static HANDLE hDlg;
static int RecurseLevel;
static int BreakMainThread;
static int PluginMode;

static HANDLE hPluginMutex;

static int UseAllTables=FALSE,UseDecodeTable=FALSE,UseANSI=FALSE,UseUnicode=FALSE,TableNum=0,UseFilter=0;
static int EnableSearchInFirst=FALSE;
static __int64 SearchInFirst=_i64(0);
static struct CharTableSet TableSet;

/* $ 01.07.2001 IS
   ������ "����� ������". ������ ��� ����� ������������ ��� �������� �����
   ����� �� ���������� � �������.
*/
static CFileMask FileMaskForFindFile;

/* $ 05.10.2003 KM
   ��������� �� ������ ������� ��������
*/
static FileFilter *Filter;

int _cdecl SortItems(const void *p1,const void *p2)
{
  PluginPanelItem *Item1=(PluginPanelItem *)p1;
  PluginPanelItem *Item2=(PluginPanelItem *)p2;
  string strN1, strN2;
  if (*Item1->FindData.lpwszFileName)
    strN1 = Item1->FindData.lpwszFileName;
  if (*Item2->FindData.lpwszFileName)
    strN2 = Item2->FindData.lpwszFileName;

  CutToSlash(strN1);
  CutToSlash(strN2);
  return StrCmpI(strN2,strN1);
}

LONG_PTR WINAPI FindFiles::MainDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  const wchar_t *FindText=MSG(MFindFileText),*FindHex=MSG(MFindFileHex),*FindCode=MSG(MFindFileCodePage);
  string strDataStr;

  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      /* $ 21.09.2003 KM
         ������������ ��������� ������ ����� �������� ������
         � ����������� �� Dlg->Item[13].Selected
      */
      if (Dlg->Item[13]->Selected) // [ ] Search for hexadecimal code
      {
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,FALSE);
      }
      else
      {
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,TRUE);
      }

      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,5,1);
      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,6,1);

      int W=Dlg->Item[7]->X1-Dlg->Item[4]->X1-5;

      if (StrLength((Dlg->Item[13]->Selected?FindHex:FindText))>W)
      {
        strDataStr = Dlg->Item[13]->Selected?FindHex:FindText;
        TruncStr(strDataStr, W);
      }
      else
        strDataStr = (Dlg->Item[13]->Selected?FindHex:FindText);

      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(LONG_PTR)(const wchar_t*)strDataStr);

      W=Dlg->Item[0]->X2-Dlg->Item[7]->X1-3;
      if (StrLength(FindCode)>W)
      {
        strDataStr = FindCode;
        TruncStr(strDataStr, W);
      }
      else
        strDataStr = FindCode;
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(LONG_PTR)(const wchar_t*)strDataStr);

      /* ��������� ����������� ����� ���������� */
      UseAllTables=Opt.CharTable.AllTables;
      UseANSI=Opt.CharTable.AnsiTable;
      UseUnicode=Opt.CharTable.UnicodeTable;
      UseDecodeTable=((UseAllTables==0) && (UseUnicode==0) && (UseANSI==0) && (Opt.CharTable.TableNum>0));
      if (UseDecodeTable)
        TableNum=Opt.CharTable.TableNum-1;
      else
        TableNum=0;
      /* -------------------------------------- */

      string strTableName;

      if (UseAllTables)
        strTableName = MSG(MFindFileAllTables);
      else if (UseUnicode)
        strTableName = L"Unicode";
      else if (UseANSI)
      {
        GetTable(&TableSet,TRUE,TableNum,UseUnicode);
        strTableName = MSG(MGetTableWindowsText);
      }
      else if (!UseDecodeTable)
        strTableName = MSG(MGetTableNormalText);
      else
        PrepareTable(&TableSet,TableNum,TRUE);
      RemoveChar(strTableName,L'&',TRUE);

      UnicodeToAnsi (strTableName, TableSet.TableName, sizeof(TableSet.TableName)); //BUGBUG

      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,8,(LONG_PTR)(const wchar_t*)strTableName);

      FindFoldersChanged = FALSE;
      SearchFromChanged=FALSE;

      if (Dlg->Item[23]->Selected==1)
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,32,TRUE);
      else
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,32,FALSE);

      return TRUE;
    }
    case DN_LISTCHANGE:
    {
      if (Param1==8)
      {
        /* $ 20.09.2003 KM
           ������� ��������� ANSI �������
        */
        UseAllTables=(Param2==0);
        UseANSI=(Param2==3);
        UseUnicode=(Param2==4);
        UseDecodeTable=(Param2>=(CHAR_TABLE_SIZE+1));
        TableNum=(int)Param2-(CHAR_TABLE_SIZE+1);

        string strTableName;

        if (UseAllTables)
          strTableName = MSG(MFindFileAllTables);
        else if (UseUnicode)
          strTableName = L"Unicode";
        else if (UseANSI)
        {
          GetTable(&TableSet,TRUE,TableNum,UseUnicode);
          strTableName = MSG(MGetTableWindowsText);
        }
        else if (!UseDecodeTable)
          strTableName = MSG(MGetTableNormalText);
        else
          PrepareTable(&TableSet,TableNum,TRUE);

        if ( !strTableName.IsEmpty() ) // BUGBUG
        	UnicodeToAnsi (strTableName, TableSet.TableName, sizeof(TableSet.TableName));
      }
      return TRUE;
    }
    /* 22.11.2001 VVM
      ! ������������������� FindFolders ��� ����� ������.
        �� ������ ���� �� ������ ���� ��������� ������� */
    case DN_BTNCLICK:
    {
      Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
      string strSearchFromRoot;

      /* $ 23.11.2001 KM
         - ����! ������ ���� � ���� ������� :-(
           ��������� �������� �������� � �����������.
      */
      /* $ 22.11.2001 KM
         - ������ ������, ��� DN_BTNCLICK �������� �� ����� "�������������"
           ����������, ������ � �������� ������� ��-�������, ������� �������
           ������� ENTER �� ������� Find ��� Cancel �� � ���� �� ���������.
      */
      if (Param1==31 || Param1==35) // [ Find ] ��� [ Cancel ]
        return FALSE;
      else if (Param1==32) // [ Drive ]
      {
        IsRedrawFramesInProcess++;
        ActivePanel->ChangeDisk();
        // �� ��� �, ��� ����� ����� ������ ��������� ������
        // ����� ����� ��������.
        //FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
        FrameManager->ResizeAllFrame();
        IsRedrawFramesInProcess--;

        PrepareDriveNameStr(strSearchFromRoot, PARTIAL_DLG_STR_LEN);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,23,(LONG_PTR)(const wchar_t*)strSearchFromRoot);
        PluginMode=CtrlObject->Cp()->ActivePanel->GetMode()==PLUGIN_PANEL;
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,PluginMode?FALSE:TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,20,PluginMode?FALSE:TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,21,PluginMode?FALSE:TRUE);
      }
      else if (Param1==33) // Filter
        Filter->FilterEdit();
      else if (Param1==34) // Advanced
        AdvancedDialog();
      else if (Param1>=20 && Param1<=26)
      {
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,32,Param1==23?TRUE:FALSE);
        SearchFromChanged=TRUE;
      }
      else if (Param1==15) // [ ] Search for folders
        FindFoldersChanged = TRUE;
      else if (Param1==13) // [ ] Search for hexadecimal code
      {
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        /* $ 21.09.2003 KM
           ������������ ��������� ������ ����� �������� ������
           � ����������� �� �������������� �������� hex mode
        */
        //int LenDataStr=sizeof(DataStr); //BUGBUG
        if (Param2)
        {
          Transform(strDataStr,Dlg->Item[5]->strData,L'X');
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,6,(LONG_PTR)(const wchar_t*)strDataStr);

          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,FALSE);
        }
        else
        {
          Transform(strDataStr,Dlg->Item[6]->strData,L'S');
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,5,(LONG_PTR)(const wchar_t*)strDataStr);

          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,TRUE);
        }

        int W=Dlg->Item[7]->X1-Dlg->Item[4]->X1-5;
        if (StrLength((Param2?FindHex:FindText))>W)
        {
          strDataStr = (Param2?FindHex:FindText);
          TruncStr(strDataStr, W);
        }
        else
          strDataStr = (Param2?FindHex:FindText);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(LONG_PTR)(const wchar_t*)strDataStr);

        if (strDataStr.GetLength()>0)
        {
          int UnchangeFlag=(int)Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,5,-1);
          Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,6,UnchangeFlag);
        }

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      else if (Param1==29) // [ ] Advanced options
        EnableSearchInFirst=(int)Param2;
      return TRUE;
    }
    case DN_EDITCHANGE:
    {
      FarDialogItem &Item=*reinterpret_cast<FarDialogItem*>(Param2);

      if (Param1==5)
      {
        if(!FindFoldersChanged)
        // ������ "���������� �����"
        {

          BOOL Checked = (Item.PtrData && *Item.PtrData)?FALSE:Opt.FindOpt.FindFolders;
          if (Checked)
            Dialog::SendDlgMessage(hDlg, DM_SETCHECK, 15, BSTATE_CHECKED);
          else
            Dialog::SendDlgMessage(hDlg, DM_SETCHECK, 15, BSTATE_UNCHECKED);
        }
      }
      return TRUE;
    }
    case DN_HOTKEY:
    {
      if (Param1==4)
      {
        if (Dlg->Item[13]->Selected)
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,6,0);
        else
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,5,0);
        return FALSE;
      }
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

FindFiles::FindFiles()
{
  _ALGO(CleverSysLog clv(L"FindFiles::FindFiles()"));
  static string strLastFindMask=L"*.*", strLastFindStr;
  // ����������� ��������� � ����������� ����������
  static string strSearchFromRoot;
  static int LastCmpCase=0,LastWholeWords=0,LastSearchInArchives=0,LastSearchHex=0;
  int I;

  // �������� ������ �������
  Filter=new FileFilter(CtrlObject->Cp()->ActivePanel,FFT_FINDFILE);

  CmpCase=LastCmpCase;
  WholeWords=LastWholeWords;
  SearchInArchives=LastSearchInArchives;
  SearchHex=LastSearchHex;
  SearchMode=Opt.FindOpt.FileSearchMode;
  UseFilter=Opt.FindOpt.UseFilter;

  // ��-��������� �������������� ��������� �� ������������
  EnableSearchInFirst=FALSE;

  strFindMask = strLastFindMask;
  strFindStr = strLastFindStr;
  BreakMainThread=0;

  strSearchFromRoot = MSG(MSearchFromRootFolder);

  FarList TableList;
  FarListItem *TableItem=(FarListItem *)xf_malloc(sizeof(FarListItem)*CHAR_TABLE_SIZE);
  TableList.Items=TableItem;
  TableList.ItemsNumber=CHAR_TABLE_SIZE;

  memset(TableItem,0,sizeof(FarListItem)*CHAR_TABLE_SIZE);
  TableItem[0].Text=MSG(MFindFileAllTables);
  TableItem[1].Flags=LIF_SEPARATOR;
  TableItem[2].Text=MSG(MGetTableNormalText);
  TableItem[3].Text=MSG(MGetTableWindowsText);
  TableItem[4].Text=L"Unicode";

  for (I=0;;I++)
  {
    CharTableSet cts;
    int RetVal=FarCharTable(I,(char *)&cts,sizeof(cts));
    if (RetVal==-1)
      break;

    if (I==0)
    {
      TableItem=(FarListItem *)xf_realloc(TableItem,sizeof(FarListItem)*(CHAR_TABLE_SIZE+1));
      if (TableItem==NULL)
        return;
      memset(&TableItem[CHAR_TABLE_SIZE],0,sizeof(FarListItem));
      TableItem[CHAR_TABLE_SIZE].Flags=LIF_SEPARATOR;
      TableList.Items=TableItem;
      TableList.ItemsNumber++;
    }

    TableItem=(FarListItem *)xf_realloc(TableItem,sizeof(FarListItem)*(I+CHAR_TABLE_SIZE+2));
    if (TableItem==NULL)
      return;
    memset(&TableItem[I+CHAR_TABLE_SIZE+1],0,sizeof(FarListItem));

    int BufSize=(int)strlen(cts.TableName)+1;
    wchar_t* TableName=(wchar_t*)xf_malloc(BufSize*sizeof(wchar_t));
    OEMToUnicode(cts.TableName,TableName,BufSize);

    TableItem[I+CHAR_TABLE_SIZE+1].Text=TableName;

    ///RemoveChar(TableItem[I+CHAR_TABLE_SIZE+1].Text,L'&',TRUE); //BUGBUG!!!
    TableList.Items=TableItem;
    TableList.ItemsNumber++;
  }

  FindList = NULL;
  ArcList = NULL;
  hPluginMutex=CreateMutexW(NULL,FALSE,NULL);

  do
  {
    ClearAllLists();

    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

    PrepareDriveNameStr(strSearchFromRoot,PARTIAL_DLG_STR_LEN);

    const wchar_t *MasksHistoryName=L"Masks",*TextHistoryName=L"SearchText";
    const wchar_t *HexMask=L"HH HH HH HH HH HH HH HH HH HH HH"; // HH HH HH HH HH HH HH HH HH HH HH HH";

/*
    000000000011111111112222222222333333333344444444445555555555666666666677777777
    012345678901234567890123456789012345678901234567890123456789012345678901234567
00  """"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
01  "  +----------------------------- Find file ------------------------------+  "
02  "  � A file mask or several file masks:                                   �  "
03  "  � *.*#################################################################.�  "
04  "  �----------------------------------+-----------------------------------�  "
05  "  � Containing text:                 � Using character table:            �  "
06  "  � ################################.� Windows text#####################.�  "
07  "  �----------------------------------+-----------------------------------�  "
08  "  � [ ] Case sensitive               � [ ] Search in archives            �  "
09  "  � [ ] Whole words                  � [x] Search for folders            �  "
10  "  � [ ] Search for hex               � [x] Search in symbolic links      �  "
11  "  �----------------------------------+-----------------------------------�  "
12  "  � Select place to search                                               �  "
13  "  � ( ) In all non-removable drives    (.) From the current folder       �  "
14  "  � ( ) In all local drives            ( ) The current folder only       �  "
15  "  � ( ) In PATH folders                ( ) Selected folders              �  "
    "  � ( ) From the root of C:                                              �  "
16  "  �----------------------------------------------------------------------�  "
17  "  � [ ] Use filter                     [ ] Advanced options              �  "
18  "  �----------------------------------------------------------------------�  "
19  "  �      [ Find ]  [ Drive ]  [ Filter ]  [ Advanced ]  [ Cancel ]       �  "
20  "  +----------------------------------------------------------------------+  "
21  "                                                                            "
22  """"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
*/

    static struct DialogDataEx FindAskDlgData[]=
    {
      /* 00 */DI_DOUBLEBOX,3,1,DLG_WIDTH,DLG_HEIGHT-2,0,0,0,0,(const wchar_t *)MFindFileTitle,
      /* 01 */DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MFindFileMasks,
      /* 02 */DI_EDIT,5,3,72,3,1,(DWORD_PTR)MasksHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,L"",
      /* 03 */DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,L"",
      /* 04 */DI_TEXT,5,5,0,5,0,0,0,0,L"",
      /* 05 */DI_EDIT,5,6,36,6,0,(DWORD_PTR)TextHistoryName,DIF_HISTORY,0,L"",
      /* 06 */DI_FIXEDIT,5,6,36,6,0,(DWORD_PTR)HexMask,DIF_MASKEDIT,0,L"",
      /* 07 */DI_TEXT,40,5,0,5,0,0,0,0,L"",
      /* 08 */DI_COMBOBOX,40,6,72,18,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
      /* 09 */DI_TEXT,3,7,0,7,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 10 */DI_VTEXT,38,4,0,4,0,0,DIF_BOXCOLOR,0,L"\x2564\x2502\x2502\x253C",
      /* 11 */DI_CHECKBOX,5,8,0,8,0,0,0,0,(const wchar_t *)MFindFileCase,
      /* 12 */DI_CHECKBOX,5,9,0,9,0,0,0,0,(const wchar_t *)MFindFileWholeWords,
      /* 13 */DI_CHECKBOX,5,10,0,10,0,0,0,0,(const wchar_t *)MSearchForHex,
      /* 14 */DI_CHECKBOX,40,8,0,8,0,0,0,0,(const wchar_t *)MFindArchives,
      /* 15 */DI_CHECKBOX,40,9,0,9,0,0,0,0,(const wchar_t *)MFindFolders,
      /* 16 */DI_CHECKBOX,40,10,0,10,0,0,0,0,(const wchar_t *)MFindSymLinks,
      /* 17 */DI_TEXT,3,11,0,11,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,L"",
      /* 18 */DI_VTEXT,38,7,0,7,0,0,DIF_BOXCOLOR,0,L"\x253C\x2502\x2502\x2502\x2567",
      /* 19 */DI_TEXT,5,12,0,12,0,0,0,0,(const wchar_t *)MSearchWhere,
      /* 20 */DI_RADIOBUTTON,5,13,0,13,0,0,DIF_GROUP,0,(const wchar_t *)MSearchAllDisks,
      /* 21 */DI_RADIOBUTTON,5,14,0,14,0,1,0,0,(const wchar_t *)MSearchAllButNetwork,
      /* 22 */DI_RADIOBUTTON,5,15,0,15,0,1,0,0,(const wchar_t *)MSearchInPATH,
      /* 23 */DI_RADIOBUTTON,5,16,0,16,0,1,0,0,L"",
      /* 24 */DI_RADIOBUTTON,40,13,0,13,0,0,0,0,(const wchar_t *)MSearchFromCurrent,
      /* 25 */DI_RADIOBUTTON,40,14,0,14,0,0,0,0,(const wchar_t *)MSearchInCurrent,
      /* 26 */DI_RADIOBUTTON,40,15,0,15,0,0,0,0,(const wchar_t *)MSearchInSelected,
      /* 27 */DI_TEXT,3,17,0,17,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 28 */DI_CHECKBOX,5,18,0,18,0,0,0,0,(const wchar_t *)MFindUseFilter,
      /* 29 */DI_CHECKBOX,40,18,0,19,0,0,0,0,(const wchar_t *)MFindAdvancedOptions,
      /* 30 */DI_TEXT,3,19,0,19,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 31 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MFindFileFind,
      /* 32 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileDrive,
      /* 33 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileSetFilter,
      /* 34 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileAdvanced,
      /* 35 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
    };

    FindAskDlgData[23].Data = strSearchFromRoot; //BUGBUG

    MakeDialogItemsEx(FindAskDlgData,FindAskDlg);

    if ( strFindStr.IsEmpty() )
      FindAskDlg[15].Selected=Opt.FindOpt.FindFolders;
    for(I=20; I <= 26; ++I)
      FindAskDlg[I].Selected=0;
    FindAskDlg[20+SearchMode].Selected=1;

    {
      if (PluginMode)
      {
        struct OpenPluginInfo Info;
        HANDLE hPlugin=ActivePanel->GetPluginHandle();
        CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

        if ((Info.Flags & OPIF_REALNAMES)==0)
          FindAskDlg[14].Flags |= DIF_DISABLE;

        if (FindAskDlg[20].Selected || FindAskDlg[21].Selected)
        {
          FindAskDlg[20].Selected=FindAskDlg[21].Selected=0;
          FindAskDlg[23].Selected=1;
        }
        FindAskDlg[20].Flags=FindAskDlg[21].Flags|=DIF_DISABLE;
      }
    }

    if (PluginMode)
    {
      FindAskDlg[16].Selected=0;
      FindAskDlg[16].Flags|=DIF_DISABLE;
    }
    else
      FindAskDlg[16].Selected=Opt.FindOpt.FindSymLinks;

    /* $ 14.05.2001 DJ
       �� �������� �������, ���� ������ ������ � �������
    */
    if (!(FindAskDlg[14].Flags & DIF_DISABLE))
      FindAskDlg[14].Selected=SearchInArchives;

    FindAskDlg[2].strData = strFindMask;

    if (SearchHex)
      FindAskDlg[6].strData = strFindStr;
    else
      FindAskDlg[5].strData = strFindStr;

    FindAskDlg[11].Selected=CmpCase;
    FindAskDlg[12].Selected=WholeWords;
    FindAskDlg[13].Selected=SearchHex;

    // ������������ ������. KM
    FindAskDlg[28].Selected=UseFilter;

    // �������� �������������� ����� ������. KM
    FindAskDlg[29].Selected=EnableSearchInFirst;

    while (1)
    {
      int ExitCode;
      {
        FindAskDlg[8].ListItems=&TableList;

        Dialog Dlg(FindAskDlg,sizeof(FindAskDlg)/sizeof(FindAskDlg[0]),MainDlgProc);

        Dlg.SetHelp(L"FindFile");
        Dlg.SetPosition(-1,-1,DLG_WIDTH+4,DLG_HEIGHT);
        Dlg.Process();
        ExitCode=Dlg.GetExitCode();
      }

      //������ �������� ������� ��� ������� ����� ����� ������ �� �������
      Filter->UpdateCurrentTime();

      if (ExitCode!=31)
      {
        for(int i=CHAR_TABLE_SIZE+1;i<TableList.ItemsNumber;i++)
          xf_free((void*)TableItem[i].Text);
        xf_free(TableItem);
        CloseHandle(hPluginMutex);
        return;
      }

      /* ����������� ������������� ���������� */
      Opt.CharTable.AllTables=UseAllTables;
      Opt.CharTable.AnsiTable=UseANSI;
      Opt.CharTable.UnicodeTable=UseUnicode;
      if (UseDecodeTable)
        Opt.CharTable.TableNum=TableNum+1;
      else
        Opt.CharTable.TableNum=0;
      /****************************************/

      /* $ 01.07.2001 IS
         �������� ����� �� ������������
      */
      if( FindAskDlg[2].strData.IsEmpty() )             // ���� ������ � ������� �����,
         FindAskDlg[2].strData = L"*"; // �� �������, ��� ����� ���� "*"

      if(FileMaskForFindFile.Set(FindAskDlg[2].strData, 0))
           break;
    }

    CmpCase=FindAskDlg[11].Selected;
    WholeWords=FindAskDlg[12].Selected;
    SearchHex=FindAskDlg[13].Selected;
    SearchInArchives=FindAskDlg[14].Selected;
    if (FindFoldersChanged)
      Opt.FindOpt.FindFolders=FindAskDlg[15].Selected;

    if (!PluginMode && FindAskDlg[16].Selected != Opt.FindOpt.FindSymLinks)
    {
      Opt.FindOpt.FindSymLinks=FindAskDlg[16].Selected;
    }

    // ��������� ������� ������������� �������. KM
    Opt.FindOpt.UseFilter=UseFilter=FindAskDlg[28].Selected;

    strFindMask = !FindAskDlg[2].strData.IsEmpty() ? FindAskDlg[2].strData:L"*";

    if (SearchHex)
    {
      strFindStr = FindAskDlg[6].strData;
      RemoveTrailingSpaces(strFindStr);
    }
    else
      strFindStr = FindAskDlg[5].strData;

    if ( !strFindStr.IsEmpty())
    {
      UnicodeToAnsi(strFindStr, GlobalSearchString, sizeof (GlobalSearchString));
      GlobalSearchCase=CmpCase;
      GlobalSearchWholeWords=WholeWords;
      GlobalSearchHex=SearchHex;
    }
    if (FindAskDlg[20].Selected)
      SearchMode=FFSEARCH_ALL;
    if (FindAskDlg[21].Selected)
      SearchMode=FFSEARCH_ALL_BUTNETWORK;
    if (FindAskDlg[22].Selected)
      SearchMode=FFSEARCH_INPATH;
    if (FindAskDlg[23].Selected)
      SearchMode=FFSEARCH_ROOT;
    if (FindAskDlg[24].Selected)
      SearchMode=FFSEARCH_FROM_CURRENT;
    if (FindAskDlg[25].Selected)
      SearchMode=FFSEARCH_CURRENT_ONLY;
    if (FindAskDlg[26].Selected)
        SearchMode=FFSEARCH_SELECTED;
    if (SearchFromChanged)
    {
      Opt.FindOpt.FileSearchMode=SearchMode;
    }
    LastCmpCase=CmpCase;
    LastWholeWords=WholeWords;
    LastSearchHex=SearchHex;
    LastSearchInArchives=SearchInArchives;
    strLastFindMask = strFindMask;
    strLastFindStr = strFindStr;
    if ( !strFindStr.IsEmpty() )
      Editor::SetReplaceMode(FALSE);
  } while (FindFilesProcess());
  CloseHandle(hPluginMutex);
  for(int i=CHAR_TABLE_SIZE+1;i<TableList.ItemsNumber;i++)
    xf_free((void*)TableItem[i].Text);
  xf_free(TableItem);
  CtrlObject->Cp()->ActivePanel->SetTitle();
}


FindFiles::~FindFiles()
{
  FileMaskForFindFile.Free();
  ClearAllLists();
  ScrBuf.ResetShadow();

  if (Filter)
    delete Filter;
}

LONG_PTR WINAPI FindFiles::AdvancedDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch (Msg)
  {
    case DN_CLOSE:
      if (Param1 == 4)
      {
        string strTemp;
        wchar_t *temp = strTemp.GetBuffer(Dialog::SendDlgMessage(hDlg,DM_GETTEXTPTR,2,0));
        Dialog::SendDlgMessage(hDlg,DM_GETTEXTPTR,2,(LONG_PTR)temp);
        if (!CheckFileSizeStringFormat(temp))
        {
          Message(MSG_WARNING,1,MSG(MFindFileAdvancedTitle),MSG(MBadFileSizeFormat),MSG(MOk));
          return FALSE;
        }
      }
      break;
  }

  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

void FindFiles::AdvancedDialog()
{
  static struct DialogDataEx AdvancedDlgData[]=
  {
    /* 00 */DI_DOUBLEBOX,3,1,38,6,0,0,0,0,(const wchar_t *)MFindFileAdvancedTitle,
    /* 01 */DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MFindFileSearchFirst,
    /* 02 */DI_EDIT,5,3,36,3,0,0,0,0,L"",
    /* 03 */DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    /* 04 */DI_BUTTON,0,5,0,5,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
    /* 05 */DI_BUTTON,0,5,0,5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
  };

  MakeDialogItemsEx(AdvancedDlgData,AdvancedDlg);

  // ��������� ������, ������� ����� ����������� ��� ����������� ������ ������
  AdvancedDlg[2].strData = Opt.FindOpt.strSearchInFirstSize;

  Dialog Dlg(AdvancedDlg,sizeof(AdvancedDlg)/sizeof(AdvancedDlg[0]),AdvancedDlgProc);

  Dlg.SetHelp(L"FindFileAdvanced");
  Dlg.SetPosition(-1,-1,38+4,6+2);
  Dlg.Process();
  int ExitCode=Dlg.GetExitCode();

  if (ExitCode==4) // OK
  {
    Opt.FindOpt.strSearchInFirstSize = AdvancedDlg[2].strData;
    SearchInFirst=ConvertFileSizeString(Opt.FindOpt.strSearchInFirstSize);
  }
}

int FindFiles::GetPluginFile(DWORD ArcIndex, struct PluginPanelItem *PanelItem,
                             const wchar_t *DestPath, string &strResultName)
{
  _ALGO(CleverSysLog clv(L"FindFiles::GetPluginFile()"));
  HANDLE hPlugin = ArcList[ArcIndex]->hPlugin;
  string strSaveDir;
  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

  strSaveDir = Info.CurDir;
  AddEndSlash(strSaveDir);

  CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT|OPM_FIND);

  string strFileName;

  strFileName = ArcList[ArcIndex]->strRootPath;
  SetPluginDirectory(strFileName,hPlugin);

  strFileName = PanelItem->FindData.lpwszFileName;
  SetPluginDirectory(strFileName,hPlugin);

  int nItemsNumber;
  struct PluginPanelItem *pItems;
  int nResult = 0;

  wchar_t *lpFileNameToFind = xf_wcsdup (PanelItem->FindData.lpwszFileName);
  wchar_t *lpFileNameToFindShort = xf_wcsdup (PanelItem->FindData.lpwszAlternateFileName);

  lpFileNameToFind = (wchar_t*)PointToName(RemovePseudoBackSlash(lpFileNameToFind));
  lpFileNameToFindShort = (wchar_t*)PointToName(RemovePseudoBackSlash(lpFileNameToFindShort));

  if ( CtrlObject->Plugins.GetFindData (
      hPlugin,
      &pItems,
      &nItemsNumber,
      OPM_FIND
      ) )
  {
    for (int i = 0; i < nItemsNumber; i++)
    {
      PluginPanelItem *pItem = &pItems[i];
      PluginPanelItem Item = *pItem;

      wchar_t *lpwszFileName = xf_wcsdup(pItem->FindData.lpwszFileName);
      Item.FindData.lpwszFileName = xf_wcsdup (PointToName(RemovePseudoBackSlash(lpwszFileName)));
      xf_free (lpwszFileName);

      lpwszFileName = xf_wcsdup(pItem->FindData.lpwszAlternateFileName);
      Item.FindData.lpwszAlternateFileName = xf_wcsdup (PointToName(RemovePseudoBackSlash(lpwszFileName)));
      xf_free (lpwszFileName);

      if ( !StrCmp (lpFileNameToFind, Item.FindData.lpwszFileName) &&
           !StrCmp (lpFileNameToFindShort, Item.FindData.lpwszAlternateFileName) )
      {
        nResult = CtrlObject->Plugins.GetFile (
                            hPlugin,
                            &Item,
                            DestPath,
                            strResultName,
                            OPM_SILENT|OPM_FIND
                            );

        apiFreeFindData(&Item.FindData);
        break;
      }
      apiFreeFindData(&Item.FindData);
    }

    CtrlObject->Plugins.FreeFindData (hPlugin, pItems, nItemsNumber);
  }

  xf_free (lpFileNameToFind);
  xf_free (lpFileNameToFindShort);

  CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT|OPM_FIND);
//  SetPluginDirectory(ArcList[ArcIndex].RootPath,hPlugin);
  SetPluginDirectory(strSaveDir,hPlugin);

  return nResult;
}

LONG_PTR WINAPI FindFiles::FindDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  VMenu *ListBox=Dlg->Item[1]->ListPtr;

  CriticalSectionLock Lock(Dlg->CS);

  switch(Msg)
  {
    case DN_DRAWDIALOGDONE:
    {
      Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
      // ���������� ����� �� ������ [Go To]
      if((FindDirCount || FindFileCount) && !FindPositionChanged)
      {
        FindPositionChanged=TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,6/* [Go To] */,0);
      }
//      else
//        ScrBuf.Flush();
      return TRUE;
    }
/*
    case DN_HELP: // � ������ ������ �������� ����� ������, ���... ��������� � �����������
    {
      return !SearchDone?NULL:Param2;
    }
*/
    case DN_MOUSECLICK:
    {
      /* $ 07.04.2003 VVM
         ! ���� �� ����� ������ ������� ��������� ���� ����� - ��������.
         �� ���� �������� ��������� �� �������� �������. ��� �� � ����... */
      SMALL_RECT drect,rect;
      LONG_PTR Ret = FALSE;
      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(LONG_PTR)&drect);
      Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,1,(LONG_PTR)&rect);
      if (Param1==1 && ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.X<drect.Left+rect.Right)
      {
        Ret = FindDlgProc(hDlg,DN_BTNCLICK,6,0); // emulates a [ Go to ] button pressing
      }
      return Ret;
    }

    case DN_KEY:
    {
      if (!StopSearch && ((Param2==KEY_ESC) || (Param2 == KEY_F10)) )
      {
        PauseSearch=TRUE;
        IsProcessAssignMacroKey++; // �������� ���� �������
                                   // �.�. � ���� ������� ������ ������ Alt-F9!
        int LocalRes=TRUE;
        if (Opt.Confirm.Esc)
          LocalRes=AbortMessage();
        IsProcessAssignMacroKey--;
        PauseSearch=FALSE;
        StopSearch=LocalRes;

        return TRUE;
      }

      if (!ListBox)
      {
        return TRUE;
      }

      if(Param2 == KEY_LEFT || Param2 == KEY_RIGHT || Param2 == KEY_NUMPAD4 || Param2 == KEY_NUMPAD6)
        FindPositionChanged = TRUE;

      // �������� ����.������� ����� �����������.
      if(Param2 == KEY_CTRLALTSHIFTPRESS || Param2 == KEY_ALTF9)
      {
        IsProcessAssignMacroKey--;
        FrameManager->ProcessKey((DWORD)Param2);
        IsProcessAssignMacroKey++;
        return TRUE;
      }

      if (Param1==9 && (Param2==KEY_RIGHT || Param2 == KEY_NUMPAD6 || Param2==KEY_TAB)) // [ Stop ] button
      {
        FindPositionChanged = TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,5/* [ New search ] */,0);
        return TRUE;
      }
      else if (Param1==5 && (Param2==KEY_LEFT || Param2 == KEY_NUMPAD4 || Param2==KEY_SHIFTTAB)) // [ New search ] button
      {
        FindPositionChanged = TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,9/* [ Stop ] */,0);
        return TRUE;
      }
      else if (Param2==KEY_UP || Param2==KEY_DOWN ||
               Param2==KEY_NUMPAD2 || Param2==KEY_NUMPAD8 ||

               Param2==KEY_PGUP || Param2==KEY_PGDN ||
               Param2==KEY_NUMPAD3 || Param2==KEY_NUMPAD9 ||

               Param2==KEY_HOME || Param2==KEY_END ||
               Param2==KEY_NUMPAD1 || Param2==KEY_NUMPAD7 ||

               Param2==KEY_MSWHEEL_LEFT || Param2==KEY_MSWHEEL_RIGHT ||
               Param2==KEY_MSWHEEL_UP || Param2==KEY_MSWHEEL_DOWN)
      {
        ListBox->ProcessKey((int)Param2);
        return TRUE;
      }
      else if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5 || Param2==KEY_F4 ||
               ((Param2==KEY_ENTER||Param2==KEY_NUMENTER) && Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == 7)
              )
      {
        if (ListBox->GetItemCount()==0)
        {
          return TRUE;
        }

        if((Param2==KEY_ENTER||Param2==KEY_NUMENTER) && Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == 7)
          Param2=KEY_F3;

        ffCS.Enter ();

        DWORD ItemIndex = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0);
        if (ItemIndex != LIST_INDEX_NONE)
        {
          int RemoveTemp=FALSE;
          int ClosePlugin=FALSE; // ������� ���� ���������, ���� �������.
          string strSearchFileName;
          string strTempDir;

          if (FindList[ItemIndex]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          {
            ffCS.Leave ();

            return TRUE;
          }
          string strFileName=FindList[ItemIndex]->FindData.strFileName;
          // FindFileArcIndex ������ ����� ������������
          // �� ����� ���� ��� ������.
          if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
              (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
          {
            string strFindArcName = ArcList[FindList[ItemIndex]->ArcIndex]->strArcName;
            if (ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == INVALID_HANDLE_VALUE)
            {
              char *Buffer=new char[Opt.PluginMaxReadData];
              if (Buffer)
              {
                FILE *ProcessFile=_wfopen(strFindArcName,L"rb");
                if (ProcessFile==NULL)
                {
                  delete[] Buffer;

                  ffCS.Leave ();
                  return TRUE;
                }
                int ReadSize=(int)fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
                fclose(ProcessFile);

                int SavePluginsOutput=DisablePluginsOutput;
                DisablePluginsOutput=TRUE;

                WaitForSingleObject(hPluginMutex,INFINITE);

                ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = CtrlObject->Plugins.OpenFilePlugin(strFindArcName,(unsigned char *)Buffer,ReadSize, 0);

                ReleaseMutex(hPluginMutex);

                DisablePluginsOutput=SavePluginsOutput;

                delete[] Buffer;

                if (ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == (HANDLE)-2 ||
                    ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == INVALID_HANDLE_VALUE)
                {
                  ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;

                  ffCS.Leave ();
                  return TRUE;
                }
                ClosePlugin = TRUE;
              }
            }

            PluginPanelItem FileItem;
            memset(&FileItem,0,sizeof(FileItem));

            apiFindDataExToData(&FindList[ItemIndex]->FindData, &FileItem.FindData);

            FarMkTempEx(strTempDir); // � �������� �� NULL???
            CreateDirectoryW(strTempDir, NULL);

//            if (!CtrlObject->Plugins.GetFile(ArcList[FindList[ItemIndex].ArcIndex].hPlugin,&FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
            WaitForSingleObject(hPluginMutex,INFINITE);
            if (!GetPluginFile(FindList[ItemIndex]->ArcIndex,&FileItem,strTempDir,strSearchFileName))
            {
              apiFreeFindData(&FileItem.FindData);

              apiRemoveDirectory(strTempDir);
              if (ClosePlugin)
              {
                CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin);
                ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
              }
              ReleaseMutex(hPluginMutex);

              ffCS.Leave ();
              return FALSE;
            }
            else
            {
              apiFreeFindData(&FileItem.FindData);

              if (ClosePlugin)
              {
                CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin);
                ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
              }
              ReleaseMutex(hPluginMutex);
            }
            RemoveTemp=TRUE;
          }
          else
          {
            strSearchFileName = FindList[ItemIndex]->FindData.strFileName;
            if(GetFileAttributesW(strSearchFileName) == INVALID_FILE_ATTRIBUTES && GetFileAttributesW(FindList[ItemIndex]->FindData.strAlternateFileName) != INVALID_FILE_ATTRIBUTES)
              strSearchFileName = FindList[ItemIndex]->FindData.strAlternateFileName;
          }

          DWORD FileAttr;
          if ((FileAttr=GetFileAttributesW(strSearchFileName))!=INVALID_FILE_ATTRIBUTES)
          {
            string strOldTitle;
            apiGetConsoleTitle (strOldTitle);

            if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5)
            {
              int ListSize=ListBox->GetItemCount();
              NamesList ViewList;
              // ������� ��� �����, ������� ����� �������� �����...
              if(Opt.FindOpt.CollectFiles)
              {
                DWORD Index;
                for (int I=0;I<ListSize;I++)
                {
                  Index = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0, I);
                  LPFINDLIST PtrFindList=FindList[Index];
                  if ((Index != LIST_INDEX_NONE) &&
                      ((PtrFindList->ArcIndex == LIST_INDEX_NONE) ||
                       (ArcList[PtrFindList->ArcIndex]->Flags & OPIF_REALNAMES)))
                  {
                    // �� ��������� ����� � ������� � OPIF_REALNAMES
                    if ( !PtrFindList->FindData.strFileName.IsEmpty() && !(PtrFindList->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                        ViewList.AddName(PtrFindList->FindData.strFileName, PtrFindList->FindData.strAlternateFileName);
                  } /* if */
                } /* for */

                string strCurDir = FindList[ItemIndex]->FindData.strFileName;

                ViewList.SetCurName(strCurDir);
              }
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              {
                FileViewer ShellViewer (strSearchFileName,FALSE,FALSE,FALSE,-1,NULL,(FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE)?NULL:(Opt.FindOpt.CollectFiles?&ViewList:NULL));
                ShellViewer.SetDynamicallyBorn(FALSE);
                ShellViewer.SetEnableF6(TRUE);
                // FindFileArcIndex ������ ����� ������������
                // �� ����� ���� ��� ������.
                if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
                    (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
                  ShellViewer.SetSaveToSaveAs(TRUE);
                //!Mutex.Unlock();

                ffCS.Leave (); // ����� ����� �����������, ���� �� ��� ������ �����

                IsProcessVE_FindFile++;
                FrameManager->EnterModalEV();
                FrameManager->ExecuteModal();
                FrameManager->ExitModalEV();
                IsProcessVE_FindFile--;

                ffCS.Enter ();

                // ���������� ���������� �����
                FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
              }

              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            else
            {
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              {
/* $ 14.08.2002 VVM
  ! ����-��� �������� �� ������ ������������� � �������� ��������.
    � ���������, ������� �� ��� �� �������� ������
                int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,SearchFileName);
                int SwitchTo=FALSE;
                if (FramePos!=-1)
                {
                  if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
                      Opt.Confirm.AllowReedit)
                  {
                    char MsgFullFileName[NM];
                    xstrncpy(MsgFullFileName,SearchFileName,sizeof(MsgFullFileName)-1);
                    int MsgCode=Message(0,2,MSG(MFindFileTitle),
                          TruncPathStr(MsgFullFileName,ScrX-16),
                          MSG(MAskReload),
                          MSG(MCurrent),MSG(MNewOpen));
                    if (MsgCode==0)
                    {
                      SwitchTo=TRUE;
                    }
                    else if (MsgCode==1)
                    {
                      SwitchTo=FALSE;
                    }
                    else
                    {
                      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
                      Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
                      return TRUE;
                    }
                  }
                  else
                  {
                    SwitchTo=TRUE;
                  }
                }
                if (SwitchTo)
                {
                  (*FrameManager)[FramePos]->SetCanLoseFocus(FALSE);
                  (*FrameManager)[FramePos]->SetDynamicallyBorn(FALSE);
                  FrameManager->ActivateFrame(FramePos);
                  IsProcessVE_FindFile++;
                  FrameManager->EnterModalEV();
                  FrameManager->ExecuteModal ();
                  FrameManager->ExitModalEV();
//                  FrameManager->ExecuteNonModal();
                  IsProcessVE_FindFile--;
                  // ���������� ���������� �����
                  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
                }
                else
*/
                {
                  FileEditor ShellEditor (strSearchFileName,CP_AUTODETECT,0);
                  ShellEditor.SetDynamicallyBorn(FALSE);
                  ShellEditor.SetEnableF6 (TRUE);
                  // FindFileArcIndex ������ ����� ������������
                  // �� ����� ���� ��� ������.
                  if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
                      (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
                    ShellEditor.SetSaveToSaveAs(TRUE);

                  ffCS.Leave (); // ����� ����� �����������, ���� �� ��� ������ �����
                  IsProcessVE_FindFile++;
                  FrameManager->EnterModalEV();
                  FrameManager->ExecuteModal ();
                  FrameManager->ExitModalEV();
                  IsProcessVE_FindFile--;

                  ffCS.Enter ();
                  // ���������� ���������� �����
                  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
                }
              }
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            SetConsoleTitleW (strOldTitle);
          }
          if (RemoveTemp)
          {
            DeleteFileWithFolder(strSearchFileName);
          }
        }

        ffCS.Leave ();

        return TRUE;
      }
      return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
    }
    case DN_BTNCLICK:
    {

      FindPositionChanged = TRUE;

      if (Param1==5) // [ New search ] button pressed
      {
        StopSearch=TRUE;
        FindExitCode=FIND_EXIT_SEARCHAGAIN;
        return FALSE;
      }
      else if (Param1==9) // [ Stop ] button pressed
      {
        if (StopSearch)
          return FALSE;
        StopSearch=TRUE;
        return TRUE;
      }
      else if (Param1==6) // [ Goto ] button pressed
      {
        if (!ListBox)
        {
          return FALSE;
        }

        // ������� ����� ������ ��� �� ����� ������ �� �������.
        // ������� ������ ��� [ Panel ]
        if (ListBox->GetItemCount()==0)
        {
          return (TRUE);
        }
        FindExitIndex = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0);
        if (FindExitIndex != LIST_INDEX_NONE)
          FindExitCode = FIND_EXIT_GOTO;
        return (FALSE);
      }
      else if (Param1==7) // [ View ] button pressed
      {
        FindDlgProc(hDlg,DN_KEY,1,KEY_F3);
        return TRUE;
      }
      else if (Param1==8) // [ Panel ] button pressed
      {
        if (!ListBox)
        {
          return FALSE;
        }

        // �� ������ ����� �������� �� � �������, � �����.
        // ����� ��������� ������. ����� �������� ��������, ����� ��
        // ���� �� ������, ����� �� ������� � ������� ����� (� �����-��
        // ����!) � � ���������� ��� ���������.
        if (ListBox->GetItemCount()==0)
        {
          return (TRUE);
        }
        FindExitCode = FIND_EXIT_PANEL;
        FindExitIndex = (DWORD)(DWORD_PTR)ListBox->GetUserData(NULL, 0);
        return (FALSE);
      }
    }
    case DN_CTLCOLORDLGLIST:
    {
      if (Param2)
        ((struct FarListColors *)Param2)->Colors[VMenuColorDisabled]=FarColorToReal(COL_DIALOGLISTTEXT);
      return TRUE;
    }
    case DN_CLOSE:
    {
      StopSearch=TRUE;
      return TRUE;
    }
    /* 10.08.2001 KM
       ��������� �������� ������� ������ ��� ��������� �������� �������.
    */
    case DN_RESIZECONSOLE:
    {
      COORD coord=(*(COORD*)Param2);
      SMALL_RECT rect;
      int IncY=coord.Y-DlgHeight-3;
      int I;

      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
      for (I=0;I<10;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,FALSE);

      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(LONG_PTR)&rect);
      coord.X=rect.Right-rect.Left+1;
      DlgHeight+=IncY;
      coord.Y=DlgHeight;

      if (IncY>0)
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(LONG_PTR)&coord);

      for (I=0;I<2;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(LONG_PTR)&rect);
        rect.Bottom+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(LONG_PTR)&rect);
      }

      for (I=2;I<10;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(LONG_PTR)&rect);
        if (I==2)
          rect.Left=-1;
        rect.Top+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(LONG_PTR)&rect);
      }

      if (!(IncY>0))
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(LONG_PTR)&coord);

      for (I=0;I<10;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,TRUE);
      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);

      return TRUE;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int FindFiles::FindFilesProcess()
{
  _ALGO(CleverSysLog clv(L"FindFiles::FindFilesProcess()"));
  // � ����������� ��������� ����� � ����������� ����������
  static string strTitle = L"";
  static string strSearchStr;

  /* $ 05.10.2003 KM
     ���� ������������ ������ ��������, �� �� ����� ������ �������� �� ����
  */
  if ( !strFindMask.IsEmpty() )
    if (UseFilter)
      strTitle.Format (L"%s: %s (%s)",MSG(MFindFileTitle),(const wchar_t*)strFindMask,MSG(MFindUsingFilter));
    else
      strTitle.Format (L"%s: %s",MSG(MFindFileTitle),(const wchar_t*)strFindMask);
  else
    if (UseFilter)
      strTitle.Format (L"%s (%s)",MSG(MFindFileTitle),MSG(MFindUsingFilter));
    else
      strTitle.Format (L"%s", MSG(MFindFileTitle));

  if ( !strFindStr.IsEmpty() )
  {
    string strTemp, strFStr;

    strFStr = strFindStr;

    TruncStrFromEnd(strFStr,10);
    strTemp.Format (L" \"%s\"", (const wchar_t*)strFStr);
    strSearchStr.Format(MSG(MFindSearchingIn), (const wchar_t*)strTemp);
  }
  else
    strSearchStr.Format (MSG(MFindSearchingIn), L"");

  struct DialogDataEx FindDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,DLG_WIDTH,DLG_HEIGHT-4,0,0,DIF_SHOWAMPERSAND,0,strTitle,
  /* 01 */DI_LISTBOX,4,2,73,DLG_HEIGHT-9,0,0,DIF_LISTNOBOX,0,(const wchar_t*)0,
  /* 02 */DI_TEXT,-1,DLG_HEIGHT-8,0,DLG_HEIGHT-8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 03 */DI_TEXT,5,DLG_HEIGHT-7,0,DLG_HEIGHT-7,0,0,DIF_SHOWAMPERSAND,0,strSearchStr,
  /* 04 */DI_TEXT,3,DLG_HEIGHT-6,0,DLG_HEIGHT-6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 05 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,1,0,DIF_CENTERGROUP,1,(const wchar_t *)MFindNewSearch,
  /* 06 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindGoTo,
  /* 07 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindView,
  /* 08 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindPanel,
  /* 09 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindStop
  };

  MakeDialogItemsEx(FindDlgData,FindDlg);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  DlgHeight=DLG_HEIGHT-2;
  DlgWidth=FindDlg[0].X2-FindDlg[0].X1-4;

  int IncY=ScrY>24 ? ScrY-24:0;
  FindDlg[0].Y2+=IncY;
  FindDlg[1].Y2+=IncY;
  FindDlg[2].Y1+=IncY;
  FindDlg[3].Y1+=IncY;
  FindDlg[4].Y1+=IncY;
  FindDlg[5].Y1+=IncY;
  FindDlg[6].Y1+=IncY;
  FindDlg[7].Y1+=IncY;
  FindDlg[8].Y1+=IncY;
  FindDlg[9].Y1+=IncY;

  DlgHeight+=IncY;

  if (PluginMode)
  {
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    HANDLE hPlugin=ActivePanel->GetPluginHandle();
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    FindFileArcIndex = AddArcListItem(Info.HostFile, hPlugin, Info.Flags, Info.CurDir);

    if (FindFileArcIndex == LIST_INDEX_NONE)
      return(FALSE);

    if ((Info.Flags & OPIF_REALNAMES)==0)
    {
      FindDlg[8].Type=DI_TEXT;
      FindDlg[8].strData=L"";
    }
  }

  Dialog Dlg=Dialog(FindDlg,sizeof(FindDlg)/sizeof(FindDlg[0]),FindDlgProc);
  hDlg=(HANDLE)&Dlg;
//  pDlg->SetDynamicallyBorn();
  Dlg.SetHelp(L"FindFileResult");
  Dlg.SetPosition(-1,-1,DLG_WIDTH+4,DLG_HEIGHT-2+IncY);
  // ���� �� �������� ������, � �� ������������� ��������� �����������
  // ������ ��� ������ � ������ �������� �� �����������
  Dlg.InitDialog();
  Dlg.Show();

  LastFoundNumber=0;
  SearchDone=FALSE;
  StopSearch=FALSE;
  PauseSearch=FALSE;
  WriteDataUsed=FALSE;
  PrepareFilesListUsed=0;
  FindFileCount=FindDirCount=0;
  FindExitIndex = LIST_INDEX_NONE;
  FindExitCode = FIND_EXIT_NONE;
  FindMessageReady=FindCountReady=FindPositionChanged=0;
  strLastDirName = L"";
  strFindMessage = L"";

  if (PluginMode)
  {
    if (_beginthread(PreparePluginList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }
  else
  {
    if (_beginthread(PrepareFilesList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }

  // ����� ��� ������ � ������� ���������� � ���� ������
  if (_beginthread(WriteDialogData,0,NULL)==(unsigned long)-1)
    return FALSE;

  IsProcessAssignMacroKey++; // �������� ��� ����. �������
  Dlg.Process();
  IsProcessAssignMacroKey--;

  while(WriteDataUsed || PrepareFilesListUsed > 0)
    Sleep(10);

  ::hDlg=NULL;

  switch (FindExitCode)
  {
    case FIND_EXIT_SEARCHAGAIN:
    {
      return TRUE;
    }
    case FIND_EXIT_PANEL:
    // ���������� ���������� �� ��������� ������
    {
      DWORD ListSize = FindListCount;
      PluginPanelItem *PanelItems=new PluginPanelItem[ListSize];
      if (PanelItems==NULL)
        ListSize=0;
      int ItemsNumber=0;
      for (DWORD i=0;i<ListSize;i++)
      {
        if (StrLength(FindList[i]->FindData.strFileName)>0 && FindList[i]->Used)
        // ��������� ������, ���� ��� ������
        {
          // ��� �������� � ������������ ������� ������� ��� ����� �� ��� ������.
          // ������ ���� ������ ������ �����.
          int IsArchive = ((FindList[i]->ArcIndex != LIST_INDEX_NONE) &&
                          !(ArcList[FindList[i]->ArcIndex]->Flags&OPIF_REALNAMES));

          // ��������� ������ ����� ��� ����� ������� ��� ����� ����� �������
          if (IsArchive || (Opt.FindOpt.FindFolders && !SearchHex) ||
              !(FindList[i]->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
          {
            if (IsArchive)
              FindList[i]->FindData.strFileName = ArcList[FindList[i]->ArcIndex]->strArcName;

            PluginPanelItem *pi=&PanelItems[ItemsNumber++];
            memset(pi,0,sizeof(*pi));

            apiFindDataExToData (&FindList[i]->FindData, &pi->FindData);

            if (IsArchive)
              pi->FindData.dwFileAttributes = 0;
            /* $ 21.03.2002 VVM
              + �������� ����� ��������� ��� ��������������� "\" */
            if (pi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              int Length = StrLength(pi->FindData.lpwszFileName);
              if ((Length) && (pi->FindData.lpwszFileName[Length-1]=='\\'))
                pi->FindData.lpwszFileName[Length-1] = 0;
            }
          }
        } /* if */
      } /* for */

      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFindListPlugin(PanelItems,ItemsNumber);
      if (hNewPlugin!=INVALID_HANDLE_VALUE)
      {
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,L"",true);
        NewPanel->SetVisible(TRUE);
        NewPanel->Update(0);
//        if (FindExitIndex != LIST_INDEX_NONE)
//          NewPanel->GoToFile(FindList[FindExitIndex].FindData.cFileName);
        NewPanel->Show();
      }

      for (int i = 0; i < ItemsNumber; i++)
          apiFreeFindData (&PanelItems[i].FindData);

      delete[] PanelItems;
      break;
    } /* case FIND_EXIT_PANEL */
    case FIND_EXIT_GOTO:
    {
      string strFileName=FindList[FindExitIndex]->FindData.strFileName;
      Panel *FindPanel=CtrlObject->Cp()->ActivePanel;

      if ((FindList[FindExitIndex]->ArcIndex != LIST_INDEX_NONE) &&
          (!(ArcList[FindList[FindExitIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
      {
        HANDLE hPlugin = ArcList[FindList[FindExitIndex]->ArcIndex]->hPlugin;
        if (hPlugin == INVALID_HANDLE_VALUE)
        {
          string strArcName, strArcPath;

          strArcName = ArcList[FindList[FindExitIndex]->ArcIndex]->strArcName;
          if (FindPanel->GetType()!=FILE_PANEL)
            FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);

          strArcPath = strArcName;

          CutToSlash(strArcPath);
          FindPanel->SetCurDir(strArcPath,TRUE);
          hPlugin=((FileList *)FindPanel)->OpenFilePlugin(strArcName,FALSE);
          if (hPlugin==(HANDLE)-2)
            hPlugin = INVALID_HANDLE_VALUE;
        } /* if */
        if (hPlugin != INVALID_HANDLE_VALUE)
        {
          struct OpenPluginInfo Info;
          CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

          /* $ 19.01.2003 KM
             ��������� �������� � ������ ������� �������.
          */
          if (SearchMode==FFSEARCH_ROOT ||
              SearchMode==FFSEARCH_ALL ||
              SearchMode==FFSEARCH_ALL_BUTNETWORK ||
              SearchMode==FFSEARCH_INPATH)
            CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_FIND);

          SetPluginDirectory(strFileName,hPlugin,TRUE);
        }
      } /* if */
      else
      {
        string strSetName;
        int Length;
        if ((Length=(int)strFileName.GetLength())==0)
          break;

        if (Length>1 && strFileName.At(Length-1)==L'\\' && strFileName.At(Length-2)!=L':')
          strFileName.SetLength(Length-1);

        if ( (GetFileAttributesW(strFileName)==INVALID_FILE_ATTRIBUTES) && (GetLastError () != ERROR_ACCESS_DENIED))
          break;
        {
          const wchar_t *NamePtr = PointToName(strFileName);

          strSetName = NamePtr;

          strFileName.SetLength(NamePtr-(const wchar_t *)strFileName);

          Length=(int)strFileName.GetLength();

          if (Length>1 && strFileName.At(Length-1)==L'\\' && strFileName.At(Length-2)!=L':')
            strFileName.SetLength(Length-1);
        }
        if ( strFileName.IsEmpty() )
          break;
        if (FindPanel->GetType()!=FILE_PANEL &&
            CtrlObject->Cp()->GetAnotherPanel(FindPanel)->GetType()==FILE_PANEL)
          FindPanel=CtrlObject->Cp()->GetAnotherPanel(FindPanel);
        if ((FindPanel->GetType()!=FILE_PANEL) || (FindPanel->GetMode()!=NORMAL_PANEL))
        // ������ ������ �� ������� ��������...
        {
          FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
          FindPanel->SetVisible(TRUE);
          FindPanel->Update(0);
        }
        /* $ 09.06.2001 IS
           ! �� ������ �������, ���� �� ��� � ��� ���������. ��� �����
             ���������� ����, ��� ��������� � ��������� ������ �� ������������.
        */
        {
          string strDirTmp;
          FindPanel->GetCurDir(strDirTmp);
          Length=(int)strDirTmp.GetLength();

          if (Length>1 && strDirTmp.At(Length-1)==L'\\' && strDirTmp.At(Length-2)!=L':')
            strDirTmp.SetLength(Length-1);

          if (0!=StrCmpI(strFileName, strDirTmp))
            FindPanel->SetCurDir(strFileName,TRUE);
        }
        if ( !strSetName.IsEmpty() )
          FindPanel->GoToFile(strSetName);
        FindPanel->Show();
        FindPanel->SetFocus();
      }
      break;
    } /* case FIND_EXIT_GOTO */
  } /* switch */

  return FALSE;
}


void FindFiles::SetPluginDirectory(const wchar_t *DirName,HANDLE hPlugin,int UpdatePanel)
{
  string strName;
  wchar_t *StartName,*EndName;
  int IsPluginDir;

  /* $ 19.01.2003 KM
     ����������� ��������� �� 4 ����. ���� � DirName ����
     ������ '\x1' ������ ��� ���� �� �������. ����� �������
     ����� ���������� ���������� ���� �, ��������������,
     ������� ���������� �������.
  */
  strName = DirName;
  if ( strName.GetLength() > 0 )
    IsPluginDir=strName.Contains(L'\x1')?TRUE:FALSE;
  else
    IsPluginDir=FALSE;

  StartName = strName.GetBuffer();
  while(IsPluginDir?(EndName=wcschr(StartName,L'\x1'))!=NULL:(EndName=wcschr(StartName,L'\\'))!=NULL)
  {
    *EndName=0;
    // RereadPlugin
    {
      int FileCount=0;
      PluginPanelItem *PanelData=NULL;
      if (CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&FileCount,OPM_FIND))
        CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,FileCount);
    }
    CtrlObject->Plugins.SetDirectory(hPlugin,StartName,OPM_FIND);
    StartName=EndName+1;
  }

  //   �������� ������ ��� �������������.
  if (UpdatePanel)
  {
    CtrlObject->Cp()->ActivePanel->Update(UPDATE_KEEP_SELECTION);
    if (!CtrlObject->Cp()->ActivePanel->GoToFile(StartName))
      CtrlObject->Cp()->ActivePanel->GoToFile(DirName);
    CtrlObject->Cp()->ActivePanel->Show();
  }

  //strName.ReleaseBuffer(); �� ����. ������ ��� ����� ���������, ������ ����� StrLength.
}

void FindFiles::DoScanTree(string& strRoot, FAR_FIND_DATA_EX& FindData, string& strFullName)
{
	{
		ScanTree ScTree(FALSE,!(SearchMode==FFSEARCH_CURRENT_ONLY||SearchMode==FFSEARCH_INPATH),Opt.FindOpt.FindSymLinks);

		string strSelName;
		DWORD FileAttr;
		if (SearchMode==FFSEARCH_SELECTED)
			CtrlObject->Cp()->ActivePanel->GetSelName(NULL,FileAttr);

		while (1)
		{
			string strCurRoot;
			if (SearchMode==FFSEARCH_SELECTED)
			{
				if (!CtrlObject->Cp()->ActivePanel->GetSelName(&strSelName,FileAttr))
					break;
				if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0 || TestParentFolderName(strSelName) ||
						StrCmp(strSelName,L".")==0)
					continue;

				strCurRoot = strRoot;
				AddEndSlash(strCurRoot);
				strCurRoot += strSelName;
			}
			else
			{
				strCurRoot = strRoot;
			}

			ScTree.SetFindPath(strCurRoot,L"*.*");

			statusCS.Enter ();

			strFindMessage = strCurRoot;
			FindMessageReady=TRUE;

			statusCS.Leave ();

			while (!StopSearch && ScTree.GetNextName(&FindData,strFullName))
			{
				while (PauseSearch)
					Sleep(10);

				/* $ 30.09.2003 KM
					����������� ����� �� ���������� � ����������� ������
				*/
				if (UseFilter)
				{
					if (!Filter->FileInFilter(&FindData))
					{
						if (FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
							ScTree.SkipDir();
						continue;
					}
				}

				/* $ 14.06.2004 KM
					��������� �������� ��� ��������� ���������
				*/
				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					statusCS.Enter();

					strFindMessage = strFullName;
					FindMessageReady=TRUE;

					statusCS.Leave();
				}

				if (IsFileIncluded(NULL,strFullName,FindData.dwFileAttributes))
					AddMenuRecord(strFullName,&FindData);

				if (SearchInArchives)
					ArchiveSearch(strFullName);
			}
			if (SearchMode!=FFSEARCH_SELECTED)
				break;
		}
	}
}
void _cdecl FindFiles::DoPrepareFileList(string& strRoot, FAR_FIND_DATA_EX& FindData, string& strFullName)
{
  TRY {
    wchar_t *PathEnv=NULL, *Ptr=NULL; //BUGBUG
    PrepareFilesListUsed++;
    DWORD DiskMask=FarGetLogicalDrives();

    //string strRoot; //BUGBUG
    CtrlObject->CmdLine->GetCurDir(strRoot);

    if(SearchMode==FFSEARCH_INPATH)
    {
      DWORD SizeStr=GetEnvironmentVariableW(L"PATH",NULL,0);
      if((PathEnv=(wchar_t *)alloca((SizeStr+2)*sizeof (wchar_t))) != NULL)
      {
        GetEnvironmentVariableW(L"PATH",PathEnv,SizeStr+1);
        PathEnv[StrLength(PathEnv)]=0;
        Ptr=PathEnv;
        while(*Ptr)
        {
          if(*Ptr==L';')
            *Ptr=0;
          ++Ptr;
        }
      }
      Ptr=PathEnv;
    }

    for (int CurrentDisk=0;;CurrentDisk++,DiskMask>>=1)
    {
      if (SearchMode==FFSEARCH_ALL ||
          SearchMode==FFSEARCH_ALL_BUTNETWORK)
      {
        if(DiskMask==0)
          break;

        if ((DiskMask & 1)==0)
          continue;

        strRoot.Format (L"%c:\\", L'A'+CurrentDisk);
        int DriveType=FAR_GetDriveType(strRoot);
        if (DriveType==DRIVE_REMOVABLE || IsDriveTypeCDROM(DriveType) ||
           (DriveType==DRIVE_REMOTE && SearchMode==FFSEARCH_ALL_BUTNETWORK))
        {
          if (DiskMask==1)
            break;
          else
            continue;
        }
      }
      else if (SearchMode==FFSEARCH_ROOT)
        GetPathRootOne(strRoot,strRoot);
      else if(SearchMode==FFSEARCH_INPATH)
      {
        if(!*Ptr)
          break;
        strRoot = Ptr;
        Ptr+=StrLength(Ptr)+1;
      }

      DoScanTree(strRoot, FindData, strFullName);

      if (SearchMode!=FFSEARCH_ALL && SearchMode!=FFSEARCH_ALL_BUTNETWORK && SearchMode!=FFSEARCH_INPATH)
        break;
    }

    while (!StopSearch && FindMessageReady)
      Sleep(10);
  //  sprintf(FindMessage,MSG(MFindDone),FindFileCount,FindDirCount);

    statusCS.Enter ();

    strFindMessage.Format (MSG(MFindDone),FindFileCount,FindDirCount);

    SetFarTitle(strFindMessage);

    SearchDone=TRUE;
    FindMessageReady=TRUE;

    statusCS.Leave ();

    PrepareFilesListUsed--;
  }
  EXCEPT (xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
  {
    TerminateProcess( GetCurrentProcess(), 1);
  }
}
void _cdecl FindFiles::PrepareFilesList(void *Param)
{
  FAR_FIND_DATA_EX FindData;
  string strFullName;
  string strRoot;

  DoPrepareFileList(strRoot, FindData, strFullName);

}

void FindFiles::ArchiveSearch(const wchar_t *ArcName)
{
  _ALGO(CleverSysLog clv(L"FindFiles::ArchiveSearch()"));
  _ALGO(SysLog(L"ArcName='%s'",(ArcName?ArcName:"NULL")));
  char *Buffer=new char[Opt.PluginMaxReadData];
  if ( !Buffer )
  {
    _ALGO(SysLog(L"ERROR: alloc buffer (size=%u)",Opt.PluginMaxReadData));
    return;
  }

  FILE *ProcessFile=_wfopen(ArcName,L"rb");
  if (ProcessFile==NULL)
  {
    delete[] Buffer;
    return;
  }
  int ReadSize=(int)fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
  fclose(ProcessFile);

  int SavePluginsOutput=DisablePluginsOutput;
  DisablePluginsOutput=TRUE;

  string strArcName = ArcName;

  HANDLE hArc=CtrlObject->Plugins.OpenFilePlugin(strArcName,(unsigned char *)Buffer,ReadSize,OPM_FIND);
  DisablePluginsOutput=SavePluginsOutput;

  delete[] Buffer;

  if (hArc==(HANDLE)-2)
  {
    BreakMainThread=TRUE;
    _ALGO(SysLog(L"return: hArc==(HANDLE)-2"));
    return;
  }

  if (hArc==INVALID_HANDLE_VALUE)
  {
    _ALGO(SysLog(L"return: hArc==INVALID_HANDLE_VALUE"));
    return;
  }

  int SaveSearchMode=SearchMode;
  DWORD SaveArcIndex = FindFileArcIndex;
  {
    SearchMode=FFSEARCH_FROM_CURRENT;
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hArc,&Info);

    FindFileArcIndex = AddArcListItem(ArcName, hArc, Info.Flags, Info.CurDir);
    /* $ 11.12.2001 VVM
      - �������� ������� ����� ������� � ������.
        � ���� ������ �� ����� - �� ������ ��� ����� */
    {
      string strSaveDirName, strSaveSearchPath;
      DWORD SaveListCount = FindListCount;
      /* $ 19.01.2003 KM
         �������� ���� ������ � �������, ��� ����� ����������.
      */
      strSaveSearchPath = strPluginSearchPath;

      strSaveDirName = strLastDirName;
      strLastDirName = L"";
      PreparePluginList((void *)1);
      strPluginSearchPath = strSaveSearchPath;
      WaitForSingleObject(hPluginMutex,INFINITE);
      CtrlObject->Plugins.ClosePlugin(ArcList[FindFileArcIndex]->hPlugin);
      ArcList[FindFileArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
      ReleaseMutex(hPluginMutex);
      if (SaveListCount == FindListCount)
        strLastDirName = strSaveDirName;
    }
  }
  FindFileArcIndex = SaveArcIndex;
  SearchMode=SaveSearchMode;
}

/* $ 01.07.2001 IS
   ���������� FileMaskForFindFile ������ GetCommaWord
*/
int FindFiles::IsFileIncluded(PluginPanelItem *FileItem,const wchar_t *FullName,DWORD FileAttr)
{
  CriticalSectionLock Lock (ffCS);

  int FileFound=FileMaskForFindFile.Compare(FullName);
  HANDLE hPlugin=INVALID_HANDLE_VALUE;
  if (FindFileArcIndex != LIST_INDEX_NONE)
    hPlugin = ArcList[FindFileArcIndex]->hPlugin;
  while(FileFound)
  {
    /* $ 24.09.2003 KM
       ���� ������� ����� ������ hex-�����, ����� ����� � ����� �� ��������
    */
    /* $ 17.01.2002 VVM
      ! ��������� ������ � ������� � ������ ������� � ������ -
        ���� � ������� ������� ���� ������������ */
    if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && ((Opt.FindOpt.FindFolders==0) || SearchHex))
//        ((hPlugin == INVALID_HANDLE_VALUE) ||
//        (ArcList[FindFileArcIndex].Flags & OPIF_FINDFOLDERS)==0))
      return FALSE;

    if ( !strFindStr.IsEmpty() && FileFound)
    {
      FileFound=FALSE;
      if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
        break;
      string strSearchFileName;
      int RemoveTemp=FALSE;
      if ((hPlugin != INVALID_HANDLE_VALUE) && (ArcList[FindFileArcIndex]->Flags & OPIF_REALNAMES)==0)
      {
        string strTempDir;
        FarMkTempEx(strTempDir); // � �������� �� NULL???
        CreateDirectoryW(strTempDir,NULL);
        WaitForSingleObject(hPluginMutex,INFINITE);
        if (!CtrlObject->Plugins.GetFile(hPlugin,FileItem,strTempDir,strSearchFileName,OPM_SILENT|OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);
          apiRemoveDirectory(strTempDir);
          break;
        }
        else
          ReleaseMutex(hPluginMutex);
        RemoveTemp=TRUE;
      }
      else
      {
        strSearchFileName = FullName;
      }

      if (LookForString(strSearchFileName))
        FileFound=TRUE;

      if (RemoveTemp)
      {
        DeleteFileWithFolder(strSearchFileName);
      }
    }
    break;
  }
  return(FileFound);
}


void FindFiles::AddMenuRecord(const wchar_t *FullName, FAR_FIND_DATA *FindData)
{
    FAR_FIND_DATA_EX fdata;

    apiFindDataToDataEx (FindData, &fdata);

    AddMenuRecord (FullName, &fdata);
}

void FindFiles::AddMenuRecord(const wchar_t *FullName, FAR_FIND_DATA_EX *FindData)
{
  string strMenuText, strFileText, strSizeText;
  string strAttr;
  string strDateStr, strTimeStr, strDate;
  MenuItemEx ListItem;

  Dialog* Dlg=(Dialog*)hDlg;
  if(!Dlg)
  {
    return;
  }

  VMenu *ListBox=Dlg->Item[1]->ListPtr;

  if (!ListBox)
  {
    return;
  }

  ListItem.Clear ();

  if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    strSizeText.Format (L"%13s", MSG(MFindFileFolder));
  }
  else
  {
    wchar_t *wszSizeText = strSizeText.GetBuffer (100); //BUGBUG, function!!!

    _ui64tow(FindData->nFileSize, wszSizeText, 10);

    strSizeText.ReleaseBuffer ();
  }
  const wchar_t *DisplayName=FindData->strFileName;
  /* $ 24.03.2002 KM
     � �������� ������������� �������� ��������� � ����� �� ���
     ��� ����������� ��� ����������� � ������, �������� ����,
     �.�. ��������� ������� ���������� ��� ������ � ������ ����,
     � ������� ��������� ������.
  */
  if (FindFileArcIndex != LIST_INDEX_NONE)
    DisplayName=PointToName(DisplayName);

  strFileText.Format (L" %-26.26s %13.13s",DisplayName,(const wchar_t*)strSizeText);


  /* $ 05.10.2003 KM
     ��� ������������� ������� �� ���� ����� ���������� ��� ���
     ����, ������� ����� � �������.

     $ 24.01.2007 AY
     ��� ��� ������ ���� ������ ��� ���� ������ �� ���� ������ �
     ���� �� ���� ��� ������� �������.

  if (UseFilter && Filter->GetParams()->FDate.Used)
  {
    switch(Filter->GetParams()->FDate.DateType)
    {
      case FDATE_MODIFIED:
        // ���������� ���� ���������� ���������
        ConvertDate(FindData->ftLastWriteTime,strDateStr,strTimeStr,5);
        break;
      case FDATE_CREATED:
        // ���������� ���� ��������
        ConvertDate(FindData->ftCreationTime,strDateStr,strTimeStr,5);
        break;
      case FDATE_OPENED:
        // ���������� ���� ���������� �������
        ConvertDate(FindData->ftLastAccessTime,strDateStr,strTimeStr,5);
        break;
    }
  }
  else
  */
    // ���������� ���� ���������� ���������
    ConvertDate(FindData->ftLastWriteTime,strDateStr,strTimeStr,5);
  strDate.Format (L"  %s  %s", (const wchar_t*)strDateStr, (const wchar_t*)strTimeStr);
  strFileText += strDate;

  /* $ 05.10.2003 KM
     ��������� � ������ ������ �������� ��������� ������
  */
  wchar_t AttrStr[16];
  DWORD FileAttr=FindData->dwFileAttributes;
  AttrStr[0]=(FileAttr & FILE_ATTRIBUTE_READONLY) ? L'R':L' ';
  AttrStr[1]=(FileAttr & FILE_ATTRIBUTE_SYSTEM) ? L'S':L' ';
  AttrStr[2]=(FileAttr & FILE_ATTRIBUTE_HIDDEN) ? L'H':L' ';
  AttrStr[3]=(FileAttr & FILE_ATTRIBUTE_ARCHIVE) ? L'A':L' ';
  AttrStr[4]=(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? L'L' : ((FileAttr & FILE_ATTRIBUTE_SPARSE_FILE) ? L'$':L' ');
  AttrStr[5]=(FileAttr & FILE_ATTRIBUTE_COMPRESSED) ? L'C':((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?L'E':L' ');
  AttrStr[6]=(FileAttr & FILE_ATTRIBUTE_TEMPORARY) ? L'T':' ';
  AttrStr[7]=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) ? L'I':L' ';
  AttrStr[8]=0;

  strAttr.Format (L" %s", AttrStr);
  strFileText += strAttr;
  strMenuText.Format (L" %-*.*s",DlgWidth-3,DlgWidth-3, (const wchar_t*)strFileText);

  string strPathName;
  strPathName = FullName;

  RemovePseudoBackSlash(strPathName);

  CutToSlash(strPathName);

  if ( strPathName.IsEmpty() )
      strPathName = L".\\";

  AddEndSlash(strPathName);

  if ( StrCmpI(strPathName,strLastDirName)!=0)
  {
    if ( !strLastDirName.IsEmpty() )
    {
      /* $ 12.05.2001 DJ
         ������ �� ��������������� �� ������ ������� ����� ����������
      */
      ListItem.Flags|=LIF_DISABLE;
      // � ������ VVM ������� ���������� � ������ ������� LIST_INDEX_NONE �� ������ �������
      ListBox->SetUserData((void*)(DWORD_PTR)LIST_INDEX_NONE,sizeof(LIST_INDEX_NONE),ListBox->AddItem(&ListItem));
      ListItem.Flags&=~LIF_DISABLE;
    }
    ffCS.Enter ();

    strLastDirName = strPathName;

    if ((FindFileArcIndex != LIST_INDEX_NONE) &&
        (!(ArcList[FindFileArcIndex]->Flags & OPIF_REALNAMES)) &&
        (!ArcList[FindFileArcIndex]->strArcName.IsEmpty()))
    {
      string strArcPathName;

      const wchar_t *ArcName = ArcList[FindFileArcIndex]->strArcName;

      strArcPathName.Format (L"%s:%s", ArcName, strPathName.At(0)==L'.' ? L"\\":(const wchar_t*)strPathName);
      strPathName = strArcPathName;
    }
    strSizeText = MSG(MFindFileFolder);

    TruncPathStr(strPathName,50);

    strFileText.Format (L"%-50.50s  <%6.6s>", (const wchar_t*)strPathName, (const wchar_t*)strSizeText);
    ListItem.strName.Format (L"%-*.*s",DlgWidth-2,DlgWidth-2, (const wchar_t*)strFileText);

    DWORD ItemIndex = AddFindListItem(FindData);
    if (ItemIndex != LIST_INDEX_NONE)
    {
      // ������� ������ � FindData. ��� ��� �� �����
      FindList[ItemIndex]->FindData.Clear();
      // ���������� LastDirName, �.�. PathName ��� ����� ���� ��������

      FindList[ItemIndex]->FindData.strFileName = strLastDirName;
      /* $ 07.04.2002 KM
        - ������ ������� ����� ���������� ����, � ���������
          ������ �� �������� ������� �� ������� �� ������.
          Used=0 - ��� �� �������� �� ��������� ������.
      */
      FindList[ItemIndex]->Used=0;
      // �������� ������� � ��������, ���-�� �� �� ��� ������ :)
      FindList[ItemIndex]->FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
      if (FindFileArcIndex != LIST_INDEX_NONE)
        FindList[ItemIndex]->ArcIndex = FindFileArcIndex;

      ListBox->SetUserData((void*)(DWORD_PTR)ItemIndex,sizeof(ItemIndex),
                           ListBox->AddItem(&ListItem));
    }

    ffCS.Leave ();
  }

  /* $ 24.03.2002 KM
     ������������� ��������� � ������ ����������
     �����. ��� ���� � 1.65 � � 3 ����, �� ���
     ������ ��������� ������ � ��� ���-�� �������,
     ������ ��������� �� �����.
  */

  ffCS.Enter ();

  DWORD ItemIndex = AddFindListItem(FindData);
  if (ItemIndex != LIST_INDEX_NONE)
  {

    FindList[ItemIndex]->FindData.strFileName = FullName;
    /* $ 07.04.2002 KM
       Used=1 - ��� �������� �� ��������� ������.
    */
    FindList[ItemIndex]->Used=1;
    if (FindFileArcIndex != LIST_INDEX_NONE)
      FindList[ItemIndex]->ArcIndex = FindFileArcIndex;
  }
  ListItem.strName = strMenuText;

  ffCS.Leave ();
  /* $ 17.01.2002 VVM
    ! �������� ����� �� � ���������, � � ������. ���� �� �������� ��������� */
//  ListItem.SetSelect(!FindFileCount);

  int ListPos = ListBox->AddItem(&ListItem);
  ListBox->SetUserData((void*)(DWORD_PTR)ItemIndex,sizeof(ItemIndex), ListPos);
  // ������� ��� �������� - � ������.
  if (!FindFileCount && !FindDirCount)
    ListBox->SetSelectPos(ListPos, -1);

  statusCS.Enter();

  if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    FindDirCount++;
  else
    FindFileCount++;

  statusCS.Leave();

  LastFoundNumber++;
  FindCountReady=TRUE;

}


int FindFiles::LookForString(const wchar_t *Name)
{
	int Length;
	if ((Length=strFindStr.GetLength())==0)
    	return(TRUE);

	char FindStr[512];
	UnicodeToAnsi(strFindStr, FindStr, 512);

	HANDLE FileHandle=apiCreateFile (Name, FILE_READ_ATTRIBUTES|FILE_READ_DATA|FILE_WRITE_ATTRIBUTES,
									FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
									FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	FILETIME LastAccess;
	int TimeRead=0;
	if (FileHandle==INVALID_HANDLE_VALUE)
		FileHandle=apiCreateFile (Name, FILE_READ_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
								OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	else
		TimeRead=GetFileTime (FileHandle, NULL, &LastAccess, NULL);

	if (FileHandle==INVALID_HANDLE_VALUE) return (FALSE);

	char Buf[32768],SaveBuf[32768],CmpStr[sizeof(FindStr)];
	int ReadSize=0,SaveReadSize=0;

	if (SearchHex)
	{
		int LenCmpStr=sizeof(CmpStr);
		TransformA((unsigned char *)CmpStr,LenCmpStr,(char *)FindStr,'S');
		Length=LenCmpStr;
	}
	else
	{
		xstrncpy(CmpStr,FindStr,sizeof(CmpStr)-1);
	}

	if (!CmpCase && !SearchHex)
		LocalStrupr(CmpStr);

	char *lpWordDiv = NULL;
	if (WholeWords && !SearchHex)
		lpWordDiv = UnicodeToAnsi (Opt.strWordDiv);

	int FirstIteration=TRUE;
	int ReverseBOM=FALSE;
	int IsFirst=FALSE;

	// ��� ������� �� �����. ������������ ��� ���������
	// � ������������ ��������, � ������� ������������ �����
	__int64 AlreadyRead=_i64(0);

	while ( !StopSearch && ReadFile (FileHandle, Buf, sizeof(Buf)/sizeof(Buf[0]), ((LPDWORD)&ReadSize), NULL) )
	{
		if (ReadSize==0) break;
		/* $ 12.04.2005 KM
		���� ������������ ����������� �� ������ �� ������ ������ �� �����,
		�������� �� ������� �� �� ��� ������� ����������� ����������� �������
		*/
		if (EnableSearchInFirst && SearchInFirst)
		{
			if (AlreadyRead+ReadSize>SearchInFirst)
			{
				ReadSize=static_cast<int>(SearchInFirst-AlreadyRead);
			}

			if (ReadSize<=0) break;

			AlreadyRead+=ReadSize;
		}

		/* $ 11.09.2005 KM
		   - Bug #1377
		*/
		int DecodeTableNum=0;
		int ANSISearch=UseANSI;
		int UnicodeSearch=UseUnicode;
		int RealReadSize=ReadSize;

		// ������� � ������ ���������� ���������� ���������: OEM, ANSI � Unicode
		int UseInnerTables=true;

		/* $ 22.09.2003 KM
		   ����� �� hex-�����
		*/
		if (!SearchHex)
		{
			if (UseAllTables || UseUnicode)
			{
				// ��� ����� ��� �� ���� ���������� ��� � �������,
				// �������� ��������� ������ ������
				SaveReadSize=ReadSize;

				// � ����� �������� ��������� �����.
				memcpy(SaveBuf,Buf,ReadSize);

			/* $ 21.10.2000 SVS
			   ������� ����������, � ��� ��� �������, �� ���� � FFFE-������
			*/
				if(!IsFirst)
				{
					IsFirst=TRUE;
					if(*(WORD*)Buf == 0xFFFE)	// The text contains the Unicode
					ReverseBOM=TRUE;			// byte-reversed byte-order mark
												// (Reverse BOM) 0xFFFE as its first character.
				}
			}
		}

		while (1)
		{
			/* $ 22.09.2003 KM
			   ����� �� hex-�����
			*/
			if (!SearchHex)
			{
				if (UnicodeSearch)
				{
					char BOMBuf[sizeof(SaveBuf)];
					char *BufPtr=SaveBuf;

					if(ReverseBOM)
					{
						BufPtr=BOMBuf;

						for(int I=0; I < SaveReadSize; I+=2)
						{
							BOMBuf[I]=SaveBuf[I+1];
							BOMBuf[I+1]=SaveBuf[I];
						}
					}

					WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)BufPtr,SaveReadSize/2,Buf,sizeof(Buf),NULL,NULL);
					ReadSize=SaveReadSize/2;
				}
				else
				{
					// ���� ����� ��� �� ���� ����������, ����������� ����������� �����
					// � ��� ������ ��� ������ � ������������ ��������� ������ � ������ ���������
					if (UseAllTables)
					{
						ReadSize=SaveReadSize;
						memcpy(Buf,SaveBuf,ReadSize);
					}

					/* $ 20.09.2003 KM
					  ������� ��������� ANSI �������
					*/
					if (UseDecodeTable || DecodeTableNum>0 || ANSISearch)
					{
						// ��� ��������� �� ������ � ANSI ���������, ���������� ������� �������������
						if (ANSISearch)
							GetTable(&TableSet,TRUE,TableNum,UseUnicode);

						for (int I=0;I<ReadSize;I++)
							Buf[I]=TableSet.DecodeTable[(unsigned)Buf[I]];
					}
				}
				if (!CmpCase)
					LocalUpperBuf(Buf,ReadSize);
			}

			int CheckSize=ReadSize-Length+1;
			/* $ 30.07.2000 KM
			  ��������� "Whole words" � ������
			*/
			for (int I=0;I<CheckSize;I++)
			{
				int cmpResult;
				int locResultLeft=FALSE;
				int locResultRight=FALSE;

				/* $ 22.09.2003 KM
				  ����� �� hex-�����
				*/

				if (WholeWords && !SearchHex)
				{
					if (!FirstIteration)
					{
						if (IsSpaceA(Buf[I-1]) || IsEolA(Buf[I-1]) ||
							(strchr(lpWordDiv,Buf[I-1])!=NULL))
							locResultLeft=TRUE;
					}
					else
					{
						FirstIteration=FALSE;
						locResultLeft=TRUE;
					}

					if (RealReadSize!=sizeof(Buf) && I+Length>=RealReadSize)
						locResultRight=TRUE;
					else
						if (I+Length<RealReadSize &&
							(IsSpaceA(Buf[I+Length]) || IsEolA(Buf[I+Length]) ||
							(strchr(lpWordDiv,Buf[I+Length])!=NULL)))
								locResultRight=TRUE;
				}
				else
				{
					locResultLeft=TRUE;
					locResultRight=TRUE;
				}

				cmpResult=locResultLeft && locResultRight && CmpStr[0]==Buf[I] &&
					(Length==1 || (CmpStr[1]==Buf[I+1] &&
					(Length==2 || memcmp(CmpStr+2,&Buf[I+2],Length-2)==0)));

				if (cmpResult)
				{
					if (lpWordDiv)
						xf_free (lpWordDiv);
					if (TimeRead)
						SetFileTime(FileHandle,NULL,&LastAccess,NULL);
					CloseHandle (FileHandle);
					return(TRUE);
				}
			}
			/* $ 22.09.2003 KM
			  ����� �� hex-�����
			*/
			if (UseAllTables && !SearchHex)
			{
				if (!ANSISearch && !UnicodeSearch && UseInnerTables)
				{
					// ��� �� ����� � OEM ���������, ������ ������ � ANSI.
					ANSISearch=true;
				}
				else
				{
					if (!UnicodeSearch && UseInnerTables)
					{
						// �� ����� � ANSI ���������, ������ ������ � Unicode.
						UnicodeSearch=true;
						ANSISearch=false;
					}
					else
					{
						// ���������� ������� ��� ������, ������ �������� ������ � ����������������
						UseInnerTables=false;

						UnicodeSearch=false;
						ANSISearch=false;

						// �� ����� �� � OEM, �� � ANSI, �� � Unicode, ������ ����� ������
						// � ������������� ���������������� ����������.
						if (PrepareTable(&TableSet,DecodeTableNum,TRUE))
						{
							DecodeTableNum++;
							xstrncpy(CmpStr,FindStr,sizeof(CmpStr)-1);
							if (!CmpCase)
								LocalStrupr(CmpStr);
						}
						else
							break;
					}
				}
			}
			else
				break;
		}

		if (RealReadSize==sizeof(Buf)/sizeof(Buf[0]))
		{
			/* $ 22.09.2003 KM
			  ����� �� hex-�����
			*/
			/* $ 30.07.2000 KM
			  ��������� offset ��� ������ ������ ����� � ������ WordDiv
			*/
			//��� ������ �� ���� �������� �� �� ���� ��� ����� ���������� ����� � � �������
			//����� �� �������� FileSize/sizeof(Buf)*(Length+1) ���� ����� �������
			//�� ���� ��� �� ������ �� ��� ������ �� ���� �������� � ������� �� �����
			//��������� ���� ���������� ������.
			int offset=(Length/*+1*/);
			if ((UseAllTables || UnicodeSearch) && !SearchHex) offset*=2;

			if ( INVALID_SET_FILE_POINTER != SetFilePointer (FileHandle, -offset, NULL, FILE_CURRENT) )
			{
				if ((EnableSearchInFirst && SearchInFirst) ) AlreadyRead-=offset;
			}
		}
	}
	if (lpWordDiv)
		xf_free (lpWordDiv);
	if (TimeRead)
		SetFileTime(FileHandle,NULL,&LastAccess,NULL);
	CloseHandle (FileHandle);
	return (FALSE);
}

void FindFiles::DoPreparePluginList(void* Param, string& strSaveDir)
{
  TRY {

    Sleep(200);
    strPluginSearchPath=L"";
    HANDLE hPlugin=ArcList[FindFileArcIndex]->hPlugin;
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    strSaveDir = Info.CurDir;
    WaitForSingleObject(hPluginMutex,INFINITE);
    if (SearchMode==FFSEARCH_ROOT ||
        SearchMode==FFSEARCH_ALL ||
        SearchMode==FFSEARCH_ALL_BUTNETWORK ||
        SearchMode==FFSEARCH_INPATH)
      CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_FIND);
    ReleaseMutex(hPluginMutex);
    RecurseLevel=0;
    ScanPluginTree(hPlugin,ArcList[FindFileArcIndex]->Flags);
    WaitForSingleObject(hPluginMutex,INFINITE);
    if (SearchMode==FFSEARCH_ROOT ||
        SearchMode==FFSEARCH_ALL ||
        SearchMode==FFSEARCH_ALL_BUTNETWORK ||
        SearchMode==FFSEARCH_INPATH)
      CtrlObject->Plugins.SetDirectory(hPlugin,strSaveDir,OPM_FIND);
    ReleaseMutex(hPluginMutex);
    while (!StopSearch && FindMessageReady)
      Sleep(10);
    if (Param==NULL)
    {
      statusCS.Enter();

      strFindMessage.Format(MSG(MFindDone),FindFileCount,FindDirCount);
      FindMessageReady=TRUE;
      SearchDone=TRUE;

      statusCS.Leave();
    }
  }
  EXCEPT (xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
  {
    TerminateProcess( GetCurrentProcess(), 1);
  }
}

void _cdecl FindFiles::PreparePluginList(void *Param)
{
  string strSaveDir;

  DoPreparePluginList(Param, strSaveDir);
}

void FindFiles::ScanPluginTree(HANDLE hPlugin, DWORD Flags)
{
  PluginPanelItem *PanelData=NULL;
  int ItemCount=0;

  WaitForSingleObject(hPluginMutex,INFINITE);
  if (StopSearch || !CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&ItemCount,OPM_FIND))
  {
    ReleaseMutex(hPluginMutex);
    return;
  }
  else
    ReleaseMutex(hPluginMutex);
  RecurseLevel++;

  /* $ 19.01.2003 KM
     ������� ���� ����������, ���-�� ���������� ����������
     �� ������ ��, ������� � ������.
  */
  /* $ 24.03.2002 KM
     ���������� � �������� �� ������ ����� ������ � ������,
     ���� ��� ����� �������� � ���� ����, � �� ������ ���
     OPIF_REALNAMES, � ��������� ������ � ����������� ������
     ���������� ���������� ���������� ������ ��������������
     ���������� ����� �� ������.
  */
//  if (PanelData && strlen(PointToName(PanelData->FindData.cFileName))>0)
//    far_qsort((void *)PanelData,ItemCount,sizeof(*PanelData),SortItems);

  if (SearchMode!=FFSEARCH_SELECTED || RecurseLevel!=1)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      while (PauseSearch)
        Sleep(10);

      PluginPanelItem *CurPanelItem=PanelData+I;
      string strCurName=CurPanelItem->FindData.lpwszFileName;
      string strFullName;
      if (StrCmp(strCurName,L".")==0 || TestParentFolderName(strCurName))
        continue;
      if (Flags & OPIF_REALNAMES)
      {
        strFullName = strCurName;
      }
      else
      {
        strFullName = strPluginSearchPath;
        strFullName += strCurName;
      }

      /* $ 30.09.2003 KM
        ����������� ����� �� ���������� � ����������� ������
      */
      if (!UseFilter || Filter->FileInFilter(&CurPanelItem->FindData))
      {
        /* $ 14.06.2004 KM
          ��������� �������� ��� ��������� ���������
        */
        if (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          statusCS.Enter();

          strFindMessage = strFullName;
          RemovePseudoBackSlash(strFindMessage);
          FindMessageReady=TRUE;

          statusCS.Leave();

        }

        if (IsFileIncluded(CurPanelItem,strCurName,CurPanelItem->FindData.dwFileAttributes))
          AddMenuRecord(strFullName,&CurPanelItem->FindData);

        if (SearchInArchives && (hPlugin != INVALID_HANDLE_VALUE) && (Flags & OPIF_REALNAMES))
          ArchiveSearch(strFullName);
      }
    }
  }
  if (SearchMode!=FFSEARCH_CURRENT_ONLY)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      PluginPanelItem *CurPanelItem=PanelData+I;
      string strCurName=CurPanelItem->FindData.lpwszFileName;
      if ((CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
          StrCmp(strCurName,L".")!=0 && !TestParentFolderName(strCurName) &&
          (!UseFilter || Filter->FileInFilter(&CurPanelItem->FindData)) &&
          (SearchMode!=FFSEARCH_SELECTED || RecurseLevel!=1 ||
          CtrlObject->Cp()->ActivePanel->IsSelected(strCurName)))
      {
        WaitForSingleObject(hPluginMutex,INFINITE);

				size_t pos;
        if (!strCurName.Contains(L'\x1') && CtrlObject->Plugins.SetDirectory(hPlugin,strCurName,OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);

          strPluginSearchPath += strCurName;
          strPluginSearchPath += L"\x1";
          ScanPluginTree(hPlugin, Flags);
          if (strPluginSearchPath.RPos(pos,L'\x1'))
            strPluginSearchPath.SetLength(pos);
        }
        if (strPluginSearchPath.RPos(pos,L'\x1'))
          strPluginSearchPath.SetLength(pos+1);
        else
          strPluginSearchPath.SetLength(0);
        WaitForSingleObject(hPluginMutex,INFINITE);
        if (!CtrlObject->Plugins.SetDirectory(hPlugin,L"..",OPM_FIND))
          StopSearch=TRUE;
        ReleaseMutex(hPluginMutex);
        if (StopSearch) break;
      }
      else
        ReleaseMutex(hPluginMutex);
    }
  }
  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,ItemCount);
  RecurseLevel--;
}

void _cdecl FindFiles::WriteDialogData(void *Param)
{
  string strDataStr;

  WriteDataUsed=TRUE;

  while( true )
  {
    Dialog* Dlg=(Dialog*)hDlg;

    if( !Dlg )
      break;

    VMenu *ListBox=Dlg->Item[1]->ListPtr;

    if (ListBox && !PauseSearch && !ScreenSaverActive)
    {
      if (BreakMainThread)
        StopSearch=TRUE;

      if (FindCountReady)
      {
        statusCS.Enter ();

        strDataStr.Format (L" %s: %d ", MSG(MFindFound),FindFileCount+FindDirCount);

        statusCS.Leave ();

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)(const wchar_t*)strDataStr);

        FindCountReady=FALSE;
      }

      if (FindMessageReady)
      {
        string strSearchStr;

        if ( !strFindStr.IsEmpty() )
        {
          string strTemp, strFStr;

          strFStr = strFindStr;

          TruncStrFromEnd(strFStr,10);

          strTemp.Format (L" \"%s\"", (const wchar_t*)strFStr);
          strSearchStr.Format (MSG(MFindSearchingIn), (const wchar_t*)strTemp);
        }
        else
          strSearchStr.Format (MSG(MFindSearchingIn), L"");

        int Wid1=(int)strSearchStr.GetLength();
        int Wid2=DlgWidth-(int)strSearchStr.GetLength()-1;

        if (SearchDone)
        {
          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

          strDataStr = MSG(MFindCancel);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,9,(LONG_PTR)(const wchar_t*)strDataStr);

          statusCS.Enter ();
          strDataStr.Format (L"%-*.*s",DlgWidth,DlgWidth, (const wchar_t*)strFindMessage);
          statusCS.Leave ();

          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,3,(LONG_PTR)(const wchar_t*)strDataStr);

          strDataStr = L"";
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)(const wchar_t*)strDataStr);

          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);

          SetFarTitle(strFindMessage);
          StopSearch=TRUE;
        }
        else
        {
          statusCS.Enter ();

          TruncPathStr(strFindMessage, Wid2);

          strDataStr.Format(L"%-*.*s %-*.*s",Wid1,Wid1, (const wchar_t*)strSearchStr,Wid2,Wid2,(const wchar_t*)strFindMessage);
          statusCS.Leave ();

          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,3,(LONG_PTR)(const wchar_t*)strDataStr);
        }

        FindMessageReady=FALSE;
      }

      if (LastFoundNumber && ListBox)
      {
        LastFoundNumber=0;

        if ( ListBox->UpdateRequired () )
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,1,1);
      }
    }

    if (StopSearch && SearchDone && !FindMessageReady && !FindCountReady && !LastFoundNumber)
        break;

    Sleep(20);
  }

  WriteDataUsed=FALSE;
}

BOOL FindFiles::FindListGrow()
{
  DWORD Delta = (FindListCapacity < 256)?LIST_DELTA:FindListCapacity/2;
  LPFINDLIST* NewList = (LPFINDLIST*)xf_realloc(FindList, (FindListCapacity + Delta) * sizeof(*FindList));
  if (NewList)
  {
    FindList = NewList;
    FindListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

BOOL FindFiles::ArcListGrow()
{
  DWORD Delta = (ArcListCapacity < 256)?LIST_DELTA:ArcListCapacity/2;
  LPARCLIST* NewList = (LPARCLIST*)xf_realloc(ArcList, (ArcListCapacity + Delta) * sizeof(*ArcList));
  if (NewList)
  {
    ArcList = NewList;
    ArcListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

DWORD FindFiles::AddFindListItem(FAR_FIND_DATA_EX *FindData)
{
  if ((FindListCount == FindListCapacity) &&
      (!FindListGrow()))
    return(LIST_INDEX_NONE);

  FindList[FindListCount] = new _FINDLIST;

  FindList[FindListCount]->FindData = *FindData;
  FindList[FindListCount]->ArcIndex = LIST_INDEX_NONE;
  return(FindListCount++);
}

DWORD FindFiles::AddArcListItem(const wchar_t *ArcName, HANDLE hPlugin,
                                DWORD dwFlags, const wchar_t *RootPath)
{
  if ((ArcListCount == ArcListCapacity) &&
      (!ArcListGrow()))
    return(LIST_INDEX_NONE);

  ArcList[ArcListCount] = new _ARCLIST;

  ArcList[ArcListCount]->strArcName = ArcName;
  ArcList[ArcListCount]->hPlugin = hPlugin;
  ArcList[ArcListCount]->Flags = dwFlags;
  ArcList[ArcListCount]->strRootPath = RootPath;
  AddEndSlash(ArcList[ArcListCount]->strRootPath);
  return(ArcListCount++);
}

void FindFiles::ClearAllLists()
{
  CriticalSectionLock Lock(ffCS);

  FindFileArcIndex = LIST_INDEX_NONE;

  if (FindList)
  {
      for (unsigned int i = 0; i < FindListCount; i++)
          delete FindList[i];

    xf_free(FindList);
  }
  FindList = NULL;
  FindListCapacity = FindListCount = 0;

  if (ArcList)
  {
      for (unsigned int i = 0; i < ArcListCount; i++)
          delete ArcList[i];

    xf_free(ArcList);
  }
  ArcList = NULL;
  ArcListCapacity = ArcListCount = 0;
}

string &FindFiles::PrepareDriveNameStr(string &strSearchFromRoot, size_t sz)
{
  string strCurDir, strMsgStr, strMsgStr1;
  int MsgLen,DrvLen,MsgLenDiff;

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

  CtrlObject->CmdLine->GetCurDir(strCurDir);

  GetPathRootOne(strCurDir, strCurDir);

  if (strCurDir.At(strCurDir.GetLength()-1)==L'\\')
    strCurDir.SetLength(strCurDir.GetLength()-1);

  if (strCurDir.IsEmpty() || PluginMode)
  {
    strSearchFromRoot = MSG(MSearchFromRootFolder);
    strMsgStr1 = strSearchFromRoot;

    RemoveHighlights(strMsgStr1);
    MsgLen=(int)strMsgStr1.GetLength();

    // ������� � ����� ����� � '&' � ���. ����� ��� �����������
    // ����� ������ ����� ������, ��� ����� '&'
    MsgLenDiff=(int)strSearchFromRoot.GetLength()-MsgLen;
  }
  else
  {
    strMsgStr = MSG(MSearchFromRootOfDrive);
    strMsgStr1 = strMsgStr;

    RemoveHighlights(strMsgStr1);
    MsgLen=(int)strMsgStr1.GetLength();

    // ������� � ����� ����� � '&' � ���. ����� ��� �����������
    // ����� ������ ����� ������, ��� ����� '&'
    MsgLenDiff=(int)strMsgStr.GetLength()-MsgLen;

    DrvLen=(int)sz-MsgLen-1-MsgLenDiff; // -1 - ��� ������ ����� ������� � ������, -MsgLenDiff - ��� ���� ������� '&'
    if (DrvLen<7)
    {
      DrvLen=7; // ������� ����������� ������ ����� ����� 7 ��������
                // (���� ������ TruncPathStr � UNC, ����� ���� ���-�� ���� �����)
      MsgLen=(int)sz-DrvLen-1-MsgLenDiff;
    }

    TruncStrFromEnd(strMsgStr,MsgLen+MsgLenDiff);
    TruncPathStr(strCurDir,DrvLen);

    strSearchFromRoot.Format (L"%s %s", (const wchar_t*)strMsgStr, (const wchar_t*)strCurDir);
  }

  //SearchFromRoot[sz-1+MsgLenDiff]=0; //BUGBUG

  return strSearchFromRoot;
}

wchar_t *FindFiles::RemovePseudoBackSlash(wchar_t *FileName)
{
	for (int i=0;FileName[i]!=0;i++)
	{
		if (FileName[i]==L'\x1')
			FileName[i]=L'\\';
	}
	return FileName;
}

string &FindFiles::RemovePseudoBackSlash(string &strFileName)
{
	wchar_t *lpwszFileName = strFileName.GetBuffer ();

	RemovePseudoBackSlash (lpwszFileName);

	strFileName.ReleaseBuffer ();

	return strFileName;
}
