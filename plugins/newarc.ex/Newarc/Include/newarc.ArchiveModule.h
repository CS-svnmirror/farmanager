#pragma once
#include "newarc.h"

typedef int (__stdcall* MODULEENTRY) (int nFunctionID, void *pParams);

class ArchiveModule {

private:

	ArchiveModuleManager* m_pManager;

	GUID m_uid;
	DWORD m_dwFlags;

	string m_strModuleName;

	MODULEENTRY m_pfnModuleEntry;

	HMODULE m_hModule;
	TCHAR *m_pCommands[11];

	TCHAR **m_pLanguageStrings;
	int m_nStringsCount;

	ObjectArray<ArchivePlugin*> m_pPlugins;

public:

	ArchiveModule(ArchiveModuleManager* pManager);
	~ArchiveModule();

	bool Load(const TCHAR* lpModuleName, const TCHAR* lpLanguage);

	const TCHAR* GetModuleName() const;
	const GUID& GetUID() const;

	ArchiveModuleManager* GetManager() const;
	bool QueryCapability(DWORD dwFlags) const;
	
	Array<ArchivePlugin*>& GetPlugins();
	int GetPlugins(Array<ArchivePlugin*>& plugins);

	int GetFormats(Array<ArchiveFormat*>& formats);

	ArchivePlugin* GetPlugin(const GUID& uid);
	ArchiveFormat* GetFormat(const GUID& uidPlugin, const GUID& uidFormat);

	int QueryArchives(
			const GUID* puidPlugin,
			const GUID* puidFormat,
			const TCHAR *lpFileName, 
			const unsigned char *pBuffer, 
			DWORD dwBufferSize, 
			Array<ArchiveFormat*>& result
			);

	HANDLE OpenCreateArchive(
			const GUID& uidPlugin,
			const GUID& uidFormat,
			const TCHAR* lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			);

	void CloseArchive(const GUID& uidPlugin, HANDLE hArchive);

	void ReloadLanguage(const TCHAR *lpLanguage);

	void Configure(const GUID& uid);
	bool GetDefaultCommand(const GUID& uidPlugin, const GUID& uidArchive, int nCommand, string& strCommand, bool& bEnabledByDefault);

//new

	int GetArchiveInfo(HANDLE hArchive, const ArchiveInfoItem** pItems);

	bool StartOperation(HANDLE hArchive, int nOperation, bool bInternal);
	bool EndOperation(HANDLE hArchive, int nOperation, bool bInternal);
	
	int GetArchiveItem(HANDLE hArchive, ArchiveItem* pItem);
	bool FreeArchiveItem(HANDLE hArchive, ArchiveItem* pItem);

	int Extract(HANDLE hArchive, const ArchiveItemArray& items, const TCHAR* lpDestDiskPath, const TCHAR* lpFolderInArchive);
	int AddFiles(HANDLE hArchive, const ArchiveItemArray& items, const TCHAR* lpSourceDiskPath, const TCHAR* lpFolderInArchive);
	int Delete(HANDLE hArchive, const ArchiveItemArray& items);
	int Test(HANDLE hArchive, const ArchiveItemArray& items);

	bool GetArchiveFormat(HANDLE hArchive, GUID* puid);

private:

	static const TCHAR* __stdcall GetMsg(INT_PTR nModuleNumber, int nID);

};
