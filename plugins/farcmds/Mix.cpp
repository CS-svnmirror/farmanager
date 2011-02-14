#ifndef __mix_cpp
#define __mix_cpp

/*
 �����頥� �᫮, ��१�� ��� �� ��ப�, ��� -2 � ��砥 �訡��
 Start, End - ��砫� � ����� ��ப�
*/
int GetInt(wchar_t *Start, wchar_t *End)
{
	int Ret=-2;

	if (End >= Start)
	{
		wchar_t Tmp[11];
		int Size=(int)(End-Start);

		if (Size)
		{
			if (Size < 11)
			{
				wmemcpy(Tmp,Start,Size);
				Tmp[Size]=0;
				Ret=FSF.atoi(Tmp);
			}
		}
		else
			Ret=0;
	}

	return Ret;
}

const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

/*�����頥� TRUE, �᫨ 䠩� name �������*/
BOOL FileExists(const wchar_t *Name)
{
	return GetFileAttributes(Name)!=0xFFFFFFFF;
}

wchar_t *GetCommaWord(wchar_t *Src,wchar_t *Word,wchar_t Separator)
{
	int WordPos,SkipBrackets;

	if (*Src==0)
		return(NULL);

	SkipBrackets=FALSE;

	for (WordPos=0; *Src!=0; Src++,WordPos++)
	{
		if (*Src==L'[' && wcschr(Src+1,L']')!=NULL)
			SkipBrackets=TRUE;

		if (*Src==L']')
			SkipBrackets=FALSE;

		if (*Src==Separator && !SkipBrackets)
		{
			Word[WordPos]=0;
			Src++;

			while (IsSpace(*Src))
				Src++;

			return(Src);
		}
		else
			Word[WordPos]=*Src;
	}

	Word[WordPos]=0;
	return(Src);
}


// �������� � ��ப� Str Count �宦����� �����ப� FindStr �� �����ப� ReplStr
// �᫨ Count < 0 - �������� "�� ������ ������"
// Return - ������⢮ �����
int ReplaceStrings(wchar_t *Str,const wchar_t *FindStr,const wchar_t *ReplStr,int Count,BOOL IgnoreCase)
{
	int I=0, J=0, Res;
	int LenReplStr=(int)lstrlen(ReplStr);
	int LenFindStr=(int)lstrlen(FindStr);
	int L=(int)lstrlen(Str);

	while (I <= L-LenFindStr)
	{
		Res=IgnoreCase?_memicmp(Str+I, FindStr, LenFindStr*sizeof(wchar_t)):memcmp(Str+I, FindStr, LenFindStr*sizeof(wchar_t));

		if (Res == 0)
		{
			if (LenReplStr > LenFindStr)
				wmemmove(Str+I+(LenReplStr-LenFindStr),Str+I,lstrlen(Str+I)+1); // >>
			else if (LenReplStr < LenFindStr)
				wmemmove(Str+I,Str+I+(LenFindStr-LenReplStr),lstrlen(Str+I+(LenFindStr-LenReplStr))+1); //??

			wmemcpy(Str+I,ReplStr,LenReplStr);
			I += LenReplStr;

			if (++J == Count && Count > 0)
				break;
		}
		else
			I++;

		L=(int)lstrlen(Str);
	}

	return J;
}

/*
 �����頥� PipeFound
*/
int PartCmdLine(const wchar_t *CmdStr,wchar_t *NewCmdStr,int SizeNewCmdStr,wchar_t *NewCmdPar,int SizeNewCmdPar)
{
	int PipeFound = FALSE;
	int QuoteFound = FALSE;
	ExpandEnvironmentStrings(CmdStr, NewCmdStr, SizeNewCmdStr);
	FSF.Trim(NewCmdStr);
	wchar_t *CmdPtr = NewCmdStr;
	wchar_t *ParPtr = NULL;
	// �������� ᮡ�⢥��� ������� ��� �ᯮ������ � ��ࠬ����.
	// �� �⮬ ������ ��।���� ����稥 ᨬ����� ��८�।������ ��⮪��
	// ����⠥� � ��⮬ ����祪. �.�. ���� � ����窠� - �� ����.

	while (*CmdPtr)
	{
		if (*CmdPtr == L'"')
			QuoteFound = !QuoteFound;

		if (!QuoteFound && CmdPtr != NewCmdStr)
		{
			if (*CmdPtr == L'>' || *CmdPtr == L'<' ||
			        *CmdPtr == L'|' || *CmdPtr == L' ' ||
			        *CmdPtr == L'/' ||      // ��ਠ�� "far.exe/?"
			        *CmdPtr == L'&'
			   )
			{
				if (!ParPtr)
					ParPtr = CmdPtr;

				if (*CmdPtr != L' ' && *CmdPtr != L'/')
					PipeFound = TRUE;
			}
		}

		if (ParPtr && PipeFound) // ��� ����� ��祣� �� ���� 㧭�����
			break;

		CmdPtr++;
	}

	if (ParPtr) // �� ��諨 ��ࠬ���� � �⤥�塞 ��� �� ��⫥�
	{
		if (*ParPtr == L' ') //AY: ���� �஡�� ����� �������� � ��ࠬ��ࠬ� �� �㦥�,
			*(ParPtr++)=0;     //    �� ���������� ������ � Execute.

		lstrcpyn(NewCmdPar, ParPtr, SizeNewCmdPar-1);
		*ParPtr = 0;
	}

	FSF.Unquote(NewCmdStr);
	return PipeFound;
}

BOOL ProcessOSAliases(wchar_t *Str,int SizeStr)
{
	typedef DWORD (WINAPI *PGETCONSOLEALIAS)(
	    wchar_t *lpSource,
	    wchar_t *lpTargetBuffer,
	    DWORD TargetBufferLength,
	    wchar_t *lpExeName
	);
	static PGETCONSOLEALIAS pGetConsoleAlias=NULL;

	if (!pGetConsoleAlias)
	{
		pGetConsoleAlias = (PGETCONSOLEALIAS)GetProcAddress(GetModuleHandleW(L"kernel32"),"GetConsoleAliasW");

		if (!pGetConsoleAlias)
			return FALSE;
	}

	wchar_t NewCmdStr[4096];
	wchar_t NewCmdPar[4096];
	*NewCmdStr=0;
	*NewCmdPar=0;
	PartCmdLine(Str,NewCmdStr,ARRAYSIZE(NewCmdStr),NewCmdPar,ARRAYSIZE(NewCmdPar));
	wchar_t ModuleName[MAX_PATH];
	GetModuleFileName(NULL,ModuleName,ARRAYSIZE(ModuleName));
	wchar_t* ExeName=(wchar_t*)FSF.PointToName(ModuleName);
	int ret=pGetConsoleAlias(NewCmdStr,NewCmdStr,sizeof(NewCmdStr),ExeName);

	if (!ret)
	{
		if (ExpandEnvironmentStrings(L"%COMSPEC%",ModuleName,ARRAYSIZE(ModuleName)))
		{
			ExeName=(wchar_t*)FSF.PointToName(ModuleName);
			ret=pGetConsoleAlias(NewCmdStr,NewCmdStr,sizeof(NewCmdStr),ExeName);
		}
	}

	if (!ret)
	{
		return FALSE;
	}

	if (!ReplaceStrings(NewCmdStr,L"$*",NewCmdPar,-1,FALSE))
	{
		lstrcat(NewCmdStr,L" ");
		lstrcat(NewCmdStr,NewCmdPar);
	}

	lstrcpyn(Str,NewCmdStr,SizeStr-1);
	return TRUE;
}

#endif
