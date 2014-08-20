#pragma once

/*
foldtree.hpp

����� �������� �� Alt-F10
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

#include "frame.hpp"
#include "keybar.hpp"
#include "macro.hpp"
#include "config.hpp"

class TreeList;
class EditControl;
class SaveScreen;

class FolderTree:public Frame
{
public:
	FolderTree(string &strResultFolder, int ModalMode, int IsStandalone = TRUE, bool IsFullScreen = true);
	virtual ~FolderTree();

	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual void InitKeyBar() override;
	virtual void OnChangeFocus(int focus) override; // ���������� ��� ����� ������
	virtual void SetScreenPosition() override;
	virtual void ResizeConsole() override;
	/* $ ������� ��� ���� CtrlAltShift OT */
	virtual bool CanFastHide() const override;
	virtual const wchar_t *GetTypeName() override { return L"[FolderTree]"; }
	virtual int GetTypeAndName(string &strType, string &strName) override;
	virtual int GetType() const override { return MODALTYPE_FINDFOLDER; }

private:
	virtual string GetTitle() const override { return string(); }
	virtual void DisplayObject() override;
	void DrawEdit();
	void SetCoords();

	TreeList *Tree;
	std::unique_ptr<EditControl> FindEdit;
	// ������
	KeyBar TreeKeyBar;
	int ModalMode;
	bool IsFullScreen;
	int IsStandalone;
	string strNewFolder;
	string strLastName;
};
