/*
language.cpp

������ � lng �������
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

#include "language.hpp"
#include "scantree.hpp"
#include "vmenu2.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "filestr.hpp"
#include "interf.hpp"
#include "lasterror.hpp"

const wchar_t LangFileMask[] = L"*.lng";

bool OpenLangFile(api::File& LangFile, const string& Path,const string& Mask,const string& Language, string &strFileName, uintptr_t &nCodePage, bool StrongLang,string *pstrLangName)
{
	strFileName.clear();
	string strFullName, strEngFileName;
	api::FAR_FIND_DATA FindData;
	string strLangName;
	ScanTree ScTree(FALSE,FALSE);
	ScTree.SetFindPath(Path,Mask);

	while (ScTree.GetNextName(&FindData, strFullName))
	{
		strFileName = strFullName;

		if (!LangFile.Open(strFileName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
		{
			strFileName.clear();
		}
		else
		{
			GetFileFormat(LangFile, nCodePage, nullptr, false);

			if (GetLangParam(LangFile,L"Language",&strLangName,nullptr, nCodePage) && !StrCmpI(strLangName, Language))
				break;

			LangFile.Close();

			if (StrongLang)
			{
				strFileName.clear();
				strEngFileName.clear();
				break;
			}

			if (!StrCmpI(strLangName.data(),L"English"))
				strEngFileName = strFileName;
		}
	}

	if (!LangFile.Opened())
	{
		if (!strEngFileName.empty())
			strFileName = strEngFileName;

		if (!strFileName.empty())
		{
			LangFile.Open(strFileName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING);

			if (pstrLangName)
				*pstrLangName=L"English";
		}
	}

	return LangFile.Opened();
}


int GetLangParam(api::File& LangFile,const string& ParamName,string *strParam1, string *strParam2, UINT nCodePage)
{
	wchar_t* ReadStr;
	string strFullParamName = L".";
	strFullParamName += ParamName;
	int Length=(int)strFullParamName.size();
	/* $ 29.11.2001 DJ
	   �� ������� ������� � �����; ������ @Contents �� ������
	*/
	BOOL Found = FALSE;
	auto OldPos = LangFile.GetPointer();

	size_t ReadLength;
	GetFileString GetStr(LangFile, nCodePage);
	while (GetStr.GetString(&ReadStr, ReadLength))
	{
		if (!StrCmpNI(ReadStr,strFullParamName.data(),Length))
		{
			wchar_t *Ptr=wcschr(ReadStr,L'=');

			if (Ptr)
			{
				*strParam1 = Ptr+1;

				if (strParam2)
					strParam2->clear();

				size_t pos = strParam1->find(L',');

				if (pos != string::npos)
				{
					if (strParam2)
					{
						*strParam2 = *strParam1;
						strParam2->erase(0, pos+1);
						RemoveTrailingSpaces(*strParam2);
					}

					strParam1->resize(pos);
				}

				RemoveTrailingSpaces(*strParam1);
				Found = TRUE;
				break;
			}
		}
		else if (!StrCmpNI(ReadStr, L"@Contents", 9))
			break;
	}

	LangFile.SetPointer(OldPos, nullptr, FILE_BEGIN);
	return(Found);
}

static bool SelectLanguage(bool HelpLanguage)
{
	const wchar_t *Title,*Mask;
	StringOption *strDest;

	if (HelpLanguage)
	{
		Title=MSG(MHelpLangTitle);
		Mask=Global->HelpFileMask;
		strDest=&Global->Opt->strHelpLanguage;
	}
	else
	{
		Title=MSG(MLangTitle);
		Mask=LangFileMask;
		strDest=&Global->Opt->strLanguage;
	}

	VMenu2 LangMenu(Title,nullptr,0,ScrY-4);
	LangMenu.SetFlags(VMENU_WRAPMODE);
	LangMenu.SetPosition(ScrX/2-8+5*HelpLanguage,ScrY/2-4+2*HelpLanguage,0,0);
	string strFullName;
	api::FAR_FIND_DATA FindData;
	ScanTree ScTree(FALSE,FALSE);
	ScTree.SetFindPath(Global->g_strFarPath, Mask);

	while (ScTree.GetNextName(&FindData,strFullName))
	{
		api::File LangFile;
		if (!LangFile.Open(strFullName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
			continue;

		uintptr_t nCodePage=CP_OEMCP;
		GetFileFormat(LangFile, nCodePage, nullptr, false);
		string strLangName, strLangDescr;

		if (GetLangParam(LangFile,L"Language",&strLangName,&strLangDescr,nCodePage))
		{
			string strEntryName;

			if (!HelpLanguage || (!GetLangParam(LangFile,L"PluginContents",&strEntryName,nullptr,nCodePage) &&
			                      !GetLangParam(LangFile,L"DocumentContents",&strEntryName,nullptr,nCodePage)))
			{
				MenuItemEx LangMenuItem(str_printf(L"%.40s", !strLangDescr.empty() ? strLangDescr.data():strLangName.data()));

				/* $ 01.08.2001 SVS
				   �� ��������� ����������!
				   ���� � ������� � ����� �������� ��� ���� HLF � �����������
				   ������, ��... ����� ���������� ��� ������ �����.
				*/
				if (LangMenu.FindItem(0,LangMenuItem.strName,LIFIND_EXACTMATCH) == -1)
				{
					LangMenuItem.SetSelect(!StrCmpI(*strDest, strLangName));
					LangMenu.SetUserData(strLangName.data(), (strLangName.size()+1)*sizeof(wchar_t), LangMenu.AddItem(LangMenuItem));
				}
			}
		}
	}

	LangMenu.AssignHighlights(FALSE);
	LangMenu.Run();

	if (LangMenu.GetExitCode()<0)
		return false;

	*strDest = static_cast<const wchar_t*>(LangMenu.GetUserData(nullptr, 0));
	return true;
}

bool SelectInterfaceLanguage() {return SelectLanguage(false);}
bool SelectHelpLanguage() {return SelectLanguage(true);}


/* $ 01.09.2000 SVS
  + ����� �����, ��� ��������� ���������� ��� .Options
   .Options <KeyName>=<Value>
*/
int GetOptionsParam(api::File& SrcFile,const wchar_t *KeyName,string &strValue, UINT nCodePage)
{
	wchar_t* ReadStr;
	string strFullParamName;
	int Length=StrLength(L".Options");
	auto CurFilePos = SrcFile.GetPointer();
	GetFileString GetStr(SrcFile, nCodePage);
	size_t ReadSize;
	while (GetStr.GetString(&ReadStr, ReadSize))
	{
		if (!StrCmpNI(ReadStr,L".Options",Length))
		{
			strFullParamName = RemoveExternalSpaces(ReadStr+Length);
			size_t pos = strFullParamName.rfind(L'=');
			if (pos != string::npos)
			{
				strValue = strFullParamName;
				strValue.erase(0, pos+1);
				RemoveExternalSpaces(strValue);
				strFullParamName.resize(pos);
				RemoveExternalSpaces(strFullParamName);

				if (!StrCmpI(strFullParamName.data(),KeyName))
				{
					SrcFile.SetPointer(CurFilePos, nullptr, FILE_BEGIN);
					return TRUE;
				}
			}
		}
	}

	SrcFile.SetPointer(CurFilePos, nullptr, FILE_BEGIN);
	return FALSE;
}

Language::Language():
	MsgList(nullptr),
#ifndef NO_WRAPPER
	MsgListA(nullptr),
#endif // NO_WRAPPER
	MsgCount(0),
	LastError(LERROR_SUCCESS),
#ifndef NO_WRAPPER
	m_bUnicode(true),
#endif // NO_WRAPPER
	LanguageLoaded(false)
{
}

void ConvertString(const wchar_t *Src,string &strDest)
{
	strDest.reserve(wcslen(Src));
	while (*Src)
	{
		switch (*Src)
		{
		case L'\\':
			switch (Src[1])
			{
			case L'\\':
				strDest.push_back(L'\\');
				Src+=2;
				break;
			case L'\"':
				strDest.push_back(L'\"');
				Src+=2;
				break;
			case L'n':
				strDest.push_back(L'\n');
				Src+=2;
				break;
			case L'r':
				strDest.push_back(L'\r');
				Src+=2;
				break;
			case L'b':
				strDest.push_back(L'\b');
				Src+=2;
				break;
			case L't':
				strDest.push_back('\t');
				Src+=2;
				break;
			default:
				strDest.push_back(L'\\');
				Src++;
				break;
			}
			break;
		case L'"':
			strDest.push_back(L'"');
			Src+=(Src[1]==L'"') ? 2:1;
			break;
		default:
			strDest.push_back(*(Src++));
			break;
		}
	}
}

bool Language::Init(const string& Path, int CountNeed)
{
	if (MsgList
#ifndef NO_WRAPPER
	|| MsgListA
#endif // NO_WRAPPER
	)
		return true;
	SCOPED_ACTION(GuardLastError);
	LastError = LERROR_SUCCESS;
	uintptr_t nCodePage = CP_OEMCP;
	string strLangName=Global->Opt->strLanguage.Get();
	api::File LangFile;

	if (!OpenLangFile(LangFile,Path,LangFileMask,Global->Opt->strLanguage,strMessageFile, nCodePage, false, &strLangName))
	{
		LastError = LERROR_FILE_NOT_FOUND;
		return false;
	}
	if (this == Global->Lang && StrCmpI(Global->Opt->strLanguage, strLangName))
		Global->Opt->strLanguage=strLangName;

	UINT64 FileSize;
	LangFile.GetSize(FileSize);

#ifndef NO_WRAPPER
	if (!m_bUnicode)
	{
		MsgListA = static_cast<char*>(xf_malloc(FileSize));
	}
	else
#endif // NO_WRAPPER
	{
		MsgList = static_cast<wchar_t*>(xf_malloc(FileSize * sizeof(wchar_t)));
	}

	wchar_t* ReadStr;

	size_t MsgSize = 0;

	GetFileString GetStr(LangFile, nCodePage);
	size_t ReadSize;
	while (GetStr.GetString(&ReadStr, ReadSize))
	{
		string strDestStr;
		RemoveExternalSpaces(ReadStr);

		if (*ReadStr != L'\"')
			continue;

		int SrcLength=StrLength(ReadStr);

		if (ReadStr[SrcLength-1]==L'\"')
			ReadStr[SrcLength-1]=0;

		ConvertString(ReadStr+1,strDestStr);
		size_t DestLength=strDestStr.size()+1;

#ifndef NO_WRAPPER
		if (m_bUnicode)
#endif // NO_WRAPPER
		{
			wcscpy(MsgList+MsgSize, strDestStr.data());
		}
#ifndef NO_WRAPPER
		else
		{
			WideCharToMultiByte(CP_OEMCP, 0, strDestStr.data(), -1, MsgListA+MsgSize, static_cast<int>(DestLength), nullptr, nullptr);
		}
#endif // NO_WRAPPER
		MsgSize+=DestLength;
		MsgCount++;
	}

	//   �������� �������� �� ���������� ����� � LNG-������
	if (CountNeed != -1 && CountNeed != MsgCount-1)
	{
		LastError = LERROR_BAD_FILE;
		return false;
	}

#ifndef NO_WRAPPER
	if (!m_bUnicode)
	{
		MsgListA = static_cast<char*>(xf_realloc(MsgListA, MsgSize));
	}
	else
#endif // NO_WRAPPER
	{
		MsgList = static_cast<wchar_t*>(xf_realloc(MsgList, MsgSize * sizeof(wchar_t)));
	}

#ifndef NO_WRAPPER
	if (m_bUnicode)
#endif // NO_WRAPPER
	{
		wchar_t *CurAddr = MsgList;
		Messages.resize(MsgCount);

		std::for_each(RANGE(Messages, i)
		{
			i = CurAddr;
			CurAddr+=StrLength(CurAddr)+1;
		});
	}
#ifndef NO_WRAPPER
	else
	{
		char *CurAddrA = MsgListA;
		AnsiMessages.resize(MsgCount);

		std::for_each(RANGE(AnsiMessages, i)
		{
			i = CurAddrA;
			CurAddrA+=strlen(CurAddrA)+1;
		});
	}
#endif // NO_WRAPPER

	if (this == Global->Lang)
		Global->OldLang->Free();

	LanguageLoaded=true;
	return true;
}

#ifndef NO_WRAPPER
bool Language::InitA(const string& Path, int CountNeed)
{
	m_bUnicode = false;
	return Init(Path, CountNeed);
}
#endif // NO_WRAPPER

Language::~Language()
{
	Free();
}

void Language::Free()
{
	MsgCount=0;

	Messages.clear();
	if (MsgList)
	{
		xf_free(MsgList);
		MsgList=nullptr;
	}
#ifndef NO_WRAPPER
	AnsiMessages.clear();
	if (MsgListA)
	{
		xf_free(MsgListA);
		MsgListA=nullptr;
	}
	m_bUnicode = true;
#endif // NO_WRAPPER
}

void Language::Close()
{
	if (this == Global->Lang)
	{
		if (Global->OldLang->MsgCount)
			Global->OldLang->Free();

		Global->OldLang->MsgList=MsgList;
		Global->OldLang->Messages.swap(Messages);
#ifndef NO_WRAPPER
		Global->OldLang->MsgListA=MsgListA;
		Global->OldLang->AnsiMessages.swap(AnsiMessages);
		Global->OldLang->m_bUnicode=m_bUnicode;
#endif // NO_WRAPPER
		Global->OldLang->MsgCount=MsgCount;

		MsgList=nullptr;
	#ifndef NO_WRAPPER
		MsgListA=nullptr;
		m_bUnicode = true;
	#endif // NO_WRAPPER
		MsgCount=0;
	}
	else
	{
		Free();
	}
	LanguageLoaded=false;
}

bool Language::CheckMsgId(LNGID MsgId) const
{
	/* $ 19.03.2002 DJ
	   ��� ������������� ������� - ����� ������� ��������� �� ������
	   (��� �����, ��� ���������)
	*/
	if (MsgId>=MsgCount || MsgId < 0)
	{
		if (this == Global->Lang && !LanguageLoaded && this != Global->OldLang && Global->OldLang->CheckMsgId(MsgId))
			return true;

		/* $ 26.03.2002 DJ
		   ���� �������� ��� � ����� - ��������� �� �������
		*/
		if (!Global->FrameManager->ManagerIsDown())
		{
			/* $ 03.09.2000 IS
			   ! ���������� ��������� �� ���������� ������ � �������� �����
			     (������ ��� ����� ���������� ������ � ����������� ������ ������ - �
			     ����� �� ����� ������)
			*/
			string strMsg1(L"Incorrect or damaged ");
			strMsg1+=strMessageFile;
			/* IS $ */
			if (Message(MSG_WARNING, 2,
				L"Error",
				strMsg1.data(),
				(L"Message " + std::to_wstring(MsgId) + L" not found").data(),
				L"Ok", L"Quit")==1)
				exit(0);
		}

		return false;
	}

	return true;
}

const wchar_t* Language::GetMsg(LNGID nID) const
{
	if (
#ifndef NO_WRAPPER
	!m_bUnicode ||
#endif // NO_WRAPPER
	!CheckMsgId(nID))
		return L"";

	if (this == Global->Lang && this != Global->OldLang && !LanguageLoaded && Global->OldLang->MsgCount > 0)
		return Global->OldLang->Messages[nID];

	return Messages[nID];
}

#ifndef NO_WRAPPER
const char* Language::GetMsgA(LNGID nID) const
{
	if (m_bUnicode || !CheckMsgId(nID))
		return "";

	if (this == Global->Lang && this != Global->OldLang && !LanguageLoaded && Global->OldLang->MsgCount > 0)
		return Global->OldLang->AnsiMessages[nID];

	return AnsiMessages[nID];
}
#endif // NO_WRAPPER
