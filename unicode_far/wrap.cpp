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

void AnsiToUnicodeBin(const char *lpszAnsiString, wchar_t *lpwszUnicodeString, int nLength,bool UseGlyphChars=true)
{
	if(lpszAnsiString && lpwszUnicodeString && nLength)
	{
		wmemset (lpwszUnicodeString, 0, nLength);
		MultiByteToWideChar(CP_OEMCP,UseGlyphChars?MB_USEGLYPHCHARS:0,lpszAnsiString,nLength,lpwszUnicodeString,nLength);
	}
}

wchar_t *AnsiToUnicodeBin(const char *lpszAnsiString, int nLength,bool UseGlyphChars=true)
{
	wchar_t *lpResult = (wchar_t*)xf_malloc(nLength*sizeof(wchar_t));
	AnsiToUnicodeBin(lpszAnsiString,lpResult,nLength,UseGlyphChars);
	return lpResult;
}

wchar_t *AnsiToUnicode(const char *lpszAnsiString,bool UseGlyphChars=true)
{
	if(!lpszAnsiString)
		return NULL;
	return AnsiToUnicodeBin(lpszAnsiString,(int)strlen(lpszAnsiString)+1,UseGlyphChars);
}

char *UnicodeToAnsiBin (const wchar_t *lpwszUnicodeString, int nLength)
{
	/* $ 06.01.2008 TS
	   ! �������� ������ ���������� ��� ������ ������ �� 1 ���� ��� ����������
	   	 ������ ������ ��������, ������� �� �����, ��� ���� �������� �� �����,
	   	 � �� �� ����������� ���� (�������� � EditorGetString.StringText).
	*/

  if(!lpwszUnicodeString || !nLength)
    return NULL;

  char *lpResult = (char*)xf_malloc (nLength+1);

  memset (lpResult, 0, nLength+1);

  WideCharToMultiByte (
          CP_OEMCP,
          0,
          lpwszUnicodeString,
          nLength,
          lpResult,
          nLength,
          NULL,
          NULL
          );

  return lpResult;
}

wchar_t **ArrayAnsiToUnicode (char ** lpaszAnsiString, int iCount)
{
	wchar_t** lpaResult = NULL;

	if (lpaszAnsiString)
	{
		lpaResult = (wchar_t**) xf_malloc((iCount+1)*sizeof(wchar_t*));
		if (lpaResult)
		{
			for (int i=0;i<iCount;i++)
			{
			  lpaResult[i]=(lpaszAnsiString[i])?AnsiToUnicode(lpaszAnsiString[i]):NULL;
			}
			lpaResult[iCount] = (wchar_t*)(LONG_PTR) 1; //Array end mark
		}
	}

  return lpaResult;
}

void FreeArrayUnicode (wchar_t ** lpawszUnicodeString)
{
	if (lpawszUnicodeString)
	{
		for (int i=0;(LONG_PTR)lpawszUnicodeString[i] != 1;i++) //Until end mark
		{
			if (lpawszUnicodeString[i]) xf_free(lpawszUnicodeString[i]);
		}
		xf_free(lpawszUnicodeString);
	}
}

DWORD OldKeyToKey (DWORD dOldKey)
{
	if (dOldKey&0x100) dOldKey=(dOldKey^0x100)|EXTENDED_KEY_BASE;
		else if (dOldKey&0x200) dOldKey=(dOldKey^0x200)|INTERNAL_KEY_BASE;
	return dOldKey;
}

DWORD KeyToOldKey (DWORD dKey)
{
	if (dKey&EXTENDED_KEY_BASE) dKey=(dKey^EXTENDED_KEY_BASE)|0x100;
		else if (dKey&INTERNAL_KEY_BASE) dKey=(dKey^INTERNAL_KEY_BASE)|0x200;
	return dKey;
}


void ConvertInfoPanelLinesA(const oldfar::InfoPanelLine *iplA, InfoPanelLine **piplW, int iCount)
{
	if (iplA && piplW && (iCount>0))
	{
		InfoPanelLine *iplW = (InfoPanelLine *) xf_malloc(iCount*sizeof(InfoPanelLine));
		if (iplW)
		{
			for (int i=0;i<iCount;i++)
			{
				AnsiToUnicodeBin(iplA[i].Text,iplW[i].Text,80); //BUGBUG
				AnsiToUnicodeBin(iplA[i].Data,iplW[i].Data,80); //BUGBUG
				iplW[i].Separator=iplA[i].Separator;
			}
		}
		*piplW = iplW;
	}
}

void FreeInfoPanelLinesW(InfoPanelLine *iplW)
{
	if (iplW)	xf_free((void*)iplW);
}

void ConvertPanelModesA(const oldfar::PanelMode *pnmA, PanelMode **ppnmW, int iCount)
{
	if (pnmA && ppnmW && (iCount>0))
	{
		PanelMode *pnmW = (PanelMode *) xf_malloc(iCount*sizeof(PanelMode));
		if (pnmW)
		{
			memset(pnmW,0,iCount*sizeof(PanelMode));
			for (int i=0;i<iCount;i++)
			{
				int iColumnCount = 0;

				if (pnmA[i].ColumnTypes)
				{
					char *lpTypes = strdup(pnmA[i].ColumnTypes);

					const char *lpToken = strtok(lpTypes, ",");

					while ( lpToken && *lpToken )
					{
						iColumnCount++;

						lpToken = strtok(NULL, ",");
					}

					xf_free (lpTypes);
				}

				pnmW[i].ColumnTypes		=	(pnmA[i].ColumnTypes)?AnsiToUnicode(pnmA[i].ColumnTypes):NULL;
				pnmW[i].ColumnWidths	=	(pnmA[i].ColumnWidths)?AnsiToUnicode(pnmA[i].ColumnWidths):NULL;

				pnmW[i].ColumnTitles	= (pnmA[i].ColumnTitles && (iColumnCount>0))?ArrayAnsiToUnicode(pnmA[i].ColumnTitles,iColumnCount):NULL;

				pnmW[i].FullScreen			= pnmA[i].FullScreen;
				pnmW[i].DetailedStatus	= pnmA[i].DetailedStatus;
				pnmW[i].AlignExtensions	= pnmA[i].AlignExtensions;
				pnmW[i].CaseConversion	= pnmA[i].CaseConversion;
				pnmW[i].StatusColumnTypes		=	(pnmA[i].StatusColumnTypes)?AnsiToUnicode(pnmA[i].StatusColumnTypes):NULL;
				pnmW[i].StatusColumnWidths	=	(pnmA[i].StatusColumnWidths)?AnsiToUnicode(pnmA[i].StatusColumnWidths):NULL;
			}
		}
		*ppnmW = pnmW;
	}
}

void FreePanelModesW(PanelMode *pnmW, int iCount)
{
	if (pnmW)
	{
		for (int i=0;i<iCount;i++)
		{
			if (pnmW[i].ColumnTypes) xf_free(pnmW[i].ColumnTypes);
			if (pnmW[i].ColumnWidths) xf_free(pnmW[i].ColumnWidths);
			if (pnmW[i].ColumnTitles)	FreeArrayUnicode(pnmW[i].ColumnTitles);
			if (pnmW[i].StatusColumnTypes) xf_free(pnmW[i].StatusColumnTypes);
			if (pnmW[i].StatusColumnWidths) xf_free(pnmW[i].StatusColumnWidths);
		}
		xf_free((void*)pnmW);
	}
}

void ConvertKeyBarTitlesA(const oldfar::KeyBarTitles *kbtA, KeyBarTitles *kbtW, bool FullStruct=true)
{
	if (kbtA && kbtW)
	{
		for(int i=0;i<12;i++)
		{
			kbtW->Titles[i]          = kbtA->Titles[i]?          AnsiToUnicode(kbtA->Titles[i]):          NULL;
			kbtW->CtrlTitles[i]      = kbtA->CtrlTitles[i]?      AnsiToUnicode(kbtA->CtrlTitles[i]):      NULL;
			kbtW->AltTitles[i]       = kbtA->AltTitles[i]?       AnsiToUnicode(kbtA->AltTitles[i]):       NULL;
			kbtW->ShiftTitles[i]     = kbtA->ShiftTitles[i]?     AnsiToUnicode(kbtA->ShiftTitles[i]):     NULL;
			kbtW->CtrlShiftTitles[i] = FullStruct && kbtA->CtrlShiftTitles[i]? AnsiToUnicode(kbtA->CtrlShiftTitles[i]): NULL;
			kbtW->AltShiftTitles[i]  = FullStruct && kbtA->AltShiftTitles[i]?  AnsiToUnicode(kbtA->AltShiftTitles[i]):  NULL;
			kbtW->CtrlAltTitles[i]   = FullStruct && kbtA->CtrlAltTitles[i]?   AnsiToUnicode(kbtA->CtrlAltTitles[i]):   NULL;
		}
	}
}

void FreeKeyBarTitlesW(KeyBarTitles *kbtW)
{
	if (kbtW)
	{
		for(int i=0;i<12;i++)
		{
			if (kbtW->Titles[i])					xf_free (kbtW->Titles[i]);
			if (kbtW->CtrlTitles[i])			xf_free (kbtW->CtrlTitles[i]);
			if (kbtW->AltTitles[i])				xf_free (kbtW->AltTitles[i]);
			if (kbtW->ShiftTitles[i])			xf_free (kbtW->ShiftTitles[i]);
			if (kbtW->CtrlShiftTitles[i])	xf_free (kbtW->CtrlShiftTitles[i]);
			if (kbtW->AltShiftTitles[i])	xf_free (kbtW->AltShiftTitles[i]);
			if (kbtW->CtrlAltTitles[i])		xf_free (kbtW->CtrlAltTitles[i]);
		}
	}
}

void ConvertPanelItemA(const oldfar::PluginPanelItem *PanelItemA, PluginPanelItem **PanelItemW, int ItemsNumber)
{
	*PanelItemW = (PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(PluginPanelItem));

	memset(*PanelItemW,0,ItemsNumber*sizeof(PluginPanelItem));

	for (int i=0; i<ItemsNumber; i++)
	{
		(*PanelItemW)[i].Flags = PanelItemA[i].Flags;
		(*PanelItemW)[i].NumberOfLinks = PanelItemA[i].NumberOfLinks;

		if (PanelItemA[i].Description)
			(*PanelItemW)[i].Description = AnsiToUnicode(PanelItemA[i].Description);

		if (PanelItemA[i].Owner)
			(*PanelItemW)[i].Owner = AnsiToUnicode(PanelItemA[i].Owner);

		if (PanelItemA[i].CustomColumnNumber)
		{
			(*PanelItemW)[i].CustomColumnNumber = PanelItemA[i].CustomColumnNumber;
			(*PanelItemW)[i].CustomColumnData = ArrayAnsiToUnicode(PanelItemA[i].CustomColumnData,PanelItemA[i].CustomColumnNumber);
		}

		(*PanelItemW)[i].UserData = PanelItemA[i].UserData;
		(*PanelItemW)[i].CRC32 = PanelItemA[i].CRC32;

		(*PanelItemW)[i].FindData.dwFileAttributes = PanelItemA[i].FindData.dwFileAttributes;
		(*PanelItemW)[i].FindData.ftCreationTime = PanelItemA[i].FindData.ftCreationTime;
		(*PanelItemW)[i].FindData.ftLastAccessTime = PanelItemA[i].FindData.ftLastAccessTime;
		(*PanelItemW)[i].FindData.ftLastWriteTime = PanelItemA[i].FindData.ftLastWriteTime;
		(*PanelItemW)[i].FindData.nFileSize = (unsigned __int64)PanelItemA[i].FindData.nFileSizeLow + (((unsigned __int64)PanelItemA[i].FindData.nFileSizeHigh)<<32);
		(*PanelItemW)[i].FindData.nPackSize = (unsigned __int64)PanelItemA[i].PackSize + (((unsigned __int64)PanelItemA[i].PackSizeHigh)<<32);
		(*PanelItemW)[i].FindData.lpwszFileName = AnsiToUnicode(PanelItemA[i].FindData.cFileName);
		(*PanelItemW)[i].FindData.lpwszAlternateFileName = AnsiToUnicode(PanelItemA[i].FindData.cAlternateFileName);
	}
}

void ConvertPanelItemToAnsi(const PluginPanelItem &PanelItem, oldfar::PluginPanelItem &PanelItemA)
{
	PanelItemA.Flags = PanelItem.Flags;
	PanelItemA.NumberOfLinks=PanelItem.NumberOfLinks;

	if(PanelItem.Description)
		PanelItemA.Description=UnicodeToAnsi(PanelItem.Description);

	if(PanelItem.Owner)
		PanelItemA.Owner=UnicodeToAnsi(PanelItem.Owner);

	if (PanelItem.CustomColumnNumber)
	{
		PanelItemA.CustomColumnNumber=PanelItem.CustomColumnNumber;
		PanelItemA.CustomColumnData=(char **)xf_malloc(PanelItem.CustomColumnNumber*sizeof(char *));

		for (int j=0; j<PanelItem.CustomColumnNumber; j++)
			PanelItemA.CustomColumnData[j] = UnicodeToAnsi(PanelItem.CustomColumnData[j]);
	}

	PanelItemA.UserData = PanelItem.UserData;
	PanelItemA.CRC32 = PanelItem.CRC32;

	PanelItemA.FindData.dwFileAttributes = PanelItem.FindData.dwFileAttributes;
	PanelItemA.FindData.ftCreationTime = PanelItem.FindData.ftCreationTime;
	PanelItemA.FindData.ftLastAccessTime = PanelItem.FindData.ftLastAccessTime;
	PanelItemA.FindData.ftLastWriteTime = PanelItem.FindData.ftLastWriteTime;
	PanelItemA.FindData.nFileSizeLow = (DWORD)PanelItem.FindData.nFileSize;
	PanelItemA.FindData.nFileSizeHigh = (DWORD)(PanelItem.FindData.nFileSize>>32);
	PanelItemA.PackSize = (DWORD)PanelItem.FindData.nPackSize;
	PanelItemA.PackSizeHigh = (DWORD)(PanelItem.FindData.nPackSize>>32);
	UnicodeToAnsi(PanelItem.FindData.lpwszFileName,PanelItemA.FindData.cFileName,sizeof(PanelItemA.FindData.cFileName));
	UnicodeToAnsi(PanelItem.FindData.lpwszAlternateFileName,PanelItemA.FindData.cAlternateFileName,sizeof(PanelItemA.FindData.cAlternateFileName));
}


void ConvertPanelItemsArrayToAnsi(const PluginPanelItem *PanelItemW, oldfar::PluginPanelItem *&PanelItemA, int ItemsNumber)
{
	PanelItemA = (oldfar::PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem));
	memset(PanelItemA,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

	for (int i=0; i<ItemsNumber; i++)
	{
		ConvertPanelItemToAnsi(PanelItemW[i],PanelItemA[i]);
	}
}

void ConvertPanelItemsPtrArrayToAnsi(PluginPanelItem **PanelItemW, oldfar::PluginPanelItem *&PanelItemA, int ItemsNumber)
{
	PanelItemA = (oldfar::PluginPanelItem *)xf_malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem));
	memset(PanelItemA,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

	for (int i=0; i<ItemsNumber; i++)
	{
		ConvertPanelItemToAnsi(*PanelItemW[i],PanelItemA[i]);
	}
}

void FreePanelItemW(PluginPanelItem *PanelItem, int ItemsNumber)
{
	for (int i=0; i<ItemsNumber; i++)
	{
		if (PanelItem[i].Description)
			xf_free(PanelItem[i].Description);

		if (PanelItem[i].Owner)
			xf_free(PanelItem[i].Owner);

		if (PanelItem[i].CustomColumnNumber)
		{
			for (int j=0; j<PanelItem[i].CustomColumnNumber; j++)
				xf_free(PanelItem[i].CustomColumnData[j]);

			xf_free(PanelItem[i].CustomColumnData);
		}
		apiFreeFindData(&PanelItem[i].FindData);
	}

	xf_free(PanelItem);
}

void FreePanelItemA(oldfar::PluginPanelItem *PanelItem, int ItemsNumber)
{
	for (int i=0; i<ItemsNumber; i++)
	{
		if (PanelItem[i].Description)
			xf_free(PanelItem[i].Description);

		if (PanelItem[i].Owner)
			xf_free(PanelItem[i].Owner);

		if (PanelItem[i].CustomColumnNumber)
		{
			for (int j=0; j<PanelItem[i].CustomColumnNumber; j++)
				xf_free(PanelItem[i].CustomColumnData[j]);

			xf_free(PanelItem[i].CustomColumnData);
		}
	}

	xf_free(PanelItem);
}

char *WINAPI FarItoaA(int value, char *string, int radix)
{
  if(string)
    return itoa(value,string,radix);
  return NULL;
}

char *WINAPI FarItoa64A(__int64 value, char *string, int radix)
{
  if(string)
    return _i64toa(value, string, radix);
  return NULL;
}

int WINAPI FarAtoiA(const char *s)
{
  if(s)
    return atoi(s);
  return 0;
}

__int64 WINAPI FarAtoi64A(const char *s)
{
  if(s)
    return _atoi64(s);
  return _i64(0);
}

char* WINAPI PointToNameA(char *Path)
{
  if(!Path)
    return NULL;

  char *NamePtr=Path;
  while (*Path)
  {
    if (IsSlashA(*Path) || (*Path==':' && Path==NamePtr+1))
      NamePtr=Path+1;
    Path++;
  }
  return(NamePtr);
}

void WINAPI UnquoteA(char *Str)
{
  if (!Str)
    return;
  char *Dst=Str;
  while (*Str)
  {
    if (*Str!='\"')
      *Dst++=*Str;
    Str++;
  }
  *Dst=0;
}

char* WINAPI RemoveLeadingSpacesA(char *Str)
{
  char *ChPtr;
  if((ChPtr=Str) == 0)
    return NULL;

  for (; IsSpaceA(*ChPtr); ChPtr++)
         ;
  if (ChPtr!=Str)
    memmove(Str,ChPtr,strlen(ChPtr)+1);
  return Str;
}

/*
char* WINAPI RemoveTrailingSpacesA(char *Str)
{
  if(!Str)
    return NULL;
  if (*Str == '\0')
    return Str;

  char *ChPtr;
  int I;

  for (ChPtr=Str+(I=(int)strlen((char *)Str)-1); I >= 0; I--, ChPtr--)
    if (IsSpaceA(*ChPtr) || IsEolA(*ChPtr))
      *ChPtr=0;
    else
      break;

  return Str;
}
*/

char* WINAPI RemoveExternalSpacesA(char *Str)
{
  return RemoveTrailingSpacesA(RemoveLeadingSpacesA(Str));
}

char* WINAPI TruncStrA(char *Str,int MaxLength)
{
  if(Str)
  {
    int Length;
    if (MaxLength<0)
      MaxLength=0;
    if ((Length=(int)strlen(Str))>MaxLength)
    {
      if (MaxLength>3)
      {
        char *MovePos = Str+Length-MaxLength+3;
        memmove(Str+3,MovePos,strlen(MovePos)+1);
        memcpy(Str,"...",3);
      }
      Str[MaxLength]=0;
    }
  }
  return(Str);
}

char* WINAPI TruncPathStrA(char *Str, int MaxLength)
{
  if (Str)
  {
    int nLength = (int)strlen (Str);

    if (nLength > MaxLength)
    {
      char *lpStart = NULL;

      if ( *Str && (Str[1] == ':') && (Str[2] == '\\') )
         lpStart = Str+3;
      else
      {
        if ( (Str[0] == '\\') && (Str[1] == '\\') )
        {
          if ( (lpStart = strchr (Str+2, '\\')) != NULL )
            if ( (lpStart = strchr (lpStart+1, '\\')) != NULL )
              lpStart++;
        }
      }

      if ( !lpStart || (lpStart-Str > MaxLength-5) )
        return TruncStrA (Str, MaxLength);

      char *lpInPos = lpStart+3+(nLength-MaxLength);
      memmove (lpStart+3, lpInPos, strlen (lpInPos)+1);
      memcpy (lpStart, "...", 3);
    }
  }

  return Str;
}

char *InsertQuoteA(char *Str)
{
  size_t l = strlen(Str);
  if(*Str != '"')
  {
    memmove(Str+1,Str,++l);
    *Str='"';
  }
  if(Str[l-1] != '"')
  {
    Str[l++] = '\"';
    Str[l] = 0;
  }
  return Str;
}

char* WINAPI QuoteSpaceOnlyA(char *Str)
{
  if (Str && strchr(Str,' ')!=NULL)
    InsertQuoteA(Str);
  return(Str);
}

BOOL AddEndSlashA(char *Path,char TypeSlash)
{
  BOOL Result=FALSE;
  if(Path)
  {
    /* $ 06.12.2000 IS
      ! ������ ������� �������� � ������ ������ ������, ����� ����������
        ��������� ��� ������������� ��������� ����� �� �����, �������
        ����������� ����.
    */
    char *end;
    int Slash=0, BackSlash=0;
    if(!TypeSlash)
    {
      end=Path;
      while(*end)
      {
       Slash+=(*end=='\\');
       BackSlash+=(*end=='/');
       end++;
      }
    }
    else
    {
      end=Path+strlen(Path);
      if(TypeSlash == '\\')
        Slash=1;
      else
        BackSlash=1;
    }
    int Length=(int)(end-Path);
    char c=(Slash<BackSlash)?'/':'\\';
    Result=TRUE;
    if (Length==0)
    {
       *end=c;
       end[1]=0;
    }
    else
    {
     end--;
     if (!IsSlashA(*end))
     {
       end[1]=c;
       end[2]=0;
     }
     else
       *end=c;
    }
    /* IS $ */
  }
  return Result;
}

BOOL WINAPI AddEndSlashA(char *Path)
{
  return AddEndSlashA(Path,0);
}

void WINAPI GetPathRootA(const char *Path, char *Root)
{
	string strPath(Path), strRoot;

	GetPathRoot(strPath,strRoot);

	strRoot.GetCharString(Root,strRoot.GetLength());
}

int WINAPI CopyToClipboardA(const char *Data)
{
	wchar_t *p = Data!=NULL?AnsiToUnicode(Data):NULL;
	int ret = CopyToClipboard(p);
	if (p) xf_free(p);
	return ret;
}

char* WINAPI PasteFromClipboardA(void)
{
	wchar_t *p = PasteFromClipboard();
	if (p)
		return UnicodeToAnsi(p);
	return NULL;
}

void WINAPI DeleteBufferA(void *Buffer)
{
	if(Buffer) xf_free(Buffer);
}

int WINAPI ProcessNameA(const char *Param1,char *Param2,DWORD Flags)
{
	string strP1(Param1), strP2(Param2);
	int size = (int)(strP1.GetLength()+strP2.GetLength()+NM)+1; //� ���� ��� ��� ������� ����� ��� ���� Param2
	wchar_t *p=(wchar_t *)xf_malloc(size*sizeof(wchar_t));
	wcscpy(p,strP2);
	int newFlags = 0;
	if(Flags&oldfar::PN_SKIPPATH)
	{
		newFlags|=PN_SKIPPATH;
		Flags &= ~oldfar::PN_SKIPPATH;
	}
	if(Flags == oldfar::PN_CMPNAME)
	{
		newFlags|=PN_CMPNAME;
	}
	else if(Flags == oldfar::PN_CMPNAMELIST)
	{
		newFlags|=PN_CMPNAMELIST;
	}
	else if(Flags&oldfar::PN_GENERATENAME)
	{
		newFlags|=PN_GENERATENAME|(Flags&0xFF);
	}
	int ret = ProcessName(strP1,p,size,newFlags);
	UnicodeToAnsi(p,Param2);
	xf_free(p);
	return ret;
}

int WINAPI KeyNameToKeyA(const char *Name)
{
	string strN(Name);
	return KeyToOldKey(KeyNameToKey(strN));
}

BOOL WINAPI FarKeyToNameA(int Key,char *KeyText,int Size)
{
	string strKT;
	int ret=KeyToText(OldKeyToKey(Key),strKT);
	if (ret)
		strKT.GetCharString(KeyText,Size>0?Size:32);
	return ret;
}

char* WINAPI FarMkTempA(char *Dest, const char *Prefix)
{
	string strP((Prefix?Prefix:""));
	wchar_t D[NM] = {0};

	FarMkTemp(D,countof(D),strP);

	UnicodeToAnsi(D,Dest);
	return Dest;
}

int WINAPI FarMkLinkA(const char *Src,const char *Dest, DWORD Flags)
{
	string s(Src), d(Dest);

	int flg=0;
	switch(Flags&0xf)
	{
		case oldfar::FLINK_HARDLINK:    flg = FLINK_HARDLINK; break;
		case oldfar::FLINK_JUNCTION:    flg = FLINK_JUNCTION; break;
		case oldfar::FLINK_VOLMOUNT:    flg = FLINK_VOLMOUNT; break;
		case oldfar::FLINK_SYMLINKFILE: flg = FLINK_SYMLINKFILE; break;
		case oldfar::FLINK_SYMLINKDIR:  flg = FLINK_SYMLINKDIR; break;
	}
	if (Flags&oldfar::FLINK_SHOWERRMSG)       flg|=FLINK_SHOWERRMSG;
	if (Flags&oldfar::FLINK_DONOTUPDATEPANEL) flg|=FLINK_DONOTUPDATEPANEL;

	return FarMkLink(s, d, flg);
}

int WINAPI GetNumberOfLinksA(const char *Name)
{
	string n(Name);
	return GetNumberOfLinks(n);
}

int WINAPI ConvertNameToRealA(const char *Src,char *Dest,int DestSize)
{
	string strSrc(Src),strDest;
	OldConvertNameToReal(strSrc,strDest);
	if(!Dest)
		return (int)strDest.GetLength();
	else
		strDest.GetCharString(Dest,DestSize);
	return Min((int)strDest.GetLength(),DestSize);
}

typedef struct _FAR_SEARCH_A_CALLBACK_PARAM
{
	oldfar::FRSUSERFUNC Func;
	void *Param;
}FAR_SEARCH_A_CALLBACK_PARAM, *PFAR_SEARCH_A_CALLBACK_PARAM;

static int WINAPI FarRecursiveSearchA_Callback(const FAR_FIND_DATA *FData,const wchar_t *FullName,void *param)
{
	PFAR_SEARCH_A_CALLBACK_PARAM pCallbackParam = static_cast<PFAR_SEARCH_A_CALLBACK_PARAM>(param);

	WIN32_FIND_DATAA FindData;
	memset(&FindData,0,sizeof(FindData));
	FindData.dwFileAttributes = FData->dwFileAttributes;
	FindData.ftCreationTime = FData->ftCreationTime;
	FindData.ftLastAccessTime = FData->ftLastAccessTime;
	FindData.ftLastWriteTime = FData->ftLastWriteTime;
	FindData.nFileSizeLow = (DWORD)FData->nFileSize;
	FindData.nFileSizeHigh = (DWORD)(FData->nFileSize>>32);
	UnicodeToAnsi(FData->lpwszFileName,FindData.cFileName,sizeof(FindData.cFileName));
	UnicodeToAnsi(FData->lpwszAlternateFileName,FindData.cAlternateFileName,sizeof(FindData.cAlternateFileName));

	char FullNameA[NM];
	UnicodeToAnsi(FullName,FullNameA,sizeof(FullNameA));

	return pCallbackParam->Func(&FindData,FullNameA,pCallbackParam->Param);
}

void WINAPI FarRecursiveSearchA(const char *InitDir,const char *Mask,oldfar::FRSUSERFUNC Func,DWORD Flags,void *Param)
{
	string strInitDir(InitDir);
	string strMask(Mask);

	FAR_SEARCH_A_CALLBACK_PARAM CallbackParam;
	CallbackParam.Func = Func;
	CallbackParam.Param = Param;
	int newFlags = 0;
	if(Flags&oldfar::FRS_RETUPDIR) newFlags|=FRS_RETUPDIR;
	if(Flags&oldfar::FRS_RECUR) newFlags|=FRS_RECUR;
	if(Flags&oldfar::FRS_SCANSYMLINK) newFlags|=FRS_SCANSYMLINK;
	FarRecursiveSearch(static_cast<const wchar_t *>(strInitDir),static_cast<const wchar_t *>(strMask),FarRecursiveSearchA_Callback,newFlags,static_cast<void *>(&CallbackParam));
}

DWORD WINAPI ExpandEnvironmentStrA(const char *src, char *dest, size_t size)
{
	string strS(src), strD;

	apiExpandEnvironmentStrings(strS,strD);
	DWORD len = (DWORD)Min(strD.GetLength(),size-1);

	strD.GetCharString(dest,len+1);
	dest[len]=0;
	return len;
}

int WINAPI FarViewerA(const char *FileName,const char *Title,int X1,int Y1,int X2,int Y2,DWORD Flags)
{
	string strFN(FileName), strT(Title);
	return FarViewer(strFN,strT,X1,Y1,X2,Y2,Flags);
}

int WINAPI FarEditorA(const char *FileName,const char *Title,int X1,int Y1,int X2,int Y2,DWORD Flags,int StartLine,int StartChar)
{
	string strFN(FileName), strT(Title);
	return FarEditor(strFN,strT,X1,Y1,X2,Y2,Flags,StartLine,StartChar);
}

int WINAPI FarCmpNameA(const char *pattern,const char *str,int skippath)
{
	string strP(pattern), strS(str);
	return FarCmpName(strP,strS,skippath);
}

void WINAPI FarTextA(int X,int Y,int Color,const char *Str)
{
	if (!Str) return FarText(X,Y,Color,NULL);
	string strS(Str);
	return FarText(X,Y,Color,strS);
}

BOOL WINAPI FarShowHelpA(const char *ModuleName,const char *HelpTopic,DWORD Flags)
{
	string strMN((ModuleName?ModuleName:"")), strHT((HelpTopic?HelpTopic:""));
	return FarShowHelp(strMN,(HelpTopic?(const wchar_t *)strHT:NULL),Flags);
}

int WINAPI FarInputBoxA(const char *Title,const char *Prompt,const char *HistoryName,const char *SrcText,char *DestText,int DestLength,const char *HelpTopic,DWORD Flags)
{
	string strT((Title?Title:"")), strP((Prompt?Prompt:"")), strHN((HistoryName?HistoryName:"")), strST((SrcText?SrcText:"")), strD, strHT((HelpTopic?HelpTopic:""));
	wchar_t *D = strD.GetBuffer(DestLength);
	int ret = FarInputBox((Title?(const wchar_t *)strT:NULL),(Prompt?(const wchar_t *)strP:NULL),(HistoryName?(const wchar_t *)strHN:NULL),(SrcText?(const wchar_t *)strST:NULL),D,DestLength,(HelpTopic?(const wchar_t *)strHT:NULL),Flags);
	strD.ReleaseBuffer();
	if (ret && DestText)
		strD.GetCharString(DestText,DestLength);
	return ret;
}

int WINAPI FarMessageFnA(INT_PTR PluginNumber,DWORD Flags,const char *HelpTopic,const char * const *Items,int ItemsNumber,int ButtonsNumber)
{
	string strHT((HelpTopic?HelpTopic:""));
	wchar_t **p;
	int c=0;

	if (Flags&FMSG_ALLINONE)
	{
		p = (wchar_t **)AnsiToUnicode((const char *)Items,false);
	}
	else
	{
		c = ItemsNumber;
		p = (wchar_t **)xf_malloc(c*sizeof(wchar_t*));
		for (int i=0; i<c; i++)
			p[i] = AnsiToUnicode(Items[i],false);
	}

	int ret = FarMessageFn(PluginNumber,Flags,(HelpTopic?(const wchar_t *)strHT:NULL),p,ItemsNumber,ButtonsNumber);

	for (int i=0; i<c; i++)
		xf_free(p[i]);
	xf_free(p);

	return ret;
}

const char * WINAPI FarGetMsgFnA(INT_PTR PluginHandle,int MsgId)
{
	//BUGBUG, ���� ���������, ��� PluginHandle - ������

	Plugin *pPlugin = (Plugin*)PluginHandle;

	string strPath = pPlugin->GetModuleName();

	CutToSlash(strPath);

	if ( pPlugin->InitLang(strPath) )
		return pPlugin->GetMsgA(MsgId);

	return "";
}

int WINAPI FarMenuFnA(INT_PTR PluginNumber,int X,int Y,int MaxHeight,DWORD Flags,const char *Title,const char *Bottom,const char *HelpTopic,const int *BreakKeys,int *BreakCode,const struct oldfar::FarMenuItem *Item,int ItemsNumber)
{
	string strT((Title?Title:"")), strB((Bottom?Bottom:"")), strHT((HelpTopic?HelpTopic:""));

	if (!Item || !ItemsNumber)	return -1;

	FarMenuItemEx *mi = (FarMenuItemEx *)xf_malloc(ItemsNumber*sizeof(*mi));

	if (Flags&FMENU_USEEXT)
	{
		oldfar::FarMenuItemEx *p = (oldfar::FarMenuItemEx *)Item;

		for (int i=0; i<ItemsNumber; i++)
		{
			mi[i].Flags = p[i].Flags;
			if(mi[i].Flags&MIF_USETEXTPTR)
			{
				mi[i].Flags&=~MIF_USETEXTPTR;
				mi[i].Flags|=LIF_USETEXTPTR;
			}
			mi[i].Text = AnsiToUnicode(mi[i].Flags&LIF_USETEXTPTR?p[i].Text.TextPtr:p[i].Text.Text);
			mi[i].AccelKey = OldKeyToKey(p[i].AccelKey);
			mi[i].Reserved = p[i].Reserved;
			mi[i].UserData = p[i].UserData;
		}
	}
	else
	{
		for (int i=0; i<ItemsNumber; i++)
		{
			mi[i].Flags=0;
			if (Item[i].Selected)
				mi[i].Flags|=MIF_SELECTED;
			if (Item[i].Checked)
			{
				mi[i].Flags|=MIF_CHECKED;
				if (Item[i].Checked>1)
					AnsiToUnicodeBin((const char*)&Item[i].Checked,(wchar_t*)&mi[i].Flags,1);
			}
			if (Item[i].Separator)
			{
				mi[i].Flags|=MIF_SEPARATOR;
				mi[i].Text = 0;
			}
			else
				mi[i].Text = AnsiToUnicode(Item[i].Text);
			mi[i].AccelKey = 0;
			mi[i].Reserved = 0;
			mi[i].UserData = 0;
		}
	}

	int ret = FarMenuFn(PluginNumber,X,Y,MaxHeight,Flags|FMENU_USEEXT,(Title?(const wchar_t *)strT:NULL),(Bottom?(const wchar_t *)strB:NULL),(HelpTopic?(const wchar_t *)strHT:NULL),BreakKeys,BreakCode,(FarMenuItem *)mi,ItemsNumber);

	for (int i=0; i<ItemsNumber; i++)
		if (mi[i].Text) xf_free((wchar_t *)mi[i].Text);
	if (mi) xf_free(mi);

	return ret;
}

struct DlgData
{
	LONG_PTR DlgProc;
	HANDLE hDlg;
	oldfar::FarDialogItem *diA;
	FarDialogItem *di;
	DlgData* Prev;
}
*DialogData=NULL;

oldfar::FarDialogItem* OneDialogItem=NULL;

DlgData* FindCurrentDlgData(HANDLE hDlg)
{
	DlgData* TmpDialogData=DialogData;
	while(TmpDialogData && TmpDialogData->hDlg!=hDlg)
		TmpDialogData=TmpDialogData->Prev;
	return TmpDialogData;
}

oldfar::FarDialogItem* CurrentDialogItemA(HANDLE hDlg,int ItemNumber)
{
	DlgData* TmpDialogData=FindCurrentDlgData(hDlg);
	if(!TmpDialogData)
		return NULL;
	return &TmpDialogData->diA[ItemNumber];
}

FarDialogItem* CurrentDialogItem(HANDLE hDlg,int ItemNumber)
{
	DlgData* TmpDialogData=FindCurrentDlgData(hDlg);
	if(!TmpDialogData)
		return NULL;
	return &TmpDialogData->di[ItemNumber];
}

LONG_PTR WINAPI CurrentDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	DlgData* TmpDialogData=FindCurrentDlgData(hDlg);
	if(!TmpDialogData)
		return 0;
	FARWINDOWPROC Proc = (FARWINDOWPROC)TmpDialogData->DlgProc;
	return Proc?Proc(TmpDialogData->hDlg, Msg, Param1, Param2):0;
}

void UnicodeListItemToAnsi(FarListItem* li, oldfar::FarListItem* liA)
{
	UnicodeToOEM(li->Text, liA->Text, sizeof(liA->Text)-1);
	liA->Flags=0;
	if(li->Flags&LIF_SELECTED)       liA->Flags|=oldfar::LIF_SELECTED;
	if(li->Flags&LIF_CHECKED)        liA->Flags|=oldfar::LIF_CHECKED;
	if(li->Flags&LIF_SEPARATOR)      liA->Flags|=oldfar::LIF_SEPARATOR;
	if(li->Flags&LIF_DISABLE)        liA->Flags|=oldfar::LIF_DISABLE;
#ifdef FAR_USE_INTERNALS
	if(li->Flags&LIF_GRAYED)         liA->Flags|=oldfar::LIF_GRAYED;
#endif // END FAR_USE_INTERNALS
	if(li->Flags&LIF_DELETEUSERDATA) liA->Flags|=oldfar::LIF_DELETEUSERDATA;
}

int GetAnsiVBufSize(oldfar::FarDialogItem &diA)
{
	return (diA.X2-diA.X1+1)*(diA.Y2-diA.Y1+1);
}

CHAR_INFO *GetAnsiVBufPtr(CHAR_INFO *VBuf, int iSize)
{
	return (VBuf)?(*((CHAR_INFO **)&(VBuf[iSize]))):NULL;
}

void SetAnsiVBufPtr(CHAR_INFO *VBuf, CHAR_INFO *VBufA, int iSize)
{
	if (VBuf) *((CHAR_INFO **)&(VBuf[iSize])) = VBufA;
}

void AnsiVBufToUnicode (CHAR_INFO *VBufA, CHAR_INFO *VBuf, int iSize,bool NoCvt)
{
	if(VBuf && VBufA)
	{
		for(int i=0;i<iSize;i++)
		{
			if(NoCvt)
				VBuf[i].Char.UnicodeChar=VBufA[i].Char.UnicodeChar;
			else
				AnsiToUnicodeBin(&VBufA[i].Char.AsciiChar,&VBuf[i].Char.UnicodeChar,1);
			VBuf[i].Attributes = VBufA[i].Attributes;
		}
	}
}

CHAR_INFO *AnsiVBufToUnicode (oldfar::FarDialogItem &diA)
{
	CHAR_INFO *VBuf = NULL;
	if(diA.Param.VBuf)
	{
		int iSize = GetAnsiVBufSize(diA);
		//+sizeof(CHAR_INFO*) ������ ��� ��� ������ ������� �� ���� vbuf.
		VBuf = (CHAR_INFO*)xf_malloc(iSize*sizeof(CHAR_INFO)+sizeof(CHAR_INFO*));
		if (VBuf)
		{
			AnsiVBufToUnicode(diA.Param.VBuf, VBuf, iSize,(diA.Flags&DIF_NOTCVTUSERCONTROL)==DIF_NOTCVTUSERCONTROL);
			SetAnsiVBufPtr(VBuf, diA.Param.VBuf, iSize);
		}
	}
	return VBuf;
}

void AnsiListItemToUnicode(oldfar::FarListItem* liA, FarListItem* li)
{
	wchar_t* ListItemText=(wchar_t*)xf_malloc(countof(liA->Text)*sizeof(wchar_t));
	OEMToUnicode(liA->Text, ListItemText, sizeof(liA->Text)-1);
	li->Text=ListItemText;
	li->Flags=0;
	if(liA->Flags&oldfar::LIF_SELECTED)       li->Flags|=LIF_SELECTED;
	if(liA->Flags&oldfar::LIF_CHECKED)        li->Flags|=LIF_CHECKED;
	if(liA->Flags&oldfar::LIF_SEPARATOR)      li->Flags|=LIF_SEPARATOR;
	if(liA->Flags&oldfar::LIF_DISABLE)        li->Flags|=LIF_DISABLE;
#ifdef FAR_USE_INTERNALS
	if(liA->Flags&oldfar::LIF_GRAYED)         li->Flags|=LIF_GRAYED;
#endif // END FAR_USE_INTERNALS
	if(liA->Flags&oldfar::LIF_DELETEUSERDATA) li->Flags|=LIF_DELETEUSERDATA;
}

void AnsiDialogItemToUnicodeSafe(oldfar::FarDialogItem &diA, FarDialogItem &di)
{
	switch(diA.Type)
	{
		case oldfar::DI_TEXT:
			di.Type=DI_TEXT;
			break;
		case oldfar::DI_VTEXT:
			di.Type=DI_VTEXT;
			break;
		case oldfar::DI_SINGLEBOX:
			di.Type=DI_SINGLEBOX;
			break;
		case oldfar::DI_DOUBLEBOX:
			di.Type=DI_DOUBLEBOX;
			break;
		case oldfar::DI_EDIT:
			di.Type=DI_EDIT;
			break;
		case oldfar::DI_PSWEDIT:
			di.Type=DI_PSWEDIT;
			break;
		case oldfar::DI_FIXEDIT:
			di.Type=DI_FIXEDIT;
			break;
		case oldfar::DI_BUTTON:
			di.Type=DI_BUTTON;
			di.Param.Selected=diA.Param.Selected;
			break;
		case oldfar::DI_CHECKBOX:
			di.Type=DI_CHECKBOX;
			di.Param.Selected=diA.Param.Selected;
			break;
		case oldfar::DI_RADIOBUTTON:
			di.Type=DI_RADIOBUTTON;
			di.Param.Selected=diA.Param.Selected;
			break;
		case oldfar::DI_COMBOBOX:
			di.Type=DI_COMBOBOX;
			di.Param.ListPos=diA.Param.ListPos;
			break;
		case oldfar::DI_LISTBOX:
			di.Type=DI_LISTBOX;
			di.Param.ListPos=diA.Param.ListPos;
			break;
#ifdef FAR_USE_INTERNALS
		case oldfar::DI_MEMOEDIT:
			di.Type=DI_MEMOEDIT;
			break;
#endif // END FAR_USE_INTERNALS
		case oldfar::DI_USERCONTROL:
			di.Type=DI_USERCONTROL;
			break;
	}
	di.X1=diA.X1;
	di.Y1=diA.Y1;
	di.X2=diA.X2;
	di.Y2=diA.Y2;
	di.Focus=diA.Focus;
	di.Flags=0;
	if(diA.Flags)
	{
		if(diA.Flags&oldfar::DIF_SETCOLOR)
			di.Flags|=DIF_SETCOLOR|(diA.Flags&oldfar::DIF_COLORMASK);
		if(diA.Flags&oldfar::DIF_BOXCOLOR)
			di.Flags|=DIF_BOXCOLOR;
		if(diA.Flags&oldfar::DIF_GROUP)
			di.Flags|=DIF_GROUP;
		if(diA.Flags&oldfar::DIF_LEFTTEXT)
			di.Flags|=DIF_LEFTTEXT;
		if(diA.Flags&oldfar::DIF_MOVESELECT)
			di.Flags|=DIF_MOVESELECT;
		if(diA.Flags&oldfar::DIF_SHOWAMPERSAND)
			di.Flags|=DIF_SHOWAMPERSAND;
		if(diA.Flags&oldfar::DIF_CENTERGROUP)
			di.Flags|=DIF_CENTERGROUP;
		if(diA.Flags&oldfar::DIF_NOBRACKETS)
			di.Flags|=DIF_NOBRACKETS;
		if(diA.Flags&oldfar::DIF_MANUALADDHISTORY)
			di.Flags|=DIF_MANUALADDHISTORY;
		if(diA.Flags&oldfar::DIF_SEPARATOR)
			di.Flags|=DIF_SEPARATOR;
		if(diA.Flags&oldfar::DIF_SEPARATOR2)
			di.Flags|=DIF_SEPARATOR2;
		if(diA.Flags&oldfar::DIF_EDITOR)
			di.Flags|=DIF_EDITOR;
		if(diA.Flags&oldfar::DIF_LISTNOAMPERSAND)
			di.Flags|=DIF_LISTNOAMPERSAND;
		if(diA.Flags&oldfar::DIF_LISTNOBOX)
			di.Flags|=DIF_LISTNOBOX;
		if(diA.Flags&oldfar::DIF_HISTORY)
			di.Flags|=DIF_HISTORY;
		if(diA.Flags&oldfar::DIF_BTNNOCLOSE)
			di.Flags|=DIF_BTNNOCLOSE;
		if(diA.Flags&oldfar::DIF_CENTERTEXT)
			di.Flags|=DIF_CENTERTEXT;
		if(diA.Flags&oldfar::DIF_NOTCVTUSERCONTROL)
			di.Flags|=DIF_NOTCVTUSERCONTROL;
#ifdef FAR_USE_INTERNALS
		if(diA.Flags&oldfar::DIF_SEPARATORUSER)
			di.Flags|=DIF_SEPARATORUSER;
#endif // END FAR_USE_INTERNALS
		if(diA.Flags&oldfar::DIF_EDITEXPAND)
			di.Flags|=DIF_EDITEXPAND;
		if(diA.Flags&oldfar::DIF_DROPDOWNLIST)
			di.Flags|=DIF_DROPDOWNLIST;
		if(diA.Flags&oldfar::DIF_USELASTHISTORY)
			di.Flags|=DIF_USELASTHISTORY;
		if(diA.Flags&oldfar::DIF_MASKEDIT)
			di.Flags|=DIF_MASKEDIT;
		if(diA.Flags&oldfar::DIF_SELECTONENTRY)
			di.Flags|=DIF_SELECTONENTRY;
		if(diA.Flags&oldfar::DIF_3STATE)
			di.Flags|=DIF_3STATE;
#ifdef FAR_USE_INTERNALS
		if(diA.Flags&oldfar::DIF_EDITPATH)
			di.Flags|=DIF_EDITPATH;
#endif // END FAR_USE_INTERNALS
		if(diA.Flags&oldfar::DIF_LISTWRAPMODE)
			di.Flags|=DIF_LISTWRAPMODE;
		if(diA.Flags&oldfar::DIF_LISTAUTOHIGHLIGHT)
			di.Flags|=DIF_LISTAUTOHIGHLIGHT;
#ifdef FAR_USE_INTERNALS
		if(diA.Flags&oldfar::DIF_AUTOMATION)
			di.Flags|=DIF_AUTOMATION;
#endif // END FAR_USE_INTERNALS
		if(diA.Flags&oldfar::DIF_HIDDEN)
			di.Flags|=DIF_HIDDEN;
		if(diA.Flags&oldfar::DIF_READONLY)
			di.Flags|=DIF_READONLY;
		if(diA.Flags&oldfar::DIF_NOFOCUS)
			di.Flags|=DIF_NOFOCUS;
		if(diA.Flags&oldfar::DIF_DISABLE)
			di.Flags|=DIF_DISABLE;
	}
	di.DefaultButton=diA.DefaultButton;
}

void AnsiDialogItemToUnicode(oldfar::FarDialogItem &diA, FarDialogItem &di)
{
	memset(&di,0,sizeof(FarDialogItem));
	AnsiDialogItemToUnicodeSafe(diA,di);
	switch(di.Type)
	{
		case DI_LISTBOX:
		case DI_COMBOBOX:
		{
			if (diA.Param.ListItems && !IsBadReadPtr(diA.Param.ListItems,sizeof(oldfar::FarList)))
			{
				di.Param.ListItems=(FarList *)xf_malloc(sizeof(FarList));
				di.Param.ListItems->Items = (FarListItem *)xf_malloc(diA.Param.ListItems->ItemsNumber*sizeof(FarListItem));
				di.Param.ListItems->ItemsNumber = diA.Param.ListItems->ItemsNumber;
				for(int j=0;j<di.Param.ListItems->ItemsNumber;j++)
					AnsiListItemToUnicode(&diA.Param.ListItems->Items[j], &di.Param.ListItems->Items[j]);
			}
			break;
		}
		case DI_USERCONTROL:
			di.Param.VBuf = AnsiVBufToUnicode(diA);
			break;
		case DI_EDIT:
		case DI_FIXEDIT:
		{
			if (diA.Flags&oldfar::DIF_HISTORY && diA.Param.History)
				di.Param.History=AnsiToUnicode(diA.Param.History);
			else if (diA.Flags&oldfar::DIF_MASKEDIT && diA.Param.Mask)
				di.Param.Mask=AnsiToUnicode(diA.Param.Mask);
			break;
		}
	}

	if (diA.Type==oldfar::DI_USERCONTROL)
	{
		di.PtrData = (wchar_t*)xf_malloc(sizeof(diA.Data.Data));
		if (di.PtrData) memcpy((char*)di.PtrData,diA.Data.Data,sizeof(diA.Data.Data));
		di.MaxLen = 0;
	}
	else if ((diA.Type==oldfar::DI_EDIT || diA.Type==oldfar::DI_COMBOBOX) && diA.Flags&oldfar::DIF_VAREDIT)
		di.PtrData = AnsiToUnicode(diA.Data.Ptr.PtrData);
	else
		di.PtrData = AnsiToUnicode(diA.Data.Data);
	//BUGBUG ��� ���� ��������� ��� ������� �����: maxlen=513 �������� � ����� �������� ��� ������ ��� DIF_VAREDIT
	//di->MaxLen = 0;
}

void FreeUnicodeDialogItem(FarDialogItem &di)
{
	switch(di.Type)
	{
		case DI_EDIT:
		case DI_FIXEDIT:
			if((di.Flags&DIF_HISTORY) && di.Param.History)
				xf_free((void *)di.Param.History);
			else if((di.Flags&DIF_MASKEDIT) && di.Param.Mask)
				xf_free((void *)di.Param.Mask);
			break;
		case DI_LISTBOX:
		case DI_COMBOBOX:
			if(di.Param.ListItems && di.Param.ListPos!=-1) //BUGBUG?
			{
				if(di.Param.ListItems->Items)
				{
					for(int i=0;i<di.Param.ListItems->ItemsNumber;i++)
					{
						if(di.Param.ListItems->Items[i].Text)
							xf_free((void *)di.Param.ListItems->Items[i].Text);
					}
					xf_free(di.Param.ListItems->Items);
				}
				xf_free(di.Param.ListItems);
			}
			break;
		case DI_USERCONTROL:
			if(di.Param.VBuf)
				xf_free(di.Param.VBuf);
			break;
	}
	if (di.PtrData)
		xf_free((void *)di.PtrData);
}

void FreeAnsiDialogItem(oldfar::FarDialogItem &diA)
{
	if((diA.Type==oldfar::DI_EDIT || diA.Type==oldfar::DI_FIXEDIT) &&
	   (diA.Flags&oldfar::DIF_HISTORY ||diA.Flags&oldfar::DIF_MASKEDIT) &&
	    diA.Param.History)
		xf_free((void*)diA.Param.History);

	if((diA.Type==oldfar::DI_EDIT || diA.Type==oldfar::DI_COMBOBOX) &&
	    diA.Flags&oldfar::DIF_VAREDIT && diA.Data.Ptr.PtrData)
		xf_free(diA.Data.Ptr.PtrData);

	memset(&diA,0,sizeof(oldfar::FarDialogItem));
}

void UnicodeDialogItemToAnsiSafe(FarDialogItem &di,oldfar::FarDialogItem &diA)
{
	switch(di.Type)
	{
		case DI_TEXT:
			diA.Type=oldfar::DI_TEXT;
			break;
		case DI_VTEXT:
			diA.Type=oldfar::DI_VTEXT;
			break;
		case DI_SINGLEBOX:
			diA.Type=oldfar::DI_SINGLEBOX;
			break;
		case DI_DOUBLEBOX:
			diA.Type=oldfar::DI_DOUBLEBOX;
			break;
		case DI_EDIT:
			diA.Type=oldfar::DI_EDIT;
			break;
		case DI_PSWEDIT:
			diA.Type=oldfar::DI_PSWEDIT;
			break;
		case DI_FIXEDIT:
			diA.Type=oldfar::DI_FIXEDIT;
			break;
		case DI_BUTTON:
			diA.Type=oldfar::DI_BUTTON;
			diA.Param.Selected=di.Param.Selected;
			break;
		case DI_CHECKBOX:
			diA.Type=oldfar::DI_CHECKBOX;
			diA.Param.Selected=di.Param.Selected;
			break;
		case DI_RADIOBUTTON:
			diA.Type=oldfar::DI_RADIOBUTTON;
			diA.Param.Selected=di.Param.Selected;
			break;
		case DI_COMBOBOX:
			diA.Type=oldfar::DI_COMBOBOX;
			diA.Param.ListPos=di.Param.ListPos;
			break;
		case DI_LISTBOX:
			diA.Type=oldfar::DI_LISTBOX;
			diA.Param.ListPos=di.Param.ListPos;
			break;
#ifdef FAR_USE_INTERNALS
		case DI_MEMOEDIT:
			diA.Type=oldfar::DI_MEMOEDIT;
			break;
#endif // END FAR_USE_INTERNALS
		case DI_USERCONTROL:
			diA.Type=oldfar::DI_USERCONTROL;
			break;
	}
	diA.X1=di.X1;
	diA.Y1=di.Y1;
	diA.X2=di.X2;
	diA.Y2=di.Y2;
	diA.Focus=di.Focus;
	diA.Flags=0;
	if(di.Flags)
	{
		if(di.Flags&DIF_SETCOLOR)
			diA.Flags|=oldfar::DIF_SETCOLOR|(di.Flags&DIF_COLORMASK);
		if(di.Flags&DIF_BOXCOLOR)
			diA.Flags|=oldfar::DIF_BOXCOLOR;
		if(di.Flags&DIF_GROUP)
			diA.Flags|=oldfar::DIF_GROUP;
		if(di.Flags&DIF_LEFTTEXT)
			diA.Flags|=oldfar::DIF_LEFTTEXT;
		if(di.Flags&DIF_MOVESELECT)
			diA.Flags|=oldfar::DIF_MOVESELECT;
		if(di.Flags&DIF_SHOWAMPERSAND)
			diA.Flags|=oldfar::DIF_SHOWAMPERSAND;
		if(di.Flags&DIF_CENTERGROUP)
			diA.Flags|=oldfar::DIF_CENTERGROUP;
		if(di.Flags&DIF_NOBRACKETS)
			diA.Flags|=oldfar::DIF_NOBRACKETS;
		if(di.Flags&DIF_MANUALADDHISTORY)
			diA.Flags|=oldfar::DIF_MANUALADDHISTORY;
		if(di.Flags&DIF_SEPARATOR)
			diA.Flags|=oldfar::DIF_SEPARATOR;
		if(di.Flags&DIF_SEPARATOR2)
			diA.Flags|=oldfar::DIF_SEPARATOR2;
		if(di.Flags&DIF_EDITOR)
			diA.Flags|=oldfar::DIF_EDITOR;
		if(di.Flags&DIF_LISTNOAMPERSAND)
			diA.Flags|=oldfar::DIF_LISTNOAMPERSAND;
		if(di.Flags&DIF_LISTNOBOX)
			diA.Flags|=oldfar::DIF_LISTNOBOX;
		if(di.Flags&DIF_HISTORY)
			diA.Flags|=oldfar::DIF_HISTORY;
		if(di.Flags&DIF_BTNNOCLOSE)
			diA.Flags|=oldfar::DIF_BTNNOCLOSE;
		if(di.Flags&DIF_CENTERTEXT)
			diA.Flags|=oldfar::DIF_CENTERTEXT;
		if(di.Flags&DIF_NOTCVTUSERCONTROL)
			diA.Flags|=oldfar::DIF_NOTCVTUSERCONTROL;
#ifdef FAR_USE_INTERNALS
		if(di.Flags&DIF_SEPARATORUSER)
			diA.Flags|=oldfar::DIF_SEPARATORUSER;
#endif // END FAR_USE_INTERNALS
		if(di.Flags&DIF_EDITEXPAND)
			diA.Flags|=oldfar::DIF_EDITEXPAND;
		if(di.Flags&DIF_DROPDOWNLIST)
			diA.Flags|=oldfar::DIF_DROPDOWNLIST;
		if(di.Flags&DIF_USELASTHISTORY)
			diA.Flags|=oldfar::DIF_USELASTHISTORY;
		if(di.Flags&DIF_MASKEDIT)
			diA.Flags|=oldfar::DIF_MASKEDIT;
		if(di.Flags&DIF_SELECTONENTRY)
			diA.Flags|=oldfar::DIF_SELECTONENTRY;
		if(di.Flags&DIF_3STATE)
			diA.Flags|=oldfar::DIF_3STATE;
#ifdef FAR_USE_INTERNALS
		if(di.Flags&DIF_EDITPATH)
			diA.Flags|=oldfar::DIF_EDITPATH;
#endif // END FAR_USE_INTERNALS
		if(di.Flags&DIF_LISTWRAPMODE)
			diA.Flags|=oldfar::DIF_LISTWRAPMODE;
		if(di.Flags&DIF_LISTAUTOHIGHLIGHT)
			diA.Flags|=oldfar::DIF_LISTAUTOHIGHLIGHT;
#ifdef FAR_USE_INTERNALS
		if(di.Flags&DIF_AUTOMATION)
			diA.Flags|=oldfar::DIF_AUTOMATION;
#endif // END FAR_USE_INTERNALS
		if(di.Flags&DIF_HIDDEN)
			diA.Flags|=oldfar::DIF_HIDDEN;
		if(di.Flags&DIF_READONLY)
			diA.Flags|=oldfar::DIF_READONLY;
		if(di.Flags&DIF_NOFOCUS)
			diA.Flags|=oldfar::DIF_NOFOCUS;
		if(di.Flags&DIF_DISABLE)
			diA.Flags|=oldfar::DIF_DISABLE;
	}
	diA.DefaultButton=di.DefaultButton;
}

oldfar::FarDialogItem* UnicodeDialogItemToAnsi(FarDialogItem &di,HANDLE hDlg,int ItemNumber)
{
	oldfar::FarDialogItem *diA=CurrentDialogItemA(hDlg,ItemNumber);
	if(!diA)
	{
		if(OneDialogItem)
			xf_free(OneDialogItem);
		OneDialogItem=(oldfar::FarDialogItem*)xf_malloc(sizeof(oldfar::FarDialogItem));
		memset(OneDialogItem,0,sizeof(oldfar::FarDialogItem));
		diA=OneDialogItem;
	}
	FreeAnsiDialogItem(*diA);
	UnicodeDialogItemToAnsiSafe(di,*diA);
	switch(diA->Type)
	{
		case oldfar::DI_USERCONTROL:
			diA->Param.VBuf=GetAnsiVBufPtr(di.Param.VBuf, GetAnsiVBufSize(*diA));
			break;
		case oldfar::DI_EDIT:
		case oldfar::DI_FIXEDIT:
			{
				if (di.Flags&DIF_HISTORY)
					diA->Param.History=UnicodeToAnsi(di.Param.History);
				else if (di.Flags&DIF_MASKEDIT)
					diA->Param.Mask=UnicodeToAnsi(di.Param.Mask);
			}
			break;
	}
	if (diA->Type==oldfar::DI_USERCONTROL)
	{
		if (di.PtrData) memcpy(diA->Data.Data,(char*)di.PtrData,sizeof(diA->Data.Data));
	}
	else if ((diA->Type==oldfar::DI_EDIT || diA->Type==oldfar::DI_COMBOBOX) && diA->Flags&oldfar::DIF_VAREDIT)
	{
		diA->Data.Ptr.PtrLength=StrLength(di.PtrData);
		diA->Data.Ptr.PtrData=(char*)xf_malloc(diA->Data.Ptr.PtrLength+1);
		UnicodeToAnsi(di.PtrData,diA->Data.Ptr.PtrData,diA->Data.Ptr.PtrLength+1);
	}
	else
		UnicodeToAnsi(di.PtrData,diA->Data.Data,sizeof(diA->Data.Data));
	return diA;
}

LONG_PTR WINAPI DlgProcA(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	static wchar_t* HelpTopic = NULL;
	switch(Msg)
	{
		case DN_LISTHOTKEY:      Msg=oldfar::DN_LISTHOTKEY; break;
		case DN_BTNCLICK:        Msg=oldfar::DN_BTNCLICK; break;
		case DN_CTLCOLORDIALOG:  Msg=oldfar::DN_CTLCOLORDIALOG; break;
		case DN_CTLCOLORDLGITEM: Msg=oldfar::DN_CTLCOLORDLGITEM; break;
		case DN_CTLCOLORDLGLIST: Msg=oldfar::DN_CTLCOLORDLGLIST; break;
		case DN_DRAWDIALOG:      Msg=oldfar::DN_DRAWDIALOG; break;

		case DN_DRAWDLGITEM:
		{
			Msg=oldfar::DN_DRAWDLGITEM;
			FarDialogItem *di = (FarDialogItem *)Param2;
			oldfar::FarDialogItem *FarDiA=UnicodeDialogItemToAnsi(*di,hDlg,Param1);

			LONG_PTR ret = CurrentDlgProc(hDlg, Msg, Param1, (LONG_PTR)FarDiA);

			if (ret && (di->Type==DI_USERCONTROL) && (di->Param.VBuf))
			{
				AnsiVBufToUnicode(FarDiA->Param.VBuf, di->Param.VBuf, GetAnsiVBufSize(*FarDiA),(FarDiA->Flags&DIF_NOTCVTUSERCONTROL)==DIF_NOTCVTUSERCONTROL);
			}
			return ret;
		}

		case DN_EDITCHANGE:
			Msg=oldfar::DN_EDITCHANGE;
			return Param2?CurrentDlgProc(hDlg,Msg,Param1,(LONG_PTR)UnicodeDialogItemToAnsi(*((FarDialogItem *)Param2),hDlg,Param1)):FALSE;

		case DN_ENTERIDLE: Msg=oldfar::DN_ENTERIDLE; break;
		case DN_GOTFOCUS:  Msg=oldfar::DN_GOTFOCUS; break;

		case DN_HELP:
		{
			char* HelpTopicA = UnicodeToAnsi((const wchar_t *)Param2);
			LONG_PTR ret = CurrentDlgProc(hDlg, oldfar::DN_HELP, Param1, (LONG_PTR)HelpTopicA);

			if(ret)
			{
				if(HelpTopic) xf_free(HelpTopic);
				HelpTopic = AnsiToUnicode((const char *)ret);
				ret = (LONG_PTR)HelpTopic;
			}

			xf_free (HelpTopicA);
			return ret;
		}

		case DN_HOTKEY:
			Msg=oldfar::DN_HOTKEY;
			break;

		case DN_INITDIALOG:
			Msg=oldfar::DN_INITDIALOG;
			break;

		case DN_KILLFOCUS:      Msg=oldfar::DN_KILLFOCUS; break;
		case DN_LISTCHANGE:     Msg=oldfar::DN_LISTCHANGE; break;
		case DN_MOUSECLICK:     Msg=oldfar::DN_MOUSECLICK; break;
		case DN_DRAGGED:        Msg=oldfar::DN_DRAGGED; break;
		case DN_RESIZECONSOLE:  Msg=oldfar::DN_RESIZECONSOLE; break;
		case DN_MOUSEEVENT:     Msg=oldfar::DN_MOUSEEVENT; break;
		case DN_DRAWDIALOGDONE: Msg=oldfar::DN_DRAWDIALOGDONE; break;
#ifdef FAR_USE_INTERNALS
		case DM_KILLSAVESCREEN: Msg=oldfar::DM_KILLSAVESCREEN; break;
		case DM_ALLKEYMODE:     Msg=oldfar::DM_ALLKEYMODE; break;
		case DN_ACTIVATEAPP:    Msg=oldfar::DN_ACTIVATEAPP; break;
#endif // END FAR_USE_INTERNALS
			break;

		case DN_KEY:
			Msg=oldfar::DN_KEY;
			Param2=KeyToOldKey((DWORD)Param2);
			break;
	}
	return CurrentDlgProc(hDlg, Msg, Param1, Param2);
}

LONG_PTR WINAPI FarDefDlgProcA(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	return FarDefDlgProc(hDlg, Msg, Param1, Param2);
}

LONG_PTR WINAPI FarSendDlgMessageA(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	switch(Msg)
	{
		case oldfar::DM_CLOSE:        Msg = DM_CLOSE; break;
		case oldfar::DM_ENABLE:       Msg = DM_ENABLE; break;
		case oldfar::DM_ENABLEREDRAW: Msg = DM_ENABLEREDRAW; break;
		case oldfar::DM_GETDLGDATA:   Msg = DM_GETDLGDATA; break;

		case oldfar::DM_GETDLGITEM:
		{
			FarDialogItem *di = (FarDialogItem *)FarSendDlgMessage(hDlg, DM_GETDLGITEM, Param1, 0);

			if (di)
			{
				oldfar::FarDialogItem *FarDiA=UnicodeDialogItemToAnsi(*di,hDlg,Param1);
				FarSendDlgMessage(hDlg, DM_FREEDLGITEM, 0, (LONG_PTR)di);

				memcpy((oldfar::FarDialogItem *)Param2,FarDiA,sizeof(oldfar::FarDialogItem));
				return TRUE;
			}

			return FALSE;
		}

		case oldfar::DM_GETDLGRECT: Msg = DM_GETDLGRECT; break;

		case oldfar::DM_GETTEXT:
		{
			if (!Param2)
				return FarSendDlgMessage(hDlg, DM_GETTEXT, Param1, 0);

			oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;
			wchar_t* text = (wchar_t*) xf_malloc((didA->PtrLength+1)*sizeof(wchar_t));

			//BUGBUG: ���� didA->PtrLength=0, �� ������� � ������ '\0', � ��� ��������, ��� ���, �� ��� ���������.
			FarDialogItemData did = {didA->PtrLength, text};

			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_GETTEXT, Param1, (LONG_PTR)&did);
			didA->PtrLength = (unsigned)did.PtrLength;
			UnicodeToAnsi(text,didA->PtrData);
			xf_free(text);
			return ret;
		}

		case oldfar::DM_GETTEXTLENGTH: Msg = DM_GETTEXTLENGTH; break;

		case oldfar::DM_KEY:
		{
			if(!Param1 || !Param2) return FALSE;
			int Count = (int)Param1;
			DWORD* KeysA = (DWORD*)Param2;
			DWORD* KeysW = (DWORD*)xf_malloc(Count*sizeof(DWORD));
			for(int i=0;i<Count;i++)
			{
				KeysW[i]=OldKeyToKey(KeysA[i]);
			}
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_KEY, Param1, (LONG_PTR)KeysW);
			xf_free(KeysW);
			return ret;
		}

		case oldfar::DM_MOVEDIALOG: Msg = DM_MOVEDIALOG; break;
		case oldfar::DM_SETDLGDATA: Msg = DM_SETDLGDATA; break;

		case oldfar::DM_SETDLGITEM:
		{
			if(!Param2)
				return FALSE;

			FarDialogItem *di=CurrentDialogItem(hDlg,Param1);
			FreeUnicodeDialogItem(*di);

			oldfar::FarDialogItem *diA = (oldfar::FarDialogItem *)Param2;
			AnsiDialogItemToUnicode(*diA,*di);
			return FarSendDlgMessage(hDlg, DM_SETDLGITEM, Param1, (LONG_PTR)di);
		}

		case oldfar::DM_SETFOCUS: Msg = DM_SETFOCUS; break;
		case oldfar::DM_REDRAW:   Msg = DM_REDRAW; break;

		case oldfar::DM_SETTEXT:
		{
			if (!Param2)return 0;
			oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;
			if (!didA->PtrData) return 0;
			wchar_t* text = AnsiToUnicode(didA->PtrData);

			//BUGBUG - PtrLength �� �� ��� �� ������.
			FarDialogItemData di = {didA->PtrLength,text};

			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_SETTEXT, Param1, (LONG_PTR)&di);
			xf_free (text);
			return ret;
		}

		case oldfar::DM_SETMAXTEXTLENGTH: Msg = DM_SETMAXTEXTLENGTH; break;
		case oldfar::DM_SHOWDIALOG:       Msg = DM_SHOWDIALOG; break;
		case oldfar::DM_GETFOCUS:         Msg = DM_GETFOCUS; break;
		case oldfar::DM_GETCURSORPOS:     Msg = DM_GETCURSORPOS; break;
		case oldfar::DM_SETCURSORPOS:     Msg = DM_SETCURSORPOS; break;

		case oldfar::DM_GETTEXTPTR:
		{
			LONG_PTR length = FarSendDlgMessage(hDlg, DM_GETTEXTPTR, Param1, 0);
			if (!Param2) return length;

			wchar_t* text = (wchar_t *) xf_malloc ((length +1)* sizeof(wchar_t));
			length = FarSendDlgMessage(hDlg, DM_GETTEXTPTR, Param1, (LONG_PTR)text);
			UnicodeToAnsi(text, (char *)Param2);
			xf_free(text);
			return length;
		}

		case oldfar::DM_SETTEXTPTR:
		{
			if (!Param2) return FALSE;
			wchar_t* text = AnsiToUnicode((char*)Param2);
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_SETTEXTPTR, Param1, (LONG_PTR)text);
			xf_free (text);
			return ret;
		}

		case oldfar::DM_SHOWITEM: Msg = DM_SHOWITEM; break;

		case oldfar::DM_ADDHISTORY:
		{
			if (!Param2) return FALSE;
			wchar_t* history = AnsiToUnicode((char*)Param2);
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_ADDHISTORY, Param1, (LONG_PTR)history);
			xf_free (history);
			return ret;
		}

		case oldfar::DM_GETCHECK:
		{
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_GETCHECK, Param1, 0);
			LONG_PTR state = 0;
			if      (ret == oldfar::BSTATE_UNCHECKED) state=BSTATE_UNCHECKED;
			else if (ret == oldfar::BSTATE_CHECKED)   state=BSTATE_CHECKED;
			else if (ret == oldfar::BSTATE_3STATE)    state=BSTATE_3STATE;
			return state;
		}

		case oldfar::DM_SETCHECK:
		{
			LONG_PTR state = 0;
			if      (Param2 == oldfar::BSTATE_UNCHECKED) state=BSTATE_UNCHECKED;
			else if (Param2 == oldfar::BSTATE_CHECKED)   state=BSTATE_CHECKED;
			else if (Param2 == oldfar::BSTATE_3STATE)    state=BSTATE_3STATE;
			else if (Param2 == oldfar::BSTATE_TOGGLE)    state=BSTATE_TOGGLE;
			return FarSendDlgMessage(hDlg, DM_SETCHECK, Param1, state);
		}

		case oldfar::DM_SET3STATE: Msg = DM_SET3STATE; break;
		case oldfar::DM_LISTSORT:  Msg = DM_LISTSORT; break;

		case oldfar::DM_LISTGETITEM: //BUGBUG, ���������� � ����
		{
			if (!Param2) return FALSE;
			oldfar::FarListGetItem* lgiA = (oldfar::FarListGetItem*)Param2;
			FarListGetItem lgi = {lgiA->ItemIndex};
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTGETITEM, Param1, (LONG_PTR)&lgi);
			UnicodeListItemToAnsi(&lgi.Item, &lgiA->Item);
			return ret;
		}

		case oldfar::DM_LISTGETCURPOS:
			if(Param2)
			{
				FarListPos lp;
				LONG_PTR ret=FarSendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, (LONG_PTR)&lp);
				oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
				lpA->SelectPos=lp.SelectPos;
				lpA->TopPos=lp.TopPos;
				return ret;
			}
			else return FarSendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, 0);

		case oldfar::DM_LISTSETCURPOS:
		{
			if(!Param2) return FALSE;
			oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
			FarListPos lp = {lpA->SelectPos,lpA->TopPos};
			return FarSendDlgMessage(hDlg, DM_LISTSETCURPOS, Param1, (LONG_PTR)&lp);
		}

		case oldfar::DM_LISTDELETE:
		{
			oldfar::FarListDelete *ldA = (oldfar::FarListDelete *)Param2;
			FarListDelete ld;
			if(Param2)
			{
				ld.Count = ldA->Count;
				ld.StartIndex = ldA->StartIndex;
			}
			return FarSendDlgMessage(hDlg, DM_LISTDELETE, Param1, Param2?(LONG_PTR)&ld:0);
		}

		case oldfar::DM_LISTADD:
		{
			FarList newlist = {0,0};
			if (Param2)
			{
				oldfar::FarList *oldlist = (oldfar::FarList*) Param2;
				newlist.ItemsNumber = oldlist->ItemsNumber;
				if (newlist.ItemsNumber)
				{
					newlist.Items = (FarListItem*)xf_malloc(newlist.ItemsNumber*sizeof(FarListItem));
					if (newlist.Items)
					{
						for (int i=0;i<newlist.ItemsNumber;i++)
							AnsiListItemToUnicode(&oldlist->Items[i], &newlist.Items[i]);
					}
				}
			}
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTADD, Param1, Param2?(LONG_PTR)&newlist:0);
			if (newlist.Items)
			{
				for (int i=0;i<newlist.ItemsNumber;i++)
					if (newlist.Items[i].Text) xf_free((void*)newlist.Items[i].Text);
				xf_free(newlist.Items);
			}
			return ret;
		}

		case oldfar::DM_LISTADDSTR:
		{
			wchar_t* newstr = NULL;
			if (Param2)
			{
				newstr = AnsiToUnicode((char*)Param2);
			}
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTADDSTR, Param1, (LONG_PTR)newstr);
			if (newstr) xf_free (newstr);
			return ret;
		}

		case oldfar::DM_LISTUPDATE:
		{
			FarListUpdate newui = {0,0};
			if (Param2)
			{
				oldfar::FarListUpdate *oldui = (oldfar::FarListUpdate*) Param2;
				newui.Index=oldui->Index;
				AnsiListItemToUnicode(&oldui->Item, &newui.Item);
			}
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTUPDATE, Param1, Param2?(LONG_PTR)&newui:0);
			if (newui.Item.Text) xf_free((void*)newui.Item.Text);
			return ret;
		}

		case oldfar::DM_LISTINSERT:
		{
			FarListInsert newli = {0,0};
			if (Param2)
			{
				oldfar::FarListInsert *oldli = (oldfar::FarListInsert*) Param2;
				newli.Index=oldli->Index;
				AnsiListItemToUnicode(&oldli->Item, &newli.Item);
			}
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTINSERT, Param1, Param2?(LONG_PTR)&newli:0);
			if (newli.Item.Text) xf_free((void*)newli.Item.Text);
			return ret;
		}

		case oldfar::DM_LISTFINDSTRING:
		{
			FarListFind newlf = {0,0,0,0};
			if (Param2)
			{
				oldfar::FarListFind *oldlf = (oldfar::FarListFind*) Param2;
				newlf.StartIndex=oldlf->StartIndex;
				newlf.Pattern = (oldlf->Pattern)?AnsiToUnicode(oldlf->Pattern):NULL;
				if(oldlf->Flags&oldfar::LIFIND_EXACTMATCH) newlf.Flags|=LIFIND_EXACTMATCH;
			}
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTFINDSTRING, Param1, Param2?(LONG_PTR)&newlf:0);
			if (newlf.Pattern) xf_free((void*)newlf.Pattern);
			return ret;
		}

		case oldfar::DM_LISTINFO:
		{
			if(!Param2) return FALSE;
			oldfar::FarListInfo *liA = (oldfar::FarListInfo *)Param2;
			FarListInfo li={0,liA->ItemsNumber,liA->SelectPos,liA->TopPos,liA->MaxHeight,liA->MaxLength};
			if(liA ->Flags&oldfar::LINFO_SHOWNOBOX) li.Flags|=LINFO_SHOWNOBOX;
			if(liA ->Flags&oldfar::LINFO_AUTOHIGHLIGHT) li.Flags|=LINFO_AUTOHIGHLIGHT;
			if(liA ->Flags&oldfar::LINFO_REVERSEHIGHLIGHT) li.Flags|=LINFO_REVERSEHIGHLIGHT;
			if(liA ->Flags&oldfar::LINFO_WRAPMODE) li.Flags|=LINFO_WRAPMODE;
			if(liA ->Flags&oldfar::LINFO_SHOWAMPERSAND) li.Flags|=LINFO_SHOWAMPERSAND;
			return FarSendDlgMessage(hDlg, DM_LISTINFO, Param1, Param2);
		}

		case oldfar::DM_LISTGETDATA:	Msg = DM_LISTGETDATA; break;

		case oldfar::DM_LISTSETDATA:
		{
			FarListItemData newlid = {0,0,0,0};
			if(Param2)
			{
				oldfar::FarListItemData *oldlid = (oldfar::FarListItemData*) Param2;
				newlid.Index=oldlid->Index;
				newlid.DataSize=oldlid->DataSize;
				newlid.Data=oldlid->Data;
			}
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTSETDATA, Param1, Param2?(LONG_PTR)&newlid:0);
			return ret;
		}

		case oldfar::DM_LISTSETTITLES:
		{
			if (!Param2) return FALSE;
			oldfar::FarListTitles *ltA = (oldfar::FarListTitles *)Param2;
			FarListTitles lt = {0,ltA->Title!=NULL?AnsiToUnicode(ltA->Title):NULL,0,ltA->Bottom!=NULL?AnsiToUnicode(ltA->Bottom):NULL};
			Param2 = (LONG_PTR)&lt;
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTSETTITLES, Param1, Param2);
			if (lt.Bottom) xf_free ((wchar_t *)lt.Bottom);
			if (lt.Title) xf_free ((wchar_t *)lt.Title);
			return ret;
		}

		case oldfar::DM_LISTGETTITLES:
		{
			//BUGBUG ���� ������� ����� DM_LISTGETTITLES ����� �������� � ����� ����
			return FALSE;
		}

		case oldfar::DM_RESIZEDIALOG:      Msg = DM_RESIZEDIALOG; break;
		case oldfar::DM_SETITEMPOSITION:   Msg = DM_SETITEMPOSITION; break;
		case oldfar::DM_GETDROPDOWNOPENED: Msg = DM_GETDROPDOWNOPENED; break;
		case oldfar::DM_SETDROPDOWNOPENED: Msg = DM_SETDROPDOWNOPENED; break;

		case oldfar::DM_SETHISTORY:
			if(!Param2)
				return FarSendDlgMessage(hDlg, DM_SETHISTORY, Param1, 0);
			else
			{
				FarDialogItem *di=CurrentDialogItem(hDlg,Param1);
					xf_free((void*)di->Param.History);
				di->Param.History = AnsiToUnicode((const char *)Param2);
				return FarSendDlgMessage(hDlg, DM_SETHISTORY, Param1, (LONG_PTR)di->Param.History);
			}
		case oldfar::DM_GETITEMPOSITION:     Msg = DM_GETITEMPOSITION; break;
		case oldfar::DM_SETMOUSEEVENTNOTIFY: Msg = DM_SETMOUSEEVENTNOTIFY; break;
		case oldfar::DM_EDITUNCHANGEDFLAG:   Msg = DM_EDITUNCHANGEDFLAG; break;
		case oldfar::DM_GETITEMDATA:         Msg = DM_GETITEMDATA; break;
		case oldfar::DM_SETITEMDATA:         Msg = DM_SETITEMDATA; break;

		case oldfar::DM_LISTSET:
		{
			FarList newlist = {0,0};
			if (Param2)
			{
				oldfar::FarList *oldlist = (oldfar::FarList*) Param2;
				newlist.ItemsNumber = oldlist->ItemsNumber;
				if (newlist.ItemsNumber)
				{
					newlist.Items = (FarListItem*)xf_malloc(newlist.ItemsNumber*sizeof(FarListItem));
					if (newlist.Items)
					{
						for (int i=0;i<newlist.ItemsNumber;i++)
							AnsiListItemToUnicode(&oldlist->Items[i], &newlist.Items[i]);
					}
				}
			}
			LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTSET, Param1, Param2?(LONG_PTR)&newlist:0);
			if (newlist.Items)
			{
				for (int i=0;i<newlist.ItemsNumber;i++)
					if (newlist.Items[i].Text) xf_free((void*)newlist.Items[i].Text);
				xf_free(newlist.Items);
			}
			return ret;
		}

		case oldfar::DM_LISTSETMOUSEREACTION:
		{
			LONG_PTR type=0;
			     if (Param2 == oldfar::LMRT_ONLYFOCUS) type=LMRT_ONLYFOCUS;
			else if (Param2 == oldfar::LMRT_ALWAYS)    type=LMRT_ALWAYS;
			else if (Param2 == oldfar::LMRT_NEVER)     type=LMRT_NEVER;
			return FarSendDlgMessage(hDlg, DM_LISTSETMOUSEREACTION, Param1, type);
		}

		case oldfar::DM_GETCURSORSIZE:   Msg = DM_GETCURSORSIZE; break;
		case oldfar::DM_SETCURSORSIZE:   Msg = DM_SETCURSORSIZE; break;
		case oldfar::DM_LISTGETDATASIZE: Msg = DM_LISTGETDATASIZE; break;

		case oldfar::DM_GETSELECTION:
		{
			if (!Param2) return FALSE;

			EditorSelect es;
			LONG_PTR ret=FarSendDlgMessage(hDlg, DM_GETSELECTION, Param1, (LONG_PTR)&es);
			oldfar::EditorSelect *esA = (oldfar::EditorSelect *)Param2;
			esA->BlockType      = es.BlockType;
			esA->BlockStartLine = es.BlockStartLine;
			esA->BlockStartPos  = es.BlockStartPos;
			esA->BlockWidth     = es.BlockWidth;
			esA->BlockHeight    = es.BlockHeight;
			return ret;
		}

		case oldfar::DM_SETSELECTION:
		{
			if (!Param2) return FALSE;
			oldfar::EditorSelect *esA = (oldfar::EditorSelect *)Param2;
			EditorSelect es;
			es.BlockType      = esA->BlockType;
			es.BlockStartLine = esA->BlockStartLine;
			es.BlockStartPos  = esA->BlockStartPos;
			es.BlockWidth     = esA->BlockWidth;
			es.BlockHeight    = esA->BlockHeight;
			return FarSendDlgMessage(hDlg, DM_SETSELECTION, Param1, (LONG_PTR)&es);
		}

#ifdef FAR_USE_INTERNALS
		case oldfar::DM_KILLSAVESCREEN:
		case oldfar::DM_ALLKEYMODE:
		case oldfar::DN_ACTIVATEAPP:
#endif // END FAR_USE_INTERNALS
			break;
	}
	return FarSendDlgMessage(hDlg, Msg, Param1, Param2);
}

int WINAPI FarDialogExA(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,struct oldfar::FarDialogItem *Item,int ItemsNumber,DWORD Reserved,DWORD Flags,oldfar::FARWINDOWPROC DlgProc,LONG_PTR Param)
{
	string strHT((HelpTopic?HelpTopic:""));

	if (!Item || !ItemsNumber) return -1;

	oldfar::FarDialogItem *diA=(oldfar::FarDialogItem *)xf_malloc(ItemsNumber*sizeof(oldfar::FarDialogItem));
	memset(diA,0,ItemsNumber*sizeof(oldfar::FarDialogItem));

	FarDialogItem *di = (FarDialogItem *)xf_malloc(ItemsNumber*sizeof(FarDialogItem));

	for (int i=0; i<ItemsNumber; i++)
	{
		AnsiDialogItemToUnicode(Item[i],di[i]);
	}

	DWORD DlgFlags = 0;

	if (Flags&oldfar::FDLG_WARNING)      DlgFlags|=FDLG_WARNING;
	if (Flags&oldfar::FDLG_SMALLDIALOG)  DlgFlags|=FDLG_SMALLDIALOG;
	if (Flags&oldfar::FDLG_NODRAWSHADOW) DlgFlags|=FDLG_NODRAWSHADOW;
	if (Flags&oldfar::FDLG_NODRAWPANEL)  DlgFlags|=FDLG_NODRAWPANEL;
#ifdef FAR_USE_INTERNALS
	if (Flags&oldfar::FDLG_NONMODAL)     DlgFlags|=FDLG_NONMODAL;
#endif // END FAR_USE_INTERNALS

	int ret = -1;

	HANDLE hDlg = FarDialogInit(PluginNumber, X1, Y1, X2, Y2, (HelpTopic?(const wchar_t *)strHT:NULL), (FarDialogItem *)di, ItemsNumber, 0, DlgFlags, DlgProc?DlgProcA:0, Param);

	DlgData* NewDialogData=(DlgData*)xf_malloc(sizeof(DlgData));
	NewDialogData->DlgProc=(LONG_PTR)DlgProc;
	NewDialogData->hDlg=hDlg;
	NewDialogData->Prev=DialogData;
	NewDialogData->diA=diA;
	NewDialogData->di=di;

	DialogData=NewDialogData;

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ret = FarDialogRun(hDlg);

		for (int i=0; i<ItemsNumber; i++)
		{
			FarDialogItem *pdi = (FarDialogItem *)FarSendDlgMessage(hDlg, DM_GETDLGITEM, i, 0);
			if (pdi)
			{
				UnicodeDialogItemToAnsiSafe(*pdi,Item[i]);
				const wchar_t *res = pdi->PtrData;
				if (!res) res = L"";
				if ((di[i].Type==DI_EDIT || di[i].Type==DI_COMBOBOX) && Item[i].Flags&oldfar::DIF_VAREDIT)
					UnicodeToAnsi(res, Item[i].Data.Ptr.PtrData, Item[i].Data.Ptr.PtrLength+1);
				else
					UnicodeToAnsi(res, Item[i].Data.Data, sizeof(Item[i].Data.Data));

				if(pdi->Type==DI_USERCONTROL)
				{
					di[i].Param.VBuf=pdi->Param.VBuf;
					Item[i].Param.VBuf=GetAnsiVBufPtr(pdi->Param.VBuf,GetAnsiVBufSize(Item[i]));
				}
				FarSendDlgMessage(hDlg, DM_FREEDLGITEM, 0, (LONG_PTR)pdi);
			}
			FreeAnsiDialogItem(diA[i]);
		}
		FarDialogFree(hDlg);

		for (int i=0; i<ItemsNumber; i++)
			FreeUnicodeDialogItem(di[i]);
	}

	DlgData* TmpDlgData=DialogData;
	DialogData=DialogData->Prev;
	xf_free(TmpDlgData);

	if (diA)
		xf_free(diA);
	if (di)
		xf_free(di);

	return ret;
}

int WINAPI FarDialogFnA(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,struct oldfar::FarDialogItem *Item,int ItemsNumber)
{
	return FarDialogExA(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, 0, 0, 0, 0);
}

void ConvertUnicodePanelInfoToAnsi(PanelInfo* PIW, oldfar::PanelInfo* PIA, BOOL Short)
{
	PIA->PanelType = 0;
	switch (PIW->PanelType)
	{
		case PTYPE_FILEPANEL:  PIA->PanelType = oldfar::PTYPE_FILEPANEL;  break;
		case PTYPE_TREEPANEL:  PIA->PanelType = oldfar::PTYPE_TREEPANEL;  break;
		case PTYPE_QVIEWPANEL: PIA->PanelType = oldfar::PTYPE_QVIEWPANEL; break;
		case PTYPE_INFOPANEL:  PIA->PanelType = oldfar::PTYPE_INFOPANEL;  break;
	}

	PIA->Plugin = PIW->Plugin;

	PIA->PanelRect.left   = PIW->PanelRect.left;
	PIA->PanelRect.top    = PIW->PanelRect.top;
	PIA->PanelRect.right  = PIW->PanelRect.right;
	PIA->PanelRect.bottom = PIW->PanelRect.bottom;

	PIA->ItemsNumber = PIW->ItemsNumber;
	PIA->SelectedItemsNumber = PIW->SelectedItemsNumber;

	if(Short) //FCTL_GET[ANOTHER]PANELSHORTINFO
	{
		PIA->PanelItems = NULL;
		PIA->SelectedItems = NULL;
	}
	else //FCTL_GET[ANOTHER]PANELINFO
	{
		ConvertPanelItemsArrayToAnsi(PIW->PanelItems, PIA->PanelItems, PIW->ItemsNumber);
		ConvertPanelItemsPtrArrayToAnsi(PIW->SelectedItems, PIA->SelectedItems, PIW->SelectedItemsNumber);
	}

	PIA->CurrentItem = PIW->CurrentItem;
	PIA->TopPanelItem = PIW->TopPanelItem;

	PIA->Visible = PIW->Visible;
	PIA->Focus = PIW->Focus;
	PIA->ViewMode = PIW->ViewMode;

	UnicodeToAnsi(PIW->lpwszColumnTypes, PIA->ColumnTypes, sizeof(PIA->ColumnTypes));
	UnicodeToAnsi(PIW->lpwszColumnWidths, PIA->ColumnWidths, sizeof(PIA->ColumnWidths));

	UnicodeToAnsi(PIW->lpwszCurDir, PIA->CurDir, sizeof(PIA->CurDir));

	PIA->ShortNames = PIW->ShortNames;

	PIA->SortMode = 0;
	switch (PIW->SortMode)
	{
		case SM_DEFAULT:        PIA->SortMode = oldfar::SM_DEFAULT;        break;
		case SM_UNSORTED:       PIA->SortMode = oldfar::SM_UNSORTED;       break;
		case SM_NAME:           PIA->SortMode = oldfar::SM_NAME;           break;
		case SM_EXT:            PIA->SortMode = oldfar::SM_EXT;            break;
		case SM_MTIME:          PIA->SortMode = oldfar::SM_MTIME;          break;
		case SM_CTIME:          PIA->SortMode = oldfar::SM_CTIME;          break;
		case SM_ATIME:          PIA->SortMode = oldfar::SM_ATIME;          break;
		case SM_SIZE:           PIA->SortMode = oldfar::SM_SIZE;           break;
		case SM_DESCR:          PIA->SortMode = oldfar::SM_DESCR;          break;
		case SM_OWNER:          PIA->SortMode = oldfar::SM_OWNER;          break;
		case SM_COMPRESSEDSIZE: PIA->SortMode = oldfar::SM_COMPRESSEDSIZE; break;
		case SM_NUMLINKS:       PIA->SortMode = oldfar::SM_NUMLINKS;       break;
	}

	PIA->Flags = 0;
	if (PIW->Flags&PFLAGS_SHOWHIDDEN)       PIA->Flags|=oldfar::PFLAGS_SHOWHIDDEN;
	if (PIW->Flags&PFLAGS_HIGHLIGHT)        PIA->Flags|=oldfar::PFLAGS_HIGHLIGHT;
	if (PIW->Flags&PFLAGS_REVERSESORTORDER) PIA->Flags|=oldfar::PFLAGS_REVERSESORTORDER;
	if (PIW->Flags&PFLAGS_USESORTGROUPS)    PIA->Flags|=oldfar::PFLAGS_USESORTGROUPS;
	if (PIW->Flags&PFLAGS_SELECTEDFIRST)    PIA->Flags|=oldfar::PFLAGS_SELECTEDFIRST;
	if (PIW->Flags&PFLAGS_REALNAMES)        PIA->Flags|=oldfar::PFLAGS_REALNAMES;
	if (PIW->Flags&PFLAGS_NUMERICSORT)      PIA->Flags|=oldfar::PFLAGS_NUMERICSORT;
	if (PIW->Flags&PFLAGS_PANELLEFT)        PIA->Flags|=oldfar::PFLAGS_PANELLEFT;

	PIA->Reserved = PIW->Reserved;
}

void FreeAnsiPanelInfo(oldfar::PanelInfo* PIA)
{
	if (PIA->PanelItems)
		FreePanelItemA(PIA->PanelItems,PIA->ItemsNumber);
	if (PIA->SelectedItems)
		FreePanelItemA(PIA->SelectedItems,PIA->SelectedItemsNumber);
	memset(PIA,0,sizeof(oldfar::PanelInfo));
}

int WINAPI FarControlA(HANDLE hPlugin,int Command,void *Param)
{
	static oldfar::PanelInfo PanelInfoA={0},AnotherPanelInfoA={0};
	static PanelInfo PnI={0},AnotherPnI={0};

	if(hPlugin==INVALID_HANDLE_VALUE)
		hPlugin=PANEL_ACTIVE;

	switch (Command)
	{
		case oldfar::FCTL_CHECKPANELSEXIST:
			return FarControl(hPlugin,FCTL_CHECKPANELSEXIST,Param);

		case oldfar::FCTL_CLOSEPLUGIN:
		{
			wchar_t *ParamW = NULL;
			if (Param)
				ParamW = AnsiToUnicode((const char *)Param);
			int ret = FarControl(hPlugin,FCTL_CLOSEPLUGIN,ParamW);
			if (ParamW) xf_free(ParamW);
			return ret;
		}

		case oldfar::FCTL_GETANOTHERPANELSHORTINFO:
		case oldfar::FCTL_GETANOTHERPANELINFO:
		case oldfar::FCTL_GETPANELSHORTINFO:
		case oldfar::FCTL_GETPANELINFO:
			{
				if(!Param )
					return FALSE;
				bool Short=(Command==oldfar::FCTL_GETPANELSHORTINFO || Command==oldfar::FCTL_GETANOTHERPANELSHORTINFO);
				bool Passive=(Command==oldfar::FCTL_GETANOTHERPANELINFO || Command==oldfar::FCTL_GETANOTHERPANELSHORTINFO);
				if(Passive)
					hPlugin=PANEL_PASSIVE;
				FarControl(hPlugin,FCTL_FREEPANELINFO,Passive?&AnotherPnI:&PnI);
				FreeAnsiPanelInfo(Passive?&AnotherPanelInfoA:&PanelInfoA);
				int ret = FarControl(hPlugin,FCTL_GETPANELINFO,Passive?&AnotherPnI:&PnI);
				if(ret)
				{
					ConvertUnicodePanelInfoToAnsi(Passive?&AnotherPnI:&PnI, Passive?&AnotherPanelInfoA:&PanelInfoA,Short);
					*(oldfar::PanelInfo*)Param=Passive?AnotherPanelInfoA:PanelInfoA;
					if(Short)
					{
						// ����� FCTL_GET[ANOTHER]PANELSHORTINFO ��� �� ���� ������� PnI
						// ��� ���������� FCTL_SET[ANOTHER]SELECTION, ������ ����� ��� � ���������.
						FarControl(hPlugin,FCTL_FREEPANELINFO,Passive?&AnotherPnI:&PnI);
					}
				}
				return ret;
			}

		case oldfar::FCTL_SETANOTHERSELECTION:
			hPlugin=PANEL_PASSIVE;
		case oldfar::FCTL_SETSELECTION:
			{
				if(!Param )
					return FALSE;
				oldfar::PanelInfo *pPIA=(oldfar::PanelInfo*)Param;
				PanelInfo *PnIPtr=(hPlugin==PANEL_PASSIVE)?&AnotherPnI:&PnI;
				for(int i=0;i<pPIA->ItemsNumber;i++)
				{
					if(pPIA->PanelItems[i].Flags & oldfar::PPIF_SELECTED)
						PnIPtr->PanelItems[i].Flags|=PPIF_SELECTED;
					else
						PnIPtr->PanelItems[i].Flags&=~PPIF_SELECTED;
				}
				return FarControl(hPlugin,FCTL_SETSELECTION,PnIPtr);
			}

		case oldfar::FCTL_REDRAWANOTHERPANEL:
			hPlugin = PANEL_PASSIVE;

		case oldfar::FCTL_REDRAWPANEL:
		{
			if ( !Param )
				return FarControl(hPlugin, FCTL_REDRAWPANEL, NULL);

			oldfar::PanelRedrawInfo* priA = (oldfar::PanelRedrawInfo*)Param;
			PanelRedrawInfo pri = {priA->CurrentItem,priA->TopPanelItem};

			return FarControl(hPlugin, FCTL_REDRAWPANEL, &pri);
		}

		case oldfar::FCTL_SETANOTHERNUMERICSORT:
			hPlugin = PANEL_PASSIVE;

		case oldfar::FCTL_SETNUMERICSORT:
			return FarControl(hPlugin, FCTL_SETNUMERICSORT, Param);

		case oldfar::FCTL_SETANOTHERPANELDIR:
			hPlugin = PANEL_PASSIVE;

		case oldfar::FCTL_SETPANELDIR:
		{
			if ( !Param )
				return FALSE;

			wchar_t* Dir = AnsiToUnicode((char*)Param);
			int ret = FarControl(hPlugin, FCTL_SETPANELDIR, Dir);
			xf_free(Dir);

			return ret;
		}

		case oldfar::FCTL_SETANOTHERSORTMODE:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETSORTMODE:

			if ( !Param )
				return FALSE;

			return FarControl(hPlugin, FCTL_SETSORTMODE, Param);

		case oldfar::FCTL_SETANOTHERSORTORDER:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETSORTORDER:
			return FarControl(hPlugin, FCTL_SETSORTORDER, Param);

		case oldfar::FCTL_SETANOTHERVIEWMODE:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_SETVIEWMODE:
			return FarControl(hPlugin, FCTL_SETVIEWMODE, Param);

		case oldfar::FCTL_UPDATEANOTHERPANEL:
			hPlugin = PANEL_PASSIVE;
		case oldfar::FCTL_UPDATEPANEL:
			return FarControl(hPlugin, FCTL_UPDATEPANEL, Param);


		case oldfar::FCTL_GETCMDLINE:
		case oldfar::FCTL_GETCMDLINESELECTEDTEXT:
		{
			if ( !Param || IsBadWritePtr(Param, sizeof(char) * 1024) )
				return FALSE;
			int CmdW=(Command==oldfar::FCTL_GETCMDLINE)?FCTL_GETCMDLINE:FCTL_GETCMDLINESELECTEDTEXT;
			wchar_t *s=(wchar_t*)xf_malloc((FarControl(hPlugin,CmdW,NULL)+1)*sizeof(wchar_t));
			FarControl(hPlugin,CmdW,s);
			UnicodeToAnsi(s, (char*)Param, 1024);
			return TRUE;
		}

		case oldfar::FCTL_GETCMDLINEPOS:
			if ( !Param )
				return FALSE;

			return FarControl(hPlugin,FCTL_GETCMDLINEPOS,Param);

		case oldfar::FCTL_GETCMDLINESELECTION:
		{
			if ( !Param )
				return FALSE;

			CmdLineSelect cls;

			int ret = FarControl(hPlugin, FCTL_GETCMDLINESELECTION, &cls);

			if ( ret )
			{
				oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
				clsA->SelStart = cls.SelStart;
				clsA->SelEnd = cls.SelEnd;
			}

			return ret;
		}

		case oldfar::FCTL_INSERTCMDLINE:
		{
			if ( !Param )
				return FALSE;

			wchar_t* s = AnsiToUnicode((const char*)Param);

			int ret = FarControl(hPlugin, FCTL_INSERTCMDLINE, s);

			xf_free(s);
			return ret;
		}

		case oldfar::FCTL_SETCMDLINE:
		{
			if ( !Param )
				return FALSE;

			wchar_t* s = AnsiToUnicode((const char*)Param);

			int ret = FarControl(hPlugin, FCTL_SETCMDLINE, s);

			xf_free(s);
			return ret;
		}

		case oldfar::FCTL_SETCMDLINEPOS:
			if ( !Param )
				return FALSE;

			return FarControl(hPlugin, FCTL_SETCMDLINEPOS, Param);

		case oldfar::FCTL_SETCMDLINESELECTION:
		{
			if ( !Param )
				return FALSE;

			oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
			CmdLineSelect cls = {clsA->SelStart,clsA->SelEnd};

			return FarControl(hPlugin, FCTL_SETCMDLINESELECTION, &cls);
		}

		case oldfar::FCTL_GETUSERSCREEN:
			return FarControl(hPlugin, FCTL_GETUSERSCREEN, NULL);

		case oldfar::FCTL_SETUSERSCREEN:
			return FarControl(hPlugin, FCTL_SETUSERSCREEN, NULL);
	}
	return FALSE;
}

int WINAPI FarGetDirListA(const char *Dir,struct oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber)
{
	return FALSE;
}

int WINAPI FarGetPluginDirListA(INT_PTR PluginNumber,HANDLE hPlugin,const char *Dir,struct oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber)
{
	return FALSE;
}

void WINAPI FarFreeDirListA(const struct oldfar::PluginPanelItem *PanelItem)
{
}

INT_PTR WINAPI FarAdvControlA(INT_PTR ModuleNumber,int Command,void *Param)
{
	static char *ErrMsg1 = NULL;
	static char *ErrMsg2 = NULL;
	static char *ErrMsg3 = NULL;

	switch (Command)
	{
		case oldfar::ACTL_GETFARVERSION:
		{
			DWORD FarVer=(DWORD)FarAdvControl(ModuleNumber,ACTL_GETFARVERSION,NULL);

			int OldFarVer;
			GetRegKey(L"wrapper",L"version",OldFarVer,FarVer);
			if(
			   //�� ���� ������� ������
			   (LOWORD(OldFarVer)<LOWORD(FarVer) || ((LOWORD(OldFarVer)==LOWORD(FarVer)) && HIWORD(OldFarVer)<HIWORD(FarVer))) &&
			   //� �� ���� 1.70.1
			   LOWORD(OldFarVer)>=0x0146 && HIWORD(OldFarVer)>=0x1)
				FarVer=OldFarVer;

			if(Param)
				*(DWORD*)Param=FarVer;
			return FarVer;
		}

		case oldfar::ACTL_CONSOLEMODE:
			return FarAdvControl(ModuleNumber, ACTL_CONSOLEMODE, Param);

		case oldfar::ACTL_GETSYSWORDDIV:
		{
			INT_PTR Length = FarAdvControl(ModuleNumber, ACTL_GETSYSWORDDIV, NULL);
			if(Param)
			{
				wchar_t *SysWordDiv = (wchar_t*)xf_malloc((Length+1)*sizeof(wchar_t));
				FarAdvControl(ModuleNumber, ACTL_GETSYSWORDDIV, SysWordDiv);
				UnicodeToAnsi(SysWordDiv,(char*)Param,NM);
				xf_free (SysWordDiv);
			}
			return Length;
		}

		case oldfar::ACTL_WAITKEY:
			return FarAdvControl(ModuleNumber, ACTL_WAITKEY, Param);

		case oldfar::ACTL_GETCOLOR:
			return FarAdvControl(ModuleNumber, ACTL_GETCOLOR, Param);

		case oldfar::ACTL_GETARRAYCOLOR:
			return FarAdvControl(ModuleNumber, ACTL_GETARRAYCOLOR, Param);

		case oldfar::ACTL_EJECTMEDIA:
			return FarAdvControl(ModuleNumber, ACTL_EJECTMEDIA, Param);

		case oldfar::ACTL_KEYMACRO:
		{
			if (!Param) return FALSE;
			ActlKeyMacro km;
			memset(&km,0,sizeof(km));
			oldfar::ActlKeyMacro *kmA=(oldfar::ActlKeyMacro *)Param;
			switch(kmA->Command)
			{
				case oldfar::MCMD_LOADALL:
					km.Command=MCMD_LOADALL;
					break;
				case oldfar::MCMD_SAVEALL:
					km.Command=MCMD_SAVEALL;
					break;
				case oldfar::MCMD_POSTMACROSTRING:
					km.Command=MCMD_POSTMACROSTRING;
					km.Param.PlainText.SequenceText=AnsiToUnicode(kmA->Param.PlainText.SequenceText);
					if(kmA->Param.PlainText.Flags&oldfar::KSFLAGS_DISABLEOUTPUT) km.Param.PlainText.Flags|=KSFLAGS_DISABLEOUTPUT;
					if(kmA->Param.PlainText.Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS) km.Param.PlainText.Flags|=KSFLAGS_NOSENDKEYSTOPLUGINS;
					if(kmA->Param.PlainText.Flags&oldfar::KSFLAGS_REG_MULTI_SZ) km.Param.PlainText.Flags|=KSFLAGS_REG_MULTI_SZ;
					break;
				case oldfar::MCMD_GETSTATE:
					km.Command=MCMD_GETSTATE;
					break;
	#ifdef FAR_USE_INTERNALS
					/*
				case oldfar::MCMD_COMPILEMACRO:
					km.Command=MCMD_COMPILEMACRO;
					km.Param.Compile.Count = kmA->Param.Compile.Count;
					km.Param.Compile.Flags = kmA->Param.Compile.Flags;
					km.Param.Compile.Sequence = AnsiToUnicode(kmA->Param.Compile.Sequence);
					break;
					*/
				case oldfar::MCMD_CHECKMACRO:
					km.Command=MCMD_CHECKMACRO;
					km.Param.PlainText.SequenceText=AnsiToUnicode(kmA->Param.PlainText.SequenceText);
					break;
	#endif // END FAR_USE_INTERNALS
			}
			INT_PTR res = FarAdvControl(ModuleNumber, ACTL_KEYMACRO, &km);
			switch (km.Command)
			{
				case MCMD_CHECKMACRO:
					if (ErrMsg1) xf_free(ErrMsg1);
					if (ErrMsg2) xf_free(ErrMsg2);
					if (ErrMsg3) xf_free(ErrMsg3);
					kmA->Param.MacroResult.ErrMsg1 = ErrMsg1 = UnicodeToAnsi(km.Param.MacroResult.ErrMsg1);
					kmA->Param.MacroResult.ErrMsg2 = ErrMsg2 = UnicodeToAnsi(km.Param.MacroResult.ErrMsg2);
					kmA->Param.MacroResult.ErrMsg3 = ErrMsg3 = UnicodeToAnsi(km.Param.MacroResult.ErrMsg3);
					if (km.Param.PlainText.SequenceText)
						xf_free(km.Param.PlainText.SequenceText);
					break;

				case MCMD_COMPILEMACRO:
					if (km.Param.Compile.Sequence)
						xf_free(km.Param.Compile.Sequence);
					break;

				case MCMD_POSTMACROSTRING:
					if (km.Param.PlainText.SequenceText)
						xf_free(km.Param.PlainText.SequenceText);
					break;
			}
			return res;
		}

		case oldfar::ACTL_POSTKEYSEQUENCE:
		{
			if (!Param) return FALSE;
			KeySequence ks;
			oldfar::KeySequence *ksA = (oldfar::KeySequence*)Param;
			if(!ksA->Count || !ksA->Sequence) return FALSE;
			ks.Count = ksA->Count;
			if (ksA->Flags&oldfar::KSFLAGS_DISABLEOUTPUT) ks.Flags|=KSFLAGS_DISABLEOUTPUT;
			if (ksA->Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS) ks.Flags|=KSFLAGS_NOSENDKEYSTOPLUGINS;
			ks.Sequence = (DWORD*)xf_malloc(ks.Count*sizeof(DWORD));
			for (int i=0;i<ks.Count;i++)
			{
				ks.Sequence[i]=OldKeyToKey(ksA->Sequence[i]);
			}
			LONG_PTR ret = FarAdvControl(ModuleNumber, ACTL_POSTKEYSEQUENCE, &ks);
			xf_free (ks.Sequence);
			return ret;
		}

		case oldfar::ACTL_GETSHORTWINDOWINFO:
		case oldfar::ACTL_GETWINDOWINFO:
		{
			if (!Param)
				return FALSE;
			int cmd = (Command==oldfar::ACTL_GETWINDOWINFO)?ACTL_GETWINDOWINFO:ACTL_GETSHORTWINDOWINFO;
			oldfar::WindowInfo *wiA = (oldfar::WindowInfo *)Param;
			WindowInfo wi={wiA->Pos};
			INT_PTR ret = FarAdvControl(ModuleNumber, cmd, &wi);
			if(ret)
			{
				switch (wi.Type)
				{
					case WTYPE_PANELS: wiA->Type = oldfar::WTYPE_PANELS; break;
					case WTYPE_VIEWER: wiA->Type = oldfar::WTYPE_VIEWER; break;
					case WTYPE_EDITOR: wiA->Type = oldfar::WTYPE_EDITOR; break;
					case WTYPE_DIALOG: wiA->Type = oldfar::WTYPE_DIALOG; break;
					case WTYPE_VMENU:  wiA->Type = oldfar::WTYPE_VMENU;  break;
					case WTYPE_HELP:   wiA->Type = oldfar::WTYPE_HELP;   break;
				}
				wiA->Modified = wi.Modified;
				wiA->Current = wi.Current;
				if(cmd==ACTL_GETWINDOWINFO)
				{
					UnicodeToAnsi(wi.TypeName,wiA->TypeName,sizeof(wiA->TypeName));
					UnicodeToAnsi(wi.Name,wiA->Name,sizeof(wiA->Name));
					FarAdvControl(ModuleNumber,ACTL_FREEWINDOWINFO,&wi);
				}
				else
				{
					*wiA->TypeName=0;
					*wiA->Name=0;
				}
			}
			return ret;
		}

		case oldfar::ACTL_GETWINDOWCOUNT:
			return FarAdvControl(ModuleNumber, ACTL_GETWINDOWCOUNT, 0);

		case oldfar::ACTL_SETCURRENTWINDOW:
			return FarAdvControl(ModuleNumber, ACTL_SETCURRENTWINDOW, Param);

		case oldfar::ACTL_COMMIT:
			return FarAdvControl(ModuleNumber, ACTL_COMMIT, 0);

		case oldfar::ACTL_GETFARHWND:
			return FarAdvControl(ModuleNumber, ACTL_GETFARHWND, 0);

		case oldfar::ACTL_GETSYSTEMSETTINGS:
		{
			INT_PTR ss = FarAdvControl(ModuleNumber, ACTL_GETSYSTEMSETTINGS, 0);
			INT_PTR ret = 0;
			if (ss&oldfar::FSS_CLEARROATTRIBUTE)          ret|=FSS_CLEARROATTRIBUTE;
			if (ss&oldfar::FSS_DELETETORECYCLEBIN)        ret|=FSS_DELETETORECYCLEBIN;
			if (ss&oldfar::FSS_USESYSTEMCOPYROUTINE)      ret|=FSS_USESYSTEMCOPYROUTINE;
			if (ss&oldfar::FSS_COPYFILESOPENEDFORWRITING) ret|=FSS_COPYFILESOPENEDFORWRITING;
			if (ss&oldfar::FSS_CREATEFOLDERSINUPPERCASE)  ret|=FSS_CREATEFOLDERSINUPPERCASE;
			if (ss&oldfar::FSS_SAVECOMMANDSHISTORY)       ret|=FSS_SAVECOMMANDSHISTORY;
			if (ss&oldfar::FSS_SAVEFOLDERSHISTORY)        ret|=FSS_SAVEFOLDERSHISTORY;
			if (ss&oldfar::FSS_SAVEVIEWANDEDITHISTORY)    ret|=FSS_SAVEVIEWANDEDITHISTORY;
			if (ss&oldfar::FSS_USEWINDOWSREGISTEREDTYPES) ret|=FSS_USEWINDOWSREGISTEREDTYPES;
			if (ss&oldfar::FSS_AUTOSAVESETUP)             ret|=FSS_AUTOSAVESETUP;
			if (ss&oldfar::FSS_SCANSYMLINK)               ret|=FSS_SCANSYMLINK;
			return ret;
		}

		case oldfar::ACTL_GETPANELSETTINGS:
		{
			INT_PTR ps = FarAdvControl(ModuleNumber, ACTL_GETPANELSETTINGS, 0);
			INT_PTR ret = 0;
			if (ps&oldfar::FPS_SHOWHIDDENANDSYSTEMFILES)    ret|=FPS_SHOWHIDDENANDSYSTEMFILES;
			if (ps&oldfar::FPS_HIGHLIGHTFILES)              ret|=FPS_HIGHLIGHTFILES;
			if (ps&oldfar::FPS_AUTOCHANGEFOLDER)            ret|=FPS_AUTOCHANGEFOLDER;
			if (ps&oldfar::FPS_SELECTFOLDERS)               ret|=FPS_SELECTFOLDERS;
			if (ps&oldfar::FPS_ALLOWREVERSESORTMODES)       ret|=FPS_ALLOWREVERSESORTMODES;
			if (ps&oldfar::FPS_SHOWCOLUMNTITLES)            ret|=FPS_SHOWCOLUMNTITLES;
			if (ps&oldfar::FPS_SHOWSTATUSLINE)              ret|=FPS_SHOWSTATUSLINE;
			if (ps&oldfar::FPS_SHOWFILESTOTALINFORMATION)   ret|=FPS_SHOWFILESTOTALINFORMATION;
			if (ps&oldfar::FPS_SHOWFREESIZE)                ret|=FPS_SHOWFREESIZE;
			if (ps&oldfar::FPS_SHOWSCROLLBAR)               ret|=FPS_SHOWSCROLLBAR;
			if (ps&oldfar::FPS_SHOWBACKGROUNDSCREENSNUMBER) ret|=FPS_SHOWBACKGROUNDSCREENSNUMBER;
			if (ps&oldfar::FPS_SHOWSORTMODELETTER)          ret|=FPS_SHOWSORTMODELETTER;
			return ret;
		}

		case oldfar::ACTL_GETINTERFACESETTINGS:
		{
			INT_PTR is = FarAdvControl(ModuleNumber, ACTL_GETINTERFACESETTINGS, 0);
			INT_PTR ret = 0;
			if (is&oldfar::FIS_CLOCKINPANELS)                  ret|=FIS_CLOCKINPANELS;
			if (is&oldfar::FIS_CLOCKINVIEWERANDEDITOR)         ret|=FIS_CLOCKINVIEWERANDEDITOR;
			if (is&oldfar::FIS_MOUSE)                          ret|=FIS_MOUSE;
			if (is&oldfar::FIS_SHOWKEYBAR)                     ret|=FIS_SHOWKEYBAR;
			if (is&oldfar::FIS_ALWAYSSHOWMENUBAR)              ret|=FIS_ALWAYSSHOWMENUBAR;
			if (is&oldfar::FIS_USERIGHTALTASALTGR)             ret|=FIS_USERIGHTALTASALTGR;
			if (is&oldfar::FIS_SHOWTOTALCOPYPROGRESSINDICATOR) ret|=FIS_SHOWTOTALCOPYPROGRESSINDICATOR;
			if (is&oldfar::FIS_SHOWCOPYINGTIMEINFO)            ret|=FIS_SHOWCOPYINGTIMEINFO;
			if (is&oldfar::FIS_USECTRLPGUPTOCHANGEDRIVE)       ret|=FIS_USECTRLPGUPTOCHANGEDRIVE;
			return ret;
		}

		case oldfar::ACTL_GETCONFIRMATIONS:
		{
			INT_PTR cs = FarAdvControl(ModuleNumber, ACTL_GETCONFIRMATIONS, 0);
			INT_PTR ret = 0;
			if (cs&oldfar::FCS_COPYOVERWRITE)          ret|=FCS_COPYOVERWRITE;
			if (cs&oldfar::FCS_MOVEOVERWRITE)          ret|=FCS_MOVEOVERWRITE;
			if (cs&oldfar::FCS_DRAGANDDROP)            ret|=FCS_DRAGANDDROP;
			if (cs&oldfar::FCS_DELETE)                 ret|=FCS_DELETE;
			if (cs&oldfar::FCS_DELETENONEMPTYFOLDERS)  ret|=FCS_DELETENONEMPTYFOLDERS;
			if (cs&oldfar::FCS_INTERRUPTOPERATION)     ret|=FCS_INTERRUPTOPERATION;
			if (cs&oldfar::FCS_DISCONNECTNETWORKDRIVE) ret|=FCS_DISCONNECTNETWORKDRIVE;
			if (cs&oldfar::FCS_RELOADEDITEDFILE)       ret|=FCS_RELOADEDITEDFILE;
			if (cs&oldfar::FCS_CLEARHISTORYLIST)       ret|=FCS_CLEARHISTORYLIST;
			if (cs&oldfar::FCS_EXIT)                   ret|=FCS_EXIT;
			return ret;
		}

		case oldfar::ACTL_GETDESCSETTINGS:
		{
			INT_PTR ds = FarAdvControl(ModuleNumber, ACTL_GETDESCSETTINGS, 0);
			INT_PTR ret = 0;
			if (ds&oldfar::FDS_UPDATEALWAYS)      ret|=FDS_UPDATEALWAYS;
			if (ds&oldfar::FDS_UPDATEIFDISPLAYED) ret|=FDS_UPDATEIFDISPLAYED;
			if (ds&oldfar::FDS_SETHIDDEN)         ret|=FDS_SETHIDDEN;
			if (ds&oldfar::FDS_UPDATEREADONLY)    ret|=FDS_UPDATEREADONLY;
			return ret;
		}

		case oldfar::ACTL_SETARRAYCOLOR:
		{
			if (!Param) return FALSE;
			oldfar::FarSetColors *scA = (oldfar::FarSetColors *)Param;
			FarSetColors sc = {0, scA->StartIndex, scA->ColorCount, scA->Colors};
			if (scA->Flags&oldfar::FCLR_REDRAW) sc.Flags|=FCLR_REDRAW;
			return FarAdvControl(ModuleNumber, ACTL_SETARRAYCOLOR, &sc);
		}

		case oldfar::ACTL_GETWCHARMODE:
			return FarAdvControl(ModuleNumber, ACTL_GETWCHARMODE, 0);

		case oldfar::ACTL_GETPLUGINMAXREADDATA:
			return FarAdvControl(ModuleNumber, ACTL_GETPLUGINMAXREADDATA, 0);

		case oldfar::ACTL_GETDIALOGSETTINGS:
		{
			INT_PTR ds = FarAdvControl(ModuleNumber, ACTL_GETDIALOGSETTINGS, 0);
			INT_PTR ret = 0;
			if (ds&oldfar::FDIS_HISTORYINDIALOGEDITCONTROLS)    ret|=FDIS_HISTORYINDIALOGEDITCONTROLS;
			if (ds&oldfar::FDIS_HISTORYINDIALOGEDITCONTROLS)    ret|=FDIS_HISTORYINDIALOGEDITCONTROLS;
			if (ds&oldfar::FDIS_PERSISTENTBLOCKSINEDITCONTROLS) ret|=FDIS_PERSISTENTBLOCKSINEDITCONTROLS;
			if (ds&oldfar::FDIS_BSDELETEUNCHANGEDTEXT)          ret|=FDIS_BSDELETEUNCHANGEDTEXT;
			return ret;
		}
	#ifdef FAR_USE_INTERNALS
		case oldfar::ACTL_REMOVEMEDIA:
		case oldfar::ACTL_GETMEDIATYPE:
		case oldfar::ACTL_GETPOLICIES:
			return FALSE;
	#endif // END FAR_USE_INTERNALS
		case oldfar::ACTL_REDRAWALL:
			return FarAdvControl(ModuleNumber, ACTL_REDRAWALL, 0);
	}
	return FALSE;
}

int WINAPI FarEditorControlA(int Command,void* Param)
{
	static char *gt=NULL;
	static char *geol=NULL;
	static char *fn=NULL;

	switch (Command)
	{
		case oldfar::ECTL_ADDCOLOR:			Command = ECTL_ADDCOLOR; break;
		case oldfar::ECTL_DELETEBLOCK:	Command = ECTL_DELETEBLOCK; break;
		case oldfar::ECTL_DELETECHAR:		Command = ECTL_DELETECHAR; break;
		case oldfar::ECTL_DELETESTRING:	Command = ECTL_DELETESTRING; break;
		case oldfar::ECTL_EXPANDTABS:		Command = ECTL_EXPANDTABS; break;
		case oldfar::ECTL_GETCOLOR:			Command = ECTL_GETCOLOR; break;
		case oldfar::ECTL_GETBOOKMARKS:	Command = ECTL_GETBOOKMARKS; break;
		case oldfar::ECTL_INSERTSTRING:	Command = ECTL_INSERTSTRING; break;
		case oldfar::ECTL_QUIT:					Command = ECTL_QUIT; break;
		case oldfar::ECTL_REALTOTAB:		Command = ECTL_REALTOTAB; break;
		case oldfar::ECTL_REDRAW:				Command = ECTL_REDRAW; break;
		case oldfar::ECTL_SELECT:				Command = ECTL_SELECT; break;
		case oldfar::ECTL_SETPOSITION:	Command = ECTL_SETPOSITION; break;
		case oldfar::ECTL_TABTOREAL:		Command = ECTL_TABTOREAL; break;
		case oldfar::ECTL_TURNOFFMARKINGBLOCK:	Command = ECTL_TURNOFFMARKINGBLOCK; break;
		case oldfar::ECTL_ADDSTACKBOOKMARK:			Command = ECTL_ADDSTACKBOOKMARK; break;
		case oldfar::ECTL_PREVSTACKBOOKMARK:		Command = ECTL_PREVSTACKBOOKMARK; break;
		case oldfar::ECTL_NEXTSTACKBOOKMARK:		Command = ECTL_NEXTSTACKBOOKMARK; break;
		case oldfar::ECTL_CLEARSTACKBOOKMARKS:	Command = ECTL_CLEARSTACKBOOKMARKS; break;
		case oldfar::ECTL_DELETESTACKBOOKMARK:	Command = ECTL_DELETESTACKBOOKMARK; break;
		case oldfar::ECTL_GETSTACKBOOKMARKS:		Command = ECTL_GETSTACKBOOKMARKS; break;

		case oldfar::ECTL_GETSTRING:
		{
			EditorGetString egs;
			oldfar::EditorGetString *oegs=(oldfar::EditorGetString *)Param;
			if (!oegs) return FALSE;
			egs.StringNumber=oegs->StringNumber;
			int ret=FarEditorControl(ECTL_GETSTRING,&egs);
			if (ret)
			{
				oegs->StringNumber=egs.StringNumber;
				oegs->StringLength=egs.StringLength;
				oegs->SelStart=egs.SelStart;
				oegs->SelEnd=egs.SelEnd;
				if (gt) xf_free(gt);
				if (geol) xf_free(geol);
				gt = UnicodeToAnsiBin(egs.StringText,egs.StringLength);
				geol = UnicodeToAnsi(egs.StringEOL);
				oegs->StringText=gt;
				oegs->StringEOL=geol;
				return TRUE;
			}
			return FALSE;
		}

		case oldfar::ECTL_INSERTTEXT:
		{
			const char *p=(const char *)Param;
			if (!p) return FALSE;
			string strP(p);
			return FarEditorControl(ECTL_INSERTTEXT,(void *)(const wchar_t *)strP);
		}

		case oldfar::ECTL_GETINFO:
		{
			EditorInfo ei;
			oldfar::EditorInfo *oei=(oldfar::EditorInfo *)Param;
			if (!oei) return FALSE;
			int ret=FarEditorControl(ECTL_GETINFO,&ei);
			if (ret)
			{
				memset(oei,0,sizeof(*oei));
				oei->EditorID=ei.EditorID;
				if (fn)	xf_free(fn);
				fn = UnicodeToAnsi(ei.FileName);
				oei->FileName=fn;
				oei->WindowSizeX=ei.WindowSizeX;
				oei->WindowSizeY=ei.WindowSizeY;
				oei->TotalLines=ei.TotalLines;
				oei->CurLine=ei.CurLine;
				oei->CurPos=ei.CurPos;
				oei->CurTabPos=ei.CurTabPos;
				oei->TopScreenLine=ei.TopScreenLine;
				oei->LeftPos=ei.LeftPos;
				oei->Overtype=ei.Overtype;
				oei->BlockType=ei.BlockType;
				oei->BlockStartLine=ei.BlockStartLine;
				oei->AnsiMode=0;
				oei->TableNum=-1;
				oei->Options=ei.Options;
				oei->TabSize=ei.TabSize;
				oei->BookMarkCount=ei.BookMarkCount;
				oei->CurState=ei.CurState;
				return TRUE;
			}
			return FALSE;
		}

		case oldfar::ECTL_EDITORTOOEM:
		case oldfar::ECTL_OEMTOEDITOR:
			return TRUE;

		case oldfar::ECTL_SAVEFILE:
	{
		EditorSaveFile newsf = {0,0};
		if (Param)
		{
			oldfar::EditorSaveFile *oldsf = (oldfar::EditorSaveFile*) Param;
			newsf.FileName=(oldsf->FileName)?AnsiToUnicode(oldsf->FileName):NULL;
			newsf.FileEOL=(oldsf->FileEOL)?AnsiToUnicode(oldsf->FileEOL):NULL;
		}
		int ret = FarEditorControl(ECTL_SAVEFILE, Param?(void *)&newsf:0);
		if (newsf.FileName) xf_free((void*)newsf.FileName);
		if (newsf.FileEOL) xf_free((void*)newsf.FileEOL);
		return ret;
	}

		case oldfar::ECTL_PROCESSINPUT:	//BUGBUG?
		{
			if (Param)
			{
				INPUT_RECORD *pIR = (INPUT_RECORD*) Param;
				switch(pIR->EventType)
				{
					case KEY_EVENT:
					case FARMACRO_KEY_EVENT:
						{
							wchar_t res;
							MultiByteToWideChar (
											CP_OEMCP,
											0,
											&pIR->Event.KeyEvent.uChar.AsciiChar,
											1,
											&res,
											1
											);

							 pIR->Event.KeyEvent.uChar.UnicodeChar=res;
						}
				}
			}
			return FarEditorControl(ECTL_PROCESSINPUT, Param);
		}

		case oldfar::ECTL_PROCESSKEY:
		{
			return FarEditorControl(ECTL_PROCESSKEY, (void*)(DWORD_PTR)OldKeyToKey((DWORD)(DWORD_PTR)Param));
		}

		case oldfar::ECTL_READINPUT:	//BUGBUG?
		{
			int ret = FarEditorControl(ECTL_READINPUT, Param);
			if (Param)
			{
				INPUT_RECORD *pIR = (INPUT_RECORD*) Param;
				switch(pIR->EventType)
				{
					case KEY_EVENT:
					case FARMACRO_KEY_EVENT:
						{
							char res;
							WideCharToMultiByte (
									CP_OEMCP,
									0,
									&pIR->Event.KeyEvent.uChar.UnicodeChar,
									1,
									&res,
									1,
									NULL,
									NULL
									);
						pIR->Event.KeyEvent.uChar.UnicodeChar=res;
					}
				}
			}
			return ret;
		}

		case oldfar::ECTL_SETKEYBAR:
		{
			switch((LONG_PTR)Param)
			{
				case NULL:
				case -1:
					return FarEditorControl(ECTL_SETKEYBAR, Param);
				default:
					oldfar::KeyBarTitles* oldkbt = (oldfar::KeyBarTitles*)Param;
					KeyBarTitles newkbt;
					ConvertKeyBarTitlesA(oldkbt, &newkbt);
					int ret = FarEditorControl(ECTL_SETKEYBAR, (void*)&newkbt);
					FreeKeyBarTitlesW(&newkbt);
					return ret;
			}
		}

		case oldfar::ECTL_SETPARAM:
		{
			EditorSetParameter newsp = {0,0,0,0};
			if (Param)
			{
				oldfar::EditorSetParameter *oldsp= (oldfar::EditorSetParameter*) Param;

				newsp.Param.iParam = oldsp->Param.iParam;

				switch (oldsp->Type)
				{
					case oldfar::ESPT_AUTOINDENT:				newsp.Type = ESPT_AUTOINDENT; break;
					case oldfar::ESPT_CHARCODEBASE:			newsp.Type = ESPT_CHARCODEBASE; break;
					case oldfar::ESPT_CURSORBEYONDEOL:	newsp.Type = ESPT_CURSORBEYONDEOL; break;
					case oldfar::ESPT_LOCKMODE:					newsp.Type = ESPT_LOCKMODE; break;
					case oldfar::ESPT_SAVEFILEPOSITION:	newsp.Type = ESPT_SAVEFILEPOSITION; break;
					case oldfar::ESPT_TABSIZE:					newsp.Type = ESPT_TABSIZE; break;

					case oldfar::ESPT_CHARTABLE: //BUGBUG, ���������� � ����
					{
						newsp.Type = ESPT_CHARTABLE;
						break;
					}

					case oldfar::ESPT_EXPANDTABS:
					{
						newsp.Type = ESPT_EXPANDTABS;
						switch (oldsp->Param.iParam)
						{
							case oldfar::EXPAND_NOTABS:		newsp.Param.iParam = EXPAND_NOTABS; break;
							case oldfar::EXPAND_ALLTABS:	newsp.Param.iParam = EXPAND_ALLTABS; break;
							case oldfar::EXPAND_NEWTABS: 	newsp.Param.iParam = EXPAND_NEWTABS; break;
							default: return FALSE;
						}
						break;
					}

					case oldfar::ESPT_SETWORDDIV:
					{
						newsp.Type = ESPT_SETWORDDIV;
						newsp.Param.cParam = (oldsp->Param.cParam)?AnsiToUnicode(oldsp->Param.cParam):NULL;
						int ret = FarEditorControl(ECTL_SETPARAM, (void *)&newsp);
						if (newsp.Param.cParam) xf_free(newsp.Param.cParam);
						return ret;
					}

					case oldfar::ESPT_GETWORDDIV:
					{
						if(!oldsp->Param.cParam) return FALSE;

						*oldsp->Param.cParam=0;

						newsp.Type = ESPT_GETWORDDIV;
						newsp.Param.cParam = (wchar_t*)xf_malloc(8192*sizeof(wchar_t)); //BUGBUG, ���������� ����� �����. ������

						if (newsp.Param.cParam)
						{
							int ret = FarEditorControl(ECTL_SETPARAM, (void *)&newsp);
							char *olddiv = UnicodeToAnsi(newsp.Param.cParam);
							if (olddiv)
							{
								int l = Min((int)strlen (olddiv),255);
								memcpy(oldsp->Param.cParam,olddiv,l);
								oldsp->Param.cParam[l+1]=0;
								xf_free(olddiv);
							}
							xf_free(newsp.Param.cParam);
							return ret;
						}
						return FALSE;
					}

					default:
						return FALSE;
				}
			}
			return FarEditorControl(ECTL_SETPARAM, Param?(void *)&newsp:0);
		}

		case oldfar::ECTL_SETSTRING:
		{
			EditorSetString newss = {0,0,0,0};
			if (Param)
			{
				oldfar::EditorSetString *oldss = (oldfar::EditorSetString*) Param;
				newss.StringNumber=oldss->StringNumber;
				newss.StringText=(oldss->StringText)?AnsiToUnicodeBin(oldss->StringText, oldss->StringLength):NULL;
				newss.StringEOL=(oldss->StringEOL)?AnsiToUnicode(oldss->StringEOL):NULL;
				newss.StringLength=oldss->StringLength;
			}
			int ret = FarEditorControl(ECTL_SETSTRING, Param?(void *)&newss:0);
			if (newss.StringText) xf_free((void*)newss.StringText);
			if (newss.StringEOL) xf_free((void*)newss.StringEOL);
			return ret;
		}

		case oldfar::ECTL_SETTITLE:
		{
			wchar_t* newtit = NULL;
			if (Param)
			{
				newtit=AnsiToUnicode((char*)Param);
			}
			int ret = FarEditorControl(ECTL_SETTITLE, (void *)newtit);
			if (newtit) xf_free(newtit);
			return ret;
		}

		default:
			return FALSE;
	}

	return FarEditorControl(Command,Param);
}

int WINAPI FarViewerControlA(int Command,void* Param)
{
	static char* filename=NULL;

	switch (Command)
	{
		case oldfar::VCTL_GETINFO:
		{
			if (!Param) return FALSE;
			oldfar::ViewerInfo* viA = (oldfar::ViewerInfo*)Param;
			if (!viA->StructSize) return FALSE;
			ViewerInfo viW;
			viW.StructSize = sizeof(ViewerInfo); //BUGBUG?
			if (FarViewerControl(VCTL_GETINFO, &viW) == FALSE) return FALSE;

			viA->ViewerID = viW.ViewerID;

			if (filename) xf_free (filename);
			filename = UnicodeToAnsi(viW.FileName);
			viA->FileName = filename;

			viA->FileSize.i64 = viW.FileSize.i64;
			viA->FilePos.i64 = viW.FilePos.i64;
			viA->WindowSizeX = viW.WindowSizeX;
			viA->WindowSizeY = viW.WindowSizeY;

			viA->Options = 0;
			if (viW.Options&VOPT_SAVEFILEPOSITION) viA->Options |= oldfar::VOPT_SAVEFILEPOSITION;
			if (viW.Options&VOPT_AUTODETECTTABLE)  viA->Options |= oldfar::VOPT_AUTODETECTTABLE;

			viA->TabSize = viW.TabSize;

			viA->CurMode.UseDecodeTable = viW.CurMode.UseDecodeTable;
			viA->CurMode.TableNum       = viW.CurMode.TableNum;
			viA->CurMode.AnsiMode       = viW.CurMode.AnsiMode;
			viA->CurMode.Unicode        = viW.CurMode.Unicode;
			viA->CurMode.Wrap           = viW.CurMode.Wrap;
			viA->CurMode.WordWrap       = viW.CurMode.WordWrap;
			viA->CurMode.Hex            = viW.CurMode.Hex;

			viA->LeftPos = viW.LeftPos;
			viA->Reserved3 = 0;
			break;
		}

		case oldfar::VCTL_QUIT:
			return FarViewerControl(VCTL_QUIT, NULL);

		case oldfar::VCTL_REDRAW:
			return FarViewerControl(VCTL_REDRAW, NULL);

		case oldfar::VCTL_SETKEYBAR:
		{
			switch((LONG_PTR)Param)
			{
				case NULL:
				case -1:
					return FarViewerControl(VCTL_SETKEYBAR, Param);
				default:
					oldfar::KeyBarTitles* kbtA = (oldfar::KeyBarTitles*)Param;
					KeyBarTitles kbt;
					ConvertKeyBarTitlesA(kbtA, &kbt);
					int ret=FarViewerControl(VCTL_SETKEYBAR, &kbt);
					FreeKeyBarTitlesW(&kbt);
					return ret;
			}
		}

		case oldfar::VCTL_SETPOSITION:
		{
			if (!Param) return FALSE;
			oldfar::ViewerSetPosition* vspA = (oldfar::ViewerSetPosition*)Param;
			ViewerSetPosition vsp;
			vsp.Flags = 0;
			if(vspA->Flags&oldfar::VSP_NOREDRAW)    vsp.Flags|=VSP_NOREDRAW;
			if(vspA->Flags&oldfar::VSP_PERCENT)     vsp.Flags|=VSP_PERCENT;
			if(vspA->Flags&oldfar::VSP_RELATIVE)    vsp.Flags|=VSP_RELATIVE;
			if(vspA->Flags&oldfar::VSP_NORETNEWPOS) vsp.Flags|=VSP_NORETNEWPOS;
			vsp.StartPos.i64 = vspA->StartPos.i64;
			vsp.LeftPos = vspA->LeftPos;
			int ret = FarViewerControl(VCTL_SETPOSITION, &vsp);
			vspA->StartPos.i64 = vsp.StartPos.i64;
			return ret;
		}

		case oldfar::VCTL_SELECT:
		{
			if (!Param) return FarViewerControl(VCTL_SELECT, NULL);

			oldfar::ViewerSelect* vsA = (oldfar::ViewerSelect*)Param;
			ViewerSelect vs = {vsA->BlockStartPos.i64,vsA->BlockLen};
			return FarViewerControl(VCTL_SELECT, &vs);
		}

		case oldfar::VCTL_SETMODE:
		{
			if (!Param) return FALSE;
			oldfar::ViewerSetMode* vsmA = (oldfar::ViewerSetMode*)Param;
			ViewerSetMode vsm;
			vsm.Type = 0;
			switch(vsmA->Type)
			{
				case oldfar::VSMT_HEX:      vsm.Type = VSMT_HEX;      break;
				case oldfar::VSMT_WRAP:     vsm.Type = VSMT_WRAP;     break;
				case oldfar::VSMT_WORDWRAP: vsm.Type = VSMT_WORDWRAP; break;
			}
			vsm.Param.iParam = vsmA->Param.iParam;
			vsm.Flags = 0;
			if(vsmA->Flags&oldfar::VSMFL_REDRAW) vsm.Flags|=VSMFL_REDRAW;
			vsm.Reserved = 0;
			return FarViewerControl(VCTL_SETMODE, &vsm);
		}
	}
	return TRUE;
}

int WINAPI FarCharTableA(int Command,char *Buffer,int BufferSize) //BUGBUG
{
 	return FarCharTable(Command,Buffer,BufferSize);
}

int WINAPI GetFileOwnerA(const char *Computer,const char *Name, char *Owner)
{
	string strComputer=Computer,strName=Name,strOwner;
	int Ret=GetFileOwner(strComputer,strName,strOwner);
	strOwner.GetCharString(Owner,NM);
	return Ret;
}
