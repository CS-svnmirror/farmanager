/*
flupdate.cpp

�������� ������ - ������ ���� ������
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

#include "filelist.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "plugin.hpp"
#include "colors.hpp"
#include "lang.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "filefilter.hpp"
#include "hilight.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"

int _cdecl SortSearchList(const void *el1,const void *el2);

void FileList::Update(int Mode)
{
  _ALGO(CleverSysLog clv(L"FileList::Update"));
  _ALGO(SysLog(L"(Mode=[%d/0x%08X] %s)",Mode,Mode,(Mode==UPDATE_KEEP_SELECTION?"UPDATE_KEEP_SELECTION":"")));

  if (EnableUpdate)
    switch(PanelMode)
    {
      case NORMAL_PANEL:
        ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
        break;
      case PLUGIN_PANEL:
        {
          struct OpenPluginInfo Info;
          CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
          ProcessPluginCommand();
          if (PanelMode!=PLUGIN_PANEL)
            ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
          else
            if ((Info.Flags & OPIF_REALNAMES) ||
                CtrlObject->Cp()->GetAnotherPanel(this)->GetMode()==PLUGIN_PANEL ||
                (Mode & UPDATE_SECONDARY)==0)
              UpdatePlugin(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE);
        }
        ProcessPluginCommand();
        break;
    }
  LastUpdateTime=clock();
}

void FileList::UpdateIfRequired()
{
  if (UpdateRequired)
  {
    UpdateRequired = FALSE;
    Update (UpdateRequiredMode | UPDATE_IGNORE_VISIBLE);
  }
}

void ReadFileNamesMsg(const wchar_t *Msg)
{
  Message(0,0,UMSG(MReadingTitleFiles),Msg);
  PreRedrawParam.Param1=(void*)Msg;
}

static void PR_ReadFileNamesMsg(void)
{
  ReadFileNamesMsg((wchar_t *)PreRedrawParam.Param1);
}


// ��� ���� ����� ����� ��� ���������� ������������� Far Manager
// ��� ���������� �����������

void FileList::ReadFileNames(int KeepSelection, int IgnoreVisible, int DrawMessage)
{
  if (!IsVisible() && !IgnoreVisible)
  {
    UpdateRequired=TRUE;
    UpdateRequiredMode=KeepSelection;
    return;
  }

  Is_FS_NTFS=FALSE;
  UpdateRequired=FALSE;
  AccessTimeUpdateRequired=FALSE;
  DizRead=FALSE;
  HANDLE FindHandle;
  FAR_FIND_DATA_EX fdata;
  struct FileListItem *CurPtr=0,**OldData=0;
  string strCurName, strNextCurName;
  long OldFileCount=0;
  int Done;
  int I;

  clock_t StartTime=clock();

  CloseChangeNotification();

  if (this!=CtrlObject->Cp()->LeftPanel && this!=CtrlObject->Cp()->RightPanel )
    return;

  string strSaveDir;
  FarGetCurDir(strSaveDir);
  {
    string strOldCurDir = strCurDir;
    if (!SetCurPath())
    {
      FlushInputBuffer(); // ������� ������ �����, �.�. �� ��� ����� ���� � ������ �����...
      if (StrCmp(strCurDir, strOldCurDir) == 0) //?? i??
      {
        GetPathRootOne(strOldCurDir,strOldCurDir);
        if(!IsDiskInDrive(strOldCurDir))
          IfGoHome(strOldCurDir.At(0));
        /* ��� ����� �������� ���� �� ��������� */
      }
      return;
    }
  }

  SortGroupsRead=FALSE;

  if (Filter==NULL)
    Filter=new FileFilter(this,FFT_PANEL);

  if (GetFocus())
    CtrlObject->CmdLine->SetCurDir(strCurDir);

  {
    string strFileSysName;
    string strRootDir;

    ConvertNameToFull(strCurDir,strRootDir);
    GetPathRoot(strRootDir, strRootDir);
    if ( apiGetVolumeInformation (strRootDir,NULL,NULL,NULL,NULL,&strFileSysName))
      Is_FS_NTFS=!StrCmpI(strFileSysName,L"NTFS")?TRUE:FALSE;
  }

  LastCurFile=-1;

  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  AnotherPanel->QViewDelTempName();

  int PrevSelFileCount=SelFileCount;

  SelFileCount=0;
  SelFileSize=0;
  TotalFileCount=0;
  TotalFileSize=0;

  if (Opt.ShowPanelFree)
  {
    unsigned __int64 TotalSize,TotalFree;
    string strDriveRoot;
    GetPathRoot(strCurDir,strDriveRoot);
    if (!GetDiskSize(strDriveRoot,&TotalSize,&TotalFree,&FreeDiskSize))
      FreeDiskSize=0;
  }

  if (FileCount>0)
  {
    strCurName = ListData[CurFile]->strName;
    if (ListData[CurFile]->Selected)
    {
      for (I=CurFile+1; I < FileCount; I++)
      {
          CurPtr = ListData[I];

        if (!CurPtr->Selected)
        {
          strNextCurName = CurPtr->strName;
          break;
        }
      }
    }
  }
  if (KeepSelection || PrevSelFileCount>0)
  {
    OldData=ListData;
    OldFileCount=FileCount;
  }
  else
    DeleteListData(ListData,FileCount);

  ListData=NULL;

  int DotsPresent=0;

  int ReadOwners=IsColumnDisplayed(OWNER_COLUMN);
  int ReadPacked=IsColumnDisplayed(PACKED_COLUMN);
  int ReadNumLinks=IsColumnDisplayed(NUMLINK_COLUMN);

  string strComputerName;

  if (ReadOwners)
  {
    CurPath2ComputerName(strCurDir, strComputerName);
    // ������� ��� SID`��
    SIDCacheFlush();
  }

  SetLastError(0);

  //BUGBUG!!!
  Done=((FindHandle=apiFindFirstFile(L"*.*",&fdata))==INVALID_HANDLE_VALUE);

  int AllocatedCount=0;
  struct FileListItem *NewPtr;

  // ���������� ��������� ��� �����
  wchar_t Title[2048];
  int TitleLength=Min((int)X2-X1-1,(int)(sizeof(Title)/sizeof(wchar_t))-1);
  //wmemset(Title,0x0CD,TitleLength); //BUGBUG
  //Title[TitleLength]=0;
  MakeSeparator (TitleLength, Title, 9, NULL);
  BOOL IsShowTitle=FALSE;
  BOOL NeedHighlight=Opt.Highlight && PanelMode != PLUGIN_PANEL;

  for (FileCount=0; !Done; )
  {
    if ((fdata.strFileName.At(0) != L'.' || fdata.strFileName.At(1) != 0) &&
        (Opt.ShowHidden || (fdata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))==0) &&
        (//(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
        Filter->FileInFilter(&fdata)))
    {
      int UpperDir=FALSE;
      if (fdata.strFileName.At(0) == L'.' && fdata.strFileName.At(1) == L'.' && fdata.strFileName.At(2) == 0)
      {
        UpperDir=TRUE;
        DotsPresent=TRUE;
        if (IsLocalRootPath(strCurDir))
        {
          Done=!apiFindNextFile (FindHandle,&fdata);
          continue;
        }
      }
      if (FileCount>=AllocatedCount)
      {
        AllocatedCount=AllocatedCount+256+AllocatedCount/4;

        FileListItem **pTemp;

        if ((pTemp=(struct FileListItem **)xf_realloc(ListData,AllocatedCount*sizeof(*ListData)))==NULL)
          break;
        ListData=pTemp;
      }

      ListData[FileCount] = new FileListItem;
      ListData[FileCount]->Clear ();

      NewPtr=ListData[FileCount];

      NewPtr->FileAttr = fdata.dwFileAttributes;
      NewPtr->CreationTime = fdata.ftCreationTime;
      NewPtr->AccessTime = fdata.ftLastAccessTime;
      NewPtr->WriteTime = fdata.ftLastWriteTime;

      NewPtr->UnpSize = fdata.nFileSize;

      NewPtr->strName = fdata.strFileName;
      NewPtr->strShortName = fdata.strAlternateFileName;

      NewPtr->Position=FileCount++;
      NewPtr->NumberOfLinks=1;

      if(fdata.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
      {
        NewPtr->ReparseTag=fdata.dwReserved0; //MSDN
      }

      if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      {
        TotalFileSize += NewPtr->UnpSize;
        int Compressed=FALSE;
        if (ReadPacked && ((fdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) || (fdata.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)))
        {
          DWORD dwLoPart, dwHighPart;

          dwLoPart = GetCompressedFileSizeW (fdata.strFileName, &dwHighPart);

          if ( (dwLoPart != INVALID_FILE_SIZE) || (GetLastError () != NO_ERROR) )
          {
            NewPtr->PackSize = dwHighPart*_ui64(0x100000000)+dwLoPart;
            Compressed=TRUE;
          }
        }
        if (!Compressed)
          NewPtr->PackSize = fdata.nFileSize;
        if (ReadNumLinks)
          NewPtr->NumberOfLinks=GetNumberOfLinks(fdata.strFileName);
      }
      else
        NewPtr->PackSize = 0;

      NewPtr->SortGroup=DEFAULT_SORT_GROUP;
      if (ReadOwners)
      {
        string strOwner;
        GetFileOwner(strComputerName, NewPtr->strName,strOwner);
        NewPtr->strOwner = strOwner;
      }

      if (NeedHighlight)
        CtrlObject->HiFiles->GetHiColor(&NewPtr,1);

      if (!UpperDir && (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
        TotalFileCount++;

      //memcpy(ListData+FileCount,&NewPtr,sizeof(NewPtr));
//      FileCount++;

      if ((FileCount & 0x3f)==0 && clock()-StartTime>1000)
      {

        if (IsVisible())
        {
          string strReadMsg;
          if(!IsShowTitle)
          {
            if(DrawMessage)
              SetPreRedrawFunc(PR_ReadFileNamesMsg);
            else
            {
              Text(X1+1,Y1,COL_PANELBOX,Title);
              IsShowTitle=TRUE;
              SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
            }
          }

          strReadMsg.Format (UMSG(MReadingFiles),FileCount);
          if(DrawMessage)
            ReadFileNamesMsg(strReadMsg);
          else
          {
            TruncStr(strReadMsg,TitleLength-2);
            int MsgLength=(int)strReadMsg.GetLength();
            GotoXY(X1+1+(TitleLength-MsgLength-1)/2,Y1);
            mprintf(L" %s ", (const wchar_t*)strReadMsg);
          }
        }
        if (CheckForEsc())
        {
          Message(MSG_WARNING,1,UMSG(MUserBreakTitle),UMSG(MOperationNotCompleted),UMSG(MOk));
          break;
        }
      }
    }
    Done=!apiFindNextFile(FindHandle,&fdata);
  }

  SetPreRedrawFunc(NULL);

  int ErrCode=GetLastError();
  if (!(ErrCode==ERROR_SUCCESS || ErrCode==ERROR_NO_MORE_FILES || ErrCode==ERROR_FILE_NOT_FOUND ||
        (ErrCode==ERROR_BAD_PATHNAME && WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT && Opt.IgnoreErrorBadPathName)))
    Message(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MReadFolderError),UMSG(MOk));

  apiFindClose(FindHandle);

  // "����������" ������� � ��������� ���� - �� ��������� ������� �����������
  // ������ ������� �������, � ��������� �����.
//  UpdateColorItems();

  if (IsColumnDisplayed(DIZ_COLUMN))
    ReadDiz();

  /*
  int NetRoot=FALSE;
  if (strCurDir.At(0)==L'\\' && strCurDir.At(1)==L'\\')
  {
    const wchar_t *ChPtr=wcschr((const wchar_t*)strCurDir+2,'\\');
    if (ChPtr==NULL || wcschr(ChPtr+1,L'\\')==NULL)
      NetRoot=TRUE;
  }
  */

  // ���� ����� �����������, �������� �� ���� � �� ����������.
  if (!DotsPresent && !IsLocalRootPath(strCurDir) )// && !NetRoot)
  {
    if (FileCount>=AllocatedCount)
    {
        FileListItem **pTemp;
      if ((pTemp=(struct FileListItem **)xf_realloc(ListData,(FileCount+1)*sizeof(*ListData)))!=NULL)
        ListData=pTemp;
    }
    if (ListData!=NULL)
    {
      ListData[FileCount] = new FileListItem;
      AddParentPoint(ListData[FileCount],FileCount);
      if (NeedHighlight)
        CtrlObject->HiFiles->GetHiColor(&ListData[FileCount],1);
      FileCount++;
    }
  }

  if (AnotherPanel->GetMode()==PLUGIN_PANEL)
  {
    HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
    PluginPanelItem *PanelData=NULL, *PtrPanelData;
    string strPath;
    int PanelCount=0;

    strPath = strCurDir;
    AddEndSlash(strPath);
    if (CtrlObject->Plugins.GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,strPath))
    {
      FileListItem **pTemp;
      if ((pTemp=(struct FileListItem **)xf_realloc(ListData,(FileCount+PanelCount)*sizeof(*ListData)))!=NULL)
      {
        ListData=pTemp;
        for (PtrPanelData=PanelData, I=0; I < PanelCount; I++, CurPtr++, PtrPanelData++)
        {
          CurPtr = ListData[FileCount+I];
          FAR_FIND_DATA &fdata=PtrPanelData->FindData;
          PluginToFileListItem(PtrPanelData,CurPtr);
          CurPtr->Position=FileCount;
          TotalFileSize += fdata.nFileSize;
          CurPtr->PrevSelected=CurPtr->Selected=0;
          CurPtr->ShowFolderSize=0;
          CurPtr->SortGroup=CtrlObject->HiFiles->GetGroup(&fdata);
          if (!TestParentFolderName(fdata.lpwszFileName) && (CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
            TotalFileCount++;
        }
        // �������� ������ ��������� � ����� �����, �� ���� ���
        CtrlObject->HiFiles->GetHiColor(&ListData[FileCount],PanelCount);
        FileCount+=PanelCount;
      }
      CtrlObject->Plugins.FreeVirtualFindData(hAnotherPlugin,PanelData,PanelCount);
    }
  }

  CreateChangeNotification(FALSE);

  CorrectPosition();

  if (KeepSelection || PrevSelFileCount>0)
  {
    MoveSelection(ListData,FileCount,OldData,OldFileCount);
    DeleteListData(OldData,OldFileCount);
  }

  if (SortGroups)
    ReadSortGroups();

  if (!KeepSelection && PrevSelFileCount>0)
  {
    SaveSelection();
    ClearSelection();
  }

  SortFileList(FALSE);

  if (CurFile>=FileCount || StrCmpI(ListData[CurFile]->strName,strCurName)!=0)
    if (!GoToFile(strCurName) && !strNextCurName.IsEmpty())
      GoToFile(strNextCurName);

  /* $ 13.02.2002 DJ
     SetTitle() - ������ ���� �� ������� �����!
  */
  if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
    SetTitle();

  FarChDir(strSaveDir); //???
}

/*$ 22.06.2001 SKV
  �������� �������� ��� ������ ����� ���������� �������.
*/
int FileList::UpdateIfChanged(int UpdateMode)
{
  //_SVS(SysLog(L"CurDir='%s' Opt.AutoUpdateLimit=%d <= FileCount=%d",CurDir,Opt.AutoUpdateLimit,FileCount));
  if(!Opt.AutoUpdateLimit || static_cast<DWORD>(FileCount) <= Opt.AutoUpdateLimit)
  {
    /* $ 19.12.2001 VVM
      ! ������ ����������. ��� Force ���������� ������! */
    if ((IsVisible() && (clock()-LastUpdateTime>2000)) || (UpdateMode != UIC_UPDATE_NORMAL))
    {
      if(UpdateMode == UIC_UPDATE_NORMAL)
        ProcessPluginEvent(FE_IDLE,NULL);
      /* $ 24.12.2002 VVM
        ! �������� ������ ���������� �������. */
      if(// ���������� ������, �� ��� ����������� ����������� � ���� ������
         (PanelMode==NORMAL_PANEL && hListChange!=INVALID_HANDLE_VALUE && WaitForSingleObject(hListChange,0)==WAIT_OBJECT_0) ||
         // ��� ���������� ������, �� ��� ����������� � �� ��������� �������� ����� UPDATE_FORCE
         (PanelMode==NORMAL_PANEL && hListChange==INVALID_HANDLE_VALUE && UpdateMode==UIC_UPDATE_FORCE) ||
         // ��� ��������� ������ � ��������� ����� UPDATE_FORCE
         (PanelMode!=NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE)
        )
        {
          Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
          // � ���� ������ - ������ ����������
//          UpdateColorItems();
          if (AnotherPanel->GetType()==INFO_PANEL)
          {
            AnotherPanel->Update(UPDATE_KEEP_SELECTION);
            if (UpdateMode==UIC_UPDATE_NORMAL)
              AnotherPanel->Redraw();
          }
          Update(UPDATE_KEEP_SELECTION);
          if (UpdateMode==UIC_UPDATE_NORMAL)
            Show();
          return(TRUE);
        }
    }
  }
  return(FALSE);
}

void FileList::UpdateColorItems(void)
{
  if (Opt.Highlight && PanelMode != PLUGIN_PANEL)
    CtrlObject->HiFiles->GetHiColor(ListData,FileCount);
}


void FileList::CreateChangeNotification(int CheckTree)
{
  wchar_t RootDir[4]=L" :\\";
  DWORD DriveType=DRIVE_REMOTE;

  CloseChangeNotification();

  if(IsLocalPath(strCurDir))
  {
    RootDir[0]=strCurDir.At(0);
    DriveType=FAR_GetDriveType(RootDir);
  }

  if(Opt.AutoUpdateRemoteDrive || (!Opt.AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE))
  {
    hListChange=FindFirstChangeNotificationW(strCurDir,CheckTree,
                        FILE_NOTIFY_CHANGE_FILE_NAME|
                        FILE_NOTIFY_CHANGE_DIR_NAME|
                        FILE_NOTIFY_CHANGE_ATTRIBUTES|
                        FILE_NOTIFY_CHANGE_SIZE|
                        FILE_NOTIFY_CHANGE_LAST_WRITE);
  }
}


void FileList::CloseChangeNotification()
{
  if (hListChange!=INVALID_HANDLE_VALUE)
  {
    FindCloseChangeNotification(hListChange);
    hListChange=INVALID_HANDLE_VALUE;
  }
}


void FileList::MoveSelection(struct FileListItem **ListData,long FileCount,
                             struct FileListItem **OldData,long OldFileCount)
{
  struct FileListItem **OldPtr;
  SelFileCount=0;
  SelFileSize=0;
  far_qsort((void *)OldData,OldFileCount,sizeof(*OldData),SortSearchList);
  while (FileCount--)
  {
    OldPtr=(struct FileListItem **)bsearch((void *)ListData,(void *)OldData,
                                  OldFileCount,sizeof(*ListData),SortSearchList);
    if (OldPtr!=NULL)
    {
      if (OldPtr[0]->ShowFolderSize)
      {
        ListData[0]->ShowFolderSize=2;
        ListData[0]->UnpSize=OldPtr[0]->UnpSize;
        ListData[0]->PackSize=OldPtr[0]->PackSize;
      }
      Select(ListData[0],OldPtr[0]->Selected);
      ListData[0]->PrevSelected=OldPtr[0]->PrevSelected;
    }
    ListData++;
  }
}

void FileList::UpdatePlugin(int KeepSelection, int IgnoreVisible)
{
  _ALGO(CleverSysLog clv(L"FileList::UpdatePlugin"));
  _ALGO(SysLog(L"(KeepSelection=%d, IgnoreVisible=%d)",KeepSelection,IgnoreVisible));
  if (!IsVisible() && !IgnoreVisible)
  {
    UpdateRequired=TRUE;
    UpdateRequiredMode=KeepSelection;
    return;
  }
  DizRead=FALSE;

  int I;
  struct FileListItem *CurPtr, **OldData=0;
  string strCurName, strNextCurName;
  long OldFileCount=0;

  CloseChangeNotification();

  LastCurFile=-1;

  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

  if (Opt.ShowPanelFree && (Info.Flags & OPIF_REALNAMES))
  {
    unsigned __int64 TotalSize,TotalFree;
    string strDriveRoot;
    GetPathRoot(strCurDir,strDriveRoot);
    if (!GetDiskSize(strDriveRoot,&TotalSize,&TotalFree,&FreeDiskSize))
      FreeDiskSize=0;
  }

  PluginPanelItem *PanelData=NULL;
  int PluginFileCount;

  if (!CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0))
  {
    DeleteListData(ListData,FileCount);
    PopPlugin(TRUE);
    Update(KeepSelection);

    // WARP> ����� ���, �� ����� ������������ - ��������������� ������� �� ������ ��� ������ ������ ������.
    if ( PrevDataStackSize )
      GoToFile(PrevDataStack[PrevDataStackSize-1]->strPrevName);

    return;
  }

  int PrevSelFileCount=SelFileCount;
  SelFileCount=0;
  SelFileSize=0;
  TotalFileCount=0;
  TotalFileSize=0;

  strPluginDizName = L"";

  if (FileCount>0)
  {
    CurPtr=ListData[CurFile];
    strCurName = CurPtr->strName;
    if (CurPtr->Selected)
    {
      for (I=CurFile+1; I < FileCount; I++)
      {
          CurPtr = ListData[I];
        if (!CurPtr->Selected)
        {
          strNextCurName = CurPtr->strName;
          break;
        }
      }
    }
  }
  else
    if (Info.Flags & OPIF_ADDDOTS)
      strCurName = L"..";
  if (KeepSelection || PrevSelFileCount>0)
  {
    OldData=ListData;
    OldFileCount=FileCount;
  }
  else
    DeleteListData(ListData,FileCount);

  FileCount=PluginFileCount;
  ListData=(struct FileListItem**)xf_malloc(sizeof(struct FileListItem*)*(FileCount+1));

  if (ListData==NULL)
  {
    FileCount=0;
    return;
  }

  if (Filter==NULL)
    Filter=new FileFilter(this,FFT_PANEL);

  int DotsPresent=FALSE;

  int FileListCount=0;

  struct PluginPanelItem *CurPanelData=PanelData;
  for (I=0; I < FileCount; I++, CurPanelData++)
  {
    ListData[FileListCount] = new FileListItem;

    struct FileListItem *CurListData=ListData[FileListCount];

	CurListData->Clear ();

    if (Info.Flags & OPIF_USEFILTER)
      //if ((CurPanelData->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
        if (!Filter->FileInFilter(&CurPanelData->FindData))
          continue;
    if (!Opt.ShowHidden && (CurPanelData->FindData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
      continue;


    //memset(CurListData,0,sizeof(*CurListData));
    PluginToFileListItem(CurPanelData,CurListData);
    if(Info.Flags & OPIF_REALNAMES)
    {
        ConvertNameToShort (CurListData->strName, CurListData->strShortName);
    }
    CurListData->Position=I;
    if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
      CtrlObject->HiFiles->GetHiColor(&CurPanelData->FindData,&CurListData->Colors,(Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);
    if ((Info.Flags & OPIF_USESORTGROUPS)/* && (CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0*/)
      CurListData->SortGroup=CtrlObject->HiFiles->GetGroup(&CurPanelData->FindData);
    else
      CurListData->SortGroup=DEFAULT_SORT_GROUP;
    if (CurListData->DizText==NULL)
    {
      CurListData->DeleteDiz=FALSE;
      //CurListData->DizText=NULL;
    }
    if (TestParentFolderName(CurListData->strName))
    {
      DotsPresent=TRUE;
      CurListData->FileAttr|=FILE_ATTRIBUTE_DIRECTORY;
    }
    else
      if ((CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
        TotalFileCount++;
    TotalFileSize += CurListData->UnpSize;
    FileListCount++;
  }

  FileCount=FileListCount;

  if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
  {
    ListData[FileCount] = new FileListItem;

    struct FileListItem *CurPtr = ListData[FileCount];

	CurPtr->Clear ();

    AddParentPoint(CurPtr,FileCount);

    if ((Info.Flags & OPIF_USEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
      CtrlObject->HiFiles->GetHiColor(&CurPtr,1,(Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

    if (Info.HostFile && *Info.HostFile)
    {
      FAR_FIND_DATA_EX FindData;

      if ( apiGetFindDataEx (Info.HostFile,&FindData) )
      {
        CurPtr->WriteTime=FindData.ftLastWriteTime;
        CurPtr->CreationTime=FindData.ftCreationTime;
        CurPtr->AccessTime=FindData.ftLastAccessTime;
      }
    }
    FileCount++;
  }

  /* $ 25.02.2001 VVM
      ! �� ��������� �������� ������ ������ � ������ ������� */
  if (IsColumnDisplayed(DIZ_COLUMN))
    ReadDiz(PanelData,PluginFileCount,RDF_NO_UPDATE);

  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);

  CorrectPosition();

  if (KeepSelection || PrevSelFileCount>0)
  {
    MoveSelection(ListData,FileCount,OldData,OldFileCount);
    DeleteListData(OldData,OldFileCount);
  }

  if (!KeepSelection && PrevSelFileCount>0)
  {
    SaveSelection();
    ClearSelection();
  }

  SortFileList(FALSE);

  if (CurFile>=FileCount || StrCmpI(ListData[CurFile]->strName,strCurName)!=0)
      if (!GoToFile(strCurName) && !strNextCurName.IsEmpty() )
        GoToFile(strNextCurName);
  SetTitle();
}


void FileList::ReadDiz(struct PluginPanelItem *ItemList,int ItemLength,DWORD dwFlags)
{
  if (DizRead)
    return;
  DizRead=TRUE;
  Diz.Reset();

  if (PanelMode==NORMAL_PANEL)
    Diz.Read(strCurDir);
  else
  {
    PluginPanelItem *PanelData=NULL;
    int PluginFileCount=0;

    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    if (Info.DescrFilesNumber==0)
      return;

    int GetCode=TRUE;
    /* $ 25.02.2001 VVM
        + ��������� ����� RDF_NO_UPDATE */
    if ((ItemList==NULL) && ((dwFlags & RDF_NO_UPDATE) == 0))
      GetCode=CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0);
    else
    {
      PanelData=ItemList;
      PluginFileCount=ItemLength;
    }
    if (GetCode)
    {
      for (int I=0;I<Info.DescrFilesNumber;I++)
      {
        PluginPanelItem *CurPanelData=PanelData;
        for (int J=0; J < PluginFileCount; J++, CurPanelData++)
        {
            string strFileName = CurPanelData->FindData.lpwszFileName;

          if (StrCmpI(strFileName,Info.DescrFiles[I])==0)
          {
            string strTempDir, strDizName;
            if (FarMkTempEx(strTempDir) && CreateDirectoryW(strTempDir,NULL))
            {
              if (CtrlObject->Plugins.GetFile(hPlugin,CurPanelData,strTempDir,strDizName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW|OPM_DESCR))
              {
                strPluginDizName = Info.DescrFiles[I];

                Diz.Read(L"",strDizName);

                DeleteFileWithFolder(strDizName);
                I=Info.DescrFilesNumber;
                break;
              }
              apiRemoveDirectory(strTempDir);
              //ViewPanel->ShowFile(NULL,FALSE,NULL);
            }
          }
        }
      }
      /* $ 25.02.2001 VVM
          + ��������� ����� RDF_NO_UPDATE */
      if ((ItemList==NULL) && ((dwFlags & RDF_NO_UPDATE) == 0))
        CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);
    }
  }
  struct FileListItem *CurPtr;
  for (int I=0;I<FileCount;I++)
  {
    CurPtr = ListData[I];
    if (CurPtr->DizText==NULL)
    {
      CurPtr->DeleteDiz=FALSE;
      CurPtr->DizText=(wchar_t*)Diz.GetDizTextAddr(CurPtr->strName,CurPtr->strShortName,CurPtr->UnpSize);
    }
  }
}


void FileList::ReadSortGroups()
{
  if (SortGroupsRead)
    return;
  SortGroupsRead=TRUE;
  struct FileListItem *CurPtr;
  for (int I=0;I<FileCount;I++)
  {
      CurPtr = ListData[I];
    //if ((CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
      CurPtr->SortGroup=CtrlObject->HiFiles->GetGroup(CurPtr);
    //else
      //CurPtr->SortGroup=DEFAULT_SORT_GROUP;
  }
}

// �������� ������� CurPtr � ������� ���������������� ������ ��� �������� ".."
void FileList::AddParentPoint(struct FileListItem *CurPtr,long CurFilePos)
{
	CurPtr->Clear ();

	CurPtr->FileAttr = FILE_ATTRIBUTE_DIRECTORY;
	CurPtr->strName = L"..";
	CurPtr->strShortName = L"..";
	CurPtr->Position = CurFilePos;
}
