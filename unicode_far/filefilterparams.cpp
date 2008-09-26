/*
filefilterparams.cpp

��������� ��������� �������
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

#include "colors.hpp"
#include "CFileMask.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filelist.hpp"
#include "filefilterparams.hpp"

FileFilterParams::FileFilterParams()
{
  m_Title=L"";
  SetMask(1,L"*");
  SetSize(0,FSIZE_INBYTES,_i64(-1),_i64(-1));
  memset(&FDate,0,sizeof(FDate));
  memset(&FAttr,0,sizeof(FAttr));
  memset(&FHighlight.Colors,0,sizeof(FHighlight.Colors));
  FHighlight.SortGroup=DEFAULT_SORT_GROUP;
  FHighlight.bContinueProcessing=false;
  Flags.ClearAll();
}

const FileFilterParams &FileFilterParams::operator=(const FileFilterParams &FF)
{
  SetTitle(FF.GetTitle());
  const wchar_t *Mask;
  FF.GetMask(&Mask);
  SetMask(FF.GetMask(NULL),Mask);
  memcpy(&FSize,&FF.FSize,sizeof(FSize));
  memcpy(&FDate,&FF.FDate,sizeof(FDate));
  memcpy(&FAttr,&FF.FAttr,sizeof(FAttr));
  FF.GetColors(&FHighlight.Colors);
  FHighlight.SortGroup=FF.GetSortGroup();
  FHighlight.bContinueProcessing=FF.GetContinueProcessing();
  Flags.Flags=FF.Flags.Flags;
  return *this;
}

void FileFilterParams::SetTitle(const wchar_t *Title)
{
  m_Title = Title;
}

void FileFilterParams::SetMask(DWORD Used, const wchar_t *Mask)
{
  FMask.Used = Used;
  FMask.strMask = Mask;

  /* ��������� %PATHEXT% */
  string strMask = Mask;
  wchar_t *Ptr = strMask.GetBuffer();
  wchar_t *PtrMask = Ptr;
  // ��������
  if((Ptr=wcschr(Ptr, L'%')) != NULL && !StrCmpNI(Ptr,L"%PATHEXT%",9))
  {
    int IQ1=(*(Ptr+9) == L',')?10:9, offsetPtr=(int)((Ptr-PtrMask));
    // ���� ����������� %pathext%, �� ������� � �����...
    wmemmove(Ptr,Ptr+IQ1,StrLength(Ptr+IQ1)+1);

    strMask.ReleaseBuffer();

    string strTmp1 = strMask;

    wchar_t *pSeparator, *lpwszTmp1;

    lpwszTmp1 = strTmp1.GetBuffer();

    pSeparator=wcschr(lpwszTmp1, EXCLUDEMASKSEPARATOR);

    if(pSeparator)
    {
      Ptr=lpwszTmp1+offsetPtr;
      if(Ptr>pSeparator) // PATHEXT ��������� � ������ ����������
      {
        strTmp1.ReleaseBuffer();
        Add_PATHEXT(strMask); // ��������� ��, ���� ����.
      }
      else
      {
        string strTmp2;
        strTmp2 = (pSeparator+1);
        *pSeparator=0;

        strTmp1.ReleaseBuffer();

        Add_PATHEXT(strTmp1);
        strMask.Format (L"%s|%s", (const wchar_t*)strTmp1, (const wchar_t*)strTmp2);
      }


    }
    else
    {
      strTmp1.ReleaseBuffer();
      Add_PATHEXT(strMask); // ��������� ��, ���� ����.
    }
  }

  // �������� �� ���������� ������� �������� �������
  if (!FMask.FilterMask.Set(strMask,FMF_SILENT))
  {
    FMask.strMask = L"*";
    FMask.FilterMask.Set(FMask.strMask,FMF_SILENT);
  }
}

void FileFilterParams::SetDate(DWORD Used, DWORD DateType, FILETIME DateAfter, FILETIME DateBefore)
{
  FDate.Used=Used;
  FDate.DateType=(FDateType)DateType;
  if (DateType>=FDATE_COUNT)
    FDate.DateType=FDATE_MODIFIED;
  FDate.DateAfter=DateAfter;
  FDate.DateBefore=DateBefore;
}

void FileFilterParams::SetSize(DWORD Used, DWORD SizeType, __int64 SizeAbove, __int64 SizeBelow)
{
  FSize.Used=Used;
  FSize.SizeType=(FSizeType)SizeType;
  if (SizeType>=FSIZE_COUNT)
    FSize.SizeType=FSIZE_INBYTES;
  FSize.SizeAbove=SizeAbove;
  FSize.SizeBelow=SizeBelow;
  FSize.SizeAboveReal=SizeAbove;
  FSize.SizeBelowReal=SizeBelow;
  switch (FSize.SizeType)
  {
    case FSIZE_INBYTES:
      // ������ ����� � ������, ������ ������ �� ������.
      break;
    case FSIZE_INKBYTES:
      // ������ ����� � ����������, �������� ��� � �����.
      // !!! �������� �� ���������� ������������� �������� �� �������� !!!
      FSize.SizeAboveReal<<=10;
      FSize.SizeBelowReal<<=10;
      break;
    case FSIZE_INMBYTES:
      // ����� // ������ ����� � ����������, �������� ��� � �����.
      // !!! �������� �� ���������� ������������� �������� �� �������� !!!
      FSize.SizeAboveReal<<=20;
      FSize.SizeBelowReal<<=20;
      break;
    case FSIZE_INGBYTES:
      // ����� // ������ ����� � ����������, �������� ��� � �����.
      // !!! �������� �� ���������� ������������� �������� �� �������� !!!
      FSize.SizeAboveReal<<=30;
      FSize.SizeBelowReal<<=30;
      break;
  }
}

void FileFilterParams::SetAttr(DWORD Used, DWORD AttrSet, DWORD AttrClear)
{
  FAttr.Used=Used;
  FAttr.AttrSet=AttrSet;
  FAttr.AttrClear=AttrClear;
}

void FileFilterParams::SetColors(HighlightDataColor *Colors)
{
  memcpy(&FHighlight.Colors,Colors,sizeof(FHighlight.Colors));
}

const wchar_t *FileFilterParams::GetTitle() const
{
  return m_Title;
}

DWORD FileFilterParams::GetMask(const wchar_t **Mask) const
{
  if (Mask)
    *Mask=FMask.strMask;
  return FMask.Used;
}

DWORD FileFilterParams::GetDate(DWORD *DateType, FILETIME *DateAfter, FILETIME *DateBefore) const
{
  if (DateType)
    *DateType=FDate.DateType;
  if (DateAfter)
    *DateAfter=FDate.DateAfter;
  if (DateBefore)
    *DateBefore=FDate.DateBefore;
  return FDate.Used;
}

DWORD FileFilterParams::GetSize(DWORD *SizeType, __int64 *SizeAbove, __int64 *SizeBelow) const
{
  if (SizeType)
    *SizeType=FSize.SizeType;
  if (SizeAbove)
    *SizeAbove=FSize.SizeAbove;
  if (SizeBelow)
    *SizeBelow=FSize.SizeBelow;
  return FSize.Used;
}

DWORD FileFilterParams::GetAttr(DWORD *AttrSet, DWORD *AttrClear) const
{
  if (AttrSet)
    *AttrSet=FAttr.AttrSet;
  if (AttrClear)
    *AttrClear=FAttr.AttrClear;
  return FAttr.Used;
}

void FileFilterParams::GetColors(HighlightDataColor *Colors) const
{
  memcpy(Colors,&FHighlight.Colors,sizeof(*Colors));
}

int FileFilterParams::GetMarkChar() const
{
  return FHighlight.Colors.MarkChar;
}

bool FileFilterParams::FileInFilter(const FileListItem *fli)
{
  FAR_FIND_DATA fd;

  fd.dwFileAttributes=fli->FileAttr;
  fd.ftCreationTime=fli->CreationTime;
  fd.ftLastAccessTime=fli->AccessTime;
  fd.ftLastWriteTime=fli->WriteTime;
  fd.nFileSize=fli->UnpSize;
  fd.nPackSize=fli->PackSize;
  fd.lpwszFileName=(wchar_t *)(const wchar_t *)fli->strName;
  fd.lpwszAlternateFileName=(wchar_t *)(const wchar_t *)fli->strShortName;

  return FileInFilter(&fd);
}

bool FileFilterParams::FileInFilter(const FAR_FIND_DATA_EX *fde)
{
  FAR_FIND_DATA fd;

  fd.dwFileAttributes=fde->dwFileAttributes;
  fd.ftCreationTime=fde->ftCreationTime;
  fd.ftLastAccessTime=fde->ftLastAccessTime;
  fd.ftLastWriteTime=fde->ftLastWriteTime;
  fd.nFileSize=fde->nFileSize;
  fd.nPackSize=fde->nPackSize;
  fd.lpwszFileName=(wchar_t *)(const wchar_t *)fde->strFileName;
  fd.lpwszAlternateFileName=(wchar_t *)(const wchar_t *)fde->strAlternateFileName;

  return FileInFilter(&fd);
}

bool FileFilterParams::FileInFilter(const FAR_FIND_DATA *fd)
{
  // ������ ��������?
  //if (fd==NULL)
    //return false;

  // ����� �������� ��������� ����� �������?
  if (FAttr.Used)
  {
    // �������� ��������� ����� �� ������������� ���������
    if ((fd->dwFileAttributes & FAttr.AttrSet) != FAttr.AttrSet)
      return false;

    // �������� ��������� ����� �� ������������� ���������
    if (fd->dwFileAttributes & FAttr.AttrClear)
      return false;
  }

  // ����� �������� ������� ����� �������?
  if (FSize.Used)
  {
    if (FSize.SizeAbove != _i64(-1))
    {
      if (fd->nFileSize < FSize.SizeAboveReal) // ������ ����� ������ ������������ ������������ �� �������?
        return false;                          // �� ���������� ���� ����
    }

    if (FSize.SizeBelow != _i64(-1))
    {

      if (fd->nFileSize > FSize.SizeBelowReal) // ������ ����� ������ ������������� ������������ �� �������?
        return false;                          // �� ���������� ���� ����
    }
  }

  // ����� �������� ������� ����� �������?
  if (FDate.Used)
  {
    // ����������� FILETIME � ����������� __int64
    unsigned __int64 after=FileTimeToUI64(&FDate.DateAfter);
    unsigned __int64 before=FileTimeToUI64(&FDate.DateBefore);

    if (after!=_ui64(0) || before!=_ui64(0))
    {
      unsigned __int64 ftime=_ui64(0);

      switch (FDate.DateType)
      {
        case FDATE_MODIFIED:
          ftime=FileTimeToUI64(&fd->ftLastWriteTime);
          break;
        case FDATE_CREATED:
          ftime=FileTimeToUI64(&fd->ftCreationTime);
          break;
        case FDATE_OPENED:
          ftime=FileTimeToUI64(&fd->ftLastAccessTime);
          break;
      }

      // ���� �������� ������������� ��������� ����?
      if (after!=_ui64(0))
        // ���� ����� ������ ��������� ���� �� �������?
        if (ftime<after)
          // �� ���������� ���� ����
          return false;

      // ���� �������� ������������� �������� ����?
      if (before!=_ui64(0))
        // ���� ����� ������ �������� ���� �� �������?
        if (ftime>before)
          return false;
    }
  }

  // ����� �������� ����� ����� �������?
  if (FMask.Used)
  {
    // ���� �� �������� ��� ����� �������� � �������?
    if (!FMask.FilterMask.Compare(fd->lpwszFileName))
      // �� ���������� ���� ����
      return false;
  }

  // ��! ���� �������� ��� ��������� � ����� ������� � �������������
  // � ��������� ��� ������� ��������.
  return true;
}

//���������������� ������� ��� �������� ����� ���� ��������� ��������.
void MenuString(string &dest, FileFilterParams *FF, bool bHighightType, bool bPanelType, const wchar_t *FMask, const wchar_t *Title)
{
  const wchar_t AttrC[] = L"RAHSDCEI$TLOV";
  const DWORD   AttrF[] = {
                            FILE_ATTRIBUTE_READONLY,
                            FILE_ATTRIBUTE_ARCHIVE,
                            FILE_ATTRIBUTE_HIDDEN,
                            FILE_ATTRIBUTE_SYSTEM,
                            FILE_ATTRIBUTE_DIRECTORY,
                            FILE_ATTRIBUTE_COMPRESSED,
                            FILE_ATTRIBUTE_ENCRYPTED,
                            FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
                            FILE_ATTRIBUTE_SPARSE_FILE,
                            FILE_ATTRIBUTE_TEMPORARY,
                            FILE_ATTRIBUTE_REPARSE_POINT,
                            FILE_ATTRIBUTE_OFFLINE,
                            FILE_ATTRIBUTE_VIRTUAL
                          };

  const wchar_t Format1[] = L"%-21.21s %c %-26.26s %-2.2s %c %s";
  const wchar_t Format2[] = L"%-3.3s %c %-26.26s %-2.2s %c %s";

  const wchar_t *Name, *Mask;
  wchar_t MarkChar[]=L"\" \"";
  DWORD IncludeAttr, ExcludeAttr;
  DWORD UseMask, UseSize, UseDate;

  if (bPanelType)
  {
    Name=Title;
    UseMask=1;
    Mask=FMask;
    IncludeAttr=0;
    ExcludeAttr=FILE_ATTRIBUTE_DIRECTORY;
    UseDate=UseSize=0;
  }
  else
  {
    MarkChar[1]=(wchar_t)FF->GetMarkChar();
    if (MarkChar[1]==0)
      *MarkChar=0;
    Name=FF->GetTitle();
    UseMask=FF->GetMask(&Mask);
    if (!FF->GetAttr(&IncludeAttr,&ExcludeAttr))
      IncludeAttr=ExcludeAttr=0;
    UseSize=FF->GetSize(NULL,NULL,NULL);
    UseDate=FF->GetDate(NULL,NULL,NULL);
  }

  wchar_t Attr[countof(AttrC)*2] = {0};
  for (size_t i=0; i<countof(AttrF); i++)
  {
    wchar_t *Ptr=Attr+i*2;
    *Ptr=AttrC[i];
    if (IncludeAttr&AttrF[i])
      *(Ptr+1)=L'+';
    else if (ExcludeAttr&AttrF[i])
      *(Ptr+1)=L'-';
    else
      *Ptr=*(Ptr+1)=L'.';
  }

  wchar_t SizeDate[3] = L"..";
  if (UseSize)
    SizeDate[0]=L'S';
  if (UseDate)
    SizeDate[1]=L'D';

  if (bHighightType)
    dest.Format(Format2, MarkChar, VerticalLine, Attr, SizeDate, VerticalLine, UseMask ? Mask : L"");
  else
    dest.Format(Format1, Name, VerticalLine, Attr, SizeDate, VerticalLine, UseMask ? Mask : L"");
  RemoveTrailingSpaces(dest);
}

enum enumFileFilterConfig {
    ID_FF_TITLE,

    ID_FF_NAME,
    ID_FF_NAMEEDIT,

    ID_FF_SEPARATOR1,

    ID_FF_MATCHMASK,
    ID_FF_MASKEDIT,

    ID_FF_SEPARATOR2,

    ID_FF_MATCHSIZE,
    ID_FF_SIZEDIVIDER,
    ID_FF_SIZEFROM,
    ID_FF_SIZEFROMEDIT,
    ID_FF_SIZETO,
    ID_FF_SIZETOEDIT,

    ID_FF_MATCHDATE,
    ID_FF_DATETYPE,
    ID_FF_DATEAFTER,
    ID_FF_DATEAFTEREDIT,
    ID_FF_TIMEAFTEREDIT,
    ID_FF_DATEBEFORE,
    ID_FF_DATEBEFOREEDIT,
    ID_FF_TIMEBEFOREEDIT,
    ID_FF_CURRENT,
    ID_FF_BLANK,

    ID_FF_SEPARATOR3,
    ID_FF_VSEPARATOR1,

    ID_FF_MATCHATTRIBUTES,
    ID_FF_READONLY,
    ID_FF_ARCHIVE,
    ID_FF_HIDDEN,
    ID_FF_SYSTEM,
    ID_FF_REPARSEPOINT,
    ID_FF_DIRECTORY,
    ID_FF_COMPRESSED,
    ID_FF_ENCRYPTED,
    ID_FF_NOTINDEXED,
    ID_FF_SPARSE,
    ID_FF_TEMP,
    ID_FF_OFFLINE,
    ID_FF_VIRTUAL,

    ID_HER_SEPARATOR3,
    ID_HER_MARK_TITLE,
    ID_HER_MARKEDIT,
    ID_HER_MARKTRANSPARENT,

    ID_HER_NORMALFILE,
    ID_HER_NORMALMARKING,
    ID_HER_SELECTEDFILE,
    ID_HER_SELECTEDMARKING,
    ID_HER_CURSORFILE,
    ID_HER_CURSORMARKING,
    ID_HER_SELECTEDCURSORFILE,
    ID_HER_SELECTEDCURSORMARKING,

    ID_HER_COLOREXAMPLE,
    ID_HER_CONTINUEPROCESSING,
    ID_FF_SEPARATOR4,

    ID_FF_OK,
    ID_FF_RESET,
    ID_FF_CANCEL,
    ID_FF_MAKETRANSPARENT,
};

void HighlightDlgUpdateUserControl(CHAR_INFO *VBufColorExample, struct HighlightDataColor &Colors)
{
  const wchar_t *ptr;
  DWORD Color;
  const DWORD FarColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};
  for (int j=0; j<4; j++)
  {
    Color=(DWORD)(Colors.Color[HIGHLIGHTCOLORTYPE_FILE][j]&0x00FF);
    if (!Color)
      Color=FarColorToReal(FarColor[j]);
    if (Colors.MarkChar&0x0000FFFF)
      ptr=MSG(MHighlightExample2);
    else
      ptr=MSG(MHighlightExample1);
    for (int k=0; k<15; k++)
    {
      VBufColorExample[15*j+k].Char.UnicodeChar=ptr[k];
      VBufColorExample[15*j+k].Attributes=(WORD)Color;
    }
    if (Colors.MarkChar&0x0000FFFF)
    {
      VBufColorExample[15*j+1].Char.UnicodeChar=(WCHAR)Colors.MarkChar&0x0000FFFF;
      if (Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][j]&0x00FF)
        VBufColorExample[15*j+1].Attributes=Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][j]&0x00FF;
    }
    VBufColorExample[15*j].Attributes=FarColorToReal(COL_PANELBOX);
    VBufColorExample[15*j+14].Attributes=FarColorToReal(COL_PANELBOX);
  }
}

LONG_PTR WINAPI FileFilterConfigDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch(Msg)
  {
    case DN_BTNCLICK:
    {
      if (Param1==ID_FF_CURRENT || Param1==ID_FF_BLANK) //Current � Blank
      {
        FILETIME ft;
        string strDate, strTime;

        if (Param1==ID_FF_CURRENT)
        {
          GetSystemTimeAsFileTime(&ft);
          ConvertDate(ft,strDate,strTime,8,FALSE,FALSE,TRUE);
        }
        else
          strDate=strTime=L"";

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(LONG_PTR)(const wchar_t*)strDate);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(LONG_PTR)(const wchar_t*)strTime);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(LONG_PTR)(const wchar_t*)strDate);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(LONG_PTR)(const wchar_t*)strTime);

        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,ID_FF_DATEAFTEREDIT,0);
        COORD r;
        r.X=r.Y=0;
        Dialog::SendDlgMessage(hDlg,DM_SETCURSORPOS,ID_FF_DATEAFTEREDIT,(LONG_PTR)&r);

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
        break;
      }
      else if (Param1==ID_FF_RESET) // Reset
      {
        // ������� �������
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_MASKEDIT,(LONG_PTR)L"*");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZEFROMEDIT,(LONG_PTR)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZETOEDIT,(LONG_PTR)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(LONG_PTR)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(LONG_PTR)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(LONG_PTR)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(LONG_PTR)L"");

        /* 14.06.2004 KM
           ������� BSTATE_UNCHECKED �� BSTATE_3STATE, � ������
           ������ ��� ����� ��������, �.�. ��������� ��������
        */
        for(int I=ID_FF_READONLY; I <= ID_FF_VIRTUAL; ++I)
          Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);

        // 6, 13 - ������� � ������
        struct FarListPos LPos={0,0};
        Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_SIZEDIVIDER,(LONG_PTR)&LPos);
        Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_DATETYPE,(LONG_PTR)&LPos);

        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHMASK,BSTATE_CHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHSIZE,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHDATE,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHATTRIBUTES,BSTATE_UNCHECKED);

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
        break;
      }
      else if (Param1==ID_FF_MAKETRANSPARENT)
      {
        HighlightDataColor *Colors = (HighlightDataColor *) Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);
        for (int i=0; i<2; i++)
          for (int j=0; j<4; j++)
            Colors->Color[i][j]|=0xFF00;
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_HER_MARKTRANSPARENT,BSTATE_CHECKED);
        break;
      }
    }
    case DN_MOUSECLICK:
      if((Msg==DN_BTNCLICK && Param1 >= ID_HER_NORMALFILE && Param1 <= ID_HER_SELECTEDCURSORMARKING)
         || (Msg==DN_MOUSECLICK && Param1==ID_HER_COLOREXAMPLE && ((MOUSE_EVENT_RECORD *)Param2)->dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED))
      {
        HighlightDataColor *EditData = (HighlightDataColor *) Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);

        if (Msg==DN_MOUSECLICK)
        {
          Param1 = ID_HER_NORMALFILE + ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.Y*2;
          if (((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.X==1 && (EditData->MarkChar&0x0000FFFF))
            Param1 = ID_HER_NORMALMARKING + ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.Y*2;
        }

        //Color[0=file, 1=mark][0=normal,1=selected,2=undercursor,3=selectedundercursor]
        unsigned int Color=(unsigned int)EditData->Color[(Param1-ID_HER_NORMALFILE)&1][(Param1-ID_HER_NORMALFILE)/2];
        GetColorDialog(Color,true,true);
        EditData->Color[(Param1-ID_HER_NORMALFILE)&1][(Param1-ID_HER_NORMALFILE)/2]=(WORD)Color;

        FarDialogItem *ColorExample = (FarDialogItem *)Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,0);
        wchar_t MarkChar[2];
        //MarkChar ��� FIXEDIT �������� � 1 ������ ��� ��� ��������� ������ ������ �� ����
        Dialog::SendDlgMessage(hDlg,DM_GETTEXTPTR,ID_HER_MARKEDIT,(LONG_PTR)MarkChar);
        EditData->MarkChar=*MarkChar;
        HighlightDlgUpdateUserControl(ColorExample->Param.VBuf,*EditData);
        Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_HER_COLOREXAMPLE,(LONG_PTR)ColorExample);
        Dialog::SendDlgMessage(hDlg,DM_FREEDLGITEM,0,(LONG_PTR)ColorExample);
        return TRUE;
      }
      break;

    case DN_EDITCHANGE:
      if (Param1 == ID_HER_MARKEDIT)
      {
        HighlightDataColor *EditData = (HighlightDataColor *) Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);
        FarDialogItem *ColorExample = (FarDialogItem *)Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,0);
        wchar_t MarkChar[2];
        //MarkChar ��� FIXEDIT �������� � 1 ������ ��� ��� ��������� ������ ������ �� ����
        Dialog::SendDlgMessage(hDlg,DM_GETTEXTPTR,ID_HER_MARKEDIT,(LONG_PTR)MarkChar);
        EditData->MarkChar=*MarkChar;
        HighlightDlgUpdateUserControl(ColorExample->Param.VBuf,*EditData);
        Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_HER_COLOREXAMPLE,(LONG_PTR)ColorExample);
        Dialog::SendDlgMessage(hDlg,DM_FREEDLGITEM,0,(LONG_PTR)ColorExample);
        return TRUE;
      }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool FileFilterConfig(FileFilterParams *FF, bool ColorConfig)
{
/*
  |         |         |         |         |         |         |         |
  00000000001111111111222222222233333333334444444444555555555566666666667777777777
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
01+-------------------------- ������ �������� --------------------------+
02| �������� �������:                                                   |
03+-------------------------- ��������� ����� --------------------------+
04| [ ] ���������� � ������ (�������)                                   |
05+------------------------------+--------------------------------------+
06| [x] ������ �                 � [x] ����/�����                       |
07|   ������ ��� �����: "      " �   ������� �:  "  .  .   " "  :  :  " |
08|   ������ ��� �����: "      " �   ����������: "  .  .   " "  :  :  " |
09|                              �                [ ������� ] [ ����� ] |
10+------------------------------+--------------------------------------+
11| [ ] ��������                                                        |
12|   [?] ������ ��� ������  [?] �������           [?] �����������      |
13|   [?] ��������           [?] ������            [?] ���������        |
14|   [?] �������            [?] �������������     [?] ����������       |
15|   [?] ���������          [?] ���������������   [?] �����������      |
16|   [?] ������. �����                                                 |
17+---------------------------------------------------------------------+
18|                  [ �� ]  [ �������� ]  [ ������ ]                   |
19+---------------------------------------------------------------------+
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  00000000001111111111222222222233333333334444444444555555555566666666667777777777
  |         |         |         |         |         |         |         |
*/

  // ��������� �����.
  CFileMask FileMask;
  const wchar_t VerticalLine[] = {0x252C,0x2502,0x2502,0x2502,0x2502,0x2534,0};
  // ����� ��� ����� �������� �����
  const wchar_t DigitMask[] = L"99999999999999999999";
  // ������� ��� ����� ������
  const wchar_t FilterMasksHistoryName[] = L"FilterMasks";
  // ������� ��� ����� �������
  const wchar_t FilterNameHistoryName[] = L"FilterName";
  // ����� ��� ������� ���������
  string strDateMask, strTimeMask;

  // ����������� ���������� ���� � ������� � �������.
  int DateSeparator=GetDateSeparator();
  int TimeSeparator=GetTimeSeparator();
  int DateFormat=GetDateFormat();

  switch(DateFormat)
  {
    case 0:
      // ����� ���� ��� �������� DD.MM.YYYY � MM.DD.YYYY
      strDateMask.Format(L"99%c99%c9999",DateSeparator,DateSeparator);
      break;
    case 1:
      // ����� ���� ��� �������� DD.MM.YYYY � MM.DD.YYYY
      strDateMask.Format(L"99%c99%c9999",DateSeparator,DateSeparator);
      break;
    default:
      // ����� ���� ��� ������� YYYY.MM.DD
      strDateMask.Format(L"9999%c99%c99",DateSeparator,DateSeparator);
      break;
  }
  // ����� �������
  strTimeMask.Format(L"99%c99%c99",TimeSeparator,TimeSeparator);

  struct DialogDataEx FilterDlgData[]=
  {
    DI_DOUBLEBOX,3,1,73,19,0,0,DIF_SHOWAMPERSAND,0,(const wchar_t *)MFileFilterTitle,

    DI_TEXT,5,2,0,2,1,0,0,0,(const wchar_t *)MFileFilterName,
    DI_EDIT,5,2,71,2,0,(DWORD_PTR)(const wchar_t *)FilterNameHistoryName,DIF_HISTORY,0,L"",

    DI_TEXT,0,3,0,3,0,0,DIF_SEPARATOR,0,L"",

    DI_CHECKBOX,5,4,0,4,0,0,DIF_AUTOMATION,0,(const wchar_t *)MFileFilterMatchMask,
    DI_EDIT,5,4,71,4,0,(DWORD_PTR)(const wchar_t *)FilterMasksHistoryName,DIF_HISTORY,0,L"",

    DI_TEXT,0,5,0,5,0,0,DIF_SEPARATOR,0,L"",

    DI_CHECKBOX,5,6,0,6,0,0,DIF_AUTOMATION,0,(const wchar_t *)MFileFilterSize,
    DI_COMBOBOX,20,6,36,6,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
    DI_TEXT,7,7,11,7,0,0,0,0,(const wchar_t *)MFileFilterSizeFrom,
    DI_FIXEDIT,20,7,36,7,0,(DWORD_PTR)DigitMask,DIF_MASKEDIT,0,L"",
    DI_TEXT,7,8,11,8,0,0,0,0,(const wchar_t *)MFileFilterSizeTo,
    DI_FIXEDIT,20,8,36,8,0,(DWORD_PTR)DigitMask,DIF_MASKEDIT,0,L"",

    DI_CHECKBOX,40,6,0,6,0,0,DIF_AUTOMATION,0,(const wchar_t *)MFileFilterDate,
    DI_COMBOBOX,58,6,71,6,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
    DI_TEXT,42,7,48,7,0,0,0,0,(const wchar_t *)MFileFilterAfter,
    DI_FIXEDIT,53,7,62,7,0,(DWORD_PTR)(const wchar_t *)strDateMask,DIF_MASKEDIT,0,L"",
    DI_FIXEDIT,64,7,71,7,0,(DWORD_PTR)(const wchar_t *)strTimeMask,DIF_MASKEDIT,0,L"",
    DI_TEXT,42,8,48,8,0,0,0,0,(const wchar_t *)MFileFilterBefore,
    DI_FIXEDIT,53,8,62,8,0,(DWORD_PTR)(const wchar_t *)strDateMask,DIF_MASKEDIT,0,L"",
    DI_FIXEDIT,64,8,71,8,0,(DWORD_PTR)(const wchar_t *)strTimeMask,DIF_MASKEDIT,0,L"",
    DI_BUTTON,0,9,0,9,0,0,DIF_BTNNOCLOSE,0,(const wchar_t *)MFileFilterCurrent,
    DI_BUTTON,0,9,71,9,0,0,DIF_BTNNOCLOSE,0,(const wchar_t *)MFileFilterBlank,

    DI_TEXT,0,10,0,10,0,0,DIF_SEPARATOR,0,L"",

    DI_VTEXT,38,5,38,10,0,0,0,0,VerticalLine,

    DI_CHECKBOX, 5,11,0,11,0,0,DIF_AUTOMATION,0,(const wchar_t *)MFileFilterAttr,

    DI_CHECKBOX, 7,12,0,12,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrR,
    DI_CHECKBOX, 7,13,0,13,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrA,
    DI_CHECKBOX, 7,14,0,14,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrH,
    DI_CHECKBOX, 7,15,0,15,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrS,
    DI_CHECKBOX, 7,16,0,16,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrReparse,

    DI_CHECKBOX,29,12,0,12,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrD,
    DI_CHECKBOX,29,13,0,13,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrC,
    DI_CHECKBOX,29,14,0,14,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrE,
    DI_CHECKBOX,29,15,0,15,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrNI,

    DI_CHECKBOX,51,12,0,12,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrSparse,
    DI_CHECKBOX,51,13,0,13,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrT,
    DI_CHECKBOX,51,14,0,15,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrOffline,
    DI_CHECKBOX,51,15,0,15,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrVirtual,

    DI_TEXT,-1,15,0,15,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,(const wchar_t *)MHighlightColors,
    DI_TEXT,7,16,0,16,0,0,0,0,(const wchar_t *)MHighlightMarkChar,
    DI_FIXEDIT,5,16,5,16,0,0,0,0,L"",
    DI_CHECKBOX,0,16,0,16,0,0,0,0,(const wchar_t *)MHighlightTransparentMarkChar,
    DI_BUTTON,5,17,0,17,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(const wchar_t *)MHighlightFileName1,
    DI_BUTTON,0,17,0,17,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(const wchar_t *)MHighlightMarking1,
    DI_BUTTON,5,18,0,18,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(const wchar_t *)MHighlightFileName2,
    DI_BUTTON,0,18,0,18,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(const wchar_t *)MHighlightMarking2,
    DI_BUTTON,5,19,0,19,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(const wchar_t *)MHighlightFileName3,
    DI_BUTTON,0,19,0,19,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(const wchar_t *)MHighlightMarking3,
    DI_BUTTON,5,20,0,20,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(const wchar_t *)MHighlightFileName4,
    DI_BUTTON,0,20,0,20,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(const wchar_t *)MHighlightMarking4,

    DI_USERCONTROL,73-15-1,17,73-2,20,0,0,DIF_NOFOCUS,0,L"",
    DI_CHECKBOX,5,21,0,21,0,0,0,0,(const wchar_t *)MHighlightContinueProcessing,

    DI_TEXT,0,17,0,17,0,0,DIF_SEPARATOR,0,L"",

    DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MFileFilterOk,
    DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(const wchar_t *)MFileFilterReset,
    DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFileFilterCancel,
    DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(const wchar_t *)MFileFilterMakeTransparent,
  };
  FilterDlgData[0].Data=(const wchar_t *)(ColorConfig?MFileHilightTitle:MFileFilterTitle);

  MakeDialogItemsEx(FilterDlgData,FilterDlg);

  if (ColorConfig)
  {
    FilterDlg[ID_FF_TITLE].Y2+=5;

    for (int i=ID_FF_NAME; i<=ID_FF_SEPARATOR1; i++)
      FilterDlg[i].Flags|=DIF_HIDDEN;

    for (int i=ID_FF_MATCHMASK; i<=ID_FF_VIRTUAL; i++)
    {
      FilterDlg[i].Y1-=2;
      FilterDlg[i].Y2-=2;
    }

    for (int i=ID_FF_SEPARATOR4; i<=ID_FF_MAKETRANSPARENT; i++)
    {
      FilterDlg[i].Y1+=5;
      FilterDlg[i].Y2+=5;
    }
  }
  else
  {
    for (int i=ID_HER_SEPARATOR3; i<=ID_HER_CONTINUEPROCESSING; i++)
      FilterDlg[i].Flags|=DIF_HIDDEN;
    FilterDlg[ID_FF_MAKETRANSPARENT].Flags=DIF_HIDDEN;
  }

  //FilterDlg[ID_FF_SIZEDIVIDER].X1=FilterDlg[ID_FF_MATCHSIZE].X1+strlen(FilterDlg[ID_FF_MATCHSIZE].Data)-(strchr(FilterDlg[ID_FF_MATCHSIZE].Data,'&')?1:0)+5;
  //FilterDlg[ID_FF_SIZEDIVIDER].X2+=FilterDlg[ID_FF_SIZEDIVIDER].X1;
  //FilterDlg[ID_FF_DATETYPE].X1=FilterDlg[ID_FF_MATCHDATE].X1+strlen(FilterDlg[ID_FF_MATCHDATE].Data)-(strchr(FilterDlg[ID_FF_MATCHDATE].Data,'&')?1:0)+5;
  //FilterDlg[ID_FF_DATETYPE].X2+=FilterDlg[ID_FF_DATETYPE].X1;

  FilterDlg[ID_FF_NAMEEDIT].X1=FilterDlg[ID_FF_NAME].X1+StrLength(FilterDlg[ID_FF_NAME].strData)-(wcschr(FilterDlg[ID_FF_NAME].strData,L'&')?1:0)+1;

  FilterDlg[ID_FF_MASKEDIT].X1=FilterDlg[ID_FF_MATCHMASK].X1+StrLength(FilterDlg[ID_FF_MATCHMASK].strData)-(wcschr(FilterDlg[ID_FF_MATCHMASK].strData,L'&')?1:0)+5;

  FilterDlg[ID_FF_BLANK].X1=FilterDlg[ID_FF_BLANK].X2-StrLength(FilterDlg[ID_FF_BLANK].strData)+(wcschr(FilterDlg[ID_FF_BLANK].strData,L'&')?1:0)-3;
  FilterDlg[ID_FF_CURRENT].X2=FilterDlg[ID_FF_BLANK].X1-2;
  FilterDlg[ID_FF_CURRENT].X1=FilterDlg[ID_FF_CURRENT].X2-StrLength(FilterDlg[ID_FF_CURRENT].strData)+(wcschr(FilterDlg[ID_FF_CURRENT].strData,L'&')?1:0)-3;

  FilterDlg[ID_HER_MARKTRANSPARENT].X1=FilterDlg[ID_HER_MARK_TITLE].X1+StrLength(FilterDlg[ID_HER_MARK_TITLE].strData)-(wcschr(FilterDlg[ID_HER_MARK_TITLE].strData,L'&')?1:0)+1;

  for (int i=ID_HER_NORMALMARKING; i<=ID_HER_SELECTEDCURSORMARKING; i+=2)
    FilterDlg[i].X1=FilterDlg[ID_HER_NORMALFILE].X1+StrLength(FilterDlg[ID_HER_NORMALFILE].strData)-(wcschr(FilterDlg[ID_HER_NORMALFILE].strData,L'&')?1:0)+1;

  CHAR_INFO VBufColorExample[15*4];
  HighlightDataColor Colors;

  FF->GetColors(&Colors);
  memset(VBufColorExample,0,sizeof(VBufColorExample));
  HighlightDlgUpdateUserControl(VBufColorExample,Colors);
  FilterDlg[ID_HER_COLOREXAMPLE].VBuf=VBufColorExample;

  wchar_t MarkChar[] = {(wchar_t)Colors.MarkChar&0x0000FFFF, 0};
  FilterDlg[ID_HER_MARKEDIT].strData=MarkChar;
  FilterDlg[ID_HER_MARKTRANSPARENT].Selected=(Colors.MarkChar&0xFF0000?1:0);

  FilterDlg[ID_HER_CONTINUEPROCESSING].Selected=(FF->GetContinueProcessing()?1:0);

  FilterDlg[ID_FF_NAMEEDIT].strData=FF->GetTitle();

  const wchar_t *FMask;
  FilterDlg[ID_FF_MATCHMASK].Selected=FF->GetMask(&FMask);
  FilterDlg[ID_FF_MASKEDIT].strData=FMask;
  if (!FilterDlg[ID_FF_MATCHMASK].Selected)
    FilterDlg[ID_FF_MASKEDIT].Flags|=DIF_DISABLE;

  // ���� ��� ����������: ����� - ���������
  FarList SizeList;
  FarListItem TableItemSize[FSIZE_COUNT];
  // ��������� ������ ���������� ��� ���� �������
  SizeList.Items=TableItemSize;
  SizeList.ItemsNumber=FSIZE_COUNT;

  memset(TableItemSize,0,sizeof(TableItemSize));
  for(int i=0; i < FSIZE_COUNT; ++i)
    TableItemSize[i].Text=MSG(MFileFilterSizeInBytes+i);

  DWORD SizeType;
  __int64 SizeAbove, SizeBelow;
  FilterDlg[ID_FF_MATCHSIZE].Selected=FF->GetSize(&SizeType,&SizeAbove,&SizeBelow);
  FilterDlg[ID_FF_SIZEDIVIDER].ListItems=&SizeList;
  TableItemSize[SizeType].Flags=LIF_SELECTED;

  if (SizeAbove != _i64(-1))
  {
    wchar_t Str[100];
    _ui64tow(SizeAbove,Str,10);
    FilterDlg[ID_FF_SIZEFROMEDIT].strData = Str;
  }

  if (SizeBelow != _i64(-1))
  {
    wchar_t Str[100];
    _ui64tow(SizeBelow,Str,10);
    FilterDlg[ID_FF_SIZETOEDIT].strData = Str;
  }

  if (!FilterDlg[ID_FF_MATCHSIZE].Selected)
    for(int i=ID_FF_SIZEDIVIDER; i <= ID_FF_SIZETOEDIT; i++)
      FilterDlg[i].Flags|=DIF_DISABLE;

  // ���� ��� ���������� ������� �����
  FarList DateList;
  FarListItem TableItemDate[FDATE_COUNT];
  // ��������� ������ ����� ��� �����
  DateList.Items=TableItemDate;
  DateList.ItemsNumber=FDATE_COUNT;

  memset(TableItemDate,0,sizeof(TableItemDate));
  for(int i=0; i < FDATE_COUNT; ++i)
    TableItemDate[i].Text=MSG(MFileFilterModified+i);

  DWORD DateType;
  FILETIME DateAfter, DateBefore;
  FilterDlg[ID_FF_MATCHDATE].Selected=FF->GetDate(&DateType,&DateAfter,&DateBefore);
  FilterDlg[ID_FF_DATETYPE].ListItems=&DateList;
  TableItemDate[DateType].Flags=LIF_SELECTED;

  ConvertDate(DateAfter,FilterDlg[ID_FF_DATEAFTEREDIT].strData,FilterDlg[ID_FF_TIMEAFTEREDIT].strData,8,FALSE,FALSE,TRUE);
  ConvertDate(DateBefore,FilterDlg[ID_FF_DATEBEFOREEDIT].strData,FilterDlg[ID_FF_TIMEBEFOREEDIT].strData,8,FALSE,FALSE,TRUE);

  if (!FilterDlg[ID_FF_MATCHDATE].Selected)
    for(int i=ID_FF_DATETYPE; i <= ID_FF_BLANK; i++)
      FilterDlg[i].Flags|=DIF_DISABLE;

  DWORD AttrSet, AttrClear;
  FilterDlg[ID_FF_MATCHATTRIBUTES].Selected=FF->GetAttr(&AttrSet,&AttrClear);
  FilterDlg[ID_FF_READONLY].Selected=(AttrSet & FILE_ATTRIBUTE_READONLY?1:AttrClear & FILE_ATTRIBUTE_READONLY?0:2);
  FilterDlg[ID_FF_ARCHIVE].Selected=(AttrSet & FILE_ATTRIBUTE_ARCHIVE?1:AttrClear & FILE_ATTRIBUTE_ARCHIVE?0:2);
  FilterDlg[ID_FF_HIDDEN].Selected=(AttrSet & FILE_ATTRIBUTE_HIDDEN?1:AttrClear & FILE_ATTRIBUTE_HIDDEN?0:2);
  FilterDlg[ID_FF_SYSTEM].Selected=(AttrSet & FILE_ATTRIBUTE_SYSTEM?1:AttrClear & FILE_ATTRIBUTE_SYSTEM?0:2);
  FilterDlg[ID_FF_COMPRESSED].Selected=(AttrSet & FILE_ATTRIBUTE_COMPRESSED?1:AttrClear & FILE_ATTRIBUTE_COMPRESSED?0:2);
  FilterDlg[ID_FF_ENCRYPTED].Selected=(AttrSet & FILE_ATTRIBUTE_ENCRYPTED?1:AttrClear & FILE_ATTRIBUTE_ENCRYPTED?0:2);
  FilterDlg[ID_FF_DIRECTORY].Selected=(AttrSet & FILE_ATTRIBUTE_DIRECTORY?1:AttrClear & FILE_ATTRIBUTE_DIRECTORY?0:2);
  FilterDlg[ID_FF_NOTINDEXED].Selected=(AttrSet & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?1:AttrClear & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?0:2);
  FilterDlg[ID_FF_SPARSE].Selected=(AttrSet & FILE_ATTRIBUTE_SPARSE_FILE?1:AttrClear & FILE_ATTRIBUTE_SPARSE_FILE?0:2);
  FilterDlg[ID_FF_TEMP].Selected=(AttrSet & FILE_ATTRIBUTE_TEMPORARY?1:AttrClear & FILE_ATTRIBUTE_TEMPORARY?0:2);
  FilterDlg[ID_FF_REPARSEPOINT].Selected=(AttrSet & FILE_ATTRIBUTE_REPARSE_POINT?1:AttrClear & FILE_ATTRIBUTE_REPARSE_POINT?0:2);
  FilterDlg[ID_FF_OFFLINE].Selected=(AttrSet & FILE_ATTRIBUTE_OFFLINE?1:AttrClear & FILE_ATTRIBUTE_OFFLINE?0:2);
  FilterDlg[ID_FF_VIRTUAL].Selected=(AttrSet & FILE_ATTRIBUTE_VIRTUAL?1:AttrClear & FILE_ATTRIBUTE_VIRTUAL?0:2);

  if (!FilterDlg[ID_FF_MATCHATTRIBUTES].Selected)
  {
    for(int i=ID_FF_READONLY; i <= ID_FF_VIRTUAL; i++)
      FilterDlg[i].Flags|=DIF_DISABLE;
  }

  Dialog Dlg(FilterDlg,countof(FilterDlg),FileFilterConfigDlgProc,(LONG_PTR)&Colors);

  Dlg.SetHelp(ColorConfig?L"HighlightEdit":L"Filter");
  Dlg.SetPosition(-1,-1,FilterDlg[ID_FF_TITLE].X2+4,FilterDlg[ID_FF_TITLE].Y2+2);

  Dlg.SetAutomation(ID_FF_MATCHMASK,ID_FF_MASKEDIT,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEDIVIDER,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROM,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETO,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOEDIT,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATETYPE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTER,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFORE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_CURRENT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_BLANK,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_READONLY,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ARCHIVE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_HIDDEN,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SYSTEM,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_COMPRESSED,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ENCRYPTED,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_NOTINDEXED,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SPARSE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_TEMP,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_REPARSEPOINT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_OFFLINE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_VIRTUAL,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_DIRECTORY,DIF_DISABLE,0,0,DIF_DISABLE);

  for (;;)
  {
    Dlg.ClearDone();
    Dlg.Process();
    int ExitCode=Dlg.GetExitCode();

    if (ExitCode==ID_FF_OK) // Ok
    {
      // ���� �������� ������������� ����� �� ���������, ����� ������� � ������
      if (FilterDlg[ID_FF_MATCHMASK].Selected && !FileMask.Set((const wchar_t *)FilterDlg[ID_FF_MASKEDIT].strData,0))
        continue;

      if (FilterDlg[ID_HER_MARKTRANSPARENT].Selected)
        Colors.MarkChar|=0x00FF0000;
      else
        Colors.MarkChar&=0x0000FFFF;

      FF->SetColors(&Colors);

      FF->SetContinueProcessing(FilterDlg[ID_HER_CONTINUEPROCESSING].Selected!=0);

      FF->SetTitle(FilterDlg[ID_FF_NAMEEDIT].strData);

      FF->SetMask(FilterDlg[ID_FF_MATCHMASK].Selected,
                  FilterDlg[ID_FF_MASKEDIT].strData);

      RemoveExternalSpaces(FilterDlg[ID_FF_SIZEFROMEDIT].strData);

      if( FilterDlg[ID_FF_SIZEFROMEDIT].strData.IsEmpty() )
        SizeAbove=_i64(-1);
      else
        SizeAbove=_wtoi64(FilterDlg[ID_FF_SIZEFROMEDIT].strData);

      RemoveExternalSpaces(FilterDlg[ID_FF_SIZETOEDIT].strData);

      if ( FilterDlg[ID_FF_SIZETOEDIT].strData.IsEmpty() )
        SizeBelow=_i64(-1);
      else
        SizeBelow=_wtoi64(FilterDlg[ID_FF_SIZETOEDIT].strData);

      FF->SetSize(FilterDlg[ID_FF_MATCHSIZE].Selected,
                  FilterDlg[ID_FF_SIZEDIVIDER].ListPos,
                  SizeAbove,
                  SizeBelow);

      StrToDateTime(FilterDlg[ID_FF_DATEAFTEREDIT].strData,FilterDlg[ID_FF_TIMEAFTEREDIT].strData,DateAfter,DateFormat,DateSeparator,TimeSeparator);
      StrToDateTime(FilterDlg[ID_FF_DATEBEFOREEDIT].strData,FilterDlg[ID_FF_TIMEBEFOREEDIT].strData,DateBefore,DateFormat,DateSeparator,TimeSeparator);

      FF->SetDate(FilterDlg[ID_FF_MATCHDATE].Selected,
                  FilterDlg[ID_FF_DATETYPE].ListPos,
                  DateAfter,
                  DateBefore);

      AttrSet=0;
      AttrClear=0;

      AttrSet|=(FilterDlg[ID_FF_READONLY].Selected==1?FILE_ATTRIBUTE_READONLY:0);
      AttrSet|=(FilterDlg[ID_FF_ARCHIVE].Selected==1?FILE_ATTRIBUTE_ARCHIVE:0);
      AttrSet|=(FilterDlg[ID_FF_HIDDEN].Selected==1?FILE_ATTRIBUTE_HIDDEN:0);
      AttrSet|=(FilterDlg[ID_FF_SYSTEM].Selected==1?FILE_ATTRIBUTE_SYSTEM:0);
      AttrSet|=(FilterDlg[ID_FF_COMPRESSED].Selected==1?FILE_ATTRIBUTE_COMPRESSED:0);
      AttrSet|=(FilterDlg[ID_FF_ENCRYPTED].Selected==1?FILE_ATTRIBUTE_ENCRYPTED:0);
      AttrSet|=(FilterDlg[ID_FF_DIRECTORY].Selected==1?FILE_ATTRIBUTE_DIRECTORY:0);
      AttrSet|=(FilterDlg[ID_FF_NOTINDEXED].Selected==1?FILE_ATTRIBUTE_NOT_CONTENT_INDEXED:0);
      AttrSet|=(FilterDlg[ID_FF_SPARSE].Selected==1?FILE_ATTRIBUTE_SPARSE_FILE:0);
      AttrSet|=(FilterDlg[ID_FF_TEMP].Selected==1?FILE_ATTRIBUTE_TEMPORARY:0);
      AttrSet|=(FilterDlg[ID_FF_REPARSEPOINT].Selected==1?FILE_ATTRIBUTE_REPARSE_POINT:0);
      AttrSet|=(FilterDlg[ID_FF_OFFLINE].Selected==1?FILE_ATTRIBUTE_OFFLINE:0);
      AttrSet|=(FilterDlg[ID_FF_VIRTUAL].Selected==1?FILE_ATTRIBUTE_VIRTUAL:0);
      AttrClear|=(FilterDlg[ID_FF_READONLY].Selected==0?FILE_ATTRIBUTE_READONLY:0);
      AttrClear|=(FilterDlg[ID_FF_ARCHIVE].Selected==0?FILE_ATTRIBUTE_ARCHIVE:0);
      AttrClear|=(FilterDlg[ID_FF_HIDDEN].Selected==0?FILE_ATTRIBUTE_HIDDEN:0);
      AttrClear|=(FilterDlg[ID_FF_SYSTEM].Selected==0?FILE_ATTRIBUTE_SYSTEM:0);
      AttrClear|=(FilterDlg[ID_FF_COMPRESSED].Selected==0?FILE_ATTRIBUTE_COMPRESSED:0);
      AttrClear|=(FilterDlg[ID_FF_ENCRYPTED].Selected==0?FILE_ATTRIBUTE_ENCRYPTED:0);
      AttrClear|=(FilterDlg[ID_FF_DIRECTORY].Selected==0?FILE_ATTRIBUTE_DIRECTORY:0);
      AttrClear|=(FilterDlg[ID_FF_NOTINDEXED].Selected==0?FILE_ATTRIBUTE_NOT_CONTENT_INDEXED:0);
      AttrClear|=(FilterDlg[ID_FF_SPARSE].Selected==0?FILE_ATTRIBUTE_SPARSE_FILE:0);
      AttrClear|=(FilterDlg[ID_FF_TEMP].Selected==0?FILE_ATTRIBUTE_TEMPORARY:0);
      AttrClear|=(FilterDlg[ID_FF_REPARSEPOINT].Selected==0?FILE_ATTRIBUTE_REPARSE_POINT:0);
      AttrClear|=(FilterDlg[ID_FF_OFFLINE].Selected==0?FILE_ATTRIBUTE_OFFLINE:0);
      AttrClear|=(FilterDlg[ID_FF_VIRTUAL].Selected==0?FILE_ATTRIBUTE_VIRTUAL:0);

      FF->SetAttr(FilterDlg[ID_FF_MATCHATTRIBUTES].Selected, AttrSet, AttrClear);

      return true;
    }
    else
      break;
  }

  return false;
}
