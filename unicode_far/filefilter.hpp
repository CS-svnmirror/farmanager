#ifndef __FILEFILTER_HPP__
#define __FILEFILTER_HPP__
/*
filefilter.hpp

�������� ������
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
#include "filefilterparams.hpp"

class VMenu;
class Panel;

enum enumFileFilterType {
  FFT_PANEL = 0,
  FFT_FINDFILE,
  FFT_COPY,
  FFT_SELECT,
};

class FileFilter
{
  private:
    Panel *m_HostPanel;
    enumFileFilterType m_FilterType;

  private:
    int  ParseAndAddMasks(wchar_t **ExtPtr,const wchar_t *FileName,DWORD FileAttr,int& ExtCount,int Check);
    void ProcessSelection(VMenu *FilterList);
    void GetIncludeExcludeFlags(DWORD &Inc, DWORD &Exc);
    int  GetCheck(FileFilterParams *FFP);
    static void SwapPanelFlags(FileFilterParams *CurFilterData);

  public:
    FileFilter(Panel *HostPanel, enumFileFilterType FilterType);
    ~FileFilter();

  public:
    bool FilterEdit();
    bool FileInFilter(FileListItem *fli);
    bool FileInFilter(const FAR_FIND_DATA *fd);
    bool FileInFilter(const FAR_FIND_DATA_EX *fde);
    bool IsEnabledOnPanel();

    static void InitFilter();
    static void CloseFilter();
    static void SwapFilter();
    static void SaveFilters();
};

#endif  // __FINDFILES_HPP__
