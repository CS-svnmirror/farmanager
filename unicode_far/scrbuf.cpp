/*
scrbuf.cpp

����������� ������ �� �����, ���� ����� ���� ����� ���� �����
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

#include "scrbuf.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "colors.hpp"
#include "ctrlobj.hpp"

enum{
  SBFLAGS_FLUSHED         = 0x00000001,
  SBFLAGS_FLUSHEDCURPOS   = 0x00000002,
  SBFLAGS_FLUSHEDCURTYPE  = 0x00000004,
  SBFLAGS_USESHADOW       = 0x00000008,
};


//#if defined(SYSLOG_OT)
// #define DIRECT_SCREEN_OUT
//#endif

#ifdef DIRECT_RT
extern int DirectRT;
#endif

extern WCHAR Oem2Unicode[];

ScreenBuf ScrBuf;

ScreenBuf::ScreenBuf()
{
  Buf=Shadow=NULL;
  BufX=BufY=0;
  SBFlags.Set(SBFLAGS_FLUSHED|SBFLAGS_FLUSHEDCURPOS|SBFLAGS_FLUSHEDCURTYPE);
  LockCount=0;
  hScreen=NULL;
}


ScreenBuf::~ScreenBuf()
{
  if(Buf)    delete[] Buf;
  if(Shadow) delete[] Shadow;
}


void ScreenBuf::AllocBuf(int X,int Y)
{
  CriticalSectionLock Lock(CS);

  if (X==BufX && Y==BufY)
    return;
  if(Buf) delete[] Buf;
  if(Shadow) delete[] Shadow;

  unsigned Cnt=X*Y;
  Buf=new CHAR_INFO[Cnt];
  Shadow=new CHAR_INFO[Cnt];
#if !defined(ALLOC)
  /* � ��� ����� ����� ������������: ��� ���������������� ������� �������
     ������ - ������ ��� �������������������� ������ ;-)
     ������� ���� ����� �����������...
  */
  memset(Buf,0,Cnt*sizeof(CHAR_INFO));
  memset(Shadow,0,Cnt*sizeof(CHAR_INFO));
#endif
  BufX=X;
  BufY=Y;
}

/* ���������� ������������ ������ ��������� �� �������.
*/
void ScreenBuf::FillBuf()
{
  CriticalSectionLock Lock(CS);

  COORD Size,Corner;
  SMALL_RECT Coord;
  Size.X=BufX;
  Size.Y=BufY;
  Corner.X=0;
  Corner.Y=0;
  Coord.Left=0;
  Coord.Top=0;
  Coord.Right=BufX-1;
  Coord.Bottom=BufY-1;

  _tran(SysLog(L"BufX*BufY=%i",BufX*BufY));
  if ( BufX*BufY>6000 )
  {
    _tran(SysLog(L"fucked method"));
    CHAR_INFO *ci=(CHAR_INFO*)Buf;
    for ( int y=0; y<BufY; y++ )
    {
        Size.Y=1;
        Coord.Top=y;
        Coord.Bottom=y;
        BOOL r;

        r=ReadConsoleOutputW(hScreen,ci,Size,Corner,&Coord);

        ci+=BufX;
    }
    _tran(SysLog(L"fucked method end"));
  }
  else
  {
    ReadConsoleOutputW(hScreen,Buf,Size,Corner,&Coord);
  }

  memcpy(Shadow,Buf,BufX*BufY*sizeof(CHAR_INFO));
  SBFlags.Set(SBFLAGS_USESHADOW);

  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hScreen,&csbi);
  CurX=csbi.dwCursorPosition.X;
  CurY=csbi.dwCursorPosition.Y;
}

/* �������� Text � ����������� �����
*/
void ScreenBuf::Write(int X,int Y,const CHAR_INFO *Text,int TextLength)
{
  CriticalSectionLock Lock(CS);

  if (X>=BufX || Y>=BufY || TextLength==0)
    return;
  if(X+TextLength >= BufX)
    TextLength=BufX-X; //??
	CHAR_INFO *PtrBuf=Buf+Y*BufX+X;
	for(int i=0;i<TextLength;i++)
	{
		SetVidChar(PtrBuf[i],Text[i].Char.UnicodeChar);
		PtrBuf[i].Attributes=Text[i].Attributes;
	}
  SBFlags.Clear(SBFLAGS_FLUSHED);

#ifdef DIRECT_SCREEN_OUT
  Flush();
#elif defined(DIRECT_RT)
  if ( DirectRT  )
    Flush();
#endif
}


/* ������ ���� �� ������������ ������.
*/
void ScreenBuf::Read(int X1,int Y1,int X2,int Y2,CHAR_INFO *Text,int MaxTextLength)
{
  CriticalSectionLock Lock(CS);

  int Width=X2-X1+1;
  int Height=Y2-Y1+1;
  int I, Idx;

  for (Idx=I=0; I < Height; I++, Idx+=Width)
    memcpy(Text+Idx,Buf+(Y1+I)*BufX+X1,Min((int)sizeof(CHAR_INFO)*Width,(int)MaxTextLength));

  if (X1==0 && Y1==0 &&
      CtrlObject!=NULL &&
      CtrlObject->Macro.IsRecording() &&
      GetVidChar(MacroChar) != L'R')
    Text[0]=MacroChar;
}

/* �������� �������� �������� ��������� � ������������ � ������
   (� �������� ����������� ��� "��������" ����)
*/
void ScreenBuf::ApplyColorMask(int X1,int Y1,int X2,int Y2,WORD ColorMask)
{
  CriticalSectionLock Lock(CS);

  int Width=X2-X1+1;
  int Height=Y2-Y1+1;
  int I, J;

  for (I=0;I < Height; I++)
  {
    CHAR_INFO *PtrBuf=Buf+(Y1+I)*BufX+X1;
    for (J=0; J < Width; J++, ++PtrBuf)
    {
      if((PtrBuf->Attributes&=~ColorMask) == 0)
        PtrBuf->Attributes=0x08;
    }
  }

#ifdef DIRECT_SCREEN_OUT
  Flush();
#elif defined(DIRECT_RT)
  if ( DirectRT  )
    Flush();
#endif
}

/* ���������������� ��������� �������� ���������
*/
void ScreenBuf::ApplyColor(int X1,int Y1,int X2,int Y2,int Color)
{
  CriticalSectionLock Lock(CS);

  int Width=X2-X1+1;
  int Height=Y2-Y1+1;
  int I, J;

  for (I=0;I < Height; I++)
  {
    CHAR_INFO *PtrBuf=Buf+(Y1+I)*BufX+X1;
    for (J=0; J < Width; J++, ++PtrBuf)
      PtrBuf->Attributes=Color;
      //Buf[K+J].Attributes=Color;
  }

#ifdef DIRECT_SCREEN_OUT
  Flush();
#elif defined(DIRECT_RT)
  if ( DirectRT  )
    Flush();
#endif
}

/* ��������� ������������� �������� Ch � ������ Color
*/
void ScreenBuf::FillRect(int X1,int Y1,int X2,int Y2,int Ch,int Color)
{
  CriticalSectionLock Lock(CS);

  int Width=X2-X1+1;
  int Height=Y2-Y1+1;
  int I, J;

  CHAR_INFO CI,*PtrBuf;
  CI.Attributes=Color;
  SetVidChar(CI,Ch);

  for (I=0; I < Height; I++)
  {
    for (PtrBuf=Buf+(Y1+I)*BufX+X1, J=0; J < Width; J++, ++PtrBuf)
      *PtrBuf=CI;
  }

  SBFlags.Clear(SBFLAGS_FLUSHED);

#ifdef DIRECT_SCREEN_OUT
  Flush();
#elif defined(DIRECT_RT)
  if ( DirectRT  )
    Flush();
#endif
}

/* "��������" ����������� ����� �� �������
*/
void ScreenBuf::Flush()
{
  CriticalSectionLock Lock(CS);

  if (LockCount>0)
    return;

  if (CtrlObject!=NULL && CtrlObject->Macro.IsRecording())
  {
    if (GetVidChar(Buf[0])!=L'R')
      MacroChar=Buf[0];
    SetVidChar(Buf[0],L'R');
    Buf[0].Attributes=FarColorToReal(COL_WARNDIALOGTEXT);
  }

  if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && !CurVisible)
  {
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize=CurSize;
    cci.bVisible=CurVisible;
    SetConsoleCursorInfo(hScreen,&cci);
    SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
  }

  if (!SBFlags.Check(SBFLAGS_FLUSHED))
  {
    SBFlags.Set(SBFLAGS_FLUSHED);
    if (WaitInMainLoop && Opt.Clock && !ProcessShowClock)
      ShowTime(FALSE);

    int WriteX1=BufX-1,WriteY1=BufY-1,WriteX2=0,WriteY2=0;
    int NoChanges=TRUE;
    if (SBFlags.Check(SBFLAGS_USESHADOW))
    {
      CHAR_INFO *PtrBuf=Buf, *PtrShadow=Shadow;

      for (int I=0; I < BufY; I++)
      {
        for (int J=0; J < BufX; J++, ++PtrBuf, ++PtrShadow)
        {
          if (PtrBuf->Attributes  != PtrShadow->Attributes ||
              GetVidChar(*PtrBuf) != GetVidChar(*PtrShadow))
          {
            if (WriteX1>J)
              WriteX1=J;
            if (WriteX2<J)
              WriteX2=J;
            if (WriteY1>I)
              WriteY1=I;
            if (WriteY2<I)
              WriteY2=I;
            NoChanges=FALSE;
          }
        }
      }
    }
    else
    {
      NoChanges=FALSE;
      WriteX1=0;
      WriteY1=0;
      WriteX2=BufX-1;
      WriteY2=BufY-1;
    }
    if (!NoChanges)
    {
      COORD Size,Corner;
      SMALL_RECT Coord;
      Size.X=BufX;
      Size.Y=BufY;
      Corner.X=WriteX1;
      Corner.Y=WriteY1;
      Coord.Left=WriteX1;
      Coord.Top=WriteY1;
      Coord.Right=WriteX2;
      Coord.Bottom=WriteY2;

      if (BufX*BufY*sizeof(CHAR_INFO)>0xFFFF) // See REMINDER file section scrbuf.cpp
      {
        Corner.Y=0;
        PCHAR_INFO BufPtr;
        int maxY;

        for (int yy=WriteY1; yy<=WriteY2;)
        {
          Coord.Top=yy;
          BufPtr=Buf+yy*BufX;
          maxY=0xFFFF/(BufX*sizeof(CHAR_INFO));
          if (maxY==0)
          {
            maxY=1;
          }
          else if (maxY-1+yy>WriteY2)
          {
            maxY=WriteY2-yy+1;
          }
          Size.Y=maxY;
          yy+=maxY;
          Coord.Bottom=yy-1;

          WriteConsoleOutputW (hScreen, BufPtr, Size, Corner, &Coord);
        }
      }
      else
      {
      	WriteConsoleOutputW (hScreen, Buf, Size, Corner, &Coord);
      }

      memcpy(Shadow,Buf,BufX*BufY*sizeof(CHAR_INFO));
    }
  }
  if (!SBFlags.Check(SBFLAGS_FLUSHEDCURPOS))
  {
    COORD C;
    C.X=CurX;
    C.Y=CurY;
    SetConsoleCursorPosition(hScreen,C);
    SBFlags.Set(SBFLAGS_FLUSHEDCURPOS);
  }
  if (!SBFlags.Check(SBFLAGS_FLUSHEDCURTYPE) && CurVisible)
  {
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize=CurSize;
    cci.bVisible=CurVisible;
    SetConsoleCursorInfo(hScreen,&cci);
    SBFlags.Set(SBFLAGS_FLUSHEDCURTYPE);
  }
  SBFlags.Set(SBFLAGS_USESHADOW|SBFLAGS_FLUSHED);
}


void ScreenBuf::Lock()
{
  LockCount++;
}


void ScreenBuf::Unlock()
{
  if (LockCount>0)
    LockCount--;
}


void ScreenBuf::SetHandle(HANDLE hScreen)
{
  ScreenBuf::hScreen=hScreen;
}


void ScreenBuf::ResetShadow()
{
  SBFlags.Clear(SBFLAGS_FLUSHED|SBFLAGS_FLUSHEDCURTYPE|SBFLAGS_FLUSHEDCURPOS|SBFLAGS_USESHADOW);
}


void ScreenBuf::MoveCursor(int X,int Y)
{
  CriticalSectionLock Lock(CS);

  CurX=X;
  CurY=Y;
  SBFlags.Clear(SBFLAGS_FLUSHEDCURPOS);
}


void ScreenBuf::GetCursorPos(int& X,int& Y)
{
  X=CurX;
  Y=CurY;
}


void ScreenBuf::SetCursorType(int Visible,int Size)
{
  /* $ 09.01.2001 SVS
     �� ������� ER - � SetCursorType �� ������� ������
     ������� ��������� �������
  */
  if (CurVisible!=Visible || CurSize!=Size)
  {
    CurVisible=Visible;
    CurSize=Size;
    SBFlags.Clear(SBFLAGS_FLUSHEDCURTYPE);
  }
}

void ScreenBuf::GetCursorType(int &Visible,int &Size)
{
  Visible=CurVisible;
  Size=CurSize;
}


void ScreenBuf::RestoreMacroChar()
{
  Write(0,0,&MacroChar,1);
  ResetShadow();
}

//  ���������������� ������ �� ���� ������ �����.
void ScreenBuf::Scroll(int Num)
{
  CriticalSectionLock Lock(CS);

  if(Num > 0 && Num < BufY)
    memmove(Buf,Buf+Num*BufX,(BufY-Num)*BufX*sizeof(CHAR_INFO));
#ifdef DIRECT_SCREEN_OUT
  Flush();
#elif defined(DIRECT_RT)
  if ( DirectRT  )
    Flush();
#endif
}
