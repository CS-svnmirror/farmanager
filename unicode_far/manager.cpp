/*
manager.cpp

������������ ����� ����������� file panels, viewers, editors, dialogs
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

#include "manager.hpp"
#include "keys.hpp"
#include "frame.hpp"
#include "vmenu2.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "grabber.hpp"
#include "message.hpp"
#include "config.hpp"
#include "plist.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "scrbuf.hpp"
#include "console.hpp"
#include "configdb.hpp"
#include "DlgGuid.hpp"
#include "plugins.hpp"

Manager *FrameManager;
long CurrentWindowType=-1;

Manager::Manager():
	InsertedFrame(nullptr),
	DeletedFrame(nullptr),
	ActivatedFrame(nullptr),
	RefreshedFrame(nullptr),
	ModalizedFrame(nullptr),
	UnmodalizedFrame(nullptr),
	DeactivatedFrame(nullptr),
	ExecutedFrame(nullptr),
	CurrentFrame(nullptr),
	FramePos(-1),
	ModalEVCount(0),
	EndLoop(FALSE),
	ModalExitCode(-1),
	StartManager(FALSE)
{
	Frames.reserve(1024);
	ModalFrames.reserve(1024);
}

Manager::~Manager()
{
}


/* $ 29.12.2000 IS
  ������ CloseAll, �� ��������� ����������� ����������� ������ � ����,
  ���� ������������ ��������� ������������� ����.
  ���������� TRUE, ���� ��� ������� � ����� �������� �� ����.
*/
BOOL Manager::ExitAll()
{
	_MANAGER(CleverSysLog clv(L"Manager::ExitAll()"));

	// BUGBUG don't use iterators here, may be invalidated by DeleteCommit()
	for(size_t i = ModalFrames.size(); i; --i)
	{
		if (i - 1 >= ModalFrames.size())
			continue;
		auto CurFrame = ModalFrames[i - 1];
		if (!CurFrame->GetCanLoseFocus(TRUE))
		{
			auto PrevFrameCount = ModalFrames.size();
			CurFrame->ProcessKey(KEY_ESC);
			Commit();

			if (PrevFrameCount == ModalFrames.size())
			{
				return FALSE;
			}
		}
	}

	// BUGBUG don't use iterators here, may be invalidated by DeleteCommit()
	for(size_t i = Frames.size(); i; --i)
	{
		if (i - 1 >= Frames.size())
			continue;
		auto CurFrame = Frames[i - 1];
		if (!CurFrame->GetCanLoseFocus(TRUE))
		{
			ActivateFrame(CurFrame);
			Commit();
			auto PrevFrameCount = Frames.size();
			CurFrame->ProcessKey(KEY_ESC);
			Commit();

			if (PrevFrameCount == Frames.size())
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

void Manager::CloseAll()
{
	_MANAGER(CleverSysLog clv(L"Manager::CloseAll()"));

	// BUGBUG don't use iterators here, may be invalidated by DeleteCommit()
	while(!ModalFrames.empty())
	{
		DeleteFrame(ModalFrames.back());
		DeleteCommit();
		DeletedFrame=nullptr;
	}

	// BUGBUG don't use iterators here, may be invalidated by DeleteCommit()
	while(!Frames.empty())
	{
		DeleteFrame(Frames.back());
		DeleteCommit();
		DeletedFrame=nullptr;
	}

	Frames.clear();
}

void Manager::InsertFrame(Frame *Inserted)
{
	_MANAGER(CleverSysLog clv(L"Manager::InsertFrame(Frame *Inserted, int Index)"));
	_MANAGER(SysLog(L"Inserted=%p, Index=%i",Inserted, Index));

	InsertedFrame=Inserted;
}

void Manager::DeleteFrame(Frame *Deleted)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeleteFrame(Frame *Deleted)"));
	_MANAGER(SysLog(L"Deleted=%p",Deleted));

	if (std::any_of(CONST_RANGE(Frames, i) {return i->RemoveModal(Deleted);}))
		return;

	if (!Deleted)
	{
		DeletedFrame=CurrentFrame;
	}
	else
	{
		DeletedFrame=Deleted;
	}
}

void Manager::DeleteFrame(int Index)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeleteFrame(int Index)"));
	_MANAGER(SysLog(L"Index=%i",Index));
	DeleteFrame(GetFrame(Index));
}


void Manager::ModalizeFrame(Frame *Modalized, int Mode)
{
	_MANAGER(CleverSysLog clv(L"Manager::ModalizeFrame (Frame *Modalized, int Mode)"));
	_MANAGER(SysLog(L"Modalized=%p",Modalized));
	ModalizedFrame=Modalized;
	ModalizeCommit();
}

void Manager::UnmodalizeFrame(Frame *Unmodalized)
{
	_MANAGER(CleverSysLog clv(L"Manager::UnmodalizeFrame (Frame *Unmodalized)"));
	_MANAGER(SysLog(L"Unmodalized=%p",Unmodalized));
	UnmodalizedFrame=Unmodalized;
	UnmodalizeCommit();
}

void Manager::ExecuteNonModal()
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteNonModal ()"));
	_MANAGER(SysLog(L"ExecutedFrame=%p, InsertedFrame=%p, DeletedFrame=%p",ExecutedFrame, InsertedFrame, DeletedFrame));
	Frame *NonModal=InsertedFrame?InsertedFrame:(ExecutedFrame?ExecutedFrame:ActivatedFrame);

	if (!NonModal)
	{
		return;
	}

	/* $ 14.05.2002 SKV
	  ������� ������� ����� � ������ "���������" ������������� �������
	*/
	//Frame *SaveFrame=CurrentFrame;
	//AddSemiModalBackFrame(SaveFrame);
	int NonModalIndex=IndexOf(NonModal);

	if (-1==NonModalIndex)
	{
		InsertedFrame=NonModal;
		ExecutedFrame=nullptr;
		InsertCommit();
		InsertedFrame=nullptr;
	}
	else
	{
		ActivateFrame(NonModalIndex);
	}

	//Frame* ModalStartLevel=NonModal;
	for (;;)
	{
		Commit();

		if (CurrentFrame!=NonModal || EndLoop)
		{
			break;
		}

		ProcessMainLoop();
	}

	//ExecuteModal(NonModal);
	/* $ 14.05.2002 SKV
	  ... � ����� ��� ��.
	*/
	//RemoveSemiModalBackFrame(SaveFrame);
}

void Manager::ExecuteModal(Frame *Executed)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteModal (Frame *Executed)"));
	_MANAGER(SysLog(L"Executed=%p, ExecutedFrame=%p",Executed,ExecutedFrame));

	if (!Executed && !ExecutedFrame)
	{
		return;
	}

	if (Executed)
	{
		if (ExecutedFrame)
		{
			_MANAGER(SysLog(L"WARNING! ������� � ����� ����� ��������� � ��������� ������ ��� ������. Executed=%p, ExecitedFrame=%p",Executed, ExecutedFrame));
			return;// nullptr; //?? ����������, ����� �������� ��������� ���������� � ���� ������
		}
		else
		{
			ExecutedFrame=Executed;
		}
	}

	auto ModalStartLevel=ModalFrames.size();
	int OriginalStartManager=StartManager;
	StartManager=TRUE;

	for (;;)
	{
		Commit();

		if (ModalFrames.size()<=ModalStartLevel)
		{
			break;
		}

		ProcessMainLoop();
	}

	StartManager=OriginalStartManager;
	return;// GetModalExitCode();
}

int Manager::GetModalExitCode()
{
	return ModalExitCode;
}

/* $ 11.10.2001 IS
   ���������� ���������� ������� � ��������� ������.
*/
int Manager::CountFramesWithName(const string& Name, BOOL IgnoreCase)
{
	int Counter=0;
	typedef int (*CompareFunction)(const string&, const string&);
	CompareFunction CaseSenitive = StrCmp, CaseInsensitive = StrCmpI;
	CompareFunction CmpFunction = IgnoreCase? CaseInsensitive : CaseSenitive;

	string strType, strCurName;

	std::for_each(CONST_RANGE(Frames, i)
	{
		i->GetTypeAndName(strType, strCurName);
		if (!CmpFunction(Name, strCurName))
			++Counter;
	});

	return Counter;
}

/*!
  \return ���������� nullptr ���� ����� "�����" ��� ���� ����� ������� �����.
  ������� �������, ���� ����������� ����� �� ���������.
  ���� �� ����� ���������, �� ����� ������� ������ ����������
  ��������� �� ���������� �����.
*/
Frame *Manager::FrameMenu()
{
	/* $ 28.04.2002 KM
	    ���� ��� ����������� ����, ��� ���� ������������
	    ������� ��� ������������.
	*/
	static int AlreadyShown=FALSE;

	if (AlreadyShown)
		return nullptr;

	int ExitCode, CheckCanLoseFocus=CurrentFrame->GetCanLoseFocus();
	{
		MenuItemEx ModalMenuItem;
		VMenu2 ModalMenu(MSG(MScreensTitle),nullptr,0,ScrY-4);
		ModalMenu.SetHelp(L"ScrSwitch");
		ModalMenu.SetFlags(VMENU_WRAPMODE);
		ModalMenu.SetPosition(-1,-1,0,0);
		ModalMenu.SetId(ScreensSwitchId);

		if (!CheckCanLoseFocus)
			ModalMenuItem.SetDisable(TRUE);

		size_t n = 0;
		std::for_each(CONST_RANGE(Frames, i)
		{
			string strType, strName, strNumText;
			i->GetTypeAndName(strType, strName);
			ModalMenuItem.Clear();

			if (n < 10)
				strNumText.Format(L"&%d. ", n);
			else if (n < 36)
				strNumText.Format(L"&%c. ", n + 55);  // 55='A'-10
			else
				strNumText = L"&   ";

			//TruncPathStr(strName,ScrX-24);
			ReplaceStrings(strName,L"&",L"&&",-1);
			/*  ����������� "*" ���� ���� ������� */
			ModalMenuItem.strName.Format(L"%s%-10.10s %c %s", strNumText.c_str(), strType.c_str(),(i->IsFileModified()?L'*':L' '), strName.c_str());
			ModalMenuItem.SetSelect(static_cast<int>(n) == FramePos);
			ModalMenu.AddItem(&ModalMenuItem);
			++n;
		});

		AlreadyShown=TRUE;
		ExitCode=ModalMenu.Run();
		AlreadyShown=FALSE;
	}

	if (CheckCanLoseFocus)
	{
		if (ExitCode>=0)
		{
			ActivateFrame(ExitCode);
			return (ActivatedFrame==CurrentFrame || !CurrentFrame->GetCanLoseFocus()?nullptr:CurrentFrame);
		}

		return (ActivatedFrame==CurrentFrame?nullptr:CurrentFrame);
	}

	return nullptr;
}


int Manager::GetFrameCountByType(int Type)
{
	int ret=0;

	std::for_each(CONST_RANGE(Frames, i)
	{
		// �� ��������� �����, ������� ���������� �������
		if (i != DeletedFrame && i->GetExitCode() != XC_QUIT && i->GetType() == Type)
			ret++;
	});

	return ret;
}

/*$ 11.05.2001 OT ������ ����� ������ ���� �� ������ �� ������� �����, �� � �������� - ����, �������� ��� */
int  Manager::FindFrameByFile(int ModalType,const string& FileName, const wchar_t *Dir)
{
	string strBufFileName;
	string strFullFileName = FileName;

	if (Dir)
	{
		strBufFileName = Dir;
		AddEndSlash(strBufFileName);
		strBufFileName += FileName;
		strFullFileName = strBufFileName;
	}

	int n = 0;
	if (std::any_of(CONST_RANGE(Frames, i)->bool
	{
		string strType, strName;

		// Mantis#0000469 - �������� Name ����� ������ ��� ���������� ModalType
		if (i->GetType()==ModalType)
		{
			i->GetTypeAndName(strType, strName);

			if (!StrCmpI(strName.c_str(), strFullFileName.c_str()))
				return true;
		}
		++n;
		return false;
	}))
		return n;

	return -1;
}

BOOL Manager::ShowBackground()
{
	if (Global->CtrlObject->CmdLine)
	{
		Global->CtrlObject->CmdLine->ShowBackground();
		return TRUE;
	}
	return FALSE;
}


void Manager::ActivateFrame(Frame *Activated)
{
	_MANAGER(CleverSysLog clv(L"Manager::ActivateFrame(Frame *Activated)"));
	_MANAGER(SysLog(L"Activated=%i",Activated));

	if (IndexOf(Activated)==-1 && IndexOfStack(Activated)==-1)
		return;

	if (!ActivatedFrame)
	{
		ActivatedFrame=Activated;
	}
}

void Manager::ActivateFrame(int Index)
{
	_MANAGER(CleverSysLog clv(L"Manager::ActivateFrame(int Index)"));
	_MANAGER(SysLog(L"Index=%i",Index));
	ActivateFrame(GetFrame(Index));
}

void Manager::DeactivateFrame(Frame *Deactivated,int Direction)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeactivateFrame (Frame *Deactivated,int Direction)"));
	_MANAGER(SysLog(L"Deactivated=%p, Direction=%d",Deactivated,Direction));

	if (Direction)
	{
		FramePos+=Direction;

		if (Direction>0)
		{
			if (FramePos >= static_cast<int>(Frames.size()))
			{
				FramePos=0;
			}
		}
		else
		{
			if (FramePos<0)
			{
				FramePos = static_cast<int>(Frames.size()-1);
			}
		}

		ActivateFrame(FramePos);
	}
	else
	{
		// Direction==0
		// Direct access from menu or (in future) from plugin
	}

	DeactivatedFrame=Deactivated;
}

void Manager::RefreshFrame(Frame *Refreshed)
{
	_MANAGER(CleverSysLog clv(L"Manager::RefreshFrame(Frame *Refreshed)"));
	_MANAGER(SysLog(L"Refreshed=%p",Refreshed));

	if (ActivatedFrame)
		return;

	if (Refreshed)
	{
		RefreshedFrame=Refreshed;
	}
	else
	{
		RefreshedFrame=CurrentFrame;
	}

	if (IndexOf(Refreshed)==-1 && IndexOfStack(Refreshed)==-1)
		return;
}

void Manager::RefreshFrame(int Index)
{
	_MANAGER(CleverSysLog clv(L"Manager::RefreshFrame(int Index)"));
	_MANAGER(SysLog(L"Index=%d",Index));
	RefreshFrame(GetFrame(Index));
}

void Manager::ExecuteFrame(Frame *Executed)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteFrame(Frame *Executed)"));
	_MANAGER(SysLog(L"Executed=%p",Executed));
	ExecutedFrame=Executed;
}


/* $ 10.05.2001 DJ
   ������������� �� ������ (����� � ������� 0)
*/

void Manager::SwitchToPanels()
{
	_MANAGER(CleverSysLog clv(L"Manager::SwitchToPanels()"));
	ActivateFrame(0);
}


int Manager::HaveAnyFrame()
{
	if (!Frames.empty() || InsertedFrame || DeletedFrame || ActivatedFrame || RefreshedFrame ||
	        ModalizedFrame || DeactivatedFrame || ExecutedFrame || CurrentFrame)
		return 1;

	return 0;
}

void Manager::EnterMainLoop()
{
	Global->WaitInFastFind=0;
	StartManager=TRUE;

	for (;;)
	{
		Commit();

		if (EndLoop || !HaveAnyFrame())
		{
			break;
		}

		ProcessMainLoop();
	}
}

void Manager::SetLastInputRecord(const INPUT_RECORD *Rec)
{
	if (&LastInputRecord != Rec)
		LastInputRecord=*Rec;
}


void Manager::ProcessMainLoop()
{
	if ( CurrentFrame )
		Global->CtrlObject->Macro.SetMode(CurrentFrame->GetMacroMode());

	if ( CurrentFrame && !CurrentFrame->ProcessEvents() )
	{
		ProcessKey(KEY_IDLE);
	}
	else
	{
		// Mantis#0000073: �� �������� ������������ � QView
		Global->WaitInMainLoop=IsPanelsActive(true);
		//WaitInFastFind++;
		int Key=GetInputRecord(&LastInputRecord);
		//WaitInFastFind--;
		Global->WaitInMainLoop=FALSE;

		if (EndLoop)
			return;

		if (LastInputRecord.EventType==MOUSE_EVENT && !(Key==KEY_MSWHEEL_UP || Key==KEY_MSWHEEL_DOWN || Key==KEY_MSWHEEL_RIGHT || Key==KEY_MSWHEEL_LEFT))
		{
				// ���������� ����� ���������, �.�. LastInputRecord ����� �������� ���������� �� ����� ���������� ProcessMouse
				MOUSE_EVENT_RECORD mer=LastInputRecord.Event.MouseEvent;
				ProcessMouse(&mer);
		}
		else
			ProcessKey(Key);
	}

	if(IsPanelsActive())
	{
		if(!Global->PluginPanelsCount)
		{
			Global->CtrlObject->Plugins->RefreshPluginsList();
		}
	}
}

void Manager::ExitMainLoop(int Ask)
{
	if (Global->CloseFAR)
	{
		Global->CloseFARMenu=TRUE;
	};

	const wchar_t* const Items[] = {MSG(MAskQuit),MSG(MYes),MSG(MNo)};

	if (!Ask || !Global->Opt->Confirm.Exit || !Message(0,2,MSG(MQuit),Items, ARRAYSIZE(Items), nullptr, nullptr, &FarAskQuitId))
	{
		/* $ 29.12.2000 IS
		   + ���������, ��������� �� ��� ���������� �����. ���� ���, �� �� �������
		     �� ����.
		*/
		if (ExitAll() || Global->CloseFAR)
		{
			FilePanels *cp;

			if (!(cp = Global->CtrlObject->Cp())
			        || (!cp->LeftPanel->ProcessPluginEvent(FE_CLOSE,nullptr) && !cp->RightPanel->ProcessPluginEvent(FE_CLOSE,nullptr)))
				EndLoop=TRUE;
		}
		else
		{
			Global->CloseFARMenu=FALSE;
		}
	}
}

#if defined(FAR_ALPHA_VERSION)
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4717)
#ifndef _M_IA64
extern "C" void __ud2();
#else
extern "C" void __setReg(int, unsigned __int64);
#endif
#endif
static void Test_EXCEPTION_STACK_OVERFLOW(char* target)
{
	char Buffer[1024]; /* ����� ������� ������� */
	strcpy(Buffer, "zzzz");
	Test_EXCEPTION_STACK_OVERFLOW(Buffer);

	// "side effect" to prevent deletion of this function call due to C4718.
	Sleep(0);
}
#if defined(_MSC_VER)
#pragma warning( pop )
#endif
#endif


int Manager::ProcessKey(DWORD Key)
{
	int ret=FALSE;

	if (CurrentFrame)
	{
		DWORD KeyM=(Key&(~KEY_CTRLMASK));

		if (!((KeyM >= KEY_MACRO_BASE && KeyM <= KEY_MACRO_ENDBASE) || (KeyM >= KEY_OP_BASE && KeyM <= KEY_OP_ENDBASE))) // ��������� �����-����
		{
			switch (CurrentFrame->GetType())
			{
				case MODALTYPE_PANELS:
				{
					_ALGO(CleverSysLog clv(L"Manager::ProcessKey()"));
					_ALGO(SysLog(L"Key=%s",_FARKEY_ToName(Key)));
#ifndef NO_WRAPPER
					if (Global->CtrlObject->Cp()->ActivePanel->GetMode() == PLUGIN_PANEL)
					{
						PluginHandle *ph=(PluginHandle*)Global->CtrlObject->Cp()->ActivePanel->GetPluginHandle();
						if (ph && ph->pPlugin->IsOemPlugin())
							if (Global->CtrlObject->Cp()->ActivePanel->SendKeyToPlugin(Key,TRUE))
								return TRUE;
					}
#endif // NO_WRAPPER
					break;
				}
			#if 0
				case MODALTYPE_VIEWER:
					//if(((FileViewer*)CurrentFrame)->ProcessViewerInput(FrameManager->GetLastInputRecord()))
					//  return TRUE;
					break;
				case MODALTYPE_EDITOR:
					//if(((FileEditor*)CurrentFrame)->ProcessEditorInput(FrameManager->GetLastInputRecord()))
					//  return TRUE;
					break;
				case MODALTYPE_DIALOG:
					//((Dialog*)CurrentFrame)->CallDlgProc(DN_KEY,((Dialog*)CurrentFrame)->GetDlgFocusPos(),Key);
					break;
				case MODALTYPE_VMENU:
				case MODALTYPE_HELP:
				case MODALTYPE_COMBOBOX:
				case MODALTYPE_USER:
				case MODALTYPE_FINDFOLDER:
				default:
					break;
			#endif
			}
		}

#if defined(FAR_ALPHA_VERSION)

// ��� ��� ��� �������� ����������, ������� �� ������� :-)
		if (Key == KEY_CTRLALTAPPS || Key == KEY_RCTRLRALTAPPS || Key == KEY_CTRLRALTAPPS || Key == KEY_RCTRLALTAPPS)
		{
			struct __ECODE
			{
				NTSTATUS Code;
				const wchar_t *Name;
			} ECode[]=
			{
				{EXCEPTION_ACCESS_VIOLATION,L"Access Violation (Read)"},
				{EXCEPTION_ACCESS_VIOLATION,L"Access Violation (Write)"},
				{EXCEPTION_INT_DIVIDE_BY_ZERO,L"Divide by zero"},
				{EXCEPTION_ILLEGAL_INSTRUCTION,L"Illegal instruction"},
				{EXCEPTION_STACK_OVERFLOW,L"Stack Overflow"},
				{EXCEPTION_FLT_DIVIDE_BY_ZERO,L"Floating-point divide by zero"},
				{EXCEPTION_BREAKPOINT,L"Breakpoint"},
#ifdef _M_IA64
				{EXCEPTION_DATATYPE_MISALIGNMENT,L"Alignment fault (IA64 specific)",},
#endif
				/*
				        {EXCEPTION_FLT_OVERFLOW,"EXCEPTION_FLT_OVERFLOW"},
				        {EXCEPTION_SINGLE_STEP,"EXCEPTION_SINGLE_STEP",},
				        {EXCEPTION_ARRAY_BOUNDS_EXCEEDED,"EXCEPTION_ARRAY_BOUNDS_EXCEEDED",},
				        {EXCEPTION_FLT_DENORMAL_OPERAND,"EXCEPTION_FLT_DENORMAL_OPERAND",},
				        {EXCEPTION_FLT_INEXACT_RESULT,"EXCEPTION_FLT_INEXACT_RESULT",},
				        {EXCEPTION_FLT_INVALID_OPERATION,"EXCEPTION_FLT_INVALID_OPERATION",},
				        {EXCEPTION_FLT_STACK_CHECK,"EXCEPTION_FLT_STACK_CHECK",},
				        {EXCEPTION_FLT_UNDERFLOW,"EXCEPTION_FLT_UNDERFLOW",},
				        {EXCEPTION_INT_OVERFLOW,"EXCEPTION_INT_OVERFLOW",0},
				        {EXCEPTION_PRIV_INSTRUCTION,"EXCEPTION_PRIV_INSTRUCTION",0},
				        {EXCEPTION_IN_PAGE_ERROR,"EXCEPTION_IN_PAGE_ERROR",0},
				        {EXCEPTION_NONCONTINUABLE_EXCEPTION,"EXCEPTION_NONCONTINUABLE_EXCEPTION",0},
				        {EXCEPTION_INVALID_DISPOSITION,"EXCEPTION_INVALID_DISPOSITION",0},
				        {EXCEPTION_GUARD_PAGE,"EXCEPTION_GUARD_PAGE",0},
				        {EXCEPTION_INVALID_HANDLE,"EXCEPTION_INVALID_HANDLE",0},
				*/
			};
			static union
			{
				int     i;
				int     *iptr;
				double  d;
			} zero_const; //, refers;
			zero_const.i=0L;
			MenuItemEx ModalMenuItem;
			ModalMenuItem.Clear();
			VMenu2 ModalMenu(L"Test Exceptions",nullptr,0,ScrY-4);
			ModalMenu.SetFlags(VMENU_WRAPMODE);
			ModalMenu.SetPosition(-1,-1,0,0);

			std::for_each(CONST_RANGE(ECode, i)
			{
				ModalMenuItem.strName = i.Name;
				ModalMenu.AddItem(&ModalMenuItem);
			});

			int ExitCode=ModalMenu.Run();

			switch (ExitCode)
			{
				case -1:
					return TRUE;
				case 0:
					zero_const.i=*zero_const.iptr;
					break;
				case 1:
					*zero_const.iptr = 0;
					break;
				case 2:
					zero_const.i=1/zero_const.i;
					break;
				case 3:
#if defined(_MSC_VER)
#ifdef _M_IA64
#define __REG_IA64_IntR0 1024
					__setReg(__REG_IA64_IntR0, 666);
#else
					__ud2();
#endif
#elif defined(__GNUC__)
					asm("ud2");
#else
#error "Unsupported compiler"
#endif
					break;
				case 4:
					Test_EXCEPTION_STACK_OVERFLOW(nullptr);
					break;
				case 5:
					//refers.d = 1.0/zero_const.d;
					break;
				case 6:
					DebugBreak();
					break;
#ifdef _M_IA64
				case 7:
				{
					BYTE temp[10]={};
					double* val;
					val = (double*)(&temp[3]);
					printf("%lf\n", *val);
				}
#endif
			}

			Message(MSG_WARNING, 1, L"Test Exceptions failed", L"", ECode[ExitCode].Name, L"", MSG(MOk));
			return TRUE;
		}

#endif
		/*** ���� ����������������� ������ ! ***/

		/***   ������� ������ �����������    ***/
		switch (Key)
		{
			case KEY_ALT|KEY_NUMPAD0:
			case KEY_RALT|KEY_NUMPAD0:
			case KEY_ALTINS:
			case KEY_RALTINS:
			{
				RunGraber();
				return TRUE;
			}
			case KEY_CONSOLE_BUFFER_RESIZE:
				Sleep(1);
				ResizeAllFrame();
				return TRUE;
		}

		/*** � ��� ����� - ��� ���������! ***/
		if (!Global->IsProcessAssignMacroKey)
			// � ����� ������ ���� ����-�� �� ����� ��� ������� ���
		{
			bool scrollable = false;
			if ( Global->Opt->WindowMode )
			{
				int frame_type = CurrentFrame->GetType();
				scrollable = frame_type != MODALTYPE_EDITOR && frame_type != MODALTYPE_VIEWER;
			};

			switch (Key)
			{
				// <������� ����� ��������� ������������ Scroll>
				case KEY_CTRLALTUP:
				case KEY_RCTRLRALTUP:
				case KEY_CTRLRALTUP:
				case KEY_RCTRLALTUP:
					if(scrollable)
					{
						Global->Console->ScrollWindow(-1);
						return TRUE;
					}
					break;

				case KEY_CTRLALTDOWN:
				case KEY_RCTRLRALTDOWN:
				case KEY_CTRLRALTDOWN:
				case KEY_RCTRLALTDOWN:
					if(scrollable)
					{
						Global->Console->ScrollWindow(1);
						return TRUE;
					}
					break;

				case KEY_CTRLALTPGUP:
				case KEY_RCTRLRALTPGUP:
				case KEY_CTRLRALTPGUP:
				case KEY_RCTRLALTPGUP:
					if(scrollable)
					{
						Global->Console->ScrollWindow(-ScrY);
						return TRUE;
					}
					break;

				case KEY_CTRLALTHOME:
				case KEY_RCTRLRALTHOME:
				case KEY_CTRLRALTHOME:
				case KEY_RCTRLALTHOME:
					if(scrollable)
					{
						Global->Console->ScrollWindowToBegin();
						return TRUE;
					}
					break;

				case KEY_CTRLALTPGDN:
				case KEY_RCTRLRALTPGDN:
				case KEY_CTRLRALTPGDN:
				case KEY_RCTRLALTPGDN:
					if(scrollable)
					{
						Global->Console->ScrollWindow(ScrY);
						return TRUE;
					}
					break;

				case KEY_CTRLALTEND:
				case KEY_RCTRLRALTEND:
				case KEY_CTRLRALTEND:
				case KEY_RCTRLALTEND:
					if(scrollable)
					{
						Global->Console->ScrollWindowToEnd();
						return TRUE;
					}
					break;
				// </������� ����� ��������� ������������ Scroll>

				case KEY_CTRLW:
				case KEY_RCTRLW:
					ShowProcessList();
					return TRUE;

				case KEY_F11:
				{
					int TypeFrame=FrameManager->GetCurrentFrame()->GetType();
					static int reentry=0;
					if(!reentry && (TypeFrame == MODALTYPE_DIALOG || TypeFrame == MODALTYPE_VMENU))
					{
						++reentry;
						int r=CurrentFrame->ProcessKey(Key);
						--reentry;
						return r;
					}

					PluginsMenu();
					FrameManager->RefreshFrame();
					//_MANAGER(SysLog(-1));
					return TRUE;
				}
				case KEY_ALTF9:
				case KEY_RALTF9:
				{
					//_MANAGER(SysLog(1,"Manager::ProcessKey, KEY_ALTF9 pressed..."));
					Sleep(1);
					SetVideoMode();
					Sleep(1);

					/* � �������� ���������� Alt-F9 (� ���������� ������) � �������
					   ������� �������� WINDOW_BUFFER_SIZE_EVENT, ����������� �
					   ChangeVideoMode().
					   � ������ ���������� �������� ��� �� ���������� �� ������ ��������
					   ��������.
					*/
					if (Global->CtrlObject->Macro.IsExecuting())
					{
						int PScrX=ScrX;
						int PScrY=ScrY;
						Sleep(1);
						GetVideoMode(CurSize);

						if (PScrX+1 == CurSize.X && PScrY+1 == CurSize.Y)
						{
							//_MANAGER(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_NONE"));
							return TRUE;
						}
						else
						{
							PrevScrX=PScrX;
							PrevScrY=PScrY;
							//_MANAGER(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_CONSOLE_BUFFER_RESIZE"));
							Sleep(1);
							return ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
						}
					}

					//_MANAGER(SysLog(-1));
					return TRUE;
				}
				case KEY_F12:
				{
					int TypeFrame=FrameManager->GetCurrentFrame()->GetType();

					if (TypeFrame != MODALTYPE_HELP && TypeFrame != MODALTYPE_DIALOG && TypeFrame != MODALTYPE_VMENU)
					{
						DeactivateFrame(FrameMenu(),0);
						//_MANAGER(SysLog(-1));
						return TRUE;
					}

					break; // ������� F12 ������ �� �������
				}

				case KEY_CTRLALTSHIFTPRESS:
				case KEY_RCTRLALTSHIFTPRESS:
				{
					if (!(Global->Opt->CASRule&1) && Key == KEY_CTRLALTSHIFTPRESS)
						break;

					if (!(Global->Opt->CASRule&2) && Key == KEY_RCTRLALTSHIFTPRESS)
						break;

					if (!Global->Opt->OnlyEditorViewerUsed)
					{
						if (CurrentFrame->FastHide())
						{
							int isPanelFocus=CurrentFrame->GetType() == MODALTYPE_PANELS;

							if (isPanelFocus)
							{
								int LeftVisible=Global->CtrlObject->Cp()->LeftPanel->IsVisible();
								int RightVisible=Global->CtrlObject->Cp()->RightPanel->IsVisible();
								int CmdLineVisible=Global->CtrlObject->CmdLine->IsVisible();
								int KeyBarVisible=Global->CtrlObject->Cp()->MainKeyBar.IsVisible();
								Global->CtrlObject->CmdLine->ShowBackground();
								Global->CtrlObject->Cp()->LeftPanel->Hide0();
								Global->CtrlObject->Cp()->RightPanel->Hide0();

								switch (Global->Opt->PanelCtrlAltShiftRule)
								{
									case 0:
										Global->CtrlObject->CmdLine->Show();
										Global->CtrlObject->Cp()->MainKeyBar.Show();
										break;
									case 1:
										Global->CtrlObject->Cp()->MainKeyBar.Show();
										break;
								}

								WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:KEY_RCTRLALTSHIFTRELEASE);

								if (LeftVisible)      Global->CtrlObject->Cp()->LeftPanel->Show();

								if (RightVisible)     Global->CtrlObject->Cp()->RightPanel->Show();

								if (CmdLineVisible)   Global->CtrlObject->CmdLine->Show();

								if (KeyBarVisible)    Global->CtrlObject->Cp()->MainKeyBar.Show();
							}
							else
							{
								ImmediateHide();
								WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:KEY_RCTRLALTSHIFTRELEASE);
							}

							FrameManager->RefreshFrame();
						}

						return TRUE;
					}

					break;
				}
				case KEY_CTRLTAB:
				case KEY_RCTRLTAB:
				case KEY_CTRLSHIFTTAB:
				case KEY_RCTRLSHIFTTAB:

					if (CurrentFrame->GetCanLoseFocus())
					{
						DeactivateFrame(CurrentFrame,(Key==KEY_CTRLTAB||Key==KEY_RCTRLTAB)?1:-1);
					}
					else
						break;

					_MANAGER(SysLog(-1));
					return TRUE;
			}
		}

		CurrentFrame->UpdateKeyBar();
		CurrentFrame->ProcessKey(Key);
	}

	_MANAGER(SysLog(-1));
	return ret;
}

int Manager::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	int ret=FALSE;

//    _D(SysLog(1,"Manager::ProcessMouse()"));
	if (CurrentFrame)
		ret=CurrentFrame->ProcessMouse(MouseEvent);

//    _D(SysLog(L"Manager::ProcessMouse() ret=%i",ret));
	_MANAGER(SysLog(-1));
	return ret;
}

void Manager::PluginsMenu()
{
	_MANAGER(SysLog(1));
	int curType = CurrentFrame->GetType();

	if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER || curType == MODALTYPE_DIALOG || curType == MODALTYPE_VMENU)
	{
		/* 02.01.2002 IS
		   ! ����� ���������� ������ �� Shift-F1 � ���� �������� � ���������/������/�������
		   ! ���� �� ������ QVIEW ��� INFO ������ ����, �� �������, ��� ���
		     ����������� ����� � ��������� � ��������������� ���������� �������
		*/
		if (curType==MODALTYPE_PANELS)
		{
			int pType=Global->CtrlObject->Cp()->ActivePanel->GetType();

			if (pType==QVIEW_PANEL || pType==INFO_PANEL)
			{
				string strType, strCurFileName;
				Global->CtrlObject->Cp()->GetTypeAndName(strType, strCurFileName);

				if (!strCurFileName.empty())
				{
					DWORD Attr=apiGetFileAttributes(strCurFileName);

					// ���������� ������ ������� �����
					if (Attr!=INVALID_FILE_ATTRIBUTES && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
						curType=MODALTYPE_VIEWER;
				}
			}
		}

		// � ���������, ������ ��� ������� ������� ���� ������ �� Shift-F1
		const wchar_t *Topic=curType==MODALTYPE_EDITOR?L"Editor":
		                     curType==MODALTYPE_VIEWER?L"Viewer":
		                     curType==MODALTYPE_DIALOG?L"Dialog":nullptr;
		Global->CtrlObject->Plugins->CommandsMenu(curType,0,Topic);
	}

	_MANAGER(SysLog(-1));
}

bool Manager::IsPanelsActive(bool and_not_qview)
{
	if (FramePos>=0 && CurrentFrame)
	{
		return CurrentFrame->GetType() == MODALTYPE_PANELS &&
		   (!and_not_qview || ((FilePanels*)CurrentFrame)->ActivePanel->GetType()!=QVIEW_PANEL)
		;
	}
	else
	{
		return false;
	}
}

Frame* Manager::GetFrame(size_t Index) const
{
	if (Index>=static_cast<size_t>(Frames.size()) || Frames.empty())
	{
		return nullptr;
	}

	return Frames[Index];
}

int Manager::IndexOfStack(Frame *Frame)
{
	int Result = 0;
	return std::any_of(CONST_RANGE(ModalFrames, i)->bool
	{
		++Result;
		return Frame==i;
	})? Result - 1 : -1;
}

int Manager::IndexOf(Frame *Frame)
{
	int Result = 0;
	return std::any_of(CONST_RANGE(Frames, i)->bool
	{
		++Result;
		return Frame==i;
	})? Result - 1 : -1;
}

BOOL Manager::Commit()
{
	_MANAGER(CleverSysLog clv(L"Manager::Commit()"));
	_MANAGER(ManagerClass_Dump(L"ManagerClass"));
	int Result = false;

	if (DeletedFrame && (InsertedFrame||ExecutedFrame))
	{
		UpdateCommit();
		DeletedFrame = nullptr;
		InsertedFrame = nullptr;
		ExecutedFrame=nullptr;
		Result=true;
	}
	else if (ExecutedFrame)
	{
		ExecuteCommit();
		ExecutedFrame=nullptr;
		Result=true;
	}
	else if (DeletedFrame)
	{
		DeleteCommit();
		DeletedFrame = nullptr;
		Result=true;
	}
	else if (InsertedFrame)
	{
		InsertCommit();
		InsertedFrame = nullptr;
		Result=true;
	}
	else if (DeactivatedFrame)
	{
		DeactivateCommit();
		DeactivatedFrame=nullptr;
		Result=true;
	}
	else if (ActivatedFrame)
	{
		ActivateCommit();
		ActivatedFrame=nullptr;
		Result=true;
	}
	else if (RefreshedFrame)
	{
		RefreshCommit();
		RefreshedFrame=nullptr;
		Result=true;
	}
	else if (ModalizedFrame)
	{
		ModalizeCommit();
//    ModalizedFrame=nullptr;
		Result=true;
	}
	else if (UnmodalizedFrame)
	{
		UnmodalizeCommit();
//    UnmodalizedFrame=nullptr;
		Result=true;
	}

	if (Result)
	{
		Result=Commit();
	}

	return Result;
}

void Manager::DeactivateCommit()
{
	_MANAGER(CleverSysLog clv(L"Manager::DeactivateCommit()"));
	_MANAGER(SysLog(L"DeactivatedFrame=%p",DeactivatedFrame));

	/*$ 18.04.2002 skv
	  ���� ������ ������������, �� � �����-�� �� ���� � ��������������.
	*/
	if (!DeactivatedFrame || !ActivatedFrame)
	{
		return;
	}

	if (!ActivatedFrame)
	{
		_MANAGER("WARNING! !ActivatedFrame");
	}

	if (DeactivatedFrame)
	{
		DeactivatedFrame->OnChangeFocus(0);
	}

	int modalIndex=IndexOfStack(DeactivatedFrame);

	if (-1 != modalIndex && modalIndex == static_cast<int>(ModalFrames.size() - 1))
	{
		/*if (IsSemiModalBackFrame(ActivatedFrame))
		{ // �������� �� "���������" �������������� ������?
		  ModalStackCount--;
		}
		else
		{*/
		if (IndexOfStack(ActivatedFrame)==-1)
		{
			ModalFrames.back() = ActivatedFrame;
		}
		else
		{
			ModalFrames.pop_back();
		}

//    }
	}
}


void Manager::ActivateCommit()
{
	_MANAGER(CleverSysLog clv(L"Manager::ActivateCommit()"));
	_MANAGER(SysLog(L"ActivatedFrame=%p",ActivatedFrame));

	if (CurrentFrame==ActivatedFrame)
	{
		RefreshedFrame=ActivatedFrame;
		return;
	}

	int FrameIndex=IndexOf(ActivatedFrame);

	if (-1!=FrameIndex)
	{
		FramePos=FrameIndex;
	}

	/* 14.05.2002 SKV
	  ���� �� �������� ������������ ������������� �����,
	  �� ���� ��� ������� �� ���� ����� �������.
	*/

	std::any_of(RANGE(ModalFrames, i)->bool
	{
		if (i == ActivatedFrame)
		{
			std::swap(ModalFrames.back(), i);
			return true;
		}
		return false;
	});

	RefreshedFrame=CurrentFrame=ActivatedFrame;
	InterlockedExchange(&CurrentWindowType,CurrentFrame->GetType());
}

void Manager::UpdateCommit()
{
	_MANAGER(CleverSysLog clv(L"Manager::UpdateCommit()"));
	_MANAGER(SysLog(L"DeletedFrame=%p, InsertedFrame=%p, ExecutedFrame=%p",DeletedFrame,InsertedFrame, ExecutedFrame));

	if (ExecutedFrame)
	{
		DeleteCommit();
		ExecuteCommit();
		return;
	}

	int FrameIndex=IndexOf(DeletedFrame);

	if (-1!=FrameIndex)
	{
		ActivateFrame(Frames[FrameIndex] = InsertedFrame);
		ActivatedFrame->FrameToBack=CurrentFrame;
		DeleteCommit();
	}
	else
	{
		_MANAGER(SysLog(L"ERROR! DeletedFrame not found"));
	}
}

//! ������� DeletedFrame ��� ���� ��������!
//! ��������� ��������� ��������, (������ �� ����� �������������)
//! �� ������ � ��� ������, ���� �������� ����� ��� �� �������� �������.
void Manager::DeleteCommit()
{
	_MANAGER(CleverSysLog clv(L"Manager::DeleteCommit()"));
	_MANAGER(SysLog(L"DeletedFrame=%p",DeletedFrame));

	if (!DeletedFrame)
	{
		return;
	}

	// <ifDoubleInstance>
	//BOOL ifDoubI=ifDoubleInstance(DeletedFrame);
	// </ifDoubleInstance>
	int ModalIndex=IndexOfStack(DeletedFrame);

	if (ModalIndex!=-1)
	{
		/* $ 14.05.2002 SKV
		  ������� ����� � ������� ������ ��, ���
		  �����, � �� ������ �������.
		*/
		auto frame = std::find(ModalFrames.begin(), ModalFrames.end(), DeletedFrame);
		if (frame != ModalFrames.end())
			ModalFrames.erase(frame);

		if (!ModalFrames.empty())
		{
			ActivateFrame(ModalFrames.back());
		}
	}

	std::for_each(CONST_RANGE(Frames, i)
	{
		if (i->FrameToBack==DeletedFrame)
		{
			i->FrameToBack=Global->CtrlObject->Cp();
		}
	});

	int FrameIndex=IndexOf(DeletedFrame);

	if (-1!=FrameIndex)
	{
		Frames.erase(Frames.begin() + FrameIndex);

		if (FramePos >= static_cast<int>(Frames.size()))
		{
			FramePos=0;
		}

		if (DeletedFrame->FrameToBack==Global->CtrlObject->Cp())
		{
			ActivateFrame(Frames[FramePos]);
		}
		else
		{
			ActivateFrame(DeletedFrame->FrameToBack);
		}
	}

	DeletedFrame->OnDestroy();

	if (CurrentFrame==DeletedFrame)
	{
		CurrentFrame=0;
		InterlockedExchange(&CurrentWindowType,-1);
	}

	if (DeletedFrame->GetDynamicallyBorn())
	{
		_MANAGER(SysLog(L"delete DeletedFrame %p", DeletedFrame));

		/* $ 14.05.2002 SKV
		  ��� ��� � ����������� ������ ������ ����� ����
		  ������ commit, �� ���� ���������������.
		*/
		Frame *tmp=DeletedFrame;
		DeletedFrame=nullptr;
		delete tmp;
	}

	// ���������� �� ��, ��� � ActevateFrame �� ����� ��������� ���
	// �����������  ActivatedFrame
	if (!ModalFrames.empty())
	{
		ActivateFrame(ModalFrames.back());
	}
	else
	{
		ActivateFrame(FramePos);
	}
}

void Manager::InsertCommit()
{
	_MANAGER(CleverSysLog clv(L"Manager::InsertCommit()"));
	_MANAGER(SysLog(L"InsertedFrame=%p",InsertedFrame));

	if (InsertedFrame)
	{
		InsertedFrame->FrameToBack=CurrentFrame;
		Frames.emplace_back(InsertedFrame);

		if (!ActivatedFrame)
		{
			ActivatedFrame=InsertedFrame;
		}
	}
}

void Manager::RefreshCommit()
{
	_MANAGER(CleverSysLog clv(L"Manager::RefreshCommit()"));
	_MANAGER(SysLog(L"RefreshedFrame=%p",RefreshedFrame));

	if (!RefreshedFrame)
		return;

	if (IndexOf(RefreshedFrame)==-1 && IndexOfStack(RefreshedFrame)==-1)
		return;

	if (!RefreshedFrame->Locked())
	{
		if (!Global->IsRedrawFramesInProcess)
			RefreshedFrame->ShowConsoleTitle();

		if (RefreshedFrame)
			RefreshedFrame->Refresh();

		if (!RefreshedFrame)
			return;

		Global->CtrlObject->Macro.SetMode(RefreshedFrame->GetMacroMode());
	}

	if ((Global->Opt->ViewerEditorClock &&
	        (RefreshedFrame->GetType() == MODALTYPE_EDITOR ||
	         RefreshedFrame->GetType() == MODALTYPE_VIEWER))
	        || (Global->WaitInMainLoop && Global->Opt->Clock))
		ShowTime(1);
}

void Manager::ExecuteCommit()
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteCommit()"));
	_MANAGER(SysLog(L"ExecutedFrame=%p",ExecutedFrame));

	if (!ExecutedFrame)
	{
		return;
	}

	ModalFrames.emplace_back(ExecutedFrame);
	ActivatedFrame=ExecutedFrame;
}

/*$ 26.06.2001 SKV
  ��� ������ �� �������� ����������� ACTL_COMMIT
*/
BOOL Manager::PluginCommit()
{
	return Commit();
}

/* $ ������� ��� ���� CtrlAltShift OT */
void Manager::ImmediateHide()
{
	if (FramePos<0)
		return;

	// ������� ���������, ���� �� � ������������ ������ SaveScreen
	if (CurrentFrame->HasSaveScreen())
	{
		CurrentFrame->Hide();
		return;
	}

	// ������ ����������������, ������ ��� ������
	// �� ���������� ��������� �������, ����� �� �������.
	if (!ModalFrames.empty())
	{
		/* $ 28.04.2002 KM
		    ��������, � �� ��������� �� �������� ��� ������ �� �������
		    ���������� �����? � ���� ��, ������� User screen.
		*/
		if (ModalFrames.back()->GetType()==MODALTYPE_EDITOR || ModalFrames.back()->GetType()==MODALTYPE_VIEWER)
		{
			if (Global->CtrlObject->CmdLine)
				Global->CtrlObject->CmdLine->ShowBackground();
		}
		else
		{
			int UnlockCount=0;
			Global->IsRedrawFramesInProcess++;

			while (GetFrame(FramePos)->Locked())
			{
				GetFrame(FramePos)->Unlock();
				UnlockCount++;
			}

			RefreshFrame(GetFrame(FramePos));
			Commit();

			for (int i=0; i<UnlockCount; i++)
			{
				GetFrame(FramePos)->Lock();
			}

			if (ModalFrames.size() > 1)
			{
				for(auto i = ModalFrames.cbegin(); i != ModalFrames.cend() - 1; ++i)
				{
					if (!((*i)->FastHide() & CASR_HELP))
					{
						RefreshFrame(*i);
						Commit();
					}
					else
					{
						break;
					}
				}
			}

			/* $ 04.04.2002 KM
			   ���������� ��������� ������ � ��������� ������.
			   ���� �� ������������� ��������� ��������� �������
			   ��� ����������� ���� �������.
			*/
			Global->IsRedrawFramesInProcess--;
			CurrentFrame->ShowConsoleTitle();
		}
	}
	else
	{
		if (Global->CtrlObject->CmdLine)
			Global->CtrlObject->CmdLine->ShowBackground();
	}
}

void Manager::ModalizeCommit()
{
	CurrentFrame->Push(ModalizedFrame);
	ModalizedFrame=nullptr;
}

void Manager::UnmodalizeCommit()
{
	std::any_of(CONST_RANGE(Frames, i)
	{
		return i->RemoveModal(UnmodalizedFrame);
	});

	std::any_of(CONST_RANGE(ModalFrames, i)
	{
		return i->RemoveModal(UnmodalizedFrame);
	});

	UnmodalizedFrame=nullptr;
}

/*  ����� ResizeConsole ��� ���� NextModal �
    ���������� ������. KM
*/
void Manager::ResizeAllModal(Frame *ModalFrame)
{
	if (!ModalFrame->NextModal)
		return;

	Frame *iModal=ModalFrame->NextModal;

	while (iModal)
	{
		iModal->ResizeConsole();
		iModal=iModal->NextModal;
	}
}

void Manager::ResizeAllFrame()
{
	Global->ScrBuf->Lock();
	std::for_each(CONST_RANGE(Frames, i)
	{
		i->ResizeConsole();
	});

	std::for_each(CONST_RANGE(ModalFrames, i)
	{
		i->ResizeConsole();
		/* $ 13.04.2002 KM
		  - � ������ ����������� ��� NextModal...
		*/
		ResizeAllModal(i);
	});

	ImmediateHide();
	FrameManager->RefreshFrame();
	//RefreshFrame();
	Global->ScrBuf->Unlock();
}

void Manager::InitKeyBar()
{
	std::for_each(CONST_RANGE(Frames, i)
	{
		i->InitKeyBar();
	});
}

// ���������� top-����� ��� ��� �����, ���� � ������ ���� �������
Frame* Manager::GetTopModal()
{
	Frame *f=CurrentFrame, *fo=nullptr;

	while (f)
	{
		fo=f;
		f=f->GetTopModal();
	}

	if (!f)
		f=fo;

	return f;
}
