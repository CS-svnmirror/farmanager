#pragma once

/*
config.hpp

������������
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

#include "panelctype.hpp"
#include "configdb.hpp"
#include "palette.hpp"

enum
{
	CASR_PANEL  = 0x0001,
	CASR_EDITOR = 0x0002,
	CASR_VIEWER = 0x0004,
	CASR_HELP   = 0x0008,
	CASR_DIALOG = 0x0010,
};

enum ExcludeCmdHistoryType
{
	EXCLUDECMDHISTORY_NOTWINASS    = 0x00000001,  // �� �������� � ������� ������� ���������� Windows
	EXCLUDECMDHISTORY_NOTFARASS    = 0x00000002,  // �� �������� � ������� ������� ���������� ���������� ������
	EXCLUDECMDHISTORY_NOTPANEL     = 0x00000004,  // �� �������� � ������� ������� ���������� � ������
	EXCLUDECMDHISTORY_NOTCMDLINE   = 0x00000008,  // �� �������� � ������� ������� ���������� � ���.������
	//EXCLUDECMDHISTORY_NOTAPPLYCMD   = 0x00000010,  // �� �������� � ������� ������� ���������� �� "Apply Command"
};

enum QUOTEDNAMETYPE
{
	QUOTEDNAME_INSERT         = 0x00000001,            // �������� ��� ������ � ��������� ������, � �������� � ���������
	QUOTEDNAME_CLIPBOARD      = 0x00000002,            // �������� ��� ��������� � ����� ������
};

enum
{
	DMOUSEBUTTON_LEFT = 0x00000001,
	DMOUSEBUTTON_RIGHT = 0x00000002,
};

enum
{
	VMENUCLICK_IGNORE = 0,
	VMENUCLICK_CANCEL = 1,
	VMENUCLICK_APPLY = 2,
};

enum DIZUPDATETYPE
{
	DIZ_NOT_UPDATE,
	DIZ_UPDATE_IF_DISPLAYED,
	DIZ_UPDATE_ALWAYS
};

enum FarPoliciesFlags
{
	FFPOL_MAINMENUSYSTEM        = 0x00000001,
	FFPOL_MAINMENUPANEL         = 0x00000002,
	FFPOL_MAINMENUINTERFACE     = 0x00000004,
	FFPOL_MAINMENULANGUAGE      = 0x00000008,
	FFPOL_MAINMENUPLUGINS       = 0x00000010,
	FFPOL_MAINMENUDIALOGS       = 0x00000020,
	FFPOL_MAINMENUCONFIRMATIONS = 0x00000040,
	FFPOL_MAINMENUPANELMODE     = 0x00000080,
	FFPOL_MAINMENUFILEDESCR     = 0x00000100,
	FFPOL_MAINMENUFOLDERDESCR   = 0x00000200,
	FFPOL_MAINMENUVIEWER        = 0x00000800,
	FFPOL_MAINMENUEDITOR        = 0x00001000,
	FFPOL_MAINMENUCOLORS        = 0x00004000,
	FFPOL_MAINMENUHILIGHT       = 0x00008000,
	FFPOL_MAINMENUSAVEPARAMS    = 0x00020000,

	FFPOL_CREATEMACRO           = 0x00040000,
	FFPOL_USEPSWITCH            = 0x00080000,
	FFPOL_PERSONALPATH          = 0x00100000,
	FFPOL_KILLTASK              = 0x00200000,
	FFPOL_SHOWHIDDENDRIVES      = 0x80000000,
};

struct default_value;

class Option
{
public:
	enum OptionType
	{
		TYPE_BOOLEAN,
		TYPE_BOOLEAN3,
		TYPE_INTEGER,
		TYPE_STRING,
		TYPE_LAST = TYPE_STRING,
	};
	explicit Option(const string& Value):sValue(new string(Value)), ValueChanged(false){}
	explicit Option(const long long Value):iValue(Value), ValueChanged(false){}
	virtual ~Option(){}
	bool Changed() const {return ValueChanged;}
	virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const = 0;
	virtual const string toString() const = 0;
	virtual const string ExInfo() const = 0;
	virtual const OptionType getType() const = 0;
	virtual const string typeToString() const = 0;
	virtual bool IsDefault(const struct FARConfigItem* Holder) const = 0;
	virtual void SetDefault(const struct FARConfigItem* Holder) = 0;
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) = 0;
protected:
	const string& GetString() const {return *sValue;}
	const long long GetInt() const {return iValue;}
	void Set(const string& NewValue) {if(*sValue != NewValue) {*sValue = NewValue; ValueChanged = true;}}
	void Set(const long long NewValue) {if(iValue != NewValue) {iValue = NewValue; ValueChanged = true;}}
	virtual bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default) = 0;
	void Free() {delete sValue;}
private:
	void MakeUnchanged(){ValueChanged = false;}
	union
	{
		string* sValue;
		int iValue;
	};
	bool ValueChanged;
	friend class Options;
};

class BoolOption:public Option
{
public:
	BoolOption():Option(false){}
	BoolOption(const bool& Value):Option(Value){}
	BoolOption(const BoolOption& Value):Option(Value.Get()){}
	BoolOption& operator=(bool Value){Set(Value); return *this;}
	BoolOption& operator=(const BoolOption& Value){Set(Value); return *this;}
	const bool Get() const {return GetInt() != 0;}
	operator bool() const {return GetInt() != 0;}
	bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool Default);
	virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const override;
	virtual const string toString() const override {return Get()? L"true":L"false";}
	virtual const string ExInfo() const override {return L"";}
	virtual const OptionType getType() const override {return TYPE_BOOLEAN;}
	virtual const string typeToString() const override {return L"boolean";}
	virtual bool IsDefault(const struct FARConfigItem* Holder) const override;
	virtual void SetDefault(const struct FARConfigItem* Holder) override;
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
private:
	virtual bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default) override;
};

class Bool3Option:public Option
{
public:
	Bool3Option():Option(0){}
	Bool3Option(const int& Value):Option(Value % 3){}
	Bool3Option(const Bool3Option& Value):Option(Value.Get() % 3){}
	const int Get() const {return GetInt() % 3;}
	Bool3Option& operator=(int Value){Set(Value % 3); return *this;}
	Bool3Option& operator=(const Bool3Option& Value){Set(Value); return *this;}
	Bool3Option& operator--(){Set((GetInt()+2) % 3); return *this;}
	Bool3Option& operator++(){Set((GetInt()+1) % 3); return *this;}
	Bool3Option operator--(int){int Current = GetInt() % 3; Set((Current+2) % 3); return Current;}
	Bool3Option operator++(int){int Current = GetInt() % 3; Set((Current+1) % 3); return Current;}
	operator int() const {return GetInt() % 3;}
	bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, int Default);
	virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const override;
	virtual const string toString() const override { int v = Get(); return v ? (v == 1 ? L"True" : L"Other") : L"False"; }
	virtual const string ExInfo() const override {return L"";}
	virtual const OptionType getType() const override {return TYPE_BOOLEAN3;}
	virtual const string typeToString() const override {return L"3-state";}
	virtual bool IsDefault(const struct FARConfigItem* Holder) const override;
	virtual void SetDefault(const struct FARConfigItem* Holder) override;
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
private:
	virtual bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default) override;
};

class IntOption:public Option
{
public:
	IntOption():Option(0){}
	IntOption(long long Value):Option(Value){}
	IntOption(const IntOption& Value):Option(Value.Get()){}
	const long long Get() const {return GetInt();}
	IntOption& operator=(long long Value){Set(Value); return *this;}
	IntOption& operator=(const IntOption& Value){Set(Value); return *this;}
	IntOption& operator|=(long long Value){Set(GetInt()|Value); return *this;}
	IntOption& operator&=(long long Value){Set(GetInt()&Value); return *this;}
	IntOption& operator%=(long long Value){Set(GetInt()%Value); return *this;}
	IntOption& operator^=(long long Value){Set(GetInt()^Value); return *this;}
	IntOption& operator--(){Set(GetInt()-1); return *this;}
	IntOption& operator++(){Set(GetInt()+1); return *this;}
	IntOption operator--(int){long long Current = GetInt(); Set(Current-1); return Current;}
	IntOption operator++(int){long long Current = GetInt(); Set(Current+1); return Current;}
	operator long long() const {return GetInt();}
	bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string&  ValueName, long long Default);
	virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const override;
	virtual const string toString() const override {FormatString s; s << Get(); return s;}
	virtual const string ExInfo() const override;
	virtual const OptionType getType() const override {return TYPE_INTEGER;}
	virtual const string typeToString() const override {return L"integer";}
	virtual bool IsDefault(const struct FARConfigItem* Holder) const override;
	virtual void SetDefault(const struct FARConfigItem* Holder) override;
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
private:
	virtual bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default) override;
};

class StringOption:public Option
{
public:
	StringOption():Option(L""){}
	StringOption(const StringOption& Value):Option(Value.Get()){}
	StringOption(const string& Value):Option(Value){}
	~StringOption(){Free();}
	const string& Get() const {return GetString();}
	const wchar_t* c_str() const {return GetString().c_str();}
	operator const string&() const {return GetString();}
	void clear() {Set(L"");}
	bool empty() const {return GetString().empty();}
	size_t size() const {return Get().size();}
	wchar_t operator[] (size_t index) const {return Get()[index];}
	StringOption& operator=(const wchar_t* Value) {Set(Value); return *this;}
	StringOption& operator=(const string& Value) {Set(Value); return *this;}
	StringOption& operator=(const StringOption& Value) {Set(Value); return *this;}
	StringOption& operator+=(const string& Value) {Set(Get()+Value); return *this;}
	bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const wchar_t* Default);
	virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName) const override;
	virtual const string toString() const override {return Get();}
	virtual const string ExInfo() const override {return L"";}
	virtual const OptionType getType() const override {return TYPE_STRING;}
	virtual const string typeToString() const override {return L"string";}
	virtual bool IsDefault(const struct FARConfigItem* Holder) const override;
	virtual void SetDefault(const struct FARConfigItem* Holder) override;
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
private:
	virtual bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const default_value* Default) override;
};

class Options
{
	enum farconfig_mode
	{
		cfg_roaming,
		cfg_local,
	};

public:
	struct ViewerOptions;
	struct EditorOptions;

	Options();
	void ShellOptions(int LastCommand, const MOUSE_EVENT_RECORD *MouseEvent);
	void Load();
	void Save(bool Ask);
	bool GetConfigValue(const wchar_t *Key, const wchar_t *Name, string &strValue);
	bool GetConfigValue(size_t Root, const wchar_t* Name, Option::OptionType& Type, Option*& Data);
	bool AdvancedConfig(farconfig_mode Mode = cfg_roaming);
	void LocalViewerConfig(Options::ViewerOptions &ViOptRef) {return ViewerConfig(ViOptRef, true);}
	void LocalEditorConfig(Options::EditorOptions &EdOptRef) {return EditorConfig(EdOptRef, true);}

	struct PanelOptions
	{
		IntOption Type;
		BoolOption Visible;
		IntOption ViewMode;
		IntOption SortMode;
		IntOption SortOrder;
		BoolOption SortGroups;
		BoolOption ShowShortNames;
		BoolOption NumericSort;
		BoolOption CaseSensitiveSort;
		BoolOption SelectedFirst;
		BoolOption DirectoriesFirst;
		StringOption Folder;
		StringOption CurFile;
	};

	struct AutoCompleteOptions
	{
		BoolOption ShowList;
		BoolOption ModalList;
		BoolOption AppendCompletion;

		Bool3Option UseFilesystem;
		Bool3Option UseHistory;
		Bool3Option UsePath;
	};

	struct PluginConfirmation
	{
		Bool3Option OpenFilePlugin;
		BoolOption StandardAssociation;
		BoolOption EvenIfOnlyOnePlugin;
		BoolOption SetFindList;
		BoolOption Prefix;
	};

	struct Confirmation
	{
		BoolOption Copy;
		BoolOption Move;
		BoolOption RO;
		BoolOption Drag;
		BoolOption Delete;
		BoolOption DeleteFolder;
		BoolOption Exit;
		BoolOption Esc;
		BoolOption EscTwiceToInterrupt;
		BoolOption RemoveConnection;
		BoolOption AllowReedit;
		BoolOption HistoryClear;
		BoolOption RemoveSUBST;
		BoolOption RemoveHotPlug;
		BoolOption DetachVHD;
	};

	struct DizOptions
	{
		StringOption strListNames;
		BoolOption ROUpdate;
		IntOption UpdateMode;
		BoolOption SetHidden;
		IntOption StartPos;
		BoolOption AnsiByDefault;
		BoolOption SaveInUTF;
	};

	struct CodeXLAT
	{
		HKL Layouts[10];
		StringOption strLayouts;
		StringOption Rules[3]; // �������:
		// [0] "���� ���������� ������ ���������"
		// [1] "���� ���������� ������ ����������� ������"
		// [2] "���� ���������� ������ �� ���/lat"
		StringOption Table[2]; // [0] non-english �����, [1] english �����
		StringOption strWordDivForXlat;
		IntOption Flags;
		int CurrentLayout;
	};

	struct EditorOptions
	{
		IntOption TabSize;
		IntOption ExpandTabs;
		BoolOption PersistentBlocks;
		BoolOption DelRemovesBlocks;
		BoolOption AutoIndent;
		BoolOption AutoDetectCodePage;
		IntOption DefaultCodePage;
		BoolOption CursorBeyondEOL;
		BoolOption BSLikeDel;
		IntOption CharCodeBase;
		BoolOption SavePos;
		BoolOption SaveShortPos;
		BoolOption F7Rules;
		BoolOption AllowEmptySpaceAfterEof;
		IntOption ReadOnlyLock;
		IntOption UndoSize;
		BoolOption UseExternalEditor;
		IntOption FileSizeLimitLo;
		IntOption FileSizeLimitHi;
		BoolOption ShowKeyBar;
		BoolOption ShowTitleBar;
		BoolOption ShowScrollBar;
		BoolOption EditOpenedForWrite;
		BoolOption SearchSelFound;
		BoolOption SearchCursorAtEnd;
		BoolOption SearchRegexp;
		BoolOption SearchPickUpWord;
		Bool3Option ShowWhiteSpace;

		StringOption strWordDiv;

		BoolOption KeepEOL;
		BoolOption AddUnicodeBOM;
	};

	struct ViewerOptions
	{
		enum
		{
			eMinLineSize = 1*1000,
			eDefLineSize = 10*1000,
			eMaxLineSize = 100*1000
		};

		IntOption  TabSize;
		BoolOption AutoDetectCodePage;
		BoolOption ShowScrollbar;
		BoolOption ShowArrows;
		BoolOption PersistentBlocks;
		BoolOption ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
		BoolOption ViewerWrap; // Wrap=0|WordWarp=1
		BoolOption SavePos;
		BoolOption SaveCodepage;
		BoolOption SaveWrapMode;
		BoolOption SaveShortPos;
		BoolOption UseExternalViewer;
		BoolOption ShowKeyBar;
		IntOption  DefaultCodePage;
		BoolOption ShowTitleBar;
		BoolOption SearchRegexp;
		IntOption  MaxLineSize; // 1000..100000, default=10000
		BoolOption SearchEditFocus; // auto-focus on edit text/hex window
		BoolOption Visible0x00;
		IntOption  ZeroChar;
	};

	struct PoliciesOptions
	{
		IntOption DisabledOptions;  // ������������� ���� ������������
		BoolOption ShowHiddenDrives; // ���������� ������� ���������� �����
	};

	struct DialogsOptions
	{
		BoolOption EditBlock;            // ���������� ����� � ������� �����
		BoolOption EditHistory;          // ��������� � �������?
		BoolOption AutoComplete;         // ��������� ��������������?
		BoolOption EULBsClear;           // = 1 - BS � �������� ��� UnChanged ������ ������� ����� ������ �����, ��� � Del
		IntOption EditLine;             // ����� ���������� � ������ ����� (������ ��� ����... ��������� ��������� ����������)
		IntOption MouseButton;          // ���������� ���������� ������/����� ������ ���� ��� ������ �������� ���� �������
		BoolOption DelRemovesBlocks;
		IntOption CBoxMaxHeight;        // ������������ ������ ������������ ������ (�� ���������=8)
	};

	struct VMenuOptions
	{
		IntOption LBtnClick;
		IntOption RBtnClick;
		IntOption MBtnClick;
	};

	struct CommandLineOptions
	{
		BoolOption EditBlock;
		BoolOption DelRemovesBlocks;
		BoolOption AutoComplete;
		BoolOption UsePromptFormat;
		StringOption strPromptFormat;
	};

	struct NowellOptions
	{
		// ����� ��������� Move ������� R/S/H ��������, ����� �������� - ���������� �������
		BoolOption MoveRO;
	};

	struct ScreenSizes
	{
		// �� ������� ���. �������� ������� ��� ������������ ������
		IntOption DeltaX;
		IntOption DeltaY;
	};

	struct LoadPluginsOptions
	{
		string strCustomPluginsPath;  // ���� ��� ������ ��������, ��������� � /p
		string strPersonalPluginsPath;
		bool MainPluginDir; // true - ������������ ����������� ���� � �������� ��������
		bool PluginsCacheOnly; // set by '/co' switch, not saved
		bool PluginsPersonal;

		BoolOption SilentLoadPlugin;
#ifndef NO_WRAPPER
		BoolOption OEMPluginsSupport;
#endif // NO_WRAPPER
		BoolOption ScanSymlinks;
	};

	struct FindFileOptions
	{
		IntOption FileSearchMode;
		BoolOption FindFolders;
		BoolOption FindSymLinks;
		BoolOption CollectFiles;
		BoolOption UseFilter;
		BoolOption FindAlternateStreams;
		StringOption strSearchInFirstSize;

		StringOption strSearchOutFormat;
		StringOption strSearchOutFormatWidth;
		int OutColumnCount;
		unsigned __int64 OutColumnTypes[PANEL_COLUMNCOUNT];
		int OutColumnWidths[PANEL_COLUMNCOUNT];
		int OutColumnWidthType[PANEL_COLUMNCOUNT];
	};

	struct InfoPanelOptions
	{
		IntOption ComputerNameFormat;
		IntOption UserNameFormat;
		BoolOption ShowPowerStatus;
		StringOption strShowStatusInfo;
		StringOption strFolderInfoFiles;
		BoolOption ShowCDInfo;
	};

	struct TreeOptions
	{
		IntOption MinTreeCount;         // ����������� ���������� ����� ��� ���������� ������ � �����.
		BoolOption AutoChangeFolder;     // ��������� ����� ��� ����������� �� ������
		IntOption TreeFileAttr;       // �������� �������� ��� ������-�������

#if defined(TREEFILE_PROJECT)
		BoolOption LocalDisk;            // ������� ���� ��������� ����� ��� ��������� ������
		BoolOption NetDisk;              // ������� ���� ��������� ����� ��� ������� ������
		BoolOption NetPath;              // ������� ���� ��������� ����� ��� ������� �����
		BoolOption RemovableDisk;        // ������� ���� ��������� ����� ��� ������� ������
		BoolOption CDDisk;               // ������� ���� ��������� ����� ��� CD/DVD/BD/etc ������

		StringOption strLocalDisk;      // ������ ����� �����-�������� ��� ��������� ������
		StringOption strNetDisk;        // ������ ����� �����-�������� ��� ������� ������
		StringOption strNetPath;        // ������ ����� �����-�������� ��� ������� �����
		StringOption strRemovableDisk;  // ������ ����� �����-�������� ��� ������� ������
		StringOption strCDDisk;         // ������ ����� �����-�������� ��� CD/DVD/BD/etc ������

		StringOption strExceptPath;     // ��� ������������� ����� �� �������

		StringOption strSaveLocalPath;  // ���� ��������� ��������� �����
		StringOption strSaveNetPath;    // ���� ��������� ������� �����
#endif
	};

	struct CopyMoveOptions
	{
		BoolOption UseSystemCopy;         // ������������ ��������� ������� �����������
		BoolOption CopyOpened;            // ���������� �������� �� ������ �����
		BoolOption CopyShowTotal;         // �������� ����� ��������� �����������
		BoolOption MultiCopy;             // "��������� �����������������/�����������/�������� ������"
		IntOption CopySecurityOptions; // ��� �������� Move - ��� ������ � ������ "Copy access rights"
		IntOption CopyTimeRule;          // $ 30.01.2001 VVM  ���������� ����� �����������,���������� ����� � ������� ��������
		IntOption BufferSize;
	};

	struct DeleteOptions
	{
		BoolOption DelShowTotal;         // �������� ����� ��������� ��������
	};

	struct MacroOptions
	{
		BoolOption MacroReuseRules; // ������� �� ���� �������� ������������� ����������� ������
		int DisableMacro; // ��������� /m ��� /ma ��� /m....
		// config
		StringOption strKeyMacroCtrlDot, strKeyMacroRCtrlDot; // ��� KEY_CTRLDOT/KEY_RCTRLDOT
		StringOption strKeyMacroCtrlShiftDot, strKeyMacroRCtrlShiftDot; // ��� KEY_CTRLSHIFTDOT/KEY_RCTRLSHIFTDOT
		// internal
		DWORD KeyMacroCtrlDot, KeyMacroRCtrlDot;
		DWORD KeyMacroCtrlShiftDot, KeyMacroRCtrlShiftDot;
		StringOption strMacroCONVFMT; // ������ �������������� double � ������
		StringOption strDateFormat; // ��� $Date
		BoolOption ShowPlayIndicator; // �������� ����� 'P' �� ����� ������������ �������
	};

	struct KnownModulesIDs
	{
		GUID Network;
		StringOption NetworkGuidStr;
		GUID Emenu;
		StringOption EmenuGuidStr;
	};

	struct ExecuteOptions
	{
		BoolOption RestoreCPAfterExecute;
		BoolOption ExecuteUseAppPath;
		BoolOption ExecuteFullTitle;
		BoolOption ExecuteSilentExternal;
		StringOption strExecuteBatchType;
		StringOption strExcludeCmds;
		BoolOption   UseHomeDir; // cd ~
		StringOption strHomeDir; // cd ~
	};

	palette Palette;
	BoolOption Clock;
	BoolOption Mouse;
	BoolOption ShowKeyBar;
	BoolOption ScreenSaver;
	IntOption ScreenSaverTime;
	BoolOption UseVk_oem_x;
	BoolOption ShowHidden;
	BoolOption ShortcutAlwaysChdir;
	BoolOption Highlight;
	BoolOption RightClickSelect;

	BoolOption SelectFolders;
	BoolOption ReverseSort;
	BoolOption SortFolderExt;
	BoolOption DeleteToRecycleBin;
	BoolOption DeleteToRecycleBinKillLink; // ����� ��������� ����� � ������� ������� ��������� ��������.
	IntOption WipeSymbol; // ������ ����������� ��� "ZAP-��������"

	CopyMoveOptions CMOpt;

	DeleteOptions DelOpt;

	BoolOption MultiMakeDir; // ����� �������� ���������� ��������� �� ���� �����

	BoolOption CreateUppercaseFolders;
	BoolOption UseRegisteredTypes;

	BoolOption ViewerEditorClock;
	IntOption OnlyEditorViewerUsed; // =1, ���� ����� ��� /e; =2, ���� ����� ��� /v
	BoolOption SaveViewHistory;
	IntOption ViewHistoryCount;
	IntOption ViewHistoryLifetime;

	StringOption strExternalEditor;
	EditorOptions EdOpt;
	StringOption strExternalViewer;
	ViewerOptions ViOpt;


	StringOption strWordDiv; // $ 03.08.2000 SVS �������������� ���� �� �������
	StringOption strQuotedSymbols;
	IntOption QuotedName;
	BoolOption AutoSaveSetup;
	IntOption ChangeDriveMode;
	BoolOption ChangeDriveDisconnectMode;

	BoolOption SaveHistory;
	IntOption HistoryCount;
	IntOption HistoryLifetime;
	BoolOption SaveFoldersHistory;
	IntOption FoldersHistoryCount;
	IntOption FoldersHistoryLifetime;
	IntOption DialogsHistoryCount;
	IntOption DialogsHistoryLifetime;

	FindFileOptions FindOpt;

	IntOption LeftHeightDecrement;
	IntOption RightHeightDecrement;
	IntOption WidthDecrement;

	BoolOption ShowColumnTitles;
	BoolOption ShowPanelStatus;
	BoolOption ShowPanelTotals;
	BoolOption ShowPanelFree;
	BoolOption PanelDetailedJunction;
	BoolOption ShowUnknownReparsePoint;
	BoolOption HighlightColumnSeparator;
	BoolOption DoubleGlobalColumnSeparator;

	BoolOption ShowPanelScrollbar;
	BoolOption ShowMenuScrollbar;
	BoolOption ShowScreensNumber;
	BoolOption ShowSortMode;
	BoolOption ShowMenuBar;
	IntOption FormatNumberSeparators;
	BoolOption CleanAscii;
	BoolOption NoGraphics;

	Confirmation Confirm;
	PluginConfirmation PluginConfirm;

	DizOptions Diz;

	BoolOption ShellRightLeftArrowsRule;
	PanelOptions LeftPanel;
	PanelOptions RightPanel;
	BoolOption LeftFocus;

	AutoCompleteOptions AutoComplete;

	// ���� ����� ���������� ������������� �� ��������� ������.
	IntOption  AutoUpdateLimit;
	BoolOption AutoUpdateRemoteDrive;

	StringOption strLanguage;
	BoolOption SetIcon;
	BoolOption SetAdminIcon;
	IntOption PanelRightClickRule;
	IntOption PanelCtrlAltShiftRule;
	// ��������� Ctrl-F. ���� = 0, �� ���������� ���� ��� ����, ����� - � ������ ����������� �� ������
	BoolOption PanelCtrlFRule;
	/*
	��������� Ctrl-Alt-Shift
	��� ���������� - ������� ��������:
	0 - Panel
	1 - Edit
	2 - View
	3 - Help
	4 - Dialog
	*/
	IntOption AllCtrlAltShiftRule;

	IntOption CASRule; // 18.12.2003 - ������� ��������� ����� � ������ CAS (������� #1).
	/*
	  ������ ��������� Esc ��� ��������� ������:
	    =1 - �� �������� ��������� � History, ���� ����� Ctrl-E/Ctrl/-X
	         ������ ESC (��������� - ��� VC).
	    =0 - ��������� ��� � ���� - �������� ��������� � History
	*/
	BoolOption CmdHistoryRule;

	IntOption ExcludeCmdHistory;
	BoolOption SubstPluginPrefix; // 1 = ��������������� ������� ������� (��� Ctrl-[ � ��� ��������)
	BoolOption SetAttrFolderRules;

	BoolOption ExceptUsed;
	StringOption strExceptEventSvc;
	/*
	������� �� ���� ������ ��������� ����������
	Alt-����� ��� ����������� ������� � �������� "`-=[]\;',./" �
	�������������� Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt-
	*/
	BoolOption ShiftsKeyRules;
	IntOption CursorSize[4];

	CodeXLAT XLat;

	StringOption ConsoleDetachKey; // ���������� ������ ��� ������ Far'������ ������� �� ����������� ���������������� �������� � ��� �����������.

	StringOption strHelpLanguage;
	BoolOption FullScreenHelp;
	IntOption HelpTabSize;

	IntOption HelpURLRules; // =0 ��������� ����������� ������� URL-����������
	BoolOption HelpSearchRegexp;

	// ���������� ���������� ����� � �� ���������� ������ ���. ��� �������������� "����������" "�������" ������.
	BoolOption RememberLogicalDrives;
	BoolOption FlagPosixSemantics;

	IntOption MsWheelDelta; // ������ �������� ��� ���������
	IntOption MsWheelDeltaView;
	IntOption MsWheelDeltaEdit;
	IntOption MsWheelDeltaHelp;
	// �������������� ���������
	IntOption MsHWheelDelta;
	IntOption MsHWheelDeltaView;
	IntOption MsHWheelDeltaEdit;

	/*
	������� �����:
	    0 - ���� ����������, �� ���������� ������� ����� ��� GetSubstName()
	    1 - ���� ����������, �� ���������� ��� ��������� ��� GetSubstName()
	*/
	IntOption SubstNameRule;

	/* $ 23.05.2001 AltF9
	  + ���� ��������� ������� ��������  ������ ���������� Alt-F9
	       (��������� ������� ������) � ������� ������. �� ��������� - 1.
	    0 - ������������ ��������, ����������� � FAR ������ 1.70 beta 3 �
	       ����, �.�. ������������ 25/50 �����.
	    1 - ������������ ������������������� �������� - ���� FAR Manager
	       ����� ������������� � ����������� �� ����������� ��������� ������
	       ����������� ���� � �������.*/
	BoolOption AltF9;

	BoolOption ClearType;

	Bool3Option PgUpChangeDisk;
	BoolOption ShowDotsInRoot;
	BoolOption ShowCheckingFile;
	BoolOption CloseCDGate;       // ���������������� CD
	BoolOption UpdateEnvironment;

	ExecuteOptions Exec;

	IntOption PluginMaxReadData;
	BoolOption ScanJunction;

	IntOption RedrawTimeout;
	IntOption DelThreadPriority; // ��������� �������� ��������, �� ��������� = THREAD_PRIORITY_NORMAL

	LoadPluginsOptions LoadPlug;

	DialogsOptions Dialogs;
	VMenuOptions VMenu;
	CommandLineOptions CmdLine;
	PoliciesOptions Policies;
	NowellOptions Nowell;
	ScreenSizes ScrSize;
	MacroOptions Macro;

	IntOption FindCodePage;

	TreeOptions Tree;
	InfoPanelOptions InfoPanel;

	BoolOption CPMenuMode;
	StringOption strNoAutoDetectCP;
	// ������������� ����� ������� �������� ����� ��������� �� �������������� nsUniversalDetectorEx.
	// ���������� ��������� ������� �� ����� �� �������, ������� UTF-8 ����� ������������ ���� ����
	// 65001 ����� ������������. ���� UniversalDetector ������ �������� �� ����� ������, ��� �����
	// �������� �� ������������� ANSI ��� OEM, � ����������� �� ��������.
	// ������: L"1250,1252,1253,1255,855,10005,28592,28595,28597,28598,38598,65001"
	// ���� ������ ������ ������� ���������� ������� ������� � UCD ������� �� �����.
	// ���� "-1", �� � ����������� CPMenuMode (Ctrl-H � ���� ������� ������� ���������� UCD ���� �����
	// ���������, ���� ����� ���������� ������ '�������' � ��������� (OEM ANSI) ������� ��������.

	StringOption strTitleAddons;
	StringOption strEditorTitleFormat;
	StringOption strViewerTitleFormat;

	IntOption StoredElevationMode;

	BoolOption StoredWindowMode;

	string ProfilePath;
	string LocalProfilePath;
	string TemplateProfilePath;
	string GlobalUserMenuDir;
	KnownModulesIDs KnownIDs;

	StringOption strBoxSymbols;

	BoolOption SmartFolderMonitor; // def: 0=always monitor panel folder(s), 1=only when FAR has input focus

	int ReadOnlyConfig;
	int UseExceptionHandler;
	int ElevationMode;
	int WindowMode;

	const std::vector<struct PanelViewSettings>& ViewSettings;

private:
	void InitConfig();
	void InitRoamingCFG();
	void InitLocalCFG();
	intptr_t AdvancedConfigDlgProc(class Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void SystemSettings();
	void PanelSettings();
	void InterfaceSettings();
	void DialogSettings();
	void VMenuSettings();
	void CmdlineSettings();
	void SetConfirmations();
	void PluginsManagerSettings();
	void SetDizConfig();
	void ViewerConfig(Options::ViewerOptions &ViOptRef, bool Local = false);
	void EditorConfig(Options::EditorOptions &EdOptRef, bool Local = false);
	void SetFolderInfoFiles();
	void InfoPanelSettings();
	void MaskGroupsSettings();
	void AutoCompleteSettings();
	void TreeSettings();
	void SetFilePanelModes();
	void SetViewSettings(size_t Index, const struct PanelViewSettings* Data);
	void AddViewSettings(size_t Index, const struct PanelViewSettings* Data);
	void DeleteViewSettings(size_t Index);
	void ReadPanelModes();
	void SavePanelModes();

	class farconfig
	{
	public:
		FARConfigItem* begin() const;
		FARConfigItem* end() const;
		const FARConfigItem* cbegin() const;
		const FARConfigItem* cend() const;
		size_t size() const {return m_size;}
		FARConfigItem& operator[](size_t i) const;

		void assign(FARConfigItem* Items, size_t Size) {m_items = Items; m_size = Size;}

		typedef FARConfigItem value_type;

#if defined(_MSC_VER) && _MSC_VER < 1700
		// buggy implementation of begin()/end() in VC10, name "iterator" is hardcoded.
		typedef FARConfigItem* iterator;
#endif
	private:
		FARConfigItem *m_items;
		size_t m_size;
	};

	std::vector<std::pair<GeneralConfig*, farconfig>> Config;
	farconfig_mode CurrentConfig;
	std::vector<struct PanelViewSettings> m_ViewSettings;
	bool m_ViewSettingsChanged;
};
