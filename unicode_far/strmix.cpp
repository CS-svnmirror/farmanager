/*
strmix.cpp

���� ������ ��������������� ������� �� ������ �� ��������
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

#include "strmix.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "config.hpp"
#include "pathmix.hpp"

string &FormatNumber(const wchar_t *Src, string &strDest, int NumDigits)
{
	static bool first = true;
	static NUMBERFMT fmt;
	static wchar_t DecimalSep[4];
	static wchar_t ThousandSep[4];
	if (first)
	{
		GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STHOUSAND,ThousandSep,countof(ThousandSep));
		GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,DecimalSep,countof(DecimalSep));
		DecimalSep[1]=0;  //� ����� ���������� ���� ����� ���� ������ ������ �������
		ThousandSep[1]=0; //�� ��� ��� ��� ����� �� ����� ������

		if(LOWORD(Opt.FormatNumberSeparators))
			*DecimalSep=LOWORD(Opt.FormatNumberSeparators);
		if(HIWORD(Opt.FormatNumberSeparators))
			*ThousandSep=HIWORD(Opt.FormatNumberSeparators);

		fmt.LeadingZero = 1;
		fmt.Grouping = 3;
		fmt.lpDecimalSep = DecimalSep;
		fmt.lpThousandSep = ThousandSep;
		fmt.NegativeOrder = 1;
		first = false;
	}
	fmt.NumDigits = NumDigits;
	string strSrc=Src;
	int Size=GetNumberFormat(LOCALE_USER_DEFAULT,0,strSrc,&fmt,NULL,0);
	wchar_t* lpwszDest=strDest.GetBuffer(Size);
	GetNumberFormat(LOCALE_USER_DEFAULT,0,strSrc,&fmt,lpwszDest,Size);
	strDest.ReleaseBuffer();
	return strDest;
}

string &InsertCommas(unsigned __int64 li,string &strDest)
{
   strDest.Format (L"%I64u", li);
   return FormatNumber(strDest,strDest);
/*
   wchar_t *lpwszDest = strDest.GetBuffer(strDest.GetLength() << 1); //BUGBUG

   for (int I=StrLength(lpwszDest)-4;I>=0;I-=3)
   {
      if (lpwszDest[I])
      {
        wmemmove(lpwszDest+I+2,lpwszDest+I+1,StrLength(lpwszDest+I));
        lpwszDest[I+1]=L',';
      }
   }

   strDest.ReleaseBuffer();

   return strDest;
*/
}

wchar_t * WINAPI InsertQuote(wchar_t *Str)
{
  size_t l = StrLength(Str);
  if (*Str != L'"')
  {
    wmemmove(Str+1,Str,++l);
    *Str=L'"';
  }
  if ((l-1) == 0 || Str[l-1] != L'"')
  {
    Str[l++] = L'\"';
    Str[l] = 0;
  }
  return Str;
}

wchar_t* WINAPI QuoteSpace(wchar_t *Str)
{
  if ( wcspbrk(Str, Opt.strQuotedSymbols) != NULL )
    InsertQuote(Str);

  return Str;
}


string& InsertQuote(string &strStr)
{
  size_t l = strStr.GetLength();
  wchar_t *Str = strStr.GetBuffer (strStr.GetLength()+3);

  if (*Str != L'"')
  {
    wmemmove(Str+1,Str,++l);
    *Str=L'"';
  }
  if((l-1) == 0 || Str[l-1] != L'"')
  {
    Str[l++] = L'\"';
  }

  strStr.ReleaseBuffer (l);

  return strStr;
}

string &QuoteSpace(string &strStr)
{
	if (wcspbrk(strStr, Opt.strQuotedSymbols) != NULL)
		InsertQuote(strStr);

	return strStr;
}


wchar_t*  WINAPI QuoteSpaceOnly(wchar_t *Str)
{
  if (wcschr(Str,L' ')!=NULL)
    InsertQuote(Str);

  return Str;
}


string& WINAPI QuoteSpaceOnly(string &strStr)
{
  if (strStr.Contains(L' '))
    InsertQuote(strStr);

  return(strStr);
}


string& __stdcall TruncStrFromEnd(string &strStr, int MaxLength)
{
	wchar_t *lpwszBuffer = strStr.GetBuffer ();

	TruncStrFromEnd(lpwszBuffer, MaxLength);

	strStr.ReleaseBuffer ();

	return strStr;
}

wchar_t* WINAPI TruncStrFromEnd(wchar_t *Str,int MaxLength)
{
  if ( Str )
  {
    int Length = StrLength(Str);

    if (Length > MaxLength)
    {
      if (MaxLength>3)
        wmemcpy(Str+MaxLength-3, L"...", 3);
      Str[MaxLength]=0;
    }
  }
  return Str;
}


wchar_t* WINAPI TruncStr(wchar_t *Str,int MaxLength)
{
  if (Str)
  {
    int Length=StrLength(Str);
    if (MaxLength<0)
      MaxLength=0;
    if (Length > MaxLength)
    {
      if (MaxLength>3)
      {
        wchar_t *MovePos = Str+Length-MaxLength+3;
        wmemmove(Str+3, MovePos, StrLength(MovePos)+1);
        wmemcpy(Str,L"...",3);
      }

      Str[MaxLength]=0;
    }
  }

  return Str;
}


string& __stdcall TruncStr(string &strStr, int MaxLength)
{
	wchar_t *lpwszBuffer = strStr.GetBuffer ();

	TruncStr(lpwszBuffer, MaxLength);

	strStr.ReleaseBuffer ();

	return strStr;
}


wchar_t* WINAPI TruncPathStr(wchar_t *Str, int MaxLength)
{
  if (Str)
  {
    int nLength = (int)wcslen (Str);

		if((nLength > MaxLength) && (nLength >= 2))
    {
      wchar_t *lpStart = NULL;

			if ( *Str && (Str[1] == L':') && IsSlash(Str[2]) )
         lpStart = Str+3;
      else
      {
        if ( (Str[0] == L'\\') && (Str[1] == L'\\') )
        {
					if ((lpStart = const_cast<wchar_t*>(FirstSlash(Str+2))) != NULL)
					{
						wchar_t *lpStart2=lpStart;
						if ( (lpStart-Str < nLength) && ((lpStart=const_cast<wchar_t*>(FirstSlash(lpStart2+1)))!=NULL))
              lpStart++;
					}
        }
      }

      if ( !lpStart || (lpStart-Str > MaxLength-5) )
        return TruncStr(Str, MaxLength);

      wchar_t *lpInPos = lpStart+3+(nLength-MaxLength);
      wmemmove (lpStart+3, lpInPos, (wcslen (lpInPos)+1));
      wmemcpy (lpStart, L"...", 3);
    }
  }

  return Str;
}


string& __stdcall TruncPathStr(string &strStr, int MaxLength)
{
	wchar_t *lpwszStr= strStr.GetBuffer ();

	TruncPathStr(lpwszStr, MaxLength);

	strStr.ReleaseBuffer ();

	return strStr;
}


wchar_t* WINAPI RemoveLeadingSpaces(wchar_t *Str)
{
  wchar_t *ChPtr;
  if ((ChPtr=Str) == 0)
    return NULL;

  for (; IsSpace(*ChPtr); ChPtr++)
         ;
  if (ChPtr!=Str)
    wmemmove(Str,ChPtr,StrLength(ChPtr)+1);
  return Str;
}


string& WINAPI RemoveLeadingSpaces(string &strStr)
{
  wchar_t *ChPtr = strStr.GetBuffer ();
  wchar_t *Str = ChPtr;

  for (; IsSpace(*ChPtr); ChPtr++)
    ;

  if (ChPtr!=Str)
    wmemmove(Str,ChPtr,(StrLength(ChPtr)+1));

  strStr.ReleaseBuffer ();

  return strStr;
}


// ������� �������� �������
wchar_t* WINAPI RemoveTrailingSpaces(wchar_t *Str)
{
  if (!Str)
    return NULL;
  if (*Str == L'\0')
    return Str;

  wchar_t *ChPtr;
  size_t I;

  for (ChPtr=Str+(I=StrLength(Str))-1; I > 0; I--, ChPtr--)
  {
    if (IsSpace(*ChPtr) || IsEol(*ChPtr))
      *ChPtr=0;
    else
      break;
  }

  return Str;
}


string& WINAPI RemoveTrailingSpaces(string &strStr)
{
  if ( strStr.IsEmpty () )
    return strStr;

  RemoveTrailingSpaces(strStr.GetBuffer());

  strStr.ReleaseBuffer ();

  return strStr;
}


wchar_t* WINAPI RemoveExternalSpaces(wchar_t *Str)
{
  return RemoveTrailingSpaces(RemoveLeadingSpaces(Str));
}

string&  WINAPI RemoveExternalSpaces(string &strStr)
{
  return RemoveTrailingSpaces(RemoveLeadingSpaces(strStr));
}


/* $ 02.02.2001 IS
   �������� ��������� ���������� ������� � ������. � ��������� ������
   �������������� ������ cr � lf.
*/
string& WINAPI RemoveUnprintableCharacters(string &strStr)
{
  wchar_t *Str = strStr.GetBuffer ();
  wchar_t *p = Str;

  while (*p)
  {
     if ( IsEol(*p) )
       *p=L' ';
     p++;
  }

  strStr.ReleaseBuffer();

  return RemoveExternalSpaces(strStr);
}


// ������� ������ Target �� ������ Str (�����!)
string &RemoveChar(string &strStr,wchar_t Target,BOOL Dup)
{
  wchar_t *Ptr = strStr.GetBuffer ();
  wchar_t *Str = Ptr, Chr;
  while ((Chr=*Str++) != 0)
  {
    if (Chr == Target)
    {
      if(Dup && *Str == Target)
      {
        *Ptr++ = Chr;
        ++Str;
      }
      continue;
    }
    *Ptr++ = Chr;
  }
  *Ptr = L'\0';

  strStr.ReleaseBuffer ();

  return strStr;
}

string& CenterStr(const wchar_t *Src, string &strDest, int Length)
{
  int SrcLength=StrLength(Src);

  string strTempStr = Src; //���� Src == strDest, �� ���� ���������� Src!

  if (SrcLength >= Length)
  {
    /* ����� �� ���� �������� 1 �� �����, �.�. strlen �� ��������� \0
       � �� �������� ���������� ������ */
    strDest = strTempStr;

    strDest.SetLength (Length);
  }
  else
  {
    int Space=(Length-SrcLength)/2;

    strDest.Format (L"%*s%s%*s",Space,L"",(const wchar_t*)strTempStr,Length-Space-SrcLength,L"");
  }

  return strDest;
}


const wchar_t *GetCommaWord(const wchar_t *Src, string &strWord,wchar_t Separator)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=FALSE;

  wchar_t *lpwszWord = strWord.GetBuffer (StrLength(Src)+1); //BUGBUG

  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src==L'[' && wcschr(Src+1,L']')!=NULL)
      SkipBrackets=TRUE;
    if (*Src==L']')
      SkipBrackets=FALSE;
    if (*Src==Separator && !SkipBrackets)
    {
      lpwszWord[WordPos]=0;
      Src++;
      while (IsSpace(*Src))
        Src++;

      strWord.ReleaseBuffer ();
      return(Src);
    }
    else
      lpwszWord[WordPos]=*Src;
  }
  lpwszWord[WordPos]=0;

  strWord.ReleaseBuffer ();

  return(Src);
}


BOOL IsCaseMixed (
	const string &strSrc
	)
{
	const wchar_t *lpwszSrc = (const wchar_t*)strSrc;

	while ( *lpwszSrc && !IsAlpha (*lpwszSrc) )
		lpwszSrc++;

	int Case = IsLower (*lpwszSrc);

	while ( *(lpwszSrc++) )
		if ( IsAlpha(*lpwszSrc) && (IsLower(*lpwszSrc) != Case) )
			return TRUE;

	return FALSE;
}

BOOL IsCaseLower (
	const string &strSrc
	)
{
	const wchar_t *lpwszSrc = (const wchar_t*)strSrc;

	while ( *lpwszSrc )
	{
		if ( !IsLower (*lpwszSrc) )
			return FALSE;

		lpwszSrc++;
	}

	return TRUE;
}



void WINAPI Unquote(wchar_t *Str)
{
  if (!Str)
    return;
  wchar_t *Dst=Str;
  while (*Str)
  {
    if (*Str!=L'\"')
      *Dst++=*Str;
    Str++;
  }
  *Dst=0;
}


void WINAPI Unquote(string &strStr)
{
  wchar_t *Dst = strStr.GetBuffer ();
  const wchar_t *Str = Dst;

  while (*Str)
  {
    if (*Str!=L'\"')
      *Dst++=*Str;
    Str++;
  }
  *Dst=0;

  strStr.ReleaseBuffer ();
}


void UnquoteExternal(string &strStr)
{
  if (strStr.At(0) == L'\"' && strStr.At(strStr.GetLength()-1) == L'\"')
  {
    strStr.SetLength(strStr.GetLength()-1);
    strStr.LShift(1);
  }
}


/* FileSizeToStr()
   �������������� ������� ����� � ������������� ���.
*/
#define MAX_KMGTBSTR_SIZE 16
static wchar_t KMGTbStrW[5][2][MAX_KMGTBSTR_SIZE]={0};

void __PrepareKMGTbStr(void)
{
  for(int I=0; I < 5; ++I)
  {
    xwcsncpy(KMGTbStrW[I][0],MSG(MListBytes+I),MAX_KMGTBSTR_SIZE-1);
    wcscpy(KMGTbStrW[I][1],KMGTbStrW[I][0]);
		CharLower(KMGTbStrW[I][0]);
		CharUpper(KMGTbStrW[I][1]);
  }
}


string & WINAPI FileSizeToStr(string &strDestStr, unsigned __int64 Size, int Width, int ViewFlags)
{
  string strStr;
  unsigned __int64 Divider;
  int IndexDiv, IndexB;

  // ���������������� �����������
  if(KMGTbStrW[0][0][0] == 0)
  {
    __PrepareKMGTbStr();
  }

  int Commas=(ViewFlags & COLUMN_COMMAS);
  int FloatSize=(ViewFlags & COLUMN_FLOATSIZE);
  int Economic=(ViewFlags & COLUMN_ECONOMIC);
  int UseMinSizeIndex=(ViewFlags & COLUMN_MINSIZEINDEX);
  int MinSizeIndex=(ViewFlags & COLUMN_MINSIZEINDEX_MASK)+1;
  int ShowBytesIndex=(ViewFlags & COLUMN_SHOWBYTESINDEX);

  if (ViewFlags & COLUMN_THOUSAND)
  {
    Divider=_ui64(1000);
    IndexDiv=0;
  }
  else
  {
    Divider=_ui64(1024);
    IndexDiv=1;
  }

  unsigned __int64 Sz = Size, Divider2 = Divider/2, Divider64 = Divider, OldSize;

  if (FloatSize)
  {
    unsigned __int64 Divider64F = 1, Divider64F_mul = _ui64(1000), Divider64F2 = _ui64(1), Divider64F2_mul = Divider;
    //������������ ��� �� 1000 �� ���� ������� ���������� �� Divider
    //�������� 999 bytes ��������� ��� 999 � ��� 1000 bytes ��� ��������� ��� 0.97 K
    for (IndexB=0; IndexB<4; IndexB++)
    {
      if (Sz < Divider64F*Divider64F_mul)
        break;
      Divider64F = Divider64F*Divider64F_mul;
      Divider64F2  = Divider64F2*Divider64F2_mul;
    }
    if (IndexB==0)
      strStr.Format (L"%d", (DWORD)Sz);
    else
    {
      Sz = (OldSize=Sz) / Divider64F2;
      OldSize = (OldSize % Divider64F2) / (Divider64F2 / Divider64F2_mul);
      DWORD Decimal = (DWORD)(0.5+(double)(DWORD)OldSize/(double)Divider*100.0);
      if (Decimal >= 100)
      {
        Decimal -= 100;
        Sz++;
      }
      strStr.Format (L"%d.%02d", (DWORD)Sz,Decimal);
      FormatNumber(strStr,strStr,2);
    }
    if (IndexB>0 || ShowBytesIndex)
    {
      Width-=(Economic?1:2);
      if (Width<0)
        Width=0;
      if (Economic)
        strDestStr.Format (L"%*.*s%1.1s",Width,Width,(const wchar_t *)strStr,KMGTbStrW[IndexB][IndexDiv]);
      else
        strDestStr.Format (L"%*.*s %1.1s",Width,Width,(const wchar_t *)strStr,KMGTbStrW[IndexB][IndexDiv]);
    }
    else
      strDestStr.Format (L"%*.*s",Width,Width,(const wchar_t *)strStr);

    return strDestStr;
  }

  if (Commas)
    InsertCommas(Sz,strStr);
  else
    strStr.Format (L"%I64u", Sz);

  if ((!UseMinSizeIndex && strStr.GetLength()<=static_cast<size_t>(Width)) || Width<5)
  {
    if (ShowBytesIndex)
    {
      Width-=(Economic?1:2);
      if (Width<0)
        Width=0;
      if (Economic)
        strDestStr.Format (L"%*.*s%1.1s",Width,Width,(const wchar_t *)strStr,KMGTbStrW[0][IndexDiv]);
      else
        strDestStr.Format (L"%*.*s %1.1s",Width,Width,(const wchar_t *)strStr,KMGTbStrW[0][IndexDiv]);
    }
    else
      strDestStr.Format (L"%*.*s",Width,Width,(const wchar_t *)strStr);
  }
  else
  {
    Width-=(Economic?1:2);
    IndexB=0;
    do{
      //Sz=(Sz+Divider2)/Divider64;
      Sz = (OldSize=Sz) / Divider64;
      if ((OldSize % Divider64) > Divider2)
        ++Sz;

      IndexB++;

      if (Commas)
        InsertCommas(Sz,strStr);
      else
        strStr.Format (L"%I64u",Sz);
    } while((UseMinSizeIndex && IndexB<MinSizeIndex) || strStr.GetLength() > static_cast<size_t>(Width));

    if (Economic)
      strDestStr.Format (L"%*.*s%1.1s",Width,Width,(const wchar_t*)strStr,KMGTbStrW[IndexB][IndexDiv]);
    else
      strDestStr.Format (L"%*.*s %1.1s",Width,Width,(const wchar_t*)strStr,KMGTbStrW[IndexB][IndexDiv]);
  }
  return strDestStr;
}



// �������� � ������� Pos � Str ������ InsStr (�������� InsSize ����)
// ���� InsSize = 0, ��... ��������� ��� ������ InsStr
// ���������� ��������� �� Str

wchar_t *InsertString(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize)
{
  int InsLen=StrLength(InsStr);
  if(InsSize && InsSize < InsLen)
    InsLen=InsSize;
  wmemmove(Str+Pos+InsLen, Str+Pos, (StrLength(Str+Pos)+1));
  wmemcpy(Str+Pos, InsStr, InsLen);
  return Str;
}


// �������� � ������ Str Count ��������� ��������� FindStr �� ��������� ReplStr
// ���� Count < 0 - �������� "�� ������ ������"
// Return - ���������� �����
int ReplaceStrings(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count,BOOL IgnoreCase)
{
  int I=0, J=0, Res;
  int LenReplStr=StrLength(ReplStr);
  int LenFindStr=StrLength(FindStr);
  int L=(int)strStr.GetLength ();

  wchar_t *Str = strStr.GetBuffer (1024); //BUGBUG!!!

  while(I <= L-LenFindStr)
  {
    Res=IgnoreCase?StrCmpNI(Str+I, FindStr, LenFindStr):StrCmpN(Str+I, FindStr, LenFindStr);
    if(Res == 0)
    {
      if(LenReplStr > LenFindStr)
        wmemmove(Str+I+(LenReplStr-LenFindStr),Str+I,(StrLength(Str+I)+1)); // >>
      else if(LenReplStr < LenFindStr)
        wmemmove(Str+I,Str+I+(LenFindStr-LenReplStr),(StrLength(Str+I+(LenFindStr-LenReplStr))+1)); //??
      wmemcpy(Str+I,ReplStr,LenReplStr);
      I += LenReplStr;
      if(++J == Count && Count > 0)
        break;
    }
    else
      I++;
    L=StrLength(Str);
  }

  strStr.ReleaseBuffer ();

  return J;
}

/*
From PHP 4.x.x
����������� �������� ����� �� �������� ������, ���������
�������������� ������. ���������� ������ SrcText ��������
� �������, �������� ���������� Width. ������ ������� ���
������ ������ Break.

��������� �� ������ � �������������� �����.

���� �������� Flahs & FFTM_BREAKLONGWORD, �� ������ ������
������������� �� �������� ������. ��� ���� � ��� ���� �����,
������� ������ �������� ������, �� ��� ����� ��������� �� �����.

Example 1.
FarFormatText("������ ������, ������� ����� ������� �� ��������� ����� �� ������ � 20 ��������.", 20 ,Dest, "\n", 0);
���� ������ ������:
---
������ ������,
������� �����
������� ��
��������� ����� ��
������ � 20
��������.
---

Example 2.
FarFormatText( "��� ������ �������� ��������������������������� ������ �����", 9, Dest, NULL, FFTM_BREAKLONGWORD);
���� ������ ������:

---
���
������
��������
���������
���������
���������
������
�����
---

*/

enum FFTMODE{
  FFTM_BREAKLONGWORD = 0x00000001,
};

string& WINAPI FarFormatText(const wchar_t *SrcText,     // ��������
                           int Width,               // �������� ������
                           string &strDestText,          // ��������
                           const wchar_t* Break,       // ����, ���� = NULL, �� ����������� '\n'
                           DWORD Flags)             // ���� �� FFTM_*
{
  const wchar_t *breakchar;
  breakchar = Break?Break:L"\n";

  if(!SrcText || !*SrcText)
  {
        strDestText = L"";
        return strDestText;
  }

  string strSrc = SrcText; //copy string in case of SrcText == strDestText

  if(!wcspbrk(strSrc,breakchar) && strSrc.GetLength() <= static_cast<size_t>(Width))
  {
    strDestText = strSrc;
    return strDestText;
  }

  long i=0, l=0, pgr=0, last=0;
  wchar_t *newtext;

  const wchar_t *text= strSrc;
  long linelength = Width;
  int breakcharlen = StrLength(breakchar);
  int docut = Flags&FFTM_BREAKLONGWORD?1:0;

  /* Special case for a single-character break as it needs no
     additional storage space */

  if (breakcharlen == 1 && docut == 0)
  {
    newtext = xf_wcsdup (text);
    if(!newtext)
    {
        strDestText = L"";
        return strDestText;
    }

    while (newtext[i] != L'\0')
    {
      /* prescan line to see if it is greater than linelength */
      l = 0;
      while (newtext[i+l] != breakchar[0])
      {
        if (newtext[i+l] == L'\0')
        {
          l--;
          break;
        }
        l++;
      }
      if (l >= linelength)
      {
        pgr = l;
        l = linelength;
        /* needs breaking; work backwards to find previous word */
        while (l >= 0)
        {
          if (newtext[i+l] == L' ')
          {
            newtext[i+l] = breakchar[0];
            break;
          }
          l--;
        }
        if (l == -1)
        {
          /* couldn't break is backwards, try looking forwards */
          l = linelength;
          while (l <= pgr)
          {
            if(newtext[i+l] == L' ')
            {
              newtext[i+l] = breakchar[0];
              break;
            }
            l++;
          }
        }
      }
      i += l+1;
    }
  }
  else
  {
    /* Multiple character line break */
    newtext = (wchar_t*)xf_malloc((strSrc.GetLength() * (breakcharlen+1)+1)*sizeof(wchar_t));
    if(!newtext)
    {
        strDestText = L"";
        return strDestText;
    }

    newtext[0] = L'\0';

    i = 0;
    while (text[i] != L'\0')
    {
      /* prescan line to see if it is greater than linelength */
      l = 0;
      while (text[i+l] != L'\0')
      {
        if (text[i+l] == breakchar[0])
        {
          if (breakcharlen == 1 || StrCmpN(text+i+l, breakchar, breakcharlen)==0)
            break;
        }
        l++;
      }
      if (l >= linelength)
      {
        pgr = l;
        l = linelength;

        /* needs breaking; work backwards to find previous word */
        while (l >= 0)
        {
          if (text[i+l] == L' ')
          {
            wcsncat(newtext, text+last, i+l-last);
            wcscat(newtext, breakchar);
            last = i + l + 1;
            break;
          }
          l--;
        }
        if (l == -1)
        {
          /* couldn't break it backwards, try looking forwards */
          l = linelength - 1;
          while (l <= pgr)
          {
            if (docut == 0)
            {
              if (text[i+l] == L' ')
              {
                wcsncat(newtext, text+last, i+l-last);
                wcscat(newtext, breakchar);
                last = i + l + 1;
                break;
              }
            }
            if (docut == 1)
            {
              if (text[i+l] == L' ' || l > i-last)
              {
                wcsncat(newtext, text+last, i+l-last+1);
                wcscat(newtext, breakchar);
                last = i + l + 1;
                break;
              }
            }
            l++;
          }
        }
        i += l+1;
      }
      else
      {
        i += (l ? l : 1);
      }
    }

    if (i+l > last)
    {
      wcscat(newtext, text+last);
    }
  }

  strDestText = newtext;
  xf_free(newtext);
  return strDestText;
}

/*
  Ptr=CalcWordFromString(Str,I,&Start,&End);
  xstrncpy(Dest,Ptr,End-Start+1);
  Dest[End-Start+1]=0;

// ���������:
//   WordDiv  - ����� ������������ ����� � ��������� OEM
  ���������� ��������� �� ������ �����
*/
const wchar_t * const CalcWordFromString(const wchar_t *Str,int CurPos,int *Start,int *End, const wchar_t *WordDiv0)
{
  int I, J, StartWPos, EndWPos;
  DWORD DistLeft, DistRight;
  int StrSize=StrLength(Str);

  if(CurPos >= StrSize)
    return NULL;

  string strWordDiv(WordDiv0);
  strWordDiv += L" \t\n\r";

  if(IsWordDiv(strWordDiv,Str[CurPos]))
  {
    // ��������� ��������� - ���� ������, ��� ����� ����� - ����� ��� ������
    I=J=CurPos;

    // ������ �����
    DistLeft=-1;
    while(I >= 0 && IsWordDiv(strWordDiv,Str[I]))
    {
      DistLeft++;
      I--;
    }
    if(I < 0)
      DistLeft=-1;

    // ������ ������
    DistRight=-1;
    while(J < StrSize && IsWordDiv(strWordDiv,Str[J]))
    {
      DistRight++;
      J++;
    }
    if(J >= StrSize)
      DistRight=-1;

    if(DistLeft > DistRight) // ?? >=
      EndWPos=StartWPos=J;
    else
      EndWPos=StartWPos=I;
  }
  else // ����� ��� ���, �.�. ����� �� �������
    EndWPos=StartWPos=CurPos;

  if(StartWPos < StrSize)
  {
    while(StartWPos >= 0)
      if(IsWordDiv(strWordDiv,Str[StartWPos]))
      {
        StartWPos++;
        break;
      }
      else
        StartWPos--;
    while(EndWPos < StrSize)
      if(IsWordDiv(strWordDiv,Str[EndWPos]))
      {
        EndWPos--;
        break;
      }
      else
        EndWPos++;
  }

  if(StartWPos < 0)
    StartWPos=0;
  if(EndWPos >= StrSize)
    EndWPos=StrSize;

  *Start=StartWPos;
  *End=EndWPos;

  return Str+StartWPos;
}


bool CheckFileSizeStringFormat(const wchar_t *FileSizeStr)
{
//��������� ���� ������ ������ �����: [0-9]+[BbKkMmGgTt]?

  const wchar_t *p = FileSizeStr;

  while (iswdigit(*p))
    p++;

  if (p == FileSizeStr)
    return false;

  if (*p)
  {
    if (*(p+1))
      return false;

    if (!StrStrI(L"BKMGT", p))
      return false;
  }

  return true;
}

unsigned __int64 ConvertFileSizeString(const wchar_t *FileSizeStr)
{
  if (!CheckFileSizeStringFormat(FileSizeStr))
    return _ui64(0);

  unsigned __int64 n = _wtoi64(FileSizeStr);

  wchar_t c = Upper(FileSizeStr[StrLength(FileSizeStr)-1]);

  switch (c)
  {
    case L'K':
      n <<= 10;
      break;

    case L'M':
      n <<= 20;
      break;

    case L'G':
      n <<= 30;
      break;

    case L'T':
      n <<= 40;
      break;
  }

  return n;
}

/* $ 21.09.2003 KM
   ������������� ������ �� ��������� ����.
*/
void Transform(string &strBuffer,const wchar_t *ConvStr,wchar_t TransformType)
{
  string strTemp;
  switch(TransformType)
  {
    case L'X': // Convert common string to hexadecimal string representation
    {
      string strHex;
      while(*ConvStr)
      {
        strHex.Format(L"%02X",*ConvStr);
        strTemp += strHex;
        ConvStr++;
      }
      break;
    }
    case L'S': // Convert hexadecimal string representation to common string
    {
      const wchar_t *ptrConvStr=ConvStr;
      while(*ptrConvStr)
      {
        if(*ptrConvStr != L' ')
        {
          WCHAR Hex[]={ptrConvStr[0],ptrConvStr[1],0};
          size_t l=strTemp.GetLength();
          wchar_t *Temp=strTemp.GetBuffer(l+2);
          Temp[l]=(wchar_t)wcstoul(Hex,NULL,16)&0xFFFF;
          strTemp.ReleaseBuffer(l+1);
          ptrConvStr++;
        }
        ptrConvStr++;
      }
      break;
    }
    default:
      break;
  }
  strBuffer=strTemp;
}
