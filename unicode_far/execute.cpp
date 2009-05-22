/*
execute.cpp

"�����������" ��������.
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

#include "keyboard.hpp"
#include "filepanels.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "chgprior.hpp"
#include "cmdline.hpp"
#include "panel.hpp"
#include "rdrwdsk.hpp"
#include "udlist.hpp"
#include "imports.hpp"
#include "registry.hpp"
#include "localOEM.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "iswind.hpp"
#include "message.hpp"

static const wchar_t strSystemExecutor[]=L"System\\Executor";

// ��������� ����� �� �������� GetFileInfo, �������� ����������� ���������� � ���� PE-������

// ���������� ��������� IMAGE_SUBSYSTEM_* ���� ������� ��������
// ��� ������ �� ��������� IMAGE_SUBSYTEM_UNKNOWN ��������
// "���� �� �������� �����������".
// ��� DOS-���������� ��������� ��� ���� �������� �����.

//#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

struct IMAGE_HEADERS
{
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	union
	{
		IMAGE_OPTIONAL_HEADER32 OptionalHeader32;
		IMAGE_OPTIONAL_HEADER64 OptionalHeader64;
	};
};

static bool GetImageSubsystem(const wchar_t *FileName,DWORD& ImageSubsystem)
{
	bool Result=false;
	ImageSubsystem=IMAGE_SUBSYSTEM_UNKNOWN;
	HANDLE hModuleFile=apiCreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0);
	if(hModuleFile!=INVALID_HANDLE_VALUE)
	{
		IMAGE_DOS_HEADER DOSHeader;
		DWORD ReadSize;
		if(ReadFile(hModuleFile,&DOSHeader,sizeof(DOSHeader),&ReadSize,NULL))
		{
			if(DOSHeader.e_magic==IMAGE_DOS_SIGNATURE)
			{
				//ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
				Result=true;
				if(apiSetFilePointerEx(hModuleFile,DOSHeader.e_lfanew,NULL,FILE_BEGIN))
				{
					IMAGE_HEADERS PEHeader;
					if(ReadFile(hModuleFile,&PEHeader,sizeof(PEHeader),&ReadSize,NULL))
					{
						if(PEHeader.Signature==IMAGE_NT_SIGNATURE)
						{
							switch(PEHeader.OptionalHeader32.Magic)
							{
							case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
								{
									ImageSubsystem=PEHeader.OptionalHeader32.Subsystem;
								}
								break;
							case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
								{
									ImageSubsystem=PEHeader.OptionalHeader64.Subsystem;
								}
								break;
							/*default:
								{
									// unknown magic
								}*/
							}
						}
						else if((WORD)PEHeader.Signature==IMAGE_OS2_SIGNATURE)
						{
							/*
							NE,  ���...  � ��� ���������� ��� ��� ������?

							Andrzej Novosiolov <andrzej@se.kiev.ua>
							AN> ��������������� �� ����� "Target operating system" NE-���������
							AN> (1 ���� �� �������� 0x36). ���� ��� Windows (�������� 2, 4) - �������������
							AN> GUI, ���� OS/2 � ������ �������� (��������� ��������) - ������������� �������.
							*/
							BYTE ne_exetyp=reinterpret_cast<PIMAGE_OS2_HEADER>(&PEHeader)->ne_exetyp;
							if(ne_exetyp==2||ne_exetyp==4)
							{
								ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_GUI;
							}
						}
						/*else
						{
							// unknown signature
						}*/
					}
					/*else
					{
						// ������ ����� � ������� ���������� ��������� ;-(
					}*/
				}
				/*else
				{
					// ������ ������� ���� ���� � �����, �.�. dos_head.e_lfanew ������
					// ������� � �������������� ����� (�������� ��� ������ ���� DOS-����)
				}*/
			}
			/*else
			{
				// ��� �� ����������� ���� - � ���� ���� ��������� MZ, ��������, NLM-������
				// TODO: ����� ����� ��������� POSIX �������, �������� "/usr/bin/sh"
			}*/
		}
		/*else
		{
			// ������ ������
		}*/
		CloseHandle(hModuleFile);
	}
	/*else
	{
		// ������ ��������
	}*/
	return Result;
}

bool GetShellType(const wchar_t *Ext, string &strType,ASSOCIATIONTYPE aType)
{
	bool bVistaType = false;

	strType.SetLength(0);

	if (WinVer.dwMajorVersion >= 6 && ifn.pfnSHCreateAssociationRegistration)
	{
		IApplicationAssociationRegistration* pAAR;
		HRESULT hr = ifn.pfnSHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, (void**)&pAAR);
		if (SUCCEEDED(hr))
		{
			wchar_t *p;
			if (pAAR->QueryCurrentDefault(Ext, aType, AL_EFFECTIVE, &p) == S_OK)
			{
				bVistaType = true;
				strType = p;
				CoTaskMemFree(p);
			}
			pAAR->Release();
		}
	}

	if (!bVistaType)
	{
		HKEY hKey;
		if (RegOpenKeyW(HKEY_CLASSES_ROOT,Ext,&hKey)!=ERROR_SUCCESS)
			return false;
		if(aType==AT_URLPROTOCOL)
		{
			strType=Ext;
		}
		else
		{
			if (RegQueryStringValue(hKey,L"",strType,L"")!=ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
				return false;
			}
		}
		RegCloseKey(hKey);
	}

	return !strType.IsEmpty();
}

// �� ����� ����� (�� ��� ����������) �������� ������� ���������
// ������������� ��������� �������� �������-����������
// (����� �� ����� ����������)
const wchar_t *GetShellAction(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& Error)
{
  string strValue;
  string strNewValue;
  const wchar_t *ExtPtr;
  const wchar_t *RetPtr;
  const wchar_t command_action[]=L"\\command";

  Error = ERROR_SUCCESS;
  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

  if ((ExtPtr=wcsrchr(FileName,L'.'))==NULL)
    return(NULL);

  if (!GetShellType(ExtPtr, strValue))
  	return NULL;

	HKEY hKey;
	if(RegOpenKeyExW(HKEY_CLASSES_ROOT,(const wchar_t *)strValue,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
	{
		int nResult=RegQueryValueExW(hKey,L"IsShortcut",NULL,NULL,NULL,NULL);
		RegCloseKey(hKey);
		if(nResult==ERROR_SUCCESS)
			return NULL;
	}

  strValue += L"\\shell";

  if (RegOpenKeyW(HKEY_CLASSES_ROOT,(const wchar_t *)strValue,&hKey)!=ERROR_SUCCESS)
    return(NULL);

  static string strAction;

  int RetQuery = RegQueryStringValueEx(hKey,L"",strAction,L"");

  strValue += L"\\";

  if (RetQuery == ERROR_SUCCESS)
  {
    UserDefinedList ActionList(0,0,ULF_UNIQUE);

    RetPtr = (strAction.IsEmpty() ? NULL : (const wchar_t *)strAction);
    const wchar_t *ActionPtr;

    LONG RetEnum = ERROR_SUCCESS;
    if (RetPtr != NULL && ActionList.Set(strAction))
    {
      HKEY hOpenKey;

      ActionList.Reset();
      while (RetEnum == ERROR_SUCCESS && (ActionPtr = ActionList.GetNext()) != NULL)
      {
        strNewValue = strValue;
        strNewValue += ActionPtr;
        strNewValue += command_action;

        if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          strValue += ActionPtr;
          strAction = ActionPtr;
          RetPtr = (const wchar_t *)strAction;
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* while */
    } /* if */
    else
      strValue += strAction;

    if(RetEnum != ERROR_NO_MORE_ITEMS) // ���� ������ �� �����, ��...
      RetPtr=NULL;
  }
  else
  {
    // This member defaults to "Open" if no verb is specified.
    // �.�. ���� �� ������� NULL, �� ��������������� ������� "Open"
    RetPtr=NULL;
  }

  // ���� RetPtr==NULL - �� �� ����� default action.
  // ��������� - ���� �� ������ ���-������ � ����� ����������
  if (RetPtr==NULL)
  {
    LONG RetEnum = ERROR_SUCCESS;
    DWORD dwIndex = 0;
    HKEY hOpenKey;

    // ������� �������� "open"...
    strAction = L"open";

    strNewValue = strValue;
    strNewValue += strAction;
    strNewValue += command_action;

    if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
    {
      RegCloseKey(hOpenKey);
      strValue += strAction;
      RetPtr = (const wchar_t *)strAction;
      RetEnum = ERROR_NO_MORE_ITEMS;
    } /* if */

    // ... � ������ ��� ���������, ���� "open" ����
    while (RetEnum == ERROR_SUCCESS)
    {
			RetEnum=apiRegEnumKeyEx(hKey, dwIndex++,strAction);
      if (RetEnum == ERROR_SUCCESS)
      {
        // �������� ������� "�������" � ����� �����
        strNewValue = strValue;
        strNewValue += strAction;
        strNewValue += command_action;
        if (RegOpenKeyW(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
        {
          RegCloseKey(hOpenKey);
          strValue += strAction;
          RetPtr = (const wchar_t *)strAction;
          RetEnum = ERROR_NO_MORE_ITEMS;
        } /* if */
      } /* if */
    } /* while */
  } /* if */

  RegCloseKey(hKey);

  if (RetPtr != NULL)
  {
    strValue += command_action;

    // � ������ �������� �������� ����������� �����
    if (RegOpenKeyW(HKEY_CLASSES_ROOT,strValue,&hKey)==ERROR_SUCCESS)
    {
      RetQuery=RegQueryStringValueEx(hKey,L"",strNewValue,L"");
      RegCloseKey(hKey);

      if(RetQuery == ERROR_SUCCESS && !strNewValue.IsEmpty())
      {
        apiExpandEnvironmentStrings(strNewValue,strNewValue);

        wchar_t *Ptr = strNewValue.GetBuffer ();
        // �������� ��� ������
        if (*Ptr==L'\"')
        {
          wchar_t *QPtr = wcschr(Ptr + 1,L'\"');
          if (QPtr!=NULL)
          {
            *QPtr=0;
            wmemmove(Ptr, Ptr + 1, QPtr-Ptr);
          }
        }
        else
        {
          if ((Ptr=wcspbrk(Ptr,L" \t/"))!=NULL)
            *Ptr=0;
        }

        strNewValue.ReleaseBuffer ();

				GetImageSubsystem(strNewValue,ImageSubsystem);
      }
      else
      {
        Error=ERROR_NO_ASSOCIATION;
        RetPtr=NULL;
      }
    }
  }

  return RetPtr;
}

/*
������ FindModule �������� ����� ����������� ������ (� �.�. � ��
%PATHEXT%). � ������ ������ �������� � Module ������, ������������� ��
����������� ������ �� ��������� ��������, �������� ��������� � strDest �
�������� ��������� ��������� PE �� �������� (����� ��������� �������
� ��������� ���� � �� ����� ����������).
� ������ ������� strDest �� �����������!
Return: true/false - �����/�� �����
������� � ������� ���������� ��� �������������. ������ �� ������.
� ��������� ������ �� ����, �.�. ��� ��������� �� ������� ������
*/

bool WINAPI FindModule(const wchar_t *Module, string &strDest,DWORD &ImageSubsystem)
{
	bool Result=false;
	ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;
	if(Module&&*Module)
	{
		// ������� ������ - ������� ����������
		// ����� "����������" �� �������, ������� ������ ����������� ��������,
		// ��������, ��������� ���������� ������� ���. ����������.
		if(GetRegKey(strSystemExecutor,L"Type",0))
		{
			string strExcludeCmds;
			GetRegKey(strSystemExecutor,L"ExcludeCmds",strExcludeCmds,L"");
			UserDefinedList ExcludeCmdsList;
			ExcludeCmdsList.Set(strExcludeCmds);
			while(!ExcludeCmdsList.IsEmpty())
			{
				if(!StrCmpI(Module,ExcludeCmdsList.GetNext()))
				{
					ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_CUI;
					Result=true;
					break;
				}
			}
		}
		if(!Result)
		{
			string strFullName=Module;
			LPCWSTR ModuleExt=wcsrchr(Module,L'.');
			string strPathExt=L".COM;.EXE;.BAT;.CMD;.VBS;.JS;.WSH";
			apiGetEnvironmentVariable(L"PATHEXT",strPathExt);
			UserDefinedList PathExtList;
			PathExtList.Set(strPathExt);
			PathExtList.Reset();
			while(!PathExtList.IsEmpty()) // ������ ������ - � ������� ��������
			{
				LPCWSTR Ext=PathExtList.GetNext();
				string strTmpName=strFullName;
				if(!ModuleExt)
				{
					strTmpName+=Ext;
				}
				DWORD Attr=apiGetFileAttributes(strTmpName);
				if((Attr!=INVALID_FILE_ATTRIBUTES)&&!(Attr&FILE_ATTRIBUTE_DIRECTORY))
				{
					ConvertNameToFull(strTmpName,strFullName);
					Result=true;
					break;
				}
				if(ModuleExt)
				{
					break;
				}
			}
			if(!Result) // ������ ������ - �� �������� SearchPath
			{
				string strPathEnv;
				if(apiGetEnvironmentVariable(L"PATH",strPathEnv))
				{
					PathExtList.Reset();
					while(!PathExtList.IsEmpty())
					{
						LPCWSTR Ext=PathExtList.GetNext();
						DWORD dwSize=SearchPathW(strPathEnv,strFullName,Ext,0,NULL,NULL);
						if(dwSize)
						{
							wchar_t *lpwszFullName=strFullName.GetBuffer(dwSize);
							SearchPathW(strPathEnv,string(lpwszFullName),Ext,dwSize,lpwszFullName,NULL);
							strFullName.ReleaseBuffer();
							Result=true;
							break;
						}
					}
				}
				if(!Result)
				{
					PathExtList.Reset();
					while(!PathExtList.IsEmpty())
					{
						LPCWSTR Ext=PathExtList.GetNext();
						DWORD dwSize=SearchPathW(NULL,strFullName,Ext,0,NULL,NULL);
						if(dwSize)
						{
							wchar_t *lpwszFullName=strFullName.GetBuffer(dwSize);
							SearchPathW(NULL,string(lpwszFullName),Ext,dwSize,lpwszFullName,NULL);
							strFullName.ReleaseBuffer();
							Result=true;
							break;
						}
					}
				}
				if(!Result && Opt.ExecuteUseAppPath) // ������ ������ - ����� � ������ � "App Paths"
				{
					LPCWSTR RegPath=L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
					// � ������ Module ������� ����������� ������ �� ������ ����, �������
					// ������� �� SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
					// ������� ������� � HKCU, ����� - � HKLM
					HKEY RootFindKey[]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE};
					strFullName=RegPath;
					strFullName+=Module;
					for(size_t i=0;i<countof(RootFindKey);i++)
					{
						HKEY hKey;
						if (RegOpenKeyExW(RootFindKey[i],strFullName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
						{
							int RegResult=RegQueryStringValueEx(hKey,L"",strFullName,L"");
							RegCloseKey(hKey);
							if(RegResult==ERROR_SUCCESS)
							{
								apiExpandEnvironmentStrings(strFullName,strFullName);
								Unquote(strFullName);
								Result=true;
								break;
							}
						}
					}
					if(!Result)
					{
						PathExtList.Reset();
						while(!PathExtList.IsEmpty()&&!Result)
						{
							LPCWSTR Ext=PathExtList.GetNext();
							strFullName=RegPath;
							strFullName+=Module;
							strFullName+=Ext;
							for(size_t i=0;i<countof(RootFindKey);i++)
							{
								HKEY hKey;
								if(RegOpenKeyExW(RootFindKey[i],strFullName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
								{
									int RegResult=RegQueryStringValueEx(hKey,L"",strFullName,L"");
									RegCloseKey(hKey);
									if(RegResult==ERROR_SUCCESS)
									{
										apiExpandEnvironmentStrings(strFullName,strFullName);
										Unquote(strFullName);
										Result=true;
										break;
									}
								}
							}
						}
					}
				}
			}
			if(Result) // ��������� "�������" ������
			{
				GetImageSubsystem(strFullName,ImageSubsystem);
				strDest=strFullName;
			}
		}
	}
	return Result;
}



/* �������-��������� ������� ���������
   ���������� -1 � ������ ������ ���...
*/
int Execute(const wchar_t *CmdStr,    // ���.������ ��� ����������
            int AlwaysWaitFinish,  // ����� ���������� ��������?
            int SeparateWindow,    // ��������� � ��������� ����? =2 ��� ������ ShellExecuteEx()
            int DirectRun,         // ��������� ��������? (��� CMD)
            int FolderRun)         // ��� ������?
{
  int nResult = -1;

  string strNewCmdStr;
  string strNewCmdPar;
  string strExecLine;


  PartCmdLine(
          CmdStr,
          strNewCmdStr,
          strNewCmdPar
          );

  /* $ 05.04.2005 AY: ��� �� ���������, ���� ������� ������ ������ ������,
                      ��� ������ � ������ PartCmdLine.
  if(*NewCmdPar)
    RemoveExternalSpaces(NewCmdPar);
  AY $ */

	DWORD dwAttr = apiGetFileAttributes(strNewCmdStr);

  if ( SeparateWindow == 1 )
  {
      if ( strNewCmdPar.IsEmpty() && dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
      {
          ConvertNameToFull(strNewCmdStr, strNewCmdStr);
          SeparateWindow=2;
          FolderRun=TRUE;
      }
  }

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  memset (&si, 0, sizeof (si));

  si.cb = sizeof (si);

  string strComspec;
  apiGetEnvironmentVariable (L"COMSPEC", strComspec);

  if ( strComspec.IsEmpty() && (SeparateWindow != 2) )
  {
    Message(MSG_WARNING, 1, MSG(MWarning), MSG(MComspecNotFound), MSG(MErrorCancelled), MSG(MOk));
    return -1;
  }

  int Visible, Size;

  GetCursorType(Visible,Size);
  SetInitialCursorType();

  ScrBuf.SetLockCount(0);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  int ConsoleCP = GetConsoleCP();
  int ConsoleOutputCP = GetConsoleOutputCP();

  FlushInputBuffer();
  ChangeConsoleMode(InitialConsoleMode);

  CONSOLE_SCREEN_BUFFER_INFO sbi={0,};
  GetConsoleScreenBufferInfo(hConOut,&sbi);

  wchar_t OldTitle[512];
	GetConsoleTitleW(OldTitle, countof(OldTitle));

  DWORD dwSubSystem;
  DWORD dwError = 0;

  HANDLE hProcess = NULL, hThread = NULL;
	LPCWSTR lpVerb=NULL;

  if(FolderRun && SeparateWindow==2)
    AddEndSlash(strNewCmdStr); // ����, ����� ShellExecuteEx "�������" BAT/CMD/��.�����, �� �� �������
  else
  {
		FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem);

    if(/*!*NewCmdPar && */ dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
    {
      DWORD Error=0, dwSubSystem2=0;
      const wchar_t *ExtPtr=wcsrchr(strNewCmdStr,L'.');

      if(ExtPtr && !(StrCmpI(ExtPtr,L".exe")==0 || StrCmpI(ExtPtr,L".com")==0 ||
         IsBatchExtType(ExtPtr)))
			{
				lpVerb=GetShellAction(strNewCmdStr,dwSubSystem2,Error);
				if(lpVerb && Error != ERROR_NO_ASSOCIATION)
				{
					dwSubSystem=dwSubSystem2;
				}
			}
    }

    if ( dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI )
      SeparateWindow = 2;
  }

  ScrBuf.Flush ();

  if ( SeparateWindow == 2 )
  {
		SHELLEXECUTEINFOW seInfo;
		memset (&seInfo, 0, sizeof (seInfo));
		seInfo.cbSize = sizeof (seInfo);
    seInfo.lpFile = strNewCmdStr;
    seInfo.lpParameters = strNewCmdPar;
    seInfo.nShow = SW_SHOWNORMAL;

		seInfo.lpVerb = (dwAttr&FILE_ATTRIBUTE_DIRECTORY)?NULL:lpVerb?lpVerb:GetShellAction(strNewCmdStr, dwSubSystem, dwError);
    //seInfo.lpVerb = "open";
		seInfo.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_FLAG_DDEWAIT|SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOZONECHECKS;

    if ( !dwError )
    {
      if ( ShellExecuteExW (&seInfo) )
      {
        hProcess = seInfo.hProcess;
        StartExecTime=clock();
      }
      else
        dwError = GetLastError ();
    }
  }
  else
  {
    string strFarTitle;
    if(!Opt.ExecuteFullTitle)
    {
      strFarTitle=CmdStr;
    }
    else
    {
      strFarTitle = strNewCmdStr;
      if ( !strNewCmdPar.IsEmpty() )
      {
        strFarTitle += L" ";
        strFarTitle += strNewCmdPar;
      }
    }
    SetConsoleTitleW(strFarTitle);

    if (SeparateWindow)
      si.lpTitle=(wchar_t*)(const wchar_t*)strFarTitle;

    QuoteSpace(strNewCmdStr);

    strExecLine = strComspec;
    strExecLine += L" /C ";

    bool bDoubleQ = false;

    if ( wcspbrk (strNewCmdStr, L"&<>()@^|=;,") )
      bDoubleQ = true;

    if ( !strNewCmdPar.IsEmpty() || bDoubleQ )
      strExecLine += L"\"";

    strExecLine += strNewCmdStr;

    if ( !strNewCmdPar.IsEmpty() )
    {
      strExecLine += L" ";
      strExecLine += strNewCmdPar;
    }

    if ( !strNewCmdPar.IsEmpty() || bDoubleQ )
      strExecLine += L"\"";

    // // ������� ������ � ����� ����� � 4NT ��� ������ �������
    SetRealColor (COL_COMMANDLINEUSERSCREEN);

    if ( CreateProcessW (
        NULL,
        (wchar_t*)(const wchar_t*)strExecLine,
        NULL,
        NULL,
        false,
        SeparateWindow?CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE:CREATE_DEFAULT_ERROR_MODE,
        NULL,
        NULL,
        &si,
        &pi
        ) )
     {
       hProcess = pi.hProcess;
       hThread = pi.hThread;

       StartExecTime=clock();
     }
    else
       dwError = GetLastError ();
  }

  if ( !dwError )
  {
    if ( hProcess )
    {
      ScrBuf.Flush ();

      if ( AlwaysWaitFinish || !SeparateWindow )
      {
        if ( Opt.ConsoleDetachKey == 0 )
          WaitForSingleObject(hProcess,INFINITE);
        else
        {
          /*$ 12.02.2001 SKV
            ����� ����� ;)
            ��������� ��������� ������� �� ���������������� ��������.
            ������� ������� � System/ConsoleDetachKey
          */
          HANDLE hHandles[2];
          HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
          HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

          INPUT_RECORD ir[256];
          DWORD rd;

          int vkey=0,ctrl=0;
          TranslateKeyToVK(Opt.ConsoleDetachKey,vkey,ctrl,NULL);
          int alt=ctrl&PKF_ALT;
          int shift=ctrl&PKF_SHIFT;
          ctrl=ctrl&PKF_CONTROL;

          hHandles[0] = hProcess;
          hHandles[1] = hInput;

          bool bAlt, bShift, bCtrl;
          DWORD dwControlKeyState;

          while( WaitForMultipleObjects (
              2,
              hHandles,
              FALSE,
              INFINITE
              ) != WAIT_OBJECT_0
              )
          {
            if ( PeekConsoleInputW(hHandles[1],ir,256,&rd) && rd)
            {
              int stop=0;

              for(DWORD i=0;i<rd;i++)
              {
                PINPUT_RECORD pir=&ir[i];

                if(pir->EventType==KEY_EVENT)
                {
                  dwControlKeyState = pir->Event.KeyEvent.dwControlKeyState;

                  bAlt = (dwControlKeyState & LEFT_ALT_PRESSED) || (dwControlKeyState & RIGHT_ALT_PRESSED);
                  bCtrl = (dwControlKeyState & LEFT_CTRL_PRESSED) || (dwControlKeyState & RIGHT_CTRL_PRESSED);
                  bShift = (dwControlKeyState & SHIFT_PRESSED) != 0;

                  if ( vkey==pir->Event.KeyEvent.wVirtualKeyCode &&
                     (alt ?bAlt:!bAlt) &&
                     (ctrl ?bCtrl:!bCtrl) &&
                     (shift ?bShift:!bShift) )
                  {
                    HICON hSmallIcon=NULL,hLargeIcon=NULL;

                    if ( hFarWnd )
                    {
                      hSmallIcon = CopyIcon((HICON)SendMessageW(hFarWnd,WM_SETICON,0,(LPARAM)0));
                      hLargeIcon = CopyIcon((HICON)SendMessageW(hFarWnd,WM_SETICON,1,(LPARAM)0));
                    }

                    ReadConsoleInputW(hInput,ir,256,&rd);

                    /*
                      �� ����� ������� CloseConsole, ������, ��� ��� ��������
                      ConsoleMode �� ���, ��� ��� �� ������� Far'�,
                      ���� ���������� ���������� ����� � �� �������.
                    */
                    CloseHandle(hInput);
                    CloseHandle(hOutput);

                    delete KeyQueue;
                    KeyQueue=NULL;

                    FreeConsole();
                    AllocConsole();

                    if ( hFarWnd ) // ���� ���� ����� HOTKEY, �� ������ ������ ��� ������.
                      SendMessageW(hFarWnd,WM_SETHOTKEY,0,(LPARAM)0);

                    SetConsoleScreenBufferSize(hOutput,sbi.dwSize);
                    SetConsoleWindowInfo(hOutput,TRUE,&sbi.srWindow);
                    SetConsoleScreenBufferSize(hOutput,sbi.dwSize);

                    Sleep(100);
                    InitConsole(0);

                    hFarWnd = 0;
                    InitDetectWindowedMode();

                    if ( hFarWnd )
                    {
                      if ( Opt.SmallIcon )
                      {
                        string strFarName;
                        apiGetModuleFileName (NULL, strFarName);
                        ExtractIconExW(strFarName,0,&hLargeIcon,&hSmallIcon,1);
                      }

                      if ( hLargeIcon != NULL )
                        SendMessageW(hFarWnd,WM_SETICON,1,(LPARAM)hLargeIcon);

                      if ( hSmallIcon != NULL )
                        SendMessageW(hFarWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
                    }

                    stop=1;
                    break;
                  }
                }
              }

              if ( stop )
                break;
            }

            Sleep(100);
          }
        }
      }

//      MessageBox (0, "close", "asd", MB_OK);

      ScrBuf.FillBuf();

      CloseHandle (hProcess);
    }

     if ( hThread )
       CloseHandle (hThread);

    nResult = 0;
  }
  else
  {
    string strOutStr;

    if( Opt.ExecuteShowErrorMessage )
    {
      SetMessageHelp(L"ErrCannotExecute");

      strOutStr = strNewCmdStr;
      Unquote(strOutStr);

      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),strOutStr,MSG(MOk));
    }
    else
    {
      ScrBuf.Flush ();

      strOutStr.Format (MSG(MExecuteErrorMessage),(const wchar_t *)strNewCmdStr);
      string strPtrStr=FarFormatText(strOutStr,ScrX, strPtrStr,L"\n",0);

      wprintf(strPtrStr);

      ScrBuf.FillBuf();
    }

  }

  SetFarConsoleMode(TRUE);

  /* �������������� ��������� �������, �.�. SetCursorType ������ �� �������
      ���������� ����� �����������, ������� � ������ ������ ������� �����.
  */
  SetCursorType(Visible,Size);
  SetRealCursorType(Visible,Size);

  SetConsoleTitleW(OldTitle);

  /* ���� ���� �������� ������� �������, ��������
     mode con lines=50 cols=100
     �� ��� �� ���� �� ��������� ������� �������.
     ��� ����� ���� ���� ��������� ������ ��� :-)
  */
  GenerateWINDOW_BUFFER_SIZE_EVENT(-1,-1); //����...

  if( Opt.RestoreCPAfterExecute )
  {
    // ����������� CP-������� ����� ���������� �����
    SetConsoleCP(ConsoleCP);
    SetConsoleOutputCP(ConsoleOutputCP);
  }


  return nResult;
}


int CommandLine::CmdExecute(const wchar_t *CmdLine,int AlwaysWaitFinish,
                            int SeparateWindow,int DirectRun)
{
  LastCmdPartLength=-1;

  if (!SeparateWindow && CtrlObject->Plugins.ProcessCommandLine(CmdLine))
  {
    /* $ 12.05.2001 DJ
       �������� ������ ���� �������� ������� �������
    */
    if (CtrlObject->Cp()->IsTopFrame())
    {
      //CmdStr.SetString(L"");
      GotoXY(X1,Y1);
      mprintf(L"%*s",X2-X1+1,L"");
      Show();
      ScrBuf.Flush();
    }
    return(-1);
  }
  int Code;
  /* 21.11.2001 VVM
    ! � ��������� ��� �������� � ����������� ����.
      ����� �� ������ ������� ����� :) */
  {
    CONSOLE_SCREEN_BUFFER_INFO sbi0,sbi1;
    GetConsoleScreenBufferInfo(hConOut,&sbi0);
    {
      RedrawDesktop Redraw(TRUE);

      ScrollScreen(1);
      MoveCursor(X1,Y1);
      if ( !strCurDir.IsEmpty() && strCurDir.At(1)==L':')
        FarChDir(strCurDir);
      CmdStr.SetString(L"");
      if ((Code=ProcessOSCommands(CmdLine,SeparateWindow)) == TRUE)
        Code=-1;
      else
      {
        string strTempStr;
        strTempStr = CmdLine;
        if(Code == -1)
          ReplaceStrings(strTempStr,L"/",L"\\",-1);
        Code=Execute(strTempStr,AlwaysWaitFinish,SeparateWindow,DirectRun);
      }

      GetConsoleScreenBufferInfo(hConOut,&sbi1);
      if(!(sbi0.dwSize.X == sbi1.dwSize.X && sbi0.dwSize.Y == sbi1.dwSize.Y))
        CtrlObject->CmdLine->CorrectRealScreenCoord();

      //if(Code != -1)
      {
        int CurX,CurY;
        GetCursorPos(CurX,CurY);
        if (CurY>=Y1-1)
          ScrollScreen(Min(CurY-Y1+2,2/*Opt.ShowKeyBar ? 2:1*/));
      }
    }

    if(!Flags.Check(FCMDOBJ_LOCKUPDATEPANEL))
      ShellUpdatePanels(CtrlObject->Cp()->ActivePanel,FALSE);
    /*else
    {
      CtrlObject->Cp()->LeftPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
      CtrlObject->Cp()->RightPanel->UpdateIfChanged(UIC_UPDATE_FORCE);
      CtrlObject->Cp()->Redraw();
    }*/
  }

  ScrBuf.Flush();
  return(Code);
}


/* $ 14.01.2001 SVS
   + � ProcessOSCommands ��������� ���������
     "IF [NOT] EXIST filename command"
     "IF [NOT] DEFINED variable command"

   ��� ������� ������������� ��� ��������� ���������� IF`�
   CmdLine - ������ ������ ����
     if exist file if exist file2 command
   Return - ��������� �� "command"
            ������ ������ - ������� �� ���������
            NULL - �� ������� "IF" ��� ������ � �����������, ��������
                   �� exist, � exist ��� ����������� �������.

   DEFINED - ������� EXIST, �� ��������� � ����������� �����

   �������� ������ (CmdLine) �� ��������������!!! - �� ��� ���� ��������� const
                                                    IS 20.03.2002 :-)
*/
const wchar_t* WINAPI PrepareOSIfExist(const wchar_t *CmdLine)
{
  if(!CmdLine || !*CmdLine)
    return NULL;

  string strCmd;
  string strExpandedStr;
  const wchar_t *PtrCmd=CmdLine, *CmdStart;
  int Not=FALSE;
  int Exist=0; // ������� ������� ����������� "IF [NOT] EXIST filename command"
               // > 0 - ���� ����� �����������

  /* $ 25.04.2001 DJ
     ��������� @ � IF EXIST
  */
  if (*PtrCmd == L'@')
  {
    // ����� @ ������������; �� ������� � ���������� ����� �������
    // ExtractIfExistCommand � filetype.cpp
    PtrCmd++;
    while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
  }

  while(1)
  {
    if (!PtrCmd || !*PtrCmd || StrCmpNI(PtrCmd,L"IF ",3))
      break;

    PtrCmd+=3; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;

    if (StrCmpNI(PtrCmd,L"NOT ",4)==0)
    {
      Not=TRUE;
      PtrCmd+=4; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
    }

    if (*PtrCmd && !StrCmpNI(PtrCmd,L"EXIST ",6))
    {
      PtrCmd+=6; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;

      /* $ 25.04.01 DJ
         ��������� ������� ������ ����� ����� � IF EXIST
      */
      BOOL InQuotes=FALSE;
      while (*PtrCmd)
      {
        if (*PtrCmd == L'\"')
          InQuotes = !InQuotes;
        else if (*PtrCmd == L' ' && !InQuotes)
          break;
        PtrCmd++;
      }

      if(PtrCmd && *PtrCmd && *PtrCmd == L' ')
      {
        {
          wchar_t *lpwszCmd = strCmd.GetBuffer(PtrCmd-CmdStart+1);
          wmemcpy(lpwszCmd,CmdStart,PtrCmd-CmdStart);
          strCmd.ReleaseBuffer(PtrCmd-CmdStart);
        }
        Unquote(strCmd);
//_SVS(SysLog(L"Cmd='%s'",(const wchar_t *)strCmd));
        if (apiExpandEnvironmentStrings(strCmd,strExpandedStr)!=0)
        {
          string strFullPath;
          if (!(strCmd.At(1) == L':' || (strCmd.At(0) == L'\\' && strCmd.At(1)==L'\\') || strExpandedStr.At(1) == L':' || (strExpandedStr.At(0) == L'\\' && strExpandedStr.At(1)==L'\\')))
          {
            if(CtrlObject)
              CtrlObject->CmdLine->GetCurDir(strFullPath);
            else
							apiGetCurrentDirectory(strFullPath);
            AddEndSlash(strFullPath);
          }
          strFullPath += strExpandedStr;
          DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
          if (wcspbrk((const wchar_t *)strExpandedStr+(PathPrefix(strExpandedStr)?4:0), L"*?")) // ��� �����?
          {
            FAR_FIND_DATA_EX wfd;

            if ( apiGetFindDataEx (strFullPath, &wfd) )
              FileAttr=wfd.dwFileAttributes;
          }
          else
          {
            ConvertNameToFull(strFullPath, strFullPath);
						FileAttr=apiGetFileAttributes(strFullPath);
          }
//_SVS(SysLog(L"%08X FullPath=%s",FileAttr,FullPath));
          if ((FileAttr != INVALID_FILE_ATTRIBUTES && !Not) || (FileAttr == INVALID_FILE_ATTRIBUTES && Not))
          {
            while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return L"";
        }
      }
    }

    // "IF [NOT] DEFINED variable command"
    else if (*PtrCmd && !StrCmpNI(PtrCmd,L"DEFINED ",8))
    {
      PtrCmd+=8; while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;
      if(*PtrCmd == L'"')
        PtrCmd=wcschr(PtrCmd+1,L'"');

      if(PtrCmd && *PtrCmd)
      {
        PtrCmd=wcschr(PtrCmd,L' ');
        if(PtrCmd && *PtrCmd && *PtrCmd == L' ')
        {
          {
            wchar_t *lpwszCmd = strCmd.GetBuffer(PtrCmd-CmdStart+1);
            wmemcpy(lpwszCmd,CmdStart,PtrCmd-CmdStart);
            strCmd.ReleaseBuffer(PtrCmd-CmdStart);
          }
          DWORD ERet=apiGetEnvironmentVariable(strCmd,strExpandedStr);
//_SVS(SysLog(Cmd));
          if ((ERet && !Not) || (!ERet && Not))
          {
            while(*PtrCmd && IsSpace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return L"";
        }
      }
    }
  }
  return Exist?PtrCmd:NULL;
}

int CommandLine::ProcessOSCommands(const wchar_t *CmdLine,int SeparateWindow)
{
  Panel *SetPanel;
  int Length;

  string strCmdLine = CmdLine;

  SetPanel=CtrlObject->Cp()->ActivePanel;

  if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
    SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

  RemoveTrailingSpaces(strCmdLine);

  bool SilentInt=false;
  if(*CmdLine == L'@')
  {
    SilentInt=true;
    strCmdLine.LShift(1);
  }

	if (!SeparateWindow && strCmdLine.At(0) && strCmdLine.At(1)==L':' && strCmdLine.At(2)==0)
  {
    FarChDir(strCmdLine);
		wchar_t NewDir[]={Upper(strCmdLine.At(0)),L':',L'\\',0};
    if (getdisk()!=(int)(NewDir[0]-L'A'))
    {
      FarChDir(NewDir);
    }
    SetPanel->ChangeDirToCurrent();
    return(TRUE);
  }

  // SET [����������=[������]]
	else if (!StrCmpNI(strCmdLine,L"SET",3) && IsSpace(strCmdLine.At(3)))
  {
    size_t pos;
    string strCmd = (const wchar_t *)strCmdLine+4;

    if (!strCmd.Pos(pos,L'='))
      return FALSE;

    if (strCmd.GetLength() == pos+1) //set var=
    {
    	strCmd.SetLength(pos);
      SetEnvironmentVariableW(strCmd,NULL);
    }
    else
    {
      string strExpandedStr;

      if (apiExpandEnvironmentStrings((const wchar_t *)strCmd+pos+1,strExpandedStr) != 0)
      {
      	strCmd.SetLength(pos);
        SetEnvironmentVariableW(strCmd,strExpandedStr);
      }
    }

    return TRUE;
  }

	else if ((!StrCmpNI(strCmdLine,L"REM",3) && IsSpace(strCmdLine.At(3))) || !StrCmpNI(strCmdLine,L"::",2))
  {
    return TRUE;
  }

	else if (!StrCmpI(strCmdLine,L"CLS"))
  {
    ClearScreen(COL_COMMANDLINEUSERSCREEN);
    return TRUE;
  }

	// PUSHD ���� | ..
	else if (!StrCmpNI(strCmdLine,L"PUSHD",5) && IsSpace(strCmdLine.At(5)))
	{
		string strCmd = (const wchar_t *)CmdLine+6;
		RemoveExternalSpaces(strCmd);

		if(!StrCmpI(strCmd,L"/?") || !StrCmpI(strCmd,L"-?"))
			return FALSE; // ����� cmd ������ ��� ���������

		PushPopRecord prec;
		prec.strName = strCurDir;
		if(IntChDir(strCmd,true,SilentInt))
		{
			ppstack.Push(prec);
			SetEnvironmentVariableW(L"FARDIRSTACK",prec.strName);
		}
		else
		{
			;
		}
		return TRUE;
	}

	// POPD
	else if (!StrCmpI(CmdLine,L"POPD"))
	{
		PushPopRecord prec;
		if(ppstack.Pop(prec))
		{
			int Ret=IntChDir(prec.strName,true,SilentInt);
			PushPopRecord *ptrprec=ppstack.Peek();
			SetEnvironmentVariableW(L"FARDIRSTACK",(ptrprec?ptrprec->strName.CPtr():NULL));
			return Ret;
		}
		return TRUE;
	}

	// CLRD
	else if (!StrCmpI(CmdLine,L"CLRD"))
	{
		ppstack.Free();
		SetEnvironmentVariableW(L"FARDIRSTACK",NULL);
		return TRUE;
	}

  /*
  Displays or sets the active code page number.
  CHCP [nnn]
    nnn   Specifies a code page number (Dec or Hex).
  Type CHCP without a parameter to display the active code page number.
  */
	else if (!StrCmpNI(strCmdLine,L"CHCP",4) && IsSpace(strCmdLine.At(4)))
  {
    strCmdLine = (const wchar_t*)strCmdLine+5;

    const wchar_t *Ptr=RemoveExternalSpaces(strCmdLine);
    wchar_t Chr;

    if(!iswdigit(*Ptr))
      return FALSE;

    while((Chr=*Ptr) != 0)
    {
      if(!iswdigit(Chr))
        break;
      ++Ptr;
    }

    wchar_t *Ptr2;

    UINT cp=(UINT)wcstol((const wchar_t*)strCmdLine+5,&Ptr2,10); //BUGBUG
    BOOL r1=SetConsoleCP(cp);
    BOOL r2=SetConsoleOutputCP(cp);
    if(r1 && r2) // ���� ��� ���, �� ���  �...
    {
      InitRecodeOutTable(cp);
      LocalUpperInit();
      InitLCIDSort();
      InitKeysArray();
      CtrlObject->Cp()->Redraw();
      ScrBuf.Flush();
      return TRUE;
    }
    else  // ��� ������ ������� chcp ���� ������ ;-)
     return FALSE;
  }

	else if (!StrCmpNI(strCmdLine,L"IF",2) && IsSpace(strCmdLine.At(2)))
	{
		const wchar_t *PtrCmd=PrepareOSIfExist(strCmdLine);
		// ����� PtrCmd - ��� ������� �������, ��� IF

		if(PtrCmd && *PtrCmd && CtrlObject->Plugins.ProcessCommandLine(PtrCmd))
		{
			//CmdStr.SetString(L"");
			GotoXY(X1,Y1);
			mprintf(L"%*s",X2-X1+1,L"");
			Show();
			return TRUE;
		}
		return FALSE;
	}

  /* $ 16.04.2002 DJ
     ���������� ���������, ���� ����� Shift-Enter
  */
	else if (!SeparateWindow &&
			(StrCmpNI(strCmdLine,L"CD",Length=2)==0 || StrCmpNI(strCmdLine,L"CHDIR",Length=5)==0) &&
			(IsSpace(strCmdLine.At(Length)) || IsSlash(strCmdLine.At(Length)) ||
			TestParentFolderName((const wchar_t*)strCmdLine+Length)))
	{
		int ChDir=(Length==5);

		while (IsSpace(strCmdLine.At(Length)))
			Length++;

		IntChDir((const wchar_t *)strCmdLine+Length,ChDir,SilentInt);

		return(TRUE);
	}


	else if(!StrCmpI(strCmdLine,L"EXIT"))
	{
		FrameManager->ExitMainLoop(FALSE);
		return TRUE;
	}

  return(FALSE);
}

BOOL CommandLine::IntChDir(const wchar_t *CmdLine,int ClosePlugin,bool Selent)
{
	Panel *SetPanel;

	SetPanel=CtrlObject->Cp()->ActivePanel;

	if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	string strExpandedDir;
	strExpandedDir = (const wchar_t*)CmdLine;

	Unquote(strExpandedDir);
	apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);


    // ������������� ����� ����� �� "���������"
//    if(ExpandedDir[1] == L':' && iswalpha(ExpandedDir[0])) //BUGBUG
//      ExpandedDir[0]=towupper(ExpandedDir[0]);

	if (SetPanel->GetMode()!=PLUGIN_PANEL && strExpandedDir.At(0) == L'~' && (!strExpandedDir.At(1) || IsSlash(strExpandedDir.At(1))) && apiGetFileAttributes(strExpandedDir) == INVALID_FILE_ATTRIBUTES)
	{
		string strTemp;
		GetRegKey(strSystemExecutor,L"~",strTemp,g_strFarPath);
		if(strExpandedDir.At(1))
		{
			AddEndSlash(strTemp);
			strTemp += (const wchar_t*)strExpandedDir+2;
		}
		DeleteEndSlash(strTemp);
		strExpandedDir=strTemp;
	}

	if (wcspbrk(&strExpandedDir[PathPrefix(strExpandedDir)?4:0],L"?*")) // ��� �����?
	{
		FAR_FIND_DATA_EX wfd;
		if (apiGetFindDataEx(strExpandedDir, &wfd))
		{
			size_t pos;
			if(LastSlash(strExpandedDir,pos))
				strExpandedDir.SetLength(pos+1);
			else
				strExpandedDir.SetLength(0);

			strExpandedDir += wfd.strFileName;
		}
	}
	/* $ 15.11.2001 OT
		������� ��������� ���� �� ����� "�������" ����������.
		���� �� ���, �� ����� �������� ������, ��� ��� ���������� ���������
	*/
	DWORD DirAtt=apiGetFileAttributes(strExpandedDir);
	if (DirAtt!=INVALID_FILE_ATTRIBUTES && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && PathMayBeAbsolute(strExpandedDir))
	{
		ReplaceStrings(strExpandedDir,L"/",L"\\",-1);
		SetPanel->SetCurDir(strExpandedDir,TRUE);
		return TRUE;
	}

    /* $ 20.09.2002 SKV
      ��� ��������� ����������� ��������� ����� ������� ���:
      cd net:server � cd ftp://server/dir
      ��� ��� ��� �� �� ������� �������� �
      cd s&r:, cd make: � �.�., ������� � �����
      �������� �� ����� �������� ���������.
    */
    /*
    if (CtrlObject->Plugins.ProcessCommandLine(ExpandedDir))
    {
      //CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return(TRUE);
    }
    */

	strExpandedDir.ReleaseBuffer ();

	if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
	{
		SetPanel->SetCurDir(strExpandedDir,ClosePlugin);
		return(TRUE);
	}

	if(FarChDir(strExpandedDir))
	{
		SetPanel->ChangeDirToCurrent();
		if(!SetPanel->IsVisible())
			SetPanel->SetTitle();
	}
	else
	{
		if(!Selent)
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strExpandedDir,MSG(MOk));
		return FALSE;
	}
	return TRUE;
}

// ��������� "��� ������?"
bool IsBatchExtType(const wchar_t *ExtPtr)
{
	bool Result=false;
	UserDefinedList BatchExtList;
	BatchExtList.Set(Opt.strExecuteBatchType);
	while(!Result&&!BatchExtList.IsEmpty())
	{
		if(!StrCmpI(ExtPtr,BatchExtList.GetNext()))
		{
			Result=true;
		}
	}
	return Result;
}
