/*
editcontrol.cpp

���������� ��� Edit.
��������� ������ ����� ��� �������� � ��������� (�� ��� ���������)

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

#include "editcontrol.hpp"
#include "config.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
#include "language.hpp"
#include "pathmix.hpp"
#include "history.hpp"
#include "vmenu2.hpp"
#include "imports.hpp"
#include "console.hpp"
#include "elevation.hpp"
#include "colormix.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "ctrlobj.hpp"

EditControl::EditControl(ScreenObject *pOwner,Callback* aCallback,bool bAllocateData,History* iHistory,FarList* iList,DWORD iFlags):
	Edit(pOwner,bAllocateData),
	pHistory(iHistory),
	pList(iList),
	MaxLength(-1),
	CursorSize(-1),
	CursorPos(0),
	PrevCurPos(0),
	MacroSelectionStart(-1),
	SelectionStart(-1),
	MacroAreaAC(MACROAREA_DIALOGAUTOCOMPLETION),
	ECFlags(iFlags),
	Selection(false),
	MenuUp(false),
	ACState(ECFlags.Check(EC_ENABLEAUTOCOMPLETE)),
	CallbackSaveState(false)
{
	SetObjectColor();

	if (aCallback)
	{
		m_Callback=*aCallback;
	}
	else
	{
		m_Callback.Active=true;
		m_Callback.m_Callback=nullptr;
		m_Callback.m_Param=nullptr;
	}
}

void EditControl::Show()
{
	if(X2-X1+1>StrSize)
	{
		SetLeftPos(0);
	}
	if (pOwner->IsVisible())
	{
		Edit::Show();
	}
}

void EditControl::Changed(bool DelBlock)
{
	Flags.Set(FEDITLINE_CMP_CHANGED);
	if(m_Callback.Active)
	{
		if(m_Callback.m_Callback)
		{
			m_Callback.m_Callback(m_Callback.m_Param);
		}
		AutoComplete(false, DelBlock);
	}
}

void EditControl::SetMenuPos(VMenu2& menu)
{
	int MaxHeight = std::min(Global->Opt->Dialogs.CBoxMaxHeight.Get(),(long long)menu.GetItemCount())+2;
	if((ScrY-Y1<MaxHeight && Y1>ScrY/2) || MenuUp)
	{
		MenuUp = true;
		menu.SetPosition(X1, std::max(0, Y1-1-MaxHeight), std::min(ScrX-2,(int)X2), Y1-1);
	}
	else
	{
		menu.SetPosition(X1,Y1+1,X2,std::min(static_cast<int>(ScrY), Y1+1+MaxHeight));
	}
}

void EnumFiles(VMenu2& Menu, const string& Str)
{
	if(!Str.empty())
	{
		string strStr(Str);

		bool OddQuote = false;
		for(size_t i=0; i<strStr.size(); i++)
		{
			if(strStr[i] == L'"')
			{
				OddQuote = !OddQuote;
			}
		}

		size_t Pos = 0;
		if(OddQuote)
		{
			Pos = strStr.rfind(L'"');
		}
		else
		{
			for(Pos=strStr.size()-1; Pos!=static_cast<size_t>(-1); Pos--)
			{
				if(strStr[Pos]==L'"')
				{
					Pos--;
					while(strStr[Pos]!=L'"' && Pos!=static_cast<size_t>(-1))
					{
						Pos--;
					}
				}
				else if(strStr[Pos]==L' ')
				{
					Pos++;
					break;
				}
			}
		}
		if(Pos==static_cast<size_t>(-1))
		{
			Pos=0;
		}
		bool StartQuote=false;
		if(Pos < strStr.size() && strStr[Pos]==L'"')
		{
			Pos++;
			StartQuote=true;
		}
		string strStart(strStr.data(),Pos);
		strStr.erase(0, Pos);
		Unquote(strStr);
		if(!strStr.empty())
		{
			FAR_FIND_DATA d;
			string strExp;
			apiExpandEnvironmentStrings(strStr,strExp);
			FindFile Find(strExp+L"*");
			bool Separator=false;
			while(Find.Get(d))
			{
				const wchar_t* FileName=PointToName(strStr);
				bool NameMatch=!StrCmpNI(FileName,d.strFileName.data(),StrLength(FileName)),AltNameMatch=NameMatch?false:!StrCmpNI(FileName,d.strAlternateFileName.data(),StrLength(FileName));
				if(NameMatch || AltNameMatch)
				{
					strStr.resize(FileName-strStr.data());
					string strAdd (strStr + (NameMatch ? d.strFileName : d.strAlternateFileName));
					if (!StartQuote)
						QuoteSpace(strAdd);

					string strTmp(strStart+strAdd);
					if(StartQuote)
						strTmp += L'"';

					if(!Separator)
					{
						if(Menu.GetItemCount())
						{
							MenuItemEx Item={};
							Item.strName = MSG(MCompletionFilesTitle);
							Item.Flags=LIF_SEPARATOR;
							Menu.AddItem(&Item);
						}
						else
						{
							Menu.SetTitle(MSG(MCompletionFilesTitle));
						}
						Separator=true;
					}
					Menu.AddItem(strTmp);
				}
			}
		}
	}
}

bool EnumModules(const string& Module, VMenu2* DestMenu)
{
	bool Result=false;

	DisableElevation de;

	if(!Module.empty() && !FirstSlash(Module.data()))
	{
		std::list<string> List;
		string str;
		int ModuleLength = static_cast<int>(Module.size());
		auto ExcludeCmdsList(StringToList(Global->Opt->Exec.strExcludeCmds));
		std::for_each(CONST_RANGE(ExcludeCmdsList, i)
		{
			if (!StrCmpNI(Module.data(), i.data(), ModuleLength))
			{
				Result=true;
				str = i;
				if(std::find(List.cbegin(), List.cend(), str) == List.cend())
				{
					List.emplace_back(str);
				}
			}
		});

		string strName=Module;
		string strPathExt(L".COM;.EXE;.BAT;.CMD;.VBS;.JS;.WSH");
		apiGetEnvironmentVariable(L"PATHEXT",strPathExt);
		auto PathExtList(StringToList(strPathExt));

		string strPathEnv;
		if (apiGetEnvironmentVariable(L"PATH", strPathEnv))
		{
			auto PathList(StringToList(strPathEnv));

			std::for_each(CONST_RANGE(PathList, i)
			{
				string strDest;

				FAR_FIND_DATA data;
				string str(i);
				AddEndSlash(str);
				str.append(strName).append(L"*");
				FindFile Find(str);
				while(Find.Get(data))
				{
					std::for_each(CONST_RANGE(PathExtList, Ext)
					{
						LPCWSTR ModuleExt=wcsrchr(data.strFileName.data(),L'.');
						if(!StrCmpI(ModuleExt, Ext.data()))
						{
							str = data.strFileName;
							if(std::find(List.cbegin(), List.cend(), str) == List.cend())
							{
								List.emplace_back(str);
							}
							Result=true;
						}
					});
				}
			});
		}


		static const WCHAR RegPath[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
		HKEY RootFindKey[]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE,HKEY_LOCAL_MACHINE};

		DWORD samDesired = KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE;
		DWORD RedirectionFlag = 0;
		// App Paths key is shared in Windows 7 and above
		if (Global->WinVer() < _WIN32_WINNT_WIN7)
		{
#ifdef _WIN64
			RedirectionFlag = KEY_WOW64_32KEY;
#else
			BOOL Wow64Process = FALSE;
			if (Global->ifn->IsWow64Process(GetCurrentProcess(), &Wow64Process) && Wow64Process)
			{
				RedirectionFlag = KEY_WOW64_64KEY;
			}
#endif
		}
		for (size_t i=0; i<ARRAYSIZE(RootFindKey); i++)
		{
			if (i==ARRAYSIZE(RootFindKey)-1)
			{
				if(RedirectionFlag)
				{
					samDesired|=RedirectionFlag;
				}
				else
				{
					break;
				}
			}
			HKEY hKey;
			if (RegOpenKeyEx(RootFindKey[i], RegPath, 0, samDesired, &hKey) == ERROR_SUCCESS)
			{
				DWORD Index = 0;
				DWORD RetEnum = ERROR_SUCCESS;
				while (RetEnum == ERROR_SUCCESS)
				{
					RetEnum = apiRegEnumKeyEx(hKey, Index++, strName);
					if(RetEnum == ERROR_SUCCESS)
					{
						HKEY hSubKey;
						if (RegOpenKeyEx(hKey, strName.data(), 0, samDesired, &hSubKey) == ERROR_SUCCESS)
						{
							DWORD cbSize = 0;
							if(RegQueryValueEx(hSubKey, L"", nullptr, nullptr, nullptr, &cbSize) == ERROR_SUCCESS)
							{
								if (!StrCmpNI(Module.data(), strName.data(), ModuleLength))
								{
									if(std::find(List.cbegin(), List.cend(), strName) == List.cend())
									{
										List.emplace_back(strName);
									}
									Result=true;
								}
							}
							RegCloseKey(hSubKey);
						}
					}
				}
				RegCloseKey(hKey);
			}
		}

		bool Separator = false;
		for(int i = 0; i != DestMenu->GetItemCount(); ++i)
		{
			if(DestMenu->GetItemPtr(i)->Flags&LIF_SEPARATOR)
			{
				Separator = true;
				break;
			}
		}
		if(!Separator)
		{
			if(DestMenu->GetItemCount())
			{
				MenuItemEx Item={};
				Item.strName = MSG(MCompletionFilesTitle);
				Item.Flags=LIF_SEPARATOR;
				DestMenu->AddItem(&Item);
			}
			else
			{
				DestMenu->SetTitle(MSG(MCompletionFilesTitle));
			}
		}

		std::for_each(CONST_RANGE(List, i)
		{
			DestMenu->AddItem(i);
		});
	}
	return Result;
}

int EditControl::AutoCompleteProc(bool Manual,bool DelBlock,int& BackKey, FARMACROAREA Area)
{
	int Result=0;
	static int Reenter=0;
	string CurrentLine;
	size_t EventsCount = 0;
	Global->Console->GetNumberOfInputEvents(EventsCount);
	if(ECFlags.Check(EC_ENABLEAUTOCOMPLETE) && *Str && !Reenter && !EventsCount && (Global->CtrlObject->Macro.GetCurRecord() == MACROMODE_NOMACRO || Manual))
	{
		Reenter++;

		if(Global->Opt->AutoComplete.AppendCompletion && !Flags.Check(FEDITLINE_CMP_CHANGED))
		{
			CurrentLine = Str;
			DeleteBlock();
		}
		Flags.Clear(FEDITLINE_CMP_CHANGED);

		VMenu2 ComplMenu(string(),nullptr,0,0);
		ComplMenu.SetDialogMode(DMODE_NODRAWSHADOW);
		ComplMenu.SetModeMoving(false);
		string strTemp=Str;

		ComplMenu.SetMacroMode(Area);

#define CMP_ENABLED(c) ((Manual && (c)) || (!Manual && ((c) == 1)))

		if(pHistory && ECFlags.Check(EC_COMPLETE_HISTORY) && CMP_ENABLED(Global->Opt->AutoComplete.UseHistory))
		{
			if(pHistory->GetAllSimilar(ComplMenu,strTemp))
			{
				ComplMenu.SetTitle(MSG(MCompletionHistoryTitle));
			}
		}
		else if(pList)
		{
			for(size_t i=0;i<pList->ItemsNumber;i++)
			{
				if (!StrCmpNI(pList->Items[i].Text, strTemp.data(), static_cast<int>(strTemp.size())) && pList->Items[i].Text != strTemp.data())
				{
					ComplMenu.AddItem(pList->Items[i].Text);
				}
			}
		}
		if(ECFlags.Check(EC_COMPLETE_FILESYSTEM) && CMP_ENABLED(Global->Opt->AutoComplete.UseFilesystem))
		{
			EnumFiles(ComplMenu,strTemp);
		}
		if(ECFlags.Check(EC_COMPLETE_PATH) && CMP_ENABLED(Global->Opt->AutoComplete.UsePath))
		{
			EnumModules(strTemp, &ComplMenu);
		}
		if(ComplMenu.GetItemCount()>1 || (ComplMenu.GetItemCount()==1 && StrCmpI(strTemp.data(),ComplMenu.GetItemPtr(0)->strName.data())))
		{
			ComplMenu.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
			if(!DelBlock && Global->Opt->AutoComplete.AppendCompletion && (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS) || Global->Opt->AutoComplete.ShowList))
			{
				int SelStart=GetLength();

				// magic
				if(IsSlash(Str[SelStart-1]) && Str[SelStart-2] == L'"' && IsSlash(ComplMenu.GetItemPtr(0)->strName[SelStart-2]))
				{
					Str[SelStart-2] = Str[SelStart-1];
					StrSize--;
					SelStart--;
					CurPos--;
				}
				int Offset = 0;
				if(!CurrentLine.empty())
				{
					int Count = ComplMenu.GetItemCount();
					while(Offset < Count && (StrCmpI(ComplMenu.GetItemPtr(Offset)->strName.data(), CurrentLine.data()) || ComplMenu.GetItemPtr(Offset)->Flags&LIF_SEPARATOR))
						++Offset;
					if(Offset < Count)
						++Offset;
					if(Offset < Count && (ComplMenu.GetItemPtr(Offset)->Flags&LIF_SEPARATOR))
						++Offset;
					if(Offset >= Count)
						Offset = 0;
				}
				AppendString(ComplMenu.GetItemPtr(Offset)->strName.data()+SelStart);
				Select(SelStart, GetLength());
				Flags.Clear(FEDITLINE_CMP_CHANGED);
				CurPos = GetLength();
				Show();
			}
			if(Global->Opt->AutoComplete.ShowList)
			{
				MenuItemEx EmptyItem={};
				ComplMenu.AddItem(&EmptyItem,0);
				SetMenuPos(ComplMenu);
				ComplMenu.SetSelectPos(0,0);
				ComplMenu.SetBoxType(SHORT_SINGLE_BOX);
				Show();
				int PrevPos=0;

				bool Visible;
				DWORD Size;
				::GetCursorType(Visible, Size);
				ComplMenu.Key(KEY_NONE);

				int ExitCode=ComplMenu.Run([&](int MenuKey)->int
				{
					::SetCursorType(Visible, Size);

					if(!Global->Opt->AutoComplete.ModalList)
					{
						int CurPos=ComplMenu.GetSelectPos();
						if(CurPos>=0 && PrevPos!=CurPos)
						{
							PrevPos=CurPos;
							SetString(CurPos?ComplMenu.GetItemPtr(CurPos)->strName.data():strTemp.data());
							Show();
						}
					}
					if(MenuKey==KEY_CONSOLE_BUFFER_RESIZE)
						SetMenuPos(ComplMenu);
					else if(MenuKey!=KEY_NONE)
					{
						// ����
						if((MenuKey>=L' ' && MenuKey<=static_cast<int>(WCHAR_MAX)) || MenuKey==KEY_BS || MenuKey==KEY_DEL || MenuKey==KEY_NUMDEL)
						{
							string strPrev;
							DeleteBlock();
							GetString(strPrev);
							ProcessKey(MenuKey);
							GetString(strTemp);
							if(strPrev != strTemp)
							{
								ComplMenu.DeleteItems();
								PrevPos=0;
								if(!strTemp.empty())
								{
									if(pHistory && ECFlags.Check(EC_COMPLETE_HISTORY) && CMP_ENABLED(Global->Opt->AutoComplete.UseHistory))
									{
										if(pHistory->GetAllSimilar(ComplMenu,strTemp))
										{
											ComplMenu.SetTitle(MSG(MCompletionHistoryTitle));
										}
									}
									else if(pList)
									{
										for(size_t i=0;i<pList->ItemsNumber;i++)
										{
											if (!StrCmpNI(pList->Items[i].Text, strTemp.data(), static_cast<int>(strTemp.size())) && pList->Items[i].Text != strTemp.data())
											{
												ComplMenu.AddItem(pList->Items[i].Text);
											}
										}
									}
								}
								if(ECFlags.Check(EC_COMPLETE_FILESYSTEM) && CMP_ENABLED(Global->Opt->AutoComplete.UseFilesystem))
								{
									EnumFiles(ComplMenu,strTemp);
								}
								if(ECFlags.Check(EC_COMPLETE_PATH) && CMP_ENABLED(Global->Opt->AutoComplete.UsePath))
								{
									EnumModules(strTemp, &ComplMenu);
								}
								if(ComplMenu.GetItemCount()>1 || (ComplMenu.GetItemCount()==1 && StrCmpI(strTemp.data(),ComplMenu.GetItemPtr(0)->strName.data())))
								{
									if(MenuKey!=KEY_BS && MenuKey!=KEY_DEL && MenuKey!=KEY_NUMDEL && Global->Opt->AutoComplete.AppendCompletion)
									{
										int SelStart=GetLength();

										// magic
										if(IsSlash(Str[SelStart-1]) && Str[SelStart-2] == L'"' && IsSlash(ComplMenu.GetItemPtr(0)->strName[SelStart-2]))
										{
											Str[SelStart-2] = Str[SelStart-1];
											StrSize--;
											SelStart--;
											CurPos--;
										}

										DisableCallback();
										AppendString(ComplMenu.GetItemPtr(0)->strName.data()+SelStart);
										if(X2-X1>GetLength())
											SetLeftPos(0);
										this->Select(SelStart, GetLength());
										RevertCallback();
									}
									ComplMenu.AddItem(&EmptyItem,0);
									SetMenuPos(ComplMenu);
									ComplMenu.SetSelectPos(0,0);
								}
								else
								{
									ComplMenu.Close(-1);
								}
								Show();
							}
							return 1;
						}
						else
						{
							switch(MenuKey)
							{
							// "������������" �������
							case KEY_CTRLEND:
							case KEY_RCTRLEND:

							case KEY_CTRLSPACE:
							case KEY_RCTRLSPACE:
								{
									ComplMenu.Key(KEY_DOWN);
									return 1;
								}

							case KEY_SHIFTDEL:
							case KEY_SHIFTNUMDEL:
								{
									if(ComplMenu.GetItemCount()>1)
									{
										unsigned __int64* CurrentRecord = static_cast<unsigned __int64*>(ComplMenu.GetUserData(nullptr, 0));
										if(CurrentRecord && pHistory->DeleteIfUnlocked(*CurrentRecord))
										{
											ComplMenu.DeleteItem(ComplMenu.GetSelectPos());
											if(ComplMenu.GetItemCount()>1)
											{
												SetMenuPos(ComplMenu);
												Show();
											}
											else
											{
												ComplMenu.Close(-1);
											}
										}
									}
								}
								break;

							// ��������� �� ������ �����
							case KEY_LEFT:
							case KEY_NUMPAD4:
							case KEY_CTRLS:     case KEY_RCTRLS:
							case KEY_RIGHT:
							case KEY_NUMPAD6:
							case KEY_CTRLD:     case KEY_RCTRLD:
							case KEY_CTRLLEFT:  case KEY_RCTRLLEFT:
							case KEY_CTRLRIGHT: case KEY_RCTRLRIGHT:
							case KEY_CTRLHOME:  case KEY_RCTRLHOME:
								{
									if(MenuKey == KEY_LEFT || MenuKey == KEY_NUMPAD4)
									{
										MenuKey = KEY_CTRLS;
									}
									else if(MenuKey == KEY_RIGHT || MenuKey == KEY_NUMPAD6)
									{
										MenuKey = KEY_CTRLD;
									}
									pOwner->ProcessKey(MenuKey);
									Show();
									return 1;
								}

							// ��������� �� ������
							case KEY_SHIFT:
							case KEY_ALT:
							case KEY_RALT:
							case KEY_CTRL:
							case KEY_RCTRL:
							case KEY_HOME:
							case KEY_NUMPAD7:
							case KEY_END:
							case KEY_NUMPAD1:
							case KEY_IDLE:
							case KEY_NONE:
							case KEY_ESC:
							case KEY_F10:
							case KEY_ALTF9:
							case KEY_RALTF9:
							case KEY_UP:
							case KEY_NUMPAD8:
							case KEY_DOWN:
							case KEY_NUMPAD2:
							case KEY_PGUP:
							case KEY_NUMPAD9:
							case KEY_PGDN:
							case KEY_NUMPAD3:
							case KEY_ALTLEFT:
							case KEY_ALTRIGHT:
							case KEY_ALTHOME:
							case KEY_ALTEND:
							case KEY_RALTLEFT:
							case KEY_RALTRIGHT:
							case KEY_RALTHOME:
							case KEY_RALTEND:
							case KEY_MSWHEEL_UP:
							case KEY_MSWHEEL_DOWN:
							case KEY_MSWHEEL_LEFT:
							case KEY_MSWHEEL_RIGHT:
								{
									break;
								}

							case KEY_MSLCLICK:
								MenuKey = KEY_ENTER;
							case KEY_ENTER:
							case KEY_NUMENTER:
								{
									if(Global->Opt->AutoComplete.ModalList)
										break;
								}

							// �� ��������� ��������� ������ � ��� ���������
							default:
								{
									ComplMenu.Close(-1);
									BackKey=MenuKey;
									Result=1;
								}
							}
						}
					}
					return 0;
				});
				// mouse click
				if(ExitCode>0)
				{
					if(Global->Opt->AutoComplete.ModalList)
					{
						SetString(ComplMenu.GetItemPtr(ExitCode)->strName.data());
						Show();
					}
					else
					{
						BackKey = KEY_ENTER;
						Result=1;
					}
				}
			}
		}

		Reenter--;
	}
	return Result;
}

void EditControl::AutoComplete(bool Manual,bool DelBlock)
{
	int Key=0;
	FARMACROAREA PrevMacroMode=Global->CtrlObject->Macro.GetMode();
	if(Global->Opt->AutoComplete.ShowList)
		Global->CtrlObject->Macro.SetMode(MacroAreaAC);
	if(AutoCompleteProc(Manual,DelBlock,Key,MacroAreaAC))
	{
		// BUGBUG, hack
		int Wait=Global->WaitInMainLoop;
		Global->WaitInMainLoop=1;
		struct FAR_INPUT_RECORD irec={(DWORD)Key,*FrameManager->GetLastInputRecord()};
		if(!Global->CtrlObject->Macro.ProcessEvent(&irec))
			pOwner->ProcessKey(Key);
		Global->WaitInMainLoop=Wait;
		int CurWindowType = FrameManager->GetCurrentFrame()->GetType();
		if (CurWindowType == MODALTYPE_DIALOG || CurWindowType == MODALTYPE_PANELS)
		{
			Show();
		}
	}
	Global->CtrlObject->Macro.SetMode(PrevMacroMode);
}

int EditControl::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if(Edit::ProcessMouse(MouseEvent))
	{
		while(IsMouseButtonPressed()==FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			Flags.Clear(FEDITLINE_CLEARFLAG);
			SetTabCurPos(IntKeyState.MouseX - X1 + GetLeftPos());
			if(IntKeyState.MouseEventFlags&MOUSE_MOVED)
			{
				if(!Selection)
				{
					Selection=true;
					SelectionStart=-1;
					Select(SelectionStart,0);
				}
				else
				{
					if(SelectionStart==-1)
					{
						SelectionStart=CurPos;
					}
					Select(std::min(SelectionStart,CurPos),std::min(StrSize,std::max(SelectionStart,CurPos)));
					Show();
				}
			}
		}
		Selection=false;
		return TRUE;
	}
	return FALSE;
}

void EditControl::SetObjectColor(PaletteColors Color,PaletteColors SelColor,PaletteColors ColorUnChanged)
{
	this->Color=ColorIndexToColor(Color);
	this->SelColor=ColorIndexToColor(SelColor);
	this->ColorUnChanged=ColorIndexToColor(ColorUnChanged);
}

void EditControl::SetObjectColor(const FarColor& Color,const FarColor& SelColor, const FarColor& ColorUnChanged)
{
	this->Color=Color;
	this->SelColor=SelColor;
	this->ColorUnChanged=ColorUnChanged;
}

void EditControl::GetObjectColor(FarColor& Color, FarColor& SelColor, FarColor& ColorUnChanged)
{
	Color = this->Color;
	SelColor = this->SelColor;
	ColorUnChanged = this->ColorUnChanged;
}

const FarColor& EditControl::GetNormalColor() const
{
	return Color;
}

const FarColor& EditControl::GetSelectedColor() const
{
	return SelColor;
}

const FarColor& EditControl::GetUnchangedColor() const
{
	return ColorUnChanged;
}

const int EditControl::GetTabSize() const
{
	return Global->Opt->EdOpt.TabSize;
}

const EXPAND_TABS EditControl::GetTabExpandMode() const
{
	return EXPAND_NOTABS;
}

const void EditControl::SetInputMask(const string& InputMask)
{
	Mask = InputMask;
	if (!Mask.empty())
	{
		RefreshStrByMask(TRUE);
	}
}

// ������� ���������� ��������� ������ ����� �� ����������� Mask
void EditControl::RefreshStrByMask(int InitMode)
{
	auto Mask = GetInputMask();
	if (!Mask.empty())
	{
		int MaskLen = static_cast<int>(Mask.size());

		if (StrSize!=MaskLen)
		{
			wchar_t *NewStr=(wchar_t *)xf_realloc(Str,(MaskLen+1)*sizeof(wchar_t));

			if (!NewStr)
				return;

			Str=NewStr;

			for (int i=StrSize; i<MaskLen; i++)
				Str[i]=L' ';

			StrSize=MaxLength=MaskLen;
			Str[StrSize]=0;
		}

		for (int i=0; i<MaskLen; i++)
		{
			if (InitMode)
				Str[i]=L' ';

			if (!CheckCharMask(Mask[i]))
				Str[i]=Mask[i];
		}
	}
}

const string& EditControl::WordDiv() const
{
	return Global->Opt->strWordDiv;
}
