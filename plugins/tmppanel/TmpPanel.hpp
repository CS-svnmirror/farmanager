/*
TMPPANEL.HPP

Temporary panel header file

*/

#ifndef __TMPPANEL_HPP__
#define __TMPPANEL_HPP__

#include "plugin.hpp"
#include "CRT/crt.hpp"
#include <shellapi.h>

#include "TmpLng.hpp"
#include "TmpClass.hpp"
#include "TmpCfg.hpp"

#define COMMONPANELSNUMBER 10

typedef struct _MyInitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  DWORD Flags;
  signed char Data;
} MyInitDialogItem;

typedef struct _PluginPanels
{
  PluginPanelItem *Items;
  unsigned int ItemsNumber;
  unsigned int OpenFrom;
} PluginPanels;

extern PluginPanels CommonPanels[COMMONPANELSNUMBER];

extern unsigned int CurrentCommonPanel;

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;

extern int StartupOptFullScreenPanel,StartupOptCommonPanel,StartupOpenFrom;
extern TCHAR PluginRootKey[80];

const TCHAR *GetMsg(int MsgId);
void InitDialogItems(const MyInitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber);

int Config();
void GoToFile(const TCHAR *Target, BOOL AnotherPanel);
void FreePanelItems(PluginPanelItem *Items, DWORD Total);

TCHAR *ParseParam(TCHAR *& str);
void GetOptions(void);
void WFD2FFD(WIN32_FIND_DATA &wfd, FAR_FIND_DATA &ffd);
#ifdef UNICODE
wchar_t* NtPath(const wchar_t* path, wchar_t* buf);
#else
#define NtPath(path, buf) path
#endif

#ifndef UNICODE
#define ExpandEnvStrs   FSF.ExpandEnvironmentStr
#else
#define ExpandEnvStrs   ExpandEnvironmentStrings
#endif

#ifdef UNICODE
#define BOM_UCS2     0xFEFF
#define BOM_UCS2_BE  0xFFFE
#define BOM_UTF8     0xBFBBEF
#endif

#define NT_MAX_PATH 32768

class StrBuf
{
  TCHAR *ptr;
  int len;

private:
  StrBuf(const StrBuf &);
  StrBuf & operator=(const StrBuf &);

public:
  StrBuf() { ptr = NULL; len = 0; }
  StrBuf(int len) { ptr = NULL; Reset(len); }
  void Reset(int len) { if (ptr) free(ptr); ptr = (TCHAR *) malloc(len * sizeof(TCHAR)); *ptr = 0; this->len = len; }
  operator TCHAR*() { return ptr; }
  TCHAR *Ptr() { return ptr; }
  int Size() const { return len; }
  ~StrBuf() { free(ptr); }
};

class PtrGuard
{
  TCHAR *ptr;

private:
  PtrGuard(const PtrGuard &);
  PtrGuard & operator=(const PtrGuard &);

public:
  PtrGuard() { ptr = NULL; }
  PtrGuard(TCHAR *ptr) { this->ptr = ptr; }
  PtrGuard & operator=(TCHAR *ptr) { free(this->ptr); this->ptr = ptr; return *this; }
  operator TCHAR*() { return ptr; }
  TCHAR *Ptr() { return ptr; }
  TCHAR **PtrPtr() { return &ptr; }
  ~PtrGuard() { free(ptr); }
};

#endif /* __TMPPANEL_HPP__ */
