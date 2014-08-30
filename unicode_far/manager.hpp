#pragma once

/*
manager.hpp

������������ ����� ����������� file panels, viewers, editors
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

class window;

class Manager: NonCopyable
{
public:
	struct Key
	{
		INPUT_RECORD Event;
		int FarKey;
		bool EventFilled;
		Key(): Event(), FarKey(0), EventFilled(false) {}
		explicit Key(int Key): Event(), FarKey(Key), EventFilled(false) {}
		//Key(INPUT_RECORD Key): EventFilled(true), Event(Key) {FarKey=0; /*FIXME*/ }
	};

	class MessageAbstract;

public:
	Manager();
	~Manager();

	// ��� ������� ����� ��������� �������� ����������� �� ������ ����� ����
	// ��� ��� �� ����������� ���������� � ���, ��� ����� ����� ������� � ������ ��� ��������� ������ Commit()
	void InsertWindow(window *NewWindow);
	void DeleteWindow(window *Deleted = nullptr);
	void DeleteWindow(int Index);
	void DeactivateWindow(window *Deactivated, int Direction);
	void ActivateWindow(window *Activated);
	void ActivateWindow(int Index);
	void RefreshWindow(window *Refreshed = nullptr);
	void RefreshWindow(int Index);
	void UpdateWindow(window* Old,window* New);
	void CallbackWindow(const std::function<void(void)>& Callback);
	//! ������� ��� ������� ��������� ����.
	void ExecuteWindow(window *Executed);
	//! ������ � ����� ���� ��������� �������
	void ExecuteModal(window *Executed);
	//! ��������� ����������� ���� � ��������� ������
	void ExecuteNonModal(const window *NonModal);
	void CloseAll();
	/* $ 29.12.2000 IS
	������ CloseAll, �� ��������� ����������� ����������� ������ � ����,
	���� ������������ ��������� ������������� ����.
	���������� TRUE, ���� ��� ������� � ����� �������� �� ����.
	*/
	BOOL ExitAll();
	size_t GetWindowCount()const { return m_windows.size(); }
	int  GetWindowCountByType(int Type);
	/*$ 26.06.2001 SKV
	��� ������ ����� ACTL_COMMIT
	*/
	void PluginCommit();
	int CountWindowsWithName(const string& Name, BOOL IgnoreCase = TRUE);
	bool IsPanelsActive(bool and_not_qview = false) const; // ������������ ��� ������� Global->WaitInMainLoop
	window* FindWindowByFile(int ModalType, const string& FileName, const wchar_t *Dir = nullptr);
	void EnterMainLoop();
	void ProcessMainLoop();
	void ExitMainLoop(int Ask);
	int ProcessKey(Key key);
	int ProcessMouse(const MOUSE_EVENT_RECORD *me);
	void PluginsMenu() const; // �������� ���� �� F11
	void SwitchToPanels();
	const INPUT_RECORD& GetLastInputRecord() const { return LastInputRecord; }
	void SetLastInputRecord(const INPUT_RECORD *Rec);
	void ResetLastInputRecord() { LastInputRecord.EventType = 0; }
	window *GetCurrentWindow() { return m_currentWindow; }
	window* GetWindow(size_t Index) const;
	int IndexOf(window* Window);
	int IndexOfStack(window* Window);
	window *GetBottomWindow() { return GetWindow(m_windowPos); }
	bool ManagerIsDown() const { return EndLoop; }
	bool ManagerStarted() const { return StartManager; }
	void InitKeyBar();
	void ExecuteModalEV(window *Executed) { ++ModalEVCount; ExecuteModal(Executed); --ModalEVCount; }
	bool InModalEV() const { return ModalEVCount != 0; }
	void ResizeAllWindows();
	size_t GetModalWindowCount() const { return m_nodalWindows.size(); }
	window* GetModalWindow(size_t index) const { return m_nodalWindows[index]; }

	void AddGlobalKeyHandler(const std::function<int(Key)>& Handler);

	static long GetCurrentWindowType() { return CurrentWindowType; }
	static bool ShowBackground();

	void UpdateMacroArea(void);
private:
#if defined(SYSLOG)
	friend void ManagerClass_Dump(const wchar_t *Title, FILE *fp);
#endif

	window *WindowMenu(); //    ������ void SelectWindow(); // show window menu (F12)
	bool HaveAnyWindow() const;
	bool OnlyDesktop() const;
	void Commit(void);         // ��������� ���������� �� ���������� � ����������� ����
	// ��� � ����� �������� ����, ���� ������ ���� �� ���������� ������� �� nullptr
	// �������, "����������� ����������" - Commit'a
	// ������ ���������� �� ������ �� ���� � �� ������ ����
	void InsertCommit(window* Param);
	void DeleteCommit(window* Param);
	void ActivateCommit(window* Param);
	void ActivateCommit(int Index);
	void RefreshCommit(window* Param);
	void DeactivateCommit(window* Param);
	void ExecuteCommit(window* Param);
	void UpdateCommit(window* Old,window* New);
	int GetModalExitCode() const;
	// BUGBUG, do we need this?
	void ImmediateHide();

	typedef void(Manager::*window_callback)(window*);

	void PushWindow(window* Param, window_callback Callback);
	void CheckAndPushWindow(window* Param, window_callback Callback);
	void ProcessWindowByPos(int Index, window_callback Callback);
	void RedeleteWindow(window *Deleted);

	INPUT_RECORD LastInputRecord;
	window *m_currentWindow;     // ������� ����. ��� ����� ���������� ��� � �����������, ��� � � ��������� ����������, ��� ����� �������� � ������� WindowManager->GetCurrentWindow();
	std::vector<window*> m_nodalWindows;
	std::vector<window*> m_windows;
	int  m_windowPos;           // ������ �������� ������������ ����. ��� �� ������ ��������� � CurrentWindow
	// ������� ����������� ���� ����� �������� � ������� WindowManager->GetBottomWindow();
	/* $ 15.05.2002 SKV
		��� ��� ���� ����������, ��� � �� ���� ��������,
		������ ������� ��������� editor/viewer'��.
		ĸ����� ���  ���� ������� ����� ������� ExecuteModal.
		� ��������� ������, ��� ��� ExecuteModal ����������
		1) �� ������ ��� ��������� ������� (��� ��� �� �������������),
		2) �� ������ ��� editor/viewer'��.
	*/
	int ModalEVCount;
	bool EndLoop;            // ������� ������ �� �����
	int ModalExitCode;
	bool StartManager;
	static long CurrentWindowType;
	std::list<std::unique_ptr<MessageAbstract>> m_Queue;
	std::vector<std::function<int(Key)>> m_GlobalKeyHandlers;
	std::map<window*,volatile bool*> m_Executed;
};
