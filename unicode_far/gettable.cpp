/*
gettable.cpp

������ � ��������� ��������
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

#include "gettable.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "savefpos.hpp"
#include "keys.hpp"
#include "registry.hpp"
#include "language.hpp"
#include "dialog.hpp"
#include "interf.hpp"

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

// ������
static HANDLE dialog;
// ������������ �������
static UINT control;
// ����
static VMenu *tables = NULL;
// ������� ������� ��������
static UINT currentCodePage;
// ���������� ��������� � ������������ ������ ��������
static int favoriteCodePages, normalCodePages;
// ������� ������������� ���������� ������� �������� ��� ������
static bool selectedCodePages;

// ��������� ������� ��������
void AddTable(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true, bool checked = false)
{
	if (!tables)
	{
		// ��������� ������� ������������ ��������
		if (position==-1)
		{
			FarListInfo info;
			Dialog::SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
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
		if (checked)
			item.Item.Flags |= MIF_CHECKED;
		Dialog::SendDlgMessage(dialog, DM_LISTINSERT, control, (LONG_PTR)&item);
		// ������������� ������ ��� ��������
		FarListItemData data;
		data.Index = position;
		data.Data = (void*)(DWORD_PTR)codePage;
		data.DataSize = sizeof(UINT);
		Dialog::SendDlgMessage(dialog, DM_LISTSETDATA, control, (LONG_PTR)&data);
		// ���� ���� �������� �������
		FarListInfo info;
		Dialog::SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
		if (info.ItemsNumber==1 || (codePage==currentCodePage && (UINT)Dialog::SendDlgMessage(dialog, DM_LISTGETDATA, control, info.SelectPos)!=codePage))
		{
			Dialog::SendDlgMessage(dialog, DM_LISTSETCURPOS, control, (LONG_PTR)&position);
			Dialog::SendDlgMessage(dialog, DM_SETTEXTPTR, control, (LONG_PTR)item.Item.Text);
		}
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
			tables->AddItem(&item, position);
		else
			tables->AddItem(&item);
		// ���� ���� ������������� ������ �� ����������� �������
		if (currentCodePage==codePage && (tables->GetSelectPos()==-1 || (UINT)(UINT_PTR)tables->GetItemPtr()->UserData!=codePage))
			tables->SetSelectPos(position>=0?position:tables->GetItemCount()-1, 1);
		// ������������ ������� ���������� ��������
		else if (position!=-1 && tables->GetSelectPos()>=position)
			tables->SetSelectPos(tables->GetSelectPos()+1, 1);
	}
}

// ��������� ����������� ������� ��������
void AddStandardTable(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true)
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
	AddTable(codePageName, codePage, position, enabled, checked);
}

// ��������� �����������
void AddSeparator(int position = -1)
{
	if (!tables)
	{
		if (position==-1)
		{
			FarListInfo info;
			Dialog::SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
			position = info.ItemsNumber;
		}
		FarListInsert item = {position};
		item.Item.Flags = LIF_SEPARATOR;
		Dialog::SendDlgMessage(dialog, DM_LISTINSERT, control, (LONG_PTR)&item);
	}
	else
	{
		MenuItemEx item;
		item.Clear();
		item.Flags = MIF_SEPARATOR;
		if (position>=0)
			tables->AddItem(&item, position);
		else
			tables->AddItem(&item);
		// ������������ ������� ���������� ��������
		if (position!=-1 && tables->GetSelectPos()>=position)
			tables->SetSelectPos(tables->GetSelectPos()+1, 1);
	}
}

// �������� ���������� ��������� � ������
int GetItemsCount()
{
	if (tables)
		return tables->GetItemCount();
	else
	{
		FarListInfo info;
		Dialog::SendDlgMessage(dialog, DM_LISTINFO, control, (LONG_PTR)&info);
		return info.ItemsNumber;
	}
}

// �������� ������� ��� ������� ������� � ������ ���������� �� ������ ������� ��������
int GetTableInsertPosition(UINT codePage, int start, int length)
{
	if (length==0)
		return start;

	int position = start;
	do
	{
		UINT itemCodePage;
		if (tables)
			itemCodePage = (UINT)(UINT_PTR)tables->GetItemPtr(position)->UserData;
		else
			itemCodePage = (UINT)(UINT_PTR)Dialog::SendDlgMessage(dialog, DM_LISTGETDATA, control, position);
		if (itemCodePage >= codePage)
			return position;
	} while (position++<start+length);

	return position-1;
}

// Callback-������� ��������� ������ ��������
BOOL __stdcall EnumCodePagesProc(const wchar_t *lpwszCodePage)
{
	UINT codePage = _wtoi(lpwszCodePage);
	// BUBBUG: ���������� ����� ��������� � cpi.MaxCharSize > 1, ���� �� �� ������������
	CPINFOEXW cpi;
	if (GetCPInfoExW(codePage, 0, &cpi) && cpi.MaxCharSize == 1)
	{
		// ��������� ��� ������ ��������
		// ��� ������ �� ����� "XXXX (Name)" � �������� ��� wine ������ "Name"
		wchar_t *codePageName = wcschr(cpi.CodePageName, L'(');
		if (codePageName && *(++codePageName))
			codePageName[wcslen(codePageName)-1] = L'\0';
		else
			codePageName = cpi.CodePageName;
		// �������� ������� ����������� ������� ��������
		int selectType = 0;
		GetRegKey(FavoriteCodePagesKey, lpwszCodePage, selectType, 0);
		// ��������� ������� �������� ���� � ����������, ���� � ��������� ������� ��������
		if (selectType & CPST_FAVORITE)
		{
			// ��������� ����������� ����� ������������ � ���������� ��������� ��������
			if (!favoriteCodePages && !normalCodePages)
				AddSeparator();
			// ��������� ������� �������� � ���������
			AddTable(
					codePageName,
					codePage,
					GetTableInsertPosition(
							codePage,
							GetItemsCount()-normalCodePages-favoriteCodePages-((favoriteCodePages && normalCodePages)?1:0),
							favoriteCodePages
						),
					true,
					selectType & CPST_FIND ? true : false
				);
			// ���� ���� ��������� ����������� ����� ���������� � ����������� ��������� ��������
			if (!favoriteCodePages && normalCodePages)
				AddSeparator(GetItemsCount()-normalCodePages);
			// ����������� ������� ��������� ������ ��������
			favoriteCodePages++;
		}
		else if (!tables || !Opt.CPMenuMode)
		{
			// ��������� ����������� ����� ������������ � ���������� ��������� ��������
			if (!favoriteCodePages && !normalCodePages)
				AddSeparator();
			// ��������� ������� �������� � ����������
			AddTable(
					codePageName,
					codePage,
					GetTableInsertPosition(
							codePage,
							GetItemsCount()-normalCodePages,
							normalCodePages
						)
				);
			// ���� ���� ��������� ����������� ����� ���������� � ����������� ��������� ��������
			if (favoriteCodePages && !normalCodePages)
				AddSeparator(GetItemsCount()-normalCodePages);
			// ����������� ������� ��������� ������ ��������
			normalCodePages++;
		}
	}
	return TRUE;
}

void AddTables(DWORD codePages)
{
	// ��������� ����������� ������� ��������
	if ((codePages & ::SearchAll) || (codePages & ::Auto))
	{
		AddStandardTable((codePages & ::Auto) ? MSG(MEditOpenAutoDetect) : MSG(MFindFileAllCodePages), CP_AUTODETECT, -1, true);
		AddSeparator();
	}
	AddStandardTable(L"OEM", GetOEMCP(), -1, (codePages & ::OEM)?1:0);
	AddStandardTable(L"ANSI", GetACP(), -1, (codePages & ::ANSI)?1:0);
	AddSeparator();
	AddStandardTable(L"UTF-7", CP_UTF7, -1, (codePages & ::UTF7)?1:0);
	AddStandardTable(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8)?1:0);
	AddStandardTable(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE)?1:0);
	AddStandardTable(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE)?1:0);
	// �������� ������� �������� ������������� � �������
	EnumSystemCodePagesW((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
}

// ��������� ����������/�������� �/�� ������ ��������� ������ ��������
void ProcessSelected(bool select)
{
	if (Opt.CPMenuMode && select)
		return;
	MenuItemEx *curItem = tables->GetItemPtr();
	UINT itemPosition = tables->GetSelectPos();
	UINT itemCount = tables->GetItemCount();
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
		int position=tables->GetSelectPos();
		// ������� ������ ����� ����
		tables->DeleteItem(tables->GetSelectPos());
		// ��������� ����� ���� � ����� �����
		if (select)
		{
			// ��������� �����������, ���� ��������� ������� ������� ��� �� ����
			// � ����� ���������� ��������� ���������� ������� ��������
			if (!favoriteCodePages && normalCodePages>1)
				AddSeparator(tables->GetItemCount()-normalCodePages);
			// ���� �������, ���� �������� �������
			int newPosition = GetTableInsertPosition(
					codePage,
					tables->GetItemCount()-normalCodePages-favoriteCodePages,
					favoriteCodePages
				);
			// ��������� ������� �������� � ���������
			tables->AddItem(&newItem, newPosition);
			// ������� �����������, ���� ��� ������������ ������� �������
			if (normalCodePages==1)
				tables->DeleteItem(tables->GetItemCount()-1);
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
				tables->DeleteItem(tables->GetItemCount()-normalCodePages-1);
			// ��������� ������� � ���������� �������, ������ ���� ��� ������������
			if (!Opt.CPMenuMode)
			{
				// ��������� �����������, ���� �� ���� �� ����� ���������� ������� ��������
				if (!normalCodePages)
					AddSeparator();
				// ��������� ������� �������� � ����������
				tables->AddItem(
						&newItem,
						GetTableInsertPosition(
								codePage,
								tables->GetItemCount()-normalCodePages,
								normalCodePages
							)
					);
				normalCodePages++;
			}
			// ���� � ������ ������� ���������� ������ �� ������� ��������� ��������� �������, �� ������� � �����������
			else if (favoriteCodePages==1)
				tables->DeleteItem(tables->GetItemCount()-normalCodePages-1);
			favoriteCodePages--;
			if (position==tables->GetItemCount()-normalCodePages-1)
				position--;
		}
		// ������������� ������� � ����
		tables->AdjustSelectPos();
		tables->SetSelectPos(position>=tables->GetItemCount() ? tables->GetItemCount()-1 : position, 1);
		// ���������� ����
		if (Opt.CPMenuMode)
			tables->SetPosition(-1, -1, 0, 0);
		tables->Show();
	}
}

// ��������� ���� ������ ������ ��������
void FillTablesVMenu(bool bShowUnicode, bool bShowUTF)
{
	// ��������� ��������� ������� ��������
	UINT codePage = currentCodePage;
	if (tables->GetSelectPos()!=-1 && tables->GetSelectPos()<tables->GetItemCount()-normalCodePages)
		currentCodePage = (UINT)(UINT_PTR)tables->GetItemPtr()->UserData;
	// ������� ����
	favoriteCodePages = normalCodePages = 0;
	tables->DeleteItems();
	// ������������� ��������� ����
	UnicodeString title = MSG(MGetTableTitle);
	if (Opt.CPMenuMode)
		title += L"*";
	tables->SetTitle(title);
	// ��������� ������� ��������
	AddTables(::OEM | ::ANSI | (bShowUTF ? /* BUBUG ::UTF7 | */ ::UTF8 : 0) | (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0));
	// ��������������� ����������� ������� ��������
	currentCodePage = codePage;
	// ������������� ����
	tables->SetPosition(-1, -1, 0, 0);
	// ���������� ����
	tables->Show();
}

UINT GetTableEx(UINT nCurrent, bool bShowUnicode, bool bShowUTF)
{
	currentCodePage = nCurrent;
	// ������ ����
	tables = new VMenu(L"", NULL, 0, ScrY-4);
	tables->SetBottomTitle(MSG(MGetTableBottomTitle));
	tables->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	// ��������� ������� ��������
	FillTablesVMenu(bShowUnicode, bShowUTF);
	// ���������� ����
	tables->Show();
	// ���� ��������� ��������� ����
	while (!tables->Done())
	{
		switch (tables->ReadInput())
		{
			// ��������� �������/������ ��������� ������ ��������
			case KEY_CTRLH:
				Opt.CPMenuMode = !Opt.CPMenuMode;
				FillTablesVMenu(bShowUnicode, bShowUTF);
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
				tables->ProcessInput();
				break;
		}
	}

	// �������� ��������� ������� ��������
	UINT codePage = tables->Modal::GetExitCode() >= 0 ? (UINT)(UINT_PTR)tables->GetUserData(NULL, 0) : (UINT)-1;

	delete tables;
	tables = NULL;

	return codePage;
}

// ��������� ������ ��������� ��������
UINT AddCodepagesToList(HANDLE dialogHandle, UINT controlId, UINT codePage, bool allowAuto, bool allowAll)
{
	// ������������� ���������� ��� ������� �� ��������
	dialog = dialogHandle;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// ��������� ���������� �������� � ������
	AddTables((allowAuto ? ::Auto : 0) | (allowAll ? ::SearchAll : 0) | ::AllStandard);
	// ���������� ����� ������� ������ ��������
	return favoriteCodePages;
}
