#pragma once

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
#include "language.hpp"

struct export_name
{
	const wchar_t* UName;
	const char* AName;
	void* Address;
};

ENUM(ExceptFunctionsType);

struct ExecuteStruct
{
	ExecuteStruct& operator =(intptr_t value) { Result = value; return *this; }
	ExecuteStruct& operator =(HANDLE value) { Result = reinterpret_cast<intptr_t>(value); return *this; }
	operator intptr_t() const { return Result; } 
	operator HANDLE() const { return reinterpret_cast<HANDLE>(Result); }

	ExceptFunctionsType id;

	intptr_t Default, Result;
};

#define EXECUTE_FUNCTION(function) \
{ \
	Prologue(); \
	++Activity; \
	try \
	{ \
		function; \
	} \
	catch (SException &e) \
	{ \
		if (xfilter(es.id, e.GetInfo(), this, 0) == EXCEPTION_EXECUTE_HANDLER) \
		{ \
			m_owner->UnloadPlugin(this, es.id); \
			es.Result = es.Default; \
			Global->ProcessException=FALSE; \
		} \
		else \
		{ \
			throw; \
		} \
	} \
	--Activity; \
	Epilogue(); \
}

#define FUNCTION(id) reinterpret_cast<id##Prototype>(Exports[id])

#define WA(string) {L##string, string}

enum EXPORTS_ENUM
{
	iGetGlobalInfo,
	iSetStartupInfo,
	iOpen,
	iClosePanel,
	iGetPluginInfo,
	iGetOpenPanelInfo,
	iGetFindData,
	iFreeFindData,
	iGetVirtualFindData,
	iFreeVirtualFindData,
	iSetDirectory,
	iGetFiles,
	iPutFiles,
	iDeleteFiles,
	iMakeDirectory,
	iProcessHostFile,
	iSetFindList,
	iConfigure,
	iExitFAR,
	iProcessPanelInput,
	iProcessPanelEvent,
	iProcessEditorEvent,
	iCompare,
	iProcessEditorInput,
	iProcessViewerEvent,
	iProcessDialogEvent,
	iProcessSynchroEvent,
	iProcessConsoleInput,
	iAnalyse,
	iGetCustomData,
	iFreeCustomData,
	iCloseAnalyse,

	iOpenFilePlugin,
	iGetMinFarVersion,
	i_LAST
};

class PluginManager;

class Plugin
{
public:
	Plugin(PluginManager *owner, const string& ModuleName);
	virtual ~Plugin();

	static bool FindExport(const char* ExportName);

	virtual bool GetGlobalInfo(GlobalInfo *Info);
	virtual bool SetStartupInfo();
	virtual bool CheckMinFarVersion();
	virtual HANDLE Open(int OpenFrom, const GUID& Guid, intptr_t Item);
	virtual HANDLE OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, size_t DataSize, int OpMode);
	virtual int SetFindList(HANDLE hPlugin, const PluginPanelItem *PanelItem, size_t ItemsNumber);
	virtual int GetFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, int OpMode);
	virtual int GetVirtualFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, const string& Path);
	virtual int SetDirectory(HANDLE hPlugin, const string& Dir, int OpMode, struct UserDataItem *UserData=nullptr);
	virtual int GetFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, const wchar_t** DestPath, int OpMode);
	virtual int PutFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, int OpMode);
	virtual int DeleteFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode);
	virtual int MakeDirectory(HANDLE hPlugin, const wchar_t **Name, int OpMode);
	virtual int ProcessHostFile(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode);
	virtual int ProcessKey(HANDLE hPlugin, const INPUT_RECORD *Rec, bool);
	virtual int ProcessPanelEvent(HANDLE hPlugin, int Event, PVOID Param);
	virtual int Compare(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned long Mode);
	virtual int GetCustomData(const wchar_t *FilePath, wchar_t **CustomData);
	virtual void FreeCustomData(wchar_t *CustomData);
	virtual void GetOpenPanelInfo(HANDLE hPlugin, OpenPanelInfo *Info);
	virtual void FreeFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool FreeUserData);
	virtual void FreeVirtualFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber);
	virtual void ClosePanel(HANDLE hPlugin);
	virtual int ProcessEditorInput(const INPUT_RECORD *D);
	virtual int ProcessEditorEvent(int Event, PVOID Param,int EditorID);
	virtual int ProcessViewerEvent(int Event, PVOID Param,int ViewerID);
	virtual int ProcessDialogEvent(int Event, FarDialogEvent *Param);
	virtual int ProcessSynchroEvent(int Event, PVOID Param);
	virtual int ProcessConsoleInput(ProcessConsoleInputInfo *Info);
	virtual HANDLE Analyse(const AnalyseInfo *Info);
	virtual void CloseAnalyse(HANDLE hHandle);
	virtual bool GetPluginInfo(PluginInfo *pi);
	virtual int Configure(const GUID& Guid);
	virtual void ExitFAR(const ExitInfo *Info);
#ifndef NO_WRAPPER
	virtual bool IsOemPlugin() const { return false; }
#endif // NO_WRAPPER
	virtual const string& GetHotkeyName() const { return m_strGuid; }

	virtual bool InitLang(const string& Path) { return PluginLang.Init(Path); }
	void CloseLang() { PluginLang.Close(); }

	#define HAS_FUNCTION(Name) bool Has##Name() const { return Exports[i##Name] != nullptr; }

	HAS_FUNCTION(GetGlobalInfo)
	HAS_FUNCTION(Open)
	HAS_FUNCTION(MakeDirectory)
	HAS_FUNCTION(DeleteFiles)
	HAS_FUNCTION(PutFiles)
	HAS_FUNCTION(GetFiles)
	HAS_FUNCTION(SetStartupInfo)
	HAS_FUNCTION(ClosePanel)
	HAS_FUNCTION(GetPluginInfo)
	HAS_FUNCTION(GetOpenPanelInfo)
	HAS_FUNCTION(GetFindData)
	HAS_FUNCTION(FreeFindData)
	HAS_FUNCTION(GetVirtualFindData)
	HAS_FUNCTION(FreeVirtualFindData)
	HAS_FUNCTION(SetDirectory)
	HAS_FUNCTION(ProcessHostFile)
	HAS_FUNCTION(SetFindList)
	HAS_FUNCTION(Configure)
	HAS_FUNCTION(ExitFAR)
	HAS_FUNCTION(ProcessPanelInput)
	HAS_FUNCTION(ProcessPanelEvent)
	HAS_FUNCTION(ProcessEditorEvent)
	HAS_FUNCTION(Compare)
	HAS_FUNCTION(ProcessEditorInput)
	HAS_FUNCTION(ProcessViewerEvent)
	HAS_FUNCTION(ProcessDialogEvent)
	HAS_FUNCTION(ProcessSynchroEvent)
	HAS_FUNCTION(ProcessConsoleInput)
	HAS_FUNCTION(Analyse)
	HAS_FUNCTION(GetCustomData)
	HAS_FUNCTION(FreeCustomData)

	HAS_FUNCTION(OpenFilePlugin)
	HAS_FUNCTION(GetMinFarVersion)

	#undef HAS_FUNCTION

	const string &GetModuleName() const { return m_strModuleName; }
	const string& GetCacheName() const  { return m_strCacheName; }
	const string& GetTitle() const { return strTitle; }
	const string& GetDescription() const { return strDescription; }
	const string& GetAuthor() const { return strAuthor; }
	const VersionInfo& GetVersion() { return PluginVersion; }
	const VersionInfo& GetMinFarVersion() { return MinFarVersion; }
	const string& GetVersionString() { return VersionString; }
	const GUID& GetGUID() const { return m_Guid; }
	const wchar_t *GetMsg(LNGID nID) const { return PluginLang.GetMsg(nID); }

	bool CheckWorkFlags(DWORD flags) const { return WorkFlags.Check(flags)==TRUE; }
	DWORD GetWorkFlags() const { return WorkFlags.Flags(); }
	DWORD GetFuncFlags() const { return FuncFlags.Flags(); }

	bool Load();
	int Unload(bool bExitFAR = false);
	bool LoadData();
	bool LoadFromCache(const FAR_FIND_DATA &FindData);
	bool SaveToCache();
	bool IsPanelPlugin();
	bool Active() {return Activity != 0;}

protected:
	virtual void Prologue() {};
	virtual void Epilogue() {};

	void* Exports[i_LAST];
	const export_name* ExportsNames;

	PluginManager *m_owner; //BUGBUG
	Language PluginLang;
	size_t Activity;
	bool bPendingRemove;

private:
	void InitExports();
	void ClearExports();
	void SetGuid(const GUID& Guid);

	string strTitle;
	string strDescription;
	string strAuthor;

	string m_strModuleName;
	string m_strCacheName;

	BitFlags WorkFlags;      // ������� ����� �������� �������
	BitFlags FuncFlags;      // ������� ����� ������ ����.������� �������

	HMODULE m_hModule;

	VersionInfo MinFarVersion;
	VersionInfo PluginVersion;

	string VersionString;

	GUID m_Guid;
	string m_strGuid;

	friend class PluginManager;
};

extern PluginStartupInfo NativeInfo;
extern FarStandardFunctions NativeFSF;
