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

#include "farqueue.hpp"
#include "fn.hpp"
#include "filepanels.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "plugin.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "chgprior.hpp"
#include "global.hpp"
#include "cmdline.hpp"
#include "panel.hpp"
#include "rdrwdsk.hpp"
#include "udlist.hpp"

static const wchar_t strSystemExecutor[]=L"System\\Executor";

// ��������� ����� �� �������� GetFileInfo, �������� ����������� ���������� � ���� PE-������

// ���������� ��������� IMAGE_SUBSYSTEM_* ���� ������� ��������
// ��� ������ �� ��������� IMAGE_SUBSYTEM_UNKNOWN ��������
// "���� �� �������� �����������".
// ��� DOS-���������� ��������� ��� ���� �������� �����.
#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

static int IsCommandPEExeGUI(const wchar_t *FileName,DWORD& ImageSubsystem)
{
  //_SVS(CleverSysLog clvrSLog(L"IsCommandPEExeGUI()"));
  //_SVS(SysLog(L"Param: FileName='%s'",FileName));
//  char NameFile[NM];
  HANDLE hFile;
  int Ret=FALSE;
  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

  if((hFile=apiCreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL)) != INVALID_HANDLE_VALUE)
  {
    DWORD ReadSize;
    IMAGE_DOS_HEADER dos_head;

    BOOL RetReadFile=ReadFile(hFile,&dos_head,sizeof(IMAGE_DOS_HEADER),&ReadSize,NULL);

    if(RetReadFile && dos_head.e_magic == IMAGE_DOS_SIGNATURE)
    {
      Ret=TRUE;
      ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
      /*  ���� �������� ����� �� �������� 18h (OldEXE - MZ) >= 40h,
      �� �������� ����� � 3Ch �������� ��������� ��������� Windows. */
      /* 31.07.2003 VVM
        ! ������� ���� MSDN - ����� ������� �� ����� */
//      if (dos_head.e_lfarlc >= 0x40)
      {
        DWORD signature;
        #include <pshpack1.h>
        struct __HDR
        {
           DWORD signature;
           IMAGE_FILE_HEADER _head;
           union
           {
             IMAGE_OPTIONAL_HEADER32 opt_head32;
             IMAGE_OPTIONAL_HEADER64 opt_head64;
           };
           // IMAGE_SECTION_HEADER section_header[];  /* actual number in NumberOfSections */
        } header, *pheader;
        #include <poppack.h>

        if(SetFilePointer(hFile,dos_head.e_lfanew,NULL,FILE_BEGIN) != -1)
        {
          // ������ ��������� ���������
          if(ReadFile(hFile,&header,sizeof(struct __HDR),&ReadSize,NULL))
          {
            signature=header.signature;
            pheader=&header;

            if(signature == IMAGE_NT_SIGNATURE) // PE
            {
               if (header.opt_head32.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
                 ImageSubsystem = header.opt_head64.Subsystem;
               else
                 ImageSubsystem = header.opt_head32.Subsystem;
            }
//            {
//              IsPEGUI=1;
//              IsPEGUI|=(header.opt_head.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)?2:0;
//            }
            else if((WORD)signature == IMAGE_OS2_SIGNATURE) // NE
            {
              /*
                 NE,  ���...  � ��� ���������� ��� ��� ������?

                 Andrzej Novosiolov <andrzej@se.kiev.ua>
                 AN> ��������������� �� ����� "Target operating system" NE-���������
                 AN> (1 ���� �� �������� 0x36). ���� ��� Windows (�������� 2, 4) - �������������
                 AN> GUI, ���� OS/2 � ������ �������� (��������� ��������) - ������������� �������.
              */
              BYTE ne_exetyp=((IMAGE_OS2_HEADER *)pheader)->ne_exetyp;
              if(ne_exetyp == 2 || ne_exetyp == 4)
                ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_GUI;
            }
          }
          else
          {
            ; // ������ ����� � ������� ���������� ��������� ;-(
          }
        }
        else
        {
          ; // ������ �������� ���� ���� � �����, �.�. dos_head.e_lfanew
            // ������ ������� � �������������� ����� (�������� ��� ������
            // ���� DOS-����
        }
      }
/*      else
      {
        ; // ��� ������� EXE, �� �� �������� EXE
      }
*/
    }
    else
    {
      if(!RetReadFile)
        ; // ������ ������
      else
        ; // ��� �� ����������� ���� - � ���� ���� ��������� MZ, ��������, NLM-������
        // TODO: ����� ����� ��������� POSIX �������, �������� "/usr/bin/sh"
    }
    CloseHandle(hFile);
  }

  return Ret;
}

// �� ����� ����� (�� ��� ����������) �������� ������� ���������
// ������������� ��������� �������� �������-����������
// (����� �� ����� ����������)
const wchar_t *GetShellAction(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& Error)
{
  //_SVS(CleverSysLog clvrSLog(L"GetShellAction()"));
  //_SVS(SysLog(L"Param: FileName='%s'",FileName));

  string strValue;
  string strNewValue;
  const wchar_t *ExtPtr;
  const wchar_t *RetPtr;
  const wchar_t command_action[]=L"\\command";

  Error = ERROR_SUCCESS;
  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

  if ((ExtPtr=wcsrchr(FileName,L'.'))==NULL)
    return(NULL);

  if (RegQueryStringValue(HKEY_CLASSES_ROOT,ExtPtr,strValue,L"")!=ERROR_SUCCESS)
    return(NULL);

  strValue += L"\\shell";
//_SVS(SysLog(L"[%d] Value='%s'",__LINE__,(const wchar_t *)strValue));

  HKEY hKey;
  if (RegOpenKeyW(HKEY_CLASSES_ROOT,(const wchar_t *)strValue,&hKey)!=ERROR_SUCCESS)
    return(NULL);

  static string strAction;

  int RetQuery = RegQueryStringValueEx(hKey,L"",strAction,L"");

  strValue += L"\\";
//_SVS(SysLog(L"[%d] Action='%s' Value='%s'",__LINE__,(const wchar_t *)strAction,(const wchar_t *)strValue));

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

//_SVS(SysLog(L"[%d] Value='%s'",__LINE__,(const wchar_t *)strValue));
    if(RetEnum != ERROR_NO_MORE_ITEMS) // ���� ������ �� �����, ��...
      RetPtr=NULL;
  }
  else
  {
    // This member defaults to "Open" if no verb is specified.
    // �.�. ���� �� ������� NULL, �� ��������������� ������� "Open"
      RetPtr=NULL;
//    strValue += L"\\open";
  }

  // ���� RetPtr==NULL - �� �� ����� default action.
  // ��������� - ���� �� ������ ���-������ � ����� ����������
  if (RetPtr==NULL)
  {
    LONG RetEnum = ERROR_SUCCESS;
    DWORD dwIndex = 0;
    DWORD dwKeySize = 0;
    FILETIME ftLastWriteTime;
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
//_SVS(SysLog(L"[%d] Action='%s' Value='%s'",__LINE__,(const wchar_t *)strAction,(const wchar_t *)strValue));
    } /* if */

    // ... � ������ ��� ���������, ���� "open" ����
    while (RetEnum == ERROR_SUCCESS)
    {
      wchar_t *Action = 0;
      dwKeySize = 0;
      RegEnumKeyExW(hKey, dwIndex, Action, &dwKeySize, NULL, NULL, NULL, &ftLastWriteTime);
      Action = strAction.GetBuffer((int)++dwKeySize);
      *Action = 0;
      RetEnum = RegEnumKeyExW(hKey, dwIndex++, Action, &dwKeySize, NULL, NULL, NULL, &ftLastWriteTime);
      strAction.ReleaseBuffer();
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
//_SVS(SysLog(L"[%d] Action='%s' Value='%s'",__LINE__,(const wchar_t *)strAction,(const wchar_t *)strValue));
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
          if ((Ptr=wcschr(Ptr,L'\"'))!=NULL)
            *Ptr=0;
        }
        else
        {
          if ((Ptr=wcspbrk(Ptr,L" \t/"))!=NULL)
            *Ptr=0;
        }

        strNewValue.ReleaseBuffer ();

        IsCommandPEExeGUI(strNewValue,ImageSubsystem);
      }
      else
      {
        Error=ERROR_NO_ASSOCIATION;
        RetPtr=NULL;
      }
    }
  }

//_SVS(SysLog(L"[%d] Action='%s' Value='%s'",__LINE__,(const wchar_t *)strAction,(const wchar_t *)strValue));
  return RetPtr;
}

/*
 ������ PrepareExecuteModule �������� ����� ����������� ������ (� �.�. � ��
 %PATHEXT%). � ������ ������ �������� � Command ������, ������������� ��
 ����������� ������ �� ��������� ��������, �������� ��������� � Dest �
 �������� ��������� ��������� PE �� �������� (����� ��������� �������
 � ��������� ���� � �� ����� ����������).
 � ������ ������� Dest �� �����������!
 Return: TRUE/FALSE - �����/�� �����
*/
/* $ 14.06.2002 VVM
 ������� � ������� ���������� ��� �������������. ������ �� ������.
 � ��������� ������ �� ����, �.�. ��� ��������� �� ������� ������
*/
int WINAPI PrepareExecuteModule(const wchar_t *Command, string &strDest,DWORD& ImageSubsystem)
{
  int Ret, I;
  wchar_t *Ptr;

  string strCommand = Command;

  string strFileName;
  string strFullName;

  // ����� ������� �����! ������� �������,  � ����� ��������� �����.
  static wchar_t StdExecuteExt[NM]=L".BAT;.CMD;.EXE;.COM;";
  static const wchar_t RegPath[]=L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
  static int PreparePrepareExt=FALSE;

  if(!PreparePrepareExt) // ���������������������� �����
  {
    // ���� ���������� %PATHEXT% ��������...
    if((I=apiGetEnvironmentVariable(L"PATHEXT",strFullName)) != 0)
    {
      static wchar_t const * const StdExecuteExt0[4]={L".BAT;",L".CMD;",L".EXE;",L".COM;"};
      for(I=0; I < sizeof(StdExecuteExt0)/sizeof(StdExecuteExt0[0]); ++I)
        ReplaceStrings(strFullName,StdExecuteExt0[I],L"",-1);
    }


    strFullName += ";";

    Ptr=wcscat(StdExecuteExt, strFullName);  //BUGBUG
    StdExecuteExt[StrLength(StdExecuteExt)]=0;
    while(*Ptr)
    {
      if(*Ptr == L';')
        *Ptr=0;
      ++Ptr;
    }
    PreparePrepareExt=TRUE;
  }

  /* ����� "����������" �� �������, ������� ������ ����������� ��������,
     ��������, ��������� ���������� ������� ���.����������.
  */
  static wchar_t ExcludeCmds[4096]={0};
  static int PrepareExcludeCmds=FALSE;
  if(GetRegKey(strSystemExecutor,L"Type",0))
  {
    if (!PrepareExcludeCmds)
    {
      GetRegKey(strSystemExecutor,L"ExcludeCmds",(PBYTE)ExcludeCmds,(PBYTE)L"",sizeof(ExcludeCmds));
      Ptr=wcscat(ExcludeCmds,L";"); //!!!
      ExcludeCmds[StrLength(ExcludeCmds)]=0;
      while(*Ptr)
      {
        if(*Ptr == L';')
          *Ptr=0;
        ++Ptr;
      }
      PrepareExcludeCmds=TRUE;
    }
  }
  else
  {
    *ExcludeCmds=0;
    PrepareExcludeCmds=FALSE;
  }

  ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN; // GUIType ������ ������� ���������������� � FALSE
  Ret=FALSE;

  if( strCommand.IsEmpty() ) // ��� ��, ���� ��... �������� �������� :-(
    return 0;

  strFileName = strCommand;

  // ������� ������ - ������� ����������
  {
    wchar_t *Ptr=ExcludeCmds;
    while(*Ptr)
    {
      if(!StrCmpI(strFileName,Ptr))
      {
        ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
        return TRUE;
      }
      Ptr+=StrLength(Ptr)+1;
    }
  }


  {
    wchar_t *lpwszFilePart;

    strFullName = strFileName;

    const wchar_t *PtrFName = PointToName(strFullName);
    PtrFName = wcsrchr (PtrFName, L'.');

    string strWorkName;

    wchar_t *PtrExt=StdExecuteExt;
    while(*PtrExt) // ������ ������ - � ������� ��������
    {
      strWorkName = strFullName;

      if (!PtrFName)
        strWorkName += PtrExt;

      if(GetFileAttributesW(strFullName) != -1)
      {
        ConvertNameToFull (strFullName, strFullName);
        Ret=TRUE;
        break;
      }
      PtrExt+=StrLength(PtrExt)+1;
    }

    if(!Ret) // ������ ������ - �� �������� SearchPath
    {
      string strPathEnv;
      if (apiGetEnvironmentVariable(L"PATH",strPathEnv) != 0)
      {
        PtrExt=StdExecuteExt;

        DWORD dwSize = 0;

        while(*PtrExt)
        {
          strWorkName = strFullName;

          if (!PtrFName)
            strWorkName += PtrExt;

          if ( (dwSize = SearchPathW (strPathEnv, strFullName, PtrExt, 0, NULL, NULL)) != 0 )
          {
            wchar_t *lpwszFullName = strFullName.GetBuffer (dwSize);

            SearchPathW(strPathEnv,strFullName,PtrExt,dwSize,lpwszFullName,&lpwszFilePart);

            strFullName.ReleaseBuffer ();

            Ret=TRUE;
            break;
          }

          PtrExt+=StrLength(PtrExt)+1;
        }
      }

      DWORD dwSize;

      if (!Ret)
      {
        PtrExt=StdExecuteExt;
        while(*PtrExt)
        {
          strWorkName = strFullName;

          if (!PtrFName)
            strWorkName += PtrExt;



          if ( (dwSize = SearchPathW (NULL, strFullName, PtrExt, 0, NULL, NULL)) != 0 )
          {
            wchar_t *lpwszFullName = strFullName.GetBuffer (dwSize);

            SearchPathW(NULL,strFullName,PtrExt,dwSize,lpwszFullName,&lpwszFilePart);

            strFullName.ReleaseBuffer ();

            Ret=TRUE;
            break;
          }

          PtrExt+=StrLength(PtrExt)+1;
        }
      }

      if (!Ret && Opt.ExecuteUseAppPath) // ������ ������ - ����� � ������ � "App Paths"
      {
        // � ������ Command ������� ����������� ������ �� ������ ����, �������
        // ������� �� SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
        // ������� ������� � HKCU, ����� - � HKLM
        HKEY hKey;
        HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE};

        strFullName = RegPath+strFileName;

        for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
        {
          if (RegOpenKeyExW(RootFindKey[I], strFullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
          {
            DWORD Type, DataSize;

            DataSize = (DWORD)strFullName.GetLength()*2; //???

            wchar_t *lpwszFullName = strFullName.GetBuffer ();

            RegQueryValueExW(hKey,L"", 0, &Type, (LPBYTE)lpwszFullName, &DataSize);

            strFullName.ReleaseBuffer ();

            RegCloseKey(hKey);
            /* $ 03.10.2001 VVM ���������� ���������� ��������� */
            strFileName = strFullName;
            apiExpandEnvironmentStrings(strFileName, strFullName);
            Unquote(strFullName);
            Ret=TRUE;
            break;
          }
        }

        if (!Ret && Opt.ExecuteUseAppPath)
        {
          PtrExt=StdExecuteExt;

          while(*PtrExt && !Ret)
          {
            strFullName = RegPath+strFileName+PtrExt;

            for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
            {
              if (RegOpenKeyExW(RootFindKey[I], strFullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
              {
                DWORD Type, DataSize;

                DataSize = (DWORD)strFullName.GetLength()*2;

                wchar_t *lpwszFullName = strFullName.GetBuffer ();

                RegQueryValueExW(hKey,L"", 0, &Type, (LPBYTE)lpwszFullName,&DataSize);

                strFullName.GetBuffer ();

                RegCloseKey(hKey);
                /* $ 03.10.2001 VVM ���������� ���������� ��������� */
                strFileName = strFullName;
                apiExpandEnvironmentStrings(strFileName, strFullName);
                Unquote(strFullName);
                Ret=TRUE;
                break;
              }
            } /* for */
            PtrExt+=StrLength(PtrExt)+1;
          }
        } /* if */
      } /* if */
    }
  }

  if(Ret) // ��������� "�������" ������
  {
    IsCommandPEExeGUI(strFullName,ImageSubsystem);

    strDest = strFullName;
  }

  return(Ret);
}

#ifdef ADD_GUI_CHECK
DWORD IsCommandExeGUI(const char *Command)
{
  char FileName[4096],FullName[4096],*EndName,*FilePart;

  if (*Command=='\"')
  {
    FAR_OemToChar(Command+1,FullName);
    if ((EndName=strchr(FullName,'\"'))!=NULL)
      *EndName=0;
  }
  else
  {
    FAR_OemToChar(Command,FullName);
    if ((EndName=strpbrk(FullName," \t/"))!=NULL)
      *EndName=0;
  }
  int GUIType=0;

  ExpandEnvironmentStrings(FullName,FileName,sizeof(FileName));

  SetFileApisTo(APIS2ANSI);
  /*$ 18.09.2000 skv
    + to allow execution of c.bat in current directory,
      if gui program c.exe exists somewhere in PATH,
      in FAR's console and not in separate window.
      for(;;) is just to prevent multiple nested ifs.
  */
  for(;;)
  {
    if(BatchFileExist(FileName,FullName,sizeof(FullName)-1))
      break;

    if (SearchPath(NULL,FileName,".exe",sizeof(FullName),FullName,&FilePart))
    {
      SHFILEINFO sfi;
      DWORD ExeType=SHGetFileInfo(FullName,0,&sfi,sizeof(sfi),SHGFI_EXETYPE);
      GUIType=HIWORD(ExeType)>=0x0300 && HIWORD(ExeType)<=0x1000 &&
              /* $ 13.07.2000 IG
                 � VC, ������, ������ ������� ���: 0x4550 == 'PE', ����
                 ������ �������� ���������.
              */
              HIBYTE(ExeType)=='E' && (LOBYTE(ExeType)=='N' || LOBYTE(ExeType)=='P');
              /* IG $ */
    }
/*$ 18.09.2000 skv
    little trick.
*/
    break;
  }
  SetFileApisTo(APIS2OEM);
  return(GUIType);
}
#endif


/* ������� ��� ����������� ���� ��� ��������� ������
   ���� ���� �� ��������� ������ ��� �������� �� DriveLetter:
   ��� ���������� �� ���� �������� � Win9x
*/
void SetCurrentDirectoryForPassivePanel(string &strComspec,const wchar_t *CmdStr)
{
  Panel *PassivePanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);

  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && PassivePanel->GetType()==FILE_PANEL)
  {
    //for (int I=0;CmdStr[I]!=0;I++)
    //{
      //if (IsAlpha(CmdStr[I]) && CmdStr[I+1]==L':' && CmdStr[I+2]!=L'\\')
      //{
        string strSetPathCmd;
        string strSavePath;
        string strPanelPath;

        FarGetCurDir(strSavePath);
        PassivePanel->GetCurDir(strPanelPath);

        QuoteSpace(strPanelPath);

        strSetPathCmd = strComspec+L" /C chdir %s"+strPanelPath;

        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        memset (&si, 0, sizeof (si));
        si.cb = sizeof (si);

        CreateProcessW(
              NULL,
              (wchar_t*)(const wchar_t*)strSetPathCmd,
              NULL,
              NULL,
              FALSE,
              CREATE_DEFAULT_ERROR_MODE,
              NULL,
              NULL,
              &si,
              &pi
              );

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        FarChDir(strSavePath);
        //break;
      //}
    //}
  }
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

  bool bIsNT = (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

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

  DWORD dwAttr = GetFileAttributesW(strNewCmdStr);

  if ( SeparateWindow == 1 )
  {
      if ( strNewCmdPar.IsEmpty() && dwAttr != -1 && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
      {
          ConvertNameToFull(strNewCmdStr, strNewCmdStr);
          SeparateWindow=2;
          FolderRun=TRUE;
      }
  }


  SHELLEXECUTEINFOW seInfo;
  memset (&seInfo, 0, sizeof (seInfo));

  seInfo.cbSize = sizeof (seInfo);

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  memset (&si, 0, sizeof (si));

  si.cb = sizeof (si);

  string strComspec;
  apiGetEnvironmentVariable (L"COMSPEC", strComspec);

  if ( strComspec.IsEmpty() && (SeparateWindow != 2) )
  {
    Message(MSG_WARNING, 1, UMSG(MWarning), UMSG(MComspecNotFound), UMSG(MErrorCancelled), UMSG(MOk));
    return -1;
  }

  int Visible, Size;

  GetCursorType(Visible,Size);
  SetInitialCursorType();

  int PrevLockCount=ScrBuf.GetLockCount();
  ScrBuf.SetLockCount(0);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  int ConsoleCP = GetConsoleCP();
  int ConsoleOutputCP = GetConsoleOutputCP();

  FlushInputBuffer();
  ChangeConsoleMode(InitialConsoleMode);

  CONSOLE_SCREEN_BUFFER_INFO sbi={0,};
  GetConsoleScreenBufferInfo(hConOut,&sbi);

  wchar_t OldTitle[512];
  GetConsoleTitleW(OldTitle, sizeof(OldTitle)/sizeof(wchar_t));

  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && !strComspec.IsEmpty())
    SetCurrentDirectoryForPassivePanel(strComspec,CmdStr);

  DWORD dwSubSystem;
  DWORD dwError = 0;

  HANDLE hProcess = NULL, hThread = NULL;

  if(FolderRun && SeparateWindow==2)
    AddEndSlash(strNewCmdStr); // ����, ����� ShellExecuteEx "�������" BAT/CMD/��.�����, �� �� �������
  else
  {
    PrepareExecuteModule(strNewCmdStr,strNewCmdStr,dwSubSystem);

    if(/*!*NewCmdPar && */ dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
    {
      DWORD Error=0, dwSubSystem2=0;
      const wchar_t *ExtPtr=wcsrchr(strNewCmdStr,L'.');

      if(ExtPtr && !(StrCmpI(ExtPtr,L".exe")==0 || StrCmpI(ExtPtr,L".com")==0 ||
         IsBatchExtType(ExtPtr)))
        if(GetShellAction(strNewCmdStr,dwSubSystem2,Error) && Error != ERROR_NO_ASSOCIATION)
          dwSubSystem=dwSubSystem2;
    }

    if ( dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI )
      SeparateWindow = 2;
  }

  ScrBuf.Flush ();

  if ( SeparateWindow == 2 )
  {
    seInfo.lpFile = strNewCmdStr;
    seInfo.lpParameters = strNewCmdPar;
    seInfo.nShow = SW_SHOWNORMAL;

    seInfo.lpVerb = (dwAttr&FILE_ATTRIBUTE_DIRECTORY)?NULL:GetShellAction(strNewCmdStr, dwSubSystem, dwError);
    //seInfo.lpVerb = "open";
    seInfo.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_FLAG_DDEWAIT|SEE_MASK_NOCLOSEPROCESS;

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

    if ( bIsNT && wcspbrk (strNewCmdStr, L"&<>()@^|") )
      bDoubleQ = true;

    if ( (bIsNT && !strNewCmdPar.IsEmpty()) || bDoubleQ )
      strExecLine += L"\"";

    strExecLine += strNewCmdStr;

    if ( !strNewCmdPar.IsEmpty() )
    {
      strExecLine += L" ";
      strExecLine += strNewCmdPar;
    }

    if ( (bIsNT && !strNewCmdPar.IsEmpty()) || bDoubleQ)
      strExecLine += L"\"";

    // // ������� ������ � ����� ����� � 4NT ��� ������ �������
    SetRealColor (F_LIGHTGRAY|B_BLACK);

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
      TruncPathStr(strOutStr,ScrX-15);

      Message(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotExecute),strOutStr,UMSG(MOk));
    }
    else
    {
      ScrBuf.Flush ();

      strOutStr.Format (UMSG(MExecuteErrorMessage),(const wchar_t *)strNewCmdStr);
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

  wchar_t Cmd[1024]; //BUGBUG
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
        string strExpandedStr;
        wmemmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
        Cmd[PtrCmd-CmdStart]=0;
        //Unquote(Cmd); BUGBUG
//_SVS(SysLog(L"Cmd='%s'",Cmd));
        if (apiExpandEnvironmentStrings(Cmd,strExpandedStr)!=0)
        {
          string strFullPath;
          if(!(Cmd[1] == L':' || (Cmd[0] == L'\\' && Cmd[1]==L'\\') || strExpandedStr.At(1) == L':' || (strExpandedStr.At(0) == L'\\' && strExpandedStr.At(1)==L'\\')))
          {
            if(CtrlObject)
              CtrlObject->CmdLine->GetCurDir(strFullPath);
            else
              FarGetCurDir(strFullPath);
            AddEndSlash(strFullPath);
          }
          strFullPath += strExpandedStr;
          DWORD FileAttr=(DWORD)-1;
          if(wcspbrk(strExpandedStr,L"*?")) // ��� �����?
          {
            FAR_FIND_DATA_EX wfd;

            if ( apiGetFindDataEx (strFullPath, &wfd) )
              FileAttr=wfd.dwFileAttributes;
          }
          else
          {
            ConvertNameToFull(strFullPath, strFullPath);
            FileAttr=GetFileAttributesW(strFullPath);
          }
//_SVS(SysLog(L"%08X FullPath=%s",FileAttr,FullPath));
          if(FileAttr != (DWORD)-1 && !Not || FileAttr == (DWORD)-1 && Not)
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
        PtrCmd=wcschr(PtrCmd,' ');
        if(PtrCmd && *PtrCmd && *PtrCmd == ' ')
        {
          string strExpandedStr;
          wmemmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
          Cmd[PtrCmd-CmdStart]=0;
          DWORD ERet=apiGetEnvironmentVariable(Cmd,strExpandedStr);
//_SVS(SysLog(Cmd));
          if(ERet && !Not || !ERet && Not)
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

  if (!SeparateWindow && IsAlpha(strCmdLine.At(0)) && strCmdLine.At(1)==L':' && strCmdLine.At(2)==0)
  {
    wchar_t NewDir[10];
    swprintf(NewDir,L"%c:",Upper(strCmdLine.At(0)));
    FarChDir(strCmdLine);
    if (getdisk()!=NewDir[0]-L'A')
    {
      wcscat(NewDir,L"\\");
      FarChDir(NewDir);
    }
    SetPanel->ChangeDirToCurrent();
    return(TRUE);
  }

  // SET [����������=[������]]
  if (StrCmpNI(strCmdLine,L"SET ",4)==0)
  {
    string strCmd;

    strCmd = (const wchar_t*)strCmdLine+4;

    wchar_t *lpwszValue = strCmd.GetBuffer ();

    lpwszValue=wcschr(lpwszValue,L'=');

    if (lpwszValue==NULL)
      return(FALSE);

    *lpwszValue=0;

    if (lpwszValue[1]==0) //??
      SetEnvironmentVariableW(strCmd,NULL);
    else
    {
      string strExpandedStr;

      if (apiExpandEnvironmentStrings(lpwszValue+1,strExpandedStr) != 0)
        SetEnvironmentVariableW(strCmd,strExpandedStr);
    }

    strCmd.ReleaseBuffer ();

    return(TRUE);
  }

  if (!StrCmpNI(strCmdLine,L"REM ",4) || !StrCmpNI(strCmdLine,L"::",2))
  {
    return TRUE;
  }

  if (!StrCmpNI(strCmdLine,L"CLS",3))
  {
    if(strCmdLine.At(3))
      return FALSE;

    ClearScreen(F_LIGHTGRAY|B_BLACK);
    return TRUE;
  }

  /*
  Displays or sets the active code page number.
  CHCP [nnn]
    nnn   Specifies a code page number (Dec or Hex).
  Type CHCP without a parameter to display the active code page number.
  */
  if (!StrCmpNI(strCmdLine,L"CHCP",4))
  {
    if(strCmdLine.At(4) == 0 || !(strCmdLine.At(4) == L' ' || strCmdLine.At(4) == L'\t'))
      return(FALSE);

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
      InitKeysArray();
      CtrlObject->Cp()->Redraw();
      ScrBuf.Flush();
      return TRUE;
    }
    else  // ��� ������ ������� chcp ���� ������ ;-)
     return FALSE;
  }

  if (StrCmpNI(strCmdLine,L"IF ",3)==0)
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
  if (!SeparateWindow &&
      (StrCmpNI(strCmdLine,L"CD",Length=2)==0 || StrCmpNI(strCmdLine,L"CHDIR",Length=5)==0) &&
      (IsSpace(strCmdLine.At(Length)) || strCmdLine.At(Length)==L'\\' || strCmdLine.At(Length)==L'/' ||
      TestParentFolderName((const wchar_t*)strCmdLine+Length)))
  {
    int ChDir=(Length==5);

    while (IsSpace(strCmdLine.At(Length)))
      Length++;

    string strExpandedDir;
    strExpandedDir = (const wchar_t*)strCmdLine+Length;

    Unquote(strExpandedDir);
    apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);


    // ������������� ����� ����� �� "���������"
//    if(ExpandedDir[1] == L':' && iswalpha(ExpandedDir[0])) //BUGBUG
//      ExpandedDir[0]=towupper(ExpandedDir[0]);

    if(SetPanel->GetMode()!=PLUGIN_PANEL && strExpandedDir.At(0) == L'~' && !strExpandedDir.At(1) && GetFileAttributesW(strExpandedDir) == (DWORD)-1)
    {
      GetRegKey(strSystemExecutor,L"~",strExpandedDir,g_strFarPath);
      DeleteEndSlash(strExpandedDir);
    }

    if(wcspbrk(strExpandedDir,L"?*")) // ��� �����?
    {
      FAR_FIND_DATA_EX wfd;
      HANDLE hFile=apiFindFirstFile(strExpandedDir, &wfd);
      if(hFile!=INVALID_HANDLE_VALUE)
      {
        wchar_t *Ptr = strExpandedDir.GetBuffer (), *Ptr2;

        Ptr2=wcsrchr(Ptr,L'\\');
        if(!Ptr2)
          Ptr2=wcsrchr(Ptr,L'/');
        Ptr=Ptr2;

        if(Ptr)
        {
          *++Ptr=0;
          strExpandedDir.ReleaseBuffer ();
        }
        else
        {
          strExpandedDir.ReleaseBuffer ();
          strExpandedDir=L"";
        }


        strExpandedDir += wfd.strFileName;
        FindClose(hFile);
      }
    }
    /* $ 15.11.2001 OT
      ������� ��������� ���� �� ����� "�������" ����������.
      ���� �� ���, �� ����� �������� ������, ��� ��� ���������� ���������
    */
    DWORD DirAtt=GetFileAttributesW(strExpandedDir);
    if (DirAtt!=0xffffffff && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && PathMayBeAbsolute(strExpandedDir))
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
      SetPanel->SetCurDir(strExpandedDir,ChDir);
      return(TRUE);
    }

    //if (ExpandEnvironmentStrW(strExpandedDir,strExpandedDir)!=0)
    {
      if(CheckFolder(strExpandedDir) <= CHKFLD_NOTACCESS)
        return -1;

      if (!FarChDir(strExpandedDir))
        return(FALSE);
    }

    SetPanel->ChangeDirToCurrent();
    if (!SetPanel->IsVisible())
      SetPanel->SetTitle();

    return(TRUE);
  }
  return(FALSE);
}

// ��������� "��� ������?"
BOOL IsBatchExtType(const wchar_t *ExtPtr)
{
  const wchar_t *PtrBatchType=Opt.strExecuteBatchType;
  while(*PtrBatchType)
  {
    if(StrCmpI(ExtPtr,PtrBatchType)==0)
      return TRUE;
    PtrBatchType+=StrLength(PtrBatchType)+1;
  }

  return FALSE;
}

#ifdef ADD_GUI_CHECK
// ������ ����������? (� ������ ������ ��� - ����������� ����������)
BOOL BatchFileExist(const char *FileName,char *DestName,int SizeDestName)
{
  char *PtrBatchType=Opt.ExecuteBatchType;
  BOOL Result=FALSE;

  char *FullName=(char*)alloca(strlen(FileName)+64);
  if(FullName)
  {
    strcpy(FullName,FileName);
    char *FullNameExt=FullName+strlen(FullName);

    while(*PtrBatchType)
    {
      strcat(FullNameExt,PtrBatchType);

      if(GetFileAttributes(FullName)!=-1)
      {
        strncpy(DestName,FullName,SizeDestName);
        Result=TRUE;
        break;
      }

      PtrBatchType+=strlen(PtrBatchType)+1;
    }
  }

  return Result;
}
#endif
