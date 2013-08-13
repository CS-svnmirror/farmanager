-- Windows API - types, constants and functions - via LuaJIT FFI.
-- Started 2011-10-01 by Shmuel Zeigerman

local ffi = require "ffi"

if jit.arch == "x64" then
ffi.cdef[[
typedef __int64 INT_PTR;
typedef unsigned __int64 UINT_PTR;
typedef __int64 LONG_PTR;
typedef unsigned __int64 ULONG_PTR;
typedef unsigned __int64 HANDLE_PTR;
]]
else -- "x86"
ffi.cdef[[
typedef  int INT_PTR;
typedef  unsigned int UINT_PTR;
typedef  long LONG_PTR;
typedef  unsigned long ULONG_PTR;
typedef unsigned long HANDLE_PTR;
]]
end

ffi.cdef[[
typedef void VOID;
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef DWORD COLORREF;
typedef int BOOL;
typedef ULONG_PTR DWORD_PTR;
typedef wchar_t WCHAR;
typedef void *HANDLE;

typedef struct _GUID
{
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME;

typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT;

enum {
  FILE_ATTRIBUTE_READONLY            = 0x00000001,
  FILE_ATTRIBUTE_HIDDEN              = 0x00000002,
  FILE_ATTRIBUTE_SYSTEM              = 0x00000004,
  FILE_ATTRIBUTE_DIRECTORY           = 0x00000010,
  FILE_ATTRIBUTE_ARCHIVE             = 0x00000020,
  FILE_ATTRIBUTE_DEVICE              = 0x00000040,
  FILE_ATTRIBUTE_NORMAL              = 0x00000080,
  FILE_ATTRIBUTE_TEMPORARY           = 0x00000100,
  FILE_ATTRIBUTE_SPARSE_FILE         = 0x00000200,
  FILE_ATTRIBUTE_REPARSE_POINT       = 0x00000400,
  FILE_ATTRIBUTE_COMPRESSED          = 0x00000800,
  FILE_ATTRIBUTE_OFFLINE             = 0x00001000,
  FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x00002000,
  FILE_ATTRIBUTE_ENCRYPTED           = 0x00004000,
  FILE_ATTRIBUTE_VIRTUAL             = 0x00010000,
  FILE_ATTRIBUTE_VALID_FLAGS         = 0x00017fb7,
  FILE_ATTRIBUTE_VALID_SET_FLAGS     = 0x000031a7,
};

//------------------------------------------------------------------------------
enum {
  LOCALE_USER_DEFAULT   = 0x400,
  LOCALE_SYSTEM_DEFAULT = 0x800,
};

enum {
  NORM_IGNORECASE     = 0x00001,
  NORM_IGNORENONSPACE = 0x00002,
  NORM_IGNORESYMBOLS  = 0x00004,
  SORT_STRINGSORT     = 0x01000,
  NORM_IGNOREKANATYPE = 0x10000,
  NORM_IGNOREWIDTH    = 0x20000,
};

int CompareStringW (/*LCID*/ DWORD Locale, DWORD dwCmpFlags, const wchar_t* lpString1, int cchCount1,
                    const wchar_t* lpString2, int cchCount2);
//------------------------------------------------------------------------------

size_t wcslen(const wchar_t*);
int wcscmp(const wchar_t*, const wchar_t*);
int _wcsicmp(const wchar_t*, const wchar_t*);
int StrCmpLogicalW(const wchar_t*, const wchar_t*);
]]
