/*
mix.cpp

���� ������ ��������������� �������
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

#include "mix.hpp"
#include "scantree.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "frame.hpp"

#include "cmdline.hpp"
#include "dlgedit.hpp"

int ToPercent(unsigned long N1,unsigned long N2)
{
	if (N1 > 10000)
	{
		N1/=100;
		N2/=100;
	}

	if (!N2)
		return 0;

	return((int)(N1*100/N2));
}

int ToPercent64(unsigned __int64 N1, unsigned __int64 N2)
{
	if (N1 > 10000)
	{
		N1/=100;
		N2/=100;
	}

	if (!N2)
		return 0;

	return static_cast<int>(N1*100/N2);
}


/*
             v - �����
   prefXXX X X XXX
       \ / ^   ^^^\ PID + TID
        |  \------/
        |
        +---------- [0A-Z]
*/
bool FarMkTempEx(string &strDest, const wchar_t *Prefix, BOOL WithTempPath, const wchar_t *UserTempPath)
{
	static UINT s_shift = 0;
	if (!(Prefix && *Prefix))
		Prefix=L"F3T";

	string strPath = L".";

	if (WithTempPath)
	{
		apiGetTempPath(strPath);
	}
	else if(UserTempPath)
	{
		strPath=UserTempPath;
	}

	AddEndSlash(strPath);

	wchar_t_ptr Buffer(StrLength(Prefix) + strPath.size() + 13);

	UINT uniq = 23*GetCurrentProcessId() + s_shift, uniq0 = uniq ? uniq : 1;
	s_shift = (s_shift + 1) % 23;

	for (;;)
	{
		if (!uniq) ++uniq;

		if (GetTempFileName(strPath.data(), Prefix, uniq, Buffer.get()))
		{
			FindFile f(Buffer.get(), false);
			FAR_FIND_DATA ffdata;
			if (!f.Get(ffdata))
				break;
		}

		if ((++uniq & 0xffff) == (uniq0 & 0xffff))
		{
			*Buffer = L'\0';
			break;
		}
	}

	strDest.assign(Buffer.get());
	return !strDest.empty();
}

void PluginPanelItemToFindDataEx(const PluginPanelItem *pSrc, FAR_FIND_DATA *pDest)
{
	pDest->dwFileAttributes = pSrc->FileAttributes;
	pDest->ftCreationTime = pSrc->CreationTime;
	pDest->ftLastAccessTime = pSrc->LastAccessTime;
	pDest->ftLastWriteTime = pSrc->LastWriteTime;
	pDest->ftChangeTime = pSrc->ChangeTime;
	pDest->nFileSize = pSrc->FileSize;
	pDest->nAllocationSize = pSrc->AllocationSize;
	pDest->strFileName = NullToEmpty(pSrc->FileName);
	pDest->strAlternateFileName = NullToEmpty(pSrc->AlternateFileName);
}

void FindDataExToPluginPanelItem(const FAR_FIND_DATA *pSrc, PluginPanelItem *pDest)
{
	pDest->FileAttributes = pSrc->dwFileAttributes;
	pDest->CreationTime = pSrc->ftCreationTime;
	pDest->LastAccessTime = pSrc->ftLastAccessTime;
	pDest->LastWriteTime = pSrc->ftLastWriteTime;
	pDest->ChangeTime = pSrc->ftChangeTime;
	pDest->FileSize = pSrc->nFileSize;
	pDest->AllocationSize = pSrc->nAllocationSize;
	pDest->FileName = DuplicateString(pSrc->strFileName.data());
	pDest->AlternateFileName = DuplicateString(pSrc->strAlternateFileName.data());
}

void FreePluginPanelItem(PluginPanelItem& Data)
{
	delete[] Data.FileName;
	delete[] Data.AlternateFileName;
}

void FreePluginPanelItemsUserData(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber)
{
	for(size_t ii=0;ii<ItemsNumber;++ii)
	{
		if (PanelItem[ii].UserData.FreeData)
		{
			FarPanelItemFreeInfo info={sizeof(FarPanelItemFreeInfo),hPlugin};
			PanelItem[ii].UserData.FreeData(PanelItem[ii].UserData.Data,&info);
		}
	}
}

WINDOWINFO_TYPE ModalType2WType(const int fType)
{
	struct map_item
	{
		MODALFRAME_TYPE FrameType;
		WINDOWINFO_TYPE WindowType;
	};
	static map_item TypesMap[] =
	{
		{MODALTYPE_PANELS,     WTYPE_PANELS},
		{MODALTYPE_VIEWER,     WTYPE_VIEWER},
		{MODALTYPE_EDITOR,     WTYPE_EDITOR},
		{MODALTYPE_DIALOG,     WTYPE_DIALOG},
		{MODALTYPE_VMENU,      WTYPE_VMENU},
		{MODALTYPE_HELP,       WTYPE_HELP},
		{MODALTYPE_COMBOBOX,   WTYPE_COMBOBOX},
		{MODALTYPE_FINDFOLDER, WTYPE_FINDFOLDER},
		{MODALTYPE_USER,       WTYPE_USER},
		{MODALTYPE_GRABBER,    WTYPE_GRABBER},
		{MODALTYPE_HMENU,      WTYPE_HMENU},
	};

	auto ItemIterator = std::find_if(CONST_RANGE(TypesMap, i)
	{
		return i.FrameType == fType;
	});
	return ItemIterator == std::cend(TypesMap)? static_cast<WINDOWINFO_TYPE>(-1) : ItemIterator->WindowType;
}

SetAutocomplete::SetAutocomplete(EditControl* edit, bool NewState):
	edit(edit),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::SetAutocomplete(DlgEdit* dedit, bool NewState):
	edit(dedit->lineEdit),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::SetAutocomplete(CommandLine* cedit, bool NewState):
	edit(&cedit->CmdStr),
	OldState(edit->GetAutocomplete())
{
	edit->SetAutocomplete(NewState);
}

SetAutocomplete::~SetAutocomplete()
{
	edit->SetAutocomplete(OldState);
};


unsigned long CRC32(unsigned long crc, const void* buffer, size_t len)
{
	const unsigned char* buf = reinterpret_cast<const unsigned char*>(buffer);
	static unsigned long crc_table[256];

	if (!crc_table[1])
	{
		int n, k;

		for (n = 0; n < 256; n++)
		{
			unsigned long c = (unsigned long)n;

			for (k = 0; k < 8; k++) c = (c >> 1) ^(c & 1 ? 0xedb88320L : 0);

			crc_table[n] = c;
		}
	}

	crc = crc ^ 0xffffffffL;

	while (len--)
	{
		crc = crc_table[(crc ^ (*buf++)) & 0xff] ^(crc >> 8);
	}

	return crc ^ 0xffffffffL;
}
