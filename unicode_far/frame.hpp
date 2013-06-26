#pragma once

/*
frame.hpp

����������� ���� (������� ����� ��� FilePanels, FileEditor, FileViewer)
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

#include "scrobj.hpp"
#include "macro.hpp"

class KeyBar;

enum MODALFRAME_TYPE
{
	MODALTYPE_PANELS,
	MODALTYPE_VIEWER,
	MODALTYPE_EDITOR,
	MODALTYPE_DIALOG,
	MODALTYPE_VMENU,
	MODALTYPE_HELP,
	MODALTYPE_COMBOBOX,
	MODALTYPE_FINDFOLDER,
	MODALTYPE_USER,
	MODALTYPE_GRABBER,
	MODALTYPE_HMENU,
};

class Frame: public ScreenObjectWithShadow
{
public:
	Frame();
	virtual ~Frame();

	virtual int GetCanLoseFocus(int DynamicMode=FALSE) const { return(CanLoseFocus); }
	virtual void SetExitCode(int Code) { ExitCode=Code; }
	virtual BOOL IsFileModified() const { return FALSE; }
	virtual const wchar_t *GetTypeName() {return L"[FarModal]";}
	virtual int GetTypeAndName(string &strType, string &strName) = 0;
	virtual int GetType() = 0;
	virtual void OnDestroy() {}  // ���������� ����� ������������ ����
	virtual void OnChangeFocus(int focus); // ���������� ��� ����� ������
	virtual void Refresh() {OnChangeFocus(1);}  // ������ �������������� :)
	virtual void InitKeyBar() {}
	virtual void RedrawKeyBar() { UpdateKeyBar(); }
	virtual FARMACROAREA GetMacroMode() { return MacroMode; }
	virtual int FastHide();
	virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0) { return Title; }
	virtual bool ProcessEvents() {return true;}
	virtual void ResizeConsole();

	void SetCanLoseFocus(int Mode) { CanLoseFocus=Mode; }
	int GetExitCode() { return ExitCode; }
	void SetKeyBar(KeyBar *FrameKeyBar);
	void UpdateKeyBar();
	int IsTitleBarVisible() const {return TitleBarVisible;}
	int IsTopFrame();
	Frame *GetTopModal() {return NextModal;}
	void SetDynamicallyBorn(int Born) {DynamicallyBorn=Born;}
	int GetDynamicallyBorn() {return DynamicallyBorn;}
	bool RemoveModal(const Frame *aFrame);
	bool HasSaveScreen();
	void SetFlags( DWORD flags ) { Flags.Set(flags); }

protected:
	int DynamicallyBorn;
	int CanLoseFocus;
	int ExitCode;
	int KeyBarVisible;
	int TitleBarVisible;
	KeyBar *FrameKeyBar;
	FARMACROAREA MacroMode;

private:
	void Push(Frame* Modalized);
	Frame *FrameToBack;
	Frame *NextModal;
	friend class Manager;
};
