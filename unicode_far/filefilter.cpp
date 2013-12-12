/*
filefilter.cpp

�������� ������
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

#include "filefilter.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu2.hpp"
#include "scantree.hpp"
#include "filelist.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "mix.hpp"
#include "configdb.hpp"
#include "keyboard.hpp"
#include "DlgGuid.hpp"
#include "language.hpp"

static std::vector<FileFilterParams> *FilterData, *TempFilterData;

FileFilterParams *FoldersFilter;

static bool bMenuOpen = false;

static bool Changed = false;

FileFilter::FileFilter(Panel *HostPanel, FAR_FILE_FILTER_TYPE FilterType):
	m_HostPanel(HostPanel),
	m_FilterType(FilterType)
{
	UpdateCurrentTime();
}

FileFilter::~FileFilter()
{
}

Panel *FileFilter::GetHostPanel()
{
	if (!m_HostPanel || m_HostPanel == (Panel *)PANEL_ACTIVE)
	{
		return Global->CtrlObject->Cp()->ActivePanel;
	}
	else if (m_HostPanel == (Panel *)PANEL_PASSIVE)
	{
		return Global->CtrlObject->Cp()->GetAnotherPanel(Global->CtrlObject->Cp()->ActivePanel);
	}

	return m_HostPanel;
}

bool FileFilter::FilterEdit()
{
	if (bMenuOpen)
		return false;

	Changed = true;
	bMenuOpen = true;
	int ExitCode;
	bool bNeedUpdate=false;
	VMenu2 FilterList(MSG(MFilterTitle),nullptr,0,ScrY-6);
	FilterList.SetHelp(L"FiltersMenu");
	FilterList.SetPosition(-1,-1,0,0);
	FilterList.SetBottomTitle(MSG(MFilterBottom));
	FilterList.SetFlags(/*VMENU_SHOWAMPERSAND|*/VMENU_WRAPMODE);
	FilterList.SetId(FiltersMenuId);

	std::for_each(RANGE(*FilterData, i)
	{
		MenuItemEx ListItem(MenuString(&i));
		ListItem.SetCheck(GetCheck(i));
		FilterList.AddItem(ListItem);
	});

	if (m_FilterType != FFT_CUSTOM)
	{
		typedef std::list<std::pair<string, int>> extension_list;
		extension_list Extensions;

		{
			auto FFFT = GetFFFT();

			FOR_CONST_RANGE(*TempFilterData, i)
			{
				//AY: ����� ���������� ������ �� ��������� ���� �������
				//(��� ������� ���� ������ �� ������) ������� ������� � ������� ������� ����
				if (!i->GetFlags(FFFT))
					continue;

				const wchar_t *FMask;
				i->GetMask(&FMask);
				string strMask = FMask;
				Unquote(strMask);

				if (!ParseAndAddMasks(Extensions, strMask, 0, GetCheck(*i)))
					break;
			}
		}

		{
			MenuItemEx ListItem;
			ListItem.Flags|=LIF_SEPARATOR;
			FilterList.AddItem(ListItem);
		}

		{
			FoldersFilter->SetTitle(MSG(MFolderFileType));
			MenuItemEx ListItem(MenuString(FoldersFilter,false,L'0'));
			int Check = GetCheck(*FoldersFilter);

			if (Check)
				ListItem.SetCheck(Check);

			FilterList.AddItem(ListItem);
		}

		if (GetHostPanel()->GetMode()==NORMAL_PANEL)
		{
			string strFileName;
			api::FAR_FIND_DATA fdata;
			ScanTree ScTree(FALSE,FALSE);
			ScTree.SetFindPath(GetHostPanel()->GetCurDir(), L"*");

			while (ScTree.GetNextName(&fdata,strFileName))
				if (!ParseAndAddMasks(Extensions, fdata.strFileName, fdata.dwFileAttributes, 0))
					break;
		}
		else
		{
			string strFileName;
			DWORD FileAttr;

			for (int i=0; GetHostPanel()->GetFileName(strFileName,i,FileAttr); i++)
				if (!ParseAndAddMasks(Extensions, strFileName, FileAttr, 0))
					break;
		}

		Extensions.sort([](const extension_list::value_type& a, const extension_list::value_type& b)
		{
			return StrCmpI(a.first, b.first) < 0;
		});

		wchar_t h = L'1';
		for (auto i = Extensions.begin(); i != Extensions.end(); ++i, (h == L'9'? h = L'A' : (h == L'Z' || h? h++ : h=0)))
		{
			MenuItemEx ListItem(MenuString(nullptr, false, h, true, i->first.data(), MSG(MPanelFileType)));
			size_t Length = i->first.size() + 1;
			ListItem.SetCheck(i->second);
			FilterList.SetUserData(i->first.data(), Length * sizeof(wchar_t), FilterList.AddItem(ListItem));
		}
	}

	ExitCode=FilterList.RunEx([&](int Msg, void *param)->int
	{
		if (Msg==DN_LISTHOTKEY)
			return 1;
		if (Msg!=DN_INPUT)
			return 0;

		int Key=InputRecordToKey(static_cast<INPUT_RECORD*>(param));

		if (Key==KEY_ADD)
			Key=L'+';
		else if (Key==KEY_SUBTRACT)
			Key=L'-';
		else if (Key==L'i')
			Key=L'I';
		else if (Key==L'x')
			Key=L'X';

		int KeyProcessed = 1;

		switch (Key)
		{
			case L'+':
			case L'-':
			case L'I':
			case L'X':
			case KEY_SPACE:
			case KEY_BS:
			{
				int SelPos=FilterList.GetSelectPos();

				if (SelPos<0)
					break;

				int Check=FilterList.GetCheck(SelPos);
				int NewCheck;

				if (Key==KEY_BS)
					NewCheck = 0;
				else if (Key==KEY_SPACE)
					NewCheck = Check ? 0 : L'+';
				else
					NewCheck = (Check == Key) ? 0 : Key;

				FilterList.SetCheck(NewCheck,SelPos);
				FilterList.SetSelectPos(SelPos,1);
				FilterList.Key(KEY_DOWN);
				bNeedUpdate=true;
				return 1;
			}
			case KEY_SHIFTBS:
			{
				for (int I=0; I < FilterList.GetItemCount(); I++)
				{
					FilterList.SetCheck(FALSE, I);
				}

				break;
			}
			case KEY_F4:
			{
				int SelPos=FilterList.GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData->size())
				{
					if (FileFilterConfig(&FilterData->at(SelPos)))
					{
						MenuItemEx ListItem(MenuString(&FilterData->at(SelPos)));
						int Check = GetCheck(FilterData->at(SelPos));

						if (Check)
							ListItem.SetCheck(Check);

						FilterList.DeleteItem(SelPos);
						FilterList.AddItem(ListItem,SelPos);
						FilterList.SetSelectPos(SelPos,1);
						bNeedUpdate=true;
					}
				}
				else if (SelPos>(int)FilterData->size())
				{
					Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanEditCustomFilterOnly),MSG(MOk));
				}

				break;
			}
			case KEY_NUMPAD0:
			case KEY_INS:
			case KEY_F5:
			{
				int pos=FilterList.GetSelectPos();
				if (pos<0)
				{
					if (Key==KEY_F5)
						break;
					pos=0;
				}
				size_t SelPos=pos;
				size_t SelPos2=pos+1;

				SelPos = std::min(FilterData->size(), SelPos);

				auto& NewFilter = *FilterData->emplace(FilterData->begin()+SelPos, FileFilterParams());

				if (Key==KEY_F5)
				{
					if (SelPos2 < FilterData->size())
					{
						NewFilter = FilterData->at(SelPos2).Clone();
						NewFilter.SetTitle(L"");
						NewFilter.ClearAllFlags();
					}
					else if (SelPos2 == FilterData->size()+2)
					{
						NewFilter = FoldersFilter->Clone();
						NewFilter.SetTitle(L"");
						NewFilter.ClearAllFlags();
					}
					else if (SelPos2 > FilterData->size()+2)
					{
						NewFilter.SetMask(1,static_cast<const wchar_t*>(FilterList.GetUserData(nullptr, 0, static_cast<int>(SelPos2-1))));
						//���� ������� ��� ������ ��� ������, ����� �� ������ � ��� ���������
						NewFilter.SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
					}
					else
					{
						FilterData->erase(FilterData->begin()+SelPos);
						break;
					}
				}
				else
				{
					//AY: ��� ������ ����� ������ �� ����� ����� ������� ���� �� ����� ������ ��� ������
					NewFilter.SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
				}

				if (FileFilterConfig(&NewFilter))
				{
					MenuItemEx ListItem(MenuString(&NewFilter));
					FilterList.AddItem(ListItem,static_cast<int>(SelPos));
					FilterList.SetSelectPos(static_cast<int>(SelPos),1);
					bNeedUpdate=true;
				}
				else
				{
					FilterData->erase(FilterData->begin()+SelPos);
				}
				break;
			}
			case KEY_NUMDEL:
			case KEY_DEL:
			{
				int SelPos=FilterList.GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData->size())
				{
					string strQuotedTitle=FilterData->at(SelPos).GetTitle();
					InsertQuote(strQuotedTitle);

					if (!Message(0,2,MSG(MFilterTitle),MSG(MAskDeleteFilter),
					            strQuotedTitle.data(),MSG(MDelete),MSG(MCancel)))
					{
						FilterData->erase(FilterData->begin()+SelPos);
						FilterList.DeleteItem(SelPos);
						FilterList.SetSelectPos(SelPos,1);
						bNeedUpdate=true;
					}
				}
				else if (SelPos>(int)FilterData->size())
				{
					Message(MSG_WARNING,1,MSG(MFilterTitle),MSG(MCanDeleteCustomFilterOnly),MSG(MOk));
				}

				break;
			}
			case KEY_CTRLUP:
			case KEY_RCTRLUP:
			case KEY_CTRLDOWN:
			case KEY_RCTRLDOWN:
			{
				int SelPos=FilterList.GetSelectPos();
				if (SelPos<0)
					break;

				if (SelPos<(int)FilterData->size() && !((Key==KEY_CTRLUP || Key==KEY_RCTRLUP) && !SelPos) &&
					!((Key==KEY_CTRLDOWN || Key==KEY_RCTRLDOWN) && SelPos==(int)(FilterData->size()-1)))
				{
					int NewPos = SelPos + ((Key == KEY_CTRLDOWN || Key == KEY_RCTRLDOWN) ? 1 : -1);
					std::swap(*FilterList.GetItemPtr(SelPos), *FilterList.GetItemPtr(NewPos));
					auto i1 = FilterData->begin() + NewPos, i2 = FilterData->begin() + SelPos;
					std::swap(*i1, *i2);
					FilterList.SetSelectPos(NewPos,1);
					bNeedUpdate=true;
				}

				break;
			}

			default:
				KeyProcessed = 0;
		}
		return KeyProcessed;
	});


	if (ExitCode!=-1)
		ProcessSelection(&FilterList);

	if (Global->Opt->AutoSaveSetup)
		SaveFilters();

	if (ExitCode!=-1 || bNeedUpdate)
	{
		if (m_FilterType == FFT_PANEL)
		{
			GetHostPanel()->Update(UPDATE_KEEP_SELECTION);
			GetHostPanel()->Redraw();
		}
	}

	bMenuOpen = false;
	return (ExitCode!=-1);
}

const enumFileFilterFlagsType FileFilter::GetFFFT()
{
	if (m_FilterType == FFT_PANEL)
	{
		if (GetHostPanel() == Global->CtrlObject->Cp()->RightPanel)
		{
			return FFFT_RIGHTPANEL;
		}
		else
		{
			return FFFT_LEFTPANEL;
		}
	}
	else if (m_FilterType == FFT_COPY)
	{
		return FFFT_COPY;
	}
	else if (m_FilterType == FFT_FINDFILE)
	{
		return FFFT_FINDFILE;
	}
	else if (m_FilterType == FFT_SELECT)
	{
		return FFFT_SELECT;
	}
	return FFFT_CUSTOM;
}

int FileFilter::GetCheck(const FileFilterParams& FFP)
{
	DWORD Flags = FFP.GetFlags(GetFFFT());

	if (Flags&FFF_INCLUDE)
	{
		if (Flags&FFF_STRONG)
			return L'I';

		return L'+';
	}
	else if (Flags&FFF_EXCLUDE)
	{
		if (Flags&FFF_STRONG)
			return L'X';

		return L'-';
	}

	return 0;
}

void FileFilter::ProcessSelection(VMenu2 *FilterList)
{
	auto FFFT = GetFFFT();

	for (int i=0,j=0; i < FilterList->GetItemCount(); i++)
	{
		int Check=FilterList->GetCheck(i);
		FileFilterParams* CurFilterData=nullptr;

		if (i < (int)FilterData->size())
		{
			CurFilterData = &FilterData->at(i);
		}
		else if (i == (int)(FilterData->size() + 1))
		{
			CurFilterData = FoldersFilter;
		}
		else if (i > (int)(FilterData->size() + 1))
		{
			const wchar_t *FMask=nullptr;
			string Mask(static_cast<const wchar_t*>(FilterList->GetUserData(nullptr, 0, i)));
			string strMask1(Mask);
			//AY: ��� ��� � ���� �� ���������� ������ �� ��������� ���� �������
			//������� ������� � ������� ������� ���� � TempFilterData ������
			//����� ��������� ����� ������� ���� ��� ������� � ���� ���� ��
			//��� ��� ���� ������� � ������ � ��� ��� TempFilterData
			//� ���� ������� � ���� ������������� �� �������� �� �������
			//��������� ���� �� ���� ���������� � ������.
			Unquote(strMask1);

			while (j < static_cast<int>(TempFilterData->size()))
			{
				CurFilterData = &TempFilterData->at(j);
				string strMask2;
				CurFilterData->GetMask(&FMask);
				strMask2 = FMask;
				Unquote(strMask2);

				if (StrCmpI(strMask1, strMask2) < 1)
					break;

				j++;
			}

			if (CurFilterData)
			{
				if (!StrCmpI(Mask.data(),FMask))
				{
					if (!Check)
					{
						bool bCheckedNowhere = true;

						for (int n=FFFT_FIRST; n < FFFT_COUNT; n++)
						{
							if (n != FFFT && CurFilterData->GetFlags((enumFileFilterFlagsType)n))
							{
								bCheckedNowhere = false;
								break;
							}
						}

						if (bCheckedNowhere)
						{
							TempFilterData->erase(TempFilterData->begin()+j);
							continue;
						}
					}
					else
					{
						j++;
					}
				}
				else
					CurFilterData=nullptr;
			}

			if (Check && !CurFilterData)
			{
					auto& NewFilter = *TempFilterData->emplace(TempFilterData->begin() + j, FileFilterParams());
					NewFilter.SetMask(1,Mask);
					//���� ������� ��� ������ ��� ������, ����� �� ������ � ��� ���������
					NewFilter.SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
					j++;
					CurFilterData = &NewFilter;
			}
		}

		if (!CurFilterData)
			continue;

		CurFilterData->SetFlags(FFFT, FFF_NONE);

		if (Check==L'+')
			CurFilterData->SetFlags(FFFT, FFF_INCLUDE);
		else if (Check==L'-')
			CurFilterData->SetFlags(FFFT, FFF_EXCLUDE);
		else if (Check==L'I')
			CurFilterData->SetFlags(FFFT, FFF_INCLUDE|FFF_STRONG);
		else if (Check==L'X')
			CurFilterData->SetFlags(FFFT, FFF_EXCLUDE|FFF_STRONG);
	}
}

void FileFilter::UpdateCurrentTime()
{
	SYSTEMTIME cst;
	GetSystemTime(&cst);
	FILETIME cft;
	SystemTimeToFileTime(&cst, &cft);
	ULARGE_INTEGER current = {cft.dwLowDateTime, cft.dwHighDateTime};
	CurrentTime = current.QuadPart;
}

bool FileFilter::FileInFilter(const FileListItem* fli,enumFileInFilterType *foundType)
{
	api::FAR_FIND_DATA fde;
	fde.dwFileAttributes=fli->FileAttr;
	fde.ftCreationTime=fli->CreationTime;
	fde.ftLastAccessTime=fli->AccessTime;
	fde.ftLastWriteTime=fli->WriteTime;
	fde.ftChangeTime=fli->ChangeTime;
	fde.nFileSize=fli->FileSize;
	fde.nAllocationSize=fli->AllocationSize;
	fde.strFileName=fli->strName;
	fde.strAlternateFileName=fli->strShortName;
	return FileInFilter(fde, foundType, &fli->strName);
}

bool FileFilter::FileInFilter(const api::FAR_FIND_DATA& fde,enumFileInFilterType *foundType, const string* FullName)
{
	auto FFFT = GetFFFT();
	bool bFound=false;
	bool bAnyIncludeFound=false;
	bool bAnyFolderIncludeFound=false;
	bool bInc=false;
	bool bFolder=(fde.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
	DWORD Flags;

	for (size_t i=0; i<FilterData->size(); i++)
	{
		const auto& CurFilterData = FilterData->at(i);
		Flags = CurFilterData.GetFlags(FFFT);

		if (Flags)
		{
			if (bFound && !(Flags&FFF_STRONG))
				continue;

			if (Flags&FFF_INCLUDE)
			{
				bAnyIncludeFound = true;
				DWORD AttrClear;

				if (CurFilterData.GetAttr(nullptr,&AttrClear))
					bAnyFolderIncludeFound = bAnyFolderIncludeFound || !(AttrClear&FILE_ATTRIBUTE_DIRECTORY);
			}

			if (CurFilterData.FileInFilter(fde, CurrentTime, FullName))
			{
				bFound = true;

				if (Flags&FFF_INCLUDE)
					bInc = true;
				else
					bInc = false;

				if (Flags&FFF_STRONG)
					goto final;
			}
		}
	}

	//����-������ �����
	if (FFFT != FFFT_CUSTOM)
	{
		Flags = FoldersFilter->GetFlags(FFFT);

		if (Flags && (!bFound || (Flags&FFF_STRONG)))
		{
			if (Flags&FFF_INCLUDE)
			{
				bAnyIncludeFound = true;
				bAnyFolderIncludeFound = true;
			}

			if (bFolder && FoldersFilter->FileInFilter(fde, CurrentTime, FullName))
			{
				bFound = true;

				if (Flags&FFF_INCLUDE)
					bInc = true;
				else
					bInc = false;

				if (Flags&FFF_STRONG)
					goto final;
			}
		}
	}

	//����-�������
	for (size_t i=0; i<TempFilterData->size(); i++)
	{
		const auto& CurFilterData = TempFilterData->at(i);
		Flags = CurFilterData.GetFlags(FFFT);

		if (Flags && (!bFound || (Flags&FFF_STRONG)))
		{
			bAnyIncludeFound = bAnyIncludeFound || (Flags&FFF_INCLUDE);

			if (bFolder) //����-������� ������� �� ����� ���� ��� �����
				continue;

			if (CurFilterData.FileInFilter(fde, CurrentTime, FullName))
			{
				bFound = true;

				if (Flags&FFF_INCLUDE)
					bInc = true;
				else
					bInc = false;

				if (Flags&FFF_STRONG)
					goto final;
			}
		}
	}

	//���� ����� � ��� �� ������ �� ��� ����� exclude ������ �� ����� ��������
	//����� ������� �� include ���� ������ ����� include �������� �� �����.
	//� ��� Select �������� ����� �������� ����� �� ��������� �������.
	if (!bFound && bFolder && !bAnyFolderIncludeFound && m_FilterType!=FFT_SELECT)
	{
		if (foundType)
			*foundType=FIFT_INCLUDE; //???

		return true;
	}

final:

	if (foundType)
		*foundType=!bFound?FIFT_NOTINTFILTER:(bInc?FIFT_INCLUDE:FIFT_EXCLUDE);

	if (bFound) return bInc;

	//���� ������� �� ����� �� ��� ���� ������ �� �� ����� �������
	//������ ���� �� ���� �� ������ Include ������� (�.�. ���� ������ ������� ����������).
	return !bAnyIncludeFound;
}

bool FileFilter::FileInFilter(const PluginPanelItem& fd,enumFileInFilterType *foundType)
{
	api::FAR_FIND_DATA fde;
	PluginPanelItemToFindDataEx(&fd,&fde);
	return FileInFilter(fde, foundType, &fde.strFileName);
}

bool FileFilter::IsEnabledOnPanel()
{
	if (m_FilterType != FFT_PANEL)
		return false;

	auto FFFT = GetFFFT();

	if (std::any_of(CONST_RANGE(*FilterData, i) { return i.GetFlags(FFFT); }))
		return true;
	
	if (FoldersFilter->GetFlags(FFFT))
		return true;

	return std::any_of(CONST_RANGE(*TempFilterData, i) { return i.GetFlags(FFFT); });
}

void FileFilter::InitFilter()
{
	if(!FilterData)
		FilterData = new PTRTYPE(FilterData);
	if(!TempFilterData)
		TempFilterData = new PTRTYPE(TempFilterData);

	string strTitle, strMask, strSizeBelow, strSizeAbove;

	auto cfg = Global->Db->CreateFiltersConfig();

	unsigned __int64 root = cfg->GetKeyID(0, L"Filters");

	{
		static FileFilterParams _FoldersFilter;
		FoldersFilter = &_FoldersFilter;
		FoldersFilter->SetMask(0,L"*");
		FoldersFilter->SetAttr(1,FILE_ATTRIBUTE_DIRECTORY,0);

		if (!root)
		{
			return;
		}

		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(root,L"FoldersFilterFFlags", Flags, sizeof(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			FoldersFilter->SetFlags((enumFileFilterFlagsType)i, Flags[i]);
	}

	while (1)
	{
		unsigned __int64 key = cfg->GetKeyID(root, L"Filter" + std::to_wstring(FilterData->size()));

		if (!key || !cfg->GetValue(key,L"Title",strTitle))
			break;

		FilterData->emplace_back(FileFilterParams());

		//��������� �������� ������� ��� ���� ��� ����� ���������� ���������
		//��������� ������ ������ ����.
		FilterData->back().SetTitle(strTitle);

		strMask.clear();
		cfg->GetValue(key,L"Mask",strMask);
		unsigned __int64 UseMask = 1;
		cfg->GetValue(key,L"UseMask",&UseMask);
		FilterData->back().SetMask(UseMask!=0, strMask);

		FILETIME DateAfter = {}, DateBefore = {};
		cfg->GetValue(key,L"DateAfter", &DateAfter, sizeof(DateAfter));
		cfg->GetValue(key,L"DateBefore", &DateBefore, sizeof(DateBefore));

		unsigned __int64 UseDate = 0;
		cfg->GetValue(key,L"UseDate",&UseDate);
		unsigned __int64 DateType = 0;
		cfg->GetValue(key,L"DateType",&DateType);
		unsigned __int64 RelativeDate = 0;
		cfg->GetValue(key,L"RelativeDate",&RelativeDate);
		FilterData->back().SetDate(UseDate!=0, (DWORD)DateType, DateAfter, DateBefore, RelativeDate!=0);

		strSizeAbove.clear();
		cfg->GetValue(key,L"SizeAboveS",strSizeAbove);
		strSizeBelow.clear();
		cfg->GetValue(key,L"SizeBelowS",strSizeBelow);
		unsigned __int64 UseSize = 0;
		cfg->GetValue(key,L"UseSize",&UseSize);
		FilterData->back().SetSize(UseSize!=0, strSizeAbove, strSizeBelow);

		unsigned __int64 UseHardLinks = 0;
		cfg->GetValue(key,L"UseHardLinks",&UseHardLinks);
		unsigned __int64 HardLinksAbove;
		cfg->GetValue(key,L"HardLinksAbove",&HardLinksAbove);
		unsigned __int64 HardLinksBelow;
		cfg->GetValue(key,L"HardLinksAbove",&HardLinksBelow);
		FilterData->back().SetHardLinks(UseHardLinks!=0,HardLinksAbove,HardLinksBelow);

		unsigned __int64 UseAttr = 1;
		cfg->GetValue(key,L"UseAttr",&UseAttr);
		unsigned __int64 AttrSet = 0;
		cfg->GetValue(key,L"AttrSet", &AttrSet);
		unsigned __int64 AttrClear = FILE_ATTRIBUTE_DIRECTORY;
		cfg->GetValue(key,L"AttrClear",&AttrClear);
		FilterData->back().SetAttr(UseAttr!=0, (DWORD)AttrSet, (DWORD)AttrClear);

		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(key,L"FFlags", Flags, sizeof(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			FilterData->back().SetFlags((enumFileFilterFlagsType)i, Flags[i]);
	}

	while (1)
	{
		unsigned __int64 key = cfg->GetKeyID(root, L"PanelMask" + std::to_wstring(TempFilterData->size()));

		if (!key || !cfg->GetValue(key,L"Mask",strMask))
			break;

		TempFilterData->emplace_back(FileFilterParams());

		TempFilterData->back().SetMask(1,strMask);
		//���� ������� ��� ������ ��� ������, ����� �� ������ � ��� ���������
		TempFilterData->back().SetAttr(1,0,FILE_ATTRIBUTE_DIRECTORY);
		DWORD Flags[FFFT_COUNT] = {};
		cfg->GetValue(key,L"FFlags", Flags, sizeof(Flags));

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			TempFilterData->back().SetFlags((enumFileFilterFlagsType)i, Flags[i]);
	}
}


void FileFilter::CloseFilter()
{
	if(FilterData)
	{
		delete FilterData;
		FilterData = nullptr;
	}

	if(TempFilterData)
	{
		delete TempFilterData;
		TempFilterData = nullptr;
	}
}

void FileFilter::SaveFilters()
{
	if (!Changed)
		return;

	Changed = false;

	auto cfg = Global->Db->CreateFiltersConfig();

	unsigned __int64 root = cfg->GetKeyID(0, L"Filters");
	if (root)
		cfg->DeleteKeyTree(root);

	root = cfg->CreateKey(0, L"Filters");

	if (!root)
	{
		return;
	}

	for (size_t i=0; i<FilterData->size(); i++)
	{
		unsigned __int64 key = cfg->CreateKey(root, L"Filter" + std::to_wstring(i));
		if (!key)
			break;
		const auto& CurFilterData = FilterData->at(i);

		cfg->SetValue(key,L"Title",CurFilterData.GetTitle());
		const wchar_t *Mask;
		cfg->SetValue(key,L"UseMask",CurFilterData.GetMask(&Mask)?1:0);
		cfg->SetValue(key,L"Mask",Mask);
		DWORD DateType;
		FILETIME DateAfter, DateBefore;
		bool bRelative;
		cfg->SetValue(key,L"UseDate",CurFilterData.GetDate(&DateType, &DateAfter, &DateBefore, &bRelative)?1:0);
		cfg->SetValue(key,L"DateType",DateType);
		cfg->SetValue(key,L"DateAfter", &DateAfter, sizeof(DateAfter));
		cfg->SetValue(key,L"DateBefore", &DateBefore, sizeof(DateBefore));
		cfg->SetValue(key,L"RelativeDate",bRelative?1:0);
		cfg->SetValue(key, L"UseSize", CurFilterData.IsSizeUsed());
		cfg->SetValue(key, L"SizeAboveS", CurFilterData.GetSizeAbove());
		cfg->SetValue(key, L"SizeBelowS", CurFilterData.GetSizeBelow());
		DWORD HardLinksAbove,HardLinksBelow;
		cfg->SetValue(key,L"UseHardLinks",CurFilterData.GetHardLinks(&HardLinksAbove,&HardLinksBelow)?1:0);
		cfg->SetValue(key,L"HardLinksAboveS", HardLinksAbove);
		cfg->SetValue(key,L"HardLinksBelowS", HardLinksBelow);
		DWORD AttrSet, AttrClear;
		cfg->SetValue(key,L"UseAttr",CurFilterData.GetAttr(&AttrSet, &AttrClear)?1:0);
		cfg->SetValue(key,L"AttrSet",AttrSet);
		cfg->SetValue(key,L"AttrClear",AttrClear);
		DWORD Flags[FFFT_COUNT];

		for (DWORD j=FFFT_FIRST; j < FFFT_COUNT; j++)
			Flags[j] = CurFilterData.GetFlags((enumFileFilterFlagsType)j);

		cfg->SetValue(key,L"FFlags", Flags, sizeof(Flags));
	}

	for (size_t i=0; i<TempFilterData->size(); i++)
	{
		unsigned __int64 key = cfg->CreateKey(root, L"PanelMask" + std::to_wstring(i));
		if (!key)
			break;
		const auto& CurFilterData = TempFilterData->at(i);

		const wchar_t *Mask;
		CurFilterData.GetMask(&Mask);
		cfg->SetValue(key,L"Mask",Mask);
		DWORD Flags[FFFT_COUNT];

		for (DWORD j=FFFT_FIRST; j < FFFT_COUNT; j++)
			Flags[j] = CurFilterData.GetFlags((enumFileFilterFlagsType)j);

		cfg->SetValue(key,L"FFlags", Flags, sizeof(Flags));
	}

	{
		DWORD Flags[FFFT_COUNT];

		for (DWORD i=FFFT_FIRST; i < FFFT_COUNT; i++)
			Flags[i] = FoldersFilter->GetFlags((enumFileFilterFlagsType)i);

		cfg->SetValue(root,L"FoldersFilterFFlags", Flags, sizeof(Flags));
	}
}

void FileFilter::SwapPanelFlags(FileFilterParams& CurFilterData)
{
	DWORD LPFlags = CurFilterData.GetFlags(FFFT_LEFTPANEL);
	DWORD RPFlags = CurFilterData.GetFlags(FFFT_RIGHTPANEL);
	CurFilterData.SetFlags(FFFT_LEFTPANEL,  RPFlags);
	CurFilterData.SetFlags(FFFT_RIGHTPANEL, LPFlags);
}

void FileFilter::SwapFilter()
{
	Changed = true;
	std::for_each(RANGE(*FilterData, i) { SwapPanelFlags(i); });
	SwapPanelFlags(*FoldersFilter);
	std::for_each(RANGE(*TempFilterData, i) { SwapPanelFlags(i); });
}

int FileFilter::ParseAndAddMasks(std::list<std::pair<string, int>>& Extensions, const string& FileName, DWORD FileAttr, int Check)
{
	if (FileName == L"." || TestParentFolderName(FileName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		return -1;

	size_t DotPos = FileName.rfind(L'.');
	string strMask;

	// ���� ����� �������� ����������� (',' ��� ';'), �� ������� �� � �������
	if (DotPos == string::npos)
		strMask = L"*.";
	else if (FileName.find_last_of(L",;", DotPos) != string::npos)
		strMask.assign(L"\"*", 2).append(FileName, DotPos, string::npos).append(1, '"');
	else
		strMask.assign(1, L'*').append(FileName, DotPos, string::npos);

	if (std::any_of(CONST_RANGE(Extensions, i) {return !StrCmpI(i.first, strMask);}))
		return -1;

	Extensions.emplace_back(VALUE_TYPE(Extensions)(strMask, Check));
	return 1;
}
