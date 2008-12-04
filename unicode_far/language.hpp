#ifndef __LANGUAGE_HPP__
#define __LANGUAGE_HPP__
/*
language.hpp

������ � LNG-�������
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

#include "farconst.hpp"
#include "unicodestring.hpp"


class VMenu;

class Language
{
  private:
    wchar_t **MsgAddr;
    wchar_t *MsgList;

    char **MsgAddrA; //����������, ��
    char *MsgListA;

    long MsgSize;

    int MsgCount;

    string strMessageFile;

  private:
    void ConvertString(const wchar_t *Src,string &strDest);
    BOOL CheckMsgId(int MsgId);
    void Free();

  public:
    Language();
    ~Language();

  public:
    int Init(const wchar_t *Path,int CountNeed=-1);
    void Close();

    const wchar_t* GetMsg (int nID);
    const char* GetMsgA (int nID);

    static FILE* OpenLangFile(const wchar_t *Path,const wchar_t *Mask,const wchar_t *Language,string &strFileName, UINT &nCodePage, BOOL StrongLang=FALSE, string *pLangName=NULL);
    static int GetLangParam(FILE *SrcFile,const wchar_t *ParamName,string *strParam1, string *strParam2, int nCodePage);
    /*
       �����, ��� ��������� ���������� ��� .Options
        .Options <KeyName>=<Value>
    */
    static int GetOptionsParam(FILE *SrcFile,const wchar_t *KeyName,string &strValue, int nCodePage);
    static int Select(int HelpLanguage,VMenu **MenuPtr);
};

extern Language Lang;

#endif  // __LANGUAGE_HPP__
