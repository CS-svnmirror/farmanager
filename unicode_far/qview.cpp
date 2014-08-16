/*
qview.cpp

Quick view panel
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

#include "qview.hpp"
#include "macroopcode.hpp"
#include "flink.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "help.hpp"
#include "viewer.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "interf.hpp"
#include "execute.hpp"
#include "dirinfo.hpp"
#include "pathmix.hpp"
#include "mix.hpp"
#include "constitle.hpp"
#include "syslog.hpp"
#include "colormix.hpp"
#include "language.hpp"

static bool LastMode = false;
static bool LastWrapMode = false;
static bool LastWrapType = false;

QuickView::QuickView():
	QView(nullptr),
	Directory(0),
	PrevMacroMode(MACROAREA_INVALID),
	Data(),
	OldWrapMode(0),
	OldWrapType(0),
	m_TemporaryFile(),
	uncomplete_dirscan(false)
{
	Type=QVIEW_PANEL;
	if (!LastMode)
	{
		LastMode = true;
		LastWrapMode = Global->Opt->ViOpt.ViewerIsWrap;
		LastWrapType = Global->Opt->ViOpt.ViewerWrap;
	}
}


QuickView::~QuickView()
{
	CloseFile();
	SetMacroMode(TRUE);
}


string QuickView::GetTitle() const
{
	string strTitle = L" ";
	strTitle+=MSG(MQuickViewTitle);
	strTitle+=L" ";
	TruncStr(strTitle,X2-X1-3);
	return strTitle;
}

void QuickView::DisplayObject()
{
	if (Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	Flags.Set(FSCROBJ_ISREDRAWING);

	if (!QView && !ProcessingPluginCommand)
		Global->CtrlObject->Cp()->GetAnotherPanel(this)->UpdateViewPanel();

	if (this->Destroyed())
		return;

	if (QView)
		QView->SetPosition(X1+1,Y1+1,X2-1,Y2-3);

	Box(X1,Y1,X2,Y2,ColorIndexToColor(COL_PANELBOX),DOUBLE_BOX);
	SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);

	string strTitle = GetTitle();
	if (!strTitle.empty())
	{
		GotoXY(X1+(X2-X1+1-(int)strTitle.size())/2,Y1);
		Text(strTitle);
	}

	DrawSeparator(Y2-2);
	SetColor(COL_PANELTEXT);
	GotoXY(X1+1,Y2-1);
	Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(X2-X1-1)<<PointToName(strCurFileName);

	if (!strCurFileType.empty())
	{
		string strTypeText=L" ";
		strTypeText+=strCurFileType;
		strTypeText+=L" ";
		TruncStr(strTypeText,X2-X1-1);
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(X1+(X2-X1+1-(int)strTypeText.size())/2,Y2-2);
		Text(strTypeText);
	}

	if (Directory)
	{
		FormatString FString;
		string DisplayName(strCurFileName);
		TruncPathStr(DisplayName,std::max(0, X2-X1-1-StrLength(MSG(MQuickViewFolder))-5));
		FString<<MSG(MQuickViewFolder)<<L" \""<<DisplayName<<L"\"";
		SetColor(COL_PANELTEXT);
		GotoXY(X1+2,Y1+2);
		PrintText(FString);

		DWORD currAttr=api::GetFileAttributes(strCurFileName); // ������������, ���� ��� �������
		if (currAttr != INVALID_FILE_ATTRIBUTES && (currAttr&FILE_ATTRIBUTE_REPARSE_POINT))
		{
			string Tmp, Target;
			DWORD ReparseTag=0;
			const wchar_t* PtrName;
			if (GetReparsePointInfo(strCurFileName, Target, &ReparseTag))
			{
				NormalizeSymlinkName(Target);
				switch(ReparseTag)
				{
				// 0xA0000003L = Directory Junction or Volume Mount Point
				case IO_REPARSE_TAG_MOUNT_POINT:
					{
						LNGID ID_Msg = MQuickViewJunction;
						bool Root;
						if(ParsePath(Target, nullptr, &Root) == PATH_VOLUMEGUID && Root)
						{
							ID_Msg=MQuickViewVolMount;
						}
						PtrName = MSG(ID_Msg);
					}
					break;
				// 0xA000000CL = Directory or File Symbolic Link
				case IO_REPARSE_TAG_SYMLINK:
					PtrName = MSG(MQuickViewSymlink);
					break;
				// 0x8000000AL = Distributed File System
				case IO_REPARSE_TAG_DFS:
					PtrName = MSG(MQuickViewDFS);
					break;
				// 0x80000012L = Distributed File System Replication
				case IO_REPARSE_TAG_DFSR:
					PtrName = MSG(MQuickViewDFSR);
					break;
				// 0xC0000004L = Hierarchical Storage Management
				case IO_REPARSE_TAG_HSM:
					PtrName = MSG(MQuickViewHSM);
					break;
				// 0x80000006L = Hierarchical Storage Management2
				case IO_REPARSE_TAG_HSM2:
					PtrName = MSG(MQuickViewHSM2);
					break;
				// 0x80000007L = Single Instance Storage
				case IO_REPARSE_TAG_SIS:
					PtrName = MSG(MQuickViewSIS);
					break;
				// 0x80000008L = Windows Imaging Format
				case IO_REPARSE_TAG_WIM:
					PtrName = MSG(MQuickViewWIM);
					break;
				// 0x80000009L = Cluster Shared Volumes
				case IO_REPARSE_TAG_CSV:
					PtrName = MSG(MQuickViewCSV);
					break;
				case IO_REPARSE_TAG_DEDUP:
					PtrName = MSG(MQuickViewDEDUP);
					break;
				case IO_REPARSE_TAG_NFS:
					PtrName = MSG(MQuickViewNFS);
					break;
				// 0x????????L = anything else
				default:
					if (Global->Opt->ShowUnknownReparsePoint)
					{
						Tmp = FormatString() << L":" << fmt::Radix(16) << fmt::ExactWidth(8) << fmt::FillChar(L'0') << ReparseTag;
						PtrName = Tmp.data();
					}
					else
					{
						PtrName=MSG(MQuickViewUnknownReparsePoint);
					}
				}
			}
			else
			{
				PtrName = MSG(MQuickViewUnknownReparsePoint);
				Target = MSG(MQuickViewNoData);
			}

			TruncPathStr(Target,std::max(0, X2-X1-1-StrLength(PtrName)-5));
			FString.clear();
			FString<<PtrName<<L" \""<<Target<<L"\"";
			SetColor(COL_PANELTEXT);
			GotoXY(X1+2,Y1+3);
			PrintText(FString);
		}

		if (Directory==1 || Directory==4)
		{
			int iColor = uncomplete_dirscan ? COL_PANELHIGHLIGHTTEXT : COL_PANELINFOTEXT;
			const wchar_t *prefix = uncomplete_dirscan ? L"~" : L"";
			GotoXY(X1+2,Y1+4);
			PrintText(MSG(MQuickViewContains));
			GotoXY(X1+2,Y1+6);
			PrintText(MSG(MQuickViewFolders));
			SetColor(iColor);
			FString.clear();
			FString<<prefix<<Data.DirCount;
			PrintText(FString);
			SetColor(COL_PANELTEXT);
			GotoXY(X1+2,Y1+7);
			PrintText(MSG(MQuickViewFiles));
			SetColor(iColor);
			FString.clear();
			FString<<prefix<<Data.FileCount;
			PrintText(FString);
			SetColor(COL_PANELTEXT);
			GotoXY(X1+2,Y1+8);
			PrintText(MSG(MQuickViewBytes));
			SetColor(iColor);
			string strSize;
			InsertCommas(Data.FileSize,strSize);
			PrintText(prefix+strSize);
			SetColor(COL_PANELTEXT);
			GotoXY(X1+2,Y1+9);
			PrintText(MSG(MQuickViewAllocated));
			SetColor(iColor);
			InsertCommas(Data.AllocationSize,strSize);
			FString.clear();
			FString << prefix << strSize << L" (" << ToPercent(Data.AllocationSize,Data.FileSize) << L"%)";
			PrintText(FString);

			if (Directory!=4)
			{
				SetColor(COL_PANELTEXT);
				GotoXY(X1+2,Y1+11);
				PrintText(MSG(MQuickViewCluster));
				SetColor(iColor);
				InsertCommas(Data.ClusterSize,strSize);
				PrintText(prefix+strSize);

				SetColor(COL_PANELTEXT);
				GotoXY(X1+2,Y1+12);
				PrintText(MSG(MQuickViewSlack));
				SetColor(iColor);
				InsertCommas(Data.FilesSlack, strSize);
				FString.clear();
				FString << prefix << strSize << L" (" << ToPercent(Data.FilesSlack, Data.AllocationSize) << L"%)";
				PrintText(FString);

				SetColor(COL_PANELTEXT);
				GotoXY(X1+2,Y1+13);
				PrintText(MSG(MQuickViewMFTOverhead));
				SetColor(iColor);
				InsertCommas(Data.MFTOverhead, strSize);
				FString.clear();
				FString<<prefix<<strSize<<L" ("<<ToPercent(Data.MFTOverhead, Data.AllocationSize)<<L"%)";
				PrintText(FString);

			}
		}
	}
	else if (QView)
		QView->Show();

	Flags.Clear(FSCROBJ_ISREDRAWING);
}


__int64 QuickView::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (!Directory && QView)
		return QView->VMProcess(OpCode,vParam,iParam);

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return 1;
	}

	return 0;
}

int QuickView::ProcessKey(const Manager::Key& Key)
{
	int LocalKey=Key.FarKey;
	if (!IsVisible())
		return FALSE;

	if (LocalKey>=KEY_RCTRL0 && LocalKey<=KEY_RCTRL9)
	{
		ExecShortcutFolder(LocalKey-KEY_RCTRL0);
		return TRUE;
	}

	if (LocalKey == KEY_F1)
	{
		Help Hlp(L"QViewPanel");
		return TRUE;
	}

	if (LocalKey==KEY_F3 || LocalKey==KEY_NUMPAD5 || LocalKey == KEY_SHIFTNUMPAD5)
	{
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

		if (AnotherPanel->GetType()==FILE_PANEL)
			AnotherPanel->ProcessKey(Manager::Key(KEY_F3));

		return TRUE;
	}

	if (LocalKey==KEY_ADD || LocalKey==KEY_SUBTRACT)
	{
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

		if (AnotherPanel->GetType()==FILE_PANEL)
			AnotherPanel->ProcessKey(Manager::Key(LocalKey==KEY_ADD?KEY_DOWN:KEY_UP));

		return TRUE;
	}

	if (QView && !Directory && LocalKey>=256)
	{
		int ret = QView->ProcessKey(Manager::Key(LocalKey));

		if (LocalKey == KEY_F4 || LocalKey == KEY_F8 || LocalKey == KEY_F2 || LocalKey == KEY_SHIFTF2)
		{
			DynamicUpdateKeyBar();
			Global->CtrlObject->MainKeyBar->Redraw();
		}

		if (LocalKey == KEY_F7 || LocalKey == KEY_SHIFTF7)
		{
			//__int64 Pos;
			//int Length;
			//DWORD Flags;
			//QView->GetSelectedParam(Pos,Length,Flags);
			Redraw();
			Global->CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
			//QView->SelectText(Pos,Length,Flags|1);
		}

		return ret;
	}

	return FALSE;
}


int QuickView::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	int RetCode;

	if (!IsVisible())
		return FALSE;

	if (Panel::PanelProcessMouse(MouseEvent,RetCode))
		return RetCode;

	SetFocus();

	if (QView && !Directory)
		return QView->ProcessMouse(MouseEvent);

	return FALSE;
}

void QuickView::Update(int Mode)
{
	if (!EnableUpdate)
		return;

	if (strCurFileName.empty())
		Global->CtrlObject->Cp()->GetAnotherPanel(this)->UpdateViewPanel();

	Redraw();
}

void QuickView::ShowFile(const string& FileName, bool TempFile, PluginHandle* hDirPlugin)
{
	DWORD FileAttr=0;
	CloseFile();
	QView=nullptr;

	if (!IsVisible())
		return;

	if (FileName.empty())
	{
		ProcessingPluginCommand++;
		Show();
		ProcessingPluginCommand--;
		return;
	}

	string FileFullName = FileName;
	ConvertNameToFull(FileFullName, FileFullName);

	bool SameFile = strCurFileName == FileFullName;
	strCurFileName = FileFullName;

	size_t pos = strCurFileName.rfind(L'.');
	if (pos != string::npos)
	{
		string strValue;

		if (GetShellType(strCurFileName.data()+pos, strValue))
		{
			api::reg::GetValue(HKEY_CLASSES_ROOT, strValue, L"", strCurFileType);
		}
	}

	if (hDirPlugin || ((FileAttr=api::GetFileAttributes(strCurFileName))!=INVALID_FILE_ATTRIBUTES && (FileAttr & FILE_ATTRIBUTE_DIRECTORY)))
	{
		// �� ���������� ��� ����� ��� ��������� � "������� ���������"
		strCurFileType.clear();

		if (SameFile && !hDirPlugin)
		{
			Directory=1;
		}
		else if (hDirPlugin)
		{
			int ExitCode=GetPluginDirInfo(hDirPlugin,strCurFileName,Data.DirCount,
			                              Data.FileCount,Data.FileSize,Data.AllocationSize);
			Directory = (ExitCode ? 4 : 3);
			uncomplete_dirscan = (ExitCode == 0);
		}
		else
		{
			int ExitCode=GetDirInfo(MSG(MQuickViewTitle), strCurFileName, Data, 500, nullptr, GETDIRINFO_ENHBREAK|GETDIRINFO_SCANSYMLINKDEF|GETDIRINFO_DONTREDRAWFRAME);
			Directory = (ExitCode == -1 ? 2 : 1); // ExitCode: 1=done; 0=Esc,CtrlBreak; -1=Other
			uncomplete_dirscan = ExitCode != 1;
		}
	}
	else
	{
		if (!strCurFileName.empty())
		{
			QView = std::make_unique<Viewer>(true);
			QView->SetRestoreScreenMode(false);
			QView->SetPosition(X1+1,Y1+1,X2-1,Y2-3);
			QView->SetStatusMode(0);
			QView->EnableHideCursor(0);
			OldWrapMode = QView->GetWrapMode();
			OldWrapType = QView->GetWrapType();
			QView->SetWrapMode(LastWrapMode);
			QView->SetWrapType(LastWrapType);
			QView->OpenFile(strCurFileName,FALSE);
		}
	}
	if (this->Destroyed())
		return;

	m_TemporaryFile = TempFile;

	Redraw();

	if (Global->CtrlObject->Cp()->ActivePanel() == this)
	{
		DynamicUpdateKeyBar();
		Global->CtrlObject->MainKeyBar->Redraw();
	}
}


void QuickView::CloseFile()
{
	if (QView)
	{
		LastWrapMode=QView->GetWrapMode();
		LastWrapType=QView->GetWrapType();
		QView->SetWrapMode(OldWrapMode);
		QView->SetWrapType(OldWrapType);
		QView.reset();
	}

	strCurFileType.clear();
	QViewDelTempName();
	Directory=0;
}


void QuickView::QViewDelTempName()
{
	if (m_TemporaryFile)
	{
		if (QView)
		{
			LastWrapMode=QView->GetWrapMode();
			LastWrapType=QView->GetWrapType();
			QView->SetWrapMode(OldWrapMode);
			QView->SetWrapType(OldWrapType);
			QView.reset();
		}

		api::SetFileAttributes(strCurFileName, FILE_ATTRIBUTE_ARCHIVE);
		api::DeleteFile(strCurFileName);  //BUGBUG
		string TempDirectoryName = strCurFileName;
		CutToSlash(TempDirectoryName);
		api::RemoveDirectory(TempDirectoryName);
		m_TemporaryFile = false;
	}
}


void QuickView::PrintText(const string& Str)
{
	if (WhereY()>Y2-3 || WhereX()>X2-2)
		return;

	Global->FS << fmt::MaxWidth(X2-2-WhereX()+1)<<Str;
}


int QuickView::UpdateIfChanged(panel_update_mode UpdateMode)
{
	if (IsVisible() && !strCurFileName.empty() && Directory==2)
	{
		string strViewName = strCurFileName;
		ShowFile(strViewName, m_TemporaryFile, nullptr);
		return TRUE;
	}

	return FALSE;
}

void QuickView::SetTitle()
{
	if (GetFocus())
	{
		string strTitleDir(L"{");

		if (!strCurFileName.empty())
		{
			strTitleDir += strCurFileName;
			strTitleDir += L" - QuickView";
		}
		else
		{
			string strCmdText;
			Global->CtrlObject->CmdLine->GetString(strCmdText);
			strTitleDir += strCmdText;
		}

		strTitleDir += L"}";

		ConsoleTitle::SetFarTitle(strTitleDir);
	}
}

void QuickView::SetFocus()
{
	Panel::SetFocus();
	SetTitle();
	SetMacroMode(FALSE);
}

void QuickView::KillFocus()
{
	Panel::KillFocus();
	SetMacroMode(TRUE);
}

void QuickView::SetMacroMode(int Restore)
{
	if (!Global->CtrlObject)
		return;

	if (PrevMacroMode == MACROAREA_INVALID)
		PrevMacroMode = Global->CtrlObject->Macro.GetMode();

	Global->CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACROAREA_QVIEWPANEL);
}

int QuickView::GetCurName(string &strName, string &strShortName) const
{
	if (!strCurFileName.empty())
	{
		strName = strCurFileName;
		strShortName = strName;
		return TRUE;
	}

	return FALSE;
}

void QuickView::UpdateKeyBar()
{
	Global->CtrlObject->MainKeyBar->SetLabels(MQViewF1);
	DynamicUpdateKeyBar();
}

void QuickView::DynamicUpdateKeyBar() const
{
	KeyBar *KB = Global->CtrlObject->MainKeyBar;

	if (Directory || !QView)
	{
		KB->Change(MSG(MF2), 2-1);
		KB->Change(L"", 4-1);
		KB->Change(L"", 8-1);
		KB->Change(KBL_SHIFT, L"", 2-1);
		KB->Change(KBL_SHIFT, L"", 8-1);
		KB->Change(KBL_ALT, MSG(MAltF8), 8-1);  // ����������� ��� ������ - "�������"
	}
	else
	{
		if (QView->GetHexMode())
			KB->Change(MSG(MViewF4Text), 4-1);
		else
			KB->Change(MSG(MQViewF4), 4-1);

		if (QView->GetCodePage() != GetOEMCP())
			KB->Change(MSG(MViewF8DOS), 8-1);
		else
			KB->Change(MSG(MQViewF8), 8-1);

		if (!QView->GetWrapMode())
		{
			if (QView->GetWrapType())
				KB->Change(MSG(MViewShiftF2), 2-1);
			else
				KB->Change(MSG(MViewF2), 2-1);
		}
		else
			KB->Change(MSG(MViewF2Unwrap), 2-1);

		if (QView->GetWrapType())
			KB->Change(KBL_SHIFT, MSG(MViewF2), 2-1);
		else
			KB->Change(KBL_SHIFT, MSG(MViewShiftF2), 2-1);
	}

	KB->SetCustomLabels(KBA_QUICKVIEW);
}
