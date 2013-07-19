/*
help.cpp

������
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

#include "help.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "macroopcode.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "execute.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "filestr.hpp"
#include "colormix.hpp"
#include "stddlg.hpp"
#include "plugins.hpp"
#include "DlgGuid.hpp"
#include "RegExp.hpp"

static const wchar_t *FoundContents=L"__FoundContents__";
static const wchar_t *PluginContents=L"__PluginContents__";
static const wchar_t *HelpOnHelpTopic=L":Help";
static const wchar_t *HelpContents=L"Contents";

static const wchar_t HelpBeginLink = L'<';
static const wchar_t HelpEndLink = L'>';

enum HELPDOCUMENTSHELPTYPE
{
	HIDX_PLUGINS,                 // ������ ��������
	HIDX_DOCUMS,                  // ������ ����������
};

enum
{
	FHELPOBJ_ERRCANNOTOPENHELP  = 0x80000000,
};

class HelpRecord
{
public:
	string HelpStr;

	HelpRecord(const string& HStr):HelpStr(HStr){};

	HelpRecord& operator=(const HelpRecord &rhs)
	{
		if (this != &rhs)
		{
			HelpStr = rhs.HelpStr;
		}
		return *this;
	}

	bool operator==(const HelpRecord &rhs) const
	{
		return !StrCmpI(HelpStr, rhs.HelpStr);
	}

	bool operator <(const HelpRecord &rhs) const
	{
		return HelpStr < rhs.HelpStr;
	}
};

static int RunURL(const string& Protocol, const string& URLPath);

#define HelpFormatLink L"<%s\\>%s"
#define HelpFormatLinkModule L"<%s>%s"

string Help::MakeLink(const string& path, const string& topic)
{
	return string(L"<") + path + L"\\>" + topic;
}


Help::Help(const string& Topic, const wchar_t *Mask,UINT64 Flags):
	TopScreen(new SaveScreen),
	StrCount(0),
	FixCount(0),
	FixSize(0),
	MouseDownX(0),
	MouseDownY(0),
	BeforeMouseDownX(0),
	BeforeMouseDownY(0),
	MsX(-1),
	MsY(-1),
	CurColor(COL_HELPTEXT),
	CtrlTabSize(0),
	LastStartPos(0),
	StartPos(0),
	PrevMacroMode(Global->CtrlObject->Macro.GetMode()),
	MouseDown(false),
	IsNewTopic(true),
	TopicFound(false),
	ErrorHelp(true),
	LastSearchCase(Global->GlobalSearchCase),
	LastSearchWholeWords(Global->GlobalSearchWholeWords),
	LastSearchRegexp(Global->Opt->HelpSearchRegexp)
{
	CanLoseFocus=FALSE;
	KeyBarVisible=TRUE;
	/* $ OT �� ��������� ��� ����� ��������� ����������*/
	SetDynamicallyBorn(FALSE);
	Global->CtrlObject->Macro.SetMode(MACROAREA_HELP);
	StackData.Clear();
	StackData.Flags=Flags;
	StackData.strHelpMask = NullToEmpty(Mask); // �������� ����� �����
	StackData.strHelpTopic = Topic;

	if (Global->Opt->FullScreenHelp)
		SetPosition(0,0,ScrX,ScrY);
	else
		SetPosition(4,2,ScrX-4,ScrY-2);

	if (!ReadHelp(StackData.strHelpMask) && (Flags&FHELP_USECONTENTS))
	{
		StackData.strHelpTopic = Topic;

		if (StackData.strHelpTopic.front() == HelpBeginLink)
		{
			size_t pos = StackData.strHelpTopic.rfind(HelpEndLink);

			if (pos != string::npos)
				StackData.strHelpTopic.resize(pos+1);

			StackData.strHelpTopic += HelpContents;
		}

		StackData.strHelpPath.clear();
		ReadHelp(StackData.strHelpMask);
	}

	if (!HelpList.empty())
	{
		ScreenObjectWithShadow::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
		InitKeyBar();
		MacroMode = MACROAREA_HELP;
		MoveToReference(1,1);
		FrameManager->ExecuteModal(this); //OT
	}
	else
	{
		ErrorHelp=TRUE;

		if (!(Flags&FHELP_NOSHOWERROR))
		{
			if (!ScreenObjectWithShadow::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
			{
				Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),StackData.strHelpTopic.data(),MSG(MOk));
			}

			ScreenObjectWithShadow::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
		}
	}
}

Help::~Help()
{
	Global->CtrlObject->Macro.SetMode(PrevMacroMode);
	SetRestoreScreenMode(FALSE);
	delete TopScreen;
}


void Help::Hide()
{
	ScreenObjectWithShadow::Hide();
}


int Help::ReadHelp(const string& Mask)
{
	string strSplitLine;
	int Formatting=TRUE,RepeatLastLine,BreakProcess;
	size_t PosTab;
	const int MaxLength=X2-X1-1;
	string strTabSpace;
	string strPath;

	if (StackData.strHelpTopic.front()==HelpBeginLink)
	{
		strPath = StackData.strHelpTopic.data()+1;
		size_t pos = strPath.find(HelpEndLink);

		if (pos == string::npos)
			return FALSE;

		StackData.strHelpTopic = strPath.data() + pos + 1;
		strPath.resize(pos);
		DeleteEndSlash(strPath);
		AddEndSlash(strPath);
		StackData.strHelpPath = strPath;
	}
	else
	{
		strPath = !StackData.strHelpPath.empty() ? StackData.strHelpPath:Global->g_strFarPath;
	}

	if (StackData.strHelpTopic == PluginContents)
	{
		strFullHelpPathName.clear();
		ReadDocumentsHelp(HIDX_PLUGINS);
		return TRUE;
	}

	uintptr_t nCodePage = CP_OEMCP;
	File HelpFile;

	if (!OpenLangFile(HelpFile, strPath,(Mask.empty()?Global->HelpFileMask:Mask),Global->Opt->strHelpLanguage,strFullHelpPathName, nCodePage))
	{
		ErrorHelp=TRUE;

		if (!ScreenObjectWithShadow::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
		{
			ScreenObjectWithShadow::Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);

			if (!(StackData.Flags&FHELP_NOSHOWERROR))
			{
				Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MCannotOpenHelp),Mask.data(),MSG(MOk));
			}
		}

		return FALSE;
	}

	string strReadStr;

	if (GetOptionsParam(HelpFile,L"TabSize",strReadStr, nCodePage))
	{
		CtrlTabSize=_wtoi(strReadStr.data());
	}

	if (CtrlTabSize < 0 || CtrlTabSize > 16)
		CtrlTabSize=Global->Opt->HelpTabSize;

	if (GetOptionsParam(HelpFile,L"CtrlColorChar",strReadStr, nCodePage))
		strCtrlColorChar = strReadStr;
	else
		strCtrlColorChar.clear();

	if (GetOptionsParam(HelpFile,L"CtrlStartPosChar",strReadStr, nCodePage))
		strCtrlStartPosChar = strReadStr;
	else
		strCtrlStartPosChar.clear();

	/* $ 29.11.2001 DJ
	   ��������, ���� ��� �������� � PluginContents
	*/
	if (!GetLangParam(HelpFile,L"PluginContents",&strCurPluginContents, nullptr, nCodePage))
		strCurPluginContents.clear();

	strTabSpace.assign(CtrlTabSize, L' ');

	HelpList.clear();

	if (StackData.strHelpTopic == FoundContents)
	{
		Search(HelpFile,nCodePage);
		return TRUE;
	}

	StrCount=0;
	FixCount=0;
	TopicFound=0;
	RepeatLastLine=FALSE;
	BreakProcess=FALSE;
	int NearTopicFound=0;
	wchar_t PrevSymbol=0;

	StartPos = (DWORD)-1;
	LastStartPos = (DWORD)-1;
	int RealMaxLength;
	bool MacroProcess=false;
	int MI=0;
	string strMacroArea;

	GetFileString GetStr(HelpFile);
	int nStrLength;
	size_t SizeKeyName=20;

	for (;;)
	{
		if (StartPos != (DWORD)-1)
			RealMaxLength = MaxLength-StartPos;
		else
			RealMaxLength = MaxLength;

		if (!MacroProcess && !RepeatLastLine && !BreakProcess)
		{
			wchar_t *ReadStr;
			if (GetStr.GetString(&ReadStr, nCodePage, nStrLength) <= 0)
			{
				strReadStr=ReadStr;
				if (StringLen(strSplitLine)<MaxLength)
				{
					if (!strSplitLine.empty())
						AddLine(strSplitLine);
				}
				else
				{
					strReadStr.clear();
					RepeatLastLine=TRUE;
					continue;
				}

				break;
			}
			else
			{
				strReadStr=ReadStr;
			}
		}

		if (MacroProcess)
		{
			string strDescription;
			string strKeyName;
			string strOutTemp;

			if (!Global->CtrlObject->Macro.GetMacroKeyInfo(strMacroArea,MI,strKeyName,strDescription))
			{
				MacroProcess=false;
				MI=0;
				continue;
			}

			if (strKeyName.front() == L'~')
			{
				MI++;
				continue;
			}

			ReplaceStrings(strKeyName,L"~",L"~~",-1);
			ReplaceStrings(strKeyName,L"#",L"##",-1);
			ReplaceStrings(strKeyName,L"@",L"@@",-1);

			if (strKeyName.find(L'~') != string::npos) // ������������� �������
				SizeKeyName++;

			strOutTemp = str_printf(L" #%-*.*s# ",SizeKeyName,SizeKeyName,strKeyName.data());

			if (!strDescription.empty())
			{
				ReplaceStrings(strDescription,L"#",L"##",-1);
				ReplaceStrings(strDescription,L"~",L"~~",-1);
				ReplaceStrings(strDescription,L"@",L"@@",-1);
				strOutTemp+=strCtrlStartPosChar;
				strOutTemp+=strDescription;
			}

			strReadStr=strOutTemp;
			MacroProcess=true;
			MI++;
		}

		RepeatLastLine=FALSE;

		while ((PosTab = strReadStr.find(L'\t')) != string::npos)
		{
			strReadStr[PosTab] = L' ';

			if (CtrlTabSize > 1) // ������� ��������� �� ���� ���������
				strReadStr.insert(PosTab, strTabSpace.data(), CtrlTabSize - (PosTab % CtrlTabSize));
		}

		RemoveTrailingSpaces(strReadStr);

		if (!strCtrlStartPosChar.empty())
		{
			size_t pos = strReadStr.find(strCtrlStartPosChar);
			if (pos != string::npos)
			{
				LastStartPos = StringLen(strReadStr.substr(0, pos));
				strReadStr.erase(pos, strCtrlStartPosChar.size());
			}
		}

		if (TopicFound)
		{
			HighlightsCorrection(strReadStr);
		}

		if (strReadStr.front()==L'@' && !BreakProcess)
		{
			if (TopicFound)
			{
				if (strReadStr == L"@+")
				{
					Formatting=TRUE;
					PrevSymbol=0;
					continue;
				}

				if (strReadStr == L"@-")
				{
					Formatting=FALSE;
					PrevSymbol=0;
					continue;
				}

				if (strSplitLine.front())
				{
					BreakProcess=TRUE;
					strReadStr.clear();
					PrevSymbol=0;
					goto m1;
				}

				break;
			}
			else if (!StrCmpI(strReadStr.data()+1,StackData.strHelpTopic.data()))
			{
				TopicFound=1;
				NearTopicFound=1;
			}
			else // redirection @SearchTopic=RealTopic
			{
				size_t n1 = StackData.strHelpTopic.size();
				size_t n2 = strReadStr.size();
				if (1+n1+1 < n2 && !StrCmpNI(strReadStr.data()+1, StackData.strHelpTopic.data(), (int)n1) && strReadStr.at(1+n1) == L'=')
				{
					StackData.strHelpTopic = strReadStr.substr(1+n1+1);
					continue;
				}
			}
		}
		else
		{
m1:
			if (strReadStr.empty() && BreakProcess && strSplitLine.empty())
				break;

			if (TopicFound)
			{
				if (!StrCmpNI(strReadStr.data(),L"<!Macro:",8) && Global->CtrlObject)
				{
					if (((PosTab = strReadStr.find(L'>')) != string::npos) && strReadStr.at(PosTab-1) != L'!')
						continue;

					strMacroArea=strReadStr.substr(8,PosTab-1-8); //???
					MacroProcess=true;
					MI=0;
					string strDescription,strKeyName;
					while (Global->CtrlObject->Macro.GetMacroKeyInfo(strMacroArea,MI,strKeyName,strDescription))
					{
						SizeKeyName=std::max(SizeKeyName,strKeyName.size());
						MI++;
					}
					MI=0;
					continue;
				}

				if (!(strReadStr.front()==L'$' && NearTopicFound && (PrevSymbol == L'$' || PrevSymbol == L'@')))
					NearTopicFound=0;

				/* $<text> � ������ ������, ����������� ����
				   ���������� �� �������������� ������� ������
				   ���� ���� ��������� ������ ����� ����� ������ ����������� ����...
				*/
				if (NearTopicFound)
				{
					StartPos = (DWORD)-1;
					LastStartPos = (DWORD)-1;
				}

				if (strReadStr.front()==L'$' && NearTopicFound && (PrevSymbol == L'$' || PrevSymbol == L'@'))
				{
					AddLine(strReadStr.data()+1);
					FixCount++;
				}
				else
				{
					NearTopicFound=0;

					if (strReadStr.empty() || !Formatting)
					{
						if (!strSplitLine.empty())
						{
							if (StringLen(strSplitLine)<RealMaxLength)
							{
								AddLine(strSplitLine);
								strSplitLine.clear();

								if (StringLen(strReadStr)<RealMaxLength)
								{
									AddLine(strReadStr);
									LastStartPos = (DWORD)-1;
									StartPos = (DWORD)-1;
									continue;
								}
							}
							else
								RepeatLastLine=TRUE;
						}
						else if (!strReadStr.empty())
						{
							if (StringLen(strReadStr)<RealMaxLength)
							{
								AddLine(strReadStr);
								continue;
							}
						}
						else if (strReadStr.empty() && strSplitLine.empty())
						{
							AddLine(L"");
							continue;
						}
					}

					if (!strReadStr.empty() && IsSpace(strReadStr.front()) && Formatting)
					{
						if (StringLen(strSplitLine)<RealMaxLength)
						{
							if (!strSplitLine.empty())
							{
								AddLine(strSplitLine);
								StartPos = (DWORD)-1;
							}

							strSplitLine=strReadStr;
							strReadStr.clear();
							continue;
						}
						else
							RepeatLastLine=TRUE;
					}

					if (!RepeatLastLine)
					{
						if (!strSplitLine.empty())
							strSplitLine += L" ";

						strSplitLine += strReadStr;
					}

					if (StringLen(strSplitLine)<RealMaxLength)
					{
						if (strReadStr.empty() && BreakProcess)
							goto m1;

						continue;
					}

					int Splitted=0;

					for (int I=(int)strSplitLine.size()-1; I > 0; I--)
					{
						if (I > 0 && strSplitLine.at(I)==L'~' && strSplitLine.at(I-1)==L'~')
						{
							I--;
							continue;
						}

						if (I > 0 && strSplitLine.at(I)==L'~' && strSplitLine.at(I-1)!=L'~')
						{
							do
							{
								I--;
							}
							while (I > 0 && strSplitLine.at(I)!=L'~');

							continue;
						}

						if (strSplitLine[I] == L' ')
						{
							string FirstPart = strSplitLine.substr(0, I);
							if (StringLen(FirstPart.data()) < RealMaxLength)
							{
								AddLine(FirstPart.data());
								strSplitLine.erase(1, I);
								strSplitLine[0] = L' ';
								HighlightsCorrection(strSplitLine);
								Splitted=TRUE;
								break;
							}
						}
					}

					if (!Splitted)
					{
						AddLine(strSplitLine);
						strSplitLine.clear();
					}
					else
					{
						StartPos = LastStartPos;
					}
				}
			}

			if (BreakProcess)
			{
				if (!strSplitLine.empty())
					goto m1;

				break;
			}
		}

		PrevSymbol=strReadStr.front();
	}

	AddLine(L"");
	FixSize=FixCount?FixCount+1:0;
	ErrorHelp=FALSE;

	if (IsNewTopic)
	{
		StackData.CurX=StackData.CurY=0;
		StackData.TopStr=0;
	}

	return TopicFound;
}


void Help::AddLine(const string& Line)
{
	string strLine;

	if (StartPos != 0xFFFFFFFF)
	{
		DWORD StartPos0=StartPos;
		if (Line[0] == L' ')
			StartPos0--;

		if (StartPos0 > 0)
		{
			strLine.assign(StartPos0, L' ');
		}
	}

	strLine += Line;
	HelpList.emplace_back(strLine);
	StrCount++;
}

void Help::AddTitle(const string& Title)
{
	AddLine(L"^ #" + Title + L"#");
}

void Help::HighlightsCorrection(string &strStr)
{
	if ((std::count(ALL_CONST_RANGE(strStr), L'#') & 1) && strStr.front() != L'$')
		strStr.insert(0, 1, L'#');
}

void Help::DisplayObject()
{
	if (!TopScreen)
		TopScreen=new SaveScreen;

	if (!TopicFound)
	{
		if (!ErrorHelp) // ���� ��� ������, �� ��� �������������� ������
		{              // � �������� ��������� �������� � ����������� ����.
			ErrorHelp=TRUE;

			if (!(StackData.Flags&FHELP_NOSHOWERROR))
			{
				Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),StackData.strHelpTopic.data(),MSG(MOk));
			}

			ProcessKey(KEY_ALTF1);
		}

		return;
	}

	SetCursorType(0,10);

	if (StackData.strSelTopic.empty())
		MoveToReference(1,1);

	FastShow();

	if (!Global->Opt->FullScreenHelp)
	{
		HelpKeyBar.SetPosition(0,ScrY,ScrX,ScrY);

		if (Global->Opt->ShowKeyBar)
			HelpKeyBar.Show();
	}
	else
	{
		HelpKeyBar.Hide();
	}
}


void Help::FastShow()
{
	if (!Locked())
		DrawWindowFrame();

	CorrectPosition();
	StackData.strSelTopic.clear();
	/* $ 01.09.2000 SVS
	   ��������� �� ��������� ������� ���� ���������...
	   ����� ����� ���� ���������� � ����������� ����������
	*/
	CurColor=COL_HELPTEXT;

	for (int i=0; i<Y2-Y1-1; i++)
	{
		int StrPos;

		if (i<FixCount)
		{
			StrPos=i;
		}
		else if (i==FixCount && FixCount>0)
		{
			if (!Locked())
			{
				GotoXY(X1,Y1+i+1);
				SetColor(COL_HELPBOX);
				ShowSeparator(X2-X1+1,1);
			}

			continue;
		}
		else
		{
			StrPos=i+StackData.TopStr;

			if (FixCount>0)
				StrPos--;
		}

		if (StrPos<StrCount)
		{
			const HelpRecord *rec=GetHelpItem(StrPos);
			const wchar_t *OutStr=rec?rec->HelpStr.data():nullptr;

			if (!OutStr)
				OutStr=L"";

			if (*OutStr==L'^')
			{
				OutStr++;
				GotoXY(X1+1+std::max(0,(X2-X1-1-StringLen(OutStr))/2),Y1+i+1);
			}
			else
			{
				GotoXY(X1+1,Y1+i+1);
			}

			OutString(OutStr);
		}
	}

	if (!Locked())
	{
		SetColor(COL_HELPSCROLLBAR);
		ScrollBarEx(X2,Y1+FixSize+1,Y2-Y1-FixSize-1,StackData.TopStr,StrCount-FixCount);
	}
}

void Help::DrawWindowFrame()
{
	SetScreen(X1,Y1,X2,Y2,L' ',ColorIndexToColor(COL_HELPTEXT));
	Box(X1,Y1,X2,Y2,ColorIndexToColor(COL_HELPBOX),DOUBLE_BOX);
	SetColor(COL_HELPBOXTITLE);
	string strHelpTitleBuf;
	strHelpTitleBuf = MSG(MHelpTitle);
	strHelpTitleBuf += L" - ";

	if (!strCurPluginContents.empty())
		strHelpTitleBuf += strCurPluginContents;
	else
		strHelpTitleBuf += L"FAR";

	TruncStrFromEnd(strHelpTitleBuf,X2-X1-3);
	GotoXY(X1+(X2-X1+1-(int)strHelpTitleBuf.size()-2)/2,Y1);
	Global->FS << L" "<<strHelpTitleBuf<<L" ";
}

static const wchar_t *SkipLink( const wchar_t *Str, string *Name )
{
	for (;;)
	{
		while (*Str && *Str != L'@')
		{
			if (Name)
				Name->push_back(*Str);
			++Str;
		}
		if (*Str)
			++Str;
		if (*Str != L'@')
			break;
		if (Name)
			Name->push_back(*Str);
		++Str;
	}
	return Str;
}

static bool GetHelpColor(const wchar_t* &Str, wchar_t cColor, int &color)
{
	if (!cColor || Str[0] != cColor)
		return false;

	wchar_t wc1 = Str[1];
	if (wc1 == L'-')     // '\-' set default color
	{
		color = COL_HELPTEXT;
		Str += 2;
		return true;
	}

	if (!iswxdigit(wc1)) // '\hh' custom color
		return false;
	wchar_t wc2 = Str[2];
	if (!iswxdigit(wc2))
		return false;

	if (wc1 > L'9')
		wc1 -= L'A' - 10;
	if (wc2 > L'9')
		wc2 -= L'A' - 10;

	color = ((wc1 & 0x0f) << 4) | (wc2 & 0x0f);
	Str += 3;
	return true;
}

static bool FastParseLine(const wchar_t *Str, int *pLen, int x0, int realX, string *pTopic, wchar_t cColor)
{
	int x = x0, start_topic = -1;
	bool found = false;

	while (*Str)
	{
		wchar_t wc = *Str++;
		if (wc == *Str && (wc == L'~' || wc == L'@' || wc == L'#' || wc == cColor))
			++Str;
		else if (wc == L'#') // start/stop highlighting
			continue;
		else if (cColor && wc == cColor)
		{
			if (*Str == L'-')	// '\-' default color
			{
				Str += 2-1;
				continue;
			}
			else if (iswxdigit(*Str) && iswxdigit(Str[1])) // '\hh' custom color
			{
				Str += 3-1;
				continue;
			}
		}
		else if (wc == L'@')	// skip topic link //??? is it valid without ~topic~
		{
			Str = SkipLink(Str, nullptr);
			continue;
		}
		else if (wc == L'~')	// start/stop topic
		{
			if (start_topic < 0)
				start_topic = x;
			else
			{
				found = (realX >= start_topic && realX < x);
				if (*Str == L'@')
					Str = SkipLink(Str+1, found ? pTopic : nullptr);
				if (found)
					break;
				start_topic = -1;
			}
			continue;
		}

		++x;
		if (realX >= 0 && x > realX && start_topic < 0)
			break;
	}

	if (pLen)
		*pLen = x - x0;
	return found;
}

bool Help::GetTopic(int realX, int realY, string& strTopic)
{
	strTopic.clear();
	if (realY <= Y1 || realY >= Y2 || realX <= X1 || realX >= X2)
		return false;

	int y = -1;
	if (realY-Y1 <= FixSize)
	{
		if (y != FixCount)
			y = realY - Y1 - 1;
	}
	else
		y = realY - Y1 - 1 - FixSize+FixCount + StackData.TopStr;

	if (y < 0 || y >= StrCount)
		return false;
	const HelpRecord *rec = GetHelpItem(y);
	if (!rec || rec->HelpStr.empty())
		return false;

	int x = X1 + 1;
	const wchar_t *Str = rec->HelpStr.data();
	if (*Str == L'^') // center
	{
		int w = StringLen(++Str);
		x = X1 + 1 + std::max(0, (X2 - X1 - 1 - w)/2);
	}

	return FastParseLine(Str, nullptr, x, realX, &strTopic, strCtrlColorChar.front());
}

int Help::StringLen(const string& Str)
{
	int len = 0;
	FastParseLine(Str.data(), &len, 0, -1, nullptr, strCtrlColorChar.front());
	return len;
}

void Help::OutString(const wchar_t *Str)
{
	wchar_t OutStr[512]; //BUGBUG
	const wchar_t *StartTopic=nullptr;
	int OutPos=0,Highlight=0,Topic=0;
	wchar_t cColor = strCtrlColorChar.front();

	while (OutPos<(int)(ARRAYSIZE(OutStr)-10))
	{
		if ((Str[0]==L'~' && Str[1]==L'~') ||
		        (Str[0]==L'#' && Str[1]==L'#') ||
		        (Str[0]==L'@' && Str[1]==L'@') ||
		        (cColor && Str[0]==cColor && Str[1]==cColor)
		   )
		{
			OutStr[OutPos++]=*Str;
			Str+=2;
			continue;
		}

		if (*Str==L'~' || ((*Str==L'#' || *Str == cColor) && !Topic) /*|| *Str==HelpBeginLink*/ || !*Str)
		{
			OutStr[OutPos]=0;

			if (Topic)
			{
				int RealCurX,RealCurY;
				RealCurX=X1+StackData.CurX+1;
				RealCurY=Y1+StackData.CurY+FixSize+1;
				bool found = WhereY()==RealCurY && RealCurX>=WhereX() && RealCurX<WhereX()+(Str-StartTopic)-1;

				SetColor(found ? COL_HELPSELECTEDTOPIC : COL_HELPTOPIC);
				if (*Str && Str[1]==L'@')
				{
					Str = SkipLink(Str+2, found ? &StackData.strSelTopic : nullptr);
					Topic = 0;
				}
			}
			else
			{
				SetColor(Highlight ? COL_HELPHIGHLIGHTTEXT : CurColor);
			}

			/* $ 24.09.2001 VVM
			  ! ������� ������� ������ ��� ������. ����� ����� ������ ��� ������� �������... */
			if (static_cast<int>(StrLength(OutStr) + WhereX()) > X2)
				OutStr[X2 - WhereX()] = 0;

			if (Locked())
				GotoXY(WhereX()+StrLength(OutStr),WhereY());
			else
				Text(OutStr);

			OutPos=0;
		}

		if (!*Str)
		{
			break;
		}
		else if (*Str==L'~')
		{
			if (!Topic)
				StartTopic = Str;
			Topic = !Topic;
			Str++;
		}
		else if (*Str==L'@')
		{
			Str = SkipLink(Str+1, nullptr);
		}
		else if (*Str==L'#')
		{
			Highlight = !Highlight;
			Str++;
		}
		else if (!GetHelpColor(Str, cColor, CurColor))
		{
			OutStr[OutPos++]=*(Str++);
		}
	}

	if (!Locked() && WhereX()<X2)
	{
		SetColor(CurColor);
		Global->FS << fmt::MinWidth(X2-WhereX())<<L"";
	}
}


void Help::CorrectPosition()
{
	if (StackData.CurX>X2-X1-2)
		StackData.CurX=X2-X1-2;

	if (StackData.CurX<0)
		StackData.CurX=0;

	if (StackData.CurY>Y2-Y1-2-FixSize)
	{
		StackData.TopStr+=StackData.CurY-(Y2-Y1-2-FixSize);
		StackData.CurY=Y2-Y1-2-FixSize;
	}

	if (StackData.CurY<0)
	{
		StackData.TopStr+=StackData.CurY;
		StackData.CurY=0;
	}

	if (StackData.TopStr>StrCount-FixCount-(Y2-Y1-1-FixSize))
		StackData.TopStr=StrCount-FixCount-(Y2-Y1-1-FixSize);

	if (StackData.TopStr<0)
		StackData.TopStr=0;
}

__int64 Help::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_V_HELPFILENAME: // Help.FileName
			*(string *)vParam=strFullHelpPathName;     // ???
			break;
		case MCODE_V_HELPTOPIC: // Help.Topic
			*(string *)vParam=StackData.strHelpTopic;  // ???
			break;
		case MCODE_V_HELPSELTOPIC: // Help.SELTopic
			*(string *)vParam=StackData.strSelTopic;   // ???
			break;
		default:
			return 0;
	}

	return 1;
}

int Help::ProcessKey(int Key)
{
	if (StackData.strSelTopic.empty())
		StackData.CurX=StackData.CurY=0;

	switch (Key)
	{
		case KEY_NONE:
		case KEY_IDLE:
		{
			break;
		}
		case KEY_F5:
		{
			Global->Opt->FullScreenHelp=!Global->Opt->FullScreenHelp;
			ResizeConsole();
			return TRUE;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			FrameManager->DeleteFrame();
			SetExitCode(XC_QUIT);
			return TRUE;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:   case KEY_RCTRLNUMPAD7:
		case KEY_CTRLPGUP:    case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:   case KEY_RCTRLNUMPAD9:
		{
			StackData.CurX=StackData.CurY=0;
			StackData.TopStr=0;
			FastShow();

			if (StackData.strSelTopic.empty())
				MoveToReference(1,1);

			return TRUE;
		}
		case KEY_END:         case KEY_NUMPAD1:
		case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:    case KEY_RCTRLNUMPAD1:
		case KEY_CTRLPGDN:    case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN:   case KEY_RCTRLNUMPAD3:
		{
			StackData.CurX=StackData.CurY=0;
			StackData.TopStr=StrCount;
			FastShow();

			if (StackData.strSelTopic.empty())
			{
				StackData.CurX=0;
				StackData.CurY=Y2-Y1-2-FixSize;
				MoveToReference(0,1);
			}

			return TRUE;
		}
		case KEY_UP:          case KEY_NUMPAD8:
		{
			if (StackData.TopStr>0)
			{
				StackData.TopStr--;

				if (StackData.CurY<Y2-Y1-2-FixSize)
				{
					StackData.CurX=X2-X1-2;
					StackData.CurY++;
				}

				FastShow();

				if (StackData.strSelTopic.empty())
					MoveToReference(0,1);
			}
			else
				ProcessKey(KEY_SHIFTTAB);

			return TRUE;
		}
		case KEY_DOWN:        case KEY_NUMPAD2:
		{
			if (StackData.TopStr<StrCount-FixCount-(Y2-Y1-1-FixSize))
			{
				StackData.TopStr++;

				if (StackData.CurY>0)
					StackData.CurY--;

				StackData.CurX=0;
				FastShow();

				if (StackData.strSelTopic.empty())
					MoveToReference(1,1);
			}
			else
				ProcessKey(KEY_TAB);

			return TRUE;
		}
		/* $ 26.07.2001 VVM
		  + � ������ ������� �� 1 */
		case KEY_MSWHEEL_UP:
		case KEY_MSWHEEL_UP | KEY_ALT:
		case KEY_MSWHEEL_UP | KEY_RALT:
		{
			int n = (Key == KEY_MSWHEEL_UP ? (int)Global->Opt->MsWheelDeltaHelp : 1);
			while (n-- > 0)
				ProcessKey(KEY_UP);

			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case KEY_MSWHEEL_DOWN | KEY_ALT:
		case KEY_MSWHEEL_DOWN | KEY_RALT:
		{
			int n = (Key == KEY_MSWHEEL_DOWN ? (int)Global->Opt->MsWheelDeltaHelp : 1);
			while (n-- > 0)
				ProcessKey(KEY_DOWN);

			return TRUE;
		}
		case KEY_PGUP:      case KEY_NUMPAD9:
		{
			StackData.CurX=StackData.CurY=0;
			StackData.TopStr-=Y2-Y1-2-FixSize;
			FastShow();

			if (StackData.strSelTopic.empty())
			{
				StackData.CurX=StackData.CurY=0;
				MoveToReference(1,1);
			}

			return TRUE;
		}
		case KEY_PGDN:      case KEY_NUMPAD3:
		{
			{
				int PrevTopStr=StackData.TopStr;
				StackData.TopStr+=Y2-Y1-2-FixSize;
				FastShow();

				if (StackData.TopStr==PrevTopStr)
				{
					ProcessKey(KEY_CTRLPGDN);
					return TRUE;
				}
				else
					StackData.CurX=StackData.CurY=0;

				MoveToReference(1,1);
			}
			return TRUE;
		}
		case KEY_RIGHT:   case KEY_NUMPAD6:   case KEY_MSWHEEL_RIGHT:
		case KEY_TAB:
		{
			MoveToReference(1,0);
			return TRUE;
		}
		case KEY_LEFT:    case KEY_NUMPAD4:   case KEY_MSWHEEL_LEFT:
		case KEY_SHIFTTAB:
		{
			MoveToReference(0,0);
			return TRUE;
		}
		case KEY_F1:
		{
			// �� ������� SelTopic, ���� � ��� � Help on Help
			if (StrCmpI(StackData.strHelpTopic.data(),HelpOnHelpTopic))
			{
				Stack.emplace(StackData);
				IsNewTopic=TRUE;
				JumpTopic(HelpOnHelpTopic);
				IsNewTopic=FALSE;
				ErrorHelp=FALSE;
			}

			return TRUE;
		}
		case KEY_SHIFTF1:
		{
			//   �� ������� SelTopic, ���� � ��� � ���� Contents
			if (StrCmpI(StackData.strHelpTopic.data(),HelpContents))
			{
				Stack.emplace(StackData);
				IsNewTopic=TRUE;
				JumpTopic(HelpContents);
				ErrorHelp=FALSE;
				IsNewTopic=FALSE;
			}

			return TRUE;
		}
		case KEY_F7:
		{
			// �� ������� SelTopic, ���� � ��� � FoundContents
			if (StrCmpI(StackData.strHelpTopic.data(),FoundContents))
			{
				string strLastSearchStr0=strLastSearchStr;
				bool Case=LastSearchCase;
				bool WholeWords=LastSearchWholeWords;
				bool Regexp=LastSearchRegexp;

				string strTempStr;
				//int RetCode = GetString(MSG(MHelpSearchTitle),MSG(MHelpSearchingFor),L"HelpSearch",strLastSearchStr,strLastSearchStr0);
				int RetCode = GetSearchReplaceString(false, MSG(MHelpSearchTitle), MSG(MHelpSearchingFor), strLastSearchStr0, strTempStr, L"HelpSearch", L"", &Case, &WholeWords, nullptr, &Regexp, nullptr, nullptr, true, &HelpSearchId);

				if (RetCode <= 0)
					return TRUE;

				strLastSearchStr=strLastSearchStr0;
				LastSearchCase=Case;
				LastSearchWholeWords=WholeWords;
				LastSearchRegexp=Regexp;

				Stack.emplace(StackData);
				IsNewTopic=TRUE;
				JumpTopic(FoundContents);
				ErrorHelp=FALSE;
				IsNewTopic=FALSE;
			}

			return TRUE;

		}
		case KEY_SHIFTF2:
		{
			//   �� ������� SelTopic, ���� � ��� � PluginContents
			if (StrCmpI(StackData.strHelpTopic.data(),PluginContents))
			{
				Stack.emplace(StackData);
				IsNewTopic=TRUE;
				JumpTopic(PluginContents);
				ErrorHelp=FALSE;
				IsNewTopic=FALSE;
			}

			return TRUE;
		}
		case KEY_ALTF1:
		case KEY_RALTF1:
		case KEY_BS:
		{
			// ���� ���� �������� ���� - ������� �� �����
			if (!Stack.empty())
			{
				StackData = Stack.top();
				Stack.pop();
				JumpTopic(StackData.strHelpTopic);
				ErrorHelp=FALSE;
				return TRUE;
			}

			return ProcessKey(KEY_ESC);
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!StackData.strSelTopic.empty() && StrCmpI(StackData.strHelpTopic.data(),StackData.strSelTopic.data()))
			{
				Stack.push(StackData);
				IsNewTopic=TRUE;

				if (!JumpTopic())
				{
					StackData = Stack.top();
					Stack.pop();
					ReadHelp(StackData.strHelpMask); // ������ ��, ��� ����������.
				}

				ErrorHelp=FALSE;
				IsNewTopic=FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}

int Help::JumpTopic(const string& Topic)
{
	StackData.strSelTopic = Topic;
	return JumpTopic();
}

int Help::JumpTopic()
{
	string strNewTopic;
	size_t pos = 0;

	/* $ 14.07.2002 IS
	     ��� �������� �� ������� ���������� ������ ������ ���������� ����,
	     ���� ��� ��������.
	*/

	// ���� ������ �� ������ ����, ���� ������������� � ���� ��, �� ���� �����
	// ��������� ���������� ����, �� ������� ���
	if (StackData.strSelTopic.front()==HelpBeginLink
	        && (pos = StackData.strSelTopic.find(HelpEndLink,2)) != string::npos
	        && !IsAbsolutePath(StackData.strSelTopic.data()+1)
	        && !StackData.strHelpPath.empty())
	{
		strNewTopic.assign(StackData.strSelTopic.data()+1, pos);
		string strFullPath = StackData.strHelpPath;
		// ������ _���_ �������� ����� � ������� ����
		DeleteEndSlash(strFullPath);
		strFullPath.append(L"\\").append(strNewTopic.data()+(IsSlash(strNewTopic.front())?1:0));
		BOOL EndSlash = IsSlash(strFullPath.back());
		DeleteEndSlash(strFullPath);
		ConvertNameToFull(strFullPath,strNewTopic);
		strFullPath = str_printf(EndSlash? HelpFormatLink : HelpFormatLinkModule, strNewTopic.data(), wcschr(StackData.strSelTopic.data()+2, HelpEndLink)+1);
		StackData.strSelTopic = strFullPath;
	}

	//_SVS(SysLog(L"JumpTopic() = SelTopic=%s",StackData.SelTopic));
	// URL ��������� - ��� ���� ��� ������ :-)))
	{
		strNewTopic = StackData.strSelTopic;
		pos = strNewTopic.find(L':');

		if (pos != string::npos && strNewTopic.front() != L':') // �������� ��������������� URL
		{
			string Protocol(strNewTopic.data(), pos);

			if (RunURL(Protocol, StackData.strSelTopic))
			{
				return FALSE;
			}
		}
	}
	// � ��� ������ ���������...

	//_SVS(SysLog(L"JumpTopic() = SelTopic=%s, StackData.HelpPath=%s",StackData.SelTopic,StackData.HelpPath));
	if (!StackData.strHelpPath.empty() && StackData.strSelTopic.front() !=HelpBeginLink && StackData.strSelTopic != HelpOnHelpTopic)
	{
		if (StackData.strSelTopic.front()==L':')
		{
			strNewTopic = StackData.strSelTopic.data()+1;
			StackData.Flags&=~FHELP_CUSTOMFILE;
		}
		else if (StackData.Flags&FHELP_CUSTOMFILE)
			strNewTopic = StackData.strSelTopic;
		else
			strNewTopic = MakeLink(StackData.strHelpPath, StackData.strSelTopic);
	}
	else
	{
		strNewTopic = StackData.strSelTopic.data() + (StackData.strSelTopic == HelpOnHelpTopic? 1 : 0);
	}

	// ������ ������ �� .DLL
	wchar_t *lpwszNewTopic = GetStringBuffer(strNewTopic);
	wchar_t *p=wcsrchr(lpwszNewTopic,HelpEndLink);

	if (p)
	{
		if (!IsSlash(*(p-1)))
		{
			const wchar_t *p2=p;

			while (p >= lpwszNewTopic)
			{
				if (IsSlash(*p))
				{
					//++p;
					if (*p)
					{
						StackData.Flags|=FHELP_CUSTOMFILE;
						StackData.strHelpMask = p+1;
						StackData.strHelpMask.resize(StackData.strHelpMask.find(HelpEndLink));
					}

					wmemmove(p,p2,StrLength(p2)+1);
					const wchar_t *p3=wcsrchr(StackData.strHelpMask.data(),L'.');

					if (p3 && StrCmpI(p3,L".hlf"))
						StackData.strHelpMask.clear();

					break;
				}

				--p;
			}
		}
		else
		{
			StackData.Flags&=~FHELP_CUSTOMFILE;
			StackData.Flags|=FHELP_CUSTOMPATH;
		}
	}

	ReleaseStringBuffer(strNewTopic);

	//_SVS(SysLog(L"HelpMask=%s NewTopic=%s",StackData.HelpMask,NewTopic));
	if (StackData.strSelTopic.front() != L':' &&
	        (StrCmpI(StackData.strSelTopic.data(),PluginContents) || StrCmpI(StackData.strSelTopic.data(),FoundContents))
	   )
	{
		if (!(StackData.Flags&FHELP_CUSTOMFILE) && wcsrchr(strNewTopic.data(),HelpEndLink))
		{
			StackData.strHelpMask.clear();
		}
	}
	else
	{
		StackData.strHelpMask.clear();
	}

	StackData.strHelpTopic = strNewTopic;

	if (!(StackData.Flags&FHELP_CUSTOMFILE))
		StackData.strHelpPath.clear();

	if (!ReadHelp(StackData.strHelpMask))
	{
		StackData.strHelpTopic = strNewTopic;

		if (StackData.strHelpTopic.front() == HelpBeginLink)
		{
			if ((pos = StackData.strHelpTopic.rfind(HelpEndLink)) != string::npos)
			{
				StackData.strHelpTopic.resize(pos+1);
				StackData.strHelpTopic += HelpContents;
			}
		}

		StackData.strHelpPath.clear();
		ReadHelp(StackData.strHelpMask);
	}

	ScreenObjectWithShadow::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);

	if (HelpList.empty())
	{
		ErrorHelp=TRUE;

		if (!(StackData.Flags&FHELP_NOSHOWERROR))
		{
			Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),StackData.strHelpTopic.data(),MSG(MOk));
		}

		return FALSE;
	}

	// ResizeConsole();
	if (IsNewTopic
	        || !(StrCmpI(StackData.strSelTopic.data(),PluginContents)||StrCmpI(StackData.strSelTopic.data(),FoundContents)) // ��� ���������� ������� :-((
	   )
		MoveToReference(1,1);

	//FrameManager->ImmediateHide();
	FrameManager->RefreshFrame();
	return TRUE;
}



int Help::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	static const int HELPMODE_CLICKOUTSIDE = 0x20000000; // ���� ������� ���� ��� �����?

	if (HelpKeyBar.ProcessMouse(MouseEvent))
		return TRUE;

	if (MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED && MouseEvent->dwEventFlags!=MOUSE_MOVED)
	{
		ProcessKey(KEY_ENTER);
		return TRUE;
	}

	int prevMsX = MsX , prevMsY = MsY;
	MsX=MouseEvent->dwMousePosition.X;
	MsY=MouseEvent->dwMousePosition.Y;
	bool simple_move = (IntKeyState.MouseEventFlags == MOUSE_MOVED);


	if ((MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2) && IntKeyState.MouseEventFlags != MOUSE_MOVED)
	{
		if (Flags.Check(HELPMODE_CLICKOUTSIDE))
		{
			// ���������� ���� ���������� ����� �� ��� ������� ������
			if (IntKeyState.PreMouseEventFlags != DOUBLE_CLICK)
				ProcessKey(KEY_ESC);
		}

		if (MouseEvent->dwButtonState)
			Flags.Set(HELPMODE_CLICKOUTSIDE);

		return TRUE;
	}

	if (IntKeyState.MouseX==X2 && (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED))
	{
		int ScrollY=Y1+FixSize+1;
		int Height=Y2-Y1-FixSize-1;

		if (IntKeyState.MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_UP);

			return TRUE;
		}

		if (IntKeyState.MouseY==ScrollY+Height-1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_DOWN);

			return TRUE;
		}
		simple_move = false;
	}

	/* $ 15.03.2002 DJ
	   ���������� ������ � �������� ����������
	*/
	if (IntKeyState.MouseX == X2)
	{
		int ScrollY=Y1+FixSize+1;
		int Height=Y2-Y1-FixSize-1;

		if (StrCount > Height)
		{
			while (IsMouseButtonPressed())
			{
				if (IntKeyState.MouseY > ScrollY && IntKeyState.MouseY < ScrollY+Height+1)
				{
					StackData.CurX=StackData.CurY=0;
					StackData.TopStr=(IntKeyState.MouseY-ScrollY-1) * (StrCount-FixCount-Height+1) / (Height-2);
					FastShow();
				}
			}

			return TRUE;
		}
		simple_move = false;
	}

	// DoubliClock - ��������/���������� ����.
	if (MouseEvent->dwEventFlags==DOUBLE_CLICK &&
	        (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
	        MouseEvent->dwMousePosition.Y<Y1+1+FixSize)
	{
		ProcessKey(KEY_F5);
		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y<Y1+1+FixSize)
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY<Y1+1+FixSize)
			ProcessKey(KEY_UP);

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y>=Y2)
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY>=Y2)
			ProcessKey(KEY_DOWN);

		return TRUE;
	}

	/* $ 26.11.2001 VVM
	  + ��������� ������� ������� ����� � ������ � ���� ������ ����������� ��� ���������� */
	if (!MouseEvent->dwEventFlags
	 && (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
	{
		BeforeMouseDownX = StackData.CurX;
		BeforeMouseDownY = StackData.CurY;
		StackData.CurX = MouseDownX = MsX-X1-1;
		StackData.CurY = MouseDownY = MsY-Y1-1-FixSize;
		MouseDown = TRUE;
		simple_move = false;
	}

	if (!MouseEvent->dwEventFlags
	 && !(MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))
	 && MouseDown)
	{
		simple_move = false;
		MouseDown = FALSE;
		if (!StackData.strSelTopic.empty())
		{
			if (StackData.CurX == MouseDownX && StackData.CurY == MouseDownY)
				ProcessKey(KEY_ENTER);
		}
		else
		{
			if (StackData.CurX==MouseDownX && StackData.CurY==MouseDownY)
			{
				StackData.CurX = BeforeMouseDownX;
				StackData.CurY = BeforeMouseDownY;
			}
		}
	}

	if (simple_move && (prevMsX != MsX || prevMsY != MsY))
	{
		string strTopic;
		if (GetTopic(MsX, MsY, strTopic))
		{
			//if (strTopic != StackData.strSelTopic)
			{
				StackData.CurX = MsX-X1-1;
				StackData.CurY = MsY-Y1-1-FixSize;
			}
		}
	}

	FastShow();
	Sleep(1);
	return TRUE;
}


int Help::IsReferencePresent()
{
	CorrectPosition();
	int StrPos=FixCount+StackData.TopStr+StackData.CurY;

	if (StrPos >= StrCount)
	{
		return FALSE;
	}

	const HelpRecord *rec=GetHelpItem(StrPos);
	const wchar_t *OutStr=rec?rec->HelpStr.data():nullptr;
	return (OutStr  && wcschr(OutStr,L'@')  && wcschr(OutStr,L'~') );
}

const HelpRecord* Help::GetHelpItem(int Pos)
{
	if (static_cast<size_t>(Pos) < HelpList.size())
		return &HelpList[Pos];
	return nullptr;
}

void Help::MoveToReference(int Forward,int CurScreen)
{
	int StartSelection=!StackData.strSelTopic.empty();
	int SaveCurX=StackData.CurX;
	int SaveCurY=StackData.CurY;
	int SaveTopStr=StackData.TopStr;
	StackData.strSelTopic.clear();
	Lock();

	if (!ErrorHelp) while (StackData.strSelTopic.empty())
		{
			BOOL ReferencePresent=IsReferencePresent();

			if (Forward)
			{
				if (!StackData.CurX && !ReferencePresent)
					StackData.CurX=X2-X1-2;

				if (++StackData.CurX >= X2-X1-2)
				{
					StartSelection=0;
					StackData.CurX=0;
					StackData.CurY++;

					if (StackData.TopStr+StackData.CurY>=StrCount-FixCount ||
					        (CurScreen && StackData.CurY>Y2-Y1-2-FixSize))
						break;
				}
			}
			else
			{
				if (StackData.CurX==X2-X1-2 && !ReferencePresent)
					StackData.CurX=0;

				if (--StackData.CurX < 0)
				{
					StartSelection=0;
					StackData.CurX=X2-X1-2;
					StackData.CurY--;

					if (StackData.TopStr+StackData.CurY<0 ||
					        (CurScreen && StackData.CurY<0))
						break;
				}
			}

			FastShow();

			if (StackData.strSelTopic.empty())
				StartSelection=0;
			else
			{
				// ��������� �������, ��������� ���� �� ��� ������ :-)
				if (ReferencePresent && CurScreen)
					StartSelection=0;

				if (StartSelection)
					StackData.strSelTopic.clear();
			}
		}

	Unlock();

	if (StackData.strSelTopic.empty())
	{
		StackData.CurX=SaveCurX;
		StackData.CurY=SaveCurY;
		StackData.TopStr=SaveTopStr;
	}

	FastShow();
}

void Help::Search(File& HelpFile,uintptr_t nCodePage)
{
	StrCount=0;
	FixCount=1;
	FixSize=2;
	StackData.TopStr=0;
	TopicFound=TRUE;
	StackData.CurX=StackData.CurY=0;
	strCtrlColorChar.clear();

	string strTitleLine=strLastSearchStr;
	AddTitle(strTitleLine);

	bool TopicFound=false;
	GetFileString GetStr(HelpFile);
	int nStrLength;
	string strCurTopic, strEntryName, strReadStr;

	string strSlash(strLastSearchStr);
	InsertRegexpQuote(strSlash);
	std::vector<RegExpMatch> m;
	RegExp re;

	if (LastSearchRegexp)
	{
		// Q: ��� ������: ����� ������� ��� ����� RegExp`�?
		if (!re.Compile(strSlash.data(), OP_PERLSTYLE|OP_OPTIMIZE|(!LastSearchCase?OP_IGNORECASE:0)))
			return; //BUGBUG

		m.resize(re.GetBracketsCount() * 2);
	}

	string strSearchStrUpper = strLastSearchStr;
	string strSearchStrLower = strLastSearchStr;
	if (!LastSearchCase)
	{
		Upper(strSearchStrUpper);
		Lower(strSearchStrLower);
	}

	for (;;)
	{
		wchar_t *ReadStr;
		if (GetStr.GetString(&ReadStr, nCodePage, nStrLength) <= 0)
		{
			break;
		}

		strReadStr=ReadStr;
		RemoveTrailingSpaces(strReadStr);

		if (strReadStr.at(0)==L'@' && !(strReadStr.at(1)==L'+' || strReadStr.at(1)==L'-') && strReadStr.find(L'=') == string::npos)// && !TopicFound)
		{
			strEntryName=L"";
			strCurTopic=L"";
			RemoveExternalSpaces(strReadStr);
			if (StrCmpI(strReadStr.data()+1,HelpContents))
			{
				strCurTopic=strReadStr;
				TopicFound=true;
			}
		}
		else if (TopicFound && strReadStr.at(0)==L'$' && strReadStr.at(1) && !strCurTopic.empty())
		{
			strEntryName=strReadStr.data()+1;
			RemoveExternalSpaces(strEntryName);
			RemoveChar(strEntryName,L'#',false);
		}

		if (TopicFound && !strEntryName.empty())
		{
			// !!!BUGBUG: ���������� "��������" ������ strReadStr �� ��������� �������� !!!

			string ReplaceStr;
			int CurPos=0;
			int SearchLength;
			bool Result=SearchString(strReadStr.data(),(int)strReadStr.size(),strLastSearchStr,strSearchStrUpper,strSearchStrLower,re,m.data(),ReplaceStr,CurPos,0,LastSearchCase,LastSearchWholeWords,false,false,LastSearchRegexp,&SearchLength);

			if (Result)
			{
				AddLine(str_printf(L"   ~%s~%s@",strEntryName.data(), strCurTopic.data()));
				strCurTopic=L"";
				strEntryName=L"";
				TopicFound=false;
			}
		}
	}

	AddLine(L"");
	MoveToReference(1,1);
}

void Help::ReadDocumentsHelp(int TypeIndex)
{
	HelpList.clear();

	strCurPluginContents.clear();
	StrCount=0;
	FixCount=1;
	FixSize=2;
	StackData.TopStr=0;
	TopicFound=TRUE;
	StackData.CurX=StackData.CurY=0;
	strCtrlColorChar.clear();
	const wchar_t *PtrTitle=0, *ContentsName=0;
	string strPath, strFullFileName;

	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
			PtrTitle=MSG(MPluginsHelpTitle);
			ContentsName=L"PluginContents";
			break;
	}

	AddTitle(PtrTitle);
	/* TODO:
	   1. ����� (��� "����������") �� ������ � �������� Documets, ��
	      � � ��������
	*/
	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
		{
			std::for_each(CONST_RANGE(*Global->CtrlObject->Plugins, i)
			{
				strPath = i->GetModuleName();
				CutToSlash(strPath);
				uintptr_t nCodePage = CP_OEMCP;
				File HelpFile;
				if (OpenLangFile(HelpFile,strPath,Global->HelpFileMask,Global->Opt->strHelpLanguage,strFullFileName, nCodePage))
				{
					string strEntryName, strSecondParam;

					if (GetLangParam(HelpFile,ContentsName,&strEntryName,&strSecondParam, nCodePage))
					{
						string strHelpLine = L"   ~" + strEntryName;
						if (!strSecondParam.empty())
						{
							strHelpLine += L"," + strSecondParam;
						}
						strHelpLine += L"~@" + MakeLink(strPath, HelpContents) + L"@";

						AddLine(strHelpLine);
					}
				}
			});

			break;
		}
	}

	// ��������� �� ��������
	std::sort(HelpList.begin()+1, HelpList.end());
	// $ 26.06.2000 IS - ���������� ����� � ������ �� f1, shift+f2, end (������� ��������� IG)
	AddLine(L"");
}

// ������������ ������ � ������ ������ ��������
bool Help::MkTopic(const Plugin* pPlugin, const string& HelpTopic, string &strTopic)
{
	strTopic.clear();

	if (!HelpTopic.empty())
	{
		if (HelpTopic[0]==L':')
		{
			strTopic = HelpTopic.substr(1);
		}
		else
		{
			if (pPlugin && HelpTopic[0] != HelpBeginLink)
			{
				strTopic = str_printf(HelpFormatLinkModule, pPlugin->GetModuleName().data(), HelpTopic.data());
			}
			else
			{
				strTopic = HelpTopic;
			}

			if (strTopic.front()==HelpBeginLink)
			{
				wchar_t *Ptr;
				wchar_t *lpwszTopic = GetStringBuffer(strTopic, strTopic.size() * 2); //BUGBUG

				if (!(Ptr=wcschr(lpwszTopic,HelpEndLink)))
				{
					*lpwszTopic=0;
				}
				else
				{
					if (!Ptr[1]) // ���� ��� ������� ��...
						wcscat(lpwszTopic,HelpContents); // ... ������ ������� �������� ����. //BUGBUG

					/* � ��� ������ ���������...
					   ������ ����� ���� :
					     "<FullPath>Topic" ��� "<FullModuleName>Topic"
					   ��� ������ "FullPath" ���� ������ ������������� ������!
					   �.�. �� ������� ��� ��� - ��� ������ ��� ����!
					*/
					wchar_t* Ptr2=Ptr-1;

					if (!IsSlash(*Ptr2)) // ��� ��� ������?
					{
						// ������ ������ ��� ������� ��� :-)
						if (!(Ptr2=const_cast<wchar_t*>(LastSlash(lpwszTopic)))) // ��! ����� �����-�� :-(
							*lpwszTopic=0;
					}

					if (*lpwszTopic)
					{
						/* $ 21.08.2001 KM
						  - ������� ���������� ����� � ������ ������ �������,
						    � ������� ���� ��� ������ ������ ������������� "/".
						*/
						wmemmove(Ptr2+1,Ptr,StrLength(Ptr)+1); //???
						// � ��� ����� ������ ��� �� �������� Help API!
					}
				}

				ReleaseStringBuffer(strTopic);
			}
		}
	}

	return !strTopic.empty();
}

void Help::SetScreenPosition()
{
	if (Global->Opt->FullScreenHelp)
	{
		HelpKeyBar.Hide();
		SetPosition(0,0,ScrX,ScrY);
	}
	else
	{
		SetPosition(4,2,ScrX-4,ScrY-2);
	}

	Show();
}

void Help::InitKeyBar()
{
	HelpKeyBar.SetLabels(MHelpF1);
	HelpKeyBar.SetCustomLabels(KBA_HELP);
	SetKeyBar(&HelpKeyBar);
}

/* $ 25.08.2000 SVS
   ������ URL-������... ;-)
   ��� ���� ��� ������... ���?
   ������:
     0 - ��� �� URL ������ (�� ������)
     1 - CreateProcess ������ FALSE
     2 - ��� ��

   ��������� (��������):
     Protocol="mailto"
     URLPath ="mailto:vskirdin@mail.ru?Subject=Reversi"
*/
static int RunURL(const string& Protocol, const string& URLPath)
{
	int EditCode=0;

	if (!URLPath.empty() && (Global->Opt->HelpURLRules&0xFF))
	{
		string strType;

		if (GetShellType(Protocol,strType,AT_URLPROTOCOL))
		{
			string strAction;
			bool Success = false;
			if (strType.find(L"%1") != string::npos)
			{
				strAction = strType;
				Success = true;
			}
			else
			{
				strType = L"\\shell\\open\\command";
				HKEY hKey;

				if (RegOpenKeyEx(HKEY_CLASSES_ROOT,strType.data(),0,KEY_READ,&hKey) == ERROR_SUCCESS)
				{
					Success = RegQueryStringValue(hKey, L"", strAction, L"") == ERROR_SUCCESS;
					RegCloseKey(hKey);
				}
			}

			if (Success)
			{
				apiExpandEnvironmentStrings(strAction, strAction);

				string FilteredURLPath(URLPath);
				// ������ ��� ������ ������ ~~
				ReplaceStrings(FilteredURLPath, L"~~", L"~");
				// ������ ��� ������ ������ ##
				ReplaceStrings(FilteredURLPath, L"##", L"#");

				int Disposition=0;

				if (Global->Opt->HelpURLRules == 2 || Global->Opt->HelpURLRules == 2+256)
				{
					Disposition=Message(MSG_WARNING,2,MSG(MHelpTitle),
						                MSG(MHelpActivatorURL),
						                strAction.data(),
						                MSG(MHelpActivatorFormat),
						                FilteredURLPath.data(),
						                L"\x01",
						                MSG(MHelpActivatorQ),
						                MSG(MYes),MSG(MNo));
				}

				EditCode=2; // ��� Ok!

				if (!Disposition)
				{
					/*
					���� ����� ���������� ������ � ������������ ������
					���� ��� ����� ���������� - �� ����� ���� ���������!!!!!
					*/
					string strCurDir;
					apiGetCurrentDirectory(strCurDir);

					if (Global->Opt->HelpURLRules < 256) // SHELLEXECUTEEX_METHOD
					{
#if 0
						SHELLEXECUTEINFO sei={sizeof(sei)};
						sei.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
						sei.lpFile=RemoveExternalSpaces(Buf);
						sei.nShow=SW_SHOWNORMAL;
						seInfo.lpDirectory=strCurDir;

						if (ShellExecuteEx(&sei))
							EditCode=1;

#else
						strAction=FilteredURLPath;
						EditCode=ShellExecute(0, 0, RemoveExternalSpaces(strAction).data(), 0, strCurDir.data(), SW_SHOWNORMAL)?1:2;
#endif
					}
					else
					{
						STARTUPINFO si={sizeof(si)};
						PROCESS_INFORMATION pi={};

						if (ReplaceStrings(strAction, L"%1", FilteredURLPath, 1) == 0) //if %1 not found
						{
							strAction += L" ";
							strAction += FilteredURLPath;
						}

						if (!CreateProcess(nullptr, UNSAFE_CSTR(strAction),nullptr,nullptr,TRUE,0,nullptr,strCurDir.data(),&si,&pi))
						{
							EditCode=1;
						}
						else
						{
							CloseHandle(pi.hThread);
							CloseHandle(pi.hProcess);
						}
					}
				}
			}
		}
	}

	return EditCode;
}

void Help::OnChangeFocus(int Focus)
{
	if (Focus)
	{
		DisplayObject();
	}
}

void Help::ResizeConsole()
{
	bool OldIsNewTopic=IsNewTopic;
	bool ErrCannotOpenHelp=ScreenObjectWithShadow::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP);
	ScreenObjectWithShadow::Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);
	IsNewTopic=FALSE;
	delete TopScreen;
	TopScreen=nullptr;
	Hide();

	if (Global->Opt->FullScreenHelp)
	{
		HelpKeyBar.Hide();
		SetPosition(0,0,ScrX,ScrY);
	}
	else
		SetPosition(4,2,ScrX-4,ScrY-2);

	ReadHelp(StackData.strHelpMask);
	ErrorHelp=FALSE;
	//StackData.CurY--; // ��� ���� ������� (����� ���� ����� ���!)
	StackData.CurX--;
	MoveToReference(1,1);
	IsNewTopic=OldIsNewTopic;
	ScreenObjectWithShadow::Flags.Change(FHELPOBJ_ERRCANNOTOPENHELP,ErrCannotOpenHelp);
	FrameManager->ImmediateHide();
	FrameManager->RefreshFrame();
}

int Help::FastHide()
{
	return Global->Opt->AllCtrlAltShiftRule & CASR_HELP;
}


int Help::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MHelpType);
	strName = strFullHelpPathName;
	return(MODALTYPE_HELP);
}
