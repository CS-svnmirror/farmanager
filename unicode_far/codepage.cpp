/*
codepage.cpp

������ � �������� ����������
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

#include "codepage.hpp"
#include "vmenu2.hpp"
#include "keys.hpp"
#include "language.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "FarDlgBuilder.hpp"
#include "DlgGuid.hpp"

// ���� ��� �������� ����� ������� �������
const wchar_t *NamesOfCodePagesKey = L"CodePages.Names";

const wchar_t *FavoriteCodePagesKey = L"CodePages.Favorites";

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
	AllStandard = OEM | ANSI | UTF7 | UTF8 | UTF16BE | UTF16LE,
	DefaultCP = 256,
	AllowM2 = 512
};

// ������ ��������� ������� �������������� ����� ������� ��������
enum
{
	EDITCP_BORDER,
	EDITCP_EDIT,
	EDITCP_SEPARATOR,
	EDITCP_OK,
	EDITCP_CANCEL,
	EDITCP_RESET,
};

// Callback-������� ��������� ������ ��������
BOOL CALLBACK EnumCodePagesProc(wchar_t *lpwszCodePage)
{
	codepages& cp = *Global->CodePages;

	uintptr_t codePage = _wtoi(lpwszCodePage);

	// ��� ������� �������� ��� �� ���������� ���������� � ������� ��������� �������� �� �����������
	if (cp.CallbackCallSource == CodePageCheck && codePage != cp.currentCodePage)
		return TRUE;

	// �������� ���������� � ������� ��������. ���� ���������� �� �����-���� ������� �������� �� �������, ��
	// ��� ������� ���������� ����������, � ��� ��������� �� �������� ���������������� ������� �������� �������
	CPINFOEX cpiex;
	if (!cp.GetCodePageInfo(codePage, cpiex))
		return cp.CallbackCallSource != CodePageCheck;

	// ��� ������� �������� ���������������� ������� �������� �� ������ ��� �������� � ����� ��������
	if (cp.CallbackCallSource == CodePageCheck)
	{
		cp.CodePageSupported = true;
		return FALSE;
	}

	// ��������� ��� ������ ��������
	bool IsCodePageNameCustom = false;
	wchar_t *codePageName = cp.FormatCodePageName(_wtoi(lpwszCodePage), cpiex.CodePageName, sizeof(cpiex.CodePageName)/sizeof(wchar_t), IsCodePageNameCustom);
	// �������� ������� ����������� ������� ��������
	long long selectType = 0;
	Global->Db->GeneralCfg()->GetValue(FavoriteCodePagesKey, lpwszCodePage, &selectType, 0);

	// ��������� ������� �������� ���� � ����������, ���� � ��������� ������� ��������
	if (selectType & CPST_FAVORITE)
	{
		// ���� ���� ��������� ����������� ����� ���������� � ����������� ��������� ��������
		if (!cp.favoriteCodePages)
			cp.AddSeparator(MSG(MGetCodePageFavorites),cp.GetItemsCount()-cp.normalCodePages-(cp.normalCodePages?1:0));

		// ��������� ������� �������� � ���������
		cp.AddCodePage(
		    codePageName,
		    codePage,
		    cp.GetCodePageInsertPosition(
		        codePage,
		        cp.GetItemsCount()-cp.normalCodePages-cp.favoriteCodePages-(cp.normalCodePages?1:0),
		        cp.favoriteCodePages
		    ),
		    true,
		    (selectType & CPST_FIND) != 0,
			IsCodePageNameCustom
		);
		// ����������� ������� ��������� ������ ��������
		cp.favoriteCodePages++;
	}
	else if (cp.CallbackCallSource == CodePagesFill || cp.CallbackCallSource == CodePagesFill2 || !Global->Opt->CPMenuMode)
	{
		// ��������� ����������� ����� ������������ � ���������� ��������� ��������
		if (!cp.favoriteCodePages && !cp.normalCodePages)
			cp.AddSeparator(MSG(MGetCodePageOther));

		// ��������� ������� �������� � ����������
		cp.AddCodePage(
		    codePageName,
		    codePage,
		    cp.GetCodePageInsertPosition(
		        codePage,
		        cp.GetItemsCount()-cp.normalCodePages,
		        cp.normalCodePages
		    ),
			true,
			false,
			IsCodePageNameCustom
		);
		// ����������� ������� ��������� ������ ��������
		cp.normalCodePages++;
	}

	return TRUE;
}

codepages::codepages():
	dialog(nullptr),
	control(0),
	DialogBuilderList(nullptr),
	CodePagesMenu(nullptr),
	currentCodePage(0),
	favoriteCodePages(0),
	normalCodePages(0),
	selectedCodePages(false),
	CallbackCallSource(CodePageSelect),
	CodePageSupported(false)
{
}

// �������� ������� �������� ��� �������� � ����
inline uintptr_t codepages::GetMenuItemCodePage(int Position)
{
	void* Data = CodePagesMenu->GetUserData(nullptr, 0, Position);
	return Data? *static_cast<uintptr_t*>(Data) : 0;
}

inline uintptr_t codepages::GetListItemCodePage(int Position)
{
	intptr_t Data = dialog->SendMessage(DM_LISTGETDATA, control, ToPtr(Position));
	return Data? *reinterpret_cast<uintptr_t*>(Data) : 0;
}

// ��������� �������� ��� ��� ������� � �������� ����������� ������� ������� (������������ ������ ��� ������������ �� �������������)
inline bool codepages::IsPositionStandard(UINT position)
{
	return position<=(UINT)CodePagesMenu->GetItemCount()-favoriteCodePages-(favoriteCodePages?1:0)-normalCodePages-(normalCodePages?1:0);
}

// ��������� �������� ��� ��� ������� � �������� ������� ������� ������� (������������ ������ ��� ������������ �� �������������)
inline bool codepages::IsPositionFavorite(UINT position)
{
	return position>=(UINT)CodePagesMenu->GetItemCount()-normalCodePages;
}

// ��������� �������� ��� ��� ������� � �������� ������������ ������� ������� (������������ ������ ��� ������������ �� �������������)
inline bool codepages::IsPositionNormal(UINT position)
{
	UINT ItemCount = CodePagesMenu->GetItemCount();
	return position>=ItemCount-normalCodePages-favoriteCodePages-(normalCodePages?1:0) && position<ItemCount-normalCodePages;
}

// ��������� ������ ��� ����������� ������������� ������� ��������
void codepages::FormatCodePageString(uintptr_t CodePage, const wchar_t *CodePageName, FormatString &CodePageNameString, bool IsCodePageNameCustom)
{
	if (static_cast<intptr_t>(CodePage) >= 0)  // CodePage != CP_DEFAULT, CP_REDETECT
	{
		CodePageNameString<<fmt::MinWidth(5)<<CodePage<<BoxSymbols[BS_V1]<<(!IsCodePageNameCustom||CallbackCallSource==CodePagesFill||CallbackCallSource==CodePagesFill2?L' ':L'*');
	}
	CodePageNameString<<CodePageName;
}

// ��������� ������� ��������
void codepages::AddCodePage(const wchar_t *codePageName, uintptr_t codePage, int position, bool enabled, bool checked, bool IsCodePageNameCustom)
{
	if (CallbackCallSource == CodePagesFill)
	{
		// ��������� ������� ������������ ��������
		if (position==-1)
		{
			FarListInfo info={sizeof(FarListInfo)};
			dialog->SendMessage(DM_LISTINFO, control, &info);
			position = static_cast<int>(info.ItemsNumber);
		}

		// ��������� �������
		FarListInsert item = {sizeof(FarListInsert),position};

		FormatString name;
		FormatCodePageString(codePage, codePageName, name, IsCodePageNameCustom);
		item.Item.Text = name.CPtr();

		if (selectedCodePages && checked)
		{
			item.Item.Flags |= LIF_CHECKED;
		}

		if (!enabled)
		{
			item.Item.Flags |= LIF_GRAYED;
		}

		dialog->SendMessage(DM_LISTINSERT, control, &item);
		// ������������� ������ ��� ��������
		FarListItemData data={sizeof(FarListItemData)};
		data.Index = position;
		data.Data = &codePage;
		data.DataSize = sizeof(codePage);
		dialog->SendMessage(DM_LISTSETDATA, control, &data);
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		// ��������� �������
		DialogBuilderListItem2 item;

		FormatString name;
		FormatCodePageString(codePage, codePageName, name, IsCodePageNameCustom);
		item.Text = name;
		item.Flags = LIF_NONE;

		if (selectedCodePages && checked)
		{
			item.Flags |= LIF_CHECKED;
		}

		if (!enabled)
		{
			item.Flags |= LIF_GRAYED;
		}

		item.ItemValue = static_cast<int>(codePage);

		// ��������� ������� ������������ ��������
		if (position==-1 || position>=static_cast<int>(DialogBuilderList->size()))
		{
			DialogBuilderList->emplace_back(item);
		}
		else
		{
			DialogBuilderList->emplace(DialogBuilderList->begin()+position, item);
		}
	}
	else
	{
		// ������ ����� ������� ����
		MenuItemEx item;
		item.Clear();

		if (!enabled)
			item.Flags |= MIF_GRAYED;

		FormatString name;
		FormatCodePageString(codePage, codePageName, name, IsCodePageNameCustom);
		item.strName = name;

		item.UserData = &codePage;
		item.UserDataSize = sizeof(codePage);

		// ��������� ����� ������� � ����
		if (position>=0)
			CodePagesMenu->AddItem(&item, position);
		else
			CodePagesMenu->AddItem(&item);

		// ���� ���� ������������� ������ �� ����������� �������
		if (currentCodePage==codePage)
		{
			if ((CodePagesMenu->GetSelectPos()==-1 || GetMenuItemCodePage()!=codePage))
			{
				CodePagesMenu->SetSelectPos(position>=0?position:CodePagesMenu->GetItemCount()-1, 1);
			}
		}
	}
}

// ��������� ����������� ������� ��������
void codepages::AddStandardCodePage(const wchar_t *codePageName, uintptr_t codePage, int position, bool enabled)
{
	bool checked = false;

	if (selectedCodePages && codePage!=CP_DEFAULT)
	{
		long long selectType = 0;
		Global->Db->GeneralCfg()->GetValue(FavoriteCodePagesKey, FormatString() << codePage, &selectType, 0);

		if (selectType & CPST_FIND)
			checked = true;
	}

	AddCodePage(codePageName, codePage, position, enabled, checked, false);
}

// ��������� �����������
void codepages::AddSeparator(LPCWSTR Label, int position)
{
	if (CallbackCallSource == CodePagesFill)
	{
		if (position==-1)
		{
			FarListInfo info={sizeof(FarListInfo)};
			dialog->SendMessage(DM_LISTINFO, control, &info);
			position = static_cast<int>(info.ItemsNumber);
		}

		FarListInsert item = {sizeof(FarListInsert),position};
		item.Item.Text = Label;
		item.Item.Flags = LIF_SEPARATOR;
		dialog->SendMessage(DM_LISTINSERT, control, &item);
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		// ��������� �������
		DialogBuilderListItem2 item;
		item.Text = Label;
		item.Flags = LIF_SEPARATOR;
		item.ItemValue = 0;

		// ��������� ������� ������������ ��������
		if (position==-1 || position>=static_cast<int>(DialogBuilderList->size()))
		{
			DialogBuilderList->emplace_back(item);
		}
		else
		{
			DialogBuilderList->emplace(DialogBuilderList->begin()+position, item);
		}
	}
	else
	{
		MenuItemEx item;
		item.Clear();
		item.strName = Label;
		item.Flags = MIF_SEPARATOR;

		if (position>=0)
			CodePagesMenu->AddItem(&item, position);
		else
			CodePagesMenu->AddItem(&item);
	}
}

// �������� ���������� ��������� � ������
int codepages::GetItemsCount()
{
	if (CallbackCallSource == CodePageSelect)
	{
		return CodePagesMenu->GetItemCount();
	}
	else if (CallbackCallSource == CodePagesFill2)
	{
		return static_cast<int>(DialogBuilderList->size());
	}
	else
	{
		FarListInfo info={sizeof(FarListInfo)};
		dialog->SendMessage(DM_LISTINFO, control, &info);
		return static_cast<int>(info.ItemsNumber);
	}
}

// �������� ������� ��� ������� ������� � ������ ���������� �� ������ ������� ��������
int codepages::GetCodePageInsertPosition(uintptr_t codePage, int start, int length)
{
	for (int position=start; position < start+length; position++)
	{
		uintptr_t itemCodePage;

		if (CallbackCallSource == CodePageSelect)
			itemCodePage = GetMenuItemCodePage(position);
		else if (CallbackCallSource == CodePagesFill2)
			itemCodePage = (*DialogBuilderList)[position].ItemValue;
		else
			itemCodePage = GetListItemCodePage(position);

		if (itemCodePage >= codePage)
			return position;
	}

	return start+length;
}

static bool allow_m2 = false;

// �������� ���������� � ������� ��������
bool codepages::GetCodePageInfo(uintptr_t CodePage, CPINFOEX &CodePageInfoEx)
{
	if (!GetCPInfoEx(static_cast<UINT>(CodePage), 0, &CodePageInfoEx))
	{
		// GetCPInfoEx ���������� ������ ��� ������� ������� ��� ����� (�������� 1125), �������
		// ���� �� ���� ��������. ��� ���, ������ ��� ���������� ������� �������� ��-�� ������,
		// ������� �������� ��� �� ���������� ����� GetCPInfo
		CPINFO CodePageInfo;

		if (!GetCPInfo(static_cast<UINT>(CodePage), &CodePageInfo))
			return false;

		CodePageInfoEx.MaxCharSize = CodePageInfo.MaxCharSize;
		CodePageInfoEx.CodePageName[0] = L'\0';
	}

	// BUBUG: ���� �� ������������ ������������� ������� ��������
	if (CodePageInfoEx.MaxCharSize != 1)
		return (allow_m2 ? (CodePageInfoEx.MaxCharSize == 2) : false);

	return true;
}

// ��������� ��� ����������� ������� ��������
void codepages::AddCodePages(DWORD codePages)
{
	// ��������� ����������� ������� ��������

	uintptr_t cp_auto = CP_DEFAULT;
	if ( 0 != (codePages & ::DefaultCP) )
	{
		AddStandardCodePage(MSG(MDefaultCP), CP_DEFAULT, -1, true);
		cp_auto = CP_REDETECT;
	}
	AddStandardCodePage((codePages & ::SearchAll) ? MSG(MFindFileAllCodePages) : MSG(MEditOpenAutoDetect), cp_auto, -1, (codePages & (::SearchAll | ::Auto)) != 0);
	AddSeparator(MSG(MGetCodePageSystem));
	AddStandardCodePage(L"OEM", GetOEMCP(), -1, (codePages & ::OEM) != 0);
	AddStandardCodePage(L"ANSI", GetACP(), -1, (codePages & ::ANSI) != 0);
	AddSeparator(MSG(MGetCodePageUnicode));
	if (codePages & ::UTF7) AddStandardCodePage(L"UTF-7", CP_UTF7, -1, true); //?? �� ��������������, �� � ����� ��?
	AddStandardCodePage(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8) != 0);
	AddStandardCodePage(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE) != 0);
	AddStandardCodePage(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE) != 0);
	// �������� ������� �������� ������������� � �������
	allow_m2 = (codePages & ::AllowM2) != 0;
	EnumSystemCodePages(EnumCodePagesProc, CP_INSTALLED);
}

// ��������� ����������/�������� �/�� ������ ��������� ������ ��������
void codepages::ProcessSelected(bool select)
{
	if (Global->Opt->CPMenuMode && select)
		return;

	UINT itemPosition = CodePagesMenu->GetSelectPos();
	uintptr_t codePage = GetMenuItemCodePage();

	if ((select && IsPositionFavorite(itemPosition)) || (!select && IsPositionNormal(itemPosition)))
	{
		// ����������� ����� ������� �������� � ������
		FormatString strCPName;
		strCPName<<codePage;
		// �������� ������� ��������� ����� � �������
		long long selectType = 0;
		Global->Db->GeneralCfg()->GetValue(FavoriteCodePagesKey, strCPName, &selectType, 0);

		// �������/��������� � �������� ���������� � ��������� ������� ��������
		if (select)
			Global->Db->GeneralCfg()->SetValue(FavoriteCodePagesKey, strCPName, CPST_FAVORITE | (selectType & CPST_FIND ? CPST_FIND : 0));
		else if (selectType & CPST_FIND)
			Global->Db->GeneralCfg()->SetValue(FavoriteCodePagesKey, strCPName, CPST_FIND);
		else
			Global->Db->GeneralCfg()->DeleteValue(FavoriteCodePagesKey, strCPName);

		// ������ ����� ������� ����
		MenuItemEx newItem;
		newItem.Clear();
		newItem.strName = CodePagesMenu->GetItemPtr()->strName;
		newItem.UserData = &codePage;
		newItem.UserDataSize = sizeof(codePage);
		// ��������� ������� �������
		int position=CodePagesMenu->GetSelectPos();
		// ������� ������ ����� ����
		CodePagesMenu->DeleteItem(CodePagesMenu->GetSelectPos());

		// ��������� ����� ���� � ����� �����
		if (select)
		{
			// ��������� �����������, ���� ��������� ������� ������� ��� �� ����
			// � ����� ���������� ��������� ���������� ������� ��������
			if (!favoriteCodePages && normalCodePages>1)
				AddSeparator(MSG(MGetCodePageFavorites),CodePagesMenu->GetItemCount()-normalCodePages);

			// ���� �������, ���� �������� �������
			int newPosition = GetCodePageInsertPosition(
			                      codePage,
			                      CodePagesMenu->GetItemCount()-normalCodePages-favoriteCodePages,
			                      favoriteCodePages
			                  );
			// ��������� ������� �������� � ���������
			CodePagesMenu->AddItem(&newItem, newPosition);

			// ������� �����������, ���� ��� ������������ ������� �������
			if (normalCodePages==1)
				CodePagesMenu->DeleteItem(CodePagesMenu->GetItemCount()-1);

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
				CodePagesMenu->DeleteItem(CodePagesMenu->GetItemCount()-normalCodePages-2);

			// ��������� ������� � ���������� �������, ������ ���� ��� ������������
			if (!Global->Opt->CPMenuMode)
			{
				// ��������� �����������, ���� �� ���� �� ����� ���������� ������� ��������
				if (!normalCodePages)
					AddSeparator(MSG(MGetCodePageOther));

				// ��������� ������� �������� � ����������
				CodePagesMenu->AddItem(
				    &newItem,
				    GetCodePageInsertPosition(
				        codePage,
				        CodePagesMenu->GetItemCount()-normalCodePages,
				        normalCodePages
				    )
				);
				normalCodePages++;
			}
			// ���� � ������ ������� ���������� ������ �� ������� ��������� ��������� �������, �� ������� � �����������
			else if (favoriteCodePages==1)
				CodePagesMenu->DeleteItem(CodePagesMenu->GetItemCount()-normalCodePages-1);

			favoriteCodePages--;

			if (position==CodePagesMenu->GetItemCount()-normalCodePages-1)
				position--;
		}

		// ������������� ������� � ����
		CodePagesMenu->SetSelectPos(position>=CodePagesMenu->GetItemCount() ? CodePagesMenu->GetItemCount()-1 : position, 1);

		// ���������� ����
		if (Global->Opt->CPMenuMode)
			CodePagesMenu->SetPosition(-1, -1, 0, 0);
	}
}

// ��������� ���� ������ ������ ��������
void codepages::FillCodePagesVMenu(bool bShowUnicode, bool bShowUTF, bool bShowUTF7, bool bShowAutoDetect, bool bShowM2)
{
	uintptr_t codePage = currentCodePage;

	if (CodePagesMenu->GetSelectPos()!=-1 && CodePagesMenu->GetSelectPos()<CodePagesMenu->GetItemCount()-normalCodePages)
		currentCodePage = GetMenuItemCodePage();

	// ������� ����
	favoriteCodePages = normalCodePages = 0;
	CodePagesMenu->DeleteItems();

	UnicodeString title = MSG(MGetCodePageTitle);
	if (Global->Opt->CPMenuMode)
		title += L" *";
	CodePagesMenu->SetTitle(title);

	// ��������� ������� ��������
	AddCodePages(::OEM | ::ANSI
		| (bShowUTF ? ::UTF8 : 0)
		| (bShowUTF7 ? ::UTF7 : 0)
		| (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0)
		| (bShowAutoDetect ? ::Auto : 0)
		| (bShowM2 ? ::AllowM2 : 0)
	);
	// ��������������� ����������� ������� ��������
	currentCodePage = codePage;
	// ������������� ����
	CodePagesMenu->SetPosition(-1, -1, 0, 0);
	// ���������� ����
}

// ����������� ��� ������� ��������
wchar_t *codepages::FormatCodePageName(uintptr_t CodePage, wchar_t *CodePageName, size_t Length)
{
	bool IsCodePageNameCustom;
	return FormatCodePageName(CodePage, CodePageName, Length, IsCodePageNameCustom);
}

// ����������� ��� ������� ��������
wchar_t *codepages::FormatCodePageName(uintptr_t CodePage, wchar_t *CodePageName, size_t Length, bool &IsCodePageNameCustom)
{
	if (!CodePageName || !Length)
		return CodePageName;

	// �������� �������� �������� ������������� ��� ������� ��������
	FormatString strCodePage;
	strCodePage<<CodePage;
	string strCodePageName;
	if (Global->Db->GeneralCfg()->GetValue(NamesOfCodePagesKey, strCodePage, strCodePageName, L""))
	{
		Length = std::min(Length-1, strCodePageName.GetLength());
		IsCodePageNameCustom = true;
	}
	else
		IsCodePageNameCustom = false;
	if (*CodePageName)
	{
		// ��� ������ �� ����� "XXXX (Name)", �, ��������, ��� wine ������ "Name"
		wchar_t *Name = wcschr(CodePageName, L'(');
		if (Name && *(++Name))
		{
			size_t NameLength = wcslen(Name)-1;
			if (Name[NameLength] == L')')
			{
				Name[NameLength] = L'\0';
			}
		}
		if (IsCodePageNameCustom)
		{
			if (strCodePageName==Name)
			{
				Global->Db->GeneralCfg()->DeleteValue(NamesOfCodePagesKey, strCodePage);
				IsCodePageNameCustom = false;
				return Name;
			}
		}
		else
			return Name;
	}
	if (IsCodePageNameCustom)
	{
		wmemcpy(CodePageName, strCodePageName.CPtr(), Length);
		CodePageName[Length] = L'\0';
	}
	return CodePageName;
}

// ������� ��� ������� �������������� ����� ������� ��������
intptr_t codepages::EditDialogProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	if (Msg==DN_CLOSE)
	{
		if (Param1==EDITCP_OK || Param1==EDITCP_RESET)
		{
			string strCodePageName;
			uintptr_t CodePage = GetMenuItemCodePage();
			FormatString strCodePage;
			strCodePage<<CodePage;
			if (Param1==EDITCP_OK)
			{
				FarDialogItemData item = {sizeof(FarDialogItemData)};
				item.PtrLength = Dlg->SendMessage(DM_GETTEXT, EDITCP_EDIT, 0);
				item.PtrData = strCodePageName.GetBuffer(item.PtrLength+1);
				Dlg->SendMessage(DM_GETTEXT, EDITCP_EDIT, &item);
				strCodePageName.ReleaseBuffer();
			}
			// ���� ��� ������� �������� ������, �� �������, ��� ��� �� ������
			if (!strCodePageName.GetLength())
				Global->Db->GeneralCfg()->DeleteValue(NamesOfCodePagesKey, strCodePage);
			else
				Global->Db->GeneralCfg()->SetValue(NamesOfCodePagesKey, strCodePage, strCodePageName);
			// �������� ���������� � ������� ��������
			CPINFOEX cpiex;
			if (GetCodePageInfo(CodePage, cpiex))
			{
				// ��������� ��� ������ ��������
				bool IsCodePageNameCustom = false;
				wchar_t *CodePageName = FormatCodePageName(CodePage, cpiex.CodePageName, sizeof(cpiex.CodePageName)/sizeof(wchar_t), IsCodePageNameCustom);
				// ��������� ������ �������������
				strCodePage.Clear();
				FormatCodePageString(CodePage, CodePageName, strCodePage, IsCodePageNameCustom);
				// ��������� ��� ������� ��������
				int Position = CodePagesMenu->GetSelectPos();
				CodePagesMenu->DeleteItem(Position);
				MenuItemEx NewItem;
				NewItem.Clear();
				NewItem.strName = strCodePage;
				NewItem.UserData = &CodePage;
				NewItem.UserDataSize = sizeof(CodePage);
				CodePagesMenu->AddItem(&NewItem, Position);
				CodePagesMenu->SetSelectPos(Position, 1);
			}
		}
	}
	return Dlg->DefProc(Msg, Param1, Param2);
}

// ����� ��������� ����� ������� ��������
void codepages::EditCodePageName()
{
	UINT Position = CodePagesMenu->GetSelectPos();
	if (IsPositionStandard(Position))
		return;
	string CodePageName = CodePagesMenu->GetItemPtr(Position)->strName;
	size_t BoxPosition;
	if (!CodePageName.Pos(BoxPosition, BoxSymbols[BS_V1]))
		return;
	CodePageName.LShift(BoxPosition+2);
	FarDialogItem EditDialogData[]=
		{
			{DI_DOUBLEBOX, 3, 1, 50, 5, 0, nullptr, nullptr, 0, MSG(MGetCodePageEditCodePageName)},
			{DI_EDIT,      5, 2, 48, 2, 0, L"CodePageName", nullptr, DIF_FOCUS|DIF_HISTORY, CodePageName.CPtr()},
			{DI_TEXT,     -1, 3,  0, 3, 0, nullptr, nullptr, DIF_SEPARATOR, L""},
			{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_DEFAULTBUTTON|DIF_CENTERGROUP, MSG(MOk)},
			{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MCancel)},
			{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MGetCodePageResetCodePageName)}
		};
	MakeDialogItemsEx(EditDialogData, EditDialog);
	Dialog Dlg(this, &codepages::EditDialogProc, nullptr, EditDialog, ARRAYSIZE(EditDialog));
	Dlg.SetPosition(-1, -1, 54, 7);
	Dlg.SetHelp(L"EditCodePageNameDlg");
	Dlg.Process();
}

bool codepages::SelectCodePage(uintptr_t& CodePage, bool bShowUnicode, bool bShowUTF, bool bShowUTF7, bool bShowAutoDetect)
{
	bool Result = false;
	CallbackCallSource = CodePageSelect;
	currentCodePage = CodePage;
	// ������ ����
	CodePagesMenu = new VMenu2(L"", nullptr, 0, ScrY-4);
	CodePagesMenu->SetBottomTitle(MSG(!Global->Opt->CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
	CodePagesMenu->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	CodePagesMenu->SetHelp(L"CodePagesMenu");
	CodePagesMenu->SetId(CodePagesMenuId);
	// ��������� ������� ��������
	FillCodePagesVMenu(bShowUnicode, bShowUTF, bShowUTF7, bShowAutoDetect);
	// ���������� ����

	// ���� ��������� ��������� ����
	intptr_t r=CodePagesMenu->Run([&](int ReadKey)->int
	{
		int KeyProcessed = 1;
		switch (ReadKey)
		{
			// ��������� �������/������ ��������� ������ ��������
			case KEY_CTRLH:
			case KEY_RCTRLH:
				Global->Opt->CPMenuMode = !Global->Opt->CPMenuMode;
				CodePagesMenu->SetBottomTitle(MSG(!Global->Opt->CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
				FillCodePagesVMenu(bShowUnicode, bShowUTF, bShowUTF7, bShowAutoDetect);
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
			// ����������� ��� ������� ��������
			case KEY_F4:
				EditCodePageName();
				break;
			default:
				KeyProcessed = 0;
		}
		return KeyProcessed;
	});

	// �������� ��������� ������� ��������
	if (r >= 0)
	{
		CodePage = GetMenuItemCodePage();
		Result = true;
	}
	delete CodePagesMenu;
	CodePagesMenu = nullptr;
	return Result;
}

// ��������� ������ ��������� ��������
void codepages::FillCodePagesList(std::vector<DialogBuilderListItem2> &List, bool allowAuto, bool allowAll, bool allowDefault, bool allowM2)
{
	CallbackCallSource = CodePagesFill2;
	// ������������� ���������� ��� ������� �� ��������
	DialogBuilderList = &List;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// ��������� ���������� �������� � ������
	AddCodePages((allowM2 ? ::AllowM2 : 0) | (allowDefault ? ::DefaultCP : 0) | (allowAuto ? ::Auto : 0) | (allowAll ? ::SearchAll : 0) | ::AllStandard);
	DialogBuilderList = nullptr;
}


// ��������� ������ ��������� ��������
UINT codepages::FillCodePagesList(Dialog* Dlg, UINT controlId, uintptr_t codePage, bool allowAuto, bool allowAll, bool allowDefault, bool allowM2)
{
	CallbackCallSource = CodePagesFill;
	// ������������� ���������� ��� ������� �� ��������
	dialog = Dlg;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// ��������� ���������� �������� � ������
	AddCodePages((allowM2 ? ::AllowM2 : 0) | (allowDefault ? ::DefaultCP : 0) | (allowAuto ? ::Auto : 0) | (allowAll ? ::SearchAll : 0) | ::AllStandard);

	if (CallbackCallSource == CodePagesFill)
	{
		// ���� ���� �������� �������
		FarListInfo info={sizeof(FarListInfo)};
		Dlg->SendMessage(DM_LISTINFO, control, &info);

		for (int i=0; i<static_cast<int>(info.ItemsNumber); i++)
		{
			if (GetListItemCodePage(i)==codePage)
			{
				FarListGetItem Item={sizeof(FarListGetItem),i};
				dialog->SendMessage(DM_LISTGETITEM, control, &Item);
				dialog->SendMessage(DM_SETTEXTPTR, control, const_cast<wchar_t*>(Item.Item.Text));
				FarListPos Pos={sizeof(FarListPos),i,-1};
				dialog->SendMessage(DM_LISTSETCURPOS, control, &Pos);
				break;
			}
		}
	}

	// ���������� ����� ������� ������ ��������
	return favoriteCodePages;
}

bool codepages::IsCodePageSupported(uintptr_t CodePage)
{
	// ��� ����������� ������� ������� ������ ��������� �� ����
	// BUGBUG: �� �� ����� ����������� ��� ����������� ������� ��������. ��� �� �����������
	if (CodePage == CP_DEFAULT || IsStandardCodePage(CodePage))
		return true;

	// �������� �� ���� ������� ��������� ������� � ��������� ������������ �� ��� ��� �
	CallbackCallSource = CodePageCheck;
	currentCodePage = CodePage;
	CodePageSupported = false;
	EnumSystemCodePages(EnumCodePagesProc, CP_INSTALLED);
	return CodePageSupported;
}
