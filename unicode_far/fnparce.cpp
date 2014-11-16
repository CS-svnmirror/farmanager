/*
fnparce.cpp

������ �������� ����������
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

#include "fnparce.hpp"
#include "panel.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "flink.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "dialog.hpp"
#include "message.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "mix.hpp"
#include "language.hpp"

struct TSubstData
{
	// ��������� ������� SubstFileName
	const wchar_t *Name;           // ������� ���
	const wchar_t *ShortName;      // �������� ���

	string *pListName;
	string *pAnotherListName;

	string *pShortListName;
	string *pAnotherShortListName;

	// ��������� ����������
	string strAnotherName;
	string strAnotherShortName;
	string strNameOnly;
	string strShortNameOnly;
	string strAnotherNameOnly;
	string strAnotherShortNameOnly;
	string strCmdDir;
	int  PreserveLFN;
	int  PassivePanel;

	Panel *AnotherPanel;
	Panel *ActivePanel;
};


static int IsReplaceVariable(const wchar_t *str,int *scr = nullptr,
                             int *end = nullptr,
                             int *beg_scr_break = nullptr,
                             int *end_scr_break = nullptr,
                             int *beg_txt_break = nullptr,
                             int *end_txt_break = nullptr);


static int ReplaceVariables(const wchar_t *DlgTitle,string &strStr,TSubstData *PSubstData);

// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!

static const wchar_t *_SubstFileName(const wchar_t *CurStr,TSubstData *PSubstData,string &strOut)
{
	// ���������� ������������� ����������/����������� ������.
	if (!StrCmpN(CurStr,L"!#",2))
	{
		CurStr+=2;
		PSubstData->PassivePanel=TRUE;
		return CurStr;
	}

	if (!StrCmpN(CurStr,L"!^",2))
	{
		CurStr+=2;
		PSubstData->PassivePanel=FALSE;
		return CurStr;
	}

	// !! ������ '!'
	if (!StrCmpN(CurStr,L"!!",2) && CurStr[2] != L'?')
	{
		strOut += L"!";
		CurStr+=2;
		return CurStr;
	}

	// !.!      ������� ��� ����� � �����������
	if (!StrCmpN(CurStr,L"!.!",3) && CurStr[3] != L'?')
	{
		if (PSubstData->PassivePanel)
			strOut += PSubstData->strAnotherName;
		else
			strOut += PSubstData->Name;

		CurStr+=3;
		return CurStr;
	}

	// !~       �������� ��� ����� ��� ����������
	if (!StrCmpN(CurStr,L"!~",2))
	{
		strOut += PSubstData->PassivePanel ? PSubstData->strAnotherShortNameOnly : PSubstData->strShortNameOnly;
		CurStr+=2;
		return CurStr;
	}

	// !`  ������� ���������� ����� ��� �����
	if (!StrCmpN(CurStr,L"!`",2))
	{
		const wchar_t *Ext;

		if (CurStr[2] == L'~')
		{
			Ext=wcsrchr((PSubstData->PassivePanel ? PSubstData->strAnotherShortName.data():PSubstData->ShortName),L'.');
			CurStr+=3;
		}
		else
		{
			Ext=wcsrchr((PSubstData->PassivePanel ? PSubstData->strAnotherName.data():PSubstData->Name),L'.');
			CurStr+=2;
		}

		if (Ext && *Ext)
			strOut += ++Ext;

		return CurStr;
	}

	// !& !&~  ������ ������ ����������� ��������.
	if ((!StrCmpN(CurStr,L"!&~",3) && CurStr[3] != L'?') ||
	        (!StrCmpN(CurStr,L"!&",2) && CurStr[2] != L'?'))
	{
		string strFileNameL, strShortNameL;
		Panel *WPanel=PSubstData->PassivePanel?PSubstData->AnotherPanel:PSubstData->ActivePanel;
		DWORD FileAttrL;
		int ShortN0=FALSE;
		int CntSkip=2;

		if (CurStr[2] == L'~')
		{
			ShortN0=TRUE;
			CntSkip++;
		}

		WPanel->GetSelName(nullptr,FileAttrL);
		int First = TRUE;

		while (WPanel->GetSelName(&strFileNameL,FileAttrL,&strShortNameL))
		{
			if (ShortN0)
				strFileNameL = strShortNameL;
			else // � ������ ��� �� ������ ������� ��� � ��������.
				QuoteSpaceOnly(strFileNameL);

// ��� ����� ��� ��� ����� - �����/�������...
//   ���� ����� ����� - ��������������� :-)
//          if(FileAttrL & FILE_ATTRIBUTE_DIRECTORY)
//            AddEndSlash(FileNameL);
			// � ����� �� ��� ������ � ����� ������?
			if (First)
				First = FALSE;
			else
				strOut += L" ";

			strOut += strFileNameL;
		}

		CurStr+=CntSkip;
		return CurStr;
	}

	// !@  ��� �����, ����������� ����� ���������� ������
	// !$!      ��� �����, ����������� �������� ����� ���������� ������
	// ���� ���� ���������� ���� ��� ������� ��� !@! ��� � !$!
	//������-�� (�� ������������ �������������� ��� ��) - � !$! ����� ����������� ������������ Q � A
	// �� ����� ����:)
	if (!StrCmpN(CurStr,L"!@",2) || !StrCmpN(CurStr,L"!$",2))
	{
		string *pListName;
		string *pAnotherListName;
		bool ShortN0 = false;

		if (CurStr[1] == L'$')
			ShortN0 = true;

		if (ShortN0)
		{
			pListName = PSubstData->pShortListName;
			pAnotherListName = PSubstData->pAnotherShortListName;
		}
		else
		{
			pListName = PSubstData->pListName;
			pAnotherListName = PSubstData->pAnotherListName;
		}

		const wchar_t *Ptr;

		if ((Ptr=wcschr(CurStr+2,L'!')) )
		{
			if (Ptr[1] != L'?')
			{
				const string Modifers(CurStr+2, static_cast<size_t>(Ptr-(CurStr+2)));

				if (pListName)
				{
					if (PSubstData->PassivePanel && (!pAnotherListName->empty() || PSubstData->AnotherPanel->MakeListFile(*pAnotherListName, ShortN0, Modifers)))
					{
						if (ShortN0)
							ConvertNameToShort(*pAnotherListName, *pAnotherListName);

						strOut += *pAnotherListName;
					}

					if (!PSubstData->PassivePanel && (!pListName->empty() || PSubstData->ActivePanel->MakeListFile(*pListName, ShortN0, Modifers)))
					{
						if (ShortN0)
							ConvertNameToShort(*pListName,*pListName);

						strOut += *pListName;
					}
				}
				else
				{
					strOut += CurStr;
					strOut += Modifers;
					strOut += L"!";
				}

				CurStr+=Ptr-CurStr+1;
				return CurStr;
			}
		}
	}

	// !-!      �������� ��� ����� � �����������
	if (!StrCmpN(CurStr,L"!-!",3) && CurStr[3] != L'?')
	{
		if (PSubstData->PassivePanel)
			strOut += PSubstData->strAnotherShortName;
		else
			strOut += PSubstData->ShortName;

		CurStr+=3;
		return CurStr;
	}

	// !+!      ���������� !-!, �� ���� ������� ��� ����� �������
	//          ����� ���������� �������, FAR ����������� ���
	if (!StrCmpN(CurStr,L"!+!",3) && CurStr[3] != L'?')
	{
		if (PSubstData->PassivePanel)
			strOut += PSubstData->strAnotherShortName;
		else
			strOut += PSubstData->ShortName;

		CurStr+=3;
		PSubstData->PreserveLFN=TRUE;
		return CurStr;
	}

	// !:       ������� ����
	if (!StrCmpN(CurStr,L"!:",2))
	{
		string strCurDir;
		string strRootDir;

		if (*PSubstData->Name && PSubstData->Name[1]==L':')
			strCurDir = PSubstData->Name;
		else if (PSubstData->PassivePanel)
			strCurDir = PSubstData->AnotherPanel->GetCurDir();
		else
			strCurDir = PSubstData->strCmdDir;

		GetPathRoot(strCurDir,strRootDir);
		DeleteEndSlash(strRootDir);
		strOut += strRootDir;
		CurStr+=2;
		return CurStr;
	}

	// !\       ������� ����
	// !/       �������� ��� �������� ����
	// ���� ���� ���������� ���� ��� ������� ��� !\ ��� � !/
	if (!StrCmpN(CurStr,L"!\\",2) || !StrCmpN(CurStr,L"!=\\",3) || !StrCmpN(CurStr,L"!/",2) || !StrCmpN(CurStr,L"!=/",3))
	{
		string strCurDir;
		bool ShortN0 = false;
		int RealPath= CurStr[1]==L'='?1:0;

		if (CurStr[1] == L'/' || (RealPath && CurStr[2] == L'/'))
		{
			ShortN0 = true;
		}

		if (PSubstData->PassivePanel)
			strCurDir = PSubstData->AnotherPanel->GetCurDir();
		else
			strCurDir = PSubstData->strCmdDir;

		if (RealPath)
		{
			_MakePath1(PSubstData->PassivePanel?KEY_ALTSHIFTBACKBRACKET:KEY_ALTSHIFTBRACKET,strCurDir,L"",ShortN0);
			Unquote(strCurDir);
		}

		if (ShortN0)
			ConvertNameToShort(strCurDir,strCurDir);

		AddEndSlash(strCurDir);

		CurStr+=2+RealPath;

		if (*CurStr==L'!')
		{
			if (wcspbrk(PSubstData->PassivePanel?PSubstData->strAnotherName.data():PSubstData->Name,L"\\:"))
				strCurDir.clear();
		}

		strOut +=  strCurDir;
		return CurStr;
	}

	// !?<title>?<init>!
	if (!StrCmpN(CurStr,L"!?",2) && wcschr(CurStr+2,L'!'))
	{
		int j;
		int i = IsReplaceVariable(CurStr);

		if (i == -1)  // if bad format string
		{             // skip 1 char
			j = 1;
		}
		else
		{
			j = i + 1;
		}

		strOut.append(CurStr, j);
		CurStr += j;
		return CurStr;
	}

	// !        ������� ��� ����� ��� ����������
	if (*CurStr==L'!')
	{
		strOut += PointToName(PSubstData->PassivePanel ? PSubstData->strAnotherNameOnly : PSubstData->strNameOnly);
		CurStr++;
	}

	return CurStr;
}


/*
  SubstFileName()
  �������������� ������������ ���������� ������ � �������� ��������

*/
int SubstFileName(const wchar_t *DlgTitle,
                  string &strStr,            // �������������� ������
                  const string& Name,           // ������� ���
                  const string& ShortName,      // �������� ���

                  string *pListName,
                  string *pAnotherListName,
                  string *pShortListName,
                  string *pAnotherShortListName,
                  int   IgnoreInput,    // TRUE - �� ��������� "!?<title>?<init>!"
                  const wchar_t *CmdLineDir)     // ������� ����������
{
	if (pListName)
		pListName->clear();

	if (pAnotherListName)
		pAnotherListName->clear();

	if (pShortListName)
		pShortListName->clear();

	if (pAnotherShortListName)
		pAnotherShortListName->clear();

	/* $ 19.06.2001 SVS
	  ��������! ��� �������������� ������������, �� ���������� �� "!",
	  ����� ����� ���� ������ ��� �������� ���� �������� ������� (���������
	  ����������������!)
	*/
	if (strStr.find(L'!') == string::npos)
		return FALSE;

	TSubstData SubstData, *PSubstData=&SubstData;
	PSubstData->Name=Name.data();                    // ������� ���
	PSubstData->ShortName=ShortName.data();          // �������� ���
	PSubstData->pListName=pListName;            // ������� ��� �����-������
	PSubstData->pAnotherListName=pAnotherListName;            // ������� ��� �����-������
	PSubstData->pShortListName=pShortListName;  // �������� ��� �����-������
	PSubstData->pAnotherShortListName=pAnotherShortListName;  // �������� ��� �����-������
	// ���� ��� �������� �������� �� ������...
	if (CmdLineDir)
		PSubstData->strCmdDir = CmdLineDir;
	else // ...������� � ���.������
		Global->CtrlObject->CmdLine()->GetCurDir(PSubstData->strCmdDir);

	// �������������� ������� ��������� "���������" :-)
	PSubstData->strNameOnly = Name;

	size_t pos = PSubstData->strNameOnly.rfind(L'.');
	if (pos != string::npos)
		PSubstData->strNameOnly.resize(pos);

	PSubstData->strShortNameOnly = ShortName;

	if ((pos = PSubstData->strShortNameOnly.rfind(L'.')) != string::npos)
		PSubstData->strShortNameOnly.resize(pos);

	PSubstData->ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
	PSubstData->AnotherPanel=Global->CtrlObject->Cp()->PassivePanel();
	PSubstData->AnotherPanel->GetCurName(PSubstData->strAnotherName,PSubstData->strAnotherShortName);
	PSubstData->strAnotherNameOnly = PSubstData->strAnotherName;

	if ((pos = PSubstData->strAnotherNameOnly.rfind(L'.')) != string::npos)
		PSubstData->strAnotherNameOnly.resize(pos);

	PSubstData->strAnotherShortNameOnly = PSubstData->strAnotherShortName;

	if ((pos = PSubstData->strAnotherShortNameOnly.rfind(L'.')) != string::npos)
		PSubstData->strAnotherShortNameOnly.resize(pos);

	PSubstData->PreserveLFN=FALSE;
	PSubstData->PassivePanel=FALSE; // ������������� ���� ���� ��� �������� ������!
	string strTmp = strStr;

	if (!IgnoreInput)
	{
		string title = NullToEmpty(DlgTitle);
		SubstFileName(nullptr,title,Name,ShortName,nullptr,nullptr,nullptr,nullptr,TRUE);
		ReplaceVariables(api::env::expand_strings(title).data(), strTmp, PSubstData);
	}

	const wchar_t *CurStr = strTmp.data();
	string strOut;

	while (*CurStr)
	{
		if (*CurStr == L'!')
		{
			CurStr=_SubstFileName(CurStr,PSubstData,strOut);
		}
		else
		{
			strOut.append(CurStr,1);
			CurStr++;
		}
	}

	strStr = strOut;
	return PSubstData->PreserveLFN;
}

int ReplaceVariables(const wchar_t *DlgTitle,string &strStr,TSubstData *PSubstData)
{
	const wchar_t *Str=strStr.data();
	const wchar_t * const StartStr=Str;

	// TODO: use DialogBuilder

	std::vector<DialogItemEx> DlgData;
	DlgData.reserve(30);

	struct pos_item
	{
		ptrdiff_t Pos;
		ptrdiff_t EndPos;
	};
	std::vector<pos_item> Positions;
	Positions.reserve(128);

	{
		DialogItemEx Item;
		Item.Type = DI_DOUBLEBOX;
		Item.X1 = 3;
		Item.Y1 = 1;
		Item.X2 = 72;
		Item.strData = NullToEmpty(DlgTitle);
		DlgData.emplace_back(Item);
	}

	while (*Str)
	{
		if (*(Str++)!=L'!')
			continue;

		if (!*Str)
			break;

		if (*(Str++)!=L'?')
			continue;

		if (!*Str)
			break;

		// �������� ��� �� ������
		// �������� ����� ���������� ������� ����������� ������
		// ��������� �� �������
		int scr,end, beg_t,end_t,beg_s,end_s;
		scr = end = beg_t = end_t = beg_s = end_s = 0;
		int ii = IsReplaceVariable(Str-2,&scr,&end,&beg_t,&end_t,&beg_s,&end_s);

		if (ii == -1)
		{
			strStr.clear();
			return 0;
		}

		{
			pos_item Item = {Str - StartStr - 2, Str - StartStr - 2 + ii};
			Positions.emplace_back(Item);
		}

		{
			DialogItemEx Item;
			Item.Type = DI_TEXT;
			Item.X1 = 5;
			Item.Y1 = Item.Y2 = DlgData.size() + 1;
			DlgData.emplace_back(Item);
		}

		{
			DialogItemEx Item;
			Item.Type = DI_EDIT;
			Item.X1 = 5;
			Item.X2 = 70;
			Item.Y1 = Item.Y2 = DlgData.size() + 1;
			Item.Flags = DIF_HISTORY | DIF_USELASTHISTORY;
			Item.strHistory = L"UserVar" + std::to_wstring((DlgData.size() - 1) / 2);
			DlgData.emplace_back(Item);
		}

		string strTitle;

		if (scr > 2)          // if between !? and ? exist some
			strTitle.assign(Str,scr-2);

		size_t hist_correct = 0;

		if (!strTitle.empty())
		{
			if (strTitle[0] == L'$')        // begin of history name
			{
				const wchar_t *p = strTitle.data() + 1;
				const wchar_t *p1 = wcschr(p,L'$');

				if (p1)
				{
					DlgData.back().strHistory.assign(p,p1-p);
					strTitle = ++p1;
					hist_correct = p1 - p + 1;
				}
			}
		}

		if ((beg_t == -1) || (end_t == -1))
		{
			//
		}
		else if ((end_t - beg_t) > 1) //if between ( and ) exist some
		{
			// !?$zz$xxxx(fffff)ddddd
			//                  ^   ^
			string strTitle2 = strTitle.substr(end_t - 2 + 1 - hist_correct, scr - end_t - 1);

			// !?$zz$xxxx(ffffff)ddddd
			//            ^    ^
			string strTitle3 = strTitle.substr(beg_t - 2 + 1 - hist_correct, end_t - beg_t - 1);

			// !?$zz$xxxx(fffff)ddddd
			//       ^  ^
			strTitle.resize(beg_t - 2 - hist_correct);

			string strTmp;
			const wchar_t *CurStr = strTitle3.data();

			while (*CurStr)
			{
				if (*CurStr == L'!')
				{
					CurStr=_SubstFileName(CurStr,PSubstData,strTmp);
				}
				else
				{
					strTmp.append(CurStr,1);
					CurStr++;
				}
			}

			strTitle += strTmp;
			strTitle += strTitle2;
		}

		//do it - ���� ����� ��� ��� �������� � �������������
		DlgData[DlgData.size() - 2].strData = api::env::expand_strings(strTitle);

		// ��������� ���� ����� �������� �������� - ���� ����
		string strTxt;

		if ((end-scr) > 1)  //if between ? and ! exist some
			strTxt.append((Str-2)+scr+1,(end-scr)-1);

		if ((beg_s == -1) || (end_s == -1))
		{
			//
		}
		else if ((end_s - beg_s) > 1) //if between ( and ) exist some
		{
			// !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
			//                                  ^   ^
			string strTxt2 = strTxt.substr(end_s - scr, end - end_s - 1);

			// !?$zz$xxxx(ffffff)ddddd?rrrr(pppp)qqqqq!
			//                              ^  ^
			string strTxt3 = strTxt.substr(beg_s - scr, end_s - beg_s - 1);

			// !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
			//                        ^  ^
			strTxt.resize(beg_s - scr - 1);

			string strTmp;
			const wchar_t *CurStr = strTxt3.data();

			while (*CurStr)
			{
				if (*CurStr == L'!')
				{
					CurStr=_SubstFileName(CurStr,PSubstData,strTmp);
				}
				else
				{
					strTmp.push_back(*CurStr);
					CurStr++;
				}
			}

			strTxt += strTmp;
			strTxt += strTxt2;
		}

		DlgData.back().strData = strTxt;
	}

	if (DlgData.size() <= 1)
	{
		return 0;
	}

	{
		DialogItemEx Item;
		Item.Type = DI_TEXT;
		Item.Flags = DIF_SEPARATOR;
		Item.Y1 = Item.Y2 = DlgData.size() + 1;
		DlgData.emplace_back(Item);
	}

	{
		DialogItemEx Item;
		Item.Type = DI_BUTTON;
		Item.Flags = DIF_DEFAULTBUTTON|DIF_CENTERGROUP;
		Item.Y1 = Item.Y2 = DlgData.size() + 1;
		Item.strData = MSG(MOk);
		DlgData.emplace_back(Item);

		Item.strData = MSG(MCancel);
		Item.Flags &= ~DIF_DEFAULTBUTTON;
		DlgData.emplace_back(Item);
	}

	// correct Dlg Title
	DlgData[0].Y2 = DlgData.size();

	int ExitCode;
	{
		auto Dlg = Dialog::create(DlgData);
		Dlg->SetPosition(-1, -1, 76, static_cast<int>(DlgData.size() + 2));
		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode < 0 || ExitCode == static_cast<int>(DlgData.size() - 1))
	{
		strStr.clear();
		return 0;
	}

	string strTmpStr;

	for (Str=StartStr; *Str; Str++)
	{
		auto ItemIterator = std::find_if(CONST_RANGE(Positions, i) { return i.Pos == Str - StartStr; });
		if (ItemIterator != Positions.cend())
		{
			strTmpStr += DlgData[(ItemIterator - Positions.cbegin()) * 2 + 2].strData;
			Str = StartStr + ItemIterator->EndPos;
		}
		else
		{
			strTmpStr.append(Str,1);
		}
	}

	strStr = api::env::expand_strings(strTmpStr);
	return 1;
}

bool Panel::MakeListFile(string &strListFileName,bool ShortNames,const string& Modifers)
{
	bool Ret=false;

	if (FarMkTempEx(strListFileName))
	{
		api::File ListFile;
		if (ListFile.Open(strListFileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,CREATE_ALWAYS))
		{
			uintptr_t CodePage=CP_OEMCP;
			LPCVOID Eol="\r\n";
			DWORD EolSize=2;

			if (!Modifers.empty())
			{
				if (Modifers.find(L'A') != string::npos) // ANSI
				{
					CodePage=CP_ACP;
				}
				else
				{
					DWORD Signature=0;
					int SignatureSize=0;

					if (Modifers.find(L'W') != string::npos) // UTF16LE
					{
						CodePage=CP_UNICODE;
						Signature=SIGN_UNICODE;
						SignatureSize=2;
						Eol=DOS_EOL_fmt;
						EolSize=2*sizeof(WCHAR);
					}
					else
					{
						if (Modifers.find(L'U') != string::npos) // UTF8
						{
							CodePage=CP_UTF8;
							Signature=SIGN_UTF8;
							SignatureSize=3;
						}
					}

					if (Signature && SignatureSize)
					{
						size_t NumberOfBytesWritten;
						ListFile.Write(&Signature,SignatureSize, NumberOfBytesWritten);
					}
				}
			}

			string strFileName,strShortName;
			DWORD FileAttr;
			GetSelName(nullptr,FileAttr);

			while (GetSelName(&strFileName,FileAttr,&strShortName))
			{
				if (ShortNames)
					strFileName = strShortName;

				if (!Modifers.empty())
				{
					if (Modifers.find(L'F') != string::npos && PointToName(strFileName) == strFileName.data()) // 'F' - ������������ ������ ����; //BUGBUG ?
					{
						string strTempFileName(m_CurDir);

						if (ShortNames)
							ConvertNameToShort(strTempFileName,strTempFileName);

						AddEndSlash(strTempFileName);
						strTempFileName+=strFileName; //BUGBUG ?
						strFileName=strTempFileName;
					}

					if (Modifers.find(L'Q') != string::npos) // 'Q' - ��������� ����� � ��������� � �������;
						QuoteSpaceOnly(strFileName);

					if (Modifers.find(L'S') != string::npos) // 'S' - ������������ '/' ������ '\' � ����� ������;
					{
						std::replace(ALL_RANGE(strFileName), L'\\', L'/');
					}
				}

				LPCVOID Ptr=nullptr;
				char_ptr Buffer;
				size_t NumberOfBytesToWrite = 0, NumberOfBytesWritten = 0;

				if (CodePage==CP_UNICODE)
				{
					Ptr=strFileName.data();
					NumberOfBytesToWrite = strFileName.size()*sizeof(WCHAR);
				}
				else
				{
					int Size=WideCharToMultiByte(CodePage,0,strFileName.data(),static_cast<int>(strFileName.size()),nullptr,0,nullptr,nullptr);

					if (Size)
					{
						Buffer.reset(Size);

						if (Buffer)
						{
							NumberOfBytesToWrite=WideCharToMultiByte(CodePage, 0, strFileName.data(), static_cast<int>(strFileName.size()), Buffer.get(), Size, nullptr, nullptr);
							Ptr=Buffer.get();
						}
					}
				}

				BOOL Written=ListFile.Write(Ptr,NumberOfBytesToWrite,NumberOfBytesWritten);

				Buffer.reset();

				if (Written && NumberOfBytesWritten==NumberOfBytesToWrite)
				{
					if (ListFile.Write(Eol,EolSize,NumberOfBytesWritten) && NumberOfBytesWritten==EolSize)
					{
						Ret=true;
					}
				}
				else
				{
					Global->CatchError();
					Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListWrite),MSG(MOk));
					api::DeleteFile(strListFileName);
					break;
				}
			}

			ListFile.Close();
		}
		else
		{
			Global->CatchError();
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListTemp),MSG(MOk));
		}
	}
	else
	{
		Global->CatchError();
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListTemp),MSG(MOk));
	}

	return Ret;
}

static int IsReplaceVariable(const wchar_t *str,
                             int *scr,
                             int *end,
                             int *beg_scr_break,
                             int *end_scr_break,
                             int *beg_txt_break,
                             int *end_txt_break)
// ��� ����� ������ - ��������e 4 ��������� - ��� �������� �� str
// ������ ������ � ������ ��������, ����� ���� ������, ������ ������ � ������ ���������� ����������, �� � ����� �����.
// ������ ��� ������� ������ (������� � ��������� �����) ��� �������� ������:
// i = IsReplaceVariable(str) - ���� ��� ���� ������ ��������� ��������� ������ � ������ ?!
// ���  i - ��� ������, ������� ���� ���������, ���� �������� �� ����� ! ��������� !??!
{
	const wchar_t *s      = str;
	const wchar_t *scrtxt = str;
	int count_scob = 0;
	int second_count_scob = 0;
	bool was_quest = false;         //  ?
	bool was_asterics = false;      //  !
	bool in_firstpart_was_scob = false;
	const wchar_t *beg_firstpart_scob = nullptr;
	const wchar_t *end_firstpart_scob = nullptr;
	bool in_secondpart_was_scob = false;
	const wchar_t *beg_secondpart_scob = nullptr;
	const wchar_t *end_secondpart_scob = nullptr;

	if (!s)
		return -1;

	if (!StrCmpN(s,L"!?",2))
		s = s + 2;
	else
		return -1;

	//
	for (;;)   // analize from !? to ?
	{
		if (!*s)
			return -1;

		if (*s == L'(')
		{
			if (in_firstpart_was_scob)
			{
				//return -1;
			}
			else
			{
				in_firstpart_was_scob = true;
				beg_firstpart_scob = s;     //remember where is first break
			}

			count_scob += 1;
		}
		else if (*s == L')')
		{
			count_scob -= 1;

			if (!count_scob)
			{
				if (!end_firstpart_scob)
					end_firstpart_scob = s;   //remember where is last break
			}
			else if (count_scob < 0)
				return -1;
		}
		else if ((*s == L'?') && ((!beg_firstpart_scob && !end_firstpart_scob) || (beg_firstpart_scob && end_firstpart_scob)))
		{
			was_quest = true;
		}

		s++;

		if (was_quest) break;
	}

	if (count_scob ) return -1;

	scrtxt = s - 1; //remember s for return

	for (;;)   //analize from ? or !
	{
		if (!*s)
			return -1;

		if (*s == L'(')
		{
			if (in_secondpart_was_scob)
			{
				//return -1;
			}
			else
			{
				in_secondpart_was_scob = true;
				beg_secondpart_scob = s;    //remember where is first break
			}

			second_count_scob += 1;
		}
		else if (*s == L')')
		{
			second_count_scob -= 1;

			if (!second_count_scob)
			{
				if (!end_secondpart_scob)
					end_secondpart_scob = s;  //remember where is last break
			}
			else if (second_count_scob < 0)
				return -1;
		}
		else if ((*s == L'!') && ((!beg_secondpart_scob && !end_secondpart_scob) || (beg_secondpart_scob && end_secondpart_scob)))
		{
			was_asterics = true;
		}

		s++;

		if (was_asterics) break;
	}

	if (second_count_scob ) return -1;

	//
	if (scr )
		*scr = (int)(scrtxt - str);

	if (end )
		*end = (int)(s - str) - 1;

	if (in_firstpart_was_scob)
	{
		if (beg_scr_break )
			*beg_scr_break = (int)(beg_firstpart_scob - str);

		if (end_scr_break )
			*end_scr_break = (int)(end_firstpart_scob - str);
	}
	else
	{
		if (beg_scr_break )
			*beg_scr_break = -1;

		if (end_scr_break )
			*end_scr_break = -1;
	}

	if (in_secondpart_was_scob)
	{
		if (beg_txt_break )
			*beg_txt_break = (int)(beg_secondpart_scob - str);

		if (end_txt_break )
			*end_txt_break = (int)(end_secondpart_scob - str);
	}
	else
	{
		if (beg_txt_break )
			*beg_txt_break = -1;

		if (end_txt_break )
			*end_txt_break = -1;
	}

	return (int)((s - str) - 1);
}
