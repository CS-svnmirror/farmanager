/*
panelmix.cpp

Commonly used panel related functions
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

#include "panelmix.hpp"
#include "strmix.hpp"
#include "filepanels.hpp"
#include "config.hpp"
#include "panel.hpp"
#include "ctrlobj.hpp"
#include "keys.hpp"
#include "treelist.hpp"
#include "filelist.hpp"
#include "pathmix.hpp"
#include "panelctype.hpp"
#include "datetime.hpp"
#include "flink.hpp"

int ColumnTypeWidth[]={0, 6, 6, 8, 5, 14, 14, 14, 14, 6, 0, 0, 3, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const wchar_t *ColumnSymbol[]={L"N",L"S",L"P",L"D",L"T",L"DM",L"DC",L"DA",L"DE",L"A",L"Z",L"O",L"LN",L"F",L"G",L"X",L"C0",L"C1",L"C2",L"C3",L"C4",L"C5",L"C6",L"C7",L"C8",L"C9"};


void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir)
{
	if (!SrcPanel)
		SrcPanel=Global->CtrlObject->Cp()->ActivePanel;

	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel);

	switch (SrcPanel->GetType())
	{
		case QVIEW_PANEL:
		case INFO_PANEL:
			SrcPanel=Global->CtrlObject->Cp()->GetAnotherPanel(AnotherPanel=SrcPanel);
	}

	int AnotherType=AnotherPanel->GetType();

	if (AnotherType!=QVIEW_PANEL && AnotherType!=INFO_PANEL)
	{
		if (NeedSetUpADir)
		{
			AnotherPanel->SetCurDir(SrcPanel->GetCurDir(), true);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		}
		else
		{
			// TODO: ???
			//if(AnotherPanel->NeedUpdatePanel(SrcPanel))
			//  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			//else
			{
				// ������� ����� ���������� ������. ���� ��� ���� ����������� - ��������� ����.
				if (AnotherType==FILE_PANEL)
					((FileList *)AnotherPanel)->ResetLastUpdateTime();

				AnotherPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
			}
		}
	}

	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (AnotherType==QVIEW_PANEL)
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);

	Global->CtrlObject->Cp()->Redraw();
}

int CheckUpdateAnotherPanel(Panel *SrcPanel, const string& SelName)
{
	if (!SrcPanel)
		SrcPanel=Global->CtrlObject->Cp()->ActivePanel;

	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
	AnotherPanel->CloseFile();

	if (AnotherPanel->GetMode() == NORMAL_PANEL)
	{
		string strFullName;
		string strAnotherCurDir(AnotherPanel->GetCurDir());
		AddEndSlash(strAnotherCurDir);
		ConvertNameToFull(SelName, strFullName);
		AddEndSlash(strFullName);

		if (wcsstr(strAnotherCurDir.data(),strFullName.data()))
		{
			((FileList*)AnotherPanel)->StopFSWatcher();
			return TRUE;
		}
	}

	return FALSE;
}

int _MakePath1(DWORD Key, string &strPathName, const wchar_t *Param2,int ShortNameAsIs)
{
	int RetCode=FALSE;
	int NeedRealName=FALSE;
	strPathName.clear();

	switch (Key)
	{
		case KEY_CTRLALTBRACKET:       // �������� ������� (UNC) ���� �� ����� ������
		case KEY_RCTRLRALTBRACKET:
		case KEY_CTRLRALTBRACKET:
		case KEY_RCTRLALTBRACKET:
		case KEY_CTRLALTBACKBRACKET:   // �������� ������� (UNC) ���� �� ������ ������
		case KEY_RCTRLRALTBACKBRACKET:
		case KEY_CTRLRALTBACKBRACKET:
		case KEY_RCTRLALTBACKBRACKET:
		case KEY_ALTSHIFTBRACKET:      // �������� ������� (UNC) ���� �� �������� ������
		case KEY_RALTSHIFTBRACKET:
		case KEY_ALTSHIFTBACKBRACKET:  // �������� ������� (UNC) ���� �� ��������� ������
		case KEY_RALTSHIFTBACKBRACKET:
			NeedRealName=TRUE;
		case KEY_CTRLBRACKET:          // �������� ���� �� ����� ������
		case KEY_RCTRLBRACKET:
		case KEY_CTRLBACKBRACKET:      // �������� ���� �� ������ ������
		case KEY_RCTRLBACKBRACKET:
		case KEY_CTRLSHIFTBRACKET:     // �������� ���� �� �������� ������
		case KEY_RCTRLSHIFTBRACKET:
		case KEY_CTRLSHIFTBACKBRACKET: // �������� ���� �� ��������� ������
		case KEY_RCTRLSHIFTBACKBRACKET:
		case KEY_CTRLSHIFTNUMENTER:    // ������� ���� � ����.������
		case KEY_RCTRLSHIFTNUMENTER:
		case KEY_SHIFTNUMENTER:        // ������� ���� � �����.������
		case KEY_CTRLSHIFTENTER:       // ������� ���� � ����.������
		case KEY_RCTRLSHIFTENTER:
		case KEY_SHIFTENTER:           // ������� ���� � �����.������
		{
			Panel *SrcPanel=nullptr;
			FilePanels *Cp=Global->CtrlObject->Cp();

			switch (Key)
			{
				case KEY_CTRLALTBRACKET:
				case KEY_RCTRLRALTBRACKET:
				case KEY_CTRLRALTBRACKET:
				case KEY_RCTRLALTBRACKET:
				case KEY_CTRLBRACKET:
				case KEY_RCTRLBRACKET:
					SrcPanel=Cp->LeftPanel;
					break;
				case KEY_CTRLALTBACKBRACKET:
				case KEY_RCTRLRALTBACKBRACKET:
				case KEY_CTRLRALTBACKBRACKET:
				case KEY_RCTRLALTBACKBRACKET:
				case KEY_CTRLBACKBRACKET:
				case KEY_RCTRLBACKBRACKET:
					SrcPanel=Cp->RightPanel;
					break;
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:
				case KEY_ALTSHIFTBRACKET:
				case KEY_RALTSHIFTBRACKET:
				case KEY_CTRLSHIFTBRACKET:
				case KEY_RCTRLSHIFTBRACKET:
					SrcPanel=Cp->ActivePanel;
					break;
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_RCTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
				case KEY_RCTRLSHIFTENTER:
				case KEY_ALTSHIFTBACKBRACKET:
				case KEY_RALTSHIFTBACKBRACKET:
				case KEY_CTRLSHIFTBACKBRACKET:
				case KEY_RCTRLSHIFTBACKBRACKET:
					SrcPanel=Cp->GetAnotherPanel(Cp->ActivePanel);
					break;
			}

			if (SrcPanel)
			{
				if (Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER || Key == KEY_RCTRLSHIFTENTER || Key == KEY_SHIFTNUMENTER || Key == KEY_CTRLSHIFTNUMENTER || Key == KEY_RCTRLSHIFTNUMENTER)
				{
					string strShortFileName;
					SrcPanel->GetCurName(strPathName,strShortFileName);

					if (SrcPanel->GetShowShortNamesMode()) // ����� ���������� ���� :-)
						strPathName = strShortFileName;
				}
				else
				{
					/* TODO: ����� ����� ������, ��� � TreeList ���� ���� ���� :-) */
					if (!(SrcPanel->GetType()==FILE_PANEL || SrcPanel->GetType()==TREE_PANEL))
						return FALSE;

					strPathName = SrcPanel->GetCurDir();

					if (SrcPanel->GetMode()!=PLUGIN_PANEL)
					{
						FileList *SrcFilePanel=(FileList *)SrcPanel;
						strPathName = SrcFilePanel->GetCurDir();
						{
							if (NeedRealName)
								SrcFilePanel->CreateFullPathName(strPathName, strPathName,FILE_ATTRIBUTE_DIRECTORY, strPathName,TRUE,ShortNameAsIs);
						}

						if (SrcFilePanel->GetShowShortNamesMode() && ShortNameAsIs)
							ConvertNameToShort(strPathName,strPathName);
					}
					else
					{
						FileList *SrcFilePanel=(FileList *)SrcPanel;
						OpenPanelInfo Info;
						Global->CtrlObject->Plugins->GetOpenPanelInfo(SrcFilePanel->GetPluginHandle(),&Info);
						FileList::AddPluginPrefix(SrcFilePanel,strPathName);
						if (Info.HostFile && *Info.HostFile)
						{
							strPathName += Info.HostFile;
							strPathName += L"/";
						}
						strPathName += Info.CurDir;
					}

					AddEndSlash(strPathName);
				}

				if (Global->Opt->QuotedName&QUOTEDNAME_INSERT)
					QuoteSpace(strPathName);

				if (Param2)
					strPathName += Param2;

				RetCode=TRUE;
			}
		}
		break;
	}

	return RetCode;
}


void TextToViewSettings(const string& ColumnTitles,const string& ColumnWidths,
						unsigned __int64 *ViewColumnTypes,int *ViewColumnWidths,int *ViewColumnWidthsTypes,int &ColumnCount)
{
	// BUGBUG, add error checking

	const wchar_t *TextPtr=ColumnTitles.data();

	for (ColumnCount=0; ColumnCount < PANEL_COLUMNCOUNT; ColumnCount++)
	{
		string strArgName;

		if (!(TextPtr=GetCommaWord(TextPtr,strArgName)))
			break;

		Upper(strArgName);

		if (strArgName.front()==L'N')
		{
			unsigned __int64 &ColumnType=ViewColumnTypes[ColumnCount];
			ColumnType=NAME_COLUMN;
			const wchar_t *Ptr = strArgName.data()+1;

			while (*Ptr)
			{
				switch (*Ptr)
				{
					case L'M':
						ColumnType|=COLUMN_MARK;
						break;
					case L'O':
						ColumnType|=COLUMN_NAMEONLY;
						break;
					case L'R':
						ColumnType|=COLUMN_RIGHTALIGN;
						break;
					case L'F':
						ColumnType|=COLUMN_RIGHTALIGNFORCE;
						break;
					case L'N':
						ColumnType|=COLUMN_NOEXTENSION;
						break;
				}

				Ptr++;
			}
		}
		else
		{
			if (strArgName.front()==L'S' || strArgName.front()==L'P' || strArgName.front()==L'G')
			{
				unsigned __int64 &ColumnType=ViewColumnTypes[ColumnCount];
				ColumnType=(strArgName.front()==L'S') ? SIZE_COLUMN:(strArgName.front()==L'P')?PACKED_COLUMN:STREAMSSIZE_COLUMN;
				const wchar_t *Ptr = strArgName.data()+1;

				while (*Ptr)
				{
					switch (*Ptr)
					{
						case L'C':
							ColumnType|=COLUMN_COMMAS;
							break;
						case L'E':
							ColumnType|=COLUMN_ECONOMIC;
							break;
						case L'F':
							ColumnType|=COLUMN_FLOATSIZE;
							break;
						case L'T':
							ColumnType|=COLUMN_THOUSAND;
							break;
					}

					Ptr++;
				}
			}
			else
			{
				if (!StrCmpN(strArgName.data(),L"DM",2) || !StrCmpN(strArgName.data(),L"DC",2) || !StrCmpN(strArgName.data(),L"DA",2) || !StrCmpN(strArgName.data(),L"DE",2))
				{
					unsigned __int64 &ColumnType=ViewColumnTypes[ColumnCount];

					switch (strArgName.at(1))
					{
						case L'M':
							ColumnType=WDATE_COLUMN;
							break;
						case L'C':
							ColumnType=CDATE_COLUMN;
							break;
						case L'A':
							ColumnType=ADATE_COLUMN;
							break;
						case L'E':
							ColumnType=CHDATE_COLUMN;
							break;
					}

					const wchar_t *Ptr = strArgName.data()+2;

					while (*Ptr)
					{
						switch (*Ptr)
						{
							case L'B':
								ColumnType|=COLUMN_BRIEF;
								break;
							case L'M':
								ColumnType|=COLUMN_MONTH;
								break;
						}

						Ptr++;
					}
				}
				else
				{
					if (strArgName.front()==L'O')
					{
						unsigned __int64 &ColumnType=ViewColumnTypes[ColumnCount];
						ColumnType=OWNER_COLUMN;

						if (strArgName.at(1)==L'L')
							ColumnType|=COLUMN_FULLOWNER;
					}
					else if (strArgName.front()==L'X')
					{
						unsigned __int64 &ColumnType=ViewColumnTypes[ColumnCount];
						ColumnType=EXTENSION_COLUMN;

						if (strArgName.at(1)==L'R')
							ColumnType|=COLUMN_RIGHTALIGN;
					}
					else
					{
						for (unsigned I=0; I<ARRAYSIZE(ColumnSymbol); I++)
						{
							if (strArgName == ColumnSymbol[I])
							{
								ViewColumnTypes[ColumnCount]=I;
								break;
							}
						}
					}
				}
			}
		}
	}

	TextPtr=ColumnWidths.data();

	for (int I=0; I<ColumnCount; I++)
	{
		string strArgName;

		if (!(TextPtr=GetCommaWord(TextPtr,strArgName)))
			break;

		ViewColumnWidths[I]=_wtoi(strArgName.data());
		ViewColumnWidthsTypes[I]=COUNT_WIDTH;

		if (strArgName.size()>1)
		{
			switch (strArgName.back())
			{
				case L'%':
					ViewColumnWidthsTypes[I]=PERCENT_WIDTH;
					break;
			}
		}
	}

	if (!ColumnCount)
	{
		ColumnCount = 1;
		ViewColumnTypes[0] = NAME_COLUMN;
	}
}


void ViewSettingsToText(const unsigned __int64 *ViewColumnTypes, const int *ViewColumnWidths, const int *ViewColumnWidthsTypes, int ColumnCount,
						string &strColumnTitles, string &strColumnWidths)
{
	strColumnTitles.clear();
	strColumnWidths.clear();

	for (int I=0; I<ColumnCount; I++)
	{
		string strType;
		int ColumnType=static_cast<int>(ViewColumnTypes[I] & 0xff);
		strType = ColumnSymbol[ColumnType];

		if (ColumnType==NAME_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_MARK)
				strType += L"M";

			if (ViewColumnTypes[I] & COLUMN_NAMEONLY)
				strType += L"O";

			if (ViewColumnTypes[I] & COLUMN_RIGHTALIGN)
			{
				strType += L"R";
				if (ViewColumnTypes[I] & COLUMN_RIGHTALIGNFORCE)
					strType += L"F";
			}

			if (ViewColumnTypes[I] & COLUMN_NOEXTENSION)
				strType += L"N";
		}

		if (ColumnType==SIZE_COLUMN || ColumnType==PACKED_COLUMN || ColumnType==STREAMSSIZE_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_COMMAS)
				strType += L"C";

			if (ViewColumnTypes[I] & COLUMN_ECONOMIC)
				strType += L"E";

			if (ViewColumnTypes[I] & COLUMN_FLOATSIZE)
				strType += L"F";

			if (ViewColumnTypes[I] & COLUMN_THOUSAND)
				strType += L"T";
		}

		if (ColumnType==WDATE_COLUMN || ColumnType==ADATE_COLUMN || ColumnType==CDATE_COLUMN  || ColumnType==CHDATE_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_BRIEF)
				strType += L"B";

			if (ViewColumnTypes[I] & COLUMN_MONTH)
				strType += L"M";
		}

		if (ColumnType==OWNER_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_FULLOWNER)
				strType += L"L";
		}

		if (ColumnType==EXTENSION_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_RIGHTALIGN)
				strType += L"R";
		}

		strColumnTitles += strType;
		wchar_t *lpwszWidth = strType.GetBuffer(20);
		_itow(ViewColumnWidths[I],lpwszWidth,10);
		strType.ReleaseBuffer();
		strColumnWidths += strType;

		switch (ViewColumnWidthsTypes[I])
		{
			case PERCENT_WIDTH:
				strColumnWidths += L"%";
				break;
		}

		if (I<ColumnCount-1)
		{
			strColumnTitles += L",";
			strColumnWidths += L",";
		}
	}
}

const string FormatStr_Attribute(DWORD FileAttributes,int Width)
{
	FormatString strResult;

	wchar_t OutStr[]=
	{
		FileAttributes&FILE_ATTRIBUTE_READONLY?L'R':L' ',
		FileAttributes&FILE_ATTRIBUTE_SYSTEM?L'S':L' ',
		FileAttributes&FILE_ATTRIBUTE_HIDDEN?L'H':L' ',
		FileAttributes&FILE_ATTRIBUTE_ARCHIVE?L'A':L' ',
		FileAttributes&FILE_ATTRIBUTE_SPARSE_FILE?L'P':L' ',
		FileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?L'L':L' ',
		FileAttributes&FILE_ATTRIBUTE_COMPRESSED?L'C':FileAttributes&FILE_ATTRIBUTE_ENCRYPTED?L'E':L' ',
		FileAttributes&FILE_ATTRIBUTE_TEMPORARY?L'T':L' ',
		FileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?L'I':L' ',
		FileAttributes&FILE_ATTRIBUTE_OFFLINE?L'O':L' ',
		FileAttributes&FILE_ATTRIBUTE_VIRTUAL?L'V':L' ',
		0
	};

	if (Width > 0)
	{
		strResult<<fmt::ExactWidth(Width);

		int n = static_cast<int>(ARRAYSIZE(OutStr)) - 1;
		while (n > Width && OutStr[n-1] == L' ')
			OutStr[--n] = L'\0';

		wchar_t *ps;
		while (n > Width && nullptr != (ps = wcsrchr(OutStr, L' ')))
		{
			wcscpy(ps, ps+1);
			--n;
		}
	}
	strResult<<OutStr;
	return strResult;
}

const string FormatStr_DateTime(const FILETIME *FileTime,int ColumnType,unsigned __int64 Flags,int Width)
{
	FormatString strResult;

	if (Width < 0)
	{
		if (ColumnType == DATE_COLUMN)
			Width=0;
		else
			return strResult;
	}

	int ColumnWidth=Width;
	bool Brief = (Flags & COLUMN_BRIEF) != 0;
	bool TextMonth = (Flags & COLUMN_MONTH) != 0;
	bool FullYear = false;

	switch(ColumnType)
	{
		case DATE_COLUMN:
		case TIME_COLUMN:
		{
			Brief = false;
			TextMonth = false;
			if (ColumnType == DATE_COLUMN)
				FullYear=ColumnWidth>9;
			break;
		}
		case WDATE_COLUMN:
		case CDATE_COLUMN:
		case ADATE_COLUMN:
		case CHDATE_COLUMN:
		{
			if (!Brief)
			{
				int CmpWidth=ColumnWidth-(TextMonth? 1: 0);

				if (CmpWidth==15 || CmpWidth==16 || CmpWidth==18 || CmpWidth==19 || CmpWidth>21)
					FullYear = true;
			}
			ColumnWidth-=9;
			break;
		}
	}

	string strDateStr,strTimeStr;

	ConvertDate(*FileTime,strDateStr,strTimeStr,ColumnWidth,Brief,TextMonth,FullYear);

	string strOutStr;
	switch(ColumnType)
	{
		case DATE_COLUMN:
			strOutStr=strDateStr;
			break;
		case TIME_COLUMN:
			strOutStr=strTimeStr;
			break;
		default:
			strOutStr=strDateStr+L" "+strTimeStr;
			break;
	}

	strResult<<fmt::ExactWidth(Width)<<strOutStr;

	return strResult;
}

const string FormatStr_Size(__int64 FileSize, __int64 AllocationSize, __int64 StreamsSize, const string& strName,
							DWORD FileAttributes,DWORD ShowFolderSize,DWORD ReparseTag,int ColumnType,
							unsigned __int64 Flags,int Width,const wchar_t *CurDir)
{
	FormatString strResult;

	bool Packed=(ColumnType==PACKED_COLUMN);
	bool Streams=(ColumnType==STREAMSSIZE_COLUMN);

	if (ShowFolderSize==2)
	{
		Width--;
		strResult<<L"~";
	}

	bool dir = (0 != (FileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	bool rpt = (0 != (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT));
	bool spf = (0 != (FileAttributes &  FILE_ATTRIBUTE_SPARSE_FILE));
	bool dir_link = dir || (rpt && !spf);

	if (!Streams && !Packed && dir_link && !ShowFolderSize)
	{
		string strMsg;
		const wchar_t *PtrName=MSG(MListFolder);

		if (TestParentFolderName(strName))
		{
			PtrName=MSG(MListUp);
		}
		else
		{
			if (FileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
			{
				switch(ReparseTag)
				{
				// 0xA0000003L = Directory Junction or Volume Mount Point
				case IO_REPARSE_TAG_MOUNT_POINT:
					{
						LNGID ID_Msg = MListJunction;
						if (Global->Opt->PanelDetailedJunction)
						{
							string strLinkName=CurDir?CurDir:L"";
							AddEndSlash(strLinkName);
							strLinkName+=PointToName(strName);

							if (GetReparsePointInfo(strLinkName, strLinkName))
							{
								NormalizeSymlinkName(strLinkName);
								bool Root;
								if(ParsePath(strLinkName, nullptr, &Root) == PATH_VOLUMEGUID && Root)
								{
									ID_Msg=MListVolMount;
								}
							}
						}
						PtrName=MSG(ID_Msg);
					}
					break;
				// 0xA000000CL = Directory or File Symbolic Link
				case IO_REPARSE_TAG_SYMLINK:
					PtrName = MSG(MListSymlink);
					break;
				// 0x8000000AL = Distributed File System
				case IO_REPARSE_TAG_DFS:
					PtrName = MSG(MListDFS);
					break;
				// 0x80000012L = Distributed File System Replication
				case IO_REPARSE_TAG_DFSR:
					PtrName = MSG(MListDFSR);
					break;
				// 0xC0000004L = Hierarchical Storage Management
				case IO_REPARSE_TAG_HSM:
					PtrName = MSG(MListHSM);
					break;
				// 0x80000006L = Hierarchical Storage Management2
				case IO_REPARSE_TAG_HSM2:
					PtrName = MSG(MListHSM2);
					break;
				// 0x80000007L = Single Instance Storage
				case IO_REPARSE_TAG_SIS:
					PtrName = MSG(MListSIS);
					break;
				// 0x80000008L = Windows Imaging Format
				case IO_REPARSE_TAG_WIM:
					PtrName = MSG(MListWIM);
					break;
				// 0x80000009L = Cluster Shared Volumes
				case IO_REPARSE_TAG_CSV:
					PtrName = MSG(MListCSV);
					break;
				case IO_REPARSE_TAG_DEDUP:
					PtrName = MSG(MListDEDUP);
					break;
				case IO_REPARSE_TAG_NFS:
					PtrName = MSG(MListNFS);
					break;
				// 0x????????L = anything else
				default:
					if (Global->Opt->ShowUnknownReparsePoint)
					{
						strMsg = FormatString() << L":" << fmt::Radix(16) << fmt::ExactWidth(8) << fmt::FillChar(L'0') << ReparseTag;
						PtrName = strMsg.data();
					}
					else
					{
						PtrName=MSG(MListUnknownReparsePoint);
					}
				}
			}
		}

		FormatString strStr;
		if(*PtrName)
		{
			if (StrLength(PtrName) <= Width-2 && MSG(MListBrackets)[0] && MSG(MListBrackets)[1])
			{
				strStr << MSG(MListBrackets)[0] << PtrName << MSG(MListBrackets)[1];
			}
			else
			{
				strStr << PtrName;
			}
		}
		strResult<<fmt::ExactWidth(Width)<<strStr;
	}
	else
	{
		string strOutStr;
		strResult<<FileSizeToStr(strOutStr,Packed?AllocationSize:Streams?StreamsSize:FileSize,Width,Flags);
	}

	return strResult;
}
