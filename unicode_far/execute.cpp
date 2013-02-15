/*
execute.cpp

"�����������" ��������.
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

#include "execute.hpp"
#include "keyboard.hpp"
#include "filepanels.hpp"
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
#include "manager.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "syslog.hpp"
#include "constitle.hpp"
#include "console.hpp"
#include "constitle.hpp"
#include "configdb.hpp"
#include "mix.hpp"

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

static bool GetImageSubsystem(const string& FileName,DWORD& ImageSubsystem)
{
	bool Result=false;
	ImageSubsystem=IMAGE_SUBSYSTEM_UNKNOWN;
	File ModuleFile;
	if(ModuleFile.Open(FileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING))
	{
		IMAGE_DOS_HEADER DOSHeader;
		DWORD ReadSize;

		if (ModuleFile.Read(&DOSHeader, sizeof(DOSHeader), ReadSize) && ReadSize==sizeof(DOSHeader))
		{
			if (DOSHeader.e_magic==IMAGE_DOS_SIGNATURE)
			{
				//ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
				Result=true;

				if (ModuleFile.SetPointer(DOSHeader.e_lfanew,nullptr,FILE_BEGIN))
				{
					IMAGE_HEADERS PEHeader;

					if (ModuleFile.Read(&PEHeader, sizeof(PEHeader), ReadSize) && ReadSize==sizeof(PEHeader))
					{
						if (PEHeader.Signature==IMAGE_NT_SIGNATURE)
						{
							switch (PEHeader.OptionalHeader32.Magic)
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
						else if ((WORD)PEHeader.Signature==IMAGE_OS2_SIGNATURE)
						{
							/*
							NE,  ���...  � ��� ���������� ��� ��� ������?

							Andrzej Novosiolov <andrzej@se.kiev.ua>
							AN> ��������������� �� ����� "Target operating system" NE-���������
							AN> (1 ���� �� �������� 0x36). ���� ��� Windows (�������� 2, 4) - �������������
							AN> GUI, ���� OS/2 � ������ �������� (��������� ��������) - ������������� �������.
							*/
							PIMAGE_OS2_HEADER OS2Hdr = reinterpret_cast<PIMAGE_OS2_HEADER>(&PEHeader);
							if (OS2Hdr->ne_exetyp==2 || OS2Hdr->ne_exetyp==4)
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
		ModuleFile.Close();
	}

	/*else
	{
		// ������ ��������
	}*/
	return Result;
}

static bool IsProperProgID(const wchar_t* ProgID)
{
	if (ProgID && *ProgID)
	{
		HKEY hProgID;

		if (RegOpenKey(HKEY_CLASSES_ROOT, ProgID, &hProgID) == ERROR_SUCCESS)
		{
			RegCloseKey(hProgID);
			return true;
		}
	}

	return false;
}

/*
���� �������� ProgID ��� ��������� ���������� �� ������ ���������
hExtKey - �������� ���� ��� ������ (���� ����������)
strType - ���� ��������� ���������, ���� ����� ������
*/
static bool SearchExtHandlerFromList(HKEY hExtKey, string &strType)
{
	const DWORD dwNameBufSize = 64;
	HKEY hExtIDListSubKey;

	if (RegOpenKey(hExtKey, L"OpenWithProgids", &hExtIDListSubKey) == ERROR_SUCCESS)
	{
		DWORD nValueIndex = 0;
		LONG nRet;
		wchar_t wszValueName[dwNameBufSize];
		DWORD nValueNameSize = dwNameBufSize;
		DWORD nValueType;

		// ��������� �� ���� ��������� � �������� ����� �� �������������� � �������� �����
		while ((nRet = RegEnumValue(hExtIDListSubKey, nValueIndex, wszValueName, &nValueNameSize, nullptr, &nValueType, nullptr, nullptr)) != ERROR_NO_MORE_ITEMS)
		{
			if (nRet != ERROR_SUCCESS) break;

			if ((nValueType == REG_SZ || nValueType == REG_NONE) && IsProperProgID(wszValueName))
			{
				strType = wszValueName;
				RegCloseKey(hExtIDListSubKey);
				return true;
			}

			nValueIndex++;
			nValueNameSize = dwNameBufSize;	// ������� �������� �� ������� ������� ������ (����� ������� ��� ����� ������ ������)
		}

		RegCloseKey(hExtIDListSubKey);
	}

	return false;
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
static bool FindModule(const wchar_t *Module, string &strDest,DWORD &ImageSubsystem,bool &Internal)
{
	bool Result=false;
	ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;
	Internal = false;

	if (Module && *Module)
	{
		// ������� ������ - ������� ����������
		// ����� "����������" �� �������, ������� ������ ����������� ��������,
		// ��������, ��������� ���������� ������� ���. ����������.
		UserDefinedList ExcludeCmdsList(ULF_UNIQUE);
		ExcludeCmdsList.Set(Global->Opt->Exec.strExcludeCmds);

		while (!ExcludeCmdsList.IsEmpty())
		{
			if (!StrCmpI(Module,ExcludeCmdsList.GetNext()))
			{
				ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_CUI;
				Result=true;
				Internal = true;
				break;
			}
		}

		if (!Result)
		{
			string strFullName=Module;
			LPCWSTR ModuleExt=wcsrchr(PointToName(Module),L'.');
			string strPathExt(L".COM;.EXE;.BAT;.CMD;.VBS;.JS;.WSH");
			apiGetEnvironmentVariable(L"PATHEXT",strPathExt);
			UserDefinedList PathExtList(ULF_UNIQUE);
			PathExtList.Set(strPathExt);
			PathExtList.Reset();

			while (!PathExtList.IsEmpty()) // ������ ������ - � ������� ��������
			{
				LPCWSTR Ext=PathExtList.GetNext();
				string strTmpName=strFullName;

				if (!ModuleExt)
				{
					strTmpName+=Ext;
				}

				DWORD Attr=apiGetFileAttributes(strTmpName);

				if ((Attr!=INVALID_FILE_ATTRIBUTES) && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
				{
					ConvertNameToFull(strTmpName,strFullName);
					Result=true;
					break;
				}

				if (ModuleExt)
				{
					break;
				}
			}

			if (!Result) // ������ ������ - �� �������� SearchPath
			{
				string strPathEnv;

				if (apiGetEnvironmentVariable(L"PATH", strPathEnv))
				{
					UserDefinedList PathList(ULF_UNIQUE);
					PathList.Set(strPathEnv);

					while (!PathList.IsEmpty() && !Result)
					{
						LPCWSTR Path=PathList.GetNext();
						PathExtList.Reset();

						while (!PathExtList.IsEmpty())
						{
							string strDest;
							LPCWSTR Ext=PathExtList.GetNext();

							if (apiSearchPath(Path,strFullName,Ext,strDest))
							{
								DWORD Attr=apiGetFileAttributes(strDest);

								if ((Attr!=INVALID_FILE_ATTRIBUTES) && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
								{
									strFullName=strDest;
									Result=true;
									break;
								}
							}
						}
					}
				}

				if (!Result)
				{
					PathExtList.Reset();

					while (!PathExtList.IsEmpty())
					{
						string strDest;
						LPCWSTR Ext=PathExtList.GetNext();

						if (apiSearchPath(nullptr,strFullName,Ext,strDest))
						{
							DWORD Attr=apiGetFileAttributes(strDest);

							if ((Attr!=INVALID_FILE_ATTRIBUTES) && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
							{
								strFullName=strDest;
								Result=true;
								break;
							}
						}
					}
				}

				// ������ ������ - ����� � ������ � "App Paths"
				if (!Result && Global->Opt->Exec.ExecuteUseAppPath && !strFullName.Contains(L'\\'))
				{
					static const WCHAR RegPath[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
					// � ������ Module �������� ����������� ������ �� ������ ����, �������
					// ������� �� SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
					// ������� ������� � HKCU, ����� - � HKLM
					HKEY RootFindKey[]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE,HKEY_LOCAL_MACHINE};
					strFullName=RegPath;
					strFullName+=Module;

					DWORD samDesired = KEY_QUERY_VALUE;
					DWORD RedirectionFlag = 0;
					// App Paths key is shared in Windows 7 and above
					if (Global->WinVer() < _WIN32_WINNT_WIN7)
					{
#ifdef _WIN64
						RedirectionFlag = KEY_WOW64_32KEY;
#else
						BOOL Wow64Process = FALSE;
						if (Global->ifn->IsWow64Process(GetCurrentProcess(), &Wow64Process) && Wow64Process)
						{
							RedirectionFlag = KEY_WOW64_64KEY;
						}
#endif
					}
					for (size_t i=0; i<ARRAYSIZE(RootFindKey); i++)
					{
						if (i==ARRAYSIZE(RootFindKey)-1)
						{
							if(RedirectionFlag)
							{
								samDesired|=RedirectionFlag;
							}
							else
							{
								break;
							}
						}
						HKEY hKey;
						if (RegOpenKeyEx(RootFindKey[i],strFullName, 0, samDesired, &hKey)==ERROR_SUCCESS)
						{
							int RegResult=RegQueryStringValue(hKey, L"", strFullName, L"");
							RegCloseKey(hKey);

							if (RegResult==ERROR_SUCCESS)
							{
								apiExpandEnvironmentStrings(strFullName,strFullName);
								Unquote(strFullName);
								Result=true;
								break;
							}
						}
					}

					if (!Result)
					{
						PathExtList.Reset();

						while (!PathExtList.IsEmpty() && !Result)
						{
							LPCWSTR Ext=PathExtList.GetNext();
							strFullName=RegPath;
							strFullName+=Module;
							strFullName+=Ext;

							for (size_t i=0; i<ARRAYSIZE(RootFindKey); i++)
							{
								HKEY hKey;

								if (RegOpenKeyEx(RootFindKey[i],strFullName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
								{
									int RegResult=RegQueryStringValue(hKey, L"", strFullName, L"");
									RegCloseKey(hKey);

									if (RegResult==ERROR_SUCCESS)
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

			if (Result) // ��������� "�������" ������
			{
				GetImageSubsystem(strFullName,ImageSubsystem);
				strDest=strFullName;
			}
		}
	}

	return Result;
}

/*
 ���������� 2*PipeFound + 1*Escaped
*/
static int PartCmdLine(const string& CmdStr, string &strNewCmdStr, string &strNewCmdPar)
{
	int PipeFound = 0, Escaped = 0;
	bool quoted = false;
	apiExpandEnvironmentStrings(CmdStr, strNewCmdStr);
	RemoveExternalSpaces(strNewCmdStr);
	wchar_t *NewCmdStr = strNewCmdStr.GetBuffer();
	wchar_t *CmdPtr = NewCmdStr;
	wchar_t *ParPtr = nullptr;
	// �������� ���������� ������� ��� ���������� � ���������.
	// ��� ���� ������ ��������� ������� �������� ��������������� �������
	// �������� � ������ �������. �.�. ���� � �������� - �� ����.

	static const wchar_t ending_chars[] = L"/ <>|&";

	while (*CmdPtr)
	{
		if (*CmdPtr == L'"')
			quoted = !quoted;

		if (!quoted && *CmdPtr == L'^' && CmdPtr[1] > L' ') // "^>" � ��� � ���
		{
			Escaped = 1; //
			CmdPtr++;    // ??? ����� ���� '^' ���� �������...
		}
		else if (!quoted && CmdPtr != NewCmdStr)
		{
			const wchar_t *ending = wcschr(ending_chars, *CmdPtr);
			if ( ending )
			{
				if (!ParPtr)
					ParPtr = CmdPtr;

				if (ending >= ending_chars+2)
					PipeFound = 1;
			}
		}

		if (ParPtr && PipeFound) // ��� ������ ������ �� ���� ��������
			break;

		CmdPtr++;
	}

	if (ParPtr) // �� ����� ��������� � �������� ��� �� ������
	{
		if (*ParPtr == L' ') //AY: ������ ������ ����� �������� � ����������� �� �����,
			*(ParPtr++)=0;    //    �� ����������� ������ � Execute.

		strNewCmdPar = ParPtr;
		*ParPtr = 0;
	}

	strNewCmdStr.ReleaseBuffer();
	Unquote(strNewCmdStr);

	return 2*PipeFound + 1*Escaped;
}

static bool RunAsSupported(LPCWSTR Name)
{
	bool Result = false;
	string Extension(PointToExt(Name));
	if(Extension)
	{
		string strType;
		if(GetShellType(Extension, strType))
		{
			HKEY hKey;

			if (RegOpenKey(HKEY_CLASSES_ROOT,strType+L"\\shell\\runas\\command",&hKey)==ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
				Result = true;
			}
		}
	}
	return Result;
}

/*
�� ����� ����� (�� ��� ����������) �������� ������� ���������
������������� ��������� �������� �������-����������
(����� �� ����� ����������)
*/
static const wchar_t *GetShellAction(const string& FileName,DWORD& ImageSubsystem,DWORD& Error)
{
	string strValue;
	string strNewValue;
	const wchar_t *ExtPtr;
	const wchar_t *RetPtr;
	const wchar_t command_action[]=L"\\command";
	Error = ERROR_SUCCESS;
	ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

	if (!(ExtPtr=wcsrchr(FileName,L'.')))
		return nullptr;

	if (!GetShellType(ExtPtr, strValue))
		return nullptr;

	HKEY hKey;

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT,strValue,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
	{
		int nResult=RegQueryValueEx(hKey,L"IsShortcut",nullptr,nullptr,nullptr,nullptr);
		RegCloseKey(hKey);

		if (nResult==ERROR_SUCCESS)
			return nullptr;
	}

	strValue += L"\\shell";

	if (RegOpenKey(HKEY_CLASSES_ROOT,strValue,&hKey)!=ERROR_SUCCESS)
		return nullptr;

	static string strAction;
	int RetQuery = RegQueryStringValue(hKey, L"", strAction, L"");
	strValue += L"\\";

	if (RetQuery == ERROR_SUCCESS)
	{
		UserDefinedList ActionList(ULF_UNIQUE);
		RetPtr = (strAction.IsEmpty() ? nullptr : strAction.CPtr());
		const wchar_t *ActionPtr;
		LONG RetEnum = ERROR_SUCCESS;

		if (RetPtr  && ActionList.Set(strAction))
		{
			HKEY hOpenKey;
			ActionList.Reset();

			while (RetEnum == ERROR_SUCCESS && (ActionPtr = ActionList.GetNext()) )
			{
				strNewValue = strValue;
				strNewValue += ActionPtr;
				strNewValue += command_action;

				if (RegOpenKey(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
				{
					RegCloseKey(hOpenKey);
					strValue += ActionPtr;
					strAction = ActionPtr;
					RetPtr = strAction;
					RetEnum = ERROR_NO_MORE_ITEMS;
				} /* if */
			} /* while */
		} /* if */
		else
		{
			strValue += strAction;
		}

		if (RetEnum != ERROR_NO_MORE_ITEMS) // ���� ������ �� �����, ��...
			RetPtr=nullptr;
	}
	else
	{
		// This member defaults to "Open" if no verb is specified.
		// �.�. ���� �� ������� nullptr, �� ��������������� ������� "Open"
		RetPtr=nullptr;
	}

	// ���� RetPtr==nullptr - �� �� ����� default action.
	// ��������� - ���� �� ������ ���-������ � ����� ����������
	if (!RetPtr)
	{
		LONG RetEnum = ERROR_SUCCESS;
		DWORD dwIndex = 0;
		HKEY hOpenKey;
		// ������� �������� "open"...
		strAction = L"open";
		strNewValue = strValue;
		strNewValue += strAction;
		strNewValue += command_action;

		if (RegOpenKey(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
		{
			RegCloseKey(hOpenKey);
			strValue += strAction;
			RetPtr = strAction;
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

				if (RegOpenKey(HKEY_CLASSES_ROOT,strNewValue,&hOpenKey)==ERROR_SUCCESS)
				{
					RegCloseKey(hOpenKey);
					strValue += strAction;
					RetPtr = strAction;
					RetEnum = ERROR_NO_MORE_ITEMS;
				} /* if */
			} /* if */
		} /* while */
	} /* if */

	RegCloseKey(hKey);

	if (RetPtr )
	{
		strValue += command_action;

		// � ������ �������� �������� ����������� �����
		if (RegOpenKey(HKEY_CLASSES_ROOT,strValue,&hKey)==ERROR_SUCCESS)
		{
			RetQuery=RegQueryStringValue(hKey, L"", strNewValue, L"");
			RegCloseKey(hKey);

			if (RetQuery == ERROR_SUCCESS && !strNewValue.IsEmpty())
			{
				apiExpandEnvironmentStrings(strNewValue,strNewValue);
				wchar_t *Ptr = strNewValue.GetBuffer();

				// �������� ��� ������
				if (*Ptr==L'\"')
				{
					wchar_t *QPtr = wcschr(Ptr + 1,L'\"');

					if (QPtr)
					{
						*QPtr=0;
						wmemmove(Ptr, Ptr + 1, QPtr-Ptr);
					}
				}
				else
				{
					if ((Ptr=wcspbrk(Ptr,L" \t/")))
						*Ptr=0;
				}

				strNewValue.ReleaseBuffer();
				GetImageSubsystem(strNewValue,ImageSubsystem);
			}
			else
			{
				Error=ERROR_NO_ASSOCIATION;
				RetPtr=nullptr;
			}
		}
	}

	return RetPtr;
}

bool GetShellType(const string& Ext, string &strType,ASSOCIATIONTYPE aType)
{
	bool bVistaType = false;
	strType.Clear();

	if (Global->WinVer() >= _WIN32_WINNT_VISTA)
	{
		IApplicationAssociationRegistration* pAAR;
		HRESULT hr = Global->ifn->SHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, (void**)&pAAR);

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
		if (aType == AT_URLPROTOCOL)
		{
			strType = Ext;
			return true;
		}

		HKEY hCRKey = 0, hUserKey = 0;
		string strFoundValue;

		if (aType == AT_FILEEXTENSION)
		{
			string strExplorerTypeKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\");
			strExplorerTypeKey.Append(Ext);

			// ������� ��������� ���������� ���������� � HKEY_CURRENT_USER
			if (RegOpenKey(HKEY_CURRENT_USER, strExplorerTypeKey, &hUserKey) == ERROR_SUCCESS)
			{
				if ((RegQueryStringValue(hUserKey, L"ProgID", strFoundValue) == ERROR_SUCCESS) && IsProperProgID(strFoundValue))
				{
					strType = strFoundValue;
				}
			}
		}

		// ������� ��������� ���������� ���������� � HKEY_CLASSES_ROOT
		if (strType.IsEmpty() && (RegOpenKey(HKEY_CLASSES_ROOT, Ext, &hCRKey) == ERROR_SUCCESS))
		{
			if ((RegQueryStringValue(hCRKey, L"", strFoundValue) == ERROR_SUCCESS) && IsProperProgID(strFoundValue))
			{
				strType = strFoundValue;
			}
		}

		if (strType.IsEmpty() && hUserKey)
			SearchExtHandlerFromList(hUserKey, strType);

		if (strType.IsEmpty() && hCRKey)
			SearchExtHandlerFromList(hCRKey, strType);

		if (hUserKey)
			RegCloseKey(hUserKey);

		if (hCRKey)
			RegCloseKey(hCRKey);
	}

	return !strType.IsEmpty();
}

/*
�������-��������� ������� ���������
���������� -1 � ������ ������ ���...
*/
int Execute(const string& CmdStr,  // ���.������ ��� ����������
            bool AlwaysWaitFinish, // ����� ���������� ��������?
            bool SeparateWindow,   // ��������� � ��������� ����?
            bool DirectRun,        // ��������� ��������? (��� CMD)
            bool FolderRun,        // ��� ������?
            bool WaitForIdle,      // for list files
            bool Silent,
            bool RunAs             // elevation
            )
{
	int nResult = -1;
	string strNewCmdStr;
	string strNewCmdPar;

	int PipeOrEscaped = PartCmdLine(CmdStr, strNewCmdStr, strNewCmdPar);

	DWORD dwAttr = apiGetFileAttributes(strNewCmdStr);

	if(RunAs)
	{
		SeparateWindow = true;
	}

	if(FolderRun)
	{
		Silent = true;
	}

	if (SeparateWindow)
	{
		if(Global->Opt->Exec.ExecuteSilentExternal)
		{
			Silent = true;
		}
		if (strNewCmdPar.IsEmpty() && dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			ConvertNameToFull(strNewCmdStr, strNewCmdStr);
			DirectRun = true;
			FolderRun=true;
		}
	}

	string strComspec;
	apiGetEnvironmentVariable(L"COMSPEC", strComspec);
	if (strComspec.IsEmpty() && !DirectRun)
	{
		Message(MSG_WARNING, 1, MSG(MError), MSG(MComspecNotFound), MSG(MOk));
		return -1;
	}

	DWORD dwSubSystem = IMAGE_SUBSYSTEM_UNKNOWN;
	HANDLE hProcess = nullptr;
	LPCWSTR lpVerb = nullptr;

	if (FolderRun && DirectRun)
	{
		AddEndSlash(strNewCmdStr); // ����, ����� ShellExecuteEx "�������" BAT/CMD/��.�����, �� �� �������
	}
	else
	{
		bool internal;
		FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem,internal);

		if (/*!*NewCmdPar && */ dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
		{

			const wchar_t *ExtPtr=wcsrchr(PointToName(strNewCmdStr), L'.');
			if (ExtPtr)
			{
				DWORD Error=0, dwSubSystem2=0;
				if (!(!StrCmpI(ExtPtr,L".exe") || !StrCmpI(ExtPtr,L".com") || IsBatchExtType(ExtPtr)))
				{
					lpVerb=GetShellAction(strNewCmdStr,dwSubSystem2,Error);

					if (lpVerb && Error != ERROR_NO_ASSOCIATION)
					{
						dwSubSystem=dwSubSystem2;
					}
				}

				if (dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN && !StrCmpNI(strNewCmdStr,L"ECHO.",5)) // ������� "echo."
				{
					strNewCmdStr.Replace(4,1,L' ');
					PartCmdLine(strNewCmdStr,strNewCmdStr,strNewCmdPar);

					if (strNewCmdPar.IsEmpty())
						strNewCmdStr+=L'.';

					FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem,internal);
				}
			}
		}

		if (dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
		{
			if ( !DirectRun )
			{
				DirectRun = (PipeOrEscaped < 1); //??? <= 1 ���� �� '^' ���� �������
			}
			if(DirectRun && Global->Opt->Exec.ExecuteSilentExternal)
			{
				Silent = true;
			}
			SeparateWindow = true;
		}
		else if (dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_CUI && !DirectRun && !internal)
		{
			DirectRun = (PipeOrEscaped < 1); //??? <= 1 ���� �� '^' ���� �������
		}
	}

	bool Visible=false;
	DWORD Size=0;
	SMALL_RECT ConsoleWindowRect;
	COORD ConsoleSize={};
	int ConsoleCP = CP_OEMCP;
	int ConsoleOutputCP = CP_OEMCP;
	int add_show_clock = 0;

	if(!Silent)
	{
		int X1, X2, Y1, Y2;
		Global->CtrlObject->CmdLine->GetPosition(X1, Y1, X2, Y2);
		Global->ProcessShowClock += (add_show_clock = 1);
		Global->CtrlObject->CmdLine->ShowBackground();
		Global->CtrlObject->CmdLine->Redraw();
		GotoXY(X2+1,Y1);
		Text(L" ");
		MoveCursor(X1,Y1);
		GetCursorType(Visible,Size);
		SetInitialCursorType();
	}

	Global->CtrlObject->CmdLine->SetString(L"", Silent);

	if(!Silent)
	{
		// BUGBUG: ���� ������� ���������� � "@", �� ��� ������ ����� ��� ���������
		// TODO: ����� ���������� ���������� ����������� �����, � ����� ��� ��������� ��������� � ScrBuf
		Global->ScrBuf->SetLockCount(0);
		Global->ScrBuf->Flush();
	}

	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	if(!Silent)
	{
		ConsoleCP = Global->Console->GetInputCodepage();
		ConsoleOutputCP = Global->Console->GetOutputCodepage();
		FlushInputBuffer();
		ChangeConsoleMode(InitialConsoleMode);
		Global->Console->GetWindowRect(ConsoleWindowRect);
		Global->Console->GetSize(ConsoleSize);
	}
	ConsoleTitle OldTitle;

	SHELLEXECUTEINFO seInfo={sizeof(seInfo)};
	string strCurDir;
	apiGetCurrentDirectory(strCurDir);
	seInfo.lpDirectory=strCurDir;
	seInfo.nShow = SW_SHOWNORMAL;

	string strFarTitle;
	if(!Silent)
	{
		if (Global->Opt->Exec.ExecuteFullTitle)
		{
			strFarTitle += strNewCmdStr;
			if (!strNewCmdPar.IsEmpty())
			{
				strFarTitle.Append(L" ").Append(strNewCmdPar);
			}
		}
		else
		{
			strFarTitle+=CmdStr;
		}
		ConsoleTitle::SetFarTitle(strFarTitle);
	}

	string ComSpecParams(L"/C ");
	if (DirectRun)
	{
		seInfo.lpFile = strNewCmdStr;
		if(!strNewCmdPar.IsEmpty())
		{
			seInfo.lpParameters = strNewCmdPar;
		}
		//Maximus: �������� dwSubSystem
		DWORD dwSubSystem2 = IMAGE_SUBSYSTEM_UNKNOWN;
		DWORD dwError = 0;
		seInfo.lpVerb = dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr&FILE_ATTRIBUTE_DIRECTORY)?nullptr:lpVerb?lpVerb:GetShellAction(strNewCmdStr, dwSubSystem2, dwError);
		if (dwSubSystem2!=IMAGE_SUBSYSTEM_UNKNOWN && dwSubSystem==IMAGE_SUBSYSTEM_UNKNOWN)
			dwSubSystem=dwSubSystem2;
	}
	else
	{
		QuoteSpace(strNewCmdStr);
		bool bDoubleQ = wcspbrk(strNewCmdStr, L"&<>()@^|=;, ") != nullptr;
		if (!strNewCmdPar.IsEmpty() || bDoubleQ)
		{
			ComSpecParams += L"\"";
		}
		ComSpecParams += strNewCmdStr;
		if (!strNewCmdPar.IsEmpty())
		{
			ComSpecParams.Append(L" ").Append(strNewCmdPar);
		}
		if (!strNewCmdPar.IsEmpty() || bDoubleQ)
		{
			ComSpecParams += L"\"";
		}

		seInfo.lpFile = strComspec;
		seInfo.lpParameters = ComSpecParams;
		seInfo.lpVerb = nullptr;
	}

	if(RunAs && RunAsSupported(seInfo.lpFile))
	{
		seInfo.lpVerb = L"runas";
	}

	seInfo.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS|(SeparateWindow?0:SEE_MASK_NO_CONSOLE);
	if (dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)  // ��� .exe �� �������� - ������ �������� � ��������
		if (Global->WinVer() >= _WIN32_WINNT_VISTA)         // ShexxExecuteEx error, see
			seInfo.fMask |= SEE_MASK_INVOKEIDLIST; // http://us.generation-nt.com/answer/shellexecuteex-does-not-allow-openas-verb-windows-7-help-31497352.html

	if(!Silent)
	{
		Global->Console->ScrollScreenBuffer(((DirectRun && dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || SeparateWindow)?2:1);
	}

	DWORD dwError = 0;
	if (ShellExecuteEx(&seInfo))
	{
		hProcess = seInfo.hProcess;
	}
	else
	{
		dwError = GetLastError();
	}

	if (!dwError)
	{
		if (hProcess)
		{
			if (!Silent)
			{
				Global->ScrBuf->Flush();
			}
			if (AlwaysWaitFinish || !SeparateWindow)
			{
				if (Global->Opt->ConsoleDetachKey.IsEmpty())
				{
					WaitForSingleObject(hProcess,INFINITE);
				}
				else
				{
					/*$ 12.02.2001 SKV
					  ����� ����� ;)
					  ��������� ��������� ������� �� ���������������� ��������.
					  ������� ������� � System/ConsoleDetachKey
					*/
					HANDLE hOutput = Global->Console->GetOutputHandle();
					HANDLE hInput = Global->Console->GetInputHandle();
					INPUT_RECORD ir[256];
					size_t rd;
					int vkey=0,ctrl=0;
					TranslateKeyToVK(KeyNameToKey(Global->Opt->ConsoleDetachKey),vkey,ctrl,nullptr);
					int alt=ctrl&(PKF_ALT|PKF_RALT);
					int shift=ctrl&PKF_SHIFT;
					ctrl=ctrl&(PKF_CONTROL|PKF_RCONTROL);
					bool bAlt, bShift, bCtrl;
					DWORD dwControlKeyState;

					//��� ������ ������ WaitForMultipleObjects �� �� ���� � Win7 ��� ������ � ������
					while (WaitForSingleObject(hProcess, 100) != WAIT_OBJECT_0)
					{
						if (WaitForSingleObject(hInput, 100)==WAIT_OBJECT_0 && Global->Console->PeekInput(ir, 256, rd) && rd)
						{
							int stop=0;

							for (DWORD i=0; i<rd; i++)
							{
								PINPUT_RECORD pir=&ir[i];

								if (pir->EventType==KEY_EVENT)
								{
									dwControlKeyState = pir->Event.KeyEvent.dwControlKeyState;
									bAlt = (dwControlKeyState & LEFT_ALT_PRESSED) || (dwControlKeyState & RIGHT_ALT_PRESSED);
									bCtrl = (dwControlKeyState & LEFT_CTRL_PRESSED) || (dwControlKeyState & RIGHT_CTRL_PRESSED);
									bShift = (dwControlKeyState & SHIFT_PRESSED)!=0;

									if (vkey==pir->Event.KeyEvent.wVirtualKeyCode &&
									        (alt ?bAlt:!bAlt) &&
									        (ctrl ?bCtrl:!bCtrl) &&
									        (shift ?bShift:!bShift))
									{
										Global->ConsoleIcons->restorePreviousIcons();

										Global->Console->ReadInput(ir, 256, rd);
										/*
										  �� ����� �������� CloseConsole, ������, ��� ��� ��������
										  ConsoleMode �� ���, ��� ��� �� ������� Far'�,
										  ���� ���������� ���������� ����� � �� �������.
										*/
										CloseHandle(hInput);
										CloseHandle(hOutput);
										delete KeyQueue;
										KeyQueue=nullptr;
										Global->Console->Free();
										Global->Console->Allocate();

										HWND hWnd = Global->Console->GetWindow();
										if (hWnd)   // ���� ���� ����� HOTKEY, �� ������ ������ ��� ������.
											SendMessage(hWnd,WM_SETHOTKEY,0,(LPARAM)0);

										Global->Console->SetSize(ConsoleSize);
										Global->Console->SetWindowRect(ConsoleWindowRect);
										Global->Console->SetSize(ConsoleSize);
										Sleep(100);
										InitConsole(0);

										Global->ConsoleIcons->setFarIcons();

										stop=1;
										break;
									}
								}
							}

							if (stop)
								break;
						}
					}
				}

				if(!Silent)
				{
					bool SkipScroll = false;
					COORD Size;
					if(Global->Console->GetSize(Size))
					{
						COORD BufferSize = {Size.X, static_cast<SHORT>(Global->Opt->ShowKeyBar?3:2)};
						FAR_CHAR_INFO* Buffer = new FAR_CHAR_INFO[BufferSize.X * BufferSize.Y];
						COORD BufferCoord = {};
						SMALL_RECT ReadRegion = {0, static_cast<SHORT>(Size.Y - BufferSize.Y), static_cast<SHORT>(Size.X-1), static_cast<SHORT>(Size.Y-1)};
						if(Global->Console->ReadOutput(Buffer, BufferSize, BufferCoord, ReadRegion))
						{
							FarColor Attributes = Buffer[BufferSize.X*BufferSize.Y-1].Attributes;
							SkipScroll = true;
							for(int i = 0; i < BufferSize.X*BufferSize.Y; i++)
							{
								if(Buffer[i].Char != L' ' || Buffer[i].Attributes.ForegroundColor != Attributes.ForegroundColor || Buffer[i].Attributes.BackgroundColor != Attributes.BackgroundColor || Buffer[i].Attributes.Flags != Attributes.Flags)
								{
									SkipScroll = false;
									break;
								}
							}
							delete[] Buffer;
						}
					}
					if(!SkipScroll)
					{
						Global->Console->ScrollScreenBuffer(Global->Opt->ShowKeyBar?2:1);
					}
				}

			}
			if(WaitForIdle)
			{
				WaitForInputIdle(hProcess, INFINITE);
			}
			CloseHandle(hProcess);
		}

		nResult = 0;

	}
	Global->ProcessShowClock -= add_show_clock;

	SetFarConsoleMode(TRUE);
	/* �������������� ��������� �������, �.�. SetCursorType ������ �� �������
	    ���������� ����� �����������, ������� � ������ ������ ������� �����.
	*/
	SetCursorType(Visible,Size);
	CONSOLE_CURSOR_INFO cci={Size, Visible};
	Global->Console->SetCursorInfo(cci);

	COORD ConSize;
	Global->Console->GetSize(ConSize);
	if(ConSize.X!=ScrX+1 || ConSize.Y!=ScrY+1)
	{
		ChangeVideoMode(ConSize.Y, ConSize.X);
	}
	else
	{
		if(!Silent)
		{
			Global->ScrBuf->FillBuf();
			Global->CtrlObject->CmdLine->SaveBackground();
		}
	}

	if (Global->Opt->Exec.RestoreCPAfterExecute)
	{
		// ����������� CP-������� ����� ���������� �����
		Global->Console->SetInputCodepage(ConsoleCP);
		Global->Console->SetOutputCodepage(ConsoleOutputCP);
	}

	Global->Console->SetTextAttributes(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));

	if(dwError)
	{
		if (!Silent)
		{
			Global->CtrlObject->Cp()->Redraw();
			if (Global->Opt->ShowKeyBar)
			{
				Global->CtrlObject->MainKeyBar->Show();
			}
			if (Global->Opt->Clock)
				ShowTime(1);
		}

		const wchar_t* Items[4];
		size_t ItemsSize;

		if(DirectRun)
		{
			Items[0] = MSG(MCannotExecute);
			Items[1] = strNewCmdStr;
			Items[2] = MSG(MOk);
			ItemsSize = 3;
		}
		else
		{
			Items[0] = MSG(MCannotInvokeComspec);
			Items[1] = strComspec;
			Items[2] = MSG(MCheckComspecVar);
			Items[3] = MSG(MOk);
			ItemsSize = 4;
		}
		Message(MSG_WARNING|MSG_ERRORTYPE|MSG_INSERT_STR2, 1, MSG(MError), Items, ItemsSize, L"ErrCannotExecute");
	}

	return nResult;
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
            nullptr - �� ������� "IF" ��� ������ � �����������, ��������
                   �� exist, � exist ��� ����������� �������.

   DEFINED - ������� EXIST, �� ��������� � ����������� �����

   �������� ������ (CmdLine) �� ��������������!!! - �� ��� ���� ��������� const
                                                    IS 20.03.2002 :-)
*/
const wchar_t *PrepareOSIfExist(const string& CmdLine)
{
	if (CmdLine.IsEmpty())
		return nullptr;

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

		while (*PtrCmd && IsSpace(*PtrCmd))
			++PtrCmd;
	}

	for (;;)
	{
		if (!PtrCmd || !*PtrCmd || StrCmpNI(PtrCmd,L"IF ",3)) //??? IF/I �� ��������������
			break;

		PtrCmd+=3;

		while (*PtrCmd && IsSpace(*PtrCmd))
			++PtrCmd;

		if (!*PtrCmd)
			break;

		if (!StrCmpNI(PtrCmd,L"NOT ",4))
		{
			Not=TRUE;

			PtrCmd+=4;

			while (*PtrCmd && IsSpace(*PtrCmd))
				++PtrCmd;

			if (!*PtrCmd)
				break;
		}

		if (*PtrCmd && !StrCmpNI(PtrCmd,L"EXIST ",6))
		{

			PtrCmd+=6;

			while (*PtrCmd && IsSpace(*PtrCmd))
				++PtrCmd;

			if (!*PtrCmd)
				break;

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

			if (PtrCmd && *PtrCmd && *PtrCmd == L' ')
			{
				strCmd.Copy(CmdStart,PtrCmd-CmdStart);
				Unquote(strCmd);

//_SVS(SysLog(L"Cmd='%s'", strCmd.CPtr()));
				if (apiExpandEnvironmentStrings(strCmd,strExpandedStr))
				{
					string strFullPath;

					if (!(strCmd.At(1) == L':' || (strCmd.At(0) == L'\\' && strCmd.At(1)==L'\\') || strExpandedStr.At(1) == L':' || (strExpandedStr.At(0) == L'\\' && strExpandedStr.At(1)==L'\\')))
					{
						if (Global->CtrlObject)
							Global->CtrlObject->CmdLine->GetCurDir(strFullPath);
						else
							apiGetCurrentDirectory(strFullPath);

						AddEndSlash(strFullPath);
					}

					strFullPath += strExpandedStr;
					DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

					const wchar_t* DirPtr = strExpandedStr;
					ParsePath(strExpandedStr, &DirPtr);
					if (wcspbrk(DirPtr, L"*?")) // ��� �����?
					{
						FAR_FIND_DATA wfd;

						if (apiGetFindDataEx(strFullPath, wfd))
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
						while (*PtrCmd && IsSpace(*PtrCmd))
							++PtrCmd;

						Exist++;
					}
					else
					{
						return L"";
					}
				}
			}
		}
		// "IF [NOT] DEFINED variable command"
		else if (*PtrCmd && !StrCmpNI(PtrCmd,L"DEFINED ",8))
		{

			PtrCmd+=8;

			while (*PtrCmd && IsSpace(*PtrCmd))
				++PtrCmd;

			if (!*PtrCmd)
				break;

			CmdStart=PtrCmd;

			if (*PtrCmd == L'"')
				PtrCmd=wcschr(PtrCmd+1,L'"');

			if (PtrCmd && *PtrCmd)
			{
				PtrCmd=wcschr(PtrCmd,L' ');

				if (PtrCmd && *PtrCmd && *PtrCmd == L' ')
				{
					strCmd.Copy(CmdStart,PtrCmd-CmdStart);

					DWORD ERet=apiGetEnvironmentVariable(strCmd,strExpandedStr);

//_SVS(SysLog(Cmd));
					if ((ERet && !Not) || (!ERet && Not))
					{
						while (*PtrCmd && IsSpace(*PtrCmd))
							++PtrCmd;

						Exist++;
					}
					else
					{
						return L"";
					}
				}
			}
		}
	}

	return Exist?PtrCmd:nullptr;
}

/*
��������� "��� ������?"
*/
bool IsBatchExtType(const string& ExtPtr)
{
	UserDefinedList BatchExtList(ULF_UNIQUE);
	BatchExtList.Set(Global->Opt->Exec.strExecuteBatchType);

	while (!BatchExtList.IsEmpty())
	{
		if (!StrCmpI(ExtPtr,BatchExtList.GetNext()))
			return true;
	}

	return false;
}

bool ProcessOSAliases(string &strStr)
{
	string strNewCmdStr;
	string strNewCmdPar;

	PartCmdLine(strStr,strNewCmdStr,strNewCmdPar);

	const wchar_t *lpwszExeName=PointToName(Global->g_strFarModuleName);
	int nSize=(int)strNewCmdStr.GetLength()+4096;
	wchar_t* lpwszNewCmdStr=strNewCmdStr.GetBuffer(nSize);
	int ret=Global->Console->GetAlias(lpwszNewCmdStr,lpwszNewCmdStr,nSize*sizeof(wchar_t),lpwszExeName);

	if (!ret)
	{
		string strComspec;
		if (apiGetEnvironmentVariable(L"COMSPEC",strComspec))
		{
			lpwszExeName=PointToName(strComspec);
			ret=Global->Console->GetAlias(lpwszNewCmdStr,lpwszNewCmdStr,nSize*sizeof(wchar_t),lpwszExeName);
		}
	}

	strNewCmdStr.ReleaseBuffer();

	if (!ret)
		return false;

	if (!ReplaceStrings(strNewCmdStr,L"$*",strNewCmdPar))
		strNewCmdStr+=L" "+strNewCmdPar;

	strStr=strNewCmdStr;

	return true;
}
