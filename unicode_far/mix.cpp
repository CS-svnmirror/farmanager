/*
mix.cpp

���� ������ ��������������� �������
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

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "treelist.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "savefpos.hpp"
#include "chgprior.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
#include "constitle.hpp"
#include "udlist.hpp"
#include "manager.hpp"
#include "lockscrn.hpp"
#include "lasterror.hpp"
#include "RefreshFrameManager.hpp"
#include "filefilter.hpp"
#include "imports.hpp"

long filelen(FILE *FPtr)
{
  SaveFilePos SavePos(FPtr);
  fseek(FPtr,0,SEEK_END);
  return(ftell(FPtr));
}

__int64 filelen64(FILE *FPtr)
{
  SaveFilePos SavePos(FPtr);
  fseek64(FPtr,0,SEEK_END);
  return(ftell64(FPtr));
}

BOOL FarChDir(const wchar_t *NewDir, BOOL ChangeDir)
{
  if(!NewDir || *NewDir == 0)
    return FALSE;

  BOOL rc=FALSE;
  wchar_t Drive[4]=L"=A:";

  string strCurDir;
  wchar_t *lpwszCurDir;

  if(IsAlpha(*NewDir) && NewDir[1]==L':' && NewDir[2]==0)// ���� ������� ������
  {                                                     // ����� �����, �� ����
    Drive[1]=Upper(*NewDir);                          // ������� �� ����������

    if ( !apiGetEnvironmentVariable (Drive, strCurDir) )
    {
      strCurDir = NewDir;
      AddEndSlash(strCurDir);

      wchar_t *lpwszChr = strCurDir.GetBuffer();

      while(*lpwszChr)
      {
        if(*lpwszChr==L'/')
           *lpwszChr=L'\\';
        ++lpwszChr;
      }

      strCurDir.ReleaseBuffer ();
    }
    //*CurDir=toupper(*CurDir); ����!
    if(ChangeDir)
    {
      rc=SetCurrentDirectoryW(strCurDir);
    }
  }
  else
  {
    strCurDir = NewDir;

    if(!StrCmp(strCurDir,L"\\"))
      FarGetCurDir(strCurDir); // ����� ����� ������

    wchar_t *lpwszChr = strCurDir.GetBuffer();

    while(*lpwszChr)
    {
      if(*lpwszChr==L'/')
         *lpwszChr=L'\\';
      ++lpwszChr;
    }

    strCurDir.ReleaseBuffer ();

    if(ChangeDir)
    {
      wchar_t *ptr;

      //Mantis#459 - ����� +3 ��� ��� � Win2K SP4 ���� ���� � GetFullPathNameW
      int nSize = GetFullPathNameW(NewDir,0,NULL,&ptr) + 3;
      lpwszCurDir = strCurDir.GetBuffer (nSize+1);
      GetFullPathNameW(NewDir,nSize,lpwszCurDir,&ptr);
      AddEndSlash(lpwszCurDir); //???????????????
      strCurDir.ReleaseBuffer ();

      PrepareDiskPath(strCurDir);
      rc=SetCurrentDirectoryW(strCurDir);

    }
  }

  if(rc || !ChangeDir)
  {
    int nSize = GetCurrentDirectoryW (0, NULL);

    lpwszCurDir = strCurDir.GetBuffer (nSize);

    if ((!ChangeDir || GetCurrentDirectoryW(nSize,lpwszCurDir)) &&
        IsAlpha(*lpwszCurDir) && lpwszCurDir[1]==L':')
    {
      Drive[1]=Upper(*lpwszCurDir);
      SetEnvironmentVariableW(Drive,lpwszCurDir);
    }

    strCurDir.ReleaseBuffer ();
  }
  return rc;
}

/* $ 20.03.2002 SVS
 ������� ������ ������� ��������� �������� ����.
 ��� ���������� ���� ��������� ����� ����� � uppercase
*/

DWORD FarGetCurDir(string &strBuffer)
{
    int nLength = GetCurrentDirectoryW (0, NULL);

    wchar_t *lpwszBuffer = strBuffer.GetBuffer (nLength);

    DWORD Result = GetCurrentDirectoryW (nLength, lpwszBuffer);

    /*
    ��� � GetCurrentDirectory:
    ���� ������� ������� - "\\?\Volume{GUID}\",
    �� � Buffer �� �������� ����������� ����
    � ���������� ������ � ����� ���� ������������.
    */
    if(IsLocalVolumeRootPath(lpwszBuffer))
      AddEndSlash(lpwszBuffer);

    if ( Result &&
         IsAlpha (*lpwszBuffer) &&
         lpwszBuffer[1] == L':' &&
         (lpwszBuffer[2] == 0 || lpwszBuffer[2] == L'\\')
         )
         *lpwszBuffer = Upper (*lpwszBuffer);

    strBuffer.ReleaseBuffer ();

    return Result;
}


DWORD NTTimeToDos(FILETIME *ft)
{
  WORD DosDate,DosTime;
  FILETIME ct;
  FileTimeToLocalFileTime(ft,&ct);
  FileTimeToDosDateTime(&ct,&DosDate,&DosTime);
  return(((DWORD)DosDate<<16)|DosTime);
}

void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator)
{
  string strDigit;
  const wchar_t *PtrDigit;
  int I;

  Dst[0]=Dst[1]=Dst[2]=(unsigned)-1;
  I=0;
  const wchar_t *Ptr=Src;
  while((Ptr=GetCommaWord(Ptr,strDigit,Separator)) != NULL)
  {
    PtrDigit=strDigit;
    while (*PtrDigit && !iswdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      Dst[I]=_wtoi(PtrDigit);
    ++I;
  }
}

void StrToDateTime(const wchar_t *CDate, const wchar_t *CTime, FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator)
{
  unsigned DateN[3],TimeN[3];
  SYSTEMTIME st;
  FILETIME lft;

  // ����������� �������� ������������� ���� � �����
  GetFileDateAndTime(CDate,DateN,DateSeparator);
  GetFileDateAndTime(CTime,TimeN,GetTimeSeparator());
  if(DateN[0] == (unsigned)-1 || DateN[1] == (unsigned)-1 || DateN[2] == (unsigned)-1)
  {
    // ������������ ������� ���� ������, ������ ������� ���� � �����.
    memset(&ft,0,sizeof(ft));
    return;
  }

  memset(&st,0,sizeof(st));

  // "�������"
  switch(DateFormat)
  {
    case 0:
      st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
    case 1:
      st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
    default:
      st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
  }
  st.wHour   = TimeN[0]!=(unsigned)-1?(TimeN[0]):0;
  st.wMinute = TimeN[1]!=(unsigned)-1?(TimeN[1]):0;
  st.wSecond = TimeN[2]!=(unsigned)-1?(TimeN[2]):0;

  if (st.wYear<100)
  {
    if (st.wYear<80)
      st.wYear+=2000;
    else
      st.wYear+=1900;
  }

  // �������������� � "������������" ������
  SystemTimeToFileTime(&st,&lft);
  LocalFileTimeToFileTime(&lft,&ft);
  return;
}

void ConvertDate (const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
                 int Brief,int TextMonth,int FullYear,int DynInit)
{
  static int WDateFormat,WDateSeparator,WTimeSeparator;
  static int Init=FALSE;
  static SYSTEMTIME lt;
  int DateFormat,DateSeparator,TimeSeparator;
  if (!Init)
  {
    WDateFormat=GetDateFormat();
    WDateSeparator=GetDateSeparator();
    WTimeSeparator=GetTimeSeparator();
    GetLocalTime(&lt);
    Init=TRUE;
  }
  DateFormat=DynInit?GetDateFormat():WDateFormat;
  DateSeparator=DynInit?GetDateSeparator():WDateSeparator;
  TimeSeparator=DynInit?GetTimeSeparator():WTimeSeparator;

  int CurDateFormat=DateFormat;
  if (Brief && CurDateFormat==2)
    CurDateFormat=0;

  SYSTEMTIME st;
  FILETIME ct;

  if (ft.dwHighDateTime==0)
  {
    strDateText=L"";
    strTimeText=L"";
    return;
  }

  FileTimeToLocalFileTime(&ft,&ct);
  FileTimeToSystemTime(&ct,&st);

  //if ( !strTimeText.IsEmpty() )
  {
    const wchar_t *Letter=L"";
    if (TimeLength==6)
    {
      Letter=(st.wHour<12) ? L"a":L"p";
      if (st.wHour>12)
        st.wHour-=12;
      if (st.wHour==0)
        st.wHour=12;
    }
    if (TimeLength<7)
      strTimeText.Format (L"%02d%c%02d%s",st.wHour,TimeSeparator,st.wMinute,Letter);
    else
    {
      string strFullTime;
      strFullTime.Format (L"%02d%c%02d%c%02d.%03d",st.wHour,TimeSeparator,
              st.wMinute,TimeSeparator,st.wSecond,st.wMilliseconds);
      strTimeText.Format (L"%.*s",TimeLength, (const wchar_t*)strFullTime);
    }
  }

  //if ( !strDateText.IsEmpty() )
  {
    int Year=st.wYear;
    if (!FullYear)
      Year%=100;
    if (TextMonth)
    {
      const wchar_t *Month=MSG(MMonthJan+st.wMonth-1);
      switch(CurDateFormat)
      {
        case 0:
          strDateText.Format (L"%3.3s %2d %02d",Month,st.wDay,Year);
          break;
        case 1:
          strDateText.Format (L"%2d %3.3s %02d",st.wDay,Month,Year);
          break;
        default:
          strDateText.Format (L"%02d %3.3s %2d",Year,Month,st.wDay);
          break;
      }
    }
    else
    {
      int p1,p2,p3=Year;
      switch(CurDateFormat)
      {
        case 0:
          p1=st.wMonth;
          p2=st.wDay;
          break;
        case 1:
          p1=st.wDay;
          p2=st.wMonth;
          break;
        default:
          p1=Year;
          p2=st.wMonth;
          p3=st.wDay;
          break;
      }
      strDateText.Format (L"%02d%c%02d%c%02d",p1,DateSeparator,p2,DateSeparator,p3);
    }
  }

  if (Brief)
  {
    strDateText.SetLength(TextMonth ? 6 : 5);

    if (lt.wYear!=st.wYear)
      strTimeText.Format (L"%5d",st.wYear);
  }
}

int GetDateFormat()
{
  wchar_t Info[100];
  GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_IDATE,Info, sizeof(Info)/sizeof (wchar_t));
  return(_wtoi(Info));
}


int GetDateSeparator()
{
  wchar_t Info[100];
  GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_SDATE,Info,sizeof(Info)/sizeof (wchar_t));
  return(*Info);
}


int GetTimeSeparator()
{
  wchar_t Info[100];
  GetLocaleInfoW(LOCALE_USER_DEFAULT,LOCALE_STIME,Info,sizeof(Info)/sizeof (wchar_t));
  return(*Info);
}


int ToPercent(unsigned long N1,unsigned long N2)
{
  if (N1 > 10000)
  {
    N1/=100;
    N2/=100;
  }
  if (N2==0)
    return(0);
  if (N2<N1)
    return(100);
  return((int)(N1*100/N2));
}

int ToPercent64(unsigned __int64 N1, unsigned __int64 N2)
{
  if (N1 > _ui64(10000))
  {
    N1/=_ui64(100);
    N2/=_ui64(100);
  }
  if (N2==_ui64(0))
    return(_ui64(0));
  if (N2<N1)
    return(100);
  return((int)(N1*_ui64(100)/N2));
}



/* $ 09.10.2000 IS
    + ����� ������� ��� ��������� ����� �����
*/
// ���������� ��� �����: �������� � ������, �������, ������������� �� �����
int WINAPI ProcessName (const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags)
{
  int skippath=flags&PN_SKIPPATH;

  if(flags&PN_CMPNAME)
    return CmpName (param1, param2, skippath);

  if(flags&PN_CMPNAMELIST)
  {
    int Found=FALSE;
    string strFileMask;
    const wchar_t *MaskPtr;
    MaskPtr=param1;

    while ((MaskPtr=GetCommaWord(MaskPtr,strFileMask))!=NULL)
      if (CmpName(strFileMask,param2,skippath))
      {
        Found=TRUE;
        break;
      }
    return Found;
  }

  if(flags&PN_GENERATENAME)
  {
      string strResult;

      int nResult = ConvertWildcards(param1, strResult, flags & 0xFFFF);

      xwcsncpy(param2, strResult, size); //?? � ����� �� size-1

      return nResult;
  }

  return FALSE;
}


int GetFileTypeByName(const wchar_t *Name)
{
  HANDLE hFile=apiCreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return(FILE_TYPE_UNKNOWN);
  int Type=GetFileType(hFile);
  CloseHandle(hFile);
  return(Type);
}


static void DrawGetDirInfoMsg(const wchar_t *Title,const wchar_t *Name)
{
  Message(0,0,Title,MSG(MScanningFolder),Name);
  PreRedrawParam.Param1=(void*)Title;
  PreRedrawParam.Param2=(void*)Name;
}

static void PR_DrawGetDirInfoMsg(void)
{
  DrawGetDirInfoMsg((const wchar_t*)PreRedrawParam.Param1,(const wchar_t *)PreRedrawParam.Param2);
}

int GetDirInfo(const wchar_t *Title,
               const wchar_t *DirName,
               unsigned long &DirCount,
               unsigned long &FileCount,
               unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,
               unsigned __int64 &RealSize,
               unsigned long &ClusterSize,
               clock_t MsgWaitTime,
               FileFilter *Filter,
               DWORD Flags)
{
  string strFullDirName, strDriveRoot;
  string strFullName, strCurDirName, strLastDirName;

  ConvertNameToFull(DirName, strFullDirName);

  SaveScreen SaveScr;
  UndoGlobalSaveScrPtr UndSaveScr(&SaveScr);

  ScanTree ScTree(FALSE,TRUE,(Flags&GETDIRINFO_SCANSYMLINKDEF?(DWORD)-1:(Flags&GETDIRINFO_SCANSYMLINK)));
  FAR_FIND_DATA_EX FindData;
  int MsgOut=0;
  clock_t StartTime=clock();

  SetCursorType(FALSE,0);
  GetPathRoot(strFullDirName,strDriveRoot);

  /* $ 20.03.2002 DJ
     ��� . - ������� ��� ������������� ��������
  */
  const wchar_t *ShowDirName = DirName;
  if (DirName[0] == L'.' && DirName[1] == 0)
  {
    const wchar_t *p = wcsrchr (strFullDirName, L'\\');
    if (p)
      ShowDirName = p + 1;
  }

  ConsoleTitle OldTitle;
  RefreshFrameManager frref(ScrX,ScrY,MsgWaitTime,Flags&GETDIRINFO_DONTREDRAWFRAME);

  PREREDRAWFUNC OldPreRedrawFunc=PreRedrawFunc;

  if ((ClusterSize=GetClusterSize(strDriveRoot))==0)
  {
    DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

    if (GetDiskFreeSpaceW(strDriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
      ClusterSize=SectorsPerCluster*BytesPerSector;
  }

  // ��������� ��������� ��� ���������
  strLastDirName=L"";
  strCurDirName=L"";

  DirCount=FileCount=0;
  FileSize=CompressedFileSize=RealSize=0;
  ScTree.SetFindPath(DirName,L"*.*");

  while (ScTree.GetNextName(&FindData,strFullName))
  {
    if (!CtrlObject->Macro.IsExecuting())
    {
      INPUT_RECORD rec;
      switch(PeekInputRecord(&rec))
      {
        case 0:
        case KEY_IDLE:
          break;
        case KEY_NONE:
        case KEY_ALT:
        case KEY_CTRL:
        case KEY_SHIFT:
        case KEY_RALT:
        case KEY_RCTRL:
          GetInputRecord(&rec);
          break;
        case KEY_ESC:
        case KEY_BREAK:
          GetInputRecord(&rec);
          SetPreRedrawFunc(OldPreRedrawFunc);
          return(0);
        default:
          if (Flags&GETDIRINFO_ENHBREAK)
          {
            SetPreRedrawFunc(OldPreRedrawFunc);
            return(-1);
          }
          GetInputRecord(&rec);
          break;
      }
    }

    if (!MsgOut && MsgWaitTime!=-1 && clock()-StartTime > MsgWaitTime)
    {
      OldTitle.Set(L"%s %s",MSG(MScanningFolder), ShowDirName); // ������� ��������� �������
      SetCursorType(FALSE,0);
      SetPreRedrawFunc(PR_DrawGetDirInfoMsg);
      DrawGetDirInfoMsg(Title,ShowDirName);
      MsgOut=1;
    }

    if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      // ������� ��������� ���������� ������ ���� �� ������� ������,
      // � ��������� ������ ��� ����� ������ � �������� ���������� ������
      if (!(Flags&GETDIRINFO_USEFILTER))
        DirCount++;
      else
      {
        // ���� ����� ������������� ������ � ��������� ��������, �� �������
        // ���� ��������� ���������� - ����� ��� ���������� �������� total
        // �� ������ (mantiss 551)
        if (Filter->FileInFilter(&FindData, true))
          ScTree.SkipDir();
      }
    }
    else
    {
      /* $ 17.04.2005 KM
         �������� ��������� ����� � ������� ������
      */
      if ((Flags&GETDIRINFO_USEFILTER))
      {
        if (!Filter->FileInFilter(&FindData))
          continue;
      }

      // ���������� ������� ��������� ��� ���������� ������� ������ �����,
      // ����� � ����� �������� ������ ����, ��������������� ��������
      // �������.
      if ((Flags&GETDIRINFO_USEFILTER))
      {
        strCurDirName = strFullName;

        CutToSlash(strCurDirName); //???

        if (StrCmpI(strCurDirName,strLastDirName)!=0)
        {
          DirCount++;
          strLastDirName = strCurDirName;
        }
      }

      FileCount++;

      unsigned __int64 CurSize = FindData.nFileSize;
      FileSize+=CurSize;
      if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) || (FindData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE))
      {
        DWORD CompressedSize,CompressedSizeHigh;
        CompressedSize=GetCompressedFileSizeW(strFullName,&CompressedSizeHigh);
        if (CompressedSize!=INVALID_FILE_SIZE || GetLastError()==NO_ERROR)
          CurSize = CompressedSizeHigh*_ui64(0x100000000)+CompressedSize;
      }
      CompressedFileSize+=CurSize;
      if (ClusterSize>0)
      {
        RealSize+=CurSize;
        int Slack=(__int32)(CurSize%ClusterSize);
        if (Slack>0)
          RealSize+=ClusterSize-Slack;
      }
    }
  }

  SetPreRedrawFunc(OldPreRedrawFunc);
  return(1);
}


int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize)
{
  struct PluginPanelItem *PanelItem=NULL;
  int ItemsNumber,ExitCode;
  DirCount=FileCount=0;
  FileSize=CompressedFileSize=0;

  PluginHandle *ph = (PluginHandle*)hPlugin;

  if ((ExitCode=FarGetPluginDirList((INT_PTR)ph->pPlugin, ph->hPlugin, DirName, &PanelItem,&ItemsNumber))==TRUE) //INT_PTR - BUGBUG
  {
    for (int I=0;I<ItemsNumber;I++)
    {
      if (PanelItem[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        DirCount++;
      else
      {
        FileCount++;
        unsigned __int64 CurSize = PanelItem[I].FindData.nFileSize;
        FileSize+=CurSize;
        if (PanelItem[I].FindData.nPackSize)
          CompressedFileSize+=CurSize;
        else
        {
          unsigned __int64 AddSize = PanelItem[I].FindData.nPackSize;
          CompressedFileSize+=AddSize;
        }
      }
    }
  }
  if (PanelItem!=NULL)
    FarFreePluginDirList(PanelItem, ItemsNumber);
  return(ExitCode);
}

/*
  ������� CheckFolder ���������� ���� ��������� ������������ ��������:

    CHKFLD_NOTFOUND   (2) - ��� ������
    CHKFLD_NOTEMPTY   (1) - �� �����
    CHKFLD_EMPTY      (0) - �����
    CHKFLD_NOTACCESS (-1) - ��� �������
    CHKFLD_ERROR     (-2) - ������ (��������� - ������ ��� ��������� ������ ��� ��������� ������������� �������)
*/

int CheckFolder(const wchar_t *Path)
{
  if(!(Path || *Path)) // �������� �� ��������
    return CHKFLD_ERROR;

/*  int LenFindPath=Max(StrLength(Path),2048)+8;
  char *FindPath=(char *)alloca(LenFindPath); // ����� alloca - ����� _�����_ ������� �� ��� ��� ���.
  if(!FindPath)
    return CHKFLD_ERROR;*/

  HANDLE FindHandle;
  FAR_FIND_DATA_EX fdata;
  int Done=FALSE;

  string strFindPath = Path;


  // ��������� ����� ��� ������.
  AddEndSlash(strFindPath);

  strFindPath += L"*.*";

  // ������ �������� - ��-���� ������� �����?
  if((FindHandle=apiFindFirstFile(strFindPath,&fdata)) == INVALID_HANDLE_VALUE)
  {
    GuardLastError lstError;
    if(lstError.Get() == ERROR_FILE_NOT_FOUND)
      return CHKFLD_EMPTY;

    // ����������... �� ����, ��� ���� �� ������, �.�. �� ������ ����� � ����� ���� ���� "."
    // ������� ��������� �� Root
    GetPathRootOne(Path,strFindPath);


    if(!StrCmp(Path,strFindPath))
    {
      // �������� ��������� ������������� ������ - ��� ���� BugZ#743 ��� ������ ������ �����.
      if(GetFileAttributesW(strFindPath)!=INVALID_FILE_ATTRIBUTES)
      {
        if(lstError.Get() == ERROR_ACCESS_DENIED)
          return CHKFLD_NOTACCESS;
        return CHKFLD_EMPTY;
      }
    }

    strFindPath = Path;

    if(CheckShortcutFolder(&strFindPath,FALSE,TRUE))
    {
      if(StrCmp(Path,strFindPath))
        return CHKFLD_NOTFOUND;
    }

    return CHKFLD_NOTACCESS;
  }

  // ��. ���-�� ����. ��������� �������� �� ������ "����� �������?"
  while(!Done)
  {
    if (fdata.strFileName.At(0) == L'.' && (fdata.strFileName.At(1) == 0 || (fdata.strFileName.At(1) == L'.' && fdata.strFileName.At(2) == 0)))
      ; // ���������� "." � ".."
    else
    {
      // ���-�� ����, �������� �� "." � ".." - ������� �� ����
      apiFindClose(FindHandle);
      return CHKFLD_NOTEMPTY;
    }
    Done=!apiFindNextFile(FindHandle,&fdata);
  }

  // ���������� ������� ����
  apiFindClose(FindHandle);
  return CHKFLD_EMPTY;
}

const wchar_t* GetLanguageString (int nID)
{
	return Lang.GetMsg(nID);
}

BOOL GetDiskSize(const wchar_t *Root,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree)
{
  int ExitCode=0;

#if 0
  // ���� �������
  ULARGE_INTEGER uiTotalSize,uiTotalFree,uiUserFree;

  uiUserFree.QuadPart = 0;
  uiTotalSize.QuadPart = 0;
  uiTotalFree.QuadPart = 0;

  if ( ifn.pfnGetDiskFreeSpaceEx )
  {
    ExitCode = ifn.pfnGetDiskFreeSpaceEx(Root,&uiUserFree,&uiTotalSize,&uiTotalFree);
    if (uiUserFree.QuadPart > uiTotalFree.QuadPart)
      uiUserFree.QuadPart=uiTotalFree.QuadPart;
  }

  if ( !ifn.pfnGetDiskFreeSpaceEx || ExitCode==0 || uiTotalSize.QuadPart==0)
  {
    DWORD SectorsPerCluster,BytesPerSector,FreeClusters,Clusters;
    ExitCode=GetDiskFreeSpaceW(Root,&SectorsPerCluster,&BytesPerSector,
                              &FreeClusters,&Clusters);
    uiTotalSize.QuadPart=SectorsPerCluster*BytesPerSector*Clusters;
    uiTotalFree.QuadPart=SectorsPerCluster*BytesPerSector*FreeClusters;
    uiUserFree.QuadPart=uiTotalFree.QuadPart;
  }

  if ( TotalSize )
    *TotalSize = uiTotalSize.QuadPart;
  if ( TotalFree )
    *TotalFree = uiTotalFree.QuadPart;
  if ( UserFree )
    *UserFree = uiUserFree.QuadPart;
#else
  unsigned __int64 uiTotalSize,uiTotalFree,uiUserFree;
  uiUserFree=_i64(0);
  uiTotalSize=_i64(0);
  uiTotalFree=_i64(0);

  if ( ifn.pfnGetDiskFreeSpaceEx )
    ExitCode=ifn.pfnGetDiskFreeSpaceEx(Root,(PULARGE_INTEGER)&uiUserFree,(PULARGE_INTEGER)&uiTotalSize,(PULARGE_INTEGER)&uiTotalFree);

  if ( !ifn.pfnGetDiskFreeSpaceEx || ExitCode==0 || uiTotalSize == _i64(0) )
  {
    DWORD SectorsPerCluster,BytesPerSector,FreeClusters,Clusters;
    ExitCode=GetDiskFreeSpaceW(Root,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters);
    uiTotalSize=(unsigned __int64)SectorsPerCluster*(unsigned __int64)BytesPerSector*(unsigned __int64)Clusters;
    uiTotalFree=(unsigned __int64)SectorsPerCluster*(unsigned __int64)BytesPerSector*(unsigned __int64)FreeClusters;
    uiUserFree=uiTotalFree;
  }

  if ( TotalSize )
    *TotalSize = uiTotalSize;
  if ( TotalFree )
    *TotalFree = uiTotalFree;
  if ( UserFree )
    *UserFree = uiUserFree;

#endif
  return(ExitCode);
}

int GetClusterSize(const wchar_t *Root)
{
#ifndef _WIN64
  struct ExtGetDskFreSpc
  {
    WORD ExtFree_Size;
    WORD ExtFree_Level;
    DWORD ExtFree_SectorsPerCluster;
    DWORD ExtFree_BytesPerSector;
    DWORD ExtFree_AvailableClusters;
    DWORD ExtFree_TotalClusters;
    DWORD ExtFree_AvailablePhysSectors;
    DWORD ExtFree_TotalPhysSectors;
    DWORD ExtFree_AvailableAllocationUnits;
    DWORD ExtFree_TotalAllocationUnits;
    DWORD ExtFree_Rsvd[2];
  } DiskInfo;

  struct _DIOC_REGISTERS
  {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
  } reg;

  BOOL fResult;
  DWORD cb;

  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_WINDOWS ||
      WinVer.dwBuildNumber<0x04000457)
    return(0);

  HANDLE hDevice = apiCreateFile(L"\\\\.\\vwin32", 0, 0, NULL, 0,
                              FILE_FLAG_DELETE_ON_CLOSE, NULL);

  if (hDevice==INVALID_HANDLE_VALUE)
    return(0);

  DiskInfo.ExtFree_Level=0;

  char *lpRoot = UnicodeToAnsi (Root);

  reg.reg_EAX = 0x7303;
  reg.reg_EDX = (DWORD)(DWORD_PTR)lpRoot;
  reg.reg_EDI = (DWORD)(DWORD_PTR)&DiskInfo;
  reg.reg_ECX = sizeof(DiskInfo);
  reg.reg_Flags = 0x0001;

  fResult=DeviceIoControl(hDevice,6,&reg,sizeof(reg),&reg,sizeof(reg),&cb,0);

  xf_free (lpRoot);

  CloseHandle(hDevice);
  if (!fResult || (reg.reg_Flags & 0x0001))
    return(0);
  return(DiskInfo.ExtFree_SectorsPerCluster*DiskInfo.ExtFree_BytesPerSector);
#else
  return 0;
#endif
}




#if 0
/*
In: "C:\WINNT\SYSTEM32\FOO.TXT", "%SystemRoot%"
Out: "%SystemRoot%\SYSTEM32\FOO.TXT"
*/
BOOL UnExpandEnvString(const char *Path, const char *EnvVar, char* Dest, int DestSize)
{
  int I;
  char Temp[NM*2];
  Temp[0] = 0;

  ExpandEnvironmentStr(EnvVar, Temp, sizeof(Temp));
  I = strlen(Temp);

  if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, Temp, I, Path, I) == 2)
  {
    if (strlen(Path)-I+strlen(EnvVar) < DestSize)
    {
      xstrncpy(Dest, EnvVar, DestSize-1);
      xstrncat(Dest, Path + I, DestSize-1);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL PathUnExpandEnvStr(const char *Path, char* Dest, int DestSize)
{
  const char *StdEnv[]={
     "%TEMP%",               // C:\Documents and Settings\Administrator\Local Settings\Temp
     "%APPDATA%",            // C:\Documents and Settings\Administrator\Application Data
     "%USERPROFILE%",        // C:\Documents and Settings\Administrator
     "%ALLUSERSPROFILE%",    // C:\Documents and Settings\All Users
     "%CommonProgramFiles%", // C:\Program Files\Common Files
     "%ProgramFiles%",       // C:\Program Files
     "%SystemRoot%",         // C:\WINNT
  };

  for(int I=0; I < sizeof(StdEnv)/sizeof(StdEnv[0]); ++I)
  {
    if(UnExpandEnvironmentString(Path, StdEnv[I], Dest, DestSize))
      return TRUE;
  }
  xstrncpy(Dest, Path, DestSize-1);
  return FALSE;

}
#endif

/* $ 30.07.2001 IS
     1. ��������� ������������ ����������.
     2. ������ ��������� ��������� �� ������� �� ����� ������
     3. ����� ����� ���� ������������ ���������� ���� (�� ��������,
        ������������� � ��.). ����� ���� ��������� ����� ������, �����������
        �������� ��� ������ � �������, ����� ��������� ����� ����������,
        ����� ��������� ����� � �������. ������, ��� ��� � ������ ���� :-)
*/
void WINAPI FarRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,FRSUSERFUNCW Func,DWORD Flags,void *Param)
{
  if(Func && InitDir && *InitDir && Mask && *Mask)
  {
    CFileMask FMask;
    if(!FMask.Set(Mask, FMF_SILENT)) return;

    Flags=Flags&0x000000FF; // ������ ������� ����!
    ScanTree ScTree(Flags & FRS_RETUPDIR,Flags & FRS_RECUR, Flags & FRS_SCANSYMLINK);
    FAR_FIND_DATA_EX FindData;

    string strFullName;

    ScTree.SetFindPath(InitDir,L"*");
    while (ScTree.GetNextName(&FindData,strFullName))
    {
      if ( FMask.Compare(FindData.strFileName) || FMask.Compare(FindData.strAlternateFileName) )
      {
          FAR_FIND_DATA fdata;

          apiFindDataExToData (&FindData, &fdata);

          if ( Func(&fdata,strFullName,Param) == 0)
          {
            apiFreeFindData(&fdata);
            break;
          }

          apiFreeFindData(&fdata);
      }
    }
  }
}

/* $ 14.09.2000 SVS
 + ������� FarMkTemp - ��������� ����� ���������� ����� � ������ �����.
    Dest - �������� ���������� (������ ���� ���������� �������, �������� NM
    Template - ������ �� �������� ������� mktemp, �������� "FarTmpXXXXXX"
   ������ ���� NULL, ���� ��������� �� Dest.
*/
/* $ 25.10.2000 IS
 ! ������� mktemp �� ����� ��������������� ������� �������, �.�. ����������
   ������� �������� � ������ (�������� �� Multiarc'�)
   �������� Prefix - ������, ����������� �� ������ ������� ����� ����������
   �����. ������������ ������ ������ 3 ������� �� ���� ������.
*/

/*
char* WINAPI FarMkTemp(char *Dest, const char *Prefix)
{
  return FarMkTempEx(Dest,Prefix,TRUE);
}

*/
/*
             v - �����
   prefXXX X X XXX
       \ / ^   ^^^\ PID + TID
        |  \------/
        |
        +---------- [0A-Z]
*/
/*
char* FarMkTempEx(char *Dest, const char *Prefix, BOOL WithPath)
{
  if(Dest)
  {
    if(!(Prefix && *Prefix))
      Prefix="FTMP";

    char TempName[NM];
    TempName[0]=0;
    if(WithPath)
      strcpy(TempName,Opt.TempPath);
    strcat(TempName,"0000XXXXXXXX");
    memcpy(TempName+strlen(TempName)-12,Prefix,Min((int)strlen(Prefix),4));
    if (farmktemp(TempName)!=NULL)
    {
      strcpy(Dest,strupr(TempName));
      return Dest;
    }
  }
  return NULL;
}
*/

wchar_t* __stdcall FarMkTemp (wchar_t *Dest, DWORD size, const wchar_t *Prefix)
{
    string strDest;

    if ( FarMkTempEx(strDest, Prefix, TRUE) )
    {
        xwcsncpy (Dest, strDest, size);  //?? � ����� �� size-1
        return Dest;
    }

    return NULL;
}

string& FarMkTempEx(string &strDest, const wchar_t *Prefix, BOOL WithPath)
{
  if(!(Prefix && *Prefix))
    Prefix=L"FTMP";

  string strPath = L".";
  if(WithPath)
      strPath = Opt.strTempPath;

  wchar_t *lpwszDest = strDest.GetBuffer (StrLength(Prefix)+strPath.GetLength()+13);

  UINT uniq = GetCurrentProcessId(), savePid = uniq;
  for(;;) {
    if(!uniq) ++uniq;
    if(   GetTempFileNameW (strPath, Prefix, uniq, lpwszDest)
       && GetFileAttributesW (lpwszDest) == INVALID_FILE_ATTRIBUTES) break;
    if(++uniq == savePid) {
      *lpwszDest = 0;
      break;
    }
  }

  strDest.ReleaseBuffer ();

  return strDest;
}

/*$ 27.09.2000 skv
  + �������� ������ ����������� ����� new char[n];
    ������� ��� �������� ������������� PasteFromClipboard
*/
void WINAPI DeleteBuffer(char *Buffer)
{
  if(Buffer) xf_free(Buffer);
}


string &DriveLocalToRemoteName(int DriveType,wchar_t Letter,string &strDest)
{
  int NetPathShown=FALSE, IsOK=FALSE;
  wchar_t LocalName[8]=L" :\0\0\0", RemoteName[NM]; //BUGBUG
  DWORD RemoteNameSize=sizeof(RemoteName)/sizeof (wchar_t);

  *LocalName=Letter;
  strDest=L"";

  if(DriveType == DRIVE_UNKNOWN)
  {
    LocalName[2]=L'\\';
    DriveType = FAR_GetDriveType(LocalName);
    LocalName[2]=0;
  }

  if (DriveType==DRIVE_REMOTE)
  {
    if (WNetGetConnectionW(LocalName,RemoteName,&RemoteNameSize)==NO_ERROR)
    {
      NetPathShown=TRUE;
      IsOK=TRUE;
    }
  }
  string strRemoteName = RemoteName;

  if (!NetPathShown)
    if (GetSubstName(DriveType,LocalName,strRemoteName))
      IsOK=TRUE;

  if(IsOK)
    strDest = strRemoteName;

  return strDest;
}


/*
  FarGetLogicalDrives
  �������� ������ GetLogicalDrives, � ������ ������� ���������� ������
  HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer
  NoDrives:DWORD
    ��������� 26 ��� ���������� ����� ������ �� A �� Z (������ ������ ������).
    ���� ����� ��� ������������� 0 � ����� ��� �������� 1.
    ���� A ����������� ������ ��������� ������ ��� �������� �������������.
    ��������, �������� 00000000000000000000010101(0x7h)
    �������� ����� A, C, � E
*/
DWORD WINAPI FarGetLogicalDrives(void)
{
  static DWORD LogicalDrivesMask = 0;
  DWORD NoDrives=0;
  if ((!Opt.RememberLogicalDrives) || (LogicalDrivesMask==0))
    LogicalDrivesMask=GetLogicalDrives();

  if(!Opt.Policies.ShowHiddenDrives)
  {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS && hKey)
    {
      int ExitCode;
      DWORD Type,Size=sizeof(NoDrives);
      ExitCode=RegQueryValueExW(hKey,L"NoDrives",0,&Type,(BYTE *)&NoDrives,&Size);
      RegCloseKey(hKey);
      if(ExitCode != ERROR_SUCCESS)
        NoDrives=0;
    }
  }
  return LogicalDrivesMask&(~NoDrives);
}

/* $ 13.10.2002 IS
   ���������� ������ � ������ ����, ����� ���������� �� strstr �
   GetCommaWord - �� ��� ������ ��������, � ���������, �� �������� ���������
   �� ����� "%pathext%,*.lnk,*.pif,*.awk,*.pln", ���� %pathext% ���������
   ".pl", �.�. ��� ��������� ������� � "*.pln"
*/

// �������������� �������� ������� PATHEXT � ��������� :-)
// ������� ���������� ������ ����������, ��� ���� ��������� ��, ��� ����
// � %PATHEXT%
// IS: ��������� �� ���������� ��������� ����� � ���, ��� ������� � Dest
// IS: �� ��������, �.�. ����� ���� �������� ��� ���������� �����
string &Add_PATHEXT(string &strDest)
{
  string strBuf;
  size_t curpos=strDest.GetLength()-1;
  UserDefinedList MaskList(0,0,ULF_UNIQUE);
  if( apiGetEnvironmentVariable(L"PATHEXT",strBuf) && MaskList.Set(strBuf))
  {
    /* $ 13.10.2002 IS �������� �� '|' (����� ����������) */
    if( !strDest.IsEmpty() && strDest.At(curpos)!=L',' && strDest.At(curpos)!=L'|')
      strDest += L",";
    const wchar_t *Ptr;
    MaskList.Reset();
    while(NULL!=(Ptr=MaskList.GetNext()))
    {
      strDest += L"*";
      strDest += Ptr;
      strDest += L",";
    }
  }
  // ������ ������� - � ����!
  /* $ 13.10.2002 IS ����������� �� �������� */
  curpos=strDest.GetLength()-1;
  if(strDest.At(curpos) == L',')
    strDest.SetLength(curpos);
  return strDest;
}


void CreatePath(string &strPath)
{
  wchar_t *ChPtr = strPath.GetBuffer ();
  wchar_t *DirPart = ChPtr;

  BOOL bEnd = FALSE;

  while ( TRUE )
  {
    if ( (*ChPtr == 0) || (*ChPtr == L'\\') )
    {
      if ( *ChPtr == 0 )
        bEnd = TRUE;

      *ChPtr = 0;

      if ( Opt.CreateUppercaseFolders && !IsCaseMixed(DirPart) && GetFileAttributesW(strPath) == INVALID_FILE_ATTRIBUTES) //BUGBUG
        CharUpperW (DirPart);

      if ( CreateDirectoryW(strPath, NULL) )
        TreeList::AddTreeName(strPath);

      if ( bEnd )
        break;

      *ChPtr = L'\\';
      DirPart = ChPtr+1;
    }

    ChPtr++;
  }
}


void SetPreRedrawFunc(PREREDRAWFUNC Func)
{
  if((PreRedrawFunc=Func) == NULL)
    memset(&PreRedrawParam,0,sizeof(PreRedrawParam));
}

int PathMayBeAbsolute(const wchar_t *Path)
{
    return (Path &&
           (
             (IsAlpha(*Path) && Path[1]==L':') ||
             (Path[0]==L'\\'  && Path[1]==L'\\') ||
             (Path[0]==L'/'   && Path[1]==L'/')
           )
         );
}

BOOL IsNetworkPath(const wchar_t *Path)
{
  return (Path && Path[0] == L'\\' && Path[1] == L'\\' && Path[2] != L'\\' && wcsrchr(Path+2,L'\\'));
}

BOOL IsLocalPath(const wchar_t *Path)
{
  return (Path && IsAlpha(*Path) && Path[1]==L':' && Path[2]);
}

BOOL IsLocalRootPath(const wchar_t *Path)
{
  return (Path && IsAlpha(*Path) && Path[1]==L':' && Path[2] == L'\\' && !Path[3]);
}

bool PathPrefix(const wchar_t *Path)
{
/*
	\\?\
	\\.\
	\??\
*/
	return Path && Path[0] == L'\\' && (Path[1] == L'\\' || Path[1] == L'?') && (Path[2] == L'?' || Path[2] == L'.') && Path[3] == L'\\';
}

BOOL IsLocalPrefixPath(const wchar_t *Path)
{
	return PathPrefix(Path) && IsAlpha(Path[4]) && Path[5] == L':' && Path[6] == L'\\';
}

BOOL IsLocalVolumePath(const wchar_t *Path)
{
	return PathPrefix(Path) && !wcsnicmp(&Path[4],L"Volume{",7) && Path[47] == L'}';
}

BOOL IsLocalVolumeRootPath(const wchar_t *Path)
{
	return IsLocalVolumePath(Path) && !Path[48];
}

// ������������� �������������� ������ ����.
// CheckFullPath ������������ � FCTL_SET[ANOTHER]PANELDIR

string& PrepareDiskPath(string &strPath,BOOL CheckFullPath)
{
	if( !strPath.IsEmpty() )
	{
		if(((IsAlpha(strPath.At(0)) && strPath.At(1)==L':') || (strPath.At(0)==L'\\' && strPath.At(1)==L'\\')))
		{
			if(CheckFullPath)
			{
				wchar_t *lpwszPath=strPath.GetBuffer(),*Src=lpwszPath,*Dst=lpwszPath;
				if(IsLocalPath(lpwszPath))
				{
					Src+=3;
					Dst+=3;
				}
				for(wchar_t c=*Src;c;Src++,c=*Src)
				{
					if (!c||c==L'\\'||c==L'/')
					{
						*Src=0;
						FAR_FIND_DATA_EX fd;
						BOOL find=apiGetFindDataEx(lpwszPath,&fd,false);
						*Src=c;
						if(find)
						{
              size_t n=fd.strFileName.GetLength();
              size_t n1 = n-(Src-Dst);
              if((int)n1>0)
							{
                size_t dSrc=Src-lpwszPath,dDst=Dst-lpwszPath;
								strPath.ReleaseBuffer();
                lpwszPath=strPath.GetBuffer(int(strPath.GetLength()+n1));
								Src=lpwszPath+dSrc;
								Dst=lpwszPath+dDst;
								wmemmove(Src+n1,Src,StrLength(Src)+1);
								Src+=n1;
							}
							wcsncpy(Dst,fd.strFileName,n);
							Dst+=n;
							wcscpy(Dst,Src);
							Dst++;
							Src=Dst;
            } //n n1 dSrc
						else
						{
							Src++;
							Dst=Src;
						}
					}
				}
				strPath.ReleaseBuffer();
			}

			wchar_t *lpwszPath = strPath.GetBuffer ();

			if (lpwszPath[0]==L'\\' && lpwszPath[1]==L'\\')
			{
				if(IsLocalPrefixPath(lpwszPath))
					lpwszPath[4] = Upper(lpwszPath[4]);
				else
				{
					wchar_t *ptr=&lpwszPath[2];
					while (*ptr && *ptr!=L'\\')
					{
						*ptr=Upper(*ptr);
						ptr++;
					}
				}
			}
			else
				lpwszPath[0]=Upper(lpwszPath[0]);

			strPath.ReleaseBuffer ();
		}
	}
	return strPath;
}

/*
   �������� ���� ��� ����-����� �� �������������
   ���� ���� �������� ���� (IsHostFile=FALSE), �� �����
   ����������� ������� ����� ��������� ����. ��������� �������
   ������������ � ���������� TestPath.

   Return: 0 - ����.
           1 - ���!,
          -1 - ����� ��� ���, �� ProcessPluginEvent ������ TRUE
   TestPath ����� ���� ������, ����� ������ �������� ProcessPluginEvent()

*/

int CheckShortcutFolder(string *pTestPath,int IsHostFile, BOOL Silent)
{
  if( pTestPath && !pTestPath->IsEmpty() && GetFileAttributesW(*pTestPath) == INVALID_FILE_ATTRIBUTES)
  {
    int FoundPath=0;

    string strTarget = *pTestPath;

    TruncPathStr(strTarget, ScrX-16);

    if(IsHostFile)
    {
      SetLastError(ERROR_FILE_NOT_FOUND);
      if(!Silent)
        Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG (MError), strTarget, MSG (MOk));
    }
    else // ������� �����!
    {
      SetLastError(ERROR_PATH_NOT_FOUND);
      if(Silent || Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG (MError), strTarget, MSG (MNeedNearPath), MSG(MHYes),MSG(MHNo)) == 0)
      {
        string strTestPathTemp = *pTestPath;

        while ( true )
        {
					if (!CutToSlash(strTestPathTemp,true))
						break;

					if(GetFileAttributesW(strTestPathTemp) != INVALID_FILE_ATTRIBUTES)
					{
						int ChkFld=CheckFolder(strTestPathTemp);
						if(ChkFld > CHKFLD_ERROR && ChkFld < CHKFLD_NOTFOUND)
						{
							if(!(pTestPath->At(0) == L'\\' && pTestPath->At(1) == L'\\' && strTestPathTemp.At(1) == 0))
							{
								*pTestPath = strTestPathTemp;

								if( pTestPath->GetLength() == 2) // ��� ������ "C:", ����� ������� � ������� ������� ����� C:
									AddEndSlash(*pTestPath);
								FoundPath=1;
							}
							break;
						}
					}
        }
      }
    }
    if(!FoundPath)
      return 0;
  }
  if(CtrlObject->Cp()->ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL))
    return -1;
  return 1;
}


BOOL IsDiskInDrive(const wchar_t *Root)
{
  string strVolName;
  string strDrive;
  DWORD  MaxComSize;
  DWORD  Flags;
  string strFS;

  strDrive = Root;

  AddEndSlash(strDrive);
  //UINT ErrMode = SetErrorMode ( SEM_FAILCRITICALERRORS );
  //���� �� ������� SetErrorMode - �������� ����������� ������ "Drive Not Ready"
  BOOL Res = apiGetVolumeInformation (strDrive, &strVolName, NULL, &MaxComSize, &Flags, &strFS);
  //SetErrorMode(ErrMode);
  return Res;
}


void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir)
{
  if(!SrcPanel)
    SrcPanel=CtrlObject->Cp()->ActivePanel;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  switch ( SrcPanel->GetType() ) {
    case QVIEW_PANEL:
    case INFO_PANEL:
      SrcPanel=CtrlObject->Cp()->GetAnotherPanel(AnotherPanel=SrcPanel);
  }

  int AnotherType=AnotherPanel->GetType();

  if (AnotherType!=QVIEW_PANEL && AnotherType!=INFO_PANEL)
  {
    if(NeedSetUpADir)
    {
      string strCurDir;
      SrcPanel->GetCurDir(strCurDir);
      AnotherPanel->SetCurDir(strCurDir,TRUE);
      AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    }
    else
    {
      // TODO: ???
      //if(AnotherPanel->NeedUpdatePanel(SrcPanel))
      //  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
      //else
      {
        // ������� ����� ���������� ������. ���� ��� ���� ����������� - ��������� ����.
        if (AnotherType==FILE_PANEL)
          ((FileList *)AnotherPanel)->ResetLastUpdateTime();
        AnotherPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
      }
    }
  }
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  if (AnotherType==QVIEW_PANEL)
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  CtrlObject->Cp()->Redraw();
}

int CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName)
{
  if(!SrcPanel)
    SrcPanel=CtrlObject->Cp()->ActivePanel;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  AnotherPanel->CloseFile();
  if(AnotherPanel->GetMode() == NORMAL_PANEL)
  {
    string strAnotherCurDir;
    string strFullName;

    AnotherPanel->GetCurDir(strAnotherCurDir);
    AddEndSlash(strAnotherCurDir);

    ConvertNameToFull(SelName, strFullName);
    AddEndSlash(strFullName);

    if(wcsstr(strAnotherCurDir,strFullName))
    {
      ((FileList*)AnotherPanel)->CloseChangeNotification();
      return TRUE;
    }
  }
  return FALSE;
}

/* $ 21.09.2003 KM
   ������������� ������ �� ��������� ����.
*/
void Transform(string &strBuffer,const wchar_t *ConvStr,wchar_t TransformType)
{
  string strTemp, strTemp2;
  wchar_t Hex[16], *stop;

  switch(TransformType)
  {
    case L'X': // Convert common string to hexadecimal string representation
    {
      while(*ConvStr)
      {
        swprintf(Hex,L"%02X",*ConvStr);
        strTemp += (const wchar_t*)Hex;
        ConvStr++;
      }
      break;
    }
    case L'S': // Convert hexadecimal string representation to common string
    {
      const wchar_t *ptrConvStr=ConvStr;
      Hex[2]=0;
      while(*ptrConvStr)
      {
        if(*ptrConvStr != L' ')
        {
          Hex[0]=ptrConvStr[0];
          Hex[1]=ptrConvStr[1];
          DWORD value=wcstoul(Hex,&stop,16);
          Hex[0]=static_cast<wchar_t>(value);
          Hex[1]=0;
          strTemp += (const wchar_t*)Hex;
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

void TransformA(unsigned char *Buffer,int &BufLen,const char *ConvStr,char TransformType)
{
  int I,J,L,N;
  char *stop,HexNum[3];

  switch(TransformType)
  {
    case 'X': // Convert common string to hexadecimal string representation
    {
      *(char *)Buffer=0;
      L=(int)strlen(ConvStr);
      N=Min((BufLen-1)/2,L);
      for (I=0,J=0;I<N;I++,J+=2)
      {
        // "%02X" - ��� ��������� ������� �� ������ ���� ��������
        sprintf((char *)Buffer+J,"%02X",ConvStr[I]);
        BufLen=J+1;
      }

      RemoveTrailingSpacesA((char *)Buffer);
      break;
    }
    case 'S': // Convert hexadecimal string representation to common string
    {
      *(char *)Buffer=0;

      L=(int)strlen(ConvStr);
      char *NewStr=new char[L+1];
      if (NewStr==NULL)
        return;

      // ���������� ��������� ������
      memset(NewStr,0,L+1);

      // ��������� hex-������: ������� ������� ����� �������.
      for (I=0,J=0;ConvStr[I];++I)
      {
        if (ConvStr[I]==' ')
          continue;
        NewStr[J]=ConvStr[I];
        ++J;
      }

      L=(int)strlen(NewStr);
      N=Min(BufLen-1,L);
      for (I=0,J=0;I<N;I+=2,J++)
      {
        // "HH" - ��� �������� ������� �� ������ ���� ���������
        xstrncpy(HexNum,&NewStr[I],2);
        HexNum[2]=0;
        unsigned long value=strtoul(HexNum,&stop,16);
        Buffer[J]=static_cast<unsigned char>(value);
        BufLen=J+1;
      }
      Buffer[J]=0;

      delete []NewStr;
      break;
    }
    default:
      break;
  }
}

/*
 ���������� PipeFound
*/
int PartCmdLine(const wchar_t *CmdStr, string &strNewCmdStr, string &strNewCmdPar)
{
  int PipeFound = FALSE;
  int QuoteFound = FALSE;

  apiExpandEnvironmentStrings (CmdStr, strNewCmdStr);
  RemoveExternalSpaces(strNewCmdStr);

  wchar_t *NewCmdStr = strNewCmdStr.GetBuffer();
  wchar_t *CmdPtr = NewCmdStr;
  wchar_t *ParPtr = NULL;

  // �������� ���������� ������� ��� ���������� � ���������.
  // ��� ���� ������ ��������� ������� �������� ��������������� �������
  // �������� � ������ �������. �.�. ���� � �������� - �� ����.

  while (*CmdPtr)
  {
    if (*CmdPtr == L'"')
      QuoteFound = !QuoteFound;
    if (!QuoteFound && CmdPtr != NewCmdStr)
    {
      if (*CmdPtr == L'>' || *CmdPtr == L'<' ||
          *CmdPtr == L'|' || *CmdPtr == L' ' ||
          *CmdPtr == L'/' ||      // ������� "far.exe/?"
          (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && *CmdPtr == L'&') // ��� ��/2� ���������� ����������� ������
         )
      {
        if (!ParPtr)
          ParPtr = CmdPtr;
        if (*CmdPtr != L' ' && *CmdPtr != L'/')
          PipeFound = TRUE;
      }
    }

    if (ParPtr && PipeFound)
    // ��� ������ ������ �� ���� ��������
      break;
    CmdPtr++;
  }

  if (ParPtr) // �� ����� ��������� � �������� ��� �� ������
  {
    if (*ParPtr == L' ') //AY: ������ ������ ����� �������� � ����������� �� �����,
      *(ParPtr++)=0;     //    �� ����������� ������ � Execute.

    strNewCmdPar = ParPtr;
    *ParPtr = 0;
  }

  strNewCmdStr.ReleaseBuffer ();

  Unquote(strNewCmdStr);

  return PipeFound;
}


BOOL ProcessOSAliases(string &strStr)
{
  if(WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT)
    return FALSE;

  if(!ifn.pfnGetConsoleAlias)
    return FALSE;

  string strNewCmdStr;
  string strNewCmdPar;

  PartCmdLine(strStr,strNewCmdStr,strNewCmdPar);

  string strModuleName;
  apiGetModuleFileName(NULL,strModuleName);
  const wchar_t* lpwszExeName=PointToName(strModuleName);
  int nSize=(int)strNewCmdStr.GetLength()+4096;
  wchar_t* lpwszNewCmdStr=strNewCmdStr.GetBuffer(nSize);
  int ret=ifn.pfnGetConsoleAlias(lpwszNewCmdStr,lpwszNewCmdStr,nSize*sizeof(wchar_t),(wchar_t*)lpwszExeName);
  if(!ret)
  {
    if(apiExpandEnvironmentStrings(L"%COMSPEC%",strModuleName))
    {
      lpwszExeName=PointToName(strModuleName);
      ret=ifn.pfnGetConsoleAlias(lpwszNewCmdStr,lpwszNewCmdStr,nSize*sizeof(wchar_t),(wchar_t*)lpwszExeName);
    }
  }
  strNewCmdStr.ReleaseBuffer();
  if(!ret)
    return FALSE;
  if(!ReplaceStrings(strNewCmdStr,L"$*",strNewCmdPar))
    strNewCmdStr+=L" "+strNewCmdPar;
  strStr=strNewCmdStr;
  return TRUE;
}


int _MakePath1(DWORD Key, string &strPathName, const wchar_t *Param2,int ShortNameAsIs)
{
  int RetCode=FALSE;
  int NeedRealName=FALSE;

  strPathName = L"";
  switch(Key)
  {
    case KEY_CTRLALTBRACKET:       // �������� ������� (UNC) ���� �� ����� ������
    case KEY_CTRLALTBACKBRACKET:   // �������� ������� (UNC) ���� �� ������ ������
    case KEY_ALTSHIFTBRACKET:      // �������� ������� (UNC) ���� �� �������� ������
    case KEY_ALTSHIFTBACKBRACKET:  // �������� ������� (UNC) ���� �� ��������� ������
      NeedRealName=TRUE;
    case KEY_CTRLBRACKET:          // �������� ���� �� ����� ������
    case KEY_CTRLBACKBRACKET:      // �������� ���� �� ������ ������
    case KEY_CTRLSHIFTBRACKET:     // �������� ���� �� �������� ������
    case KEY_CTRLSHIFTBACKBRACKET: // �������� ���� �� ��������� ������

    case KEY_CTRLSHIFTNUMENTER:       // ������� ���� � ����.������
    case KEY_SHIFTNUMENTER:           // ������� ���� � �����.������
    case KEY_CTRLSHIFTENTER:       // ������� ���� � ����.������
    case KEY_SHIFTENTER:           // ������� ���� � �����.������
    {
      Panel *SrcPanel=NULL;
      FilePanels *Cp=CtrlObject->Cp();
      switch(Key)
      {
        case KEY_CTRLALTBRACKET:
        case KEY_CTRLBRACKET:
          SrcPanel=Cp->LeftPanel;
          break;
        case KEY_CTRLALTBACKBRACKET:
        case KEY_CTRLBACKBRACKET:
          SrcPanel=Cp->RightPanel;
          break;
        case KEY_SHIFTNUMENTER:
        case KEY_SHIFTENTER:
        case KEY_ALTSHIFTBRACKET:
        case KEY_CTRLSHIFTBRACKET:
          SrcPanel=Cp->ActivePanel;
          break;
        case KEY_CTRLSHIFTNUMENTER:
        case KEY_CTRLSHIFTENTER:
        case KEY_ALTSHIFTBACKBRACKET:
        case KEY_CTRLSHIFTBACKBRACKET:
          SrcPanel=Cp->GetAnotherPanel(Cp->ActivePanel);
          break;
      }

      if (SrcPanel!=NULL)
      {
        if(Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER || Key == KEY_SHIFTNUMENTER || Key == KEY_CTRLSHIFTNUMENTER)
        {
          string strShortFileName;
          SrcPanel->GetCurName(strPathName,strShortFileName);
          if(SrcPanel->GetShowShortNamesMode()) // ����� ���������� ���� :-)
            strPathName = strShortFileName;
        }
        else
        {
          /* TODO: ����� ����� ������, ��� � TreeList ���� ���� ���� :-) */
          if (!(SrcPanel->GetType()==FILE_PANEL || SrcPanel->GetType()==TREE_PANEL))
            return(FALSE);

          SrcPanel->GetCurDir(strPathName);
          if (SrcPanel->GetMode()!=PLUGIN_PANEL)
          {
            FileList *SrcFilePanel=(FileList *)SrcPanel;
            SrcFilePanel->GetCurDir(strPathName);

            {
                if(NeedRealName)
                    SrcFilePanel->CreateFullPathName(strPathName, strPathName,FILE_ATTRIBUTE_DIRECTORY, strPathName,TRUE,ShortNameAsIs);
            }


            if (SrcFilePanel->GetShowShortNamesMode() && ShortNameAsIs)
              ConvertNameToShort(strPathName,strPathName);
          }
          else
          {
            FileList *SrcFilePanel=(FileList *)SrcPanel;
            struct OpenPluginInfo Info;

            CtrlObject->Plugins.GetOpenPluginInfo(SrcFilePanel->GetPluginHandle(),&Info);
            FileList::AddPluginPrefix(SrcFilePanel,strPathName);

            strPathName += Info.CurDir;

          }
          AddEndSlash(strPathName);
        }

        if(Opt.QuotedName&QUOTEDNAME_INSERT)
          QuoteSpace(strPathName);

        if ( Param2 )
            strPathName += Param2;

        RetCode=TRUE;
      }
    }
    break;
  }
  return RetCode;
}


string &CurPath2ComputerName(const wchar_t *CurDir, string &strComputerName)
{
  string strNetDir;

  strComputerName=L"";

  if ( CurDir[0]==L'\\' && CurDir[1]==L'\\')
    strNetDir = CurDir;
  else
  {
    /* $ 28.03.2002 KM
       - ������� VC ��
         char *LocalName="A:";
         *LocalName=*CurDir;
         ��� ��� ������ � LocalName ReadOnly.
    */
    wchar_t LocalName[3];

    xwcsncpy (LocalName, CurDir, 2);

    apiWNetGetConnection (LocalName, strNetDir);
  }

  if ( strNetDir.At(0)==L'\\' && strNetDir.At(1) == L'\\')
  {
    strComputerName = (const wchar_t*)strNetDir+2;

    wchar_t *ComputerName = strComputerName.GetBuffer ();

    wchar_t *EndSlash=wcschr (ComputerName, L'\\');

    if (EndSlash==NULL)
    {
      strComputerName.ReleaseBuffer ();
      strComputerName=L"";
    }
    else
    {
      *EndSlash=0;
      strComputerName.ReleaseBuffer ();
    }
  }

  return strComputerName;
}

int CheckDisksProps(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType)
{
  string strSrcRoot, strDestRoot;
  int SrcDriveType, DestDriveType;

  DWORD SrcVolumeNumber=0, DestVolumeNumber=0;
  string strSrcVolumeName, strDestVolumeName;
  string strSrcFileSystemName, strDestFileSystemName;
  DWORD SrcFileSystemFlags, DestFileSystemFlags;
  DWORD SrcMaximumComponentLength, DestMaximumComponentLength;

  strSrcRoot=SrcPath;
  strDestRoot=DestPath;
  ConvertNameToUNC(strSrcRoot);
  ConvertNameToUNC(strDestRoot);
  GetPathRoot(strSrcRoot,strSrcRoot);
  GetPathRoot(strDestRoot,strDestRoot);

  SrcDriveType=FAR_GetDriveType(strSrcRoot,NULL,TRUE);
  DestDriveType=FAR_GetDriveType(strDestRoot,NULL,TRUE);

  if (!apiGetVolumeInformation(strSrcRoot,&strSrcVolumeName,&SrcVolumeNumber,&SrcMaximumComponentLength,&SrcFileSystemFlags,&strSrcFileSystemName))
    return(FALSE);

  if (!apiGetVolumeInformation(strDestRoot,&strDestVolumeName,&DestVolumeNumber,&DestMaximumComponentLength,&DestFileSystemFlags,&strDestFileSystemName))
    return(FALSE);

  if(CheckedType == CHECKEDPROPS_ISSAMEDISK)
  {
    if (wcspbrk(DestPath,L"\\:")==NULL)
      return TRUE;

    if (((strSrcRoot.At(0)==L'\\' && strSrcRoot.At(1)==L'\\') || (strDestRoot.At(0)==L'\\' && strDestRoot.At(1)==L'\\')) &&
        StrCmpI(strSrcRoot,strDestRoot)!=0)
      return FALSE;

    if ( *SrcPath == 0 || *DestPath == 0 || (SrcPath[1]!=L':' && DestPath[1]!=L':')) //????
      return TRUE;

    if (Upper(strDestRoot.At(0))==Upper(strSrcRoot.At(0)))
        return TRUE;

    unsigned __int64 SrcTotalSize,SrcTotalFree,SrcUserFree;
    unsigned __int64 DestTotalSize,DestTotalFree,DestUserFree;

    if (!GetDiskSize(strSrcRoot,&SrcTotalSize,&SrcTotalFree,&SrcUserFree))
      return FALSE;
    if (!GetDiskSize(strDestRoot,&DestTotalSize,&DestTotalFree,&DestUserFree))
      return FALSE;

    if (!(SrcVolumeNumber!=0 &&
        SrcVolumeNumber==DestVolumeNumber &&
        StrCmpI(strSrcVolumeName, strDestVolumeName)==0 &&
        SrcTotalSize==DestTotalSize))
      return FALSE;
  }

  else if(CheckedType == CHECKEDPROPS_ISDST_ENCRYPTION)
  {
    if(!(DestFileSystemFlags&FILE_SUPPORTS_ENCRYPTION))
      return FALSE;
    if(!(DestDriveType==DRIVE_REMOVABLE || DestDriveType==DRIVE_FIXED || DestDriveType==DRIVE_REMOTE))
      return FALSE;
  }

  return TRUE;
}

int GetFileFormat (FILE *file, bool *pSignatureFound)
{
	DWORD dwTemp;
	bool bSignatureFound = false;
	int nCodePage = CP_OEMCP;

	if ( fread (&dwTemp, 4, 1, file) == 1 )
	{
		if ( LOWORD (dwTemp) == 0xFEFF )
		{
			nCodePage = CP_UNICODE;
			fseek (file, 2, SEEK_SET);
			bSignatureFound = true;
		}
		else

		if ( LOWORD (dwTemp) == 0xFFFE )
		{
			nCodePage = CP_REVERSEBOM;
			fseek (file, 2, SEEK_SET);
			bSignatureFound = true;
		}
		else

		if ( (dwTemp & 0x00FFFFFF) == 0xBFBBEF )
		{
			nCodePage = CP_UTF8;
			fseek (file, 3, SEEK_SET);
			bSignatureFound = true;
		}
		else
			fseek (file, 0, SEEK_SET);
	}
	else
		fseek (file, 0, SEEK_SET);

	if ( pSignatureFound )
		*pSignatureFound = bSignatureFound;

	return nCodePage;
}

__int64 FileTimeDifference(const FILETIME *a, const FILETIME* b)
{
	LARGE_INTEGER A, B;

	A.u.LowPart  = a->dwLowDateTime;
	A.u.HighPart = a->dwHighDateTime;
	B.u.LowPart  = b->dwLowDateTime;
	B.u.HighPart = b->dwHighDateTime;

	return A.QuadPart - B.QuadPart;
}

unsigned __int64 FileTimeToUI64(const FILETIME *ft)
{
	ULARGE_INTEGER A;

	A.u.LowPart  = ft->dwLowDateTime;
	A.u.HighPart = ft->dwHighDateTime;

	return A.QuadPart;
}

void ConvertNameToUNC(string &strFileName)
{
	ConvertNameToFull(strFileName,strFileName);
	// ��������� �� ��� �������� �������
	string strFileSystemName;
	string strTemp;
	GetPathRoot(strFileName,strTemp);

	if(!apiGetVolumeInformation (strTemp,NULL,NULL,NULL,NULL,&strFileSystemName))
		strFileSystemName=L"";

	DWORD uniSize = 1024;
	UNIVERSAL_NAME_INFOW *uni=(UNIVERSAL_NAME_INFOW*)xf_malloc(uniSize);

	// ��������� WNetGetUniversalName ��� ���� ������, ������ �� ��� Novell`�
	if (StrCmpI(strFileSystemName,L"NWFS"))
	{
		DWORD dwRet=WNetGetUniversalNameW(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize);
		switch(dwRet)
		{
		case NO_ERROR:
			strFileName = uni->lpUniversalName;
			break;
		case ERROR_MORE_DATA:
			uni=(UNIVERSAL_NAME_INFOW*)xf_realloc(uni,uniSize);
			if(WNetGetUniversalNameW(strFileName,UNIVERSAL_NAME_INFO_LEVEL,uni,&uniSize)==NO_ERROR)
				strFileName = uni->lpUniversalName;
			break;
		}
	}
	else if(strFileName.At(1) == L':')
	{
		// BugZ#449 - �������� ������ CtrlAltF � ��������� Novell DS
		// �����, ���� �� ���������� �������� UniversalName � ���� ���
		// ��������� ���� - �������� ��� ��� ���� ������ ������

		if(!DriveLocalToRemoteName(DRIVE_UNKNOWN,strFileName.At(0),strTemp).IsEmpty())
		{
			const wchar_t *NamePtr;
			if((NamePtr=wcschr(strFileName, L'/')) == NULL)
				NamePtr=wcschr(strFileName, L'\\');
			if(NamePtr != NULL)
			{
				AddEndSlash(strTemp);
				strTemp += &NamePtr[1];
			}
			strFileName = strTemp;
		}
	}
	xf_free(uni);
	ConvertNameToReal(strFileName,strFileName);
}
