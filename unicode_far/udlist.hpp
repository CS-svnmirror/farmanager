#pragma once

/*
udlist.hpp

������ ����-����, �������������� ����� ������-�����������. ���� �����, �����
������� ������ �������� �����������, �� ���� ������� ������� ��������� �
�������. ���� ����� ����������� ������ ������ � ������ ���, �� ���������, ���
��� �� �����������, � ������� ������.
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


#include "bitflags.hpp"

enum UDL_FLAGS
{
	// ������ "*.*" � ������ �������� ������ "*", ������ "***" � ������ �������� ������ "*"
	ULF_PACKASTERISKS  =0x00000002,
	// ��������� ���������� ������ ��� ������� ������ �������������
	ULF_PROCESSBRACKETS=0x00000004,
	// ������� ������������� ��������
	ULF_UNIQUE         =0x00000010,
	// ������������� (� ������ ��������)
	ULF_SORT           =0x00000020,
	// �� ������� �������
	ULF_NOTRIM         =0x00000040,
	// �� �������������
	ULF_NOUNQUOTE      =0x00000080,
};


class UserDefinedListItem
{
public:

	UserDefinedListItem():index(0) {}
	~UserDefinedListItem();
	bool operator==(const UserDefinedListItem &rhs) const;
	int operator<(const UserDefinedListItem &rhs) const;
	const string& Get() const {return Str;}
private:
	string Str;
	size_t index;

	friend class UserDefinedList;
};

class UserDefinedList:NonCopyable
{
	public:
		typedef UserDefinedListItem value_type;

		// �� ��������� ������������ ��������� ';' � ',', �
		// ProcessBrackets=AddAsterisk=PackAsterisks=false
		// Unique=Sort=false
		UserDefinedList();

		// ���� ����������� �����������. ��. �������� SetParameters
		UserDefinedList(DWORD Flags, const wchar_t* Separators = nullptr);
		~UserDefinedList() { Free(); }

		// ������� �������-����������� � ��������� ��� ��������� ���������
		// ���������� ������.
		// ���� ����������� �� ������, �� ����������������� ����������� ��
		// ��������� (';' & ',').
		// ���������� false, ���� ���� �� ������������ �������� �������� ���
		// �������� ��������� ������ � ���� �� ������������ �������� ����������
		// �������.
		bool SetParameters(DWORD Flags, const wchar_t* Separators = nullptr);

		// �������������� ������. ��������� ������, ����������� �������������.
		// ���������� false ��� �������.
		bool Set(const string& List, bool AddToList=false);

		// ���������� � ��� ������������� ������
		bool Add(const string& List)
		{
			return Set(List, true);
		}

		std::list<UserDefinedListItem>::iterator begin() {return ItemsList.begin();}
		std::list<UserDefinedListItem>::iterator end() {return ItemsList.end();}
		bool empty() const {return ItemsList.empty();}
		size_t size() const {return ItemsList.size();}

		// ���������� ������
		void Free();

	private:
		bool CheckSeparators() const; // �������� ������������ �� ������������
		void SetDefaultSeparators();
		const wchar_t *Skip(const wchar_t *Str, int &Length, int &RealLength, bool &Error);

		std::list<UserDefinedListItem> ItemsList;
		string strSeparators;
		BitFlags Flags;
};
