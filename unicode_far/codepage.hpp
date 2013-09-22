#pragma once

/*
codepage.hpp

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

// ��� ��������� ������� ��������
enum CPSelectType
{
	CPST_FAVORITE = 1, // "�������" ������� ��������
	CPST_FIND     = 2  // ������� �������� ����������� � ������ �� ���� �������� ��������
};

extern const wchar_t *FavoriteCodePagesKey;

const int StandardCPCount = 2 /* OEM, ANSI */ + 2 /* UTF-16 LE, UTF-16 BE */ + 1 /* UTF-8 */;

inline bool IsStandardCodePage(uintptr_t cp) {
	return cp==CP_UNICODE || cp==CP_UTF8 || cp==CP_REVERSEBOM || cp==GetOEMCP() || cp==GetACP();
}

inline bool IsUnicodeCodePage(uintptr_t cp) { return cp==CP_UNICODE || cp==CP_REVERSEBOM; }

inline bool IsUnicodeOrUtfCodePage(uintptr_t cp) {
	return cp==CP_UNICODE || cp==CP_UTF8 || cp==CP_REVERSEBOM || cp==CP_UTF7;
}

// �������� ������ �������� ������� �� ������� ���������
enum CodePagesCallbackCallSource
{
	CodePageSelect,
	CodePagesFill,
	CodePagesFill2,
	CodePageCheck
};

class Dialog;
struct DialogBuilderListItem2;
class VMenu2;

class codepages
{
public:
	codepages();
	~codepages();

	int GetCodePageInfo(UINT cp, wchar_t *name=nullptr, size_t cb=0); // returns MaxCharSize

	bool IsCodePageSupported(uintptr_t CodePage);
	bool SelectCodePage(uintptr_t& CodePage, bool bShowUnicode, bool ViewOnly=false, bool bShowAutoDetect=false);
	UINT FillCodePagesList(Dialog* Dlg, UINT controlId, uintptr_t codePage, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly=false);
	void FillCodePagesList(std::vector<DialogBuilderListItem2> &List, bool allowAuto, bool allowAll, bool allowDefault, bool bViewOnly=false);
	wchar_t *FormatCodePageName(uintptr_t CodePage, wchar_t *CodePageName, size_t Length);

private:
	wchar_t *FormatCodePageName(uintptr_t CodePage, wchar_t *CodePageName, size_t Length, bool &IsCodePageNameCustom);
	inline uintptr_t GetMenuItemCodePage(int Position=-1);
	inline uintptr_t GetListItemCodePage(int Position=-1);
	inline bool IsPositionStandard(UINT position);
	inline bool IsPositionFavorite(UINT position);
	inline bool IsPositionNormal(UINT position);
	void FormatCodePageString(uintptr_t CodePage, const wchar_t *CodePageName, string& CodePageNameString, bool IsCodePageNameCustom);
	void AddCodePage(const wchar_t *codePageName, uintptr_t codePage, int position, bool enabled, bool checked, bool IsCodePageNameCustom);
	void AddStandardCodePage(const wchar_t *codePageName, uintptr_t codePage, int position=-1, bool enabled=true);
	void AddSeparator(LPCWSTR Label=nullptr,int position=-1);
	int GetItemsCount();
	int GetCodePageInsertPosition(uintptr_t codePage, int start, int length);
	void AddCodePages(DWORD codePages);
	void ProcessSelected(bool select);
	void FillCodePagesVMenu(bool bShowUnicode, bool bViewOnly=false, bool bShowAutoDetect=false);
	intptr_t EditDialogProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void EditCodePageName();

	friend class system_codepages_enumerator;

	Dialog* dialog;
	UINT control;
	std::vector<DialogBuilderListItem2> *DialogBuilderList;
	std::unique_ptr<VMenu2> CodePagesMenu;
	uintptr_t currentCodePage;
	int favoriteCodePages, normalCodePages;
	bool selectedCodePages;
	CodePagesCallbackCallSource CallbackCallSource;
	std::map<UINT, string> installed_cp;
};

//#############################################################################

class MultibyteCodepageDecoder
{
public:
	UINT current_cp;
	int  current_mb;

	static int MaxLen(UINT cp);

	bool SetCP(UINT cp);

	int GetChar(const BYTE *buff, size_t cb, wchar_t& wchar) const;

	MultibyteCodepageDecoder() : current_cp(0), current_mb(0) {}

private:
	std::vector<BYTE> len_mask; //[256]
	std::vector<wchar_t> m1;    //[256]
	std::vector<wchar_t> m2;  //[65536]
};

//#############################################################################

