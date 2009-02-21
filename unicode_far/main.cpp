/*
main.cpp

������� main.
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


#include "lang.hpp"
#include "keys.hpp"
#include "chgprior.hpp"
#include "colors.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "fileedit.hpp"
#include "fileview.hpp"
#include "lockscrn.hpp"
#include "hilight.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "language.hpp"
#include "farexcpt.hpp"
#include "imports.hpp"
#include "syslog.hpp"
#include "registry.hpp"
#include "localOEM.hpp"

#ifdef DIRECT_RT
int DirectRT=0;
#endif

class TConsoleRestore{
  private:
    string strOldTitle;
    CONSOLE_SCREEN_BUFFER_INFO sbi;
    HANDLE hOutput;
    BOOL IsRectoreConsole;
  public:
    TConsoleRestore(BOOL RectoreConsole){
      hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
      apiGetConsoleTitle (strOldTitle);
      GetConsoleScreenBufferInfo(hOutput,&sbi);
      IsRectoreConsole=RectoreConsole;
    };
    ~TConsoleRestore(){
      if(IsRectoreConsole)
      {
        SetConsoleTitleW(strOldTitle);
        //SetConsoleScreenBufferSize(hOutput,sbi.dwSize);
        SetConsoleWindowInfo(hOutput,TRUE,&sbi.srWindow);
        SetConsoleScreenBufferSize(hOutput,sbi.dwSize);
      }
    };
};

static void CopyGlobalSettings();

static void show_help(void)
{
wprintf(
L"Usage: far [switches] [apath [ppath]]\n\n"
L"wheren\n"
L"  apath - path to a folder (or a file or an archive) for the active panel\n"
L"  ppath - path to a folder (or a file or an archive) for the passive panel\n\n"
L"The following switches may be used in the command line:\n\n"
L" /?   This help.\n"
L" /a   Disable display of characters with codes 0 - 31 and 255.\n"
L" /ag  Disable display of pseudographics characters.\n"
L" /e[<line>[:<pos>]] <filename>\n"
L"      Edit the specified file.\n"
L" /i   Set small (16x16) icon for FAR console window.\n"
L" /p[<path>]\n"
L"      Search for \"common\" plugins in the directory, specified by <path>.\n"
L" /co  Forces FAR to load plugins from the cache only.\n"
L" /rc  Restore console windows settings upon exiting FAR.\n"
L" /m   Do not load macros.\n"
L" /ma  Do not execute auto run macros.\n"
L" /u <username>\n"
L"      Allows to have separate settings for different users.\n"
L" /v <filename>\n"
L"      View the specified file. If <filename> is -, data is read from the stdin.\n"
L" /x   Disable exception handling.\n");
#if defined(_DEBUGEXC)
wprintf(
L" /xd  Enable exception handling.\n");
#endif
#ifdef DIRECT_RT
wprintf(
L" /do  Direct output.\n");
#endif
}

static int MainProcess(
        const wchar_t *lpwszEditName,
        const wchar_t *lpwszViewName,
        const wchar_t *lpwszDestName1,
        const wchar_t *lpwszDestName2,
        int StartLine,
        int StartChar,
        int RegOpt
        )
{
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    ControlObject CtrlObj;

    CONSOLE_SCREEN_BUFFER_INFO InitCsbi;
    GetConsoleScreenBufferInfo(hConOut,&InitCsbi);
    SetRealColor(COL_COMMANDLINEUSERSCREEN);

    // ����� ��������� ������������ ���� ��� ������
    if(IsZoomed(hFarWnd)) ChangeVideoMode(1);

    if ( *lpwszEditName || *lpwszViewName )
    {
      NotUseCAS=TRUE;
      Opt.OnlyEditorViewerUsed=1;
      Panel *DummyPanel=new Panel;
      CmdMode=TRUE;
      _tran(SysLog(L"create dummy panels"));
      CtrlObj.CreateFilePanels();
      CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=DummyPanel;
      CtrlObj.Plugins.LoadPlugins();

      if ( *lpwszEditName )
      {
        FileEditor *ShellEditor=new FileEditor(lpwszEditName,CP_AUTODETECT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6,StartLine,StartChar);
        _tran(SysLog(L"make shelleditor %p",ShellEditor));
        if (!ShellEditor->GetExitCode()){ // ????????????
          FrameManager->ExitMainLoop(0);
        }
      }
      else // TODO: ���� else ������ ������ ����� �������� � ������������ �������� ��������� /e � /v � ���.������
      if ( *lpwszViewName )
      {
        FileViewer *ShellViewer=new FileViewer(lpwszViewName,FALSE);
        if (!ShellViewer->GetExitCode()){
          FrameManager->ExitMainLoop(0);
        }
        _tran(SysLog(L"make shellviewer, %p",ShellViewer));
      }

      FrameManager->EnterMainLoop();
      CtrlObj.Cp()->LeftPanel=CtrlObj.Cp()->RightPanel=CtrlObj.Cp()->ActivePanel=NULL;
      delete DummyPanel;
      _tran(SysLog(L"editor/viewer closed, delete dummy panels"));
    }
    else
    {
      Opt.OnlyEditorViewerUsed=0;
      NotUseCAS=FALSE;

      Opt.SetupArgv=0;

      string strPath;
      // ������������� ���, ��� ControlObject::Init() ������� ������
      // ���� Opt.*
      if( *lpwszDestName1 ) // ������� ������
      {
        Opt.SetupArgv++;

        strPath = lpwszDestName1;

        CutToNameUNC(strPath);

        DeleteEndSlash(strPath); // BUGBUG!! ���� �������� ����� �� ������ - �������� �������� ������ - ����������� ".."

        if((strPath.At(1)==L':' && !strPath.At(2)) || (PathPrefix(strPath) && strPath.At(5)==L':' && !strPath.At(6)))
          AddEndSlash(strPath);

        // �� ������, ������� ����� ����� - ������� (������ �� �������� � ����� ������ ;-)
        if(Opt.LeftPanel.Focus)
        {
          Opt.LeftPanel.Type=FILE_PANEL;  // ������ ���� ������
          Opt.LeftPanel.Visible=TRUE;     // � ������� ��

          Opt.strLeftFolder = strPath;
        }
        else
        {
          Opt.RightPanel.Type=FILE_PANEL;
          Opt.RightPanel.Visible=TRUE;

          Opt.strRightFolder = strPath;
        }

        if( *lpwszDestName2 ) // ��������� ������
        {
          Opt.SetupArgv++;

          strPath = lpwszDestName2;

          CutToNameUNC(strPath);

          DeleteEndSlash(strPath); //BUGBUG

          if((strPath.At(1)==L':' && !strPath.At(2)) || (PathPrefix(strPath) && strPath.At(5)==L':' && !strPath.At(6)))
            AddEndSlash(strPath);

          // � ����� � �������� �������� - ������������ ��������� ������
          if(Opt.LeftPanel.Focus)
          {
            Opt.RightPanel.Type=FILE_PANEL; // ������ ���� ������
            Opt.RightPanel.Visible=TRUE;    // � ������� ��
            Opt.strRightFolder = strPath;
          }
          else
          {
            Opt.LeftPanel.Type=FILE_PANEL;
            Opt.LeftPanel.Visible=TRUE;
            Opt.strLeftFolder = strPath;
          }
        }
      }

      // ������ ��� ������ - ������� ������!
      CtrlObj.Init();

      // � ������ "����������" � ������� ��� ����-���� (���� ��������� ;-)
      if( *lpwszDestName1 ) // ������� ������
      {
        LockScreen LockScr;
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

        strPath = PointToNameUNC(lpwszDestName1);

        if ( !strPath.IsEmpty() )
        {
          if (ActivePanel->GoToFile(strPath))
            ActivePanel->ProcessKey(KEY_CTRLPGDN);
        }

        if( *lpwszDestName2 ) // ��������� ������
        {
          strPath = PointToNameUNC(lpwszDestName2);

          if ( !strPath.IsEmpty() )
          {
            if (AnotherPanel->GoToFile(strPath))
              AnotherPanel->ProcessKey(KEY_CTRLPGDN);
          }
        }

        // !!! �������� !!!
        // ������� �������� ��������� ������, � ����� ��������!
        AnotherPanel->Redraw();
        ActivePanel->Redraw();
      }

      FrameManager->EnterMainLoop();
    }

    // ������� �� �����!
    SetScreen(0,0,ScrX,ScrY,L' ',COL_COMMANDLINEUSERSCREEN);
    SetConsoleTextAttribute(hConOut,InitCsbi.wAttributes);
    ScrBuf.ResetShadow();
    ScrBuf.Flush();
    MoveRealCursor(0,0);
  }

  CloseConsole();
  RestoreIcons();
  return(0);
}

#ifdef __GNUC__
int _cdecl wmain(int Argc, wchar_t *Argv[]);

int main()
{
    LPWSTR* wstrCmdLineArgs;
    int nArgs;
    wstrCmdLineArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);

    return wmain(nArgs, wstrCmdLineArgs);
}
#endif
int wmain_sehed(string& strEditName,string& strViewName,string& DestName1,string& DestName2,int StartLine,int StartChar,int RegOpt)
{
  TRY{
    return MainProcess(strEditName,strViewName,DestName1,DestName2,StartLine,StartChar,RegOpt);
  }
  EXCEPT(xfilter((int)(INT_PTR)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1)){
     TerminateProcess( GetCurrentProcess(), 1);
  }
  return -1; // ������� ���� �� �������
}
int _cdecl wmain(int Argc, wchar_t *Argv[])
{
  ifn.Load();

  _OT(SysLog(L"[[[[[[[[New Session of FAR]]]]]]]]]"));

  string strEditName;
  string strViewName;

  string DestNames[2];

  int StartLine=-1,StartChar=-1,RegOpt=FALSE,RectoreConsole=FALSE;
  int CntDestName=0; // ���������� ����������-���� ���������

  CmdMode=FALSE;

  WinVer.dwOSVersionInfoSize=sizeof(WinVer);
  GetVersionExW(&WinVer);

  /*$ 18.04.2002 SKV
    ���������� floating point ��� �� �������������������� vc-��� fprtl.
  */
#ifdef _MSC_VER
  float x=1.1f;
  char buf[15];
  sprintf(buf,"%f",x);
#endif

  // ���� ��� ���������, �� ��������� ���������� ����������,
  //  ����� - ������ ��� ������ �����.
#if defined(_DEBUGEXC)
  Opt.ExceptRules=-1;
#else
  Opt.ExceptRules=IsDebuggerPresent()?0:-1;
#endif


//  Opt.ExceptRules=-1;
//_SVS(SysLog(L"Opt.ExceptRules=%d",Opt.ExceptRules));


  SetRegRootKey(HKEY_CURRENT_USER);
  Opt.strRegRoot = L"Software\\Far2";
  // �� ��������� - ����� ������� �� ��������� ��������
  Opt.LoadPlug.MainPluginDir=TRUE;
  Opt.LoadPlug.PluginsPersonal=TRUE;
  Opt.LoadPlug.PluginsCacheOnly=FALSE;

  if ( apiGetModuleFileName (NULL, g_strFarPath) )
  {
    CutToSlash(g_strFarPath,true);

    ConvertNameToLong (g_strFarPath, g_strFarPath);
		PrepareDiskPath(g_strFarPath);
    SetEnvironmentVariableW (L"FARHOME", g_strFarPath);

    AddEndSlash(g_strFarPath);
  }

  // ������� �� ��������
  Opt.DisableMacro=0;

  for (int I=1;I<Argc;I++)
  {
    if ((Argv[I][0]==L'/' || Argv[I][0]==L'-') && Argv[I][1])
    {
      switch(Upper(Argv[I][1]))
      {
        case L'A':
          switch (Upper(Argv[I][2]))
          {
            case 0:
              Opt.CleanAscii=TRUE;
              break;
            case L'G':
              if(!Argv[I][3])
                Opt.NoGraphics=TRUE;
              break;
          }
          break;
        case L'E':
          if (iswdigit(Argv[I][2]))
          {
            StartLine=_wtoi(&Argv[I][2]);
            wchar_t *ChPtr=wcschr(&Argv[I][2],L':');
            if (ChPtr!=NULL)
              StartChar=_wtoi(ChPtr+1);
          }
          if (I+1<Argc)
          {
            strEditName = Argv[I+1];
            I++;
          }
          break;
        case L'V':
          if (I+1<Argc)
          {
            strViewName = Argv[I+1];
            I++;
          }
          break;
        case L'R':
          switch (Upper(Argv[I][2]))
          {
            case 0:
              RegOpt=TRUE;
              break;
            case L'C':
              if(!Argv[I][3])
                RectoreConsole=TRUE;
              break;
          }
          break;
        case L'M':
          switch (Upper(Argv[I][2]))
          {
            case 0:
              Opt.DisableMacro|=MDOL_ALL;
              break;
            case L'A':
              if(!Argv[I][3])
                Opt.DisableMacro|=MDOL_AUTOSTART;
              break;
          }
          break;
        case L'I':
          Opt.SmallIcon=TRUE;
          break;
        case L'X':
          Opt.ExceptRules=0;
#if defined(_DEBUGEXC)
          if ( Upper(Argv[I][2])==L'D' && !Argv[I][3] )
            Opt.ExceptRules=1;
#endif
          break;
        case L'U':
          if (I+1<Argc)
          {
            Opt.strRegRoot += L"\\Users\\";
            Opt.strRegRoot += Argv[I+1];
            SetEnvironmentVariableW(L"FARUSER", Argv[I+1]);
            CopyGlobalSettings();
            I++;
          }
          break;
        case L'P':
        {
          // ������� 19
          if(Opt.Policies.DisabledOptions&FFPOL_USEPSWITCH)
             break;
          Opt.LoadPlug.PluginsPersonal=FALSE;
          Opt.LoadPlug.MainPluginDir=FALSE;

          if (Argv[I][2])
          {
            apiExpandEnvironmentStrings (&Argv[I][2], Opt.LoadPlug.strCustomPluginsPath);
            Unquote(Opt.LoadPlug.strCustomPluginsPath);
            ConvertNameToFull(Opt.LoadPlug.strCustomPluginsPath,Opt.LoadPlug.strCustomPluginsPath);
/*
            if(Argv[I][2]==L'.' && (Argv[I][3]==0 || Argv[I][3]==L'\\' || Argv[I][3]==L'.'))
            {
              GetCurrentDirectoryExW(Opt.LoadPlug.strCustomPluginsPath);
              AddEndSlashW(Opt.LoadPlug.strCustomPluginsPath);
              Opt.LoadPlug.strCustomPluginsPath += Argv[I][2];
            }
            else
                Opt.LoadPlug.strCustomPluginsPath = Argv[I][2];
*/
              //xstrncpy(Opt.LoadPlug.CustomPluginsPath,szPath,sizeof(Opt.LoadPlug.CustomPluginsPath));

            //CharToOemA(Opt.LoadPlug.CustomPluginsPath,Opt.LoadPlug.CustomPluginsPath);
          }
          else
          {
            // ���� ������ -P ��� <����>, ��, �������, ��� ��������
            //  ������� �� ��������� ��������!!!
            Opt.LoadPlug.strCustomPluginsPath=L"";
          }
          break;
        }
        case L'C':
            if (Upper(Argv[I][2])==L'O' && !Argv[I][3])
            {
              Opt.LoadPlug.PluginsCacheOnly=TRUE;
              Opt.LoadPlug.PluginsPersonal=FALSE;
            }
            break;
        case L'?':
        case L'H':
          ControlObject::ShowCopyright(1);
          show_help();
          exit(0);
#ifdef DIRECT_RT
        case L'D':
          if ( Upper(Argv[I][2])==L'O' && !Argv[I][3])
            DirectRT=1;
          break;
#endif
      }
    }
    else // ������� ���������. �� ����� ���� max ��� �����.
    {
      if(CntDestName < 2)
      {
        apiExpandEnvironmentStrings (Argv[I], DestNames[CntDestName]);
        Unquote(DestNames[CntDestName]);
        ConvertNameToFull(Argv[I],DestNames[CntDestName]);
				if(apiGetFileAttributes(DestNames[CntDestName]) != INVALID_FILE_ATTRIBUTES)
          CntDestName++; //???
      }
    }
  }

  //��������� OEM ����������. ������ ���� ����� CopyGlobalSettings � ����� InitKeysArray!
	LocalUpperInit();
	InitLCIDSort();
  //������������� ������� ������. ������ ���� ����� CopyGlobalSettings!
  InitKeysArray();

  WaitForInputIdle(GetCurrentProcess(),0);

  TConsoleRestore __ConsoleRestore(RectoreConsole);

  std::set_new_handler(0);

  /* $ 08.01.2003 SVS
     BugZ#765 - ����� ��������� ������ �������� ������������.
  */
  if(Opt.LoadPlug.MainPluginDir)
    Opt.LoadPlug.PluginsCacheOnly=FALSE;

  if(Opt.LoadPlug.PluginsCacheOnly)
  {
    Opt.LoadPlug.strCustomPluginsPath=L"";
    Opt.LoadPlug.MainPluginDir=FALSE;
    Opt.LoadPlug.PluginsPersonal=FALSE;
  }

  InitDetectWindowedMode();
  InitConsole();
  GetRegKey(L"Language",L"Main",Opt.strLanguage,L"English");
  if (!Lang.Init(g_strFarPath,true,MNewFileName))
  {
    ControlObject::ShowCopyright(1);
    fprintf(stderr,"\nError: Cannot load language data\n\nPress any key...");
    FlushConsoleInputBuffer(hConInp);
    WaitKey(); // � ����� �� ������� �������??? �����
    exit(0);
  }
	strInitedLanguage=Opt.strLanguage;
  SetEnvironmentVariableW(L"FARLANG",Opt.strLanguage);
  SetHighlighting();


  DeleteEmptyKey(HKEY_CLASSES_ROOT,L"Directory\\shellex\\CopyHookHandlers");

  initMacroVarTable(1);

  int Result=0;
  SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX | SEM_NOGPFAULTERRORBOX
#if defined (_M_IA64) && defined (_WIN64)
  | ( GetRegKey (L"System\\Exception", L"IgnoreDataAlignmentFaults", 0) ? SEM_NOALIGNMENTFAULTEXCEPT:0 )
#endif
  );
  if(Opt.ExceptRules)
  {
    Result=wmain_sehed(strEditName,strViewName,DestNames[0],DestNames[1],StartLine,StartChar,RegOpt);
  }
  else
    Result=MainProcess(strEditName,strViewName,DestNames[0],DestNames[1],StartLine,StartChar,RegOpt);

  UsedInternalClipboard=TRUE;
  FAR_EmptyClipboard();


  doneMacroVarTable(1);

  _OT(SysLog(L"[[[[[Exit of FAR]]]]]]]]]"));
  return Result;
}

/* $ 03.08.2000 SVS
  ! �� ���������� ������ ������ ������ ��� ���-������
*/
void CopyGlobalSettings()
{
  if (CheckRegKey(L"")) // ��� ������������ - ������������
    return;
  // ������ ������ ���� - ��������� ������!
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  CopyKeyTree(L"Software\\Far2",Opt.strRegRoot,L"Software\\Far2\\Users\0");
  SetRegRootKey(HKEY_CURRENT_USER);
  CopyKeyTree(L"Software\\Far2",Opt.strRegRoot,L"Software\\Far2\\Users\0Software\\Far2\\PluginsCache\0");
  //  "��������" ���� �� �������!!!
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  GetRegKey(L"System",L"TemplatePluginsPath",Opt.LoadPlug.strPersonalPluginsPath,L"");
  // ������!!!
  DeleteRegKey(L"System");
  // ������� ����� ��������!
  SetRegRootKey(HKEY_CURRENT_USER);
  SetRegKey(L"System",L"PersonalPluginsPath",Opt.LoadPlug.strPersonalPluginsPath);
}
