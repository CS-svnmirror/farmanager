#pragma once

/*
copy.hpp

class ShellCopy - ����������� ������
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

#include "dizlist.hpp"

class Panel;
class Dialog;
class CopyProgress;

ENUM(COPY_CODES);
ENUM(ReparsePointTypes);

class ShellCopy: NonCopyable
{
public:
	ShellCopy(Panel *SrcPanel,int Move,int Link,int CurrentOnly,int Ask, int &ToPlugin, const wchar_t* PluginDestPath, bool ToSubdir=false);
	~ShellCopy();

private:
	COPY_CODES CopyFileTree(const string&  Dest);
	COPY_CODES ShellCopyOneFile(const string& Src, const api::FAR_FIND_DATA &SrcData, string &strDest, int KeepPathPos, int Rename);
	COPY_CODES CheckStreams(const string& Src,const string& DestPath);
	int ShellCopyFile(const string& SrcName,const api::FAR_FIND_DATA &SrcData, string &strDestName,DWORD &DestAttr,int Append);
	int ShellSystemCopy(const string& SrcName,const string& DestName,const api::FAR_FIND_DATA &SrcData);
	int DeleteAfterMove(const string& Name,DWORD Attr);
	int AskOverwrite(const api::FAR_FIND_DATA &SrcData,const string& SrcName,const string& DestName, DWORD DestAttr,int SameName,int Rename,int AskAppend, int &Append,string &strNewName,int &RetCode);
	static int GetSecurity(const string& FileName, api::FAR_SECURITY_DESCRIPTOR& sd);
	static int SetSecurity(const string& FileName,const api::FAR_SECURITY_DESCRIPTOR& sd);
	int SetRecursiveSecurity(const string& FileName,const api::FAR_SECURITY_DESCRIPTOR& sd);
	bool CalcTotalSize();
	bool ShellSetAttr(const string& Dest,DWORD Attr);
	void SetDestDizPath(const string& DestPath);
	static intptr_t WarnDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t CopyDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);

	std::unique_ptr<CopyProgress> CP;
	DWORD Flags;
	Panel *SrcPanel,*DestPanel;
	int SrcPanelMode,DestPanelMode;
	int SrcDriveType,DestDriveType;
	string strSrcDriveRoot;
	string strDestDriveRoot;
	string strDestFSName;
	DizList DestDiz;
	string strDestDizPath;
	char_ptr CopyBuffer;
	size_t CopyBufferSize;
	string strCopiedName;
	string strRenamedName;
	string strRenamedFilesPath;
	int OvrMode;
	int ReadOnlyOvrMode;
	int ReadOnlyDelMode;
	int SkipMode;          // ...��� �������� ��� ����������� ���������� ������.
	int SkipEncMode;
	int SkipDeleteMode;
	int SelectedFolderNameLength;
	std::list<string> DestList;
	// ��� ������������ ������������.
	// ��� AltF6 ����� ��, ��� ������ ���� � �������,
	// � ��������� ������� - RP_EXACTCOPY - ��� � ���������
	ReparsePointTypes RPT;
	string strPluginFormat;
	int AltF10;
	int CopySecurity;
	size_t SelCount;
	DWORD FileAttr;
	bool FolderPresent;
	bool FilesPresent;
	bool AskRO;
};
