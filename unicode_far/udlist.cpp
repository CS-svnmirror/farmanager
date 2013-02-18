/*
udlist.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "udlist.hpp"

UserDefinedListItem::~UserDefinedListItem()
{
}

bool UserDefinedListItem::operator==(const UserDefinedListItem &rhs) const
{
	return StrCmpI(Str, rhs.Str) == 0;
}

UserDefinedList::UserDefinedList()
{
	SetParameters(0, nullptr);
}

UserDefinedList::UserDefinedList(DWORD Flags, const wchar_t* Separators)
{
	SetParameters(Flags, Separators);
}

void UserDefinedList::SetDefaultSeparators()
{
	strSeparators=L";,";
}

bool UserDefinedList::CheckSeparators() const
{
	return !(
		(!Flags.Check(ULF_NOUNQUOTE) && strSeparators.Contains(L'\"')) ||
		(Flags.Check(ULF_PROCESSBRACKETS) && strSeparators.ContainsAny(L"[]"))
			);
}

bool UserDefinedList::SetParameters(DWORD Flags, const wchar_t* Separators)
{
	Free();
	this->Flags.Set(Flags);
	if (Separators && *Separators)
	{
		strSeparators = Separators;
	}
	else
	{
		SetDefaultSeparators();
	}
	return CheckSeparators();
}

void UserDefinedList::Free()
{
	ItemsList.clear();
}

bool UserDefinedList::Set(const string& List, bool AddToList)
{
	if (AddToList)
	{
		if (List.IsEmpty()) // �����, ������ ���������
			return true;
	}
	else
		Free();

	bool rc=false;

	if (CheckSeparators() && !List.IsEmpty())
	{
		UserDefinedListItem item;
		item.index=ItemsList.size();

		bool Error=false;
		const wchar_t *CurList=List;
		int Length, RealLength;
		while (!Error && CurList && *CurList)
		{
			CurList=Skip(CurList, Length, RealLength, Error);
			if (Length > 0)
			{
				if (Flags.Check(ULF_PACKASTERISKS) && 3==Length && 0==memcmp(CurList, L"*.*", 6))
				{
					item.Str = L"*";
					ItemsList.push_back(item);
				}
				else
				{
					item.Str.Copy(CurList, Length);

					if (Flags.Check(ULF_PACKASTERISKS))
					{
						int i=0;
						bool lastAsterisk=false;

						while (i<Length)
						{
							if (item.Str[i]==L'*')
							{
								if (!lastAsterisk)
									lastAsterisk=true;
								else
								{
									item.Str.Remove(i);
									--i;
								}
							}
							else
								lastAsterisk=false;

							++i;
						}
					}
					ItemsList.push_back(item);
				}

				CurList+=RealLength;
				++item.index;
			}
		}

		rc=true;
	}

	if (rc)
	{
		if (Flags.Check(ULF_UNIQUE|ULF_SORT))
		{
			ItemsList.sort([](const UserDefinedListItem& a, const UserDefinedListItem& b)
			{
				return a.index < b.index;
			});

			if(Flags.Check(ULF_UNIQUE))
			{
				ItemsList.unique([](UserDefinedListItem& a, UserDefinedListItem& b)->bool
				{
					if (a.index > b.index)
						a.index = b.index;
					return a == b;
				});
			}
		}

		size_t n=0;
		std::for_each(RANGE(ItemsList, i)
		{
			i.index = n++;
		});
	}
	else
		Free();

	return rc;
}

const wchar_t *UserDefinedList::Skip(const wchar_t *Str, int &Length, int &RealLength, bool &Error)
{
	Length=RealLength=0;
	Error=false;

	if (!Flags.Check(ULF_NOTRIM))
		while (IsSpace(*Str)) ++Str;

	if (strSeparators.Contains(*Str))
		++Str;

	if (!Flags.Check(ULF_NOTRIM))
		while (IsSpace(*Str)) ++Str;

	if (!*Str) return nullptr;

	const wchar_t *cur=Str;
	bool InBrackets=false, InQoutes = (*cur==L'\"');

	if (!InQoutes) // ���� �� � ��������, �� ��������� ����� ����� � ���� �������
		while (*cur) // �����! �������� *cur ������ ������ ������
		{
			if (Flags.Check(ULF_PROCESSBRACKETS)) // ����� �� ����������� ��� ���������������
			{
				if (*cur==L']')
					InBrackets=false;

				if (*cur==L'[' && nullptr!=wcschr(cur+1, L']'))
					InBrackets=true;
			}

			if (!InBrackets && strSeparators.Contains(*cur))
				break;

			++cur;
		}

	if (!InQoutes || !*cur)
	{
		RealLength=Length=(int)(cur-Str);
		--cur;

		if (!Flags.Check(ULF_NOTRIM))
			while (IsSpace(*cur))
			{
				--Length;
				--cur;
			}

		return Str;
	}

	// �� � �������� - �������� ��� ������ � �� ��������� �������
	++cur;
	const wchar_t *QuoteEnd=wcschr(cur, L'\"');

	if (!QuoteEnd)
	{
		Error=true;
		return nullptr;
	}

	const wchar_t *End=QuoteEnd+1;

	if (!Flags.Check(ULF_NOTRIM))
		while (IsSpace(*End)) ++End;

	if (!*End || strSeparators.Contains(*End))
	{
		if (!Flags.Check(ULF_NOUNQUOTE))
		{
			Length=(int)(QuoteEnd-cur);
			RealLength=(int)(End-cur);
		}
		else
		{
			Length=(int)(End-cur)+1;
			RealLength=Length;
			--cur;
		}
		return cur;
	}

	Error=true;
	return nullptr;
}
