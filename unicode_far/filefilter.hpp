#pragma once

/*
filefilter.hpp

�������� ������
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

#include "filefilterparams.hpp"

class VMenu2;
class Panel;

// ������ FileInFilter ������ true ��� false
enum enumFileInFilterType
{
	FIFT_NOTINTFILTER = 0,   // �������� ������ �� ����� �� � ���� �� ��������
	FIFT_INCLUDE,            // �������� ������ ����� � Include
	FIFT_EXCLUDE,            // �������� ������ ����� � Exclude
};


class FileFilter: noncopyable
{
public:
	FileFilter(Panel *HostPanel, FAR_FILE_FILTER_TYPE FilterType);
	~FileFilter();

	bool FilterEdit();
	void UpdateCurrentTime();
	bool FileInFilter(const FileListItem* fli, enumFileInFilterType *foundType = nullptr);
	bool FileInFilter(const os::FAR_FIND_DATA& fde, enumFileInFilterType *foundType = nullptr, const string* FullName = nullptr);
	bool FileInFilter(const PluginPanelItem& fd, enumFileInFilterType *foundType = nullptr);
	bool IsEnabledOnPanel();

	static void InitFilter();
	static void CloseFilter();
	static void SwapFilter();
	static void Save(bool always);

private:
	Panel *GetHostPanel();
	void ProcessSelection(VMenu2 *FilterList);
	const enumFileFilterFlagsType GetFFFT();
	int  GetCheck(const FileFilterParams& FFP);
	static void SwapPanelFlags(FileFilterParams& CurFilterData);
	static int  ParseAndAddMasks(std::list<std::pair<string, int>>& Extensions, const string& FileName, DWORD FileAttr, int Check);

	Panel *m_HostPanel;
	FAR_FILE_FILTER_TYPE m_FilterType;
	unsigned __int64 CurrentTime;
};
