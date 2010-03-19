#pragma once

/*
help.hpp

������
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

class CallBackStack;

#define HelpBeginLink L'<'
#define HelpEndLink L'>'
#define HelpFormatLink L"<%s\\>%s"

#define HELPMODE_CLICKOUTSIDE  0x20000000 // ���� ������� ���� ��� �����?

struct StackHelpData
{
	DWORD Flags;                  // �����
	int   TopStr;                 // ����� ������� ������� ������ ����
	int   CurX,CurY;              // ���������� (???)

	string strHelpMask;           // �������� �����
	string strHelpPath;           // ���� � ������
	string strHelpTopic;         // ������� �����
	string strSelTopic;          // ���������� ����� (???)

	void Clear()
	{
		Flags=0;
		TopStr=0;
		CurX=CurY=0;
		strHelpMask.Clear();
		strHelpPath.Clear();
		strHelpTopic.Clear();
		strSelTopic.Clear();
	}
};

enum HELPDOCUMENTSHELPTYPE
{
	HIDX_PLUGINS,                 // ������ ��������
	HIDX_DOCUMS,                  // ������ ����������
};

enum
{
	FHELPOBJ_ERRCANNOTOPENHELP  = 0x80000000,
};

class Help:public Frame
{
	private:
		BOOL  ErrorHelp;            // TRUE - ������! �������� - ��� ������ ������
		SaveScreen *TopScreen;      // ������� ���������� ��� ������
		KeyBar      HelpKeyBar;     // ������
		CallBackStack *Stack;       // ���� ��������
		string  strFullHelpPathName;

		StackHelpData StackData;
		wchar_t *HelpData;             // "����" � ������.
		int   StrCount;             // ���������� ����� � ����
		int   FixCount;             // ���������� ����� ���������������� �������
		int   FixSize;              // ������ ���������������� �������
		int   TopicFound;           // TRUE - ����� ������
		int   IsNewTopic;           // ��� ����� �����?
		int   MouseDown;

		string strCtrlColorChar;    // CtrlColorChar - �����! ��� �����������-
		//   ������� - ��� ���������
		int   CurColor;             // CurColor - ������� ���� ���������
		int   CtrlTabSize;          // CtrlTabSize - �����! ������ ���������

		int   PrevMacroMode;        // ���������� ����� �������

		string strCurPluginContents; // ������ PluginContents (��� ����������� � ���������)

		DWORD LastStartPos;
		DWORD StartPos;

		string strCtrlStartPosChar;

#if defined(WORK_HELP_FIND)
	private:
		DWORD LastSearchPos;
		unsigned char LastSearchStr[SEARCHSTRINGBUFSIZE];
		int LastSearchCase,LastSearchWholeWords,LastSearchReverse;

	private:
		int Search(int Next);
		void KeepInitParameters();
#endif

	private:
		virtual void DisplayObject();
		int  ReadHelp(const wchar_t *Mask=nullptr);
		void AddLine(const wchar_t *Line);
		void AddTitle(const wchar_t *Title);
		void HighlightsCorrection(wchar_t *Str);
		void FastShow();
		void DrawWindowFrame();
		void OutString(const wchar_t *Str);
		int  StringLen(const wchar_t *Str);
		void CorrectPosition();
		int  IsReferencePresent();
		void MoveToReference(int Forward,int CurScreen);
		void ReadDocumentsHelp(int TypeIndex);
		int  JumpTopic(const wchar_t *JumpTopic=nullptr);

	public:
		Help(const wchar_t *Topic,const wchar_t *Mask=nullptr,DWORD Flags=0);
		virtual ~Help();

	public:
		virtual void Hide();
		virtual int  ProcessKey(int Key);
		virtual int  ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual void InitKeyBar();
		BOOL GetError() {return ErrorHelp;}
		virtual void SetScreenPosition();
		virtual void OnChangeFocus(int focus); // ���������� ��� ����� ������
		virtual void ResizeConsole();

		virtual int  FastHide(); // ������� ��� ���� CtrlAltShift

		virtual const wchar_t *GetTypeName() {return L"[Help]";}
		virtual int GetTypeAndName(string &strType, string &strName);
		virtual int GetType() { return MODALTYPE_HELP; }

		virtual __int64 VMProcess(int OpCode,void *vParam,__int64 iParam);

		static string &MkTopic(INT_PTR PluginNumber,const wchar_t *HelpTopic,string &strTopic);
};
