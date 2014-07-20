#pragma once

/*
exception.cpp

��� ��� ����������
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

enum
{
	FLOG_SYSINFO     = 0x00000001, // ���������� � �������
	FLOG_EXCEPTION   = 0x00000002, // ��� ����������
	FLOG_PLUGIN      = 0x00000004, // ���������� � �������
	FLOG_FARAREA     = 0x00000008, // "��� �� ������ ���������?"
	FLOG_MACRO       = 0x00000010, // �������
	FLOG_RAWDARA     = 0x00000020, // ������������ ������
	FLOG_PLUGINSINFO = 0x80000000, // ���������� � ��������

	FLOG_ALL         = 0xFFFFFFFF
};

template<char c0, char c1, char c2, char c3>
struct MakeFourCC
{
	enum { value = MAKELONG(MAKEWORD(c0, c1), MAKEWORD(c2, c3)) };
};

enum FARRECORDTYPE
{
	RTYPE_PLUGIN       = MakeFourCC<'C','P','L','G'>::value, // ���������� � ������� �������
};

struct RECHEADER          // ��������� �������
{
	DWORD TypeRec;          // ��� ������
	DWORD SizeRec;          // ������ ���������
	RECHEADER *Next; // ��������� ������� � ������
	// Data                 // ������ �������� SizeRec
};

struct SYSINFOHEADER      // ���������� � �������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_SYSINFO
	DWORD SizeRec;          // ������ ������ = sizeof(DUMPHEADER)-sizeof(WORD)*2
	RECHEADER *Next; // ��������� ������� � ������
	DWORD DumpFlags;        // �������������� ����� (���� =0)
	VersionInfo FARVersion;       // ������ FAR Manager � ������� FAR_VERSION
	SYSTEMTIME DumpTime;    // the system time is expressed in Coordinated Universal Time (UTC))
	OSVERSIONINFO WinVer;   // ������ ������
};

struct PLUGINRECORD       // ���������� � �������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_PLUGIN
	DWORD SizeRec;          // ������
	DWORD Reserved1[4];

	// DWORD SysID; GUID

	const wchar_t *ModuleName;

	DWORD Reserved2[2];    // ������ :-)

	DWORD SizeModuleName;
};

struct EXCEPTIONRECORD    // ��� ����������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_EXCEPTION
	DWORD SizeRec;          // ������ ������
	RECHEADER *Next; // ��������� ������� � ������

	EXCEPTION_POINTERS *Exception;
};

struct MACRORECORD        // �������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_MACRO
	DWORD SizeRec;          // ������
	RECHEADER *Next; // ��������� ������� � ������
	WORD  MacroStatus;      // 0 - �� � ������ �����, 1 - Recording, 2 - Executing
	WORD  MacroPos;         // ������� ������� � MacroKeyBuffer
	DWORD MacroFlags;       // ����� - ������� ����� = MACRO_AREA
	DWORD MacroKey;         // ����������� ������������
	DWORD MacroBufferSize;  // ������ ����������������������� MacroKeyBuffer
	// DWORD MacroKeyBuffer[0];// �����-������������������
};

struct FARAREARECORD      // "��� �� ������ ���������?"
{
	DWORD TypeRec;          // ��� ������ = RTYPE_FARAREA
	DWORD SizeRec;          // ������ ������
	RECHEADER *Next; // ��������� ������� � ������
	DWORD ObjectType;       // ��, ��� ���������� CtrlObject->Cp()->GetType()
	COORD ScrWH;            // ������� ������ - ������, ������
};

enum
{
	RAWTYPE_BINARY =0,
	RAWTYPE_STRING   =1,
};

struct RAWDARARECORD      // ������������ ������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_RAWDARA
	DWORD SizeRec;          // ������ ������
	RECHEADER *Next; // ��������� ������� � ������
	DWORD RawFlags;         // �������������� ����� ��� ���������������� :-)
	DWORD RawType;          // ��� ������ = RAWTYPE_BINARY, RAWTYPE_STRING
	DWORD SizeData;         // ������ ������������ ������
	//BYTE Data[0];         // ���� SizeRec=0, �� ����� ���� ����
};


//�������� ��������� ��� �������� ������� � ������
//����� ��������� ������
struct COMBINE_RECORD
{
	RECHEADER Header;
	char      Data[ 1 /*SizeRec*/ ];
};
//�������� ��������� ��� ������� � ����������� ������
//����� ������
struct MACRORECORD_t : public MACRORECORD
{
	DWORD MacroKeyBuffer[ 1 /*MacroBufferSize*/ ];
};
struct PLUGINRECORD_t : public PLUGINRECORD
{
	char ModuleName[ 1 /*SizeModuleName*/ ];
};
struct RAWDARARECORD_t : public RAWDARARECORD
{
	LPBYTE RawDataPtr;  //The pointer to allocated raw data can be placed here
};

struct FARExceptionState
{
	DWORD               StructSize;
	DWORD               Version;       // ������ "��������"

	//FAR additional error info
	char RecomendedDumpFileName[_MAX_PATH];
	const char         *RootKey;

	//FAR error context
	RECHEADER   *Head;
};

typedef BOOL (WINAPI *FARPROCESSEVENT)(FARExceptionState * Context);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp=nullptr,
               class Plugin *Module=nullptr,
               void *RawData=nullptr,DWORD RawDataSize=0,
               DWORD RawDataFlags=0,DWORD RawType=RAWTYPE_BINARY);


// for plugins
DWORD WINAPI xfilter(Plugin *Module, const wchar_t* function, EXCEPTION_POINTERS *xp);

// for Far
inline DWORD WINAPI xfilter(const wchar_t* function, EXCEPTION_POINTERS *xp) { return xfilter(nullptr, function, xp); }

class SException: public std::exception
{
public: 
	SException(int Code, EXCEPTION_POINTERS* Info):m_Code(Code), m_Info(Info) {}
	int GetCode() const { return m_Code; }
	EXCEPTION_POINTERS* GetInfo() const { return m_Info; }

private:
	int m_Code;
	EXCEPTION_POINTERS* m_Info;
};

inline void SETranslator(UINT Code, EXCEPTION_POINTERS* ExceptionInfo)
{
	throw SException(Code, ExceptionInfo);
}

inline void EnableSeTranslation()
{
#ifdef _MSC_VER
	_set_se_translator(SETranslator);
#endif
}
