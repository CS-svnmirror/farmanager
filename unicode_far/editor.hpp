#ifndef __EDITOR_HPP__
#define __EDITOR_HPP__
/*
editor.hpp

��������
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

#include "scrobj.hpp"

#include "plugin.hpp"

#include "bitflags.hpp"
#include "fn.hpp"

class FileEditor;
class KeyBar;

struct InternalEditorBookMark{
  DWORD Line[BOOKMARK_COUNT];
  DWORD Cursor[BOOKMARK_COUNT];
  DWORD ScreenLine[BOOKMARK_COUNT];
  DWORD LeftPos[BOOKMARK_COUNT];
};

struct InternalEditorStackBookMark{
  DWORD Line;
  DWORD Cursor;
  DWORD ScreenLine;
  DWORD LeftPos;
  struct InternalEditorStackBookMark *prev, *next;
};

struct EditorCacheParams {
	int Line;
	int LinePos;
	int ScreenLine;
	int LeftPos;
	int Table; //CODEPAGE!!! //BUGBUG

	InternalEditorBookMark SavePos;
};



struct EditorUndoData
{
  int Type;
  int UndoNext;
  int StrPos;
  int StrNum;
  wchar_t EOL[10];
  wchar_t *Str;
};

// ������� ���� (����� 0xFF) ������� ������� ScreenObject!!!
enum FLAGS_CLASS_EDITOR{
  FEDITOR_MODIFIED              = 0x00000200,
  FEDITOR_JUSTMODIFIED          = 0x00000400,   // 10.08.2000 skv: need to send EE_REDRAW 2.
                                                // set to 1 by TextChanged, no matter what
                                                // is value of State.
  FEDITOR_MARKINGBLOCK          = 0x00000800,
  FEDITOR_MARKINGVBLOCK         = 0x00001000,
  FEDITOR_WASCHANGED            = 0x00002000,
  FEDITOR_OVERTYPE              = 0x00004000,
  FEDITOR_UNDOOVERFLOW          = 0x00008000,   // ������������ � ����?
  FEDITOR_NEWUNDO               = 0x00010000,
  FEDITOR_DISABLEUNDO           = 0x00040000,   // �������� ������� Undo ��� ����?
  FEDITOR_LOCKMODE              = 0x00080000,
  FEDITOR_CURPOSCHANGEDBYPLUGIN = 0x00100000,   // TRUE, ���� ������� � ��������� ���� ��������
                                                // �������� (ECTL_SETPOSITION)
  FEDITOR_ISRESIZEDCONSOLE      = 0x00800000,
  FEDITOR_PROCESSCTRLQ          = 0x02000000,   // ������ Ctrl-Q � ���� ������� ������� ���� �������
  FEDITOR_DIALOGMEMOEDIT        = 0x80000000,   // Editor ������������ � ������� � �������� DI_MEMOEDIT
};

class Edit;



class Editor:public ScreenObject
{
  friend class DlgEdit;
  friend class FileEditor;
  private:

    /* $ 04.11.2003 SKV
      �� ����� ������ ���� ���� ������ ������ ���������,
      � ��� ��� "�����" (������� 0-� ������), �� ��� ���� ������.
    */
    struct EditorBlockGuard{
      Editor& ed;
      void (Editor::*method)();
      bool needCheckUnmark;
      EditorBlockGuard(Editor& ed,void (Editor::*method)()):ed(ed),method(method),needCheckUnmark(false)
      {
      }
      ~EditorBlockGuard()
      {
        if(needCheckUnmark)(ed.*method)();
      }
    };

	Edit *TopList;
    Edit *EndList;
    Edit *TopScreen;
    Edit *CurLine;

    struct EditorUndoData *UndoData;  // $ 03.12.2001 IS: ������ ���������, �.�. ������ ����� ��������
    int UndoDataPos;
    int UndoSavePos;

    int LastChangeStrPos;
    int NumLastLine;
    int NumLine;
    /* $ 26.02.2001 IS
         ���� �������� ������ ��������� � � ���������� ����� ������������ ���,
         � �� Opt.TabSize
    */
    struct EditorOptions EdOpt;

    int Pasting;
    wchar_t GlobalEOL[10];

    Edit *BlockStart;
    int BlockStartLine;
    Edit *VBlockStart;

    int VBlockX;
    int VBlockSizeX;
    int VBlockY;
    int VBlockSizeY;
    int BlockUndo;

    int MaxRightPos;

    int XX2; //scrollbar

    string strLastSearchStr;
    /* $ 30.07.2000 KM
       ����� ���������� ��� ������ "Whole words"
    */
    int LastSearchCase,LastSearchWholeWords,LastSearchReverse,LastSearchSelFound;
    int SuccessfulSearch; // successful search indicator

    UINT m_codepage; //BUGBUG

    int StartLine;
    int StartChar;

    struct InternalEditorBookMark SavePos;

    struct InternalEditorStackBookMark *StackPos;
    BOOL NewStackPos;

    int EditorID;

    FileEditor *HostFileEditor;

  private:
    virtual void DisplayObject();
    void ShowEditor(int CurLineOnly);
    void DeleteString(Edit *DelPtr,int DeleteLast,int UndoLine);
    void InsertString();
    void Up();
    void Down();
    void ScrollDown();
    void ScrollUp();
    BOOL Search(int Next);

    void GoToLine(int Line);
    void GoToPosition();

    void TextChanged(int State);

    int  CalcDistance(Edit *From, Edit *To,int MaxDist);
    void Paste(const wchar_t *Src=NULL);
    void Copy(int Append);
    void DeleteBlock();
    void UnmarkBlock();
    void UnmarkEmptyBlock();

    void AddUndoData(const wchar_t *Str,const wchar_t *Eol,int StrNum,int StrPos,int Type);
    void Undo();
    void SelectAll();
    //void SetStringsTable();
    void BlockLeft();
    void BlockRight();
    void DeleteVBlock();
    void VCopy(int Append);
    void VPaste(wchar_t *ClipText);
    void VBlockShift(int Left);
    Edit* GetStringByNumber(int DestLine);
    static void EditorShowMsg(const wchar_t *Title,const wchar_t *Msg, const wchar_t* Name);

    int SetBookmark(DWORD Pos);
    int GotoBookmark(DWORD Pos);

    int AddStackBookmark();
    int RestoreStackBookmark();
    int PrevStackBookmark();
    int NextStackBookmark();
    int ClearStackBookmarks();
    InternalEditorStackBookMark* PointerToStackBookmark(int iIdx);
    int DeleteStackBookmark(InternalEditorStackBookMark *sb_delete);
    int GetStackBookmark(int iIdx,EditorBookMarks *Param);
    int GetStackBookmarks(EditorBookMarks *Param);

  public:
    Editor(ScreenObject *pOwner=NULL,bool DialogUsed=false);
    virtual ~Editor();

  public:

    void SetCacheParams (EditorCacheParams *pp);
    void GetCacheParams (EditorCacheParams *pp);

    bool SetCodePage (UINT codepage); //BUGBUG
    UINT GetCodePage (); //BUGBUG

    int ReadData(LPCSTR SrcBuf,int SizeSrcBuf);                  // �������������� �� ������ � ������
    int SaveData(char **DestBuf,int& SizeDestBuf,int TextFormat); // �������������� �� ������ � �����

    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

    void KeepInitParameters();
    void SetStartPos(int LineNum,int CharNum);
    int IsFileModified();
    int IsFileChanged();
    void SetTitle(const wchar_t *Title);
    long GetCurPos();
    int EditorControl(int Command,void *Param);
    void SetHostFileEditor(FileEditor *Editor) {HostFileEditor=Editor;};
    static void SetReplaceMode(int Mode);
    FileEditor *GetHostFileEditor() {return HostFileEditor;};
    void PrepareResizedConsole(){Flags.Set(FEDITOR_ISRESIZEDCONSOLE);}

    void SetTabSize(int NewSize);
    int  GetTabSize(void) const {return EdOpt.TabSize; }

    void SetConvertTabs(int NewMode);
    int  GetConvertTabs(void) const {return EdOpt.ExpandTabs; }

    void SetDelRemovesBlocks(int NewMode);
    int  GetDelRemovesBlocks(void) const {return EdOpt.DelRemovesBlocks; }

    void SetPersistentBlocks(int NewMode);
    int  GetPersistentBlocks(void) const {return EdOpt.PersistentBlocks; }

    void SetAutoIndent(int NewMode) { EdOpt.AutoIndent=NewMode; }
    int  GetAutoIndent(void) const {return EdOpt.AutoIndent; }

    void SetAutoDetectTable(int NewMode) { EdOpt.AutoDetectTable=NewMode; }
    int  GetAutoDetectTable(void) const {return EdOpt.AutoDetectTable; }

    void SetCursorBeyondEOL(int NewMode);
    int  GetCursorBeyondEOL(void) const {return EdOpt.CursorBeyondEOL; }

    void SetBSLikeDel(int NewMode) { EdOpt.BSLikeDel=NewMode; }
    int  GetBSLikeDel(void) const {return EdOpt.BSLikeDel; }

    void SetCharCodeBase(int NewMode) { EdOpt.CharCodeBase=NewMode%3; }
    int  GetCharCodeBase(void) const {return EdOpt.CharCodeBase; }

    void SetReadOnlyLock(int NewMode)  { EdOpt.ReadOnlyLock=NewMode&3; }
    int  GetReadOnlyLock(void) const {return EdOpt.ReadOnlyLock; }

    void SetShowScrollBar(int NewMode){EdOpt.ShowScrollBar=NewMode;}

    void SetWordDiv(const wchar_t *WordDiv) { EdOpt.strWordDiv = WordDiv; }
    const wchar_t *GetWordDiv() { return (const wchar_t*)EdOpt.strWordDiv; }

    void GetSavePosMode(int &SavePos, int &SaveShortPos);

    // ����������� � �������� �������� ��������� "-1" ��� ���������,
    // ������� �� ����� ������
    void SetSavePosMode(int SavePos, int SaveShortPos);

    void GetRowCol(const wchar_t *argv,int *row,int *col);

    int  GetLineCurPos();
    void BeginVBlockMarking();
    void AdjustVBlock(int PrevX);

    void Xlat();
    static void PR_EditorShowMsg(void);

    void FreeAllocatedData(bool FreeUndo=true);

    Edit *CreateString (const wchar_t *lpwszStr, int nLength);
    Edit *InsertString (const wchar_t *lpwszStr, int nLength, Edit *pAfter = NULL);

    void SetDialogParent(DWORD Sets);
    void SetReadOnly(int NewReadOnly) {Flags.Change(FEDITOR_LOCKMODE,NewReadOnly);};
    int  GetReadOnly() {return Flags.Check(FEDITOR_LOCKMODE);};
    void SetOvertypeMode(int Mode);
    int  GetOvertypeMode();
    void SetEditBeyondEnd(int Mode);
    void SetClearFlag(int Flag);
    int  GetClearFlag(void);

    int  GetCurCol();
    int  GetCurRow(){return NumLine;};
    void SetCurPos(int NewCol, int NewRow=-1);
    void SetCursorType(int Visible,int Size);
    void GetCursorType(int &Visible,int &Size);
    void SetObjectColor(int Color,int SelColor,int ColorUnChanged);
    void DrawScrollbar();
};

#endif // __EDITOR_HPP__
