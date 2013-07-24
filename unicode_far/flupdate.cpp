/*
flupdate.cpp

�������� ������ - ������ ���� ������
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

#include "filelist.hpp"
#include "flink.hpp"
#include "colors.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "filefilter.hpp"
#include "hilight.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "fileowner.hpp"
#include "delete.hpp"
#include "pathmix.hpp"
#include "network.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "colormix.hpp"
#include "plugins.hpp"

// ����� ��� ReadDiz()
enum ReadDizFlags
{
	RDF_NO_UPDATE         = 0x00000001UL,
};

void FileList::Update(int Mode)
{
	_ALGO(CleverSysLog clv(L"FileList::Update"));
	_ALGO(SysLog(L"(Mode=[%d/0x%08X] %s)",Mode,Mode,(Mode==UPDATE_KEEP_SELECTION?L"UPDATE_KEEP_SELECTION":L"")));

	if (EnableUpdate)
		switch (PanelMode)
		{
			case NORMAL_PANEL:
				ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				break;
			case PLUGIN_PANEL:
			{
				OpenPanelInfo Info;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
				ProcessPluginCommand();

				if (PanelMode!=PLUGIN_PANEL)
					ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				else if ((Info.Flags & OPIF_REALNAMES) ||
				         Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetMode()==PLUGIN_PANEL ||
				         !(Mode & UPDATE_SECONDARY))
					UpdatePlugin(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE);
			}
			ProcessPluginCommand();
			break;
		}

	LastUpdateTime=clock();
}

void FileList::UpdateIfRequired()
{
	if (UpdateRequired && !UpdateDisabled)
	{
		UpdateRequired = FALSE;
		Update(UpdateRequiredMode | UPDATE_IGNORE_VISIBLE);
	}
}

void ReadFileNamesMsg(const string& Msg)
{
	Message(0,0,MSG(MReadingTitleFiles),Msg.data());
	if (!Global->PreRedraw->empty())
	{
		Global->PreRedraw->top().Param.Param1=(void*)Msg.data();
	}
}

static void PR_ReadFileNamesMsg()
{
	if (!Global->PreRedraw->empty())
	{
		ReadFileNamesMsg((wchar_t *)Global->PreRedraw->top().Param.Param1);
	}
}

void FileList::ReadFileNames(int KeepSelection, int UpdateEvenIfPanelInvisible, int DrawMessage)
{
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_ReadFileNamesMsg);
	TaskBar TB(false);

	strOriginalCurDir = strCurDir;

	if (!IsVisible() && !UpdateEvenIfPanelInvisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	UpdateRequired=FALSE;
	AccessTimeUpdateRequired=FALSE;
	DizRead=FALSE;
	FAR_FIND_DATA fdata;
	decltype(ListData) OldData;
	string strCurName, strNextCurName;
	StopFSWatcher();

	if (this!=Global->CtrlObject->Cp()->LeftPanel && this!=Global->CtrlObject->Cp()->RightPanel)
		return;

	string strSaveDir;
	apiGetCurrentDirectory(strSaveDir);
	{
		string strOldCurDir(strCurDir);

		if (!SetCurPath())
		{
			FlushInputBuffer(); // ������� ������ �����, �.�. �� ��� ����� ���� � ������ �����...

			if (strCurDir == strOldCurDir) //?? i??
			{
				GetPathRoot(strOldCurDir,strOldCurDir);

				if (!apiIsDiskInDrive(strOldCurDir))
					IfGoHome(strOldCurDir.front());

				/* ��� ����� �������� ���� �� ��������� */
			}

			return;
		}
	}
	SortGroupsRead=FALSE;

	if (GetFocus())
		Global->CtrlObject->CmdLine->SetCurDir(strCurDir);

	LastCurFile=-1;
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	AnotherPanel->QViewDelTempName();
	size_t PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	FreeDiskSize = -1;
	if (Global->Opt->ShowPanelFree)
	{
		apiGetDiskSize(strCurDir, nullptr, nullptr, &FreeDiskSize);
	}

	if (!ListData.empty())
	{
		strCurName = ListData[CurFile]->strName;

		if (ListData[CurFile]->Selected && !ReturnCurrentFile)
		{
			for (size_t i=CurFile+1; i < ListData.size(); i++)
			{
				if (!ListData[i]->Selected)
				{
					strNextCurName = ListData[i]->strName;
					break;
				}
			}
		}
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData = std::move(ListData);
	}
	else
		DeleteListData(ListData);

	DWORD FileSystemFlags = 0;
	string PathRoot;
	GetPathRoot(strCurDir, PathRoot);
	apiGetVolumeInformation(PathRoot, nullptr, nullptr, nullptr, &FileSystemFlags, nullptr);

	ListData.clear();

	bool ReadOwners = IsColumnDisplayed(OWNER_COLUMN);
	bool ReadNumLinks = IsColumnDisplayed(NUMLINK_COLUMN);
	bool ReadNumStreams = IsColumnDisplayed(NUMSTREAMS_COLUMN);
	bool ReadStreamsSize = IsColumnDisplayed(STREAMSSIZE_COLUMN);

	if (!(FileSystemFlags&FILE_SUPPORTS_HARD_LINKS) && Global->WinVer() >= _WIN32_WINNT_WIN7)
	{
		ReadNumLinks = false;
	}

	if(!(FileSystemFlags&FILE_NAMED_STREAMS))
	{
		ReadNumStreams = false;
		ReadStreamsSize = false;
	}

	string strComputerName;

	if (ReadOwners)
	{
		CurPath2ComputerName(strCurDir, strComputerName);
		// ������� ��� SID`��
		SIDCacheFlush();
	}

	SetLastError(ERROR_SUCCESS);
	// ���������� ��������� ��� �����
	string Title = MakeSeparator(X2-X1-1, 9, nullptr);
	BOOL IsShowTitle=FALSE;
	BOOL NeedHighlight=Global->Opt->Highlight && PanelMode != PLUGIN_PANEL;

	if (!Filter)
		Filter.reset(new FileFilter(this,FFT_PANEL));

	//������ �������� ������� ��� ������� ����� ������� ��������
	Filter->UpdateCurrentTime();
	Global->CtrlObject->HiFiles->UpdateCurrentTime();
	bool bCurDirRoot = false;
	ParsePath(strCurDir, nullptr, &bCurDirRoot);
	PATH_TYPE Type = ParsePath(strCurDir, nullptr, &bCurDirRoot);
	bool NetRoot = bCurDirRoot && (Type == PATH_REMOTE || Type == PATH_REMOTEUNC);

	string strFind(strCurDir);
	AddEndSlash(strFind);
	strFind+=L'*';
	::FindFile Find(strFind, true);
	DWORD FindErrorCode = ERROR_SUCCESS;
	bool UseFilter=Filter->IsEnabledOnPanel();
	bool ReadCustomData=IsColumnDisplayed(CUSTOM_COLUMN0)!=0;

	DWORD StartTime = GetTickCount();

	while (Find.Get(fdata))
	{
		Global->CatchError();
		FindErrorCode = Global->CaughtError();

		if ((Global->Opt->ShowHidden || !(fdata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))) && (!UseFilter || Filter->FileInFilter(fdata, nullptr, &fdata.strFileName)))
		{
			if (ListData.size() == ListData.capacity())
				ListData.reserve(ListData.size() + 4096);

			auto NewItem = new FileListItem;
			NewItem->FileAttr = fdata.dwFileAttributes;
			NewItem->CreationTime = fdata.ftCreationTime;
			NewItem->AccessTime = fdata.ftLastAccessTime;
			NewItem->WriteTime = fdata.ftLastWriteTime;
			NewItem->ChangeTime = fdata.ftChangeTime;
			NewItem->FileSize = fdata.nFileSize;
			NewItem->AllocationSize = fdata.nAllocationSize;
			NewItem->strName = fdata.strFileName;
			NewItem->strShortName = fdata.strAlternateFileName;
			NewItem->Position = ListData.size();
			NewItem->NumberOfLinks=1;

			if (fdata.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				NewItem->ReparseTag=fdata.dwReserved0; //MSDN
			}
			if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				TotalFileSize += NewItem->FileSize;

				if (ReadNumLinks)
					NewItem->NumberOfLinks = GetNumberOfLinks(fdata.strFileName, true);
			}
			else
			{
				NewItem->AllocationSize = 0;
			}

			NewItem->SortGroup=DEFAULT_SORT_GROUP;

			if (ReadOwners)
			{
				string strOwner;
				GetFileOwner(strComputerName, NewItem->strName,strOwner);
				NewItem->strOwner = strOwner;
			}

			NewItem->NumberOfStreams=NewItem->FileAttr&FILE_ATTRIBUTE_DIRECTORY?0:1;
			NewItem->StreamsSize=NewItem->FileSize;

			if (ReadNumStreams||ReadStreamsSize)
			{
				EnumStreams(TestParentFolderName(fdata.strFileName)? strCurDir : fdata.strFileName, NewItem->StreamsSize, NewItem->NumberOfStreams);
			}

			if (ReadCustomData)
				NewItem->strCustomData = Global->CtrlObject->Plugins->GetCustomData(NewItem->strName);

			ListData.emplace_back(NewItem);

			if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				TotalFileCount++;

			DWORD CurTime = GetTickCount();
			if (CurTime - StartTime > (DWORD)Global->Opt->RedrawTimeout)
			{
				StartTime = CurTime;
				if (IsVisible())
				{
					if (!IsShowTitle)
					{
						if (!DrawMessage)
						{
							Text(X1+1,Y1,ColorIndexToColor(COL_PANELBOX),Title);
							IsShowTitle=TRUE;
							SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
						}
					}

					LangString strReadMsg(MReadingFiles);
					strReadMsg << ListData.size();

					if (DrawMessage)
					{
						ReadFileNamesMsg(strReadMsg);
					}
					else
					{
						TruncStr(strReadMsg,static_cast<int>(Title.size())-2);
						int MsgLength=(int)strReadMsg.size();
						GotoXY(X1+1+(static_cast<int>(Title.size())-MsgLength-1)/2,Y1);
						Global->FS << L" "<<strReadMsg<<L" ";
					}
				}

				Global->CtrlObject->Macro.SuspendMacros(true);
				bool check = CheckForEsc();
				Global->CtrlObject->Macro.SuspendMacros(false);
				if (check)
					break;
			}
		}
	}

	if (!(FindErrorCode==ERROR_SUCCESS || FindErrorCode==ERROR_NO_MORE_FILES || FindErrorCode==ERROR_FILE_NOT_FOUND))
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MReadFolderError),MSG(MOk));

	if ((Global->Opt->ShowDotsInRoot || !bCurDirRoot) || (NetRoot && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Network))) // NetWork Plugin
	{
		auto NewItem = new FileListItem;

		string TwoDotsOwner;
		if (ReadOwners)
		{
			GetFileOwner(strComputerName,strCurDir,TwoDotsOwner);
		}

		FILETIME TwoDotsTimes[4]={};
		GetFileTimeSimple(strCurDir,&TwoDotsTimes[0],&TwoDotsTimes[1],&TwoDotsTimes[2],&TwoDotsTimes[3]);

		AddParentPoint(NewItem, ListData.size(), TwoDotsTimes, TwoDotsOwner);

		ListData.emplace_back(NewItem);
	}

	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz();

	if (NeedHighlight)
	{
		std::for_each(RANGE(ListData, i)
		{
			Global->CtrlObject->HiFiles->GetHiColor(i.get());
		});
	}

	if (AnotherPanel->GetMode()==PLUGIN_PANEL)
	{
		HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
		PluginPanelItem *PanelData=nullptr;
		string strPath(strCurDir);
		AddEndSlash(strPath);
		size_t PanelCount=0;

		if (Global->CtrlObject->Plugins->GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,strPath))
		{
			auto OldSize = ListData.size(), Position = OldSize - 1;
			ListData.resize(ListData.size() + PanelCount);

			auto PluginPtr = PanelData;
			for (auto i = ListData.begin() + OldSize; i != ListData.end(); ++i, ++PluginPtr, ++Position)
			{
				PluginToFileListItem(PluginPtr, i->get());
				(*i)->Position = Position;
				TotalFileSize += PluginPtr->FileSize;
				(*i)->PrevSelected = (*i)->Selected=0;
				(*i)->ShowFolderSize = 0;
				(*i)->SortGroup=Global->CtrlObject->HiFiles->GetGroup(i->get());

				if (!TestParentFolderName(PluginPtr->FileName) && !((*i)->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
					TotalFileCount++;
			}

			// �������� ������ ��������� � ����� �����, �� ���� ���
			std::for_each(ListData.begin() + OldSize, ListData.begin() + OldSize + PanelCount, LAMBDA_PREDICATE(ListData, i)
			{
				Global->CtrlObject->HiFiles->GetHiColor(i.get());
			});
			Global->CtrlObject->Plugins->FreeVirtualFindData(hAnotherPlugin,PanelData,PanelCount);
		}
	}

	InitFSWatcher(false);
	CorrectPosition();

	string strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && static_cast<size_t>(LastSelPosition) < OldData.size())
			strLastSel = OldData[LastSelPosition]->strName;
		if (GetSelPosition >= 0 && static_cast<size_t>(GetSelPosition) < OldData.size())
			strGetSel = OldData[GetSelPosition]->strName;

		MoveSelection(OldData, ListData);
		DeleteListData(OldData);
	}

	if (SortGroups)
		ReadSortGroups(false);

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (!strLastSel.empty())
		LastSelPosition = FindFile(strLastSel, FALSE);
	if (!strGetSel.empty())
		GetSelPosition = FindFile(strGetSel, FALSE);

	if (CurFile >= static_cast<int>(ListData.size()) || StrCmpI(ListData[CurFile]->strName.data(), strCurName.data()))
		if (!GoToFile(strCurName) && !strNextCurName.empty())
			GoToFile(strNextCurName);

	/* $ 13.02.2002 DJ
		SetTitle() - ������ ���� �� ������� �����!
	*/
	if (Global->CtrlObject->Cp() == FrameManager->GetCurrentFrame())
		SetTitle();

	FarChDir(strSaveDir); //???
}

/*$ 22.06.2001 SKV
  �������� �������� ��� ������ ����� ���������� �������.
*/
int FileList::UpdateIfChanged(panel_update_mode UpdateMode)
{
	//_SVS(SysLog(L"CurDir='%s' Global->Opt->AutoUpdateLimit=%d <= FileCount=%d",CurDir,Global->Opt->AutoUpdateLimit,FileCount));
	if (!Global->Opt->AutoUpdateLimit || ListData.size() <= static_cast<size_t>(Global->Opt->AutoUpdateLimit))
	{
		/* $ 19.12.2001 VVM
		  ! ������ ����������. ��� Force ���������� ������! */
		if ((IsVisible() && (clock()-LastUpdateTime>2000)) || (UpdateMode != UIC_UPDATE_NORMAL))
		{
			if (UpdateMode == UIC_UPDATE_NORMAL)
				ProcessPluginEvent(FE_IDLE,nullptr);

			/* $ 24.12.2002 VVM
			  ! �������� ������ ���������� �������. */
			if (// ���������� ������, �� ��� ����������� ����������� � ���� ������
			    (PanelMode==NORMAL_PANEL && FSWatcher.Signaled()) ||
			    // ��� ���������� ������, �� ��� ����������� � �� ��������� �������� ����� UPDATE_FORCE
			    (PanelMode==NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE) ||
			    // ��� ��������� ������ � ��������� ����� UPDATE_FORCE
			    (PanelMode!=NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE)
			)
			{
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

				if (AnotherPanel->GetType()==INFO_PANEL)
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					if (UpdateMode==UIC_UPDATE_NORMAL)
						AnotherPanel->Redraw();
				}

				Update(UPDATE_KEEP_SELECTION);

				if (UpdateMode==UIC_UPDATE_NORMAL)
					Show();

				return TRUE;
			}
		}
	}

	return FALSE;
}

void FileList::InitFSWatcher(bool CheckTree)
{
	DWORD DriveType=DRIVE_REMOTE;
	StopFSWatcher();
	PATH_TYPE Type = ParsePath(strCurDir);

	if (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC)
	{
		wchar_t RootDir[4]=L" :\\";
		RootDir[0] = strCurDir[(Type == PATH_DRIVELETTER)? 0 : 4];
		DriveType=FAR_GetDriveType(RootDir);
	}

	if (Global->Opt->AutoUpdateRemoteDrive || (!Global->Opt->AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE) || Type == PATH_VOLUMEGUID)
	{
		FSWatcher.Set(strCurDir, CheckTree);
		StartFSWatcher(false, false); //check_time=false, prevent reading file time twice (slow on network)
	}
}

void FileList::StartFSWatcher(bool got_focus, bool check_time)
{
	FSWatcher.Watch(got_focus, check_time);
}

void FileList::StopFSWatcher()
{
	FSWatcher.Release();
}

struct search_list_less
{
	bool operator()(const std::unique_ptr<FileListItem>& a, const std::unique_ptr<FileListItem>& b) const
	{
		return a->strName < b->strName;
	}
}
SearchListLess;

void FileList::MoveSelection(std::vector<std::unique_ptr<FileListItem>>& From, std::vector<std::unique_ptr<FileListItem>>& To)
{
	SelFileCount=0;
	SelFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;

	std::sort(From.begin(), From.end(), SearchListLess);

	std::for_each(CONST_RANGE(To, i)
	{
		auto Iterator = std::lower_bound(ALL_CONST_RANGE(From), i, SearchListLess);
		if (Iterator != From.end())
		{
			auto OldItem = Iterator->get();
			if (OldItem->strName == i->strName)
			{
				if (OldItem->ShowFolderSize)
				{
					i->ShowFolderSize = 2;
					i->FileSize = OldItem->FileSize;
					i->AllocationSize = OldItem->AllocationSize;
				}

				this->Select(i.get(), OldItem->Selected);
				i->PrevSelected = OldItem->PrevSelected;
			}
		}
	});
}

void FileList::UpdatePlugin(int KeepSelection, int UpdateEvenIfPanelInvisible)
{
	_ALGO(CleverSysLog clv(L"FileList::UpdatePlugin"));
	_ALGO(SysLog(L"(KeepSelection=%d, IgnoreVisible=%d)",KeepSelection,UpdateEvenIfPanelInvisible));

	if (!IsVisible() && !UpdateEvenIfPanelInvisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	DizRead=FALSE;
	std::vector<std::unique_ptr<FileListItem>> OldData;
	string strCurName, strNextCurName;
	StopFSWatcher();
	LastCurFile=-1;
	OpenPanelInfo Info;
	Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

	FreeDiskSize=-1;
	if (Global->Opt->ShowPanelFree)
	{
		if (Info.Flags & OPIF_REALNAMES)
		{
			apiGetDiskSize(strCurDir, nullptr, nullptr, &FreeDiskSize);
		}
		else if (Info.Flags & OPIF_USEFREESIZE)
			FreeDiskSize=Info.FreeSize;
	}

	PluginPanelItem *PanelData=nullptr;
	size_t PluginFileCount;

	if (!Global->CtrlObject->Plugins->GetFindData(hPlugin,&PanelData,&PluginFileCount,0))
	{
		PopPlugin(TRUE);
		Update(KeepSelection);

		// WARP> ����� ���, �� ����� ������������ - ��������������� ������� �� ������ ��� ������ ������ ������.
		if (!PrevDataList.empty())
			GoToFile(PrevDataList.back().strPrevName);

		return;
	}

	size_t PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	strPluginDizName.clear();

	if (!ListData.empty())
	{
		auto CurPtr=ListData[CurFile].get();
		strCurName = CurPtr->strName;

		if (CurPtr->Selected)
		{
			for (size_t i = CurFile + 1; i < ListData.size(); ++i)
			{
				CurPtr = ListData[i].get();

				if (!CurPtr->Selected)
				{
					strNextCurName = CurPtr->strName;
					break;
				}
			}
		}
	}
	else if (Info.Flags & OPIF_ADDDOTS)
	{
		strCurName = L"..";
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData = std::move(ListData);
	}
	else
	{
		DeleteListData(ListData);
	}

	ListData.resize(PluginFileCount);

	if (!Filter)
		Filter.reset(new FileFilter(this, FFT_PANEL));

	//������ �������� ������� ��� ������� ����� ������� ��������
	Filter->UpdateCurrentTime();
	Global->CtrlObject->HiFiles->UpdateCurrentTime();
	int DotsPresent=FALSE;
	int FileListCount=0;
	bool UseFilter=Filter->IsEnabledOnPanel();

	for (size_t i = 0; i < ListData.size(); i++)
	{
		ListData[FileListCount].reset(new FileListItem);
		FileListItem& CurListData = *ListData[FileListCount].get();

		if (UseFilter && !(Info.Flags & OPIF_DISABLEFILTER))
		{
			//if (!(CurPanelData->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			if (!Filter->FileInFilter(PanelData[i]))
				continue;
		}

		if (!Global->Opt->ShowHidden && (PanelData[i].FileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			continue;

		//ClearStruct(*CurListData);
		PluginToFileListItem(&PanelData[i], &CurListData);
		CurListData.Position=i;

		if (!(Info.Flags & OPIF_DISABLESORTGROUPS)/* && !(CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY)*/)
			CurListData.SortGroup=Global->CtrlObject->HiFiles->GetGroup(&CurListData);
		else
			CurListData.SortGroup=DEFAULT_SORT_GROUP;

		if (!CurListData.DizText)
		{
			CurListData.DeleteDiz=FALSE;
			//CurListData.DizText=nullptr;
		}

		if (TestParentFolderName(CurListData.strName))
		{
			DotsPresent=TRUE;
			CurListData.FileAttr|=FILE_ATTRIBUTE_DIRECTORY;
		}
		else if (!(CurListData.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			TotalFileCount++;
		}

		TotalFileSize += CurListData.FileSize;
		FileListCount++;
	}

	ListData.resize(FileListCount);

	if (!(Info.Flags & OPIF_DISABLEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
	{
		std::for_each(RANGE(ListData, i)
		{
			Global->CtrlObject->HiFiles->GetHiColor(i.get(), (Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);
		});
	}

	if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
	{
		auto NewItem = new FileListItem;
		AddParentPoint(NewItem, ListData.size());
		ListData.emplace_back(NewItem);

		if (!(Info.Flags & OPIF_DISABLEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
			Global->CtrlObject->HiFiles->GetHiColor(ListData.back().get(), (Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

		if (Info.HostFile && *Info.HostFile)
		{
			FAR_FIND_DATA FindData;

			if (apiGetFindDataEx(Info.HostFile, FindData))
			{
				NewItem->WriteTime=FindData.ftLastWriteTime;
				NewItem->CreationTime=FindData.ftCreationTime;
				NewItem->AccessTime=FindData.ftLastAccessTime;
				NewItem->ChangeTime=FindData.ftChangeTime;
			}
		}
	}

	if (CurFile >= static_cast<int>(ListData.size()))
		CurFile = ListData.size() ? static_cast<int>(ListData.size() - 1) : 0;

	/* $ 25.02.2001 VVM
	    ! �� ��������� �������� ������ ������ � ������ ������� */
	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz(PanelData,static_cast<int>(PluginFileCount),RDF_NO_UPDATE);

	CorrectPosition();
	Global->CtrlObject->Plugins->FreeFindData(hPlugin,PanelData,PluginFileCount,false);

	string strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && LastSelPosition < static_cast<long>(OldData.size()))
			strLastSel = OldData[LastSelPosition]->strName;
		if (GetSelPosition >= 0 && GetSelPosition < static_cast<long>(OldData.size()))
			strGetSel = OldData[GetSelPosition]->strName;

		MoveSelection(OldData, ListData);
		DeleteListData(OldData);
	}

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (!strLastSel.empty())
		LastSelPosition = FindFile(strLastSel, FALSE);
	if (!strGetSel.empty())
		GetSelPosition = FindFile(strGetSel, FALSE);

	if (CurFile >= static_cast<int>(ListData.size()) || StrCmpI(ListData[CurFile]->strName.data(),strCurName.data()))
		if (!GoToFile(strCurName) && !strNextCurName.empty())
			GoToFile(strNextCurName);

	SetTitle();
}


void FileList::ReadDiz(PluginPanelItem *ItemList,int ItemLength,DWORD dwFlags)
{
	if (DizRead)
		return;

	DizRead=TRUE;
	Diz.Reset();

	if (PanelMode==NORMAL_PANEL)
	{
		Diz.Read(strCurDir);
	}
	else
	{
		PluginPanelItem *PanelData=nullptr;
		size_t PluginFileCount=0;
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (!Info.DescrFilesNumber)
			return;

		int GetCode=TRUE;

		/* $ 25.02.2001 VVM
		    + ��������� ����� RDF_NO_UPDATE */
		if (!ItemList && !(dwFlags & RDF_NO_UPDATE))
		{
			GetCode=Global->CtrlObject->Plugins->GetFindData(hPlugin,&PanelData,&PluginFileCount,0);
		}
		else
		{
			PanelData=ItemList;
			PluginFileCount=ItemLength;
		}

		if (GetCode)
		{
			for (size_t I=0; I<Info.DescrFilesNumber; I++)
			{
				PluginPanelItem *CurPanelData=PanelData;

				for (size_t J=0; J < PluginFileCount; J++, CurPanelData++)
				{
					string strFileName = CurPanelData->FileName;

					if (!StrCmpI(strFileName.data(),Info.DescrFiles[I]))
					{
						string strTempDir, strDizName;

						if (FarMkTempEx(strTempDir) && apiCreateDirectory(strTempDir,nullptr))
						{
							if (Global->CtrlObject->Plugins->GetFile(hPlugin,CurPanelData,strTempDir,strDizName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW|OPM_DESCR))
							{
								strPluginDizName = Info.DescrFiles[I];
								Diz.Read(L"", &strDizName);
								DeleteFileWithFolder(strDizName);
								I=Info.DescrFilesNumber;
								break;
							}

							apiRemoveDirectory(strTempDir);
							//ViewPanel->ShowFile(nullptr,FALSE,nullptr);
						}
					}
				}
			}

			/* $ 25.02.2001 VVM
			    + ��������� ����� RDF_NO_UPDATE */
			if (!ItemList && !(dwFlags & RDF_NO_UPDATE))
				Global->CtrlObject->Plugins->FreeFindData(hPlugin,PanelData,PluginFileCount,true);
		}
	}

	std::for_each(CONST_RANGE(ListData, i)
	{
		if (!i->DizText)
		{
			i->DeleteDiz = FALSE;
			i->DizText = const_cast<wchar_t*>(Diz.GetDizTextAddr(i->strName, i->strShortName, i->FileSize));
		}
	});
}


void FileList::ReadSortGroups(bool UpdateFilterCurrentTime)
{
	if (!SortGroupsRead)
	{
		if (UpdateFilterCurrentTime)
		{
			Global->CtrlObject->HiFiles->UpdateCurrentTime();
		}

		SortGroupsRead=TRUE;

		std::for_each(CONST_RANGE(ListData, i)
		{
			i->SortGroup=Global->CtrlObject->HiFiles->GetGroup(i.get());
		});
	}
}

// ������� ���������������� ������ ��� �������� "..". ���������, ��� CurPtr ����.
void FileList::AddParentPoint(FileListItem *CurPtr, size_t CurFilePos, const FILETIME* Times, const string& Owner)
{
	CurPtr->FileAttr = FILE_ATTRIBUTE_DIRECTORY;
	CurPtr->strName = L"..";

	if (Times)
	{
		CurPtr->CreationTime = Times[0];
		CurPtr->AccessTime = Times[1];
		CurPtr->WriteTime = Times[2];
		CurPtr->ChangeTime = Times[3];
	}

	CurPtr->strOwner = Owner;
	CurPtr->Position = CurFilePos;
}
