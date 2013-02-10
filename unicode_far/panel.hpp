#pragma once

/*
panel.hpp

Parent class ��� �������
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

#include "scrobj.hpp"
#include "panelctype.hpp"
#include "config.hpp"

class DizList;

struct PanelViewSettings
{
	unsigned __int64 ColumnType[PANEL_COLUMNCOUNT];
	int ColumnWidth[PANEL_COLUMNCOUNT];
	int ColumnCount;
	unsigned __int64 StatusColumnType[PANEL_COLUMNCOUNT];
	int StatusColumnWidth[PANEL_COLUMNCOUNT];
	int StatusColumnCount;
	DWORD Flags;
	int ColumnWidthType[PANEL_COLUMNCOUNT];
	int StatusColumnWidthType[PANEL_COLUMNCOUNT];
};

enum
{
	PVS_FULLSCREEN            = 0x00000001,
	PVS_ALIGNEXTENSIONS       = 0x00000002,
	PVS_FOLDERALIGNEXTENSIONS = 0x00000004,
	PVS_FOLDERUPPERCASE       = 0x00000008,
	PVS_FILELOWERCASE         = 0x00000010,
	PVS_FILEUPPERTOLOWERCASE  = 0x00000020,
};

enum
{
	FILE_PANEL,
	TREE_PANEL,
	QVIEW_PANEL,
	INFO_PANEL
};

enum
{
	UNSORTED,
	BY_NAME,
	BY_EXT,
	BY_MTIME,
	BY_CTIME,
	BY_ATIME,
	BY_SIZE,
	BY_DIZ,
	BY_OWNER,
	BY_COMPRESSEDSIZE,
	BY_NUMLINKS,
	BY_NUMSTREAMS,
	BY_STREAMSSIZE,
	BY_FULLNAME,
	BY_CHTIME,
	BY_CUSTOMDATA,
	SORTMODE_LAST
};

enum {VIEW_0=0,VIEW_1,VIEW_2,VIEW_3,VIEW_4,VIEW_5,VIEW_6,VIEW_7,VIEW_8,VIEW_9};

enum
{
	DRIVE_SHOW_TYPE       = 0x00000001,
	DRIVE_SHOW_NETNAME    = 0x00000002,
	DRIVE_SHOW_LABEL      = 0x00000004,
	DRIVE_SHOW_FILESYSTEM = 0x00000008,
	DRIVE_SHOW_SIZE       = 0x00000010,
	DRIVE_SHOW_REMOVABLE  = 0x00000020,
	DRIVE_SHOW_PLUGINS    = 0x00000040,
	DRIVE_SHOW_CDROM      = 0x00000080,
	DRIVE_SHOW_SIZE_FLOAT = 0x00000100,
	DRIVE_SHOW_REMOTE     = 0x00000200,
	DRIVE_SORT_PLUGINS_BY_HOTKEY = 0x00000400,
};

enum {UPDATE_KEEP_SELECTION=1,UPDATE_SECONDARY=2,UPDATE_IGNORE_VISIBLE=4,UPDATE_DRAW_MESSAGE=8};

enum {NORMAL_PANEL,PLUGIN_PANEL};

enum {DRIVE_DEL_FAIL, DRIVE_DEL_SUCCESS, DRIVE_DEL_EJECT, DRIVE_DEL_NONE};

enum {UIC_UPDATE_NORMAL, UIC_UPDATE_FORCE, UIC_UPDATE_FORCE_NOTIFICATION};

class VMenu2;
class Edit;
struct PanelMenuItem;

class DelayedDestroy
{
	bool           destroyed;
	int prevent_delete_count;

public:
	DelayedDestroy() : destroyed(false), prevent_delete_count(0) {}

protected:
	virtual ~DelayedDestroy() {
		assert(destroyed);
	}

public:
	bool Destroy()	{
		assert(!destroyed);
		destroyed = true;
		if (prevent_delete_count > 0) {
			return false;
		}
		else {
			delete this;
			return true;
		}
	}

	bool Destroyed() { return destroyed; }

	int AddRef() { return ++prevent_delete_count; }

   int Release() {
		assert(prevent_delete_count > 0);
		if (--prevent_delete_count > 0 || !destroyed)
			return prevent_delete_count;
		else {
			delete this;
			return 0;
		}
	}
};

class DelayDestroy
{
	DelayedDestroy *host;
public:
	DelayDestroy(DelayedDestroy *_host) { (host = _host)->AddRef(); }
   ~DelayDestroy() { host->Release(); }
};

class Panel:public ScreenObject, public DelayedDestroy
{
	protected:
		PanelOptions* Options;
		bool Focus;
		int EnableUpdate;
		int PanelMode;
		int PrevViewMode;
		int CurTopFile;
		int CurFile;
		int ModalMode;
		int PluginCommand;
		string strPluginParam;

	public:
		void SwapOptions(Panel* Another) {std::swap(Options, Another->Options);}

		struct PanelViewSettings ViewSettings;
		int ProcessingPluginCommand;

	private:
		int ChangeDiskMenu(int Pos,int FirstCall);
		int DisconnectDrive(PanelMenuItem *item, VMenu2 &ChDisk);
		void RemoveHotplugDevice(PanelMenuItem *item, VMenu2 &ChDisk);
		int ProcessDelDisk(wchar_t Drive, int DriveType,VMenu2 *ChDiskMenu);
		void FastFindShow(int FindX,int FindY);
		void FastFindProcessName(Edit *FindEdit,const wchar_t *Src,string &strLastName, string &strName);
		void DragMessage(int X,int Y,int Move);

		string strDragName;

	private:
		struct ShortcutInfo
		{
			string ShortcutFolder;
			string PluginFile;
			string PluginData;
			GUID PluginGuid;
		};
		bool GetShortcutInfo(ShortcutInfo& Info);

	protected:
		void FastFind(int FirstKey);
		void DrawSeparator(int Y);
		void ShowScreensCount();
		int  IsDragging();
		virtual void ClearAllItem(){}

	public:
		Panel(PanelOptions* Options);
	protected:
		virtual ~Panel();

	public:
		virtual int SendKeyToPlugin(DWORD Key,bool Pred=false) {return FALSE;}
		virtual BOOL SetCurDir(const string& NewDir,int ClosePanel,BOOL IsUpdated=TRUE);
		virtual void ChangeDirToCurrent();

		virtual const string& GetCurDir();

		virtual size_t GetSelCount() {return 0;}
		virtual size_t GetRealSelCount() {return 0;}
		virtual int GetSelName(string *strName,DWORD &FileAttr,string *ShortName=nullptr,FAR_FIND_DATA_EX *fd=nullptr) {return FALSE;}
		virtual void UngetSelName() {}
		virtual void ClearLastGetSelection() {}
		virtual unsigned __int64 GetLastSelectedSize() {return (unsigned __int64)(-1);}
		virtual int GetLastSelectedItem(struct FileListItem *LastItem) {return 0;}

		virtual int GetCurName(string &strName, string &strShortName);
		virtual int GetCurBaseName(string &strName, string &strShortName);
		virtual int GetFileName(string &strName,int Pos,DWORD &FileAttr) {return FALSE;}

		virtual int GetCurrentPos() {return 0;}
		virtual void SetFocus();
		virtual void KillFocus();
		virtual void Update(int Mode) {}
		/*$ 22.06.2001 SKV
		  �������� ��� ������������� ������� ���������� Update.
		  ������������ ��� Update ����� ���������� �������.
		*/
		virtual int UpdateIfChanged(int UpdateMode) {return 0;}
		/* $ 19.03.2002 DJ
		   UpdateIfRequired() - ��������, ���� ������ ��� �������� ��-�� ����,
		   ��� ������ ��������
		*/
		virtual void UpdateIfRequired() {}

		virtual void StartFSWatcher(bool got_focus=false) {}
		virtual void StopFSWatcher() {}
		virtual int FindPartName(const wchar_t *Name,int Next,int Direct=1,int ExcludeSets=0) {return FALSE;}
		virtual bool GetPlainString(string& Dest,int ListPos){return false;}


		virtual int GoToFile(long idxItem) {return TRUE;}
		virtual int GoToFile(const wchar_t *Name,BOOL OnlyPartName=FALSE) {return TRUE;}
		virtual long FindFile(const wchar_t *Name,BOOL OnlyPartName=FALSE) {return -1;}

		virtual int IsSelected(const wchar_t *Name) {return FALSE;}
		virtual int IsSelected(size_t indItem) {return FALSE;}

		virtual long FindFirst(const wchar_t *Name) {return -1;}
		virtual long FindNext(int StartPos, const wchar_t *Name) {return -1;}

		virtual void SetSelectedFirstMode(bool) {}
		virtual bool GetSelectedFirstMode() {return false;}
		int GetMode() {return(PanelMode);}
		void SetMode(int Mode) {PanelMode=Mode;}
		int GetModalMode() {return(ModalMode);}
		void SetModalMode(int ModalMode) {Panel::ModalMode=ModalMode;}
		int GetViewMode() {return(Options->ViewMode);}
		virtual void SetViewMode(int ViewMode);
		virtual int GetPrevViewMode() {return(PrevViewMode);}
		void SetPrevViewMode(int PrevViewMode) {Panel::PrevViewMode=PrevViewMode;}
		virtual int GetPrevSortMode() {return(Options->SortMode);}
		virtual int GetPrevSortOrder() {return(Options->SortOrder);}
		int GetSortMode() {return(Options->SortMode);}
		virtual bool GetPrevNumericSort() {return Options->NumericSort;}
		bool GetNumericSort() { return Options->NumericSort; }
		void SetNumericSort(bool Mode) { Options->NumericSort = Mode; }
		virtual void ChangeNumericSort(bool Mode) { SetNumericSort(Mode); }
		virtual bool GetPrevCaseSensitiveSort() {return Options->CaseSensitiveSort;}
		bool GetCaseSensitiveSort() {return Options->CaseSensitiveSort;}
		void SetCaseSensitiveSort(bool Mode) {Options->CaseSensitiveSort = Mode;}
		virtual void ChangeCaseSensitiveSort(bool Mode) {SetCaseSensitiveSort(Mode);}
		virtual bool GetPrevDirectoriesFirst() {return Options->DirectoriesFirst;}
		bool GetDirectoriesFirst() { return Options->DirectoriesFirst; }
		void SetDirectoriesFirst(bool Mode) { Options->DirectoriesFirst = Mode != 0; }
		virtual void ChangeDirectoriesFirst(bool Mode) { SetDirectoriesFirst(Mode); }
		virtual void SetSortMode(int SortMode) {Options->SortMode=SortMode;}
		int GetSortOrder() {return(Options->SortOrder);}
		void SetSortOrder(int SortOrder) {Options->SortOrder=SortOrder;}
		virtual void ChangeSortOrder(int NewOrder) {SetSortOrder(NewOrder);}
		bool GetSortGroups() {return(Options->SortGroups);}
		void SetSortGroups(bool SortGroups) {Options->SortGroups=SortGroups;}
		bool GetShowShortNamesMode() {return(Options->ShowShortNames);}
		void SetShowShortNamesMode(bool Mode) {Options->ShowShortNames=Mode;}
		void InitCurDir(const wchar_t *CurDir);
		virtual void CloseFile() {}
		virtual void UpdateViewPanel() {}
		virtual void CompareDir() {}
		virtual void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent) {}
		virtual void ClearSelection() {}
		virtual void SaveSelection() {}
		virtual void RestoreSelection() {}
		virtual void SortFileList(int KeepPosition) {}
		virtual void EditFilter() {}
		virtual bool FileInFilter(size_t idxItem) {return true;}
		virtual bool FilterIsEnabled() {return false;}
		virtual void ReadDiz(struct PluginPanelItem *ItemList=nullptr,int ItemLength=0, DWORD dwFlags=0) {}
		virtual void DeleteDiz(const string& Name,const string& ShortName) {}
		virtual void GetDizName(string &strDizName) {}
		virtual void FlushDiz() {}
		virtual void CopyDiz(const string& Name,const string& ShortName,const string& DestName, const string& DestShortName,DizList *DestDiz) {}
		virtual bool IsFullScreen() {return (ViewSettings.Flags&PVS_FULLSCREEN)==PVS_FULLSCREEN;}
		virtual bool IsDizDisplayed() {return false;}
		virtual bool IsColumnDisplayed(int Type) {return false;}
		virtual int GetColumnsCount() { return 1;}
		virtual void SetReturnCurrentFile(int Mode) {}
		virtual void QViewDelTempName() {}
		virtual void GetOpenPanelInfo(struct OpenPanelInfo *Info) {}
		virtual void SetPluginMode(HANDLE hPlugin,const wchar_t *PluginFile,bool SendOnFocus=false) {}
		virtual void SetPluginModified() {}
		virtual int ProcessPluginEvent(int Event,void *Param) {return FALSE;}
		virtual HANDLE GetPluginHandle() {return nullptr;}
		virtual void SetTitle();
		virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0);

		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);

		virtual void IfGoHome(wchar_t Drive) {}

		/* $ 30.04.2001 DJ
		   ������� ���������� ��� ���������� �������; ���� ���������� FALSE,
		   ������������ ����������� ������
		*/
		virtual BOOL UpdateKeyBar() { return FALSE; }

		virtual size_t GetFileCount() {return 0;}
		virtual BOOL GetItem(int,void *) {return FALSE;}

		bool ExecShortcutFolder(int Pos, bool raw=false);
		bool ExecShortcutFolder(string& strShortcutFolder,const GUID& PluginGuid,string& strPluginFile,const string& strPluginData,bool CheckType);
		bool SaveShortcutFolder(int Pos, bool Add);

		static void EndDrag();
		virtual void Hide();
		virtual void Show();
		int SetPluginCommand(int Command,int Param1,void* Param2);
		int PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode);
		void ChangeDisk();
		bool GetFocus() {return Focus;}
		int GetType() {return(Options->Type);}
		void SetType(int Type) {Options->Type = Type;}
		void SetUpdateMode(int Mode) {EnableUpdate=Mode;}
		bool MakeListFile(string &strListFileName,bool ShortNames,const wchar_t *Modifers=nullptr);
		int SetCurPath();

		BOOL NeedUpdatePanel(Panel *AnotherPanel);
};
