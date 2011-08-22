#include <plugin.hpp>
#include <CRT/crt.hpp>
#include <PluginSettings.hpp>
#include <SimpleString.hpp>
#include "CompareLng.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const wchar_t *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#define CheckDisabled(i) (!((int)Info.SendDlgMessage(hDlg,DM_ENABLE,i,(void *)-1)))

/****************************************************************************
 * ����� ��� ���������� ��������� startup-���� ��� ���������� ��� GCC
 ****************************************************************************/
#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"
{
#endif
	BOOL WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	(void) lpReserved;
	(void) dwReason;
	(void) hDll;
	return TRUE;
}
#endif

/****************************************************************************
 * ������� ��������� �������
 ****************************************************************************/
struct Options
{
	int ProcessSubfolders,
	UseMaxScanDepth,
	MaxScanDepth,
	ProcessSelected,
	ProcessHidden,
	CompareTime,
	LowPrecisionTime,
	IgnorePossibleTimeZoneDifferences,
	CompareSize,
	CompareContents,
	CompareContentsIgnore,
	IgnoreWhitespace,
	IgnoreNewLines,
	MessageWhenNoDiff;
} Opt;

/****************************************************************************
 * ����� ����������� �������� FAR
 ****************************************************************************/
static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;

static void WFD2FFD(WIN32_FIND_DATA &wfd, PluginPanelItem &ffd)
{
	memset(&ffd, 0, sizeof(ffd));
	ffd.FileAttributes     = wfd.dwFileAttributes;
	ffd.CreationTime       = wfd.ftCreationTime;
	ffd.LastAccessTime     = wfd.ftLastAccessTime;
	ffd.LastWriteTime      = wfd.ftLastWriteTime;
	ffd.FileSize           = ((__int64)wfd.nFileSizeHigh << 32) | wfd.nFileSizeLow;
	ffd.PackSize           = 0;
	ffd.FileName           = wcsdup(wfd.cFileName);
	ffd.AlternateFileName  = wcsdup(wfd.cAlternateFileName);
}

struct OwnPanelInfo
{
	int PanelType;
	bool Plugin;
	PluginPanelItem *PanelItems;
	size_t ItemsNumber;
	PluginPanelItem *SelectedItems;
	size_t SelectedItemsNumber;
	wchar_t *lpwszCurDir;
};

/****************************************************************************
 * ������ ��������� ������� FAR: ��������� ������ �� .lng-�����
 ****************************************************************************/
static const wchar_t *GetMsg(int CompareLng)
{
	return Info.GetMsg(&MainGuid, CompareLng);
}


static int iTruncLen;

/****************************************************************************
 * ������� ������ ������� ���� ������ (��� ��������� �������� �����)
 * ��� ����������� ������ � ��������� ���������
 ****************************************************************************/
static void TrunCopy(wchar_t *cpDest, const wchar_t *cpSrc)
{
	int iLen = (int)lstrlen(FSF.TruncStr(lstrcpy(cpDest, cpSrc), iTruncLen));

	if (iLen < iTruncLen)
	{
		wmemset(&cpDest[iLen], L' ', iTruncLen - iLen);
		cpDest[iTruncLen] = L'\0';
	}
}

static bool bStart;
static bool bOpenFail;

/****************************************************************************
 * ���������� ��������� � ��������� ���� ������
 ****************************************************************************/
static void ShowMessage(const wchar_t *Name1, const wchar_t *Name2)
{
	static DWORD dwTicks;
	DWORD dwNewTicks = GetTickCount();

	if (dwNewTicks - dwTicks < 500)
		return;

	dwTicks = dwNewTicks;
	wchar_t TruncName1[MAX_PATH], TruncName2[MAX_PATH];
	TrunCopy(TruncName1, Name1);
	TrunCopy(TruncName2, Name2);
	const wchar_t *MsgItems[] =
	{
		GetMsg(MCmpTitle),
		GetMsg(MComparing),
		TruncName1,
		GetMsg(MComparingWith),
		TruncName2
	};
	Info.Message(&MainGuid, nullptr, bStart ? FMSG_LEFTALIGN :
	             FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,
	             NULL, MsgItems, ARRAYSIZE(MsgItems), 0);
	bStart = false;
}


/****************************************************************************
 * ���������� ������� ��� ShowDialog
 ****************************************************************************/
INT_PTR WINAPI ShowDialogProc(HANDLE hDlg, int Msg, int Param1, void *Param2)
{
	static int CompareContents,
	CompareContentsIgnore,
	ProcessSubfolders,
	CompareTime;

	switch (Msg)
	{
		case DN_INITDIALOG:
			CompareContents = ((DWORD)Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0)) & 0x000000FF;
			CompareContentsIgnore = CompareContents + 1;
			ProcessSubfolders = (((DWORD)Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0)) >> 8) & 0x000000FF;
			CompareTime = ((DWORD)Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0)) >> 16;
			break;
		case DN_BTNCLICK:

			if (Param1 == CompareTime || Param1 == ProcessSubfolders || Param1 == CompareContents || Param1 == CompareContentsIgnore)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+1, (void *)TRUE);

					if (!(Param1 == CompareContents && !Info.SendDlgMessage(hDlg, DM_GETCHECK, CompareContentsIgnore, 0)))
					{
						Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+2, (void *)TRUE);

						if (Param1 == CompareContents)
							Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+3, (void *)TRUE);
					}
				}
				else
				{
					Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+1, (void *)FALSE);
					Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+2, (void *)FALSE);

					if (Param1 == CompareContents)
						Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+3, (void *)FALSE);
				}
			}

			break;
	}

	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

/****************************************************************************
 * ������ ��������� �� �������, ���������� ������ � ������� ���������,
 * ��������� ��������� Opt, ��������� (���� ����) ����� ��������� � �������,
 * ���������� true, ���� ������������ ����� OK
 ****************************************************************************/
static bool ShowDialog(bool bPluginPanels, bool bSelectionPresent)
{
	static struct InitDialogItem
	{
		FARDIALOGITEMTYPES Type;
		unsigned char X1, Y1, X2, Y2;
		int           Data;
		int           DefaultRegValue;
		const wchar_t   *SelectedRegValue;
		unsigned int  Flags;
		int          *StoreTo;
	} InitItems[] =
	{
		/* 0*/ { DI_DOUBLEBOX,    3,  1, 62, 20, MCmpTitle,                0, NULL,                                 0, NULL },
		/* 1*/ { DI_TEXT,         5,  2,  0,  0, MProcessBox,              0, NULL,                                 0, NULL },
		/* 2*/ { DI_CHECKBOX,     5,  3,  0,  0, MProcessSubfolders,       0, L"ProcessSubfolders",                 0, &Opt.ProcessSubfolders },
		/* 3*/ { DI_CHECKBOX,     9,  4,  0,  0, MUseMaxScanDepth,         0, L"UseMaxScanDepth",                   0, &Opt.UseMaxScanDepth },
		/* 4*/ { DI_FIXEDIT,      0,  4,  4,  0, MNoLngStringDefined,     99, L"MaxScanDepth",           DIF_MASKEDIT, &Opt.MaxScanDepth },
		/* 5*/ { DI_CHECKBOX,     5,  5,  0,  0, MProcessSelected,         0, L"ProcessSelected",                   0, &Opt.ProcessSelected },
		/* 6*/ { DI_TEXT,         0,  6,  0,  0, MNoLngStringDefined,      0, NULL,                     DIF_SEPARATOR, NULL },
		/* 7*/ { DI_TEXT,         5,  7,  0,  0, MCompareBox,              0, NULL,                                 0, NULL },
		/* 8*/ { DI_CHECKBOX,     5,  8,  0,  0, MCompareTime,             1, L"CompareTime",                       0, &Opt.CompareTime },
		/* 9*/ { DI_CHECKBOX,     9,  9,  0,  0, MCompareLowPrecision,     1, L"LowPrecisionTime",                  0, &Opt.LowPrecisionTime },
		/*10*/ { DI_CHECKBOX,     9, 10,  0,  0, MCompareIgnoreTimeZone,   1, L"IgnorePossibleTimeZoneDifferences", 0, &Opt.IgnorePossibleTimeZoneDifferences },
		/*11*/ { DI_CHECKBOX,     5, 11,  0,  0, MCompareSize,             1, L"CompareSize",                       0, &Opt.CompareSize },
		/*12*/ { DI_CHECKBOX,     5, 12,  0,  0, MCompareContents,         0, L"CompareContents",                   0, &Opt.CompareContents },
		/*13*/ { DI_CHECKBOX,     9, 13,  0,  0, MCompareContentsIgnore,   0, L"CompareContentsIgnore",             0, &Opt.CompareContentsIgnore },
		/*14*/ { DI_RADIOBUTTON, 13, 14,  0,  0, MCompareIgnoreNewLines,   1, L"IgnoreNewLines",            DIF_GROUP, &Opt.IgnoreNewLines },
		/*15*/ { DI_RADIOBUTTON, 13, 15,  0,  0, MCompareIgnoreWhitespace, 0, L"IgnoreWhitespace",                  0, &Opt.IgnoreWhitespace },
		/*16*/ { DI_TEXT,         0, 16,  0,  0, MNoLngStringDefined,      0, NULL,                     DIF_SEPARATOR, NULL },
		/*17*/ { DI_CHECKBOX,     5, 17,  0,  0, MMessageWhenNoDiff,       0, L"MessageWhenNoDiff",                 0, &Opt.MessageWhenNoDiff },
		/*18*/ { DI_TEXT,         0, 18,  0,  0, MNoLngStringDefined,      0, NULL,                     DIF_SEPARATOR, NULL },
		/*19*/ { DI_BUTTON,       0, 19,  0,  0, MOK,                      0, NULL,                   DIF_CENTERGROUP, NULL },
		/*20*/ { DI_BUTTON,       0, 19,  0,  0, MCancel,                  0, NULL,                   DIF_CENTERGROUP, NULL }
	};
	struct FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
	wchar_t Mask[] = L"99999";
	wchar_t tmpnum[ARRAYSIZE(InitItems)][32];
	memset(DialogItems,0,sizeof(DialogItems));
	PluginSettings settings(MainGuid, Info.SettingsControl);
	size_t DlgData=0;
	bool bNoFocus = true;
	size_t i;

	for (i = 0; i < ARRAYSIZE(InitItems); i++)
	{
		DialogItems[i].Type           = InitItems[i].Type;
		DialogItems[i].X1             = InitItems[i].X1;
		DialogItems[i].Y1             = InitItems[i].Y1;
		DialogItems[i].X2             = InitItems[i].X2;
		DialogItems[i].Y2             = InitItems[i].Y2;
		DialogItems[i].Flags          = InitItems[i].Flags;
		DialogItems[i].Data = (InitItems[i].Data == MNoLngStringDefined) ? L"" : GetMsg(InitItems[i].Data);
		int Value = InitItems[i].SelectedRegValue ? settings.Get(0, InitItems[i].SelectedRegValue, InitItems[i].DefaultRegValue) : InitItems[i].DefaultRegValue;

		if (DialogItems[i].Type == DI_CHECKBOX || DialogItems[i].Type == DI_RADIOBUTTON)
		{
			DialogItems[i].Selected = Value;
		}
		else if (DialogItems[i].Type == DI_FIXEDIT)
		{
			DialogItems[i].Data = tmpnum[i];
			FSF.itoa(Value, (wchar_t *)DialogItems[i].Data, 10);
			DialogItems[i].Mask = Mask;
			DialogItems[i].X1 = DialogItems[i-1].X1 + lstrlen(DialogItems[i-1].Data) - (wcschr(DialogItems[i-1].Data, L'&')?1:0) + 5;
			DialogItems[i].X2 += DialogItems[i].X1;
		}

		switch (InitItems[i].Data)
		{
			case MCompareContents:
				DlgData += i;

				if (bPluginPanels)
				{
					DialogItems[i].Flags |= DIF_DISABLE;
					DialogItems[i].Selected = 0;
				}

				if (!DialogItems[i].Selected)
				{
					InitItems[i+1].Flags |= DIF_DISABLE;
					InitItems[i+2].Flags |= DIF_DISABLE;
					InitItems[i+3].Flags |= DIF_DISABLE;
				}
				else
				{
					InitItems[i+1].Flags &= ~DIF_DISABLE;
					InitItems[i+2].Flags &= ~DIF_DISABLE;
					InitItems[i+3].Flags &= ~DIF_DISABLE;
				}

				break;
			case MCompareContentsIgnore:

				if (!DialogItems[i].Selected || DialogItems[i].Flags & DIF_DISABLE)
				{
					InitItems[i+1].Flags |= DIF_DISABLE;
					InitItems[i+2].Flags |= DIF_DISABLE;
				}
				else
				{
					InitItems[i+1].Flags &= ~DIF_DISABLE;
					InitItems[i+2].Flags &= ~DIF_DISABLE;
				}

				break;
			case MCompareIgnoreWhitespace:

				if (DialogItems[i].Selected == DialogItems[i-1].Selected)
				{
					DialogItems[i-1].Selected = 1;
					DialogItems[i].Selected = 0;
				}

				break;
			case MProcessSubfolders:
				DlgData += i<<8;

				if (bPluginPanels)
				{
					DialogItems[i].Flags |= DIF_DISABLE;
					DialogItems[i].Selected = 0;
				}

				if (!DialogItems[i].Selected)
				{
					InitItems[i+1].Flags |= DIF_DISABLE;
					InitItems[i+2].Flags |= DIF_DISABLE;
				}
				else
				{
					InitItems[i+1].Flags &= ~DIF_DISABLE;
					InitItems[i+2].Flags &= ~DIF_DISABLE;
				}

				break;
			case MProcessSelected:

				if (!bSelectionPresent)
				{
					DialogItems[i].Flags |= DIF_DISABLE;
					DialogItems[i].Selected = 0;
				}

				break;
			case MCompareTime:
				DlgData += i<<16;

				if (!DialogItems[i].Selected)
				{
					InitItems[i+1].Flags |= DIF_DISABLE;
					InitItems[i+2].Flags |= DIF_DISABLE;
				}
				else
				{
					InitItems[i+1].Flags &= ~DIF_DISABLE;
					InitItems[i+2].Flags &= ~DIF_DISABLE;
				}

				break;
			case MOK:
				DialogItems[i].Flags |= DIF_DEFAULTBUTTON;
				break;
		}

		if (bNoFocus && DialogItems[i].Type == DI_CHECKBOX && !(DialogItems[i].Flags & DIF_DISABLE))
		{
			DialogItems[i].Flags |= DIF_FOCUS;
			bNoFocus = false;
		}
	}

	HANDLE hDlg = Info.DialogInit(&MainGuid, &DialogGuid, -1, -1, 66, 22, L"Contents",
	                              DialogItems, ARRAYSIZE(DialogItems), 0, 0,
	                              ShowDialogProc, (void *)DlgData);

	if (hDlg == INVALID_HANDLE_VALUE)
		return false;

	int ExitCode = Info.DialogRun(hDlg);

	if (ExitCode == (ARRAYSIZE(InitItems) - 2))
	{
		for (i = 0; i < ARRAYSIZE(InitItems); i++)
		{
			if (InitItems[i].StoreTo)
			{
				if (InitItems[i].Type == DI_CHECKBOX || InitItems[i].Type == DI_RADIOBUTTON)
					*InitItems[i].StoreTo = (DWORD)GetCheck((int)i);
				else if (InitItems[i].Type == DI_FIXEDIT)
					*InitItems[i].StoreTo = FSF.atoi(GetDataPtr((int)i));
			}
		}

		for (i = 0; i < ARRAYSIZE(InitItems); i++)
		{
			if (!(CheckDisabled((int)i)) && InitItems[i].SelectedRegValue)
			{
				settings.Set(0, InitItems[i].SelectedRegValue, *InitItems[i].StoreTo);
			}
		}

		if (bPluginPanels)
		{
			Opt.ProcessSubfolders = FALSE;
			Opt.CompareContents = FALSE;
		}

		Opt.ProcessHidden = (Info.AdvControl(&MainGuid, ACTL_GETPANELSETTINGS, 0, NULL) & FPS_SHOWHIDDENANDSYSTEMFILES) != 0;
		Info.DialogFree(hDlg);
		return true;
	}

	Info.DialogFree(hDlg);
	return false;
}

static bool bBrokenByEsc;
static HANDLE hConInp = INVALID_HANDLE_VALUE;

/****************************************************************************
 * �������� �� Esc. ���������� true, ���� ������������ ����� Esc
 ****************************************************************************/
static bool CheckForEsc(void)
{
	if (hConInp == INVALID_HANDLE_VALUE)
		return false;

	static DWORD dwTicks;
	DWORD dwNewTicks = GetTickCount();

	if (dwNewTicks - dwTicks < 500)
		return false;

	dwTicks = dwNewTicks;
	INPUT_RECORD rec;
	DWORD ReadCount;

	while (PeekConsoleInput(hConInp, &rec, 1, &ReadCount) && ReadCount)
	{
		ReadConsoleInput(hConInp, &rec, 1, &ReadCount);

		if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
		        rec.Event.KeyEvent.bKeyDown)
			// ������������ ������������� ���������� �� Esc
		{
			if (Info.AdvControl(&MainGuid, ACTL_GETCONFIRMATIONS, 0, NULL) & FCS_INTERRUPTOPERATION)
			{
				const wchar_t *MsgItems[] =
				{
					GetMsg(MEscTitle),
					GetMsg(MEscBody),
					GetMsg(MOK),
					GetMsg(MCancel)
				};

				if (!Info.Message(&MainGuid, nullptr, FMSG_WARNING, NULL, MsgItems, ARRAYSIZE(MsgItems), 2))
					return bBrokenByEsc = true;
			}
			else
			{
				return bBrokenByEsc = true;
			}
		}
	}

	return false;
}

/****************************************************************************
 * ������ ������ ��� ����� �� ���� � �����
 ****************************************************************************/
static wchar_t *BuildFullFilename(const wchar_t *cpDir, const wchar_t *cpFileName)
{
	static wchar_t cName[MAX_PATH];
	FSF.AddEndSlash(lstrcpy(cName, cpDir));
	return lstrcat(cName, cpFileName);
}

struct FileIndex
{
	PluginPanelItem **ppi;
	int iCount;
};

/****************************************************************************
 * ������� ��������� ��� ������ � ���� ���������� PluginPanelItem
 * ��� ���� qsort()
 ****************************************************************************/
static int __cdecl PICompare(const void *el1, const void *el2)
{
	const PluginPanelItem *ppi1 = *(const PluginPanelItem **)el1, *ppi2 = *(const PluginPanelItem **)el2;

	if (ppi1->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (!(ppi2->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return -1;
	}
	else
	{
		if (ppi2->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return 1;
	}

	return -FSF.LStricmp(ppi1->FileName, ppi2->FileName);
}

/****************************************************************************
 * ���������� �������������� ������ ������ ��� �������� ���������
 ****************************************************************************/
static bool BuildPanelIndex(const OwnPanelInfo *pInfo, struct FileIndex *pIndex, HANDLE Filter)
{
	bool bProcessSelected;
	pIndex->ppi = NULL;
	pIndex->iCount = (int)((bProcessSelected = (Opt.ProcessSelected && pInfo->SelectedItemsNumber &&
	                        (pInfo->SelectedItems[0].Flags & PPIF_SELECTED))) ? pInfo->SelectedItemsNumber :
	                       pInfo->ItemsNumber);

	if (!pIndex->iCount)
		return true;

	if (!(pIndex->ppi = (PluginPanelItem **)malloc(pIndex->iCount*sizeof(pIndex->ppi[0]))))
		return false;

	int j = 0;

	for (int i = (int)pInfo->ItemsNumber - 1; i >= 0 && j < pIndex->iCount; i--)
	{
		if ((Opt.ProcessSubfolders ||
		        !(pInfo->PanelItems[i].FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) &&
		        (!bProcessSelected || (pInfo->PanelItems[i].Flags & PPIF_SELECTED)) &&
		        lstrcmp(pInfo->PanelItems[i].FileName, L"..") &&
		        lstrcmp(pInfo->PanelItems[i].FileName, L"."))
		{
			if (!Info.FileFilterControl(Filter,FFCTL_ISFILEINFILTER,0,&pInfo->PanelItems[i]))
				continue;

			pIndex->ppi[j++] = &pInfo->PanelItems[i];
		}
	}

	if ((pIndex->iCount = j) != 0)
	{
		FSF.qsort(pIndex->ppi, j, sizeof(pIndex->ppi[0]), PICompare);
	}
	else
	{
		free(pIndex->ppi);
		pIndex->ppi = NULL;
		pIndex->iCount = 0;
	}

	return true;
}

/****************************************************************************
 * ������������ ������
 ****************************************************************************/
static void FreePanelIndex(struct FileIndex *pIndex)
{
	if (pIndex->ppi)
		free(pIndex->ppi);

	pIndex->ppi = NULL;
	pIndex->iCount = 0;
}

/****************************************************************************
 * ������ ��������� ������� Info.GetDirList(). � ������� �� ���� ����������
 * ������ ������ ������ � �������� Dir, ��� ������������.
 ****************************************************************************/
static int GetDirList(OwnPanelInfo *PInfo, const wchar_t *Dir)
{
	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind;
	struct PluginPanelItem **pPanelItem = &PInfo->PanelItems;
	size_t *pItemsNumber = &PInfo->ItemsNumber;
	PInfo->lpwszCurDir = wcsdup(Dir);
	*pPanelItem = NULL;
	*pItemsNumber = 0;
	string strPathMask(Dir);
	strPathMask += L"\\*";

	if ((hFind = FindFirstFile(strPathMask, &wfdFindData)) == INVALID_HANDLE_VALUE)
		return TRUE;

	int iRet = TRUE;

	do
	{
		if (!lstrcmp(wfdFindData.cFileName, L".") || !lstrcmp(wfdFindData.cFileName, L".."))
			continue;

		if ((wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !Opt.ProcessHidden)
			continue;

		struct PluginPanelItem *pPPI;

		if (!(pPPI = (struct PluginPanelItem *)realloc(*pPanelItem, (*pItemsNumber + 1) * sizeof(*pPPI))))
		{
			iRet = FALSE;
			break;
		}

		*pPanelItem = pPPI;
		WFD2FFD(wfdFindData,(*pPanelItem)[(*pItemsNumber)++]);
	}
	while (FindNextFile(hFind, &wfdFindData));

	FindClose(hFind);
	return iRet;
}

/****************************************************************************
 * ������ ��������� ������� Info.FreeDirList().
 ****************************************************************************/
static void FreeDirList(OwnPanelInfo *AInfo)
{
	if (AInfo->PanelItems)
	{
		for (size_t i = 0; i < AInfo->ItemsNumber; i++)
		{
			free((void*)AInfo->PanelItems[i].AlternateFileName);
			free((void*)AInfo->PanelItems[i].FileName);
		}

		free(AInfo->lpwszCurDir);
		free(AInfo->PanelItems);
	}
}

static bool CompareDirs(const OwnPanelInfo *AInfo, const OwnPanelInfo *PInfo, bool bCompareAll, int ScanDepth);
static DWORD bufSize;
static HANDLE AFilter, PFilter;

//TODO: ��� ����� (�� ����� CompareFiles) �� ������������ � unicode/locale
//      �, ��� ����, � ��������������� ������
static char *ABuf, *PBuf;

bool isnewline(int c)
{
	return (c == '\r' || c == '\n');
}

/****************************************************************************
 * ��������� ��������� � ������� ��� ���� ���������� ��������� (������ ���
 * ������������).
 * ���������� true, ���� ��� ���������.
 ****************************************************************************/
static bool CompareFiles(const PluginPanelItem *AData, const PluginPanelItem *PData,
                         const wchar_t *ACurDir, const wchar_t *PCurDir, int ScanDepth)
{
	if (AData->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		// ����� ���������� ��� �����������
		if (Opt.ProcessSubfolders)
		{
			if (Opt.UseMaxScanDepth && Opt.MaxScanDepth<ScanDepth+1)
				return true;

			// �������� ������ ������ � ������������
			OwnPanelInfo AInfo, PInfo;
			memset(&AInfo, 0, sizeof(AInfo));
			memset(&PInfo, 0, sizeof(PInfo));
			bool bEqual;

			if (!GetDirList(&AInfo, BuildFullFilename(ACurDir, AData->FileName))
			        || !GetDirList(&PInfo, BuildFullFilename(PCurDir, PData->FileName)))
			{
				bBrokenByEsc = true; // �� �� ���� �������, �� �� ������ ������
				bEqual = false; // ��������� ���������
			}
			else
				bEqual = CompareDirs(&AInfo, &PInfo, false, ScanDepth+1);

			FreeDirList(&AInfo);
			FreeDirList(&PInfo);
			return bEqual;
		}
	}
	else
	{
		// ����� ���������� ��� �����
		if (Opt.CompareSize)
			if (AData->FileSize != PData->FileSize)
				return false;

		if (Opt.CompareTime)
		{
			if (Opt.LowPrecisionTime || Opt.IgnorePossibleTimeZoneDifferences)
			{
				union
				{
					__int64 num;
					struct
					{
						DWORD lo;
						DWORD hi;
					} hilo;
				} Precision, Difference, TimeDelta, temp;
				Precision.hilo.hi = 0;
				Precision.hilo.lo = Opt.LowPrecisionTime ? 20000000 : 0; //2s or 0s
				Difference.num = _i64(9000000000); //15m

				if (AData->LastWriteTime.dwHighDateTime > PData->LastWriteTime.dwHighDateTime)
				{
					TimeDelta.hilo.hi = AData->LastWriteTime.dwHighDateTime - PData->LastWriteTime.dwHighDateTime;
					TimeDelta.hilo.lo = AData->LastWriteTime.dwLowDateTime  - PData->LastWriteTime.dwLowDateTime;

					if (TimeDelta.hilo.lo > AData->LastWriteTime.dwLowDateTime)
						--TimeDelta.hilo.hi;
				}
				else
				{
					if (AData->LastWriteTime.dwHighDateTime == PData->LastWriteTime.dwHighDateTime)
					{
						TimeDelta.hilo.hi = 0;
						TimeDelta.hilo.lo = max(PData->LastWriteTime.dwLowDateTime,AData->LastWriteTime.dwLowDateTime)-
						                    min(PData->LastWriteTime.dwLowDateTime,AData->LastWriteTime.dwLowDateTime);
					}
					else
					{
						TimeDelta.hilo.hi = PData->LastWriteTime.dwHighDateTime - AData->LastWriteTime.dwHighDateTime;
						TimeDelta.hilo.lo = PData->LastWriteTime.dwLowDateTime  - AData->LastWriteTime.dwLowDateTime;

						if (TimeDelta.hilo.lo > PData->LastWriteTime.dwLowDateTime)
							--TimeDelta.hilo.hi;
					}
				}

				//������������ �������� �� ������ ��� 26 �����.
				if (Opt.IgnorePossibleTimeZoneDifferences)
				{
					int counter=0;

					while (TimeDelta.hilo.hi > Difference.hilo.hi && counter<=26*4)
					{
						temp.hilo.lo = TimeDelta.hilo.lo - Difference.hilo.lo;
						temp.hilo.hi = TimeDelta.hilo.hi - Difference.hilo.hi;

						if (temp.hilo.lo > TimeDelta.hilo.lo)
							--temp.hilo.hi;

						TimeDelta.hilo.lo = temp.hilo.lo;
						TimeDelta.hilo.hi = temp.hilo.hi;
						++counter;
					}

					if (counter<=26*4 && TimeDelta.hilo.hi == Difference.hilo.hi)
					{
						TimeDelta.hilo.hi = 0;
						TimeDelta.hilo.lo = max(TimeDelta.hilo.lo,Difference.hilo.lo) - min(TimeDelta.hilo.lo,Difference.hilo.lo);
					}
				}

				if (Precision.hilo.hi < TimeDelta.hilo.hi ||
				        (Precision.hilo.hi == TimeDelta.hilo.hi && Precision.hilo.lo < TimeDelta.hilo.lo))
					return false;
			}
			else if (AData->LastWriteTime.dwLowDateTime  != PData->LastWriteTime.dwLowDateTime ||
			         AData->LastWriteTime.dwHighDateTime != PData->LastWriteTime.dwHighDateTime)
				return false;
		}

		if (Opt.CompareContents)
		{
			HANDLE hFileA, hFileP;
			wchar_t cpFileA[MAX_PATH], cpFileP[MAX_PATH];
			ShowMessage(lstrcpy(cpFileA, BuildFullFilename(ACurDir, AData->FileName)),
			            lstrcpy(cpFileP, BuildFullFilename(PCurDir, PData->FileName)));

			if ((hFileA = CreateFile(cpFileA, GENERIC_READ, FILE_SHARE_READ , NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
			{
				bOpenFail = true;
				return false;
			}

			if ((hFileP = CreateFile(cpFileP, GENERIC_READ, FILE_SHARE_READ , NULL,
			                         OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFileA);
				bOpenFail = true;
				return false;
			}

			bool bEqual = true;
			DWORD ReadSizeA, ReadSizeP;

			if (!Opt.CompareContentsIgnore)
			{
				do
				{
					if (CheckForEsc()
					        || !ReadFile(hFileA, ABuf, bufSize, &ReadSizeA, NULL)
					        || !ReadFile(hFileP, PBuf, bufSize, &ReadSizeP, NULL)
					        || ReadSizeA != ReadSizeP
					        || memcmp(ABuf, PBuf, ReadSizeA))
					{
						bEqual = false;
						break;
					}
				}
				while (ReadSizeA == bufSize);
			}
			else
			{
				ReadSizeA=1;
				ReadSizeP=1;
				char *PtrA=ABuf+ReadSizeA, *PtrP=PBuf+ReadSizeP;
				bool bExpectNewLineA = false;
				bool bExpectNewLineP = false;

				while (true)
				{
					while (PtrA >= ABuf+ReadSizeA && ReadSizeA)
					{
						if (CheckForEsc() || !ReadFile(hFileA, ABuf, bufSize, &ReadSizeA, NULL))
						{
							bEqual = false;
							break;
						}

						PtrA=ABuf;
					}

					if (!bEqual)
						break;

					while (PtrP >= PBuf+ReadSizeP && ReadSizeP)
					{
						if (CheckForEsc() || !ReadFile(hFileP, PBuf, bufSize, &ReadSizeP, NULL))
						{
							bEqual = false;
							break;
						}

						PtrP=PBuf;
					}

					if (!bEqual || (!ReadSizeP && !ReadSizeA))
						break;

					if (Opt.IgnoreWhitespace)
					{
						while (PtrA < ABuf+ReadSizeA && PtrP < PBuf+ReadSizeP && !isspace(*PtrA) && !isspace(*PtrP))
						{
							if (*PtrA != *PtrP)
							{
								bEqual = false;
								break;
							}

							++PtrA;
							++PtrP;
						}

						if (!bEqual)
							break;

						while (PtrA < ABuf+ReadSizeA && isspace(*PtrA))
							++PtrA;

						while (PtrP < PBuf+ReadSizeP && isspace(*PtrP))
							++PtrP;
					}
					else
					{
						if (bExpectNewLineA)
						{
							bExpectNewLineA = false;

							if (PtrA < ABuf+ReadSizeA && *PtrA == '\n')
								++PtrA;
						}

						if (bExpectNewLineP)
						{
							bExpectNewLineP = false;

							if (PtrP < PBuf+ReadSizeP && *PtrP == '\n')
								++PtrP;
						}

						while (PtrA < ABuf+ReadSizeA && PtrP < PBuf+ReadSizeP && !isnewline(*PtrA) && !isnewline(*PtrP))
						{
							if (*PtrA != *PtrP)
							{
								bEqual = false;
								break;
							}

							++PtrA;
							++PtrP;
						}

						if (!bEqual)
							break;

						if (PtrA < ABuf+ReadSizeA && PtrP < PBuf+ReadSizeP && (!isnewline(*PtrA) || !isnewline(*PtrP)))
						{
							bEqual = false;
							break;
						}

						if (PtrA < ABuf+ReadSizeA && PtrP < PBuf+ReadSizeP)
						{
							if (*PtrA == '\r')
								bExpectNewLineA = true;

							if (*PtrP == '\r')
								bExpectNewLineP = true;

							++PtrA;
							++PtrP;
						}
					}

					if (PtrA < ABuf+ReadSizeA && !ReadSizeP)
					{
						bEqual = false;
						break;
					}

					if (PtrP < PBuf+ReadSizeP && !ReadSizeA)
					{
						bEqual = false;
						break;
					}
				}
			}

			CloseHandle(hFileA);
			CloseHandle(hFileP);
			return bEqual;
		}
	}

	return true;
}

/****************************************************************************
 * ��������� ���� ���������, ��������� ����������� AInfo � PInfo.
 * ���������� true, ���� ��� ���������.
 * �������� bCompareAll ����������,
 * ���� �� ���������� ��� ����� � �������� PPIF_SELECTED (bCompareAll == true)
 * ��� ������ ������� false ��� ������ ������������ (bCompareAll == false).
 ****************************************************************************/
static bool CompareDirs(const OwnPanelInfo *AInfo, const OwnPanelInfo *PInfo, bool bCompareAll, int ScanDepth)
{
	// ������ ������� ������ ��� �������� ���������
	struct FileIndex sfiA, sfiP;
	wchar_t DirA[MAX_PATH], DirP[MAX_PATH];
	ShowMessage(lstrcpy(DirA, BuildFullFilename(AInfo->lpwszCurDir, L"*")),
	            lstrcpy(DirP, BuildFullFilename(PInfo->lpwszCurDir, L"*")));

	if (!BuildPanelIndex(AInfo, &sfiA, AFilter) || !BuildPanelIndex(PInfo, &sfiP, PFilter))
	{
		const wchar_t *MsgItems[] =
		{
			GetMsg(MNoMemTitle),
			GetMsg(MNoMemBody),
			GetMsg(MOK)
		};
		Info.Message(&MainGuid, nullptr, FMSG_WARNING, NULL, MsgItems, ARRAYSIZE(MsgItems), 1);
		bBrokenByEsc = true;
		FreePanelIndex(&sfiA);
		FreePanelIndex(&sfiP);
		return true;
	}

	bool bDifferenceNotFound = true;
	int i = sfiA.iCount - 1, j = sfiP.iCount - 1;

	while (i >= 0 && j >= 0 && (bDifferenceNotFound || bCompareAll) && !bBrokenByEsc)
	{
		const int iMaxCounter = 256;
		static int iCounter = iMaxCounter;

		if (!--iCounter)
		{
			iCounter = iMaxCounter;

			if (CheckForEsc())
				break;
		}

		switch (PICompare(&sfiA.ppi[i], &sfiP.ppi[j]))
		{
			case 0: // ����� ������� - ��������� �� ���������

				if (CompareFiles(sfiA.ppi[i], sfiP.ppi[j], AInfo->lpwszCurDir, PInfo->lpwszCurDir, ScanDepth))
				{ // � ��������� �������
					sfiA.ppi[i--]->Flags &= ~PPIF_SELECTED;
					sfiP.ppi[j--]->Flags &= ~PPIF_SELECTED;
				}
				else
				{
					bDifferenceNotFound = false;
					sfiA.ppi[i--]->Flags |= PPIF_SELECTED;
					sfiP.ppi[j--]->Flags |= PPIF_SELECTED;
				}

				break;
			case 1: // ������� sfiA.ppi[i] �� ����� ���������� � sfiP.ppi
				bDifferenceNotFound = false;
				sfiA.ppi[i--]->Flags |= PPIF_SELECTED;
				break;
			case -1: // ������� sfiP.ppi[j] �� ����� ���������� � sfiA.ppi
				bDifferenceNotFound = false;
				sfiP.ppi[j--]->Flags |= PPIF_SELECTED;
				break;
		}
	}

	if (!bBrokenByEsc)
	{
		// ���������� ��������� ��������. ������� ��, ��� �������� �������������� � ��������
		if (i >= 0)
		{
			bDifferenceNotFound = false;

			if (bCompareAll)
				for (; i >= 0; i--)
					sfiA.ppi[i]->Flags |= PPIF_SELECTED;
		}

		if (j >= 0)
		{
			bDifferenceNotFound = false;

			if (bCompareAll)
				for (; j >= 0; j--)
					sfiP.ppi[j]->Flags |= PPIF_SELECTED;
		}
	}

	FreePanelIndex(&sfiA);
	FreePanelIndex(&sfiP);
	return bDifferenceNotFound;
}

/****************************************************************************
 ***************************** Exported functions ***************************
 ****************************************************************************/

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

/****************************************************************************
 * ��� ������� ������� FAR �������� � ������ �������
 ****************************************************************************/
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info = *Info;
	FSF = *Info->FSF;
}

/****************************************************************************
 * ��� ������� ������� FAR �������� �� ������ �������
 ****************************************************************************/
void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=0;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MCompare);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

void GetPanelItem(HANDLE hPlugin,FILE_CONTROL_COMMANDS Command,int Param1,PluginPanelItem* Param2)
{
	size_t Size = Info.PanelControl(hPlugin,Command,Param1,0);
	PluginPanelItem* item=(PluginPanelItem*)malloc(Size);

	if (item)
	{
		FarGetPluginPanelItem gpi = {Size, item};
		Info.PanelControl(hPlugin,Command,Param1,&gpi);
		*Param2=*item;
		Param2->FileName=wcsdup(item->FileName);
		Param2->AlternateFileName=wcsdup(item->AlternateFileName);
		Param2->Description=NULL;
		Param2->Owner=NULL;
		Param2->CustomColumnData=NULL;
		Param2->CustomColumnNumber=0;
		Param2->UserData=0;
		free(item);
	}
}

void FreePanelItems(OwnPanelInfo &AInfo,OwnPanelInfo &PInfo)
{
	for (size_t i=0; i<AInfo.ItemsNumber; i++)
	{
		free((void*)AInfo.PanelItems[i].FileName);
		free((void*)AInfo.PanelItems[i].AlternateFileName);
	}

	for (size_t i=0; i<AInfo.SelectedItemsNumber; i++)
	{
		free((void*)AInfo.SelectedItems[i].FileName);
		free((void*)AInfo.SelectedItems[i].AlternateFileName);
	}

	delete[] AInfo.PanelItems;
	delete[] AInfo.SelectedItems;

	for (size_t i=0; i<PInfo.ItemsNumber; i++)
	{
		free((void*)PInfo.PanelItems[i].FileName);
		free((void*)PInfo.PanelItems[i].AlternateFileName);
	}

	for (size_t i=0; i<PInfo.SelectedItemsNumber; i++)
	{
		free((void*)PInfo.SelectedItems[i].FileName);
		free((void*)PInfo.SelectedItems[i].AlternateFileName);
	}

	delete[] PInfo.PanelItems;
	delete[] PInfo.SelectedItems;
}


/****************************************************************************
 * �������� ������� �������. FAR � ��������, ����� ������������ ���� ������
 ****************************************************************************/
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	OwnPanelInfo AInfo, PInfo;
	memset(&AInfo,0,sizeof(OwnPanelInfo));
	memset(&PInfo,0,sizeof(OwnPanelInfo));
	PanelInfo AI,PI;
	Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO,0,&AI);
	Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO,0,&PI);
	AInfo.PanelType=AI.PanelType;
	AInfo.Plugin=(AI.Flags&PFLAGS_PLUGIN) == PFLAGS_PLUGIN;
	AInfo.ItemsNumber=AI.ItemsNumber;
	AInfo.SelectedItemsNumber=AI.SelectedItemsNumber;
	int Size=(int)Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELDIR,0,0);
	AInfo.lpwszCurDir=new wchar_t[Size];
	Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELDIR,Size,AInfo.lpwszCurDir);

	if (AInfo.ItemsNumber)
	{
		AInfo.PanelItems=new PluginPanelItem[AInfo.ItemsNumber];

		for (size_t i=0; i<AInfo.ItemsNumber; i++)
			GetPanelItem(PANEL_ACTIVE, FCTL_GETPANELITEM,i,&AInfo.PanelItems[i]);
	}
	else
	{
		AInfo.PanelItems=NULL;
	}

	if (AInfo.SelectedItemsNumber)
	{
		AInfo.SelectedItems=new PluginPanelItem[AInfo.SelectedItemsNumber];

		for (size_t i=0; i<AInfo.SelectedItemsNumber; i++)
			GetPanelItem(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM,i,&AInfo.SelectedItems[i]);
	}
	else
	{
		AInfo.SelectedItems=NULL;
	}

	PInfo.PanelType=PI.PanelType;
	PInfo.Plugin=(PI.Flags&PFLAGS_PLUGIN) == PFLAGS_PLUGIN;
	PInfo.ItemsNumber=PI.ItemsNumber;
	PInfo.SelectedItemsNumber=PI.SelectedItemsNumber;
	Size=(int)Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELDIR,0,0);
	PInfo.lpwszCurDir=new wchar_t[Size];
	Info.PanelControl(PANEL_PASSIVE, FCTL_GETPANELDIR,Size,PInfo.lpwszCurDir);

	if (PInfo.ItemsNumber)
	{
		PInfo.PanelItems=new PluginPanelItem[PInfo.ItemsNumber];

		for (size_t i=0; i<PInfo.ItemsNumber; i++)
			GetPanelItem(PANEL_PASSIVE, FCTL_GETPANELITEM,i,&PInfo.PanelItems[i]);
	}
	else
	{
		PInfo.PanelItems=NULL;
	}

	if (PInfo.SelectedItemsNumber)
	{
		PInfo.SelectedItems=new PluginPanelItem[PInfo.SelectedItemsNumber];

		for (size_t i=0; i<PInfo.SelectedItemsNumber; i++)
			GetPanelItem(PANEL_PASSIVE, FCTL_GETSELECTEDPANELITEM,i,&PInfo.SelectedItems[i]);
	}
	else
	{
		PInfo.SelectedItems=NULL;
	}

	// ���� ������ ����������...
	if (AInfo.PanelType != PTYPE_FILEPANEL || PInfo.PanelType != PTYPE_FILEPANEL)
	{
		const wchar_t *MsgItems[] =
		{
			GetMsg(MCmpTitle),
			GetMsg(MFilePanelsRequired),
			GetMsg(MOK)
		};
		Info.Message(&MainGuid, nullptr, FMSG_WARNING, NULL, MsgItems, ARRAYSIZE(MsgItems), 1);
		FreePanelItems(AInfo,PInfo);
		return INVALID_HANDLE_VALUE;
	}

	// ���� �� ����� �������� ������ �������...
	if (!ShowDialog(AInfo.Plugin || PInfo.Plugin,
	                (AInfo.SelectedItemsNumber && (AInfo.SelectedItems[0].Flags & PPIF_SELECTED)) ||
	                (PInfo.SelectedItemsNumber && (PInfo.SelectedItems[0].Flags & PPIF_SELECTED))))
	{
		FreePanelItems(AInfo,PInfo);
		return INVALID_HANDLE_VALUE;
	}

	// ������� ���������� ���� ��� �������� �� Esc...
	HANDLE hScreen = Info.SaveScreen(0, 0, -1, -1);
	hConInp = CreateFile(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	// ��������� ����������� ������ ������� ���������...
	SMALL_RECT rcFar = {0};

	if (Info.AdvControl(&MainGuid, ACTL_GETFARRECT, 0, &rcFar))
	{
		SHORT X = rcFar.Right - rcFar.Left + 1;

		if ((iTruncLen = X - 20) > MAX_PATH - 2)
			iTruncLen = MAX_PATH - 2;
		else if (iTruncLen < 0)
			iTruncLen = X - X / 4;
	}
	else
	{
		iTruncLen = 60;
	}

	// �� ����� ��������� ������� ��������� ������� ����...
	wchar_t cConsoleTitle[MAX_PATH], cBuffer[MAX_PATH];
	DWORD dwTitleSaved = GetConsoleTitle(cConsoleTitle, sizeof(cConsoleTitle));
	lstrcpy(cBuffer, GetMsg(MComparingFiles));
	SetConsoleTitle(cBuffer);
	// ������ ������ ������ ��������� �� ��������...
	{
		PluginSettings settings(MainGuid, Info.SettingsControl);
		bufSize = settings.Get(0, L"CompareBufferSize", 32768);

		if (bufSize < 32768)
			bufSize = 32768;
	}
	ABuf = (char*)malloc(bufSize);
	PBuf = (char*)malloc(bufSize);
	bBrokenByEsc = false;
	bStart = true;
	bOpenFail = false;
	bool bDifferenceNotFound = false;
	AFilter = INVALID_HANDLE_VALUE;
	PFilter = INVALID_HANDLE_VALUE;
	Info.FileFilterControl(PANEL_ACTIVE,  FFCTL_CREATEFILEFILTER, FFT_PANEL, &AFilter);
	Info.FileFilterControl(PANEL_PASSIVE, FFCTL_CREATEFILEFILTER, FFT_PANEL, &PFilter);
	Info.FileFilterControl(AFilter, FFCTL_STARTINGTOFILTER, 0, 0);
	Info.FileFilterControl(PFilter, FFCTL_STARTINGTOFILTER, 0, 0);

	// ������ ����� �������� ������� �� �������...
	if (ABuf && PBuf && AFilter != INVALID_HANDLE_VALUE && PFilter != INVALID_HANDLE_VALUE)
	{
		bDifferenceNotFound = CompareDirs(&AInfo, &PInfo, true, 0);
	}

	Info.FileFilterControl(AFilter, FFCTL_FREEFILEFILTER, 0, 0);
	Info.FileFilterControl(PFilter, FFCTL_FREEFILEFILTER, 0, 0);
	free(ABuf);
	free(PBuf);
	CloseHandle(hConInp);
	Info.RestoreScreen(hScreen);

	// �������� ����� � �������������� ������. ���� ����� ���������� ���������...
	if (!bBrokenByEsc)
	{
		Info.PanelControl(PANEL_ACTIVE,FCTL_BEGINSELECTION,0,0);

		for (size_t i=0; i<AInfo.ItemsNumber; i++)
		{
			Info.PanelControl(PANEL_ACTIVE, FCTL_SETSELECTION,i,(void *)(AInfo.PanelItems[i].Flags&PPIF_SELECTED));
		}

		Info.PanelControl(PANEL_ACTIVE,FCTL_ENDSELECTION,0,0);
		Info.PanelControl(PANEL_PASSIVE,FCTL_BEGINSELECTION,0,0);

		for (size_t i=0; i<PInfo.ItemsNumber; i++)
		{
			Info.PanelControl(PANEL_PASSIVE, FCTL_SETSELECTION,i,(void *)(PInfo.PanelItems[i].Flags&PPIF_SELECTED));
		}

		Info.PanelControl(PANEL_PASSIVE,FCTL_ENDSELECTION,0,0);
		Info.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL,0,0);
		Info.PanelControl(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,0);

		if (bOpenFail)
		{
			const wchar_t *MsgItems[] =
			{
				GetMsg(MOpenErrorTitle),
				GetMsg(MOpenErrorBody),
				GetMsg(MOK),
			};
			Info.Message(&MainGuid, nullptr, FMSG_WARNING, NULL, MsgItems, ARRAYSIZE(MsgItems), 1);
		}

		if (bDifferenceNotFound && Opt.MessageWhenNoDiff)
		{
			const wchar_t *MsgItems[] =
			{
				GetMsg(MNoDiffTitle),
				GetMsg(MNoDiffBody),
				GetMsg(MOK)
			};
			Info.Message(&MainGuid, nullptr, 0, NULL, MsgItems, ARRAYSIZE(MsgItems), 1);
		}
	}

	// ����������� ��������� ������� ����...
	if (dwTitleSaved)
		SetConsoleTitle(cConsoleTitle);

	FreePanelItems(AInfo,PInfo);
	return INVALID_HANDLE_VALUE;
}
