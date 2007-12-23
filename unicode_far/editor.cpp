/*
editor.cpp

��������
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
#include "editor.hpp"
#include "edit.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "poscache.hpp"
#include "chgprior.hpp"
#include "filestr.hpp"
#include "dialog.hpp"
#include "fileedit.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"

static int ReplaceMode,ReplaceAll;

static int EditorID=0;

// struct EditorUndoData
enum {UNDO_NONE=0,UNDO_EDIT,UNDO_INSSTR,UNDO_DELSTR};

Editor::Editor(ScreenObject *pOwner,bool DialogUsed)
{
  _KEYMACRO(SysLog(L"Editor::Editor()"));
  _KEYMACRO(SysLog(1));

  Opt.EdOpt.CopyTo (EdOpt);
  SetOwner (pOwner);

  if(DialogUsed)
    Flags.Set(FEDITOR_DIALOGMEMOEDIT);

  /* $ 26.10.2003 KM
     ���� ���������� ���������� ����� ������ 16-������ �����, �����
     ������������� GlobalSearchString � ������, ��� ��� �������� ������ �
     16-������ �������������.
  */
  if (GlobalSearchHex)
  {
    /*int LenSearchStr=sizeof(LastSearchStr);
    Transform(LastSearchStr,LenSearchStr,GlobalSearchString,'S');*/ //BUGBUG
  }
  else
    strLastSearchStr.SetData (GlobalSearchString, CP_OEMCP); //BUGBUG;

  LastSearchCase=GlobalSearchCase;
  LastSearchWholeWords=GlobalSearchWholeWords;
  LastSearchReverse=GlobalSearchReverse;

  Pasting=0;
  NumLine=0;
  NumLastLine=0;
  LastChangeStrPos=0;
  BlockStart=NULL;
  BlockStartLine=0;
  /* $ 12.01.2002 IS
     �� ��������� ����� ������ ��� ��� ����� ����� \r\n, ������� ������
     ������� �����, �������� ��� ����.
  */
  wcscpy(GlobalEOL,DOS_EOL_fmt);

  UndoData=static_cast<EditorUndoData*>(xf_malloc(EdOpt.UndoSize*sizeof(EditorUndoData)));
  if(UndoData)
    memset(UndoData,0,EdOpt.UndoSize*sizeof(EditorUndoData));

  UndoDataPos=0;
  StartLine=StartChar=-1;
  BlockUndo=FALSE;
  VBlockStart=NULL;
  memset(&SavePos,0xff,sizeof(SavePos));
  MaxRightPos=0;
  UndoSavePos=0;
  Editor::EditorID=::EditorID++;

  HostFileEditor=NULL;

  InsertString (NULL, 0);

  m_codepage = CP_OEMCP; //BUGBUG
}


Editor::~Editor()
{
  //_SVS(SysLog(L"[%p] Editor::~Editor()",this));
  FreeAllocatedData();

  KeepInitParameters();

  _KEYMACRO(SysLog(-1));
  _KEYMACRO(SysLog(L"Editor::~Editor()"));
}

void Editor::FreeAllocatedData(bool FreeUndo)
{
  while (EndList!=NULL)
  {
    Edit *Prev=EndList->m_prev;
    delete EndList;
    EndList=Prev;
  }

  if ( UndoData )
  {
    for (int I=0;I<EdOpt.UndoSize;++I)
      if (UndoData[I].Type!=UNDO_NONE && UndoData[I].Str!=NULL)
        delete UndoData[I].Str;

    if(FreeUndo)
    {
      xf_free(UndoData);
      UndoData=NULL;
    }
    else
    {
      memset(UndoData,0,EdOpt.UndoSize*sizeof(EditorUndoData));
    }
  }

  TopList=EndList=CurLine=NULL;
  NumLastLine = 0;
}

void Editor::KeepInitParameters()
{
  /* $ 26.10.2003 KM
     ! �������������� GlobalSearchString � ������ ����������� 16-������� ������,
       � ����� ���� �� ������ �� ������ ��� ���� ������� ������ �� ����������
       ����� ������ � ���������.
  */
  // ���������� ���������� ����� ������ 16-������ ������?
  if (GlobalSearchHex)
  {
    // ��! ����� ��������, ���������� �� LastSearchStr � ��������� ������������� GlobalSearchString...
   /* char SearchStr[2*NM];
    int LenSearchStr=sizeof(SearchStr);
    Transform((unsigned char *)SearchStr,LenSearchStr,(char *)GlobalSearchString,'S');

    // LastSearchStr ���������� �� ���������� ������������� GlobalSearchString
    if (memcmp(LastSearchStr,SearchStr,LenSearchStr)!=0)
    {
      // ��! ����������, ������ ������������� ����� �� ���������, �������
      // ������������� ��� �������� � 16-������ �������������.
      int LenSearchStr=sizeof(GlobalSearchString);
      Transform((unsigned char *)GlobalSearchString,LenSearchStr,(char *)LastSearchStr,'X');
    }*/ //BUGBUG
  }
  else
      UnicodeToAnsi(strLastSearchStr, GlobalSearchString, sizeof (GlobalSearchString)-1);

  GlobalSearchCase=LastSearchCase;
  GlobalSearchWholeWords=LastSearchWholeWords;
  GlobalSearchReverse=LastSearchReverse;
}

/*
int Editor::ReadFile(const wchar_t *Name,int &UserBreak, EditorCacheParams *pp)
{
  FILE *EditFile;
  Edit *PrevPtr;
  int Count=0,LastLineCR=0,MessageShown=FALSE;

  UserBreak=0;
  Flags.Clear(FEDITOR_OPENFAILED);


  HANDLE hEdit = FAR_CreateFileW (
      Name,
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL
      );

  if ( hEdit == INVALID_HANDLE_VALUE )
  {
    int LastError=GetLastError();
    SetLastError(LastError);

    if ( (LastError != ERROR_FILE_NOT_FOUND) &&
       (LastError != ERROR_PATH_NOT_FOUND) )
    {
      UserBreak = -1;
      Flags.Set(FEDITOR_OPENFAILED);
    }

    return FALSE;
  }

  int EditHandle=_open_osfhandle((long)hEdit,O_BINARY);

  if ( EditHandle == -1 )
    return FALSE;

  if ((EditFile=fdopen(EditHandle,"rb"))==NULL)
    return FALSE;

  if ( GetFileType(hEdit) != FILE_TYPE_DISK )
  {
    fclose(EditFile);
    SetLastError(ERROR_INVALID_NAME);

    UserBreak=-1;
    Flags.Set(FEDITOR_OPENFAILED);
    return FALSE;
  }


  if(EdOpt.FileSizeLimitLo || EdOpt.FileSizeLimitHi)
  {
    unsigned __int64 RealSizeFile;

    if ( apiGetFileSize(hEdit, &RealSizeFile) )
    {
      unsigned __int64 NeedSizeFile = EdOpt.FileSizeLimitHi*_ui64(0x100000000)+EdOpt.FileSizeLimitLo;
      if(RealSizeFile > NeedSizeFile)
      {
        string strTempStr1, strTempStr2, strTempStr3, strTempStr4;
        // ������ = 8 - ��� �����... � Kb � ����...
        FileSizeToStr(strTempStr1,RealSizeFile,8);
        FileSizeToStr(strTempStr2,NeedSizeFile,8);
        strTempStr3.Format (UMSG(MEditFileLong),(const wchar_t *)RemoveExternalSpaces(strTempStr1));
        strTempStr4.Format (UMSG(MEditFileLong2),(const wchar_t *)RemoveExternalSpaces(strTempStr2));

        if(Message(MSG_WARNING,2,UMSG(MEditTitle),
                    Name,
                    strTempStr3,
                    strTempStr4,
                    UMSG(MEditROOpen),
                    UMSG(MYes),UMSG(MNo)))
        {
          fclose(EditFile);
          SetLastError(ERROR_OPEN_FAILED);
          UserBreak=1;
          Flags.Set(FEDITOR_OPENFAILED);
          return(FALSE);
        }
      }
    }
  }
  {
    DWORD FileAttributes=HostFileEditor?HostFileEditor->GetFileAttributes(Name):(DWORD)-1;
    if((EdOpt.ReadOnlyLock&1) &&
       FileAttributes != -1 &&
       (FileAttributes &
          (FILE_ATTRIBUTE_READONLY|
             ((EdOpt.ReadOnlyLock&0x60)>>4)
          )
       )
     )
      Flags.Swap(FEDITOR_LOCKMODE);
  }
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    GetFileString GetStr(EditFile);
    //SaveScreen SaveScr;
    NumLastLine=0;
    *GlobalEOL=0;
    wchar_t *Str;
    int StrLength,GetCode;

    clock_t StartTime=clock();

    if (EdOpt.AutoDetectTable)
    {
      UseDecodeTable=DetectTable(EditFile,&TableSet,TableNum);
      AnsiText=FALSE;
    }

    int nCodePage = GetFileFormat (EditFile);

    while ((GetCode=GetStr.GetString(&Str, nCodePage, StrLength))!=0)
    {
      if (GetCode==-1)
      {
        fclose(EditFile);
        SetPreRedrawFunc(NULL);
        return(FALSE);
      }
      LastLineCR=0;

      if ((++Count & 0xfff)==0 && clock()-StartTime>500)
      {
        if (CheckForEsc())
        {
          UserBreak=1;
          fclose(EditFile);
          SetPreRedrawFunc(NULL);
          return(FALSE);
        }
        if (!MessageShown)
        {
          SetCursorType(FALSE,0);
          SetPreRedrawFunc(Editor::PR_EditorShowMsg);
          EditorShowMsg(UMSG(MEditTitle),UMSG(MEditReading),Name);
          MessageShown=TRUE;
        }
      }

      const wchar_t *CurEOL;
      if (!LastLineCR && ((CurEOL=wmemchr(Str,L'\r',StrLength))!=NULL ||
          (CurEOL=wmemchr(Str,L'\n',StrLength))!=NULL))
      {
        xwcsncpy(GlobalEOL,CurEOL,countof(GlobalEOL)-1);
        GlobalEOL[countof(GlobalEOL)-1]=0;
        LastLineCR=1;
      }

      AddString (Str, StrLength);
    }
    SetPreRedrawFunc(NULL);

    if (LastLineCR)
       AddString (L"", sizeof (wchar_t));

  }
  if (NumLine>0)
    NumLastLine--;
  if (NumLastLine==0)
    NumLastLine=1;
  fclose(EditFile);
  if (StartLine==-2)
  {
    Edit *CurPtr=TopList;
    long TotalSize=0;
    while (CurPtr!=NULL && CurPtr->m_next!=NULL)
    {
      const wchar_t *SaveStr,*EndSeq;
      int Length;
      CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);
      TotalSize+=Length+StrLength(EndSeq);
      if (TotalSize>StartChar)
        break;
      CurPtr=CurPtr->m_next;
      NumLine++;
    }
    TopScreen=CurLine=CurPtr;

    ParseCacheParams (pp);
    //LoadFromCache (Name);

    if (EdOpt.SavePos && CtrlObject!=NULL)
    {
      unsigned int Line,ScreenLine,LinePos,LeftPos=0;

      string strCacheName;
      if (HostFileEditor && *HostFileEditor->GetPluginData())
        strCacheName.Format (L"%s%s",HostFileEditor->GetPluginData(),PointToNameW(Name));
      else
      {
        strCacheName = Name;

        wchar_t *lpwszCacheName = strCacheName.GetBuffer();
        for(int i=0;lpwszCacheName[i];i++)
        {
          if(lpwszCacheName[i]==L'/')
              lpwszCacheName[i]=L'\\';
        }

        strCacheName.ReleaseBuffer();
      }
      unsigned int Table;
      {
        struct TPosCache32 PosCache={0};
        if(Opt.EdOpt.SaveShortPos)
        {
          PosCache.Position[0]=SavePos.Line;
          PosCache.Position[1]=SavePos.Cursor;
          PosCache.Position[2]=SavePos.ScreenLine;
          PosCache.Position[3]=SavePos.LeftPos;
        }
        CtrlObject->EditorPosCache->GetPosition(strCacheName,&PosCache);
        Line=PosCache.Param[0];
        ScreenLine=PosCache.Param[1];
        LinePos=PosCache.Param[2];
        LeftPos=PosCache.Param[3];
        Table=PosCache.Param[4];
      }
      //_D(SysLog(L"after Get cache, LeftPos=%i",LeftPos));
      if((int)Line < 0) Line=0;
      if((int)ScreenLine < 0) ScreenLine=0;
      if((int)LinePos < 0) LinePos=0;
      if((int)LeftPos < 0) LeftPos=0;
      if((int)Table < 0) Table=0;
      Flags.Change(FEDITOR_TABLECHANGEDBYUSER,(Table!=0));
      switch(Table)
      {
        case 0:
          break;
        case 1:
          AnsiText=UseDecodeTable=0;
          break;
        case 2:
          {
            AnsiText=TRUE;
            UseDecodeTable=TRUE;
            TableNum=0;
            int UseUnicode=FALSE;
            GetTable(&TableSet,TRUE,TableNum,UseUnicode);
          }
          break;
        default:
          AnsiText=0;
          UseDecodeTable=1;
          TableNum=Table-2;
          PrepareTable(&TableSet,Table-3);
          break;
      }
      if (NumLine==Line-ScreenLine)
      {
        Lock ();
        for (DWORD I=0;I<ScreenLine;I++)
          ProcessKey(KEY_DOWN);
        CurLine->SetTabCurPos(LinePos);
        Unlock ();
      }
      //_D(SysLog(L"Setleftpos to %i",LeftPos));
      CurLine->SetLeftPos(LeftPos);
    }
  }
  else
    if (StartLine!=-1 || EdOpt.SavePos && CtrlObject!=NULL)
    {
      unsigned int Line,ScreenLine,LinePos,LeftPos=0;
      if (StartLine!=-1)
      {
        Line=StartLine-1;
        ScreenLine=ObjHeight/2; //ScrY
        if (ScreenLine>Line)
          ScreenLine=Line;
        LinePos=(StartChar>0) ? StartChar-1:0;
      }
      else
      {
        string strCacheName;
        if (HostFileEditor && *HostFileEditor->GetPluginData())
          strCacheName.Format (L"%s%s",HostFileEditor->GetPluginData(),PointToNameW(Name));
        else
        {
          strCacheName = Name;

          wchar_t *lpwszCacheName = strCacheName.GetBuffer();
          for(int i=0;lpwszCacheName[i];i++)
          {
            if(lpwszCacheName[i]==L'/')
                lpwszCacheName[i]=L'\\';
          }

          strCacheName.ReleaseBuffer();
        }
        unsigned int Table;
        {
          struct TPosCache32 PosCache={0};
          if(Opt.EdOpt.SaveShortPos)
          {
            PosCache.Position[0]=SavePos.Line;
            PosCache.Position[1]=SavePos.Cursor;
            PosCache.Position[2]=SavePos.ScreenLine;
            PosCache.Position[3]=SavePos.LeftPos;
          }
          CtrlObject->EditorPosCache->GetPosition(strCacheName,&PosCache);
          Line=PosCache.Param[0];
          ScreenLine=PosCache.Param[1];
          LinePos=PosCache.Param[2];
          LeftPos=PosCache.Param[3];
          Table=PosCache.Param[4];
        }
        //_D(SysLog(L"after Get cache 2, LeftPos=%i",LeftPos));
        if((int)Line < 0) Line=0;
        if((int)ScreenLine < 0) ScreenLine=0;
        if((int)LinePos < 0) LinePos=0;
        if((int)LeftPos < 0) LeftPos=0;
        if((int)Table < 0) Table=0;
        Flags.Change(FEDITOR_TABLECHANGEDBYUSER,(Table!=0));
        switch(Table)
        {
          case 0:
            break;
          case 1:
            AnsiText=UseDecodeTable=0;
            break;
          case 2:
            {
              AnsiText=TRUE;
              UseDecodeTable=TRUE;
              TableNum=0;
              int UseUnicode=FALSE;
              GetTable(&TableSet,TRUE,TableNum,UseUnicode);
            }
            break;
          default:
            AnsiText=0;
            UseDecodeTable=1;
            TableNum=Table-2;
            PrepareTable(&TableSet,Table-3);
            break;
        }
      }
      if (ScreenLine>static_cast<DWORD>(ObjHeight))//ScrY
        ScreenLine=ObjHeight;//ScrY;
      if (Line>=ScreenLine)
      {
        Lock ();
        GoToLine(Line-ScreenLine);
        TopScreen=CurLine;
        for (unsigned int I=0;I<ScreenLine;I++)
          ProcessKey(KEY_DOWN);
        CurLine->SetTabCurPos(LinePos);
        //_D(SysLog(L"Setleftpos 2 to %i",LeftPos));
        CurLine->SetLeftPos(LeftPos);
        Unlock ();
      }
    }
  if (UseDecodeTable)
    for (Edit *CurPtr=TopList;CurPtr!=NULL;CurPtr=CurPtr->m_next)
      CurPtr->SetTables(&TableSet);
  else
    TableNum=0;

  return(TRUE);
}
*/

// �������������� �� ������ � ������
int Editor::ReadData(LPCSTR SrcBuf,int SizeSrcBuf)
{
#if defined(PROJECT_DI_MEMOEDIT)
  Edit *PrevPtr;
  int Count=0,LastLineCR=0;

  UserBreak=0;

  {
    GetFileString GetStr(EditFile);
    NumLastLine=0;
    *GlobalEOL=0;
    char *Str;
    int StrLength,GetCode;

    if (EdOpt.AutoDetectTable)
    {
      UseDecodeTable=DetectTable(EditFile,&TableSet,TableNum);
      AnsiText=FALSE;
    }

    while ((GetCode=GetStr.GetString(&Str,StrLength))!=0)
    {
      if (GetCode==-1)
      {
        return(FALSE);
      }
      LastLineCR=0;

      char *CurEOL;
      if (!LastLineCR && ((CurEOL=(char *)memchr(Str,'\r',StrLength))!=NULL ||
          (CurEOL=(char *)memchr(Str,'\n',StrLength))!=NULL))
      {
        xstrncpy(GlobalEOL,CurEOL,sizeof(GlobalEOL)-1);
        GlobalEOL[sizeof(GlobalEOL)-1]=0;
        LastLineCR=1;
      }

      if (NumLastLine!=0)
      {
        EndList->m_next=new Edit(this);
        if (EndList->m_next==NULL)
        {
          return(FALSE);
        }
        PrevPtr=EndList;
        EndList=EndList->m_next;
        EndList->m_prev=PrevPtr;
        EndList->m_next=NULL;
      }

      EndList->SetTabSize(EdOpt.TabSize);
      EndList->SetPersistentBlocks(EdOpt.PersistentBlocks);
      EndList->SetConvertTabs(EdOpt.ExpandTabs);
      EndList->SetBinaryString(Str,StrLength);
      EndList->SetCurPos(0);
      EndList->SetObjectColor(COL_EDITORTEXT,COL_EDITORSELECTEDTEXT);
      EndList->SetEditorMode(TRUE);
      EndList->SetWordDiv(EdOpt.WordDiv);

      NumLastLine++;
    }

    if (LastLineCR && ((EndList->m_next=new Edit(this))!=NULL))
    {
      PrevPtr=EndList;
      EndList=EndList->m_next;
      EndList->m_prev=PrevPtr;
      EndList->m_next=NULL;
      EndList->SetTabSize(EdOpt.TabSize);
      EndList->SetPersistentBlocks(EdOpt.PersistentBlocks);
      EndList->SetConvertTabs(EdOpt.ExpandTabs);
      EndList->SetString("");
      EndList->SetCurPos(0);
      EndList->SetObjectColor(COL_EDITORTEXT,COL_EDITORSELECTEDTEXT);
      EndList->SetEditorMode(TRUE);
      EndList->SetWordDiv(EdOpt.WordDiv);
      NumLastLine++;
    }
  }

  if (NumLine>0)
    NumLastLine--;

  if (NumLastLine==0)
    NumLastLine=1;

  if (UseDecodeTable)
    for (Edit *CurPtr=TopList;CurPtr!=NULL;CurPtr=CurPtr->m_next)
      CurPtr->SetTables(&TableSet);
  else
    TableNum=0;

#endif
  return(TRUE);
}

/*
  Editor::SaveData - �������������� �� ������ � �����

    DestBuf     - ���� ��������� (���������� �����������!)
    SizeDestBuf - ������ ����������
    TextFormat  - ��� �������� �����
*/
int Editor::SaveData(char **DestBuf,int& SizeDestBuf,int TextFormat)
{
#if defined(PROJECT_DI_MEMOEDIT)
  char *PDest=NULL;
  SizeDestBuf=0; // ����� ������ = 0

  // ���������� EOL
  switch(TextFormat)
  {
    case 1:
      strcpy(GlobalEOL,DOS_EOL_fmt);
      break;
    case 2:
      strcpy(GlobalEOL,UNIX_EOL_fmt);
      break;
    case 3:
      strcpy(GlobalEOL,MAC_EOL_fmt);
      break;
    case 4:
      strcpy(GlobalEOL,WIN_EOL_fmt);
      break;
  }

  int StrLength=0;
  const char *SaveStr, *EndSeq;
  int Length;

  // ��������� ���������� ����� � ����� ������ ������ (����� �� ������� realloc)
  Edit *CurPtr=TopList;

  DWORD AllLength=0;
  while (CurPtr!=NULL)
  {
    CurPtr->GetBinaryString(SaveStr,&EndSeq,Length);
    // ���������� �������� �����
    if (*EndSeq==0 && CurPtr->m_next!=NULL)
      EndSeq=*GlobalEOL ? GlobalEOL:DOS_EOL_fmt;

    if (TextFormat!=0 && *EndSeq!=0)
    {
      if (TextFormat==1)
        EndSeq=DOS_EOL_fmt;
      else if (TextFormat==2)
        EndSeq=UNIX_EOL_fmt;
      else if (TextFormat==3)
        EndSeq=MAC_EOL_fmt;
      else
        EndSeq=WIN_EOL_fmt;

      CurPtr->SetEOL(EndSeq);
    }
    AllLength+=Length+strlen(EndSeq)+16;
  }

  char *MemEditStr=(char *)xf_malloc(sizeof(char) * AllLength);

  if(MemEditStr)
  {
    *MemEditStr=0;
    PDest=MemEditStr;
    // �������� �� ������ �����
    CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->GetBinaryString(SaveStr,&EndSeq,Length);

      strcpy(PDest,SaveStr);
      strcat(PDest,EndSeq);
      PDest+=strlen(PDest);

      CurPtr=CurPtr->m_next;
    }
    SizeDestBuf=strlen(MemEditStr);
    DestBuf=&MemEditStr;
    return TRUE;
  }
  else
    return FALSE;
#else
  return TRUE;
#endif
}


void Editor::DisplayObject()
{
  ShowEditor(FALSE);
}


void Editor::ShowEditor(int CurLineOnly)
{
  if ( Locked () || !TopList )
    return;

  Edit *CurPtr;
  int LeftPos,CurPos,Y;

//_SVS(SysLog(L"Enter to ShowEditor, CurLineOnly=%i",CurLineOnly));
  /*$ 10.08.2000 skv
    To make sure that CurEditor is set to required value.
  */
  if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
    CtrlObject->Plugins.CurEditor=HostFileEditor; // this;

  /* 17.04.2002 skv
    ��� � ������ �� ����� ��� Alt-F9 � ����� �������� �����.
    ���� �� ������ ���� ��������� �����, � ���� ����� ������,
    �����������������.
  */

  if(!EdOpt.AllowEmptySpaceAfterEof)
  {

    while(CalcDistance(TopScreen,NULL,Y2-Y1)<Y2-Y1)
    {
      if(TopScreen->m_prev)
        TopScreen=TopScreen->m_prev;
      else
        break;
    }
  }
  /*
    ���� ������ ����� �������� "�� �������",
    �������� ����� ��� ������, � ��
    ������ ������� � �����.
  */

  while (CalcDistance(TopScreen,CurLine,-1)>=Y2-Y1+1)
  {
    TopScreen=TopScreen->m_next;
    //DisableOut=TRUE;
    //ProcessKey(KEY_UP);
    //DisableOut=FALSE;
  }


  CurPos=CurLine->GetTabCurPos();
  if (!EdOpt.CursorBeyondEOL)
  {
    MaxRightPos=CurPos;
    int RealCurPos=CurLine->GetCurPos();
    int Length=CurLine->GetLength();

    if (RealCurPos>Length)
    {
      CurLine->SetCurPos(Length);
      CurLine->SetLeftPos(0);
      //_D(SysLog(L"call CurLine->FastShow()"));
      //CurLine->FastShow();
      CurPos=CurLine->GetTabCurPos();
    }
  }

  if (!Pasting)
  {
    /*$ 10.08.2000 skv
      Don't send EE_REDRAW while macro is being executed.
      Send EE_REDRAW with param=2 if text was just modified.

    */
    _SYS_EE_REDRAW(CleverSysLog Clev(L"Editor::ShowEditor()"));
    if(!ScrBuf.GetLockCount())
    {
      /*$ 13.09.2000 skv
        EE_REDRAW 1 and 2 replaced.
      */
      if(Flags.Check(FEDITOR_JUSTMODIFIED))
      {
        Flags.Clear(FEDITOR_JUSTMODIFIED);
        if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
        {
          _SYS_EE_REDRAW(SysLog(L"Call ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE)"));
          CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
        }
      }
      else
      {
        if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
        {
          _SYS_EE_REDRAW(SysLog(L"Call ProcessEditorEvent(EE_REDRAW,%s)",(CurLineOnly?"EEREDRAW_LINE":"EEREDRAW_ALL")));
          CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,CurLineOnly?EEREDRAW_LINE:EEREDRAW_ALL);
        }
      }
    }
    _SYS_EE_REDRAW(else SysLog(L"ScrBuf Locked !!!"));
  }

  if (!CurLineOnly)
  {
    LeftPos=CurLine->GetLeftPos();
#if 1
    // ������ ����������������� �����!
    if(CurPos+LeftPos < X2 )
      LeftPos=0;
    else if(CurLine->X2 < X2)
      LeftPos=CurLine->GetLength()-CurPos;
    if(LeftPos < 0)
      LeftPos=0;
#endif

    for (CurPtr=TopScreen,Y=Y1;Y<=Y2;Y++)
      if (CurPtr!=NULL)
      {
        CurPtr->SetEditBeyondEnd(TRUE);
        CurPtr->SetPosition(X1,Y,X2,Y);
        //CurPtr->SetTables(UseDecodeTable ? &TableSet:NULL);
        //_D(SysLog(L"Setleftpos 3 to %i",LeftPos));
        CurPtr->SetLeftPos(LeftPos);
        CurPtr->SetTabCurPos(CurPos);
        CurPtr->FastShow();
        CurPtr->SetEditBeyondEnd(EdOpt.CursorBeyondEOL);
        CurPtr=CurPtr->m_next;
      }
      else
      {
        //GotoXY(X1,Y);
        //SetColor(COL_EDITORTEXT);
        //mprintf("%*s",ObjWidth,"");
        SetScreen(X1,Y,X2,Y,L' ',COL_EDITORTEXT); //??
      }
  }

  CurLine->SetOvertypeMode(Flags.Check(FEDITOR_OVERTYPE));
  CurLine->Show();

  if (VBlockStart!=NULL && VBlockSizeX>0 && VBlockSizeY>0)
  {
    int CurScreenLine=NumLine-CalcDistance(TopScreen,CurLine,-1);
    LeftPos=CurLine->GetLeftPos();
    for (CurPtr=TopScreen,Y=Y1;Y<=Y2;Y++)
    {
      if (CurPtr!=NULL)
      {
        if (CurScreenLine>=VBlockY && CurScreenLine<VBlockY+VBlockSizeY)
        {
          int BlockX1=VBlockX-LeftPos+X1;
          int BlockX2=VBlockX+VBlockSizeX-1-LeftPos+X1;
          if (BlockX1<X1)
            BlockX1=X1;
          if (BlockX2>X2)
            BlockX2=X2;
          if (BlockX1<=X2 && BlockX2>=X1)
            ChangeBlockColor(BlockX1,Y,BlockX2,Y,COL_EDITORSELECTEDTEXT);
        }
        CurPtr=CurPtr->m_next;
        CurScreenLine++;
      }
    }
  }

  if(HostFileEditor) HostFileEditor->ShowStatus();
//_SVS(SysLog(L"Exit from ShowEditor"));
}


/*$ 10.08.2000 skv
  Wrapper for Modified.
  Set JustModified every call to 1
  to track any text state change.
  Even if state==0, this can be
  last UNDO.
*/
void Editor::TextChanged(int State)
{
  Flags.Change(FEDITOR_MODIFIED,State);
  Flags.Set(FEDITOR_JUSTMODIFIED);
}


__int64 Editor::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
  int CurPos=CurLine->GetCurPos();
  switch(OpCode)
  {
    case MCODE_C_EMPTY:
      return (__int64)(!CurLine->m_next && !CurLine->m_prev); //??
    case MCODE_C_EOF:
      return (__int64)(!CurLine->m_next && CurPos>=CurLine->GetLength());
    case MCODE_C_BOF:
      return (__int64)(!CurLine->m_prev && CurPos==0);
    case MCODE_C_SELECTED:
      return (__int64)(BlockStart || VBlockStart?TRUE:FALSE);
    case MCODE_V_EDITORCURPOS:
      return (__int64)(CurLine->GetTabCurPos()+1);
    case MCODE_V_EDITORCURLINE:
      return (__int64)(NumLine+1);
    case MCODE_V_ITEMCOUNT:
    case MCODE_V_EDITORLINES:
      return (__int64)NumLastLine;

  }
  return _i64(0);
}


int Editor::ProcessKey(int Key)
{
  if (Key==KEY_IDLE)
  {
    if (Opt.ViewerEditorClock && HostFileEditor!=NULL && HostFileEditor->IsFullScreen())
      ShowTime(FALSE);
    return(TRUE);
  }

  if (Key==KEY_NONE)
    return(TRUE);

  _KEYMACRO(CleverSysLog SL(L"Editor::ProcessKey()"));
  _KEYMACRO(SysLog(L"Key=%s",_FARKEY_ToName(Key)));

  int CurPos,CurVisPos,I;
  CurPos=CurLine->GetCurPos();
  CurVisPos=GetLineCurPos();

  int isk=IsShiftKey(Key);
  _SVS(SysLog(L"[%d] isk=%d",__LINE__,isk));
  //if ((!isk || CtrlObject->Macro.IsExecuting()) && !isk && !Pasting)
  if (!isk && !Pasting && !(Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE || Key>=KEY_OP_BASE && Key <=KEY_OP_ENDBASE))
  {
    _SVS(SysLog(L"[%d] BlockStart=(%d,%d)",__LINE__,BlockStart,VBlockStart));
    if (BlockStart!=NULL || VBlockStart!=NULL)
    {
      Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
    }
    if ((BlockStart!=NULL || VBlockStart!=NULL) && !EdOpt.PersistentBlocks)
//    if (BlockStart!=NULL || VBlockStart!=NULL && !EdOpt.PersistentBlocks)
    {
      Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
      if (!EdOpt.PersistentBlocks)
      {
        static int UnmarkKeys[]={
           KEY_LEFT,      KEY_NUMPAD4,
           KEY_RIGHT,     KEY_NUMPAD6,
           KEY_HOME,      KEY_NUMPAD7,
           KEY_END,       KEY_NUMPAD1,
           KEY_UP,        KEY_NUMPAD8,
           KEY_DOWN,      KEY_NUMPAD2,
           KEY_PGUP,      KEY_NUMPAD9,
           KEY_PGDN,      KEY_NUMPAD3,
           KEY_CTRLHOME,  KEY_CTRLNUMPAD7,
           KEY_CTRLPGUP,  KEY_CTRLNUMPAD9,
           KEY_CTRLEND,   KEY_CTRLNUMPAD1,
           KEY_CTRLPGDN,  KEY_CTRLNUMPAD3,
           KEY_CTRLLEFT,  KEY_CTRLNUMPAD4,
           KEY_CTRLRIGHT, KEY_CTRLNUMPAD7,
           KEY_CTRLUP,    KEY_CTRLNUMPAD8,
           KEY_CTRLDOWN,  KEY_CTRLNUMPAD2,
           KEY_CTRLN,
           KEY_CTRLE,
           KEY_CTRLS,
        };
        for (int I=0;I<sizeof(UnmarkKeys)/sizeof(UnmarkKeys[0]);I++)
          if (Key==UnmarkKeys[I])
          {
            UnmarkBlock();
            break;
          }
      }
      else
      {
        int StartSel,EndSel;
//        Edit *BStart=!BlockStart?VBlockStart:BlockStart;
//        BStart->GetRealSelection(StartSel,EndSel);
        BlockStart->GetRealSelection(StartSel,EndSel);
        _SVS(SysLog(L"[%d] PersistentBlocks! StartSel=%d, EndSel=%d",__LINE__,StartSel,EndSel));
        if (StartSel==-1 || StartSel==EndSel)
          UnmarkBlock();
      }
    }
  }

  if (Key==KEY_ALTD)
    Key=KEY_CTRLK;

  // ������ � ����������
  if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
    return GotoBookmark(Key-KEY_CTRL0);
  if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
    Key=Key-KEY_CTRLSHIFT0+KEY_RCTRL0;
  if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
    return SetBookmark(Key-KEY_RCTRL0);

  int SelStart=0,SelEnd=0;
  int SelFirst=FALSE;
  int SelAtBeginning=FALSE;

  EditorBlockGuard _bg(*this,&Editor::UnmarkEmptyBlock);

  switch(Key)
  {
    case KEY_SHIFTLEFT:    case KEY_SHIFTRIGHT:
    case KEY_SHIFTUP:      case KEY_SHIFTDOWN:
    case KEY_SHIFTHOME:    case KEY_SHIFTEND:
    case KEY_SHIFTNUMPAD4: case KEY_SHIFTNUMPAD6:
    case KEY_SHIFTNUMPAD8: case KEY_SHIFTNUMPAD2:
    case KEY_SHIFTNUMPAD7: case KEY_SHIFTNUMPAD1:
    case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:   /* 12.11.2002 DJ */
    {
      _KEYMACRO(CleverSysLog SL(L"Editor::ProcessKey(KEY_SHIFT*)"));
      _SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));
      UnmarkEmptyBlock(); // ������ ���������, ���� ��� ������ ����� 0
      _bg.needCheckUnmark=true;
      CurLine->GetRealSelection(SelStart,SelEnd);
      if(Flags.Check(FEDITOR_CURPOSCHANGEDBYPLUGIN))
      {
        if(SelStart!=-1 && (CurPos<SelStart || // ���� ������ �� ���������
           (SelEnd!=-1 && (CurPos>SelEnd ||    // ... ����� ���������
            (CurPos>SelStart && CurPos<SelEnd)))) &&
           CurPos<CurLine->GetLength()) // ... ������ ��������
          Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
        Flags.Clear(FEDITOR_CURPOSCHANGEDBYPLUGIN);
      }

      _SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));
      if (!Flags.Check(FEDITOR_MARKINGBLOCK))
      {
        UnmarkBlock();
        Flags.Set(FEDITOR_MARKINGBLOCK);
        BlockStart=CurLine;
        BlockStartLine=NumLine;
        SelFirst=TRUE;
        SelStart=SelEnd=CurPos;
      }
      else
      {
        SelAtBeginning=CurLine==BlockStart && CurPos==SelStart;
        if(SelStart==-1)
        {
          SelStart=SelEnd=CurPos;
        }
      }
      _SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));
    }
  }

  switch(Key)
  {
    case KEY_CTRLSHIFTPGUP:   case KEY_CTRLSHIFTNUMPAD9:
    case KEY_CTRLSHIFTHOME:   case KEY_CTRLSHIFTNUMPAD7:
    {
      Lock ();
      Pasting++;
      while (CurLine!=TopList)
      {

        ProcessKey(KEY_SHIFTPGUP);
      }

      if(Key == KEY_CTRLSHIFTHOME || Key == KEY_CTRLSHIFTNUMPAD7)
        ProcessKey(KEY_SHIFTHOME);

      Pasting--;
      Unlock ();

      Show();
      return(TRUE);
    }

    case KEY_CTRLSHIFTPGDN:   case KEY_CTRLSHIFTNUMPAD3:
    case KEY_CTRLSHIFTEND:    case KEY_CTRLSHIFTNUMPAD1:
    {
      Lock ();
      Pasting++;
      while (CurLine!=EndList)
      {

        ProcessKey(KEY_SHIFTPGDN);
      }
      /* $ 06.02.2002 IS
         ������������� ������� ���� ����, ��� ������� �������� ��������.
         ��� ����:
           ��� ���������� "ProcessKey(KEY_SHIFTPGDN)" (��. ���� ����)
           ������� ������� (� ���� ������ - �������) ����� �������
           ECTL_SETPOSITION, � ���������� ���� ������������ ����
           FEDITOR_CURPOSCHANGEDBYPLUGIN. � ��� ��������� KEY_SHIFTEND
           ��������� � �������� ������ ���������� � ����, ��� ������ �� ���
           ���������� ���������� KEY_SHIFTPGDN.
      */
      Flags.Clear(FEDITOR_CURPOSCHANGEDBYPLUGIN);

      if(Key == KEY_CTRLSHIFTEND || Key == KEY_CTRLSHIFTNUMPAD1)
        ProcessKey(KEY_SHIFTEND);

      Pasting--;
      Unlock ();

      Show();
      return(TRUE);
    }

    case KEY_SHIFTPGUP:       case KEY_SHIFTNUMPAD9:
    {
      Pasting++;
      Lock ();

      for (I=Y1;I<Y2;I++)
      {
        ProcessKey(KEY_SHIFTUP);
        if(!EdOpt.CursorBeyondEOL)
        {
          if(CurLine->GetCurPos()>CurLine->GetLength())
          {
            CurLine->SetCurPos(CurLine->GetLength());
          }
        }
      }
      Pasting--;
      Unlock ();

      Show();
      return(TRUE);
    }

    case KEY_SHIFTPGDN:       case KEY_SHIFTNUMPAD3:
    {
      Pasting++;
      Lock ();

      for (I=Y1;I<Y2;I++)
      {
        ProcessKey(KEY_SHIFTDOWN);
        if(!EdOpt.CursorBeyondEOL)
        {
          if(CurLine->GetCurPos()>CurLine->GetLength())
          {
            CurLine->SetCurPos(CurLine->GetLength());
          }
        }
      }
      Pasting--;
      Unlock ();

      Show();
      return(TRUE);
    }

    case KEY_SHIFTHOME:       case KEY_SHIFTNUMPAD7:
    {
      Pasting++;
      Lock ();

      if(SelAtBeginning)
      {
        CurLine->Select(0,SelEnd);
      }else
      {
        if(SelStart==0)
        {
          CurLine->Select(-1,0);
        }else
        {
          CurLine->Select(0,SelStart);
        }
      }
      ProcessKey(KEY_HOME);
      Pasting--;
      Unlock ();

      Show();
      return(TRUE);
    }

    case KEY_SHIFTEND:
	case KEY_SHIFTNUMPAD1:
    {
      {
        int LeftPos=CurLine->GetLeftPos();
        Pasting++;
        Lock ();

        int CurLength=CurLine->GetLength();

        if(!SelAtBeginning || SelFirst)
        {
          CurLine->Select(SelStart,CurLength);
        }else
        {
          if(SelEnd!=-1)
            CurLine->Select(SelEnd,CurLength);
          else
            CurLine->Select(CurLength,-1);
        }

        CurLine->ObjWidth=X2-X1;

        ProcessKey(KEY_END);

        Pasting--;
        Unlock ();


        if(EdOpt.PersistentBlocks)
          Show();
        else
        {
          CurLine->FastShow();
          ShowEditor(LeftPos==CurLine->GetLeftPos());
        }
      }
      return(TRUE);
    }

    case KEY_SHIFTLEFT:  case KEY_SHIFTNUMPAD4:
    {
      _SVS(CleverSysLog SL(L"case KEY_SHIFTLEFT"));
      if (CurPos==0 && CurLine->m_prev==NULL)return TRUE;
      if (CurPos==0) //������ � ������ ������
      {
        if(SelAtBeginning) //������ � ������ �����
        {
          BlockStart=CurLine->m_prev;
          CurLine->m_prev->Select(CurLine->m_prev->GetLength(),-1);
        }
        else // ������ � ����� �����
        {
          CurLine->Select(-1,0);
          CurLine->m_prev->GetRealSelection(SelStart,SelEnd);
          CurLine->m_prev->Select(SelStart,CurLine->m_prev->GetLength());
        }
      }
      else
      {
        if(SelAtBeginning || SelFirst)
        {
          CurLine->Select(SelStart-1,SelEnd);
        }
        else
        {
          CurLine->Select(SelStart,SelEnd-1);
        }
      }
      int LeftPos=CurLine->GetLeftPos();
      Edit *OldCur=CurLine;
      int _OldNumLine=NumLine;
      Pasting++;
      ProcessKey(KEY_LEFT);
      Pasting--;

      if(_OldNumLine!=NumLine)
      {
        BlockStartLine=NumLine;
      }

      ShowEditor(OldCur==CurLine && LeftPos==CurLine->GetLeftPos());
      return(TRUE);
    }

    case KEY_SHIFTRIGHT:  case KEY_SHIFTNUMPAD6:
    {
      _SVS(CleverSysLog SL(L"case KEY_SHIFTRIGHT"));
      if(CurLine->m_next==NULL && CurPos==CurLine->GetLength() && !EdOpt.CursorBeyondEOL)
      {
        return TRUE;
      }

      if(SelAtBeginning)
      {
        CurLine->Select(SelStart+1,SelEnd);
      }
      else
      {
        CurLine->Select(SelStart,SelEnd+1);
      }
      Edit *OldCur=CurLine;
      int OldLeft=CurLine->GetLeftPos();
      Pasting++;
      ProcessKey(KEY_RIGHT);
      Pasting--;
      if(OldCur!=CurLine)
      {
        if(SelAtBeginning)
        {
          OldCur->Select(-1,0);
          BlockStart=CurLine;
          BlockStartLine=NumLine;
        }
        else
        {
          OldCur->Select(SelStart,-1);
        }
      }
      ShowEditor(OldCur==CurLine && OldLeft==CurLine->GetLeftPos());
      return(TRUE);
    }

    case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
    {
      _SVS(CleverSysLog SL(L"case KEY_CTRLSHIFTLEFT"));
      _SVS(SysLog(L"[%d] Pasting=%d, SelEnd=%d",__LINE__,Pasting,SelEnd));
      {
        int SkipSpace=TRUE;
        Pasting++;
        Lock ();

        int CurPos;
        while (1)
        {
          const wchar_t *Str;
          int Length;
          CurLine->GetBinaryString(&Str,NULL,Length);
          /* $ 12.11.2002 DJ
             ��������� ���������� ������ Ctrl-Shift-Left �� ������ ������
          */
          CurPos=CurLine->GetCurPos();
          if (CurPos>Length)
          {
            int SelStartPos = CurPos;
            CurLine->ProcessKey(KEY_END);
            CurPos=CurLine->GetCurPos();
            if (CurLine->SelStart >= 0)
            {
              if (!SelAtBeginning)
                CurLine->Select(CurLine->SelStart, CurPos);
              else
                CurLine->Select(CurPos, CurLine->SelEnd);
            }
            else
              CurLine->Select(CurPos, SelStartPos);
          }

          if (CurPos==0)
            break;
          /* $ 12.01.2004 IS
             ��� ��������� � WordDiv ���������� IsWordDiv, � �� strchr, �.�.
             ������� ��������� ����� ���������� �� ��������� WordDiv (������� OEM)
          */
          if (IsSpace(Str[CurPos-1]) ||
              IsWordDiv(NULL,EdOpt.strWordDiv,Str[CurPos-1])) //BUGBUG
              //IsWordDiv((AnsiText || UseDecodeTable)?&TableSet:NULL,EdOpt.strWordDiv,Str[CurPos-1])) //BUGBUG
            if (SkipSpace)
            {
              ProcessKey(KEY_SHIFTLEFT);
              continue;
            }
            else
              break;
          SkipSpace=FALSE;
          ProcessKey(KEY_SHIFTLEFT);
        }
        Pasting--;
        Unlock ();

        Show();
      }
      return(TRUE);
    }

    case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
    {
      _SVS(CleverSysLog SL(L"case KEY_CTRLSHIFTRIGHT"));
      _SVS(SysLog(L"[%d] Pasting=%d, SelEnd=%d",__LINE__,Pasting,SelEnd));
      {
        int SkipSpace=TRUE;
        Pasting++;
        Lock ();


        int CurPos;
        while (1)
        {
          const wchar_t *Str;
          int Length;
          CurLine->GetBinaryString(&Str,NULL,Length);
          CurPos=CurLine->GetCurPos();
          if (CurPos>=Length)
            break;
          if (IsSpace(Str[CurPos]) ||
              IsWordDiv(NULL,EdOpt.strWordDiv,Str[CurPos])) //BUGBUG
            //  IsWordDiv((AnsiText || UseDecodeTable)?&TableSet:NULL,EdOpt.strWordDiv,Str[CurPos])) //BUGBUG
            if (SkipSpace)
            {
              ProcessKey(KEY_SHIFTRIGHT);
              continue;
            }
            else
              break;
          SkipSpace=FALSE;
          ProcessKey(KEY_SHIFTRIGHT);
        }
        Pasting--;
        Unlock ();

        Show();
      }
      return(TRUE);
    }

    case KEY_SHIFTDOWN:  case KEY_SHIFTNUMPAD2:
    {
      if (CurLine->m_next==NULL)return TRUE;
      CurPos=CurLine->RealPosToTab(CurPos);
      if(SelAtBeginning)//������� ���������
      {
        if(SelEnd==-1)
        {
          CurLine->Select(-1,0);
          BlockStart=CurLine->m_next;
          BlockStartLine=NumLine+1;
        }
        else
        {
          CurLine->Select(SelEnd,-1);
        }
        CurLine->m_next->GetRealSelection(SelStart,SelEnd);
        if(SelStart!=-1)SelStart=CurLine->m_next->RealPosToTab(SelStart);
        if(SelEnd!=-1)SelEnd=CurLine->m_next->RealPosToTab(SelEnd);
        if(SelStart==-1)
        {
          SelStart=0;
          SelEnd=CurPos;
        }
        else
        {
          if(SelEnd!=-1 && SelEnd<CurPos)
          {
            SelStart=SelEnd;
            SelEnd=CurPos;
          }
          else
          {
            SelStart=CurPos;
          }
        }
        if(SelStart!=-1)SelStart=CurLine->m_next->TabPosToReal(SelStart);
        if(SelEnd!=-1)SelEnd=CurLine->m_next->TabPosToReal(SelEnd);
        /*if(!EdOpt.CursorBeyondEOL && SelEnd>CurLine->m_next->GetLength())
        {
          SelEnd=CurLine->m_next->GetLength();
        }
        if(!EdOpt.CursorBeyondEOL && SelStart>CurLine->m_next->GetLength())
        {
          SelStart=CurLine->m_next->GetLength();
        }*/
      }
      else //��������� ���������
      {
        CurLine->Select(SelStart,-1);
        SelStart=0;
        SelEnd=CurPos;
        if(SelStart!=-1)SelStart=CurLine->m_next->TabPosToReal(SelStart);
        if(SelEnd!=-1)SelEnd=CurLine->m_next->TabPosToReal(SelEnd);
      }

      if(!EdOpt.CursorBeyondEOL && SelEnd > CurLine->m_next->GetLength())
      {
        SelEnd=CurLine->m_next->GetLength();
      }

      if(!EdOpt.CursorBeyondEOL && SelStart > CurLine->m_next->GetLength())
      {
        SelStart=CurLine->m_next->GetLength();
      }

//      if(!SelStart && !SelEnd)
//        CurLine->m_next->Select(-1,0);
//      else
        CurLine->m_next->Select(SelStart,SelEnd);

      Down();
      Show();
      return(TRUE);
    }

    case KEY_SHIFTUP: case KEY_SHIFTNUMPAD8:
    {
      if (CurLine->m_prev==NULL) return 0;
      if(SelAtBeginning || SelFirst) // ��������� ���������
      {
        CurLine->Select(0,SelEnd);
        SelStart=CurLine->RealPosToTab(CurPos);
        if(!EdOpt.CursorBeyondEOL &&
            CurLine->m_prev->TabPosToReal(SelStart)>CurLine->m_prev->GetLength())
        {
          SelStart=CurLine->m_prev->RealPosToTab(CurLine->m_prev->GetLength());
        }
        SelStart=CurLine->m_prev->TabPosToReal(SelStart);
        CurLine->m_prev->Select(SelStart,-1);
        BlockStart=CurLine->m_prev;
        BlockStartLine=NumLine-1;
      }
      else // ������� ���������
      {
        CurPos=CurLine->RealPosToTab(CurPos);
        if(SelStart==0)
        {
          CurLine->Select(-1,0);
        }
        else
        {
          CurLine->Select(0,SelStart);
        }
        CurLine->m_prev->GetRealSelection(SelStart,SelEnd);
        if(SelStart!=-1)SelStart=CurLine->m_prev->RealPosToTab(SelStart);
        if(SelStart!=-1)SelEnd=CurLine->m_prev->RealPosToTab(SelEnd);
        if(SelStart==-1)
        {
          BlockStart=CurLine->m_prev;
          BlockStartLine=NumLine-1;
          SelStart=CurLine->m_prev->TabPosToReal(CurPos);
          SelEnd=-1;
        }
        else
        {
          if(CurPos<SelStart)
          {
            SelEnd=SelStart;
            SelStart=CurPos;
          }
          else
          {
            SelEnd=CurPos;
          }

          SelStart=CurLine->m_prev->TabPosToReal(SelStart);
          SelEnd=CurLine->m_prev->TabPosToReal(SelEnd);

          if(!EdOpt.CursorBeyondEOL && SelEnd>CurLine->m_prev->GetLength())
          {
            SelEnd=CurLine->m_prev->GetLength();
          }

          if(!EdOpt.CursorBeyondEOL && SelStart>CurLine->m_prev->GetLength())
          {
            SelStart=CurLine->m_prev->GetLength();
          }
        }
        CurLine->m_prev->Select(SelStart,SelEnd);
      }
      Up();
      Show();
      return(TRUE);
    }

    case KEY_CTRLADD:
    {
      Copy(TRUE);
      return(TRUE);
    }

    case KEY_CTRLA:
    {
      UnmarkBlock();
      SelectAll();
      return(TRUE);
    }

    case KEY_CTRLU:
    {
      UnmarkBlock();
      return(TRUE);
    }

    case KEY_CTRLC:
    case KEY_CTRLINS:    case KEY_CTRLNUMPAD0:
    {
      if (/*!EdOpt.PersistentBlocks && */BlockStart==NULL && VBlockStart==NULL)
      {
        BlockStart=CurLine;
        BlockStartLine=NumLine;
        CurLine->AddSelect(0,-1);
        Show();
      }
      Copy(FALSE);
      return(TRUE);
    }

    case KEY_CTRLP:
    case KEY_CTRLM:
    {
      if (Flags.Check(FEDITOR_LOCKMODE))
        return TRUE;
      if (BlockStart!=NULL || VBlockStart!=NULL)
      {
        int SelStart,SelEnd;
        CurLine->GetSelection(SelStart,SelEnd);

        Pasting++;
        int OldUsedInternalClipboard=UsedInternalClipboard;
        UsedInternalClipboard=1;
        ProcessKey(Key==KEY_CTRLP ? KEY_CTRLINS:KEY_SHIFTDEL);

        /* $ 10.04.2001 SVS
          ^P/^M - ����������� ��������: ������ ��� CurPos ������ ���� ">=",
           � �� "������".
        */
        if (Key==KEY_CTRLM && SelStart!=-1 && SelEnd!=-1)
          if (CurPos>=SelEnd)
            CurLine->SetCurPos(CurPos-(SelEnd-SelStart));
          else
            CurLine->SetCurPos(CurPos);

        ProcessKey(KEY_SHIFTINS);
        Pasting--;
        FAR_EmptyClipboard();
        UsedInternalClipboard=OldUsedInternalClipboard;

        /*$ 08.02.2001 SKV
          �� �������� � pasting'��, ������� redraw �������� �� ����.
          ������� ���.
        */
        Show();
      }
      return(TRUE);
    }

    case KEY_CTRLX:
    case KEY_SHIFTDEL:
    case KEY_SHIFTNUMDEL:
    case KEY_SHIFTDECIMAL:
    {
      Copy(FALSE);
    }
    case KEY_CTRLD:
    {
      if (Flags.Check(FEDITOR_LOCKMODE))
        return TRUE;
      Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
      DeleteBlock();
      Show();
      return(TRUE);
    }

    case KEY_CTRLV:
    case KEY_SHIFTINS: case KEY_SHIFTNUMPAD0:
    {
      if (Flags.Check(FEDITOR_LOCKMODE))
        return TRUE;

      Pasting++;
      if (!EdOpt.PersistentBlocks && VBlockStart==NULL)
        DeleteBlock();

      Paste();
      // MarkingBlock=(VBlockStart==NULL);
      Flags.Change(FEDITOR_MARKINGBLOCK,(VBlockStart==NULL));
      Flags.Clear(FEDITOR_MARKINGVBLOCK);
      if (!EdOpt.PersistentBlocks)
        UnmarkBlock();
      Pasting--;
      Show();
      return(TRUE);
    }

    case KEY_LEFT: case KEY_NUMPAD4:
    {
      Flags.Set(FEDITOR_NEWUNDO);
      if (CurPos==0 && CurLine->m_prev!=NULL)
      {
        Up();
        Show();
        CurLine->ProcessKey(KEY_END);
        Show();
      }
      else
      {
        int LeftPos=CurLine->GetLeftPos();
        CurLine->ProcessKey(KEY_LEFT);
        ShowEditor(LeftPos==CurLine->GetLeftPos());
      }
      return(TRUE);
    }

    case KEY_INS: case KEY_NUMPAD0:
    {
      Flags.Swap(FEDITOR_OVERTYPE);
      Show();
      return(TRUE);
    }

    case KEY_NUMDEL:
    case KEY_DEL:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        // Del � ����� ��������� ������� ������ �� �������, ������� �� ������������...
        if(!CurLine->m_next && CurPos>=CurLine->GetLength() && BlockStart==NULL && VBlockStart==NULL)
          return TRUE;
        /* $ 07.03.2002 IS
           ������ ���������, ���� ���� ��� ����� ������
        */
        if(!Pasting)
          UnmarkEmptyBlock();

        if (!Pasting && EdOpt.DelRemovesBlocks && (BlockStart!=NULL || VBlockStart!=NULL))
          DeleteBlock();
        else
        {
          AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,
                      CurLine->GetCurPos(),UNDO_EDIT);
          if (CurPos>=CurLine->GetLength())
          {
            if (CurLine->m_next==NULL)
              CurLine->SetEOL(L"");
            else
            {
              int SelStart,SelEnd,NextSelStart,NextSelEnd;
              int Length=CurLine->GetLength();
              CurLine->GetSelection(SelStart,SelEnd);
              CurLine->m_next->GetSelection(NextSelStart,NextSelEnd);

              const wchar_t *Str;
              int NextLength;
              CurLine->m_next->GetBinaryString(&Str,NULL,NextLength);
              CurLine->InsertBinaryString(Str,NextLength);
              CurLine->SetEOL(CurLine->m_next->GetEOL());
              CurLine->SetCurPos(CurPos);

              BlockUndo++;
              DeleteString(CurLine->m_next,TRUE,NumLine+1);
              BlockUndo--;
              if (NextLength==0)
                CurLine->SetEOL(L"");

              if (NextSelStart!=-1)
                if (SelStart==-1)
                {
                  CurLine->Select(Length+NextSelStart,NextSelEnd==-1 ? -1:Length+NextSelEnd);
                  BlockStart=CurLine;
                  BlockStartLine=NumLine;
                }
                else
                  CurLine->Select(SelStart,NextSelEnd==-1 ? -1:Length+NextSelEnd);

            }
          }
          else
            CurLine->ProcessKey(KEY_DEL);
          TextChanged(1);
        }
        Show();
      }
      return(TRUE);
    }

    case KEY_BS:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        // Bs � ����� ������ ������� ������ �� �������, ������ �� ����� ����������
        if(!CurLine->m_prev && !CurPos && BlockStart==NULL && VBlockStart==NULL)
          return TRUE;

        TextChanged(1);

        int IsDelBlock=FALSE;
        if(EdOpt.BSLikeDel)
        {
          if (!Pasting && EdOpt.DelRemovesBlocks && (BlockStart!=NULL || VBlockStart!=NULL))
            IsDelBlock=TRUE;
        }
        else
        {
          if (!Pasting && !EdOpt.PersistentBlocks && BlockStart!=NULL)
            IsDelBlock=TRUE;
        }
        if (IsDelBlock)
          DeleteBlock();
        else
          if (CurPos==0 && CurLine->m_prev!=NULL)
          {
            Pasting++;
            Up();
            CurLine->ProcessKey(KEY_CTRLEND);
            ProcessKey(KEY_DEL);
            Pasting--;
          }
          else
          {
            AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,
                        CurLine->GetCurPos(),UNDO_EDIT);
            CurLine->ProcessKey(KEY_BS);
          }

        Show();
      }
      return(TRUE);
    }

    case KEY_CTRLBS:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        TextChanged(1);
        if (!Pasting && !EdOpt.PersistentBlocks && BlockStart!=NULL)
          DeleteBlock();
        else
          if (CurPos==0 && CurLine->m_prev!=NULL)
            ProcessKey(KEY_BS);
          else
          {
            AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,
                        CurLine->GetCurPos(),UNDO_EDIT);
            CurLine->ProcessKey(KEY_CTRLBS);
          }
        Show();
      }
      return(TRUE);
    }

    case KEY_UP: case KEY_NUMPAD8:
    {
      {
        Flags.Set(FEDITOR_NEWUNDO);
        int PrevMaxPos=MaxRightPos;
        Edit *LastTopScreen=TopScreen;
        Up();
        if (TopScreen==LastTopScreen)
          ShowEditor(TRUE);
        else
          Show();
        if (PrevMaxPos>CurLine->GetTabCurPos())
        {
          CurLine->SetTabCurPos(PrevMaxPos);
          CurLine->FastShow();
          CurLine->SetTabCurPos(PrevMaxPos);
          Show();
        }
      }
      return(TRUE);
    }

    case KEY_DOWN: case KEY_NUMPAD2:
    {
      {
        Flags.Set(FEDITOR_NEWUNDO);
        int PrevMaxPos=MaxRightPos;
        Edit *LastTopScreen=TopScreen;
        Down();
        if (TopScreen==LastTopScreen)
          ShowEditor(TRUE);
        else
          Show();
        if (PrevMaxPos>CurLine->GetTabCurPos())
        {
          CurLine->SetTabCurPos(PrevMaxPos);
          CurLine->FastShow();
          CurLine->SetTabCurPos(PrevMaxPos);
          Show();
        }
      }
      return(TRUE);
    }

    case KEY_MSWHEEL_UP:
    case (KEY_MSWHEEL_UP | KEY_ALT):
    {
      int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaEdit;
      for (int i=0; i<Roll; i++)
        ProcessKey(KEY_CTRLUP);
      return(TRUE);
    }

    case KEY_MSWHEEL_DOWN:
    case (KEY_MSWHEEL_DOWN | KEY_ALT):
    {
      int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaEdit;
      for (int i=0; i<Roll; i++)
        ProcessKey(KEY_CTRLDOWN);
      return(TRUE);
    }

    case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
    {
      Flags.Set(FEDITOR_NEWUNDO);
      ScrollUp();
      Show();
      return(TRUE);
    }

    case KEY_CTRLDOWN: case KEY_CTRLNUMPAD2:
    {
      Flags.Set(FEDITOR_NEWUNDO);
      ScrollDown();
      Show();
      return(TRUE);
    }

    case KEY_PGUP:     case KEY_NUMPAD9:
    {
      Flags.Set(FEDITOR_NEWUNDO);
      for (I=Y1;I<Y2;I++)
        ScrollUp();
      Show();
      return(TRUE);
    }

    case KEY_PGDN:    case KEY_NUMPAD3:
    {
      Flags.Set(FEDITOR_NEWUNDO);
      for (I=Y1;I<Y2;I++)
        ScrollDown();
      Show();
      return(TRUE);
    }

    case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
    case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
    {
      {
        Flags.Set(FEDITOR_NEWUNDO);
        int StartPos=CurLine->GetTabCurPos();
        NumLine=0;
        TopScreen=CurLine=TopList;
        if (Key==KEY_CTRLHOME)
          CurLine->SetCurPos(0);
        else
          CurLine->SetTabCurPos(StartPos);
        Show();
      }
      return(TRUE);
    }

    case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
    case KEY_CTRLPGDN:  case KEY_CTRLNUMPAD3:
    {
      {
        Flags.Set(FEDITOR_NEWUNDO);
        int StartPos=CurLine->GetTabCurPos();
        NumLine=NumLastLine-1;
        CurLine=EndList;
        for (TopScreen=CurLine,I=Y1;I<Y2 && TopScreen->m_prev!=NULL;I++)
        {
          TopScreen->SetPosition(X1,I,X2,I);
          TopScreen=TopScreen->m_prev;
        }
        CurLine->SetLeftPos(0);
        if (Key==KEY_CTRLEND)
        {
          CurLine->SetCurPos(CurLine->GetLength());
          CurLine->FastShow();
        }
        else
          CurLine->SetTabCurPos(StartPos);
        Show();
      }
      return(TRUE);
    }

    case KEY_NUMENTER:
    case KEY_ENTER:
    {
      if (Pasting || !ShiftPressed || CtrlObject->Macro.IsExecuting())
      {
        if (!Pasting && !EdOpt.PersistentBlocks && BlockStart!=NULL)
          DeleteBlock();
        Flags.Set(FEDITOR_NEWUNDO);
        InsertString();
        CurLine->FastShow();
        Show();
      }
      return(TRUE);
    }

    case KEY_CTRLN:
    {
      Flags.Set(FEDITOR_NEWUNDO);
      while (CurLine!=TopScreen)
      {
        CurLine=CurLine->m_prev;
        NumLine--;
      }
      CurLine->SetCurPos(CurPos);
      Show();
      return(TRUE);
    }

    case KEY_CTRLE:
    {
      {
        Flags.Set(FEDITOR_NEWUNDO);
        Edit *CurPtr=TopScreen;
        int CurLineFound=FALSE;
        for (I=Y1;I<Y2;I++)
        {
          if (CurPtr->m_next==NULL)
            break;
          if (CurPtr==CurLine)
            CurLineFound=TRUE;
          if (CurLineFound)
            NumLine++;
          CurPtr=CurPtr->m_next;
        }
        CurLine=CurPtr;
        CurLine->SetCurPos(CurPos);
        Show();
      }
      return(TRUE);
    }

    case KEY_CTRLL:
    {
      Flags.Swap(FEDITOR_LOCKMODE);
      if(HostFileEditor) HostFileEditor->ShowStatus();
      return(TRUE);
    }

    case KEY_CTRLY:
    {
      DeleteString(CurLine,FALSE,NumLine);
      Show();
      return(TRUE);
    }

    case KEY_F7:
    {
      int ReplaceMode0=ReplaceMode;
      int ReplaceAll0=ReplaceAll;
      ReplaceMode=ReplaceAll=FALSE;
      if(!Search(FALSE))
      {
        ReplaceMode=ReplaceMode0;
        ReplaceAll=ReplaceAll0;
      }
      return(TRUE);
    }

    case KEY_CTRLF7:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        int ReplaceMode0=ReplaceMode;
        int ReplaceAll0=ReplaceAll;
        ReplaceMode=TRUE;
        ReplaceAll=FALSE;
        if(!Search(FALSE))
        {
          ReplaceMode=ReplaceMode0;
          ReplaceAll=ReplaceAll0;
        }
      }
      return(TRUE);
    }

    case KEY_SHIFTF7:
    {
      /* $ 20.09.2000 SVS
         ��� All ����� ������� Shift-F7 ������� ����� ��������...
      */
      //ReplaceAll=FALSE;

      /* $ 07.05.2001 IS
         ������� � ����� "Shift-F7 ���������� _�����_"
      */
      //ReplaceMode=FALSE;
      Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
      Search(TRUE);
      return(TRUE);
    }

    /*case KEY_F8:
    {
      Flags.Set(FEDITOR_TABLECHANGEDBYUSER);
      if ((AnsiText=!AnsiText)!=0)
      {
        int UseUnicode=FALSE;
        GetTable(&TableSet,TRUE,TableNum,UseUnicode);
      }
      TableNum=0;
      UseDecodeTable=AnsiText;
      SetStringsTable();
      if (HostFileEditor) HostFileEditor->ChangeEditKeyBar();
      Show();
      return(TRUE);
    } */ //BUGBUGBUG

    /*case KEY_SHIFTF8:
    {
      {
        int UseUnicode=FALSE;
        int GetTableCode=GetTable(&TableSet,FALSE,TableNum,UseUnicode);
        if (GetTableCode!=-1)
        {
          Flags.Set(FEDITOR_TABLECHANGEDBYUSER);
          UseDecodeTable=GetTableCode;
          AnsiText=FALSE;
          SetStringsTable();
          if (HostFileEditor) HostFileEditor->ChangeEditKeyBar();
          Show();
        }
      }
      return(TRUE); //BUGBUGBUG
    } */

    case KEY_F11:
    {
/*
      if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
      {
        CtrlObject->Plugins.CurEditor=HostFileEditor; // this;
        if (CtrlObject->Plugins.CommandsMenu(MODALTYPE_EDITOR,0,"Editor"))
          *PluginTitle=0;
        Show();
      }
*/
      return(TRUE);
    }

    case KEY_ALTBS:
    case KEY_CTRLZ:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        Lock ();
        Undo();
        Unlock ();
        Show();
      }
      return(TRUE);
    }

    case KEY_ALTF8:
    {
      {
        GoToPosition();
        // <GOTO_UNMARK:1>
        if (!EdOpt.PersistentBlocks)
          UnmarkBlock();
        // </GOTO_UNMARK>
        Show();
      }
      return(TRUE);
    }

    case KEY_ALTU:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        BlockLeft();
        Show();
      }
      return(TRUE);
    }

    case KEY_ALTI:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        BlockRight();
        Show();
      }
      return(TRUE);
    }

    case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
    case KEY_ALTLEFT:
    {
      if (CurPos==0)
        return(TRUE);
      if (!Flags.Check(FEDITOR_MARKINGVBLOCK))
        BeginVBlockMarking();
      Pasting++;
      {
        int Delta=CurLine->GetTabCurPos()-CurLine->RealPosToTab(CurPos-1);
        if (CurLine->GetTabCurPos()>VBlockX)
          VBlockSizeX-=Delta;
        else
        {
          VBlockX-=Delta;
          VBlockSizeX+=Delta;
        }
        /* $ 25.07.2000 tran
           ������� ���� 22 - ��������� ��� �������� �� ������� ����� */
        if ( VBlockSizeX<0 )
        {
            VBlockSizeX=-VBlockSizeX;
            VBlockX-=VBlockSizeX;
        }
        ProcessKey(KEY_LEFT);
      }
      Pasting--;
      Show();
      //_D(SysLog(L"VBlockX=%i, VBlockSizeX=%i, GetLineCurPos=%i",VBlockX,VBlockSizeX,GetLineCurPos()));
      //_D(SysLog(L"~~~~~~~~~~~~~~~~ KEY_ALTLEFT END, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
      return(TRUE);
    }

    case KEY_ALTSHIFTRIGHT:  case KEY_ALTSHIFTNUMPAD6:
    case KEY_ALTRIGHT:
    {
      /* $ 23.10.2000 tran
         ������ GetTabCurPos ���� �������� GetCurPos -
         ���������� �������� ������� � �������� ������
         � ���� ��������� ������� �������� � �������� ������*/
      if (!EdOpt.CursorBeyondEOL && CurLine->GetCurPos()>=CurLine->GetLength())
        return(TRUE);

      if (!Flags.Check(FEDITOR_MARKINGVBLOCK))
        BeginVBlockMarking();

      //_D(SysLog(L"---------------- KEY_ALTRIGHT, getLineCurPos=%i",GetLineCurPos()));
      Pasting++;
      {
        int Delta;
        /* $ 18.07.2000 tran
             ������ � ������ ������, ����� alt-right, alt-pagedown,
             ��������� ���� ������� � 1 �������, ����� ��� alt-right
             ��������� ���������
        */
        int VisPos=CurLine->RealPosToTab(CurPos),
            NextVisPos=CurLine->RealPosToTab(CurPos+1);
        //_D(SysLog(L"CurPos=%i, VisPos=%i, NextVisPos=%i",
        //    CurPos,VisPos, NextVisPos); //,CurLine->GetTabCurPos()));

        Delta=NextVisPos-VisPos;
         //_D(SysLog(L"Delta=%i",Delta));

        if (CurLine->GetTabCurPos()>=VBlockX+VBlockSizeX)
          VBlockSizeX+=Delta;
        else
        {
          VBlockX+=Delta;
          VBlockSizeX-=Delta;
        }
        /* $ 25.07.2000 tran
           ������� ���� 22 - ��������� ��� �������� �� ������� ����� */
        if ( VBlockSizeX<0 )
        {
            VBlockSizeX=-VBlockSizeX;
            VBlockX-=VBlockSizeX;
        }
        ProcessKey(KEY_RIGHT);
        //_D(SysLog(L"VBlockX=%i, VBlockSizeX=%i, GetLineCurPos=%i",VBlockX,VBlockSizeX,GetLineCurPos()));
      }
      Pasting--;
      Show();
      //_D(SysLog(L"~~~~~~~~~~~~~~~~ KEY_ALTRIGHT END, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));

      return(TRUE);
    }

    /* $ 29.06.2000 IG
      + CtrlAltLeft, CtrlAltRight ��� ������������ ������
    */
    case KEY_CTRLALTLEFT: case KEY_CTRLALTNUMPAD4:
    {
      {
        int SkipSpace=TRUE;
        Pasting++;
        Lock ();

        while (1)
        {
          const wchar_t *Str;
          int Length;
          CurLine->GetBinaryString(&Str,NULL,Length);
          int CurPos=CurLine->GetCurPos();
          if (CurPos>Length)
          {
            CurLine->ProcessKey(KEY_END);
            CurPos=CurLine->GetCurPos();
          }
          if (CurPos==0)
            break;
          if (IsSpace(Str[CurPos-1]) ||
              IsWordDiv(NULL,EdOpt.strWordDiv,Str[CurPos-1])) //BUGBUG
          //    IsWordDiv((AnsiText || UseDecodeTable)?&TableSet:NULL,EdOpt.strWordDiv,Str[CurPos-1])) //BUGBUG
            if (SkipSpace)
            {
              ProcessKey(KEY_ALTSHIFTLEFT);
              continue;
            }
            else
              break;
          SkipSpace=FALSE;
          ProcessKey(KEY_ALTSHIFTLEFT);
        }
        Pasting--;

        Unlock ();
        Show();
      }
      return(TRUE);
    }

    case KEY_CTRLALTRIGHT: case KEY_CTRLALTNUMPAD6:
    {
      {
        int SkipSpace=TRUE;
        Pasting++;
        Lock ();

        while (1)
        {
          const wchar_t *Str;
          int Length;
          CurLine->GetBinaryString(&Str,NULL,Length);
          int CurPos=CurLine->GetCurPos();
          if (CurPos>=Length)
            break;
          if (IsSpace(Str[CurPos]) ||
              IsWordDiv(NULL,EdOpt.strWordDiv,Str[CurPos])) //BUGBUG
          //    IsWordDiv((AnsiText || UseDecodeTable)?&TableSet:NULL,EdOpt.strWordDiv,Str[CurPos])) //BUGBUG
            if (SkipSpace)
            {
              ProcessKey(KEY_ALTSHIFTRIGHT);
              continue;
            }
            else
              break;
          SkipSpace=FALSE;
          ProcessKey(KEY_ALTSHIFTRIGHT);
        }
        Pasting--;
        Unlock ();

        Show();
      }
      return(TRUE);
    }

    case KEY_ALTSHIFTUP:    case KEY_ALTSHIFTNUMPAD8:
    case KEY_ALTUP:
    {
      if (CurLine->m_prev==NULL)
        return(TRUE);

      if (!Flags.Check(FEDITOR_MARKINGVBLOCK))
        BeginVBlockMarking();

      if (!EdOpt.CursorBeyondEOL && VBlockX>=CurLine->m_prev->GetLength())
        return(TRUE);
      Pasting++;
      if (NumLine>VBlockY)
        VBlockSizeY--;
      else
      {
        VBlockY--;
        VBlockSizeY++;
        VBlockStart=VBlockStart->m_prev;
        BlockStartLine--;
      }
      ProcessKey(KEY_UP);
      AdjustVBlock(CurVisPos);
      Pasting--;
      Show();
      //_D(SysLog(L"~~~~~~~~ ALT_PGUP, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
      return(TRUE);
    }

    case KEY_ALTSHIFTDOWN:  case KEY_ALTSHIFTNUMPAD2:
    case KEY_ALTDOWN:
    {
      if (CurLine->m_next==NULL)
        return(TRUE);
      if (!Flags.Check(FEDITOR_MARKINGVBLOCK))
        BeginVBlockMarking();
      if (!EdOpt.CursorBeyondEOL && VBlockX>=CurLine->m_next->GetLength())
        return(TRUE);
      Pasting++;
      if (NumLine>=VBlockY+VBlockSizeY-1)
        VBlockSizeY++;
      else
      {
        VBlockY++;
        VBlockSizeY--;
        VBlockStart=VBlockStart->m_next;
        BlockStartLine++;
      }
      ProcessKey(KEY_DOWN);
      AdjustVBlock(CurVisPos);
      Pasting--;
      Show();
      //_D(SysLog(L"~~~~ Key_AltDOWN: VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
      return(TRUE);
    }

    case KEY_ALTSHIFTHOME: case KEY_ALTSHIFTNUMPAD7:
    case KEY_ALTHOME:
    {
      Pasting++;
      Lock ();
      while (CurLine->GetCurPos()>0)
        ProcessKey(KEY_ALTSHIFTLEFT);
      Unlock ();
      Pasting--;
      Show();
      return(TRUE);
    }

    case KEY_ALTSHIFTEND: case KEY_ALTSHIFTNUMPAD1:
    case KEY_ALTEND:
    {
      Pasting++;
      Lock ();
      if (CurLine->GetCurPos()<CurLine->GetLength())
        while (CurLine->GetCurPos()<CurLine->GetLength())
          ProcessKey(KEY_ALTSHIFTRIGHT);
      if (CurLine->GetCurPos()>CurLine->GetLength())
        while (CurLine->GetCurPos()>CurLine->GetLength())
          ProcessKey(KEY_ALTSHIFTLEFT);
      Unlock ();
      Pasting--;
      Show();
      return(TRUE);
    }

    case KEY_ALTSHIFTPGUP: case KEY_ALTSHIFTNUMPAD9:
    case KEY_ALTPGUP:
    {
      Pasting++;
      Lock ();
      for (I=Y1;I<Y2;I++)
        ProcessKey(KEY_ALTSHIFTUP);
      Unlock ();
      Pasting--;
      Show();
      return(TRUE);
    }

    case KEY_ALTSHIFTPGDN: case KEY_ALTSHIFTNUMPAD3:
    case KEY_ALTPGDN:
    {
      Pasting++;
      Lock ();
      for (I=Y1;I<Y2;I++)
        ProcessKey(KEY_ALTSHIFTDOWN);
      Unlock ();
      Pasting--;
      Show();
      return(TRUE);
    }

    case KEY_CTRLALTPGUP: case KEY_CTRLALTNUMPAD9:
    case KEY_CTRLALTHOME: case KEY_CTRLALTNUMPAD7:
    {
      Lock ();
      Pasting++;
      while (CurLine!=TopList)
      {

        ProcessKey(KEY_ALTUP);
      }
      Pasting--;
      Unlock ();

      Show();
      return(TRUE);
    }

    case KEY_CTRLALTPGDN:  case KEY_CTRLALTNUMPAD3:
    case KEY_CTRLALTEND:   case KEY_CTRLALTNUMPAD1:
    {
      Lock ();
      Pasting++;
      while (CurLine!=EndList)
      {

        ProcessKey(KEY_ALTDOWN);
      }
      Pasting--;
      Unlock ();

      Show();
      return(TRUE);
    }

    case KEY_CTRLALTBRACKET:       // �������� ������� (UNC) ���� �� ����� ������
    case KEY_CTRLALTBACKBRACKET:   // �������� ������� (UNC) ���� �� ������ ������
    case KEY_ALTSHIFTBRACKET:      // �������� ������� (UNC) ���� �� �������� ������
    case KEY_ALTSHIFTBACKBRACKET:  // �������� ������� (UNC) ���� �� ��������� ������
    case KEY_CTRLBRACKET:          // �������� ���� �� ����� ������
    case KEY_CTRLBACKBRACKET:      // �������� ���� �� ������ ������
    case KEY_CTRLSHIFTBRACKET:     // �������� ���� �� �������� ������
    case KEY_CTRLSHIFTBACKBRACKET: // �������� ���� �� ��������� ������

    case KEY_CTRLSHIFTNUMENTER:
    case KEY_SHIFTNUMENTER:
    case KEY_CTRLSHIFTENTER:
    case KEY_SHIFTENTER:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        Pasting++;
        TextChanged(1);
        if (!EdOpt.PersistentBlocks && BlockStart!=NULL)
        {
          Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
          DeleteBlock();
        }
        AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,
                        CurLine->GetCurPos(),UNDO_EDIT);
        CurLine->ProcessKey(Key);
        Pasting--;
        Show();
      }
      return(TRUE);
    }

    case KEY_CTRLQ:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        Flags.Set(FEDITOR_PROCESSCTRLQ);
        if(HostFileEditor) HostFileEditor->ShowStatus();
        Pasting++;
        TextChanged(1);
        if (!EdOpt.PersistentBlocks && BlockStart!=NULL)
        {
          Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
          DeleteBlock();
        }
        AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,
                        CurLine->GetCurPos(),UNDO_EDIT);
        CurLine->ProcessCtrlQ();
        Flags.Clear(FEDITOR_PROCESSCTRLQ);
        Pasting--;
        Show();
      }
      return(TRUE);
    }

    case KEY_OP_SELWORD:
    {
      int OldCurPos=CurPos;
      int SStart, SEnd;
      Pasting++;
      Lock ();

      UnmarkBlock();
      // CurLine->TableSet ??? => UseDecodeTable?CurLine->TableSet:NULL !!!
      CalcWordFromString(CurLine->GetStringAddrW(),CurPos,&SStart,&SEnd,CurLine->TableSet,EdOpt.strWordDiv);
      CurLine->Select(SStart,SEnd+(SEnd < CurLine->StrSize?1:0));

      Flags.Set(FEDITOR_MARKINGBLOCK);
      BlockStart=CurLine;
      BlockStartLine=NumLine;
      //SelFirst=TRUE;
      SelStart=SStart;
      SelEnd=SEnd;

      //CurLine->ProcessKey(MCODE_OP_SELWORD);

      CurPos=OldCurPos; // ���������� �������
      Pasting--;
      Unlock ();

      Show();
      return TRUE;
    }

    case KEY_OP_DATE:
    case KEY_OP_PLAINTEXT:
    {
      if (!Flags.Check(FEDITOR_LOCKMODE))
      {
        const wchar_t *Fmt = eStackAsString();
        string strTStr;

        if(Key == KEY_OP_PLAINTEXT)
          strTStr = Fmt;
        if(Key == KEY_OP_PLAINTEXT || MkStrFTime(strTStr, Fmt))
        {
          wchar_t *Ptr=strTStr.GetBuffer();
          while(*Ptr) // ������� 0x0A �� 0x0D �� �������� Paset ;-)
          {
            if(*Ptr == 10)
              *Ptr=13;
            ++Ptr;
          }

          strTStr.ReleaseBuffer();

          Pasting++;
          //_SVS(SysLogDump(Fmt,0,TStr,strlen(TStr),NULL));
          TextChanged(1);
          BOOL IsBlock=VBlockStart || BlockStart;
          if (!EdOpt.PersistentBlocks && IsBlock)
          {
            Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
            DeleteBlock();
          }
          //AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,
          //              CurLine->GetCurPos(),UNDO_EDIT);

          Paste(strTStr);

          //if (!EdOpt.PersistentBlocks && IsBlock)
          UnmarkBlock();
          Pasting--;
          Show();
        }
      }
      return(TRUE);
    }

    default:
    {
      {
        if ((Key==KEY_CTRLDEL || Key==KEY_CTRLNUMDEL || Key==KEY_CTRLDECIMAL || Key==KEY_CTRLT) && CurPos>=CurLine->GetLength())
        {
         /*$ 08.12.2000 skv
           - CTRL-DEL � ������ ������ ��� ���������� ����� �
             ���������� EditorDelRemovesBlocks
         */
          int save=EdOpt.DelRemovesBlocks;
          EdOpt.DelRemovesBlocks=0;
          int ret=ProcessKey(KEY_DEL);
          EdOpt.DelRemovesBlocks=save;
          return ret;
        }

        if (!Pasting && !EdOpt.PersistentBlocks && BlockStart!=NULL)
          if (Key>=32 && Key<256 || Key==KEY_ADD || Key==KEY_SUBTRACT ||
              Key==KEY_MULTIPLY || Key==KEY_DIVIDE || Key==KEY_TAB)
          {
            DeleteBlock();
            /* $ 19.09.2002 SKV
              ������ ����.
              ����� ���� ��� ������� ��������� ��������
              ����� � ������ ����� �� ��������� � ���������
              ���������� ���� ����� �������.
            */
            Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
            Show();
          }

        int SkipCheckUndo=(Key==KEY_RIGHT     || Key==KEY_NUMPAD6     ||
                           Key==KEY_CTRLLEFT  || Key==KEY_CTRLNUMPAD4 ||
                           Key==KEY_CTRLRIGHT || Key==KEY_CTRLNUMPAD6 ||
                           Key==KEY_HOME      || Key==KEY_NUMPAD7     ||
                           Key==KEY_END       || Key==KEY_NUMPAD1     ||
                           Key==KEY_CTRLS);

        if (Flags.Check(FEDITOR_LOCKMODE) && !SkipCheckUndo)
          return(TRUE);

        if ((Key==KEY_CTRLLEFT || Key==KEY_CTRLNUMPAD4) && CurLine->GetCurPos()==0)
        {
          Pasting++;
          ProcessKey(KEY_LEFT);
          Pasting--;
          /* $ 24.9.2001 SKV
            fix ���� � ctrl-left � ������ ������
            � ����� � ��������������� �������� �����.
          */
          ShowEditor(FALSE);
          //if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT)){
          //CtrlObject->Plugins.CurEditor=HostFileEditor; // this;
          //_D(SysLog(L"%08d EE_REDRAW",__LINE__));
          //CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
          //}
          return(TRUE);
        }

        if ((!EdOpt.CursorBeyondEOL && Key==KEY_RIGHT || Key==KEY_NUMPAD6 || Key==KEY_CTRLRIGHT || Key==KEY_CTRLNUMPAD6) &&
            CurLine->GetCurPos()>=CurLine->GetLength() &&
            CurLine->m_next!=NULL)
        {
          Pasting++;
          ProcessKey(KEY_HOME);
          ProcessKey(KEY_DOWN);
          Pasting--;
          if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
          {
            CtrlObject->Plugins.CurEditor=HostFileEditor; // this;
            //_D(SysLog(L"%08d EE_REDRAW",__LINE__));
            _SYS_EE_REDRAW(SysLog(L"Editor::ProcessKey[%d](!EdOpt.CursorBeyondEOL): EE_REDRAW(EEREDRAW_ALL)",__LINE__));
            CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
          }
          /*$ 03.02.2001 SKV
            � �� EEREDRAW_ALL �� ������, � �� ����� ����
            ������ ������� ����� ����������������.
          */
          ShowEditor(0);
          return(TRUE);
        }

        const wchar_t *Str;
        wchar_t *CmpStr=0;
        int Length,CurPos;

        CurLine->GetBinaryString(&Str,NULL,Length);
        CurPos=CurLine->GetCurPos();

        if (Key<256 && CurPos>0 && Length==0)
        {
          Edit *PrevLine=CurLine->m_prev;
          while (PrevLine!=NULL && PrevLine->GetLength()==0)
            PrevLine=PrevLine->m_prev;
          if (PrevLine!=NULL)
          {
            int TabPos=CurLine->GetTabCurPos();
            CurLine->SetCurPos(0);
            const wchar_t *PrevStr=NULL;
            int PrevLength=0;
            PrevLine->GetBinaryString(&PrevStr,NULL,PrevLength);
            for (int I=0;I<PrevLength && IsSpace(PrevStr[I]);I++)
            {
              int NewTabPos=CurLine->GetTabCurPos();
              if (NewTabPos==TabPos)
                break;
              if (NewTabPos>TabPos)
              {
                CurLine->ProcessKey(KEY_BS);
                while (CurLine->GetTabCurPos()<TabPos)
                  CurLine->ProcessKey(' ');
                break;
              }
              if (NewTabPos<TabPos)
                CurLine->ProcessKey(PrevStr[I]);
            }
            CurLine->SetTabCurPos(TabPos);
          }
        }

        if (!SkipCheckUndo)
        {
          CurLine->GetBinaryString(&Str,NULL,Length);
          CurPos=CurLine->GetCurPos();
          CmpStr=new wchar_t[Length+1];
          wmemcpy(CmpStr,Str,Length);
          CmpStr[Length]=0;
        }

        int LeftPos=CurLine->GetLeftPos();

        if((Opt.XLat.XLatEditorKey && Key == Opt.XLat.XLatEditorKey ||
            Opt.XLat.XLatAltEditorKey && Key == Opt.XLat.XLatAltEditorKey) ||
            Key == KEY_OP_XLAT)
        {
          Xlat();
          Show();
          return TRUE;
        }

        // <comment> - ��� ��������� ��� ���������� ������ ������ ������ ��� Ctrl-K
        int PreSelStart,PreSelEnd;
        CurLine->GetSelection(PreSelStart,PreSelEnd);
        // </comment>

        //AY: ��� ��� �� ��� FastShow LeftPos �� ���������� � ����� ������.
        CurLine->ObjWidth=X2-X1+1;

        if (CurLine->ProcessKey(Key))
        {
          int SelStart,SelEnd;
          /* $ 17.09.2002 SKV
            ���� ��������� � �������� �����,
            � ������ ������, � �������� tab, ������� ����������
            �� �������, ��������� ������. ��� ����.
          */
          if(Key==KEY_TAB && CurLine->GetConvertTabs() &&
             BlockStart!=NULL && BlockStart!=CurLine)
          {
            CurLine->GetSelection(SelStart,SelEnd);
            CurLine->Select(SelStart==-1?-1:0,SelEnd);
          }
          if (!SkipCheckUndo)
          {
            const wchar_t *NewCmpStr;
            int NewLength;
            CurLine->GetBinaryString(&NewCmpStr,NULL,NewLength);
            if (NewLength!=Length || memcmp(CmpStr,NewCmpStr,Length)!=0)
            {
              AddUndoData(CmpStr,CurLine->GetEOL(),NumLine,CurPos,UNDO_EDIT); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
              TextChanged(1);
            }
            delete[] CmpStr;
          }
          // <Bug 794>
          // ���������� ������ ������ � ��������� ������ � ������
          if(Key == KEY_CTRLK && EdOpt.PersistentBlocks)
          {
             if(CurLine==BlockStart)
             {
               if(CurPos)
               {
                 CurLine->GetSelection(SelStart,SelEnd);
                 // 1. ���� �� ������ ������ (CurPos ��� ����� � ������, ��� SelStart)
                 if(SelEnd == -1 && PreSelStart > CurPos || SelEnd > CurPos)
                   SelStart=SelEnd=-1; // � ���� ������ ������� ���������
                 // 2. CurPos ������ �����
                 else if(SelEnd == -1 && PreSelEnd > CurPos && SelStart < CurPos)
                   SelEnd=PreSelEnd;   // � ���� ������ ������� ����
                 // 3. ���� ������� ����� �� CurPos ��� ��������� ����� ����� (��. ����)
                 if(SelEnd >= CurPos || SelStart==-1)
                   CurLine->Select(SelStart,CurPos);
               }
               else
               {
                 CurLine->Select(-1,-1);
                 BlockStart=BlockStart->m_next;
               }
             }
             else // ����� ������ !!! ���� ���������� ���� ���������� ������� (�� �������), �� ���� ��������... ����� ��������...
             {
               // ������ ��� ��������� ������ (� ��������� �� ���)
               Edit *CurPtrBlock=BlockStart,*CurPtrBlock2=BlockStart;
               while (CurPtrBlock!=NULL)
               {
                 CurPtrBlock->GetRealSelection(SelStart,SelEnd);
                 if (SelStart==-1)
                   break;
                 CurPtrBlock2=CurPtrBlock;
                 CurPtrBlock=CurPtrBlock->m_next;
               }

               if(CurLine==CurPtrBlock2)
               {
                 if(CurPos)
                 {
                   CurLine->GetSelection(SelStart,SelEnd);
                   CurLine->Select(SelStart,CurPos);
                 }
                 else
                 {
                   CurLine->Select(-1,-1);
                   CurPtrBlock2=CurPtrBlock2->m_next;
                 }
               }

             }
          }
          // </Bug 794>

          ShowEditor(LeftPos==CurLine->GetLeftPos());
          return(TRUE);
        }
        else
          if (!SkipCheckUndo)
            delete[] CmpStr;
        if (VBlockStart!=NULL)
          Show();
      }
      return(FALSE);
    }
  }
}


int Editor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  Edit *NewPtr;
  int NewDist,Dist;

  // $ 28.12.2000 VVM - ������ ������ ������� ������������ ���� ������
  if ((MouseEvent->dwButtonState & 3)!=0)
  {
    Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
    if ((!EdOpt.PersistentBlocks) && (BlockStart!=NULL || VBlockStart!=NULL))
    {
      UnmarkBlock();
      Show();
    }
  }

  if (CurLine->ProcessMouse(MouseEvent))
  {
    if(HostFileEditor) HostFileEditor->ShowStatus();
    if (VBlockStart!=NULL)
      Show();
    else
    {
      if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
      {
        CtrlObject->Plugins.CurEditor=HostFileEditor; // this;
        _SYS_EE_REDRAW(SysLog(L"Editor::ProcessMouse[%08d] ProcessEditorEvent(EE_REDRAW,EEREDRAW_LINE)",__LINE__));
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_LINE);
      }
    }
    return(TRUE);
  }

  if ((MouseEvent->dwButtonState & 3)==0)
    return(FALSE);

  // scroll up
  if (MouseEvent->dwMousePosition.Y==Y1-1)
  {
    while (IsMouseButtonPressed() && MouseY==Y1-1)
      ProcessKey(KEY_UP);
    return(TRUE);
  }

  // scroll down
  if (MouseEvent->dwMousePosition.Y==Y2+1)
  {
    while (IsMouseButtonPressed() && MouseY==Y2+1)
      ProcessKey(KEY_DOWN);
    return(TRUE);
  }

  if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
      MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2)
    return(FALSE);

  NewDist=MouseEvent->dwMousePosition.Y-Y1;
  NewPtr=TopScreen;
  while (NewDist-- && NewPtr->m_next)
    NewPtr=NewPtr->m_next;

  Dist=CalcDistance(TopScreen,NewPtr,-1)-CalcDistance(TopScreen,CurLine,-1);

  if (Dist>0)
    while (Dist--)
      Down();
  else
    while (Dist++)
      Up();

  CurLine->ProcessMouse(MouseEvent);
  Show();
  return(TRUE);
}


int Editor::CalcDistance(Edit *From, Edit *To,int MaxDist)
{
  int Distance=0;
  while (From!=To && From->m_next!=NULL && (MaxDist==-1 || MaxDist-- > 0))
  {
    Distance++;
    From=From->m_next;
  }
  return(Distance);
}



void Editor::DeleteString(Edit *DelPtr,int DeleteLast,int UndoLine)
{
  if (Flags.Check(FEDITOR_LOCKMODE))
    return;
  /* $ 16.12.2000 OT
     CtrlY �� ��������� ������ � ���������� ������������ ������ �� ������ ��������� */
  if (VBlockStart!=NULL && NumLine<VBlockY+VBlockSizeY)
    if (NumLine<VBlockY)
    {
      if (VBlockY>0)
      {
        VBlockY--;
        BlockStartLine--;
      }
    }
    else
      if (--VBlockSizeY<=0)
        VBlockStart=NULL;

  TextChanged(1);
  if (DelPtr->m_next==NULL && (!DeleteLast || DelPtr->m_prev==NULL))
  {
    AddUndoData(DelPtr->GetStringAddrW(),DelPtr->GetEOL(),UndoLine,
                DelPtr->GetCurPos(),UNDO_EDIT);
    DelPtr->SetString(L"");
    return;
  }

  for (int I=0;I<sizeof(SavePos.Line)/sizeof(SavePos.Line[0]);I++)
    if (SavePos.Line[I]!=0xffffffff && UndoLine<static_cast<int>(SavePos.Line[I]))
      SavePos.Line[I]--;

  NumLastLine--;

  if (CurLine==DelPtr)
  {
    int LeftPos,CurPos;
    CurPos=DelPtr->GetTabCurPos();
    LeftPos=DelPtr->GetLeftPos();
    if (DelPtr->m_next!=NULL)
      CurLine=DelPtr->m_next;
    else
    {
      CurLine=DelPtr->m_prev;
      /* $ 04.11.2002 SKV
        ����� ��� ���� ��� ���������, ����� ������� ������ ���� ��������.
      */
      NumLine--;
    }
    CurLine->SetLeftPos(LeftPos);
    CurLine->SetTabCurPos(CurPos);
  }

  if (DelPtr->m_prev)
  {
    DelPtr->m_prev->m_next=DelPtr->m_next;
    if (DelPtr==EndList)
      EndList=EndList->m_prev;
  }
  if (DelPtr->m_next!=NULL)
    DelPtr->m_next->m_prev=DelPtr->m_prev;
  if (DelPtr==TopScreen)
    if (TopScreen->m_next!=NULL)
      TopScreen=TopScreen->m_next;
    else
      TopScreen=TopScreen->m_prev;
  if (DelPtr==TopList)
    TopList=TopList->m_next;
  if (DelPtr==BlockStart)
  {
    BlockStart=BlockStart->m_next;
    // Mantis#0000316: �� �������� ����������� ������
    if(BlockStart && !BlockStart->IsSelection())
       BlockStart=NULL;
  }
  if (DelPtr==VBlockStart)
    VBlockStart=VBlockStart->m_next;
  if (UndoLine!=-1)
    AddUndoData(DelPtr->GetStringAddrW(),DelPtr->GetEOL(),UndoLine,0,UNDO_DELSTR);
  delete DelPtr;
}


void Editor::InsertString()
{
  if (Flags.Check(FEDITOR_LOCKMODE))
    return;
  /*$ 10.08.2000 skv
    There is only one return - if new will fail.
    In this case things are realy bad.
    Move TextChanged to the end of functions
    AFTER all modifications are made.
  */
//  TextChanged(1);

  Edit *NewString;
  Edit *SrcIndent=NULL;
  int SelStart,SelEnd;
  int CurPos;
  int NewLineEmpty=TRUE;

  NewString = InsertString (NULL, 0, CurLine);

  if ( !NewString )
  	return;

  //NewString->SetTables(UseDecodeTable ? &TableSet:NULL); // ??

  int Length;
  const wchar_t *CurLineStr;
  const wchar_t *EndSeq;
  CurLine->GetBinaryString(&CurLineStr,&EndSeq,Length);

  /* $ 13.01.2002 IS
     ���� �� ��� ��������� ��� ����� ������, �� ������� ��� ����� ������
     � ��� ����� DOS_EOL_fmt � ��������� ��� ����.
  */
  if (!*EndSeq)
      CurLine->SetEOL(*GlobalEOL?GlobalEOL:DOS_EOL_fmt);

  CurPos=CurLine->GetCurPos();
  CurLine->GetSelection(SelStart,SelEnd);

  for (int I=0;I<sizeof(SavePos.Line)/sizeof(SavePos.Line[0]);I++)
    if (SavePos.Line[I]!=0xffffffff &&
        (NumLine<static_cast<int>(SavePos.Line[I]) || NumLine==SavePos.Line[I] && CurPos==0))
      SavePos.Line[I]++;

  int IndentPos=0;

  if (EdOpt.AutoIndent && !Pasting)
  {
    Edit *PrevLine=CurLine;
    while (PrevLine!=NULL)
    {
      const wchar_t *Str;
      int Length,Found=FALSE;
      PrevLine->GetBinaryString(&Str,NULL,Length);
      for (int I=0;I<Length;I++)
        if (!IsSpace(Str[I]))
        {
          PrevLine->SetCurPos(I);
          IndentPos=PrevLine->GetTabCurPos();
          SrcIndent=PrevLine;
          Found=TRUE;
          break;
        }
      if (Found)
        break;
      PrevLine=PrevLine->m_prev;
    }
  }

  int SpaceOnly=TRUE;

  if (CurPos<Length)
  {


    if (IndentPos>0)
      for (int I=0;I<CurPos;I++)
        if (!IsSpace(CurLineStr[I]))
        {
          SpaceOnly=FALSE;
          break;
        }

    NewString->SetBinaryString(&CurLineStr[CurPos],Length-CurPos);

    for ( int i0=0; i0<Length-CurPos; i0++ )
    {
        if (!IsSpace(CurLineStr[i0+CurPos]))
        {
            NewLineEmpty=FALSE;
            break;
        }
    }

    AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,
                CurLine->GetCurPos(),UNDO_EDIT);
    BlockUndo++;
    AddUndoData(NULL,EndList==CurLine?L"":GlobalEOL,NumLine+1,0,UNDO_INSSTR); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
    BlockUndo--;

    wchar_t *NewCurLineStr = (wchar_t *) xf_malloc((CurPos+1)*sizeof(wchar_t));
    if (!NewCurLineStr)
      return;
    wmemcpy(NewCurLineStr,CurLineStr,CurPos);
    NewCurLineStr[CurPos]=0;
    int StrSize=CurPos;

    if (EdOpt.AutoIndent && NewLineEmpty)
    {
      RemoveTrailingSpaces(NewCurLineStr);
      StrSize=StrLength(NewCurLineStr);
    }

    CurLine->SetBinaryString(NewCurLineStr,StrSize);
    CurLine->SetEOL(EndSeq);

    xf_free(NewCurLineStr);
  }
  else
  {
    NewString->SetString(L"");
    AddUndoData(NULL,L"",NumLine+1,0,UNDO_INSSTR);// EOL? - CurLine->GetEOL()  GlobalEOL   ""
  }

  if (VBlockStart!=NULL && NumLine<VBlockY+VBlockSizeY)
    if (NumLine<VBlockY)
    {
      VBlockY++;
      BlockStartLine++;
    }
    else
      VBlockSizeY++;

  if (SelStart!=-1 && (SelEnd==-1 || CurPos<SelEnd))
  {
    if (CurPos>=SelStart)
    {
      CurLine->Select(SelStart,-1);
      NewString->Select(0,SelEnd==-1 ? -1:SelEnd-CurPos);
    }
    else
    {
      CurLine->Select(-1,0);
      NewString->Select(SelStart-CurPos,SelEnd==-1 ? -1:SelEnd-CurPos);
      BlockStart=NewString;
      BlockStartLine++;
    }
  }
  else
    if (BlockStart!=NULL && NumLine<BlockStartLine)
      BlockStartLine++;

  NewString->SetEOL(EndSeq);

  CurLine->SetCurPos(0);
  if (CurLine==EndList)
    EndList=NewString;
  NumLastLine++;
  Down();

  if (IndentPos>0)
  {
    int OrgIndentPos=IndentPos;
    ShowEditor(FALSE);

    CurLine->GetBinaryString(&CurLineStr,NULL,Length);

    if (SpaceOnly)
    {
      int Decrement=0;
      for (int I=0;I<IndentPos && I<Length;I++)
      {
        if (!IsSpace(CurLineStr[I]))
          break;
        if (CurLineStr[I]==L' ')
          Decrement++;
        else
        {
          int TabPos=CurLine->RealPosToTab(I);
          Decrement+=EdOpt.TabSize - (TabPos % EdOpt.TabSize);
        }
      }
      IndentPos-=Decrement;
    }

    if (IndentPos>0)
    {
      if (CurLine->GetLength()!=0 || !EdOpt.CursorBeyondEOL)
      {
        CurLine->ProcessKey(KEY_HOME);

        int SaveOvertypeMode=CurLine->GetOvertypeMode();
        CurLine->SetOvertypeMode(FALSE);

        const wchar_t *PrevStr=NULL;
        int PrevLength=0;

        if (SrcIndent)
        {
          SrcIndent->GetBinaryString(&PrevStr,NULL,PrevLength);
        }

        for (int I=0;CurLine->GetTabCurPos()<IndentPos;I++)
        {
          if (SrcIndent!=NULL && I<PrevLength && IsSpace(PrevStr[I]))
          {
            CurLine->ProcessKey(PrevStr[I]);
          }
          else
          {
            CurLine->ProcessKey(KEY_SPACE);
          }
        }
        while (CurLine->GetTabCurPos()>IndentPos)
          CurLine->ProcessKey(KEY_BS);

        CurLine->SetOvertypeMode(SaveOvertypeMode);
      }
      CurLine->SetTabCurPos(IndentPos);
    }

    CurLine->GetBinaryString(&CurLineStr,NULL,Length);
    CurPos=CurLine->GetCurPos();
    if (SpaceOnly)
    {
      int NewPos=0;
      for (int I=0;I<Length;I++)
      {
        NewPos=I;
        if (!IsSpace(CurLineStr[I]))
          break;
      }
      if (NewPos>OrgIndentPos)
        NewPos=OrgIndentPos;
      if (NewPos>CurPos)
        CurLine->SetCurPos(NewPos);
    }
  }
  TextChanged(1);
}



void Editor::Down()
{
  //TODO: "�������" - ���� ������ "!Flags.Check(FSCROBJ_VISIBLE)", �� ������� ���� �� ��������� ������� ������
  Edit *CurPtr;
  int LeftPos,CurPos,Y;
  if (CurLine->m_next==NULL)
    return;

  for (Y=0,CurPtr=TopScreen;CurPtr && CurPtr!=CurLine;CurPtr=CurPtr->m_next)
    Y++;

  if (Y>=Y2-Y1)
    TopScreen=TopScreen->m_next;

  CurPos=CurLine->GetTabCurPos();
  LeftPos=CurLine->GetLeftPos();
  CurLine=CurLine->m_next;
  NumLine++;
  CurLine->SetLeftPos(LeftPos);
  CurLine->SetTabCurPos(CurPos);
}


void Editor::ScrollDown()
{
  //TODO: "�������" - ���� ������ "!Flags.Check(FSCROBJ_VISIBLE)", �� ������� ���� �� ��������� ������� ������
  int LeftPos,CurPos;
  if (CurLine->m_next==NULL || TopScreen->m_next==NULL)
    return;
  if (!EdOpt.AllowEmptySpaceAfterEof && CalcDistance(TopScreen,EndList,Y2-Y1)<Y2-Y1)
  {
    Down();
    return;
  }
  TopScreen=TopScreen->m_next;
  CurPos=CurLine->GetTabCurPos();
  LeftPos=CurLine->GetLeftPos();
  CurLine=CurLine->m_next;
  NumLine++;
  CurLine->SetLeftPos(LeftPos);
  CurLine->SetTabCurPos(CurPos);
}


void Editor::Up()
{
  //TODO: "�������" - ���� ������ "!Flags.Check(FSCROBJ_VISIBLE)", �� ������� ���� �� ��������� ������� ������
  int LeftPos,CurPos;
  if (CurLine->m_prev==NULL)
    return;

  if (CurLine==TopScreen)
    TopScreen=TopScreen->m_prev;

  CurPos=CurLine->GetTabCurPos();
  LeftPos=CurLine->GetLeftPos();
  CurLine=CurLine->m_prev;
  NumLine--;
  CurLine->SetLeftPos(LeftPos);
  CurLine->SetTabCurPos(CurPos);
}


void Editor::ScrollUp()
{
  //TODO: "�������" - ���� ������ "!Flags.Check(FSCROBJ_VISIBLE)", �� ������� ���� �� ��������� ������� ������
  int LeftPos,CurPos;
  if (CurLine->m_prev==NULL)
    return;
  if (TopScreen->m_prev==NULL)
  {
    Up();
    return;
  }

  TopScreen=TopScreen->m_prev;
  CurPos=CurLine->GetTabCurPos();
  LeftPos=CurLine->GetLeftPos();
  CurLine=CurLine->m_prev;
  NumLine--;
  CurLine->SetLeftPos(LeftPos);
  CurLine->SetTabCurPos(CurPos);
}

/* $ 21.01.2001 SVS
   ������� ������/������ ������� �� Editor::Search
   � ��������� ������� GetSearchReplaceString
   (���� stddlg.cpp)
*/
BOOL Editor::Search(int Next)
{
  Edit *CurPtr,*TmpPtr;
  string strSearchStr, strReplaceStr;
  static string strLastReplaceStr;
  static int LastSuccessfulReplaceMode=0;
  string strMsgStr;
  const wchar_t *TextHistoryName=L"SearchText",*ReplaceHistoryName=L"ReplaceText";
  int CurPos,Count,Case,WholeWords,ReverseSearch,Match,NewNumLine,UserBreak;

  if (Next && strLastSearchStr.IsEmpty() )
    return TRUE;

  strSearchStr = strLastSearchStr;
  strReplaceStr = strLastReplaceStr;

  Case=LastSearchCase;
  WholeWords=LastSearchWholeWords;
  ReverseSearch=LastSearchReverse;

  if (!Next)
    if(!GetSearchReplaceString(ReplaceMode,&strSearchStr,
                   &strReplaceStr,
                   TextHistoryName,ReplaceHistoryName,
                   &Case,&WholeWords,&ReverseSearch))
      return FALSE;

  strLastSearchStr = strSearchStr;
  strLastReplaceStr = strReplaceStr;

  LastSearchCase=Case;
  LastSearchWholeWords=WholeWords;
  LastSearchReverse=ReverseSearch;

  if ( strSearchStr.IsEmpty() )
    return TRUE;

  wchar_t *SearchStr = xf_wcsdup (strSearchStr); //RAVE!!!
  wchar_t *ReplaceStr = xf_wcsdup (strReplaceStr); //BUGBUG!!!

  LastSuccessfulReplaceMode=ReplaceMode;

  if (!EdOpt.PersistentBlocks)
    UnmarkBlock();

  {
    //SaveScreen SaveScr;

    int SearchLength=(int)strSearchStr.GetLength();

    strMsgStr.Format (L"\"%s\"", (const wchar_t*)strSearchStr);
    SetCursorType(FALSE,-1);
    //SetPreRedrawFunc(Editor::PR_EditorShowMsg);
    EditorShowMsg(UMSG(MEditSearchTitle),UMSG(MEditSearchingFor),strMsgStr);

    Count=0;
    Match=0;
    UserBreak=0;
    CurPos=CurLine->GetCurPos();

    /* $ 16.10.2000 tran
       CurPos ������������� ��� ��������� ������
    */
    /* $ 28.11.2000 SVS
       "�, ��� �� ������ - ��� �������� ���� ���������" :-)
       ����� ��������� ����� ��������������
    */
    /* $ 21.12.2000 SVS
       - � ���������� ����������� ���� ������ �������� ������� ���
         ������� EditorF7Rules
    */
    /* $ 10.06.2001 IS
       - ���: �����-�� ��� ����������� _���������_ ������ �������������� �� ���
         _������_.
    */
    /* $ 09.11.2001 IS
         ��������� �����, ����.
         ����� ������, �.�. �� ������������� �����������
    */
    if( !ReverseSearch && ( Next || (EdOpt.F7Rules && !ReplaceMode) ) )
        CurPos++;

    NewNumLine=NumLine;
    CurPtr=CurLine;

    while (CurPtr!=NULL)
    {
      if ((++Count & 0xfff)==0 && CheckForEsc())
      {
        UserBreak=TRUE;
        break;
      }

      if (CurPtr->Search(SearchStr,CurPos,Case,WholeWords,ReverseSearch))
      {
        int Skip=FALSE;
        /* $ 24.01.2003 KM
           ! �� ��������� ������ �������� �� ����� ������ �� ����� ������������ ������.
        */
        /* $ 15.04.2003 VVM
           �������� �� �������� � �������� �� ���������� �������� ������ */
        int FromTop=(ScrY-2)/4;
        if (FromTop<0 || FromTop>=((ScrY-5)/2-2))
          FromTop=0;

        TmpPtr=CurLine=CurPtr;
        for (int i=0;i<FromTop;i++)
        {
          if (TmpPtr->m_prev)
            TmpPtr=TmpPtr->m_prev;
          else
            break;
        }
        TopScreen=TmpPtr;

        NumLine=NewNumLine;

        int LeftPos=CurPtr->GetLeftPos();
        int TabCurPos=CurPtr->GetTabCurPos();
        if (ObjWidth>8 && TabCurPos-LeftPos+SearchLength>ObjWidth-8)
          CurPtr->SetLeftPos(TabCurPos+SearchLength-ObjWidth+8);

        if (ReplaceMode)
        {
          int MsgCode=0;
          if (!ReplaceAll)
          {
            Show();
            int CurX,CurY;
            GetCursorPos(CurX,CurY);
            GotoXY(CurX,CurY);
            SetColor(COL_EDITORSELECTEDTEXT);
            const wchar_t *Str=CurPtr->GetStringAddrW()+CurPtr->GetCurPos();
            wchar_t *TmpStr=new wchar_t[SearchLength+1];
            xwcsncpy(TmpStr,Str,SearchLength);
            TmpStr[SearchLength]=0;

            /*
            if (UseDecodeTable)
              DecodeString(TmpStr,(unsigned char *)TableSet.DecodeTable);*/ //BUGBUG
            Text(TmpStr);
            delete[] TmpStr;

            string strQSearchStr, strQReplaceStr;
            strQSearchStr.Format (L"\"%s\"", (const wchar_t*)strLastSearchStr);
            strQReplaceStr.Format (L"\"%s\"", (const wchar_t*)strLastReplaceStr);

            MsgCode=Message(0,4,UMSG(MEditReplaceTitle),UMSG(MEditAskReplace),
              strQSearchStr,UMSG(MEditAskReplaceWith),strQReplaceStr,
              UMSG(MEditReplace),UMSG(MEditReplaceAll),UMSG(MEditSkip),UMSG(MEditCancel));
            if (MsgCode==1)
              ReplaceAll=TRUE;
            if (MsgCode==2)
              Skip=TRUE;
            if (MsgCode<0 || MsgCode==3)
            {
              UserBreak=TRUE;
              break;
            }
          }
          if (MsgCode==0 || MsgCode==1)
          {
            Pasting++;
            /*$ 15.08.2000 skv
              If Replace string doesn't contain control symbols (tab and return),
              processed with fast method, otherwise use improved old one.
            */
            if(wcschr(ReplaceStr,L'\t') || wcschr(ReplaceStr,13)) //BUGBUG!!
            {
              int SaveOvertypeMode=Flags.Check(FEDITOR_OVERTYPE);
              Flags.Set(FEDITOR_OVERTYPE);
              CurLine->SetOvertypeMode(TRUE);
              //int CurPos=CurLine->GetCurPos();
              int I;
              for (I=0;SearchStr[I]!=0 && ReplaceStr[I]!=0;I++)
              {
                int Ch=ReplaceStr[I];
                if (Ch==KEY_TAB)
                {
                  Flags.Clear(FEDITOR_OVERTYPE);
                  CurLine->SetOvertypeMode(FALSE);
                  ProcessKey(KEY_DEL);
                  ProcessKey(KEY_TAB);
                  Flags.Set(FEDITOR_OVERTYPE);
                  CurLine->SetOvertypeMode(TRUE);
                  continue;
                }
                /* $ 24.05.2002 SKV
                  ���� ��������� �� Enter, �� overtype �� �����.
                  ����� ������� ������� ��, ��� ��������.
                */
                if(Ch==0x0d)
                {
                  ProcessKey(KEY_DEL);
                }

                if (Ch!=KEY_BS && !(Ch==KEY_DEL || Ch==KEY_NUMDEL))
                  ProcessKey(Ch);
              }
              if(SearchStr[I]==0)
              {
                Flags.Clear(FEDITOR_OVERTYPE);
                CurLine->SetOvertypeMode(FALSE);
                for (;ReplaceStr[I]!=0;I++)
                {
                  int Ch=ReplaceStr[I];
                  if (Ch!=KEY_BS && !(Ch==KEY_DEL || Ch==KEY_NUMDEL))
                    ProcessKey(Ch);
                }
              }else
              {
                for (;SearchStr[I]!=0;I++)
                {
                  ProcessKey(KEY_DEL);
                }
              }
              int Cnt=0;
              wchar_t *Tmp=(wchar_t*)ReplaceStr;
              while((Tmp=wcschr(Tmp,13)) != NULL)
              {
                Cnt++;
                Tmp++;
              }
              if(Cnt>0)
              {
                CurPtr=CurLine;
                NewNumLine+=Cnt;
              }
              Flags.Change(FEDITOR_OVERTYPE,SaveOvertypeMode);
            }
            else
            {
              /* Fast method */
              const wchar_t *Str,*Eol;
              int StrLen,NewStrLen;
              int SStrLen=(int)strSearchStr.GetLength(),
                  RStrLen=(int)strReplaceStr.GetLength();
              CurLine->GetBinaryString(&Str,&Eol,StrLen);
              int EolLen=StrLength(Eol);
              NewStrLen=StrLen;
              NewStrLen-=SStrLen;
              NewStrLen+=RStrLen;
              NewStrLen+=EolLen;
              wchar_t *NewStr=new wchar_t[NewStrLen+1];
              int CurPos=CurLine->GetCurPos();
              wmemcpy(NewStr,Str,CurPos);
              wmemcpy(NewStr+CurPos,ReplaceStr,RStrLen);

              /*if(UseDecodeTable)
              {
                EncodeString(NewStr+CurPos,(unsigned char*)TableSet.EncodeTable,RStrLen);
              }*/ //BUGBUG!!!

              wmemcpy(NewStr+CurPos+RStrLen,Str+CurPos+SStrLen,StrLen-CurPos-SStrLen);
              wmemcpy(NewStr+NewStrLen-EolLen,Eol,EolLen);
              AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,
                          CurLine->GetCurPos(),UNDO_EDIT);
              CurLine->SetBinaryString(NewStr,NewStrLen);
              CurLine->SetCurPos(CurPos+RStrLen);
              delete [] NewStr;

              TextChanged(1);
            }
            /* skv$*/

            //AY: � ���� ��� ������� ���������� � ��� �������� � �� �����������
            //���������������� ��� Replace
            //if (ReverseSearch)
              //CurLine->SetCurPos(CurPos);

            Pasting--;
          }
        }
        Match=1;
        if (!ReplaceMode)
          break;
        CurPos=CurLine->GetCurPos();
        if (Skip)
          if (!ReverseSearch)
            CurPos++;
      }
      else
        if (ReverseSearch)
        {
          CurPtr=CurPtr->m_prev;
          if (CurPtr==NULL)
            break;
          CurPos=CurPtr->GetLength();
          NewNumLine--;
        }
        else
        {
          CurPos=0;
          CurPtr=CurPtr->m_next;
          NewNumLine++;
        }
    }
    //SetPreRedrawFunc(NULL);
  }
  Show();
  if (!Match && !UserBreak)
    Message(MSG_DOWN|MSG_WARNING,1,UMSG(MEditSearchTitle),UMSG(MEditNotFound),
            strMsgStr,UMSG(MOk));

  xf_free (SearchStr); //RAVE!!!
  xf_free (ReplaceStr); //BUGBUG!!!

  return TRUE;
}

void Editor::Paste(const wchar_t *Src)
{
  if (Flags.Check(FEDITOR_LOCKMODE))
    return;

  wchar_t *ClipText=(wchar_t *)Src;
  BOOL IsDeleteClipText=FALSE;

  if(!ClipText)
  {
    if ((ClipText=PasteFormatFromClipboard(FAR_VerticalBlock))!=NULL)
    {
      VPaste(ClipText);
      return;
    }
    if ((ClipText=PasteFromClipboard())==NULL)
      return;
    IsDeleteClipText=TRUE;
  }

  if (*ClipText)
  {
    Flags.Set(FEDITOR_NEWUNDO);

    /*
    if (UseDecodeTable)
      EncodeString(ClipText,(unsigned char *)TableSet.EncodeTable);*/ //BUGBUG
    TextChanged(1);
    int SaveOvertype=Flags.Check(FEDITOR_OVERTYPE);
    UnmarkBlock();
    Pasting++;
    Lock ();

    if (Flags.Check(FEDITOR_OVERTYPE))
    {
      Flags.Clear(FEDITOR_OVERTYPE);
      CurLine->SetOvertypeMode(FALSE);
    }
    BlockStart=CurLine;
    BlockStartLine=NumLine;
    /* $ 19.05.2001 IS
       ������� �������� ���������� ����������� ��������� (������� ������ ����
       ��������� � ������ ������ ��� �����������) � �������.
    */
    int StartPos=CurLine->GetCurPos(),
        oldAutoIndent=EdOpt.AutoIndent;

    for (int I=0;ClipText[I]!=0;)
      if (ClipText[I]!=10)
        if (ClipText[I]==13)
        {
          CurLine->Select(StartPos,-1);
          StartPos=0;
          EdOpt.AutoIndent=FALSE;
          ProcessKey(KEY_ENTER);
          BlockUndo=TRUE;
          I++;
        }
        else
        {
          if(EdOpt.AutoIndent)       // ������ ������ ������� ���, �����
          {                          // �������� ����������
            //ProcessKey(UseDecodeTable?TableSet.DecodeTable[(unsigned)ClipText[I]]:ClipText[I]); //BUGBUG
              ProcessKey(ClipText[I]); //BUGBUG

            I++;
            StartPos=CurLine->GetCurPos();
            if(StartPos) StartPos--;
          }

          int Pos=I;
          while (ClipText[Pos]!=0 && ClipText[Pos]!=10 && ClipText[Pos]!=13)
            Pos++;
          if (Pos>I)
          {
            const wchar_t *Str;
            int Length,CurPos;
            CurLine->GetBinaryString(&Str,NULL,Length);
            CurPos=CurLine->GetCurPos();
            AddUndoData(Str,CurLine->GetEOL(),NumLine,CurPos,UNDO_EDIT); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
            BlockUndo=TRUE;
            CurLine->InsertBinaryString(&ClipText[I],Pos-I);
          }
          I=Pos;
        }
      else
        I++;

    EdOpt.AutoIndent=oldAutoIndent;

    CurLine->Select(StartPos,CurLine->GetCurPos());
    /* IS $ */

    if (SaveOvertype)
    {
      Flags.Set(FEDITOR_OVERTYPE);
      CurLine->SetOvertypeMode(TRUE);
    }


    Pasting--;
    Unlock ();
  }
  if(IsDeleteClipText)
    xf_free(ClipText);
  BlockUndo=FALSE;
}


void Editor::Copy(int Append)
{
  if (VBlockStart!=NULL)
  {
    VCopy(Append);
    return;
  }

  Edit *CurPtr=BlockStart;
  wchar_t *CopyData=NULL;
  size_t DataSize=0;

  if (Append)
  {
    CopyData=PasteFromClipboard();
    if (CopyData!=NULL)
      DataSize=StrLength(CopyData);
  }

  while (CurPtr!=NULL)
  {
    int StartSel,EndSel;
    int Length=CurPtr->GetLength()+1;
    CurPtr->GetSelection(StartSel,EndSel);
    if (StartSel==-1)
      break;
    wchar_t *NewPtr=(wchar_t *)xf_realloc(CopyData,(DataSize+Length+2)*sizeof (wchar_t));
    if (NewPtr==NULL)
    {
      if (CopyData)
      {
        xf_free(CopyData);
        CopyData=NULL;
      }
      break;
    }
    CopyData=NewPtr;
    CurPtr->GetSelString(CopyData+DataSize,Length);
    DataSize+=StrLength(CopyData+DataSize);
    if (EndSel==-1)
    {
      wcscpy(CopyData+DataSize,DOS_EOL_fmt);
      DataSize+=2;
    }
    CurPtr=CurPtr->m_next;
  }

  if (CopyData!=NULL)
  {
    /*
    if (UseDecodeTable)
      DecodeString(CopyData+PrevSize,(unsigned char *)TableSet.DecodeTable);*/ //BUGBUG
    CopyToClipboard(CopyData);
    xf_free(CopyData);
  }
}


void Editor::DeleteBlock()
{
  if (Flags.Check(FEDITOR_LOCKMODE))
    return;

  if (VBlockStart!=NULL)
  {
    DeleteVBlock();
    return;
  }

  Edit *CurPtr=BlockStart;

  int UndoNext=FALSE;

  while (CurPtr!=NULL)
  {
    TextChanged(1);
    int StartSel,EndSel;
    /* $ 17.09.2002 SKV
      ������ �� Real ��� � ������ ��������� �� ������ ������.
    */
    CurPtr->GetRealSelection(StartSel,EndSel);
    if(EndSel!=-1 && EndSel>CurPtr->GetLength())
      EndSel=-1;
    if (StartSel==-1)
      break;
    if (StartSel==0 && EndSel==-1)
    {
      Edit *NextLine=CurPtr->m_next;
      BlockUndo=UndoNext;
      DeleteString(CurPtr,FALSE,BlockStartLine);
      UndoNext=TRUE;
      if (BlockStartLine<NumLine)
        NumLine--;
      if (NextLine!=NULL)
      {
        CurPtr=NextLine;
        continue;
      }
      else
        break;
    }
    int Length=CurPtr->GetLength();
    if (StartSel!=0 || EndSel!=0)
    {
      BlockUndo=UndoNext;
      AddUndoData(CurPtr->GetStringAddrW(),CurPtr->GetEOL(),BlockStartLine,
                  CurPtr->GetCurPos(),UNDO_EDIT);
      UndoNext=TRUE;
    }
    /* $ 17.09.2002 SKV
      ����� ��� ��������� �� ������ ������.
      InsertBinaryString ������� trailing space'��
    */
    if(StartSel>Length)
    {
      Length=StartSel;
      CurPtr->SetCurPos(Length);
      CurPtr->InsertBinaryString(L"",0);
    }
    const wchar_t *CurStr,*EndSeq;
    CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
    // ������ ����� realloc, ������� ��� malloc.
    wchar_t *TmpStr=(wchar_t*)xf_malloc((Length+3)*sizeof (wchar_t));
    wmemcpy(TmpStr,CurStr,Length);
    TmpStr[Length]=0;
    int DeleteNext=FALSE;
    if (EndSel==-1)
    {
      EndSel=Length;
      if (CurPtr->m_next!=NULL)
        DeleteNext=TRUE;
    }
    wmemmove(TmpStr+StartSel,TmpStr+EndSel,StrLength(TmpStr+EndSel)+1);
    int CurPos=StartSel;
/*    if (CurPos>=StartSel)
    {
      CurPos-=(EndSel-StartSel);
      if (CurPos<StartSel)
        CurPos=StartSel;
    }
*/
    Length-=EndSel-StartSel;
    if (DeleteNext)
    {
      const wchar_t *NextStr,*EndSeq;
      int NextLength,NextStartSel,NextEndSel;
      CurPtr->m_next->GetSelection(NextStartSel,NextEndSel);
      if (NextStartSel==-1)
        NextEndSel=0;
      if (NextEndSel==-1)
        EndSel=-1;
      else
      {
        CurPtr->m_next->GetBinaryString(&NextStr,&EndSeq,NextLength);
        NextLength-=NextEndSel;
        if(NextLength>0)
        {
          TmpStr=(wchar_t *)xf_realloc(TmpStr,(Length+NextLength+3)*sizeof(wchar_t));
          wmemcpy(TmpStr+Length,NextStr+NextEndSel,NextLength);
          Length+=NextLength;
        }
      }
      if (CurLine==CurPtr->m_next)
      {
        CurLine=CurPtr;
        NumLine--;
      }

      BlockUndo=UndoNext;
      if (CurLine==CurPtr && CurPtr->m_next!=NULL && CurPtr->m_next==TopScreen)
      {
        CurLine=CurPtr->m_next;
        NumLine++;
      }
      DeleteString(CurPtr->m_next,FALSE,BlockStartLine+1);
      UndoNext=TRUE;
      if (BlockStartLine+1<NumLine)
        NumLine--;
    }
    int EndLength=StrLength(EndSeq);
    wmemcpy(TmpStr+Length,EndSeq,EndLength);
    Length+=EndLength;
    TmpStr[Length]=0;
    CurPtr->SetBinaryString(TmpStr,Length);
    xf_free(TmpStr);
    CurPtr->SetCurPos(CurPos);
    if (DeleteNext && EndSel==-1)
    {
      CurPtr->Select(CurPtr->GetLength(),-1);
    }
    else
    {
      CurPtr->Select(-1,0);
      CurPtr=CurPtr->m_next;
      BlockStartLine++;
    }
  }
  BlockStart=NULL;
  BlockUndo=FALSE;
}


void Editor::UnmarkBlock()
{
  if (BlockStart==NULL && VBlockStart==NULL)
    return;
  VBlockStart=NULL;
  _SVS(SysLog(L"[%d] Editor::UnmarkBlock()",__LINE__));
  Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
  while (BlockStart!=NULL)
  {
    int StartSel,EndSel;
    BlockStart->GetSelection(StartSel,EndSel);
    if (StartSel==-1)
    {
      /* $ 24.06.2002 SKV
        ���� � ������� ������ ��� ���������,
        ��� ��� �� ������ ��� �� � �����.
        ��� ����� ���� ������ ������ :)
      */
      if(BlockStart->m_next)
      {
        BlockStart->m_next->GetSelection(StartSel,EndSel);
        if(StartSel==-1)
        {
          break;
        }
      }
      else
        break;
    }
    BlockStart->Select(-1,0);
    BlockStart=BlockStart->m_next;
  }
  BlockStart=NULL;
  Show();
}

/* $ 07.03.2002 IS
   ������� ���������, ���� ��� ������ (�������� ���� �������� � ������)
*/
void Editor::UnmarkEmptyBlock()
{
  _SVS(SysLog(L"[%d] Editor::UnmarkEmptyBlock()",__LINE__));
  if(BlockStart || VBlockStart)  // ������������ ���������
  {
    int Lines=0,StartSel,EndSel;
    Edit *Block=BlockStart;
    if(VBlockStart)
    {
      if(VBlockSizeX)
        Lines=VBlockSizeY;
    }
    else while(Block) // ��������� �� ���� ���������� �������
    {
      Block->GetRealSelection(StartSel,EndSel);
      if (StartSel==-1)
        break;
      if(StartSel!=EndSel)// �������� �������-�� ��������
      {
        ++Lines;           // �������� ������� �������� �����
        break;
      }
      Block=Block->m_next;
    }
    if(!Lines)             // ���� �������� ���� �������� � ������, ��
      UnmarkBlock();       // ���������� �������� ������ � ������ ���������
  }
}

/* $ 07.07.2000 tran & SVS
   + ��������� ����������� ���������� �� �������
     �� ������� [!][ROW][,COL]
     �������� ��� �������� ��� ������������� �������� � void �� int
     �� �������� ������� ���������� � �����
     '!' - ������ ������������� �������� (���� �� ����������� ;-)
*/
void Editor::GoToLine(int Line)
{
  int NewLine;

  NewLine=Line;

  int LastNumLine=NumLine;
  int CurScrLine=CalcDistance(TopScreen,CurLine,-1);
  for (NumLine=0,CurLine=TopList;
         NumLine<NewLine && CurLine->m_next!=NULL;
         NumLine++)
    CurLine=CurLine->m_next;
  CurScrLine+=NumLine-LastNumLine;

  if (CurScrLine<0 || CurScrLine>Y2-Y1)
    TopScreen=CurLine;

// <GOTO_UNMARK:2>
//  if (!EdOpt.PersistentBlocks)
//     UnmarkBlock();
// </GOTO_UNMARK>

  Show();
  return ;
}
/* tran 21.07.2000 $ */

/* $ 07.07.2000 tran & SVS
   + ��������� ����������� ���������� �� �������
     �� ������� [!][ROW][,COL]
     �������� ��� �������� ��� ������������� �������� � void �� int
     �� �������� ������� ���������� � �����
     '!' - ������ ������������� �������� (���� �� ����������� ;-)
*/
void Editor::GoToPosition()
{
  int NewLine, NewCol;
  int LeftPos=CurLine->GetTabCurPos()+1;
  int CurPos;
  CurPos=CurLine->GetCurPos();

  const wchar_t *LineHistoryName=L"LineNumber";
  static struct DialogDataEx GoToDlgData[]=
  {
    DI_DOUBLEBOX,3,1,21,3,0,0,0,0,(const wchar_t *)MEditGoToLine,
    DI_EDIT,     5,2,19,2,1,(DWORD_PTR)LineHistoryName,DIF_HISTORY|DIF_USELASTHISTORY|DIF_NOAUTOCOMPLETE,1,L"",
  };
  MakeDialogItemsEx(GoToDlgData,GoToDlg);
  /* $ 01.08.2000 tran
    PrevLine ������ �� ����� - USELASTHISTORY ����� */
  //  static char PrevLine[40]={0};

  //  strcpy(GoToDlg[1].Data,PrevLine);
  Dialog Dlg(GoToDlg,sizeof(GoToDlg)/sizeof(GoToDlg[0]));
  Dlg.SetPosition(-1,-1,25,5);
  Dlg.SetHelp(L"EditorGotoPos");
  Dlg.Process();

  /* $ 06.05.2002 KM
      ������� ShadowSaveScr ��� �������������� ���������
      �����������.
  */
  Dialog::SendDlgMessage((HANDLE)&Dlg,DM_KILLSAVESCREEN,0,0);

    // tran: was if (Dlg.GetExitCode()!=1 || !isdigit(*GoToDlg[1].Data))
  if (Dlg.GetExitCode()!=1 )
      return ;
  // �������� ����� ��������� �������� � ������� ������ ������ FAR`�
  //  xstrncpy(PrevLine,GoToDlg[1].Data,sizeof(PrevLine));

  GetRowCol(GoToDlg[1].strData,&NewLine,&NewCol);

  //_D(SysLog(L"GoToPosition: NewLine=%i, NewCol=%i",NewLine,NewCol));
  GoToLine(NewLine);

  if ( NewCol == -1)
  {
    CurLine->SetTabCurPos(CurPos);
    CurLine->SetLeftPos(LeftPos);
  }
  else
    CurLine->SetTabCurPos(NewCol);

// <GOTO_UNMARK:3>
//  if (!EdOpt.PersistentBlocks)
//     UnmarkBlock();
// </GOTO_UNMARK>

  Show();
  return ;
}

void Editor::GetRowCol(const wchar_t *_argv,int *row,int *col)
{
  int x=0xffff,y;
  int l;
  wchar_t *argvx=0;
  int LeftPos=CurLine->GetTabCurPos() + 1;

  string strArg = _argv;

  // ��� �� �� �������� "�����" ������ - ������ ��, ��� �� ����� ;-)
  // "�������" ��� ������� �������.
  RemoveExternalSpaces(strArg);

  wchar_t *argv = strArg.GetBuffer ();
  // �������� ������ ��������� ������ �����������
  // � ������� ������
  l=(int)wcscspn(argv,L",:;. ");
  // ���� ����������� ����, �� l=strlen(argv)

  if(l < StrLength(argv)) // ��������: "row,col" ��� ",col"?
  {
    argv[l]=L'\0'; // ������ ����������� ��������� "����� ������" :-)
    argvx=argv+l+1;
    x=_wtoi(argvx);
  }
  y=_wtoi(argv);

  // + ������� �� ��������
  if ( wcschr(argv,L'%')!=0 )
    y=NumLastLine * y / 100;

  //   ��������� ���������������
  if ( argv[0]==L'-' || argv[0]==L'+' )
    y=NumLine+y+1;
  if ( argvx )
  {
    if ( argvx[0]==L'-' || argvx[0]==L'+' )
    {
        x=LeftPos+x;
    }
  }

  strArg.ReleaseBuffer ();

  // ������ ������� ��������� �����
  *row=y;
  if ( x!=0xffff )
    *col=x;
  else
    *col=LeftPos;


  (*row)--;
  if (*row< 0)   // ���� ����� ",Col"
     *row=NumLine;  //   �� ��������� �� ������� ������ � �������
  (*col)--;
  if (*col< -1)
     *col=-1;
  return ;
}

void Editor::AddUndoData(const wchar_t *Str,const wchar_t *Eol,int StrNum,int StrPos,int Type)
{
  int PrevUndoDataPos;
  if (Flags.Check(FEDITOR_DISABLEUNDO) || !UndoData)
    return;
  if (StrNum==-1)
    StrNum=NumLine;
  if ((PrevUndoDataPos=UndoDataPos-1)<0)
    PrevUndoDataPos=EdOpt.UndoSize-1;
  if (!Flags.Check(FEDITOR_NEWUNDO) && Type==UNDO_EDIT &&
      UndoData[PrevUndoDataPos].Type==UNDO_EDIT &&
      StrNum==UndoData[PrevUndoDataPos].StrNum &&
      (abs(StrPos-UndoData[PrevUndoDataPos].StrPos)<=1 ||
      abs(StrPos-LastChangeStrPos)<=1))
  {
    LastChangeStrPos=StrPos;
    return;
  }
  Flags.Clear(FEDITOR_NEWUNDO);

  if (UndoData[UndoDataPos].Type!=UNDO_NONE && UndoData[UndoDataPos].Str!=NULL)
    delete[] UndoData[UndoDataPos].Str;
  UndoData[UndoDataPos].Type=Type;
  UndoData[UndoDataPos].UndoNext=BlockUndo;
  UndoData[UndoDataPos].StrPos=StrPos;
  UndoData[UndoDataPos].StrNum=StrNum;
  xwcsncpy(UndoData[UndoDataPos].EOL,Eol?Eol:L"",countof(UndoData[UndoDataPos].EOL)-1);
  UndoData[UndoDataPos].EOL[countof(UndoData[UndoDataPos].EOL)-1]=0;

  if (Str!=NULL)
  {
    UndoData[UndoDataPos].Str=new wchar_t[StrLength(Str)+1];
    if (UndoData[UndoDataPos].Str!=NULL)
      wcscpy(UndoData[UndoDataPos].Str,Str);
  }
  else
    UndoData[UndoDataPos].Str=NULL;

  if (++UndoDataPos==EdOpt.UndoSize)
    UndoDataPos=0;

  if (UndoDataPos==UndoSavePos)
    Flags.Set(FEDITOR_UNDOOVERFLOW);
}

void Editor::Undo()
{
  if(!UndoData)
    return;
  int NewPos=UndoDataPos-1;
  if (NewPos<0)
    NewPos=EdOpt.UndoSize-1;
  if (UndoData[NewPos].Type==UNDO_NONE)
    return;
  UnmarkBlock();
  UndoDataPos=NewPos;

  TextChanged(1);

  /* $ 30.03.2002 IS
     ������ ���� ������������ FEDITOR_WASCHANGED, �.�. ��� �����, ��� � �����,
     ������ ���������� ����� ��� ��������� ����� ��������������� _�� �����_, �
     �� ��������� � ����� �������� FEDITOR_MODIFIED. ��������� � �����
     ������������� � ������ �����, �� �� ��������������� ���������, ������ ���
     ��� ��������������� � "TextChanged(1)" - ��. ����.
  */
  Flags.Set(/*FEDITOR_WASCHANGED|*/FEDITOR_DISABLEUNDO);

  GoToLine(UndoData[UndoDataPos].StrNum);
  switch(UndoData[UndoDataPos].Type)
  {
    case UNDO_INSSTR:
      DeleteString(CurLine,TRUE,NumLine>0 ? NumLine-1:NumLine);
      break;
    case UNDO_DELSTR:
      Pasting++;
      if (NumLine<UndoData[UndoDataPos].StrNum)
      {
        ProcessKey(KEY_END);
        ProcessKey(KEY_ENTER);
      }
      else
      {
        ProcessKey(KEY_HOME);
        ProcessKey(KEY_ENTER);
        ProcessKey(KEY_UP);
      }
      Pasting--;
      if (UndoData[UndoDataPos].Str!=NULL)
      {
        CurLine->SetString(UndoData[UndoDataPos].Str);
        CurLine->SetEOL(UndoData[UndoDataPos].EOL); // ���������� ������������� ����������, �.�. SetString �������� Edit::SetBinaryString �... ������ �� ������
      }
      break;
    case UNDO_EDIT:
      if (UndoData[UndoDataPos].Str!=NULL)
      {
        CurLine->SetString(UndoData[UndoDataPos].Str);
        CurLine->SetEOL(UndoData[UndoDataPos].EOL); // ���������� ������������� ����������, �.�. SetString �������� Edit::SetBinaryString �... ������ �� ������
      }
      CurLine->SetCurPos(UndoData[UndoDataPos].StrPos);
      break;
  }
  if (UndoData[UndoDataPos].Str!=NULL)
    delete[] UndoData[UndoDataPos].Str;
  UndoData[UndoDataPos].Type=UNDO_NONE;
  if (UndoData[UndoDataPos].UndoNext)
    Undo();
  if (!Flags.Check(FEDITOR_UNDOOVERFLOW) && UndoDataPos==UndoSavePos)
    TextChanged(0);
  Flags.Clear(FEDITOR_DISABLEUNDO);
}

void Editor::SelectAll()
{
  Edit *CurPtr;
  BlockStart=TopList;
  BlockStartLine=0;
  for (CurPtr=TopList;CurPtr!=NULL;CurPtr=CurPtr->m_next)
    if (CurPtr->m_next!=NULL)
      CurPtr->Select(0,-1);
    else
      CurPtr->Select(0,CurPtr->GetLength());
  Show();
}


void Editor::SetStartPos(int LineNum,int CharNum)
{
  StartLine=LineNum==0 ? 1:LineNum;
  StartChar=CharNum==0 ? 1:CharNum;
}


int Editor::IsFileChanged()
{
  return(Flags.Check(FEDITOR_MODIFIED|FEDITOR_WASCHANGED));
}


int Editor::IsFileModified()
{
  return(Flags.Check(FEDITOR_MODIFIED));
}

// ������������ � FileEditor
long Editor::GetCurPos()
{
  Edit *CurPtr=TopList;
  long TotalSize=0;
  while (CurPtr!=TopScreen)
  {
    const wchar_t *SaveStr,*EndSeq;
    int Length;
    CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);
    TotalSize+=Length+StrLength(EndSeq);
    CurPtr=CurPtr->m_next;
  }
  return(TotalSize);
}


/*
void Editor::SetStringsTable()
{
  Edit *CurPtr=TopList;
  while (CurPtr!=NULL)
  {
    CurPtr->SetTables(UseDecodeTable ? &TableSet:NULL);
    CurPtr=CurPtr->m_next;
  }
}
*/


void Editor::BlockLeft()
{
  if (VBlockStart!=NULL)
  {
    VBlockShift(TRUE);
    return;
  }
  Edit *CurPtr=BlockStart;
  int LineNum=BlockStartLine;
  /* $ 14.02.2001 VVM
    + ��� ���������� ����� AltU/AltI �������� ������� ������� */
  int MoveLine = 0;
  if (CurPtr==NULL)
  {
    MoveLine = 1;
    CurPtr = CurLine;
    LineNum = NumLine;
  }

  while (CurPtr!=NULL)
  {
    int StartSel,EndSel;
    CurPtr->GetSelection(StartSel,EndSel);
    /* $ 14.02.2001 VVM
      + ����� ��� - ������� ��� ������������ */
    if (MoveLine) {
      StartSel = 0; EndSel = -1;
    }

    if (StartSel==-1)
      break;

    int Length=CurPtr->GetLength();
    wchar_t *TmpStr=new wchar_t[Length+EdOpt.TabSize+5];

    const wchar_t *CurStr,*EndSeq;
    CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);

    Length--;
    if (*CurStr==L' ')
      wmemcpy(TmpStr,CurStr+1,Length);
    else
      if (*CurStr==L'\t')
      {
        wmemset(TmpStr, L' ', EdOpt.TabSize-1);
        wmemcpy(TmpStr+EdOpt.TabSize-1,CurStr+1,Length);
        Length+=EdOpt.TabSize-1;
      }

    if ((EndSel==-1 || EndSel>StartSel) && IsSpace(*CurStr))
    {
      int EndLength=StrLength(EndSeq);
      wmemcpy(TmpStr+Length,EndSeq,EndLength);
      Length+=EndLength;
      TmpStr[Length]=0;
      AddUndoData(CurStr,CurPtr->GetEOL(),LineNum,0,UNDO_EDIT); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
      BlockUndo=TRUE;
      int CurPos=CurPtr->GetCurPos();
      CurPtr->SetBinaryString(TmpStr,Length);
      CurPtr->SetCurPos(CurPos>0 ? CurPos-1:CurPos);

      if (!MoveLine)
        CurPtr->Select(StartSel>0 ? StartSel-1:StartSel,EndSel>0 ? EndSel-1:EndSel);

      TextChanged(1);
    }

    delete[] TmpStr;
    CurPtr=CurPtr->m_next;
    LineNum++;
    MoveLine = 0;
  }
  BlockUndo=FALSE;
}


void Editor::BlockRight()
{
  if (VBlockStart!=NULL)
  {
    VBlockShift(FALSE);
    return;
  }
  Edit *CurPtr=BlockStart;
  int LineNum=BlockStartLine;
  /* $ 14.02.2001 VVM
    + ��� ���������� ����� AltU/AltI �������� ������� ������� */
  int MoveLine = 0;
  if (CurPtr==NULL)
  {
    MoveLine = 1;
    CurPtr = CurLine;
    LineNum = NumLine;
  }

  while (CurPtr!=NULL)
  {
    int StartSel,EndSel;
    CurPtr->GetSelection(StartSel,EndSel);
    /* $ 14.02.2001 VVM
      + ����� ��� - ������� ��� ������������ */
    if (MoveLine) {
      StartSel = 0; EndSel = -1;
    }

    if (StartSel==-1)
      break;

    int Length=CurPtr->GetLength();
    wchar_t *TmpStr=new wchar_t[Length+5];

    const wchar_t *CurStr,*EndSeq;
    CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
    *TmpStr=L' ';
    wmemcpy(TmpStr+1,CurStr,Length);
    Length++;

    if (EndSel==-1 || EndSel>StartSel)
    {
      int EndLength=StrLength(EndSeq);
      wmemcpy(TmpStr+Length,EndSeq,EndLength);
      TmpStr[Length+EndLength]=0;
      AddUndoData(CurStr,CurPtr->GetEOL(),LineNum,0,UNDO_EDIT); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
      BlockUndo=TRUE;
      int CurPos=CurPtr->GetCurPos();
      if (Length>1)
        CurPtr->SetBinaryString(TmpStr,Length+EndLength);
      CurPtr->SetCurPos(CurPos+1);

      if (!MoveLine)
        CurPtr->Select(StartSel>0 ? StartSel+1:StartSel,EndSel>0 ? EndSel+1:EndSel);

      TextChanged(1);
    }

    delete[] TmpStr;
    CurPtr=CurPtr->m_next;
    LineNum++;
    MoveLine = 0;
  }
  BlockUndo=FALSE;
}


void Editor::DeleteVBlock()
{
  if (Flags.Check(FEDITOR_LOCKMODE) || VBlockSizeX<=0 || VBlockSizeY<=0)
    return;

  int UndoNext=FALSE;

  if (!EdOpt.PersistentBlocks)
  {
    Edit *CurPtr=CurLine;
    Edit *NewTopScreen=TopScreen;
    while (CurPtr!=NULL)
    {
      if (CurPtr==VBlockStart)
      {
        TopScreen=NewTopScreen;
        CurLine=CurPtr;
        CurPtr->SetTabCurPos(VBlockX);
        break;
      }
      NumLine--;
      if (NewTopScreen==CurPtr && CurPtr->m_prev!=NULL)
        NewTopScreen=CurPtr->m_prev;
      CurPtr=CurPtr->m_prev;
    }
  }

  Edit *CurPtr=VBlockStart;

  for (int Line=0;CurPtr!=NULL && Line<VBlockSizeY;Line++,CurPtr=CurPtr->m_next)
  {
    TextChanged(1);

    int TBlockX=CurPtr->TabPosToReal(VBlockX);
    int TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
                    CurPtr->TabPosToReal(VBlockX);

    const wchar_t *CurStr,*EndSeq;
    int Length;
    CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
    if (TBlockX>=Length)
      continue;

    BlockUndo=UndoNext;
    AddUndoData(CurPtr->GetStringAddrW(),CurPtr->GetEOL(),BlockStartLine+Line,
                CurPtr->GetCurPos(),UNDO_EDIT);
    UndoNext=TRUE;

    wchar_t *TmpStr=new wchar_t[Length+3];
    int CurLength=TBlockX;
    wmemcpy(TmpStr,CurStr,TBlockX);
    if (Length>TBlockX+TBlockSizeX)
    {
      int CopySize=Length-(TBlockX+TBlockSizeX);
      wmemcpy(TmpStr+CurLength,CurStr+TBlockX+TBlockSizeX,CopySize);
      CurLength+=CopySize;
    }
    int EndLength=StrLength(EndSeq);
    wmemcpy(TmpStr+CurLength,EndSeq,EndLength);
    CurLength+=EndLength;
    TmpStr[CurLength]=0;

    int CurPos=CurPtr->GetCurPos();
    CurPtr->SetBinaryString(TmpStr,CurLength);
    if (CurPos>TBlockX)
    {
      CurPos-=TBlockSizeX;
      if (CurPos<TBlockX)
        CurPos=TBlockX;
    }
    CurPtr->SetCurPos(CurPos);
    delete[] TmpStr;
  }

  VBlockStart=NULL;
  BlockUndo=FALSE;
}

void Editor::VCopy(int Append)
{
  Edit *CurPtr=VBlockStart;
  wchar_t *CopyData=NULL;
  size_t DataSize=0;

  if (Append)
  {
    CopyData=PasteFormatFromClipboard(FAR_VerticalBlock);
    if (CopyData!=NULL)
      DataSize=StrLength(CopyData);
    else
    {
      CopyData=PasteFromClipboard();
      if (CopyData!=NULL)
        DataSize=StrLength(CopyData);
    }
  }

  for (int Line=0;CurPtr!=NULL && Line<VBlockSizeY;Line++,CurPtr=CurPtr->m_next)
  {
    int TBlockX=CurPtr->TabPosToReal(VBlockX);
    int TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
                    CurPtr->TabPosToReal(VBlockX);
    const wchar_t *CurStr,*EndSeq;
    int Length;
    CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);

    size_t AllocSize=Max(DataSize+Length+3,DataSize+TBlockSizeX+3);
    wchar_t *NewPtr=(wchar_t *)xf_realloc(CopyData,AllocSize*sizeof (wchar_t));
    if (NewPtr==NULL)
    {
      if (CopyData)
      {
        xf_free(CopyData);
        CopyData=NULL;
      }
      break;
    }
    CopyData=NewPtr;

    if (Length>TBlockX)
    {
      int CopySize=Length-TBlockX;
      if (CopySize>TBlockSizeX)
        CopySize=TBlockSizeX;
      wmemcpy(CopyData+DataSize,CurStr+TBlockX,CopySize);
      if (CopySize<TBlockSizeX)
        wmemset(CopyData+DataSize+CopySize,L' ',TBlockSizeX-CopySize);
    }
    else
      wmemset(CopyData+DataSize,L' ',TBlockSizeX);

    DataSize+=TBlockSizeX;


    wcscpy(CopyData+DataSize,DOS_EOL_fmt);
    DataSize+=2;
  }

  if (CopyData!=NULL)
  {
    /*if (UseDecodeTable)
      DecodeString(CopyData+PrevSize,(unsigned char *)TableSet.DecodeTable);*/ //BUGBUG
    CopyToClipboard(CopyData);
    CopyFormatToClipboard(FAR_VerticalBlock,CopyData);
    xf_free(CopyData);
  }
}

void Editor::VPaste(wchar_t *ClipText)
{
  if (Flags.Check(FEDITOR_LOCKMODE))
    return;

  if (*ClipText)
  {
    Flags.Set(FEDITOR_NEWUNDO);
    TextChanged(1);
    int SaveOvertype=Flags.Check(FEDITOR_OVERTYPE);
    UnmarkBlock();
    Pasting++;
    Lock ();

    if (Flags.Check(FEDITOR_OVERTYPE))
    {
      Flags.Clear(FEDITOR_OVERTYPE);
      CurLine->SetOvertypeMode(FALSE);
    }

    VBlockStart=CurLine;
    BlockStartLine=NumLine;

    int StartPos=CurLine->GetTabCurPos();

    VBlockX=StartPos;
    VBlockSizeX=0;
    VBlockY=NumLine;
    VBlockSizeY=0;

    Edit *SavedTopScreen=TopScreen;


    for (int I=0;ClipText[I]!=0;I++)
      if (ClipText[I]!=13 && ClipText[I+1]!=10)
        ProcessKey(ClipText[I]);
      else
      {
        BlockUndo=TRUE;
        int CurWidth=CurLine->GetTabCurPos()-StartPos;
        if (CurWidth>VBlockSizeX)
          VBlockSizeX=CurWidth;
        VBlockSizeY++;
        if (CurLine->m_next==NULL)
        {
          if (ClipText[I+2]!=0)
          {
            ProcessKey(KEY_END);
            ProcessKey(KEY_ENTER);
            /* $ 19.05.2001 IS
               �� ��������� ������� �����, ����� ��� �� ���� �� ������, �
               ������ - ��� ���������� ����������� ������ ��������� �� �����,
               ��� ���� ��������� � � ������ �����.
            */
            if(!EdOpt.AutoIndent)
              for (int I=0;I<StartPos;I++)
                ProcessKey(L' ');
          }
        }
        else
        {
          ProcessKey(KEY_DOWN);
          CurLine->SetTabCurPos(StartPos);
          CurLine->SetOvertypeMode(FALSE);
        }
        I++;
        continue;
      }

    int CurWidth=CurLine->GetTabCurPos()-StartPos;
    if (CurWidth>VBlockSizeX)
      VBlockSizeX=CurWidth;
    if (VBlockSizeY==0)
      VBlockSizeY++;

    if (SaveOvertype)
    {
      Flags.Set(FEDITOR_OVERTYPE);
      CurLine->SetOvertypeMode(TRUE);
    }

    TopScreen=SavedTopScreen;
    CurLine=VBlockStart;
    NumLine=BlockStartLine;
    CurLine->SetTabCurPos(StartPos);


    Pasting--;
    Unlock ();
  }
  xf_free(ClipText);
  BlockUndo=FALSE;
}


void Editor::VBlockShift(int Left)
{
  if (Flags.Check(FEDITOR_LOCKMODE) || Left && VBlockX==0 || VBlockSizeX<=0 || VBlockSizeY<=0)
    return;

  Edit *CurPtr=VBlockStart;

  int UndoNext=FALSE;

  for (int Line=0;CurPtr!=NULL && Line<VBlockSizeY;Line++,CurPtr=CurPtr->m_next)
  {
    TextChanged(1);

    int TBlockX=CurPtr->TabPosToReal(VBlockX);
    int TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
                    CurPtr->TabPosToReal(VBlockX);

    const wchar_t *CurStr,*EndSeq;
    int Length;
    CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
    if (TBlockX>Length)
      continue;
    if (Left && CurStr[TBlockX-1]==L'\t' ||
        !Left && TBlockX+TBlockSizeX<Length && CurStr[TBlockX+TBlockSizeX]==L'\t')
    {
      CurPtr->ReplaceTabs();
      CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
      TBlockX=CurPtr->TabPosToReal(VBlockX);
      TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
                  CurPtr->TabPosToReal(VBlockX);
    }


    BlockUndo=UndoNext;
    AddUndoData(CurPtr->GetStringAddrW(),CurPtr->GetEOL(),BlockStartLine+Line,
                CurPtr->GetCurPos(),UNDO_EDIT);
    UndoNext=TRUE;

    int StrLen=Max(Length,TBlockX+TBlockSizeX+!Left);
    wchar_t *TmpStr=new wchar_t[StrLen+3];
    wmemset(TmpStr,L' ',StrLen);
    wmemcpy(TmpStr,CurStr,Length);

    if (Left)
    {
      int Ch=TmpStr[TBlockX-1];
      for (int I=TBlockX;I<TBlockX+TBlockSizeX;I++)
        TmpStr[I-1]=TmpStr[I];
      TmpStr[TBlockX+TBlockSizeX-1]=Ch;
    }
    else
    {
      int Ch=TmpStr[TBlockX+TBlockSizeX];
      for (int I=TBlockX+TBlockSizeX-1;I>=TBlockX;I--)
        TmpStr[I+1]=TmpStr[I];
      TmpStr[TBlockX]=Ch;
    }

    while (StrLen>0 && TmpStr[StrLen-1]==L' ')
      StrLen--;
    int EndLength=StrLength(EndSeq);
    wmemcpy(TmpStr+StrLen,EndSeq,EndLength);
    StrLen+=EndLength;
    TmpStr[StrLen]=0;

    CurPtr->SetBinaryString(TmpStr,StrLen);
    delete[] TmpStr;
  }
  VBlockX+=Left ? -1:1;
  CurLine->SetTabCurPos(Left ? VBlockX:VBlockX+VBlockSizeX);
}


int Editor::EditorControl(int Command,void *Param)
{
  int I;
  _ECTLLOG(CleverSysLog SL(L"Editor::EditorControl()"));
  _ECTLLOG(SysLog(L"Command=%s Param=[%d/0x%08X]",_ECTL_ToName(Command),Param,Param));
  switch(Command)
  {
    case ECTL_GETSTRING:
    {
      struct EditorGetString *GetString=(struct EditorGetString *)Param;

      if(GetString && !IsBadReadPtr(GetString,sizeof(struct EditorGetString)))
      {
        Edit *CurPtr=GetStringByNumber(GetString->StringNumber);
        if (!CurPtr)
        {
          _ECTLLOG(SysLog(L"struct EditorGetString => GetStringByNumber(%d) return NULL",GetString->StringNumber));
          return(FALSE);
        }
        CurPtr->GetBinaryString(const_cast<const wchar_t **>(&GetString->StringText),
                                const_cast<const wchar_t **>(&GetString->StringEOL),
                                GetString->StringLength);
        GetString->SelStart=-1;
        GetString->SelEnd=0;
        int DestLine=GetString->StringNumber;
        if (DestLine==-1)
          DestLine=NumLine;
        if (BlockStart!=NULL)
        {
          CurPtr->GetRealSelection(GetString->SelStart,GetString->SelEnd);
        }
        else if (VBlockStart!=NULL && DestLine>=VBlockY && DestLine<VBlockY+VBlockSizeY)
        {
          GetString->SelStart=CurPtr->TabPosToReal(VBlockX);
          GetString->SelEnd=GetString->SelStart+
                            CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
                            CurPtr->TabPosToReal(VBlockX);
        }
        _ECTLLOG(SysLog(L"struct EditorGetString{"));
        _ECTLLOG(SysLog(L"  StringNumber    =%d",GetString->StringNumber));
        _ECTLLOG(SysLog(L"  StringText      ='%s'",GetString->StringText));
        _ECTLLOG(SysLog(L"  StringEOL       ='%s'",GetString->StringEOL?_SysLog_LinearDump((LPBYTE)GetString->StringEOL,StrLength(GetString->StringEOL)):L"(null)"));
        _ECTLLOG(SysLog(L"  StringLength    =%d",GetString->StringLength));
        _ECTLLOG(SysLog(L"  SelStart        =%d",GetString->SelStart));
        _ECTLLOG(SysLog(L"  SelEnd          =%d",GetString->SelEnd));
        _ECTLLOG(SysLog(L"}"));
        return(TRUE);
      }
      break;
    }

    case ECTL_INSERTSTRING:
    {
      if (Flags.Check(FEDITOR_LOCKMODE))
      {
        _ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
        return(FALSE);
      }
      else
      {
        int Indent=Param!=NULL && *(int *)Param!=FALSE;
        if (!Indent)
          Pasting++;
        Flags.Set(FEDITOR_NEWUNDO);
        InsertString();
        Show();
        if (!Indent)
          Pasting--;
      }
      return(TRUE);
    }

    case ECTL_INSERTTEXT:
    {
      if(!Param)
        return FALSE;

      _ECTLLOG(SysLog(L"(const wchar_t *)Param='%s'",(const wchar_t *)Param));
      if (Flags.Check(FEDITOR_LOCKMODE))
      {
        _ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
        return(FALSE);
      }
      else
      {
        const wchar_t *Str=(const wchar_t *)Param;
        Pasting++;
        Lock ();

        while (*Str)
          ProcessKey(*(Str++));

        Unlock ();
        Pasting--;
      }
      return(TRUE);
    }

    case ECTL_SETSTRING:
    {

      struct EditorSetString *SetString=(struct EditorSetString *)Param;

      if(!SetString || IsBadReadPtr(SetString,sizeof(struct EditorSetString)))
        break;

      _ECTLLOG(SysLog(L"struct EditorSetString{"));
      _ECTLLOG(SysLog(L"  StringNumber    =%d",SetString->StringNumber));
      _ECTLLOG(SysLog(L"  StringText      ='%s'",SetString->StringText));
      _ECTLLOG(SysLog(L"  StringEOL       ='%s'",SetString->StringEOL?_SysLog_LinearDump((LPBYTE)SetString->StringEOL,StrLength(SetString->StringEOL)):L"(null)"));
      _ECTLLOG(SysLog(L"  StringLength    =%d",SetString->StringLength));
      _ECTLLOG(SysLog(L"}"));

      if (Flags.Check(FEDITOR_LOCKMODE))
      {
        _ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
        break;
      }
      else
      {
        /* $ 06.08.2002 IS
           ��������� ������������ StringLength � ������ FALSE, ���� ��� ������
           ����.
        */
        int Length=SetString->StringLength;
        if(Length < 0)
        {
          _ECTLLOG(SysLog(L"SetString->StringLength < 0"));
          return(FALSE);
        }

        Edit *CurPtr=GetStringByNumber(SetString->StringNumber);
        if (CurPtr==NULL)
        {
          _ECTLLOG(SysLog(L"GetStringByNumber(%d) return NULL",SetString->StringNumber));
          return(FALSE);
        }

        const wchar_t *EOL=SetString->StringEOL==NULL ? GlobalEOL:SetString->StringEOL;

        int LengthEOL=StrLength(EOL);
        wchar_t *NewStr=(wchar_t*)xf_malloc((Length+LengthEOL+1)*sizeof (wchar_t));
        if (NewStr==NULL)
        {
          _ECTLLOG(SysLog(L"xf_malloc(%d) return NULL",Length+LengthEOL+1));
          return(FALSE);
        }

        int DestLine=SetString->StringNumber;
        if (DestLine==-1)
          DestLine=NumLine;

        wmemcpy(NewStr,SetString->StringText,Length);
        wmemcpy(NewStr+Length,EOL,LengthEOL);
        AddUndoData(CurPtr->GetStringAddrW(),CurPtr->GetEOL(),DestLine,
                    CurPtr->GetCurPos(),UNDO_EDIT);

        int CurPos=CurPtr->GetCurPos();
        CurPtr->SetBinaryString(NewStr,Length+LengthEOL);
        CurPtr->SetCurPos(CurPos);
        TextChanged(1);    // 10.08.2000 skv - Modified->TextChanged
        xf_free(NewStr);
      }
      return(TRUE);
    }

    case ECTL_DELETESTRING:
    {
      if (Flags.Check(FEDITOR_LOCKMODE))
      {
        _ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
        return(FALSE);
      }
      DeleteString(CurLine,FALSE,NumLine);
      return(TRUE);
    }

    case ECTL_DELETECHAR:
    {
      if (Flags.Check(FEDITOR_LOCKMODE))
      {
        _ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
        return(FALSE);
      }
      Pasting++;
      ProcessKey(KEY_DEL);
      Pasting--;
      return(TRUE);
    }


    case ECTL_FREEINFO:
    {
      struct EditorInfo *Info=(struct EditorInfo *)Param;
      xf_free ((void*)Info->FileName);
      return(TRUE);
    }


    case ECTL_GETINFO:
    {
      struct EditorInfo *Info=(struct EditorInfo *)Param;
      if(Info && !IsBadWritePtr(Info,sizeof(struct EditorInfo)))
      {
        memset(Info,0,sizeof(*Info));
        Info->EditorID=Editor::EditorID;
        Info->FileName=xf_wcsdup (L""); //BUGBUG mem leak
        Info->WindowSizeX=ObjWidth;
        Info->WindowSizeY=Y2-Y1+1;
        Info->TotalLines=NumLastLine;
        Info->CurLine=NumLine;
        Info->CurPos=CurLine->GetCurPos();
        Info->CurTabPos=CurLine->GetTabCurPos();
        Info->TopScreenLine=NumLine-CalcDistance(TopScreen,CurLine,-1);
        Info->LeftPos=CurLine->GetLeftPos();
        Info->Overtype=Flags.Check(FEDITOR_OVERTYPE);
        Info->BlockType=BTYPE_NONE;
        if (BlockStart!=NULL)
          Info->BlockType=BTYPE_STREAM;
        if (VBlockStart!=NULL)
          Info->BlockType=BTYPE_COLUMN;
        Info->BlockStartLine=Info->BlockType==BTYPE_NONE ? 0:BlockStartLine;
        //Info->Options=0;
        if (EdOpt.ExpandTabs == EXPAND_ALLTABS)
          Info->Options|=EOPT_EXPANDALLTABS;
        if (EdOpt.ExpandTabs == EXPAND_NEWTABS)
          Info->Options|=EOPT_EXPANDONLYNEWTABS;
        if (EdOpt.PersistentBlocks)
          Info->Options|=EOPT_PERSISTENTBLOCKS;
        if (EdOpt.DelRemovesBlocks)
          Info->Options|=EOPT_DELREMOVESBLOCKS;
        if (EdOpt.AutoIndent)
          Info->Options|=EOPT_AUTOINDENT;
        if (EdOpt.SavePos)
          Info->Options|=EOPT_SAVEFILEPOSITION;
        if (EdOpt.AutoDetectTable)
          Info->Options|=EOPT_AUTODETECTTABLE;
        if (EdOpt.CursorBeyondEOL)
          Info->Options|=EOPT_CURSORBEYONDEOL;
        Info->TabSize=EdOpt.TabSize;
        Info->BookMarkCount=BOOKMARK_COUNT;
        Info->CurState=Flags.Check(FEDITOR_LOCKMODE)?ECSTATE_LOCKED:0;
        Info->CurState|=!Flags.Check(FEDITOR_MODIFIED)?ECSTATE_SAVED:0;
        Info->CurState|=Flags.Check(FEDITOR_MODIFIED|FEDITOR_WASCHANGED)?ECSTATE_MODIFIED:0;
        return TRUE;
      }
      _ECTLLOG(SysLog(L"Error: Param == NULL or IsBadWritePtr(Param,sizeof(struct EditorInfo))"));
      return FALSE;
    }

    case ECTL_SETPOSITION:
    {
      // "������� ���� �����..."
      if(Param && !IsBadReadPtr(Param,sizeof(struct EditorSetPosition)))
      {
        // ...� ��� ������ ���������� � ���, ��� ���������
        struct EditorSetPosition *Pos=(struct EditorSetPosition *)Param;
        _ECTLLOG(SysLog(L"struct EditorSetPosition{"));
        _ECTLLOG(SysLog(L"  CurLine       = %d",Pos->CurLine));
        _ECTLLOG(SysLog(L"  CurPos        = %d",Pos->CurPos));
        _ECTLLOG(SysLog(L"  CurTabPos     = %d",Pos->CurTabPos));
        _ECTLLOG(SysLog(L"  TopScreenLine = %d",Pos->TopScreenLine));
        _ECTLLOG(SysLog(L"  LeftPos       = %d",Pos->LeftPos));
        _ECTLLOG(SysLog(L"  Overtype      = %d",Pos->Overtype));
        _ECTLLOG(SysLog(L"}"));

        Lock ();

        int CurPos=CurLine->GetCurPos();

        // �������� ���� �� ��������� ��� (���� ����)
        if ((Pos->CurLine >= 0 || Pos->CurPos >= 0)&&
            (Pos->CurLine!=NumLine || Pos->CurPos!=CurPos))
          Flags.Set(FEDITOR_CURPOSCHANGEDBYPLUGIN);

        if (Pos->CurLine >= 0) // �������� ������
        {
          if (Pos->CurLine==NumLine-1)
            Up();
          else
            if (Pos->CurLine==NumLine+1)
              Down();
            else
              GoToLine(Pos->CurLine);
        }

        if (Pos->TopScreenLine >= 0 && Pos->TopScreenLine<=NumLine)
        {
          TopScreen=CurLine;
          for (int I=NumLine;I>0 && NumLine-I<Y2-Y1 && I!=Pos->TopScreenLine;I--)
            TopScreen=TopScreen->m_prev;
        }

        if (Pos->CurPos >= 0)
          CurLine->SetCurPos(Pos->CurPos);

        if (Pos->CurTabPos >= 0)
          CurLine->SetTabCurPos(Pos->CurTabPos);

        if (Pos->LeftPos >= 0)
          CurLine->SetLeftPos(Pos->LeftPos);

        /* $ 30.08.2001 IS
           ��������� ������ ����� ���������� �����, � ��������� ������ ��������
           �����, �.�. ��������������� ������, ��� ����� �������, � ����� ����
           ��������������, � ���������� ���� �������� �������������� ���������.
        */
        if (Pos->Overtype >= 0)
        {
          Flags.Change(FEDITOR_OVERTYPE,Pos->Overtype);
          CurLine->SetOvertypeMode(Flags.Check(FEDITOR_OVERTYPE));
        }

        Unlock ();
        return TRUE;
      }
      _ECTLLOG(SysLog(L"Error: Param == NULL or IsBadReadPtr(Param,sizeof(struct EditorSetPosition))"));
      break;
    }

    case ECTL_SELECT:
    {
      struct EditorSelect *Sel=(struct EditorSelect *)Param;

      if(Sel && !IsBadReadPtr(Sel,sizeof(struct EditorSelect)))
      {
        _ECTLLOG(SysLog(L"struct EditorSelect{"));
        _ECTLLOG(SysLog(L"  BlockType     =%s (%d)",(Sel->BlockType==BTYPE_NONE?"BTYPE_NONE":(Sel->BlockType==BTYPE_STREAM?"":(Sel->BlockType==BTYPE_COLUMN?"BTYPE_COLUMN":"BTYPE_?????"))),Sel->BlockType));
        _ECTLLOG(SysLog(L"  BlockStartLine=%d",Sel->BlockStartLine));
        _ECTLLOG(SysLog(L"  BlockStartPos =%d",Sel->BlockStartPos));
        _ECTLLOG(SysLog(L"  BlockWidth    =%d",Sel->BlockWidth));
        _ECTLLOG(SysLog(L"  BlockHeight   =%d",Sel->BlockHeight));
        _ECTLLOG(SysLog(L"}"));

        UnmarkBlock();
        if (Sel->BlockType==BTYPE_NONE)
          return(TRUE);

        Edit *CurPtr=GetStringByNumber(Sel->BlockStartLine);
        if (CurPtr==NULL)
        {
          _ECTLLOG(SysLog(L"GetStringByNumber(%d) return NULL",Sel->BlockStartLine));
          return(FALSE);
        }

        if (Sel->BlockType==BTYPE_STREAM)
        {
          BlockStart=CurPtr;
          if((BlockStartLine=Sel->BlockStartLine) == -1)
            BlockStartLine=NumLine;

          for (I=0;I<Sel->BlockHeight;I++)
          {
            int SelStart=(I==0) ? Sel->BlockStartPos:0;
            int SelEnd=(I<Sel->BlockHeight-1) ? -1:Sel->BlockStartPos+Sel->BlockWidth;
            CurPtr->Select(SelStart,SelEnd);
            CurPtr=CurPtr->m_next;
            if (CurPtr==NULL)
              return(FALSE);
          }
        }
        if (Sel->BlockType==BTYPE_COLUMN)
        {
          VBlockStart=CurPtr;
          if((BlockStartLine=Sel->BlockStartLine) == -1)
            BlockStartLine=NumLine;

          if (Sel->BlockWidth==-1)
            return(FALSE);

          VBlockX=Sel->BlockStartPos;
          if((VBlockY=Sel->BlockStartLine) == -1)
             VBlockY=NumLine;

          VBlockSizeX=Sel->BlockWidth;
          VBlockSizeY=Sel->BlockHeight;
        }
        return(TRUE);
      }
      break;
    }

    case ECTL_REDRAW:
    {
      Show();
      ScrBuf.Flush();
      return(TRUE);
    }

    case ECTL_TABTOREAL:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(struct EditorConvertPos)))
      {
        struct EditorConvertPos *ecp=(struct EditorConvertPos *)Param;
        Edit *CurPtr=GetStringByNumber(ecp->StringNumber);
        if (CurPtr==NULL)
        {
          _ECTLLOG(SysLog(L"GetStringByNumber(%d) return NULL",ecp->StringNumber));
          return(FALSE);
        }
        ecp->DestPos=CurPtr->TabPosToReal(ecp->SrcPos);
        _ECTLLOG(SysLog(L"struct EditorConvertPos{"));
        _ECTLLOG(SysLog(L"  StringNumber =%d",ecp->StringNumber));
        _ECTLLOG(SysLog(L"  SrcPos       =%d",ecp->SrcPos));
        _ECTLLOG(SysLog(L"  DestPos      =%d",ecp->DestPos));
        _ECTLLOG(SysLog(L"}"));
        return(TRUE);
      }
      break;
    }

    case ECTL_REALTOTAB:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(struct EditorConvertPos)))
      {
        struct EditorConvertPos *ecp=(struct EditorConvertPos *)Param;
        Edit *CurPtr=GetStringByNumber(ecp->StringNumber);
        if (CurPtr==NULL)
        {
          _ECTLLOG(SysLog(L"GetStringByNumber(%d) return NULL",ecp->StringNumber));
          return(FALSE);
        }
        ecp->DestPos=CurPtr->RealPosToTab(ecp->SrcPos);
        _ECTLLOG(SysLog(L"struct EditorConvertPos{"));
        _ECTLLOG(SysLog(L"  StringNumber =%d",ecp->StringNumber));
        _ECTLLOG(SysLog(L"  SrcPos       =%d",ecp->SrcPos));
        _ECTLLOG(SysLog(L"  DestPos      =%d",ecp->DestPos));
        _ECTLLOG(SysLog(L"}"));
        return(TRUE);
      }
      break;
    }

    case ECTL_EXPANDTABS:
    {
      if (Flags.Check(FEDITOR_LOCKMODE))
      {
        _ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
        return FALSE;
      }
      else
      {
        int StringNumber=*(int *)Param;
        Edit *CurPtr=GetStringByNumber(StringNumber);
        if (CurPtr==NULL)
        {
          _ECTLLOG(SysLog(L"GetStringByNumber(%d) return NULL",StringNumber));
          return FALSE;
        }
        AddUndoData(CurPtr->GetStringAddrW(),CurPtr->GetEOL(),StringNumber,
                    CurPtr->GetCurPos(),UNDO_EDIT);
        CurPtr->ReplaceTabs();
      }
      return TRUE;
    }

    // TODO: ���� DI_MEMOEDIT �� ����� ����� ��������, �� ������ ����������� � FileEditor::EditorControl(), � ������� - ����� ������
    case ECTL_ADDCOLOR:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(struct EditorColor)))
      {
        struct EditorColor *col=(struct EditorColor *)Param;
        _ECTLLOG(SysLog(L"struct EditorColor{"));
        _ECTLLOG(SysLog(L"  StringNumber=%d",col->StringNumber));
        _ECTLLOG(SysLog(L"  ColorItem   =%d (0x%08X)",col->ColorItem,col->ColorItem));
        _ECTLLOG(SysLog(L"  StartPos    =%d",col->StartPos));
        _ECTLLOG(SysLog(L"  EndPos      =%d",col->EndPos));
        _ECTLLOG(SysLog(L"  Color       =%d (0x%08X)",col->Color,col->Color));
        _ECTLLOG(SysLog(L"}"));

        struct ColorItem newcol;
        newcol.StartPos=col->StartPos+(col->StartPos!=-1?X1:0);
        newcol.EndPos=col->EndPos+X1;
        newcol.Color=col->Color;
        Edit *CurPtr=GetStringByNumber(col->StringNumber);
        if (CurPtr==NULL)
        {
          _ECTLLOG(SysLog(L"GetStringByNumber(%d) return NULL",col->StringNumber));
          return FALSE;
        }
        if (col->Color==0)
          return(CurPtr->DeleteColor(newcol.StartPos));
        CurPtr->AddColor(&newcol);
        return TRUE;
      }
      break;
    }

    // TODO: ���� DI_MEMOEDIT �� ����� ����� ��������, �� ������ ����������� � FileEditor::EditorControl(), � ������� - ����� ������
    case ECTL_GETCOLOR:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(struct EditorColor)))
      {
        struct EditorColor *col=(struct EditorColor *)Param;
        Edit *CurPtr=GetStringByNumber(col->StringNumber);
        if (!CurPtr || IsBadWritePtr(col,sizeof(struct EditorColor)))
        {
          _ECTLLOG(SysLog(L"GetStringByNumber(%d) return NULL or IsBadWritePtr(col,sizeof(struct EditorColor)",col->StringNumber));
          return FALSE;
        }
        struct ColorItem curcol;
        if (!CurPtr->GetColor(&curcol,col->ColorItem))
        {
          _ECTLLOG(SysLog(L"GetColor() return NULL"));
          return FALSE;
        }
        col->StartPos=curcol.StartPos-X1;
        col->EndPos=curcol.EndPos-X1;
        col->Color=curcol.Color;
        _ECTLLOG(SysLog(L"struct EditorColor{"));
        _ECTLLOG(SysLog(L"  StringNumber=%d",col->StringNumber));
        _ECTLLOG(SysLog(L"  ColorItem   =%d (0x%08X)",col->ColorItem,col->ColorItem));
        _ECTLLOG(SysLog(L"  StartPos    =%d",col->StartPos));
        _ECTLLOG(SysLog(L"  EndPos      =%d",col->EndPos));
        _ECTLLOG(SysLog(L"  Color       =%d (0x%08X)",col->Color,col->Color));
        _ECTLLOG(SysLog(L"}"));
        return TRUE;
      }
      break;
    }

    // ������ ����������� � FileEditor::EditorControl()
    case ECTL_PROCESSKEY:
    {
      _ECTLLOG(SysLog(L"Key = %s",_FARKEY_ToName((DWORD)Param)));
      ProcessKey((int)(INT_PTR)Param);
      return TRUE;
    }
    /* $ 16.02.2001 IS
         ��������� ��������� ���������� �������� ���������. Param ��������� ��
         ��������� EditorSetParameter
    */
    case ECTL_SETPARAM:
    {
      struct EditorSetParameter *espar=(struct EditorSetParameter *)Param;
      if(espar && !IsBadReadPtr(espar,sizeof(struct EditorSetParameter)))
      {
        int rc=TRUE;
        _ECTLLOG(SysLog(L"struct EditorSetParameter{"));
        _ECTLLOG(SysLog(L"  Type        =%s",_ESPT_ToName(espar->Type)));
        switch(espar->Type)
        {
          case ESPT_GETWORDDIV:
            _ECTLLOG(SysLog(L"  cParam      =(%p)",espar->Param.cParam));
            if(!IsBadWritePtr(espar->Param.cParam, EdOpt.strWordDiv.GetLength()+1))
              xwcsncpy(espar->Param.cParam,EdOpt.strWordDiv,EdOpt.strWordDiv.GetLength()); //BUGBUG
            else
              rc=FALSE;
            break;
          case ESPT_SETWORDDIV:
            _ECTLLOG(SysLog(L"  cParam      =[%s]",espar->Param.cParam));
            SetWordDiv((!espar->Param.cParam || !*espar->Param.cParam)?Opt.strWordDiv:espar->Param.cParam);
            break;
          case ESPT_TABSIZE:
            _ECTLLOG(SysLog(L"  iParam      =%d",espar->Param.iParam));
            SetTabSize(espar->Param.iParam);
            break;
          case ESPT_EXPANDTABS:
            _ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?"On":"Off"));
            SetConvertTabs(espar->Param.iParam);
            break;
          case ESPT_AUTOINDENT:
            _ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?"On":"Off"));
            SetAutoIndent(espar->Param.iParam);
            break;
          case ESPT_CURSORBEYONDEOL:
            _ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?"On":"Off"));
            SetCursorBeyondEOL(espar->Param.iParam);
            break;
          case ESPT_CHARCODEBASE:
            _ECTLLOG(SysLog(L"  iParam      =%s",(espar->Param.iParam==0?"0 (Oct)":(espar->Param.iParam==1?"1 (Dec)":(espar->Param.iParam==2?"2 (Hex)":"?????")))));
            SetCharCodeBase(espar->Param.iParam);
            break;
          /* $ 07.08.2001 IS ������� ��������� �� ������� */
          case ESPT_CHARTABLE:
          {
            //BUGBUG

            if (HostFileEditor) HostFileEditor->ChangeEditKeyBar();
            Show();
          }
          /* $ 29.10.2001 IS ��������� ��������� "��������� ������� �����" */
          case ESPT_SAVEFILEPOSITION:
            _ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?"On":"Off"));
            SetSavePosMode(espar->Param.iParam, -1);
            break;
          /* $ 23.03.2002 IS ���������/�������� ��������� ����� */
          case ESPT_LOCKMODE:
            _ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?"On":"Off"));
            Flags.Change(FEDITOR_LOCKMODE, espar->Param.iParam);
            break;
          default:
            _ECTLLOG(SysLog(L"}"));
            return FALSE;
        }
        _ECTLLOG(SysLog(L"}"));
        return rc;
      }
      return  FALSE;
    }

    // ������ ���� ��������� "�������������� ��������� �����"
    case ECTL_TURNOFFMARKINGBLOCK:
    {
      Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
      return TRUE;
    }

    case ECTL_DELETEBLOCK:
    {
      if (Flags.Check(FEDITOR_LOCKMODE) || !(VBlockStart || BlockStart))
      {
        _ECTLLOG(if(Flags.Check(FEDITOR_LOCKMODE))SysLog(L"FEDITOR_LOCKMODE!"));
        _ECTLLOG(if(!(VBlockStart || BlockStart))SysLog(L"Not selected block!"));
        return FALSE;
      }

      Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
      DeleteBlock();
      Show();
      return TRUE;
    }
  }
  return FALSE;
}

int Editor::SetBookmark(DWORD Pos)
{
  if(Pos < BOOKMARK_COUNT)
  {
    SavePos.Line[Pos]=NumLine;
    SavePos.Cursor[Pos]=CurLine->GetCurPos();
    SavePos.LeftPos[Pos]=CurLine->GetLeftPos();
    SavePos.ScreenLine[Pos]=CalcDistance(TopScreen,CurLine,-1);
    return TRUE;
  }
  return FALSE;
}

int Editor::GotoBookmark(DWORD Pos)
{
  if(Pos < BOOKMARK_COUNT)
  {
    if (SavePos.Line[Pos]!=0xffffffff)
    {
      GoToLine(SavePos.Line[Pos]);
      CurLine->SetCurPos(SavePos.Cursor[Pos]);
      CurLine->SetLeftPos(SavePos.LeftPos[Pos]);
      TopScreen=CurLine;
      for (DWORD I=0;I<SavePos.ScreenLine[Pos] && TopScreen->m_prev!=NULL;I++)
        TopScreen=TopScreen->m_prev;
      if (!EdOpt.PersistentBlocks)
        UnmarkBlock();
      Show();
    }
    return TRUE;
  }
  return FALSE;
}

Edit * Editor::GetStringByNumber(int DestLine)
{
  if (DestLine==NumLine || DestLine<0)
    return(CurLine);
  if (DestLine>NumLastLine)
    return(NULL);

  if (DestLine>NumLine)
  {
    Edit *CurPtr=CurLine;
    for (int Line=NumLine;Line<DestLine;Line++)
    {
      CurPtr=CurPtr->m_next;
      if (CurPtr==NULL)
        return(NULL);
    }
    return(CurPtr);
  }

  if (DestLine<NumLine && DestLine>NumLine/2)
  {
    Edit *CurPtr=CurLine;
    for (int Line=NumLine;Line>DestLine;Line--)
    {
      CurPtr=CurPtr->m_prev;
      if (CurPtr==NULL)
        return(NULL);
    }
    return(CurPtr);
  }

  {
    Edit *CurPtr=TopList;
    for (int Line=0;Line<DestLine;Line++)
    {
      CurPtr=CurPtr->m_next;
      if (CurPtr==NULL)
        return(NULL);
    }
    return(CurPtr);
  }
}

void Editor::SetReplaceMode(int Mode)
{
  ::ReplaceMode=Mode;
}

int Editor::GetLineCurPos()
{
  return CurLine->GetTabCurPos();
}

void Editor::BeginVBlockMarking()
{
    UnmarkBlock();
    VBlockStart=CurLine;
    VBlockX=CurLine->GetTabCurPos();
    VBlockSizeX=0;
    VBlockY=NumLine;
    VBlockSizeY=1;
    Flags.Set(FEDITOR_MARKINGVBLOCK);
    BlockStartLine=NumLine;
    //_D(SysLog(L"BeginVBlockMarking, set vblock to  VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
}

void Editor::AdjustVBlock(int PrevX)
{
    int x=GetLineCurPos();
    int c2;

    //_D(SysLog(L"AdjustVBlock, x=%i,   vblock is VBlockY=%i:%i, VBlockX=%i:%i, PrevX=%i",x,VBlockY,VBlockSizeY,VBlockX,VBlockSizeX,PrevX));
    if ( x==VBlockX+VBlockSizeX)  // ������ �� ���������, ������� ��������� ���
        return;
    if ( x>VBlockX )  // ������ ������ ������ �����
    {
        VBlockSizeX=x-VBlockX;
        //_D(SysLog(L"x>VBlockX");
    }
    else if ( x<VBlockX ) // ������ ������ �� ������ �����
    {
        c2=VBlockX;
        if ( PrevX>VBlockX )    // ���������� ������, � ������ �����
        {
            VBlockX=x;
            VBlockSizeX=c2-x;   // ������ ����
        }
        else      // ���������� ����� � ������ ��� ������ �����
        {
            VBlockX=x;
            VBlockSizeX+=c2-x;  // ��������� ����
        }
        //_D(SysLog(L"x<VBlockX"));
    }
    else if (x==VBlockX && x!=PrevX)
    {
        VBlockSizeX=0;  // ������ � 0, ������ �������� ���� �� ���������
        //_D(SysLog(L"x==VBlockX && x!=PrevX"));
    }
    // ����������
    //   ������ x>VBLockX+VBlockSizeX �� ����� ����
    //   ������ ��� ������ ������� ����� �� ���������, �� �� ������

    //_D(SysLog(L"AdjustVBlock, changed vblock  VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
}


void Editor::Xlat()
{
/*  Edit *CurPtr;
  int Line;
  BOOL DoXlat=FALSE;

  if (VBlockStart!=NULL)
  {
    CurPtr=VBlockStart;

    for (Line=0;CurPtr!=NULL && Line<VBlockSizeY;Line++,CurPtr=CurPtr->m_next)
    {
      int TBlockX=CurPtr->TabPosToReal(VBlockX);
      int TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
                      CurPtr->TabPosToReal(VBlockX);
      const wchar_t *CurStr,*EndSeq;
      int Length;
      CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
      int CopySize=Length-TBlockX;
      if (CopySize>TBlockSizeX)
         CopySize=TBlockSizeX;
      AddUndoData(CurPtr->GetStringAddrW(),CurPtr->GetEOL(),BlockStartLine+Line,0,UNDO_EDIT);
      BlockUndo=TRUE;
      ::Xlat(CurPtr->Str,TBlockX,TBlockX+CopySize,CurPtr->TableSet,Opt.XLat.Flags);
    }
    DoXlat=TRUE;
  }
  else
  {
    Line=0;
    CurPtr=BlockStart;
    // $ 25.11.2000 IS
    //     ���� ��� ���������, �� ���������� ������� �����. ����� ������������ ��
    //     ������ ����������� ������ ������������.
    if(CurPtr!=NULL)
    {
      while (CurPtr!=NULL)
      {
        int StartSel,EndSel;
        CurPtr->GetSelection(StartSel,EndSel);
        if (StartSel==-1)
          break;
        if(EndSel == -1)
          EndSel=StrLength(CurPtr->Str);
        AddUndoData(CurPtr->GetStringAddrW(),CurPtr->GetEOL(),BlockStartLine+Line,0,UNDO_EDIT);
        ::Xlat(CurPtr->Str,StartSel,EndSel,CurPtr->TableSet,Opt.XLat.Flags);
        BlockUndo=TRUE;
        Line++;
        CurPtr=CurPtr->m_next;
      }
      DoXlat=TRUE;
    }
    else
    {
      wchar_t *Str=CurLine->Str;
      int start=CurLine->GetCurPos(), end, StrSize=StrLength(Str);
      // $ 10.12.2000 IS
      //   ������������ ������ �� �����, �� ������� ����� ������, ��� �� �����,
      //   ��� ��������� ����� ������� ������� �� 1 ������
      DoXlat=TRUE;

      if(IsWordDiv((AnsiText || UseDecodeTable)?&TableSet:NULL,Opt.XLat.strWordDivForXlat,Str[start]))
      {
         if(start) start--;
         DoXlat=(!IsWordDiv((AnsiText || UseDecodeTable)?&TableSet:NULL,Opt.XLat.strWordDivForXlat,Str[start]));
      }

      if(DoXlat)
      {
        while(start>=0 && !IsWordDiv((AnsiText || UseDecodeTable)?&TableSet:NULL,Opt.XLat.strWordDivForXlat,Str[start]))
          start--;
        start++;
        end=start+1;
        while(end<StrSize && !IsWordDiv((AnsiText || UseDecodeTable)?&TableSet:NULL,Opt.XLat.strWordDivForXlat,Str[end]))
          end++;
        AddUndoData(CurLine->GetStringAddrW(),CurLine->GetEOL(),NumLine,start,UNDO_EDIT);
        ::Xlat(Str,start,end,CurLine->TableSet,Opt.XLat.Flags);
      }
    }
  }
  BlockUndo=FALSE;
  if(DoXlat)
    TextChanged(1);*/
}
/* SVS $ */

/* $ 15.02.2001 IS
     ����������� � ���������� �� ������ ����� ������������ �����.
     ����� ���� ���������� �� ������� ���������, �� ��� ��, imho,
     ������ �� ��������.
*/
//������� ������ ���������
void Editor::SetTabSize(int NewSize)
{
  if (NewSize<1 || NewSize>512)
    NewSize=8;
  if(NewSize!=EdOpt.TabSize) /* ������ ������ ��������� ������ � ��� ������, ���� ��
                          �� ����� ���� ��������� */
  {
    EdOpt.TabSize=NewSize;
    Edit *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->SetTabSize(NewSize);
      CurPtr=CurPtr->m_next;
    }
  }
}

// ������� ����� ������� ������ ���������
// �������� ����������, ������, �.�. ������� �� ��������� ������� �� ���������
void Editor::SetConvertTabs(int NewMode)
{
  if(NewMode!=EdOpt.ExpandTabs) /* ������ ����� ������ � ��� ������, ���� ��
                              �� ����� ���� ��������� */
  {
    EdOpt.ExpandTabs=NewMode;
    Edit *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->SetConvertTabs(NewMode);

      if ( NewMode == EXPAND_ALLTABS )
        CurPtr->ReplaceTabs();

      CurPtr=CurPtr->m_next;
    }
  }
}

void Editor::SetDelRemovesBlocks(int NewMode)
{
  if(NewMode!=EdOpt.DelRemovesBlocks)
  {
    EdOpt.DelRemovesBlocks=NewMode;
    Edit *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->SetDelRemovesBlocks(NewMode);
      CurPtr=CurPtr->m_next;
    }
  }
}

void Editor::SetPersistentBlocks(int NewMode)
{
  if(NewMode!=EdOpt.PersistentBlocks)
  {
    EdOpt.PersistentBlocks=NewMode;
    Edit *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->SetPersistentBlocks(NewMode);
      CurPtr=CurPtr->m_next;
    }
  }
}

//     "������ �� ��������� ������"
void Editor::SetCursorBeyondEOL(int NewMode)
{
  if(NewMode!=EdOpt.CursorBeyondEOL)
  {
    EdOpt.CursorBeyondEOL=NewMode;
    Edit *CurPtr=TopList;
    while (CurPtr!=NULL)
    {
      CurPtr->SetEditBeyondEnd(NewMode);
      CurPtr=CurPtr->m_next;
    }
  }
  /* $ 16.10.2001 SKV
    ���� ������������� ���� ���� ���� �����,
    �� ��-�� ���� ����� ��������� ������� �����
    ��� ��������� ������������ ������.
  */
  if(EdOpt.CursorBeyondEOL)
  {
    MaxRightPos=0;
  }
}

void Editor::GetSavePosMode(int &SavePos, int &SaveShortPos)
{
   SavePos=EdOpt.SavePos;
   SaveShortPos=EdOpt.SaveShortPos;
}

// ����������� � �������� �������� ��������� "-1" ��� ���������,
// ������� �� ����� ������
void Editor::SetSavePosMode(int SavePos, int SaveShortPos)
{
   if(SavePos!=-1)
      EdOpt.SavePos=SavePos;
   if(SaveShortPos!=-1)
      EdOpt.SaveShortPos=SaveShortPos;
}

void Editor::EditorShowMsg(const wchar_t *Title,const wchar_t *Msg, const wchar_t* Name)
{
  Message(0,0,Title,Msg,Name);
  PreRedrawParam.Param1=(void *)Title;
  PreRedrawParam.Param2=(void *)Msg;
  PreRedrawParam.Param3=(void *)Name;
}

void Editor::PR_EditorShowMsg(void)
{
  Editor::EditorShowMsg((wchar_t*)PreRedrawParam.Param1,(wchar_t*)PreRedrawParam.Param2,(wchar_t*)PreRedrawParam.Param3);
}


Edit *Editor::CreateString (const wchar_t *lpwszStr, int nLength)
{
	Edit *pEdit = new Edit (this);

	if ( pEdit )
	{
		pEdit->m_next = NULL;
		pEdit->m_prev = NULL;
		pEdit->SetTabSize (EdOpt.TabSize);
		pEdit->SetPersistentBlocks (EdOpt.PersistentBlocks);
		pEdit->SetConvertTabs (EdOpt.ExpandTabs);
		pEdit->SetCodePage (m_codepage);
		if ( lpwszStr )
			pEdit->SetBinaryString(lpwszStr, nLength);
		pEdit->SetCurPos (0);
		pEdit->SetObjectColor (COL_EDITORTEXT,COL_EDITORSELECTEDTEXT);
		pEdit->SetEditorMode (TRUE);
		pEdit->SetWordDiv (EdOpt.strWordDiv);
	}

	return pEdit;
}

/*bool Editor::AddString (const wchar_t *lpwszStr, int nLength)
{
	Edit *pNewEdit = CreateString (lpwszStr, nLength);

	if ( !pNewEdit )
		return false;

	if ( !TopList || !NumLastLine ) //???
		TopList = EndList = TopScreen = CurLine = pNewEdit;
	else
	{
		Edit *PrevPtr;

		EndList->m_next = pNewEdit;

		PrevPtr = EndList;
		EndList = EndList->m_next;
		EndList->m_prev = PrevPtr;
		EndList->m_next = NULL;
	}

	NumLastLine++;

	return true;
}*/

Edit *Editor::InsertString (const wchar_t *lpwszStr, int nLength, Edit *pAfter)
{
	Edit *pNewEdit = CreateString (lpwszStr, nLength);

	if ( pNewEdit )
	{
		if ( !TopList || !NumLastLine ) //???
			TopList = EndList = TopScreen = CurLine = pNewEdit;
		else
		{
			Edit *pWork = pAfter?pAfter:EndList;

		   	Edit *pNext = pWork->m_next;

	   		pNewEdit->m_next = pNext;
		   	pNewEdit->m_prev = pWork;

   			pWork->m_next = pNewEdit;

	   		if ( pNext )
   				pNext->m_prev = pNewEdit;

		    if ( !pAfter )
    			EndList = pNewEdit;
		}

		NumLastLine++;
	}

	return pNewEdit;
}


void Editor::SetCacheParams (EditorCacheParams *pp)
{
	memcpy (&SavePos, &pp->SavePos, sizeof (InternalEditorBookMark));

	//m_codepage = pp->Table; //BUGBUG!!!, LoadFile do it itself

	if ( pp->ScreenLine > ObjHeight)//ScrY //BUGBUG
		pp->ScreenLine=ObjHeight;//ScrY;

	if ( pp->Line >= pp->ScreenLine)
	{
		Lock ();
		GoToLine (pp->Line-pp->ScreenLine);
		TopScreen = CurLine;

		for (int I=0; I < pp->ScreenLine; I++)
			ProcessKey(KEY_DOWN);

		CurLine->SetTabCurPos(pp->LinePos);
		CurLine->SetLeftPos(pp->LeftPos);

		Unlock ();
	}
}

void Editor::GetCacheParams (EditorCacheParams *pp)
{
	memset (pp, 0, sizeof (EditorCacheParams));

	pp->Line = NumLine;
	pp->ScreenLine = CalcDistance(TopScreen, CurLine,-1);
	pp->LinePos = CurLine->GetTabCurPos();
	pp->LeftPos = CurLine->GetLeftPos();

	pp->Table = m_codepage; //BUGBUG

	if( Opt.EdOpt.SaveShortPos )
		memcpy (&pp->SavePos, &SavePos, sizeof (InternalEditorBookMark));
}


bool Editor::SetCodePage (int codepage)
{
	if ( m_codepage != codepage )
    {
		m_codepage = codepage;

		Edit *current = TopList;

		while ( current )
       	{
			current->SetCodePage (m_codepage);
			current = current->m_next;
		}

		Show ();

		return true;
	}

	return false;
}

int Editor::GetCodePage ()
{
	return m_codepage;
}


void Editor::SetDialogParent(DWORD Sets)
{
}

void Editor::SetOvertypeMode(int Mode)
{
}

int Editor::GetOvertypeMode()
{
	return 0;
}

void Editor::SetEditBeyondEnd(int Mode)
{
}

void Editor::SetClearFlag(int Flag)
{
}

int Editor::GetClearFlag(void)
{
	return 0;
}

int Editor::GetCurCol()
{
	return CurLine->GetCurPos();
}

void Editor::SetCurPos(int NewCol, int NewRow)
{
	Lock ();
	GoToLine(NewRow);
	CurLine->SetTabCurPos(NewCol);
	//CurLine->SetLeftPos(LeftPos); ???
	Unlock ();
}

void Editor::SetCursorType(int Visible,int Size)
{
	CurLine->SetCursorType(Visible,Size); //???
}

void Editor::GetCursorType(int &Visible,int &Size)
{
	CurLine->GetCursorType(Visible,Size); //???
}

void Editor::SetObjectColor(int Color,int SelColor,int ColorUnChanged)
{
	for (Edit *CurPtr=TopList;CurPtr!=NULL;CurPtr=CurPtr->m_next) //???
		CurPtr->SetObjectColor (Color,SelColor,ColorUnChanged);
}
