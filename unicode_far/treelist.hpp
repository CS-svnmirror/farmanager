#pragma once

/*
treelist.hpp

Tree panel
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

#include "panel.hpp"
#include "macro.hpp"

enum
{
	MODALTREE_ACTIVE  =1,
	MODALTREE_PASSIVE =2,
	MODALTREE_FREE    =3
};

struct TreeItem:public ipanelitem
{
	std::vector<int> Last;
	int Depth;             // ������� �����������

	TreeItem():
		Last(MAX_PATH/2),
		Depth(0)
	{
	}

	TreeItem(const string& Name):
		ipanelitem(Name),
		Last(MAX_PATH/2),
		Depth(0)
	{
	}

	TreeItem& operator=(const TreeItem &tiCopy)
	{
		if (this != &tiCopy)
		{
			strName=tiCopy.strName;
			Last = tiCopy.Last;
			Depth=tiCopy.Depth;
		}

		return *this;
	}
};

enum TREELIST_FLAGS
{
	FTREELIST_TREEISPREPARED          = 0x00010000,
	FTREELIST_UPDATEREQUIRED          = 0x00020000,
	FTREELIST_ISPANEL                 = 0x00040000,
};

class TreeListCache
{
public:
	void Clean()
	{
		Names.clear();
		strTreeName.clear();
	}

	TreeListCache& operator=(const TreeListCache& from)
	{
		strTreeName = from.strTreeName;
		Names = from.Names;
		return *this;
	}

	void Sort();

	string strTreeName;
	std::list<string> Names;
};

class TreeList: public Panel
{
	private:
		FARMACROAREA PrevMacroMode;
		std::vector<std::unique_ptr<TreeItem>> ListData;
		string strRoot;
		size_t WorkDir;
		long GetSelPosition;
		bool NumericSort;
		bool CaseSensitiveSort;
		int ExitCode; // ��������� ������ ��� ������, ���������� �� ������!

		std::vector<TreeItem> SaveListData;
		size_t SaveWorkDir;

	private:
		void SetMacroMode(int Restore = FALSE);
		virtual void DisplayObject() override;
		void DisplayTree(int Fast);
		void DisplayTreeName(const wchar_t *Name, size_t Pos);
		void Up(int Count);
		void Down(int Count);
		void Scroll(int Count);
		void CorrectPosition();
		bool FillLastData();
		UINT CountSlash(const wchar_t *Str);
		int SetDirPosition(const string& NewDir);
		void GetRoot();
		Panel* GetRootPanel();
		void SyncDir();
		void SaveTreeFile();
		int ReadTreeFile();
		virtual size_t GetSelCount() override;
		void DynamicUpdateKeyBar();
		int GetNextNavPos();
		int GetPrevNavPos();
		static string &MkTreeFileName(const string& RootDir,string &strDest);
		static string &MkTreeCacheFolderName(const string& RootDir,string &strDest);
		static string &CreateTreeFileName(const string& Path,string &strDest);

		bool SaveState();
		bool RestoreState();

	private:
		static int MsgReadTree(size_t TreeCount,int &FirstCall);
		static int GetCacheTreeName(const string& Root, string& strName,int CreateDir);

	public:
		TreeList(bool IsPanel=true);
	private:
		virtual ~TreeList();

	public:
		virtual int ProcessKey(int Key) override;
		virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
		virtual void Update(int Mode) override;
		int  ReadTree();

		virtual bool SetCurDir(const string& NewDir,bool ClosePanel,bool IsUpdated=true) override;

		void SetRootDir(const string& NewRootDir);

		virtual const string& GetCurDir() override;

		virtual int GetCurName(string &strName, string &strShortName) override;

		virtual void UpdateViewPanel() override;
		virtual void MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
		virtual int FindPartName(const string& Name,int Next,int Direct=1,int ExcludeSets=0) override;
		virtual bool GetPlainString(string& Dest,int ListPos) override;

		virtual int GoToFile(long idxItem) override;
		virtual int GoToFile(const string& Name,BOOL OnlyPartName=FALSE) override;
		virtual long FindFile(const string& Name,BOOL OnlyPartName=FALSE) override;

		void ProcessEnter();

		virtual long FindFirst(const string& Name) override;
		virtual long FindNext(int StartPos, const string& Name) override;

		int GetExitCode() {return ExitCode;}
		virtual size_t GetFileCount() override {return ListData.size();}
		virtual int GetFileName(string &strName,int Pos,DWORD &FileAttr) override;

		virtual void SetTitle() override;
		virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0) override;
		virtual void SetFocus() override;
		virtual void KillFocus() override;
		virtual BOOL UpdateKeyBar() override;
		virtual TreeItem* GetItem(size_t Index) const override;
		virtual int GetCurrentPos() const override;

		virtual int GetSelName(string *strName,DWORD &FileAttr,string *ShortName=nullptr,FAR_FIND_DATA *fd=nullptr) override;

	public:
		static void AddTreeName(const string& Name);
		static void DelTreeName(const string& Name);
		static void RenTreeName(const string& SrcName, const string& DestName);
		static void ReadSubTree(const string& Path);
		static void ClearCache(int EnableFreeMem);
		static void ReadCache(const string& TreeRoot);
		static void FlushCache();

		static int MustBeCached(const string& Root); // $ 16.10.2000 tran - �������, ������������� ������������� ����������� �����
		static void PR_MsgReadTree();
};
