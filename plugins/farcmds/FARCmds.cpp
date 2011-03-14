#include <CRT/crt.hpp>
#include <plugin.hpp>
#include <DlgBuilder.hpp>
#include <PluginSettings.hpp>
#include "FARCmds.hpp"
#include "Lang.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"
{
#endif
	BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	(void) lpReserved;
	(void) dwReason;
	(void) hDll;
	return TRUE;
}
#endif

struct RegistryStr REGStr={
	L"Add2PlugMenu",
	L"Add2DisksMenu",
	L"Separator",
	L"ShowCmdOutput",
	L"CatchMode",
	L"ViewZeroFiles",
	L"EditNewFiles",
	L"MaxDataSize",
};

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
struct PanelInfo PInfo;
wchar_t selectItem[MAX_PATH*5];
wchar_t fullcmd[MAX_PATH*5],cmd[MAX_PATH*5];

#include "Mix.cpp"
#include "OpenCmd.cpp"

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psInfo)
{
	Info=*psInfo;
	FSF=*psInfo->FSF;
	Info.FSF=&FSF;

	PluginSettings settings(MainGuid, Info.SettingsControl);
	settings.Get(0,REGStr.Separator,Opt.Separator,ARRAYSIZE(Opt.Separator),L" ");
	Opt.Add2PlugMenu=settings.Get(0,REGStr.Add2PlugMenu,0);
	Opt.Add2DisksMenu=settings.Get(0,REGStr.Add2DisksMenu,0);
	Opt.ShowCmdOutput=settings.Get(0,REGStr.ShowCmdOutput,0);
	Opt.CatchMode=settings.Get(0,REGStr.CatchMode,0);
	Opt.ViewZeroFiles=settings.Get(0,REGStr.ViewZeroFiles,1);
	Opt.EditNewFiles=settings.Get(0,REGStr.EditNewFiles,1);
	Opt.MaxDataSize=settings.Get(0,REGStr.MaxDataSize,1048576);
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	Info.Control(PANEL_ACTIVE,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
	fullcmd[0]=cmd[0]=selectItem[0]=L'\0';

	if (OInfo->OpenFrom==OPEN_COMMANDLINE)
	{
		OpenFromCommandLine((wchar_t *)OInfo->Data);
	}
	else if (OInfo->OpenFrom == OPEN_PLUGINSMENU && !OInfo->Data && PInfo.PanelType != PTYPE_FILEPANEL)
	{
		return INVALID_HANDLE_VALUE;
	}
	else
	{
		Info.Control(PANEL_ACTIVE,FCTL_GETPANELDIR,ARRAYSIZE(selectItem),(LONG_PTR)selectItem);

		if (lstrlen(selectItem))
			FSF.AddEndSlash(selectItem);

		PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(PANEL_ACTIVE,FCTL_GETPANELITEM,PInfo.CurrentItem,0));

		if (PPI)
		{
			Info.Control(PANEL_ACTIVE,FCTL_GETPANELITEM,PInfo.CurrentItem,(LONG_PTR)PPI);
			lstrcat(selectItem,PPI->FileName);
			free(PPI);
		}
	}

	/*���������� ������ �� ������*/
	if (lstrlen(selectItem))
	{
		static struct PanelRedrawInfo PRI;
		static wchar_t Name[MAX_PATH], Dir[MAX_PATH*5];
		int pathlen;
		lstrcpy(Name,FSF.PointToName(selectItem));
		pathlen=(int)(FSF.PointToName(selectItem)-selectItem);

		if (pathlen)
			wmemcpy(Dir,selectItem,pathlen);

		Dir[pathlen]=0;
		FSF.Trim(Name);
		FSF.Trim(Dir);
		FSF.Unquote(Name);
		FSF.Unquote(Dir);

		if (*Dir)
			Info.Control(PANEL_PASSIVE,FCTL_SETPANELDIR,0,(LONG_PTR)&Dir);

		Info.Control(PANEL_PASSIVE,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
		PRI.CurrentItem=PInfo.CurrentItem;
		PRI.TopPanelItem=PInfo.TopPanelItem;

		for (int J=0; J < PInfo.ItemsNumber; J++)
		{
			bool Equal=false;

			PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(PANEL_PASSIVE,FCTL_GETPANELITEM,J,0));

			if (PPI)
			{
				Info.Control(PANEL_PASSIVE,FCTL_GETPANELITEM,J,(LONG_PTR)PPI);
				Equal=!FSF.LStricmp(Name,FSF.PointToName(PPI->FileName));
				free(PPI);
			}

			if (Equal)
			{
				PRI.CurrentItem=J;
				PRI.TopPanelItem=J;
				break;
			}
		}

		Info.Control(PANEL_PASSIVE,FCTL_REDRAWPANEL,0,(LONG_PTR)&PRI);
	}
	else
	{
		Info.Control(PANEL_PASSIVE,FCTL_REDRAWPANEL,0,0);
	}

	return INVALID_HANDLE_VALUE;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_FULLCMDLINE;

	static const wchar_t *PluginMenuStrings[1];
	static const wchar_t *PluginConfigStrings[1];
	static const wchar_t *DiskMenuStrings[1];

	if (Opt.Add2PlugMenu)
	{
		PluginMenuStrings[0]=GetMsg(MSetPassiveDir);
        Info->PluginMenu.Guids=&SameFolderMenuGuid;
        Info->PluginMenu.Strings=PluginMenuStrings;
        Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
	}

	if (Opt.Add2DisksMenu)
	{
		DiskMenuStrings[0]=(wchar_t*)GetMsg(MSetPassiveDir);
        Info->DiskMenu.Guids=&SameFolderMenuGuid;
        Info->DiskMenu.Strings=DiskMenuStrings;
        Info->DiskMenu.Count=ARRAYSIZE(DiskMenuStrings);
	}

	PluginConfigStrings[0]=GetMsg(MConfig);
    Info->PluginConfig.Guids=&ConfigMenuGuid;
    Info->PluginConfig.Strings=PluginConfigStrings;
    Info->PluginConfig.Count=ARRAYSIZE(PluginConfigStrings);

	Info->CommandPrefix=L"far:view:edit:goto:clip:whereis:macro:link:run:load:unload";
}

int WINAPI ConfigureW(const GUID* Guid)
{
    PluginDialogBuilder Builder(Info, MainGuid, DialogGuid, MConfig, L"Config");

    Builder.AddCheckbox(MAddSetPassiveDir2PlugMenu, &Opt.Add2PlugMenu);
    Builder.AddCheckbox(MAddToDisksMenu, &Opt.Add2DisksMenu);

    Builder.AddSeparator();

    const int CmdOutIDs[] = {MHideCmdOutput, MKeepCmdOutput, MEchoCmdOutput};
    Builder.AddRadioButtons(&Opt.ShowCmdOutput, 3, CmdOutIDs);

    Builder.AddSeparator();

    const int CatchIDs[] = {MCatchAllInOne, MCatchStdOutput, MCatchStdError, MCatchSeparate};
    Builder.AddRadioButtons(&Opt.CatchMode, 4, CatchIDs);

    Builder.AddSeparator();

    Builder.AddCheckbox(MViewZeroFiles, &Opt.ViewZeroFiles);
    Builder.AddCheckbox(MEditNewFiles, &Opt.EditNewFiles);

    Builder.AddSeparator();

    FarDialogItem *MaxData = Builder.AddIntEditField((int *)&Opt.MaxDataSize, 10);
    Builder.AddTextBefore(MaxData, MMaxDataSize);

    Builder.AddOKCancel(MOk, MCancel);

    if (Builder.ShowDialog())
	{
		PluginSettings settings(MainGuid, Info.SettingsControl);
		settings.Set(0,REGStr.Add2PlugMenu,Opt.Add2PlugMenu);
		settings.Set(0,REGStr.Add2DisksMenu,Opt.Add2DisksMenu);
		settings.Set(0,REGStr.ShowCmdOutput,Opt.ShowCmdOutput);
		settings.Set(0,REGStr.CatchMode,Opt.CatchMode);
		settings.Set(0,REGStr.ViewZeroFiles,Opt.ViewZeroFiles);
		settings.Set(0,REGStr.EditNewFiles,Opt.EditNewFiles);
		settings.Set(0,REGStr.MaxDataSize,Opt.MaxDataSize);
		return TRUE;
	}

	return FALSE;
}
