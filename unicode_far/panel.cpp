/*
panel.cpp

Parent class ��� �������
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

#include "headers.hpp"
#pragma hdrstop

#include "panel.hpp"
#include "macroopcode.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "keys.hpp"
#include "vmenu2.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "chgmmode.hpp"
#include "chgprior.hpp"
#include "edit.hpp"
#include "treelist.hpp"
#include "filelist.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "lockscrn.hpp"
#include "help.hpp"
#include "syslog.hpp"
#include "plugapi.hpp"
#include "network.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "hotplug.hpp"
#include "eject.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "scrsaver.hpp"
#include "execute.hpp"
#include "shortcuts.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "imports.hpp"
#include "constitle.hpp"
#include "FarDlgBuilder.hpp"
#include "setattr.hpp"
#include "window.hpp"
#include "colormix.hpp"
#include "FarGuid.hpp"
#include "elevation.hpp"
#include "stddlg.hpp"
#include "lang.hpp"
#include "plugins.hpp"
#include "notification.hpp"

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=nullptr;

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile);

/* $ 21.08.2002 IS
   ����� ��� �������� ������ ������� � ���� ������ ������
*/

class ChDiskPluginItem:NonCopyable
{
public:
	ChDiskPluginItem():
		HotKey()
	{}

	ChDiskPluginItem(ChDiskPluginItem&& rhs):
		HotKey()
	{
		*this = std::move(rhs);
	}

	ChDiskPluginItem& operator=(ChDiskPluginItem&& rhs)
	{
		if (this != &rhs)
		{
			Item = std::move(rhs.Item);
			HotKey=rhs.HotKey;
		}
		return *this;
	}


	bool operator ==(const ChDiskPluginItem& rhs) const
	{
		return HotKey==rhs.HotKey && !StrCmpI(Item.strName, rhs.Item.strName) && Item.UserData==rhs.Item.UserData;
	}

	bool operator <(const ChDiskPluginItem& rhs) const
	{
		return (Global->Opt->ChangeDriveMode&DRIVE_SORT_PLUGINS_BY_HOTKEY && HotKey!=rhs.HotKey)?
			HotKey-1 < rhs.HotKey-1 :
			StrCmpI(Item.strName, rhs.Item.strName) < 0;
	}

	MenuItemEx& getItem() { return Item; }
	WCHAR& getHotKey() { return HotKey; }

private:
	MenuItemEx Item;
	WCHAR HotKey;
};


Panel::Panel():
	ProcessingPluginCommand(0),
	Focus(false),
	Type(0),
	EnableUpdate(TRUE),
	PanelMode(NORMAL_PANEL),
	SortMode(UNSORTED),
	ReverseSortOrder(false),
	SortGroups(0),
	PrevViewMode(VIEW_3),
	ViewMode(0),
	CurTopFile(0),
	CurFile(0),
	ShowShortNames(0),
	NumericSort(0),
	CaseSensitiveSort(0),
	DirectoriesFirst(1),
	ModalMode(0),
	PluginCommand(0)
{
	ViewSettings.clear();
	_OT(SysLog(L"[%p] Panel::Panel()", this));
	SrcDragPanel=nullptr;
	DragX=DragY=-1;
};


Panel::~Panel()
{
	_OT(SysLog(L"[%p] Panel::~Panel()", this));
	EndDrag();
}


void Panel::SetViewMode(int ViewMode)
{
	PrevViewMode=ViewMode;
	this->ViewMode=ViewMode;
};


void Panel::ChangeDirToCurrent()
{
	string strNewDir;
	api::GetCurrentDirectory(strNewDir);
	SetCurDir(strNewDir,true);
}


void Panel::ChangeDisk()
{
	int Pos=0,FirstCall=TRUE;

	if (!strCurDir.empty() && strCurDir[1]==L':')
	{
		Pos=std::max(0, Upper(strCurDir[0])-L'A');
	}

	while (Pos!=-1)
	{
		Pos=ChangeDiskMenu(Pos,FirstCall);
		FirstCall=FALSE;
	}
}

struct PanelMenuItem
{
	bool bIsPlugin;

	union
	{
		struct
		{
			Plugin *pPlugin;
			GUID Guid;
		};

		struct
		{
			wchar_t cDrive;
			int nDriveType;
		};
	};
};

static size_t AddPluginItems(VMenu2 &ChDisk, int Pos, int DiskCount, bool SetSelected)
{
	std::list<ChDiskPluginItem> MPItems;
	int PluginItem;
	bool ItemPresent,Done=false;
	string strMenuText;
	string strPluginText;
	size_t PluginMenuItemsCount = 0;

	FOR(const auto& i, *Global->CtrlObject->Plugins)
	{
		if(Done)
			break;
		for (PluginItem=0;; ++PluginItem)
		{
			Plugin *pPlugin = i;

			WCHAR HotKey = 0;
			GUID guid;
			if (!Global->CtrlObject->Plugins->GetDiskMenuItem(
			            pPlugin,
			            PluginItem,
			            ItemPresent,
			            HotKey,
			            strPluginText,
			            guid
			        ))
			{
				Done=true;
				break;
			}

			if (!ItemPresent)
				break;

			strMenuText = strPluginText;

			if (!strMenuText.empty())
			{
				ChDiskPluginItem OneItem;
#ifndef NO_WRAPPER
				if (pPlugin->IsOemPlugin())
					OneItem.getItem().Flags=LIF_CHECKED|L'A';
#endif // NO_WRAPPER
				OneItem.getItem().strName = strMenuText;
				OneItem.getHotKey()=HotKey;

				PanelMenuItem *item = new PanelMenuItem;
				item->bIsPlugin = true;
				item->pPlugin = pPlugin;
				item->Guid = guid;
				OneItem.getItem().UserData=item;
				OneItem.getItem().UserDataSize=sizeof(*item);

				MPItems.emplace_back(std::move(OneItem));
			}
		}
	}

	MPItems.sort();
	MPItems.unique(); // ������� �����
	PluginMenuItemsCount=MPItems.size();

	if (PluginMenuItemsCount)
	{
		MenuItemEx ChDiskItem;
		ChDiskItem.Flags|=LIF_SEPARATOR;
		ChDisk.AddItem(ChDiskItem);

		for_each_cnt(RANGE(MPItems, i, size_t index)
		{
			if (Pos > DiskCount && !SetSelected)
			{
				i.getItem().SetSelect(DiskCount + static_cast<int>(index) + 1 == Pos);

				if (!SetSelected)
					SetSelected = DiskCount + static_cast<int>(index) + 1 == Pos;
			}
			wchar_t HotKey = i.getHotKey();
			const wchar_t HotKeyStr[]={HotKey? L'&' : L' ', HotKey? HotKey : L' ', L' ', HotKey? L' ' : L'\0', L'\0'};
			i.getItem().strName = string(HotKeyStr) + i.getItem().strName;
			ChDisk.AddItem(i.getItem());

			delete(PanelMenuItem*)i.getItem().UserData;  //����...
		});
	}
	return PluginMenuItemsCount;
}

static void ConfigureChangeDriveMode()
{
	DialogBuilder Builder(MChangeDriveConfigure, L"ChangeDriveMode");
	Builder.AddCheckbox(MChangeDriveShowDiskType, Global->Opt->ChangeDriveMode, DRIVE_SHOW_TYPE);
	Builder.AddCheckbox(MChangeDriveShowLabel, Global->Opt->ChangeDriveMode, DRIVE_SHOW_LABEL);
	Builder.AddCheckbox(MChangeDriveShowFileSystem, Global->Opt->ChangeDriveMode, DRIVE_SHOW_FILESYSTEM);

	BOOL ShowSizeAny = Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);

	DialogItemEx *ShowSize = Builder.AddCheckbox(MChangeDriveShowSize, &ShowSizeAny);
	DialogItemEx *ShowSizeFloat = Builder.AddCheckbox(MChangeDriveShowSizeFloat, Global->Opt->ChangeDriveMode, DRIVE_SHOW_SIZE_FLOAT);
	ShowSizeFloat->Indent(4);
	Builder.LinkFlags(ShowSize, ShowSizeFloat, DIF_DISABLE);

	Builder.AddCheckbox(MChangeDriveShowNetworkName, Global->Opt->ChangeDriveMode, DRIVE_SHOW_NETNAME);
	Builder.AddCheckbox(MChangeDriveShowPlugins, Global->Opt->ChangeDriveMode, DRIVE_SHOW_PLUGINS);
	Builder.AddCheckbox(MChangeDriveSortPluginsByHotkey, Global->Opt->ChangeDriveMode, DRIVE_SORT_PLUGINS_BY_HOTKEY)->Indent(4);
	Builder.AddCheckbox(MChangeDriveShowRemovableDrive, Global->Opt->ChangeDriveMode, DRIVE_SHOW_REMOVABLE);
	Builder.AddCheckbox(MChangeDriveShowCD, Global->Opt->ChangeDriveMode, DRIVE_SHOW_CDROM);
	Builder.AddCheckbox(MChangeDriveShowNetworkDrive, Global->Opt->ChangeDriveMode, DRIVE_SHOW_REMOTE);

	Builder.AddOKCancel();
	if (Builder.ShowDialog())
	{
		if (ShowSizeAny)
		{
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE_FLOAT)
				Global->Opt->ChangeDriveMode &= ~DRIVE_SHOW_SIZE;
			else
				Global->Opt->ChangeDriveMode |= DRIVE_SHOW_SIZE;
		}
		else
			Global->Opt->ChangeDriveMode &= ~(DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);
	}
}

class separator
{
public:
	separator():m_value(L' '){}
	const string Get()
	{
		wchar_t c = m_value;
		m_value = BoxSymbols[BS_V1];
		const wchar_t value[] = {L' ', c, L' ', 0};
		return value[1] == L' '? L" " : value;
	}
private:
	wchar_t m_value;
};

enum
{
	DRIVE_DEL_FAIL,
	DRIVE_DEL_SUCCESS,
	DRIVE_DEL_EJECT,
	DRIVE_DEL_NONE
};

int Panel::ChangeDiskMenu(int Pos,int FirstCall)
{
	class Guard_Macro_DskShowPosType  //����� �����-��
	{
	public:
		Guard_Macro_DskShowPosType(Panel *curPanel) {Global->Macro_DskShowPosType=(curPanel==Global->CtrlObject->Cp()->LeftPanel)?1:2;}
		~Guard_Macro_DskShowPosType() {Global->Macro_DskShowPosType=0;}
	}
	_guard_Macro_DskShowPosType(this);

	std::bitset<32> Mask(static_cast<int>(FarGetLogicalDrives()));
	const std::bitset<32> NetworkMask = AddSavedNetworkDisks(Mask);
	const size_t DiskCount = Mask.count();

	PanelMenuItem Item, *mitem=0;
	{ // ��� ������ ����, ��. M#605
		VMenu2 ChDisk(MSG(MChangeDriveTitle),nullptr,0,ScrY-Y1-3);
		ChDisk.SetBottomTitle(MSG(MChangeDriveMenuFooter));

		ChDisk.SetHelp(L"DriveDlg");
		ChDisk.SetFlags(VMENU_WRAPMODE);

		struct DiskMenuItem
		{
			string Letter;
			string Type;
			string Label;
			string Fs;
			string TotalSize;
			string FreeSize;
			string Path;

			int DriveType;
		};
		std::list<DiskMenuItem> Items;

		size_t TypeWidth = 0, LabelWidth = 0, FsWidth = 0, TotalSizeWidth = 0, FreeSizeWidth = 0, PathWidth = 0;


		auto DE = std::make_unique<DisableElevation>();
		/* $ 02.04.2001 VVM
		! ������� �� ������ ������ �����... */
		for (size_t i = 0; i < Mask.size(); ++i)
		{
			if (!Mask[i])   //���� �����
				continue;

			DiskMenuItem NewItem;

			wchar_t Drv[]={L'&',static_cast<wchar_t>(L'A'+i),L':',L'\\',L'\0'};
			string strRootDir=Drv+1;
			Drv[3] = 0;
			NewItem.Letter = Drv;
			NewItem.DriveType = FAR_GetDriveType(strRootDir, Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);

			if (NetworkMask[i])
				NewItem.DriveType = DRIVE_REMOTE_NOT_CONNECTED;

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_TYPE|DRIVE_SHOW_NETNAME))
			{
				string LocalName(L"?:");
				LocalName[0] = strRootDir[0];

				if (GetSubstName(NewItem.DriveType, LocalName, NewItem.Path))
				{
					NewItem.DriveType=DRIVE_SUBSTITUTE;
				}
				else if(DriveCanBeVirtual(NewItem.DriveType) && GetVHDName(LocalName, NewItem.Path))
				{
					NewItem.DriveType=DRIVE_VIRTUAL;
				}

				static const simple_pair<int, LNGID> DrTMsg[]=
				{
					{DRIVE_REMOVABLE,MChangeDriveRemovable},
					{DRIVE_FIXED,MChangeDriveFixed},
					{DRIVE_REMOTE,MChangeDriveNetwork},
					{DRIVE_REMOTE_NOT_CONNECTED,MChangeDriveDisconnectedNetwork},
					{DRIVE_CDROM,MChangeDriveCDROM},
					{DRIVE_CD_RW,MChangeDriveCD_RW},
					{DRIVE_CD_RWDVD,MChangeDriveCD_RWDVD},
					{DRIVE_DVD_ROM,MChangeDriveDVD_ROM},
					{DRIVE_DVD_RW,MChangeDriveDVD_RW},
					{DRIVE_DVD_RAM,MChangeDriveDVD_RAM},
					{DRIVE_BD_ROM,MChangeDriveBD_ROM},
					{DRIVE_BD_RW,MChangeDriveBD_RW},
					{DRIVE_HDDVD_ROM,MChangeDriveHDDVD_ROM},
					{DRIVE_HDDVD_RW,MChangeDriveHDDVD_RW},
					{DRIVE_RAMDISK,MChangeDriveRAM},
					{DRIVE_SUBSTITUTE,MChangeDriveSUBST},
					{DRIVE_VIRTUAL,MChangeDriveVirtual},
					{DRIVE_USBDRIVE,MChangeDriveRemovable},
				};

				auto ItemIterator = std::find_if(CONST_RANGE(DrTMsg, i) {return i.first == NewItem.DriveType;});
				if (ItemIterator != std::cend(DrTMsg))
					NewItem.Type = MSG(ItemIterator->second);
			}

			int ShowDisk = (NewItem.DriveType!=DRIVE_REMOVABLE || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
			               (!IsDriveTypeCDROM(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
			               (!IsDriveTypeRemote(NewItem.DriveType) || (Global->Opt->ChangeDriveMode & DRIVE_SHOW_REMOTE));

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_LABEL|DRIVE_SHOW_FILESYSTEM))
			{
				if (ShowDisk && !api::GetVolumeInformation(
				            strRootDir,
				            &NewItem.Label,
				            nullptr,
				            nullptr,
				            nullptr,
				            &NewItem.Fs
				        ))
				{
					NewItem.Label = MSG(MChangeDriveLabelAbsent);
					ShowDisk = FALSE;
				}
			}

			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
			{
				unsigned __int64 TotalSize = 0, UserFree = 0;

				if (ShowDisk && api::GetDiskSize(strRootDir,&TotalSize, nullptr, &UserFree))
				{
					if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						//������ ��� ������� � ����������
						FileSizeToStr(NewItem.TotalSize,TotalSize,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
						FileSizeToStr(NewItem.FreeSize,UserFree,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
					}
					else
					{
						//������ � ������ � ��� 0 ��������� ����� ������� (B)
						FileSizeToStr(NewItem.TotalSize,TotalSize,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
						FileSizeToStr(NewItem.FreeSize,UserFree,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
					}
					RemoveExternalSpaces(NewItem.TotalSize);
					RemoveExternalSpaces(NewItem.FreeSize);
				}
			}

			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_NETNAME)
			{
				switch(NewItem.DriveType)
				{
				case DRIVE_REMOTE:
				case DRIVE_REMOTE_NOT_CONNECTED:
					DriveLocalToRemoteName(NewItem.DriveType,strRootDir[0],NewItem.Path);
					break;
				}
			}

			TypeWidth = std::max(TypeWidth, NewItem.Type.size());
			LabelWidth = std::max(LabelWidth, NewItem.Label.size());
			FsWidth = std::max(FsWidth, NewItem.Fs.size());
			TotalSizeWidth = std::max(TotalSizeWidth, NewItem.TotalSize.size());
			FreeSizeWidth = std::max(FreeSizeWidth, NewItem.FreeSize.size());
			PathWidth = std::max(PathWidth, NewItem.Path.size());

			Items.emplace_back(NewItem);
		}

		int MenuLine = 0;

		bool SetSelected=false;
		std::for_each(CONST_RANGE(Items, i)
		{
			MenuItemEx ChDiskItem;
			int DiskNumber = i.Letter[1] - L'A';
			if (FirstCall)
			{
				ChDiskItem.SetSelect(DiskNumber==Pos);

				if (!SetSelected)
					SetSelected=(DiskNumber==Pos);
			}
			else
			{
				if (Pos < static_cast<int>(DiskCount))
				{
					ChDiskItem.SetSelect(MenuLine==Pos);

					if (!SetSelected)
						SetSelected=(MenuLine==Pos);
				}
			}
			FormatString ItemName;
			ItemName << i.Letter;

			separator Separator;

			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_TYPE)
			{
				ItemName << Separator.Get() << fmt::LeftAlign() << fmt::ExactWidth(TypeWidth) << i.Type;
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_LABEL)
			{
				ItemName << Separator.Get() << fmt::LeftAlign() << fmt::ExactWidth(LabelWidth) << i.Label;
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
			{
				ItemName << Separator.Get() << fmt::LeftAlign() << fmt::ExactWidth(FsWidth) << i.Fs;
			}
			if (Global->Opt->ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
			{
				ItemName << Separator.Get() << fmt::ExactWidth(TotalSizeWidth) << i.TotalSize;
				ItemName << Separator.Get() << fmt::ExactWidth(FreeSizeWidth) << i.FreeSize;
			}
			if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_NETNAME && PathWidth)
			{
				ItemName << Separator.Get() << i.Path;
			}

			ChDiskItem.strName = ItemName;
			PanelMenuItem item;
			item.bIsPlugin = false;
			item.cDrive = L'A' + DiskNumber;
			item.nDriveType = i.DriveType;
			ChDisk.SetUserData(&item, sizeof(item), ChDisk.AddItem(ChDiskItem));
			MenuLine++;
		});

		size_t PluginMenuItemsCount=0;

		if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_PLUGINS)
		{
			PluginMenuItemsCount = AddPluginItems(ChDisk, Pos, static_cast<int>(DiskCount), SetSelected);
		}

		DE.reset();

		int X=X1+5;

		if ((this == Global->CtrlObject->Cp()->RightPanel) && IsFullScreen() && (X2-X1 > 40))
			X = (X2-X1+1)/2+5;

		ChDisk.SetPosition(X,-1,0,0);

		int Y = (ScrY+1-static_cast<int>((DiskCount+PluginMenuItemsCount)+5))/2;
		if (Y < 3)
			ChDisk.SetBoxType(SHORT_DOUBLE_BOX);

		ChDisk.SetMacroMode(MACROAREA_DISKS);
		int RetCode=-1;

		bool NeedRefresh = false;

		listener DeviceListener(L"devices", [&NeedRefresh]()
		{
			NeedRefresh = true;
		});

		ChDisk.Run([&](int Key)->int
		{
			if(Key==KEY_NONE && NeedRefresh)
			{
				Key=KEY_CTRLR;
				NeedRefresh = false;
			}

			int SelPos=ChDisk.GetSelectPos();
			PanelMenuItem *item = (PanelMenuItem*)ChDisk.GetUserData(nullptr,0);

			int KeyProcessed = 1;

			switch (Key)
			{
				// Shift-Enter � ���� ������ ������ �������� ��������� ��� ������� �����
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:
				{
					if (item && !item->bIsPlugin)
					{
						string DosDeviceName(L"?:\\");
						DosDeviceName[0] = item->cDrive;
						Execute(DosDeviceName, false, true, true, true);
					}
				}
				break;
				case KEY_CTRLPGUP:
				case KEY_RCTRLPGUP:
				case KEY_CTRLNUMPAD9:
				case KEY_RCTRLNUMPAD9:
				{
					if (Global->Opt->PgUpChangeDisk != 0)
						ChDisk.Close(-1);
				}
				break;
				// �.�. ��� ������� �������� ��������� "����������" ����������,
				// �� ������� ��������� Ins ��� CD - "������� ����"
				case KEY_INS:
				case KEY_NUMPAD0:
				{
					if (item && !item->bIsPlugin)
					{
						if (IsDriveTypeCDROM(item->nDriveType) /* || DriveType == DRIVE_REMOVABLE*/)
						{
							SaveScreen SvScrn;
							EjectVolume(item->cDrive, EJECT_LOAD_MEDIA);
							RetCode=SelPos;
						}
					}
				}
				break;
				case KEY_NUMDEL:
				case KEY_DEL:
				{
					if (item && !item->bIsPlugin)
					{
						int Code = DisconnectDrive(item, ChDisk);
						if (Code != DRIVE_DEL_FAIL && Code != DRIVE_DEL_NONE)
						{
							Global->ScrBuf->Lock(); // �������� ������ ����������
							FrameManager->ResizeAllFrame();
							FrameManager->PluginCommit(); // ��������.
							Global->ScrBuf->Unlock(); // ��������� ����������
							RetCode=(((DiskCount-SelPos)==1) && (SelPos > 0) && (Code != DRIVE_DEL_EJECT))?SelPos-1:SelPos;
						}
					}
				}
				break;
				case KEY_F3:
				if (item && item->bIsPlugin)
				{
					Global->CtrlObject->Plugins->ShowPluginInfo(item->pPlugin, item->Guid);
				}
				break;
				case KEY_CTRLA:
				case KEY_RCTRLA:
				case KEY_F4:
				{
					if (item)
					{
						if (!item->bIsPlugin)
						{
							string DeviceName(L"?:\\");
							DeviceName[0] = item->cDrive;
							ShellSetFileAttributes(nullptr, &DeviceName);
						}
						else
						{
							string strName = ChDisk.GetItemPtr(SelPos)->strName.data() + 3;
							RemoveExternalSpaces(strName);
							if(Global->CtrlObject->Plugins->SetHotKeyDialog(item->pPlugin, item->Guid, PluginsHotkeysConfig::DRIVE_MENU, strName))
							RetCode=SelPos;
						}
					}
					break;
				}

				case KEY_APPS:
				case KEY_SHIFTAPPS:
				case KEY_MSRCLICK:
				{
					//������� EMenu ���� �� ����
					if (item && !item->bIsPlugin && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu))
					{
						const wchar_t DeviceName[] = {item->cDrive, L':', L'\\', 0};
						struct DiskMenuParam {const wchar_t* CmdLine; BOOL Apps;} p = {DeviceName, Key!=KEY_MSRCLICK};
						Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu, OPEN_LEFTDISKMENU, &p); // EMenu Plugin :-)
					}
					break;
				}

				case KEY_SHIFTNUMDEL:
				case KEY_SHIFTDECIMAL:
				case KEY_SHIFTDEL:
				{
					if (item && !item->bIsPlugin)
					{
						RemoveHotplugDevice(item, ChDisk);
						RetCode=SelPos;
					}
				}
				break;
				case KEY_CTRL1:
				case KEY_RCTRL1:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_TYPE;
					RetCode=SelPos;
					break;
				case KEY_CTRL2:
				case KEY_RCTRL2:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_NETNAME;
					RetCode=SelPos;
					break;
				case KEY_CTRL3:
				case KEY_RCTRL3:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_LABEL;
					RetCode=SelPos;
					break;
				case KEY_CTRL4:
				case KEY_RCTRL4:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_FILESYSTEM;
					RetCode=SelPos;
					break;
				case KEY_CTRL5:
				case KEY_RCTRL5:
				{
					if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_SIZE;
						Global->Opt->ChangeDriveMode |= DRIVE_SHOW_SIZE_FLOAT;
					}
					else
					{
						if (Global->Opt->ChangeDriveMode & DRIVE_SHOW_SIZE_FLOAT)
							Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_SIZE_FLOAT;
						else
							Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_SIZE;
					}

					RetCode=SelPos;
					break;
				}
				case KEY_CTRL6:
				case KEY_RCTRL6:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_REMOVABLE;
					RetCode=SelPos;
					break;
				case KEY_CTRL7:
				case KEY_RCTRL7:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_PLUGINS;
					RetCode=SelPos;
					break;
				case KEY_CTRL8:
				case KEY_RCTRL8:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_CDROM;
					RetCode=SelPos;
					break;
				case KEY_CTRL9:
				case KEY_RCTRL9:
					Global->Opt->ChangeDriveMode ^= DRIVE_SHOW_REMOTE;
					RetCode=SelPos;
					break;
				case KEY_F9:
					ConfigureChangeDriveMode();
					RetCode=SelPos;
					break;
				case KEY_SHIFTF1:
				{
					if (item && item->bIsPlugin)
					{
						// �������� ������ �����, ������� �������� � CommandsMenu()
						pluginapi::apiShowHelp(
						    item->pPlugin->GetModuleName().data(),
						    nullptr,
						    FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS
						);
					}

					break;
				}
				case KEY_ALTSHIFTF9:
				case KEY_RALTSHIFTF9:

					if (Global->Opt->ChangeDriveMode&DRIVE_SHOW_PLUGINS)
						Global->CtrlObject->Plugins->Configure();

					RetCode=SelPos;
					break;
				case KEY_SHIFTF9:

					if (item && item->bIsPlugin && item->pPlugin->HasConfigure())
						Global->CtrlObject->Plugins->ConfigureCurrent(item->pPlugin, item->Guid);

					RetCode=SelPos;
					break;
				case KEY_CTRLR:
				case KEY_RCTRLR:
					RetCode=SelPos;
					break;

				 default:
				 	KeyProcessed = 0;
			}

			if (RetCode>=0)
				ChDisk.Close(-1);

			return KeyProcessed;
		});

		if (RetCode>=0)
			return RetCode;

		if (ChDisk.GetExitCode()<0 &&
			        !strCurDir.empty() &&
			        (StrCmpN(strCurDir.data(),L"\\\\",2) ))
			{
				const wchar_t RootDir[4] = {strCurDir[0],L':',L'\\',L'\0'};

				if (FAR_GetDriveType(RootDir) == DRIVE_NO_ROOT_DIR)
				return ChDisk.GetSelectPos();
			}

		if (ChDisk.GetExitCode()<0)
			return -1;

		mitem=(PanelMenuItem*)ChDisk.GetUserData(nullptr,0);

		if (mitem)
		{
			Item=*mitem;
			mitem=&Item;
		}
	} // ��� ������ ����, ��. M#605

	if (Global->Opt->CloseCDGate && mitem && !mitem->bIsPlugin && IsDriveTypeCDROM(mitem->nDriveType))
	{
		string RootDir(L"?:");
		RootDir[0] = mitem->cDrive;

		if (!api::IsDiskInDrive(RootDir))
		{
			if (!EjectVolume(mitem->cDrive, EJECT_READY|EJECT_NO_MESSAGE))
			{
				SaveScreen SvScrn;
				Message(0,0,L"",MSG(MChangeWaitingLoadDisk));
				EjectVolume(mitem->cDrive, EJECT_LOAD_MEDIA|EJECT_NO_MESSAGE);
			}
		}
	}

	if (ProcessPluginEvent(FE_CLOSE,nullptr))
		return -1;

	Global->ScrBuf->Flush();
	INPUT_RECORD rec;
	PeekInputRecord(&rec);

	if (!mitem)
		return -1; //???

	if (!mitem->bIsPlugin)
	{
		for (;;)
		{
			wchar_t NewDir[]={mitem->cDrive,L':',0,0};

			// In general, mitem->cDrive can contain any unicode character
			if (mitem->cDrive >= L'A' && mitem->cDrive <= L'Z' && NetworkMask[mitem->cDrive-L'A'])
			{
				ConnectToNetworkDrive(NewDir);
			}

			if (FarChDir(NewDir))
			{
				break;
			}
			else
			{
				NewDir[2]=L'\\';

				if (FarChDir(NewDir))
				{
					break;
				}
			}
			Global->CatchError();

			DialogBuilder Builder(MError, nullptr);

			Builder.AddTextWrap(GetErrorString().data(), true);
			Builder.AddText(L"");

			const wchar_t Drive[] = {mitem->cDrive,L'\0'};
			string DriveLetter = Drive;
			DialogItemEx *DriveLetterEdit = Builder.AddFixEditField(&DriveLetter, 1);
			Builder.AddTextBefore(DriveLetterEdit, MChangeDriveCannotReadDisk);
			Builder.AddTextAfter(DriveLetterEdit, L":", 0);

			Builder.AddOKCancel(MRetry, MCancel);
			Builder.SetDialogMode(DMODE_WARNINGSTYLE);

			if (Builder.ShowDialog())
			{
				mitem->cDrive = Upper(DriveLetter[0]);
			}
			else
			{
				return -1;
			}
		}

		string strNewCurDir;
		api::GetCurrentDirectory(strNewCurDir);

		if ((PanelMode == NORMAL_PANEL) &&
		        (GetType() == FILE_PANEL) &&
		        !StrCmpI(strCurDir, strNewCurDir) &&
		        IsVisible())
		{
			// � ����� �� ������ ����� Update????
			Update(UPDATE_KEEP_SELECTION);
		}
		else
		{
			int Focus=GetFocus();
			Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(this, FILE_PANEL, TRUE, FALSE);
			NewPanel->SetCurDir(strNewCurDir,true);
			NewPanel->Show();

			if (Focus || !Global->CtrlObject->Cp()->GetAnotherPanel(this)->IsVisible())
				NewPanel->SetFocus();

			if (!Focus && Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				Global->CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}
	else //��� ������, ��
	{
		HANDLE hPlugin = Global->CtrlObject->Plugins->Open(
		                     mitem->pPlugin,
		                     (Global->CtrlObject->Cp()->LeftPanel == this)?OPEN_LEFTDISKMENU:OPEN_RIGHTDISKMENU,
		                     mitem->Guid,
		                     0
		                 );

		if (hPlugin)
		{
			int Focus=GetFocus();
			Panel *NewPanel = Global->CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,TRUE);
			NewPanel->SetPluginMode(hPlugin,L"",Focus || !Global->CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());
			NewPanel->Update(0);
			NewPanel->Show();

			if (!Focus && Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				Global->CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}

	return -1;
}

int Panel::DisconnectDrive(const PanelMenuItem *item, VMenu2 &ChDisk)
{
	if ((item->nDriveType == DRIVE_REMOVABLE) || IsDriveTypeCDROM(item->nDriveType))
	{
		if ((item->nDriveType == DRIVE_REMOVABLE) && !IsEjectableMedia(item->cDrive))
			return -1;

		// ������ ������� ������� ����

		if (!EjectVolume(item->cDrive, EJECT_NO_MESSAGE))
		{
			// ���������� ��������� �������
			int CMode=GetMode();
			int AMode=Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
			string strTmpCDir(GetCurDir()), strTmpADir(Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir());
			// "���� �� �������������"
			int DoneEject=FALSE;

			while (!DoneEject)
			{
				// "��������� ����" - �������� ��� ������������� � �������� �������
				// TODO: � ���� �������� ������� - CD? ;-)
				IfGoHome(item->cDrive);
				// ��������� ������� ���������� ��� ������ ���������
				int ResEject = EjectVolume(item->cDrive, EJECT_NO_MESSAGE);

				if (!ResEject)
				{
					// ����������� ���� - ��� ������� ��� �� ����� ������ � ������.
					if (AMode != PLUGIN_PANEL)
						Global->CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(strTmpADir, false);

					if (CMode != PLUGIN_PANEL)
						SetCurDir(strTmpCDir, false);

					// ... � ������� ����� �...
					SetLastError(ERROR_DRIVE_LOCKED); // ...� "The disk is in use or locked by another process."
					Global->CatchError();
					wchar_t Drive[] = {item->cDrive, L':', L'\\', 0};
					DoneEject = OperationFailed(Drive, MError, LangString(MChangeCouldNotEjectMedia) << item->cDrive, false);
				}
				else
					DoneEject=TRUE;
			}
		}
		return DRIVE_DEL_NONE;
	}
	else
	{
		return ProcessDelDisk(item->cDrive, item->nDriveType, &ChDisk);
	}
}

void Panel::RemoveHotplugDevice(const PanelMenuItem *item, VMenu2 &ChDisk)
{
	int Code = RemoveHotplugDisk(item->cDrive, EJECT_NOTIFY_AFTERREMOVE);

	if (!Code)
	{
		// ���������� ��������� �������
		int CMode=GetMode();
		int AMode=Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
		string strTmpCDir(GetCurDir()), strTmpADir(Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir());
		// "���� �� �������������"
		int DoneEject=FALSE;

		while (!DoneEject)
		{
			// "��������� ����" - �������� ��� ������������� � �������� �������
			// TODO: � ���� �������� ������� - USB? ;-)
			IfGoHome(item->cDrive);
			// ��������� ������� ���������� ��� ������ ���������
			Code = RemoveHotplugDisk(item->cDrive, EJECT_NO_MESSAGE|EJECT_NOTIFY_AFTERREMOVE);

			if (!Code)
			{
				// ����������� ���� - ��� ������� ��� �� ����� ������ � ������.
				if (AMode != PLUGIN_PANEL)
					Global->CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(strTmpADir, false);

				if (CMode != PLUGIN_PANEL)
					SetCurDir(strTmpCDir, false);

				// ... � ������� ����� �...
				SetLastError(ERROR_DRIVE_LOCKED); // ...� "The disk is in use or locked by another process."
				Global->CatchError();
				DoneEject = Message(MSG_WARNING|MSG_ERRORTYPE, 2,
				                MSG(MError),
				                (LangString(MChangeCouldNotEjectHotPlugMedia) << item->cDrive).data(),
				                MSG(MHRetry), MSG(MHCancel));
			}
			else
				DoneEject=TRUE;
		}
	}
}

/* $ 28.12.2001 DJ
   ��������� Del � ���� ������
*/

int Panel::ProcessDelDisk(wchar_t Drive, int DriveType,VMenu2 *ChDiskMenu)
{
	string DiskLetter(L"?:");
	DiskLetter[0] = Drive;

	switch(DriveType)
	{
	case DRIVE_SUBSTITUTE:
		{
			if (Global->Opt->Confirm.RemoveSUBST)
			{
				if (Message(MSG_WARNING,2,
					MSG(MChangeSUBSTDisconnectDriveTitle),
					(LangString(MChangeSUBSTDisconnectDriveQuestion) << DiskLetter).data(),
					MSG(MYes),MSG(MNo)))
				{
					return DRIVE_DEL_FAIL;
				}
			}
			if (DelSubstDrive(DiskLetter))
			{
				return DRIVE_DEL_SUCCESS;
			}
			else
			{
				Global->CatchError();
				DWORD LastError = Global->CaughtError();
				LangString strMsgText(MChangeDriveCannotDelSubst);
				strMsgText << DiskLetter;
				if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
				{
					if (!Message(MSG_WARNING|MSG_ERRORTYPE, 2,
						MSG(MError),
						strMsgText.data(),
						L"\x1",
						MSG(MChangeDriveOpenFiles),
						MSG(MChangeDriveAskDisconnect),
						MSG(MOk),MSG(MCancel)))
					{
						if (DelSubstDrive(DiskLetter))
						{
							return DRIVE_DEL_SUCCESS;
						}
					}
					else
					{
						return DRIVE_DEL_FAIL;
					}
				}
				Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText.data(),MSG(MOk));
			}
			return DRIVE_DEL_FAIL; // ����. � ������� ��� ����� ��� ��� ����...
		}
		break;

	case DRIVE_REMOTE:
	case DRIVE_REMOTE_NOT_CONNECTED:
		{
			int UpdateProfile=CONNECT_UPDATE_PROFILE;
			if (MessageRemoveConnection(Drive,UpdateProfile))
			{
				// <�������>
				LockScreen LckScr;
				// ���� �� ��������� �� ��������� ����� - ������ � ����, ����� �� ������
				// ��������
				IfGoHome(Drive);
				FrameManager->ResizeAllFrame();
				FrameManager->GetCurrentFrame()->Show();
				// </�������>

				if (WNetCancelConnection2(DiskLetter.data(),UpdateProfile,FALSE)==NO_ERROR)
				{
					return DRIVE_DEL_SUCCESS;
				}
				else
				{
					Global->CatchError();
					LangString strMsgText(MChangeDriveCannotDisconnect);
					strMsgText << DiskLetter;
					DWORD LastError = Global->CaughtError();
					if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
					{
						if (!Message(MSG_WARNING|MSG_ERRORTYPE, 2,
							MSG(MError),
							strMsgText.data(),
							L"\x1",
							MSG(MChangeDriveOpenFiles),
							MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel)))
						{
							if (WNetCancelConnection2(DiskLetter.data(),UpdateProfile,TRUE)==NO_ERROR)
							{
								return DRIVE_DEL_SUCCESS;
							}
						}
						else
						{
							return DRIVE_DEL_FAIL;
						}
					}
					const wchar_t RootDir[]={DiskLetter[0],L':',L'\\',L'\0'};
					if (FAR_GetDriveType(RootDir)==DRIVE_REMOTE)
					{
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText.data(),MSG(MOk));
					}
				}
				return DRIVE_DEL_FAIL;
			}
		}
		break;

	case DRIVE_VIRTUAL:
		{
			if (Global->Opt->Confirm.DetachVHD)
			{
				if (Message(MSG_WARNING, 2,
					MSG(MChangeVHDDisconnectDriveTitle),
					(LangString(MChangeVHDDisconnectDriveQuestion) << DiskLetter).data(),
					MSG(MYes),MSG(MNo)))
				{
					return DRIVE_DEL_FAIL;
				}
			}
			string strVhdPath;
			if(GetVHDName(DiskLetter, strVhdPath) && !strVhdPath.empty())
			{
				VIRTUAL_STORAGE_TYPE vst = {VIRTUAL_STORAGE_TYPE_DEVICE_VHD, VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT};
				OPEN_VIRTUAL_DISK_PARAMETERS ovdp = {OPEN_VIRTUAL_DISK_VERSION_1, 0};
				HANDLE Handle;
				if(api::OpenVirtualDisk(vst, strVhdPath, VIRTUAL_DISK_ACCESS_DETACH, OPEN_VIRTUAL_DISK_FLAG_NONE, ovdp, Handle))
				{
					int Result = Global->ifn->DetachVirtualDisk(Handle, DETACH_VIRTUAL_DISK_FLAG_NONE, 0) == ERROR_SUCCESS? DRIVE_DEL_SUCCESS : DRIVE_DEL_FAIL;
					CloseHandle(Handle);
					return Result;
				}
			}
		}
		break;

	}
	return DRIVE_DEL_FAIL;
}


void Panel::FastFindProcessName(Edit *FindEdit,const string& Src,string &strLastName,string &strName)
{
	wchar_t_ptr Buffer(Src.size()+StrLength(FindEdit->GetStringAddr()) + 1);

	if (Buffer)
	{
		auto Ptr = Buffer.get();
		wcscpy(Ptr,FindEdit->GetStringAddr());
		wchar_t *EndPtr=Ptr+StrLength(Ptr);
		wcscat(Ptr,Src.data());
		Unquote(EndPtr);
		EndPtr=Ptr+StrLength(Ptr);
		for (;;)
		{
			if (EndPtr == Ptr)
			{
				break;
			}

			if (FindPartName(Ptr,FALSE,1,1))
			{
				*EndPtr=0;
				FindEdit->SetString(Ptr);
				strLastName = Ptr;
				strName = Ptr;
				FindEdit->Show();
				break;
			}

			*--EndPtr=0;
		}
	}
}

__int64 Panel::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	return 0;
}

// ������������� ����
static DWORD _CorrectFastFindKbdLayout(const INPUT_RECORD *rec,DWORD Key)
{
	if ((Key&(KEY_ALT|KEY_RALT)))// && Key!=(KEY_ALT|0x3C))
	{
		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout>>> %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
		if (rec->Event.KeyEvent.uChar.UnicodeChar && (Key&KEY_MASKF) != rec->Event.KeyEvent.uChar.UnicodeChar) //???
			Key=(Key&0xFFF10000)|rec->Event.KeyEvent.uChar.UnicodeChar;   //???

		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout<<< %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
	}

	return Key;
}

void Panel::FastFind(int FirstKey)
{
	// // _SVS(CleverSysLog Clev(L"Panel::FastFind"));
	INPUT_RECORD rec;
	string strLastName, strName;
	int KeyToProcess=0;
	Global->WaitInFastFind++;
	{
		int FindX=std::min(X1+9,ScrX-22);
		int FindY=std::min(Y2,static_cast<SHORT>(ScrY-2));
		ChangeMacroMode MacroMode(MACROAREA_SEARCH);
		SaveScreen SaveScr(FindX,FindY,FindX+21,FindY+2);
		FastFindShow(FindX,FindY);
		EditControl FindEdit(this);
		FindEdit.SetPosition(FindX+2,FindY+1,FindX+19,FindY+1);
		FindEdit.SetEditBeyondEnd(FALSE);
		FindEdit.SetObjectColor(COL_DIALOGEDIT);
		FindEdit.Show();

		while (!KeyToProcess)
		{
			int Key;
			if (FirstKey)
			{
				FirstKey=_CorrectFastFindKbdLayout(FrameManager->GetLastInputRecord(),FirstKey);
				// // _SVS(SysLog(L"Panel::FastFind  FirstKey=%s  %s",_FARKEY_ToName(FirstKey),_INPUT_RECORD_Dump(FrameManager->GetLastInputRecord())));
				// // _SVS(SysLog(L"if (FirstKey)"));
				Key=FirstKey;
			}
			else
			{
				// // _SVS(SysLog(L"else if (FirstKey)"));
				Key=GetInputRecord(&rec);

				if (rec.EventType==MOUSE_EVENT)
				{
					if (!(rec.Event.MouseEvent.dwButtonState & 3))
						continue;
					else
						Key=KEY_ESC;
				}
				else if (!rec.EventType || rec.EventType==KEY_EVENT || rec.EventType==FARMACRO_KEY_EVENT)
				{
					// ��� ������� ������������� ������������...
					if (Key==KEY_CTRLV || Key==KEY_RCTRLV || Key==KEY_SHIFTINS || Key==KEY_SHIFTNUMPAD0)
					{
						string ClipText;
						if (GetClipboard(ClipText))
						{
							if (!ClipText.empty())
							{
								FastFindProcessName(&FindEdit,ClipText,strLastName,strName);
								FastFindShow(FindX,FindY);
							}
						}

						continue;
					}
					else if (Key == KEY_OP_XLAT)
					{
						string strTempName;
						FindEdit.Xlat();
						FindEdit.GetString(strTempName);
						FindEdit.SetString(L"");
						FastFindProcessName(&FindEdit,strTempName,strLastName,strName);
						FastFindShow(FindX,FindY);
						continue;
					}
					else if (Key == KEY_OP_PLAINTEXT)
					{
						string strTempName;
						FindEdit.ProcessKey(Key);
						FindEdit.GetString(strTempName);
						FindEdit.SetString(L"");
						FastFindProcessName(&FindEdit,strTempName,strLastName,strName);
						FastFindShow(FindX,FindY);
						continue;
					}
					else
						Key=_CorrectFastFindKbdLayout(&rec,Key);
				}
			}

			if (Key==KEY_ESC || Key==KEY_F10)
			{
				KeyToProcess=KEY_NONE;
				break;
			}

			// // _SVS(if (!FirstKey) SysLog(L"Panel::FastFind  Key=%s  %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(&rec)));
			if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_ALT_BASE));
			else if (Key>=KEY_RALT_BASE+0x01 && Key<=KEY_RALT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_RALT_BASE));

			if (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_ALTSHIFT_BASE));
			else if (Key>=KEY_RALTSHIFT_BASE+0x01 && Key<=KEY_RALTSHIFT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_RALTSHIFT_BASE));

			if (Key==KEY_MULTIPLY)
				Key=L'*';

			switch (Key)
			{
				case KEY_F1:
				{
					FindEdit.Hide();
					SaveScr.RestoreArea();
					{
						Help Hlp(L"FastFind");
					}
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				}
				case KEY_CTRLNUMENTER:   case KEY_RCTRLNUMENTER:
				case KEY_CTRLENTER:      case KEY_RCTRLENTER:
					FindPartName(strName,TRUE,1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_CTRLSHIFTNUMENTER:  case KEY_RCTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:     case KEY_RCTRLSHIFTENTER:
					FindPartName(strName,TRUE,-1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_NONE:
				case KEY_IDLE:
					break;
				default:

					if ((Key<32 || Key>=65536) && Key!=KEY_BS && Key!=KEY_CTRLY && Key!=KEY_RCTRLY &&
					        Key!=KEY_CTRLBS && Key!=KEY_RCTRLBS && Key!=KEY_ALT && Key!=KEY_SHIFT &&
					        Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
					        !(Key==KEY_CTRLINS||Key==KEY_CTRLNUMPAD0) && // KEY_RCTRLINS/NUMPAD0 passed to panels
							!(Key==KEY_SHIFTINS||Key==KEY_SHIFTNUMPAD0))
					{
						KeyToProcess=Key;
						break;
					}

					if (FindEdit.ProcessKey(Key))
					{
						FindEdit.GetString(strName);

						// ������ ������� '**'
						if (strName.size() > 1
						        && strName.back() == L'*'
						        && strName[strName.size()-2] == L'*')
						{
							strName.pop_back();
							FindEdit.SetString(strName.data());
						}

						/* $ 09.04.2001 SVS
						   �������� � ������� �������.
						   ��������� � 00573.ChangeDirCrash.txt
						*/
						if (strName.front() == L'"')
						{
							strName.erase(0, 1);
							FindEdit.SetString(strName.data());
						}

						if (FindPartName(strName,FALSE,1,1))
						{
							strLastName = strName;
						}
						else
						{
							if (Global->CtrlObject->Macro.IsExecuting())// && Global->CtrlObject->Macro.GetLevelState() > 0) // ���� ������� ��������...
							{
								//Global->CtrlObject->Macro.DropProcess(); // ... �� ������� ������������
								//Global->CtrlObject->Macro.PopState();
								;
							}

							FindEdit.SetString(strLastName.data());
							strName = strLastName;
						}

						FindEdit.Show();
						FastFindShow(FindX,FindY);
					}

					break;
			}

			FirstKey=0;
		}
	}
	Global->WaitInFastFind--;
	Show();
	Global->CtrlObject->MainKeyBar->Redraw();
	Global->ScrBuf->Flush();
	Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;

	if ((KeyToProcess==KEY_ENTER||KeyToProcess==KEY_NUMENTER) && ActivePanel->GetType()==TREE_PANEL)
		((TreeList *)ActivePanel)->ProcessEnter();
	else
		Global->CtrlObject->Cp()->ProcessKey(KeyToProcess);
}


void Panel::FastFindShow(int FindX,int FindY)
{
	SetColor(COL_DIALOGTEXT);
	GotoXY(FindX+1,FindY+1);
	Text(L" ");
	GotoXY(FindX+20,FindY+1);
	Text(L" ");
	Box(FindX,FindY,FindX+21,FindY+2,ColorIndexToColor(COL_DIALOGBOX),DOUBLE_BOX);
	GotoXY(FindX+7,FindY);
	SetColor(COL_DIALOGBOXTITLE);
	Text(MSearchFileTitle);
}


void Panel::SetFocus()
{
	if (Global->CtrlObject->Cp()->ActivePanel!=this)
	{
		Global->CtrlObject->Cp()->ActivePanel->KillFocus();
		Global->CtrlObject->Cp()->ActivePanel=this;
	}

	ProcessPluginEvent(FE_GOTFOCUS,nullptr);

	if (!GetFocus())
	{
		Global->CtrlObject->Cp()->RedrawKeyBar();
		Focus = true;
		Redraw();
		FarChDir(strCurDir);
	}
}


void Panel::KillFocus()
{
	Focus = false;
	ProcessPluginEvent(FE_KILLFOCUS,nullptr);
	Redraw();
}


int  Panel::PanelProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent,int &RetCode)
{
	RetCode=TRUE;

	if (!ModalMode && !MouseEvent->dwMousePosition.Y)
	{
		if (MouseEvent->dwMousePosition.X==ScrX)
		{
			if (Global->Opt->ScreenSaver && !(MouseEvent->dwButtonState & 3))
			{
				EndDrag();
				ScreenSaver(TRUE);
				return TRUE;
			}
		}
		else
		{
			if ((MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
			{
				EndDrag();

				if (!MouseEvent->dwMousePosition.X)
					Global->CtrlObject->Cp()->ProcessKey(KEY_CTRLO);
				else
					Global->Opt->ShellOptions(0,MouseEvent);

				return TRUE;
			}
		}
	}

	if (!IsVisible() ||
	        (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	         MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2))
	{
		RetCode=FALSE;
		return TRUE;
	}

	if (DragX!=-1)
	{
		if (!(MouseEvent->dwButtonState & 3))
		{
			EndDrag();

			if (!MouseEvent->dwEventFlags && SrcDragPanel!=this)
			{
				MoveToMouse(MouseEvent);
				Redraw();
				SrcDragPanel->ProcessKey(DragMove ? KEY_DRAGMOVE:KEY_DRAGCOPY);
			}

			return TRUE;
		}

		if (MouseEvent->dwMousePosition.Y<=Y1 || MouseEvent->dwMousePosition.Y>=Y2 ||
		        !Global->CtrlObject->Cp()->GetAnotherPanel(SrcDragPanel)->IsVisible())
		{
			EndDrag();
			return TRUE;
		}

		if ((MouseEvent->dwButtonState & 2) && !MouseEvent->dwEventFlags)
			DragMove=!DragMove;

		if (MouseEvent->dwButtonState & 1)
		{
			if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) &&
			        !ModalMode)
			{
				if (SrcDragPanel->GetSelCount()==1 && !DragSaveScr)
				{
					SrcDragPanel->GoToFile(strDragName);
					SrcDragPanel->Show();
				}

				DragMessage(MouseEvent->dwMousePosition.X,MouseEvent->dwMousePosition.Y,DragMove);
				return TRUE;
			}
			else
			{
				delete DragSaveScr;
				DragSaveScr=nullptr;
			}
		}
	}

	if (!(MouseEvent->dwButtonState & 3))
		return TRUE;

	if ((MouseEvent->dwButtonState & 1) && !MouseEvent->dwEventFlags &&
	        X2-X1<ScrX)
	{
		DWORD FileAttr;
		MoveToMouse(MouseEvent);
		GetSelName(nullptr,FileAttr);

		if (GetSelName(&strDragName,FileAttr) && !TestParentFolderName(strDragName))
		{
			SrcDragPanel=this;
			DragX=MouseEvent->dwMousePosition.X;
			DragY=MouseEvent->dwMousePosition.Y;
			DragMove=IntKeyState.ShiftPressed;
		}
	}

	return FALSE;
}


int  Panel::IsDragging()
{
	return DragSaveScr!=nullptr;
}


void Panel::EndDrag()
{
	delete DragSaveScr;
	DragSaveScr=nullptr;
	DragX=DragY=-1;
}


void Panel::DragMessage(int X,int Y,int Move)
{

	string strSelName;
	size_t SelCount;
	int MsgX,Length;

	if (!(SelCount=SrcDragPanel->GetSelCount()))
		return;

	if (SelCount==1)
	{
		string strCvtName;
		DWORD FileAttr;
		SrcDragPanel->GetSelName(nullptr,FileAttr);
		SrcDragPanel->GetSelName(&strSelName,FileAttr);
		strCvtName = PointToName(strSelName);
		QuoteSpace(strCvtName);
		strSelName = strCvtName;
	}
	else
	{
		strSelName = LangString(MDragFiles) << SelCount;
	}

	LangString strDragMsg(Move? MDragMove : MDragCopy);
	strDragMsg << strSelName;


	if ((Length=(int)strDragMsg.size())+X>ScrX)
	{
		MsgX=ScrX-Length;

		if (MsgX<0)
		{
			MsgX=0;
			TruncStrFromEnd(strDragMsg,ScrX);
			Length=(int)strDragMsg.size();
		}
	}
	else
		MsgX=X;

	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	delete DragSaveScr;
	DragSaveScr=new SaveScreen(MsgX,Y,MsgX+Length-1,Y);
	GotoXY(MsgX,Y);
	SetColor(COL_PANELDRAGTEXT);
	Text(strDragMsg);
}


const string& Panel::GetCurDir()
{
	return strCurDir; // TODO: ������!!!
}


bool Panel::SetCurDir(const string& CurDir,bool ClosePanel,bool /*IsUpdated*/)
{
	InitCurDir(CurDir);
	return true;
}


void Panel::InitCurDir(const string& CurDir)
{
	if (StrCmpI(strCurDir, CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		if (PanelMode!=PLUGIN_PANEL)
		{
			PrepareDiskPath(strCurDir);
			if(!IsRootPath(strCurDir))
			{
				DeleteEndSlash(strCurDir);
			}
		}
	}
}


/* $ 14.06.2001 KM
   + ��������� ��������� ���������� ���������, ������������
     ������� ���������� ������ ��� ��� ��������, ��� � ���
     ��������� ������. ��� ���������� ���������� �����������
     �� FAR.
*/
/* $ 05.10.2001 SVS
   ! ������� ��� ������ �������� ������ �������� ��� ��������� ������,
     � �� �����...
     � �� ����� �����-�� ����������...
*/
/* $ 14.01.2002 IS
   ! ����� ��������� ���������� ���������, ������ ��� ��� ������������
     � FarChDir, ������� ������ ������������ � ��� ��� ������������
     �������� ��������.
*/
int Panel::SetCurPath()
{
	if (GetMode()==PLUGIN_PANEL)
		return TRUE;

	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=PLUGIN_PANEL)
	{
		if (AnotherPanel->strCurDir.size() > 1 && AnotherPanel->strCurDir[1]==L':' &&
		        (strCurDir.empty() || Upper(AnotherPanel->strCurDir[0])!=Upper(strCurDir[0])))
		{
			// ������� ��������� ���������� ��������� ��� ��������� ������
			// (��� �������� ����� ����, ����� ������ ��� ��������� �������
			// �� ������������)
			FarChDir(AnotherPanel->strCurDir,FALSE);
		}
	}

	if (!FarChDir(strCurDir))
	{
		// ����� �� ����� :-)
#if 1

		while (!FarChDir(strCurDir))
		{
			string strRoot;
			GetPathRoot(strCurDir, strRoot);

			if (FAR_GetDriveType(strRoot) != DRIVE_REMOVABLE || api::IsDiskInDrive(strRoot))
			{
				int Result=TestFolder(strCurDir);

				if (Result == TSTFLD_NOTFOUND)
				{
					if (CheckShortcutFolder(&strCurDir, FALSE, TRUE) && FarChDir(strCurDir))
					{
						SetCurDir(strCurDir,true);
						return TRUE;
					}
				}
				else
					break;
			}

			if (FrameManager && FrameManager->ManagerStarted()) // ������� �������� - � ������� �� ��������
			{
				SetCurDir(Global->g_strFarPath,true);                    // ���� ������� - �������� ���� ������� �� ����� ����� ��� ����������
				ChangeDisk();                                    // � ������� ���� ������ ������
			}
			else                                               // ����...
			{
				string strTemp(strCurDir);
				CutToFolderNameIfFolder(strCurDir);             // ���������� �����, ��� ��������� ������ ChDir

				if (strTemp.size()==strCurDir.size())  // ����� �������� - ������ ���� ����������
				{
					SetCurDir(Global->g_strFarPath,true);                 // ����� ������ ��������� � �������, ������ ��������� FAR.
					break;
				}
				else
				{
					if (FarChDir(strCurDir))
					{
						SetCurDir(strCurDir,true);
						break;
					}
				}
			}
		}

#else

		do
		{
			BOOL IsChangeDisk=FALSE;
			char Root[1024];
			GetPathRoot(CurDir,Root);

			if (FAR_GetDriveType(Root) == DRIVE_REMOVABLE && !apiIsDiskInDrive(Root))
				IsChangeDisk=TRUE;
			else if (TestFolder(CurDir) == TSTFLD_NOTACCESS)
			{
				if (FarChDir(Root))
					SetCurDir(Root,true);
				else
					IsChangeDisk=TRUE;
			}

			if (IsChangeDisk)
				ChangeDisk();
		}
		while (!FarChDir(CurDir));

#endif
		return FALSE;
	}

	return TRUE;
}


void Panel::Hide()
{
	ScreenObject::Hide();
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible())
	{
		if (AnotherPanel->GetFocus())
			if ((AnotherPanel->GetType()==FILE_PANEL && AnotherPanel->IsFullScreen()) ||
			        (GetType()==FILE_PANEL && IsFullScreen()))
				AnotherPanel->Show();
	}
}


void Panel::Show()
{
	if (Locked())
		return;

	DelayDestroy dd(this);

	/* $ 03.10.2001 IS ���������� ������� ���� */
	if (Global->Opt->ShowMenuBar)
		Global->CtrlObject->TopMenuBar->Show();

	/* $ 09.05.2001 OT */
//  SavePrevScreen();
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible() && !GetModalMode())
	{
		if (SaveScr)
		{
			SaveScr->AppendArea(AnotherPanel->SaveScr);
		}

		if (AnotherPanel->GetFocus())
		{
			if (AnotherPanel->IsFullScreen())
			{
				SetVisible(TRUE);
				return;
			}

			if (GetType()==FILE_PANEL && IsFullScreen())
			{
				ScreenObject::Show();
				AnotherPanel->Show();
				return;
			}
		}
	}

	ScreenObject::Show();
	if (!this->Destroyed())
		ShowScreensCount();
}


void Panel::DrawSeparator(int Y)
{
	if (Y<Y2)
	{
		SetColor(COL_PANELBOX);
		GotoXY(X1,Y);
		ShowSeparator(X2-X1+1,1);
	}
}


void Panel::ShowScreensCount()
{
	if (Global->Opt->ShowScreensNumber && !X1)
	{
		int Viewers=FrameManager->GetFrameCountByType(MODALTYPE_VIEWER);
		int Editors=FrameManager->GetFrameCountByType(MODALTYPE_EDITOR);
		int Dialogs=FrameManager->GetFrameCountByType(MODALTYPE_DIALOG);

		if (Viewers>0 || Editors>0 || Dialogs > 0)
		{
			GotoXY(Global->Opt->ShowColumnTitles ? X1:X1+2,Y1);
			SetColor(COL_PANELSCREENSNUMBER);

			Global->FS << L"[" << Viewers;
			if (Editors > 0)
			{
				Global->FS << L"+" << Editors;
			}

			if (Dialogs > 0)
			{
				Global->FS << L"," << Dialogs;
			}

			Global->FS << L"]";
		}
	}
}


void Panel::SetTitle()
{
	if (GetFocus())
	{
		string strTitleDir(L"{");

		if (!strCurDir.empty())
		{
			strTitleDir += strCurDir;
		}
		else
		{
			string strCmdText;
			Global->CtrlObject->CmdLine->GetCurDir(strCmdText);
			strTitleDir += strCmdText;
		}

		strTitleDir += L"}";

		ConsoleTitle::SetFarTitle(strTitleDir);
	}
}

const string& Panel::GetTitle(string &strTitle)
{
	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		GetOpenPanelInfo(&Info);
		strTitle = NullToEmpty(Info.PanelTitle);
		RemoveExternalSpaces(strTitle);
	}
	else
	{
		if (ShowShortNames)
			ConvertNameToShort(strCurDir,strTitle);
		else
			strTitle = strCurDir;

	}

	return strTitle;
}

int Panel::SetPluginCommand(int Command,int Param1,void* Param2)
{
	_ALGO(CleverSysLog clv(L"Panel::SetPluginCommand"));
	_ALGO(SysLog(L"(Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	int Result=FALSE;
	ProcessingPluginCommand++;
	FilePanels *FPanels=Global->CtrlObject->Cp();

	switch (Command)
	{
		case FCTL_SETVIEWMODE:
			Result=FPanels->ChangePanelViewMode(this,Param1,FPanels->IsTopFrame());
			break;

		case FCTL_SETSORTMODE:
		{
			int Mode=Param1;

			if ((Mode>SM_DEFAULT) && (Mode<=SM_CHTIME))
			{
				SetSortMode(--Mode); // �������� �� 1 ��-�� SM_DEFAULT
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETNUMERICSORT:
		{
			ChangeNumericSort(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETCASESENSITIVESORT:
		{
			ChangeCaseSensitiveSort(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETSORTORDER:
		{
			ChangeSortOrder(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETDIRECTORIESFIRST:
		{
			ChangeDirectoriesFirst(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_CLOSEPANEL:
			PluginCommand=Command;
			strPluginParam = NullToEmpty((const wchar_t *)Param2);
			Result=TRUE;
			break;

		case FCTL_GETPANELINFO:
		{
			PanelInfo *Info=(PanelInfo *)Param2;

			if(!CheckStructSize(Info))
				break;

			ClearStruct(*Info);
			Info->StructSize = sizeof(PanelInfo);

			UpdateIfRequired();
			Info->OwnerGuid=FarGuid;
			Info->PluginHandle=nullptr;

			switch (GetType())
			{
				case FILE_PANEL:
					Info->PanelType=PTYPE_FILEPANEL;
					break;
				case TREE_PANEL:
					Info->PanelType=PTYPE_TREEPANEL;
					break;
				case QVIEW_PANEL:
					Info->PanelType=PTYPE_QVIEWPANEL;
					break;
				case INFO_PANEL:
					Info->PanelType=PTYPE_INFOPANEL;
					break;
			}

			int X1,Y1,X2,Y2;
			GetPosition(X1,Y1,X2,Y2);
			Info->PanelRect.left=X1;
			Info->PanelRect.top=Y1;
			Info->PanelRect.right=X2;
			Info->PanelRect.bottom=Y2;
			Info->ViewMode=GetViewMode();
			Info->SortMode=static_cast<OPENPANELINFO_SORTMODES>(SM_UNSORTED-UNSORTED+GetSortMode());

			Info->Flags |= Global->Opt->ShowHidden? PFLAGS_SHOWHIDDEN : 0;
			Info->Flags |= Global->Opt->Highlight? PFLAGS_HIGHLIGHT : 0;
			Info->Flags |= GetSortOrder()? PFLAGS_REVERSESORTORDER : 0;
			Info->Flags |= GetSortGroups()? PFLAGS_USESORTGROUPS : 0;
			Info->Flags |= GetSelectedFirstMode()? PFLAGS_SELECTEDFIRST : 0;
			Info->Flags |= GetDirectoriesFirst()? PFLAGS_DIRECTORIESFIRST : 0;
			Info->Flags |= GetNumericSort()? PFLAGS_NUMERICSORT : 0;
			Info->Flags |= GetCaseSensitiveSort()? PFLAGS_CASESENSITIVESORT : 0;
			Info->Flags |= (GetMode()==PLUGIN_PANEL)? PFLAGS_PLUGIN : 0;
			Info->Flags |= IsVisible()? PFLAGS_VISIBLE : 0;
			Info->Flags |= GetFocus()? PFLAGS_FOCUS : 0;
                        Info->Flags |= this == Global->CtrlObject->Cp()->LeftPanel? PFLAGS_PANELLEFT : 0;

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;

				if (Info->Flags&PFLAGS_PLUGIN)
				{
					Info->OwnerGuid = static_cast<PluginHandle*>(DestFilePanel->GetPluginHandle())->pPlugin->GetGUID();
					Info->PluginHandle = static_cast<PluginHandle*>(DestFilePanel->GetPluginHandle())->hPlugin;
					static int Reenter=0;
					if (!Reenter)
					{
						Reenter++;
						OpenPanelInfo PInfo;
						DestFilePanel->GetOpenPanelInfo(&PInfo);

						if (PInfo.Flags & OPIF_REALNAMES)
							Info->Flags |= PFLAGS_REALNAMES;

						if (PInfo.Flags & OPIF_DISABLEHIGHLIGHTING)
							Info->Flags &= ~PFLAGS_HIGHLIGHT;

						if (PInfo.Flags & OPIF_USECRC32)
							Info->Flags |= PFLAGS_USECRC32;

						if (PInfo.Flags & OPIF_SHORTCUT)
							Info->Flags |= PFLAGS_SHORTCUT;

						Reenter--;
					}
				}

				DestFilePanel->PluginGetPanelInfo(*Info);
			}

			if (!(Info->Flags&PFLAGS_PLUGIN)) // $ 12.12.2001 DJ - �� ������������ ������ - ������ �������� �����
				Info->Flags |= PFLAGS_REALNAMES;

			Result=TRUE;
			break;
		}

		case FCTL_GETPANELPREFIX:
		{
			string strTemp;

			if (GetType()==FILE_PANEL && GetMode() == PLUGIN_PANEL)
			{
				PluginInfo PInfo = {sizeof(PInfo)};
				FileList *DestPanel = ((FileList*)this);
				if (DestPanel->GetPluginInfo(&PInfo))
					strTemp = NullToEmpty(PInfo.CommandPrefix);
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp.data(),Param1);

			Result=(int)strTemp.size()+1;
			break;
		}

		case FCTL_GETPANELHOSTFILE:
		case FCTL_GETPANELFORMAT:
		{
			string strTemp;

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;
				static int Reenter=0;

				if (!Reenter && GetMode()==PLUGIN_PANEL)
				{
					Reenter++;

					OpenPanelInfo PInfo;
					DestFilePanel->GetOpenPanelInfo(&PInfo);

					switch (Command)
					{
						case FCTL_GETPANELHOSTFILE:
							strTemp=NullToEmpty(PInfo.HostFile);
							break;
						case FCTL_GETPANELFORMAT:
							strTemp=NullToEmpty(PInfo.Format);
							break;
					}

					Reenter--;
				}
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp.data(),Param1);

			Result=(int)strTemp.size()+1;
			break;
		}
		case FCTL_GETPANELDIRECTORY:
		{
			static int Reenter=0;
			if(!Reenter)
			{
				Reenter++;
				ShortcutInfo Info;
				GetShortcutInfo(Info);
				Result=ALIGN(sizeof(FarPanelDirectory));
				size_t folderOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.ShortcutFolder.size()+1));
				size_t pluginFileOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginFile.size()+1));
				size_t pluginDataOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginData.size()+1));
				FarPanelDirectory* dirInfo=(FarPanelDirectory*)Param2;
				if(Param1>=Result && CheckStructSize(dirInfo))
				{
					dirInfo->StructSize=sizeof(FarPanelDirectory);
					dirInfo->PluginId=Info.PluginGuid;
					dirInfo->Name=(wchar_t*)((char*)Param2+folderOffset);
					dirInfo->Param=(wchar_t*)((char*)Param2+pluginDataOffset);
					dirInfo->File=(wchar_t*)((char*)Param2+pluginFileOffset);
					wmemcpy(const_cast<wchar_t*>(dirInfo->Name),Info.ShortcutFolder.data(), Info.ShortcutFolder.size()+1);
					wmemcpy(const_cast<wchar_t*>(dirInfo->Param), Info.PluginData.data(), Info.PluginData.size()+1);
					wmemcpy(const_cast<wchar_t*>(dirInfo->File), Info.PluginFile.data(), Info.PluginFile.size()+1);
				}
				Reenter--;
			}
			break;
		}

		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:

			if (GetType()==FILE_PANEL)
			{
				string strColumnTypes,strColumnWidths;
				((FileList *)this)->PluginGetColumnTypesAndWidths(strColumnTypes,strColumnWidths);

				if (Command==FCTL_GETCOLUMNTYPES)
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnTypes.data(),Param1);

					Result=(int)strColumnTypes.size()+1;
				}
				else
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnWidths.data(),Param1);

					Result=(int)strColumnWidths.size()+1;
				}
			}
			break;

		case FCTL_GETPANELITEM:
		{
			Result=CheckNullOrStructSize((FarGetPluginPanelItem*)Param2)?(int)((FileList*)this)->PluginGetPanelItem(Param1,(FarGetPluginPanelItem*)Param2):0;
			break;
		}

		case FCTL_GETSELECTEDPANELITEM:
		{
			Result=CheckNullOrStructSize((FarGetPluginPanelItem*)Param2)?(int)((FileList*)this)->PluginGetSelectedPanelItem(Param1,(FarGetPluginPanelItem*)Param2):0;
			break;
		}

		case FCTL_GETCURRENTPANELITEM:
		{
			PanelInfo Info;
			FileList *DestPanel = ((FileList*)this);
			DestPanel->PluginGetPanelInfo(Info);
			Result = CheckNullOrStructSize((FarGetPluginPanelItem*)Param2)?(int)DestPanel->PluginGetPanelItem(static_cast<int>(Info.CurrentItem),(FarGetPluginPanelItem*)Param2):0;
			break;
		}

		case FCTL_BEGINSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginBeginSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginSetSelection(Param1, Param2 != nullptr);
				Result=TRUE;
			}
			break;
		}

		case FCTL_CLEARSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				static_cast<FileList*>(this)->PluginClearSelection(Param1);
				Result=TRUE;
			}
			break;
		}

		case FCTL_ENDSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginEndSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_UPDATEPANEL:
			Update(Param1?UPDATE_KEEP_SELECTION:0);

			if (GetType() == QVIEW_PANEL)
				UpdateViewPanel();

			Result=TRUE;
			break;

		case FCTL_REDRAWPANEL:
		{
			PanelRedrawInfo *Info=(PanelRedrawInfo *)Param2;

			if (CheckStructSize(Info))
			{
				CurFile=static_cast<int>(Info->CurrentItem);
				CurTopFile=static_cast<int>(Info->TopPanelItem);
			}

			// $ 12.05.2001 DJ ���������������� ������ � ��� ������, ���� �� - ������� �����
			if (FPanels->IsTopFrame())
				Redraw();

			Result=TRUE;
			break;
		}

		case FCTL_SETPANELDIRECTORY:
		{
			FarPanelDirectory* dirInfo=(FarPanelDirectory*)Param2;
			if (CheckStructSize(dirInfo))
			{
				string strName(NullToEmpty(dirInfo->Name)), strFile(NullToEmpty(dirInfo->File)), strParam(NullToEmpty(dirInfo->Param));
				Result = ExecShortcutFolder(strName,dirInfo->PluginId,strFile,strParam,false);
				// restore current directory to active panel path
				Panel* ActivePanel = Global->CtrlObject->Cp()->ActivePanel;
				if (Result && this != ActivePanel)
				{
					ActivePanel->SetCurPath();
				}
			}
			break;
		}

		case FCTL_SETACTIVEPANEL:
		{
			if (IsVisible())
			{
				SetFocus();
				Result=TRUE;
			}
			break;
		}
	}

	ProcessingPluginCommand--;
	return Result;
}


int Panel::GetCurName(string &strName, string &strShortName)
{
	strName.clear();
	strShortName.clear();
	return FALSE;
}


int Panel::GetCurBaseName(string &strName, string &strShortName)
{
	strName.clear();
	strShortName.clear();
	return FALSE;
}

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile)
{
	/*
	  0         1         2         3         4         5         6         7
	  0123456789012345678901234567890123456789012345678901234567890123456789012345
	0
	1   +-------- ���������� �������� ���������� --------+
	2   | �� ������ ������� ���������� � ����������� C:? |
	3   | �� ���������� %c: ��������� �������            |
	4   | \\host\share                                   |
	6   +------------------------------------------------+
	7   | [ ] ��������������� ��� ����� � �������        |
	8   +------------------------------------------------+
	9   |              [ �� ]   [ ������ ]               |
	10  +------------------------------------------------+
	11
	*/
	FarDialogItem DCDlgData[]=
	{
		{DI_DOUBLEBOX, 3, 1, 72, 9, 0, nullptr, nullptr, 0,                MSG(MChangeDriveDisconnectTitle)},
		{DI_TEXT,      5, 2,  0, 2, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,L""},
		{DI_TEXT,      5, 3,  0, 3, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,L""},
		{DI_TEXT,      5, 4,  0, 4, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,L""},
		{DI_TEXT,     -1, 5,  0, 5, 0, nullptr, nullptr, DIF_SEPARATOR,    L""},
		{DI_CHECKBOX,  5, 6, 70, 6, 0, nullptr, nullptr, 0,                MSG(MChangeDriveDisconnectReconnect)},
		{DI_TEXT,     -1, 7,  0, 7, 0, nullptr, nullptr, DIF_SEPARATOR,    L""},
		{DI_BUTTON,    0, 8,  0, 8, 0, nullptr, nullptr, DIF_FOCUS|DIF_DEFAULTBUTTON|DIF_CENTERGROUP, MSG(MYes)},
		{DI_BUTTON,    0, 8,  0, 8, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MCancel)},
	};
	auto DCDlg = MakeDialogItemsEx(DCDlgData);

	LangString strMsgText;

	strMsgText = MChangeDriveDisconnectQuestion;
	strMsgText << Letter;
	DCDlg[1].strData = strMsgText;

	strMsgText = MChangeDriveDisconnectMapped;
	strMsgText << Letter;
	DCDlg[2].strData = strMsgText;

	size_t Len1 = DCDlg[0].strData.size();
	size_t Len2 = DCDlg[1].strData.size();
	size_t Len3 = DCDlg[2].strData.size();
	size_t Len4 = DCDlg[5].strData.size();
	Len1 = std::max(Len1,std::max(Len2,std::max(Len3,Len4)));
	DriveLocalToRemoteName(DRIVE_REMOTE,Letter,strMsgText);
	DCDlg[3].strData = TruncPathStr(strMsgText, static_cast<int>(Len1));
	// ��������� - ��� ���� ���������� �������� ��� ���?
	// ���� ����� � ������� HKCU\Network\���������� ���� - ���
	//   ���� ���������� �����������.
	BOOL IsPersistent = FALSE;
	{
		HKEY hKey;
		IsPersistent=TRUE;
		string KeyName(L"Network\\");
		KeyName+=Letter;

		if (RegOpenKeyEx(HKEY_CURRENT_USER,KeyName.data(),0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
		{
			DCDlg[5].Flags|=DIF_DISABLE;
			DCDlg[5].Selected=0;
			IsPersistent=FALSE;
			RegCloseKey(hKey);
		}
		else
			DCDlg[5].Selected=Global->Opt->ChangeDriveDisconnectMode;
	}
	// ������������� ������� ������� - ��� �������
	DCDlg[0].X2=DCDlg[0].X1+static_cast<int>(Len1)+3;
	int ExitCode=7;

	if (Global->Opt->Confirm.RemoveConnection)
	{
		Dialog Dlg(DCDlg);
		Dlg.SetPosition(-1,-1,DCDlg[0].X2+4,11);
		Dlg.SetHelp(L"DisconnectDrive");
		Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
	}

	UpdateProfile=DCDlg[5].Selected?0:CONNECT_UPDATE_PROFILE;

	if (IsPersistent)
		Global->Opt->ChangeDriveDisconnectMode=DCDlg[5].Selected == BSTATE_CHECKED;

	return ExitCode == 7;
}

BOOL Panel::NeedUpdatePanel(const Panel *AnotherPanel)
{
	/* ��������, ���� ���������� ��������� � ���� ��������� */
	if ((!Global->Opt->AutoUpdateLimit || static_cast<unsigned>(GetFileCount()) <= static_cast<unsigned>(Global->Opt->AutoUpdateLimit)) &&
	        !StrCmpI(AnotherPanel->strCurDir, strCurDir))
		return TRUE;

	return FALSE;
}

bool Panel::GetShortcutInfo(ShortcutInfo& ShortcutInfo)
{
	bool result=true;
	if (PanelMode==PLUGIN_PANEL)
	{
		HANDLE hPlugin=GetPluginHandle();
		PluginHandle *ph = (PluginHandle*)hPlugin;
		ShortcutInfo.PluginGuid = ph->pPlugin->GetGUID();
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		ShortcutInfo.PluginFile = NullToEmpty(Info.HostFile);
		ShortcutInfo.ShortcutFolder = NullToEmpty(Info.CurDir);
		ShortcutInfo.PluginData = NullToEmpty(Info.ShortcutData);
		if(!(Info.Flags&OPIF_SHORTCUT)) result=false;
	}
	else
	{
		ShortcutInfo.PluginGuid=FarGuid;
		ShortcutInfo.PluginFile.clear();
		ShortcutInfo.PluginData.clear();
		ShortcutInfo.ShortcutFolder = strCurDir;
	}
	return result;
}

bool Panel::SaveShortcutFolder(int Pos, bool Add)
{
	ShortcutInfo Info;
	if(GetShortcutInfo(Info))
	{
		if(Add)
		{
			Shortcuts().Add(Pos, Info.ShortcutFolder, Info.PluginGuid, Info.PluginFile, Info.PluginData);
		}
		else
		{
			Shortcuts().Set(Pos, Info.ShortcutFolder, Info.PluginGuid, Info.PluginFile, Info.PluginData);
		}
		return true;
	}
	return false;
}

/*
int Panel::ProcessShortcutFolder(int Key,BOOL ProcTreePanel)
{
	string strShortcutFolder, strPluginModule, strPluginFile, strPluginData;

	if (GetShortcutFolder(Key-KEY_RCTRL0,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
	{
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

		if (ProcTreePanel)
		{
			if (AnotherPanel->GetType()==FILE_PANEL)
			{
				AnotherPanel->SetCurDir(strShortcutFolder,true);
				AnotherPanel->Redraw();
			}
			else
			{
				SetCurDir(strShortcutFolder,true);
				ProcessKey(KEY_ENTER);
			}
		}
		else
		{
			if (AnotherPanel->GetType()==FILE_PANEL && !strPluginModule.empty())
			{
				AnotherPanel->SetCurDir(strShortcutFolder,true);
				AnotherPanel->Redraw();
			}
		}

		return TRUE;
	}

	return FALSE;
}
*/

bool Panel::ExecShortcutFolder(int Pos, bool raw)
{
	string strShortcutFolder,strPluginFile,strPluginData;
	GUID PluginGuid;

	if (Shortcuts().Get(Pos,&strShortcutFolder, &PluginGuid, &strPluginFile, &strPluginData, raw))
	{
		return ExecShortcutFolder(strShortcutFolder,PluginGuid,strPluginFile,strPluginData,true);
	}
	return false;
}

bool Panel::ExecShortcutFolder(string& strShortcutFolder, const GUID& PluginGuid, const string& strPluginFile, const string& strPluginData, bool CheckType)
{
	Panel *SrcPanel=this;
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if(CheckType)
	{
		switch (GetType())
		{
			case TREE_PANEL:
				if (AnotherPanel->GetType()==FILE_PANEL)
					SrcPanel=AnotherPanel;
				break;

			case QVIEW_PANEL:
			case INFO_PANEL:
			{
				if (AnotherPanel->GetType()==FILE_PANEL)
					SrcPanel=AnotherPanel;
				break;
			}
		}
	}

	bool CheckFullScreen=SrcPanel->IsFullScreen();

	if (PluginGuid != FarGuid)
	{
		switch (CheckShortcutFolder(nullptr,TRUE))
		{
			case 0:
				//              return FALSE;
			case -1:
				return true;
		}

		Plugin *pPlugin = Global->CtrlObject->Plugins->FindPlugin(PluginGuid);

		if (pPlugin)
		{
			if (pPlugin->HasOpen())
			{
				if (!strPluginFile.empty())
				{
					string strRealDir;
					strRealDir = strPluginFile;

					if (CutToSlash(strRealDir))
					{
						SrcPanel->SetCurDir(strRealDir,true);
						SrcPanel->GoToFile(PointToName(strPluginFile));

						SrcPanel->ClearAllItem();
					}
				}

				OpenShortcutInfo info=
				{
					sizeof(OpenShortcutInfo),
					strPluginFile.empty()?nullptr:strPluginFile.data(),
					strPluginData.empty()?nullptr:strPluginData.data(),
					(SrcPanel==Global->CtrlObject->Cp()->ActivePanel)?FOSF_ACTIVE:FOSF_NONE
				};
				HANDLE hNewPlugin=Global->CtrlObject->Plugins->Open(pPlugin,OPEN_SHORTCUT,FarGuid,(intptr_t)&info);

				if (hNewPlugin)
				{
					int CurFocus=SrcPanel->GetFocus();

					Panel *NewPanel=Global->CtrlObject->Cp()->ChangePanel(SrcPanel,FILE_PANEL,TRUE,TRUE);
					NewPanel->SetPluginMode(hNewPlugin,L"",CurFocus || !Global->CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());

					if (!strShortcutFolder.empty())
					{
						struct UserDataItem UserData={0}; //????
						Global->CtrlObject->Plugins->SetDirectory(hNewPlugin,strShortcutFolder,0,&UserData);
					}

					NewPanel->Update(0);
					NewPanel->Show();
				}
			}
		}

		return true;
	}

	switch (CheckShortcutFolder(&strShortcutFolder,FALSE))
	{
		case 0:
			//          return FALSE;
		case -1:
			return true;
	}

    /*
	if (SrcPanel->GetType()!=FILE_PANEL)
	{
		SrcPanel=Global->CtrlObject->Cp()->ChangePanel(SrcPanel,FILE_PANEL,TRUE,TRUE);
	}
    */

	SrcPanel->SetCurDir(strShortcutFolder,true);

	if (CheckFullScreen!=SrcPanel->IsFullScreen())
		Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->Show();

	SrcPanel->Redraw();
	return true;
}
