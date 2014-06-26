#ifndef __OpenFromCommandLine
#define __OpenFromCommandLine

#ifndef LIGHTGRAY
#define LIGHTGRAY 7
#endif

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8_LO    0xBBEF
#define SIGN_UTF8_HI    0xBF

static inline bool vh(HANDLE h)
{
	return h != INVALID_HANDLE_VALUE;
}

static void closeHandle(HANDLE& handle)
{
	if (vh(handle))
		CloseHandle(handle);

	handle = INVALID_HANDLE_VALUE;
}

static void killTemp(wchar_t *TempFileName)
{
	if (FileExists(TempFileName))
	{
		DeleteFile(TempFileName);
		*(wchar_t*)(FSF.PointToName(TempFileName)-1) = 0;
		RemoveDirectory(TempFileName);
	}
}

inline bool isDevice(const wchar_t* FileName, const wchar_t* dev_begin)
{
	const int len=lstrlen(dev_begin);

	if (FSF.LStrnicmp(FileName, dev_begin, len))
		return false;

	FileName+=len;

	if (!*FileName)
		return false;

	while (*FileName>=L'0' && *FileName<=L'9')
		FileName++;

	return !*FileName;
}

static bool validForView(const wchar_t *FileName, int viewEmpty, int editNew)
{
	if (!wmemcmp(FileName, L"\\\\.\\", 4) &&  // ����������� ��������� ����
			FSF.LIsAlpha(FileName[4]) &&          // ����: \\.\�����:
			FileName[5]==L':' && FileName[6]==0)
		return true;

	if (isDevice(FileName, L"\\\\.\\PhysicalDrive"))
		return true;

	if (isDevice(FileName, L"\\\\.\\cdrom"))
		return true;

	const wchar_t *ptrFileName=FileName;
	wchar_t *ptrCurDir=NULL;

	if (*ptrFileName && FSF.PointToName(ptrFileName) == ptrFileName)
	{
		int dirSize=(int)Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,0);

		if (dirSize)
		{
		    FarPanelDirectory* dirInfo=(FarPanelDirectory*)new char[dirSize];
		    dirInfo->StructSize = sizeof(FarPanelDirectory);
			Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,dirSize,dirInfo);
			int Size=lstrlen(dirInfo->Name)+1;
			ptrCurDir=new WCHAR[Size+lstrlen(FileName)+8];
			lstrcpy(ptrCurDir,dirInfo->Name);
			lstrcat(ptrCurDir,L"\\");
			lstrcat(ptrCurDir,ptrFileName);
			ptrFileName=(const wchar_t *)ptrCurDir;
			delete[](char*)dirInfo;
		}
	}

	if (*ptrFileName && FileExists(ptrFileName))
	{
		if (viewEmpty)
		{
			if (ptrCurDir)
				delete[] ptrCurDir;

			return true;
		}

		HANDLE Handle = CreateFile(ptrFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

		if (vh(Handle))
		{
			DWORD size = GetFileSize(Handle, NULL);
			CloseHandle(Handle);

			if (ptrCurDir)
				delete[] ptrCurDir;

			return size && (size != 0xFFFFFFFF);
		}
	}
	else if (editNew)
	{
		if (ptrCurDir)
			delete[] ptrCurDir;

		return true;
	}

	if (ptrCurDir)
		delete[] ptrCurDir;

	return false;
}

// ����� ������������� ������ �� ����� ��� ":<+"

#define THREADSLEEP  200
#define THREADREDRAW 10

struct TShowOutputStreamData
{
	HANDLE hRead;
	HANDLE hWrite;
	HANDLE hConsole;
};

enum enStream { enStreamOut, enStreamErr, enStreamMAX };
enum enThreadType { enThreadHideOutput, enThreadShowOutput };

struct TThreadData
{
	enThreadType type;
	bool processDone;
	TShowOutputStreamData stream[enStreamMAX];
	wchar_t title[80], cmd[1024];
};

static DWORD showPartOfOutput(TShowOutputStreamData *sd)
{
	DWORD Res = 0;

	if (sd)
	{
		wchar_t ReadBuf[4096+1];
		DWORD BytesRead = 0;

		if (ReadFile(sd->hRead, ReadBuf, sizeof(ReadBuf)-sizeof(wchar_t), &BytesRead, NULL))
		{
			if (BytesRead)
			{
				DWORD dummy;
				ReadBuf[BytesRead] = 0;

				if (vh(sd->hConsole))
					WriteConsole(sd->hConsole, ReadBuf, BytesRead, &dummy, NULL);

				Res = BytesRead;
			}
		}
	}

	return Res;
}

DWORD WINAPI ThreadWhatUpdateScreen(LPVOID par)
{
	if (par)
	{
		TThreadData *td = (TThreadData*)par;

		if (td->type == enThreadShowOutput)
		{
			for (; ;)
			{
				if (td->processDone)
					break;

				Sleep(THREADSLEEP);

				for (int i = 0 ; i < enStreamMAX ; i++)
				{
					TShowOutputStreamData *sd = &(td->stream[i]);

					if (vh(sd->hRead))
						showPartOfOutput(sd);
				}
			}
		}
		else
		{
			for (; ;)
			{
				for (int j = 0 ; j < THREADREDRAW ; j++)
				{
					if (td->processDone)
						break;

					Sleep(THREADSLEEP);
				}

				if (td->processDone)
					break;

				wchar_t buff[80];
				DWORD sCheck[enStreamMAX];

				for (int i = 0 ; i < enStreamMAX ; i++)
				{
					HANDLE hCheck = td->stream[i].hWrite;

					if (vh(hCheck))
					{
						sCheck[i] = GetFileSize(hCheck, NULL);

						if (sCheck[i] == 0xFFFFFFFF)
							sCheck[i] = 0;
					}
					else
						sCheck[i] = 0;
				}

				if (sCheck[enStreamOut])
					if (sCheck[enStreamErr])
						FSF.sprintf(buff, L"%lu/%lu", sCheck[enStreamOut], sCheck[enStreamErr]);
					else
						FSF.sprintf(buff, L"%lu", sCheck[enStreamOut]);
				else if (sCheck[enStreamErr])
					FSF.sprintf(buff, L"%lu", sCheck[enStreamErr]);
				else
					*buff = 0;

				const wchar_t *MsgItems[] = { td->title, td->cmd, buff };
				Info.Message(&MainGuid, nullptr, 0, NULL, MsgItems, ARRAYSIZE(MsgItems), 0);
			}
		}
	}

	return 0;
}

static bool MakeTempNames(wchar_t* tempFileName1, wchar_t* tempFileName2, size_t szTempNames)
{
	static const wchar_t tmpPrefix[] = L"FCP";
	wchar_t fullcmd[MAX_PATH*2];

	if (FSF.MkTemp(tempFileName1,szTempNames,tmpPrefix) > 1)
	{
		DeleteFile(tempFileName1);

		if (CreateDirectory(tempFileName1, NULL))
		{
			bool ok = true;

			if (GetTempFileName(tempFileName1, tmpPrefix, 0, fullcmd))
				lstrcpy(tempFileName2, fullcmd);
			else
				ok = false;

			if (ok && GetTempFileName(tempFileName1, tmpPrefix, 0, fullcmd))
				lstrcpy(tempFileName1, fullcmd);
			else
				ok = false;

			if (ok)
				return true;

			RemoveDirectory(tempFileName1);
		}
	}
	return false;
}
/*
  ���������� ��������� �� ���������� �����, �������� ����� ������������� ������� free

  fn - ��� �����, ������ ������
  maxSize - ������� �������� ��������� �� �����
  outputtofile - ��� ���� ��������������� ���...
  shift - ������ "���������� ������" � ����������� ������, � ������ ���������...
*/
static wchar_t *loadFile(const wchar_t *fn, DWORD maxSize, BOOL outputtofile, size_t& shift, bool& foundFile)
{
	foundFile = false;
	shift=0;

	wchar_t *Ptr = NULL, FileName[MAX_PATH*5];
	ExpandEnvironmentStrings(fn, FileName, ARRAYSIZE(FileName));
	FSF.Unquote(FileName);

	FSF.ConvertPath(CPM_NATIVE, FileName, FileName, ARRAYSIZE(FileName));

	wchar_t *ptrFileName=FileName;

	if (*ptrFileName && FileExists(ptrFileName))
	{
		foundFile = true;
		HANDLE Handle = CreateFile(ptrFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

		if (vh(Handle))
		{
			DWORD sizeFile=GetFileSize(Handle, NULL);
			DWORD size = (sizeFile+(sizeof(wchar_t)/2)) / sizeof(wchar_t);

			if (size >= maxSize)
				size = maxSize-1;

			size *= sizeof(wchar_t);

			if (size)
			{
				DWORD read;

				wchar_t *buff = (wchar_t *)malloc(size+sizeof(wchar_t));

				if (buff)
				{
					if (ReadFile(Handle, buff, size, &read, NULL) && (read >= sizeof(wchar_t) || (read == 1 && sizeFile == 1)))
					{
						if (read&1)
						{
							buff[read/sizeof(wchar_t)]=buff[read/sizeof(wchar_t)]&0xff;
							read++;
						}

						buff[read/sizeof(wchar_t)] = 0;
						Ptr = buff;
					}
					else
						free(buff);
				}
			}

			CloseHandle(Handle);
		}

	}

	if (Ptr)
	{
		if (Ptr[0]==SIGN_UNICODE)
		{
			shift=1;
		}
		else if (Ptr[0]==SIGN_REVERSEBOM)
		{
			shift=1;
			size_t PtrLength=lstrlen(Ptr);
			swab((char*)Ptr,(char*)Ptr,int(PtrLength*sizeof(wchar_t)));
		}
		else
		{
			UINT cp=outputtofile?GetConsoleOutputCP():GetACP();

			if (Ptr[0]==SIGN_UTF8_LO&&(Ptr[1]&0xff)==SIGN_UTF8_HI)
			{
				shift=1;
				cp=CP_UTF8;
			}

			size_t PtrLength=MultiByteToWideChar(cp,0,(char*)Ptr,-1,NULL,0);

			if (PtrLength)
			{
				wchar_t* NewPtr=(wchar_t*)malloc(PtrLength*sizeof(wchar_t));
				if (NewPtr)
				{
					if (MultiByteToWideChar(cp,0,(char*)Ptr,-1,NewPtr,(int)PtrLength))
					{
						free(Ptr);
						Ptr=NewPtr;
					}
					else
					{
						free(NewPtr);
					}
				}
			}
		}
	}

	return Ptr;
}

static void ParseCmdSyntax(wchar_t*& pCmd, int& ShowCmdOutput, int& stream)
{
	switch (*pCmd)
	{
		case L'*': stream = 0; ++pCmd; break;
		case L'1': stream = 1; ++pCmd; break;
		case L'2': stream = 2; ++pCmd; break;
		case L'?': stream = 3; ++pCmd; break;
	}

	bool flg_stream = false;

	switch (*pCmd)
	{
		case L'>': ShowCmdOutput = 0; flg_stream = true; ++pCmd; break;
		case L'<': ShowCmdOutput = 1; flg_stream = true; ++pCmd; break;
		case L'+': ShowCmdOutput = 2; flg_stream = true; ++pCmd; break;
		case L' ': flg_stream = true; break;
		case L'|': flg_stream = true; break;
		case L'"': flg_stream = true; break;
	}

	if ((!flg_stream) && (stream == 1 || stream == 2))
	{
		--pCmd;
	}

	FSF.LTrim(pCmd);
}


bool __proc_Load_Unload(int /*outputtofile*/,wchar_t *pCmd,bool Load)
{
	bool Ret=false;
	FSF.Unquote(pCmd);
	DWORD sizeExp=ExpandEnvironmentStrings(pCmd,NULL,0);
	wchar_t *temp=new wchar_t[sizeExp+1];
	if (temp)
	{
		ExpandEnvironmentStrings(pCmd,temp,sizeExp);

		if (Load)
			Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_LOADPLUGIN,PLT_PATH,temp);
		else
		{
			HANDLE hPlugin = reinterpret_cast<HANDLE>(Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_FINDPLUGIN,PFM_MODULENAME,temp));
			if(hPlugin)
			{
				Info.PluginsControl(hPlugin,PCTL_UNLOADPLUGIN,0,nullptr);
			}
		}

		delete[] temp;
	}

	return Ret;
}

wchar_t* __getContent(int outputtofile,wchar_t *pCmd)
{
	wchar_t *Ptr=nullptr;

	if (outputtofile)
	{
		bool foundFile=false;
		size_t shift;
		Ptr = loadFile(pCmd, Opt.MaxDataSize/sizeof(wchar_t), outputtofile, shift, foundFile);

		if (Ptr)
		{
			wchar_t *DestPath=new wchar_t[lstrlen(Ptr+shift)+1];
			if (DestPath)
			{
				lstrcpy(DestPath, Ptr+shift);
				delete[] Ptr;

				if (!(Ptr=wcschr(DestPath,L'\r')))
					Ptr=wcschr(DestPath,L'\n');

				if (Ptr)
					*Ptr=0;
				Ptr=DestPath;
			}
			else
			{
				delete[] Ptr;
				Ptr=nullptr;
			}
		}
	}
	else
	{
		Ptr=new wchar_t[lstrlen(pCmd)+1];
		if (Ptr)
			lstrcpy(Ptr,pCmd);
	}

	return Ptr;
}

/*
goto:path
goto:<path
*/
wchar_t* __proc_Goto(int outputtofile,wchar_t *pCmd)
{
	wchar_t *Ptr=nullptr;

	if (outputtofile)
		Ptr=GetShellLinkPath(pCmd);
	if (!Ptr)
		Ptr=__getContent(outputtofile,pCmd);

	if (Ptr)
	{
		DWORD sizeExp=ExpandEnvironmentStrings(Ptr,NULL,0);
		wchar_t *temp=new wchar_t[sizeExp+1];
		if (temp)
		{
			ExpandEnvironmentStrings(Ptr,temp,sizeExp);
			delete[] Ptr;
			Ptr=temp;
		}
		else
		{
			delete[] Ptr;
			Ptr=nullptr;
		}
	}

	return Ptr;
}

/*
whereis:path
whereis:<path
*/
wchar_t* __proc_WhereIs(int outputtofile,wchar_t *pCmd)
{
	wchar_t *DestPath=nullptr;
	wchar_t *Ptr=__getContent(outputtofile,pCmd);

	if (Ptr)
	{
		FSF.Unquote(Ptr);

		DWORD sizeExp=ExpandEnvironmentStrings(Ptr,NULL,0);
		wchar_t *temp=new wchar_t[sizeExp+1];
		if (temp)
		{
			ExpandEnvironmentStrings(Ptr,temp,sizeExp);

			wchar_t *Path = nullptr, *FARHOMEPath = nullptr;

			size_t CurDirLength=FSF.GetCurrentDirectory(0,nullptr);
			int    FARHOMELength=GetEnvironmentVariable(L"FARHOME", FARHOMEPath, 0);
			int    PathLength=GetEnvironmentVariable(L"PATH", Path, 0);

			wchar_t *AllPath=new wchar_t[CurDirLength+FARHOMELength+PathLength+8];
			if (AllPath)
			{
				wchar_t *ptrAllPath=AllPath;

				// 1. Current folder
				FSF.GetCurrentDirectory(CurDirLength,ptrAllPath);
				lstrcat(ptrAllPath,L";");
				ptrAllPath+=lstrlen(ptrAllPath);

				// 2. The directory pointed to by the environment variable %FARHOME%
				GetEnvironmentVariable(L"FARHOME", ptrAllPath, FARHOMELength);
				lstrcat(ptrAllPath,L";");
				ptrAllPath+=lstrlen(ptrAllPath);

				// 3. Folders in the system environment variable #PATH#
				GetEnvironmentVariable(L"PATH", ptrAllPath, PathLength);

				DWORD DestPathSize = SearchPath(AllPath,temp,nullptr,0,nullptr,nullptr);
				DestPath=new wchar_t[DestPathSize+1];
				if (DestPath)
				{
					*DestPath=0;

					wchar_t *pFile;
					SearchPath(AllPath, temp, NULL, DestPathSize, DestPath, &pFile);

					if (*DestPath==0) // 4..6
						SearchPath(NULL, temp, NULL, DestPathSize, DestPath, &pFile);
				}

				delete[] AllPath;
			}

			// 7..8 Contents of the registry key
			if (!DestPath || !*DestPath)
			{
				if (DestPath)
					delete[] DestPath;
				DestPath=nullptr;

				const wchar_t RegPath[]=L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
				wchar_t *FullKeyName=new wchar_t[lstrlen(RegPath)+lstrlen(Ptr)+1];
				if (FullKeyName)
				{
					lstrcpy(FullKeyName,RegPath);
					lstrcat(FullKeyName,Ptr);

					HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE},hKey;
					for (size_t I=0; I < ARRAYSIZE(RootFindKey); ++I)
					{
						if (RegOpenKeyEx(RootFindKey[I], FullKeyName, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
						{
							DWORD Type, DestPathSize=0;
							if (RegQueryValueEx(hKey,L"", nullptr, nullptr, nullptr, &DestPathSize) == ERROR_SUCCESS)
							{
								DestPath=new wchar_t[DestPathSize+1];
								if (DestPath)
								{
									RegQueryValueEx(hKey,L"", 0, &Type, (LPBYTE) DestPath, &DestPathSize);
									delete[] Ptr;
									Ptr=DestPath;
								}
							}
							RegCloseKey(hKey);
							break;
						}
					}

					delete[] FullKeyName;
				}
				else
				{
					delete[] Ptr;
					Ptr=nullptr;
				}
			}
			else
			{
				delete[] Ptr;
				Ptr=DestPath;
			}

			delete[] temp;
		}
		else
		{
			delete[] Ptr;
			Ptr=nullptr;
		}
	}


	return Ptr;
}

bool __proc_Link(int /*outputtofile*/,wchar_t *pCmd)
{
	bool Ret=false;
	bool NeedSymLink=false;
	MKLINK_FLAGS LinkFlags=MLF_NONE;
	wchar_t *Arg2=NULL;

	while (*pCmd && *pCmd == L'/')
	{
		if (!FSF.LStrnicmp(pCmd,L"/MSG",4))
		{
			LinkFlags|=MLF_SHOWERRMSG;
			pCmd=FSF.Trim(pCmd+4);
		}
		else if (!FSF.LStrnicmp(pCmd,L"/N",2))
		{
			LinkFlags|=MLF_DONOTUPDATEPANEL;
			pCmd=FSF.Trim(pCmd+2);
		}
		else if (!FSF.LStrnicmp(pCmd,L"/S",2))
		{
			NeedSymLink=true;
			pCmd=FSF.Trim(pCmd+2);
		}
		else
		{
			break;
		}
	}

	if (*pCmd == L'"')
	{
		Arg2=wcschr(pCmd+1,L'"');

		if (Arg2)
		{
			*++Arg2=0;
			Arg2=FSF.Trim(++Arg2);

			if (*Arg2 == L'"')
			{
				wchar_t *Arg3=wcschr(Arg2+1,L'"');

				if (Arg3)
					*++Arg3=0;

				FSF.Unquote(Arg2);
			}
		}

		FSF.Unquote(pCmd);
	}
	else
	{
		Arg2=wcschr(pCmd+1,L' ');

		if (Arg2)
		{
			*Arg2=0;
			Arg2=FSF.Trim(++Arg2);

			if (*Arg2 == L'"')
			{
				wchar_t *Arg3=wcschr(Arg2+1,L'"');

				if (Arg3)
					*++Arg3=0;

				FSF.Unquote(Arg2);
			}
		}
	}

	DWORD FTAttr=0xFFFFFFFF;

	{
		size_t tempSize=FSF.ConvertPath(CPM_FULL, pCmd, nullptr, 0);
		wchar_t *temp=new wchar_t[tempSize+1];
		if (temp)
		{
			FSF.ConvertPath(CPM_FULL, pCmd, temp, tempSize);
			FTAttr=GetFileAttributes(temp);
			delete[] temp;
		}
	}

	if (Arg2)
	{
		wchar_t Disk[16];
		LINK_TYPE LinkType=LINK_HARDLINK;

		if (pCmd[1] == L':' && ((pCmd[2] == L'\\' && pCmd[3] == 0) || pCmd[2] == 0))
		{
			if (pCmd[2] == 0)
			{
				Disk[0]=pCmd[0];
				Disk[1]=pCmd[1];
				Disk[2]=L'\\';
				Disk[3]=0;
				pCmd=Disk;
			}

			LinkType=LINK_VOLMOUNT;
		}
		else if (FTAttr != 0xFFFFFFFF)
		{
			if (FTAttr&FILE_ATTRIBUTE_DIRECTORY)
				LinkType=NeedSymLink?LINK_SYMLINKDIR:LINK_JUNCTION;
			else
				LinkType=NeedSymLink?LINK_SYMLINKFILE:LINK_HARDLINK;
		}
		FSF.MkLink(pCmd,Arg2,LinkType,LinkFlags);

		return true;
	}

	return Ret;
}


wchar_t* OpenFromCommandLine(const wchar_t *_farcmd)
{
	if (!_farcmd)
		return nullptr;

	int View=0,Edit=0,Clip=0,Goto=0,WhereIs=0,Load=0,Unload=0,Link=0,Run=0;
	size_t PrefIdx=static_cast<size_t>(-1);
	struct
	{
		int& Pref;
		LPCTSTR Name;
		LPCTSTR HelpName;
	} Pref[]=
	{
		// view:[<separator>]<object>
		// view<separator><object>
		{View,L"VIEW",L"View"},
		// clip:[<separator>]<object>
		// clip:<separator><object>
		{Clip,L"CLIP",L"Clip"},
		// whereis:[<separator>]<object>
		// whereis<separator><object>
		{WhereIs,L"WHEREIS",L"WhereIs"},
		// edit:[[<options>]<separator>]<object>
		// edit[<options>]<separator><object>
		{Edit,L"EDIT",L"Edit"},
		// goto:[<separator>]<object>
		// goto<separator><object>
		{Goto,L"GOTO",L"Goto"},
		// link:[<separator>][<op>]<separator><source><separator><dest>
		// link<separator>[<op>]<separator><source><separator><dest>
		{Link,L"LINK",L"Link"},
		// run:[<separator>]<file> < <command>
		// run<separator><file> < <command>
		{Run,L"RUN",L"Run"},
		// load:[<separator>]<file>
		// load<separator><file>
		{Load,L"LOAD",L"Load"},
		// unload:[<separator>]<file>
		// unload<separator><file>
		{Unload,L"UNLOAD",L"Unload"},
	};

	static wchar_t selectItem[MAX_PATH*5];
	static wchar_t cmd[MAX_PATH*5];
	wchar_t fullcmd[MAX_PATH*5];

	fullcmd[0]=cmd[0]=selectItem[0]=L'\0';

	static wchar_t farcmdbuf[MAX_PATH*10], *farcmd;
	farcmd=farcmdbuf;
	lstrcpy(farcmdbuf, _farcmd);
	FSF.RTrim(farcmdbuf);
	BOOL showhelp=TRUE;

	if (lstrlen(farcmd) > 3)
	{
		HANDLE FileHandleOut, FileHandleErr;
		int StartLine=-1, StartChar=-1;
		int ShowCmdOutput=Opt.ShowCmdOutput;
		int stream=Opt.CatchMode;
		int outputtofile=0;
		BOOL allOK=TRUE;
		wchar_t *pCmd=NULL;

		for (size_t I=0; I < ARRAYSIZE(Pref); ++I)
		{
			int lenPref=lstrlen(Pref[I].Name);

			if (!_memicmp(farcmd,Pref[I].Name,lenPref))
			{
				farcmd+=lenPref;
				Pref[I].Pref=lenPref;
			}

			if (Pref[I].Pref)
			{
				if (*farcmd == L':')
					farcmd++;

				PrefIdx=I;
				break;
			}
		}

		// farcmd = [[<options>]<separator>]<object>
		// farcmd = [<separator>]<object>
		if (View||Edit||Goto||Clip||WhereIs||Link||Run||Load||Unload)
		{
			int SeparatorLen=lstrlen(Opt.Separator);
			wchar_t *cBracket=NULL, runFile[MAX_PATH]=L"";
			BOOL BracketsOk=TRUE;

			if (Edit)
			{
				// edit:['['<options>']'<separator>]<object>
				//  edit['['<options>']'<separator>]<object>
				//      ^---farcmd
				wchar_t *oBracket;
				BracketsOk=FALSE;
				FSF.LTrim(farcmd);
				oBracket=wcschr(farcmd,L'[');

				if (*farcmd != L'"' && oBracket && oBracket<wcsstr(farcmd,Opt.Separator))
				{
					if ((cBracket=wcschr(oBracket,L']')) != 0 && oBracket < cBracket)
					{
						farcmd=cBracket+1;
						wchar_t *comma=wcschr(oBracket,L',');

						if (comma)
						{
							if (comma > oBracket && comma < cBracket)
								if ((StartLine=GetInt(oBracket+1,comma))>-2 && (StartChar=GetInt(comma+1,cBracket))>-2)
									BracketsOk=TRUE;
						}
						else if ((StartLine=GetInt(oBracket+1,cBracket))>-2)
							BracketsOk=TRUE;
					}
				}
				else
					BracketsOk=TRUE;
			}
			else if (Run)
			{
				pCmd = wcschr(farcmd,L'<');

				if (pCmd)
				{
					*pCmd = 0;
					lstrcpy(runFile, farcmd);
					FSF.Trim(runFile);
					*pCmd = L'<';
					farcmd = pCmd;
					showhelp=FALSE;
				}
			}

			if (*farcmd==L'<')
			{
				pCmd=farcmd+1;
				outputtofile=1;
				ParseCmdSyntax(pCmd, ShowCmdOutput, stream);
			}
			else
			{
				if (pCmd)
				{
					wchar_t *Quote=wcschr(farcmd,L'\"');

					if (Quote)
					{
						if (Quote<=pCmd)
							pCmd=farcmd;
						else
							pCmd+=SeparatorLen;
					}
					else
						pCmd+=SeparatorLen;
				}
				else
				{
					pCmd=farcmd;
				}
			}

			if (pCmd)
			{
				FSF.LTrim(pCmd);

				if (!outputtofile)
				{
					if (*pCmd==L'<')
						outputtofile=1;

					pCmd+=outputtofile;
				}

				if (outputtofile && (View||Edit||Clip))
					ParseCmdSyntax(pCmd, ShowCmdOutput, stream);

				if (*pCmd && BracketsOk)
				{
					showhelp=FALSE;

					if (Goto)
					{
						wchar_t *Ptr=__proc_Goto(outputtofile,pCmd);
						if (Ptr)
						{
							lstrcpyn(selectItem,Ptr,ARRAYSIZE(selectItem)-1);
							delete[] Ptr;
						}
					}
					else if (WhereIs)
					{
						wchar_t *Ptr=__proc_WhereIs(outputtofile,pCmd);
						if (Ptr)
						{
							lstrcpyn(selectItem,Ptr,ARRAYSIZE(selectItem)-1);
							delete[] Ptr;
						}
					}
					else if (Load || Unload)  // <file>
					{
						__proc_Load_Unload(outputtofile,pCmd,Load?true:false);
					}
					else if (Link) //link [/msg] [/n] �������� ����������
					{
						if (!__proc_Link(outputtofile,pCmd))
							Info.ShowHelp(Info.ModuleName,(PrefIdx==static_cast<size_t>(-1))?L"Contents":Pref[PrefIdx].HelpName,0);
					}
					else
					{
						if (outputtofile)
							ProcessOSAliases(pCmd,ARRAYSIZE(farcmdbuf));

						wchar_t *tempDir = NULL, temp[MAX_PATH*5];
						wchar_t TempFileNameOut[MAX_PATH*5], TempFileNameErr[ARRAYSIZE(TempFileNameOut)];
						TempFileNameOut[0] = TempFileNameErr[0] = 0;

						if (outputtofile)
						{
							if (*pCmd == L'|')
							{
								wchar_t *endTempDir = wcschr(pCmd+1, L'|');

								if (endTempDir)
								{
									*endTempDir = 0;
									tempDir = pCmd+1;
									pCmd = endTempDir;
									FSF.LTrim(++pCmd);
								}
							}

							lstrcpy(temp,pCmd);
						}
						else
							FSF.Unquote(lstrcpy(temp,pCmd));

						wchar_t ExpTemp[ARRAYSIZE(temp)];
						ExpandEnvironmentStrings(temp,ExpTemp,ARRAYSIZE(ExpTemp));
						lstrcpy(temp,ExpTemp);

						// ���������� �������
						int catchStdOutput = stream != 2;
						int catchStdError  = stream != 1;
						int catchSeparate  = (stream == 3) && (View || Edit);
						int catchAllInOne  = 0;

						if (outputtofile)
						{
							if (Run)
							{
								if (*runFile)
								{
									FSF.Unquote(runFile);
									ExpandEnvironmentStrings(runFile,TempFileNameErr,ARRAYSIZE(TempFileNameErr));
									lstrcpy(TempFileNameOut,TempFileNameErr);
									allOK = TRUE;
								}
							}
							else
								allOK = MakeTempNames(TempFileNameOut, TempFileNameErr, ARRAYSIZE(TempFileNameOut));

							if (allOK)
							{
								DWORD InMode=0, OutMode=0, ErrMode=0;
								GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &OutMode);
								GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &ErrMode);
								GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &InMode);

								allOK = FALSE;
								lstrcpy(cmd,L"%COMSPEC% /c ");

								if (*temp == L'"')
									lstrcat(cmd, L"\"");

								lstrcat(cmd, temp);
								ExpandEnvironmentStrings(cmd, fullcmd, ARRAYSIZE(fullcmd));
								lstrcpy(cmd, temp);

								if (catchStdOutput && catchStdError)
								{
									if (catchSeparate)
									{
										if (ShowCmdOutput == 2)
											ShowCmdOutput = 1;
									}
									else
									{
										catchStdError = 0;
										catchAllInOne = 1;
									}
								}

								if (catchStdOutput)
								{
									static SECURITY_ATTRIBUTES sa;
									memset(&sa, 0, sizeof(sa));
									sa.nLength = sizeof(sa);
									sa.lpSecurityDescriptor = NULL;
									sa.bInheritHandle = TRUE;
									FileHandleOut = CreateFile(TempFileNameOut,GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
								}
								else
								{
									FileHandleOut = INVALID_HANDLE_VALUE;
									killTemp(TempFileNameOut);
									TempFileNameOut[0] = 0;
								}

								if (catchStdError)
								{
									static SECURITY_ATTRIBUTES sa;
									memset(&sa, 0, sizeof(sa));
									sa.nLength = sizeof(sa);
									sa.lpSecurityDescriptor = NULL;
									sa.bInheritHandle = TRUE;
									FileHandleErr = CreateFile(TempFileNameErr,GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
								}
								else
								{
									FileHandleErr = INVALID_HANDLE_VALUE;
									killTemp(TempFileNameErr);
									TempFileNameErr[0] = 0;
								}

								if ((!catchStdOutput || vh(FileHandleOut)) && (!catchStdError || vh(FileHandleErr)))
								{
									HANDLE hScreen=NULL, StdInput, StdOutput;
									CONSOLE_SCREEN_BUFFER_INFO csbi;
									DWORD ConsoleMode;
									StdInput  = GetStdHandle(STD_INPUT_HANDLE);
									StdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
									static STARTUPINFO si;
									memset(&si,0,sizeof(si));
									si.cb=sizeof(si);
									si.dwFlags=STARTF_USESTDHANDLES;
									si.hStdInput  = StdInput;
									si.hStdOutput = catchStdOutput ? FileHandleOut : StdOutput;
									si.hStdError  = catchStdError  ? FileHandleErr : StdOutput;

									if (catchAllInOne)
										si.hStdError = si.hStdOutput;

									TThreadData *td = NULL;
									hScreen = Info.SaveScreen(0, 0, -1, -1);

									if (ShowCmdOutput)
									{
										GetConsoleScreenBufferInfo(StdOutput,&csbi);
										wchar_t Blank[1024];
										wmemset(Blank, L' ', csbi.dwSize.X);

										FarColor Color={};
										Color.Flags = FCF_FG_4BIT|FCF_BG_4BIT;
										Color.ForegroundColor = LIGHTGRAY;
										Color.BackgroundColor = 0;
										for (int Y = 0 ; Y < csbi.dwSize.Y ; Y++)
											Info.Text(0, Y, &Color, Blank);

										Info.Text(0, 0, 0, NULL);
										COORD C;
										C.X = 0;
										C.Y = csbi.dwCursorPosition.Y;
										SetConsoleCursorPosition(StdOutput,C);
										GetConsoleMode(StdInput,&ConsoleMode);
										SetConsoleMode(StdInput,ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);
									}

									if (ShowCmdOutput == 2)
									{
										// ������ ��� ����� ������������� ������
										td = (TThreadData*)malloc(sizeof(TThreadData));

										if (td)
										{
											td->type = enThreadShowOutput;
											td->processDone = false;

											for (int i = 0 ; i < enStreamMAX ; i++)
											{
												TShowOutputStreamData *sd = &(td->stream[i]);
												sd->hRead = sd->hWrite = sd->hConsole = INVALID_HANDLE_VALUE;
											}

											if (catchStdError)
											{
												TShowOutputStreamData *sd = &(td->stream[enStreamErr]);
												static SECURITY_ATTRIBUTES sa;
												memset(&sa, 0, sizeof(sa));
												sa.nLength = sizeof(sa);
												sa.lpSecurityDescriptor = NULL;
												sa.bInheritHandle = TRUE;
												sd->hRead = CreateFile(TempFileNameErr,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
												sd->hWrite   = FileHandleErr;
												sd->hConsole = StdOutput;
											}

											if (catchStdOutput)
											{
												TShowOutputStreamData *sd = &(td->stream[enStreamOut]);
												static SECURITY_ATTRIBUTES sa;
												memset(&sa, 0, sizeof(sa));
												sa.nLength = sizeof(sa);
												sa.lpSecurityDescriptor = NULL;
												sa.bInheritHandle = TRUE;
												sd->hRead = CreateFile(TempFileNameOut,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
												sd->hWrite   = FileHandleOut;
												sd->hConsole = StdOutput;
											}
										}
									}

									wchar_t SaveDir[MAX_PATH], workDir[MAX_PATH];
									static PROCESS_INFORMATION pi;
									memset(&pi,0,sizeof(pi));

									if (tempDir)
									{
										GetCurrentDirectory(ARRAYSIZE(SaveDir),SaveDir);
										ExpandEnvironmentStrings(tempDir,workDir,ARRAYSIZE(workDir));
										SetCurrentDirectory(workDir);
									}

									wchar_t consoleTitle[256];
									DWORD tlen = GetConsoleTitle(consoleTitle, 256);
									SetConsoleTitle(cmd);
									LPTSTR CurDir=NULL;
									size_t Size=FSF.GetCurrentDirectory(0,NULL);

									if (Size)
									{
										CurDir=new WCHAR[Size];
										FSF.GetCurrentDirectory(Size,CurDir);
									}

									BOOL Created=CreateProcess(NULL,fullcmd,NULL,NULL,TRUE,0,NULL,CurDir,&si,&pi);

									if (CurDir)
									{
										delete[] CurDir;
									}

									if (Created)
									{
										// ����� ������������� ������
										HANDLE hThread;
										DWORD dummy;

										if (td)
										{
											hThread = CreateThread(NULL, 0xf000, ThreadWhatUpdateScreen, td, 0, &dummy);
											WaitForSingleObject(pi.hProcess, INFINITE);
											closeHandle(FileHandleOut);
											closeHandle(FileHandleErr);
											td->processDone = true;

											if (hThread)
												WaitForSingleObject(hThread, INFINITE);

											for (int i = 0 ; i < enStreamMAX ; i++)
											{
												TShowOutputStreamData *sd = &(td->stream[i]);

												if (vh(sd->hWrite) && vh(sd->hRead))
													while (showPartOfOutput(sd))
														;

												closeHandle(sd->hRead);
											}

											free(td);
										}
										else
										{
											if (ShowCmdOutput == 0)
												td = (TThreadData*)malloc(sizeof(TThreadData));

											if (td)
											{
												td->type = enThreadHideOutput;
												lstrcpy(td->title, GetMsg(MConfig));
												lstrcpy(td->cmd, cmd);
												td->stream[enStreamOut].hWrite = FileHandleOut;
												td->stream[enStreamErr].hWrite = FileHandleErr;
												td->processDone = false;
												hThread = CreateThread(NULL, 0xf000, ThreadWhatUpdateScreen, td, 0, &dummy);
											}

											WaitForSingleObject(pi.hProcess, INFINITE);
											closeHandle(FileHandleOut);
											closeHandle(FileHandleErr);

											if (td)
											{
												td->processDone = true;

												if (hThread)
													WaitForSingleObject(hThread, INFINITE);

												free(td);
											}
										}

										closeHandle(pi.hThread);
										closeHandle(pi.hProcess);
										closeHandle(hThread);
										allOK=TRUE;
									}
									else
									{
										closeHandle(FileHandleOut);
										closeHandle(FileHandleErr);
									}

									if (tlen)
										SetConsoleTitle(consoleTitle);

									if (tempDir)
										SetCurrentDirectory(SaveDir);

									if (ShowCmdOutput)
									{
										SetConsoleMode(StdInput, ConsoleMode);
										SMALL_RECT src;
										COORD dest;
										CHAR_INFO fill;
										src.Left = 0;
										src.Top = 2;
										src.Right = csbi.dwSize.X;
										src.Bottom = csbi.dwSize.Y;
										dest.X = dest.Y = 0;
										fill.Char.UnicodeChar = L' ';
										fill.Attributes = LIGHTGRAY;
										ScrollConsoleScreenBuffer(StdOutput, &src, NULL, dest, &fill);
										Info.PanelControl(PANEL_ACTIVE, FCTL_SETUSERSCREEN,0,0);
									}

									Info.RestoreScreen(hScreen);
								}

								SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), OutMode);
								SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), ErrMode);
								SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), InMode);
							}
						}
						else
						{
							FSF.ConvertPath(CPM_FULL, temp, TempFileNameOut, ARRAYSIZE(TempFileNameOut));
						}

						if (allOK)
						{
							if (View || Edit)
							{
								wchar_t titleOut[MAX_PATH] = L"", titleErr[MAX_PATH] = L"";

								if (catchStdError && ((catchStdOutput && catchSeparate) || !catchStdOutput))
									lstrcpy(titleErr, GetMsg(MStdErr));

								if (catchStdError && catchStdOutput && catchSeparate)
									lstrcpy(titleOut, GetMsg(MStdOut));

								if (View)
								{
									DWORD Flags = VF_NONMODAL|VF_ENABLE_F6|VF_IMMEDIATERETURN;

									if (outputtofile)
										Flags |= VF_DISABLEHISTORY|VF_DELETEONCLOSE;

									if (validForView(TempFileNameErr, Opt.ViewZeroFiles, 0))
									{
										FSF.sprintf(fullcmd, L"%s%s", titleErr, cmd);
										Info.Viewer(TempFileNameErr,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags,CP_DEFAULT);
									}
									else if (outputtofile)
										killTemp(TempFileNameErr);

									if (validForView(TempFileNameOut, Opt.ViewZeroFiles, 0))
									{
										FSF.sprintf(fullcmd, L"%s%s", titleOut, cmd);
										Info.Viewer(TempFileNameOut,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags,CP_DEFAULT);
									}
									else if (outputtofile)
										killTemp(TempFileNameOut);

									outputtofile=FALSE;
								}
								else if (Edit)
								{
									DWORD Flags=EF_NONMODAL/*|EF_CREATENEW*/|EF_ENABLE_F6|EF_IMMEDIATERETURN;

									if (outputtofile)
										Flags |= EF_DISABLEHISTORY|EF_DELETEONCLOSE;

									if (validForView(TempFileNameErr, Opt.ViewZeroFiles, Opt.EditNewFiles))
									{
										FSF.sprintf(fullcmd, L"%s%s", titleErr, cmd);
										Info.Editor(TempFileNameErr,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags,StartLine,StartChar,CP_DEFAULT);
									}
									else if (outputtofile)
										killTemp(TempFileNameErr);

									if (validForView(TempFileNameOut, Opt.ViewZeroFiles, Opt.EditNewFiles))
									{
										FSF.sprintf(fullcmd, L"%s%s", titleOut, cmd);
										Info.Editor(TempFileNameOut,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags,StartLine,StartChar,CP_DEFAULT);
									}
									else if (outputtofile)
										killTemp(TempFileNameOut);

									outputtofile=FALSE;
								}
							}
							else if (Run)
							{
								outputtofile=FALSE;
							}
							else if (Clip)
							{
								size_t shift;
								bool foundFile;
								wchar_t *Ptr = loadFile(TempFileNameOut, Opt.MaxDataSize/sizeof(wchar_t), outputtofile, shift, foundFile);

								FSF.CopyToClipboard(FCT_STREAM,Ptr?Ptr+shift:nullptr);
								if (Ptr)
									delete[] Ptr;
							}
						}

						if (outputtofile)
						{
							killTemp(TempFileNameOut);
							killTemp(TempFileNameErr);
						}
					}
				} // </if(*pCmd && BracketsOk)>
			} // </if(pCmd)>
		} // </if(View||Edit||Goto)>
	} // </if(lstrlen(farcmd) > 3)>

	if (showhelp)
	{
		Info.ShowHelp(Info.ModuleName,(PrefIdx==static_cast<size_t>(-1))?L"Contents":Pref[PrefIdx].HelpName,0);
		return nullptr;
	}

	return selectItem;
}
#endif
