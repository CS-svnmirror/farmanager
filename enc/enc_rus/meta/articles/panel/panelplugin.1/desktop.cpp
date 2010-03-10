#include <windows.h>
#include <plugin.hpp>
#include <string.h>

#include "Desktoplng.hpp"
#include "Desktop.hpp"

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
	if (Opt.DefaultFolder[0] != 0)
	{
		if (OpenFrom == OPEN_PLUGINSMENU)
		{
			::Info.Control(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,Opt.DefaultFolder);
			::Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWPANEL,NULL);
		}
		else if (OpenFrom == OPEN_DISKMENU)
		{
			return (HANDLE)&Opt; // �����頥� ��, �� 㣮���, �஬� 0 � INVALID_HANDLE_VALUE; ⮣�� �ࠡ�⠥� �㭪�� GetFindData()
		}
	}
	return INVALID_HANDLE_VALUE;
}

int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,
                      int *pItemsNumber,int OpMode)
{
	::Info.Control(hPlugin,FCTL_CLOSEPLUGIN,Opt.DefaultFolder);
	return TRUE;
}
