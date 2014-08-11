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

class Frame;

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
	// ��� ��� �� ����������� ���������� � ���, ��� ����� ����� ������� � �������� ��� ��������� ������ Commit()
	void InsertFrame(Frame *NewFrame);
	void DeleteFrame(Frame *Deleted = nullptr);
	void DeleteFrame(int Index);
	void DeactivateFrame(Frame *Deactivated, int Direction);
	void ActivateFrame(Frame *Activated);
	void ActivateFrame(int Index);
	void RefreshFrame(Frame *Refreshed = nullptr);
	void RefreshFrame(int Index);
	void UpdateFrame(Frame* Old,Frame* New);
	void CallbackFrame(const std::function<void(void)>& Callback);
	//! ������� ��� ������� ��������� �������.
	void ExecuteFrame(Frame *Executed);
	//! ������ � ����� ���� ��������� �������
	void ExecuteModal(Frame *Executed = nullptr);
	//! ��������� ����������� ����� � ��������� ������
	void ExecuteNonModal(const Frame *NonModal);
	void CloseAll();
	/* $ 29.12.2000 IS
	������ CloseAll, �� ��������� ����������� ����������� ������ � ����,
	���� ������������ ��������� ������������� ����.
	���������� TRUE, ���� ��� ������� � ����� �������� �� ����.
	*/
	BOOL ExitAll();
	size_t GetFrameCount()const { return Frames.size(); }
	int  GetFrameCountByType(int Type);
	/*$ 26.06.2001 SKV
	��� ������ ����� ACTL_COMMIT
	*/
	void PluginCommit();
	int CountFramesWithName(const string& Name, BOOL IgnoreCase = TRUE);
	bool IsPanelsActive(bool and_not_qview = false) const; // ������������ ��� ������� Global->WaitInMainLoop
	Frame* FindFrameByFile(int ModalType, const string& FileName, const wchar_t *Dir = nullptr);
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
	Frame *GetCurrentFrame() { return CurrentFrame; }
	Frame* GetFrame(size_t Index) const;
	int IndexOf(Frame *Frame);
	int IndexOfStack(Frame *Frame);
	void ImmediateHide();
	Frame *GetBottomFrame() { return GetFrame(FramePos); }
	bool ManagerIsDown() const { return EndLoop; }
	bool ManagerStarted() const { return StartManager; }
	void InitKeyBar();
	void ExecuteModalEV(Frame *Executed = nullptr) { ++ModalEVCount; ExecuteModal(Executed); --ModalEVCount; }
	bool InModalEV() const { return ModalEVCount != 0; }
	void ResizeAllFrame();
	size_t GetModalStackCount() const { return ModalFrames.size(); }
	Frame* GetModalFrame(size_t index) const { return ModalFrames[index]; }
	/* $ 13.04.2002 KM
	��� ������ ResizeConsole ��� ���� NextModal �
	���������� ������.
	*/
	static long GetCurrentWindowType() { return CurrentWindowType; }
	static bool ShowBackground();

private:
#if defined(SYSLOG)
	friend void ManagerClass_Dump(const wchar_t *Title, FILE *fp);
#endif

	Frame *FrameMenu(); //    ������ void SelectFrame(); // show window menu (F12)
	bool HaveAnyFrame() const;
	bool OnlyDesktop() const;
	void Commit(void);         // ��������� ���������� �� ���������� � ������� � ����� �������
	// ��� � ����� �������� ����, ���� ������ ���� �� ���������� ������� �� nullptr
	// �������, "����������� ����������" - Commit'a
	// ������ ���������� �� ������ �� ���� � �� ������ ����
	void InsertCommit(Frame* Param);
	void DeleteCommit(Frame* Param);
	void ActivateCommit(Frame* Param);
	void ActivateCommit(int Index);
	void RefreshCommit(Frame* Param);
	void DeactivateCommit(Frame* Param);
	void ExecuteCommit(Frame* Param);
	void UpdateCommit(Frame* Old,Frame* New);
	int GetModalExitCode() const;

	typedef void(Manager::*frame_callback)(Frame*);

	void PushFrame(Frame* Param, frame_callback Callback);
	void CheckAndPushFrame(Frame* Param, frame_callback Callback);
	void ProcessFrameByPos(int Index, frame_callback Callback);
	void RedeleteFrame(Frame *Deleted);

	INPUT_RECORD LastInputRecord;
	Frame *CurrentFrame;     // ������� �����. �� ����� ����������� ��� � ����������� �������, ��� � � ��������� �����, ��� ����� �������� � ������� FrameManager->GetCurrentFrame();
	std::vector<Frame*> ModalFrames;     // ���� ��������� �������
	std::vector<Frame*> Frames;       // ������� ��������� �������
	int  FramePos;           // ������ ������� ������������ ������. �� �� ������ ��������� � CurrentFrame
	// ������� ����������� ����� ����� �������� � ������� FrameManager->GetBottomFrame();
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
};
