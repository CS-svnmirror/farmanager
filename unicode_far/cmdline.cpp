/*
cmdline.cpp

��������� ������
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

#include "cmdline.hpp"
#include "execute.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "history.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "foldtree.hpp"
#include "treelist.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "rdrwdsk.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"
#include "interf.hpp"
#include "syslog.hpp"
#include "config.hpp"
#include "usermenu.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "keyboard.hpp"
#include "mix.hpp"
#include "console.hpp"
#include "panelmix.hpp"
#include "message.hpp"
#include "network.hpp"
#include "plugins.hpp"
#include "colormix.hpp"

enum
{
	FCMDOBJ_LOCKUPDATEPANEL   = 0x00010000,
	DEFAULT_CMDLINE_WIDTH = 50,
};

CommandLine::CommandLine():
	PromptSize(DEFAULT_CMDLINE_WIDTH),
	CmdStr(Global->CtrlObject->Cp(),0,true,Global->CtrlObject->CmdHistory,0,(Global->Opt->CmdLine.AutoComplete?EditControl::EC_ENABLEAUTOCOMPLETE:0)|EditControl::EC_COMPLETE_HISTORY|EditControl::EC_COMPLETE_FILESYSTEM|EditControl::EC_COMPLETE_PATH),
	BackgroundScreen(nullptr),
	LastCmdPartLength(-1)
{
	CmdStr.SetEditBeyondEnd(FALSE);
	CmdStr.SetMacroAreaAC(MACROAREA_SHELLAUTOCOMPLETION);
	SetPersistentBlocks(Global->Opt->CmdLine.EditBlock);
	SetDelRemovesBlocks(Global->Opt->CmdLine.DelRemovesBlocks);
}

CommandLine::~CommandLine()
{
	if (BackgroundScreen)
		delete BackgroundScreen;

	Global->ScrBuf->Flush(true);
}

void CommandLine::SetPersistentBlocks(bool Mode)
{
	CmdStr.SetPersistentBlocks(Mode);
}

void CommandLine::SetDelRemovesBlocks(bool Mode)
{
	CmdStr.SetDelRemovesBlocks(Mode);
}

void CommandLine::SetAutoComplete(int Mode)
{
	CmdStr.SetAutocomplete(Mode!=0);
}

void CommandLine::DisplayObject()
{
	_OT(SysLog(L"[%p] CommandLine::DisplayObject()",this));
	string strTruncDir;
	auto PromptList = GetPrompt();
	size_t MaxLength = PromptSize*ObjWidth()/100;
	size_t CurLength = 0;
	GotoXY(X1,Y1);

	std::for_each(CONST_RANGE(PromptList, i)
	{
		SetColor(i.second);
		string str(i.first);
		if(CurLength + str.size() > MaxLength)
		TruncPathStr(str, std::max(0, static_cast<int>(MaxLength - CurLength)));
		Text(str);
		CurLength += str.size();
	});

	CmdStr.SetObjectColor(COL_COMMANDLINE,COL_COMMANDLINESELECTED);

	CmdStr.SetPosition(X1+(int)CurLength,Y1,X2,Y2);

	CmdStr.Show();

	GotoXY(X2+1,Y1);
	SetColor(COL_COMMANDLINEPREFIX);
	Text(L"\x2191");
}


void CommandLine::SetCurPos(int Pos, int LeftPos, bool Redraw)
{
	CmdStr.SetLeftPos(LeftPos);
	CmdStr.SetCurPos(Pos);
	if (Redraw)
		CmdStr.Redraw();
}

__int64 CommandLine::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (OpCode >= MCODE_C_CMDLINE_BOF && OpCode <= MCODE_C_CMDLINE_SELECTED)
		return CmdStr.VMProcess(OpCode-MCODE_C_CMDLINE_BOF+MCODE_C_BOF,vParam,iParam);

	if (OpCode >= MCODE_C_BOF && OpCode <= MCODE_C_SELECTED)
		return CmdStr.VMProcess(OpCode,vParam,iParam);

	if (OpCode == MCODE_V_ITEMCOUNT || OpCode == MCODE_V_CURPOS)
		return CmdStr.VMProcess(OpCode,vParam,iParam);

	if (OpCode == MCODE_V_CMDLINE_ITEMCOUNT || OpCode == MCODE_V_CMDLINE_CURPOS)
		return CmdStr.VMProcess(OpCode-MCODE_V_CMDLINE_ITEMCOUNT+MCODE_V_ITEMCOUNT,vParam,iParam);

	if (OpCode == MCODE_F_EDITOR_SEL)
		return CmdStr.VMProcess(MCODE_F_EDITOR_SEL,vParam,iParam);

	return 0;
}

int CommandLine::ProcessKey(int Key)
{
	const wchar_t *PStr;
	string strStr;

	if ((Key==KEY_CTRLEND || Key==KEY_RCTRLEND || Key==KEY_CTRLNUMPAD1 || Key==KEY_RCTRLNUMPAD1) && (CmdStr.GetCurPos()==CmdStr.GetLength()))
	{
		if (LastCmdPartLength==-1)
			strLastCmdStr = CmdStr.GetStringAddr();

		strStr = strLastCmdStr;
		int CurCmdPartLength=(int)strStr.size();
		Global->CtrlObject->CmdHistory->GetSimilar(strStr,LastCmdPartLength);

		if (LastCmdPartLength==-1)
		{
			strLastCmdStr = CmdStr.GetStringAddr();
			LastCmdPartLength=CurCmdPartLength;
		}

		{
			SetAutocomplete disable(&CmdStr);
			CmdStr.SetString(strStr.data());
			CmdStr.Select(LastCmdPartLength,static_cast<int>(strStr.size()));
		}

		Show();
		return TRUE;
	}

	if (Key == KEY_UP || Key == KEY_NUMPAD8)
	{
		if (Global->CtrlObject->Cp()->LeftPanel->IsVisible() || Global->CtrlObject->Cp()->RightPanel->IsVisible())
			return FALSE;

		Key=KEY_CTRLE;
	}
	else if (Key == KEY_DOWN || Key == KEY_NUMPAD2)
	{
		if (Global->CtrlObject->Cp()->LeftPanel->IsVisible() || Global->CtrlObject->Cp()->RightPanel->IsVisible())
			return FALSE;

		Key=KEY_CTRLX;
	}

	// $ 25.03.2002 VVM + ��� ���������� ������� ������� ������ �������
	if (!Global->CtrlObject->Cp()->LeftPanel->IsVisible() && !Global->CtrlObject->Cp()->RightPanel->IsVisible())
	{
		switch (Key)
		{
			case KEY_MSWHEEL_UP:    Key = KEY_CTRLE; break;
			case KEY_MSWHEEL_DOWN:  Key = KEY_CTRLX; break;
			case KEY_MSWHEEL_LEFT:  Key = KEY_CTRLS; break;
			case KEY_MSWHEEL_RIGHT: Key = KEY_CTRLD; break;
		}
	}

	switch (Key)
	{
		case KEY_CTRLE:
		case KEY_RCTRLE:
		case KEY_CTRLX:
		case KEY_RCTRLX:
			{
				if (Key == KEY_CTRLE || Key == KEY_RCTRLE)
				{
					Global->CtrlObject->CmdHistory->GetPrev(strStr);
				}
				else
				{
					Global->CtrlObject->CmdHistory->GetNext(strStr);
				}

				{
					SetAutocomplete disable(&CmdStr);
					SetString(strStr);
				}

			}
			return TRUE;

		case KEY_ESC:

			if (Key == KEY_ESC)
			{
				// $ 24.09.2000 SVS - ���� ������ ��������� �� "������������ ��� Esc", �� ������� � ������� �� ������ � ������ � ������ ���������.
				if (Global->Opt->CmdHistoryRule)
					Global->CtrlObject->CmdHistory->ResetPosition();

				PStr=L"";
			}
			else
				PStr=strStr.data();

			SetString(PStr);
			return TRUE;
		case KEY_F2:
		{
			UserMenu Menu(false);
			return TRUE;
		}
		case KEY_ALTF8:
		case KEY_RALTF8:
		{
			int Type;
			// $ 19.09.2000 SVS - ��� ������ �� History (�� Alt-F8) ������ �� ������� ����������!
			int SelectType=Global->CtrlObject->CmdHistory->Select(MSG(MHistoryTitle),L"History",strStr,Type);
			// BUGBUG, magic numbers
			if ((SelectType > 0 && SelectType <= 3) || SelectType == 7)
			{
				std::unique_ptr<SetAutocomplete> disable;
				if(SelectType<3 || SelectType == 7)
				{
					disable.reset(new DECLTYPE(disable)::element_type(&CmdStr));
				}
				SetString(strStr);

				if (SelectType < 3 || SelectType == 7)
				{
					ProcessKey(SelectType==7?static_cast<int>(KEY_CTRLALTENTER):(SelectType==1?static_cast<int>(KEY_ENTER):static_cast<int>(KEY_SHIFTENTER)));
				}
			}
		}
		return TRUE;
		case KEY_SHIFTF9:
			Global->Opt->Save(true);
			return TRUE;
		case KEY_F10:
			FrameManager->ExitMainLoop(TRUE);
			return TRUE;
		case KEY_ALTF10:
		case KEY_RALTF10:
		{
			Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
			{
				// TODO: ����� ����� �������� ��������, ��� �� � ����� ����� � ���������� ����� Tree.Far...
				FolderTree Tree(strStr,MODALTREE_ACTIVE,TRUE,FALSE);
			}
			Global->CtrlObject->Cp()->RedrawKeyBar();

			if (!strStr.empty())
			{
				ActivePanel->SetCurDir(strStr,true);
				ActivePanel->Show();

				if (ActivePanel->GetType()==TREE_PANEL)
					ActivePanel->ProcessKey(KEY_ENTER);
			}
			else
			{
				// TODO: ... � ����� ��������� ���� ���������/��������� ����� Tree.Far � �� ����� �� � ����� (����� ������ ��� �� ��������� ������)
				ActivePanel->Update(UPDATE_KEEP_SELECTION);
				ActivePanel->Redraw();
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

				if (AnotherPanel->NeedUpdatePanel(ActivePanel))
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);//|UPDATE_SECONDARY);
					AnotherPanel->Redraw();
				}
			}
		}
		return TRUE;
		case KEY_F11:
			Global->CtrlObject->Plugins->CommandsMenu(FALSE,FALSE,0);
			return TRUE;
		case KEY_ALTF11:
		case KEY_RALTF11:
			ShowViewEditHistory();
			Global->CtrlObject->Cp()->Redraw();
			return TRUE;
		case KEY_ALTF12:
		case KEY_RALTF12:
		{
			int Type;
			GUID Guid; string strFile, strData;
			int SelectType=Global->CtrlObject->FolderHistory->Select(MSG(MFolderHistoryTitle),L"HistoryFolders",strStr,Type,&Guid,&strFile,&strData);

			/*
			   SelectType = 0 - Esc
			                1 - Enter
			                2 - Shift-Enter
			                3 - Ctrl-Enter
			                6 - Ctrl-Shift-Enter - �� ��������� ������ �� ������ �������
			*/
			if (SelectType == 1 || SelectType == 2 || SelectType == 6)
			{
				if (SelectType==2)
					Global->CtrlObject->FolderHistory->SetAddMode(false,2,true);

				// ����� ������ ��� �������... ;-)
				Panel *Panel=Global->CtrlObject->Cp()->ActivePanel;

				if (SelectType == 6)
					Panel=Global->CtrlObject->Cp()->GetAnotherPanel(Panel);

				//Type==1 - ���������� ����
				//Type==0 - ������� ����
				Panel->ExecShortcutFolder(strStr,Guid,strFile,strData,true);
				// Panel may be changed
				if(SelectType == 6)
				{
					Panel=Global->CtrlObject->Cp()->ActivePanel;
					Panel->SetCurPath();
					Panel=Global->CtrlObject->Cp()->GetAnotherPanel(Panel);
				}
				else
				{
					Panel=Global->CtrlObject->Cp()->ActivePanel;
				}
				Panel->Redraw();
				Global->CtrlObject->FolderHistory->SetAddMode(true,2,true);
			}
			else if (SelectType==3)
				SetString(strStr);
		}
		return TRUE;
		case KEY_NUMENTER:
		case KEY_SHIFTNUMENTER:
		case KEY_ENTER:
		case KEY_SHIFTENTER:
		case KEY_CTRLALTENTER:
		case KEY_RCTRLRALTENTER:
		case KEY_CTRLRALTENTER:
		case KEY_RCTRLALTENTER:
		case KEY_CTRLALTNUMENTER:
		case KEY_RCTRLRALTNUMENTER:
		case KEY_CTRLRALTNUMENTER:
		case KEY_RCTRLALTNUMENTER:
		{
			Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
			CmdStr.Select(-1,0);
			CmdStr.Show();
			CmdStr.GetString(strStr);

			if (strStr.empty())
				break;

			ActivePanel->SetCurPath();

			if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTCMDLINE))
				Global->CtrlObject->CmdHistory->AddToHistory(strStr);

			if (!ActivePanel->ProcessPluginEvent(FE_COMMAND,(void *)strStr.data()))
				ExecString(strStr, false, Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER, false, false,
						Key == KEY_CTRLALTENTER || Key == KEY_RCTRLRALTENTER || Key == KEY_CTRLRALTENTER || Key == KEY_RCTRLALTENTER ||
						Key == KEY_CTRLALTNUMENTER || Key == KEY_RCTRLRALTNUMENTER || Key == KEY_CTRLRALTNUMENTER || Key == KEY_RCTRLALTNUMENTER);
		}
		return TRUE;
		case KEY_CTRLU:
		case KEY_RCTRLU:
			CmdStr.Select(-1,0);
			CmdStr.Show();
			return TRUE;
		case KEY_OP_XLAT:
		{
			// 13.12.2000 SVS - ! ��� CmdLine - ���� ��� ���������, ����������� ��� ������ (XLat)
			CmdStr.Xlat((Global->Opt->XLat.Flags&XLAT_CONVERTALLCMDLINE) != 0);

			// ����� ����������� �������� ctrl-end
			strLastCmdStr = CmdStr.GetStringAddr();
			LastCmdPartLength=(int)strLastCmdStr.size();

			return TRUE;
		}
		/* �������������� ������� ��� ��������� � ��� ������.
		   ��������!
		   ��� ���������� ���� ���� ����� ������ ������ ����� "default"
		*/
		case KEY_ALTSHIFTLEFT:   case KEY_ALTSHIFTNUMPAD4:
		case KEY_RALTSHIFTLEFT:  case KEY_RALTSHIFTNUMPAD4:
		case KEY_ALTSHIFTRIGHT:  case KEY_ALTSHIFTNUMPAD6:
		case KEY_RALTSHIFTRIGHT: case KEY_RALTSHIFTNUMPAD6:
		case KEY_ALTSHIFTEND:    case KEY_ALTSHIFTNUMPAD1:
		case KEY_RALTSHIFTEND:   case KEY_RALTSHIFTNUMPAD1:
		case KEY_ALTSHIFTHOME:   case KEY_ALTSHIFTNUMPAD7:
		case KEY_RALTSHIFTHOME:  case KEY_RALTSHIFTNUMPAD7:
			Key&=~(KEY_ALT|KEY_RALT);
		default:

			//   ���������� ��������� �� ��������� ��������
			if (!Global->Opt->CmdLine.EditBlock)
			{
				static int UnmarkKeys[]=
				{
					KEY_LEFT,       KEY_NUMPAD4,
					KEY_CTRLS,      KEY_RCTRLS,
					KEY_RIGHT,      KEY_NUMPAD6,
					KEY_CTRLD,      KEY_RCTRLD,
					KEY_CTRLLEFT,   KEY_CTRLNUMPAD4,
					KEY_RCTRLLEFT,  KEY_RCTRLNUMPAD4,
					KEY_CTRLRIGHT,  KEY_CTRLNUMPAD6,
					KEY_RCTRLRIGHT, KEY_RCTRLNUMPAD6,
					KEY_CTRLHOME,   KEY_CTRLNUMPAD7,
					KEY_RCTRLHOME,  KEY_RCTRLNUMPAD7,
					KEY_CTRLEND,    KEY_CTRLNUMPAD1,
					KEY_RCTRLEND,   KEY_RCTRLNUMPAD1,
					KEY_HOME,       KEY_NUMPAD7,
					KEY_END,        KEY_NUMPAD1
				};

				if (std::find(ALL_CONST_RANGE(UnmarkKeys), Key) != std::cend(UnmarkKeys))
				{
					CmdStr.Select(-1,0);
				}
			}

			if (Key == KEY_CTRLD || Key == KEY_RCTRLD)
				Key=KEY_RIGHT;

			if(Key == KEY_CTRLSPACE || Key == KEY_RCTRLSPACE)
			{
				SetAutocomplete enable(&CmdStr, true);
				CmdStr.AutoComplete(true,false);
				return TRUE;
			}

			if (!CmdStr.ProcessKey(Key))
				break;

			LastCmdPartLength=-1;

			return TRUE;
	}

	return FALSE;
}


void CommandLine::SetCurDir(const string& CurDir)
{
	if (StrCmpI(strCurDir.data(),CurDir.data()) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		//Mantis#2350 - ������, ��� � ��� �������� ����
		//if (Global->CtrlObject->Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
			//PrepareDiskPath(strCurDir);
	}
}


int CommandLine::GetCurDir(string &strCurDir)
{
	strCurDir = CommandLine::strCurDir;
	return (int)strCurDir.size();
}


void CommandLine::SetString(const string& Str, bool Redraw)
{
	LastCmdPartLength=-1;
	CmdStr.SetString(Str.data());
	CmdStr.SetLeftPos(0);

	if (Redraw)
		CmdStr.Show();
}

void CommandLine::InsertString(const string& Str)
{
	LastCmdPartLength=-1;
	CmdStr.InsertString(Str);
	CmdStr.Show();
}


int CommandLine::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if(MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED && MouseEvent->dwMousePosition.X==X2+1)
	{
		return ProcessKey(KEY_ALTF8);
	}
	return CmdStr.ProcessMouse(MouseEvent);
}

inline COLORREF ARGB2ABGR(COLORREF Color)
{
	return (Color & 0xFF000000) | ((Color & 0x00FF0000) >> 16) | (Color & 0x0000FF00) | ((Color & 0x000000FF) << 16);
}

inline void AssignColor(const string& Color, COLORREF& Target, FARCOLORFLAGS& TargetFlags, FARCOLORFLAGS SetFlag)
{
	if (!Color.empty())
	{
		if (Upper(Color[0]) == L'T')
		{
			TargetFlags &= ~SetFlag;
			Target = ARGB2ABGR(wcstoul(Color.data() + 1, nullptr, 16));
		}
		else
		{
			TargetFlags |= SetFlag;
			Target = wcstoul(Color.data(), nullptr, 16);
		}
	}
}

std::list<std::pair<string, FarColor>> CommandLine::GetPrompt()
{
	std::list<std::pair<string, FarColor>> Result;
	int NewPromptSize = DEFAULT_CMDLINE_WIDTH;

	string strDestStr;
	FarColor PrefixColor(ColorIndexToColor(COL_COMMANDLINEPREFIX));

	if (Global->Opt->CmdLine.UsePromptFormat)
	{
		string Format(Global->Opt->CmdLine.strPromptFormat.Get());
		FarColor F(PrefixColor);
		string Str(Format);
		FOR_CONST_RANGE(Format, Ptr)
		{
			// color in ([[T]ffffffff][:[T]bbbbbbbb]) format
			if (*Ptr == L'(')
			{
				string Color = &*Ptr + 1;
				size_t Pos = Color.find(L')');
				if(Pos != string::npos)
				{
					if (Ptr != Format.cbegin())
					{
						size_t PrevPos = Str.find(L'(');
						Str.resize(PrevPos);
						Result.emplace_back(VALUE_TYPE(Result)(Str, F));
					}
					Ptr += Pos+2;
					Color.resize(Pos);
					string BgColor;
					Pos = Color.find(L':');
					if (Pos != string::npos)
					{
						BgColor = Color.substr(Pos+1);
						Color.resize(Pos);
					}

					F = PrefixColor;

					AssignColor(Color, F.ForegroundColor, F.Flags, FCF_FG_4BIT);
					AssignColor(BgColor, F.BackgroundColor, F.Flags, FCF_BG_4BIT);

					Str = &*Ptr;
				}
			}
		}
		Result.emplace_back(VALUE_TYPE(Result)(Str, F));

		std::for_each(RANGE(Result, i)
		{
			string& strDestStr = i.first;
			string strExpandedDestStr;
			apiExpandEnvironmentStrings(strDestStr, strExpandedDestStr);
			strDestStr.clear();
			static const simple_pair<wchar_t, wchar_t> ChrFmt[] =
			{
				{L'A', L'&'},   // $A - & (Ampersand)
				{L'B', L'|'},   // $B - | (pipe)
				{L'C', L'('},   // $C - ( (Left parenthesis)
				{L'F', L')'},   // $F - ) (Right parenthesis)
				{L'G', L'>'},   // $G - > (greater-than sign)
				{L'L', L'<'},   // $L - < (less-than sign)
				{L'Q', L'='},   // $Q - = (equal sign)
				{L'S', L' '},   // $S - (space)
				{L'$', L'$'},   // $$ - $ (dollar sign)
			};

			FOR_CONST_RANGE(strExpandedDestStr, i)
			{
				if (*i==L'$')
				{
					wchar_t Chr=Upper(*++i);

					auto ItemIterator = std::find_if(CONST_RANGE(ChrFmt, i)
					{
						return i.first == Chr;
					});

					if (ItemIterator != std::cend(ChrFmt))
					{
						strDestStr += ItemIterator->second;
					}
					else
					{
						switch (Chr)
						{
								/* ��� �� �����������
								$E - Escape code (ASCII code 27)
								$V - Windows XP version number
								$_ - Carriage return and linefeed
								*/
							case L'M': // $M - ����������� ������� ����� ���������� �����, ���������� � ������ �������� �����, ��� ������ ������, ���� ������� ���� �� �������� �������.
							{
								string strTemp;
								if (DriveLocalToRemoteName(DRIVE_REMOTE,strCurDir[0],strTemp))
								{
									strDestStr += strTemp;
									//strDestStr += L" "; // ???
								}
								break;
							}
							case L'+': // $+  - ����������� ������� ����� ������ ���� (+) � ����������� �� ������� ������� ����� ��������� PUSHD, �� ������ ����� �� ������ ����������� ����.
							{
								strDestStr.append(ppstack.size(), L'+');
								break;
							}
							case L'H': // $H - Backspace (erases previous character)
							{
								if (!strDestStr.empty())
									strDestStr.pop_back();

								break;
							}
							case L'@': // $@xx - Admin
							{
								wchar_t lb=*++i;
								wchar_t rb=*++i;
								if ( Global->IsUserAdmin() )
								{
									strDestStr += lb;
									strDestStr += MSG(MConfigCmdlinePromptFormatAdmin);
									strDestStr += rb;
								}
								break;
							}
							case L'D': // $D - Current date
							case L'T': // $T - Current time
							{
								string strDateTime;
								MkStrFTime(strDateTime,(Chr==L'D'?L"%D":L"%T"));
								strDestStr += strDateTime;
								break;
							}
							case L'N': // $N - Current drive
							{
								PATH_TYPE Type = ParsePath(strCurDir);
								if(Type == PATH_DRIVELETTER)
									strDestStr += Upper(strCurDir[0]);
								else if(Type == PATH_DRIVELETTERUNC)
									strDestStr += Upper(strCurDir[4]);
								else
									strDestStr += L'?';
								break;
							}
							case L'W': // $W - ������� ������� ������� (��� �������� ����)
							{
								const wchar_t *ptrCurDir=LastSlash(strCurDir.data());
								if (ptrCurDir)
									strDestStr += ptrCurDir+1;
								break;
							}
							case L'P': // $P - Current drive and path
							{
								strDestStr += strCurDir;
								break;
							}
							case L'#': //$#nn - max promt width in %
							{
								NewPromptSize = 0;
								for (int j=0; j<2 && iswdigit(*(i+1)); j++)
								{
									NewPromptSize *= 10;
									NewPromptSize += *++i - L'0';
								}
							}
						}
					}
				}
				else
				{
					strDestStr += *i;
				}
			}
		});

	}
	else
	{
		// default prompt = "$p$g"
		Result.emplace_back(VALUE_TYPE(Result)(strCurDir + L">", PrefixColor));
	}
	SetPromptSize(NewPromptSize);
	return Result;
}


void CommandLine::ShowViewEditHistory()
{
	string strStr;
	int Type;
	int SelectType=Global->CtrlObject->ViewHistory->Select(MSG(MViewHistoryTitle),L"HistoryViews",strStr,Type);
	/*
	   SelectType = 0 - Esc
	                1 - Enter
	                2 - Shift-Enter
	                3 - Ctrl-Enter
	*/

	if (SelectType == 1 || SelectType == 2)
	{
		if (SelectType!=2)
			Global->CtrlObject->ViewHistory->AddToHistory(strStr,Type);

		Global->CtrlObject->ViewHistory->SetAddMode(false,Global->Opt->FlagPosixSemantics?1:2,true);

		switch (Type)
		{
			case 0: // ������
			{
				new FileViewer(strStr,TRUE);
				break;
			}
			case 1: // ������� �������� � ���������
			case 4: // �������� � �����
			{
				// ����� ���� ���������
				FileEditor *FEdit=new FileEditor(strStr,CP_DEFAULT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);

				if (Type == 4)
					FEdit->SetLockEditor(TRUE);

				break;
			}
			// 2 � 3 - ����������� � ProcessExternal
			case 2:
			case 3:
			{
				ExecString(strStr, Type>2, false, false, false, false, true);
				break;
			}
		}

		Global->CtrlObject->ViewHistory->SetAddMode(true,Global->Opt->FlagPosixSemantics?1:2,true);
	}
	else if (SelectType==3) // ������� �� ������� � ���.������?
		SetString(strStr);
}

void CommandLine::SaveBackground(int X1,int Y1,int X2,int Y2)
{
	if (BackgroundScreen)
	{
		delete BackgroundScreen;
	}

	BackgroundScreen=new SaveScreen(X1,Y1,X2,Y2);
}

void CommandLine::SaveBackground()
{
	if (BackgroundScreen)
	{
//		BackgroundScreen->Discard();
		BackgroundScreen->SaveArea();
	}
}
void CommandLine::ShowBackground()
{
	if (BackgroundScreen)
	{
		BackgroundScreen->RestoreArea();
	}
}

void CommandLine::CorrectRealScreenCoord()
{
	if (BackgroundScreen)
	{
		BackgroundScreen->CorrectRealScreenCoord();
	}
}

void CommandLine::ResizeConsole()
{
	BackgroundScreen->Resize(ScrX+1,ScrY+1,2,Global->Opt->WindowMode!=FALSE);
//  this->DisplayObject();
}

void CommandLine::SetPromptSize(int NewSize)
{
	PromptSize = NewSize? std::max(5, std::min(95, NewSize)) : DEFAULT_CMDLINE_WIDTH;
}

int CommandLine::ExecString(const string& InputCmdLine, bool AlwaysWaitFinish, bool SeparateWindow, bool DirectRun, bool WaitForIdle, bool RunAs, bool RestoreCmd)
{
	class preservecmdline
	{
	public:
		preservecmdline(bool Preserve):
			Preserve(Preserve)
		{
			if(Preserve)
			{
				OldCmdLineCurPos = Global->CtrlObject->CmdLine->GetCurPos();
				OldCmdLineLeftPos = Global->CtrlObject->CmdLine->GetLeftPos();
				Global->CtrlObject->CmdLine->GetString(strOldCmdLine);
				Global->CtrlObject->CmdLine->GetSelection(OldCmdLineSelStart,OldCmdLineSelEnd);
			}
		}
		~preservecmdline()
		{
			if(Preserve)
			{
				bool redraw = FrameManager->IsPanelsActive();
				Global->CtrlObject->CmdLine->SetString(strOldCmdLine, redraw);
				Global->CtrlObject->CmdLine->SetCurPos(OldCmdLineCurPos, OldCmdLineLeftPos, redraw);
				Global->CtrlObject->CmdLine->Select(OldCmdLineSelStart, OldCmdLineSelEnd);
			}
		}
	private:
		bool Preserve;
		string strOldCmdLine;
		int OldCmdLineCurPos, OldCmdLineLeftPos;
		intptr_t OldCmdLineSelStart, OldCmdLineSelEnd;
	}
	PreserveCmdline(RestoreCmd);

	string CmdLine(InputCmdLine);

	bool Silent=false;

	if (CmdLine[0] == L'@')
	{
		CmdLine.erase(0, 1);
		Silent=true;
	}

	{
		SetAutocomplete disable(&CmdStr);
		SetString(CmdLine);
	}

	ProcessOSAliases(CmdLine);

	LastCmdPartLength=-1;

	if(!StrCmpI(CmdLine.data(),L"far:config"))
	{
		SetString(L"", false);
		Show();
		return Global->Opt->AdvancedConfig();
	}

	if (!SeparateWindow && Global->CtrlObject->Plugins->ProcessCommandLine(CmdLine))
	{
		/* $ 12.05.2001 DJ - �������� ������ ���� �������� ������� ������� */
		if (Global->CtrlObject->Cp()->IsTopFrame())
		{
			//CmdStr.SetString(L"");
			GotoXY(X1,Y1);
			Global->FS << fmt::MinWidth(X2-X1+1)<<L"";
			Show();
			Global->ScrBuf->Flush();
		}

		return -1;
	}

	int Code;
	COORD Size0;
	Global->Console->GetSize(Size0);

	if (strCurDir.size() > 1 && strCurDir[1]==L':')
		FarChDir(strCurDir);

	string strPrevDir=strCurDir;
	bool PrintCommand=true;
	if ((Code=ProcessOSCommands(CmdLine,SeparateWindow,PrintCommand)) == TRUE)
	{
		if (PrintCommand)
		{
			ShowBackground();
			string strNewDir=strCurDir;
			strCurDir=strPrevDir;
			Redraw();
			strCurDir=strNewDir;
			GotoXY(X2+1,Y1);
			Text(L" ");
			ScrollScreen(2);
			SaveBackground();
		}

		SetString(L"", false);

		Code=-1;
	}
	else
	{
		string strTempStr;
		strTempStr = CmdLine;

		if (Code == -1)
			ReplaceStrings(strTempStr,L"/",L"\\",-1);

		Code=Execute(strTempStr,AlwaysWaitFinish,SeparateWindow,DirectRun, 0, WaitForIdle, Silent, RunAs);
	}

	COORD Size1;
	Global->Console->GetSize(Size1);

	if (Size0.X != Size1.X || Size0.Y != Size1.Y)
	{
		GotoXY(X2+1,Y1);
		Text(L" ");
		Global->CtrlObject->CmdLine->CorrectRealScreenCoord();
	}
	if (!Flags.Check(FCMDOBJ_LOCKUPDATEPANEL))
	{
		ShellUpdatePanels(Global->CtrlObject->Cp()->ActivePanel,FALSE);
		if (Global->Opt->ShowKeyBar)
		{
			Global->CtrlObject->MainKeyBar->Show();
		}
	}
	if (Global->Opt->Clock)
		ShowTime(0);

	if (!Silent)
	{
		Global->ScrBuf->Flush();
	}
	return Code;
}

int CommandLine::ProcessOSCommands(const string& CmdLine, bool SeparateWindow, bool &PrintCommand)
{
	string strCmdLine = CmdLine;
	Panel *SetPanel=Global->CtrlObject->Cp()->ActivePanel;
	PrintCommand=true;

	if (SetPanel->GetType()!=FILE_PANEL && Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	RemoveTrailingSpaces(strCmdLine);
	bool SilentInt=false;

	if (CmdLine[0] == L'@')
	{
		SilentInt=true;
		strCmdLine.erase(0, 1);
	}

	auto IsCommand = [&strCmdLine](const string& cmd, bool bslash)->bool
	{
		size_t n = cmd.size();
		return (!StrCmpNI(strCmdLine.data(), cmd.data(), static_cast<int>(n))
		 && (n==strCmdLine.size() || nullptr != wcschr(L"/ \t",strCmdLine[n]) || (bslash && strCmdLine[n]==L'\\')));
	};

	if (!SeparateWindow && strCmdLine.size() == 2 && strCmdLine[1]==L':')
	{
		if(!FarChDir(strCmdLine))
		{
			wchar_t NewDir[]={Upper(strCmdLine[0]),L':',L'\\',0};
			FarChDir(NewDir);
		}
		SetPanel->ChangeDirToCurrent();
		return TRUE;
	}
	// SET [����������=[������]]
	else if (IsCommand(L"SET",false))
	{
		size_t pos;
		strCmdLine.erase(0, 3);
		RemoveLeadingSpaces(strCmdLine);

		if (CheckCmdLineForHelp(strCmdLine.data()))
			return FALSE; // ��������� COMSPEC`�

		// "set" (display all) or "set var" (display all that begin with "var")
		if (strCmdLine.empty() || ((pos = strCmdLine.find(L'=')) == string::npos) || !pos)
		{
			//forward "set [prefix]| command" and "set [prefix]> file" to COMSPEC
			static const wchar_t CharsToFind[] = L"|>";
			if (std::find_first_of(ALL_CONST_RANGE(strCmdLine), ALL_CONST_RANGE(CharsToFind)) != strCmdLine.cend())
				return FALSE;

			ShowBackground();  //??? ������ �� ����� COMSPEC'�
			// display command //???
			Redraw();
			GotoXY(X2+1,Y1);
			Text(L" ");
			Global->ScrBuf->Flush();
			Global->Console->SetTextAttributes(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
			string strOut(L"\n");
			int CmdLength = static_cast<int>(strCmdLine.size());
			LPWCH Environment = GetEnvironmentStrings();
			for (LPCWSTR Ptr = Environment; *Ptr;)
			{
				int PtrLength = StrLength(Ptr);
				if (!StrCmpNI(Ptr, strCmdLine.data(), CmdLength))
				{
					strOut.append(Ptr, PtrLength).append(L"\n");
				}
				Ptr+=PtrLength+1;
			}
			FreeEnvironmentStrings(Environment);
			strOut.append(L"\n\n", Global->Opt->ShowKeyBar?2:1);
			Global->Console->Write(strOut);
			Global->Console->Commit();
			Global->ScrBuf->FillBuf();
			SaveBackground();
			PrintCommand = false;
			return TRUE;
		}

		if (CheckCmdLineForSet(strCmdLine)) // ������� ��� /A � /P
			return FALSE; //todo: /p - dialog, /a - calculation; then set variable ...

		if (strCmdLine.size() == pos+1) //set var=
		{
			strCmdLine.resize(pos);
			SetEnvironmentVariable(strCmdLine.data(),nullptr);
		}
		else
		{
			string strExpandedStr;

			if (apiExpandEnvironmentStrings(strCmdLine.data()+pos+1,strExpandedStr))
			{
				strCmdLine.resize(pos);
				SetEnvironmentVariable(strCmdLine.data(),strExpandedStr.data());
			}
		}

		return TRUE;
	}
	// REM ��� ���������
	else if (IsCommand(L"REM",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+3))
			return FALSE; // ��������� COMSPEC`�
	}
	else if (!strCmdLine.compare(0, 2, L"::", 2))
	{
		return TRUE;
	}
	else if (IsCommand(L"CLS",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+3))
			return FALSE; // ��������� COMSPEC`�

		ClearScreen(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
		SaveBackground();
		PrintCommand=false;
		return TRUE;
	}
	// PUSHD ���� | ..
	else if (IsCommand(L"PUSHD",false))
	{
		strCmdLine.erase(0, 5);
		RemoveLeadingSpaces(strCmdLine);

		if (CheckCmdLineForHelp(strCmdLine.data()))
			return FALSE; // ��������� COMSPEC`�

		PushPopRecord prec;
		prec.strName = strCurDir;

		if (IntChDir(strCmdLine,true,SilentInt))
		{
			ppstack.push(prec);
			SetEnvironmentVariable(L"FARDIRSTACK",prec.strName.data());
		}
		else
		{
			;
		}

		return TRUE;
	}
	// POPD
	// TODO: �������� �������������� �������� - �����, ������� ������� ����������, ����� ���� ��������.
	else if (IsCommand(L"POPD",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+4))
			return FALSE; // ��������� COMSPEC`�

		if (!ppstack.empty())
		{
			PushPopRecord& prec = ppstack.top();
			int Ret=IntChDir(prec.strName,true,SilentInt);
			ppstack.pop();
			const wchar_t* Ptr = nullptr;
			if (!ppstack.empty())
			{
				Ptr = ppstack.top().strName.data();
			}
			SetEnvironmentVariable(L"FARDIRSTACK", Ptr);
			return Ret;
		}

		return TRUE;
	}
	// CLRD
	else if (IsCommand(L"CLRD",false))
	{
		DECLTYPE(ppstack)().swap(ppstack);
		SetEnvironmentVariable(L"FARDIRSTACK",nullptr);
		return TRUE;
	}
	/*
		Displays or sets the active code page number.
		CHCP [nnn]
			nnn   Specifies a code page number (Dec or Hex).
		Type CHCP without a parameter to display the active code page number.
	*/
	else if (IsCommand(L"CHCP",false))
	{
		strCmdLine.erase(0, 4);

		const wchar_t *Ptr=RemoveExternalSpaces(strCmdLine).data();

		if (CheckCmdLineForHelp(Ptr))
			return FALSE; // ��������� COMSPEC`�

		if (!iswdigit(*Ptr))
			return FALSE;

		wchar_t Chr;

		while ((Chr=*Ptr) )
		{
			if (!iswdigit(Chr))
				break;

			++Ptr;
		}

		wchar_t *Ptr2;
		UINT cp=(UINT)wcstol(strCmdLine.data(),&Ptr2,10); //BUGBUG
		BOOL r1=Global->Console->SetInputCodepage(cp);
		BOOL r2=Global->Console->SetOutputCodepage(cp);

		if (r1 && r2) // ���� ��� ���, �� ���  �...
		{
			InitRecodeOutTable();
			InitKeysArray();
			Global->ScrBuf->ResetShadow();
			Global->ScrBuf->Flush();
			return TRUE;
		}
		else  // ��� ������ ������� chcp ���� ������ ;-)
		{
			return FALSE;
		}
	}
	else if (IsCommand(L"IF",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+2))
			return FALSE; // ��������� COMSPEC`�

		const wchar_t *PtrCmd=PrepareOSIfExist(strCmdLine);
		// ����� PtrCmd - ��� ������� �������, ��� IF

		if (PtrCmd && *PtrCmd && Global->CtrlObject->Plugins->ProcessCommandLine(PtrCmd))
		{
			//CmdStr.SetString(L"");
			GotoXY(X1,Y1);
			Global->FS << fmt::MinWidth(X2-X1+1)<<L"";
			Show();
			return TRUE;
		}

		return FALSE;
	}
	// ���������� ���������, ���� ����� Shift-Enter
	else if (!SeparateWindow && (IsCommand(L"CD",true) || IsCommand(L"CHDIR",true)))
	{
		int Length = IsCommand(L"CD",true)? 2 : 5;

		strCmdLine.erase(0, Length);
		RemoveLeadingSpaces(strCmdLine);

		//������������� /D
		//�� � ��� ������ ������ ���� � ��������� � ������� ��� �� �������� �������� ���� ����
		if (!StrCmpNI(strCmdLine.data(),L"/D",2) && IsSpaceOrEos(strCmdLine[2]))
		{
			strCmdLine.erase(0, 2);
			RemoveLeadingSpaces(strCmdLine);
		}

		if (strCmdLine.empty() || CheckCmdLineForHelp(strCmdLine.data()))
			return FALSE; // ��������� COMSPEC`�

		IntChDir(strCmdLine,Length==5,SilentInt);
		return TRUE;
	}
	else if (IsCommand(L"EXIT",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+4))
			return FALSE; // ��������� COMSPEC`�

		FrameManager->ExitMainLoop(FALSE);
		return TRUE;
	}

	return FALSE;
}

bool CommandLine::CheckCmdLineForHelp(const wchar_t *CmdLine)
{
	if (CmdLine && *CmdLine)
	{
		while (IsSpace(*CmdLine))
			CmdLine++;

		if (*CmdLine && (CmdLine[0] == L'/' || CmdLine[0] == L'-') && CmdLine[1] == L'?')
			return true;
	}

	return false;
}

bool CommandLine::CheckCmdLineForSet(const string& CmdLine)
{
	if (CmdLine.size()>1 && CmdLine[0]==L'/' && IsSpaceOrEos(CmdLine[2]))
		return true;

	return false;
}

bool CommandLine::IntChDir(const string& CmdLine,int ClosePanel,bool Selent)
{
	Panel *SetPanel;
	SetPanel=Global->CtrlObject->Cp()->ActivePanel;

	if (SetPanel->GetType()!=FILE_PANEL && Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	string strExpandedDir(CmdLine);
	Unquote(strExpandedDir);
	apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);

	if (SetPanel->GetMode()!=PLUGIN_PANEL && strExpandedDir[0] == L'~' && ((strExpandedDir.size() == 1 && apiGetFileAttributes(strExpandedDir) == INVALID_FILE_ATTRIBUTES) || IsSlash(strExpandedDir[1])))
	{
		if (Global->Opt->Exec.UseHomeDir && !Global->Opt->Exec.strHomeDir.empty())
		{
			string strTemp=Global->Opt->Exec.strHomeDir.Get();

			if (strExpandedDir.size() > 1)
			{
				AddEndSlash(strTemp);
				strTemp += strExpandedDir.data()+2;
			}

			DeleteEndSlash(strTemp);
			strExpandedDir=strTemp;
			apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);
		}
	}

	size_t DirOffset = 0;
	ParsePath(strExpandedDir, &DirOffset);
	if (wcspbrk(strExpandedDir.data() + DirOffset, L"?*")) // ��� �����?
	{
		FAR_FIND_DATA wfd;

		if (apiGetFindDataEx(strExpandedDir, wfd))
		{
			size_t pos;

			if (FindLastSlash(pos,strExpandedDir))
				strExpandedDir.resize(pos+1);
			else
				strExpandedDir.clear();

			strExpandedDir += wfd.strFileName;
		}
	}

	/* $ 15.11.2001 OT
		������� ��������� ���� �� ����� "�������" ����������.
		���� �� ���, �� ����� �������� ������, ��� ��� ���������� ���������
	*/
	DWORD DirAtt=apiGetFileAttributes(strExpandedDir);

	if (DirAtt!=INVALID_FILE_ATTRIBUTES && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && IsAbsolutePath(strExpandedDir))
	{
		ReplaceSlashToBSlash(strExpandedDir);
		SetPanel->SetCurDir(strExpandedDir,true);
		return true;
	}

	/* $ 20.09.2002 SKV
	  ��� ��������� ����������� ��������� ����� ������� ���:
	  cd net:server � cd ftp://server/dir
	  ��� ��� ��� �� �� ������� �������� �
	  cd s&r:, cd make: � �.�., ������� � �����
	  �������� �� ����� �������� ���������.
	*/
	/*
	if (Global->CtrlObject->Plugins->ProcessCommandLine(ExpandedDir))
	{
	  //CmdStr.SetString(L"");
	  GotoXY(X1,Y1);
	  Global->FS << fmt::Width(X2-X1+1)<<L"";
	  Show();
	  return true;
	}
	*/

	if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
	{
		SetPanel->SetCurDir(strExpandedDir,ClosePanel!=0);
		return true;
	}

	if (FarChDir(strExpandedDir))
	{
		SetPanel->ChangeDirToCurrent();

		if (!SetPanel->IsVisible())
			SetPanel->SetTitle();
	}
	else
	{
		Global->CatchError();
		if (!Selent)
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strExpandedDir.data(),MSG(MOk));

		return false;
	}

	return true;
}

void CommandLine::LockUpdatePanel(bool Mode)
{
	Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);
}
