/*
TMPCLASS.CPP

Temporary panel plugin class implementation

*/

#include "TmpPanel.hpp"
#include "guid.hpp"

TmpPanel::TmpPanel()
{
	LastOwnersRead=FALSE;
	LastLinksRead=FALSE;
	UpdateNotNeeded=FALSE;
	TmpPanelItem=NULL;
	TmpItemsNumber=0;
	PanelIndex=CurrentCommonPanel;
	IfOptCommonPanel();
}


TmpPanel::~TmpPanel()
{
	if (!StartupOptCommonPanel)
		FreePanelItems(TmpPanelItem, TmpItemsNumber);
}

int TmpPanel::GetFindData(PluginPanelItem **pPanelItem,size_t *pItemsNumber,const OPERATION_MODES OpMode)
{
	IfOptCommonPanel();
	size_t Size=Info.PanelControl(this,FCTL_GETCOLUMNTYPES,0,NULL);
	wchar_t* ColumnTypes=new wchar_t[Size];
	Info.PanelControl(this,FCTL_GETCOLUMNTYPES,static_cast<int>(Size),ColumnTypes);
	UpdateItems(IsOwnersDisplayed(ColumnTypes),IsLinksDisplayed(ColumnTypes));
	delete[] ColumnTypes;
	*pPanelItem=TmpPanelItem;
	*pItemsNumber=TmpItemsNumber;
	return(TRUE);
}


void TmpPanel::GetOpenPanelInfo(struct OpenPanelInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=OPIF_ADDDOTS|OPIF_SHOWNAMESONLY;

	if (!Opt.SafeModePanel) Info->Flags|=OPIF_REALNAMES;

	Info->HostFile=NULL;
	Info->CurDir=L"";
	Info->Format=(wchar_t*)GetMsg(MTempPanel);
	static wchar_t Title[100];

	if (StartupOptCommonPanel)
		FSF.sprintf(Title,GetMsg(MTempPanelTitleNum),(Opt.SafeModePanel ? L"(R) " : L""),PanelIndex);
	else
		FSF.sprintf(Title,L" %s%s ",(Opt.SafeModePanel ? L"(R) " : L""),GetMsg(MTempPanel));

	Info->PanelTitle=Title;
	static struct PanelMode PanelModesArray[10];
	PanelModesArray[4].Flags=PMFLAGS_CASECONVERSION;
	if ((StartupOpenFrom==OPEN_COMMANDLINE)?Opt.FullScreenPanel:StartupOptFullScreenPanel)
		PanelModesArray[4].Flags|=PMFLAGS_FULLSCREEN;
	PanelModesArray[4].ColumnTypes=Opt.ColumnTypes;
	PanelModesArray[4].ColumnWidths=Opt.ColumnWidths;
	PanelModesArray[4].StatusColumnTypes=Opt.StatusColumnTypes;
	PanelModesArray[4].StatusColumnWidths=Opt.StatusColumnWidths;
	Info->PanelModesArray=PanelModesArray;
	Info->PanelModesNumber=ARRAYSIZE(PanelModesArray);
	Info->StartPanelMode=L'4';

	static WORD FKeys[]=
	{
		VK_F7,0,MF7,
		VK_F2,SHIFT_PRESSED|LEFT_ALT_PRESSED,MAltShiftF2,
		VK_F3,SHIFT_PRESSED|LEFT_ALT_PRESSED,MAltShiftF3,
		VK_F12,SHIFT_PRESSED|LEFT_ALT_PRESSED,MAltShiftF12,
	};

	static struct KeyBarLabel kbl[ARRAYSIZE(FKeys)/3];
	static struct KeyBarTitles kbt = {ARRAYSIZE(kbl), kbl};

	for (size_t j=0,i=0; i < ARRAYSIZE(FKeys); i+=3, ++j)
	{
		kbl[j].Key.VirtualKeyCode = FKeys[i];
		kbl[j].Key.ControlKeyState = FKeys[i+1];

		if (FKeys[i+2])
		{
			kbl[j].Text = kbl[j].LongText = GetMsg(FKeys[i+2]);
			if (!StartupOptCommonPanel && kbl[j].Key.VirtualKeyCode == VK_F12 && kbl[j].Key.ControlKeyState == (SHIFT_PRESSED|LEFT_ALT_PRESSED))
				kbl[j].Text = kbl[j].LongText = L"";
		}
		else
		{
			kbl[j].Text = kbl[j].LongText = L"";
		}
	}

	Info->KeyBar=&kbt;
}


int TmpPanel::SetDirectory(const wchar_t *Dir,const OPERATION_MODES OpMode)
{
	if ((OpMode & OPM_FIND)/* || lstrcmp(Dir,L"\\")==0*/)
		return(FALSE);

	if (lstrcmp(Dir,L"\\")==0)
		Info.PanelControl(this,FCTL_CLOSEPANEL,0,NULL);
	else
		Info.PanelControl(this,FCTL_CLOSEPANEL,0,(void*)Dir);
	return(TRUE);
}


int TmpPanel::PutFiles(struct PluginPanelItem *PanelItem,size_t ItemsNumber,int,const wchar_t *SrcPath,const OPERATION_MODES)
{
	UpdateNotNeeded=FALSE;
	HANDLE hScreen = BeginPutFiles();

	for (size_t i=0; i<ItemsNumber; i++)
	{
		if (!PutOneFile(SrcPath, PanelItem[i]))
		{
			CommitPutFiles(hScreen, FALSE);
			return FALSE;
		}
	}

	CommitPutFiles(hScreen, TRUE);
	return(1);
}

HANDLE TmpPanel::BeginPutFiles()
{
	IfOptCommonPanel();
	Opt.SelectedCopyContents = Opt.CopyContents;
	HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
	const wchar_t *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempSendFiles)};
	Info.Message(&MainGuid,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
	return hScreen;
}

static inline int cmp_names(const WIN32_FIND_DATA &wfd, const PluginPanelItem &ffd)
{
	return lstrcmp(wfd.cFileName, FSF.PointToName(ffd.FileName));
}

int TmpPanel::PutDirectoryContents(const wchar_t* Path)
{
	if (Opt.SelectedCopyContents==2)
	{
		const wchar_t *MsgItems[]={GetMsg(MWarning),GetMsg(MCopyContensMsg)};
		Opt.SelectedCopyContents=!Info.Message(&MainGuid,FMSG_MB_YESNO,L"Config",MsgItems,ARRAYSIZE(MsgItems),0);
	}

	if (Opt.SelectedCopyContents)
	{
		PluginPanelItem *DirItems;
		size_t DirItemsNumber;

		if (!Info.GetDirList(Path, &DirItems, &DirItemsNumber))
		{
			FreePanelItems(TmpPanelItem, TmpItemsNumber);
			TmpPanelItem=NULL;
			TmpItemsNumber=0;
			return FALSE;
		}

		struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+DirItemsNumber));

		if (NewPanelItem==NULL)
			return FALSE;

		TmpPanelItem=NewPanelItem;
		memset(&TmpPanelItem[TmpItemsNumber],0,sizeof(*TmpPanelItem)*DirItemsNumber);
		size_t PathLen = lstrlen(Path);

		for (int i=0; i<DirItemsNumber; i++)
		{
			struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber];
			CurPanelItem->UserData = TmpItemsNumber;
			TmpItemsNumber++;
			CurPanelItem->FileAttributes=DirItems[i].FileAttributes;
			CurPanelItem->CreationTime=DirItems[i].CreationTime;
			CurPanelItem->LastAccessTime=DirItems[i].LastAccessTime;
			CurPanelItem->LastWriteTime=DirItems[i].LastWriteTime;
			CurPanelItem->ChangeTime=DirItems[i].ChangeTime;
			CurPanelItem->FileSize=DirItems[i].FileSize;
			CurPanelItem->PackSize=DirItems[i].PackSize;

			CurPanelItem->FileName = wcsdup(DirItems[i].FileName);
			CurPanelItem->AlternateFileName = NULL;
		}

		Info.FreeDirList(DirItems, DirItemsNumber);
	}

	return TRUE;
}

int TmpPanel::PutOneFile(const wchar_t* SrcPath, PluginPanelItem &PanelItem)
{
	struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));

	if (NewPanelItem==NULL)
		return FALSE;

	TmpPanelItem=NewPanelItem;
	struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber];
	memset(CurPanelItem,0,sizeof(*CurPanelItem));
	CurPanelItem->FileAttributes=PanelItem.FileAttributes;
	CurPanelItem->CreationTime=PanelItem.CreationTime;
	CurPanelItem->LastAccessTime=PanelItem.LastAccessTime;
	CurPanelItem->LastWriteTime=PanelItem.LastWriteTime;
	CurPanelItem->ChangeTime=PanelItem.ChangeTime;
	CurPanelItem->FileSize=PanelItem.FileSize;
	CurPanelItem->PackSize=PanelItem.PackSize;
	CurPanelItem->UserData = TmpItemsNumber;
	CurPanelItem->FileName = reinterpret_cast<wchar_t*>(malloc((lstrlen(SrcPath)+1+lstrlen(PanelItem.FileName)+1)*sizeof(wchar_t)));

	if (CurPanelItem->FileName==NULL)
		return FALSE;

	CurPanelItem->AlternateFileName = NULL;

	if (*SrcPath)
	{
		lstrcpy((wchar_t*)CurPanelItem->FileName, SrcPath);
		FSF.AddEndSlash((wchar_t*)CurPanelItem->FileName);
	}

	lstrcat((wchar_t*)CurPanelItem->FileName, PanelItem.FileName);
	TmpItemsNumber++;

	if (Opt.SelectedCopyContents && (CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return PutDirectoryContents(CurPanelItem->FileName);

	return TRUE;
}

int TmpPanel::PutOneFile(const wchar_t* FilePath)
{
	struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));

	if (NewPanelItem==NULL)
		return FALSE;

	TmpPanelItem=NewPanelItem;
	struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber];
	memset(CurPanelItem,0,sizeof(*CurPanelItem));
	CurPanelItem->UserData = TmpItemsNumber;

	if (GetFileInfoAndValidate(FilePath, CurPanelItem, Opt.AnyInPanel))
	{
		TmpItemsNumber++;

		if (Opt.SelectedCopyContents && (CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return PutDirectoryContents(CurPanelItem->FileName);
	}

	return TRUE;
}

void TmpPanel::CommitPutFiles(HANDLE hRestoreScreen, int Success)
{
	if (Success)
		RemoveDups();

	Info.RestoreScreen(hRestoreScreen);
}


int TmpPanel::SetFindList(const struct PluginPanelItem *PanelItem,size_t ItemsNumber)
{
	HANDLE hScreen = BeginPutFiles();
	FindSearchResultsPanel();
	FreePanelItems(TmpPanelItem, TmpItemsNumber);
	TmpItemsNumber=0;
	TmpPanelItem=(PluginPanelItem*)malloc(sizeof(PluginPanelItem)*ItemsNumber);

	if (TmpPanelItem)
	{
		TmpItemsNumber=ItemsNumber;
		memset(TmpPanelItem,0,TmpItemsNumber*sizeof(*TmpPanelItem));

		for (size_t i=0; i<ItemsNumber; ++i)
		{
			TmpPanelItem[i].UserData = i;

			TmpPanelItem[i].FileAttributes=PanelItem[i].FileAttributes;
			TmpPanelItem[i].CreationTime=PanelItem[i].CreationTime;
			TmpPanelItem[i].LastAccessTime=PanelItem[i].LastAccessTime;
			TmpPanelItem[i].LastWriteTime=PanelItem[i].LastWriteTime;
			TmpPanelItem[i].ChangeTime=PanelItem[i].ChangeTime;
			TmpPanelItem[i].FileSize=PanelItem[i].FileSize;
			TmpPanelItem[i].PackSize=PanelItem[i].PackSize;

			if (TmpPanelItem[i].FileName)
				TmpPanelItem[i].FileName = wcsdup(TmpPanelItem[i].FileName);

			TmpPanelItem[i].AlternateFileName = NULL;
		}
	}

	CommitPutFiles(hScreen, TRUE);
	UpdateNotNeeded=TRUE;
	return(TRUE);
}


void TmpPanel::FindSearchResultsPanel()
{
	if (StartupOptCommonPanel)
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

			if (PanelIndex != SearchResultsPanel)
			{
				CommonPanels[PanelIndex].Items = TmpPanelItem;
				CommonPanels[PanelIndex].ItemsNumber = (UINT)TmpItemsNumber;
				PanelIndex = SearchResultsPanel;
				TmpPanelItem = CommonPanels[PanelIndex].Items;
				TmpItemsNumber = CommonPanels[PanelIndex].ItemsNumber;
			}

			CurrentCommonPanel = PanelIndex;
		}
	}
}

int _cdecl SortListCmp(const void *el1, const void *el2, void *userparam)
{
	PluginPanelItem* TmpPanelItem = reinterpret_cast<PluginPanelItem*>(userparam);
	int idx1 = *reinterpret_cast<const int*>(el1);
	int idx2 = *reinterpret_cast<const int*>(el2);
	int res = lstrcmp(TmpPanelItem[idx1].FileName, TmpPanelItem[idx2].FileName);

	if (res == 0)
	{
		if (idx1 < idx2) return -1;
		else if (idx1 == idx2) return 0;
		else return 1;
	}
	else
		return res;
}

void TmpPanel::RemoveDups()
{
	int* indices = reinterpret_cast<int*>(malloc(TmpItemsNumber*sizeof(int)));

	if (indices == NULL)
		return;

	for (int i = 0; i < TmpItemsNumber; i++)
		indices[i] = i;

	FSF.qsortex(indices, TmpItemsNumber, sizeof(int), SortListCmp, TmpPanelItem);

	for (int i = 0; i + 1 < TmpItemsNumber; i++)
		if (lstrcmp(TmpPanelItem[indices[i]].FileName, TmpPanelItem[indices[i + 1]].FileName) == 0)
			TmpPanelItem[indices[i + 1]].Flags |= REMOVE_FLAG;

	free(indices);
	RemoveEmptyItems();
}

void TmpPanel::RemoveEmptyItems()
{
	int EmptyCount=0;
	struct PluginPanelItem *CurItem=TmpPanelItem;

	for (int i=0; i<TmpItemsNumber; i++,CurItem++)
		if (CurItem->Flags & REMOVE_FLAG)
		{
			if (CurItem->Owner)
			{
				free((void*)CurItem->Owner);
				CurItem->Owner = NULL;
			}

			if (CurItem->FileName)
			{
				free((wchar_t*)CurItem->FileName);
				CurItem->FileName = NULL;
			}
			EmptyCount++;
		}
		else if (EmptyCount)
		{
			CurItem->UserData -= EmptyCount;
			*(CurItem-EmptyCount)=*CurItem;
			memset(CurItem, 0, sizeof(*CurItem));
		}

	TmpItemsNumber-=EmptyCount;

	if (EmptyCount>1)
		TmpPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));

	if (StartupOptCommonPanel)
	{
		CommonPanels[PanelIndex].Items=TmpPanelItem;
		CommonPanels[PanelIndex].ItemsNumber=(UINT)TmpItemsNumber;
	}
}


void TmpPanel::UpdateItems(int ShowOwners,int ShowLinks)
{
	if (UpdateNotNeeded || TmpItemsNumber == 0)
	{
		UpdateNotNeeded=FALSE;
		return;
	}

	HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
	const wchar_t *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempUpdate)};
	Info.Message(&MainGuid,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
	LastOwnersRead=ShowOwners;
	LastLinksRead=ShowLinks;
	struct PluginPanelItem *CurItem=TmpPanelItem;

	for (int i=0; i<TmpItemsNumber; i++,CurItem++)
	{
		HANDLE FindHandle;
		const wchar_t *lpFullName = CurItem->FileName;
		const wchar_t *lpSlash = _tcsrchr(lpFullName,L'\\');
		int Length=lpSlash ? (int)(lpSlash-lpFullName+1):0;
		int SameFolderItems=1;

		/* $ 23.12.2001 DJ
		   ���� FullName - ��� �������, �� FindFirstFile (FullName+"*.*")
		   ���� ������� �� ������. ������� ��� ��������� ����������� �
		   SameFolderItems ����������.
		*/
		if (Length>0 && Length > (int)lstrlen(lpFullName))    /* DJ $ */
		{
			for (int j=1; i+j<TmpItemsNumber; j++)
			{
				if (memcmp(lpFullName,CurItem[j].FileName,Length*sizeof(wchar_t))==0 &&
				        _tcschr((const wchar_t*)CurItem[j].FileName+Length,L'\\')==NULL)
				{
					SameFolderItems++;
				}
				else
				{
					break;
				}
			}
		}

		// SameFolderItems - ����������� ��� ������, ����� � ������ �����
		// ��������� ������ �� ������ � ���� �� ��������. ��� ����
		// FindFirstFile() �������� ���� ��� �� �������, � �� �������� ���
		// ������� �����.
		if (SameFolderItems>2)
		{
			WIN32_FIND_DATA FindData;
			StrBuf FindFile((int)(lpSlash-lpFullName)+1+1+1);
			lstrcpyn(FindFile, lpFullName, (int)(lpSlash-lpFullName)+1);
			lstrcpy(FindFile+(lpSlash+1-lpFullName),L"*");
			StrBuf NtPath;
			FormNtPath(FindFile, NtPath);

			for (int J=0; J<SameFolderItems; J++)
				CurItem[J].Flags|=REMOVE_FLAG;

			int Done=(FindHandle=FindFirstFile(NtPath,&FindData))==INVALID_HANDLE_VALUE;

			while (!Done)
			{
				for (int J=0; J<SameFolderItems; J++)
				{
					if ((CurItem[J].Flags & 1) && cmp_names(FindData, CurItem[J])==0)
					{
						CurItem[J].Flags&=~REMOVE_FLAG;
						const wchar_t *save = CurItem[J].FileName;
						WFD2FFD(FindData,CurItem[J]);
						free((wchar_t*)CurItem[J].FileName);
						CurItem[J].FileName = save;
						break;
					}
				}

				Done=!FindNextFile(FindHandle,&FindData);
			}

			FindClose(FindHandle);
			i+=SameFolderItems-1;
			CurItem+=SameFolderItems-1;
		}
		else
		{
			if (!GetFileInfoAndValidate(lpFullName,CurItem,Opt.AnyInPanel))
				CurItem->Flags|=REMOVE_FLAG;
		}
	}

	RemoveEmptyItems();

	if (ShowOwners || ShowLinks)
	{
		struct PluginPanelItem *CurItem=TmpPanelItem;

		for (int i=0; i<TmpItemsNumber; i++,CurItem++)
		{
			if (ShowOwners)
			{
				wchar_t Owner[80];

				if (CurItem->Owner)
				{
					free((void*)CurItem->Owner);
					CurItem->Owner=NULL;
				}

				if (FSF.GetFileOwner(NULL,CurItem->FileName,Owner,80))
					CurItem->Owner=_tcsdup(Owner);
			}

			if (ShowLinks)
				CurItem->NumberOfLinks=FSF.GetNumberOfLinks(CurItem->FileName);
		}
	}

	Info.RestoreScreen(hScreen);
}


int TmpPanel::ProcessEvent(int Event,void *)
{
	if (Event==FE_CHANGEVIEWMODE)
	{
		IfOptCommonPanel();
		size_t Size=Info.PanelControl(this,FCTL_GETCOLUMNTYPES,0,NULL);
		wchar_t* ColumnTypes=new wchar_t[Size];
		Info.PanelControl(this,FCTL_GETCOLUMNTYPES,static_cast<int>(Size),ColumnTypes);
		int UpdateOwners=IsOwnersDisplayed(ColumnTypes) && !LastOwnersRead;
		int UpdateLinks=IsLinksDisplayed(ColumnTypes) && !LastLinksRead;
		delete[] ColumnTypes;

		if (UpdateOwners || UpdateLinks)
		{
			UpdateItems(UpdateOwners,UpdateLinks);
			Info.PanelControl(this,FCTL_UPDATEPANEL,TRUE,NULL);
			Info.PanelControl(this,FCTL_REDRAWPANEL,0,NULL);
		}
	}

	return(FALSE);
}


bool TmpPanel::IsCurrentFileCorrect(wchar_t **pCurFileName)
{
	struct PanelInfo PInfo;
	const wchar_t *CurFileName=NULL;

	if (pCurFileName)
		*pCurFileName = NULL;

	Info.PanelControl(this,FCTL_GETPANELINFO,0,&PInfo);
	size_t Size=Info.PanelControl(this,FCTL_GETPANELITEM,PInfo.CurrentItem,0);
	PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

	if (PPI)
	{
		FarGetPluginPanelItem gpi={Size, PPI};
		Info.PanelControl(this,FCTL_GETPANELITEM,PInfo.CurrentItem,&gpi);
		CurFileName = PPI->FileName;
	}
	else
	{
		return false;
	}

	bool IsCorrectFile = false;

	if (lstrcmp(CurFileName, L"..") == 0)
	{
		IsCorrectFile = true;
	}
	else
	{
		PluginPanelItem TempFindData;
		IsCorrectFile=GetFileInfoAndValidate(CurFileName,&TempFindData,FALSE);
	}

	if (pCurFileName)
	{
		*pCurFileName = (wchar_t *) malloc((lstrlen(CurFileName)+1)*sizeof(wchar_t));
		lstrcpy(*pCurFileName, CurFileName);
	}

	free(PPI);
	return IsCorrectFile;
}

int TmpPanel::ProcessKey(const INPUT_RECORD *Rec)
{
	if (!(Rec->EventType == KEY_EVENT || Rec->EventType == FARMACRO_KEY_EVENT))
		return FALSE;

	int Key=Rec->Event.KeyEvent.wVirtualKeyCode;
	unsigned int ControlState=Rec->Event.KeyEvent.dwControlKeyState;

	if (ControlState==(SHIFT_PRESSED|LEFT_ALT_PRESSED) && Key==VK_F3)
	{
		PtrGuard CurFileName;

		if (IsCurrentFileCorrect(CurFileName.PtrPtr()))
		{
			struct PanelInfo PInfo;
			Info.PanelControl(this,FCTL_GETPANELINFO,0,&PInfo);

			if (lstrcmp(CurFileName,L"..")!=0)
			{
				size_t Size=Info.PanelControl(this,FCTL_GETPANELITEM,PInfo.CurrentItem,0);
				PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);
				DWORD attributes=0;

				if (PPI)
				{
					FarGetPluginPanelItem gpi={Size, PPI};
					Info.PanelControl(this,FCTL_GETPANELITEM,PInfo.CurrentItem,&gpi);
					attributes=PPI->FileAttributes;
					free(PPI);
				}

				if (attributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					Info.PanelControl(PANEL_PASSIVE, FCTL_SETPANELDIR,0,CurFileName.Ptr());
				}
				else
				{
					GoToFile(CurFileName, true);
				}

				Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);
				return(TRUE);
			}
		}
	}

	if (ControlState!=LEFT_CTRL_PRESSED && Key>=VK_F3 && Key<=VK_F8 && Key!=VK_F7)
	{
		if (!IsCurrentFileCorrect(NULL))
			return(TRUE);
	}

	if (ControlState==0 && Key==VK_RETURN && Opt.AnyInPanel)
	{
		PtrGuard CurFileName;

		if (!IsCurrentFileCorrect(CurFileName.PtrPtr()))
		{
			Info.PanelControl(this,FCTL_SETCMDLINE,0,CurFileName.Ptr());
			return(TRUE);
		}
	}

	if (Opt.SafeModePanel && ControlState == LEFT_CTRL_PRESSED && Key == VK_PRIOR)
	{
		PtrGuard CurFileName;

		if (IsCurrentFileCorrect(CurFileName.PtrPtr()))
		{
			if (lstrcmp(CurFileName,L".."))
			{
				GoToFile(CurFileName, false);
				return TRUE;
			}
		}

		if (CurFileName.Ptr() && !lstrcmp(CurFileName,L".."))
		{
			SetDirectory(L".",0);
			return TRUE;
		}
	}

	if (ControlState==0 && Key==VK_F7)
	{
		ProcessRemoveKey();
		return TRUE;
	}
	else if (ControlState == (SHIFT_PRESSED|LEFT_ALT_PRESSED) && Key == VK_F2)
	{
		ProcessSaveListKey();
		return TRUE;
	}
	else
	{
		if (StartupOptCommonPanel && ControlState==(SHIFT_PRESSED|LEFT_ALT_PRESSED))
		{
			if (Key==VK_F12)
			{
				ProcessPanelSwitchMenu();
				return(TRUE);
			}
			else if (Key >= L'0' && Key <= L'9')
			{
				SwitchToPanel(Key - L'0');
				return TRUE;
			}
		}
	}

	return(FALSE);
}


void TmpPanel::ProcessRemoveKey()
{
	IfOptCommonPanel();
	struct PanelInfo PInfo;
	Info.PanelControl(this,FCTL_GETPANELINFO,0,&PInfo);

	for (int i=0; i<PInfo.SelectedItemsNumber; i++)
	{
		struct PluginPanelItem *RemovedItem=NULL;
		size_t Size=Info.PanelControl(this,FCTL_GETSELECTEDPANELITEM,i,0);
		PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

		if (PPI)
		{
			FarGetPluginPanelItem gpi={Size, PPI};
			Info.PanelControl(this,FCTL_GETSELECTEDPANELITEM,i,&gpi);
			RemovedItem = TmpPanelItem + PPI->UserData;
		}

		if (RemovedItem!=NULL)
			RemovedItem->Flags|=REMOVE_FLAG;

		free(PPI);
	}

	RemoveEmptyItems();
	Info.PanelControl(this,FCTL_UPDATEPANEL,0,NULL);
	Info.PanelControl(this,FCTL_REDRAWPANEL,0,NULL);
	Info.PanelControl(PANEL_PASSIVE,FCTL_GETPANELINFO,0,&PInfo);

	if (PInfo.PanelType==PTYPE_QVIEWPANEL)
	{
		Info.PanelControl(PANEL_PASSIVE,FCTL_UPDATEPANEL,0,NULL);
		Info.PanelControl(PANEL_PASSIVE,FCTL_REDRAWPANEL,0,NULL);
	}
}

void TmpPanel::ProcessSaveListKey()
{
	IfOptCommonPanel();

	if (TmpItemsNumber == 0)
		return;

	// default path: opposite panel directory\panel<index>.<mask extension>
	StrBuf ListPath(NT_MAX_PATH+20+512);
	Info.PanelControl(PANEL_PASSIVE,FCTL_GETPANELDIR,NT_MAX_PATH,ListPath.Ptr());
	FSF.AddEndSlash(ListPath);
	lstrcat(ListPath, L"panel");

	if (Opt.CommonPanel)
		FSF.itoa(PanelIndex, ListPath.Ptr() + lstrlen(ListPath), 10);

	wchar_t ExtBuf [512];
	lstrcpy(ExtBuf, Opt.Mask);
	wchar_t *comma = _tcschr(ExtBuf, L',');

	if (comma)
		*comma = L'\0';

	wchar_t *ext = _tcschr(ExtBuf, L'.');

	if (ext && !_tcschr(ext, L'*') && !_tcschr(ext, L'?'))
		lstrcat(ListPath, ext);

	if (Info.InputBox(&MainGuid, GetMsg(MTempPanel), GetMsg(MListFilePath),
	                  L"TmpPanel.SaveList", ListPath, ListPath, ListPath.Size()-1,
	                  NULL, FIB_BUTTONS))
	{
		SaveListFile(ListPath);
		Info.PanelControl(PANEL_PASSIVE, FCTL_UPDATEPANEL,0,NULL);
		Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);
	}

#undef _HANDLE
#undef _UPDATE
#undef _REDRAW
#undef _GET
}

void TmpPanel::SaveListFile(const wchar_t *Path)
{
	IfOptCommonPanel();

	if (!TmpItemsNumber)
		return;

	StrBuf FullPath;
	GetFullPath(Path, FullPath);
	StrBuf NtPath;
	FormNtPath(FullPath, NtPath);
	HANDLE hFile = CreateFile(NtPath, GENERIC_WRITE, 0, NULL,
	                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		const wchar_t *Items[] = { GetMsg(MError) };
		Info.Message(&MainGuid, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK, NULL, Items, 1, 0);
		return;
	}

	DWORD BytesWritten;
	static const unsigned short bom = BOM_UCS2;
	WriteFile(hFile, &bom, sizeof(bom), &BytesWritten, NULL);
	int i = 0;

	do
	{
		static const wchar_t *CRLF = L"\r\n";
		const wchar_t *FName = TmpPanelItem[i].FileName;
		WriteFile(hFile, FName, sizeof(wchar_t)*lstrlen(FName), &BytesWritten, NULL);
		WriteFile(hFile, CRLF, 2*sizeof(wchar_t), &BytesWritten, NULL);
	}
	while (++i < TmpItemsNumber);

	CloseHandle(hFile);
}

void TmpPanel::SwitchToPanel(int NewPanelIndex)
{
	if ((unsigned)NewPanelIndex<COMMONPANELSNUMBER && NewPanelIndex!=(int)PanelIndex)
	{
		CommonPanels[PanelIndex].Items=TmpPanelItem;
		CommonPanels[PanelIndex].ItemsNumber=(UINT)TmpItemsNumber;

		if (!CommonPanels[NewPanelIndex].Items)
		{
			CommonPanels[NewPanelIndex].ItemsNumber=0;
			CommonPanels[NewPanelIndex].Items=(PluginPanelItem*)calloc(1,sizeof(PluginPanelItem));
		}

		if (CommonPanels[NewPanelIndex].Items)
		{
			CurrentCommonPanel = PanelIndex = NewPanelIndex;
			Info.PanelControl(this,FCTL_UPDATEPANEL,0,NULL);
			Info.PanelControl(this,FCTL_REDRAWPANEL,0,NULL);
		}
	}
}


void TmpPanel::ProcessPanelSwitchMenu()
{
	FarMenuItem fmi[COMMONPANELSNUMBER];
	memset(&fmi,0,sizeof(FarMenuItem)*COMMONPANELSNUMBER);
	const wchar_t *txt=GetMsg(MSwitchMenuTxt);
	wchar_t tmpstr[COMMONPANELSNUMBER][128];
	static const wchar_t fmt1[]=L"&%c. %s %d";

	for (unsigned int i=0; i<COMMONPANELSNUMBER; ++i)
	{
		fmi[i].Text = tmpstr[i];

		if (i<10)
			FSF.sprintf(tmpstr[i],fmt1,L'0'+i,txt,CommonPanels[i].ItemsNumber);
		else if (i<36)
			FSF.sprintf(tmpstr[i],fmt1,L'A'-10+i,txt,CommonPanels[i].ItemsNumber);
		else
			FSF.sprintf(tmpstr[i],L"   %s %d",txt,CommonPanels[i].ItemsNumber);

	}

	fmi[PanelIndex].Flags|=MIF_SELECTED;
	int ExitCode=Info.Menu(&MainGuid,-1,-1,0,
	                       FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
	                       GetMsg(MSwitchMenuTitle),NULL,NULL,
	                       NULL,NULL,fmi,COMMONPANELSNUMBER);
	SwitchToPanel(ExitCode);
}

int TmpPanel::IsOwnersDisplayed(LPCTSTR ColumnTypes)
{
	for (int i=0; ColumnTypes[i]; i++)
		if (ColumnTypes[i]==L'O' && (i==0 || ColumnTypes[i-1]==L',') &&
		        (ColumnTypes[i+1]==L',' || ColumnTypes[i+1]==0))
			return(TRUE);

	return(FALSE);
}


int TmpPanel::IsLinksDisplayed(LPCTSTR ColumnTypes)
{
	for (int i=0; ColumnTypes[i]; i++)
		if (ColumnTypes[i]==L'L' && ColumnTypes[i+1]==L'N' &&
		        (i==0 || ColumnTypes[i-1]==L',') &&
		        (ColumnTypes[i+2]==L',' || ColumnTypes[i+2]==0))
			return(TRUE);

	return(FALSE);
}

inline bool isDevice(const wchar_t* FileName, const wchar_t* dev_begin)
{
	const int len=(int)lstrlen(dev_begin);

	if (FSF.LStrnicmp(FileName, dev_begin, len)) return false;

	FileName+=len;

	if (!*FileName) return false;

	while (*FileName>=L'0' && *FileName<=L'9') FileName++;

	return !*FileName;
}

bool TmpPanel::GetFileInfoAndValidate(const wchar_t *FilePath, /*FAR_FIND_DATA*/PluginPanelItem* FindData, int Any)
{
	StrBuf ExpFilePath;
	ExpandEnvStrs(FilePath,ExpFilePath);
	wchar_t* FileName = ExpFilePath;
	ParseParam(FileName);
	StrBuf FullPath;
	GetFullPath(FileName, FullPath);
	StrBuf NtPath;
	FormNtPath(FullPath, NtPath);

	if (!FSF.LStrnicmp(FileName, L"\\\\.\\", 4) && FSF.LIsAlpha(FileName[4]) && FileName[5]==L':' && FileName[6]==0)
	{
copy_name_set_attr:
		FindData->FileAttributes = FILE_ATTRIBUTE_ARCHIVE;
copy_name:
		if (FindData->FileName)
			free((void*)FindData->FileName);

		FindData->FileName = wcsdup(FileName);
		return(TRUE);
	}

	if (isDevice(FileName, L"\\\\.\\PhysicalDrive") || isDevice(FileName, L"\\\\.\\cdrom"))
		goto copy_name_set_attr;

	if (lstrlen(FileName))
	{
		DWORD dwAttr=GetFileAttributes(NtPath);

		if (dwAttr!=INVALID_FILE_ATTRIBUTES)
		{
			WIN32_FIND_DATA wfd;
			HANDLE fff=FindFirstFile(NtPath, &wfd);

			if (fff != INVALID_HANDLE_VALUE)
			{
				WFD2FFD(wfd,*FindData);
				FindClose(fff);
				FileName = FullPath;
				goto copy_name;
			}
			else
			{
				wfd.dwFileAttributes=dwAttr;
				HANDLE hFile=CreateFile(NtPath,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_POSIX_SEMANTICS,NULL);

				if (hFile!=INVALID_HANDLE_VALUE)
				{
					GetFileTime(hFile, &wfd.ftCreationTime, &wfd.ftLastAccessTime, &wfd.ftLastWriteTime);
					wfd.nFileSizeLow = GetFileSize(hFile, &wfd.nFileSizeHigh);
					CloseHandle(hFile);
				}

				wfd.dwReserved0=0;
				wfd.dwReserved1=0;
				WFD2FFD(wfd, *FindData);
				FileName = FullPath;
				goto copy_name;
			}
		}

		if (Any)
			goto copy_name_set_attr;
	}

	return(FALSE);
}


void TmpPanel::IfOptCommonPanel(void)
{
	if (StartupOptCommonPanel)
	{
		TmpPanelItem=CommonPanels[PanelIndex].Items;
		TmpItemsNumber=CommonPanels[PanelIndex].ItemsNumber;
	}
}
