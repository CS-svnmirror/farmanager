#include "7z.h"
#include <array.hpp>
#include <FarDialogs.hpp>

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

pointer_array <SevenZipModule*> modules;
ArchiveFormatInfo *pFormatInfo = NULL;

struct ArchiveModuleInformation {

	int nTypes;
	char *pTypeNames;

	int nConfigStringsNumber;

	char **pConfigStrings;
};

const SevenZipModule *GetModuleFromGUID (const GUID &uid, unsigned int *formatIndex)
{
	for (int i = 0; i < modules.count(); i++)
	{
		SevenZipModule *pModule = modules[i];

		for (int j = 0; j < pModule->m_nNumberOfFormats; j++)
		{
			if ( pModule->m_pInfo[j].uid == uid )
			{
				if ( formatIndex )
					*formatIndex = j;

				return pModule;
			}
		}
	}

	return NULL;
}

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;

	modules.create (ARRAY_OPTIONS_DELETE);

	WIN32_FIND_DATA fdata;
	char *lpMask = StrDuplicate (Info.ModuleName, 260);

	CutToSlash (lpMask);
	strcat (lpMask, "Formats\\*.dll");

	HANDLE hSearch = FindFirstFile (lpMask, &fdata);

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do {

			if ( !OptionIsOn (fdata.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
			{
				SevenZipModule *pModule = new SevenZipModule;

				char *lpModuleName = StrDuplicate (Info.ModuleName, 260);
				CutToSlash(lpModuleName);

				strcat (lpModuleName, "Formats\\");
				strcat (lpModuleName, fdata.cFileName);

				if ( pModule->Initialize (lpModuleName) )
					modules.add (pModule);
				else
					delete pModule;

				free (lpModuleName);
			}
		} while ( FindNextFile (hSearch, &fdata) );

		FindClose (hSearch);
	}

	pFormatInfo = NULL;

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	modules.free ();
	free (pFormatInfo);

	return NAERROR_SUCCESS;
}

extern int FindFormats (const char *lpFileName, pointer_array<FormatPosition*> &formats);

int __cdecl SortFormats (
    FormatPosition *pos1,
    FormatPosition *pos2,
    void *pParam
    )
{
	if ( pos1->position > pos2->position )
		return 1;

	if ( pos1->position < pos2->position )
		return -1;

	//if ( pos1->position == pos2->position )
	return 0;
}


int OnQueryArchive (QueryArchiveStruct *pQAS)
{
/*	SevenZipModule *pSplitModule = NULL;
	bool bSplit;

   	for (int i = 0; i < Formats.GetCount (); i++)
   	{
   		bSplit = false;
   		SevenZipModule *pModule = Formats[i];

   		if ( pModule->IsSplitModule() )
   		{
   			pSplitModule = pModule;
   			bSplit = true;
		}

   		if ( pModule && !bSplit )
   		{
   			SevenZipArchive *pArchive = new SevenZipArchive (pModule, pQAS->lpFileName);

   			pArchive->m_bIsArchive = false;

   			if ( pArchive->pOpenArchive (0, NULL) )
   				pArchive->pCloseArchive ();

   				if ( pArchive->m_bIsArchive )
   				{
   					ArchiveFormatInfo info;

   					pModule->GetArchiveFormatInfo (&info);

   					MessageBox (0, info.lpName, "asd", MB_OK);
	   				pQAS->hResult = (HANDLE)pArchive;

   					return NAERROR_SUCCESS;
				}

   			delete pArchive;
   		}
   	}

   	if ( pSplitModule )
   	{
		SevenZipArchive *pArchive = new SevenZipArchive (pSplitModule, pQAS->lpFileName);

		pArchive->m_bIsArchive = false;

		if ( pArchive->pOpenArchive (0, NULL) )
			pArchive->pCloseArchive ();

			if ( pArchive->m_bIsArchive )
			{
				pQAS->hResult = (HANDLE)pArchive;

				return NAERROR_SUCCESS;
			}

		delete pArchive;
   	} */


	pointer_array<FormatPosition*> formats (ARRAY_OPTIONS_DELETE);

	FindFormats (pQAS->lpFileName, formats);

	formats.sort ((void *)SortFormats, NULL);

	for (int j = 0; j < formats.count(); j++)
	{
		FormatPosition *pos = formats[j];

		for (int i = 0; i < modules.count (); i++)
		{
			SevenZipModule *pModule = modules[i];

			for (int k = 0; k < pModule->m_nNumberOfFormats; k++)
			{
				if ( IsEqualGUID (pModule->m_pInfo[k].uid, *pos->puid) )
				{
					SevenZipArchive *pArchive = new SevenZipArchive (pModule, k, pQAS->lpFileName, false);

					pQAS->hResult = (HANDLE)pArchive;

					formats.free ();

					return NAERROR_SUCCESS;
				}
			}
		}
	}

	formats.free ();

	return NAERROR_INTERNAL;
}

int OnCreateArchive (CreateArchiveStruct *pCAS)
{
	unsigned int formatIndex = 0;
	const SevenZipModule *pModule = GetModuleFromGUID (pCAS->uid, &formatIndex);

	if ( pModule )
	{
		SevenZipArchive *pArchive = new SevenZipArchive (pModule, formatIndex, pCAS->lpFileName, true);

		pCAS->hResult = (HANDLE)pArchive;

		return NAERROR_SUCCESS;
	}

	return NAERROR_INTERNAL;
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
	SevenZipArchive *pArchive = (SevenZipArchive*)pOAS->hArchive;

	pOAS->bResult = pArchive->pOpenArchive (pOAS->nMode, pOAS->pfnCallback);

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
/*	SevenZipArchive *pArchive = (SevenZipArchive*)pCAS->hArchive;

	pArchive->pCloseArchive ();*/

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (SevenZipArchive *pArchive)
{
	pArchive->pCloseArchive ();
	delete pArchive;

	return NAERROR_SUCCESS;
}

int OnGetArchivePluginInfo (
		ArchivePluginInfo *ai
		)
{
	int nCount = 0;

	for (int i = 0; i < modules.count (); i++)
		nCount += modules[i]->m_nNumberOfFormats;

	pFormatInfo = (ArchiveFormatInfo*)realloc (pFormatInfo, nCount*sizeof (ArchiveFormatInfo));

	int index = 0;

	for (int i = 0; i < modules.count (); i++)
	{
		SevenZipModule *pModule = modules[i];

		for (int j = 0; j < pModule->m_nNumberOfFormats; j++)
		{
			pModule->GetArchiveFormatInfo (j, &pFormatInfo[index]);
			index++;
		}
	}


	ai->nFormats = nCount;
	ai->pFormatInfo = pFormatInfo;

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pGAI->hArchive;

	pGAI->nResult = pArchive->pGetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

int OnGetArchiveFormat (GetArchiveFormatStruct *pGAF)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pGAF->hArchive;
	pGAF->uid = pArchive->m_pModule->m_pInfo[pArchive->m_nFormatIndex].uid;

	return NAERROR_SUCCESS;
}

int OnExtract (ExtractStruct *pES)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pES->hArchive;

	pES->bResult = pArchive->pExtract (
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnTest (TestStruct *pTS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pTS->hArchive;

	pTS->bResult = pArchive->pTest (
			pTS->pItems,
			pTS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}

int OnAdd (AddStruct *pAS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pAS->hArchive;

	pAS->bResult = pArchive->pAddFiles (
			pAS->lpSourcePath,
			pAS->lpCurrentPath,
			pAS->pItems,
			pAS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}


int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
	pGDC->bResult = GetFormatCommand (pGDC->uid, pGDC->nCommand, pGDC->lpCommand);
	return NAERROR_SUCCESS;
}

int OnDelete (DeleteStruct *pDS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pDS->hArchive;

	pDS->bResult = pArchive->pDelete (
			pDS->pItems,
			pDS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}

int OnNotify (NotifyStruct *pNS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pNS->hArchive;

	pArchive->pNotify (
			pNS->nEvent,
			pNS->pEventData
			);

	return NAERROR_SUCCESS;
}


int OnConfigureFormat (ConfigureFormatStruct *pCF)
{
//	if ( pCF->uid == CLSID_CFormat7z )
	{
		FarDialog D(-1, -1, 60, 20);

		D.DoubleBox (2, 2, 57, 17, "7z config");

		D.Show ();
	}

	return NAERROR_SUCCESS;
}


int __stdcall PluginEntry (
		int nFunctionID,
		void *pParams
		)
{
	switch ( nFunctionID ) {

	case FID_INITIALIZE:
		return OnInitialize ((StartupInfo*)pParams);

	case FID_FINALIZE:
		return OnFinalize ();

	case FID_QUERYARCHIVE:
		return OnQueryArchive ((QueryArchiveStruct*)pParams);

	case FID_OPENARCHIVE:
		return OnOpenArchive ((OpenArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_FINALIZEARCHIVE:
		return OnFinalizeArchive ((SevenZipArchive *)pParams);

	case FID_GETARCHIVEPLUGININFO:
		return OnGetArchivePluginInfo ((ArchivePluginInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_GETARCHIVEFORMAT:
		return OnGetArchiveFormat ((GetArchiveFormatStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

	case FID_TEST:
		return OnTest ((TestStruct*)pParams);

	case FID_GETDEFAULTCOMMAND:
		return OnGetDefaultCommand ((GetDefaultCommandStruct*)pParams);

	case FID_DELETE:
		return OnDelete ((DeleteStruct*)pParams);

	case FID_ADD:
		return OnAdd ((AddStruct*)pParams);

	case FID_CREATEARCHIVE:
		return OnCreateArchive ((CreateArchiveStruct*)pParams);

	case FID_NOTIFY:
		return OnNotify ((NotifyStruct*)pParams);

	case FID_CONFIGUREFORMAT:
		return OnConfigureFormat ((ConfigureFormatStruct*)pParams);
	}

	return NAERROR_NOTIMPLEMENTED;
}


#if !defined(__GNUC__)

BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return TRUE;
}

#endif
