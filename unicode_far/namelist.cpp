/*
namelist.cpp

������ ���� ������, ������������ � viewer ��� ������� Gray+/Gray-
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

#include "namelist.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"

void NamesList::AddName(const string& Name,const string& ShortName)
{
	Names.emplace_back(std::make_pair(Name, ShortName));
	CurrentName = Names.end();
	--CurrentName;
}

bool NamesList::GetNextName(string &strName, string &strShortName)
{
	++CurrentName;
	if (CurrentName == Names.end())
	{
		--CurrentName;
		return false;
	}

	strName = CurrentName->first;
	strShortName = CurrentName->second;
	return true;
}

bool NamesList::GetPrevName(string &strName, string &strShortName)
{
	if (CurrentName == Names.begin())
		return false;

	--CurrentName;
	strName = CurrentName->first;
	strShortName = CurrentName->second;
	return true;
}

void NamesList::SetCurName(const string& Name)
{
	auto ItemIterator = std::find_if(CONST_RANGE(Names, i) { return i.first == Name; });

	if (ItemIterator != Names.cend())
		CurrentName = ItemIterator;
}

void NamesList::SetCurDir(const string& Dir)
{
	if (StrCmpI(strCurrentDir, Dir) || !TestCurrentDirectory(Dir))
	{
		strCurrentDir = Dir;
		PrepareDiskPath(strCurrentDir);
	}
}
