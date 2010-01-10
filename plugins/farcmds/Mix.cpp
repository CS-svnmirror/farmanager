#ifndef __mix_cpp
#define __mix_cpp

/*
 �����頥� �᫮, ��१�� ��� �� ��ப�, ��� -2 � ��砥 �訡��
 Start, End - ��砫� � ����� ��ப�
*/
int GetInt(TCHAR *Start, TCHAR *End)
{
  int Ret=-2;
  if(End >= Start)
  {
    TCHAR Tmp[11];
    int Size=(int)(End-Start);

    if(Size)
    {
      if(Size < 11)
      {
        _tmemcpy(Tmp,Start,Size);
        Tmp[Size]=0;
        Ret=FarAtoi(Tmp);
      }
    }
    else
      Ret=0;
  }
  return Ret;
}

const TCHAR *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

/*�����頥� TRUE, �᫨ 䠩� name �������*/
BOOL FileExists(const TCHAR *Name)
{
  return GetFileAttributes(Name)!=0xFFFFFFFF;
}

void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
{
  int I;
  struct FarDialogItem *PItem=Item;
  const struct InitDialogItem *PInit=Init;
  for (I=0;I<ItemsNumber;I++,PItem++,PInit++)
  {
    PItem->Type=PInit->Type;
    PItem->X1=PInit->X1;
    PItem->Y1=PInit->Y1;
    PItem->X2=PInit->X2;
    PItem->Y2=PInit->Y2;
    PItem->Focus=PInit->Focus;
    PItem->History=(const TCHAR *)PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
#ifdef UNICODE
    PItem->MaxLen=0;
#endif
    if ((DWORD_PTR)PInit->Data<2000)
#ifndef UNICODE
      lstrcpy(PItem->Data,GetMsg((unsigned int)(DWORD_PTR)PInit->Data));
#else
      PItem->PtrData = GetMsg((unsigned int)(DWORD_PTR)PInit->Data);
#endif
    else
#ifndef UNICODE
      lstrcpy(PItem->Data,PInit->Data);
#else
      PItem->PtrData = PInit->Data;
#endif
  }
}

TCHAR *GetCommaWord(TCHAR *Src,TCHAR *Word,TCHAR Separator)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=FALSE;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src==_T('[') && _tcschr(Src+1,_T(']'))!=NULL)
      SkipBrackets=TRUE;
    if (*Src==_T(']'))
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
int ReplaceStrings(TCHAR *Str,const TCHAR *FindStr,const TCHAR *ReplStr,int Count,BOOL IgnoreCase)
{
  int I=0, J=0, Res;
  int LenReplStr=(int)lstrlen(ReplStr);
  int LenFindStr=(int)lstrlen(FindStr);
  int L=(int)lstrlen(Str);

  while(I <= L-LenFindStr)
  {
    Res=IgnoreCase?_memicmp(Str+I, FindStr, LenFindStr*sizeof(TCHAR)):memcmp(Str+I, FindStr, LenFindStr*sizeof(TCHAR));
    if(Res == 0)
    {
      if(LenReplStr > LenFindStr)
        _tmemmove(Str+I+(LenReplStr-LenFindStr),Str+I,lstrlen(Str+I)+1); // >>
      else if(LenReplStr < LenFindStr)
        _tmemmove(Str+I,Str+I+(LenFindStr-LenReplStr),lstrlen(Str+I+(LenFindStr-LenReplStr))+1); //??
      _tmemcpy(Str+I,ReplStr,LenReplStr);
      I += LenReplStr;
      if(++J == Count && Count > 0)
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
int PartCmdLine(const TCHAR *CmdStr,TCHAR *NewCmdStr,int SizeNewCmdStr,TCHAR *NewCmdPar,int SizeNewCmdPar)
{
  int PipeFound = FALSE;
  int QuoteFound = FALSE;

  ExpandEnvironmentStr(CmdStr, NewCmdStr, SizeNewCmdStr);
  FarTrim(NewCmdStr);

  TCHAR *CmdPtr = NewCmdStr;
  TCHAR *ParPtr = NULL;

  // �������� ᮡ�⢥��� ������� ��� �ᯮ������ � ��ࠬ����.
  // �� �⮬ ������ ��।���� ����稥 ᨬ����� ��८�।������ ��⮪��
  // ����⠥� � ��⮬ ����祪. �.�. ���� � ����窠� - �� ����.

  while (*CmdPtr)
  {
    if (*CmdPtr == _T('"'))
      QuoteFound = !QuoteFound;
    if (!QuoteFound && CmdPtr != NewCmdStr)
    {
      if (*CmdPtr == _T('>') || *CmdPtr == _T('<') ||
          *CmdPtr == _T('|') || *CmdPtr == _T(' ') ||
          *CmdPtr == _T('/') ||      // ��ਠ�� "far.exe/?"
          (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && *CmdPtr == _T('&')) // ��� ��/2� ��ࠡ�⠥� ࠧ����⥫� ������
         )
      {
        if (!ParPtr)
          ParPtr = CmdPtr;
        if (*CmdPtr != _T(' ') && *CmdPtr != _T('/'))
          PipeFound = TRUE;
      }
    }

    if (ParPtr && PipeFound) // ��� ����� ��祣� �� ���� 㧭�����
      break;
    CmdPtr++;
  }

  if (ParPtr) // �� ��諨 ��ࠬ���� � �⤥�塞 ��� �� ��⫥�
  {
    if (*ParPtr == _T(' ')) //AY: ���� �஡�� ����� �������� � ��ࠬ��ࠬ� �� �㦥�,
      *(ParPtr++)=0;     //    �� ���������� ������ � Execute.

    lstrcpyn(NewCmdPar, ParPtr, SizeNewCmdPar-1);
    *ParPtr = 0;
  }

  Unquote(NewCmdStr);
  return PipeFound;
}

BOOL ProcessOSAliases(TCHAR *Str,int SizeStr)
{
  if(WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT)
    return FALSE;

  typedef DWORD (WINAPI *PGETCONSOLEALIAS)(
    TCHAR *lpSource,
    TCHAR *lpTargetBuffer,
    DWORD TargetBufferLength,
    TCHAR *lpExeName
  );

  static PGETCONSOLEALIAS pGetConsoleAlias=NULL;
  if(!pGetConsoleAlias)
  {
  #ifdef UNICODE
    pGetConsoleAlias = (PGETCONSOLEALIAS)GetProcAddress(GetModuleHandleW(_T("kernel32")),"GetConsoleAliasW");
  #else
    pGetConsoleAlias = (PGETCONSOLEALIAS)GetProcAddress(GetModuleHandle(_T("kernel32")),"GetConsoleAliasA");
  #endif // !UNICODE
    if(!pGetConsoleAlias)
      return FALSE;
  }

  TCHAR NewCmdStr[4096];
  TCHAR NewCmdPar[4096];
  *NewCmdStr=0;
  *NewCmdPar=0;

  PartCmdLine(Str,NewCmdStr,ArraySize(NewCmdStr),NewCmdPar,ArraySize(NewCmdPar));

  TCHAR ModuleName[MAX_PATH];
  GetModuleFileName(NULL,ModuleName,ArraySize(ModuleName));
  TCHAR* ExeName=(TCHAR*)PointToName(ModuleName);

  int ret=pGetConsoleAlias(NewCmdStr,NewCmdStr,sizeof(NewCmdStr),ExeName);
  if(!ret)
  {
    if(ExpandEnvironmentStr(_T("%COMSPEC%"),ModuleName,ArraySize(ModuleName)))
    {
      ExeName=(TCHAR*)PointToName(ModuleName);
      ret=pGetConsoleAlias(NewCmdStr,NewCmdStr,sizeof(NewCmdStr),ExeName);
    }
  }
  if(!ret)
  {
    return FALSE;
  }
  if(!ReplaceStrings(NewCmdStr,_T("$*"),NewCmdPar,-1,FALSE))
  {
    lstrcat(NewCmdStr,_T(" "));
    lstrcat(NewCmdStr,NewCmdPar);
  }
  lstrcpyn(Str,NewCmdStr,SizeStr-1);
  return TRUE;
}

#endif
