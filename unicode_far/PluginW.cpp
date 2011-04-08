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

#include "plugins.hpp"
#include "plugapi.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "flink.hpp"
#include "scantree.hpp"
#include "chgprior.hpp"
#include "constitle.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "rdrwdsk.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "udlist.hpp"
#include "farexcpt.hpp"
#include "fileedit.hpp"
#include "RefreshFrameManager.hpp"
#include "plclass.hpp"
#include "PluginW.hpp"
#include "registry.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "processname.hpp"
#include "mix.hpp"
#include "lasterror.hpp"
#include "FarGuid.hpp"
#include "synchro.hpp"
#include "farversion.hpp"
#include "colormix.hpp"
#include "setcolor.hpp"

static const wchar_t *wszReg_Preload=L"Preload";
static const wchar_t *wszReg_MinFarVersion=L"MinFarVersion";
static const wchar_t *wszReg_Version=L"Version";
static const wchar_t *wszReg_Guid=L"Guid";
static const wchar_t *wszReg_Title=L"Title";
static const wchar_t *wszReg_Description=L"Description";
static const wchar_t *wszReg_Author=L"Author";

static const wchar_t wszReg_GetGlobalInfo[]=L"GetGlobalInfoW";
static const wchar_t wszReg_OpenPanel[]=L"OpenW";
static const wchar_t wszReg_SetFindList[]=L"SetFindListW";
static const wchar_t wszReg_ProcessEditorInput[]=L"ProcessEditorInputW";
static const wchar_t wszReg_ProcessEditorEvent[]=L"ProcessEditorEventW";
static const wchar_t wszReg_ProcessViewerEvent[]=L"ProcessViewerEventW";
static const wchar_t wszReg_ProcessDialogEvent[]=L"ProcessDialogEventW";
static const wchar_t wszReg_ProcessSynchroEvent[]=L"ProcessSynchroEventW";
#if defined(PROCPLUGINMACROFUNC)
static const wchar_t wszReg_ProcessMacroFunc[]=L"ProcessMacroFuncW";
#endif
static const wchar_t wszReg_SetStartupInfo[]=L"SetStartupInfoW";
static const wchar_t wszReg_ClosePanel[]=L"ClosePanelW";
static const wchar_t wszReg_GetPluginInfo[]=L"GetPluginInfoW";
static const wchar_t wszReg_GetOpenPanelInfo[]=L"GetOpenPanelInfoW";
static const wchar_t wszReg_GetFindData[]=L"GetFindDataW";
static const wchar_t wszReg_FreeFindData[]=L"FreeFindDataW";
static const wchar_t wszReg_GetVirtualFindData[]=L"GetVirtualFindDataW";
static const wchar_t wszReg_FreeVirtualFindData[]=L"FreeVirtualFindDataW";
static const wchar_t wszReg_SetDirectory[]=L"SetDirectoryW";
static const wchar_t wszReg_GetFiles[]=L"GetFilesW";
static const wchar_t wszReg_PutFiles[]=L"PutFilesW";
static const wchar_t wszReg_DeleteFiles[]=L"DeleteFilesW";
static const wchar_t wszReg_MakeDirectory[]=L"MakeDirectoryW";
static const wchar_t wszReg_ProcessHostFile[]=L"ProcessHostFileW";
static const wchar_t wszReg_Configure[]=L"ConfigureW";
static const wchar_t wszReg_ExitFAR[]=L"ExitFARW";
static const wchar_t wszReg_ProcessKey[]=L"ProcessKeyW";
static const wchar_t wszReg_ProcessEvent[]=L"ProcessEventW";
static const wchar_t wszReg_Compare[]=L"CompareW";
static const wchar_t wszReg_Analyse[] = L"AnalyseW";
static const wchar_t wszReg_GetCustomData[] = L"GetCustomDataW";
static const wchar_t wszReg_FreeCustomData[] = L"FreeCustomDataW";

static const char NFMP_GetGlobalInfo[]="GetGlobalInfoW";
static const char NFMP_OpenPanel[]="OpenW";
static const char NFMP_SetFindList[]="SetFindListW";
static const char NFMP_ProcessEditorInput[]="ProcessEditorInputW";
static const char NFMP_ProcessEditorEvent[]="ProcessEditorEventW";
static const char NFMP_ProcessViewerEvent[]="ProcessViewerEventW";
static const char NFMP_ProcessDialogEvent[]="ProcessDialogEventW";
static const char NFMP_ProcessSynchroEvent[]="ProcessSynchroEventW";
#if defined(PROCPLUGINMACROFUNC)
static const char NFMP_ProcessMacroFunc[]="ProcessMacroFuncW";
#endif
static const char NFMP_SetStartupInfo[]="SetStartupInfoW";
static const char NFMP_ClosePanel[]="ClosePanelW";
static const char NFMP_GetPluginInfo[]="GetPluginInfoW";
static const char NFMP_GetOpenPanelInfo[]="GetOpenPanelInfoW";
static const char NFMP_GetFindData[]="GetFindDataW";
static const char NFMP_FreeFindData[]="FreeFindDataW";
static const char NFMP_GetVirtualFindData[]="GetVirtualFindDataW";
static const char NFMP_FreeVirtualFindData[]="FreeVirtualFindDataW";
static const char NFMP_SetDirectory[]="SetDirectoryW";
static const char NFMP_GetFiles[]="GetFilesW";
static const char NFMP_PutFiles[]="PutFilesW";
static const char NFMP_DeleteFiles[]="DeleteFilesW";
static const char NFMP_MakeDirectory[]="MakeDirectoryW";
static const char NFMP_ProcessHostFile[]="ProcessHostFileW";
static const char NFMP_Configure[]="ConfigureW";
static const char NFMP_ExitFAR[]="ExitFARW";
static const char NFMP_ProcessKey[]="ProcessKeyW";
static const char NFMP_ProcessEvent[]="ProcessEventW";
static const char NFMP_Compare[]="CompareW";
static const char NFMP_Analyse[]="AnalyseW";
static const char NFMP_GetCustomData[]="GetCustomDataW";
static const char NFMP_FreeCustomData[]="FreeCustomDataW";


static BOOL PrepareModulePath(const wchar_t *ModuleName)
{
	string strModulePath;
	strModulePath = ModuleName;
	CutToSlash(strModulePath); //??
	return FarChDir(strModulePath);
}

static void CheckScreenLock()
{
	if (ScrBuf.GetLockCount() > 0 && !CtrlObject->Macro.PeekKey())
	{
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
	}
}

static size_t WINAPI FarKeyToName(int Key,wchar_t *KeyText,size_t Size)
{
	string strKT;

	if (!KeyToText(Key,strKT))
		return 0;

	size_t len = strKT.GetLength();

	if (Size && KeyText)
	{
		if (Size <= len) len = Size-1;

		wmemcpy(KeyText, strKT.CPtr(), len);
		KeyText[len] = 0;
	}
	else if (KeyText) *KeyText = 0;

	return (len+1);
}

int WINAPI KeyNameToKeyW(const wchar_t *Name)
{
	string strN(Name);
	return KeyNameToKey(strN);
}

#define GetPluginNumber(Id) (CtrlObject?CtrlObject->Plugins.PluginGuidToPluginNumber(*Id):-1)

static int WINAPI FarGetPluginDirListW(const GUID* PluginId,HANDLE hPlugin,
                               const wchar_t *Dir,struct PluginPanelItem **pPanelItem,
                               int *pItemsNumber)
{
	return FarGetPluginDirList(GetPluginNumber(PluginId),hPlugin,Dir,pPanelItem,pItemsNumber);
}

static int WINAPI FarMenuFnW(const GUID* PluginId,int X,int Y,int MaxHeight,
                     unsigned __int64 Flags,const wchar_t *Title,const wchar_t *Bottom,
                     const wchar_t *HelpTopic,const FarKey *BreakKeys,int *BreakCode,
                     const struct FarMenuItem *Item, size_t ItemsNumber)
{
	return FarMenuFn(GetPluginNumber(PluginId),X,Y,MaxHeight,Flags,Title,Bottom,HelpTopic,BreakKeys,BreakCode,Item,ItemsNumber);
}

static int WINAPI FarMessageFnW(const GUID* PluginId,unsigned __int64 Flags,
                        const wchar_t *HelpTopic,const wchar_t * const *Items,size_t ItemsNumber,
                        int ButtonsNumber)
{
  return FarMessageFn(GetPluginNumber(PluginId),Flags,HelpTopic,Items,ItemsNumber,ButtonsNumber);
}

static int WINAPI FarInputBoxW(const GUID* PluginId,const wchar_t *Title,const wchar_t *Prompt,
                       const wchar_t *HistoryName,const wchar_t *SrcText,
                       wchar_t *DestText,int DestLength,
                       const wchar_t *HelpTopic,unsigned __int64 Flags)
{
	return FarInputBox(GetPluginNumber(PluginId),Title,Prompt,HistoryName,SrcText,DestText,DestLength,HelpTopic,Flags);
}

static BOOL WINAPI farColorDialog(const GUID* PluginId, COLORDIALOGFLAGS Flags, struct FarColor *Color)
{
	BOOL Result = FALSE;
	if (!FrameManager->ManagerIsDown())
	{
		WORD Clr = Colors::FarColorToColor(*Color);
		if(GetColorDialog(Clr, true, false))
		{
			Colors::ColorToFarColor(Clr, *Color);
			Result = TRUE;
		}
	}
	return Result;
}
static INT_PTR WINAPI FarAdvControlW(const GUID* PluginId, ADVANCED_CONTROL_COMMANDS Command, void *Param)
{
	if (ACTL_SYNCHRO==Command) //must be first
	{
		PluginSynchroManager.Synchro(true, *PluginId, Param);
		return 0;
	}
	if (ACTL_GETWINDOWTYPE==Command)
	{
		WindowType* info=(WindowType*)Param;
		if (info&&info->StructSize>=sizeof(WindowType))
		{
			long type=CurrentWindowType;
			switch(type)
			{
				case WTYPE_PANELS:
				case WTYPE_VIEWER:
				case WTYPE_EDITOR:
				case WTYPE_DIALOG:
				case WTYPE_VMENU:
				case WTYPE_HELP:
					info->Type=type;
					return TRUE;
			}
		}
		return FALSE;
	}
	return FarAdvControl(GetPluginNumber(PluginId),Command,Param);
}

static HANDLE WINAPI FarDialogInitW(const GUID* PluginId, const GUID* Id, int X1, int Y1, int X2, int Y2,
                            const wchar_t *HelpTopic, struct FarDialogItem *Item,
                            unsigned int ItemsNumber, DWORD Reserved, unsigned __int64 Flags,
                            FARWINDOWPROC Proc, INT_PTR Param)
{
	return FarDialogInit(GetPluginNumber(PluginId),Id,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,Proc,Param);
}

static const wchar_t* WINAPI FarGetMsgFnW(const GUID* PluginId,int MsgId)
{
	return FarGetMsgFn(GetPluginNumber(PluginId),MsgId);
}

PluginW::PluginW(PluginManager *owner, const wchar_t *lpwszModuleName):
	m_owner(owner),
	m_strModuleName(lpwszModuleName),
	m_strCacheName(lpwszModuleName),
	m_hModule(nullptr)
	//more initialization here!!!
{
	wchar_t *p = m_strCacheName.GetBuffer();
	while (*p)
	{
		if (*p == L'\\')
			*p = L'/';

		p++;
	}
	m_strCacheName.ReleaseBuffer();
	ClearExports();
	SetGuid(FarGuid);
}

PluginW::~PluginW()
{
	Lang.Close();
}


bool PluginW::LoadFromCache(const FAR_FIND_DATA_EX &FindData)
{
	string strRegKey;
	strRegKey.Format(FmtPluginsCache_PluginS, m_strCacheName.CPtr());

	if (CheckRegKey(strRegKey))
	{
		if (GetRegKey(strRegKey,wszReg_Preload,0) == 1)   //PF_PRELOAD plugin, skip cache
		{
			WorkFlags.Set(PIWF_PRELOADED);
			return false;
		}

		{
			string strPluginID, strCurPluginID;
			strCurPluginID.Format(
			    L"%I64x%x%x",
			    FindData.nFileSize,
			    FindData.ftCreationTime.dwLowDateTime,
			    FindData.ftLastWriteTime.dwLowDateTime
			);
			GetRegKey(strRegKey, L"ID", strPluginID, L"");

			if (StrCmp(strPluginID, strCurPluginID) )   //���������� �� ���������?
				return false;
		}
		GetRegKey(strRegKey,wszReg_MinFarVersion,reinterpret_cast<LPBYTE>(&MinFarVersion),reinterpret_cast<const BYTE*>(&FAR_VERSION),sizeof(MinFarVersion));
		VersionInfo Default = {};
		GetRegKey(strRegKey,wszReg_Version,reinterpret_cast<LPBYTE>(&PluginVersion),reinterpret_cast<const BYTE*>(&Default),sizeof(PluginVersion));
		GetRegKey(strRegKey,wszReg_Guid,m_strGuid,L"");
		SetGuid(StrToGuid(m_strGuid,m_Guid)?m_Guid:FarGuid);
		GetRegKey(strRegKey,wszReg_Title,strTitle,L"");
		GetRegKey(strRegKey,wszReg_Description,strDescription,L"");
		GetRegKey(strRegKey,wszReg_Author,strAuthor,L"");

		strRegKey += L"\\Exports";
		pOpenPanelW=(PLUGINOPENPANELW)(INT_PTR)GetRegKey(strRegKey,wszReg_OpenPanel,0);
		pSetFindListW=(PLUGINSETFINDLISTW)(INT_PTR)GetRegKey(strRegKey,wszReg_SetFindList,0);
		pProcessEditorInputW=(PLUGINPROCESSEDITORINPUTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessEditorInput,0);
		pProcessEditorEventW=(PLUGINPROCESSEDITOREVENTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessEditorEvent,0);
		pProcessViewerEventW=(PLUGINPROCESSVIEWEREVENTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessViewerEvent,0);
		pProcessDialogEventW=(PLUGINPROCESSDIALOGEVENTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessDialogEvent,0);
		pProcessSynchroEventW=(PLUGINPROCESSSYNCHROEVENTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessSynchroEvent,0);
#if defined(PROCPLUGINMACROFUNC)
		pProcessMacroFuncW=(PLUGINPROCESSMACROFUNCW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessMacroFunc,0);
#endif
		pConfigureW=(PLUGINCONFIGUREW)(INT_PTR)GetRegKey(strRegKey,wszReg_Configure,0);
		pAnalyseW=(PLUGINANALYSEW)(INT_PTR)GetRegKey(strRegKey, wszReg_Analyse, 0);
		pGetCustomDataW=(PLUGINGETCUSTOMDATAW)(INT_PTR)GetRegKey(strRegKey, wszReg_GetCustomData, 0);
		WorkFlags.Set(PIWF_CACHED); //too much "cached" flags
		return true;
	}

	return false;
}

bool PluginW::SaveToCache()
{
	if (pGetGlobalInfoW ||
	        pGetPluginInfoW ||
	        pOpenPanelW ||
	        pSetFindListW ||
	        pProcessEditorInputW ||
	        pProcessEditorEventW ||
	        pProcessViewerEventW ||
	        pProcessDialogEventW ||
	        pProcessSynchroEventW ||
#if defined(PROCPLUGINMACROFUNC)
	        pProcessMacroFuncW ||
#endif
	        pAnalyseW ||
	        pGetCustomDataW
	   )
	{
		PluginInfo Info;
		GetPluginInfo(&Info);
		string strRegKey;
		strRegKey.Format(FmtPluginsCache_PluginS, m_strCacheName.CPtr());
		DeleteKeyTree(strRegKey);
		{
			bool bPreload = (Info.Flags & PF_PRELOAD);
			SetRegKey(strRegKey, wszReg_Preload, bPreload?1:0);
			WorkFlags.Change(PIWF_PRELOADED, bPreload);

			if (bPreload)
				return true;
		}
		{
			string strCurPluginID;
			FAR_FIND_DATA_EX fdata;
			apiGetFindDataEx(m_strModuleName, fdata);
			strCurPluginID.Format(
			    L"%I64x%x%x",
			    fdata.nFileSize,
			    fdata.ftCreationTime.dwLowDateTime,
			    fdata.ftLastWriteTime.dwLowDateTime
			);
			SetRegKey(strRegKey, L"ID", strCurPluginID);
		}

		for (int i = 0; i < Info.DiskMenu.Count; i++)
		{
			string strValue;
			strValue.Format(FmtDiskMenuStringD, i);
			SetRegKey(strRegKey, strValue, Info.DiskMenu.Strings[i]);
			strValue.Format(FmtDiskMenuGuidD, i);
			SetRegKey(strRegKey, strValue, GuidToStr(Info.DiskMenu.Guids[i]));
		}

		for (int i = 0; i < Info.PluginMenu.Count; i++)
		{
			string strValue;
			strValue.Format(FmtPluginMenuStringD, i);
			SetRegKey(strRegKey, strValue, Info.PluginMenu.Strings[i]);
			strValue.Format(FmtPluginMenuGuidD, i);
			SetRegKey(strRegKey, strValue, GuidToStr(Info.PluginMenu.Guids[i]));
		}

		for (int i = 0; i < Info.PluginConfig.Count; i++)
		{
			string strValue;
			strValue.Format(FmtPluginConfigStringD, i);
			SetRegKey(strRegKey,strValue,Info.PluginConfig.Strings[i]);
			strValue.Format(FmtPluginConfigGuidD, i);
			SetRegKey(strRegKey,strValue,GuidToStr(Info.PluginConfig.Guids[i]));
		}

		SetRegKey(strRegKey, L"CommandPrefix", NullToEmpty(Info.CommandPrefix));
		SetRegKey64(strRegKey, L"Flags", Info.Flags);
		SetRegKey(strRegKey, wszReg_MinFarVersion, reinterpret_cast<const BYTE*>(&MinFarVersion),sizeof(MinFarVersion));
		SetRegKey(strRegKey, wszReg_Guid, m_strGuid);
		SetRegKey(strRegKey,wszReg_Version, reinterpret_cast<const BYTE*>(&PluginVersion),sizeof(PluginVersion));
		SetRegKey(strRegKey,wszReg_Title, strTitle);
		SetRegKey(strRegKey,wszReg_Description, strDescription);
		SetRegKey(strRegKey,wszReg_Author, strAuthor);

		strRegKey += L"\\Exports";
		SetRegKey(strRegKey, wszReg_OpenPanel, pOpenPanelW!=nullptr);
		SetRegKey(strRegKey, wszReg_SetFindList, pSetFindListW!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessEditorInput, pProcessEditorInputW!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessEditorEvent, pProcessEditorEventW!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessViewerEvent, pProcessViewerEventW!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessDialogEvent, pProcessDialogEventW!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessSynchroEvent, pProcessSynchroEventW!=nullptr);
#if defined(PROCPLUGINMACROFUNC)
		SetRegKey(strRegKey, wszReg_ProcessMacroFunc, pProcessMacroFuncW!=nullptr);
#endif
		SetRegKey(strRegKey, wszReg_Configure, pConfigureW!=nullptr);
		SetRegKey(strRegKey, wszReg_Analyse, pAnalyseW!=nullptr);
		SetRegKey(strRegKey, wszReg_GetCustomData, pGetCustomDataW!=nullptr);
		return true;
	}

	return false;
}

bool PluginW::LoadData(void)
{
	if (WorkFlags.Check(PIWF_DONTLOADAGAIN))
		return false;

	if (WorkFlags.Check(PIWF_DATALOADED))
		return true;

	if (m_hModule)
		return true;

	if (!m_hModule)
	{
		string strCurPath, strCurPlugDiskPath;
		wchar_t Drive[]={0,L' ',L':',0}; //������ 0, ��� ������� ����, ��� ������� ������� ������!
		apiGetCurrentDirectory(strCurPath);

		if (IsLocalPath(m_strModuleName))  // ���� ������ ��������� ����, ��...
		{
			Drive[0] = L'=';
			Drive[1] = m_strModuleName.At(0);
			apiGetEnvironmentVariable(Drive,strCurPlugDiskPath);
		}

		PrepareModulePath(m_strModuleName);
		m_hModule = LoadLibraryEx(m_strModuleName,nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
		GuardLastError Err;
		FarChDir(strCurPath);

		if (Drive[0]) // ������ �� (���������� ���������) �������
			SetEnvironmentVariable(Drive,strCurPlugDiskPath);
	}

	if (!m_hModule)
	{
		if (!Opt.LoadPlug.SilentLoadPlugin) //������ � PluginSet
		{
			SetMessageHelp(L"ErrLoadPlugin");
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MPlgLoadPluginError),m_strModuleName,MSG(MOk));
		}

		//���� �� �������� ��������� ����� � �� ������ ����� ��������� ������������.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		return false;
	}

	WorkFlags.Clear(PIWF_CACHED);
	pGetGlobalInfoW=(PLUGINGETGLOBALINFOW)GetProcAddress(m_hModule,NFMP_GetGlobalInfo);
	pSetStartupInfoW=(PLUGINSETSTARTUPINFOW)GetProcAddress(m_hModule,NFMP_SetStartupInfo);
	pOpenPanelW=(PLUGINOPENPANELW)GetProcAddress(m_hModule,NFMP_OpenPanel);
	pClosePanelW=(PLUGINCLOSEPANELW)GetProcAddress(m_hModule,NFMP_ClosePanel);
	pGetPluginInfoW=(PLUGINGETPLUGININFOW)GetProcAddress(m_hModule,NFMP_GetPluginInfo);
	pGetOpenPanelInfoW=(PLUGINGETOPENPANELINFOW)GetProcAddress(m_hModule,NFMP_GetOpenPanelInfo);
	pGetFindDataW=(PLUGINGETFINDDATAW)GetProcAddress(m_hModule,NFMP_GetFindData);
	pFreeFindDataW=(PLUGINFREEFINDDATAW)GetProcAddress(m_hModule,NFMP_FreeFindData);
	pGetVirtualFindDataW=(PLUGINGETVIRTUALFINDDATAW)GetProcAddress(m_hModule,NFMP_GetVirtualFindData);
	pFreeVirtualFindDataW=(PLUGINFREEVIRTUALFINDDATAW)GetProcAddress(m_hModule,NFMP_FreeVirtualFindData);
	pSetDirectoryW=(PLUGINSETDIRECTORYW)GetProcAddress(m_hModule,NFMP_SetDirectory);
	pGetFilesW=(PLUGINGETFILESW)GetProcAddress(m_hModule,NFMP_GetFiles);
	pPutFilesW=(PLUGINPUTFILESW)GetProcAddress(m_hModule,NFMP_PutFiles);
	pDeleteFilesW=(PLUGINDELETEFILESW)GetProcAddress(m_hModule,NFMP_DeleteFiles);
	pMakeDirectoryW=(PLUGINMAKEDIRECTORYW)GetProcAddress(m_hModule,NFMP_MakeDirectory);
	pProcessHostFileW=(PLUGINPROCESSHOSTFILEW)GetProcAddress(m_hModule,NFMP_ProcessHostFile);
	pSetFindListW=(PLUGINSETFINDLISTW)GetProcAddress(m_hModule,NFMP_SetFindList);
	pConfigureW=(PLUGINCONFIGUREW)GetProcAddress(m_hModule,NFMP_Configure);
	pExitFARW=(PLUGINEXITFARW)GetProcAddress(m_hModule,NFMP_ExitFAR);
	pProcessKeyW=(PLUGINPROCESSKEYW)GetProcAddress(m_hModule,NFMP_ProcessKey);
	pProcessEventW=(PLUGINPROCESSEVENTW)GetProcAddress(m_hModule,NFMP_ProcessEvent);
	pCompareW=(PLUGINCOMPAREW)GetProcAddress(m_hModule,NFMP_Compare);
	pProcessEditorInputW=(PLUGINPROCESSEDITORINPUTW)GetProcAddress(m_hModule,NFMP_ProcessEditorInput);
	pProcessEditorEventW=(PLUGINPROCESSEDITOREVENTW)GetProcAddress(m_hModule,NFMP_ProcessEditorEvent);
	pProcessViewerEventW=(PLUGINPROCESSVIEWEREVENTW)GetProcAddress(m_hModule,NFMP_ProcessViewerEvent);
	pProcessDialogEventW=(PLUGINPROCESSDIALOGEVENTW)GetProcAddress(m_hModule,NFMP_ProcessDialogEvent);
	pProcessSynchroEventW=(PLUGINPROCESSSYNCHROEVENTW)GetProcAddress(m_hModule,NFMP_ProcessSynchroEvent);
#if defined(PROCPLUGINMACROFUNC)
	pProcessMacroFuncW=(PLUGINPROCESSMACROFUNCW)GetProcAddress(m_hModule,NFMP_ProcessMacroFunc);
#endif
	pAnalyseW=(PLUGINANALYSEW)GetProcAddress(m_hModule, NFMP_Analyse);
	pGetCustomDataW=(PLUGINGETCUSTOMDATAW)GetProcAddress(m_hModule, NFMP_GetCustomData);
	pFreeCustomDataW=(PLUGINFREECUSTOMDATAW)GetProcAddress(m_hModule, NFMP_FreeCustomData);

	GlobalInfo Info;
	if(GetGlobalInfo(&Info) &&
		Info.StructSize &&
		Info.Title && *Info.Title &&
		Info.Description && *Info.Description &&
		Info.Author && *Info.Author)
	{
		MinFarVersion = Info.MinFarVersion;
		PluginVersion = Info.Version;
		strTitle = Info.Title;
		strDescription = Info.Description;
		strAuthor = Info.Author;
		SetGuid(Info.Guid);
		WorkFlags.Set(PIWF_DATALOADED);
		return true;
	}
	Unload();
	//���� �� �������� ��������� ����� � �� ������ ����� ��������� ������������.
	WorkFlags.Set(PIWF_DONTLOADAGAIN);
	return false;
}

bool PluginW::Load()
{
	if (WorkFlags.Check(PIWF_DONTLOADAGAIN))
		return false;

	if (!WorkFlags.Check(PIWF_DATALOADED)&&!LoadData())
		return false;

	if (FuncFlags.Check(PICFF_LOADED))
		return true;

	bool bUnloaded = false;

	if (CheckMinFarVersion(bUnloaded) && SetStartupInfo(bUnloaded))
	{
		FuncFlags.Set(PICFF_LOADED);
		SaveToCache();
		return true;
	}
	if (!bUnloaded)
	{
		Unload();
	}

	//���� �� �������� ��������� ����� � �� ������ ����� ��������� ������������.
	WorkFlags.Set(PIWF_DONTLOADAGAIN);
	return false;
}

void CreatePluginStartupInfo(Plugin *pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF)
{
	static PluginStartupInfo StartupInfo={0};
	static FarStandardFunctions StandardFunctions={0};

	// ��������� ��������� StandardFunctions ���� ���!!!
	if (!StandardFunctions.StructSize)
	{
		StandardFunctions.StructSize=sizeof(StandardFunctions);
		StandardFunctions.sprintf=swprintf;
		StandardFunctions.snprintf=_snwprintf;
		StandardFunctions.sscanf=swscanf;
		StandardFunctions.qsort=FarQsort;
		StandardFunctions.qsortex=FarQsortEx;
		StandardFunctions.atoi=FarAtoi;
		StandardFunctions.atoi64=FarAtoi64;
		StandardFunctions.itoa=FarItoa;
		StandardFunctions.itoa64=FarItoa64;
		StandardFunctions.bsearch=FarBsearch;
		StandardFunctions.LIsLower = farIsLower;
		StandardFunctions.LIsUpper = farIsUpper;
		StandardFunctions.LIsAlpha = farIsAlpha;
		StandardFunctions.LIsAlphanum = farIsAlphaNum;
		StandardFunctions.LUpper = farUpper;
		StandardFunctions.LUpperBuf = farUpperBuf;
		StandardFunctions.LLowerBuf = farLowerBuf;
		StandardFunctions.LLower = farLower;
		StandardFunctions.LStrupr = farStrUpper;
		StandardFunctions.LStrlwr = farStrLower;
		StandardFunctions.LStricmp = farStrCmpI;
		StandardFunctions.LStrnicmp = farStrCmpNI;
		StandardFunctions.Unquote=Unquote;
		StandardFunctions.LTrim=RemoveLeadingSpaces;
		StandardFunctions.RTrim=RemoveTrailingSpaces;
		StandardFunctions.Trim=RemoveExternalSpaces;
		StandardFunctions.TruncStr=TruncStr;
		StandardFunctions.TruncPathStr=TruncPathStr;
		StandardFunctions.QuoteSpaceOnly=QuoteSpaceOnly;
		StandardFunctions.PointToName=PointToName;
		StandardFunctions.GetPathRoot=farGetPathRoot;
		StandardFunctions.AddEndSlash=AddEndSlash;
		StandardFunctions.CopyToClipboard=CopyToClipboard;
		StandardFunctions.PasteFromClipboard=PasteFromClipboard;
		StandardFunctions.FarKeyToName=FarKeyToName;
		StandardFunctions.FarNameToKey=KeyNameToKeyW;
		StandardFunctions.FarInputRecordToKey=InputRecordToKey;
		StandardFunctions.FarKeyToInputRecord=KeyToInputRecord;
		StandardFunctions.XLat=Xlat;
		StandardFunctions.GetFileOwner=farGetFileOwner;
		StandardFunctions.GetNumberOfLinks=GetNumberOfLinks;
		StandardFunctions.FarRecursiveSearch=FarRecursiveSearch;
		StandardFunctions.MkTemp=FarMkTemp;
		StandardFunctions.DeleteBuffer=DeleteBuffer;
		StandardFunctions.ProcessName=ProcessName;
		StandardFunctions.MkLink=FarMkLink;
		StandardFunctions.ConvertPath=farConvertPath;
		StandardFunctions.GetReparsePointInfo=farGetReparsePointInfo;
		StandardFunctions.GetCurrentDirectory=farGetCurrentDirectory;
	}

	if (!StartupInfo.StructSize)
	{
		StartupInfo.StructSize=sizeof(StartupInfo);
		StartupInfo.Menu=FarMenuFnW;
		StartupInfo.GetMsg=FarGetMsgFnW;
		StartupInfo.Message=FarMessageFnW;
		StartupInfo.Control=FarControl;
		StartupInfo.SaveScreen=FarSaveScreen;
		StartupInfo.RestoreScreen=FarRestoreScreen;
		StartupInfo.GetDirList=FarGetDirList;
		StartupInfo.GetPluginDirList=FarGetPluginDirListW;
		StartupInfo.FreeDirList=FarFreeDirList;
		StartupInfo.FreePluginDirList=FarFreePluginDirList;
		StartupInfo.Viewer=FarViewer;
		StartupInfo.Editor=FarEditor;
		StartupInfo.Text=FarText;
		StartupInfo.EditorControl=FarEditorControl;
		StartupInfo.ViewerControl=FarViewerControl;
		StartupInfo.ShowHelp=FarShowHelp;
		StartupInfo.AdvControl=FarAdvControlW;
		StartupInfo.DialogInit=FarDialogInitW;
		StartupInfo.DialogRun=FarDialogRun;
		StartupInfo.DialogFree=FarDialogFree;
		StartupInfo.SendDlgMessage=FarSendDlgMessage;
		StartupInfo.DefDlgProc=FarDefDlgProc;
		StartupInfo.InputBox=FarInputBoxW;
		StartupInfo.ColorDialog = farColorDialog;
		StartupInfo.PluginsControl=farPluginsControl;
		StartupInfo.FileFilterControl=farFileFilterControl;
		StartupInfo.RegExpControl=farRegExpControl;
		StartupInfo.MacroControl=farMacroControl;
		StartupInfo.SettingsControl=farSettingsControl;
	}

	*PSI=StartupInfo;
	*FSF=StandardFunctions;
	PSI->FSF=FSF;

	if (pPlugin)
	{
		PSI->ModuleName = pPlugin->GetModuleName().CPtr();
	}
}

struct ExecuteStruct
{
	int id; //function id
	union
	{
		INT_PTR nResult;
		HANDLE hResult;
		BOOL bResult;
	};

	union
	{
		INT_PTR nDefaultResult;
		HANDLE hDefaultResult;
		BOOL bDefaultResult;
	};

	bool bUnloaded;
};


#define EXECUTE_FUNCTION(function, es) \
	{ \
		es.nResult = 0; \
		es.nDefaultResult = 0; \
		es.bUnloaded = false; \
		if ( Opt.ExceptRules ) \
		{ \
			__try \
			{ \
				function; \
			} \
			__except(xfilter(es.id, GetExceptionInformation(), this, 0)) \
			{ \
				m_owner->UnloadPlugin(this, es.id, true); \
				es.bUnloaded = true; \
				ProcessException=FALSE; \
			} \
		} \
		else \
			function; \
	}


#define EXECUTE_FUNCTION_EX(function, es) \
	{ \
		es.bUnloaded = false; \
		es.nResult = 0; \
		if ( Opt.ExceptRules ) \
		{ \
			__try \
			{ \
				es.nResult = (INT_PTR)function; \
			} \
			__except(xfilter(es.id, GetExceptionInformation(), this, 0)) \
			{ \
				m_owner->UnloadPlugin(this, es.id, true); \
				es.bUnloaded = true; \
				es.nResult = es.nDefaultResult; \
				ProcessException=FALSE; \
			} \
		} \
		else \
			es.nResult = (INT_PTR)function; \
	}

bool PluginW::SetStartupInfo(bool &bUnloaded)
{
	if (pSetStartupInfoW && !ProcessException)
	{
		PluginStartupInfo _info;
		FarStandardFunctions _fsf;
		CreatePluginStartupInfo(this, &_info, &_fsf);
		// ������������ ������ � �������-��������� ����
		ExecuteStruct es;
		es.id = EXCEPT_SETSTARTUPINFO;
		EXECUTE_FUNCTION(pSetStartupInfoW(&_info), es);

		if (es.bUnloaded)
		{
			bUnloaded = true;
			return false;
		}
	}

	return true;
}

static void ShowMessageAboutIllegalPluginVersion(const wchar_t* plg,const VersionInfo& required)
{
	string strMsg1, strMsg2;
	string strPlgName;
	strMsg1.Format(MSG(MPlgRequired),
	               required.Major,required.Minor,required.Revision,required.Build);
	strMsg2.Format(MSG(MPlgRequired2),
	               FAR_VERSION.Major,FAR_VERSION.Minor,FAR_VERSION.Revision,FAR_VERSION.Build);
	Message(MSG_WARNING|MSG_NOPLUGINS,1,MSG(MError),MSG(MPlgBadVers),plg,strMsg1,strMsg2,MSG(MOk));
}


bool PluginW::GetGlobalInfo(GlobalInfo *gi)
{
	if (pGetGlobalInfoW)
	{
		memset(gi, 0, sizeof(GlobalInfo));
		ExecuteStruct es;
		es.id = EXCEPT_GETGLOBALINFO;
		EXECUTE_FUNCTION(pGetGlobalInfoW(gi), es);
		return !es.bUnloaded;
	}
	return false;
}

bool PluginW::CheckMinFarVersion(bool &bUnloaded)
{
	if (!CheckVersion(&FAR_VERSION, &MinFarVersion))
	{
		ShowMessageAboutIllegalPluginVersion(m_strModuleName,MinFarVersion);
		return false;
	}

	return true;
}

int PluginW::Unload(bool bExitFAR)
{
	int nResult = TRUE;

	if (bExitFAR)
		ExitFAR();

	if (!WorkFlags.Check(PIWF_CACHED))
	{
		nResult = FreeLibrary(m_hModule);
		ClearExports();
	}

	m_hModule = nullptr;
	FuncFlags.Clear(PICFF_LOADED); //??
	return nResult;
}

bool PluginW::IsPanelPlugin()
{
	return pSetFindListW ||
	       pGetFindDataW ||
	       pGetVirtualFindDataW ||
	       pSetDirectoryW ||
	       pGetFilesW ||
	       pPutFilesW ||
	       pDeleteFilesW ||
	       pMakeDirectoryW ||
	       pProcessHostFileW ||
	       pProcessKeyW ||
	       pProcessEventW ||
	       pCompareW ||
	       pGetOpenPanelInfoW ||
	       pFreeFindDataW ||
	       pFreeVirtualFindDataW ||
	       pClosePanelW;
}

int PluginW::Analyse(const AnalyseInfo *Info)
{
	if (Load() && pAnalyseW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_ANALYSE;
		es.bDefaultResult = FALSE;
		es.bResult = FALSE;
		EXECUTE_FUNCTION_EX(pAnalyseW(Info), es);
		return es.bResult;
	}

	return FALSE;
}

HANDLE PluginW::Open(int OpenFrom, const GUID& Guid, INT_PTR Item)
{
	ChangePriority *ChPriority = new ChangePriority(THREAD_PRIORITY_NORMAL);

	CheckScreenLock(); //??

	{
//		string strCurDir;
//		CtrlObject->CmdLine->GetCurDir(strCurDir);
//		FarChDir(strCurDir);
		g_strDirToSet.Clear();
	}

	HANDLE hResult = INVALID_HANDLE_VALUE;

	if (Load() && pOpenPanelW && !ProcessException)
	{
		//CurPluginItem=this; //BUGBUG
		ExecuteStruct es;
		es.id = EXCEPT_OPEN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;
		es.hResult = INVALID_HANDLE_VALUE;
		OpenInfo Info = {sizeof(Info)};
		Info.OpenFrom = static_cast<OPENFROM>(OpenFrom);
		Info.Guid = &Guid;
		Info.Data = Item;
		EXECUTE_FUNCTION_EX(pOpenPanelW(&Info), es);
		hResult = es.hResult;
		//CurPluginItem=nullptr; //BUGBUG
		/*    CtrlObject->Macro.SetRedrawEditor(TRUE); //BUGBUG

		    if ( !es.bUnloaded )
		    {

		      if(OpenFrom == OPEN_EDITOR &&
		         !CtrlObject->Macro.IsExecuting() &&
		         CtrlObject->Plugins.CurEditor &&
		         CtrlObject->Plugins.CurEditor->IsVisible() )
		      {
		        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
		        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
		        CtrlObject->Plugins.CurEditor->Show();
		      }
		      if (hInternal!=INVALID_HANDLE_VALUE)
		      {
		        PluginHandle *hPlugin=new PluginHandle;
		        hPlugin->InternalHandle=es.hResult;
		        hPlugin->PluginNumber=(INT_PTR)this;
		        return((HANDLE)hPlugin);
		      }
		      else
		        if ( !g_strDirToSet.IsEmpty() )
		        {
							CtrlObject->Cp()->ActivePanel->SetCurDir(g_strDirToSet,TRUE);
		          CtrlObject->Cp()->ActivePanel->Redraw();
		        }
		    } */
	}

	delete ChPriority;

	return hResult;
}

//////////////////////////////////
int PluginW::SetFindList(
    HANDLE hPlugin,
    const PluginPanelItem *PanelItem,
    int ItemsNumber
)
{
	BOOL bResult = FALSE;

	if (pSetFindListW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETFINDLIST;
		es.bDefaultResult = FALSE;
		SetFindListInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		EXECUTE_FUNCTION_EX(pSetFindListW(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginW::ProcessEditorInput(
    const INPUT_RECORD *D
)
{
	BOOL bResult = FALSE;

	if (Load() && pProcessEditorInputW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITORINPUT;
		es.bDefaultResult = TRUE; //(TRUE) treat the result as a completed request on exception!
		EXECUTE_FUNCTION_EX(pProcessEditorInputW(D), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginW::ProcessEditorEvent(
    int Event,
    PVOID Param
)
{
	if (Load() && pProcessEditorEventW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITOREVENT;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pProcessEditorEventW(Event, Param), es);
	}

	return 0; //oops!
}

int PluginW::ProcessViewerEvent(
    int Event,
    void *Param
)
{
	if (Load() && pProcessViewerEventW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSVIEWEREVENT;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pProcessViewerEventW(Event, Param), es);
	}

	return 0; //oops, again!
}

int PluginW::ProcessDialogEvent(
    int Event,
    void *Param
)
{
	BOOL bResult = FALSE;

	if (Load() && pProcessDialogEventW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSDIALOGEVENT;
		es.bDefaultResult = FALSE;
		EXECUTE_FUNCTION_EX(pProcessDialogEventW(Event, Param), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginW::ProcessSynchroEvent(
    int Event,
    void *Param
)
{
	if (Load() && pProcessSynchroEventW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSSYNCHROEVENT;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pProcessSynchroEventW(Event, Param), es);
	}

	return 0; //oops, again!
}

#if defined(PROCPLUGINMACROFUNC)
int PluginW::ProcessMacroFunc(
    const wchar_t *Name,
    const FarMacroValue *Params,
    int nParams,
    FarMacroValue **Results,
    int *nResults
)
{
	int nResult = 0;

	if (Load() && pProcessMacroFuncW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSMACROFUNC;
		es.nDefaultResult = 0;
		ProcessMacroFuncInfo Info = {sizeof(Info)};
		Info.Name = Name;
		Info.Params = Params;
		Info.nParams = nParams;
		Info.Results = *Results;
		Info.nResults = *nResults;
		EXECUTE_FUNCTION_EX(pProcessMacroFuncW(&Info), es);
		*Results = Info.Results;
		*nResults = Info.nResults;
		nResult = (int)es.nResult;
	}

	return nResult;
}
#endif

int PluginW::GetVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    int *pItemsNumber,
    const wchar_t *Path
)
{
	BOOL bResult = FALSE;

	if (pGetVirtualFindDataW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETVIRTUALFINDDATA;
		es.bDefaultResult = FALSE;
		GetVirtualFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = *pPanelItem;
		Info.ItemsNumber = *pItemsNumber;
		Info.Path = Path;
		EXECUTE_FUNCTION_EX(pGetVirtualFindDataW(&Info), es);
		*pPanelItem = Info.PanelItem;
		*pItemsNumber = Info.ItemsNumber;
		bResult = es.bResult;
	}

	return bResult;
}


void PluginW::FreeVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber
)
{
	if (pFreeVirtualFindDataW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEVIRTUALFINDDATA;
		FreeFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		EXECUTE_FUNCTION(pFreeVirtualFindDataW(&Info), es);
	}
}



int PluginW::GetFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int Move,
    const wchar_t **DestPath,
    int OpMode
)
{
	int nResult = -1;

	if (pGetFilesW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFILES;
		es.nDefaultResult = -1;
		GetFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.Move = Move;
		Info.DestPath = *DestPath;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(pGetFilesW(&Info), es);
		*DestPath = Info.DestPath;
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginW::PutFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int Move,
    int OpMode
)
{
	int nResult = -1;

	if (pPutFilesW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PUTFILES;
		es.nDefaultResult = -1;
		static string strCurrentDirectory;
		apiGetCurrentDirectory(strCurrentDirectory);
		PutFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.Move = Move;
		Info.SrcPath = strCurrentDirectory;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(pPutFilesW(&Info), es);
		nResult = (int)es.nResult;
	}

	return nResult;
}

int PluginW::DeleteFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pDeleteFilesW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_DELETEFILES;
		es.bDefaultResult = FALSE;
		DeleteFilesInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(pDeleteFilesW(&Info), es);
		bResult = (int)es.bResult;
	}

	return bResult;
}


int PluginW::MakeDirectory(
    HANDLE hPlugin,
    const wchar_t **Name,
    int OpMode
)
{
	int nResult = -1;

	if (pMakeDirectoryW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_MAKEDIRECTORY;
		es.nDefaultResult = -1;
		MakeDirectoryInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Name = *Name;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(pMakeDirectoryW(&Info), es);
		*Name = Info.Name;
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginW::ProcessHostFile(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pProcessHostFileW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSHOSTFILE;
		es.bDefaultResult = FALSE;
		ProcessHostFileInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(pProcessHostFileW(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}


int PluginW::ProcessEvent(
    HANDLE hPlugin,
    int Event,
    PVOID Param
)
{
	BOOL bResult = FALSE;

	if (pProcessEventW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEVENT;
		es.bDefaultResult = FALSE;
		EXECUTE_FUNCTION_EX(pProcessEventW(hPlugin, Event, Param), es);
		bResult = es.bResult;
	}

	return bResult;
}


int PluginW::Compare(
    HANDLE hPlugin,
    const PluginPanelItem *Item1,
    const PluginPanelItem *Item2,
    DWORD Mode
)
{
	int nResult = -2;

	if (pCompareW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_COMPARE;
		es.nDefaultResult = -2;
		CompareInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Item1 = Item1;
		Info.Item2 = Item2;
		Info.Mode = static_cast<OPENPANELINFO_SORTMODES>(Mode);
		EXECUTE_FUNCTION_EX(pCompareW(&Info), es);
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginW::GetFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    int *pItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pGetFindDataW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFINDDATA;
		es.bDefaultResult = FALSE;
		GetFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = *pPanelItem;
		Info.ItemsNumber = *pItemsNumber;
		Info.OpMode = OpMode;
		EXECUTE_FUNCTION_EX(pGetFindDataW(&Info), es);
		*pPanelItem = Info.PanelItem;
		*pItemsNumber = Info.ItemsNumber;
		bResult = es.bResult;
	}

	return bResult;
}


void PluginW::FreeFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber
)
{
	if (pFreeFindDataW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEFINDDATA;
		FreeFindDataInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.PanelItem = PanelItem;
		Info.ItemsNumber = ItemsNumber;
		EXECUTE_FUNCTION(pFreeFindDataW(&Info), es);
	}
}

int PluginW::ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec, bool Pred)
{
	(void)Pred;
	BOOL bResult = FALSE;

	if (pProcessKeyW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSKEY;
		es.bDefaultResult = TRUE; // do not pass this key to far on exception
		EXECUTE_FUNCTION_EX(pProcessKeyW(hPlugin, Rec), es);
		bResult = es.bResult;
	}

	return bResult;
}


void PluginW::ClosePanel(
    HANDLE hPlugin
)
{
	if (pClosePanelW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CLOSEPANEL;
		EXECUTE_FUNCTION(pClosePanelW(hPlugin), es);
	}

//	m_pManager->m_pCurrentPlugin = (Plugin*)-1;
}


int PluginW::SetDirectory(
    HANDLE hPlugin,
    const wchar_t *Dir,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pSetDirectoryW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETDIRECTORY;
		es.bDefaultResult = FALSE;
		SetDirectoryInfo Info = {sizeof(Info)};
		Info.hPanel = hPlugin;
		Info.Dir = Dir;
		Info.OpMode = OpMode;
		Info.UserData = 0; //Reserved
		EXECUTE_FUNCTION_EX(pSetDirectoryW(&Info), es);
		bResult = es.bResult;
	}

	return bResult;
}


void PluginW::GetOpenPanelInfo(
    HANDLE hPlugin,
    OpenPanelInfo *pInfo
)
{
//	m_pManager->m_pCurrentPlugin = this;
	pInfo->StructSize = sizeof(OpenPanelInfo);

	if (pGetOpenPanelInfoW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETOPENPANELINFO;
		pInfo->hPanel = hPlugin;
		EXECUTE_FUNCTION(pGetOpenPanelInfoW(pInfo), es);
	}
}


int PluginW::Configure(const GUID& Guid)
{
	BOOL bResult = FALSE;

	if (Load() && pConfigureW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CONFIGURE;
		es.bDefaultResult = FALSE;
		EXECUTE_FUNCTION_EX(pConfigureW(&Guid), es);
		bResult = es.bResult;
	}

	return bResult;
}


bool PluginW::GetPluginInfo(PluginInfo *pi)
{
	memset(pi, 0, sizeof(PluginInfo));

	if (pGetPluginInfoW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETPLUGININFO;
		EXECUTE_FUNCTION(pGetPluginInfoW(pi), es);

		if (!es.bUnloaded)
			return true;
	}

	return false;
}

int PluginW::GetCustomData(const wchar_t *FilePath, wchar_t **CustomData)
{
	if (Load() && pGetCustomDataW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETCUSTOMDATA;
		es.bDefaultResult = 0;
		es.bResult = 0;
		EXECUTE_FUNCTION_EX(pGetCustomDataW(FilePath, CustomData), es);
		return es.bResult;
	}

	return 0;
}

void PluginW::FreeCustomData(wchar_t *CustomData)
{
	if (Load() && pFreeCustomDataW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREECUSTOMDATA;
		EXECUTE_FUNCTION(pFreeCustomDataW(CustomData), es);
	}
}

void PluginW::ExitFAR()
{
	if (pExitFARW && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_EXITFAR;
		EXECUTE_FUNCTION(pExitFARW(), es);
	}
}

void PluginW::ClearExports()
{
	pGetGlobalInfoW=0;
	pSetStartupInfoW=0;
	pOpenPanelW=0;
	pClosePanelW=0;
	pGetPluginInfoW=0;
	pGetOpenPanelInfoW=0;
	pGetFindDataW=0;
	pFreeFindDataW=0;
	pGetVirtualFindDataW=0;
	pFreeVirtualFindDataW=0;
	pSetDirectoryW=0;
	pGetFilesW=0;
	pPutFilesW=0;
	pDeleteFilesW=0;
	pMakeDirectoryW=0;
	pProcessHostFileW=0;
	pSetFindListW=0;
	pConfigureW=0;
	pExitFARW=0;
	pProcessKeyW=0;
	pProcessEventW=0;
	pCompareW=0;
	pProcessEditorInputW=0;
	pProcessEditorEventW=0;
	pProcessViewerEventW=0;
	pProcessDialogEventW=0;
	pProcessSynchroEventW=0;
#if defined(PROCPLUGINMACROFUNC)
	pProcessMacroFuncW=0;
#endif
	pAnalyseW = 0;
	pGetCustomDataW = 0;
	pFreeCustomDataW = 0;
}

void PluginW::SetGuid(const GUID& Guid)
{
	m_Guid=Guid;
	m_strGuid=GuidToStr(m_Guid);
}
