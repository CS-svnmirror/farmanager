#ifndef __TREELIST_HPP__
#define __TREELIST_HPP__
/*
treelist.hpp

Tree panel
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

#include "panel.hpp"
#include "UnicodeString.hpp"

struct TreeItem
{
  string strName;
  int Last[NM/2];        // ?
  int Depth;             // ������� �����������

  void Clear()
  {
    strName=L"";
    memset(Last,0,sizeof(Last));
    Depth=0;
  }

  TreeItem& operator=(const TreeItem &tiCopy)
  {
    strName=tiCopy.strName;
    memcpy(Last,tiCopy.Last,sizeof(Last));
    Depth=tiCopy.Depth;
    return *this;
  }
};

enum TREELIST_FLAGS{
  FTREELIST_TREEISPREPARED          = 0x00010000,
  FTREELIST_UPDATEREQUIRED          = 0x00020000,
  FTREELIST_ISPANEL                 = 0x00040000,
};

class TreeList: public Panel
{
  private:
    int PrevMacroMode;
    TreeItem **ListData;
    string strRoot;
    long TreeCount;
    long WorkDir;
    long GetSelPosition;
    int CaseSensitiveSort;
    int NumericSort;
    int ExitCode; // ��������� ������ ��� ������, ���������� �� ������!

    struct TreeItem *SaveListData;
    long SaveTreeCount;
    long SaveWorkDir;

  private:
    void SetMacroMode(int Restore = FALSE);
    virtual void DisplayObject();
    void DisplayTree(int Fast);
    void DisplayTreeName(const wchar_t *Name,int Pos);
    void Up(int Count);
    void Down(int Count);
    void Scroll(int Count);
    void CorrectPosition();
    void FillLastData();
    int CountSlash(const wchar_t *Str);
    int SetDirPosition(const wchar_t *NewDir);
    void GetRoot();
    Panel* GetRootPanel();
    void SyncDir();
    void SaveTreeFile();
    int ReadTreeFile();
    virtual int GetSelCount();
    void DynamicUpdateKeyBar();
    int GetNextNavPos();
    int GetPrevNavPos();
    static string &MkTreeFileName(const wchar_t *RootDir,string &strDest);
    static string &MkTreeCacheFolderName(const wchar_t *RootDir,string &strDest);
    static string &CreateTreeFileName(const wchar_t *Path,string &strDest);

    bool SaveState();
    bool RestoreState();

  private:
    static int MsgReadTree(int TreeCount,int &FirstCall);
    static int GetCacheTreeName(const wchar_t *Root, string &strName,int CreateDir);

  public:
    TreeList(int IsPanel=TRUE);
    virtual ~TreeList();

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);
//    virtual void KillFocus();
    virtual void Update(int Mode);
    int  ReadTree();

    virtual BOOL SetCurDir(const wchar_t *NewDir,int ClosePlugin);

    void SetRootDir(const wchar_t *NewRootDir);

    virtual int GetCurDir(string &strCurDir);

    virtual int GetCurName(string &strName, string &strShortName);

    virtual void UpdateViewPanel();
    virtual void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual int FindPartName(const wchar_t *Name,int Next,int Direct=1,int ExcludeSets=0);

    virtual int GoToFile(long idxItem);
    virtual int GoToFile(const wchar_t *Name,BOOL OnlyPartName=FALSE);
    virtual long FindFile(const wchar_t *Name,BOOL OnlyPartName=FALSE);

    void ProcessEnter();

    virtual long FindFirst(const wchar_t *Name);
    virtual long FindNext(int StartPos, const wchar_t *Name);

    int GetExitCode() {return ExitCode;}
    virtual long GetFileCount() {return TreeCount;}
    virtual int GetFileName(string &strName,int Pos,int &FileAttr);

    virtual void SetTitle();
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
    virtual void SetFocus();
    virtual void KillFocus();
    virtual BOOL UpdateKeyBar();
    virtual BOOL GetItem(int Index,void *Dest);
    virtual int GetCurrentPos();

    virtual int GetSelName(string *strName,int &FileAttr,string *ShortName=NULL,FAR_FIND_DATA_EX *fd=NULL);

  public:
    static void AddTreeName(const wchar_t *Name);
    static void DelTreeName(const wchar_t *Name);
    static void RenTreeName(const wchar_t *SrcName, const wchar_t *DestName);
    static void ReadSubTree(const wchar_t *Path);
    static void ClearCache(int EnableFreeMem);
    static void ReadCache(const wchar_t *TreeRoot);
    static void FlushCache();

    static int MustBeCached(const wchar_t *Root); // $ 16.10.2000 tran - �������, ������������� ������������� ����������� �����
    static void PR_MsgReadTree(void);
};

#endif  // __TREELIST_HPP__
