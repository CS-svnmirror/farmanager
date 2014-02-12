#pragma once

/*
filemasks.hpp

����� ��� ������ � ������� ������ (����������� ������� ����� ����������).
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

#include  "RegExp.hpp"

enum FM_FLAGS
{
	FMF_SILENT = 1,
};

class filemasks:NonCopyable
{
public:
	filemasks() {}
	~filemasks() {}
	filemasks(filemasks&& rhs) { *this = std::move(rhs); }
	MOVE_OPERATOR_BY_SWAP(filemasks);
	
	void swap(filemasks& rhs) noexcept
	{
		Include.swap(rhs.Include);
		Exclude.swap(rhs.Exclude);
	}

	bool Set(const string& Masks, DWORD Flags = 0);
	bool Compare(const string& Name) const;
	bool empty() const;

	static void ErrorMessage();

private:
	void Free();

	class masks:NonCopyable
	{
	public:
		masks(): bRE(false) {}
		masks(masks&& rhs): bRE(false) { *this = std::move(rhs); }
		~masks() {}
		MOVE_OPERATOR_BY_SWAP(masks);

		void swap(masks& rhs) noexcept
		{
			Masks.swap(rhs.Masks);
			re.swap(rhs.re);
			m.swap(rhs.m);
			std::swap(bRE, rhs.bRE);
		}

		bool Set(const string& Masks);
		bool operator ==(const string& Name) const;
		void Free();
		bool empty() const;

	private:
		std::list<string> Masks;
		std::unique_ptr<RegExp> re;
		std::vector<RegExpMatch> m;
		bool bRE;
	};
	ALLOW_SWAP_ACCESS(masks);

	std::list<masks> Include, Exclude;
};

STD_SWAP_SPEC(filemasks);
STD_SWAP_SPEC(filemasks::masks);
