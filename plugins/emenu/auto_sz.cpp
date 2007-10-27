#include "auto_sz.h"
#include <tchar.h>
#include <cassert>

#ifndef UNICODE
bool auto_sz::s_bOem=false;
#endif

auto_sz::auto_sz()
  : m_sz(NULL)
  , m_bDelete(false)
  , m_nSize(0)
#ifndef UNICODE
  , m_wsz(NULL)
#endif
{
}

auto_sz::auto_sz(LPCTSTR sz)
  : m_sz(NULL)
  , m_bDelete(false)
  , m_nSize(0)
#ifndef UNICODE
  , m_wsz(NULL)
#endif
{
  operator =(sz);
}

auto_sz::auto_sz(LPCTSTR szBuf, size_t nBufLen)
  : m_sz(NULL)
  , m_bDelete(false)
  , m_nSize(0)
#ifndef UNICODE
  , m_wsz(NULL)
#endif
{
  nBufLen++;
  Realloc(nBufLen);
  lstrcpyn(m_sz, szBuf, (int)nBufLen);
}

#ifndef UNICODE
auto_sz::auto_sz(LPCWSTR szw)
  : m_sz(NULL)
  , m_bDelete(false)
  , m_nSize(0)
  , m_wsz(NULL)
{
  operator=(szw);
}
#endif

auto_sz::auto_sz(auto_sz& str)
  : m_sz(NULL)
  , m_bDelete(false)
  , m_nSize(0)
#ifndef UNICODE
  , m_wsz(NULL)
#endif
{
  *this=str;
}

auto_sz::auto_sz(const STRRET& sr, LPCITEMIDLIST piid)
  : m_sz(NULL)
  , m_bDelete(false)
  , m_nSize(0)
#ifndef UNICODE
  , m_wsz(NULL)
#endif
{
  switch (sr.uType)
  {
  case STRRET_CSTR:
#ifndef UNICODE
    operator=(sr.cStr);
    if (s_bOem) Ansi2Oem();
#else
    {
      size_t len = lstrlenA(sr.cStr)+1;
      Realloc(len);
      MultiByteToWideChar(CP_ACP, 0, sr.cStr, -1, *this, (int)len);
    }
#endif
    break;
  case STRRET_WSTR:
    operator=(sr.pOleStr);
    break;
  case STRRET_OFFSET:
    operator=((LPCTSTR)(&piid->mkid)+sr.uOffset);
#ifndef UNICODE
    if (s_bOem) Ansi2Oem();
#endif
    break;
  default:
    assert(0);
  }
}

auto_sz::~auto_sz()
{
  Clear();
}

auto_sz::operator LPTSTR()
{
  return m_sz;
}

auto_sz::operator LPCTSTR()
{
  return m_sz;
}

auto_sz::operator LPCTSTR() const
{
  return m_sz;
}

auto_sz& auto_sz::operator =(LPCTSTR sz)
{
  if (m_sz==sz) return *this;
  Clear();
  if (!sz) return *this;
  return operator+=(sz);
}

#ifndef UNICODE
auto_sz& auto_sz::operator =(LPCWSTR szw)
{
  Clear();
  if (!szw) return *this;
  Realloc(lstrlenW(szw)+1);
  WideCharToMultiByte(s_bOem?CP_OEMCP:CP_ACP, 0, szw, -1, *this, (int)Size()
    , NULL, NULL);
  return *this;
}
#endif

auto_sz& auto_sz::operator =(const auto_sz& str)
{
  if (this!=&str)
  {
    Realloc(str.m_nSize);
    lstrcpy(m_sz, str.m_sz);
  }
  return *this;
}

#ifndef UNICODE
auto_sz& auto_sz::Ansi2Oem()
{
  ::CharToOem(*this, *this);
  return *this;
}

auto_sz& auto_sz::Oem2Ansi()
{
  ::OemToChar(*this, *this);
  return *this;
}
#endif

TCHAR auto_sz::operator[](int nPos)
{
  assert(nPos>=0);
  return m_sz[nPos];
}

#ifndef UNICODE
auto_sz::operator LPOLESTR()
{
  size_t nLen=Len()+1;
  delete[] m_wsz;
  m_wsz=new WCHAR[nLen];
  MultiByteToWideChar(s_bOem?CP_OEMCP:CP_ACP, 0, *this, -1, m_wsz, (int)nLen);
  return m_wsz;
}
#endif

auto_sz::operator void*()
{
  return m_sz;
}

void auto_sz::Alloc(size_t nSize)
{
  Clear();
  if (nSize)
  {
    m_sz=new TCHAR[nSize];
    m_sz[0]=_T('\0');
    m_bDelete=true;
    m_nSize=nSize;
  }
}

size_t auto_sz::Size() const
{
  return m_nSize;
}

void auto_sz::Clear()
{
  if (m_bDelete)
  {
    delete[] m_sz;
  }
  m_bDelete=false;
  m_sz=NULL;
  m_nSize=0;
#ifndef UNICODE
  delete[] m_wsz;
#endif
}

void auto_sz::Realloc(size_t nNewSize)
{
  if (!nNewSize)
  {
    Clear();
    return;
  }
  LPTSTR sz=new TCHAR[nNewSize];
  if (m_sz)
  {
    lstrcpyn(sz, m_sz, (int)nNewSize);
    Clear();
  }
  else
  {
    sz[0]=_T('\0');
  }
  m_sz=sz;
  m_bDelete=true;
  m_nSize=nNewSize;
}

auto_sz& auto_sz::operator +=(LPCTSTR szAdd)
{
  Realloc(lstrlen(m_sz)+lstrlen(szAdd)+1);
  lstrcat(m_sz, szAdd);
  return *this;
}

size_t auto_sz::Len() const
{
  return lstrlen(m_sz);
}

void auto_sz::Trunc(size_t nNewLen)
{
  if (nNewLen<m_nSize) m_sz[nNewLen]=_T('\0');
}

bool auto_sz::operator ==(LPCTSTR sz)
{
  return lstrcmp(m_sz, sz)==0;
}

int auto_sz::CompareNoCase(LPCTSTR sz)
{
  return lstrcmpi(m_sz, sz);
}

bool auto_sz::operator !=(LPCTSTR sz)
{
  return lstrcmp(m_sz, sz)!=0;
}

bool auto_sz::CompareExcluding(LPCTSTR sz, TCHAR chExcl)
{
  LPCTSTR szThis=m_sz;
  while (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE
    , (*szThis==chExcl?++szThis:szThis), 1
    , (*sz==chExcl?++sz:sz), 1)==CSTR_EQUAL)
  {
    if (_T('\0')==*szThis) return true;
    szThis++;
    sz++;
  }
  return false;
}

void auto_sz::RemoveTrailing(TCHAR chExcl)
{
  LPTSTR szThis=m_sz+Len();
  while (--szThis>m_sz && *szThis==chExcl) *szThis=_T('\0');
}

bool auto_sz::IsEmpty()
{
  return !m_sz || _T('\0')==m_sz[0];
}
