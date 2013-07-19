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

// ����������� �������� ���� ������� �������
enum StandardCodePagesMenuItems
{
	SearchAll = 1,	  // Find-in-Files dialog
	AutoCP = 2,		  // show <Autodetect> item
	OEM = 4,			  // show OEM codepage
	ANSI = 8,		  // show ANSI codepage
	UTF8 = 16,		  // show UTF-8 codepage
	UTF16LE = 32,	  // show UTF-16 LE codepage
	UTF16BE = 64,	  // show UTF-16 BE codepage
	VOnly = 128,	  // show only viewer-supported codepages
	AllStandard = OEM | ANSI | UTF8 | UTF16BE | UTF16LE,
	DefaultCP = 256, // show <Default> item
};

// ������ ��������� ������� �������������� ����� ������� ��������
enum EditCodePagesDialogControls
{
	EDITCP_BORDER,
	EDITCP_EDIT,
	EDITCP_SEPARATOR,
	EDITCP_OK,
	EDITCP_CANCEL,
	EDITCP_RESET,
};

BOOL CALLBACK enum_cp(wchar_t *cpNum)
{
	UINT cp = static_cast<UINT>(_wtoi(cpNum));
	if (cp == CP_UTF8)
		return TRUE; // skip standard unicode

	CPINFOEX cpix;
	if (!GetCPInfoEx(cp, 0, &cpix))
	{
		CPINFO cpi;
		if (!GetCPInfo(cp, &cpi))
			return TRUE;

		cpix.MaxCharSize = cpi.MaxCharSize;
		wcscpy(cpix.CodePageName, cpNum);
	}
	if (cpix.MaxCharSize > 0)
	{
		string cp_data(cpix.CodePageName);
		size_t pos = cp_data.find(L"(");
		// Windows: "XXXX (Name)", Wine: "Name"
		if (pos != string::npos)
		{
			cp_data = cp_data.substr(pos+1);
			pos = cp_data.find(L")");
			if (pos != string::npos)
				cp_data = cp_data.substr(0, pos);
		}
		wchar_t mcs[2] = {(wchar_t)cpix.MaxCharSize};
		Global->CodePages->installed_cp[cp] = mcs + cp_data;
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
	CallbackCallSource(CodePageSelect)
{}

void codepages::init()
{
	if (installed_cp.empty())
		EnumSystemCodePages(enum_cp, CP_INSTALLED);
}

int codepages::GetCodePageInfo(UINT cp, wchar_t *name, size_t cb)
{
	auto found = installed_cp.find(cp); // Standard unicode CP
	if (installed_cp.end() == found)		//	(1200, 1201, 65001)
		return 0;								// = NOT in the list =

	if (name && cb)
		wcsncpy(name, found->second.data()+1, cb);

	return static_cast<int>(found->second[0]); // returns MaxCharSize
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
		item.Item.Text = name.data();

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

// ��������� ��� ����������� ������� ��������
void codepages::AddCodePages(DWORD codePages)
{
	// default & re-detect
	//
	uintptr_t cp_auto = CP_DEFAULT;
	if ( 0 != (codePages & ::DefaultCP) )
	{
		AddStandardCodePage(MSG(MDefaultCP), CP_DEFAULT, -1, true);
		cp_auto = CP_REDETECT;
	}
	AddStandardCodePage((codePages & ::SearchAll) ? MSG(MFindFileAllCodePages) : MSG(MEditOpenAutoDetect), cp_auto, -1, (codePages & (::SearchAll | ::AutoCP)) != 0);

	// system codepages
	//
	AddSeparator(MSG(MGetCodePageSystem));
	AddStandardCodePage(L"OEM", GetOEMCP(), -1, (codePages & ::OEM) != 0);
	if (GetACP() != GetOEMCP())
		AddStandardCodePage(L"ANSI", GetACP(), -1, (codePages & ::ANSI) != 0);

	// unicode codepages
	//
	AddSeparator(MSG(MGetCodePageUnicode));
	AddStandardCodePage(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8) != 0);
	AddStandardCodePage(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE) != 0);
	AddStandardCodePage(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE) != 0);

	// other codepages
	//
	for (auto i = installed_cp.begin(); i != installed_cp.end(); ++i)
	{
		UINT cp = i->first;
		if (IsStandardCodePage(cp))
			continue;

		CPINFOEX cpix;
		int mb = Global->CodePages->GetCodePageInfo(cp, cpix.CodePageName, ARRAYSIZE(cpix.CodePageName));
		if (mb <= 0 || (mb > 2 && (codePages & ::VOnly)))
			continue;

		bool IsCodePageNameCustom = false;
		wchar_t *cpName = FormatCodePageName(cp, cpix.CodePageName, ARRAYSIZE(cpix.CodePageName), IsCodePageNameCustom);

		FormatString cpNum;
		cpNum << cp;
		long long selectType = 0;
		Global->Db->GeneralCfg()->GetValue(FavoriteCodePagesKey, cpNum, &selectType, 0);

		// ��������� ������� �������� ���� � ����������, ���� � ��������� ������� ��������
		if (selectType & CPST_FAVORITE)
		{
			// ���� ���� ��������� ����������� ����� ���������� � ����������� ��������� ��������
			if (!favoriteCodePages)
				AddSeparator(MSG(MGetCodePageFavorites),GetItemsCount()-normalCodePages-(normalCodePages?1:0));

			// ��������� ������� �������� � ���������
			AddCodePage(
				cpName, cp,
				GetCodePageInsertPosition(
					cp, GetItemsCount()-normalCodePages-favoriteCodePages-(normalCodePages?1:0), favoriteCodePages
				),
				true,	(selectType & CPST_FIND) != 0, IsCodePageNameCustom
			);
			// ����������� ������� ��������� ������ ��������
			favoriteCodePages++;
		}
		else if (CallbackCallSource == CodePagesFill || CallbackCallSource == CodePagesFill2 || !Global->Opt->CPMenuMode)
		{
			// ��������� ����������� ����� ������������ � ���������� ��������� ��������
			if (!favoriteCodePages && !normalCodePages)
				AddSeparator(MSG(MGetCodePageOther));

			// ��������� ������� �������� � ����������
			AddCodePage(
				cpName, cp,
				GetCodePageInsertPosition(cp, GetItemsCount()-normalCodePages, normalCodePages	),
				true,	false, IsCodePageNameCustom
			);
			// ����������� ������� ��������� ������ ��������
			normalCodePages++;
		}
	}
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
void codepages::FillCodePagesVMenu(bool bShowUnicode, bool bViewOnly, bool bShowAutoDetect)
{
	uintptr_t codePage = currentCodePage;

	if (CodePagesMenu->GetSelectPos()!=-1 && CodePagesMenu->GetSelectPos()<CodePagesMenu->GetItemCount()-normalCodePages)
		currentCodePage = GetMenuItemCodePage();

	// ������� ����
	favoriteCodePages = normalCodePages = 0;
	CodePagesMenu->DeleteItems();

	string title = MSG(MGetCodePageTitle);
	if (Global->Opt->CPMenuMode)
		title += L" *";
	CodePagesMenu->SetTitle(title);

	// ��������� ������� ��������
	AddCodePages(::OEM | ::ANSI | ::UTF8
		| (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0)
		| (bViewOnly ? ::VOnly : 0)
		| (bShowAutoDetect ? ::AutoCP : 0)
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
	FormatString strCodePage;
	strCodePage<<CodePage;
	string strCodePageName;

	if (CodePageName && Length // �������� �������� �������� ������������� ��� ������� ��������
	 && Global->Db->GeneralCfg()->GetValue(NamesOfCodePagesKey, strCodePage, strCodePageName, L""))
	{
		Length = std::min(Length-1, strCodePageName.size());
		IsCodePageNameCustom = true;
		if (*CodePageName && strCodePageName==CodePageName)
		{
			Global->Db->GeneralCfg()->DeleteValue(NamesOfCodePagesKey, strCodePage);
			IsCodePageNameCustom = false;
		}
		else
		{
			wmemcpy(CodePageName, strCodePageName.data(), Length);
			CodePageName[Length] = L'\0';
		}
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
				strCodePageName = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, EDITCP_EDIT, nullptr));
			}
			// ���� ��� ������� �������� ������, �� �������, ��� ��� �� ������
			if (!strCodePageName.size())
				Global->Db->GeneralCfg()->DeleteValue(NamesOfCodePagesKey, strCodePage);
			else
				Global->Db->GeneralCfg()->SetValue(NamesOfCodePagesKey, strCodePage, strCodePageName);
			// �������� ���������� � ������� ��������
			CPINFOEX cpix;
			if (GetCodePageInfo(static_cast<UINT>(CodePage), cpix.CodePageName, ARRAYSIZE(cpix.CodePageName)))
			{
				// ��������� ��� ������ ��������
				bool IsCodePageNameCustom = false;
				wchar_t *CodePageName = FormatCodePageName(CodePage, cpix.CodePageName, ARRAYSIZE(cpix.CodePageName), IsCodePageNameCustom);
				// ��������� ������ �������������
				strCodePage.clear();
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
	size_t BoxPosition = CodePageName.find(BoxSymbols[BS_V1]);
	if (BoxPosition == string::npos)
		return;
	CodePageName.erase(0, BoxPosition+2);
	FarDialogItem EditDialogData[]=
	{
		{DI_DOUBLEBOX, 3, 1, 50, 5, 0, nullptr, nullptr, 0, MSG(MGetCodePageEditCodePageName)},
		{DI_EDIT,      5, 2, 48, 2, 0, L"CodePageName", nullptr, DIF_FOCUS|DIF_HISTORY, CodePageName.data()},
		{DI_TEXT,     -1, 3,  0, 3, 0, nullptr, nullptr, DIF_SEPARATOR, L""},
		{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_DEFAULTBUTTON|DIF_CENTERGROUP, MSG(MOk)},
		{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MCancel)},
		{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MGetCodePageResetCodePageName)}
	};
	auto EditDialog = MakeDialogItemsEx(EditDialogData);
	Dialog Dlg(EditDialog, this, &codepages::EditDialogProc, nullptr);
	Dlg.SetPosition(-1, -1, 54, 7);
	Dlg.SetHelp(L"EditCodePageNameDlg");
	Dlg.Process();
}

bool codepages::SelectCodePage(uintptr_t& CodePage, bool bShowUnicode, bool bViewOnly, bool bShowAutoDetect)
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
	FillCodePagesVMenu(bShowUnicode, bViewOnly, bShowAutoDetect);
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
				FillCodePagesVMenu(bShowUnicode, bViewOnly, bShowAutoDetect);
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
void codepages::FillCodePagesList(std::vector<DialogBuilderListItem2> &List, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly)
{
	CallbackCallSource = CodePagesFill2;
	// ������������� ���������� ��� ������� �� ��������
	DialogBuilderList = &List;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// ��������� ���������� �������� � ������
	AddCodePages
	( (allowDefault ? ::DefaultCP : 0)
	| (allowAuto ? ::AutoCP : 0)
	| (allowAll ? ::SearchAll : 0)
	| (bViewOnly ? ::VOnly : 0)
	| ::AllStandard
	);
	DialogBuilderList = nullptr;
}


// ��������� ������ ��������� ��������
UINT codepages::FillCodePagesList(Dialog* Dlg, UINT controlId, uintptr_t codePage, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly)
{
	CallbackCallSource = CodePagesFill;
	// ������������� ���������� ��� ������� �� ��������
	dialog = Dlg;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// ��������� ���������� �������� � ������
	AddCodePages
		( (allowDefault ? ::DefaultCP : 0)
		| (allowAuto ? ::AutoCP : 0)
		| (allowAll ? ::SearchAll : 0)
		| (bViewOnly ? ::VOnly : 0)
		| ::AllStandard
	);

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
	if (CodePage == CP_DEFAULT || IsStandardCodePage(CodePage))
		return true;

	return GetCodePageInfo(static_cast<UINT>(CodePage)) > 0;
}

//#############################################################################

int MultibyteCodepageDecoder::MaxLen(UINT cp)
{
	if (cp==CP_ACP || cp==CP_OEMCP || cp==CP_MACCP || cp==CP_THREAD_ACP || cp==CP_SYMBOL)
		return 0;

	if (cp==CP_UTF8 || cp==CP_UNICODE || cp==CP_REVERSEBOM)
		return 0;

	return Global->CodePages->GetCodePageInfo(cp);
}
	
//#############################################################################

bool MultibyteCodepageDecoder::SetCP(UINT cp)
{
	if (cp && cp == current_cp)
		return true;

	int ml = MaxLen(cp);
	if (ml != 2)
		return false;

	if (len_mask.empty())
		len_mask.resize(256);
	else
		std::fill(ALL_RANGE(len_mask), 0);
	if (m1.empty())
		m1.resize(256);
	else
		std::fill(ALL_RANGE(m1), 0);
	if (m2.empty())
		m2.resize(256*256);
	else
		std::fill(ALL_RANGE(m2), 0);

	BOOL DefUsed, *pDefUsed = (cp==CP_UTF8 || cp==CP_UTF7) ? nullptr : &DefUsed;
	DWORD flags = WC_NO_BEST_FIT_CHARS;
	if (cp==CP_UTF8 || cp==CP_UTF7 || cp==54936 || cp==CP_SYMBOL
	 || (cp>=50220 && cp<=50222) || cp==50225 || cp==50227 || cp==50229 || (cp>=57002 && cp<=57011)
	) flags = 0;

	union {
		BYTE bf[2];
		BYTE    b1;
		UINT16  b2;
	} u;

	int nnn = 0, mb = 0;
	for (unsigned w=0x0000; w <= 0xffff; ++w) // only UCS2 range
	{
		DefUsed = FALSE;
		wchar_t wc = static_cast<wchar_t>(w); 
		int len = WideCharToMultiByte(cp, flags, &wc,1, (LPSTR)u.bf,(int)sizeof(u.bf), nullptr, pDefUsed);
		if (len <= 0 || DefUsed)
			continue;

		len_mask[u.b1] |= (BYTE)(1 << (len - 1));
		++nnn;
		if (mb < len) mb = len;

		switch (len)
		{
			case 1: m1[u.b1] = wc; break;
			case 2: m2[u.b2] = wc; break;
		}
	}

	assert(nnn >= 256);
	if (nnn < 256)
		return false;

	current_cp = cp;
	current_mb = mb;
	return true;
}

int MultibyteCodepageDecoder::GetChar(const BYTE *bf, size_t cb, wchar_t& wc) const
{
	if (!bf || cb < 1)
		return -11; // invalid argument

	BYTE b1 = bf[0];
	BYTE lmask = len_mask[b1];
	if (!lmask)
		return -1;
	if (lmask & 0x01)
	{
		wc = m1[b1];
		return +1;
	}

	if (cb < 2)
		return -12;

	UINT16 b2 = b1 | (bf[1] << 8);
	if (!m2[b2])
		return -2;
	else {
		wc = m2[b2];
		return +2;
	}
}
