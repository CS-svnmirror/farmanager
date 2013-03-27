/*
dirinfo.cpp

GetDirInfo & GetPluginDirInfo
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

#include "dirinfo.hpp"
#include "plugapi.hpp"
#include "keys.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "RefreshFrameManager.hpp"
#include "TPreRedrawFunc.hpp"
#include "ctrlobj.hpp"
#include "filefilter.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "TaskBar.hpp"
#include "constitle.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "wakeful.hpp"
#include "config.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "plugins.hpp"
#include "mix.hpp"

static void DrawGetDirInfoMsg(const wchar_t *Title,const wchar_t *Name,const UINT64* Size)
{
	string strSize;
	FileSizeToStr(strSize,*Size,8,COLUMN_FLOATSIZE|COLUMN_COMMAS);
	RemoveLeadingSpaces(strSize);
	Message(0,0,Title,MSG(MScanningFolder),Name,strSize);
	if (!Global->PreRedraw->empty())
	{
		PreRedrawItem& preRedrawItem(Global->PreRedraw->top());
		preRedrawItem.Param.Param1=Title;
		preRedrawItem.Param.Param2=Name;
		preRedrawItem.Param.Param3=Size;
	}
}

static void PR_DrawGetDirInfoMsg()
{
	if (!Global->PreRedraw->empty())
	{
		const PreRedrawItem& preRedrawItem(Global->PreRedraw->top());
		DrawGetDirInfoMsg(
			static_cast<const wchar_t*>(preRedrawItem.Param.Param1),
			static_cast<const wchar_t*>(preRedrawItem.Param.Param2),
			static_cast<const UINT64*>(preRedrawItem.Param.Param3)
		);
	}
}


class FileIdTree: public Tree<UINT64>
{
public:
	FileIdTree(){}
	~FileIdTree(){clear();}
	long compare(Node<UINT64>* first, UINT64* second) {return *first->data-*second;}
};

int GetDirInfo(const wchar_t *Title, const wchar_t *DirName, DirInfoData& Data, clock_t MsgWaitTime, FileFilter *Filter, DWORD Flags)
{
	string strFullDirName, strDriveRoot;
	string strFullName, strCurDirName, strLastDirName;
	ConvertNameToFull(DirName, strFullDirName);
	SaveScreen SaveScr;
	UndoGlobalSaveScrPtr UndSaveScr(&SaveScr);
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_DrawGetDirInfoMsg);
	TaskBar TB(MsgWaitTime!=-1);
	wakeful W;
	ScanTree ScTree(FALSE,TRUE,(Flags&GETDIRINFO_SCANSYMLINKDEF?(DWORD)-1:(Flags&GETDIRINFO_SCANSYMLINK)));
	FAR_FIND_DATA FindData;
	clock_t StartTime=clock();
	SetCursorType(FALSE,0);
	GetPathRoot(strFullDirName,strDriveRoot);
	/* $ 20.03.2002 DJ
	   ��� . - ������� ��� ������������� ��������
	*/
	const wchar_t *ShowDirName = DirName;

	if (DirName[0] == L'.' && !DirName[1])
	{
		const wchar_t *p = LastSlash(strFullDirName);

		if (p)
			ShowDirName = p + 1;
	}

	ConsoleTitle OldTitle;
	RefreshFrameManager frref(ScrX,ScrY,MsgWaitTime,Flags&GETDIRINFO_DONTREDRAWFRAME);
	DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

	if (GetDiskFreeSpace(strDriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
		Data.ClusterSize=SectorsPerCluster*BytesPerSector;

	// ��������� ��������� ��� ���������
	strLastDirName.Clear();
	strCurDirName.Clear();
	Data.DirCount=Data.FileCount=0;
	Data.FileSize=Data.AllocationSize=Data.FilesSlack=Data.MFTOverhead=0;
	ScTree.SetFindPath(DirName,L"*");

	FileIdTree FileIds;

	bool CheckHardlinks = false;
	DWORD FileSystemFlags = 0;
	string FileSystemName;
	string Root;
	GetPathRoot(DirName, Root);
	if(apiGetVolumeInformation(Root, nullptr, nullptr, nullptr, &FileSystemFlags, &FileSystemName))
	{
		if(Global->WinVer() < _WIN32_WINNT_WIN7)
		{
			CheckHardlinks = FileSystemName == L"NTFS";
		}
		else
		{
			CheckHardlinks = (FileSystemFlags&FILE_SUPPORTS_HARD_LINKS) != 0;
		}
	}
	while (ScTree.GetNextName(&FindData,strFullName))
	{
		if (!Global->CtrlObject->Macro.IsExecuting())
		{
			INPUT_RECORD rec;

			switch (PeekInputRecord(&rec))
			{
				case 0:
				case KEY_IDLE:
					break;
				case KEY_NONE:
				case KEY_ALT:
				case KEY_CTRL:
				case KEY_SHIFT:
				case KEY_RALT:
				case KEY_RCTRL:
				case KEY_LWIN: case KEY_RWIN:
					GetInputRecordNoMacroArea(&rec);
					break;
				case KEY_ESC:
				case KEY_BREAK:
					GetInputRecordNoMacroArea(&rec);
					return 0;
				default:

					if (Flags&GETDIRINFO_ENHBREAK)
					{
						return -1;
					}

					GetInputRecordNoMacroArea(&rec);
					break;
			}
		}

		clock_t CurTime=clock();

		if (MsgWaitTime!=-1 && CurTime-StartTime > MsgWaitTime)
		{
			StartTime=CurTime;
			MsgWaitTime=500;
			OldTitle << MSG(MScanningFolder) << L" " << ShowDirName << fmt::Flush(); // ������� ��������� �������
			SetCursorType(FALSE,0);
			DrawGetDirInfoMsg(Title,ShowDirName,&Data.FileSize);
		}

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// ������� ��������� ���������� ������ ���� �� ������� ������,
			// � ��������� ������ ��� ����� ������ � �������� ���������� ������
			if (!(Flags&GETDIRINFO_USEFILTER))
				Data.DirCount++;
			else
			{
				// ���� ������� �� �������� ��� ������ �� ��� ���� ���������
				// ���������� - ����� ��� ���������� �������� total
				// �� ������ (mantis 551)
				if (!Filter->FileInFilter(FindData, nullptr, &strFullName))
					ScTree.SkipDir();
			}
		}
		else
		{
			/* $ 17.04.2005 KM
			   �������� ��������� ����� � ������� ������
			*/
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				if (!Filter->FileInFilter(FindData,nullptr, &strFullName))
					continue;
			}

			// ���������� ������� ��������� ��� ���������� ������� ������ �����,
			// ����� � ����� �������� ������ ����, ��������������� ��������
			// �������.
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				strCurDirName = strFullName;
				CutToSlash(strCurDirName); //???

				if (StrCmpI(strCurDirName,strLastDirName))
				{
					Data.DirCount++;
					strLastDirName = strCurDirName;
				}
			}

			Data.FileCount++;

			Data.FileSize += FindData.nFileSize;

			bool IsDuplicate = false;
			if (CheckHardlinks && FindData.FileId)
			{
				if(FileIds.query(&FindData.FileId))
				{
					IsDuplicate = true;
				}
				else
				{
					FileIds.insert(new UINT64(FindData.FileId));
				}
			}
			if (!IsDuplicate)
			{
				Data.AllocationSize += FindData.nAllocationSize;
				if(FindData.nAllocationSize > FindData.nFileSize)
				{
					if(FindData.nAllocationSize >= Data.ClusterSize)
					{
						Data.FilesSlack += FindData.nAllocationSize - FindData.nFileSize;
					}
					else
					{
						Data.MFTOverhead += FindData.nAllocationSize - FindData.nFileSize;
					}
				}
			}
		}
	}

	return 1;
}

static int StopSearch;
static HANDLE hDirListPlugin;
static int PluginSearchMsgOut;

static void FarGetPluginDirListMsg(const wchar_t *Name,DWORD Flags)
{
	Message(Flags,0,L"",MSG(MPreparingList),Name);
	if (!Global->PreRedraw->empty())
	{
		PreRedrawItem& preRedrawItem(Global->PreRedraw->top());
		preRedrawItem.Param.Flags=Flags;
		preRedrawItem.Param.Param1=(void*)Name;
	}
}

static void PR_FarGetPluginDirListMsg()
{
	if (!Global->PreRedraw->empty())
	{
		const PreRedrawItem& preRedrawItem(Global->PreRedraw->top());
		FarGetPluginDirListMsg((const wchar_t *)preRedrawItem.Param.Param1,preRedrawItem.Param.Flags&(~MSG_KEEPBACKGROUND));
	}
}

static void PushPluginDirItem(std::vector<PluginPanelItem>& PluginDirList, PluginPanelItem *CurPanelItem, string& strPluginSearchPath)
{
	string strFullName;
	strFullName = strPluginSearchPath;
	strFullName += CurPanelItem->FileName;
	wchar_t *lpwszFullName = strFullName.GetBuffer();

	for (int I=0; lpwszFullName[I]; I++)
		if (lpwszFullName[I]==L'\x1')
			lpwszFullName[I]=L'\\';

	strFullName.ReleaseBuffer();
	PluginDirList.emplace_back(*CurPanelItem);

	PluginDirList.back().FileName = DuplicateString(strFullName);
	PluginDirList.back().AlternateFileName=nullptr;
}

static void ScanPluginDir(OPERATION_MODES OpMode,string& strPluginSearchPath, std::vector<PluginPanelItem>& PluginDirList)
{
	PluginPanelItem *PanelData=nullptr;
	size_t ItemCount=0;
	int AbortOp=FALSE;
	string strDirName;
	strDirName = strPluginSearchPath;
	wchar_t *lpwszDirName = strDirName.GetBuffer();

	for (int i=0; lpwszDirName[i]; i++)
		if (lpwszDirName[i]=='\x1')
			lpwszDirName[i]=lpwszDirName[i+1]?L'\\':0;

	strDirName.ReleaseBuffer();
	TruncStr(strDirName,30);
	CenterStr(strDirName,strDirName,30);

	if (CheckForEscSilent())
	{
		if (Global->Opt->Confirm.Esc) // ����� ���������� ������?
			AbortOp=TRUE;

		if (ConfirmAbortOp())
			StopSearch=TRUE;
	}

	FarGetPluginDirListMsg(strDirName,AbortOp?0:MSG_KEEPBACKGROUND);

	if (StopSearch || !Global->CtrlObject->Plugins->GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND|OpMode))
		return;

	PluginDirList.reserve(PluginDirList.size() + ItemCount);

	for (size_t i=0; i<ItemCount && !StopSearch; i++)
	{
		PluginPanelItem *CurPanelItem=PanelData+i;

		if (!(CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			PushPluginDirItem(PluginDirList, CurPanelItem, strPluginSearchPath);
	}

	for (size_t i=0; i<ItemCount && !StopSearch; i++)
	{
		PluginPanelItem *CurPanelItem=PanelData+i;

		if ((CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		        StrCmp(CurPanelItem->FileName,L".") &&
		        !TestParentFolderName(CurPanelItem->FileName))
		{
			/* $ 30.11.2001 DJ
					���������� ����� ������� ��� ����������� FindData (�� ��������
					���������� PPIF_USERDATA)
			*/
			PushPluginDirItem(PluginDirList, CurPanelItem, strPluginSearchPath);
			string strFileName = CurPanelItem->FileName;

			if (Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,strFileName,OPM_FIND|OpMode,&CurPanelItem->UserData))
			{
				strPluginSearchPath += CurPanelItem->FileName;
				strPluginSearchPath += L"\x1";
				ScanPluginDir(OpMode,strPluginSearchPath, PluginDirList);
				size_t pos = (size_t)-1;
				strPluginSearchPath.RPos(pos,L'\x1');
				strPluginSearchPath.SetLength(pos);

				if (strPluginSearchPath.RPos(pos,L'\x1'))
					strPluginSearchPath.SetLength(pos+1);
				else
					strPluginSearchPath.Clear();

				if (!Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,L"..",OPM_FIND|OpMode))
				{
					StopSearch=TRUE;
					break;
				}
			}
		}
	}

	Global->CtrlObject->Plugins->FreeFindData(hDirListPlugin,PanelData,ItemCount,true);
}

int GetPluginDirList(Plugin* PluginNumber, HANDLE hPlugin, const wchar_t *Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber)
{
	if (!StrCmp(Dir,L".") || TestParentFolderName(Dir))
		return FALSE;

	static PluginHandle DirListPlugin;
	OPERATION_MODES OpMode=0;
	if (Global->CtrlObject->Cp()->GetAnotherPanel(Global->CtrlObject->Cp()->ActivePanel)->GetType()==QVIEW_PANEL || Global->CtrlObject->Cp()->ActivePanel->GetType()==QVIEW_PANEL)
		OpMode|=OPM_QUICKVIEW;

	// � �� ����� �� ������ ���������� �� ������� ������?
	if (!hPlugin || hPlugin==PANEL_ACTIVE || hPlugin==PANEL_PASSIVE)
	{
		/* $ 30.11.2001 DJ
			� ���������� �� ��� ������?
		*/
		HANDLE Handle = ((!hPlugin || hPlugin==PANEL_ACTIVE)?Global->CtrlObject->Cp()->ActivePanel:Global->CtrlObject->Cp()->GetAnotherPanel(Global->CtrlObject->Cp()->ActivePanel))->GetPluginHandle();

		if (!Handle)
			return FALSE;

		DirListPlugin=*(PluginHandle *)Handle;
	}
	else
	{
		DirListPlugin.pPlugin=(Plugin*)PluginNumber;
		DirListPlugin.hPlugin=hPlugin;
	}


	{
		SaveScreen SaveScr;
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_FarGetPluginDirListMsg);
		{
			string strDirName;
			strDirName = Dir;
			TruncStr(strDirName,30);
			CenterStr(strDirName,strDirName,30);
			SetCursorType(FALSE,0);
			FarGetPluginDirListMsg(strDirName,0);
			PluginSearchMsgOut=FALSE;
			hDirListPlugin=(HANDLE)&DirListPlugin;
			StopSearch=FALSE;

			auto PluginDirList = new std::vector<PluginPanelItem>;
			// first item is reserved for internal needs
			PluginDirList->emplace_back();
			PluginDirList->front().Reserved[0] = reinterpret_cast<intptr_t>(PluginDirList);

			*pItemsNumber = 0;
			*pPanelItem = nullptr;
			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&Info);
			string strPrevDir = Info.CurDir;

			struct UserDataItem UserData={0};  // How to find the value of a variable?

			if (Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,Dir,OPM_SILENT|OpMode,&UserData))
			{
				string strPluginSearchPath = Dir;
				strPluginSearchPath += L"\x1";
				ScanPluginDir(OpMode,strPluginSearchPath, *PluginDirList);


				*pPanelItem = PluginDirList->data() + 1;
				*pItemsNumber = PluginDirList->size() - 1;

				Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,L"..",OPM_SILENT|OpMode);
				OpenPanelInfo NewInfo;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&NewInfo);

				if (StrCmpI(strPrevDir, NewInfo.CurDir) )
				{
					PluginPanelItem *PanelData=nullptr;
					size_t ItemCount=0;

					if (Global->CtrlObject->Plugins->GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_SILENT|OpMode))
					{
						Global->CtrlObject->Plugins->FreeFindData(hDirListPlugin,PanelData,ItemCount,true);
					}

					Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,strPrevDir,OPM_SILENT|OpMode,&Info.UserData);
				}
			}
		}
	}
	return !StopSearch;
}

void FreePluginDirList(HANDLE hPlugin, PluginPanelItem *PanelItem)
{
	if (!PanelItem)
		return;

	auto PluginDirList = reinterpret_cast<std::vector<PluginPanelItem>*>((PanelItem-1)->Reserved[0]);

	// first item is reserved for internal needs
	std::for_each(PluginDirList->begin() + 1, PluginDirList->end(), [&](VALUE_TYPE(PluginDirList)& i)
	{
		if(i.UserData.FreeData)
		{
			FarPanelItemFreeInfo info={sizeof(FarPanelItemFreeInfo),hPlugin};
			i.UserData.FreeData(i.UserData.Data,&info);
		}
		FreePluginPanelItem(&i);
	});

	delete PluginDirList;
}

int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
                     unsigned long &FileCount,unsigned __int64 &FileSize,
                     unsigned __int64 &CompressedFileSize)
{
	PluginPanelItem *PanelItem=nullptr;
	size_t ItemsNumber=0;
	int ExitCode;
	DirCount=FileCount=0;
	FileSize=CompressedFileSize=0;
	PluginHandle *ph = (PluginHandle*)hPlugin;

	if ((ExitCode=GetPluginDirList(ph->pPlugin, ph->hPlugin, DirName, &PanelItem,&ItemsNumber))==TRUE) //intptr_t - BUGBUG
	{
		for (size_t I=0; I<ItemsNumber; I++)
		{
			if (PanelItem[I].FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DirCount++;
			}
			else
			{
				FileCount++;
				FileSize+=PanelItem[I].FileSize;
				CompressedFileSize+=PanelItem[I].AllocationSize?PanelItem[I].AllocationSize:PanelItem[I].FileSize;
			}
		}
	}

	if (PanelItem)
	{
		FreePluginDirList(ph->hPlugin, PanelItem);
	}

	return(ExitCode);
}
