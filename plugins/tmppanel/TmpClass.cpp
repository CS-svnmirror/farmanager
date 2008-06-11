/*
TMPCLASS.CPP

Temporary panel plugin class implementation

*/

#include "TmpPanel.hpp"

static int _cdecl SortListCmp(const void *el1,const void *el2);

TmpPanel::TmpPanel()
{
  PanelIndex=CurrentCommonPanel;
  IfOptCommonPanel();
  if(!StartupOptCommonPanel)
  {
    TmpPanelItem=NULL;
    TmpItemsNumber=0;
  }

  LastOwnersRead=FALSE;
  LastLinksRead=FALSE;
  UpdateNotNeeded=FALSE;
}


TmpPanel::~TmpPanel()
{
  if(!StartupOptCommonPanel)
    FreePanelItems(TmpPanelItem, TmpItemsNumber);
}

int TmpPanel::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  IfOptCommonPanel();
  static struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  UpdateItems(IsOwnersDisplayed (PInfo),IsLinksDisplayed (PInfo));
  *pPanelItem=TmpPanelItem;
  *pItemsNumber=TmpItemsNumber;
  return(TRUE);
}


void TmpPanel::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_USEFILTER|OPIF_USESORTGROUPS|OPIF_USEHIGHLIGHTING|
    OPIF_ADDDOTS|OPIF_SHOWNAMESONLY;

  if(!Opt.SafeModePanel) Info->Flags|=OPIF_REALNAMES;

  Info->HostFile=NULL;
  Info->CurDir=_T("");

  Info->Format=(TCHAR*)GetMsg(MTempPanel);

  static TCHAR Title[MAX_PATH];
  static TCHAR PanelMode[16];
  lstrcpy(PanelMode, Opt.SafeModePanel ? _T("(R) ") : _T(""));
  if(StartupOptCommonPanel)
    FSF.sprintf(Title,GetMsg(MTempPanelTitleNum),PanelMode,PanelIndex);
  else
    FSF.sprintf(Title,_T(" %s%s "),PanelMode,GetMsg(MTempPanel));

  Info->PanelTitle=Title;

  static struct PanelMode PanelModesArray[10];
  PanelModesArray[4].FullScreen=(StartupOpenFrom==OPEN_COMMANDLINE)?
    Opt.FullScreenPanel:StartupOptFullScreenPanel;
  PanelModesArray[4].ColumnTypes=Opt.ColumnTypes;
  PanelModesArray[4].ColumnWidths=Opt.ColumnWidths;
  PanelModesArray[4].StatusColumnTypes=Opt.StatusColumnTypes;
  PanelModesArray[4].StatusColumnWidths=Opt.StatusColumnWidths;
  PanelModesArray[4].CaseConversion=TRUE;

  Info->PanelModesArray=PanelModesArray;
  Info->PanelModesNumber=ArraySize(PanelModesArray);
  Info->StartPanelMode=_T('4');
  static struct KeyBarTitles KeyBar;
  memset(&KeyBar,0,sizeof(KeyBar));
  KeyBar.Titles[7-1]=(TCHAR*)GetMsg(MF7);
  if(StartupOptCommonPanel)
    KeyBar.AltShiftTitles[12-1]=(TCHAR*)GetMsg(MAltShiftF12);
  KeyBar.AltShiftTitles[2-1]=(TCHAR*)GetMsg(MAltShiftF2);
  KeyBar.AltShiftTitles[3-1]=(TCHAR*)GetMsg(MAltShiftF3);
  Info->KeyBar=&KeyBar;
}


int TmpPanel::SetDirectory(const TCHAR *Dir,int OpMode)
{
  if((OpMode & OPM_FIND)/* || lstrcmp(Dir,_T("\\"))==0*/)
    return(FALSE);
  if(lstrcmp(Dir,_T("\\"))==0)
    Info.Control(this,FCTL_CLOSEPLUGIN,(void*)NULL);
  else
    Info.Control(this,FCTL_CLOSEPLUGIN,(void*)Dir);
  return(TRUE);
}


int TmpPanel::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int,int)
{
  UpdateNotNeeded=FALSE;

  HANDLE hScreen = BeginPutFiles();
  for(int i=0;i<ItemsNumber;i++)
  {
    if (!PutOneFile (PanelItem [i]))
    {
      CommitPutFiles (hScreen, FALSE);
      return FALSE;
    }
  }
  CommitPutFiles (hScreen, TRUE);
  return(1);
}

HANDLE TmpPanel::BeginPutFiles()
{
  IfOptCommonPanel();
  Opt.SelectedCopyContents = Opt.CopyContents;

  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  const TCHAR *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempSendFiles)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);
  return hScreen;
}

static inline int cmp_names(const WIN32_FIND_DATA &wfd, const FAR_FIND_DATA &ffd)
{
#ifndef UNICODE
#define _NAME cFileName
#else
#define _NAME lpwszFileName
#endif
  return lstrcmp(wfd.cFileName, FSF.PointToName(ffd._NAME));
#undef _NAME
}


int TmpPanel::PutOneFile (PluginPanelItem &PanelItem)
{
  static TCHAR CurDir[NM];
  GetCurrentDirectory(ArraySize(CurDir),CurDir);
  FSF.AddEndSlash(CurDir);

  struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));
  if(NewPanelItem==NULL)
    return FALSE;
  TmpPanelItem=NewPanelItem;
  struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber++];
  memset(CurPanelItem,0,sizeof(*CurPanelItem));
  CurPanelItem->FindData=PanelItem.FindData;

#ifdef UNICODE
#define cFileName lpwszFileName
#endif
#define CurName PanelItem.FindData.cFileName
  if(CheckForCorrect(CurName,&CurPanelItem->FindData,Opt.AnyInPanel))
  {
    int NameOnly=(FSF.PointToName(CurName)==CurName);
#undef CurName

#ifndef UNICODE
    lstrcpy(CurPanelItem->FindData.cFileName, NameOnly ? "" : CurDir);
    lstrcat(CurPanelItem->FindData.cFileName,PanelItem.FindData.cFileName);
#else
    size_t wlen = lstrlen(PanelItem.FindData.lpwszFileName);
    if (!NameOnly) wlen += lstrlen(CurDir);
    register wchar_t *wp = (wchar_t*)malloc(wlen+1);
    lstrcpy(wp, NameOnly ? L"" : CurDir);
    lstrcat(wp, PanelItem.FindData.lpwszFileName);
    CurPanelItem->FindData.lpwszFileName = wp;
#endif
    if(Opt.SelectedCopyContents && NameOnly && (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      if (Opt.SelectedCopyContents==2)
      {
        const TCHAR *MsgItems[]={GetMsg(MWarning),GetMsg(MCopyContensMsg)};
        Opt.SelectedCopyContents=!Info.Message(Info.ModuleNumber,FMSG_MB_YESNO,_T("Config"),
                                  MsgItems,ArraySize(MsgItems),0);
      }
      if (Opt.SelectedCopyContents)
      {
#ifndef UNICODE
        struct PluginPanelItem *DirItems;
#else
        FAR_FIND_DATA *DirItems;
#endif
        int DirItemsNumber;
        if(!Info.GetDirList(CurPanelItem->FindData.cFileName,
                            &DirItems,
                            &DirItemsNumber))
        {
#ifdef UNICODE
          for(int i = 0; i < TmpItemsNumber; i++)
            free((wchar_t*)TmpPanelItem[i].FindData.lpwszFileName);
#endif
          free(TmpPanelItem);
          TmpPanelItem=NULL;
          TmpItemsNumber=0;
          return FALSE;
        }
        struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+DirItemsNumber));
        if(NewPanelItem==NULL)
          return FALSE;
        TmpPanelItem=NewPanelItem;
#ifdef UNICODE
        wlen = lstrlen(CurDir) + 1;
#endif
        for(int i=0;i<DirItemsNumber;i++)
        {
          struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber++];
          memset(CurPanelItem,0,sizeof(*CurPanelItem));
#ifndef UNICODE
          CurPanelItem->FindData=DirItems[i].FindData;
          lstrcpy(CurPanelItem->FindData.cFileName,CurDir);
          lstrcat(CurPanelItem->FindData.cFileName,DirItems[i].FindData.cFileName);
#else
          CurPanelItem->FindData=DirItems[i];
          wp = (wchar_t*)malloc(wlen + lstrlen(DirItems[i].lpwszFileName));
          lstrcpy(wp, CurDir);
          lstrcat(wp, DirItems[i].lpwszFileName);
          CurPanelItem->FindData.lpwszFileName = wp;
#endif
        }
        Info.FreeDirList(DirItems
#ifdef UNICODE
                                 , DirItemsNumber
#endif
                        );
      }
    }
  }
  return TRUE;
}

void TmpPanel::CommitPutFiles (HANDLE hRestoreScreen, int Success)
{
  if (Success)
  {
    SortList();
    RemoveDups();
  }
  Info.RestoreScreen (hRestoreScreen);
}


int TmpPanel::SetFindList(const struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  HANDLE hScreen = BeginPutFiles();
  FindSearchResultsPanel();
  if(TmpPanelItem) free(TmpPanelItem);
  TmpPanelItem=(PluginPanelItem*) malloc(sizeof(PluginPanelItem)*ItemsNumber+1);
  if(TmpPanelItem==NULL)
    TmpItemsNumber=0;
  else
  {
    TmpItemsNumber=ItemsNumber;
    memcpy(TmpPanelItem,PanelItem,ItemsNumber*sizeof(*TmpPanelItem));
    for(int i=0;i<ItemsNumber;++i)
      TmpPanelItem[i].Flags&=~PPIF_SELECTED;
  }
  CommitPutFiles (hScreen, TRUE);
  UpdateNotNeeded=TRUE;
  return(TRUE);
}


void TmpPanel::FindSearchResultsPanel()
{
  if(StartupOptCommonPanel)
  {
    if (!Opt.NewPanelForSearchResults)
      IfOptCommonPanel();
    else
    {
      int SearchResultsPanel = -1;
      for (int i=0; i<COMMONPANELSNUMBER; i++)
      {
        if (CommonPanels [i].ItemsNumber == 0)
        {
          SearchResultsPanel = i;
          break;
        }
      }
      if (SearchResultsPanel < 0)
      {
        // all panels are full - use least recently used panel
        SearchResultsPanel = Opt.LastSearchResultsPanel++;
        if (Opt.LastSearchResultsPanel >= COMMONPANELSNUMBER)
          Opt.LastSearchResultsPanel = 0;
      }
      PanelIndex = SearchResultsPanel;
      CurrentCommonPanel=PanelIndex;
      TmpPanelItem = CommonPanels[PanelIndex].Items;
      TmpItemsNumber = CommonPanels[PanelIndex].ItemsNumber;
    }
  }
}


void TmpPanel::SortList()
{
  FSF.qsort(TmpPanelItem,TmpItemsNumber,sizeof(*TmpPanelItem),SortListCmp);
}


int _cdecl SortListCmp(const void *el1,const void *el2)
{
  struct PluginPanelItem *Item1,*Item2;
  Item1=(struct PluginPanelItem *)el1;
  Item2=(struct PluginPanelItem *)el2;
  return(lstrcmp(Item1->FindData.cFileName,Item2->FindData.cFileName));
}


void TmpPanel::RemoveDups()
{
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for(int i=0;i<TmpItemsNumber-1;i++,CurItem++)
    if(lstrcmp(CurItem->FindData.cFileName,CurItem[1].FindData.cFileName)==0)
      CurItem->Flags|=REMOVE_FLAG;
    RemoveEmptyItems();
}


void TmpPanel::RemoveEmptyItems()
{
  int EmptyCount=0;
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for(int i=0;i<TmpItemsNumber;i++,CurItem++)
    if(CurItem->Flags & REMOVE_FLAG)
    {
      if(CurItem->Owner)
        free (CurItem->Owner);
      EmptyCount++;
    }
    else if(EmptyCount>0)
      *(CurItem-EmptyCount)=*CurItem;
  TmpItemsNumber-=EmptyCount;
  if(StartupOptCommonPanel)
  {
    CommonPanels[PanelIndex].Items=TmpPanelItem;
    CommonPanels[PanelIndex].ItemsNumber=TmpItemsNumber;
  }
}


void TmpPanel::UpdateItems(int ShowOwners,int ShowLinks)
{
  if(UpdateNotNeeded || TmpItemsNumber == 0)
  {
    UpdateNotNeeded=FALSE;
    return;
  }
  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  const TCHAR *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempUpdate)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);
  LastOwnersRead=ShowOwners;
  LastLinksRead=ShowLinks;
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for(int i=0;i<TmpItemsNumber;i++,CurItem++)
  {
    HANDLE FindHandle;
    static TCHAR FullName[NM];
    lstrcpy(FullName,CurItem->FindData.cFileName);

    TCHAR *Slash=_tcsrchr(/*(const TCHAR*)*/FullName,_T('\\'));
    int Length=Slash ? (int)(Slash-FullName+1):0;

    int SameFolderItems=1;
    /* $ 23.12.2001 DJ
       ���� FullName - ��� �������, �� FindFirstFile (FullName+"*.*")
       ���� ������� �� ������. ������� ��� ��������� ����������� �
       SameFolderItems ����������.
    */
    if(Length>0 && Length > (int)lstrlen (FullName))   /* DJ $ */
      for(int j=1;i+j<TmpItemsNumber;j++)
        if(memcmp(FullName,CurItem[j].FindData.cFileName,Length*sizeof(TCHAR))==0 &&
          _tcschr((const TCHAR*)CurItem[j].FindData.cFileName+Length,_T('\\'))==0)
          SameFolderItems++;
        else
          break;

    // SameFolderItems - ����������� ��� ������, ����� � ������ �����
    // ��������� ������ �� ������ � ���� �� ��������. ��� ����
    // FindFirstFile() �������� ���� ��� �� �������, � �� �������� ���
    // ������� �����.
    if(SameFolderItems>2)
    {
      WIN32_FIND_DATA FindData;
      lstrcpy(Slash+1,_T("*.*"));
      for(int J=0;J<SameFolderItems;J++)
        CurItem[J].Flags|=REMOVE_FLAG;
      int Done=(FindHandle=FindFirstFile(FullName,&FindData))==INVALID_HANDLE_VALUE;
      while(!Done)
      {
        for(int J=0;J<SameFolderItems;J++)
          if((CurItem[J].Flags & 1) && cmp_names(FindData, CurItem[J].FindData)==0)
          {
            CurItem[J].Flags&=~REMOVE_FLAG;
            lstrcpy(FullName,CurItem[J].FindData.cFileName);
            WFD2FFD(FindData,CurItem[J].FindData);
            lstrcpy(CurItem[J].FindData.cFileName,FullName);
            break;
          }
          Done=!FindNextFile(FindHandle,&FindData);
      }
      FindClose(FindHandle);
      i+=SameFolderItems-1;
      CurItem+=SameFolderItems-1;
    }
    else
      if(!CheckForCorrect(FullName,&CurItem->FindData,Opt.AnyInPanel))
        CurItem->Flags|=REMOVE_FLAG;
  }

  RemoveEmptyItems();

  if(ShowOwners || ShowLinks)
  {
    struct PluginPanelItem *CurItem=TmpPanelItem;
    for(int i=0;i<TmpItemsNumber;i++,CurItem++)
    {
      if(ShowOwners)
      {
        static TCHAR Owner[80];
        if(CurItem->Owner)
        {
          free (CurItem->Owner);
          CurItem->Owner=NULL;
        }
        if(FSF.GetFileOwner(NULL,CurItem->FindData.cFileName,Owner))
        {
          CurItem->Owner=(TCHAR *) malloc((lstrlen(Owner)+1)*sizeof(TCHAR));
          if(CurItem->Owner) lstrcpy(CurItem->Owner,Owner);
        }
      }
      if(ShowLinks)
        CurItem->NumberOfLinks=FSF.GetNumberOfLinks(CurItem->FindData.cFileName);
    }
  }
  Info.RestoreScreen(hScreen);
}


int TmpPanel::ProcessEvent(int Event,void *)
{
  if(Event==FE_CHANGEVIEWMODE)
  {
    IfOptCommonPanel();
    static struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    int UpdateOwners=IsOwnersDisplayed (PInfo) && !LastOwnersRead;
    int UpdateLinks=IsLinksDisplayed (PInfo) && !LastLinksRead;
    if(UpdateOwners || UpdateLinks)
    {
      UpdateItems(UpdateOwners,UpdateLinks);
      Info.Control(this,FCTL_UPDATEPANEL,(void *)TRUE);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
  }
  return(FALSE);
}


int TmpPanel::IsCurrentFileCorrect (TCHAR *pCurFileName)
{
  static struct PanelInfo PInfo;
  static TCHAR CurFileName[NM];
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  lstrcpy(CurFileName, PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);

  BOOL IsCorrectFile = FALSE;
  if (lstrcmp (CurFileName, _T("..")) == 0)
    IsCorrectFile = TRUE;
  else
  {
    FAR_FIND_DATA TempFindData;
    IsCorrectFile=CheckForCorrect(CurFileName,&TempFindData,FALSE);
  }
  if (pCurFileName)
    lstrcpy (pCurFileName, CurFileName);
  return IsCorrectFile;
}

int TmpPanel::ProcessKey (int Key,unsigned int ControlState)
{
  if(!ControlState && Key==VK_F1)
  {
    Info.ShowHelp(Info.ModuleName, NULL, FHELP_USECONTENTS|FHELP_NOSHOWERROR);
    return TRUE;
  }

  if(ControlState==(PKF_SHIFT|PKF_ALT) && Key==VK_F9)
  {
     EXP_NAME(Configure)(0);
     return TRUE;
  }

  if(ControlState==(PKF_SHIFT|PKF_ALT) && Key==VK_F3)
  {
    TCHAR CurFileName [NM];
    if (IsCurrentFileCorrect(CurFileName))
    {
      static struct PanelInfo PInfo;
      Info.Control(this,FCTL_GETPANELINFO,&PInfo);
      if (lstrcmp(CurFileName,_T(".."))!=0)
      {
#ifndef UNICODE
#define _HANDLE INVALID_HANDLE_VALUE
#define _SET    FCTL_SETANOTHERPANELDIR
#define _REDRAW FCTL_REDRAWANOTHERPANEL
#else
#define _HANDLE ANOTHER_PANEL
#define _SET    FCTL_SETPANELDIR
#define _REDRAW FCTL_REDRAWPANEL
#endif
        if (PInfo.PanelItems[PInfo.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
          Info.Control(_HANDLE, _SET,&CurFileName);
        else
          GoToFile(CurFileName, true);
        Info.Control(_HANDLE, _REDRAW,NULL);
#undef _HANDLE
#undef _SET
#undef _REDRAW
        return(TRUE);
      }
    }
  }

  if (ControlState!=PKF_CONTROL && Key>=VK_F3 && Key<=VK_F8 && Key!=VK_F7)
  {
    if(!IsCurrentFileCorrect (NULL))
      return(TRUE);
  }

  if(ControlState==0 && Key==VK_RETURN && Opt.AnyInPanel)
  {
    TCHAR CurFileName [NM];
    if (!IsCurrentFileCorrect (CurFileName))
    {
      Info.Control(this,FCTL_SETCMDLINE,&CurFileName);
      return(TRUE);
    }
  }

  if (Opt.SafeModePanel && ControlState == PKF_CONTROL && Key == VK_PRIOR)
  {
    TCHAR CurFileName[NM];
    CurFileName[0] = _T('\0');
    if(IsCurrentFileCorrect(CurFileName) && 0!=lstrcmp(CurFileName,_T("..")))
    {
      GoToFile(CurFileName, false);
      return TRUE;
    }
    if(!lstrcmp(CurFileName,_T("..")))
    {
      SetDirectory(_T("."),0);
      return TRUE;
    }
  }

  if(ControlState==0 && Key==VK_F7)
  {
    ProcessRemoveKey();
    return TRUE;
  }
  else if (ControlState == (PKF_SHIFT|PKF_ALT) && Key == VK_F2)
  {
    ProcessSaveListKey();
    return TRUE;
  }
  else
  {
    if(StartupOptCommonPanel && ControlState==(PKF_SHIFT|PKF_ALT))
    {
      if (Key==VK_F12)
      {
        ProcessPanelSwitchMenu();
        return(TRUE);
      }
      else if (Key >= _T('0') && Key <= _T('9'))
      {
        SwitchToPanel (Key - _T('0'));
        return TRUE;
      }
    }
  }
  return(FALSE);
}


void TmpPanel::ProcessRemoveKey()
{
  IfOptCommonPanel();

  static struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);

  for(int i=0;i<PInfo.SelectedItemsNumber;i++)
  {
    struct PluginPanelItem *RemovedItem=(struct PluginPanelItem *)
      FSF.bsearch(&PInfo.SelectedItems[i],TmpPanelItem,TmpItemsNumber,
        sizeof(struct PluginPanelItem),SortListCmp);
    if(RemovedItem!=NULL)
      RemovedItem->Flags|=REMOVE_FLAG;
  }
  RemoveEmptyItems();
  Info.Control(this,FCTL_UPDATEPANEL,NULL);
  Info.Control(this,FCTL_REDRAWPANEL,NULL);

#ifndef UNICODE
#define _HANDLE this
#define _UPDATE FCTL_UPDATEANOTHERPANEL
#define _REDRAW FCTL_REDRAWANOTHERPANEL
#define _GET    FCTL_GETANOTHERPANELINFO
#else
#define _HANDLE ANOTHER_PANEL
#define _UPDATE FCTL_UPDATEPANEL
#define _REDRAW FCTL_REDRAWPANEL
#define _GET    FCTL_GETPANELINFO
#endif
  Info.Control(_HANDLE,_GET,(void *) &PInfo);
  if(PInfo.PanelType==PTYPE_QVIEWPANEL)
  {
    Info.Control(_HANDLE,_UPDATE,NULL);
    Info.Control(_HANDLE,_REDRAW,NULL);
  }
}

void TmpPanel::ProcessSaveListKey()
{
  IfOptCommonPanel();
  if (TmpItemsNumber == 0)
    return;

  PanelInfo PInfo;
  Info.Control (_HANDLE, _GET, &PInfo);

  // default path: opposite panel directory\panel<index>.<mask extension>
  static TCHAR ListPath [NM];
#ifdef UNICODE
#define CurDir  lpwszCurDir
#endif
  lstrcpy (ListPath, PInfo.CurDir);
#undef CurDir
  FSF.AddEndSlash (ListPath);
  lstrcat (ListPath, _T("panel"));
  if (Opt.CommonPanel)
    FSF.itoa (PanelIndex, ListPath + lstrlen (ListPath), 10);

  static TCHAR ExtBuf [NM];
  lstrcpy (ExtBuf, Opt.Mask);
  TCHAR *comma = _tcschr (ExtBuf, _T(','));
  if (comma)
    *comma = _T('\0');
  TCHAR *ext = _tcschr (ExtBuf, _T('.'));
  if (ext && !_tcschr (ext, _T('*')) && !_tcschr (ext, _T('?')))
    lstrcat (ListPath, ext);

  if (Info.InputBox (GetMsg (MTempPanel), GetMsg (MListFilePath),
      _T("TmpPanel.SaveList"), ListPath, ListPath, ArraySize(ListPath)-1,
      NULL, FIB_BUTTONS))
  {
    SaveListFile (ListPath);
    Info.Control (_HANDLE, _UPDATE, NULL);
    Info.Control (_HANDLE, _REDRAW, NULL);
  }
#undef _HANDLE
#undef _UPDATE
#undef _REDRAW
#undef _GET
}

void TmpPanel::SaveListFile (const TCHAR *Path)
{
  HANDLE hFile = CreateFile (Path, GENERIC_WRITE, 0, NULL,
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    const TCHAR *Items[] = { GetMsg (MError) };
    Info.Message (Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK,
      NULL, Items, 1, 0);
    return;
  }

  IfOptCommonPanel();

  for (int i=0; i<TmpItemsNumber; i++)
  {
    const TCHAR *CRLF = _T("\r\n");
    DWORD BytesWritten;
    const TCHAR *FName = TmpPanelItem [i].FindData.cFileName;
    WriteFile (hFile, FName, sizeof(TCHAR)*lstrlen(FName), &BytesWritten, NULL);
    WriteFile (hFile, CRLF, 2*sizeof(TCHAR), &BytesWritten, NULL);
  }
  CloseHandle (hFile);
}

void TmpPanel::SwitchToPanel (int NewPanelIndex)
{
  if(NewPanelIndex>-1 && NewPanelIndex<COMMONPANELSNUMBER && NewPanelIndex!=(int)PanelIndex)
  {
    CommonPanels[PanelIndex].Items=TmpPanelItem;
    CommonPanels[PanelIndex].ItemsNumber=TmpItemsNumber;
    if(!CommonPanels[NewPanelIndex].Items)
    {
      CommonPanels[NewPanelIndex].ItemsNumber=0;
      CommonPanels[NewPanelIndex].Items=(PluginPanelItem*)malloc(sizeof(PluginPanelItem));
    }
    if(CommonPanels[NewPanelIndex].Items)
    {
      CurrentCommonPanel=NewPanelIndex;
      PanelIndex=NewPanelIndex;
      Info.Control(this,FCTL_UPDATEPANEL,NULL);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
  }
}


void TmpPanel::ProcessPanelSwitchMenu()
{
  FarMenuItem *fmi=(FarMenuItem*)malloc(COMMONPANELSNUMBER*sizeof(FarMenuItem));
  if(fmi)
  {
    const TCHAR *txt=GetMsg(MSwitchMenuTxt);
#ifdef UNICODE
    wchar_t tmpstr[128][COMMONPANELSNUMBER];
#endif
    static const TCHAR fmt1[]=_T("&%c. %s %d");
    for(unsigned int i=0;i<COMMONPANELSNUMBER;++i)
    {
#ifndef UNICODE
#define _OUT  fmi[i].Text
#else
#define _OUT  tmpstr[i]
      fmi[i].Text = tmpstr[i];
#endif
      if(i<10)
        FSF.sprintf(_OUT,fmt1,_T('0')+i,txt,CommonPanels[i].ItemsNumber);
      else if(i<36)
        FSF.sprintf(_OUT,fmt1,_T('A')-10+i,txt,CommonPanels[i].ItemsNumber);
      else
        FSF.sprintf(_OUT,_T("   %s %d"),txt,CommonPanels[i].ItemsNumber);
#undef _OUT
    }
    fmi[PanelIndex].Selected=TRUE;
    int ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,
      FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
      GetMsg(MSwitchMenuTitle),NULL,NULL,
      NULL,NULL,fmi,COMMONPANELSNUMBER);
    free(fmi);
    SwitchToPanel (ExitCode);
  }
}


int TmpPanel::IsOwnersDisplayed (const struct PanelInfo &PInfo)
{
#ifdef UNICODE
#define ColumnTypes lpwszColumnTypes
#endif
  for(int i=0;PInfo.ColumnTypes[i];i++)
    if(PInfo.ColumnTypes[i]==_T('O') && (i==0 || PInfo.ColumnTypes[i-1]==_T(',')) &&
       (PInfo.ColumnTypes[i+1]==_T(',') || PInfo.ColumnTypes[i+1]==0))
      return(TRUE);
    return(FALSE);
}


int TmpPanel::IsLinksDisplayed (const struct PanelInfo &PInfo)
{
  for(int i=0;PInfo.ColumnTypes[i];i++)
    if(PInfo.ColumnTypes[i]==_T('L') && PInfo.ColumnTypes[i+1]==_T('N') &&
       (i==0 || PInfo.ColumnTypes[i-1]==_T(',')) &&
      (PInfo.ColumnTypes[i+2]==_T(',') || PInfo.ColumnTypes[i+2]==0))
      return(TRUE);
    return(FALSE);
#undef ColumnTypes
}

inline bool isDevice(const TCHAR* FileName, const TCHAR* dev_begin)
{
    const int len=(int)lstrlen(dev_begin);
    if(FSF.LStrnicmp(FileName, dev_begin, len)) return false;
    FileName+=len;
    if(!*FileName) return false;
    while(*FileName>=_T('0') && *FileName<=_T('9')) FileName++;
    return !*FileName;
}

int TmpPanel::CheckForCorrect(const TCHAR *Dir,FAR_FIND_DATA *FindData,int Any)
{
  static TCHAR TempDir[MAX_PATH], SavedDir[MAX_PATH];
  ExpandEnvStrs(Dir,TempDir,ArraySize(TempDir));
  WIN32_FIND_DATA wfd;

  TCHAR *p=TempDir;
  ParseParam(p);
  lstrcpy(SavedDir, p);

  if (!FSF.LStrnicmp(p, _T("\\\\.\\"), 4) && FSF.LIsAlpha(p[4]) && p[5]==_T(':') && p[6]==0)
  {
    lstrcpy(FindData->cFileName,p);
    FindData->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
    return(TRUE);
  }

  if (isDevice(p, _T("\\\\.\\PhysicalDrive")) || isDevice(p, _T("\\\\.\\cdrom")))
  {
    lstrcpy(FindData->cFileName,p);
    FindData->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
    return(TRUE);
  }

  if(lstrlen(p) && lstrcmp(p,_T("\\"))!=0 && lstrcmp(p,_T(".."))!=0)
  {
    HANDLE fff=FindFirstFile(p,&wfd);

    if(fff != INVALID_HANDLE_VALUE)
    {
      WFD2FFD(wfd,*FindData);
      FindClose(fff);
      lstrcpy(FindData->cFileName,p);
      return(TRUE);
    }
    else
    {
      TCHAR *t = p + lstrlen(p) - 1;
      if (*t == _T('\\')) *t = 0;
      static TCHAR TMP[MAX_PATH];
      fff=FindFirstFile(lstrcat(lstrcpy(TMP, p),_T("\\*.*")),&wfd);
      if(fff != INVALID_HANDLE_VALUE)
      {
        WFD2FFD(wfd,*FindData);
        FindClose(fff);
        FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        lstrcpy(FindData->cFileName,SavedDir);
        return(TRUE);
      }
    }
    if(Any)
    {
      FindData->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
      lstrcpy(FindData->cFileName,p);
      return(TRUE);
    }
  }
  return(FALSE);
}


void TmpPanel::IfOptCommonPanel(void)
{
  if(StartupOptCommonPanel)
  {
    TmpPanelItem=CommonPanels[PanelIndex].Items;
    TmpItemsNumber=CommonPanels[PanelIndex].ItemsNumber;
  }
}
