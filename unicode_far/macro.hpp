#pragma once

/*
macro.hpp

�������
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

#include "tvar.hpp"
#include "noncopyable.hpp"

struct GetMacroData;

// Macro Const
enum {
	constMsX          = 0,
	constMsY          = 1,
	constMsButton     = 2,
	constMsCtrlState  = 3,
	constMsEventFlags = 4,
	constMsLastCtrlState = 5,
	constMsLAST       = 6,
};

enum MACRODISABLEONLOAD
{
	MDOL_ALL            = 0x80000000, // �������� ��� ������� ��� ��������
	MDOL_AUTOSTART      = 0x00000001, // �������� �������������� �������
};

typedef unsigned __int64 MACROFLAGS_MFLAGS;
static const MACROFLAGS_MFLAGS
	// public flags, read from/saved to config
	MFLAGS_PUBLIC_MASK             =0x00000000FFFFFFFF,
	MFLAGS_ENABLEOUTPUT            =0x0000000000000001, // �� ��������� ���������� ������ �� ����� ���������� �������
	MFLAGS_NOSENDKEYSTOPLUGINS     =0x0000000000000002, // �� ���������� �������� ������� �� ����� ������/��������������� �������
	MFLAGS_RUNAFTERFARSTART        =0x0000000000000008, // ���� ������ ����������� ��� ������ ����
	MFLAGS_EMPTYCOMMANDLINE        =0x0000000000000010, // ���������, ���� ��������� ����� �����
	MFLAGS_NOTEMPTYCOMMANDLINE     =0x0000000000000020, // ���������, ���� ��������� ����� �� �����
	MFLAGS_EDITSELECTION           =0x0000000000000040, // ���������, ���� ���� ��������� � ���������
	MFLAGS_EDITNOSELECTION         =0x0000000000000080, // ���������, ���� ���� ��� ��������� � ���������
	MFLAGS_SELECTION               =0x0000000000000100, // ��������:  ���������, ���� ���� ���������
	MFLAGS_PSELECTION              =0x0000000000000200, // ���������: ���������, ���� ���� ���������
	MFLAGS_NOSELECTION             =0x0000000000000400, // ��������:  ���������, ���� ���� ��� ���������
	MFLAGS_PNOSELECTION            =0x0000000000000800, // ���������: ���������, ���� ���� ��� ���������
	MFLAGS_NOFILEPANELS            =0x0000000000001000, // ��������:  ���������, ���� ��� ���������� ������
	MFLAGS_PNOFILEPANELS           =0x0000000000002000, // ���������: ���������, ���� ��� ���������� ������
	MFLAGS_NOPLUGINPANELS          =0x0000000000004000, // ��������:  ���������, ���� ��� �������� ������
	MFLAGS_PNOPLUGINPANELS         =0x0000000000008000, // ���������: ���������, ���� ��� �������� ������
	MFLAGS_NOFOLDERS               =0x0000000000010000, // ��������:  ���������, ���� ������� ������ "����"
	MFLAGS_PNOFOLDERS              =0x0000000000020000, // ���������: ���������, ���� ������� ������ "����"
	MFLAGS_NOFILES                 =0x0000000000040000, // ��������:  ���������, ���� ������� ������ "�����"
	MFLAGS_PNOFILES                =0x0000000000080000, // ���������: ���������, ���� ������� ������ "�����"

	// private flags, for runtime purposes only
	MFLAGS_PRIVATE_MASK            =0xFFFFFFFF00000000,
	MFLAGS_POSTFROMPLUGIN          =0x0000000100000000; // ������������������ ������ �� ���


// ���� �������� ��� KeyMacro::GetCurRecord()
enum MACRORECORDANDEXECUTETYPE
{
	MACROMODE_NOMACRO          =0,  // �� � ������ �����
	MACROMODE_EXECUTING        =1,  // ����������: ��� �������� ������� ����
	MACROMODE_EXECUTING_COMMON =2,  // ����������: � ��������� ������� ����
	MACROMODE_RECORDING        =3,  // ������: ��� �������� ������� ����
	MACROMODE_RECORDING_COMMON =4,  // ������: � ��������� ������� ����
};

inline const wchar_t* GetMacroLanguage(FARKEYMACROFLAGS Flags)
{
	switch(Flags & KMFLAGS_LANGMASK)
	{
		default:
		case KMFLAGS_LUA:        return L"lua";
		case KMFLAGS_MOONSCRIPT: return L"moonscript";
	}
}

struct MacroPanelSelect
{
	__int64 Index;
	TVar    *Item;
	int     Action;
	DWORD   ActionFlags;
	int     Mode;
};

class Dialog;

class KeyMacro: NonCopyable
{
public:
	KeyMacro();
	~KeyMacro();

	int  IsRecording() const { return m_Recording; }
	int  IsExecuting() const;
	int  IsDisableOutput() const;
	bool IsHistoryDisable(int TypeHistory) const;
	DWORD SetHistoryDisableMask(DWORD Mask);
	DWORD GetHistoryDisableMask() const;
	void SetMode(FARMACROAREA Mode) { m_Mode=Mode; }
	FARMACROAREA GetMode() const { return m_Mode; }
	bool Load(bool InitedRAM=true,bool LoadAll=true);
	bool Save(bool always);
	// �������� ������ � ������� (���������� ������)
	int GetCurRecord() const;
	int ProcessEvent(const FAR_INPUT_RECORD *Rec);
	int GetKey();
	int PeekKey() const;
	bool GetMacroKeyInfo(const string& strMode,int Pos,string &strKeyName,string &strDescription);
	static void SetMacroConst(int ConstIndex, __int64 Value);
	// ������� ������ �� ���������� �������
	void SendDropProcess();
	bool CheckWaitKeyFunc() const;

	bool MacroExists(int Key, FARMACROAREA CheckMode, bool UseCommon);
	void RunStartMacro();
	bool AddMacro(const GUID& PluginId,const MacroAddMacro* Data);
	int DelMacro(const GUID& PluginId,void* Id);
	bool PostNewMacro(const wchar_t* lang,const wchar_t* PlainText,UINT64 Flags=0,DWORD AKey=0) { return PostNewMacro(0,lang,PlainText,Flags,AKey); }
	bool ParseMacroString(const wchar_t* lang,const wchar_t* Sequence,bool onlyCheck,bool skipFile);
	bool ExecuteString(MacroExecuteString *Data);
	void GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc) const;
	intptr_t CallFar(intptr_t OpCode, FarMacroCall* Data);
	void CallPluginSynchro(MacroPluginReturn *Params, FarMacroCall **Target, int *Boolean);
	const wchar_t *eStackAsString() const { return varTextDate; }
	void SuspendMacros(bool Suspend) { Suspend ? ++m_InternalInput : --m_InternalInput; }

	class MacroState;
	class MacroRecord;

private:
	bool CallMacroPlugin(OpenMacroPluginInfo* Info);
	int AssignMacroKey(DWORD& MacroKey,UINT64& Flags);
	intptr_t AssignMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t ParamMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	int GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src=nullptr,const wchar_t *Descr=nullptr);
	void InitInternalVars(bool InitedRAM=true);
	MacroRecord* CheckCurMacro();
	MacroRecord* GetCurMacro();
	const MacroRecord* GetCurMacro() const;
	MacroRecord* GetTopMacro();
	void RemoveCurMacro();
	void RestoreMacroChar() const;
	bool PostNewMacro(int macroId,const wchar_t* lang,const wchar_t* PlainText,UINT64 Flags,DWORD AKey);
	void PushState(bool withClip);
	void PopState(bool withClip);
	bool LM_GetMacro(GetMacroData* Data, FARMACROAREA Mode, const string& TextKey, bool UseCommon, bool CheckOnly);
	void LM_ProcessRecordedMacro(FARMACROAREA Mode, const string& TextKey, const string& Code, MACROFLAGS_MFLAGS Flags, const string& Description);
	void CallPlugin(MacroPluginReturn *mpr, FarMacroValue *fmv, bool CallPluginRules);

	FARMACROAREA m_Mode;
	std::unique_ptr<MacroState> m_CurState;
	std::stack<MacroState, std::vector<MacroState>> m_StateStack;
	MACRORECORDANDEXECUTETYPE m_Recording;
	string m_RecCode;
	string m_RecDescription;
	FARMACROAREA m_RecMode;
	FARMACROAREA StartMode; //FIXME
	string m_LastErrorStr;
	int m_LastErrorLine;
	int m_InternalInput;
	int m_MacroPluginIsRunning;
	int m_DisableNested;
	int m_WaitKey;
	const wchar_t* varTextDate;
};

inline bool IsMenuArea(int Area){return Area==MACROAREA_MAINMENU || Area==MACROAREA_MENU || Area==MACROAREA_DISKS || Area==MACROAREA_USERMENU || Area==MACROAREA_SHELLAUTOCOMPLETION || Area==MACROAREA_DIALOGAUTOCOMPLETION;}
