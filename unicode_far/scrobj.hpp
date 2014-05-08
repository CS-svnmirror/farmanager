#pragma once

/*
scrobj.hpp

Parent class ��� ���� screen objects
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

#include "bitflags.hpp"
#include "manager.hpp" //Manager::Key

class SaveScreen;

// ����� ������������ ������ ������� ���� (�.�. ����� 0x000000FF), ��������� �������� ����������� �������
enum
{
	FSCROBJ_VISIBLE              = 0x00000001,
	FSCROBJ_ENABLERESTORESCREEN  = 0x00000002,
	FSCROBJ_SETPOSITIONDONE      = 0x00000004,
	FSCROBJ_ISREDRAWING          = 0x00000008,   // ���� ������� Show?
};

class ScreenObject: NonCopyable
{
public:
	ScreenObject();
	ScreenObject(ScreenObject&& rhs);
	virtual ~ScreenObject();

	MOVE_OPERATOR_BY_SWAP(ScreenObject);

	void swap(ScreenObject& rhs) noexcept
	{
		std::swap(SaveScr, rhs.SaveScr);
		std::swap(pOwner, rhs.pOwner);
		std::swap(Flags, rhs.Flags);
		std::swap(nLockCount, rhs.nLockCount);
		std::swap(X1, rhs.X1);
		std::swap(Y1, rhs.Y1);
		std::swap(X2, rhs.X2);
		std::swap(Y2, rhs.Y2);
	}

	int ObjWidth() const {return X2 - X1 + 1;}
	int ObjHeight() const {return Y2 - Y1 + 1;}

	virtual int ProcessKey(const Manager::Key& Key) { return 0; }
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) { return 0; }

	virtual void Hide();
	virtual void Hide0();   // 15.07.2000 tran - dirty hack :(
	virtual void Show();
	virtual void ShowConsoleTitle() {};
	virtual void SetPosition(int X1,int Y1,int X2,int Y2);
	virtual void GetPosition(int& X1,int& Y1,int& X2,int& Y2) const;
	virtual void SetScreenPosition();
	virtual void ResizeConsole() {};
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) {return 0;}

	void Lock();
	void Unlock();
	bool Locked();
	void SavePrevScreen();
	void Redraw();
	bool IsVisible() const {return Flags.Check(FSCROBJ_VISIBLE);}
	void SetVisible(bool Visible) {Flags.Change(FSCROBJ_VISIBLE,Visible);}
	void SetRestoreScreenMode(bool Mode) {Flags.Change(FSCROBJ_ENABLERESTORESCREEN,Mode);}
	void SetOwner(ScreenObject *pOwner) {this->pOwner = pOwner;}
	ScreenObject* GetOwner() const {return pOwner;}

private:
	virtual void DisplayObject() = 0;

public:
	// KEEP ALIGNED!
	SaveScreen *SaveScr;

protected:
	// KEEP ALIGNED!
	ScreenObject *pOwner;
	BitFlags Flags;
	int nLockCount;
	SHORT X1, Y1, X2, Y2;
};


class ScreenObjectWithShadow:public ScreenObject
{
public:
	ScreenObjectWithShadow():
		ShadowSaveScr(nullptr)
	{}
	virtual ~ScreenObjectWithShadow();

	virtual void Hide() override;

	void Shadow(bool Full=false);

protected:
	SaveScreen *ShadowSaveScr;
};
