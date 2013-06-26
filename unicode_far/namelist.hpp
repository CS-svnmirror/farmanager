#pragma once

/*
namelist.hpp

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

class NamesList
{
	private:
		struct FileName2
		{
			string strName;
			string strShortName;
		};

		struct OneName
		{
			struct FileName2 Value;

			OneName(const string& Name, const string& ShortName)
			{
				Value.strName = Name;
				Value.strShortName = ShortName;
			}

			// ��� ��������������� �������� ��������� ��� � xstrncpy!
			OneName& operator=(const FileName2& rhs)
			{
				Value.strName = rhs.strName;
				Value.strShortName = rhs.strShortName;
				return *this;
			}
		};

		std::list<OneName> Names;
		std::list<OneName>::const_iterator CurrentName;

		string strCurrentDir;

	private:
		void Init();

	public:
		NamesList();
		~NamesList();

	public:
		void AddName(const string& Name,const string& ShortName);
		bool GetNextName(string &strName, string &strShortName);
		bool GetPrevName(string &strName, string &strShortName);
		void SetCurName(const string& Name);
		void MoveData(NamesList &Dest);
		void GetCurDir(string &strDir);
		void SetCurDir(const string& Dir);
};
