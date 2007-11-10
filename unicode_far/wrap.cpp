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

wchar_t *AnsiToUnicode (const char *lpszAnsiString)
{
  int nLength = (int)strlen (lpszAnsiString)+1;

  wchar_t *lpResult = (wchar_t*)malloc (nLength*sizeof(wchar_t));

  wmemset (lpResult, 0, nLength);

  MultiByteToWideChar (
          CP_OEMCP,
          0,
          lpszAnsiString,
          -1,
          lpResult,
          nLength
          );

  return lpResult;
}

char *UnicodeToAnsiBin (const wchar_t *lpwszUnicodeString, int nLength)
{
  char *lpResult = (char*)malloc (nLength);

  memset (lpResult, 0, nLength);

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

void ConvertPanelItemA(const oldfar::PluginPanelItem *PanelItemA, PluginPanelItem **PanelItemW, int ItemsNumber)
{
	*PanelItemW = (PluginPanelItem *)malloc(ItemsNumber*sizeof(PluginPanelItem));

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
			(*PanelItemW)[i].CustomColumnData = (wchar_t **)malloc(PanelItemA[i].CustomColumnNumber*sizeof(wchar_t *));

			for (int j=0; j<PanelItemA[i].CustomColumnNumber; j++)
				(*PanelItemW)[i].CustomColumnData[j] = AnsiToUnicode(PanelItemA[i].CustomColumnData[j]);
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

void ConvertPanelItemW(const PluginPanelItem *PanelItemW, oldfar::PluginPanelItem **PanelItemA, int ItemsNumber)
{
	*PanelItemA = (oldfar::PluginPanelItem *)malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem));

	memset(*PanelItemA,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

	for (int i=0; i<ItemsNumber; i++)
	{
		(*PanelItemA)[i].Flags = PanelItemW[i].Flags;
		(*PanelItemA)[i].NumberOfLinks = PanelItemW[i].NumberOfLinks;

		if (PanelItemW[i].Description)
			(*PanelItemA)[i].Description = UnicodeToAnsi(PanelItemW[i].Description);

		if (PanelItemW[i].Owner)
			(*PanelItemA)[i].Owner = UnicodeToAnsi(PanelItemW[i].Owner);

		if (PanelItemW[i].CustomColumnNumber)
		{
			(*PanelItemA)[i].CustomColumnNumber = PanelItemW[i].CustomColumnNumber;
			(*PanelItemA)[i].CustomColumnData = (char **)malloc(PanelItemW[i].CustomColumnNumber*sizeof(char *));

			for (int j=0; j<PanelItemW[i].CustomColumnNumber; j++)
				(*PanelItemA)[i].CustomColumnData[j] = UnicodeToAnsi(PanelItemW[i].CustomColumnData[j]);
		}

		(*PanelItemA)[i].UserData = PanelItemW[i].UserData;
		(*PanelItemA)[i].CRC32 = PanelItemW[i].CRC32;

		(*PanelItemA)[i].FindData.dwFileAttributes = PanelItemW[i].FindData.dwFileAttributes;
		(*PanelItemA)[i].FindData.ftCreationTime = PanelItemW[i].FindData.ftCreationTime;
		(*PanelItemA)[i].FindData.ftLastAccessTime = PanelItemW[i].FindData.ftLastAccessTime;
		(*PanelItemA)[i].FindData.ftLastWriteTime = PanelItemW[i].FindData.ftLastWriteTime;
		(*PanelItemA)[i].FindData.nFileSizeLow = (DWORD)PanelItemW[i].FindData.nFileSize;
		(*PanelItemA)[i].FindData.nFileSizeHigh = (DWORD)(PanelItemW[i].FindData.nFileSize>>32);
		(*PanelItemA)[i].PackSize = (DWORD)PanelItemW[i].FindData.nPackSize;
		(*PanelItemA)[i].PackSizeHigh = (DWORD)(PanelItemW[i].FindData.nPackSize>>32);
		UnicodeToAnsi(PanelItemW[i].FindData.lpwszFileName,(*PanelItemA)[i].FindData.cFileName,sizeof((*PanelItemA)[i].FindData.cFileName));
		UnicodeToAnsi(PanelItemW[i].FindData.lpwszAlternateFileName,(*PanelItemA)[i].FindData.cAlternateFileName,sizeof((*PanelItemA)[i].FindData.cAlternateFileName));
	}
}

void FreePanelItemW(PluginPanelItem *PanelItem, int ItemsNumber)
{
	for (int i=0; i<ItemsNumber; i++)
	{
		if (PanelItem[i].Description)
			free(PanelItem[i].Description);

		if (PanelItem[i].Owner)
			free(PanelItem[i].Owner);

		if (PanelItem[i].CustomColumnNumber)
		{
			for (int j=0; j<PanelItem[i].CustomColumnNumber; j++)
				free(PanelItem[i].CustomColumnData[j]);

			free(PanelItem[i].CustomColumnData);
		}

		free(PanelItem[i].FindData.lpwszFileName);
		free(PanelItem[i].FindData.lpwszAlternateFileName);
	}

	free(PanelItem);
}

void FreePanelItemA(oldfar::PluginPanelItem *PanelItem, int ItemsNumber)
{
	for (int i=0; i<ItemsNumber; i++)
	{
		if (PanelItem[i].Description)
			free(PanelItem[i].Description);

		if (PanelItem[i].Owner)
			free(PanelItem[i].Owner);

		if (PanelItem[i].CustomColumnNumber)
		{
			for (int j=0; j<PanelItem[i].CustomColumnNumber; j++)
				free(PanelItem[i].CustomColumnData[j]);

			free(PanelItem[i].CustomColumnData);
		}
	}

	free(PanelItem);
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
    if (*Path=='\\' || *Path=='/' || *Path==':' && Path==NamePtr+1)
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
      ! ������ �㭪�� ࠡ�⠥� � ������ ������ ᫥襩, ⠪�� �ந�室��
        ��������� 㦥 �������饣� ����筮�� ᫥� �� ⠪��, �����
        ����砥��� ��.
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
     if (*end!='\\' && *end!='/')
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
	if (p) free(p);
	return ret;
}

char* WINAPI PasteFromClipboardA(void)
{
	wchar_t *p = PasteFromClipboard();
	if (p)
		return UnicodeToAnsi(p);
	return NULL;
}

void WINAPI DeleteBufferA(char *Buffer)
{
	if(Buffer) free(Buffer);
}

int WINAPI ProcessNameA(const char *Param1,char *Param2,DWORD Flags)
{
	string strP1(Param1), strP2(Param2);
	int size = (int)(strP1.GetLength()+strP2.GetLength()+NM)+1; //� �७ ��� ��� 㣠���� ᪮�� ⠬ ��� Param2
	wchar_t *p=(wchar_t *)malloc(size*sizeof(wchar_t));
	wcscpy(p,strP2);
	int ret = ProcessName(strP1,p,size,Flags);
	UnicodeToAnsi(p,Param2);
	return ret;
}

int WINAPI KeyNameToKeyA(const char *Name)
{
	string strN(Name);
	return KeyNameToKey(strN);
}

BOOL WINAPI FarKeyToNameA(int Key,char *KeyText,int Size)
{
	string strKT;
	int ret=KeyToText(Key,strKT);
	if (ret)
		strKT.GetCharString(KeyText,Size>0?Size:32);
	return ret;
}

char* WINAPI FarMkTempA(char *Dest, const char *Prefix)
{
	string strP((Prefix?Prefix:""));
	wchar_t D[NM] = {0};

	FarMkTemp(D,sizeof(D),strP);

	UnicodeToAnsi(D,Dest);
	return Dest;
}

int WINAPI FarMkLinkA(const char *Src,const char *Dest, DWORD Flags)
{
	string s(Src), d(Dest);

	int flg=0;
	switch(Flags&0xf)
	{
		case oldfar::FLINK_HARDLINK: flg = FLINK_HARDLINK; break;
		case oldfar::FLINK_SYMLINK:  flg = FLINK_SYMLINK;  break;
		case oldfar::FLINK_VOLMOUNT: flg = FLINK_VOLMOUNT; break;
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

DWORD WINAPI ExpandEnvironmentStrA(const char *src, char *dest, size_t size)
{
	string strS(src), strD;

	apiExpandEnvironmentStrings(strS,strD);
	DWORD len = (DWORD)min(strD.GetLength(),size-1);

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
	if (DestText)
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
		p = (wchar_t **)AnsiToUnicode((const char *)Items);
	}
	else
	{
		c = ItemsNumber;
		p = (wchar_t **)malloc(c*sizeof(wchar_t*));
		for (int i=0; i<c; i++)
			p[i] = AnsiToUnicode(Items[i]);
	}

	int ret = FarMessageFn(PluginNumber,Flags,(HelpTopic?(const wchar_t *)strHT:NULL),p,ItemsNumber,ButtonsNumber);

	for (int i=0; i<c; i++)
		free(p[i]);
	free(p);

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

	FarMenuItemEx *mi = (FarMenuItemEx *)malloc(ItemsNumber*sizeof(*mi));

	if (Flags&FMENU_USEEXT)
	{
		oldfar::FarMenuItemEx *p = (oldfar::FarMenuItemEx *)Item;

		for (int i=0; i<ItemsNumber; i++)
		{
			mi[i].Flags = p[i].Flags;
			mi[i].Text = AnsiToUnicode(p[i].Flags&MIF_USETEXTPTR?p[i].Text.TextPtr:p[i].Text.Text);
			mi[i].AccelKey = p[i].AccelKey;
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
					OemToCharBuffW((const char*)&Item[i].Checked,(wchar_t*)&mi[i].Flags,1);
			}
			if (Item[i].Separator)
				mi[i].Flags|=MIF_SEPARATOR;
			mi[i].Text = AnsiToUnicode(Item[i].Text);
			mi[i].AccelKey = 0;
			mi[i].Reserved = 0;
			mi[i].UserData = 0;
		}
	}

	int ret = FarMenuFn(PluginNumber,X,Y,MaxHeight,Flags|FMENU_USEEXT,(Title?(const wchar_t *)strT:NULL),(Bottom?(const wchar_t *)strB:NULL),(HelpTopic?(const wchar_t *)strHT:NULL),BreakKeys,BreakCode,(FarMenuItem *)mi,ItemsNumber);

	for (int i=0; i<ItemsNumber; i++)
		if (mi[i].Text) free((wchar_t *)mi[i].Text);
	if (mi) free(mi);

	return ret;
}

//BUGBUG
LONG_PTR DlgProcs[512];
HANDLE hDlgs[512];
static int CurrentDlg=0;

LONG_PTR WINAPI CurrentDlgProc(int Msg, int Param1, LONG_PTR Param2)
{
	FARWINDOWPROC Proc = (FARWINDOWPROC)DlgProcs[CurrentDlg];
	return Proc(hDlgs[CurrentDlg], Msg, Param1, Param2);
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

void AnsiListItemToUnicode(oldfar::FarListItem* liA, FarListItem* li)
{
	OEMToUnicode(liA->Text, li->Text, sizeof(liA->Text)-1);
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

void AnsiDialogItemToUnicode(oldfar::FarDialogItem *diA, FarDialogItem *di, FarList *l)
{
	di->Type=0;
	switch(diA->Type)
	{
		case oldfar::DI_TEXT:       di->Type=DI_TEXT;        break;
		case oldfar::DI_VTEXT:      di->Type=DI_VTEXT;       break;
		case oldfar::DI_SINGLEBOX:  di->Type=DI_SINGLEBOX;   break;
		case oldfar::DI_DOUBLEBOX:  di->Type=DI_DOUBLEBOX;   break;
		case oldfar::DI_EDIT:       di->Type=DI_EDIT;        break;
		case oldfar::DI_PSWEDIT:    di->Type=DI_PSWEDIT;     break;
		case oldfar::DI_FIXEDIT:    di->Type=DI_FIXEDIT;     break;
		case oldfar::DI_BUTTON:     di->Type=DI_BUTTON;      break;
		case oldfar::DI_CHECKBOX:   di->Type=DI_CHECKBOX;    break;
		case oldfar::DI_RADIOBUTTON:di->Type=DI_RADIOBUTTON; break;
		case oldfar::DI_COMBOBOX:   di->Type=DI_COMBOBOX;    break;
		case oldfar::DI_LISTBOX:    di->Type=DI_LISTBOX;     break;
#ifdef FAR_USE_INTERNALS
		case oldfar::DI_MEMOEDIT:   di->Type=DI_MEMOEDIT;    break;
#endif // END FAR_USE_INTERNALS
		case oldfar::DI_USERCONTROL:di->Type=DI_USERCONTROL; break;
	}

	di->X1=diA->X1;
	di->Y1=diA->Y1;
	di->X2=diA->X2;
	di->Y2=diA->Y2;

	di->Focus=diA->Focus;

	switch(di->Type)
	{
		case DI_LISTBOX:
		case DI_COMBOBOX:
			{
				if (diA->Param.ListItems)
				{
					l->Items = (FarListItem *)malloc(diA->Param.ListItems->ItemsNumber*sizeof(FarListItem));
					l->ItemsNumber = diA->Param.ListItems->ItemsNumber;
					for(int j = 0;j<l->ItemsNumber;j++)
						AnsiListItemToUnicode(&diA->Param.ListItems->Items[j], &l->Items[j]);
					di->Param.ListItems=l;
				}
			}
			break;
		case DI_USERCONTROL:
			di->Param.VBuf=diA->Param.VBuf;
			break;
		case DI_EDIT:
		case DI_FIXEDIT:
			{
				if (diA->Flags&oldfar::DIF_HISTORY)
					di->Param.History=AnsiToUnicode(diA->Param.History);
				else if (diA->Flags&oldfar::DIF_MASKEDIT)
					di->Param.Mask=AnsiToUnicode(diA->Param.Mask);
			}
			break;
		default:
			di->Param.Selected=diA->Param.Selected;
	}

	di->Flags=0;
	if(diA->Flags)
	{
		if (diA->Flags&oldfar::DIF_SETCOLOR)
			di->Flags|=DIF_SETCOLOR|(diA->Flags&DIF_COLORMASK);
		if (diA->Flags&oldfar::DIF_BOXCOLOR)         di->Flags|=DIF_BOXCOLOR;
		if (diA->Flags&oldfar::DIF_GROUP)            di->Flags|=DIF_GROUP;
		if (diA->Flags&oldfar::DIF_LEFTTEXT)         di->Flags|=DIF_LEFTTEXT;
		if (diA->Flags&oldfar::DIF_MOVESELECT)       di->Flags|=DIF_MOVESELECT;
		if (diA->Flags&oldfar::DIF_SHOWAMPERSAND)    di->Flags|=DIF_SHOWAMPERSAND;
		if (diA->Flags&oldfar::DIF_CENTERGROUP)      di->Flags|=DIF_CENTERGROUP;
		if (diA->Flags&oldfar::DIF_NOBRACKETS)       di->Flags|=DIF_NOBRACKETS;
		if (diA->Flags&oldfar::DIF_MANUALADDHISTORY) di->Flags|=DIF_MANUALADDHISTORY;
		if (diA->Flags&oldfar::DIF_SEPARATOR)        di->Flags|=DIF_SEPARATOR;
		if (diA->Flags&oldfar::DIF_SEPARATOR2)       di->Flags|=DIF_SEPARATOR2;
		if (diA->Flags&oldfar::DIF_EDITOR)           di->Flags|=DIF_EDITOR;
		if (diA->Flags&oldfar::DIF_LISTNOAMPERSAND)  di->Flags|=DIF_LISTNOAMPERSAND;
		if (diA->Flags&oldfar::DIF_LISTNOBOX)        di->Flags|=DIF_LISTNOBOX;
		if (diA->Flags&oldfar::DIF_HISTORY)          di->Flags|=DIF_HISTORY;
		if (diA->Flags&oldfar::DIF_BTNNOCLOSE)       di->Flags|=DIF_BTNNOCLOSE;
		if (diA->Flags&oldfar::DIF_CENTERTEXT)       di->Flags|=DIF_CENTERTEXT;
		if (diA->Flags&oldfar::DIF_NOTCVTUSERCONTROL)di->Flags|=DIF_NOTCVTUSERCONTROL;
#ifdef FAR_USE_INTERNALS
		if (diA->Flags&oldfar::DIF_SEPARATORUSER)    di->Flags|=DIF_SEPARATORUSER;
#endif // END FAR_USE_INTERNALS
		if (diA->Flags&oldfar::DIF_EDITEXPAND)       di->Flags|=DIF_EDITEXPAND;
		if (diA->Flags&oldfar::DIF_DROPDOWNLIST)     di->Flags|=DIF_DROPDOWNLIST;
		if (diA->Flags&oldfar::DIF_USELASTHISTORY)   di->Flags|=DIF_USELASTHISTORY;
		if (diA->Flags&oldfar::DIF_MASKEDIT)         di->Flags|=DIF_MASKEDIT;
		if (diA->Flags&oldfar::DIF_SELECTONENTRY)    di->Flags|=DIF_SELECTONENTRY;
		if (diA->Flags&oldfar::DIF_3STATE)           di->Flags|=DIF_3STATE;
#ifdef FAR_USE_INTERNALS
		if (diA->Flags&oldfar::DIF_EDITPATH)         di->Flags|=DIF_EDITPATH;
#endif // END FAR_USE_INTERNALS
		if (diA->Flags&oldfar::DIF_LISTWRAPMODE)     di->Flags|=DIF_LISTWRAPMODE;
		if (diA->Flags&oldfar::DIF_LISTAUTOHIGHLIGHT)di->Flags|=DIF_LISTAUTOHIGHLIGHT;
#ifdef FAR_USE_INTERNALS
		if (diA->Flags&oldfar::DIF_AUTOMATION)       di->Flags|=DIF_AUTOMATION;
#endif // END FAR_USE_INTERNALS
		if (diA->Flags&oldfar::DIF_HIDDEN)           di->Flags|=DIF_HIDDEN;
		if (diA->Flags&oldfar::DIF_READONLY)         di->Flags|=DIF_READONLY;
		if (diA->Flags&oldfar::DIF_NOFOCUS)          di->Flags|=DIF_NOFOCUS;
		if (diA->Flags&oldfar::DIF_DISABLE)          di->Flags|=DIF_DISABLE;
	}

	di->DefaultButton=diA->DefaultButton;

	if ((diA->Type==oldfar::DI_EDIT || diA->Type==oldfar::DI_COMBOBOX) && diA->Flags&oldfar::DIF_VAREDIT)
    di->DataIn = AnsiToUnicode(diA->Data.Ptr.PtrData);
	else
    di->DataIn = AnsiToUnicode(diA->Data.Data);
}

void FreeUnicodeDialog(FarDialogItem *di, FarList *l)
{
		if((di->Type==DI_EDIT || di->Type==DI_FIXEDIT) && (di->Flags&DIF_HISTORY) || (di->Flags&DIF_MASKEDIT))
			if(di->Param.History) free((wchar_t *)di->Param.History);

		if(l->Items) free(l->Items);

    if (di->DataOut) free(di->DataOut);
}

void UnicodeDialogItemToAnsi(FarDialogItem *di, oldfar::FarDialogItem *diA)
{
	diA->Type=0;
	switch(di->Type)
	{
		case DI_TEXT:       diA->Type=oldfar::DI_TEXT;        break;
		case DI_VTEXT:      diA->Type=oldfar::DI_VTEXT;       break;
		case DI_SINGLEBOX:  diA->Type=oldfar::DI_SINGLEBOX;   break;
		case DI_DOUBLEBOX:  diA->Type=oldfar::DI_DOUBLEBOX;   break;
		case DI_EDIT:       diA->Type=oldfar::DI_EDIT;        break;
		case DI_PSWEDIT:    diA->Type=oldfar::DI_PSWEDIT;     break;
		case DI_FIXEDIT:    diA->Type=oldfar::DI_FIXEDIT;     break;
		case DI_BUTTON:     diA->Type=oldfar::DI_BUTTON;      break;
		case DI_CHECKBOX:   diA->Type=oldfar::DI_CHECKBOX;    break;
		case DI_RADIOBUTTON:diA->Type=oldfar::DI_RADIOBUTTON; break;
		case DI_COMBOBOX:   diA->Type=oldfar::DI_COMBOBOX;    break;
		case DI_LISTBOX:    diA->Type=oldfar::DI_LISTBOX;     break;
#ifdef FAR_USE_INTERNALS
		case DI_MEMOEDIT:   diA->Type=oldfar::DI_MEMOEDIT;    break;
#endif // END FAR_USE_INTERNALS
		case DI_USERCONTROL:diA->Type=oldfar::DI_USERCONTROL; break;
	}

	diA->X1=di->X1;
	diA->Y1=di->Y1;
	diA->X2=di->X2;
	diA->Y2=di->Y2;

	diA->Focus=di->Focus;

	switch(diA->Type)
	{
		case oldfar::DI_USERCONTROL:
			//BUGBUG
			if(di->Param.VBuf) diA->Param.VBuf = di->Param.VBuf;
			break;
/*
		case oldfar::DI_LISTBOX:
		case oldfar::DI_COMBOBOX:
			{
				l->Items = (FarListItem *)malloc(diA->Param.ListItems->ItemsNumber*sizeof(FarListItem));
				l->ItemsNumber = diA->Param.ListItems->ItemsNumber;
				for(int j = 0;j<l->ItemsNumber;j++)
					UnicodeListItemToAnsi(&l->Items[j],&diA->Param.ListItems->Items[j]);
				di->Param.ListItems=l;
			}
			break;
*/
		case oldfar::DI_EDIT:
		case oldfar::DI_FIXEDIT:
			{
				if (di->Flags&DIF_HISTORY)
					diA->Param.History=UnicodeToAnsi(di->Param.History);
				else if (di->Flags&DIF_MASKEDIT)
					diA->Param.Mask=UnicodeToAnsi(di->Param.Mask);
			}
			break;
		default:
			diA->Param.Selected=di->Param.Selected;
	}

	diA->Flags=0;
	if(di->Flags)
	{
		if (di->Flags&DIF_SETCOLOR)
			diA->Flags|=oldfar::DIF_SETCOLOR|(di->Flags&oldfar::DIF_COLORMASK);
		if (di->Flags&DIF_BOXCOLOR)         diA->Flags|=oldfar::DIF_BOXCOLOR;
		if (di->Flags&DIF_GROUP)            diA->Flags|=oldfar::DIF_GROUP;
		if (di->Flags&DIF_LEFTTEXT)         diA->Flags|=oldfar::DIF_LEFTTEXT;
		if (di->Flags&DIF_MOVESELECT)       diA->Flags|=oldfar::DIF_MOVESELECT;
		if (di->Flags&DIF_SHOWAMPERSAND)    diA->Flags|=oldfar::DIF_SHOWAMPERSAND;
		if (di->Flags&DIF_CENTERGROUP)      diA->Flags|=oldfar::DIF_CENTERGROUP;
		if (di->Flags&DIF_NOBRACKETS)       diA->Flags|=oldfar::DIF_NOBRACKETS;
		if (di->Flags&DIF_MANUALADDHISTORY) diA->Flags|=oldfar::DIF_MANUALADDHISTORY;
		if (di->Flags&DIF_SEPARATOR)        diA->Flags|=oldfar::DIF_SEPARATOR;
		if (di->Flags&DIF_SEPARATOR2)       diA->Flags|=oldfar::DIF_SEPARATOR2;
		if (di->Flags&DIF_EDITOR)           diA->Flags|=oldfar::DIF_EDITOR;
		if (di->Flags&DIF_LISTNOAMPERSAND)  diA->Flags|=oldfar::DIF_LISTNOAMPERSAND;
		if (di->Flags&DIF_LISTNOBOX)        diA->Flags|=oldfar::DIF_LISTNOBOX;
		if (di->Flags&DIF_HISTORY)          diA->Flags|=oldfar::DIF_HISTORY;
		if (di->Flags&DIF_BTNNOCLOSE)       diA->Flags|=oldfar::DIF_BTNNOCLOSE;
		if (di->Flags&DIF_CENTERTEXT)       diA->Flags|=oldfar::DIF_CENTERTEXT;
		if (di->Flags&DIF_NOTCVTUSERCONTROL)diA->Flags|=oldfar::DIF_NOTCVTUSERCONTROL;
#ifdef FAR_USE_INTERNALS
		if (di->Flags&DIF_SEPARATORUSER)    diA->Flags|=oldfar::DIF_SEPARATORUSER;
#endif // END FAR_USE_INTERNALS
		if (di->Flags&DIF_EDITEXPAND)       diA->Flags|=oldfar::DIF_EDITEXPAND;
		if (di->Flags&DIF_DROPDOWNLIST)     diA->Flags|=oldfar::DIF_DROPDOWNLIST;
		if (di->Flags&DIF_USELASTHISTORY)   diA->Flags|=oldfar::DIF_USELASTHISTORY;
		if (di->Flags&DIF_MASKEDIT)         diA->Flags|=oldfar::DIF_MASKEDIT;
		if (di->Flags&DIF_SELECTONENTRY)    diA->Flags|=oldfar::DIF_SELECTONENTRY;
		if (di->Flags&DIF_3STATE)           diA->Flags|=oldfar::DIF_3STATE;
#ifdef FAR_USE_INTERNALS
		if (di->Flags&DIF_EDITPATH)         diA->Flags|=oldfar::DIF_EDITPATH;
#endif // END FAR_USE_INTERNALS
		if (di->Flags&DIF_LISTWRAPMODE)     diA->Flags|=oldfar::DIF_LISTWRAPMODE;
		if (di->Flags&DIF_LISTAUTOHIGHLIGHT)diA->Flags|=oldfar::DIF_LISTAUTOHIGHLIGHT;
#ifdef FAR_USE_INTERNALS
		if (di->Flags&DIF_AUTOMATION)       diA->Flags|=oldfar::DIF_AUTOMATION;
#endif // END FAR_USE_INTERNALS
		if (di->Flags&DIF_HIDDEN)           diA->Flags|=oldfar::DIF_HIDDEN;
		if (di->Flags&DIF_READONLY)         diA->Flags|=oldfar::DIF_READONLY;
		if (di->Flags&DIF_NOFOCUS)          diA->Flags|=oldfar::DIF_NOFOCUS;
		if (di->Flags&DIF_DISABLE)          diA->Flags|=oldfar::DIF_DISABLE;
	}

	diA->DefaultButton=di->DefaultButton;

	if ((diA->Type==oldfar::DI_EDIT || diA->Type==oldfar::DI_COMBOBOX) && diA->Flags&oldfar::DIF_VAREDIT)
    UnicodeToAnsi(di->DataIn,diA->Data.Ptr.PtrData,diA->Data.Ptr.PtrLength-1);
	else
    UnicodeToAnsi(di->DataIn,diA->Data.Data,sizeof(diA->Data.Data)-1);
}

LONG_PTR WINAPI DlgProcA(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	switch(Msg)
	{
		case DN_LISTHOTKEY:
		case DN_FIRST:
		case DN_BTNCLICK:
		case DN_CTLCOLORDIALOG:
		case DN_CTLCOLORDLGITEM:
		case DN_CTLCOLORDLGLIST:
		case DN_DRAWDIALOG:
			break;
		case DN_DRAWDLGITEM:
		case DN_EDITCHANGE:
			/*if (Param2)
			{
				oldfar::FarDialogItem diA;
				FarDialogItem *di = (FarDialogItem *)Param2;
				UnicodeDialogItemToAnsi(di,&diA);
				Param2 = (LONG_PTR)&diA;
			}*/
			break;
		case DN_ENTERIDLE:
		case DN_GOTFOCUS:
			break;

		case DN_HELP:
			{
				char *topic = UnicodeToAnsi((const wchar_t *)Param2);
				Param2 = (LONG_PTR)topic;
				LONG_PTR ret = CurrentDlgProc(Msg, Param1, Param2);
				free (topic);
				return ret;
			}

		case DN_HOTKEY:
			break;

		case DN_INITDIALOG:
			hDlgs[CurrentDlg] = hDlg;
			break;

		case DN_KILLFOCUS:
		case DN_LISTCHANGE:
		case DN_MOUSECLICK:
		case DN_DRAGGED:
		case DN_RESIZECONSOLE:
		case DN_MOUSEEVENT:
		case DN_DRAWDIALOGDONE:

#ifdef FAR_USE_INTERNALS
		case DM_KILLSAVESCREEN:
		case DM_ALLKEYMODE:
		case DN_ACTIVATEAPP:
#endif // END FAR_USE_INTERNALS
			break;

		case DN_KEY:
			if (Param2&EXTENDED_KEY_BASE) Param2=Param2^EXTENDED_KEY_BASE|0x100;
			if (Param2&INTERNAL_KEY_BASE) Param2=Param2^INTERNAL_KEY_BASE|0x200;
	}
	return CurrentDlgProc(Msg, Param1, Param2);
}

LONG_PTR WINAPI FarDefDlgProcA(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	return FarDefDlgProc(hDlg, Msg, Param1, Param2);
}

LONG_PTR WINAPI FarSendDlgMessageA(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	switch(Msg)
	{
		case oldfar::DM_FIRST:
		case oldfar::DM_CLOSE:
		case oldfar::DM_ENABLE:
		case oldfar::DM_ENABLEREDRAW:
		case oldfar::DM_GETDLGDATA:
			break;

		case oldfar::DM_GETDLGITEM:
			{
				FarDialogItem di;
				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_GETDLGITEM, Param1, (LONG_PTR)&di);

				oldfar::FarDialogItem *diA = (oldfar::FarDialogItem *)Param2;

				UnicodeDialogItemToAnsi(&di,diA);
        if(!di.MaxLen && IsEdit(di.Type) && di.DataOut) {
          REALLOC ra = (REALLOC)FarSendDlgMessage(hDlg, DM_GETREALLOC, 0, 0);
          if(ra)  // PARANOID
            ra(di.DataOut, 0);
        }
				return ret;
			}

		case oldfar::DM_GETDLGRECT:
			break;

		case oldfar::DM_GETTEXT:
			{
				LONG_PTR length = FarSendDlgMessage(hDlg, DM_GETTEXT, Param1, 0);

				if (!Param2) return length;

				if (!length) return 0;

				wchar_t* text = (wchar_t*) malloc((length+1)*sizeof(wchar_t));
				oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;

				//BUGBUG: ���� didA->PtrLength=0, �� ������� � ������ '\0', � ��� ��������, ��� ���, �� ��� ���������.
				FarDialogItemData did = {didA->PtrLength, text};

				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_GETTEXT, Param1, (LONG_PTR)&did);
        didA->PtrLength = (unsigned)did.PtrLength;
				UnicodeToAnsi(text,didA->PtrData);
				free(text);
				return ret;
			}

		case oldfar::DM_GETTEXTLENGTH:
			return FarSendDlgMessage(hDlg, DM_GETTEXTLENGTH, Param1, 0);

		case oldfar::DM_KEY:
			{
				if(!Param1 || !Param2) return FALSE;
				int Count = (int)Param1;
				DWORD* KeysA = (DWORD*)Param2;
				DWORD* KeysW = (DWORD*)malloc(Count*sizeof(DWORD));
				for(int i=0;i<Count;i++)
				{
					if (KeysA[i]&0x100) KeysW[i]=KeysA[i]^0x100|EXTENDED_KEY_BASE;
					else if (KeysA[i]&0x200) KeysW[i]=KeysA[i]^0x200|INTERNAL_KEY_BASE;
					else KeysW[i] = KeysA[i];
				}
				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_KEY, Param1, (LONG_PTR)KeysW);
				free(KeysW);
				return ret;
			}

		case oldfar::DM_MOVEDIALOG:
		case oldfar::DM_SETDLGDATA:
			break;

		case oldfar::DM_SETDLGITEM:
			{
				oldfar::FarDialogItem *diA = (oldfar::FarDialogItem *)Param2;
				FarDialogItem di;
				FarList l = {0,0};
				AnsiDialogItemToUnicode(diA,&di,&l);
				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_SETDLGITEM, Param1, (LONG_PTR)&di);
				FreeUnicodeDialog(&di,&l);
				return ret;
			}

		case oldfar::DM_SETFOCUS:
		case oldfar::DM_REDRAW:
			break;

		case oldfar::DM_SETTEXT:
			{
				if (!Param2)return 0;
				oldfar::FarDialogItemData* didA = (oldfar::FarDialogItemData*)Param2;
				if (!didA->PtrData) return 0;
				wchar_t* text = AnsiToUnicode(didA->PtrData);

				//BUGBUG - PtrLength �� �� ��� �� ������.
				FarDialogItemData di = {didA->PtrLength,text};

				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_SETTEXT, Param1, (LONG_PTR)&di);
				free (text);
				return ret;
			}

		case oldfar::DM_SETMAXTEXTLENGTH:
		case oldfar::DM_SHOWDIALOG:
		case oldfar::DM_GETFOCUS:
		case oldfar::DM_GETCURSORPOS:
		case oldfar::DM_SETCURSORPOS:
			break;

		case oldfar::DM_GETTEXTPTR:
			{
				LONG_PTR length = FarSendDlgMessage(hDlg, DM_GETTEXTPTR, Param1, 0);
				if (!Param2) return length;

				if (length)
				{
					wchar_t* text = (wchar_t *) malloc ((length +1)* sizeof(wchar_t));
					LONG_PTR ret = FarSendDlgMessage(hDlg, DM_GETTEXTPTR, Param1, (LONG_PTR)text);
					UnicodeToAnsi(text, (char *)Param2);
					free(text);
					return ret;
				}
			}

		case oldfar::DM_SETTEXTPTR:
			{
				if (!Param2) return FALSE;
				wchar_t* text = AnsiToUnicode((char*)Param2);
				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_SETTEXTPTR, Param1, (LONG_PTR)text);
				free (text);
				return ret;
			}

		case oldfar::DM_SHOWITEM:
			break;

		case oldfar::DM_ADDHISTORY:
			{
				if (!Param2) return FALSE;
				wchar_t* history = AnsiToUnicode((char*)Param2);
				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_SETHISTORY, Param1, (LONG_PTR)history);
				free (history);
				return ret;
			}

		case oldfar::DM_GETCHECK:
			{
				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_GETCHECK, Param1, 0);
				LONG_PTR state = 0;
				if(ret&oldfar::BSTATE_UNCHECKED) state|=BSTATE_UNCHECKED;
				if(ret&oldfar::BSTATE_CHECKED)   state|=BSTATE_CHECKED;
				if(ret&oldfar::BSTATE_3STATE)    state|=BSTATE_3STATE;
				return state;
			}

		case oldfar::DM_SETCHECK:
			{
				LONG_PTR state = 0;
				if(Param2&oldfar::BSTATE_UNCHECKED) state|=BSTATE_UNCHECKED;
				if(Param2&oldfar::BSTATE_CHECKED)   state|=BSTATE_CHECKED;
				if(Param2&oldfar::BSTATE_3STATE)    state|=BSTATE_3STATE;
				if(Param2&oldfar::BSTATE_TOGGLE)    state|=BSTATE_TOGGLE;
				return FarSendDlgMessage(hDlg, DM_SETCHECK, Param1, state);
			}

		case oldfar::DM_SET3STATE:
		case oldfar::DM_LISTSORT:
			break;

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
				oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
				FarListPos lp = {lpA->SelectPos,lpA->TopPos};
				Param2 = (LONG_PTR)&lp;
				return FarSendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, (LONG_PTR)&lp);
			}
			else return FarSendDlgMessage(hDlg, DM_LISTGETCURPOS, Param1, 0);

		case oldfar::DM_LISTSETCURPOS:
			{
				if(!Param2) return FALSE;
				oldfar::FarListPos *lpA = (oldfar::FarListPos *)Param2;
				FarListPos lp = {lpA->SelectPos,lpA->TopPos};
				Param2 = (LONG_PTR)&lp;
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
				return FarSendDlgMessage(hDlg, DM_LISTSETCURPOS, Param1, Param2?(LONG_PTR)&ld:0);
			}

		case oldfar::DM_LISTADD:
		case oldfar::DM_LISTADDSTR:
		case oldfar::DM_LISTUPDATE:
		case oldfar::DM_LISTINSERT:
		case oldfar::DM_LISTFINDSTRING:
			return 0; //BUGBUG

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

		case oldfar::DM_LISTGETDATA:
		case oldfar::DM_LISTSETDATA:
			return 0; //BUGBUG

		case oldfar::DM_LISTSETTITLES:
			{
				if (!Param2) return FALSE;
				oldfar::FarListTitles *ltA = (oldfar::FarListTitles *)Param2;
				FarListTitles lt = {0,ltA->Title!=NULL?AnsiToUnicode(ltA->Title):NULL,0,ltA->Bottom!=NULL?AnsiToUnicode(ltA->Bottom):NULL};
				Param2 = (LONG_PTR)&lt;
				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_LISTSETTITLES, Param1, Param2);
				if (lt.Bottom) free ((wchar_t *)lt.Bottom);
				if (lt.Title) free ((wchar_t *)lt.Title);
				return ret;
			}

		case oldfar::DM_LISTGETTITLES:
			{
				//BUGBUG ���� ᤥ���� ����� DM_LISTGETTITLES �㤥� ࠡ���� � ᠬ�� ��
				return FALSE;
			}

		case oldfar::DM_RESIZEDIALOG:
		case oldfar::DM_SETITEMPOSITION:
		case oldfar::DM_GETDROPDOWNOPENED:
		case oldfar::DM_SETDROPDOWNOPENED:
			break;

		case oldfar::DM_SETHISTORY:
			if(!Param2) return FarSendDlgMessage(hDlg, DM_SETHISTORY, Param1, 0);
			else
			{
				wchar_t *history = AnsiToUnicode((const char *)Param2);
				LONG_PTR ret = FarSendDlgMessage(hDlg, DM_SETHISTORY, Param1, (LONG_PTR)history);
				free(history);
				return ret;
			}

		case oldfar::DM_GETITEMPOSITION:
		case oldfar::DM_SETMOUSEEVENTNOTIFY:
		case oldfar::DM_EDITUNCHANGEDFLAG:
		case oldfar::DM_GETITEMDATA:
		case oldfar::DM_SETITEMDATA:
			break;

		case oldfar::DM_LISTSET:
			return 0; //BUGBUG

		case oldfar::DM_LISTSETMOUSEREACTION:
			{
				LONG_PTR type=0;
				if (Param2&oldfar::LMRT_ONLYFOCUS) type|=LMRT_ONLYFOCUS;
				if (Param2&oldfar::LMRT_ALWAYS)    type|=LMRT_ALWAYS;
				if (Param2&oldfar::LMRT_NEVER)     type|=LMRT_NEVER;
				return FarSendDlgMessage(hDlg, DM_LISTSETMOUSEREACTION, Param1, type);
			}

		case oldfar::DM_GETCURSORSIZE:
		case oldfar::DM_SETCURSORSIZE:
		case oldfar::DM_LISTGETDATASIZE:
			break;

		case oldfar::DM_GETSELECTION:
			{
				if (!Param2) return FALSE;
				oldfar::EditorSelect *esA = (oldfar::EditorSelect *)Param2;
				EditorSelect es;
				es.BlockType      = esA->BlockType;
				es.BlockStartLine = esA->BlockStartLine;
				es.BlockStartPos  = esA->BlockStartPos;
				es.BlockWidth     = esA->BlockWidth;
				es.BlockHeight    = esA->BlockHeight;
				return FarSendDlgMessage(hDlg, DM_GETSELECTION, Param1, (LONG_PTR)&es);
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

		case DM_USER:

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

	FarList *l = (FarList *)malloc(ItemsNumber*sizeof(FarList));
	memset(l,0,ItemsNumber*sizeof(FarList));
	FarDialogItem *di = (FarDialogItem *)malloc(ItemsNumber*sizeof(*di));

	for (int i=0; i<ItemsNumber; i++)
	{
		AnsiDialogItemToUnicode(&Item[i],&di[i],&l[i]);
	}

	DWORD DlgFlags = 0;

	if (Flags&oldfar::FDLG_WARNING)      DlgFlags|=FDLG_WARNING;
	if (Flags&oldfar::FDLG_SMALLDIALOG)  DlgFlags|=FDLG_SMALLDIALOG;
	if (Flags&oldfar::FDLG_NODRAWSHADOW) DlgFlags|=FDLG_NODRAWSHADOW;
	if (Flags&oldfar::FDLG_NODRAWPANEL)  DlgFlags|=FDLG_NODRAWPANEL;
#ifdef FAR_USE_INTERNALS
	if (Flags&oldfar::FDLG_NONMODAL)     DlgFlags|=FDLG_NONMODAL;
#endif // END FAR_USE_INTERNALS

	CurrentDlg++;
	DlgProcs[CurrentDlg] = (LONG_PTR)DlgProc;
  int ret = FarDialogEx(PluginNumber, X1, Y1, X2, Y2, (HelpTopic?(const wchar_t *)strHT:NULL), (FarDialogItem *)di, ItemsNumber, 0, DlgFlags, DlgProc?DlgProcA:0, Param, realloc);
	CurrentDlg--;

	for (int i=0; i<ItemsNumber; i++)
	{
		Item[i].Param.Selected = di[i].Param.Selected;
    wchar_t *res = di[i].DataOut;
    if (!res) res = IsEdit(di[i].Type) ? L"" : di[i].DataIn;
		if ((di[i].Type==DI_EDIT || di[i].Type==DI_COMBOBOX) && Item[i].Flags&oldfar::DIF_VAREDIT)
      UnicodeToAnsi(res, Item[i].Data.Ptr.PtrData, Item[i].Data.Ptr.PtrLength);
		else
      UnicodeToAnsi(res, Item[i].Data.Data, sizeof(Item[i].Data.Data)-1);

		FreeUnicodeDialog(&di[i],&l[i]);
	}

	if (di) free(di);
	if (l) free(l);
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
		PIA->PanelItems = NULL; //BUGBUG
		PIA->SelectedItems = NULL; //BUGBUG
	}

	PIA->CurrentItem = PIW->CurrentItem;
	PIA->TopPanelItem = PIW->TopPanelItem;

	PIA->Visible = PIW->Visible;
	PIA->Focus = PIW->Focus;
	PIA->ViewMode = PIW->ViewMode;

	UnicodeToAnsi(PIW->lpwszColumnTypes, PIA->ColumnTypes, sizeof(PIA->ColumnTypes)-1);
	UnicodeToAnsi(PIW->lpwszColumnWidths, PIA->ColumnWidths, sizeof(PIA->ColumnWidths)-1);

	UnicodeToAnsi(PIW->lpwszCurDir, PIA->CurDir, sizeof(PIA->CurDir)-1);

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

	PIA->Reserved = PIW->Reserved;
}

int WINAPI FarControlA(HANDLE hPlugin,int Command,void *Param)
{
	static oldfar::PanelInfo PIA={0};
	HANDLE hPluginW = CURRENT_PANEL;

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
				if (ParamW) free(ParamW);
				return ret;
			}

		case oldfar::FCTL_GETANOTHERPANELINFO:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_GETPANELINFO:
			{
				if(!Param) return FALSE;
				oldfar::PanelInfo *PIA = (oldfar::PanelInfo *)Param;
				PanelInfo PIW;
				int ret = FarControl(hPluginW,FCTL_GETPANELINFO,(void *)&PIW);
				if (ret) ConvertUnicodePanelInfoToAnsi(&PIW, PIA, FALSE);
				return ret;
			}

		case oldfar::FCTL_GETANOTHERPANELSHORTINFO:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_GETPANELSHORTINFO:
			{
				if(!Param) return FALSE;
				oldfar::PanelInfo *PIA = (oldfar::PanelInfo *)Param;
				PanelInfo PIW;
				int ret = FarControl(hPluginW,FCTL_GETPANELSHORTINFO,(void *)&PIW);
				if (ret) ConvertUnicodePanelInfoToAnsi(&PIW, PIA, TRUE);
				return ret;
			}

		case oldfar::FCTL_REDRAWANOTHERPANEL:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_REDRAWPANEL:
			{
				if(!Param) return FarControl(hPluginW, FCTL_REDRAWPANEL, NULL);
				oldfar::PanelRedrawInfo* priA = (oldfar::PanelRedrawInfo*)Param;
				PanelRedrawInfo pri = {priA->CurrentItem,priA->TopPanelItem};
				return FarControl(hPluginW, FCTL_REDRAWPANEL, &pri);
			}

		case oldfar::FCTL_SETANOTHERNUMERICSORT:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_SETNUMERICSORT:
			return FarControl(hPluginW, FCTL_SETNUMERICSORT, Param);

		case oldfar::FCTL_SETANOTHERPANELDIR:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_SETPANELDIR:
			{
				if(!Param) return FALSE;
				wchar_t* Dir = AnsiToUnicode((char*)Param);
				int ret = FarControl(hPluginW, FCTL_SETPANELDIR, Dir);
				free(Dir);
				return ret;
			}

		case oldfar::FCTL_SETANOTHERSELECTION:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_SETSELECTION:
			return FALSE; //BUGBUG

		case oldfar::FCTL_SETANOTHERSORTMODE:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_SETSORTMODE:
			if(!Param) return FALSE;
			return FarControl(hPluginW, FCTL_SETSORTMODE, Param);

		case oldfar::FCTL_SETANOTHERSORTORDER:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_SETSORTORDER:
			return FarControl(hPluginW, FCTL_SETSORTORDER, Param);

		case oldfar::FCTL_SETANOTHERVIEWMODE:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_SETVIEWMODE:
			return FarControl(hPluginW, FCTL_SETVIEWMODE, Param);

		case oldfar::FCTL_UPDATEANOTHERPANEL:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_UPDATEPANEL:
			return FarControl(hPluginW, FCTL_UPDATEPANEL, Param);


		case oldfar::FCTL_GETCMDLINE:
			{
				if(!Param || IsBadWritePtr(Param, sizeof(char) * 1024)) return FALSE;
				wchar_t s[1024];
				int ret = FarControl(hPluginW, FCTL_GETCMDLINE, &s);
				if(ret) UnicodeToAnsi(s, (char*)Param, 1024-1);
				return ret;
			}

		case oldfar::FCTL_GETCMDLINEPOS:
			if(!Param) return FALSE;
			return FarControl(hPluginW,FCTL_GETCMDLINEPOS,Param);

		case oldfar::FCTL_GETCMDLINESELECTEDTEXT:
			{
				if(!Param || IsBadWritePtr(Param, sizeof(char) * 1024)) return FALSE;
				wchar_t s[1024];
				int ret = FarControl(hPluginW, FCTL_GETCMDLINESELECTEDTEXT, &s);
				if(ret) UnicodeToAnsi(s, (char*)Param, 1024-1);
				return ret;
			}

		case oldfar::FCTL_GETCMDLINESELECTION:
			{
				if(!Param) return FALSE;
				CmdLineSelect cls;
				int ret = FarControl(hPluginW, FCTL_GETCMDLINESELECTION, &cls);
				if (ret)
				{
					oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
					clsA->SelStart = cls.SelStart;
					clsA->SelEnd = cls.SelEnd;
				};
				return ret;
			}

		case oldfar::FCTL_INSERTCMDLINE:
			{
				if(!Param) return FALSE;
				wchar_t* s = AnsiToUnicode((const char*)Param);
				int ret = FarControl(hPluginW, FCTL_INSERTCMDLINE, s);
				free(s);
				return ret;
			}

		case oldfar::FCTL_SETCMDLINE:
			{
				if(!Param) return FALSE;
				wchar_t* s = AnsiToUnicode((const char*)Param);
				int ret = FarControl(hPluginW, FCTL_SETCMDLINE, s);
				free(s);
				return ret;
			}

		case oldfar::FCTL_SETCMDLINEPOS:
			if(!Param) return FALSE;
			return FarControl(hPluginW, FCTL_SETCMDLINEPOS, Param);

		case oldfar::FCTL_SETCMDLINESELECTION:
			{
				if(!Param) return FALSE;
				oldfar::CmdLineSelect* clsA = (oldfar::CmdLineSelect*)Param;
				CmdLineSelect cls = {clsA->SelStart,clsA->SelEnd};
				return FarControl(hPluginW, FCTL_SETCMDLINESELECTION, &cls);
			}

		case oldfar::FCTL_GETUSERSCREEN:
			return FarControl(hPluginW, FCTL_GETUSERSCREEN, NULL);

		case oldfar::FCTL_SETUSERSCREEN:
			return FarControl(hPluginW, FCTL_SETUSERSCREEN, NULL);
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
			return FarAdvControl(ModuleNumber, ACTL_GETFARVERSION, Param);

		case oldfar::ACTL_CONSOLEMODE:
			return FarAdvControl(ModuleNumber, ACTL_CONSOLEMODE, Param);

		case oldfar::ACTL_GETSYSWORDDIV:
		{
			INT_PTR Length = FarAdvControl(ModuleNumber, ACTL_GETSYSWORDDIV, NULL);
			if(Param)
			{
				wchar_t *SysWordDiv = (wchar_t*)malloc((Length+1)*sizeof(wchar_t));
				FarAdvControl(ModuleNumber, ACTL_GETSYSWORDDIV, SysWordDiv);
				UnicodeToAnsi(SysWordDiv,(char*)Param,NM-1);
				free (SysWordDiv);
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
					if (ErrMsg1) free(ErrMsg1);
					if (ErrMsg2) free(ErrMsg2);
					if (ErrMsg3) free(ErrMsg3);
					kmA->Param.MacroResult.ErrMsg1 = ErrMsg1 = UnicodeToAnsi(km.Param.MacroResult.ErrMsg1);
					kmA->Param.MacroResult.ErrMsg2 = ErrMsg2 = UnicodeToAnsi(km.Param.MacroResult.ErrMsg2);
					kmA->Param.MacroResult.ErrMsg3 = ErrMsg3 = UnicodeToAnsi(km.Param.MacroResult.ErrMsg3);
					if (km.Param.PlainText.SequenceText)
						free(km.Param.PlainText.SequenceText);
					break;

				case MCMD_COMPILEMACRO:
					if (km.Param.Compile.Sequence)
						free(km.Param.Compile.Sequence);
					break;

				case MCMD_POSTMACROSTRING:
					if (km.Param.PlainText.SequenceText)
						free(km.Param.PlainText.SequenceText);
					break;
			}
			return res;
		}

		case oldfar::ACTL_POSTKEYSEQUENCE:
			{
				if (!Param) return FALSE;
				KeySequence ks;
				oldfar::KeySequence *ksA = (oldfar::KeySequence*)Param;
				ks.Count = ksA->Count;
				if (ksA->Flags&oldfar::KSFLAGS_DISABLEOUTPUT) ks.Flags|=KSFLAGS_DISABLEOUTPUT;
				if (ksA->Flags&oldfar::KSFLAGS_NOSENDKEYSTOPLUGINS) ks.Flags|=KSFLAGS_NOSENDKEYSTOPLUGINS;
				ks.Sequence = ksA->Sequence; //BUGBUG
				for (int i=0;i<ks.Count;i++)
				{
					if (ks.Sequence[i]&0x100) ks.Sequence[i]=ks.Sequence[i]^0x100|EXTENDED_KEY_BASE;
					if (ks.Sequence[i]&0x200) ks.Sequence[i]=ks.Sequence[i]^0x200|INTERNAL_KEY_BASE;
				}
				return FarAdvControl(ModuleNumber, ACTL_POSTKEYSEQUENCE, &ks);
			}

		case oldfar::ACTL_GETSHORTWINDOWINFO:
		case oldfar::ACTL_GETWINDOWINFO:
			{
				if (!Param) return FALSE;
				int cmd = (Command==oldfar::ACTL_GETWINDOWINFO)?ACTL_GETWINDOWINFO:ACTL_GETSHORTWINDOWINFO;
				oldfar::WindowInfo *wiA = (oldfar::WindowInfo *)Param;
				WindowInfo wi={wiA->Pos};
				INT_PTR ret = FarAdvControl(ModuleNumber, cmd, &wi);
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
					// BUGBUG - Name and TypeName not unicode
					//UnicodeToAnsi(wi.TypeName,wiA->TypeName,sizeof(wiA->TypeName)-1);
					//UnicodeToAnsi(wi.Name,wiA->Name,sizeof(wiA->Name)-1);
					strncpy(wiA->TypeName,wi.TypeName,sizeof(wiA->TypeName)-1);
					strncpy(wiA->Name,wi.Name,sizeof(wiA->Name)-1);
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
		case ECTL_ADDCOLOR:
		case ECTL_DELETEBLOCK:
		case ECTL_DELETECHAR:
		case ECTL_DELETESTRING:
		case ECTL_EXPANDTABS:
		case ECTL_GETCOLOR:
		case ECTL_GETBOOKMARKS:
		case ECTL_INSERTSTRING:
		case ECTL_QUIT:
		case ECTL_REALTOTAB:
		case ECTL_REDRAW:
		case ECTL_SELECT:
		case ECTL_SETPOSITION:
		case ECTL_TABTOREAL:
		case ECTL_TURNOFFMARKINGBLOCK:
			return FarEditorControl(Command,Param);

		case ECTL_GETSTRING:
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
					if (gt) free(gt);
					if (geol) free(geol);
					gt = UnicodeToAnsiBin(egs.StringText,egs.StringLength);
					geol = UnicodeToAnsi(egs.StringEOL);
					oegs->StringText=gt;
					oegs->StringEOL=geol;
					return TRUE;
				}
				return FALSE;
			}

		case ECTL_INSERTTEXT:
		{
			const char *p=(const char *)Param;
			if (!p) return FALSE;
			string strP(p);
			return FarEditorControl(ECTL_INSERTTEXT,(void *)(const wchar_t *)strP);
		}

		case ECTL_GETINFO:
		{
			EditorInfo ei;
			oldfar::EditorInfo *oei=(oldfar::EditorInfo *)Param;
			if (!oei) return FALSE;
			int ret=FarEditorControl(ECTL_GETINFO,&ei);
			if (ret)
			{
				memset(oei,0,sizeof(*oei));
				oei->EditorID=ei.EditorID;
				if (fn)	free(fn);
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

		case ECTL_EDITORTOOEM:
		case ECTL_OEMTOEDITOR:
			return TRUE;

		case ECTL_SAVEFILE:
		case ECTL_PROCESSINPUT:
		case ECTL_PROCESSKEY:
		case ECTL_READINPUT:
		case ECTL_SETKEYBAR:
		case ECTL_SETPARAM:
		case ECTL_SETSTRING:
		case ECTL_SETTITLE:
			return FALSE;

	}

	return FALSE;
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
				ViewerInfoW viW;
				viW.StructSize = sizeof(ViewerInfoW); //BUGBUG?
				if (FarViewerControl(VCTL_GETINFO, &viW) == FALSE) return FALSE;

				viA->ViewerID = viW.ViewerID;

				if (filename) free (filename);
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
			}
			break;

		case oldfar::VCTL_QUIT:
			FarViewerControl(VCTL_QUIT, NULL);
			break;
		case oldfar::VCTL_REDRAW:
			FarViewerControl(VCTL_REDRAW, NULL);
			break;
		case oldfar::VCTL_SETKEYBAR:
			{
				switch((LONG_PTR)Param)
				{
					case NULL:
					case -1:
						FarViewerControl(VCTL_SETKEYBAR, Param);
						break;
					default:
						oldfar::KeyBarTitles* kbtA = (oldfar::KeyBarTitles*)Param;
						KeyBarTitles kbt;

						for(int i=0;i<12;i++)
						{
							kbt.Titles[i]          = kbtA->Titles[i]?          AnsiToUnicode(kbtA->Titles[i]):          NULL;
							kbt.CtrlTitles[i]      = kbtA->CtrlTitles[i]?      AnsiToUnicode(kbtA->CtrlTitles[i]):      NULL;
							kbt.AltTitles[i]       = kbtA->AltTitles[i]?       AnsiToUnicode(kbtA->AltTitles[i]):       NULL;
							kbt.ShiftTitles[i]     = kbtA->ShiftTitles[i]?     AnsiToUnicode(kbtA->ShiftTitles[i]):     NULL;
							kbt.CtrlShiftTitles[i] = kbtA->CtrlShiftTitles[i]? AnsiToUnicode(kbtA->CtrlShiftTitles[i]): NULL;
							kbt.AltShiftTitles[i]  = kbtA->AltShiftTitles[i]?  AnsiToUnicode(kbtA->AltShiftTitles[i]):  NULL;
							kbt.CtrlAltTitles[i]   = kbtA->CtrlAltTitles[i]?   AnsiToUnicode(kbtA->CtrlAltTitles[i]):   NULL;
						}
						FarViewerControl(VCTL_SETKEYBAR, &kbt);
						for(int i=0;i<12;i++)
						{
							free (kbt.Titles[i]);
							free (kbt.CtrlTitles[i]);
							free (kbt.AltTitles[i]);
							free (kbt.ShiftTitles[i]);
							free (kbt.CtrlShiftTitles[i]);
							free (kbt.AltShiftTitles[i]);
							free (kbt.CtrlAltTitles[i]);
						}
						return TRUE;
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
