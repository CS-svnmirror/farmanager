/*
poscache.cpp

��� ������� � ������ ��� viewer/editor
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

#include "poscache.hpp"
#include "udlist.hpp"
#include "registry.hpp"
#include "config.hpp"

#define MSIZE_PARAM            (Opt.MaxPositionCache*sizeof(DWORD64)*5)
#define MSIZE_POSITION         (BOOKMARK_COUNT*Opt.MaxPositionCache*sizeof(DWORD64)*4)

#define PARAM_POS(Pos)         ((Pos)*sizeof(DWORD64)*5)
#define POSITION_POS(Pos,Idx)  ((Pos)*BOOKMARK_COUNT*sizeof(DWORD64)*4+(Idx)*BOOKMARK_COUNT*sizeof(DWORD64))

LPCWSTR EmptyPos=L"0,0,0,0,0,\"$\"";

FilePositionCache::FilePositionCache()
{
	if (!Opt.MaxPositionCache)
	{
		GetRegKey(L"System",L"MaxPositionCache",Opt.MaxPositionCache,MAX_POSITIONS);
	}

	Param=NULL;
	Position=NULL;
	IsMemory=0;
	CurPos=0;
	Names=new string[Opt.MaxPositionCache];

	if (Names != NULL)
	{
		Param=(BYTE*)xf_malloc(MSIZE_PARAM);
		Position=(BYTE*)xf_malloc(MSIZE_POSITION);

		if (Param && Position)
		{
			memset(Param,0,MSIZE_PARAM);
			memset(Position,0xFF,MSIZE_POSITION);
			IsMemory=1;
		}
		else
		{
			if (Param)       { xf_free(Param);       Param=NULL; }

			if (Position)    { xf_free(Position);    Position=NULL; }
		}
	}
}

FilePositionCache::~FilePositionCache()
{
	if (Names)
	{
		delete[] Names;
	}

	if (Param)
	{
		xf_free(Param);
	}

	if (Position)
	{
		xf_free(Position);
	}
}

void FilePositionCache::AddPosition(const wchar_t *Name,PosCache& poscache)
{
	if (!IsMemory)
		return;

	string strFullName;

	if (*Name==L'<')
		strFullName = Name;
	else
		ConvertNameToFull(Name,strFullName);

	int FoundPos, Pos;
	Pos = FoundPos = FindPosition(strFullName);

	if (Pos < 0)
		Pos = CurPos;

	Names[Pos] = strFullName;
	memset(Position+POSITION_POS(Pos,0),0xFF,(BOOKMARK_COUNT*4)*sizeof(DWORD64));
	memcpy(Param+PARAM_POS(Pos),&poscache,sizeof(DWORD64)*5); // ��� �������, ��� � TPosCache?? Param ����� ������ :-)

	for (size_t i=0; i<4; i++)
	{
		if (poscache.Position[i])
		{
			memcpy(Position+POSITION_POS(Pos,i),poscache.Position[i],BOOKMARK_COUNT*sizeof(DWORD64));
		}
	}

	if (FoundPos < 0)
		if (++CurPos>=Opt.MaxPositionCache)
			CurPos=0;
}



bool FilePositionCache::GetPosition(const wchar_t *Name,PosCache& poscache)
{
	bool Result=false;
	if (IsMemory)
	{
		string strFullName;

		if (*Name==L'<')
			strFullName = Name;
		else
			ConvertNameToFull(Name, strFullName);

		int Pos = FindPosition(strFullName);
		//memset(Position+POSITION_POS(CurPos,0),0xFF,(BOOKMARK_COUNT*4)*SizeValue);
		//memcpy(Param+PARAM_POS(CurPos),PosCache,SizeValue*5); // ��� �������, ��� � TPosCache?? Param ����� ������ :-)

		if (Pos >= 0)
		{
			memcpy(&poscache,Param+PARAM_POS(Pos),sizeof(DWORD64)*5); // ��� �������, ��� � TPosCache?? Param ����� ������ :-)

			for (size_t i=0; i<4; i++)
			{
				if (poscache.Position[i])
				{
					memcpy(poscache.Position[i],Position+POSITION_POS(Pos,i),BOOKMARK_COUNT*sizeof(DWORD64));
				}
			}
			Result=true;
		}
		return Result;
	}

	memset(&poscache,0,sizeof(DWORD64)*5); // ��� �������, ��� � TPosCache?? Param ����� ������ :-)
	return FALSE;
}

int FilePositionCache::FindPosition(const wchar_t *FullName)
{
	for (int i=1; i<=Opt.MaxPositionCache; i++)
	{
		int Pos=CurPos-i;

		if (Pos<0)
			Pos+=Opt.MaxPositionCache;

		int CmpRes=0;

		if (Opt.FlagPosixSemantics)
			CmpRes = StrCmp(Names[Pos],FullName);
		else
			CmpRes = StrCmpI(Names[Pos],FullName);

		if (CmpRes == 0)
			return Pos;
	}

	return -1;
}

bool FilePositionCache::Read(const wchar_t *Key)
{
	bool Result=false;
	if (IsMemory)
	{
		BYTE DefPos[8096];
		memset(DefPos,0xff,(BOOKMARK_COUNT*4)*sizeof(DWORD64));

		for (int i=0; i < Opt.MaxPositionCache; i++)
		{
			FormatString strItem;
			strItem<<L"Item"<<i;
			FormatString strShort;
			strShort<<L"Short"<<i;
			GetRegKey(Key,strShort,(LPBYTE)Position+POSITION_POS(i,0),(LPBYTE)DefPos,(BOOKMARK_COUNT*4)*sizeof(DWORD64));
			string strDataStr;
			GetRegKey(Key,strItem,strDataStr,EmptyPos);

			if (!StrCmp(strDataStr,EmptyPos))
			{
				Names[i].Clear();
				memset(Param+PARAM_POS(i),0,sizeof(DWORD64)*5);
			}
			else
			{
				UserDefinedList DataList(0,0,0);

				if (DataList.Set(strDataStr))
				{
					DataList.Reset();
					for(int j=0;const wchar_t *DataPtr=DataList.GetNext();j++)
					{
						if (*DataPtr==L'$')
						{
							Names[i] = DataPtr+1;
						}
						else if (j >= 0 && j <= 4)
						{
							*reinterpret_cast<PDWORD64>(Param+PARAM_POS(i)+j*sizeof(DWORD64))=_wtoi64(DataPtr);
						}
					}
				}
			}
		}
		Result=true;
	}
	return Result;
}


bool FilePositionCache::Save(const wchar_t *Key)
{
	bool Result=false;
	if (IsMemory)
	{
		for (int i=0; i < Opt.MaxPositionCache; i++)
		{
			FormatString strItem;
			strItem<<L"Item"<<i;
			FormatString strShort;
			strShort<<L"Short"<<i;
			int Pos=CurPos+i;
			if (Pos>=Opt.MaxPositionCache)
			{
				Pos-=Opt.MaxPositionCache;
			}
			PDWORD64 Ptr=reinterpret_cast<PDWORD64>(Param+PARAM_POS(Pos));

			//��� ����� ������ ���� ����� � �������, �.�. ��� ����� ��������� �������-�����������
			FormatString strDataStr;
			strDataStr<<Ptr[0]<<L","<<Ptr[1]<<L","<<Ptr[2]<<L","<<Ptr[3]<<L","<<Ptr[4]<<L",\"$"<<Names[Pos]<<L"\"";

			//������ �������?
			if (!StrCmp(strDataStr,EmptyPos))
			{
				DeleteRegValue(Key,strItem);
				continue;
			}

			SetRegKey(Key,strItem,strDataStr);

			if ((Opt.ViOpt.SaveViewerShortPos && Opt.ViOpt.SaveViewerPos) ||
							(Opt.EdOpt.SaveShortPos && Opt.EdOpt.SavePos))
			{
				// ���� �� ������������ ������� �� RCtrl+<N>, �� � �� ���������� ��
				int j=0;
				for (; j < 4; j++)
				{
					if (*reinterpret_cast<PDWORD64>(Position+POSITION_POS(Pos,j)) != POS_NONE)
						break;
				}

				if (j < 4)
					SetRegKey(Key,strShort,Position+POSITION_POS(Pos,0),(BOOKMARK_COUNT*4)*sizeof(DWORD64));
				else
					DeleteRegValue(Key,strShort);
			}
		}
		Result=true;
	}
	return Result;
}
