/*
stddlg.cpp

���� ������ ����������� ��������
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "stddlg.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "dialog.hpp"
#include "ctrlobj.hpp"
#include "farexcpt.hpp"
#include "registry.hpp"
#include "strmix.hpp"

int WINAPI GetSearchReplaceString(
    int IsReplaceMode,
    string *pSearchStr,
    string *pReplaceStr,
    const wchar_t *TextHistoryName,
    const wchar_t *ReplaceHistoryName,
    int *Case,
    int *WholeWords,
    int *Reverse,
    int *SelectFound,
    int *Regexp,
    const wchar_t *HelpTopic)
{
	if (!pSearchStr || (IsReplaceMode && !pReplaceStr))
		return FALSE;

	static const wchar_t *TextHistoryName0    = L"SearchText",
	        *ReplaceHistoryName0 = L"ReplaceText";
	int HeightDialog, DeltaCol1, DeltaCol2, DeltaCol, I;

	if (!TextHistoryName)
		TextHistoryName=TextHistoryName0;

	if (!ReplaceHistoryName)
		ReplaceHistoryName=ReplaceHistoryName0;

	if (IsReplaceMode)
	{
		/*
		  0         1         2         3         4         5         6         7
		  0123456789012345678901234567890123456789012345678901234567890123456789012345
		00
		01   +----------------------------- Replace ------------------------------+
		02   | Search for                                                         |
		03   |                                                                   |
		04   | Replace with                                                       |
		05   |                                                                   |
		06   +--------------------------------------------------------------------+
		07   | [ ] Case sensitive                 [ ] Regular expressions         |
		08   | [ ] Whole words                                                    |
		09   | [ ] Reverse search                                                 |
		10   +--------------------------------------------------------------------+
		11   |                      [ Replace ]  [ Cancel ]                       |
		12   +--------------------------------------------------------------------+
		13
		*/
		static DialogDataEx ReplaceDlgData[]=
		{
			/*  0 */DI_DOUBLEBOX,3,1,72,12,0,0,0,0,(const wchar_t *)MEditReplaceTitle,
			/*  1 */DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MEditSearchFor,
			/*  2 */DI_EDIT,5,3,70,3,1,0,DIF_HISTORY|DIF_USELASTHISTORY,0,L"",
			/*  3 */DI_TEXT,5,4,0,4,0,0,0,0,(const wchar_t *)MEditReplaceWith,
			/*  4 */DI_EDIT,5,5,70,5,0,0,DIF_HISTORY/*|DIF_USELASTHISTORY*/,0,L"",
			/*  5 */DI_TEXT,3,6,0,6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/*  6 */DI_CHECKBOX,5,7,0,7,0,0,0,0,(const wchar_t *)MEditSearchCase,
			/*  7 */DI_CHECKBOX,5,8,0,8,0,0,0,0,(const wchar_t *)MEditSearchWholeWords,
			/*  8 */DI_CHECKBOX,5,9,0,9,0,0,0,0,(const wchar_t *)MEditSearchReverse,
			/*  9 */DI_CHECKBOX,40,7,0,7,0,0,0,0,(const wchar_t *)MEditSearchRegexp,
			/* 10 */DI_TEXT,3,10,0,10,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/* 11 */DI_BUTTON,0,11,0,11,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MEditReplaceReplace,
			/* 12 */DI_BUTTON,0,11,0,11,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MEditSearchCancel
		};
		//������ ������ ������� �������� ������ ������� � �������.
		//������������, ��� ������� �� ������� Y+1 ����� ������, �� ������� �������
		//�������� ��� �� ������� �� ������� Y.
		static const int COL1_HIGH=8;
		static const int COL2_HIGH=9;
		HeightDialog=14;
		DeltaCol1=0;
		DeltaCol2=0;
		MakeDialogItemsEx(ReplaceDlgData,ReplaceDlg);

		if (!*TextHistoryName)
		{
			ReplaceDlg[2].History=0;
			ReplaceDlg[2].Flags&=~DIF_HISTORY;
		}
		else
			ReplaceDlg[2].History=TextHistoryName;

		if (!*ReplaceHistoryName)
		{
			ReplaceDlg[4].History=0;
			ReplaceDlg[4].Flags&=~DIF_HISTORY;
		}
		else
			ReplaceDlg[4].History=ReplaceHistoryName;

		ReplaceDlg[2].strData = *pSearchStr;

		if (*pReplaceStr)
			ReplaceDlg[4].strData = *pReplaceStr;

		if (Case)
			ReplaceDlg[6].Selected=*Case;
		else
		{
			DeltaCol1++;
			ReplaceDlg[0].Y2--;
			ReplaceDlg[6].Type=DI_TEXT;

			for (I=7; I <= COL1_HIGH; ++I)
			{
				ReplaceDlg[I].Y1--;
				ReplaceDlg[I].Y2--;
			}
		}

		if (WholeWords)
			ReplaceDlg[7].Selected=*WholeWords;
		else
		{
			DeltaCol1++;
			ReplaceDlg[0].Y2--;
			ReplaceDlg[7].Type=DI_TEXT;

			for (I=8; I <= COL1_HIGH; ++I)
			{
				ReplaceDlg[I].Y1--;
				ReplaceDlg[I].Y2--;
			}
		}

		if (Reverse)
			ReplaceDlg[8].Selected=*Reverse;
		else
		{
			DeltaCol1++;
			ReplaceDlg[0].Y2--;
			ReplaceDlg[8].Type=DI_TEXT;

			for (I=9; I <= COL1_HIGH; ++I)
			{
				ReplaceDlg[I].Y1--;
				ReplaceDlg[I].Y2--;
			}
		}

		if (Regexp)
			ReplaceDlg[9].Selected=*Regexp;
		else
		{
			DeltaCol2++;
			ReplaceDlg[0].Y2--;
			ReplaceDlg[9].Type=DI_TEXT;

			for (I=10; I <= COL2_HIGH; ++I)
			{
				ReplaceDlg[I].Y1--;
				ReplaceDlg[I].Y2--;
			}
		}

		//�������� ������
		DeltaCol=(DeltaCol1<DeltaCol2)?DeltaCol1:DeltaCol2;

		if (DeltaCol>0)
		{
			HeightDialog-=DeltaCol;

			for (I=10; I < (int)countof(ReplaceDlgData); ++I)
			{
				ReplaceDlg[I].Y1-=DeltaCol;
				ReplaceDlg[I].Y2-=DeltaCol;
			}
		}

		// ��� �� ����� 2 �������������� �����
		if (HeightDialog == 11)
		{
			for (I=10; I < (int)countof(ReplaceDlgData); ++I)
			{
				ReplaceDlg[I].Y1--;
				ReplaceDlg[I].Y2--;
			}
		}

		{
			Dialog Dlg(ReplaceDlg,countof(ReplaceDlgData));
			Dlg.SetPosition(-1,-1,76,HeightDialog);

			if (HelpTopic && *HelpTopic)
				Dlg.SetHelp(HelpTopic);

			Dlg.Process();

			if (Dlg.GetExitCode()!=11)
				return FALSE;
		}

		*pSearchStr = ReplaceDlg[2].strData;

		if (pReplaceStr)
			*pReplaceStr = ReplaceDlg[4].strData;

		if (Case)       *Case=ReplaceDlg[6].Selected;

		if (WholeWords) *WholeWords=ReplaceDlg[7].Selected;

		if (Reverse)    *Reverse=ReplaceDlg[8].Selected;

		if (Regexp)     *Regexp=ReplaceDlg[9].Selected;
	}
	else
	{
		/*
		  0         1         2         3         4         5         6         7
		  0123456789012345678901234567890123456789012345678901234567890123456789012345
		00
		01   +------------------------------ Search ------------------------------+
		02   | Search for                                                         |
		03   |                                                                   |
		04   +--------------------------------------------------------------------+
		05   | [ ] Case sensitive                 [ ] Regular expressions         |
		06   | [ ] Whole words                    [ ] Select found                |
		07   | [ ] Reverse search                                                 |
		08   +--------------------------------------------------------------------+
		09   |                       [ Search ]  [ Cancel ]                       |
		10   +--------------------------------------------------------------------+
		*/
		static DialogDataEx SearchDlgData[]=
		{
			/*  0 */DI_DOUBLEBOX,3,1,72,10,0,0,0,0,(const wchar_t *)MEditSearchTitle,
			/*  1 */DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MEditSearchFor,
			/*  2 */DI_EDIT,5,3,70,3,1,0,DIF_HISTORY|DIF_USELASTHISTORY,0,L"",
			/*  3 */DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/*  4 */DI_CHECKBOX,5,5,0,5,0,0,0,0,(const wchar_t *)MEditSearchCase,
			/*  5 */DI_CHECKBOX,5,6,0,6,0,0,0,0,(const wchar_t *)MEditSearchWholeWords,
			/*  6 */DI_CHECKBOX,5,7,0,7,0,0,0,0,(const wchar_t *)MEditSearchReverse,
			/*  7 */DI_CHECKBOX,40,5,0,5,0,0,0,0,(const wchar_t *)MEditSearchRegexp,
			/*  8 */DI_CHECKBOX,40,6,0,6,0,0,0,0,(const wchar_t *)MEditSearchSelFound,
			/*  9 */DI_TEXT,3,8,0,8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
			/* 10 */DI_BUTTON,0,9,0,9,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MEditSearchSearch,
			/* 11 */DI_BUTTON,0,9,0,9,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MEditSearchCancel,
		};
		//������ ������ ������� �������� ������ ������� � �������.
		//������������, ��� ������� �� ������� Y+1 ����� ������, �� ������� �������
		//�������� ��� �� ������� �� ������� Y.
		static const int COL1_HIGH=6;
		static const int COL2_HIGH=8;
		HeightDialog=12;
		DeltaCol1=0;
		DeltaCol2=0;
		MakeDialogItemsEx(SearchDlgData,SearchDlg);

		if (!*TextHistoryName)
		{
			SearchDlg[2].History=0;
			SearchDlg[2].Flags&=~DIF_HISTORY;
		}
		else
			SearchDlg[2].History=TextHistoryName;

		SearchDlg[2].strData = *pSearchStr;

		if (Case)
			SearchDlg[4].Selected=*Case;
		else
		{
			DeltaCol1++;
			SearchDlg[0].Y2--;
			SearchDlg[4].Type=DI_TEXT;

			for (I=5; I <= COL1_HIGH; ++I)
			{
				SearchDlg[I].Y1--;
				SearchDlg[I].Y2--;
			}
		}

		if (WholeWords)
			SearchDlg[5].Selected=*WholeWords;
		else
		{
			DeltaCol1++;
			SearchDlg[0].Y2--;
			SearchDlg[5].Type=DI_TEXT;

			for (I=6; I <= COL1_HIGH; ++I)
			{
				SearchDlg[I].Y1--;
				SearchDlg[I].Y2--;
			}
		}

		if (Reverse)
			SearchDlg[6].Selected=*Reverse;
		else
		{
			DeltaCol1++;
			SearchDlg[0].Y2--;
			SearchDlg[6].Type=DI_TEXT;

			for (I=7; I <= COL1_HIGH; ++I)
			{
				SearchDlg[I].Y1--;
				SearchDlg[I].Y2--;
			}
		}

		if (Regexp)
			SearchDlg[7].Selected=*Regexp;
		else
		{
			DeltaCol2++;
			SearchDlg[0].Y2--;
			SearchDlg[7].Type=DI_TEXT;

			for (I=8; I <= COL2_HIGH; ++I)
			{
				SearchDlg[I].Y1--;
				SearchDlg[I].Y2--;
			}
		}

		if (SelectFound)
			SearchDlg[8].Selected=*SelectFound;
		else
		{
			DeltaCol2++;
			SearchDlg[0].Y2--;
			SearchDlg[8].Type=DI_TEXT;

			for (I=9; I <= COL2_HIGH; ++I)
			{
				SearchDlg[I].Y1--;
				SearchDlg[I].Y2--;
			}
		}

		//�������� ������
		DeltaCol=(DeltaCol1<DeltaCol2)?DeltaCol1:DeltaCol2;

		if (DeltaCol>0)
		{
			HeightDialog-=DeltaCol;

			for (I=10; I < (int)countof(SearchDlgData); ++I)
			{
				SearchDlg[I].Y1-=DeltaCol;
				SearchDlg[I].Y2-=DeltaCol;
			}
		}

		// ��� �� ����� 2 �������������� �����
		if (HeightDialog == 9)
		{
			for (I=9; I < (int)countof(SearchDlgData); ++I)
			{
				SearchDlg[I].Y1--;
				SearchDlg[I].Y2--;
			}
		}

		{
			Dialog Dlg(SearchDlg,countof(SearchDlg));
			Dlg.SetPosition(-1,-1,76,HeightDialog);

			if (HelpTopic && *HelpTopic)
				Dlg.SetHelp(HelpTopic);

			Dlg.Process();

			if (Dlg.GetExitCode()!=10)
				return FALSE;
		}

		*pSearchStr = SearchDlg[2].strData;

		if (pReplaceStr)
			pReplaceStr->Clear();

		if (Case)
			*Case=SearchDlg[4].Selected;

		if (WholeWords)
			*WholeWords=SearchDlg[5].Selected;

		if (Reverse)
			*Reverse=SearchDlg[6].Selected;

		if (Regexp)
			*Regexp=SearchDlg[7].Selected;

		if (SelectFound)
			*SelectFound=SearchDlg[8].Selected;
	}

	return TRUE;
}


// ������� ��� ��������� ��� Shift-F4 Shift-Enter ��� ���������� Shift ;-)
static LONG_PTR WINAPI GetStringDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	/*
	  if(Msg == DM_KEY)
	  {
	//    char KeyText[50];
	//    KeyToText(Param2,KeyText);
	//    _D(SysLog(L"%s (0x%08X) ShiftPressed=%d",KeyText,Param2,ShiftPressed));
	    if(ShiftPressed && (Param2 == KEY_ENTER||Param2 == KEY_NUMENTER) && !CtrlObject->Macro.IsExecuting())
	    {
	      DWORD Arr[1];
	      Arr[0]=Param2 == KEY_ENTER?KEY_SHIFTENTER:KEY_SHIFTNUMENTER;
	      SendDlgMessage(hDlg,Msg,Param1,(long)Arr);
	      return TRUE;
	    }
	  }
	*/
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}


int WINAPI GetString(
    const wchar_t *Title,
    const wchar_t *Prompt,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    string &strDestText,
    const wchar_t *HelpTopic,
    DWORD Flags,
    int *CheckBoxValue,
    const wchar_t *CheckBoxText
)
{
	int Substract=5; // �������������� �������� :-)
	int ExitCode;
	bool addCheckBox=Flags&FIB_CHECKBOX && CheckBoxValue && CheckBoxText;
	int offset=addCheckBox?2:0;
	static DialogDataEx StrDlgData[]=
	{
		/*      Type          X1 Y1 X2  Y2 Focus Flags             DefaultButton
		                                      Selected               Data
		*/
		/* 0 */ DI_DOUBLEBOX, 3, 1, 72, 4, 0, 0, 0,                                0,L"",
		/* 1 */ DI_TEXT,      5, 2,  0, 2, 0, 0, DIF_SHOWAMPERSAND,                0,L"",
		/* 2 */ DI_EDIT,      5, 3, 70, 3, 1, 0, Flags&FIB_EDITPATH?DIF_EDITPATH:0,1,L"",
		/* 3 */ DI_TEXT,      0, 4,  0, 4, 0, 0, DIF_SEPARATOR,                    0,L"",
		/* 4 */ DI_CHECKBOX,  5, 5,  0, 5, 0, 0, 0,                                0,L"",
		/* 5 */ DI_TEXT,      0, 6,  0, 6, 0, 0, DIF_SEPARATOR,                    0,L"",
		/* 6 */ DI_BUTTON,    0, 7,  0, 7, 0, 0, DIF_CENTERGROUP,                  0,L"",
		/* 7 */ DI_BUTTON,    0, 7,  0, 7, 0, 0, DIF_CENTERGROUP,                  0,L""
	};
	MakeDialogItemsEx(StrDlgData,StrDlg);

	if (addCheckBox)
	{
		Substract-=2;
		StrDlg[0].Y2+=2;
		StrDlg[4].Selected=(*CheckBoxValue)?TRUE:FALSE;
		StrDlg[4].strData = CheckBoxText;
	}

	if (Flags&FIB_BUTTONS)
	{
		Substract-=3;
		StrDlg[0].Y2+=2;
		StrDlg[2].DefaultButton=0;
		StrDlg[4+offset].DefaultButton=1;
		StrDlg[5+offset].Y1=StrDlg[4+offset].Y1=5+offset;
		StrDlg[4+offset].Type=StrDlg[5+offset].Type=DI_BUTTON;
		StrDlg[4+offset].Flags=StrDlg[5+offset].Flags=DIF_CENTERGROUP;
		StrDlg[4+offset].strData = MSG(MOk);
		StrDlg[5+offset].strData = MSG(MCancel);
	}

	if (Flags&FIB_EXPANDENV)
	{
		StrDlg[2].Flags|=DIF_EDITEXPAND;
	}

	if (Flags&FIB_EDITPATH)
	{
		StrDlg[2].Flags|=DIF_EDITPATH;
	}

	if (HistoryName!=NULL)
	{
		StrDlg[2].History=HistoryName;
		StrDlg[2].Flags|=DIF_HISTORY|(Flags&FIB_NOUSELASTHISTORY?0:DIF_USELASTHISTORY);
	}

	if (Flags&FIB_PASSWORD)
		StrDlg[2].Type=DI_PSWEDIT;

	if (Title)
		StrDlg[0].strData = Title;

	if (Prompt)
	{
		StrDlg[1].strData = Prompt;
		TruncStrFromEnd(StrDlg[1].strData, 66);

		if (Flags&FIB_NOAMPERSAND)
			StrDlg[1].Flags&=~DIF_SHOWAMPERSAND;
	}

	if (SrcText)
		StrDlg[2].strData = SrcText;

	{
		Dialog Dlg(StrDlg,countof(StrDlg)-Substract,GetStringDlgProc);
		Dlg.SetPosition(-1,-1,76,offset+((Flags&FIB_BUTTONS)?8:6));

		if (HelpTopic!=NULL)
			Dlg.SetHelp(HelpTopic);

#if 0

		if (Opt.ExceptRules)
		{
			__try
			{
				Dlg.Process();
			}
			__except(xfilter(EXCEPT_FARDIALOG,
			                 GetExceptionInformation(),NULL,1)) // NULL=???
			{
				return FALSE;
			}
		}
		else
#endif
		{
			Dlg.Process();
		}

		ExitCode=Dlg.GetExitCode();
	}

	if (ExitCode == 2 || ExitCode == 4 || (addCheckBox && ExitCode == 6))
	{
		if (!(Flags&FIB_ENABLEEMPTY) && StrDlg[2].strData.IsEmpty())
			return(FALSE);

		strDestText = StrDlg[2].strData;

		if (addCheckBox)
			*CheckBoxValue=StrDlg[4].Selected;

		return(TRUE);
	}

	return(FALSE);
}

/*
  ����������� ������ ����� ������.
  ����� ��� ���������� ���������� ������ � ������.

  Name      - ���� ����� ������� ����� (max 256 ��������!!!)
  Password  - ���� ����� ������� ������ (max 256 ��������!!!)
  Title     - ��������� ������� (����� ���� NULL)
  HelpTopic - ���� ������ (����� ���� NULL)
  Flags     - ����� (GNP_*)
*/
int WINAPI GetNameAndPassword(const wchar_t *Title, string &strUserName, string &strPassword,const wchar_t *HelpTopic,DWORD Flags)
{
	static string strLastName, strLastPassword;
	int ExitCode;
	/*
	  0         1         2         3         4         5         6         7
	  0123456789012345678901234567890123456789012345678901234567890123456789012345
	|0                                                                             |
	|1   +------------------------------- Title -------------------------------+   |
	|2   | User name                                                           |   |
	|3   | *******************************************************************|   |
	|4   | User password                                                       |   |
	|5   | ******************************************************************* |   |
	|6   +---------------------------------------------------------------------+   |
	|7   |                         [ Ok ]   [ Cancel ]                         |   |
	|8   +---------------------------------------------------------------------+   |
	|9                                                                             |
	*/
	static DialogDataEx PassDlgData[]=
	{
		/* 0 */ DI_DOUBLEBOX,  3, 1,72, 8,0,0,0,0,L"",
		/* 1 */ DI_TEXT,       5, 2, 0, 2,0,0,0,0,L"",
		/* 2 */ DI_EDIT,       5, 3,70, 3,1,0,DIF_USELASTHISTORY|DIF_HISTORY,0,L"",
		/* 3 */ DI_TEXT,       5, 4, 0, 4,0,0,0,0,L"",
		/* 4 */ DI_PSWEDIT,    5, 5,70, 5,0,0,0,0,L"",
		/* 5 */ DI_TEXT,       3, 6, 0, 6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
		/* 6 */ DI_BUTTON,     0, 7, 0, 7,0,0,DIF_CENTERGROUP,1,L"",
		/* 7 */ DI_BUTTON,     0, 7, 0, 7,0,0,DIF_CENTERGROUP,0,L""
	};
	MakeDialogItemsEx(PassDlgData,PassDlg);
	PassDlg[1].strData = MSG(MNetUserName);
	PassDlg[3].strData = MSG(MNetUserPassword);
	PassDlg[6].strData = MSG(MOk);
	PassDlg[7].strData = MSG(MCancel);

	if (Title!=NULL)
		PassDlg[0].strData = Title;

	PassDlg[2].strData = (Flags&GNP_USELAST)?strLastName:strUserName;
	PassDlg[4].strData = (Flags&GNP_USELAST)?strLastPassword:strPassword;
	{
		Dialog Dlg(PassDlg,countof(PassDlg));
		Dlg.SetPosition(-1,-1,76,10);

		if (HelpTopic!=NULL)
			Dlg.SetHelp(HelpTopic);

		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
	}

	if (ExitCode!=6)
		return(FALSE);

	// ���������� ������.
	strUserName = PassDlg[2].strData;
	strLastName = strUserName;
	strPassword = PassDlg[4].strData;
	strLastPassword = strPassword;
	return(TRUE);
}
