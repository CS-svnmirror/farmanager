#ifndef __FILEFILTERPARAMS_HPP__
#define __FILEFILTERPARAMS_HPP__
/*
filefilterparams.hpp

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

#include "plugin.hpp"
#include "struct.hpp"
#include "CFileMask.hpp"
#include "bitflags.hpp"
#include "UnicodeString.hpp"

struct FileListItem;

enum FileFilterFlags
{
  FFF_RPANELINCLUDE = 1,
  FFF_RPANELEXCLUDE = 2,
  FFF_LPANELINCLUDE = 4,
  FFF_LPANELEXCLUDE = 8,
  FFF_FINDFILEINCLUDE = 16,
  FFF_FINDFILEEXCLUDE = 32,
  FFF_COPYINCLUDE = 64,
  FFF_COPYEXCLUDE = 128,
  FFF_SELECTINCLUDE = 256,
  FFF_SELECTEXCLUDE = 512,
};

enum FDateType
{
  FDATE_MODIFIED=0,
  FDATE_CREATED,
  FDATE_OPENED,

  FDATE_COUNT, // ������ ��������� !!!
};

enum FSizeType
{
  FSIZE_INBYTES=0,
  FSIZE_INKBYTES,
  FSIZE_INMBYTES,
  FSIZE_INGBYTES,

  FSIZE_COUNT, // ������ ��������� !!!
};

class FileFilterParams
{
  private:

    string m_Title;

    struct
    {
      DWORD Used;
      string strMask;
      CFileMask FilterMask; // ��������� ���������������� �����.
    } FMask;

    struct
    {
      DWORD Used;
      FDateType DateType;
      FILETIME DateAfter;
      FILETIME DateBefore;
    } FDate;

    struct
    {
      DWORD Used;
      FSizeType SizeType;
      __int64 SizeAbove; // ����� ������ ����� ������ � SizeType ��� -1 ��� �����
      __int64 SizeBelow; // ����� ������ ����� ������ � SizeType ��� -1 ��� �����
      unsigned __int64 SizeAboveReal; // ����� ������ ����� ������ � ������
      unsigned __int64 SizeBelowReal; // ����� ������ ����� ������ � ������
    } FSize;

    struct
    {
      DWORD Used;
      DWORD AttrSet;
      DWORD AttrClear;
    } FAttr;

    struct
    {
      HighlightDataColor Colors;
      int SortGroup;
      bool bContinueProcessing;
    } FHighlight;

  public:

    BitFlags Flags; // ����� �������

  public:

    FileFilterParams();

    const FileFilterParams &operator=(const FileFilterParams &FF);

    void SetTitle(const wchar_t *Title);
    void SetMask(DWORD Used, const wchar_t *Mask);
    void SetDate(DWORD Used, DWORD DateType, FILETIME DateAfter, FILETIME DateBefore);
    void SetSize(DWORD Used, DWORD SizeType, __int64 SizeAbove, __int64 SizeBelow);
    void SetAttr(DWORD Used, DWORD AttrSet, DWORD AttrClear);
    void SetColors(HighlightDataColor *Colors);
    void SetSortGroup(int SortGroup) { FHighlight.SortGroup = SortGroup; }
    void SetContinueProcessing(bool bContinueProcessing) { FHighlight.bContinueProcessing = bContinueProcessing; }

    const wchar_t *GetTitle() const;
    DWORD GetMask(const wchar_t **Mask) const;
    DWORD GetDate(DWORD *DateType, FILETIME *DateAfter, FILETIME *DateBefore) const;
    DWORD GetSize(DWORD *SizeType, __int64 *SizeAbove, __int64 *SizeBelow) const;
    DWORD GetAttr(DWORD *AttrSet, DWORD *AttrClear) const;
    void  GetColors(HighlightDataColor *Colors) const;
    int   GetMarkChar() const;
    int   GetSortGroup() const { return FHighlight.SortGroup; }
    bool  GetContinueProcessing() const { return FHighlight.bContinueProcessing; }

    // ������ ����� ���������� "�������" � ������ ��� �����������:
    // �������� �� ���� fd ��� ������� �������������� �������.
    // ���������� true  - ��������;
    //            false - �� ��������.
    bool FileInFilter(const FAR_FIND_DATA *fd);
    bool FileInFilter(const FAR_FIND_DATA_EX *fde);
    bool FileInFilter(const FileListItem *fli);
};

bool FileFilterConfig(FileFilterParams *FF, bool ColorConfig=false);

//���������������� ������� ��� �������� ����� ���� ��������� ��������.
void MenuString(string &dest, FileFilterParams *FF, bool bHighightType=false, bool bPanelType=false, const wchar_t *FMask=NULL, const wchar_t *Title=NULL);

#endif //__FILEFILTERPARAMS_HPP__
