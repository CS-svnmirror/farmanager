/*
codepage.cpp

������ � �������� ����������
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

#include "codepage.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "savefpos.hpp"
#include "keys.hpp"
#include "registry.hpp"
#include "language.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "config.hpp"

// ���� ��� �������� ����� ������� �������
const wchar_t *NamesOfCodePagesKey = L"CodePages\\Names";

const wchar_t *FavoriteCodePagesKey = L"CodePages\\Favorites";

// ����������� ������� ��������
enum StandardCodePages
{
	SearchAll = 1,
	Auto = 2,
	OEM = 4,
	ANSI = 8,
	UTF7 = 16,
	UTF8 = 32,
	UTF16LE = 64,
	UTF16BE = 128,
	AllStandard = OEM | ANSI | UTF7 | UTF8 | UTF16BE | UTF16LE
};

// �������� ������ �������� ������� �� ������� ���������
enum CodePagesCallbackCallSource
{
	CodePageSelect,
	CodePagesFill,
	CodePageCheck
};

// ������
static HANDLE dialog;
// ������������ �������
static UINT control;
// ����
static VMenu *CodePages = NULL;
// ������� ������� ��������
static UINT currentCodePage;
// ���������� ��������� � ������������ ������ ��������
static int favoriteCodePages, normalCodePages;
// ������� ������������� ���������� ������� �������� ��� ������
static bool selectedCodePages;
// �������� ������ �������� ��� ������� EnumSystemCodePages
static CodePagesCallbackCallSource CallbackCallSource;
// ������� ����, ��� ������� �������� ��������������
static bool CodePageSupported;

// ��������� ������� ��������
void AddCodePage(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true, bool checked = false)
{
	if (CallbackCallSource == CodePagesFill)
	{
		// ��������� ������� ������������ ��������
		if (position==-1)
		{
			FarListInfo info;
			SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
			position = info.ItemsNumber;
		}

		// ��������� �������
		FarListInsert item = {position};
		UnicodeString name;

		if (codePage==CP_AUTODETECT)
			name = codePageName;
		else
			name.Format(L"%5u%c %s", codePage, BoxSymbols[BS_V1], codePageName);

		item.Item.Text = name.GetBuffer();

		if (selectedCodePages && checked)
		{
			item.Item.Flags |= MIF_CHECKED;
		}

		if (!enabled)
		{
			item.Item.Flags |= MIF_GRAYED;
		}

		SendDlgMessage(dialog, DM_LISTINSERT, control, (LONG_PTR)&item);
		// ������������� ������ ��� ��������
		FarListItemData data;
		data.Index = position;
		data.Data = (void*)(DWORD_PTR)codePage;
		data.DataSize = sizeof(UINT);
		SendDlgMessage(dialog, DM_LISTSETDATA, control, (LONG_PTR)&data);
	}
	else
	{
		// ������ ����� ������� ����
		MenuItemEx item;
		item.Clear();

		if (!enabled)
			item.Flags |= MIF_GRAYED;

		if (codePage==CP_AUTODETECT)
			item.strName = codePageName;
		else
			item.strName.Format(L"%5u%c %s", codePage, BoxSymbols[BS_V1], codePageName);

		item.UserData = (char *)(UINT_PTR)codePage;
		item.UserDataSize = sizeof(UINT);

		// ��������� ����� ������� � ����
		if (position>=0)
			CodePages->AddItem(&item, position);
		else
			CodePages->AddItem(&item);

		// ���� ���� ������������� ������ �� ����������� �������
		if (currentCodePage==codePage && (CodePages->GetSelectPos()==-1 || (UINT)(UINT_PTR)CodePages->GetItemPtr()->UserData!=codePage))
			CodePages->SetSelectPos(position>=0?position:CodePages->GetItemCount()-1, 1);
		// ������������ ������� ���������� ��������
		else if (position!=-1 && CodePages->GetSelectPos()>=position)
			CodePages->SetSelectPos(CodePages->GetSelectPos()+1, 1);
	}
}

// ��������� ����������� ������� ��������
void AddStandardCodePage(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true)
{
	bool checked = false;

	if (selectedCodePages && codePage!=CP_AUTODETECT)
	{
		string strCodePageName;
		strCodePageName.Format(L"%u", codePage);
		int selectType = 0;
		GetRegKey(FavoriteCodePagesKey, strCodePageName, selectType, 0);

		if (selectType & CPST_FIND)
			checked = true;
	}

	AddCodePage(codePageName, codePage, position, enabled, checked);
}

// ��������� �����������
void AddSeparator(LPCWSTR Label=NULL,int position = -1)
{
	if (CallbackCallSource == CodePagesFill)
	{
		if (position==-1)
		{
			FarListInfo info;
			SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
			position = info.ItemsNumber;
		}

		FarListInsert item = {position};
		item.Item.Text = Label;
		item.Item.Flags = LIF_SEPARATOR;
		SendDlgMessage(dialog, DM_LISTINSERT, control, (LONG_PTR)&item);
	}
	else
	{
		MenuItemEx item;
		item.Clear();
		item.strName = Label;
		item.Flags = MIF_SEPARATOR;

		if (position>=0)
			CodePages->AddItem(&item, position);
		else
			CodePages->AddItem(&item);

		// ������������ ������� ���������� ��������
		if (position!=-1 && CodePages->GetSelectPos()>=position)
			CodePages->SetSelectPos(CodePages->GetSelectPos()+1, 1);
	}
}

// �������� ���������� ��������� � ������
int GetItemsCount()
{
	if (CallbackCallSource == CodePageSelect)
	{
		return CodePages->GetItemCount();
	}
	else
	{
		FarListInfo info;
		SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
		return info.ItemsNumber;
	}
}

// �������� ������� ��� ������� ������� � ������ ���������� �� ������ ������� ��������
int GetCodePageInsertPosition(UINT codePage, int start, int length)
{
	if (length==0)
		return start;

	int position = start;

	do
	{
		UINT itemCodePage;

		if (CallbackCallSource == CodePageSelect)
			itemCodePage = (UINT)(UINT_PTR)CodePages->GetItemPtr(position)->UserData;
		else
			itemCodePage = (UINT)(UINT_PTR)SendDlgMessage(dialog, DM_LISTGETDATA, control, position);

		if (itemCodePage >= codePage)
			return position;
	}
	while (position++<start+length);

	return position-1;
}

// Callback-������� ��������� ������ ��������
BOOL __stdcall EnumCodePagesProc(const wchar_t *lpwszCodePage)
{
	UINT codePage = _wtoi(lpwszCodePage);

	// ��� ������� �������� ��� �� ���������� ���������� � ������� ��������� �������� �� �����������
	if (CallbackCallSource == CodePageCheck && codePage != currentCodePage)
		return TRUE;

	// BUBBUG: ���������� ����� ��������� � cpiex.MaxCharSize > 1, ���� �� �� ������������
	CPINFOEX cpiex;

	if (!GetCPInfoEx(codePage, 0, &cpiex))
	{
		// GetCPInfoEx ���������� ������ ��� ������� ������� ��� ����� (�������� 1125), �������
		// ���� �� ���� ��������. ��� ���, ������ ��� ���������� ������� �������� ��-�� ������,
		// ������� �������� ��� �� ���������� ����� GetCPInfo
		CPINFO cpi;

		if (!GetCPInfo(codePage, &cpi))
			return CallbackCallSource == CodePageCheck ? FALSE : TRUE;

		cpiex.MaxCharSize = cpi.MaxCharSize;
		cpiex.CodePageName[0] = L'\0';
	}

	// BUBUG: ���� �� ������������ ������������� ������� ��������
	if (cpiex.MaxCharSize != 1)
		return CallbackCallSource == CodePageCheck ? FALSE : TRUE;

	// ��� ������� �������� ���������������� ������� �������� �� ������ ��� �������� � ����� ��������
	if (CallbackCallSource == CodePageCheck)
	{
		CodePageSupported = true;
		return FALSE;
	}

	// ��������� ��� ������ ��������
	wchar_t *codePageName = FormatCodePageName(_wtoi(lpwszCodePage), cpiex.CodePageName, sizeof(cpiex.CodePageName)/sizeof(wchar_t));
	// �������� ������� ����������� ������� ��������
	int selectType = 0;
	GetRegKey(FavoriteCodePagesKey, lpwszCodePage, selectType, 0);

	// ��������� ������� �������� ���� � ����������, ���� � ��������� ������� ��������
	if (selectType & CPST_FAVORITE)
	{
		// ���� ���� ��������� ����������� ����� ���������� � ����������� ��������� ��������
		if (!favoriteCodePages)
			AddSeparator(MSG(MGetCodePageFavorites),GetItemsCount()-normalCodePages-(normalCodePages?1:0));

		// ��������� ������� �������� � ���������
		AddCodePage(
		    codePageName,
		    codePage,
		    GetCodePageInsertPosition(
		        codePage,
		        GetItemsCount()-normalCodePages-favoriteCodePages-(favoriteCodePages?1:0)-(normalCodePages?1:0),
		        favoriteCodePages
		    ),
		    true,
		    selectType & CPST_FIND ? true : false
		);
		// ����������� ������� ��������� ������ ��������
		favoriteCodePages++;
	}
	else if (CallbackCallSource == CodePagesFill || !Opt.CPMenuMode)
	{
		// ��������� ����������� ����� ������������ � ���������� ��������� ��������
		if (!favoriteCodePages && !normalCodePages)
			AddSeparator(MSG(MGetCodePageOther));

		// ��������� ������� �������� � ����������
		AddCodePage(
		    codePageName,
		    codePage,
		    GetCodePageInsertPosition(
		        codePage,
		        GetItemsCount()-normalCodePages,
		        normalCodePages
		    )
		);
		// ����������� ������� ��������� ������ ��������
		normalCodePages++;
	}

	return TRUE;
}

void AddCodePages(DWORD codePages)
{
	// ��������� ����������� ������� ��������
	AddStandardCodePage((codePages & ::SearchAll) ? MSG(MFindFileAllCodePages) : MSG(MEditOpenAutoDetect), CP_AUTODETECT, -1, (codePages & ::SearchAll) || (codePages & ::Auto));
	AddSeparator(MSG(MGetCodePageSystem));
	AddStandardCodePage(L"OEM", GetOEMCP(), -1, (codePages & ::OEM)?1:0);
	AddStandardCodePage(L"ANSI", GetACP(), -1, (codePages & ::ANSI)?1:0);
	AddSeparator(MSG(MGetCodePageUnicode));
	AddStandardCodePage(L"UTF-7", CP_UTF7, -1, (codePages & ::UTF7)?1:0);
	AddStandardCodePage(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8)?1:0);
	AddStandardCodePage(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE)?1:0);
	AddStandardCodePage(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE)?1:0);
	// �������� ������� �������� ������������� � �������
	EnumSystemCodePages((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
}

// ��������� ����������/�������� �/�� ������ ��������� ������ ��������
void ProcessSelected(bool select)
{
	if (Opt.CPMenuMode && select)
		return;

	MenuItemEx *curItem = CodePages->GetItemPtr();
	UINT itemPosition = CodePages->GetSelectPos();
	UINT itemCount = CodePages->GetItemCount();
	UINT codePage = (UINT)(UINT_PTR)curItem->UserData;

	if ((select && itemPosition >= itemCount-normalCodePages) || (!select && itemPosition>=itemCount-normalCodePages-favoriteCodePages-(normalCodePages?1:0) && itemPosition < itemCount-normalCodePages))
	{
		// ����������� ����� ������� �������� � ������
		string strCPName;
		strCPName.Format(L"%u", curItem->UserData);
		// �������� ������� ��������� ����� � �������
		int selectType = 0;
		GetRegKey(FavoriteCodePagesKey, strCPName, selectType, 0);

		// �������/��������� � �������� ���������� � ��������� ������� ��������
		if (select)
			SetRegKey(FavoriteCodePagesKey, strCPName, CPST_FAVORITE | (selectType & CPST_FIND ? CPST_FIND : 0));
		else if (selectType & CPST_FIND)
			SetRegKey(FavoriteCodePagesKey, strCPName, CPST_FIND);
		else
			DeleteRegValue(FavoriteCodePagesKey, strCPName);

		// ������ ����� ������� ����
		MenuItemEx newItem;
		newItem.Clear();
		newItem.strName = curItem->strName;
		newItem.UserData = (char *)(UINT_PTR)codePage;
		newItem.UserDataSize = sizeof(UINT);
		// ��������� ������� �������
		int position=CodePages->GetSelectPos();
		// ������� ������ ����� ����
		CodePages->DeleteItem(CodePages->GetSelectPos());

		// ��������� ����� ���� � ����� �����
		if (select)
		{
			// ��������� �����������, ���� ��������� ������� ������� ��� �� ����
			// � ����� ���������� ��������� ���������� ������� ��������
			if (!favoriteCodePages && normalCodePages>1)
				AddSeparator(MSG(MGetCodePageFavorites),CodePages->GetItemCount()-normalCodePages);

			// ���� �������, ���� �������� �������
			int newPosition = GetCodePageInsertPosition(
			                      codePage,
			                      CodePages->GetItemCount()-normalCodePages-favoriteCodePages,
			                      favoriteCodePages
			                  );
			// ��������� ������� �������� � ���������
			CodePages->AddItem(&newItem, newPosition);

			// ������� �����������, ���� ��� ������������ ������� �������
			if (normalCodePages==1)
				CodePages->DeleteItem(CodePages->GetItemCount()-1);

			// �������� �������� ���������� � ��������� ������� �������
			favoriteCodePages++;
			normalCodePages--;
			position++;
		}
		else
		{
			// ������� ����������, ���� ����� �������� �� ���������� �� �����
			// ��������� ������� ��������
			if (favoriteCodePages==1 && normalCodePages>0)
				CodePages->DeleteItem(CodePages->GetItemCount()-normalCodePages-2);

			// ��������� ������� � ���������� �������, ������ ���� ��� ������������
			if (!Opt.CPMenuMode)
			{
				// ��������� �����������, ���� �� ���� �� ����� ���������� ������� ��������
				if (!normalCodePages)
					AddSeparator(MSG(MGetCodePageOther));

				// ��������� ������� �������� � ����������
				CodePages->AddItem(
				    &newItem,
				    GetCodePageInsertPosition(
				        codePage,
				        CodePages->GetItemCount()-normalCodePages,
				        normalCodePages
				    )
				);
				normalCodePages++;
			}
			// ���� � ������ ������� ���������� ������ �� ������� ��������� ��������� �������, �� ������� � �����������
			else if (favoriteCodePages==1)
				CodePages->DeleteItem(CodePages->GetItemCount()-normalCodePages-1);

			favoriteCodePages--;

			if (position==CodePages->GetItemCount()-normalCodePages-1)
				position--;
		}

		// ������������� ������� � ����
		CodePages->SetSelectPos(position>=CodePages->GetItemCount() ? CodePages->GetItemCount()-1 : position, 1);

		// ���������� ����
		if (Opt.CPMenuMode)
			CodePages->SetPosition(-1, -1, 0, 0);

		CodePages->Show();
	}
}

// ��������� ���� ������ ������ ��������
void FillCodePagesVMenu(bool bShowUnicode, bool bShowUTF)
{
	// ��������� ��������� ������� ��������
	UINT codePage = currentCodePage;

	if (CodePages->GetSelectPos()!=-1 && CodePages->GetSelectPos()<CodePages->GetItemCount()-normalCodePages)
		currentCodePage = (UINT)(UINT_PTR)CodePages->GetItemPtr()->UserData;

	// ������� ����
	favoriteCodePages = normalCodePages = 0;
	CodePages->DeleteItems();
	// ������������� ��������� ����
	UnicodeString title = MSG(MGetCodePageTitle);

	if (Opt.CPMenuMode)
		title += L" *";

	CodePages->SetTitle(title);
	// ��������� ������� ��������
	AddCodePages(::OEM | ::ANSI | (bShowUTF ? /* BUBUG ::UTF7 | */ ::UTF8 : 0) | (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0));
	// ��������������� ����������� ������� ��������
	currentCodePage = codePage;
	// ������������� ����
	CodePages->SetPosition(-1, -1, 0, 0);
	// ���������� ����
	CodePages->Show();
}

// ����������� ��� ������� ��������
wchar_t *FormatCodePageName(UINT CodePage, wchar_t *CodePageName, size_t Length)
{
	if (!CodePageName || !Length)
		return CodePageName;

	// ��������� ��� ������ ��������
	if (!*CodePageName)
	{
		string strCodePage;
		strCodePage.Format(L"%u", CodePage);
		// ���� ��� �� ������, �� �������� �������� ��� �� Far2\CodePages\Names
		string strCodePageName;
		GetRegKey(NamesOfCodePagesKey, strCodePage, strCodePageName, L"");
		Length = Min(Length-1, strCodePageName.GetLength());
		wmemcpy(CodePageName, strCodePageName, Length);
		CodePageName[Length] = L'\0';
		return CodePageName;
	}
	else
	{
		// ��� ������ �� ����� "XXXX (Name)", �, ��������, ��� wine ������ "Name"
		wchar_t *Name = wcschr(CodePageName, L'(');

		if (Name && *(++Name))
		{
			Name[wcslen(Name)-1] = L'\0';
			return Name;
		}
		else
		{
			return CodePageName;
		}
	}
}

UINT SelectCodePage(UINT nCurrent, bool bShowUnicode, bool bShowUTF)
{
	CallbackCallSource = CodePageSelect;
	currentCodePage = nCurrent;
	// ������ ����
	CodePages = new VMenu(L"", NULL, 0, ScrY-4);
	CodePages->SetBottomTitle(MSG(!Opt.CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
	CodePages->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	CodePages->SetHelp(L"CodePagesMenu");
	// ��������� ������� ��������
	FillCodePagesVMenu(bShowUnicode, bShowUTF);
	// ���������� ����
	CodePages->Show();

	// ���� ��������� ��������� ����
	while (!CodePages->Done())
	{
		switch (CodePages->ReadInput())
		{
				// ��������� �������/������ ��������� ������ ��������
			case KEY_CTRLH:
				Opt.CPMenuMode = !Opt.CPMenuMode;
				CodePages->SetBottomTitle(MSG(!Opt.CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
				FillCodePagesVMenu(bShowUnicode, bShowUTF);
				break;
				// ��������� �������� ������� �������� �� ������ ���������
			case KEY_DEL:
			case KEY_NUMDEL:
				ProcessSelected(false);
				break;
				// ��������� ���������� ������� �������� � ������ ���������
			case KEY_INS:
			case KEY_NUMPAD0:
				ProcessSelected(true);
				break;
			default:
				CodePages->ProcessInput();
				break;
		}
	}

	// �������� ��������� ������� ��������
	UINT codePage = CodePages->Modal::GetExitCode() >= 0 ? (UINT)(UINT_PTR)CodePages->GetUserData(NULL, 0) : (UINT)-1;
	delete CodePages;
	CodePages = NULL;
	return codePage;
}

// ��������� ������ ��������� ��������
UINT FillCodePagesList(HANDLE dialogHandle, UINT controlId, UINT codePage, bool allowAuto, bool allowAll)
{
	CallbackCallSource = CodePagesFill;
	// ������������� ���������� ��� ������� �� ��������
	dialog = dialogHandle;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// ��������� ���������� �������� � ������
	AddCodePages((allowAuto ? ::Auto : 0) | (allowAll ? ::SearchAll : 0) | ::AllStandard);

	if (CallbackCallSource == CodePagesFill)
	{
		// ���� ���� �������� �������
		FarListInfo info;
		SendDlgMessage(dialogHandle, DM_LISTINFO, control, (LONG_PTR)&info);

		for (int i=0; i<info.ItemsNumber; i++)
		{
			if (static_cast<UINT>(SendDlgMessage(dialogHandle,DM_LISTGETDATA,controlId,i))==codePage)
			{
				FarListGetItem Item={i};
				SendDlgMessage(dialog, DM_LISTGETITEM,controlId,reinterpret_cast<LONG_PTR>(&Item));
				SendDlgMessage(dialog, DM_SETTEXTPTR,controlId,reinterpret_cast<LONG_PTR>(Item.Item.Text));
				FarListPos Pos={i,-1};
				SendDlgMessage(dialogHandle,DM_LISTSETCURPOS,controlId,reinterpret_cast<LONG_PTR>(&Pos));
				break;
			}
		}
	}

	// ���������� ����� ������� ������ ��������
	return favoriteCodePages;
}

bool IsCodePageSupported(UINT CodePage)
{
	// ��� ����������� ������� ������� ������ ��������� �� ����
	// BUGBUG: �� �� ����� ����������� ��� ����������� ������� ��������. ��� �� �����������
	if (CodePage == CP_AUTODETECT || IsStandardCodePage(CodePage))
		return true;

	// �������� �� ���� ������� ��������� ������� � ��������� ������������ �� ��� ��� �
	CallbackCallSource = CodePageCheck;
	currentCodePage = CodePage;
	CodePageSupported = false;
	EnumSystemCodePages((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
	return CodePageSupported;
}
