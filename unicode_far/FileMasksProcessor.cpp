/*
FileMasksProcessor.cpp

����� ��� ������ � �������� ������� ������ (�� ����������� ������� �����
����������).
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

#include "FileMasksProcessor.hpp"
#include "fn.hpp"


FileMasksProcessor::FileMasksProcessor():BaseFileMask()
{
	bRE = false;
	m = NULL;
	n = 0;
}

void FileMasksProcessor::Free()
{
	Masks.Free();
	re.CleanStack();
	if (m)
		xf_free(m);
	m = NULL;
	n = 0;
}

/*
 �������������� ������ �����. ��������� ������, ����������� �������.
 ���������� FALSE ��� ������� (��������, ���� ��
 ����� ����� �� ����� ����� 0)
*/

BOOL FileMasksProcessor::Set(const wchar_t *masks, DWORD Flags)
{
	// ������������ ����� �������� �� ������ �������, �� � ����� � �������!
	DWORD flags=ULF_PACKASTERISKS|ULF_PROCESSBRACKETS|ULF_SORT|ULF_UNIQUE;
	if(Flags&FMPF_ADDASTERISK) flags|=ULF_ADDASTERISK;

	bRE = (masks && *masks == L'/');
	if (m)
		xf_free(m);
	m = NULL;
	n = 0;

	if (bRE)
	{
		if (re.Compile(masks))
		{
			n = re.GetBracketsCount();
			m = (SMatch *)xf_malloc(n*sizeof(SMatch));
			if (m == NULL)
			{
				n = 0;
				return FALSE;
			}
			return TRUE;
		}
		return FALSE;
	}

	Masks.SetParameters(L',',L';',flags);
	return Masks.Set(masks);
}

BOOL FileMasksProcessor::IsEmpty(void)
{
	if (bRE)
	{
		return (n ? FALSE : TRUE);
	}

  Masks.Reset();
  return Masks.IsEmpty();
}

/* �������� ��� ����� �� ������� �����
   ���������� TRUE � ������ ������.
   ���� � ����� � FileName �� ������������ */
BOOL FileMasksProcessor::Compare(const wchar_t *FileName)
{
	if (bRE)
	{
		int i = n;
		return (re.Match(FileName,m,i) ? TRUE : FALSE);
	}

	Masks.Reset();
	while(NULL!=(MaskPtr=Masks.GetNext()))
	{
		// SkipPath=FALSE, �.�. � CFileMask ���������� PointToName
		if (CmpName(MaskPtr,FileName, FALSE))
			return TRUE;
	}
	return FALSE;
}
