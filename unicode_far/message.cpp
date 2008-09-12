/*
message.cpp

����� MessageBox
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

#include "fn.hpp"
#include "global.hpp"
#include "ctrlobj.hpp"
#include "plugin.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "dialog.hpp"
#include "scrbuf.hpp"
#include "keys.hpp"

static int MessageX1,MessageY1,MessageX2,MessageY2;
static string strMsgHelpTopic;
static int FirstButtonIndex,LastButtonIndex;
static BOOL IsWarningStyle;



int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            INT_PTR PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,NULL,NULL,NULL,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL,PluginNumber));
}

int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            INT_PTR PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,
                 NULL,NULL,NULL,NULL,NULL,NULL,NULL,PluginNumber));
}


int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9,const wchar_t *Str10,
            INT_PTR PluginNumber)
{
  return(Message(Flags,Buttons,Title,Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8,
                 Str9,Str10,NULL,NULL,NULL,NULL,PluginNumber));
}

int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9,const wchar_t *Str10,
            const wchar_t *Str11,const wchar_t *Str12,const wchar_t *Str13,
            const wchar_t *Str14,INT_PTR PluginNumber)
{
  int StrCount;
  const wchar_t *Str[14];

  Str[0]=Str1;   Str[1]=Str2;   Str[2]=Str3;   Str[3]=Str4;
  Str[4]=Str5;   Str[5]=Str6;   Str[6]=Str7;   Str[7]=Str8;
  Str[8]=Str9;   Str[9]=Str10;  Str[10]=Str11; Str[11]=Str12;
  Str[12]=Str13; Str[13]=Str14;

  StrCount=0;
  while (StrCount<(int)countof(Str) && Str[StrCount]!=NULL)
    StrCount++;

  return Message(Flags,Buttons,Title,Str,StrCount,PluginNumber);
}

LONG_PTR WINAPI MsgDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	switch(Msg)
	{
		case DN_CTLCOLORDLGITEM:
			{
				FarDialogItem *di=(FarDialogItem *)Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,Param1,0);
				int Type=di->Type;
				Dialog::SendDlgMessage(hDlg,DM_FREEDLGITEM,0,(LONG_PTR)di);
				if(Type==DI_EDIT)
				{
					int Color=FarColorToReal(IsWarningStyle?COL_WARNDIALOGTEXT:COL_DIALOGTEXT)&0xFF;
					return ((Param2&0xFF00FF00)|(Color<<16)|Color);
				}
			}
			break;
		case DN_KEY:
			{
				if(Param1==FirstButtonIndex && (Param2==KEY_LEFT || Param2 == KEY_NUMPAD4 || Param2==KEY_SHIFTTAB))
				{
					Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,LastButtonIndex,0);
					return TRUE;
				}
				else if(Param1==LastButtonIndex && (Param2==KEY_RIGHT || Param2 == KEY_NUMPAD6 || Param2==KEY_TAB))
				{
					Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,FirstButtonIndex,0);
					return TRUE;
				}
			}
			break;
	}
	return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int Message(
        DWORD Flags,
        int Buttons,
        const wchar_t *Title,
        const wchar_t * const *Items,
        int ItemsNumber,
        INT_PTR PluginNumber
        )
{
  string strTempStr;
  int X1,Y1,X2,Y2;
  int Length, BtnLength, J;
  DWORD I, MaxLength, StrCount;
  BOOL ErrorSets=FALSE;
  const wchar_t **Str;
  wchar_t *PtrStr;
  const wchar_t *CPtrStr;

  string strErrStr;

  if (Flags & MSG_ERRORTYPE)
    ErrorSets = GetErrorString (strErrStr);

  // ������� ������ ��� ������� ������ ���������� �� ������ (+����� 16)
  Str=(const wchar_t **)xf_malloc((ItemsNumber+ADDSPACEFORPSTRFORMESSAGE) * sizeof(wchar_t*));

  if ( !Str )
    return -1;

  StrCount=ItemsNumber-Buttons;

  // ��������������� ������ ������������� �������.
  for (BtnLength=0,I=0;I<static_cast<DWORD>(Buttons);I++) //??
    BtnLength+=HiStrlen(Items[I+StrCount])+2;

  for (MaxLength=BtnLength,I=0;I<StrCount;I++)
  {
    if (static_cast<DWORD>(Length=StrLength(Items[I]))>MaxLength)
      MaxLength=Length;
  }

  // ����� ��� �� ������ ���������
  if(Title && *Title)
  {
    I=(DWORD)StrLength(Title)+2;
    if (MaxLength < I)
      MaxLength=I;
  }

  #define MAX_WIDTH_MESSAGE static_cast<DWORD>(ScrX-13)

  // ����� ��������� ������������� �������
  if (MaxLength > MAX_WIDTH_MESSAGE)
    MaxLength=MAX_WIDTH_MESSAGE;

  // ������ ���������� MSG_ERRORTYPE
  DWORD CountErrorLine=0;

  if ((Flags & MSG_ERRORTYPE) && ErrorSets)
  {
    // ������� ���������� ����� �� ��������� ����������
    ++CountErrorLine;
    //InsertQuote(ErrStr); // �������

    // ���������� "���������" �������
    DWORD LenErrStr=(DWORD)StrLength(strErrStr);
    if(LenErrStr > MAX_WIDTH_MESSAGE)
    {
      // �������� ������?
      if(LenErrStr/2 < MAX_WIDTH_MESSAGE)
      {
        // � �������� + 1/3?
        if((LenErrStr+LenErrStr/3)/2 < MAX_WIDTH_MESSAGE)
          LenErrStr=(LenErrStr+LenErrStr/3)/2;
        else
          LenErrStr/=2;
      }
      else
        LenErrStr=MAX_WIDTH_MESSAGE;
    }
    else if(LenErrStr < MaxLength)
      LenErrStr=MaxLength;

    if(MaxLength > LenErrStr && MaxLength >= MAX_WIDTH_MESSAGE)
      MaxLength=LenErrStr;

    if(MaxLength < LenErrStr && LenErrStr <= MAX_WIDTH_MESSAGE)
      MaxLength=LenErrStr;

    // � ������ ���������
    //PtrStr=FarFormatText(ErrStr,MaxLength-(MaxLength > MAX_WIDTH_MESSAGE/2?1:0),ErrStr,sizeof(ErrStr),"\n",0); //?? MaxLength ??
    FarFormatText(strErrStr,LenErrStr,strErrStr,L"\n",0); //?? MaxLength ??

    PtrStr = strErrStr.GetBuffer ();

    while((PtrStr=wcschr(PtrStr,L'\n')) != NULL)
    {
      *PtrStr++=0;
      if(*PtrStr)
        CountErrorLine++;
    }

    strErrStr.ReleaseBuffer ();

    if(CountErrorLine > ADDSPACEFORPSTRFORMESSAGE)
      CountErrorLine=ADDSPACEFORPSTRFORMESSAGE; //??
  }

  // ��������� ������...
  CPtrStr=strErrStr;
  for (I=0; I < CountErrorLine;I++)
  {
    Str[I]=CPtrStr;
    CPtrStr+=StrLength(CPtrStr)+1;
    if(!*CPtrStr) // ��� ������ ������ ���� - "������" �����
    {
      ++I;
      break;
    }
  }

  for (J=0; J < ItemsNumber; ++J, ++I)
  {
    Str[I]=Items[J];
  }


  StrCount+=CountErrorLine;

  MessageX1=X1=(ScrX-MaxLength)/2-4;
  MessageX2=X2=X1+MaxLength+9;
  Y1=(ScrY-StrCount)/2-2;
  if(Y1 < 0)
    Y1=0;
  MessageY1=Y1;

  if (Flags & MSG_DOWN)
  {
    int NewY=ScrY/2-4;
    if (static_cast<int>(Y1+StrCount+3)<ScrY && NewY>Y1+2)
      Y1=NewY;
    else
      Y1+=2;
    MessageY1=Y1;
  }
  MessageY2=Y2=Y1+StrCount+3;

  string strHelpTopic;

  strHelpTopic = strMsgHelpTopic;
  strMsgHelpTopic=L"";

  // *** ������� � �������� ***

  if (Buttons>0)
  {
    DWORD ItemCount;
    struct DialogItemEx *PtrMsgDlg;
    struct DialogItemEx *MsgDlg = new DialogItemEx[ItemCount=StrCount+Buttons+1];

    if(!MsgDlg)
    {
      xf_free(Str);
      return -1;
    }

    for (DWORD i=0; i<ItemCount; i++)
      MsgDlg[i].Clear();

    int RetCode;
    MessageY2=++Y2;

    MsgDlg[0].Type=DI_DOUBLEBOX;
    MsgDlg[0].X1=3;
    MsgDlg[0].Y1=1;
    MsgDlg[0].X2=X2-X1-3;
    MsgDlg[0].Y2=Y2-Y1-1;

    if(Title && *Title)
        MsgDlg[0].strData = Title;

    int TypeItem=DI_TEXT;
    DWORD FlagsItem=DIF_SHOWAMPERSAND;
    BOOL IsButton=FALSE;
    int CurItem=0;
    for(PtrMsgDlg=MsgDlg+1,I=1; I < ItemCount; ++I, ++PtrMsgDlg, ++CurItem)
    {
      if(I==StrCount+1)
      {
        PtrMsgDlg->Focus=1;
        PtrMsgDlg->DefaultButton=1;
        TypeItem=DI_BUTTON;
        FlagsItem=DIF_CENTERGROUP|DIF_NOBRACKETS;
        IsButton=TRUE;
        FirstButtonIndex=CurItem+1;
        LastButtonIndex=CurItem;
      }

      PtrMsgDlg->Type=TypeItem;
      PtrMsgDlg->Flags=FlagsItem;
      CPtrStr=Str[CurItem];
      if(IsButton)
      {
        PtrMsgDlg->Y1=Y2-Y1-2;
        PtrMsgDlg->strData = string(L" ")+string(CPtrStr)+string(L" "); //BUGBUG!!!
        LastButtonIndex++;
      }
      else
      {
        PtrMsgDlg->X1=(Flags & MSG_LEFTALIGN)?5:-1;
        PtrMsgDlg->Y1=I+1;
        wchar_t Chr=*CPtrStr;
        if(Chr == 1 || Chr == 2)
        {
          CPtrStr++;
          PtrMsgDlg->Flags|=DIF_BOXCOLOR|(Chr==2?DIF_SEPARATOR2:DIF_SEPARATOR);
        }
        else if(StrLength(CPtrStr)>X2-X1-9)
        {
          PtrMsgDlg->Type=DI_EDIT;
          PtrMsgDlg->Flags|=DIF_READONLY|DIF_BTNNOCLOSE|DIF_SELECTONENTRY;
          PtrMsgDlg->X1=5;
          PtrMsgDlg->X2=X2-X1-5;
          PtrMsgDlg->strData=CPtrStr;
          continue;
        }
        //xstrncpy(PtrMsgDlg->Data,CPtrStr,Min((int)MAX_WIDTH_MESSAGE,(int)sizeof(PtrMsgDlg->Data))); //?? ScrX-15 ??
        PtrMsgDlg->strData = CPtrStr; //BUGBUG, wrong len
      }
    }

    {
      IsWarningStyle=Flags&MSG_WARNING;
      Dialog Dlg(MsgDlg,ItemCount,MsgDlgProc);
      Dlg.SetPosition(X1,Y1,X2,Y2);
      if ( !strHelpTopic.IsEmpty() )
        Dlg.SetHelp(strHelpTopic);
      Dlg.SetPluginNumber(PluginNumber); // �������� ����� �������
      if (Flags & MSG_WARNING)
        Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
      Dlg.SetDialogMode(DMODE_MSGINTERNAL);
      FlushInputBuffer();
      if(Flags & MSG_KILLSAVESCREEN)
        Dialog::SendDlgMessage((HANDLE)&Dlg,DM_KILLSAVESCREEN,0,0);
      Dlg.Process();
      RetCode=Dlg.GetExitCode();
    }
    delete [] MsgDlg;
    xf_free(Str);
    return(RetCode<0?RetCode:RetCode-StrCount-1);
  }


  // *** ��� �������! ***
  SetCursorType(0,0);

  if (!(Flags & MSG_KEEPBACKGROUND))
  {
    SetScreen(X1,Y1,X2,Y2,L' ',(Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
    MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
    MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
    Box(X1+3,Y1+1,X2-3,Y2-1,(Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX,DOUBLE_BOX);
  }

  SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
  if(Title && *Title)
  {
    string strTempTitle = Title;

    if ( strTempTitle.GetLength() > MaxLength )
			strTempTitle.SetLength (MaxLength);

    GotoXY(X1+(X2-X1-1-(int)strTempTitle.GetLength())/2,Y1+1);
    mprintf(L" %s ",(const wchar_t*)strTempTitle);
  }

  for (I=0;I<StrCount;I++)
  {
    int PosX;
    CPtrStr=Str[I];
    wchar_t Chr=*CPtrStr;
    if (Chr == 1 || Chr == 2)
    {
      int Length=X2-X1-5;
      if (Length>1)
      {
        SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGBOX);
        GotoXY(X1+3,Y1+I+2);
        DrawLine(Length,(Chr == 2?3:1));
        CPtrStr++;
        int TextLength=StrLength(CPtrStr);
        if (TextLength<Length)
        {
          GotoXY(X1+3+(Length-TextLength)/2,Y1+I+2);
          Text(CPtrStr);
        }
        SetColor((Flags & MSG_WARNING)?COL_WARNDIALOGBOX:COL_DIALOGTEXT);
      }
      continue;
    }
    if ((Length=StrLength(CPtrStr))>ScrX-15)
      Length=ScrX-15;
    int Width=X2-X1+1;

    wchar_t *lpwszTemp = NULL;

    if (Flags & MSG_LEFTALIGN)
    {
      lpwszTemp = (wchar_t*)xf_malloc ((Width-10+1)*sizeof (wchar_t));

      swprintf(lpwszTemp,L"%.*s",Width-10,CPtrStr);
      GotoXY(X1+5,Y1+I+2);
    }
    else
    {
      PosX=X1+(Width-Length)/2;

      lpwszTemp = (wchar_t*)xf_malloc ((PosX-X1-4+Length+X2-PosX-Length-3+1)*sizeof (wchar_t));
      swprintf(lpwszTemp,L"%*s%.*s%*s",PosX-X1-4,L"",Length,CPtrStr,X2-PosX-Length-3,L"");
      GotoXY(X1+4,Y1+I+2);
    }
    Text(lpwszTemp);

    xf_free (lpwszTemp);
  }
  /* $ 13.01.2003 IS
     - ������������� ������ ������ ��������� ������, ���� ���������� ������
       � ��������� ����� ���� � ������ �������� �����������. ��� ����������,
       ����� ��������� ��������-��� �� �������, ������� ��� ������� ��� ������
       ������� �������� ��������� (bugz#533).
  */
  xf_free(Str);
  if (Buttons==0)
  {
    if (ScrBuf.GetLockCount()>0 && !CtrlObject->Macro.PeekKey())
      ScrBuf.SetLockCount(0);
    ScrBuf.Flush();
  }
  return(0);
}


void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2)
{
  X1=MessageX1;
  Y1=MessageY1;
  X2=MessageX2;
  Y2=MessageY2;
}



int GetErrorString (string &strErrStr)
{
  int I;
  static struct TypeErrMsgs{
    DWORD WinMsg;
    int FarMsg;
  } ErrMsgs[]={
    {ERROR_INVALID_FUNCTION,MErrorInvalidFunction},
    {ERROR_BAD_COMMAND,MErrorBadCommand},
    {ERROR_CALL_NOT_IMPLEMENTED,MErrorBadCommand},
    {ERROR_FILE_NOT_FOUND,MErrorFileNotFound},
    {ERROR_PATH_NOT_FOUND,MErrorPathNotFound},
    {ERROR_TOO_MANY_OPEN_FILES,MErrorTooManyOpenFiles},
    {ERROR_ACCESS_DENIED,MErrorAccessDenied},
    {ERROR_NOT_ENOUGH_MEMORY,MErrorNotEnoughMemory},
    {ERROR_OUTOFMEMORY,MErrorNotEnoughMemory},
    {ERROR_WRITE_PROTECT,MErrorDiskRO},
    {ERROR_NOT_READY,MErrorDeviceNotReady},
    {ERROR_NOT_DOS_DISK,MErrorCannotAccessDisk},
    {ERROR_SECTOR_NOT_FOUND,MErrorSectorNotFound},
    {ERROR_OUT_OF_PAPER,MErrorOutOfPaper},
    {ERROR_WRITE_FAULT,MErrorWrite},
    {ERROR_READ_FAULT,MErrorRead},
    {ERROR_GEN_FAILURE,MErrorDeviceGeneral},
    {ERROR_SHARING_VIOLATION,MErrorFileSharing},
    {ERROR_LOCK_VIOLATION,MErrorFileSharing},
    {ERROR_BAD_NETPATH,MErrorNetworkPathNotFound},
    {ERROR_NETWORK_BUSY,MErrorNetworkBusy},
    {ERROR_NETWORK_ACCESS_DENIED,MErrorNetworkAccessDenied},
    {ERROR_NET_WRITE_FAULT,MErrorNetworkWrite},
    {ERROR_DRIVE_LOCKED,MErrorDiskLocked},
    {ERROR_ALREADY_EXISTS,MErrorFileExists},
    {ERROR_BAD_PATHNAME,MErrorInvalidName},
    {ERROR_INVALID_NAME,MErrorInvalidName},
    {ERROR_DISK_FULL,MErrorInsufficientDiskSpace},
    {ERROR_HANDLE_DISK_FULL,MErrorInsufficientDiskSpace},
    {ERROR_DIR_NOT_EMPTY,MErrorFolderNotEmpty},
    {ERROR_INTERNET_INCORRECT_USER_NAME,MErrorIncorrectUserName},
    {ERROR_INTERNET_INCORRECT_PASSWORD,MErrorIncorrectPassword},
    {ERROR_INTERNET_LOGIN_FAILURE,MErrorLoginFailure},
    {ERROR_INTERNET_CONNECTION_ABORTED,MErrorConnectionAborted},
    {ERROR_CANCELLED,MErrorCancelled},
    {ERROR_NO_NETWORK,MErrorNetAbsent},
    {ERROR_DEVICE_IN_USE,MErrorNetDeviceInUse},
    {ERROR_OPEN_FILES,MErrorNetOpenFiles},
    {ERROR_ALREADY_ASSIGNED,MErrorAlreadyAssigned},
    {ERROR_DEVICE_ALREADY_REMEMBERED,MErrorAlreadyRemebered},
    {ERROR_NOT_LOGGED_ON,MErrorNotLoggedOn},
    {ERROR_INVALID_PASSWORD,MErrorInvalidPassword},

    // "����� ������"
    {ERROR_NO_RECOVERY_POLICY,MErrorNoRecoveryPolicy},
    {ERROR_ENCRYPTION_FAILED,MErrorEncryptionFailed},
    {ERROR_DECRYPTION_FAILED,MErrorDecryptionFailed},
    {ERROR_FILE_NOT_ENCRYPTED,MErrorFileNotEncrypted},
    {ERROR_NO_ASSOCIATION,MErrorNoAssociation},
  };

  DWORD LastError = GetLastError();

  for(I=0; I < (int)countof(ErrMsgs); ++I)
    if(ErrMsgs[I].WinMsg == LastError)
    {
      strErrStr = UMSG(ErrMsgs[I].FarMsg);
      break;
    }

  if(I >= (int)countof(ErrMsgs))
  {
    if ( LastError != ERROR_SUCCESS )
    {
      LPWSTR lpBuffer;
      FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                    NULL,
                    LastError,
                    0,
                    (LPWSTR)&lpBuffer,
                    0,
                    NULL
                    );
      strErrStr=lpBuffer;
      LocalFree(lpBuffer);
      RemoveUnprintableCharacters(strErrStr);
      return TRUE;
    }

    strErrStr = L""; //???
    return FALSE;
  }

  return TRUE;
}


void SetMessageHelp(const wchar_t *Topic)
{
  strMsgHelpTopic = Topic;
}

/* $ 12.03.2002 VVM
  ����� ������� - ������������ ��������� �������� ��������.
  ������� ������.
  ����������:
   FALSE - ���������� ��������
   TRUE  - �������� ��������
*/
int AbortMessage()
{
  int Res = Message(MSG_WARNING|MSG_KILLSAVESCREEN,2,UMSG(MKeyESCWasPressed),
            UMSG((Opt.Confirm.EscTwiceToInterrupt)?MDoYouWantToStopWork2:MDoYouWantToStopWork),
            UMSG(MYes),UMSG(MNo));
  if (Res == -1) // Set "ESC" equal to "NO" button
    Res = 1;
  if ((Opt.Confirm.EscTwiceToInterrupt && Res) ||
      (!Opt.Confirm.EscTwiceToInterrupt && !Res))
    return (TRUE);
  else
    return (FALSE);
}

// �������� �� "��������������" ������������� ��... ��������, �������� ����� � ������� �������!
BOOL CheckErrorForProcessed(DWORD Err)
{
  switch(Err)
  {
    case ERROR_ACCESS_DENIED:
    case ERROR_WRITE_PROTECT:
    case ERROR_NOT_READY:
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
      return FALSE;
  }
  return TRUE;
}
