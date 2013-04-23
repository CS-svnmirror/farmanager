#pragma once

/*
vmenu.hpp

������� ������������ ����
  � ��� ��:
    * ������ � DI_COMBOBOX
    * ������ � DI_LISTBOX
    * ...
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

#include "modal.hpp"
#include "manager.hpp"
#include "frame.hpp"
#include "bitflags.hpp"
#include "synchro.hpp"
#include "macro.hpp"


// �������� �������� - ������� � ������� ������
enum
{
	VMenuColorBody                = 0,     // ��������
	VMenuColorBox                 = 1,     // �����
	VMenuColorTitle               = 2,     // ��������� - ������� � ������
	VMenuColorText                = 3,     // ����� ������
	VMenuColorHilite              = 4,     // HotKey
	VMenuColorSeparator           = 5,     // separator
	VMenuColorSelected            = 6,     // ���������
	VMenuColorHSelect             = 7,     // ��������� - HotKey
	VMenuColorScrollBar           = 8,     // ScrollBar
	VMenuColorDisabled            = 9,     // Disabled
	VMenuColorArrows              =10,     // '<' & '>' �������
	VMenuColorArrowsSelect        =11,     // '<' & '>' ���������
	VMenuColorArrowsDisabled      =12,     // '<' & '>' Disabled
	VMenuColorGrayed              =13,     // "�����"
	VMenuColorSelGrayed           =14,     // ��������� "�����"

	VMENU_COLOR_COUNT,                     // ������ ��������� - ����������� �������
};

enum VMENU_FLAGS
{
	VMENU_ALWAYSSCROLLBAR      =0x00000100, // ������ ���������� ���������
	VMENU_LISTBOX              =0x00000200, // ��� ������ � �������
	VMENU_SHOWNOBOX            =0x00000400, // �������� ��� �����
	VMENU_AUTOHIGHLIGHT        =0x00000800, // ������������� �������� ������ ���������
	VMENU_REVERSEHIGHLIGHT     =0x00001000, // ... ������ � �����
	VMENU_UPDATEREQUIRED       =0x00002000, // ���� ���������� �������� (������������)
	VMENU_DISABLEDRAWBACKGROUND=0x00004000, // �������� �� ��������
	VMENU_WRAPMODE             =0x00008000, // ����������� ������ (��� �����������)
	VMENU_SHOWAMPERSAND        =0x00010000, // ������ '&' ���������� AS IS
	VMENU_WARNDIALOG           =0x00020000, //
//	VMENU_NOTCENTER            =0x00040000, // �� ���������
//	VMENU_LEFTMOST             =0x00080000, // "������� �����" - ���������� �� 5 ������� ������ �� ������ (X1 => (ScrX+1)/2+5)
	VMENU_NOTCHANGE            =0x00100000, //
	VMENU_LISTHASFOCUS         =0x00200000, // ���� �������� ������� � ������� � ����� �����
	VMENU_COMBOBOX             =0x00400000, // ���� �������� ����������� � �������������� ���������� ��-�������.
	VMENU_MOUSEDOWN            =0x00800000, //
	VMENU_CHANGECONSOLETITLE   =0x01000000, //
	VMENU_MOUSEREACTION        =0x02000000, // ����������� �� �������� ����? (���������� ������� ��� ����������� ������� ����?)
	VMENU_DISABLED             =0x04000000, //
	VMENU_NOMERGEBORDER        =0x08000000, //
	VMENU_REFILTERREQUIRED     =0x10000000, // ����� ���������� ���������� �������� ������
	VMENU_LISTSINGLEBOX        =0x20000000, // ������, ������ � ��������� ������
};

class Dialog;
class SaveScreen;


struct MenuItemEx
{
	string strName;
	UINT64  Flags;                  // ����� ������
	void *UserData;                // ���������������� ������:
	size_t UserDataSize;           // ������ ���������������� ������
	int   ShowPos;
	DWORD  AccelKey;
	short AmpPos;                  // ������� ��������������� ���������
	short Len[2];                  // ������� 2-� ������
	short Idx2;                    // ������ 2-� �����


	UINT64 SetCheck(int Value)
	{
		if (Value)
		{
			Flags|=LIF_CHECKED;
			Flags &= ~0xFFFF;

			if (Value!=1) Flags|=Value&0xFFFF;
		}
		else
		{
			Flags&=~(0xFFFF|LIF_CHECKED);
		}

		return Flags;
	}

	UINT64 SetSelect(int Value) { if (Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
	UINT64 SetDisable(int Value) { if (Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}

	void Clear()
	{
		strName.Clear();
		Flags = 0;
		UserData = nullptr;
		UserDataSize = 0;
		ShowPos = 0;
		AccelKey = 0;
		AmpPos = 0;
		Len[0] = 0;
		Len[1] = 0;
		Idx2 = 0;
	}

	//UserData �� ����������.
	MenuItemEx& operator=(const MenuItemEx &srcMenu)
	{
		if (this != &srcMenu)
		{
			strName = srcMenu.strName;
			Flags = srcMenu.Flags;
			UserData = nullptr;
			UserDataSize = 0;
			ShowPos = srcMenu.ShowPos;
			AccelKey = srcMenu.AccelKey;
			AmpPos = srcMenu.AmpPos;
			Len[0] = srcMenu.Len[0];
			Len[1] = srcMenu.Len[1];
			Idx2 = srcMenu.Idx2;
		}

		return *this;
	}
};

struct MenuDataEx
{
	const wchar_t *Name;

	DWORD Flags;
	DWORD AccelKey;

	DWORD SetCheck(int Value)
	{
		if (Value)
		{
			Flags &= ~0xFFFF;
			Flags|=((Value&0xFFFF)|LIF_CHECKED);
		}
		else
			Flags&=~(0xFFFF|LIF_CHECKED);

		return Flags;
	}

	DWORD SetSelect(int Value) { if (Value) Flags|=LIF_SELECTED; else Flags&=~LIF_SELECTED; return Flags;}
	DWORD SetDisable(int Value) { if (Value) Flags|=LIF_DISABLE; else Flags&=~LIF_DISABLE; return Flags;}
	DWORD SetGrayed(int Value) { if (Value) Flags|=LIF_GRAYED; else Flags&=~LIF_GRAYED; return Flags;}
};

struct SortItemParam
{
	bool Reverse;
	int Offset;
};

class ConsoleTitle;
class Frame;

class VMenu: public Modal
{
	private:
		string strTitle;
		string strBottomTitle;

		int SelectPos;
		int TopPos;
		int MaxHeight;
		bool WasAutoHeight;
		int MaxLength;
		int BoxType;
		Frame *CurrentFrame;
		bool PrevCursorVisible;
		DWORD PrevCursorSize;
		MACROMODEAREA PrevMacroMode;

		// ����������, ���������� �� ����������� scrollbar � DI_LISTBOX & DI_COMBOBOX
		BitFlags VMFlags;
		BitFlags VMOldFlags;

		Dialog *ParentDialog;         // ��� LisBox - �������� � ���� �������
		size_t DialogItemID;
		FARWINDOWPROC VMenuProc;      // ������� ��������� ����

		ConsoleTitle *OldTitle;     // ���������� ���������

		CriticalSection CS;

		bool bFilterEnabled;
		bool bFilterLocked;
		string strFilter;

		std::vector<MenuItemEx*> Item;

		intptr_t ItemHiddenCount;
		intptr_t ItemSubMenusCount;

		FarColor Colors[VMENU_COLOR_COUNT];

		int MaxLineWidth;
		bool bRightBtnPressed;
		GUID MenuId;

	private:
		virtual void DisplayObject() override;
		void ShowMenu(bool IsParent=false);
		void DrawTitles();
		int GetItemPosition(int Position);
		static size_t _SetUserData(MenuItemEx *PItem,const void *Data,size_t Size);
		static void* _GetUserData(MenuItemEx *PItem,void *Data,size_t Size);
		bool CheckKeyHiOrAcc(DWORD Key,int Type,int Translate);
		int CheckHighlights(wchar_t Chr,int StartPos=0);
		wchar_t GetHighlights(const struct MenuItemEx *_item);
		bool ShiftItemShowPos(int Pos,int Direct);
		bool ItemCanHaveFocus(UINT64 Flags);
		bool ItemCanBeEntered(UINT64 Flags);
		bool ItemIsVisible(UINT64 Flags);
		void UpdateMaxLengthFromTitles();
		void UpdateMaxLength(int Length);
		void UpdateInternalCounters(UINT64 OldFlags, UINT64 NewFlags);
		bool IsFilterEditKey(int Key);
		bool ShouldSendKeyToFilter(int Key);
		//������������ ������� ������� � ������ SELECTED
		void UpdateSelectPos();

	public:

		VMenu(const wchar_t *Title,
		      MenuDataEx *Data,
		      int ItemCount,
		      int MaxHeight=0,
		      DWORD Flags=0,
		      Dialog *ParentDialog=nullptr);


		virtual ~VMenu();

		void FastShow() {ShowMenu();}
		virtual void Show() override;
		virtual void Hide() override;
		void ResetCursor();

		void SetTitle(const string& Title);
		virtual string &GetTitle(string &strDest,int SubLen=-1,int TruncSize=0) override;
		const wchar_t *GetPtrTitle() { return strTitle.CPtr(); }

		void SetBottomTitle(const wchar_t *BottomTitle);
		string &GetBottomTitle(string &strDest);
		void SetDialogStyle(bool Style) {ChangeFlags(VMENU_WARNDIALOG,Style); SetColors(nullptr);}
		void SetUpdateRequired(bool SetUpdate) {ChangeFlags(VMENU_UPDATEREQUIRED,SetUpdate);}
		void SetBoxType(int BoxType);

		void SetFlags(DWORD Flags) { VMFlags.Set(Flags); }
		void ClearFlags(DWORD Flags) { VMFlags.Clear(Flags); }
		bool CheckFlags(DWORD Flags) const { return VMFlags.Check(Flags); }
		DWORD GetFlags() const { return VMFlags.Flags(); }
		DWORD ChangeFlags(DWORD Flags,bool Status) {return VMFlags.Change(Flags,Status);}

		void AssignHighlights(int Reverse);

		void SetColors(struct FarDialogItemColors *ColorsIn=nullptr);
		void GetColors(struct FarDialogItemColors *ColorsOut);
		void SetOneColor(int Index, PaletteColors Color);

		int ProcessFilterKey(int Key);
		virtual int ProcessKey(int Key) override;
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent) override;
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
		virtual int ReadInput(INPUT_RECORD *GetReadRec=nullptr) override;

		void DeleteItems();
		int  DeleteItem(int ID,int Count=1);

		int  AddItem(const MenuItemEx *NewItem,int PosAdd=0x7FFFFFFF);
		int  AddItem(const FarList *NewItem);
		int  AddItem(const wchar_t *NewStrItem);

		int  InsertItem(const FarListInsert *NewItem);
		int  UpdateItem(const FarListUpdate *NewItem);
		int  FindItem(const FarListFind *FindItem);
		int  FindItem(int StartIndex,const string& Pattern,UINT64 Flags=0);
		void RestoreFilteredItems();
		void FilterStringUpdated();
		void FilterUpdateHeight(bool bShrink=false);
		void SetFilterEnabled(bool bEnabled) { bFilterEnabled=bEnabled; }
		void SetFilterLocked(bool bLocked) { bFilterEnabled=bLocked; }
 		bool AddToFilter(const wchar_t *str);
 		void SetFilterString(const wchar_t *str);

		intptr_t GetItemCount() { return Item.size(); }
		int  GetShowItemCount() { return static_cast<int>(Item.size())-ItemHiddenCount; }
		int  GetVisualPos(int Pos);
		int  VisualPosToReal(int VPos);

		void *GetUserData(void *Data,size_t Size,int Position=-1);
		size_t GetUserDataSize(int Position=-1);
		size_t  SetUserData(LPCVOID Data,size_t Size=0,int Position=-1);

		int  GetSelectPos() { return SelectPos; }
		int  GetSelectPos(struct FarListPos *ListPos);
		int  SetSelectPos(struct FarListPos *ListPos, int Direct=0);
		int  SetSelectPos(int Pos, int Direct, bool stop_on_edge=false);
		int  GetCheck(int Position=-1);
		void SetCheck(int Check, int Position=-1);

		bool UpdateRequired();
		void UpdateItemFlags(int Pos, UINT64 NewFlags);

		virtual void ResizeConsole() override;

		struct MenuItemEx *GetItemPtr(int Position=-1);

		void SortItems(bool Reverse = false, int Offset = 0);

		template<class predicate>
		void SortItems(predicate Pred, bool Reverse = false, int Offset = 0)
		{
			CriticalSectionLock Lock(CS);

			SortItemParam Param;
			Param.Reverse = Reverse;
			Param.Offset = Offset;

			std::sort(Item.begin(), Item.end(), [&](const MenuItemEx* a, const MenuItemEx* b)->bool
			{
				return Pred(a, b, Param);
			});

			// ������������� SelectPos
			UpdateSelectPos();

			SetFlags(VMENU_UPDATEREQUIRED);
		}

		bool Pack();

		BOOL GetVMenuInfo(struct FarListInfo* Info);

		virtual const wchar_t *GetTypeName() override {return L"[VMenu]";}
		virtual int GetTypeAndName(string &strType, string &strName) override;

		virtual int GetType() override { return CheckFlags(VMENU_COMBOBOX)?MODALTYPE_COMBOBOX:MODALTYPE_VMENU; }

		void SetMaxHeight(int NewMaxHeight);

		size_t GetVDialogItemID() const {return DialogItemID;}
		void SetVDialogItemID(size_t NewDialogItemID) {DialogItemID=NewDialogItemID;}

		static MenuItemEx *FarList2MenuItem(const FarListItem *Item,MenuItemEx *ListItem);
		static FarListItem *MenuItem2FarList(const MenuItemEx *ListItem,FarListItem *Item);

		void SetId(const GUID& Id);
		const GUID& Id(void);
};
