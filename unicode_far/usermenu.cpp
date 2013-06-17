/*
usermenu.cpp

User menu � ����
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

#include "keys.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "fileedit.hpp"
#include "preservelongname.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "constitle.hpp"
#include "message.hpp"
#include "usermenu.hpp"
#include "filetype.hpp"
#include "fnparce.hpp"
#include "execute.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "filestr.hpp"
#include "mix.hpp"
#include "savescr.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "cache.hpp"
#include "language.hpp"

#if defined(PROJECT_DI_MEMOEDIT)
/*
  ���� � ���������.
  1. ������ � ������� ��������� ��� � ������, �.�. CommandXXX
  2. ��� DI_MEMOEDIT �� �� ������ ��������������� � ���� ������
*/
#endif


// ������ ������ ���� (Menu mode)
ENUM(MENUMODE)
{
	MM_LOCAL,  // ��������� ����
	MM_USER,   // ���������������� ����
	MM_GLOBAL, // ���������� ����
};

// ���� ������ �� ���� (Exit codes)
enum
{
	EC_CLOSE_LEVEL      = -1, // ����� �� ���� �� ���� ������� �����
	EC_CLOSE_MENU       = -2, // ����� �� ���� �� SHIFT+F10
	EC_PARENT_MENU      = -3, // �������� ���� ������������� ��������
	EC_MAIN_MENU        = -4, // �������� ������� ����
	EC_COMMAND_SELECTED = -5, // ������� ������� - ������� ���� � �������� �����
};

static int PrepareHotKey(string &strHotKey)
{
	int FuncNum=0;

	if (strHotKey.GetLength() > 1)
	{
		// ���� ������ ������ 1 �������, ������� ��� ������� "F?", ������ ��� �������� ������ ����� "F1"
		FuncNum=_wtoi(strHotKey.CPtr()+1);

		if (FuncNum < 1 || FuncNum > 24)
		{
			FuncNum=1;
			strHotKey=L"F1";
		}
	}
	else
	{
		// ��� ������� "&" ������������
		if (strHotKey.At(0) == L'&')
			strHotKey += L"&";
	}

	return FuncNum;
}

const wchar_t *LocalMenuFileName=L"FarMenu.ini";

static void MenuListToFile(std::list<UserMenuItem>& Menu, CachedWrite& CW)
{
	std::for_each(CONST_RANGE(Menu, i)
	{
		CW.Write(i.strHotKey.CPtr(), static_cast<DWORD>(i.strHotKey.GetLength()*sizeof(WCHAR)));
		CW.Write(L":  ", 3*sizeof(WCHAR));
		CW.Write(i.strLabel.CPtr(), static_cast<DWORD>(i.strLabel.GetLength()*sizeof(WCHAR)));
		CW.Write(L"\r\n", 2*sizeof(WCHAR));

		if (i.Submenu)
		{
			CW.Write(L"{\r\n", 3*sizeof(WCHAR));
			if (i.Menu)
				MenuListToFile(*i.Menu, CW);
			CW.Write(L"}\r\n", 3*sizeof(WCHAR));
		}
		else
		{
			std::for_each(CONST_RANGE(i.Commands, str)
			{
				CW.Write(L"    ", 4*sizeof(WCHAR));
				CW.Write(str.CPtr(), static_cast<DWORD>(str.GetLength()*sizeof(WCHAR)));
				CW.Write(L"\r\n", 2*sizeof(WCHAR));
			});
		}
	});
}

static void MenuFileToList(std::list<UserMenuItem>& Menu, File& MenuFile, GetFileString& GetStr, uintptr_t MenuCP = CP_UNICODE)
{
	INT64 Pos = MenuFile.GetPointer();
	if (!Pos)
	{
		if (!GetFileFormat(MenuFile,MenuCP))
			MenuCP = CP_OEMCP;
	}

	LPWSTR MenuStr = nullptr;
	int MenuStrLength = 0;
	UserMenuItem *MenuItem = nullptr;

	while (GetStr.GetString(&MenuStr, MenuCP, MenuStrLength))
	{
		RemoveTrailingSpaces(MenuStr);

		if (!*MenuStr)
			continue;

		if (*MenuStr==L'{' && MenuItem && MenuItem->Menu)
		{
			MenuFileToList(*MenuItem->Menu, MenuFile, GetStr, MenuCP);
			MenuItem = nullptr;
			continue;
		}

		// '}' can be a hotkey as well
		if (*MenuStr==L'}' && MenuStr[1] != L':')
			break;

		if (!IsSpace(*MenuStr))
		{
			wchar_t *ChPtr = nullptr;

			if (!(ChPtr=wcschr(MenuStr,L':')))
				continue;

			// special case: hotkey is ':'
			if (ChPtr[1] == ':')
			{
				++ChPtr;
			}

			Menu.resize(Menu.size() + 1);
			MenuItem = &Menu.back();

			*ChPtr = 0;
			MenuItem->strHotKey = MenuStr;
			MenuItem->strLabel = ChPtr+1;
			RemoveLeadingSpaces(MenuItem->strLabel);
			MenuItem->Submenu = (GetStr.PeekString(&MenuStr, MenuCP, MenuStrLength) && *MenuStr==L'{');

			if (MenuItem->Submenu)
				MenuItem->Menu = new std::list<UserMenuItem>;

			// Support for old 1.x separator format
			if (MenuCP==CP_OEMCP && MenuItem->strHotKey==L"-" && MenuItem->strLabel.IsEmpty())
			{
				MenuItem->strHotKey += L"-";
			}
		}
		else if (MenuItem)
		{
			RemoveLeadingSpaces(MenuStr);
			MenuItem->Commands.emplace_back(MenuStr);
		}
	}
}

UserMenu::UserMenu(bool ChoiceMenuType):
	MenuMode(MM_LOCAL),
	MenuModified(false),
	MenuNeedRefresh(false),
	ItemChanged(false)
{
	ProcessUserMenu(ChoiceMenuType, L"");
}

UserMenu::UserMenu(const string& MenuFileName):
	MenuMode(MM_LOCAL),
	MenuModified(false),
	MenuNeedRefresh(false),
	ItemChanged(false)
{
	ProcessUserMenu(false, MenuFileName);
}

UserMenu::~UserMenu()
{
}

void UserMenu::SaveMenu(const string& MenuFileName)
{
	if (MenuModified)
	{
		DWORD FileAttr=apiGetFileAttributes(MenuFileName);

		if (FileAttr != INVALID_FILE_ATTRIBUTES)
		{
			if (FileAttr & FILE_ATTRIBUTE_READONLY)
			{
				int AskOverwrite;
				AskOverwrite=Message(MSG_WARNING,2,MSG(MUserMenuTitle),LocalMenuFileName,MSG(MEditRO),MSG(MEditOvr),MSG(MYes),MSG(MNo));

				if (!AskOverwrite)
					apiSetFileAttributes(MenuFileName,FileAttr & ~FILE_ATTRIBUTE_READONLY);
			}

			if (FileAttr & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))
				apiSetFileAttributes(MenuFileName,FILE_ATTRIBUTE_NORMAL);
		}

		// Don't use CreationDisposition=CREATE_ALWAYS here - it kills alternate streams
		File MenuFile;
		if (MenuFile.Open(MenuFileName,GENERIC_WRITE, FILE_SHARE_READ, nullptr, FileAttr==INVALID_FILE_ATTRIBUTES?CREATE_NEW:TRUNCATE_EXISTING))
		{
			CachedWrite CW(MenuFile);
			WCHAR Data = SIGN_UNICODE;
			CW.Write(&Data, 1*sizeof(WCHAR));
			MenuListToFile(Menu, CW);
			CW.Flush();
			UINT64 Size = 0;
			MenuFile.GetSize(Size);
			MenuFile.Close();

			// ���� ���� FarMenu.ini ����, �� ������ ���
			if (Size<3) // 2 for BOM
			{
				apiDeleteFile(MenuFileName);
			}
			else if (FileAttr!=INVALID_FILE_ATTRIBUTES)
			{
				apiSetFileAttributes(MenuFileName,FileAttr);
			}
		}
	}
}

void UserMenu::ProcessUserMenu(bool ChoiceMenuType,const string& MenuFileName)
{
	// ���� � �������� �������� � ������ LocalMenuFileName
	string strMenuFilePath;
	Global->CtrlObject->CmdLine->GetCurDir(strMenuFilePath);
	// �� ��������� ���� - ��� FarMenu.ini
	MenuMode = MM_LOCAL;
	MenuModified = false;

	if (ChoiceMenuType)
	{
		int EditChoice=Message(0,3,MSG(MUserMenuTitle),MSG(MChooseMenuType),MSG(MChooseMenuMain),MSG(MChooseMenuLocal),MSG(MCancel));

		if (EditChoice<0 || EditChoice==2)
			return;

		if (!EditChoice)
		{
			MenuMode = MM_GLOBAL;
			strMenuFilePath = Global->Opt->GlobalUserMenuDir;
		}
	}

	// �������� ���� ���������
	bool FirstRun=true;
	int ExitCode = 0;

	while ((ExitCode != EC_CLOSE_LEVEL) && (ExitCode != EC_CLOSE_MENU) && (ExitCode != EC_COMMAND_SELECTED))
	{
		string strMenuFileFullPath;
		if (MenuFileName.IsEmpty())
		{
			strMenuFileFullPath = strMenuFilePath;
			AddEndSlash(strMenuFileFullPath);
			strMenuFileFullPath += LocalMenuFileName;
		}
		else
		{
			strMenuFileFullPath = MenuFileName;
		}

		Menu.clear();

		// �������� ������� ���� �� ��������� �����
		File MenuFile;
		bool FileOpened = PathCanHoldRegularFile(strMenuFilePath) ? MenuFile.Open(strMenuFileFullPath,GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING) : false;
		if (FileOpened)
		{
			GetFileString GetStr(MenuFile);
			MenuFileToList(Menu, MenuFile, GetStr);
			MenuFile.Close();
		}
		else if (MenuMode != MM_USER)
		{
			// ���� �� ��������. ������� ������.
			if (MenuMode == MM_GLOBAL) // ��� � %FARHOME%?
			{
				MenuMode = MM_USER;
				strMenuFilePath = Global->Opt->ProfilePath;
				continue;
			}
			else if (!ChoiceMenuType)
			{
				if (!FirstRun)
				{
					// ���������� ����...
					if(!IsRootPath(strMenuFilePath))
					{
						size_t pos;
						if (FindLastSlash(pos,strMenuFilePath))
						{
							strMenuFilePath.SetLength(pos--);
							continue;
						}
					}
				}

				FirstRun = false;
				MenuMode = MM_GLOBAL;
				strMenuFilePath = Global->Opt->GlobalUserMenuDir;
				continue;
			}
		}

		FARMACROAREA PrevMacroMode=Global->CtrlObject->Macro.GetMode();
		int _CurrentFrame=FrameManager->GetCurrentFrame()->GetType();
		Global->CtrlObject->Macro.SetMode(MACROAREA_USERMENU);
		// �������� ����
		ExitCode=ProcessSingleMenu(Menu, 0, Menu, strMenuFileFullPath);

		if (_CurrentFrame == FrameManager->GetCurrentFrame()->GetType()) //???
			Global->CtrlObject->Macro.SetMode(PrevMacroMode);

		// ...������� ��������� ������� � ����
		SaveMenu(strMenuFileFullPath);

		// ��� ���� ����� ������ ����?
		switch (ExitCode)
		{
				// �������� ���� ������������� ��������
			case EC_PARENT_MENU:
			{
				if (MenuMode == MM_LOCAL)
				{
					if(!IsRootPath(strMenuFilePath))
					{
						size_t pos;
						if (FindLastSlash(pos,strMenuFilePath))
						{
							strMenuFilePath.SetLength(pos--);
							continue;
						}
					}

					MenuMode = MM_GLOBAL;
					strMenuFilePath = Global->Opt->GlobalUserMenuDir;
				}
				else
				{
					MenuMode = MM_USER;
					strMenuFilePath = Global->Opt->ProfilePath;
				}

				break;
			}
			// �������� ������� ����
			case EC_MAIN_MENU:
			{
				// $ 14.07.2000 VVM: Shift+F2 ����������� ������� ����/��������� � �����
				switch (MenuMode)
				{
					case MM_LOCAL:
						MenuMode = MM_GLOBAL;
						strMenuFilePath = Global->Opt->GlobalUserMenuDir;
						break;

					case MM_GLOBAL:
						MenuMode = MM_USER;
						strMenuFilePath = Global->Opt->ProfilePath;
						break;

					default: // MM_USER
						Global->CtrlObject->CmdLine->GetCurDir(strMenuFilePath);
						MenuMode=MM_LOCAL;
				}

				break;
			}
		}
	}

	if (FrameManager->IsPanelsActive() && (ExitCode == EC_COMMAND_SELECTED || MenuModified))
		ShellUpdatePanels(Global->CtrlObject->Cp()->ActivePanel,FALSE);
}

// ���������� ����
int FillUserMenu(VMenu2& FarUserMenu, std::list<UserMenuItem>& Menu, int MenuPos,int *FuncPos,const string& Name,const string& ShortName)
{
	FarUserMenu.DeleteItems();
	MenuItemEx FarUserMenuItem;
	int NumLines=0;

	for (auto MenuItem = Menu.begin(); MenuItem != Menu.end(); ++MenuItem, ++NumLines)
	{
		FarUserMenuItem.Clear();
		int FuncNum=0;

		// ����������� �������� ������, ����� ������ == "--"
		if (MenuItem->strHotKey == L"--")
		{
			FarUserMenuItem.Flags|=LIF_SEPARATOR;
			FarUserMenuItem.strName=MenuItem->strLabel;

			if (NumLines==MenuPos)
			{
				MenuPos++;
			}
		}
		else
		{
			string strLabel = MenuItem->strLabel;
			SubstFileName(nullptr,strLabel,Name,ShortName,nullptr,nullptr,nullptr,nullptr,TRUE);
			apiExpandEnvironmentStrings(strLabel, strLabel);
			string strHotKey = MenuItem->strHotKey;
			FuncNum = PrepareHotKey(strHotKey);
			int Offset = strHotKey.At(0)==L'&'?5:4;
			FarUserMenuItem.strName=FormatString()<<((!strHotKey.IsEmpty() && !FuncNum)?L"&":L"")<<fmt::LeftAlign()<<fmt::ExactWidth(Offset)<<strHotKey;
			FarUserMenuItem.strName+=strLabel;

			if (MenuItem->Submenu)
			{
				FarUserMenuItem.Flags|=MIF_SUBMENU;
			}

			FarUserMenuItem.SetSelect(NumLines==MenuPos);
		}

		int ItemPos=FarUserMenu.AddItem(&FarUserMenuItem);

		FarUserMenu.SetUserData(&MenuItem,sizeof(MenuItem),ItemPos);

		if (FuncNum>0)
		{
			FuncPos[FuncNum-1]=ItemPos;
		}
	}
	return NumLines;
}

// ��������� ���������� ����
int UserMenu::ProcessSingleMenu(std::list<UserMenuItem>& Menu, int MenuPos, std::list<UserMenuItem>& MenuRoot, const string& MenuFileName, const wchar_t *Title)
{
	for (;;)
	{
		int NumLine=0, ExitCode, FuncPos[24];

		// ������� F-�������
		std::fill(ALL_RANGE(FuncPos), -1);

		string strName,strShortName;
		Global->CtrlObject->Cp()->ActivePanel->GetCurName(strName,strShortName);
		/* $ 24.07.2000 VVM + ��� ������ �������� ���� � ��������� ��������� ��� - FAR/Registry */
		string strMenuTitle;

		if (Title && *Title)
		{
			strMenuTitle = Title;
		}
		else
		{
			switch (MenuMode)
			{
				case MM_LOCAL:
					strMenuTitle = MSG(MLocalMenuTitle);
					break;

				case MM_GLOBAL:
					strMenuTitle = MSG(MMainMenuTitle);
					strMenuTitle += L" (";
					strMenuTitle += MSG(MMainMenuGlobal);
					strMenuTitle += L")";
					break;

				default:
					strMenuTitle = MSG(MMainMenuTitle);
					strMenuTitle += L" (";
					strMenuTitle += MSG(MMainMenuUser);
					strMenuTitle += L")";
			}
		}

		VMenu2 UserMenu(strMenuTitle,nullptr,0,ScrY-4);
		UserMenu.SetFlags(VMENU_WRAPMODE);
		UserMenu.SetHelp(L"UserMenu");
		UserMenu.SetPosition(-1,-1,0,0);
		UserMenu.SetBottomTitle(MSG(MMainMenuBottomTitle));
		UserMenu.SetMacroMode(MACROAREA_USERMENU);

		int ReturnCode=1;

		NumLine=FillUserMenu(UserMenu,Menu,MenuPos,FuncPos,strName,strShortName);
		ITERATOR(Menu)* CurrentMenuItem;
		ExitCode=UserMenu.Run([&](int Key)->int
		{
			MenuPos=UserMenu.GetSelectPos();
			void* userdata = UserMenu.GetUserData(nullptr, 0, MenuPos);
			CurrentMenuItem = reinterpret_cast<decltype(CurrentMenuItem)>(userdata);

			if (Key==KEY_SHIFTF1)
			{
				UserMenu.Key(KEY_F1);
				return 1;
			}
			if (Key==KEY_SHIFTF11)
			{
				UserMenu.Key(KEY_F11);
				return 1;
			}
			if ((unsigned int)Key>=KEY_F1 && (unsigned int)Key<=KEY_F24)
			{
				int FuncItemPos;

				if ((FuncItemPos=FuncPos[Key-KEY_F1])!=-1)
				{
					UserMenu.Close(FuncItemPos);
					return 1;
				}
			}
			else if (Key == L' ') // ��������� ������ �� "�������"!
				return 1;

			int KeyProcessed = 1;
			switch (Key)
			{
					/* $ 24.08.2001 VVM + ������� ������/����� ���������/��������� ������� �������������� */
				case KEY_RIGHT:
				case KEY_NUMPAD6:
				case KEY_MSWHEEL_RIGHT:
					if (CurrentMenuItem && (*CurrentMenuItem)->Submenu)
						UserMenu.Close(MenuPos);
					break;

				case KEY_LEFT:
				case KEY_NUMPAD4:
				case KEY_MSWHEEL_LEFT:
					if (Title && *Title)
						UserMenu.Close(-1);
					break;

				case KEY_NUMDEL:
				case KEY_DEL:
					if (CurrentMenuItem)
					{
						DeleteMenuRecord(Menu,(*CurrentMenuItem));
						NumLine=FillUserMenu(UserMenu,Menu,MenuPos,FuncPos,strName,strShortName);
					}
					break;

				case KEY_INS:
				case KEY_F4:
				case KEY_SHIFTF4:
				case KEY_NUMPAD0:
				{
					bool bNew = Key == KEY_INS || Key == KEY_NUMPAD0;
					if (!bNew && !CurrentMenuItem)
						break;

					EditMenu(Menu, CurrentMenuItem, bNew);
					NumLine = FillUserMenu(UserMenu,Menu,MenuPos,FuncPos,strName,strShortName);
					break;
				}

				case KEY_CTRLUP:
				case KEY_RCTRLUP:
				case KEY_CTRLDOWN:
				case KEY_RCTRLDOWN:
				{

					if (CurrentMenuItem)
					{
						int Pos=UserMenu.GetSelectPos();
						if (!((Key==KEY_CTRLUP || Key==KEY_RCTRLUP) && !Pos) && !((Key==KEY_CTRLDOWN || Key==KEY_RCTRLDOWN) && Pos==UserMenu.GetItemCount()-1))
						{
							MenuModified = true;
							auto Other = *CurrentMenuItem;
							if (Key==KEY_CTRLUP || Key==KEY_RCTRLUP)
							{
								--Other;
								--MenuPos;
								Menu.splice(Other, Menu, *CurrentMenuItem);
							}
							else
							{
								++Other;
								++MenuPos;
								Menu.splice(*CurrentMenuItem, Menu, Other);
							}
							FillUserMenu(UserMenu,Menu,MenuPos,FuncPos,strName,strShortName);
						}
					}
				}
				break;

				case KEY_ALTF4:       // ������������� ��� ����
				case KEY_RALTF4:
				{
					File MenuFile;
					FrameManager->GetFrame(0)->Unlock();
					{
						ConsoleTitle *OldTitle=new ConsoleTitle;
						SaveMenu(MenuFileName);
						FileEditor ShellEditor(MenuFileName,CP_UNICODE,FFILEEDIT_DISABLEHISTORY,-1,-1,nullptr);
						delete OldTitle;
						ShellEditor.SetDynamicallyBorn(false);
						FrameManager->EnterModalEV();
						FrameManager->ExecuteModal();
						FrameManager->ExitModalEV();
						if (!ShellEditor.IsFileChanged() || (!MenuFile.Open(MenuFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING)))
						{
							ReturnCode=0;
							UserMenu.Close(-1);
							return 1;
						}
					}
					MenuRoot.clear();
					GetFileString GetStr(MenuFile);
					MenuFileToList(MenuRoot, MenuFile, GetStr);
					MenuFile.Close();
					MenuModified=true;
					ReturnCode=0;
					UserMenu.Close(-1);

					return 1; // ������� ����
				}

				/* $ 28.06.2000 tran
				����� �� ����������������� ���� �� ShiftF10 �� ������ ������
				����������� ������ ������ ExitCode -1, � ���������� FALSE -
				�� FALSE ��� � ������ ������ ������ */
				case KEY_SHIFTF10:
					//UserMenu.SetExitCode(-1);
					ReturnCode=EC_CLOSE_MENU;
					UserMenu.Close(-1);
					return 1;

				case KEY_SHIFTF2: // �������� ������� ����
					ReturnCode=EC_MAIN_MENU;
					UserMenu.Close(-1);
					return 1;

				case KEY_BS: // �������� ���� �� ������������� �������� ������ � MM_LOCAL ������

					if (MenuMode == MM_LOCAL)
					{
						ReturnCode=EC_PARENT_MENU;
						UserMenu.Close(-1);
						return 1;
					}

				default:

					KeyProcessed = 0;

					if (MenuPos!=UserMenu.GetSelectPos())
					{
						MenuPos=UserMenu.GetSelectPos();
						void* userdata = UserMenu.GetUserData(nullptr, 0, MenuPos);
						CurrentMenuItem = reinterpret_cast<decltype(CurrentMenuItem)>(userdata);
					}
			} // switch(Key)
			return KeyProcessed;
		});

		if (ReturnCode!=1)
			return ReturnCode;

		if (ExitCode<0 || ExitCode>=NumLine || !CurrentMenuItem)
			return EC_CLOSE_LEVEL; //  ����� �� ���� �������

		void* userdata = UserMenu.GetUserData(nullptr, 0, MenuPos);
		CurrentMenuItem = reinterpret_cast<decltype(CurrentMenuItem)>(userdata);

		if (!CurrentMenuItem)
			return EC_CLOSE_LEVEL; //  ����� �� ���� �������

		if ((*CurrentMenuItem)->Submenu)
		{
			/* $ 20.08.2001 VVM + ��� ��������� ���� ���������� ��������� ���������� */
			string strSubMenuLabel = (*CurrentMenuItem)->strLabel;
			SubstFileName(nullptr,strSubMenuLabel,strName,strShortName,nullptr,nullptr,nullptr,nullptr,TRUE);
			apiExpandEnvironmentStrings(strSubMenuLabel, strSubMenuLabel);

			size_t pos;
			if (strSubMenuLabel.Pos(pos,L'&'))
				strSubMenuLabel.LShift(1,pos);

			string strSubMenuTitle;
			if (Title && *Title)
			{
				strSubMenuTitle = Title;
				strSubMenuTitle += L" -> ";
				strSubMenuTitle += strSubMenuLabel;
			}
			else
			{
				strSubMenuTitle = strSubMenuLabel;
			}

			/* $ 14.07.2000 VVM ! ���� ������� �������, �� ��������. ����� �������� ���������� ���� */
			MenuPos = ProcessSingleMenu(*(*CurrentMenuItem)->Menu, 0, MenuRoot, MenuFileName, strSubMenuTitle.CPtr());

			if (MenuPos!=EC_CLOSE_LEVEL)
				return MenuPos;

			MenuPos = ExitCode;
			continue;
		}

		/* $ 01.05.2001 IS �������� �� ������ ������ */
		//int LeftVisible,RightVisible,PanelsHidden=0;
		string strCmdLineDir;
		Global->CtrlObject->CmdLine->GetCurDir(strCmdLineDir);
		Global->CtrlObject->CmdLine->LockUpdatePanel(TRUE);

		// ���� ���������� ������ ���� (CommandX)
		std::for_each(CONST_RANGE((*CurrentMenuItem)->Commands, str)
		{
			string strCommand = str;

			string strListName, strAnotherListName;
			string strShortListName, strAnotherShortListName;

			if (!((!StrCmpNI(strCommand.CPtr(),L"REM",3) && IsSpaceOrEos(strCommand.At(3))) || !StrCmpNI(strCommand.CPtr(),L"::",2)))
			{
				/*
				  �������� ��������� ���������� ��������, ��������:
				  if exist !#!\!^!.! far:edit < diff -c -p !#!\!^!.! !\!.!
				  �.�. ������� "���������" ����� "if exist !#!\!^!.!", �� � ����
				  ����������, �� ������ ������.
				  ��� ��� ������,
				  if exist ..\a.bat D:\FAR\170\DIFF.MY\mkdiff.bat !?&����� �����?!
				  ��� ����������� ������, �.�. ������� ���� ������ ����, � ����
				  ��������� ���� "if exist ..\a.bat", � �� ����� ������ ������...
				*/
				//if(ExtractIfExistCommand(Command))
				{
					/* $ 01.05.2001 IS �������� �� ������ ������ */
					/*
					if (!PanelsHidden)
					{
						LeftVisible=Global->CtrlObject->Cp()->LeftPanel->IsVisible();
						RightVisible=Global->CtrlObject->Cp()->RightPanel->IsVisible();
						Global->CtrlObject->Cp()->LeftPanel->Hide();
						Global->CtrlObject->Cp()->RightPanel->Hide();
						Global->CtrlObject->Cp()->LeftPanel->SetUpdateMode(FALSE);
						Global->CtrlObject->Cp()->RightPanel->SetUpdateMode(FALSE);
						PanelsHidden=TRUE;
					}
					*/
					//;
					string strTempStr;
					strTempStr = (*CurrentMenuItem)->strLabel;
					ReplaceStrings(strTempStr,L"&",L"",-1);

					int PreserveLFN=SubstFileName(strTempStr.CPtr(),strCommand, strName, strShortName, &strListName, &strAnotherListName, &strShortListName, &strAnotherShortListName, FALSE, strCmdLineDir.CPtr());
					bool ListFileUsed=!strListName.IsEmpty()||!strAnotherListName.IsEmpty()||!strShortListName.IsEmpty()||!strAnotherShortListName.IsEmpty();

					if (ExtractIfExistCommand(strCommand))
					{
						PreserveLongName PreserveName(strShortName,PreserveLFN);
						RemoveExternalSpaces(strCommand);

						if (!strCommand.IsEmpty())
						{
							Global->CtrlObject->CmdLine->ExecString(strCommand,FALSE, 0, 0, ListFileUsed, false, true);
						}
					}
				}
			} // strCommand != "REM"

			if (!strListName.IsEmpty())
				apiDeleteFile(strListName);

			if (!strAnotherListName.IsEmpty())
				apiDeleteFile(strAnotherListName);

			if (!strShortListName.IsEmpty())
				apiDeleteFile(strShortListName);

			if (!strAnotherShortListName.IsEmpty())
				apiDeleteFile(strAnotherShortListName);
		});

		Global->CtrlObject->CmdLine->LockUpdatePanel(FALSE);

		/* $ 01.05.2001 IS �������� �� ������ ������ */
		/*
		if (PanelsHidden)
		{
			Global->CtrlObject->Cp()->LeftPanel->SetUpdateMode(TRUE);
			Global->CtrlObject->Cp()->RightPanel->SetUpdateMode(TRUE);
			Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
			Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
			if (RightVisible)
				Global->CtrlObject->Cp()->RightPanel->Show();
			if (LeftVisible)
				Global->CtrlObject->Cp()->LeftPanel->Show();
		}
		*/
		/* $ 14.07.2000 VVM ! ������� ���� */
		/* $ 25.04.2001 DJ - ��������, ��� ���� ��������� ������� (����� ������������ ������) */
		return EC_COMMAND_SELECTED;
	}
}

enum EditMenuItems
{
	EM_DOUBLEBOX,
	EM_HOTKEY_TEXT,
	EM_HOTKEY_EDIT,
	EM_LABEL_TEXT,
	EM_LABEL_EDIT,
	EM_SEPARATOR1,
	EM_COMMANDS_TEXT,
#ifdef PROJECT_DI_MEMOEDIT
	EM_MEMOEDIT,
#else
	EM_EDITLINE_0,
	EM_EDITLINE_1,
	EM_EDITLINE_2,
	EM_EDITLINE_3,
	EM_EDITLINE_4,
	EM_EDITLINE_5,
	EM_EDITLINE_6,
	EM_EDITLINE_7,
	EM_EDITLINE_8,
	EM_EDITLINE_9,
#endif
	EM_SEPARATOR2,
	EM_BUTTON_OK,
	EM_BUTTON_CANCEL,
};

intptr_t UserMenu::EditMenuDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
		case DN_EDITCHANGE:
#ifdef PROJECT_DI_MEMOEDIT
			if (Param1 == EM_MEMOEDIT)
#else
			if (Param1 >= EM_EDITLINE_0 && Param1 <= EM_EDITLINE_9)
#endif
				ItemChanged = true;
			break;

		case DN_CLOSE:

			if (Param1==EM_BUTTON_OK)
			{
				BOOL Result=TRUE;
				LPCWSTR HotKey=reinterpret_cast<LPCWSTR>(Dlg->SendMessage(DM_GETCONSTTEXTPTR,EM_HOTKEY_EDIT,0));
				LPCWSTR Label=reinterpret_cast<LPCWSTR>(Dlg->SendMessage(DM_GETCONSTTEXTPTR,EM_LABEL_EDIT,0));
				int FocusPos=-1;

				if(StrCmp(HotKey,L"--"))
				{
					if (!*Label)
					{
						FocusPos=EM_LABEL_EDIT;
					}
					else if (StrLength(HotKey)>1)
					{
						FocusPos=EM_HOTKEY_EDIT;

						if (Upper(*HotKey)==L'F')
						{
							int FuncNum=_wtoi(HotKey+1);

							if (FuncNum > 0 && FuncNum < 25)
								FocusPos=-1;
						}
					}
				}

				if (FocusPos!=-1)
				{
					Message(MSG_WARNING,1,MSG(MUserMenuTitle),MSG((*Label?MUserMenuInvalidInputHotKey:MUserMenuInvalidInputLabel)),MSG(MOk));
					Dlg->SendMessage(DM_SETFOCUS,FocusPos,0);
					Result=FALSE;
				}

				return Result;
			}
			else if (ItemChanged)
			{
				switch(Message(MSG_WARNING, 3, MSG(MUserMenuTitle), MSG(MEditMenuConfirmation), MSG(MHYes), MSG(MHNo), MSG(MHCancel)))
				{
				case 0:
					Dlg->SendMessage( DM_CLOSE, EM_BUTTON_OK, nullptr);
					break;
				case 1:
					return TRUE;
				case 2:
				default:
					return FALSE;
				}
			}

			break;
		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}


bool UserMenu::EditMenu(std::list<UserMenuItem>& Menu, std::list<UserMenuItem>::iterator* MenuItem, bool Create)
{
	bool Result = false;
	bool SubMenu = false;
	bool Continue = true;
	ItemChanged = false;

	if (Create)
	{
		switch (Message(0,2,MSG(MUserMenuTitle),MSG(MAskInsertMenuOrCommand),MSG(MMenuInsertCommand),MSG(MMenuInsertMenu)))
		{
			case -1:
			case -2:
				Continue = false;
			case 1:
				SubMenu = true;
		}
	}
	else
	{
		SubMenu = (*MenuItem)->Submenu;
	}

	if (Continue)
	{
		const int DLG_X=76, DLG_Y=SubMenu?10:22;
		FARDIALOGITEMFLAGS State=SubMenu?DIF_HIDDEN|DIF_DISABLE:DIF_NONE;
		FarDialogItem EditDlgData[]=
		{
			{DI_DOUBLEBOX,3,1,DLG_X-4,DLG_Y-2,0,nullptr,nullptr,0,MSG(SubMenu?MEditSubmenuTitle:MEditMenuTitle)},
			{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MEditMenuHotKey)},
			{DI_FIXEDIT,5,3,7,3,0,nullptr,nullptr,DIF_FOCUS,L""},
			{DI_TEXT,5,4,0,4,0,nullptr,nullptr,0,MSG(MEditMenuLabel)},
			{DI_EDIT,5,5,DLG_X-6,5,0,nullptr,nullptr,0,L""},

			{DI_TEXT,-1,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR|State,L""},
			{DI_TEXT,5,7,0,7,0,nullptr,nullptr,State,MSG(MEditMenuCommands)},
#ifdef PROJECT_DI_MEMOEDIT
			{DI_MEMOEDIT,5, 8,DLG_X-6,17,0,nullptr,nullptr,DIF_EDITPATH,L""},
#else
			{DI_EDIT,5, 8,DLG_X-6,8,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5, 9,DLG_X-6,9,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5,10,DLG_X-6,10,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5,11,DLG_X-6,11,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5,12,DLG_X-6,12,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5,13,DLG_X-6,13,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5,14,DLG_X-6,14,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5,15,DLG_X-6,15,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5,16,DLG_X-6,16,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
			{DI_EDIT,5,17,DLG_X-6,17,0,nullptr,nullptr,DIF_EDITPATH|DIF_EDITPATHEXEC|DIF_EDITOR|State,L""},
#endif

			{DI_TEXT,-1,DLG_Y-4,0,DLG_Y-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_BUTTON,0,DLG_Y-3,0,DLG_Y-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
			{DI_BUTTON,0,DLG_Y-3,0,DLG_Y-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
		};
		MakeDialogItemsEx(EditDlgData,EditDlg);
#ifndef PROJECT_DI_MEMOEDIT
		enum {DI_EDIT_COUNT=EM_SEPARATOR2-EM_COMMANDS_TEXT-1};
#endif

		if (!Create)
		{
			EditDlg[EM_HOTKEY_EDIT].strData = (*MenuItem)->strHotKey;
			EditDlg[EM_LABEL_EDIT].strData = (*MenuItem)->strLabel;
#if defined(PROJECT_DI_MEMOEDIT)
			/*
				...
				����� ������� ����� �� "Command%d" � EMR_MEMOEDIT
				...
			*/
			string strBuffer;
			for (string *str=MenuItem->Commands.First(); str && CommandNumber < DI_EDIT_COUNT; str=MenuItem->Commands.Next(str))
			{
				strBuffer+=*str;
				strBuffer+=L"\n";    //??? "\n\r"
			}

			EditDlg[EM_MEMOEDIT].strData = strBuffer; //???
#else
			int CommandNumber=0;
			for (auto str = (*MenuItem)->Commands.begin(); str != (*MenuItem)->Commands.end() && CommandNumber < DI_EDIT_COUNT; ++str)
			{
				EditDlg[EM_EDITLINE_0+CommandNumber].strData = *str;
				CommandNumber++;
			}
#endif
		}

		Dialog Dlg(this, &UserMenu::EditMenuDlgProc, nullptr, EditDlg, ARRAYSIZE(EditDlg));
		Dlg.SetHelp(L"UserMenu");
		Dlg.SetPosition(-1,-1,DLG_X,DLG_Y);
		Dlg.Process();

		if (Dlg.GetExitCode()==EM_BUTTON_OK)
		{
			MenuModified=true;
			auto NewItemIterator = Menu.end();

			if (Create)
			{
				NewItemIterator = Menu.emplace(MenuItem? *MenuItem : Menu.begin(), UserMenuItem());
				MenuItem = &NewItemIterator;
			}

			(*MenuItem)->strHotKey = EditDlg[EM_HOTKEY_EDIT].strData;
			(*MenuItem)->strLabel = EditDlg[EM_LABEL_EDIT].strData;
			(*MenuItem)->Submenu = SubMenu;

			if (SubMenu)
			{
				if (Create)
					(*MenuItem)->Menu = new std::list<UserMenuItem>();
			}
			else
			{
#if defined(PROJECT_DI_MEMOEDIT)
				/*
				...
				����� �������������� ����������� ����� EMR_MEMOEDIT � "Command%d"
				...
				*/
#else
				int CommandNumber=0;

				for (int i=0 ; i < DI_EDIT_COUNT ; i++)
					if (!EditDlg[i+EM_EDITLINE_0].strData.IsEmpty())
						CommandNumber=i+1;

				(*MenuItem)->Commands.clear();
				for (int i=0 ; i < DI_EDIT_COUNT ; i++)
				{
					if (i>=CommandNumber)
						break;
					else
						(*MenuItem)->Commands.emplace_back(EditDlg[i+EM_EDITLINE_0].strData);
				}
#endif
			}

			Result=true;
		}
	}

	return Result;
}

bool UserMenu::DeleteMenuRecord(std::list<UserMenuItem>& Menu, std::list<UserMenuItem>::iterator& MenuItem)
{
	string strItemName=MenuItem->strLabel;
	InsertQuote(strItemName);

	if (Message(MSG_WARNING,2,MSG(MUserMenuTitle),MSG(!MenuItem->Submenu?MAskDeleteMenuItem:MAskDeleteSubMenuItem),strItemName.CPtr(),MSG(MDelete),MSG(MCancel)))
		return false;

	MenuModified=true;
	Menu.erase(MenuItem);
	return true;
}
