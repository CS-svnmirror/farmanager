/*
UnicodeString.hpp

Unicode ��ப�
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

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "UnicodeString.hpp"

size_t UnicodeString::Inflate(size_t nSize, bool bForce)
{
  if ( (m_pData->GetRef() == 1) || bForce )
  {
    m_pData->Inflate(nSize);
  }
  else
  {
    UnicodeStringData *pNewData = new UnicodeStringData(nSize);
    size_t nNewLength = min(m_pData->GetLength(),nSize-1);

    wmemcpy(pNewData->GetData(),m_pData->GetData(),nNewLength);

    pNewData->SetLength(nNewLength);

    m_pData->DecRef();

    m_pData = pNewData;
  }
  return m_pData->GetSize();
}

size_t UnicodeString::GetCharString(char *lpszStr, size_t nLength, UINT CodePage) const
{
  size_t nCopyLength = (nLength <= m_pData->GetLength() ? nLength : m_pData->GetLength());
  WideCharToMultiByte(CodePage,0,m_pData->GetData(),(int)nCopyLength,lpszStr,(int)nCopyLength+1,NULL,NULL);
  lpszStr[nCopyLength] = 0;
  return nCopyLength;
}

const UnicodeString& UnicodeString::SetData(const UnicodeString &strCopy)
{
  if ( strCopy.m_pData != m_pData )
  {
    if (m_pData)
      m_pData->DecRef();
    m_pData = strCopy.m_pData;
    m_pData->AddRef();
  }
  return *this;
}

const UnicodeString& UnicodeString::SetData(const wchar_t *lpwszData)
{
  if (m_pData)
    m_pData->DecRef();
  size_t nLength = StrLength(lpwszData);
  m_pData = new UnicodeStringData(nLength + 1);
  wchar_t *pStr = m_pData->GetData();
  if (pStr)
  {
    wmemcpy(pStr,lpwszData,nLength);
    m_pData->SetLength(nLength);
  }
  return *this;
}

const UnicodeString& UnicodeString::SetData(const char *lpszData, UINT CodePage)
{
  if (m_pData)
    m_pData->DecRef();

  size_t nSize = MultiByteToWideChar(CodePage,0,lpszData,-1,NULL,0);
  m_pData = new UnicodeStringData(nSize);
  MultiByteToWideChar(CodePage,0,lpszData,(int)nSize,m_pData->GetData(),(int)m_pData->GetSize());
  m_pData->SetLength(nSize - 1);
  return *this;
}

const UnicodeString& UnicodeString::Append(const UnicodeString &strAdd)
{
  UnicodeStringData *pAddData = strAdd.m_pData;
  size_t nNewLength = m_pData->GetLength() + pAddData->GetLength();
  Inflate(nNewLength + 1);
  wmemcpy(m_pData->GetData() + m_pData->GetLength(),pAddData->GetData(),pAddData->GetLength());
  m_pData->SetLength(nNewLength);
  return *this;
}

const UnicodeString& UnicodeString::Append(const wchar_t *lpwszAdd)
{
  size_t nAddLength = StrLength(lpwszAdd);
  size_t nNewLength = m_pData->GetLength() + nAddLength;
  Inflate(nNewLength + 1);
  wmemcpy(m_pData->GetData() + m_pData->GetLength(),lpwszAdd,nAddLength);
  m_pData->SetLength(nNewLength);
  return *this;
}

const UnicodeString& UnicodeString::Append(const char *lpszAdd, UINT CodePage)
{
  size_t nAddSize = MultiByteToWideChar(CodePage,0,lpszAdd,-1,NULL,0);
  size_t nNewLength = m_pData->GetLength() + nAddSize - 1;
  Inflate(nNewLength + 1);
  MultiByteToWideChar(CodePage,0,lpszAdd,(int)nAddSize,m_pData->GetData() + m_pData->GetLength(),(int)m_pData->GetSize());
  m_pData->SetLength(nNewLength);
  return *this;
}

const UnicodeString& UnicodeString::operator=(const UnicodeString &strCopy)
{
  return SetData(strCopy);
}

const UnicodeString& UnicodeString::operator=(const char *lpszData)
{
  return SetData(lpszData);
}

const UnicodeString& UnicodeString::operator=(const wchar_t *lpwszData)
{
  return SetData(lpwszData);
}

const UnicodeString& UnicodeString::operator+=(const UnicodeString &strAdd)
{
  return Append(strAdd);
}

const UnicodeString& UnicodeString::operator+=(const char *lpszAdd)
{
  return Append(lpszAdd);
}

const UnicodeString& UnicodeString::operator+=(const wchar_t *lpwszAdd)
{
  return Append(lpwszAdd);
}

const UnicodeString operator+(const UnicodeString &strSrc1, const UnicodeString &strSrc2)
{
  UnicodeString Result(strSrc1);
  Result += strSrc2;
  return Result;
}

const UnicodeString operator+(const UnicodeString &strSrc1, const char *lpszSrc2)
{
  UnicodeString Result(strSrc1);
  Result += lpszSrc2;
  return Result;
}

const UnicodeString operator+(const UnicodeString &strSrc1, const wchar_t *lpwszSrc2)
{
  UnicodeString Result(strSrc1);
  Result += lpwszSrc2;
  return Result;
}

wchar_t *UnicodeString::GetBuffer (int nSize)
{
	Inflate (nSize == -1?m_pData->GetSize():nSize);

	return m_pData->GetData ();
}

void UnicodeString::ReleaseBuffer (int nLength)
{
	if ( nLength != -1 )
		m_pData->SetLength (nLength);
	else
		m_pData->SetLength (StrLength(m_pData->GetData()));
}
