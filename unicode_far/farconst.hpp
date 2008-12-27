#ifndef __FARCONST_HPP__
#define __FARCONST_HPP__
/*
farconst.hpp

�������� ��� enum, #define, etc
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

#define HelpBeginLink L'<'
#define HelpEndLink L'>'
#define HelpFormatLink L"<%s\\>%s"

template <class T>
inline const T&Min(const T &a, const T &b) { return a<b?a:b; }

template <class T>
inline const T&Max(const T &a, const T &b) { return a>b?a:b; }

#define  NM          260

#define  DEFAULT_SORT_GROUP 10000

#define countof(a) (sizeof(a)/sizeof(a[0]))

// ���� �����
enum {
  NO_BOX,
  SINGLE_BOX,
  SHORT_SINGLE_BOX,
  DOUBLE_BOX,
  SHORT_DOUBLE_BOX
};

enum {
  MSG_WARNING        =0x00000001,
  MSG_ERRORTYPE      =0x00000002,
  MSG_KEEPBACKGROUND =0x00000004,
  MSG_DOWN           =0x00000008,
  MSG_LEFTALIGN      =0x00000010,
  MSG_KILLSAVESCREEN =0x00000020,
};

// ������ � ������������ ������
enum {
  FILETYPE_EXEC,       // Enter
  FILETYPE_VIEW,       // F3
  FILETYPE_EDIT,       // F4
  FILETYPE_ALTEXEC,    // Ctrl-PgDn
  FILETYPE_ALTVIEW,    // Alt-F3
  FILETYPE_ALTEDIT     // Alt-F4
};

enum DIZUPDATETYPE {
  DIZ_NOT_UPDATE,
  DIZ_UPDATE_IF_DISPLAYED,
  DIZ_UPDATE_ALWAYS
};

// *** Macros ***
enum MACRODISABLEONLOAD{
  MDOL_ALL            = 0x80000000, // �������� ��� ������� ��� ��������
  MDOL_AUTOSTART      = 0x00000001, // �������� �������������� �������
};

// ������� �������� �������� (������ ����������) -  �� ����� 0xFF ��������!
enum MACROMODEAREA {
  MACRO_FUNC         =  -3,
  MACRO_CONSTS       =  -2,
  MACRO_VARS         =  -1,

  MACRO_OTHER        =   0, // ����� ����������� ������ � ������, ������������ ����
  MACRO_SHELL        =   1, // �������� ������
  MACRO_VIEWER       =   2, // ���������� ��������� ���������
  MACRO_EDITOR       =   3, // ��������
  MACRO_DIALOG       =   4, // �������
  MACRO_SEARCH       =   5, // ������� ����� � �������
  MACRO_DISKS        =   6, // ���� ������ ������
  MACRO_MAINMENU     =   7, // �������� ����
  MACRO_MENU         =   8, // ������ ����
  MACRO_HELP         =   9, // ������� ������
  MACRO_INFOPANEL    =  10, // �������������� ������
  MACRO_QVIEWPANEL   =  11, // ������ �������� ���������
  MACRO_TREEPANEL    =  12, // ������ ������ �����
  MACRO_FINDFOLDER   =  13, // ����� �����
  MACRO_USERMENU     =  14, // ���� ������������

  MACRO_COMMON,             // �����! - ������ ���� �������������, �.�. ��������� ����� ������ !!!
  MACRO_LAST                // ������ ���� ������ ���������! ������������ � ������
};

enum MACROFLAGS_MFLAGS{
  MFLAGS_MODEMASK            =0x000000FF, // ����� ��� ��������� ������� �������� (������� ������ ����������) �������

  MFLAGS_DISABLEOUTPUT       =0x00000100, // �������� ���������� ������ �� ����� ���������� �������
  MFLAGS_NOSENDKEYSTOPLUGINS =0x00000200, // �� ���������� ������� �� ����� ������/��������������� �������
  MFLAGS_RUNAFTERFARSTARTED  =0x00000400, // ���� ������ ��� ���������� ��� ������ ����
  MFLAGS_RUNAFTERFARSTART    =0x00000800, // ���� ������ ����������� ��� ������ ����

  MFLAGS_EMPTYCOMMANDLINE    =0x00001000, // ���������, ���� ��������� ����� �����
  MFLAGS_NOTEMPTYCOMMANDLINE =0x00002000, // ���������, ���� ��������� ����� �� �����

  MFLAGS_SELECTION           =0x00004000, // ��������:  ���������, ���� ���� ���������
  MFLAGS_NOSELECTION         =0x00008000, // ��������:  ���������, ���� ���� ��� ���������
  MFLAGS_PSELECTION          =0x00010000, // ���������: ���������, ���� ���� ���������
  MFLAGS_PNOSELECTION        =0x00020000, // ���������: ���������, ���� ���� ��� ���������
  MFLAGS_EDITSELECTION       =0x00040000, // ���������, ���� ���� ��������� � ���������
  MFLAGS_EDITNOSELECTION     =0x00080000, // ���������, ���� ���� ��� ��������� � ���������
  MFLAGS_NOFILEPANELS        =0x00100000, // ��������:  ���������, ���� ��� ���������� ������
  MFLAGS_NOPLUGINPANELS      =0x00200000, // ��������:  ���������, ���� ��� �������� ������
  MFLAGS_PNOFILEPANELS       =0x00400000, // ���������: ���������, ���� ��� ���������� ������
  MFLAGS_PNOPLUGINPANELS     =0x00800000, // ���������: ���������, ���� ��� �������� ������
  MFLAGS_NOFOLDERS           =0x01000000, // ��������:  ���������, ���� ������� ������ "����"
  MFLAGS_PNOFOLDERS          =0x02000000, // ���������: ���������, ���� ������� ������ "����"
  MFLAGS_PNOFILES            =0x04000000, // ���������: ���������, ���� ������� ������ "�����"
  MFLAGS_NOFILES             =0x08000000, // ��������:  ���������, ���� ������� ������ "�����"

  MFLAGS_REG_MULTI_SZ        =0x10000000, // REG_MULTI_SZ?
  MFLAGS_REUSEMACRO          =0x20000000, // ��������� ������������� �������� (����� ������� �� �������)
  MFLAGS_NEEDSAVEMACRO       =0x40000000, // ���������� ���� ������ ���������
  MFLAGS_DISABLEMACRO        =0x80000000, // ���� ������ ��������
};


// ���� �������� ��� KeyMacro::GetCurRecord()
enum MACRORECORDANDEXECUTETYPE{
  MACROMODE_NOMACRO          =0,  // �� � ������ �����
  MACROMODE_EXECUTING        =1,  // ����������: ��� �������� ������� ����
  MACROMODE_EXECUTING_COMMON =2,  // ����������: � ��������� ������� ����
  MACROMODE_RECORDING        =3,  // ������: ��� �������� ������� ����
  MACROMODE_RECORDING_COMMON =4,  // ������: � ��������� ������� ����
};

// **************************************************

// for filelist
enum {ARCHIVE_NONE,ARCHIVE_RAR,ARCHIVE_ZIP,ARCHIVE_ARJ,ARCHIVE_LZH};

#define MAX_MSG 5000

enum {
  COLUMN_MARK           = 0x80000000,
  COLUMN_NAMEONLY       = 0x40000000,
  COLUMN_RIGHTALIGN     = 0x20000000,
  COLUMN_FORMATTED      = 0x10000000,
  COLUMN_COMMAS         = 0x08000000,
  COLUMN_THOUSAND       = 0x04000000,
  COLUMN_BRIEF          = 0x02000000,
  COLUMN_MONTH          = 0x01000000,
  COLUMN_FLOATSIZE      = 0x00800000,
  COLUMN_ECONOMIC       = 0x00400000,
  COLUMN_MINSIZEINDEX   = 0x00200000,
  COLUMN_SHOWBYTESINDEX = 0x00100000,
  COLUMN_FULLOWNER      = 0x00080000,

  //MINSIZEINDEX ����� ���� ������ 0, 1, 2 ��� 3 (K,M,G,T)
  COLUMN_MINSIZEINDEX_MASK = 0x00000003,
};

// from plugins.hpp
enum {
  PLUGIN_FARGETFILE,
  PLUGIN_FARGETFILES,
  PLUGIN_FARPUTFILES,
  PLUGIN_FARDELETEFILES,
  PLUGIN_FARMAKEDIRECTORY,
  PLUGIN_FAROTHER
};

enum {
  MODALTREE_ACTIVE  =1,
  MODALTREE_PASSIVE =2,
  MODALTREE_FREE    =3
};

//  +CASR_* ��������� Ctrl-Alt-Shift ��� AllCtrlAltShiftRule
enum {
  CASR_PANEL  = 0x0001,
  CASR_EDITOR = 0x0002,
  CASR_VIEWER = 0x0004,
  CASR_HELP   = 0x0008,
  CASR_DIALOG = 0x0010,
};

enum {
  SYSID_PRINTMANAGER      =0x6E614D50,
  SYSID_NETWORK           =0x5774654E,
};


// ����� ��� ReadDiz()
enum ReadDizFlags {
  RDF_NO_UPDATE         = 0x00000001UL,
};

#define STATUS_STRUCTWRONGFILLED       0xE0001000
#define STATUS_INVALIDFUNCTIONRESULT   0xE0002000

#if defined(__GNUC__)
 #define TRY
 #define EXCEPT(a) if (0)
#else
 #define TRY    __try
 #define EXCEPT __except
#endif

enum {
  // DRIVE_UNKNOWN            = 0,
  // DRIVE_NO_ROOT_DIR        = 1,
  // DRIVE_REMOVABLE          = 2,
  // DRIVE_FIXED              = 3,
  // DRIVE_REMOTE             = 4,
  // DRIVE_CDROM              = 5,
  // DRIVE_RAMDISK            = 6,

  DRIVE_SUBSTITUTE            =15,
  DRIVE_REMOTE_NOT_CONNECTED  =16,
  DRIVE_CD_RW                 =18,
  DRIVE_CD_RWDVD              =19,
  DRIVE_DVD_ROM               =20,
  DRIVE_DVD_RW                =21,
  DRIVE_DVD_RAM               =22,
  DRIVE_USBDRIVE              =40,
  DRIVE_NOT_INIT              =255,
};

enum CDROM_DeviceCaps
{
  CDDEV_CAPS_NONE               = 0x00000000,

  CDDEV_CAPS_READ_CDROM         = 0x00000001,
  CDDEV_CAPS_READ_CDR           = 0x00000002,
  CDDEV_CAPS_READ_CDRW          = 0x00000004,

  CDDEV_CAPS_READ_DVDROM        = 0x00000008,
  CDDEV_CAPS_READ_DVDR          = 0x00000010,
  CDDEV_CAPS_READ_DVDRW         = 0x00000020,
  CDDEV_CAPS_READ_DVDRAM        = 0x00000040,


  CDDEV_CAPS_WRITE_CDR          = 0x00020000,
  CDDEV_CAPS_WRITE_CDRW         = 0x00040000,

  CDDEV_CAPS_WRITE_DVDR         = 0x00100000,
  CDDEV_CAPS_WRITE_DVDRW        = 0x00200000,
  CDDEV_CAPS_WRITE_DVDRAM       = 0x00400000,


  CDDEV_CAPS_GENERIC_CD         = CDDEV_CAPS_READ_CDROM | CDDEV_CAPS_READ_CDR | CDDEV_CAPS_READ_CDRW,
  CDDEV_CAPS_GENERIC_CDRW       = CDDEV_CAPS_GENERIC_CD | CDDEV_CAPS_WRITE_CDR | CDDEV_CAPS_WRITE_CDRW,
  CDDEV_CAPS_GENERIC_DVD        = CDDEV_CAPS_GENERIC_CD | CDDEV_CAPS_READ_DVDROM | CDDEV_CAPS_READ_DVDR | CDDEV_CAPS_READ_DVDRW,
  CDDEV_CAPS_GENERIC_COMBO      = CDDEV_CAPS_GENERIC_DVD | CDDEV_CAPS_WRITE_CDR | CDDEV_CAPS_WRITE_CDRW,
  CDDEV_CAPS_GENERIC_DVDRW      = CDDEV_CAPS_GENERIC_COMBO | CDDEV_CAPS_WRITE_DVDR | CDDEV_CAPS_WRITE_DVDRW,
  CDDEV_CAPS_GENERIC_DVDRAM     = CDDEV_CAPS_GENERIC_DVDRW | CDDEV_CAPS_WRITE_DVDRAM
};


enum {
  SKEY_VK_KEYS           = 0x40000000,
  SKEY_IDLE              = 0x80000000,
  SKEY_NOTMACROS         = 0x00000001,
};

// ��� ������� GetNameAndPassword()
enum FlagsNameAndPassword{
  GNP_USELAST      = 0x00000001UL, // ������������ ��������� ��������� ������
  GNP_NOOEMTOCHAR  = 0x00000002UL, // �� �������������� ��� � ������ OEM->CHAR
};

enum {
    XC_QUIT                = (unsigned long) -777,
    XC_OPEN_ERROR          = 0,
    XC_MODIFIED            = 1,
    XC_NOT_MODIFIED        = 2,
    XC_LOADING_INTERRUPTED = 3,
    XC_EXISTS              = 4,
};

// ������ ������� - ��� ��� ������ History, ��� � ��� ��������!
#define HISTORY_COUNT    64
// ���������� �������� � ���������/������� �� ���� �������
#define BOOKMARK_COUNT   10

#define UnicodeToANSI(src,dst,lendst)  WideCharToMultiByte(CP_ACP,0,(src),-1,(dst),(lendst),NULL,FALSE)
#define UnicodeToOEM(src,dst,lendst)    WideCharToMultiByte(CP_OEMCP,0,(src),-1,(dst),(lendst),NULL,FALSE)
#define ANSIToUnicode(src,dst,lendst)  MultiByteToWideChar(CP_ACP,0,(src),-1,(dst),(lendst))
#define OEMToUnicode(src,dst,lendst)    MultiByteToWideChar(CP_OEMCP,0,(src),-1,(dst),(lendst))

typedef BOOL (WINAPI *PISDEBUGGERPRESENT)(VOID);

#define ADDSPACEFORPSTRFORMESSAGE 16

#define MOUSE_ANY_BUTTON_PRESSED (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED|FROM_LEFT_2ND_BUTTON_PRESSED|FROM_LEFT_3RD_BUTTON_PRESSED|FROM_LEFT_4TH_BUTTON_PRESSED)

enum FFTMODE{
  FFTM_BREAKLONGWORD = 0x00000001,
};

enum CHECKFOLDERCONST{ // for CheckFolder()
  CHKFLD_ERROR     = -2,
  CHKFLD_NOTACCESS = -1,
  CHKFLD_EMPTY     =  0,
  CHKFLD_NOTEMPTY  =  1,
  CHKFLD_NOTFOUND  =  2,
};

typedef union {
  WCHAR UnicodeChar;
  CHAR  AsciiChar;
} CHAR_WCHAR;

// ��� Opt.QuotedName
enum QUOTEDNAMETYPE{
  QUOTEDNAME_INSERT         = 0x00000001,            // �������� ��� ������ � ��������� ������, � �������� � ���������
  QUOTEDNAME_CLIPBOARD      = 0x00000002,            // �������� ��� ��������� � ����� ������
};

enum{
  APIS2OEM,
  APIS2ANSI,
};

enum ExcludeCmdHistoryType{
  EXCLUDECMDHISTORY_NOTWINASS    = 0x00000001,  // �� �������� � ������� ������� ���������� Windows
  EXCLUDECMDHISTORY_NOTFARASS    = 0x00000002,  // �� �������� � ������� ������� ���������� ���������� ������
  EXCLUDECMDHISTORY_NOTPANEL     = 0x00000004,  // �� �������� � ������� ������� ���������� � ������
  EXCLUDECMDHISTORY_NOTCMDLINE   = 0x00000008,  // �� �������� � ������� ������� ���������� � ���.������
};

enum COPYSECURITYOPTIONS{
  CSO_MOVE_SETCOPYSECURITY       = 0x00000001,  // Move: �� ��������� ���������� ����� "Copy access rights"?
  CSO_MOVE_SETINHERITSECURITY    = 0x00000003,  // Move: �� ��������� ���������� ����� "Inherit access rights"?
  CSO_MOVE_SESSIONSECURITY       = 0x00000004,  // Move: ��������� ��������� "access rights" ������ ������?
  CSO_COPY_SETCOPYSECURITY       = 0x00000008,  // Copy: �� ��������� ���������� ����� "Copy access rights"?
  CSO_COPY_SETINHERITSECURITY    = 0x00000018,  // Copy: �� ��������� ���������� ����� "Inherit access rights"?
  CSO_COPY_SESSIONSECURITY       = 0x00000020,  // Copy: ��������� ��������� "access rights" ������ ������?
};

enum GETDIRINFOFLAGS{
  GETDIRINFO_ENHBREAK           =0x00000001,
  GETDIRINFO_DONTREDRAWFRAME    =0x00000002,
  GETDIRINFO_SCANSYMLINK        =0x00000004,
  GETDIRINFO_SCANSYMLINKDEF     =0x00000008,
  GETDIRINFO_USEFILTER          =0x00000010,
};

enum CHECKEDPROPS_TYPE{
  CHECKEDPROPS_ISSAMEDISK,
  CHECKEDPROPS_ISDST_ENCRYPTION,
};

enum SETATTR_RET_CODES
{
  SETATTR_RET_ERROR,
  SETATTR_RET_OK,
  SETATTR_RET_SKIP,
  SETATTR_RET_SKIPALL,
};

#define DMOUSEBUTTON_LEFT   0x00000001
#define DMOUSEBUTTON_RIGHT  0x00000002

#define LANGUAGENAME_SIZE  80

enum ReparsePointTypes
{
	RP_EXACTCOPY,   // ��� �����������/�������� ������, ����� �������������
	RP_HARDLINK,    // ������ ������
	RP_JUNCTION,    // �����
	RP_VOLMOUNT,    // ������������� ���
	RP_SYMLINKFILE, // ����-������, NT>=6
	RP_SYMLINKDIR,  // �������-������, NT>=6
};

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8       0xBFBBEF

enum BOX_DEF_SYMBOLS
{
	BS_X_B0,          // 0xB0
	BS_X_B1,          // 0xB1
	BS_X_B2,          // 0xB2
	BS_V1,            // 0xB3
	BS_R_H1V1,        // 0xB4
	BS_R_H2V1,        // 0xB5
	BS_R_H1V2,        // 0xB6
	BS_RT_H1V2,       // 0xB7
	BS_RT_H2V1,       // 0xB8
	BS_R_H2V2,        // 0xB9
	BS_V2,            // 0xBA
	BS_RT_H2V2,       // 0xBB
	BS_RB_H2V2,       // 0xBC
	BS_RB_H1V2,       // 0xBD
	BS_RB_H2V1,       // 0xBE
	BS_RT_H1V1,       // 0xBF
	BS_LB_H1V1,       // 0x�0
	BS_B_H1V1,        // 0x�1
	BS_T_H1V1,        // 0x�2
	BS_L_H1V1,        // 0x�3
	BS_H1,            // 0x�4
	BS_C_H1V1,        // 0x�5
	BS_L_H2V1,        // 0x�6
	BS_L_H1V2,        // 0x�7
	BS_LB_H2V2,       // 0x�8
	BS_LT_H2V2,       // 0x�9
	BS_B_H2V2,        // 0x�A
	BS_T_H2V2,        // 0x�B
	BS_L_H2V2,        // 0x�C
	BS_H2,            // 0x�D
	BS_C_H2V2,        // 0x�E
	BS_B_H2V1,        // 0x�F
	BS_B_H1V2,        // 0xD0
	BS_T_H2V1,        // 0xD1
	BS_T_H1V2,        // 0xD2
	BS_LB_H1V2,       // 0xD3
	BS_LB_H2V1,       // 0xD4
	BS_LT_H2V1,       // 0xD5
	BS_LT_H1V2,       // 0xD6
	BS_C_H1V2,        // 0xD7
	BS_C_H2V1,        // 0xD8
	BS_RB_H1V1,       // 0xD9
	BS_LT_H1V1,       // 0xDA
	BS_X_DB,          // 0xDB
	BS_X_DC,          // 0xDC
	BS_X_DD,          // 0xDD
	BS_X_DE,          // 0xDE
	BS_X_DF,          // 0xDF
};

enum SetCPFlags
{
	SETCP_NOERROR    = 0x00000000,
	SETCP_WC2MBERROR = 0x00000001,
	SETCP_MB2WCERROR = 0x00000002,
	SETCP_OTHERERROR = 0x10000000,
};

#endif // __FARCONST_HPP__
