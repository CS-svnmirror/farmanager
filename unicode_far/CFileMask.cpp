/*
CFileMask.cpp

�������� ����� ��� ������ � ������� ������. ������������ ����� ������ ���.
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

#include "CFileMask.hpp"
#include "FileMasksProcessor.hpp"
#include "FileMasksWithExclude.hpp"
#include "fn.hpp"
#include "lang.hpp"

const int EXCLUDEMASKSEPARATOR=0x7C; // '|'

////////////////////////

CFileMask::CFileMask()
{
	FileMask=NULL;
}

void CFileMask::Free()
{
	if(FileMask)
		delete FileMask;
	FileMask=NULL;
}

/*
 �������������� ������ �����. ��������� ������, ����������� ������� ��� ������
 � �������. ����������� ��������� ����� ����������, ������� �� �� ��������
 �������� '|' ���������� FALSE ��� ������� (��������, ���� �� ����� ����� ��
 ����� ����� 0).
*/

BOOL CFileMask::Set(const wchar_t *Masks, DWORD Flags)
{
	Free();
	BOOL rc=FALSE;
	int Silent=Flags & FMF_SILENT;
	DWORD flags=0;
	if (Flags & FMF_ADDASTERISK) flags|=FMPF_ADDASTERISK;
	if (Masks && *Masks)
	{
		const wchar_t *pExclude = Masks;
		if (*pExclude == L'/')
		{
			pExclude++;
			while (*pExclude && (*pExclude != L'/' || *(pExclude-1) == L'\\'))
				pExclude++;
			while (*pExclude && *pExclude != EXCLUDEMASKSEPARATOR)
				pExclude++;
			if (*pExclude != EXCLUDEMASKSEPARATOR)
				pExclude = NULL;
		}
		else
		{
			pExclude = wcschr(Masks,EXCLUDEMASKSEPARATOR);
		}

		if (pExclude)
		{
			if(!(Flags&FMF_FORBIDEXCLUDE))
				FileMask=new FileMasksWithExclude;
		}
		else
		{
			FileMask=new FileMasksProcessor;
		}

		if (FileMask)
			rc=FileMask->Set(Masks, flags);

		if(!rc)
			Free();
	}

	if(!Silent && !rc)
		Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning),MSG(MIncorrectMask), MSG(MOk));

	return rc;
}

// ���������� TRUE, ���� ������ ����� ������
BOOL CFileMask::IsEmpty(void)
{
	return FileMask?FileMask->IsEmpty():TRUE;
}

/* �������� ��� ����� �� ������� �����
   ���������� TRUE � ������ ������.
   ���� � ����� ����� ������������.
*/
BOOL CFileMask::Compare(const wchar_t *FileName)
{
	return FileMask?FileMask->Compare(PointToName((wchar_t*)FileName)):FALSE;
}
