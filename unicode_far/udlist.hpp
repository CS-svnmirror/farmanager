#ifndef __UserDefinedList_HPP__
#define __UserDefinedList_HPP__
/*
udlist.hpp

������ ����-����, �������������� ����� ������-�����������. ���� �����, �����
������� ������ �������� �����������, �� ���� ������� ������� ��������� �
�������. ���� ����� ����������� ������ ������ � ������ ���, �� ���������, ���
��� �� �����������, � ������� ������.
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


#include "array.hpp"

enum UDL_FLAGS
{
  ULF_ADDASTERISK    =0x00000001, // ��������� '*' � ����� �������� ������,
                                  // ���� �� �� �������� '?', '*' � '.'
  ULF_PACKASTERISKS  =0x00000002, // ������ "*.*" � ������ �������� ������ "*"
                                  // ������ "***" � ������ �������� ������ "*"
  ULF_PROCESSBRACKETS=0x00000004, // ��������� ���������� ������ ��� �������
                                  // ������ �������������
  ULF_UNIQUE         =0x00000010, // ������� ������������� ��������
  ULF_SORT           =0x00000020, // ������������� (� ������ ��������)
};


class UserDefinedListItem
{
  public:
   unsigned int index;
   wchar_t *Str;
   UserDefinedListItem():index(0), Str(NULL)
   {
   }
   bool operator==(const UserDefinedListItem &rhs) const;
   int operator<(const UserDefinedListItem &rhs) const;
   const UserDefinedListItem& operator=(const UserDefinedListItem &rhs);
   const UserDefinedListItem& operator=(const wchar_t *rhs);
   wchar_t *set(const wchar_t *Src, unsigned int size);
   ~UserDefinedListItem();
};

class UserDefinedList
{
  private:
    TArray<UserDefinedListItem> Array;
    unsigned int CurrentItem;
    WORD Separator1, Separator2;
    BOOL ProcessBrackets, AddAsterisk, PackAsterisks, Unique, Sort;

  private:
    BOOL CheckSeparators() const; // �������� ������������ �� ������������
    void SetDefaultSeparators();
    const wchar_t *Skip(const wchar_t *Str, int &Length, int &RealLength, BOOL &Error);
    static int __cdecl CmpItems(const UserDefinedListItem **el1,
      const UserDefinedListItem **el2);

  private:
    UserDefinedList& operator=(const UserDefinedList& rhs); // ����� ��
    UserDefinedList(const UserDefinedList& rhs); // �������������� �� ���������

  public:
    // �� ��������� ������������ ��������� ';' � ',', �
    // ProcessBrackets=AddAsterisk=PackAsterisks=FALSE
    // Unique=Sort=FALSE
    UserDefinedList();

    // ���� ����������� �����������. ��. �������� SetParameters
    UserDefinedList(WORD separator1, WORD separator2, DWORD Flags);
    ~UserDefinedList() { Free(); }

  public:
    // ������� �������-����������� � ��������� ��� ��������� ���������
    // ���������� ������.
    // ���� ���� �� Separator* ����� 0x00, �� �� ������������ ��� ����������
    // (�.�. � Set)
    // ���� ��� ����������� ����� 0x00, �� ����������������� ����������� ��
    // ��������� (';' & ',').
    // ���� AddAsterisk ����� TRUE, �� � ����� �������� ������ �����
    // ����������� '*', ���� ���� ������� �� �������� '?', '*' � '.'
    // ���������� FALSE, ���� ���� �� ������������ �������� �������� ���
    // �������� ��������� ������ � ���� �� ������������ �������� ����������
    // �������.
    BOOL SetParameters(WORD Separator1, WORD Separator2, DWORD Flags);

    // �������������� ������. ��������� ������, ����������� �������������.
    // ���������� FALSE ��� �������.
    // ����: ���� List==NULL, �� ���������� ������������ ������� ����� ������
    BOOL Set(const wchar_t *List, BOOL AddToList=FALSE);

    // ���������� � ��� ������������� ������
    // ����: ���� NewItem==NULL, �� ���������� ������������ ������� �����
    // ������
    BOOL AddItem(const wchar_t *NewItem)
    {
      return Set(NewItem,TRUE);
    }

    // �������� ����� ������� ������ �� �������
    void Reset(void);

    // ������ ��������� �� ��������� ������� ������ ��� NULL
    const wchar_t *GetNext(void);

    // ���������� ������
    void Free();

    // TRUE, ���� ������ ��������� � ������ ���
    BOOL IsEmpty();

    // ������� ���������� ��������� � ������
    DWORD GetTotal () const { return Array.getSize(); }
};

#endif // __UserDefinedList_HPP
