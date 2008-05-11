/*
registry.cpp

������ � registry
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
#include "global.hpp"
#include "array.hpp"

static LONG CloseRegKey(HKEY hKey);

int CopyKeyTree(const wchar_t *Src,const wchar_t *Dest,const wchar_t *Skip=NULL);
void DeleteFullKeyTree(const wchar_t *KeyName);
static void DeleteKeyTreePart(const wchar_t *KeyName);

static int DeleteCount;

static HKEY hRegRootKey=HKEY_CURRENT_USER;
static HKEY hRegCurrentKey=NULL;
static int RequestSameKey=FALSE;

void SetRegRootKey(HKEY hRootKey)
{
  hRegRootKey=hRootKey;
}


void UseSameRegKey()
{
  CloseSameRegKey();
  RequestSameKey=TRUE;
}


void CloseSameRegKey()
{
  if (hRegCurrentKey!=NULL)
  {
    RegCloseKey(hRegCurrentKey);
    hRegCurrentKey=NULL;
  }
  RequestSameKey=FALSE;
}


LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const wchar_t * const ValueData, int SizeData, DWORD Type)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;

  if((hKey=CreateRegKey(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,Type,(unsigned char *)ValueData,(int)SizeData);
  CloseRegKey(hKey);
  return Ret;
}

LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const wchar_t * const ValueData)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;

  if((hKey=CreateRegKey(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,REG_SZ,(unsigned char *)ValueData,(int)(StrLength(ValueData)+1)*sizeof(wchar_t));
  CloseRegKey(hKey);
  return Ret;
}

LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD ValueData)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;
  if((hKey=CreateRegKey(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  CloseRegKey(hKey);
  return Ret;
}


LONG SetRegKey64(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 ValueData)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;
  if((hKey=CreateRegKey(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,REG_QWORD,(BYTE *)&ValueData,sizeof(ValueData));
  CloseRegKey(hKey);
  return Ret;
}

LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const BYTE *ValueData,DWORD ValueSize)
{
  HKEY hKey;
  LONG Ret=ERROR_SUCCESS;
  if((hKey=CreateRegKey(Key)) != NULL)
    Ret=RegSetValueExW(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize);
  CloseRegKey(hKey);
  return Ret;
}



int GetRegKeySize(const wchar_t *Key,const wchar_t *ValueName)
{
  HKEY hKey=OpenRegKey(Key);
  DWORD QueryDataSize=GetRegKeySize(hKey,ValueName);
  CloseRegKey(hKey);
  return QueryDataSize;
}


int GetRegKeySize(HKEY hKey,const wchar_t *ValueName)
{
  if(hKey)
  {
    BYTE Buffer;
    DWORD Type,QueryDataSize=sizeof(Buffer);
    int ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char *)&Buffer,&QueryDataSize);
    if(ExitCode==ERROR_SUCCESS || ExitCode == ERROR_MORE_DATA)
      return QueryDataSize;
  }
  return 0;
}


/* $ 22.02.2001 SVS
  ��� ��������� ������ (GetRegKey) ���������� �������� � ERROR_MORE_DATA
  ���� ����� �������� ����������� - ������� ������� ���� � ����� ������
*/

int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,string &strValueData,const wchar_t *Default,DWORD *pType)
{
  int ExitCode=!ERROR_SUCCESS;
  HKEY hKey=OpenRegKey(Key);
  if(hKey) // ������� ���������!
  {
    DWORD Type,QueryDataSize=0;

    if ( (ExitCode = RegQueryValueExW (
            hKey,
            ValueName,
            0,
            &Type,
            NULL,
            &QueryDataSize
            )) == ERROR_SUCCESS )
    {
      wchar_t *TempBuffer = strValueData.GetBuffer (QueryDataSize+1); // ...�� ������� ������� ����

      ExitCode = RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char *)TempBuffer,&QueryDataSize);

      strValueData.ReleaseBuffer();
    }
    if(pType)
      *pType=Type;
    CloseRegKey(hKey);
  }
  if (ExitCode!=ERROR_SUCCESS)
  {
    strValueData = Default;
    return(FALSE);
  }
  return(TRUE);
}

/* SVS $ */

int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,int &ValueData,DWORD Default)
{
  int ExitCode=!ERROR_SUCCESS;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type,Size=sizeof(ValueData);
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
    CloseRegKey(hKey);
  }
  if (ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}

int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(Key,ValueName,ValueData,Default);
  return(ValueData);
}

int GetRegKey64(const wchar_t *Key,const wchar_t *ValueName,__int64 &ValueData,unsigned __int64 Default)
{
  int ExitCode=!ERROR_SUCCESS;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type,Size=sizeof(ValueData);
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
    CloseRegKey(hKey);
  }
  if (ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}

__int64 GetRegKey64(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 Default)
{
  __int64 ValueData;
  GetRegKey64(Key,ValueName,ValueData,Default);
  return(ValueData);
}

int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize,DWORD *pType)
{
  int ExitCode=!ERROR_SUCCESS;
  HKEY hKey=OpenRegKey(Key);
  DWORD Required=DataSize;
  if(hKey)
  {
    DWORD Type;
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,ValueData,&Required);
    if(ExitCode == ERROR_MORE_DATA) // ���� ������ �� ����������...
    {
      char *TempBuffer=new char[Required+1]; // ...�� ������� ������� ����
      if(TempBuffer) // ���� � ������� ��� ���������...
      {
        if((ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char *)TempBuffer,&Required)) == ERROR_SUCCESS)
          memcpy(ValueData,TempBuffer,DataSize);  // ��������� ������� ����.
        delete[] TempBuffer;
      }
    }
    if(pType)
      *pType=Type;
    CloseRegKey(hKey);
  }
  if (ExitCode!=ERROR_SUCCESS)
  {
    if (Default!=NULL)
      memcpy(ValueData,Default,DataSize);
    else
      memset(ValueData,0,DataSize);
    return(0);
  }
  return(Required);
}

static string &MkKeyName(const wchar_t *Key, string &strDest)
{
  strDest = Opt.strRegRoot;

  if(*Key)
  {
    if( !strDest.IsEmpty() )
      strDest += L"\\";
    strDest += Key;
  }

  return strDest;
}


HKEY CreateRegKey(const wchar_t *Key)
{
  if (hRegCurrentKey)
    return(hRegCurrentKey);
  HKEY hKey;
  DWORD Disposition;

  string strFullKeyName;
  MkKeyName(Key,strFullKeyName);
  if(RegCreateKeyExW(hRegRootKey,strFullKeyName,0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition) != ERROR_SUCCESS)
    hKey=NULL;
  if (RequestSameKey)
  {
    RequestSameKey=FALSE;
    hRegCurrentKey=hKey;
  }
  return(hKey);
}


HKEY OpenRegKey(const wchar_t *Key)
{
  if (hRegCurrentKey)
    return(hRegCurrentKey);
  HKEY hKey;
  string strFullKeyName;
  MkKeyName(Key,strFullKeyName);
  if (RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,&hKey)!=ERROR_SUCCESS)
  {
    CloseSameRegKey();
    return(NULL);
  }
  if (RequestSameKey)
  {
    RequestSameKey=FALSE;
    hRegCurrentKey=hKey;
  }
  return(hKey);
}


void DeleteRegKey(const wchar_t *Key)
{
  string strFullKeyName;
  MkKeyName(Key,strFullKeyName);
  RegDeleteKeyW(hRegRootKey,strFullKeyName);
}


void DeleteRegValue(const wchar_t *Key,const wchar_t *Value)
{
  HKEY hKey;
  string strFullKeyName;
  MkKeyName(Key,strFullKeyName);
  if (RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_WRITE,&hKey)==ERROR_SUCCESS)
  {
    RegDeleteValueW(hKey,Value);
    CloseRegKey(hKey);
  }
}

void DeleteKeyRecord(const wchar_t *KeyMask,int Position)
{
  string strFullKeyName, strNextFullKeyName;
  string strMaskKeyName;

  MkKeyName(KeyMask, strMaskKeyName);

  while (1)
  {
    strFullKeyName.Format ((const wchar_t*)strMaskKeyName,Position++);
    strNextFullKeyName.Format ((const wchar_t*)strMaskKeyName,Position);
    if (!CopyKeyTree(strNextFullKeyName,strFullKeyName))
    {
      DeleteFullKeyTree(strFullKeyName);
      break;
    }
  }
}

void InsertKeyRecord(const wchar_t *KeyMask,int Position,int TotalKeys)
{
  string strFullKeyName, strPrevFullKeyName;
  string strMaskKeyName;

  MkKeyName(KeyMask,strMaskKeyName);
  for (int CurPos=TotalKeys;CurPos>Position;CurPos--)
  {
    strFullKeyName.Format ((const wchar_t*)strMaskKeyName,CurPos);
    strPrevFullKeyName.Format ((const wchar_t*)strMaskKeyName,CurPos-1);
    if (!CopyKeyTree(strPrevFullKeyName,strFullKeyName))
      break;
  }
  strFullKeyName.Format ((const wchar_t*)strMaskKeyName,Position);
  DeleteFullKeyTree(strFullKeyName);
}


class KeyRecordItem
{
  public:
   int ItemIdx;
   KeyRecordItem() { ItemIdx=0; }
   bool operator==(const KeyRecordItem &rhs) const{
     return ItemIdx == rhs.ItemIdx;
   };
   int operator<(const KeyRecordItem &rhs) const{
     return ItemIdx < rhs.ItemIdx;
   };
   const KeyRecordItem& operator=(const KeyRecordItem &rhs)
   {
     ItemIdx = rhs.ItemIdx;
     return *this;
   };

   ~KeyRecordItem()
   {
   }
};

void RenumKeyRecord(const wchar_t *KeyRoot,const wchar_t *KeyMask,const wchar_t *KeyMask0)
{
  TArray<KeyRecordItem> KAItems;
  KeyRecordItem KItem;
  int CurPos;
  string strRegKey;
  string strFullKeyName, strPrevFullKeyName;
  string strMaskKeyName;
  BOOL Processed=FALSE;

  // ���� ������
  for (CurPos=0;;CurPos++)
  {
    if(!EnumRegKey(KeyRoot,CurPos,strRegKey))
      break;
    KItem.ItemIdx=_wtoi((const wchar_t*)strRegKey+StrLength(KeyMask0));
    if(KItem.ItemIdx != CurPos)
      Processed=TRUE;
    KAItems.addItem(KItem);
  }

  if(Processed)
  {
    KAItems.Sort();

    MkKeyName(KeyMask,strMaskKeyName);
    for(int CurPos=0;;++CurPos)
    {
      KeyRecordItem *Item=KAItems.getItem(CurPos);
      if(!Item)
        break;

      // �������� ������������� CurPos
      strFullKeyName.Format (KeyMask,CurPos);
      if(!CheckRegKey(strFullKeyName))
      {
        strFullKeyName.Format ((const wchar_t*)strMaskKeyName,CurPos);
        strPrevFullKeyName.Format ((const wchar_t*)strMaskKeyName,Item->ItemIdx);
        if (!CopyKeyTree(strPrevFullKeyName,strFullKeyName))
          break;
        DeleteFullKeyTree(strPrevFullKeyName);
      }
    }
  }
}


int CopyKeyTree(const wchar_t *Src,const wchar_t *Dest,const wchar_t *Skip)
{
  HKEY hSrcKey,hDestKey;
  if (RegOpenKeyExW(hRegRootKey,Src,0,KEY_READ,&hSrcKey)!=ERROR_SUCCESS)
    return(FALSE);
  DeleteFullKeyTree(Dest);
  DWORD Disposition;
  if (RegCreateKeyExW(hRegRootKey,Dest,0,NULL,0,KEY_WRITE,NULL,&hDestKey,&Disposition)!=ERROR_SUCCESS)
  {
    CloseRegKey(hSrcKey);
    return(FALSE);
  }

  int I;
  for (I=0;;I++)
  {
    wchar_t ValueName[200],ValueData[1000]; //BUGBUG, dynamic
    DWORD Type,NameSize=sizeof(ValueName),DataSize=sizeof(ValueData);
    if (RegEnumValueW(hSrcKey,I,ValueName,&NameSize,NULL,&Type,(BYTE *)ValueData,&DataSize)!=ERROR_SUCCESS)
      break;
    RegSetValueExW(hDestKey,ValueName,0,Type,(BYTE *)ValueData,DataSize);
  }
  CloseRegKey(hDestKey);
  for (I=0;;I++)
  {
    wchar_t SubkeyName[200]; //BUGBUG, dynamic
    string strSrcKeyName, strDestKeyName;

    DWORD NameSize=sizeof(SubkeyName);

    FILETIME LastWrite;
    if (RegEnumKeyExW(hSrcKey,I,SubkeyName,&NameSize,NULL,NULL,NULL,&LastWrite)!=ERROR_SUCCESS)
      break;

    strSrcKeyName = Src;
    strSrcKeyName += L"\\";
    strSrcKeyName += SubkeyName;
    if (Skip!=NULL)
    {
      bool Found=false;
      const wchar_t *SkipName=Skip;
      while (!Found && *SkipName)
        if (StrCmpI(strSrcKeyName,SkipName)==0)
          Found=true;
        else
          SkipName+=StrLength(SkipName)+1;
      if (Found)
        continue;
    }

    strDestKeyName = Dest;
    strDestKeyName += L"\\";
    strDestKeyName += SubkeyName;
    if (RegCreateKeyExW(hRegRootKey,strDestKeyName,0,NULL,0,KEY_WRITE,NULL,&hDestKey,&Disposition)!=ERROR_SUCCESS)
      break;
    CloseRegKey(hDestKey);
    CopyKeyTree(strSrcKeyName,strDestKeyName);
  }
  CloseRegKey(hSrcKey);
  return(TRUE);
}

void DeleteKeyTree(const wchar_t *KeyName)
{
  string strFullKeyName;
  MkKeyName(KeyName,strFullKeyName);
  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_WINDOWS ||
      RegDeleteKeyW(hRegRootKey,strFullKeyName)!=ERROR_SUCCESS)
    DeleteFullKeyTree(strFullKeyName);
}

void DeleteFullKeyTree(const wchar_t *KeyName)
{
  do
  {
    DeleteCount=0;
    DeleteKeyTreePart(KeyName);
  } while (DeleteCount!=0);
}

void DeleteKeyTreePart(const wchar_t *KeyName)
{
  HKEY hKey;
  if (RegOpenKeyExW(hRegRootKey,KeyName,0,KEY_READ,&hKey)!=ERROR_SUCCESS)
    return;
  for (int I=0;;I++)
  {
    wchar_t SubkeyName[200]; //BUGBUG, dynamic
    string strFullKeyName;
    DWORD NameSize=sizeof(SubkeyName);
    FILETIME LastWrite;
    if (RegEnumKeyExW(hKey,I,SubkeyName,&NameSize,NULL,NULL,NULL,&LastWrite)!=ERROR_SUCCESS)
      break;

    strFullKeyName = KeyName;
    strFullKeyName += L"\\";
    strFullKeyName += SubkeyName;
    DeleteKeyTreePart(strFullKeyName);
  }
  CloseRegKey(hKey);
  if (RegDeleteKeyW(hRegRootKey,KeyName)==ERROR_SUCCESS)
    DeleteCount++;
}


/* 07.03.2001 IS
   �������� ������� ����� � ��� ������, ���� �� �� �������� ������� ����������
   � ���������. ���������� TRUE ��� ������.
*/

int DeleteEmptyKey(HKEY hRoot, const wchar_t *FullKeyName)
{
  HKEY hKey;
  int Exist=RegOpenKeyExW(hRoot,FullKeyName,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS;
  if(Exist)
  {
     int RetCode=FALSE;
     if(hKey)
     {
        FILETIME LastWriteTime;
        wchar_t SubName[512]; //BUGBUG, dynamic
        DWORD SubSize=sizeof(SubName);

        LONG ExitCode=RegEnumKeyExW(hKey,0,SubName,&SubSize,NULL,NULL,NULL,
                                   &LastWriteTime);

        if(ExitCode!=ERROR_SUCCESS)
           ExitCode=RegEnumValueW(hKey,0,SubName,&SubSize,NULL,NULL,NULL, NULL);
        CloseRegKey(hKey);

        if(ExitCode!=ERROR_SUCCESS)
          {
            string strKeyName = FullKeyName;
            wchar_t *pSubKey = strKeyName.GetBuffer ();

            if(NULL!=(pSubKey=wcsrchr(pSubKey,L'\\')))
              {
                 *pSubKey=0;
                 pSubKey++;
                 Exist=RegOpenKeyExW(hRoot,strKeyName,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS; //BUGBUG strKeyName
                 if(Exist && hKey)
                 {
                   RetCode=RegDeleteKeyW(hKey, pSubKey)==ERROR_SUCCESS;
                   CloseRegKey(hKey);
                 }
              }

            strKeyName.ReleaseBuffer ();
          }
     }
     return RetCode;
  }
  return TRUE;
}

int CheckRegKey(const wchar_t *Key)
{
  HKEY hKey;
  string strFullKeyName;
  MkKeyName(Key,strFullKeyName);
  int Exist=RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS;
  CloseRegKey(hKey);
  return(Exist);
}

/* 15.09.2000 IS
   ���������� FALSE, ���� ��������� ���������� �� �������� ������
   ��� ������ ������ ����� ����.
*/
int CheckRegValue(const wchar_t *Key,const wchar_t *ValueName)
{
  int ExitCode=!ERROR_SUCCESS;
  DWORD DataSize=0;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type;
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,NULL,&DataSize);
    CloseRegKey(hKey);
  }
  if (ExitCode!=ERROR_SUCCESS || !DataSize)
    return(FALSE);
  return(TRUE);
}

int EnumRegKey(const wchar_t *Key,DWORD Index,string &strDestName)
{
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    FILETIME LastWriteTime;
    wchar_t SubName[512]; //BUGBUG, dynamic
    DWORD SubSize=sizeof(SubName);
    int ExitCode=RegEnumKeyExW(hKey,Index,SubName,&SubSize,NULL,NULL,NULL,&LastWriteTime);
    CloseRegKey(hKey);
    if (ExitCode==ERROR_SUCCESS)
    {
      string strTempName;
      strTempName = Key;
      if ( !strTempName.IsEmpty() )
        AddEndSlash(strTempName);

      strTempName += SubName;

      strDestName = strTempName; //???
      return(TRUE);
    }
  }
  return(FALSE);
}

int EnumRegValue(const wchar_t *Key,DWORD Index, string &strDestName,LPBYTE SData,DWORD SDataSize,LPDWORD IData,__int64* IData64)
{
  HKEY hKey=OpenRegKey(Key);
  int RetCode=REG_NONE;

  if(hKey)
  {
    wchar_t ValueName[512]; //BUGBUG, dynamic

    while( TRUE )
    {
      DWORD ValSize=sizeof(ValueName);
      DWORD Type=(DWORD)-1;

      if (RegEnumValueW(hKey,Index,ValueName,&ValSize,NULL,&Type,SData,&SDataSize) != ERROR_SUCCESS)
        break;

      RetCode=Type;

      strDestName = ValueName;

      if(Type == REG_SZ)
        break;
      else if(Type == REG_DWORD)
      {
        if(IData)
          *IData=*(DWORD*)SData;
        break;
      }
      else if(Type == REG_QWORD)
      {
        if(IData64)
          *IData64=*(__int64*)SData;
        break;
      }
    }

    CloseRegKey(hKey);
  }
  return RetCode;
}

int EnumRegValueEx(const wchar_t *Key,DWORD Index, string &strDestName, string &strSData, LPDWORD IData,__int64* IData64)
{
  HKEY hKey=OpenRegKey(Key);
  int RetCode=REG_NONE;

  if(hKey)
  {
    wchar_t ValueName[512]; //BUGBUG, dynamic

    while( TRUE )
    {
      DWORD ValSize=sizeof(ValueName);
      DWORD Type=(DWORD)-1;
      DWORD Size = 0;

      if(RegEnumValueW(hKey,Index,ValueName,&ValSize, NULL, &Type, NULL, &Size) != ERROR_SUCCESS)
        break;

      wchar_t *Data = strSData.GetBuffer (Size/sizeof (wchar_t)+1);
      ValSize=sizeof(ValueName); // ����, ����� �������� ERROR_MORE_DATA
      int Ret=RegEnumValueW(hKey,Index,ValueName,&ValSize,NULL,&Type,(LPBYTE)Data,&Size);
      strSData.ReleaseBuffer ();

      if (Ret != ERROR_SUCCESS)
        break;

      RetCode=Type;

      strDestName = ValueName;

      if(Type == REG_SZ)
        break;
      else if(Type == REG_DWORD)
      {
        if(IData)
          *IData=*(DWORD*)(const wchar_t*)strSData;
        break;
      }
      else if(Type == REG_QWORD)
      {
        if(IData64)
          *IData64=*(__int64*)(const wchar_t*)strSData;
        break;
      }
    }

    CloseRegKey(hKey);
  }
  return RetCode;
}



LONG CloseRegKey(HKEY hKey)
{
  if (hRegCurrentKey || !hKey)
    return ERROR_SUCCESS;
  return(RegCloseKey(hKey));
}


int RegQueryStringValueEx (
        HKEY hKey,
        const wchar_t *lpwszValueName,
        string &strData,
        const wchar_t *lpwszDefault
        )
{
    DWORD cbSize = 0;

    int nResult = RegQueryValueExW (
            hKey,
            lpwszValueName,
            NULL,
            NULL,
            NULL,
            &cbSize
            );

    if ( nResult == ERROR_SUCCESS )
    {
        wchar_t *lpwszData = strData.GetBuffer (cbSize+1);

        nResult = RegQueryValueExW (
            hKey,
            lpwszValueName,
            NULL,
            NULL,
            (LPBYTE)lpwszData,
            &cbSize
            );

        strData.ReleaseBuffer ();
    }

    if ( nResult != ERROR_SUCCESS )
        strData = lpwszDefault;

    return nResult;
}

int RegQueryStringValue (
        HKEY hKey,
        const wchar_t *lpwszSubKey,
        string &strData,
        const wchar_t *lpwszDefault
        )
{
    LONG cbSize = 0;

    int nResult = RegQueryValueW (
            hKey,
            lpwszSubKey,
            NULL,
            &cbSize
            );

    if ( nResult == ERROR_SUCCESS )
    {
        wchar_t *lpwszData = strData.GetBuffer (cbSize+1);

        nResult = RegQueryValueW (
            hKey,
            lpwszSubKey,
            (LPWSTR)lpwszData,
            &cbSize
            );

        strData.ReleaseBuffer ();
    }

    if ( nResult != ERROR_SUCCESS )
        strData = lpwszDefault;

    return nResult;
}
