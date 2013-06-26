#pragma once

/*
keybar.hpp

Keybar
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

//   ������ �����
enum
{
	KBL_MAIN=0,	// ������� ������������� .lng �����
	KBL_SHIFT,
	KBL_ALT,
	KBL_CTRL,
	KBL_ALTSHIFT,
	KBL_CTRLSHIFT,
	KBL_CTRLALT,
	KBL_CTRLALTSHIFT,

	KBL_GROUP_COUNT
};

enum KEYBARAREA
{
	KBA_SHELL,
	KBA_INFO,
	KBA_TREE,
	KBA_QUICKVIEW,
	KBA_FOLDERTREE,
	KBA_EDITOR,
	KBA_VIEWER,
	KBA_HELP,

	KBA_COUNT
};

const size_t KEY_COUNT = 12;

class KeyBar: public ScreenObject
{
public:
	KeyBar();
	virtual ~KeyBar(){}

	virtual int ProcessKey(int Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;

	void SetLabels(LNGID StartIndex);
	void SetCustomLabels(KEYBARAREA Area);
	void Change(int Group,const wchar_t *NewStr,int Pos);
	void Change(const wchar_t *NewStr,int Pos) {Change(KBL_MAIN, NewStr, Pos);}
	size_t Change(const KeyBarTitles* Kbt);

	void RedrawIfChanged();

private:
	virtual void DisplayObject() override;
	void ClearKeyTitles(bool Custom);

	struct titles
	{
		string Title;
		string CustomTitle;
	};

	std::vector<std::vector<titles>> Items;
	string strLanguage;
	KEYBARAREA CustomArea;
	ScreenObject *Owner;
	int AltState,CtrlState,ShiftState;
	bool CustomLabelsReaded;
};
