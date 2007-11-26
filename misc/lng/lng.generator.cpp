/*
Copyright (c) 2003 WARP ItSelf
Copyright (c) 2005 WARP ItSelf & Alex Yaroslavsky
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
#include "lng.common.h"

#define VERSION "v1.0 rc10"

void UnquoteIfNeeded (char *lpStr)
{
	int nLen = strlen (lpStr);

	if ( *lpStr != '"' || lpStr[nLen-1] != '"' )
		return;

	int i = 0;

	for (; i < nLen-2; i++)
		lpStr[i] = lpStr[i+1];

	lpStr[i] = 0;
}

struct LanguageEntry {
	char *lpLNGFileName;
	char *lpLNGFileNameTemp;

	char *lpLanguageName;
	char *lpLanguageDescription;

	HANDLE hLNGFile;

	DWORD dwCRC32;
	DWORD dwOldCRC32;
};


bool ReadFromBuffer (
		char *&lpStart,
		char *&lpEnd,
		bool bProcessEscape = true
		)
{
	bool bInQuotes = false;
	bool bEscape = false;

	while ( true )
	{
		while ( *lpStart && ( (*lpStart == ' ') || (*lpStart == '\n') || (*lpStart == '\r') || (*lpStart == '\t') ) )
			lpStart++;

		if ( *lpStart == '#' )
		{
			while ( *lpStart != '\r' && *lpStart != '\n' )
				lpStart++;
		}
		else
			break;
	}

	if ( *lpStart )
	{
		lpEnd = lpStart;

		do {
			if ( bProcessEscape )
			{
				if ( (*lpEnd == '"') && !bEscape )
					bInQuotes = !bInQuotes;

				if ( ((*lpEnd == ' ') || (*lpEnd == '\t')) && !bInQuotes )
					break;

				if ( (*lpEnd == '\\') && !bEscape && bInQuotes )
					bEscape = true;
				else
					bEscape = false;
			}

			if ( (*lpEnd == '\n') || (*lpEnd == '\r') )
				break;

			lpEnd++;

		} while ( *lpEnd );

		return true;
	}

	return false;
}

bool ReadFromBufferEx (
		char *&lpStart,
		char **lpParam,
		bool bProcessEscape = true
		)
{
	char *lpEnd = lpStart;

	if ( ReadFromBuffer (lpStart, lpEnd, bProcessEscape) )
	{
		*lpParam = (char*)malloc (lpEnd-lpStart+1);
		memset (*lpParam, 0, lpEnd-lpStart+1);
		memcpy (*lpParam, lpStart, lpEnd-lpStart);
		lpStart = lpEnd;

		return true;
	}

	return false;
}

bool ReadComments (
		char *&lpRealStart,
		char **lpParam,
		char *lpCmtMark,
		char *lpCmtReplace
		)
{
	char *lpStart = lpRealStart;
	char *lpEnd = lpStart;
	int dwSize = 1;
	int nCmtMarkLen = strlen (lpCmtMark);
	int nCmtReplaceLen = strlen (lpCmtReplace);
	bool bFirst = true;

	*lpParam = NULL;

	while ( true )
	{
		if ( ReadFromBuffer (lpStart, lpEnd, false) )
		{
			if ( strncmp (lpStart, lpCmtMark, nCmtMarkLen) )
				break;

			dwSize += lpEnd-lpStart+2+nCmtReplaceLen-nCmtMarkLen;

			*lpParam = (char*)realloc (*lpParam, dwSize);

			if ( bFirst )
			{
				**lpParam = 0;
				bFirst = false;
			}

			strcat (*lpParam, lpCmtReplace);
			strncat (*lpParam, lpStart+nCmtMarkLen, lpEnd-lpStart-nCmtMarkLen);
			strcat (*lpParam, "\r\n" );

			lpRealStart = lpStart = lpEnd;
		}
		else
			break;
	}

	return ( *lpParam == NULL ? false : true );
}

void SmartWrite (
		HANDLE hFile,
		char *lpStr,
		DWORD *pCRC32
		)
{
	DWORD dwWritten;

	WriteFile (hFile, lpStr, strlen (lpStr), &dwWritten, NULL);

	*pCRC32 = CRC32(*pCRC32, lpStr, dwWritten);
}

char *GetTempName ()
{
	char *lpTempName = (char*)malloc (260);

	GetTempFileName (".", "lngg", 0, lpTempName);

	return lpTempName;
}


int main (int argc, const char* argv[])
{
	printf (".LNG Generator "VERSION"\n\r");
	printf ("Copyright (C) 2003-2005 WARP ItSelf\n\r");
	printf ("Copyright (C) 2005-2006 WARP ItSelf & Alex Yaroslavsky\n\n\r");

	if ( argc < 2 )
	{
		printf ("Usage: generator [options] feed_file\n\r");
		printf ("\nOptions:\n");
		printf ("\t-i filename - optional ini file with update status.\n\r");
		printf ("\t-ol output_path - language files output path.\n\r");
		printf ("\t-oh output_path - header file output path.\n\r");
		printf ("\t-nc - don't write copyright info to generated files.\n\r");
		return 0;
	}

	char *lpIniFileName = NULL;

	char *lpLNGOutputPath = NULL;
	char *lpHOutputPath = NULL;

	bool bWriteCopyright = true;

	if ( argc > 2 )
	{
		for (int i = 1; i < argc-1; i++)
		{
			if ( !lstrcmpi (argv[i],"-nc") )
				bWriteCopyright = false;
			else

			if ( !lstrcmpi (argv[i],"-i") && ++i < argc-1 )
			{
				lpIniFileName = (char*)malloc (260);
				GetFullPathName (argv[i], 260, lpIniFileName, NULL);
			}
			else

			if ( !lstrcmpi (argv[i],"-ol") && ++i < argc-1 )
			{
				lpLNGOutputPath = (char*)malloc (strlen (argv[i])+1);
				strcpy (lpLNGOutputPath, argv[i]);

				UnquoteIfNeeded (lpLNGOutputPath);
			}
			else

			if ( !lstrcmpi (argv[i],"-oh") && ++i < argc-1 )
			{
				lpHOutputPath = (char*)malloc (strlen(argv[i])+1);
				strcpy (lpHOutputPath, argv[i]);

				UnquoteIfNeeded (lpLNGOutputPath);
			}
		}
	}

    char* lpFeedFileName = (char*)malloc (strlen(argv[argc-1])+1);
    strcpy (lpFeedFileName, argv[argc-1]);

    UnquoteIfNeeded (lpFeedFileName);

	HANDLE hFeedFile = CreateFile (
			argv[argc-1],
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
			);

	if ( hFeedFile == INVALID_HANDLE_VALUE )
	{
		printf ("ERROR: Can't open the feed file, exiting.\n\r");
		return 0;
	}

	DWORD dwRead;
	DWORD dwWritten;
	DWORD dwID;

	bool bUTF8 = false;

	ReadFile (hFeedFile, &dwID, 3, &dwRead, NULL);

	bUTF8 = ((dwID & 0x00FFFFFF) == 0xBFBBEF);

	if ( !bUTF8 )
		SetFilePointer (hFeedFile, 0, NULL, FILE_BEGIN);

	bool bUpdate;

	LanguageEntry *pLangEntries;

	DWORD dwHeaderCRC32;
	DWORD dwHeaderOldCRC32;

	char *lpHPPFileName = NULL;
	char *lpHPPFileNameTemp = GetTempName ();

	char *lpFullName = (char*)malloc (260);

	DWORD dwLangs = 0;

	char *lpString = (char*)malloc (1024);

	DWORD dwSize = GetFileSize (hFeedFile, NULL);

	char *pFeedBuffer = (char*)malloc (dwSize+1);
	memset (pFeedBuffer, 0, dwSize+1);

	ReadFile (hFeedFile, pFeedBuffer, dwSize, &dwRead, NULL);

	char *lpStart = pFeedBuffer;

	// read h filename

	ReadFromBufferEx (lpStart, &lpHPPFileName);
	UnquoteIfNeeded (lpHPPFileName);

	// read language count
	char *lpLangs;
	ReadFromBufferEx (lpStart, &lpLangs);
	dwLangs = atol(lpLangs);

	free (lpLangs);

	if ( dwLangs )
	{
		wsprintfA (lpFullName, "%s\\%s", lpHOutputPath?lpHOutputPath:".", lpHPPFileName);

		dwHeaderCRC32 = 0;
		dwHeaderOldCRC32 = ((GetFileAttributes (lpFullName) != INVALID_FILE_ATTRIBUTES) && lpIniFileName) ? GetPrivateProfileInt (
				lpHPPFileName,
				"CRC32",
				0,
				lpIniFileName
				) : 0;

		HANDLE hHFile = CreateFile (
				lpHPPFileNameTemp,
				GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				0,
				NULL
				);

		if ( hHFile != INVALID_HANDLE_VALUE )
		{
				if ( bWriteCopyright )
				{
					sprintf (lpString, "// This C++ include file was generated by .LNG Generator "VERSION"\r\n// Copyright (C) 2003-2005 WARP ItSelf\r\n// Copyright (C) 2005 WARP ItSelf & Alex Yaroslavsky\r\n\r\n");
					SmartWrite (hHFile, lpString, &dwHeaderCRC32);
				}

				pLangEntries = (LanguageEntry*)malloc (dwLangs*sizeof (LanguageEntry));

				// read language names and create .lng files

				for (int i = 0; i < dwLangs; i++)
				{
					ReadFromBufferEx (lpStart, &pLangEntries[i].lpLNGFileName);
					ReadFromBufferEx (lpStart, &pLangEntries[i].lpLanguageName);
					ReadFromBufferEx (lpStart, &pLangEntries[i].lpLanguageDescription);

					UnquoteIfNeeded (pLangEntries[i].lpLanguageName);
					UnquoteIfNeeded (pLangEntries[i].lpLanguageDescription);
					UnquoteIfNeeded (pLangEntries[i].lpLNGFileName);

					wsprintfA (lpFullName, "%s\\%s", lpLNGOutputPath?lpLNGOutputPath:".", pLangEntries[i].lpLNGFileName);

					pLangEntries[i].dwCRC32 = 0;
					pLangEntries[i].dwOldCRC32 = ((GetFileAttributes (lpFullName) != INVALID_FILE_ATTRIBUTES) && lpIniFileName) ? GetPrivateProfileInt (
							pLangEntries[i].lpLNGFileName,
							"CRC32",
							0,
							lpIniFileName
							) : 0;

					pLangEntries[i].lpLNGFileNameTemp = GetTempName ();

					pLangEntries[i].hLNGFile = CreateFile (
							pLangEntries[i].lpLNGFileNameTemp,
							GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL,
							CREATE_ALWAYS,
							0,
							NULL
							);

					if ( pLangEntries[i].hLNGFile == INVALID_HANDLE_VALUE )
						printf ("WARNING: Can't create the language file \"%s\".\n\r", pLangEntries[i].lpLNGFileName);
					else
					{
						if ( bUTF8 )
							WriteFile (pLangEntries[i].hLNGFile, &dwID, 3, & dwWritten, NULL);
					}

					if ( bWriteCopyright )
					{
						sprintf (lpString, "// This .lng file was generated by .LNG Generator "VERSION"\r\n// Copyright (C) 2003-2005 WARP ItSelf\r\n// Copyright (C) 2005 WARP ItSelf & Alex Yaroslavsky\r\n\r\n");
						SmartWrite (pLangEntries[i].hLNGFile, lpString, &pLangEntries[i].dwCRC32);
					}

					sprintf (lpString, ".Language=%s,%s\r\n\r\n", pLangEntries[i].lpLanguageName, pLangEntries[i].lpLanguageDescription);
					SmartWrite (pLangEntries[i].hLNGFile, lpString, &pLangEntries[i].dwCRC32);
				}

				char *lpHHead;
				char *lpHTail;

				if ( ReadComments (lpStart, &lpHHead, "hhead:", "") )
				{
					SmartWrite (hHFile, lpHHead, &dwHeaderCRC32);
					free(lpHHead);
				}

				ReadComments (lpStart, &lpHTail, "htail:", "");

				sprintf (lpString, "enum {\r\n");
				SmartWrite (hHFile, lpString, &dwHeaderCRC32);

				// read strings

				bool bRead = true;

				while ( bRead )
				{
					char *lpMsgID;
					char *lpLNGString;
					char *lpHComments;

					if ( ReadComments(lpStart, &lpHComments, "h:", "") )
					{
						SmartWrite (hHFile, lpHComments, &dwHeaderCRC32);
						free (lpHComments);
					}

					ReadComments(lpStart, &lpHComments, "he:", "");

					bRead = ReadFromBufferEx (lpStart, &lpMsgID);

					if ( bRead )
					{
						char *lpLngComments = NULL;
						char *lpELngComments  = NULL;
						char *lpSpecificLngComments  = NULL;

						sprintf (lpString, "\t%s,\r\n", lpMsgID);
						SmartWrite (hHFile, lpString, &dwHeaderCRC32);

						ReadComments(lpStart, &lpLngComments, "l:", "");
						ReadComments(lpStart, &lpELngComments, "le:", "");

						for (int i = 0; i < dwLangs; i++)
						{
							if ( lpLngComments )
								SmartWrite (pLangEntries[i].hLNGFile, lpLngComments, &pLangEntries[i].dwCRC32);

							if ( ReadComments(lpStart, &lpSpecificLngComments, "ls:", "") )
							{
								SmartWrite (pLangEntries[i].hLNGFile, lpSpecificLngComments, &pLangEntries[i].dwCRC32);
								free (lpSpecificLngComments);
							}

							ReadComments(lpStart, &lpSpecificLngComments, "lse:", "");

							bRead = ReadFromBufferEx (lpStart, &lpLNGString);

							if ( bRead )
							{
								if ( !strncmp (lpLNGString, "upd:", 4) )
								{
									strcpy (lpLNGString, lpLNGString+4);

									printf (
											"WARNING: String %s (ID = %s) of %s language needs update!\n\r",
											lpLNGString,
											lpMsgID,
											pLangEntries[i].lpLanguageName
											);
								}

								sprintf (lpString, "%s\r\n", lpLNGString);
								SmartWrite (pLangEntries[i].hLNGFile, lpString, &pLangEntries[i].dwCRC32);
								free (lpLNGString);
							}

							if ( lpSpecificLngComments )
							{
								SmartWrite (pLangEntries[i].hLNGFile, lpSpecificLngComments, &pLangEntries[i].dwCRC32);
								free (lpSpecificLngComments);
							}

							if ( lpELngComments )
								SmartWrite (pLangEntries[i].hLNGFile, lpELngComments, &pLangEntries[i].dwCRC32);
						}

						free (lpMsgID);

						if ( lpLngComments )
							free (lpLngComments);

						if ( lpELngComments )
							free (lpELngComments);

					}

					if ( lpHComments )
					{
						SmartWrite (hHFile, lpHComments, &dwHeaderCRC32);
						free (lpHComments);
					}
				}

				// write .h file footer

				SetFilePointer (hHFile, -2, NULL, FILE_CURRENT);

				sprintf (lpString, "\r\n\t};\r\n");
				SmartWrite (hHFile, lpString, &dwHeaderCRC32);

				if ( lpHTail )
				{
					SmartWrite (hHFile, lpHTail, &dwHeaderCRC32);
					free (lpHTail);
				}

				// play with CRC

				char *lpCRC32 = (char*)malloc (32);

				for (int i = 0; i < dwLangs; i++)
				{
					CloseHandle (pLangEntries[i].hLNGFile);

					bUpdate = true;

					if ( lpIniFileName )
					{
						if ( pLangEntries[i].dwCRC32 == pLangEntries[i].dwOldCRC32 )
						{
							printf ("INFO: Language file \"%s\" doesn't need to be updated.\r\n", pLangEntries[i].lpLNGFileName);
							bUpdate = false;
						}
						else
						{
							sprintf (lpCRC32, "%d", pLangEntries[i].dwCRC32);
							WritePrivateProfileString (pLangEntries[i].lpLNGFileName, "CRC32", lpCRC32, lpIniFileName);
						}
					}

					if ( bUpdate )
					{
						wsprintfA (lpFullName, "%s\\%s", lpLNGOutputPath?lpLNGOutputPath:".", pLangEntries[i].lpLNGFileName);

						MoveFileEx (
								pLangEntries[i].lpLNGFileNameTemp,
								lpFullName,
								MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED
								);
					}

					DeleteFile (pLangEntries[i].lpLNGFileNameTemp);

					free (pLangEntries[i].lpLNGFileNameTemp);
					free (pLangEntries[i].lpLNGFileName);
					free (pLangEntries[i].lpLanguageName);
					free (pLangEntries[i].lpLanguageDescription);
				}

				CloseHandle (hHFile);

				bUpdate = true;

				if ( lpIniFileName )
				{
					if ( dwHeaderCRC32 == dwHeaderOldCRC32 )
					{
						printf ("INFO: Header file \"%s\" doesn't need to be updated.\r\n", lpHPPFileName);
						bUpdate = false;
					}
					else
					{
						sprintf (lpCRC32, "%d", dwHeaderCRC32);
						WritePrivateProfileString (lpHPPFileName, "CRC32", lpCRC32, lpIniFileName);
					}
				}

				if ( bUpdate )
				{
					wsprintfA (lpFullName, "%s\\%s", lpHOutputPath?lpHOutputPath:".", lpHPPFileName);

					MoveFileEx (
							lpHPPFileNameTemp,
							lpFullName,
							MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED
							);
				}

				free (lpCRC32);
		}
		else
			printf ("ERROR: Can't create the header file, exiting.\n\r");
	}
	else
		printf ("ERROR: Zero languages to process, exiting.\n\r");

	DeleteFile (lpHPPFileNameTemp);

	free (lpHPPFileNameTemp);
	free (lpHPPFileName);
	free (pFeedBuffer);
	free (lpString);

	CloseHandle (hFeedFile);

	free (lpFullName);
	free (lpHOutputPath);
	free (lpLNGOutputPath);
	free (lpIniFileName);

	return 0;
}
