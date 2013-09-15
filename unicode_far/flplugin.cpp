/*
flplugin.cpp

�������� ������ - ������ � ���������
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
#include "filepanels.hpp"
#include "history.hpp"
#include "ctrlobj.hpp"
#include "syslog.hpp"
#include "message.hpp"
#include "config.hpp"
#include "delete.hpp"
#include "datetime.hpp"
#include "dirmix.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "mix.hpp"
#include "plugins.hpp"

/*
   � ����� ������ ������ �� �������� - ������ ����������!
*/

void FileList::PushPlugin(HANDLE hPlugin,const string& HostFile)
{
	PluginsList.emplace_back(VALUE_TYPE(PluginsList)());
	PluginsListItem& stItem = PluginsList.back();
	++Global->PluginPanelsCount;

	stItem.hPlugin=hPlugin;
	stItem.strHostFile = HostFile;
	stItem.strPrevOriginalCurDir = strOriginalCurDir;
	strOriginalCurDir = strCurDir;
	stItem.Modified=FALSE;
	stItem.PrevViewMode=ViewMode;
	stItem.PrevSortMode=SortMode;
	stItem.PrevSortOrder=ReverseSortOrder;
	stItem.PrevNumericSort=NumericSort;
	stItem.PrevCaseSensitiveSort=CaseSensitiveSort;
	stItem.PrevViewSettings=ViewSettings;
	stItem.PrevDirectoriesFirst=DirectoriesFirst;
}


int FileList::PopPlugin(int EnableRestoreViewMode)
{
	DeleteListData(ListData);

	OpenPanelInfo Info={};

	if (PluginsList.empty())
	{
		PanelMode=NORMAL_PANEL;
		return FALSE;
	}

	const PluginsListItem& CurPlugin = PluginsList.back();

	Global->CtrlObject->Plugins->ClosePanel(hPlugin);

	if (PluginsList.size() > 1)
	{
		auto NextTopPlugin = PluginsList.end();
		std::advance(NextTopPlugin, -2);
		hPlugin=NextTopPlugin->hPlugin;
		strOriginalCurDir=CurPlugin.strPrevOriginalCurDir;

		if (EnableRestoreViewMode)
		{
			SetViewMode(CurPlugin.PrevViewMode);
			SortMode = CurPlugin.PrevSortMode;
			NumericSort = CurPlugin.PrevNumericSort;
			CaseSensitiveSort = CurPlugin.PrevCaseSensitiveSort;
			ReverseSortOrder = CurPlugin.PrevSortOrder;
			DirectoriesFirst = CurPlugin.PrevDirectoriesFirst;
		}

		if (CurPlugin.Modified)
		{
			PluginPanelItem PanelItem={};
			string strSaveDir;
			api::GetCurrentDirectory(strSaveDir);

			if (FileNameToPluginItem(CurPlugin.strHostFile,&PanelItem))
			{
				Global->CtrlObject->Plugins->PutFiles(hPlugin,&PanelItem,1,FALSE,0);
			}
			else
			{
				PanelItem.FileName = DuplicateString(PointToName(CurPlugin.strHostFile));
				Global->CtrlObject->Plugins->DeleteFiles(hPlugin,&PanelItem,1,0);
				delete[] PanelItem.FileName;
			}

			FarChDir(strSaveDir);
		}


		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			DeleteFileWithFolder(CurPlugin.strHostFile);  // �������� ����� �� ����������� �������
		}
	}
	else
	{
		PanelMode=NORMAL_PANEL;
		hPlugin = nullptr;

		if (EnableRestoreViewMode)
		{
			SetViewMode(CurPlugin.PrevViewMode);
			SortMode = CurPlugin.PrevSortMode;
			NumericSort = CurPlugin.PrevNumericSort;
			CaseSensitiveSort = CurPlugin.PrevCaseSensitiveSort;
			ReverseSortOrder = CurPlugin.PrevSortOrder;
			DirectoriesFirst = CurPlugin.PrevDirectoriesFirst;
		}
	}

	if (EnableRestoreViewMode)
		Global->CtrlObject->Cp()->RedrawKeyBar();

	PluginsList.pop_back();
	--Global->PluginPanelsCount;

	return TRUE;
}

/*
	DefaultName - ��� �������� �� ������� ���������������.
	Closed - ������ �����������, ���� � PrevDataList ���-�� ���� - ������������������ ������.
	UsePrev - ���� ���������������� �� PrevDataList, ������� ��� ���������������� ����� ������ ��.
	Position - ���� �� ������ ������������� ������� �������.
*/
void FileList::PopPrevData(const string& DefaultName,bool Closed,bool UsePrev,bool Position,bool SetDirectorySuccess)
{
	string strName(DefaultName);
	if (Closed && !PrevDataList.empty())
	{
		PrevDataItem& Item = PrevDataList.back();
		if (Item.PrevListData.size() > 1)
		{
			MoveSelection(Item.PrevListData, ListData);
			UpperFolderTopFile = Item.PrevTopFile;

			if (UsePrev)
				strName = Item.strPrevName;

			DeleteListData(Item.PrevListData);

			if (SelectedFirst)
				SortFileList(FALSE);
			else if (!ListData.empty())
				SortFileList(TRUE);
		}
		PrevDataList.pop_back();
	}
	if (Position)
	{
		long Pos=FindFile(PointToName(strName));

		if (Pos!=-1)
			CurFile=Pos;
		else
			GoToFile(strName);

		CurTopFile=UpperFolderTopFile;
		UpperFolderTopFile=0;
		CorrectPosition();
	}
	/* $ 26.04.2001 DJ
	   ������� ��� ������� ��������� ��� ������� SetDirectory
	*/
	else if (SetDirectorySuccess)
		CurFile=CurTopFile=0;
}

int FileList::FileNameToPluginItem(const string& Name,PluginPanelItem *pi)
{
	string strTempDir = Name;

	if (!CutToSlash(strTempDir,true))
		return FALSE;

	FarChDir(strTempDir);
	ClearStruct(*pi);
	api::FAR_FIND_DATA fdata;

	if (api::GetFindDataEx(Name, fdata))
	{
		FindDataExToPluginPanelItem(&fdata, pi);
		return TRUE;
	}

	return FALSE;
}


void FileList::FileListToPluginItem(const FileListItem *fi, PluginPanelItem *pi)
{
	pi->FileName = DuplicateString(fi->strName.data());
	pi->AlternateFileName = DuplicateString(fi->strShortName.data());
	pi->FileSize=fi->FileSize;
	pi->AllocationSize=fi->AllocationSize;
	pi->FileAttributes=fi->FileAttr;
	pi->LastWriteTime=fi->WriteTime;
	pi->CreationTime=fi->CreationTime;
	pi->LastAccessTime=fi->AccessTime;
	pi->ChangeTime=fi->ChangeTime;
	pi->NumberOfLinks=fi->NumberOfLinks;
	pi->Flags=fi->UserFlags;

	if (fi->Selected)
		pi->Flags|=PPIF_SELECTED;

	pi->CustomColumnData=fi->CustomColumnData;
	pi->CustomColumnNumber=fi->CustomColumnNumber;
	pi->Description=fi->DizText; //BUGBUG???

	pi->UserData.Data=fi->UserData;
	pi->UserData.FreeData=fi->Callback;

	pi->CRC32=fi->CRC32;
	pi->Reserved[0]=pi->Reserved[1]=0;
	pi->Owner = EmptyToNull(fi->strOwner.data());
}

size_t FileList::FileListToPluginItem2(FileListItem *fi,FarGetPluginPanelItem *gpi)
{
	size_t size=ALIGN(sizeof(PluginPanelItem)),offset=size;
	size+=fi->CustomColumnNumber*sizeof(wchar_t*);
	size+=sizeof(wchar_t)*(fi->strName.size()+1);
	size+=sizeof(wchar_t)*(fi->strShortName.size()+1);
	for (size_t ii=0; ii<fi->CustomColumnNumber; ii++)
	{
		size+=fi->CustomColumnData[ii]?sizeof(wchar_t)*(wcslen(fi->CustomColumnData[ii])+1):0;
	}
	size+=fi->DizText?sizeof(wchar_t)*(wcslen(fi->DizText)+1):0;
	size+=fi->strOwner.empty()?0:sizeof(wchar_t)*(fi->strOwner.size()+1);

	if (gpi)
	{
		if(gpi->Item && gpi->Size >= size)
		{
			char* data=(char*)(gpi->Item)+offset;

			gpi->Item->FileSize=fi->FileSize;
			gpi->Item->AllocationSize=fi->AllocationSize;
			gpi->Item->FileAttributes=fi->FileAttr;
			gpi->Item->LastWriteTime=fi->WriteTime;
			gpi->Item->CreationTime=fi->CreationTime;
			gpi->Item->LastAccessTime=fi->AccessTime;
			gpi->Item->ChangeTime=fi->ChangeTime;
			gpi->Item->NumberOfLinks=fi->NumberOfLinks;
			gpi->Item->Flags=fi->UserFlags;
			if (fi->Selected)
				gpi->Item->Flags|=PPIF_SELECTED;
			gpi->Item->CustomColumnNumber=fi->CustomColumnNumber;
			gpi->Item->CRC32=fi->CRC32;
			gpi->Item->Reserved[0]=gpi->Item->Reserved[1]=0;

			gpi->Item->CustomColumnData=(wchar_t**)data;
			data+=fi->CustomColumnNumber*sizeof(wchar_t*);

			gpi->Item->UserData.Data=fi->UserData;
			gpi->Item->UserData.FreeData=fi->Callback;

			gpi->Item->FileName=wcscpy((wchar_t*)data,fi->strName.data());
			data+=sizeof(wchar_t)*(fi->strName.size()+1);

			gpi->Item->AlternateFileName=wcscpy((wchar_t*)data,fi->strShortName.data());
			data+=sizeof(wchar_t)*(fi->strShortName.size()+1);

			for (size_t ii=0; ii<fi->CustomColumnNumber; ii++)
			{
				if (!fi->CustomColumnData[ii])
				{
					const_cast<const wchar_t**>(gpi->Item->CustomColumnData)[ii] = nullptr;
				}
				else
				{
					const_cast<const wchar_t**>(gpi->Item->CustomColumnData)[ii] = wcscpy(reinterpret_cast<wchar_t*>(data), fi->CustomColumnData[ii]);
					data+=sizeof(wchar_t)*(wcslen(fi->CustomColumnData[ii])+1);
				}
			}

			if (!fi->DizText)
			{
				gpi->Item->Description=nullptr;
			}
			else
			{
				gpi->Item->Description=wcscpy((wchar_t*)data,fi->DizText);
				data+=sizeof(wchar_t)*(wcslen(fi->DizText)+1);
			}


			if (fi->strOwner.empty())
			{
				gpi->Item->Owner=nullptr;
			}
			else
			{
				gpi->Item->Owner=wcscpy((wchar_t*)data,fi->strOwner.data());
			}
		}
	}
	return size;
}

void FileList::PluginToFileListItem(PluginPanelItem *pi,FileListItem *fi)
{
	fi->strName = NullToEmpty(pi->FileName);
	fi->strShortName = NullToEmpty(pi->AlternateFileName);
	fi->strOwner = NullToEmpty(pi->Owner);

	if (pi->Description)
	{
		fi->DizText=new wchar_t[StrLength(pi->Description)+1];
		wcscpy(fi->DizText, pi->Description);
		fi->DeleteDiz=TRUE;
	}
	else
		fi->DizText=nullptr;

	fi->FileSize=pi->FileSize;
	fi->AllocationSize=pi->AllocationSize;
	fi->FileAttr=pi->FileAttributes;
	fi->WriteTime=pi->LastWriteTime;
	fi->CreationTime=pi->CreationTime;
	fi->AccessTime=pi->LastAccessTime;
	fi->ChangeTime.dwHighDateTime = 0;
	fi->ChangeTime.dwLowDateTime = 0;
	fi->NumberOfLinks=pi->NumberOfLinks;
	fi->NumberOfStreams=1;
	fi->UserFlags=pi->Flags;

	fi->UserData=pi->UserData.Data;
	fi->Callback=pi->UserData.FreeData;

	if (pi->CustomColumnNumber>0)
	{
		fi->CustomColumnData=new wchar_t*[pi->CustomColumnNumber];

		for (size_t I=0; I<pi->CustomColumnNumber; I++)
			if (pi->CustomColumnData && pi->CustomColumnData[I])
			{
				fi->CustomColumnData[I]=new wchar_t[StrLength(pi->CustomColumnData[I])+1];
				wcscpy(fi->CustomColumnData[I],pi->CustomColumnData[I]);
			}
			else
			{
				fi->CustomColumnData[I]=new wchar_t[1];
				fi->CustomColumnData[I][0]=0;
			}
	}

	fi->CustomColumnNumber=pi->CustomColumnNumber;
	fi->CRC32=pi->CRC32;
}


HANDLE FileList::OpenPluginForFile(const string* FileName, DWORD FileAttr, OPENFILEPLUGINTYPE Type)
{
	HANDLE Result = nullptr;
	if(!FileName->empty() && !(FileAttr&FILE_ATTRIBUTE_DIRECTORY))
	{
		SetCurPath();
		_ALGO(SysLog(L"close AnotherPanel file"));
		Global->CtrlObject->Cp()->GetAnotherPanel(this)->CloseFile();
		_ALGO(SysLog(L"call Plugins.OpenFilePlugin {"));
		Result = Global->CtrlObject->Plugins->OpenFilePlugin(FileName, 0, Type);
		_ALGO(SysLog(L"}"));
	}
	return Result;
}


std::vector<PluginPanelItem> FileList::CreatePluginItemList(bool AddTwoDot)
{
	std::vector<PluginPanelItem> ItemList;

	if (ListData.empty())
		return ItemList;

	long SaveSelPosition=GetSelPosition;
	long OldLastSelPosition=LastSelPosition;
	string strSelName;

	ItemList.reserve(SelFileCount+1);

	DWORD FileAttr;
	GetSelName(nullptr,FileAttr);

	while (GetSelName(&strSelName,FileAttr))
	{
		if ((!(FileAttr & FILE_ATTRIBUTE_DIRECTORY) || !TestParentFolderName(strSelName)) && LastSelPosition>=0 && static_cast<size_t>(LastSelPosition) < ListData.size())
		{
			ItemList.emplace_back(VALUE_TYPE(ItemList)());
			FileListToPluginItem(ListData[LastSelPosition].get(), &ItemList.back());
		}
	}

	if (AddTwoDot && ItemList.empty() && (FileAttr & FILE_ATTRIBUTE_DIRECTORY)) // ��� ��� ".."
	{
		FileListToPluginItem(ListData[0].get(), &ItemList.front());
		//ItemList->FindData.lpwszFileName = DuplicateString(ListData[0]->strName);
		//ItemList->FindData.dwFileAttributes=ListData[0]->FileAttr;
	}

	LastSelPosition=OldLastSelPosition;
	GetSelPosition=SaveSelPosition;
	return ItemList;
}


void FileList::DeletePluginItemList(std::vector<PluginPanelItem> &ItemList)
{
	std::for_each(ALL_RANGE(ItemList), FreePluginPanelItem);
	ItemList.clear();
}


void FileList::PluginDelete()
{
	_ALGO(CleverSysLog clv(L"FileList::PluginDelete()"));
	SaveSelection();
	auto ItemList = CreatePluginItemList();

	if (!ItemList.empty())
	{
		if (Global->CtrlObject->Plugins->DeleteFiles(hPlugin, ItemList.data(), ItemList.size(), 0))
		{
			SetPluginModified();
			PutDizToPlugin(this, ItemList, TRUE, FALSE, nullptr, &Diz);
		}

		DeletePluginItemList(ItemList);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		AnotherPanel->Redraw();
	}
}


void FileList::PutDizToPlugin(FileList *DestPanel, std::vector<PluginPanelItem>& ItemList, int Delete, int Move, DizList *SrcDiz, DizList *DestDiz)
{
	_ALGO(CleverSysLog clv(L"FileList::PutDizToPlugin()"));
	OpenPanelInfo Info;
	Global->CtrlObject->Plugins->GetOpenPanelInfo(DestPanel->hPlugin,&Info);

	if (DestPanel->strPluginDizName.empty() && Info.DescrFilesNumber>0)
		DestPanel->strPluginDizName = Info.DescrFiles[0];

	if (((Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()) ||
	        Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS) && !DestPanel->strPluginDizName.empty() &&
	        (!Info.HostFile || !*Info.HostFile || DestPanel->GetModalMode() ||
	         api::GetFileAttributes(Info.HostFile)!=INVALID_FILE_ATTRIBUTES))
	{
		Global->CtrlObject->Cp()->LeftPanel->ReadDiz();
		Global->CtrlObject->Cp()->RightPanel->ReadDiz();

		if (DestPanel->GetModalMode())
			DestPanel->ReadDiz();

		bool DizPresent = false;

		std::for_each(RANGE(ItemList, i)
		{
			if (i.Flags & PPIF_PROCESSDESCR)
			{
				int Code;

				if (Delete)
					Code=DestDiz->DeleteDiz(i.FileName, i.AlternateFileName);
				else
				{
					Code=SrcDiz->CopyDiz(i.FileName, i.AlternateFileName, i.FileName, i.AlternateFileName, DestDiz);

					if (Code && Move)
						SrcDiz->DeleteDiz(i.FileName, i.AlternateFileName);
				}

				if (Code)
					DizPresent = true;
			}
		});

		if (DizPresent)
		{
			string strTempDir;

			if (FarMkTempEx(strTempDir) && api::CreateDirectory(strTempDir,nullptr))
			{
				string strSaveDir;
				api::GetCurrentDirectory(strSaveDir);
				string strDizName=strTempDir+L"\\"+DestPanel->strPluginDizName;
				DestDiz->Flush(L"", &strDizName);

				if (Move)
					SrcDiz->Flush(L"");

				PluginPanelItem PanelItem;

				if (FileNameToPluginItem(strDizName,&PanelItem))
					Global->CtrlObject->Plugins->PutFiles(DestPanel->hPlugin,&PanelItem,1,FALSE,OPM_SILENT|OPM_DESCR);
				else if (Delete)
				{
					PluginPanelItem pi={};
					pi.FileName = DuplicateString(DestPanel->strPluginDizName.data());
					Global->CtrlObject->Plugins->DeleteFiles(DestPanel->hPlugin,&pi,1,OPM_SILENT);
					delete[] pi.FileName;
				}

				FarChDir(strSaveDir);
				DeleteFileWithFolder(strDizName);
			}
		}
	}
}


void FileList::PluginGetFiles(const wchar_t **DestPath,int Move)
{
	_ALGO(CleverSysLog clv(L"FileList::PluginGetFiles()"));
	SaveSelection();
	auto ItemList = CreatePluginItemList();
	if (!ItemList.empty())
	{
		int GetCode=Global->CtrlObject->Plugins->GetFiles(hPlugin, ItemList.data(), ItemList.size(), Move!=0, DestPath, 0);

		if ((Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()) ||
		        Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
		{
			DizList DestDiz;
			int DizFound=FALSE;

			std::for_each(RANGE(ItemList, i)
			{
				if (i.Flags & PPIF_PROCESSDESCR)
				{
					if (!DizFound)
					{
						Global->CtrlObject->Cp()->LeftPanel->ReadDiz();
						Global->CtrlObject->Cp()->RightPanel->ReadDiz();
						DestDiz.Read(*DestPath);
						DizFound=TRUE;
					}
					CopyDiz(i.FileName, i.AlternateFileName, i.FileName, i.FileName, &DestDiz);
				}
			});
			DestDiz.Flush(*DestPath);
		}

		if (GetCode==1)
		{
			if (!ReturnCurrentFile)
				ClearSelection();

			if (Move)
			{
				SetPluginModified();
				PutDizToPlugin(this, ItemList, TRUE, FALSE, nullptr, &Diz);
			}
		}
		else if (!ReturnCurrentFile)
			PluginClearSelection(ItemList);

		DeletePluginItemList(ItemList);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		AnotherPanel->Redraw();
	}
}


void FileList::PluginToPluginFiles(int Move)
{
	_ALGO(CleverSysLog clv(L"FileList::PluginToPluginFiles()"));
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	string strTempDir;

	if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
		return;

	FileList *AnotherFilePanel=(FileList *)AnotherPanel;

	if (!FarMkTempEx(strTempDir))
		return;

	SaveSelection();
	api::CreateDirectory(strTempDir,nullptr);
	auto ItemList = CreatePluginItemList();

	if (!ItemList.empty())
	{
		const wchar_t *lpwszTempDir=strTempDir.data();
		int PutCode=Global->CtrlObject->Plugins->GetFiles(hPlugin, ItemList.data(), ItemList.size(), FALSE, &lpwszTempDir, OPM_SILENT);
		strTempDir=lpwszTempDir;

		if (PutCode==1 || PutCode==2)
		{
			string strSaveDir;
			api::GetCurrentDirectory(strSaveDir);
			FarChDir(strTempDir);
			PutCode=Global->CtrlObject->Plugins->PutFiles(AnotherFilePanel->hPlugin, ItemList.data(), ItemList.size(), FALSE, 0);

			if (PutCode==1 || PutCode==2)
			{
				if (!ReturnCurrentFile)
					ClearSelection();

				AnotherPanel->SetPluginModified();
				PutDizToPlugin(AnotherFilePanel, ItemList, FALSE, FALSE, &Diz, &AnotherFilePanel->Diz);

				if (Move && Global->CtrlObject->Plugins->DeleteFiles(hPlugin, ItemList.data(), ItemList.size(), OPM_SILENT))
					{
						SetPluginModified();
						PutDizToPlugin(this, ItemList, TRUE, FALSE, nullptr, &Diz);
					}
			}
			else if (!ReturnCurrentFile)
				PluginClearSelection(ItemList);

			FarChDir(strSaveDir);
		}

		DeleteDirTree(strTempDir);
		DeletePluginItemList(ItemList);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();

		if (PanelMode==PLUGIN_PANEL)
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		else
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);

		AnotherPanel->Redraw();
	}
}

void FileList::PluginHostGetFiles()
{
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	string strDestPath;
	string strSelName;
	DWORD FileAttr;
	SaveSelection();
	GetSelName(nullptr,FileAttr);

	if (!GetSelName(&strSelName,FileAttr))
		return;

	strDestPath = AnotherPanel->GetCurDir();

	if (((!AnotherPanel->IsVisible() || AnotherPanel->GetType()!=FILE_PANEL) &&
	        !SelFileCount) || strDestPath.empty())
	{
		strDestPath = PointToName(strSelName);
		// SVS: � ����� ����� ����� ����� ����� � ������?
		size_t pos = strDestPath.rfind(L'.');
		if (pos != string::npos)
			strDestPath.resize(pos);
	}

	int ExitLoop=FALSE;
	GetSelName(nullptr,FileAttr);
	std::unordered_set<Plugin*> UsedPlugins;

	while (!ExitLoop && GetSelName(&strSelName,FileAttr))
	{
		HANDLE hCurPlugin;

		if ((hCurPlugin=OpenPluginForFile(&strSelName,FileAttr, OFP_EXTRACT))!=nullptr &&
		        hCurPlugin!=PANEL_STOP)
		{
			PluginHandle *ph = (PluginHandle *)hCurPlugin;
			int OpMode=OPM_TOPLEVEL;
			if(UsedPlugins.find(ph->pPlugin) != UsedPlugins.cend())
				OpMode|=OPM_SILENT;

			PluginPanelItem *ItemList;
			size_t ItemNumber;
			_ALGO(SysLog(L"call Plugins.GetFindData()"));

			if (Global->CtrlObject->Plugins->GetFindData(hCurPlugin,&ItemList,&ItemNumber,OpMode))
			{
				_ALGO(SysLog(L"call Plugins.GetFiles()"));
				const wchar_t *lpwszDestPath=strDestPath.data();
				ExitLoop=Global->CtrlObject->Plugins->GetFiles(hCurPlugin,ItemList,ItemNumber,FALSE,&lpwszDestPath,OpMode)!=1;
				strDestPath=lpwszDestPath;

				if (!ExitLoop)
				{
					_ALGO(SysLog(L"call ClearLastGetSelection()"));
					ClearLastGetSelection();
				}

				_ALGO(SysLog(L"call Plugins.FreeFindData()"));
				Global->CtrlObject->Plugins->FreeFindData(hCurPlugin,ItemList,ItemNumber,true);
				UsedPlugins.emplace(ph->pPlugin);
			}

			_ALGO(SysLog(L"call Plugins.ClosePanel"));
			Global->CtrlObject->Plugins->ClosePanel(hCurPlugin);
		}
	}

	Update(UPDATE_KEEP_SELECTION);
	Redraw();
	AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	AnotherPanel->Redraw();
}


void FileList::PluginPutFilesToNew()
{
	_ALGO(CleverSysLog clv(L"FileList::PluginPutFilesToNew()"));
	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	_ALGO(SysLog(L"call Plugins.OpenFilePlugin(nullptr, 0)"));
	HANDLE hNewPlugin=Global->CtrlObject->Plugins->OpenFilePlugin(nullptr, 0, OFP_CREATE);

	if (hNewPlugin && hNewPlugin!=PANEL_STOP)
	{
		_ALGO(SysLog(L"Create: FileList TmpPanel, FileCount=%d",FileCount));
		FileList *TmpPanel=new FileList;
		TmpPanel->SetPluginMode(hNewPlugin,L"");  // SendOnFocus??? true???
		TmpPanel->SetModalMode(TRUE);
		auto PrevFileCount = ListData.size();
		/* $ 12.04.2002 IS
		   ���� PluginPutFilesToAnother ������� �����, �������� �� 2, �� �����
		   ����������� ���������� ������ �� ��������� ����.
		*/
		int rc=PluginPutFilesToAnother(FALSE,TmpPanel);

		if (rc != 2 && ListData.size() == PrevFileCount+1)
		{
			int LastPos = 0;
			/* �����, ��� ����������� ���������� ����� ���������� �����
			   ���������������� ���������� �� ���� � ������������ �����
			   �������� �����. ������, ���� �����-�� ������� �������� ������
			   � ������� �������� ����� � ����� �������� ������� �������,
			   �� ����������� ���������������� �� ����������!
			*/
			FileListItem *PtrLastPos = nullptr;
			int n = 0;
			std::for_each(CONST_RANGE(ListData, i)
			{
				if ((i->FileAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					if (PtrLastPos)
					{
						if (FileTimeDifference(&i->CreationTime, &PtrLastPos->CreationTime) > 0)
						{
							LastPos = n;
							PtrLastPos = i.get();
						}
					}
					else
					{
						LastPos = n;
						PtrLastPos = i.get();
					}
				}
				++n;
			});

			if (PtrLastPos)
			{
				CurFile = LastPos;
				Redraw();
			}
		}
		TmpPanel->Destroy();
	}
}


/* $ 12.04.2002 IS
     PluginPutFilesToAnother ������ int - ���������� ��, ��� ����������
     PutFiles:
     -1 - �������� ������������
      0 - �������
      1 - �����
      2 - �����, ������ ������������� ���������� �� ���� � ������ ���
          ������������� �� ����� (��. PluginPutFilesToNew)
*/
int FileList::PluginPutFilesToAnother(int Move,Panel *AnotherPanel)
{
	if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
		return 0;

	FileList *AnotherFilePanel=(FileList *)AnotherPanel;
	int PutCode=0;
	SaveSelection();
	auto ItemList = CreatePluginItemList();

	if (!ItemList.empty())
	{
		SetCurPath();
		_ALGO(SysLog(L"call Plugins.PutFiles"));
		PutCode=Global->CtrlObject->Plugins->PutFiles(AnotherFilePanel->hPlugin, ItemList.data(), ItemList.size(), Move!=0, 0);

		if (PutCode==1 || PutCode==2)
		{
			if (!ReturnCurrentFile)
			{
				_ALGO(SysLog(L"call ClearSelection()"));
				ClearSelection();
			}

			_ALGO(SysLog(L"call PutDizToPlugin"));
			PutDizToPlugin(AnotherFilePanel, ItemList, FALSE, Move, &Diz, &AnotherFilePanel->Diz);
			AnotherPanel->SetPluginModified();
		}
		else if (!ReturnCurrentFile)
			PluginClearSelection(ItemList);

		_ALGO(SysLog(L"call DeletePluginItemList"));
		DeletePluginItemList(ItemList);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();

		if (AnotherPanel==Global->CtrlObject->Cp()->GetAnotherPanel(this))
		{
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);
			AnotherPanel->Redraw();
		}
	}

	return PutCode;
}


void FileList::GetOpenPanelInfo(OpenPanelInfo *Info)
{
	_ALGO(CleverSysLog clv(L"FileList::GetOpenPanelInfo()"));
	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	ClearStruct(*Info);

	if (PanelMode==PLUGIN_PANEL)
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,Info);
}


/*
   ������� ��� ������ ������� "�������� �������" (Shift-F3)
*/
void FileList::ProcessHostFile()
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessHostFile()"));

	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	if (!ListData.empty() && SetCurPath())
	{
		int Done=FALSE;
		SaveSelection();

		if (PanelMode==PLUGIN_PANEL && !PluginsList.back().strHostFile.empty())
		{
			_ALGO(SysLog(L"call CreatePluginItemList"));
			auto ItemList = CreatePluginItemList();
			_ALGO(SysLog(L"call Plugins.ProcessHostFile"));
			Done=Global->CtrlObject->Plugins->ProcessHostFile(hPlugin, ItemList.data(), ItemList.size(), 0);

			if (Done)
				SetPluginModified();
			else
			{
				if (!ReturnCurrentFile)
					PluginClearSelection(ItemList);

				Redraw();
			}

			_ALGO(SysLog(L"call DeletePluginItemList"));
			DeletePluginItemList(ItemList);

			if (Done)
				ClearSelection();
		}
		else
		{
			size_t SCount=GetRealSelCount();

			if (SCount > 0)
			{
				FOR_CONST_RANGE(ListData, i)
				{
					if ((*i)->Selected)
					{
						Done=ProcessOneHostFile(i);

						if (Done == 1)
							Select(i->get(), 0);
						else if (Done == -1)
							continue;
						else       // ���� ��� ������, ��... ����� ���� ESC �� ������ ������
							break;   //
					}
				}

				if (SelectedFirst)
					SortFileList(TRUE);
			}
			else
			{
				if ((Done=ProcessOneHostFile(ListData.begin() + CurFile)) == 1)
					ClearSelection();
			}
		}

		if (Done)
		{
			Update(UPDATE_KEEP_SELECTION);
			Redraw();
			Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}
	}
}

/*
  ��������� ������ ����-�����.
  Return:
    -1 - ���� ���� ������� �������� �� ���������
     0 - ������ ������ FALSE
     1 - ������ ������ TRUE
*/
int FileList::ProcessOneHostFile(std::vector<std::unique_ptr<FileListItem>>::const_iterator Idx)
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessOneHostFile()"));
	int Done=-1;
	HANDLE hNewPlugin=OpenPluginForFile(&(*Idx)->strName, (*Idx)->FileAttr, OFP_COMMANDS);

	if (hNewPlugin && hNewPlugin!=PANEL_STOP)
	{
		PluginPanelItem *ItemList;
		size_t ItemNumber;
		_ALGO(SysLog(L"call Plugins.GetFindData"));

		if (Global->CtrlObject->Plugins->GetFindData(hNewPlugin,&ItemList,&ItemNumber,OPM_TOPLEVEL))
		{
			_ALGO(SysLog(L"call Plugins.ProcessHostFile"));
			Done=Global->CtrlObject->Plugins->ProcessHostFile(hNewPlugin,ItemList,ItemNumber,OPM_TOPLEVEL);
			_ALGO(SysLog(L"call Plugins.FreeFindData"));
			Global->CtrlObject->Plugins->FreeFindData(hNewPlugin,ItemList,ItemNumber,true);
		}

		_ALGO(SysLog(L"call Plugins.ClosePanel"));
		Global->CtrlObject->Plugins->ClosePanel(hNewPlugin);
	}

	return Done;
}



void FileList::SetPluginMode(HANDLE hPlugin,const string& PluginFile,bool SendOnFocus)
{
	if (PanelMode!=PLUGIN_PANEL)
	{
		Global->CtrlObject->FolderHistory->AddToHistory(strCurDir);
	}

	PushPlugin(hPlugin,PluginFile);
	FileList::hPlugin=hPlugin;
	PanelMode=PLUGIN_PANEL;

	if (SendOnFocus)
		SetFocus();

	OpenPanelInfo Info;
	Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

	if (Info.StartPanelMode)
		SetViewMode(VIEW_0+Info.StartPanelMode-L'0');

	Global->CtrlObject->Cp()->RedrawKeyBar();

	if (Info.StartSortMode)
	{
		SortMode=Info.StartSortMode-(SM_UNSORTED-UNSORTED);
		ReverseSortOrder = Info.StartSortOrder != 0;
	}

	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=FILE_PANEL)
	{
		AnotherPanel->Update(UPDATE_KEEP_SELECTION);
		AnotherPanel->Redraw();
	}
}

void FileList::PluginGetPanelInfo(PanelInfo &Info)
{
	CorrectPosition();
	Info.CurrentItem=CurFile;
	Info.TopPanelItem=CurTopFile;
	if(ShowShortNames)
		Info.Flags|=PFLAGS_ALTERNATIVENAMES;
	Info.ItemsNumber = ListData.size();
	Info.SelectedItemsNumber=ListData.empty()? 0 : GetSelCount();
}

size_t FileList::PluginGetPanelItem(int ItemNumber,FarGetPluginPanelItem *Item)
{
	size_t result=0;

	if (static_cast<size_t>(ItemNumber) < ListData.size())
	{
		result=FileListToPluginItem2(ListData[ItemNumber].get(), Item);
	}

	return result;
}

size_t FileList::PluginGetSelectedPanelItem(int ItemNumber,FarGetPluginPanelItem *Item)
{
	size_t result=0;

	if (static_cast<size_t>(ItemNumber) < ListData.size())
	{
		if (ItemNumber==CacheSelIndex)
		{
			result=FileListToPluginItem2(ListData[CacheSelPos].get(), Item);
		}
		else
		{
			if (ItemNumber<CacheSelIndex) CacheSelIndex=-1;

			int CurSel=CacheSelIndex,StartValue=CacheSelIndex>=0?CacheSelPos+1:0;

			for (size_t i=StartValue; i<ListData.size(); i++)
			{
				if (ListData[i]->Selected)
					CurSel++;

				if (CurSel==ItemNumber)
				{
					result=FileListToPluginItem2(ListData[i].get(), Item);
					CacheSelIndex=ItemNumber;
					CacheSelPos=static_cast<int>(i);
					break;
				}
			}

			if (CurSel==-1 && !ItemNumber)
			{
				result=FileListToPluginItem2(ListData[CurFile].get(), Item);
				CacheSelIndex=-1;
			}
		}
	}

	return result;
}

void FileList::PluginGetColumnTypesAndWidths(string& strColumnTypes,string& strColumnWidths)
{
	ViewSettingsToText(ViewSettings.PanelColumns, strColumnTypes, strColumnWidths);
}

void FileList::PluginBeginSelection()
{
	SaveSelection();
}

void FileList::PluginSetSelection(int ItemNumber,bool Selection)
{
	Select(ListData[ItemNumber].get(), Selection);
}

void FileList::PluginClearSelection(int SelectedItemNumber)
{
	if (static_cast<size_t>(SelectedItemNumber) < ListData.size())
	{
		if (SelectedItemNumber<=CacheSelClearIndex)
		{
			CacheSelClearIndex=-1;
		}

		int CurSel=CacheSelClearIndex,StartValue=CacheSelClearIndex>=0?CacheSelClearPos+1:0;

		for (size_t i=StartValue; i < ListData.size(); i++)
		{
			if (ListData[i]->Selected)
			{
				CurSel++;
			}

			if (CurSel==SelectedItemNumber)
			{
				Select(ListData[i].get(), FALSE);
				CacheSelClearIndex=SelectedItemNumber;
				CacheSelClearPos=static_cast<int>(i);
				break;
			}
		}
	}
}

void FileList::PluginEndSelection()
{
	if (SelectedFirst)
	{
		SortFileList(TRUE);
	}
}

void FileList::ProcessPluginCommand()
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessPluginCommand"));
	_ALGO(SysLog(L"PanelMode=%s",(PanelMode==PLUGIN_PANEL?"PLUGIN_PANEL":"NORMAL_PANEL")));
	int Command=PluginCommand;
	PluginCommand=-1;

	if (PanelMode==PLUGIN_PANEL)
		switch (Command)
		{
			case FCTL_CLOSEPANEL:
				_ALGO(SysLog(L"Command=FCTL_CLOSEPANEL"));
				SetCurDir(strPluginParam,true);

				if (strPluginParam.empty())
					Update(UPDATE_KEEP_SELECTION);

				Redraw();
				break;
		}
}

void FileList::SetPluginModified()
{
	if(!PluginsList.empty())
	{
		PluginsList.back().Modified = TRUE;
	}
}


HANDLE FileList::GetPluginHandle()
{
	return(hPlugin);
}


int FileList::ProcessPluginEvent(int Event,void *Param)
{
	if (PanelMode==PLUGIN_PANEL)
		return(Global->CtrlObject->Plugins->ProcessEvent(hPlugin,Event,Param));

	return FALSE;
}


void FileList::PluginClearSelection(const std::vector<PluginPanelItem>& ItemList)
{
	SaveSelection();
	size_t FileNumber=0,PluginNumber=0;

	while (PluginNumber < ItemList.size())
	{
		const auto& CurPlugin = ItemList[PluginNumber];

		if (!(CurPlugin.Flags & PPIF_SELECTED))
		{
			while (StrCmpI(CurPlugin.FileName, ListData[FileNumber]->strName.data()))
				if (++FileNumber >= ListData.size())
					return;

			Select(ListData[FileNumber++].get(), 0);
		}

		PluginNumber++;
	}
}
