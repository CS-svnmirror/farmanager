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


wchar_t *GetShellLinkPath(const wchar_t *LinkFile)
{
	bool Result=false;
	wchar_t *Path=nullptr;

	wchar_t FileName[MAX_PATH*5];
	ExpandEnvironmentStrings(LinkFile, FileName, ARRAYSIZE(FileName));
	FSF.Unquote(FileName);
	FSF.ConvertPath(CPM_NATIVE, FileName, FileName, ARRAYSIZE(FileName));
	wchar_t *ptrFileName=FileName;

	if (!(*ptrFileName && FileExists(ptrFileName)))
		return nullptr;

	// <Check lnk-header>
	HANDLE hFile = CreateFile(ptrFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

	if (hFile != INVALID_HANDLE_VALUE)
	{
		struct ShellLinkHeader
		{
			DWORD    HeaderSize;
			BYTE     LinkCLSID[16];
			DWORD    LinkFlags;
			DWORD    FileAttributes;
			FILETIME CreationTime;
			FILETIME AccessTime;
			FILETIME WriteTime;
			DWORD    FileSize;
			DWORD    IconIndex;
			DWORD    ShowCommand;
			WORD     HotKey;
			WORD     Reserved1;
			DWORD    Reserved2;
			DWORD    Reserved3;
		};

		ShellLinkHeader slh = { 0 };
		DWORD read = 0;
		ReadFile( hFile, &slh, sizeof( ShellLinkHeader ), &read, NULL );

		if ( read == sizeof( ShellLinkHeader ) && slh.HeaderSize == 0x0000004C)
		{
			if (!memcmp( slh.LinkCLSID, "\x01\x14\x02\x00\x00\x00\x00\x00\xC0\x00\x00\x00\x00\x00\x00\x46\x9b", 16 ))
				Result=true;
		}

		CloseHandle( hFile );
	}
	// </Check lnk-header>

	if (Result)
	{
		// <get target>
		Result=false;
		/*HRESULT hres0 = */CoInitialize(NULL);

		IShellLink* psl = NULL;
		HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf = NULL;
			hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
			if (SUCCEEDED(hres))
			{
				hres = ppf->Load(ptrFileName, STGM_READ);
				if (SUCCEEDED(hres))
				{
					hres = psl->Resolve(NULL, 0);
					if (SUCCEEDED(hres))
					{
						wchar_t TargPath[MAX_PATH] = {0};
						hres = psl->GetPath(TargPath, ARRAYSIZE(TargPath), NULL, SLGP_RAWPATH);
						if (SUCCEEDED(hres))
						{
							Path=new wchar_t[lstrlen(TargPath)+1];
							if (Path)
								lstrcpy(Path, TargPath);
						}
					}
				}
				ppf->Release();
			}
			psl->Release();
		}

		CoUninitialize();
		// </get target>
	}

	return Path;
}

wchar_t* GetSameFolder(struct PanelInfo& PInfo,HANDLE SrcPanel, HANDLE DstPanel)
{
    wchar_t *RetObject=nullptr;
	wchar_t *DestObject=nullptr;
	int dirSize=(int)Info.PanelControl(SrcPanel,FCTL_GETPANELDIRECTORY,0,0);

	FarPanelDirectory* dirInfo=(FarPanelDirectory*)malloc(dirSize);
	if (dirInfo)
	{
		dirInfo->StructSize = sizeof(FarPanelDirectory);
		Info.PanelControl(SrcPanel,FCTL_GETPANELDIRECTORY,dirSize,dirInfo);

		int lenName=lstrlen(dirInfo->Name);
		DestObject=(wchar_t*)malloc(sizeof(wchar_t)*(lenName+2));
		if (DestObject)
		{
			lstrcpy(DestObject,dirInfo->Name);

			if (*DestObject)
				FSF.AddEndSlash(DestObject);

			size_t Size = Info.PanelControl(SrcPanel,FCTL_GETPANELITEM,PInfo.CurrentItem,0);
			PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

			if (PPI)
			{
				FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
				Info.PanelControl(SrcPanel,FCTL_GETPANELITEM,PInfo.CurrentItem,&gpi);

				wchar_t *NewSrc=(wchar_t*)realloc(DestObject,sizeof(wchar_t)*(lstrlen(DestObject)+lstrlen(PPI->FileName)+1));
				if (NewSrc)
				{
					DestObject=NewSrc;
					lstrcat(DestObject,PPI->FileName);
					RetObject=DestObject;
				}
				free(PPI);
			}
		}

		free(dirInfo);
	}

	if (!RetObject)
		if (DestObject)
			free(DestObject);

	return RetObject;
}

#endif
