#pragma once
#include "7z.h"



class SevenZipArchive {

private:

	unsigned int m_uItemsCount;

	SevenZipPlugin* m_pPlugin;

	HANDLE m_hCallback;
	ARCHIVECALLBACK m_pfnCallback;

	string m_strFileName;

	GUID m_uid;
	
	IInArchive* m_pArchive;
	CInFile* m_pFile;

	bool m_bCreated;
	bool m_bOpened;

	Array<ArchiveInfoItem> m_pArchiveInfo;

public:

	SevenZipArchive(
			SevenZipPlugin* pPlugin, 
			const GUID& uid, 
			const TCHAR *lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback,
			bool bCreated
			);

	IInArchive* GetArchive();
	//IInStream* GetFile();
	const TCHAR* GetFileName();

	void SetItemsCount(unsigned int uItemsCount);
	unsigned int GetItemsCount();

	virtual ~SevenZipArchive();

	const GUID& GetUID();

	bool StartOperation(int nOperation, bool bInternal);
	bool EndOperation(int nOperation, bool bInternal);

	int GetArchiveItem(ArchiveItem* pItem);
	bool FreeArchiveItem(ArchiveItem* pItem);

	int GetArchiveInfo(const ArchiveInfoItem** pItems);

	bool Test(const ArchiveItem* pItems, int nItemsNumber);
	bool Delete(const ArchiveItem *pItems, int nItemsNumber);
	bool Extract(const ArchiveItem *pItems, int nItemsNumber, const TCHAR* lpDestDiskPath, const TCHAR* lpPathInArchive);
	bool AddFiles(const ArchiveItem* pItems, int nItemsNumber, const TCHAR* lpSourceDiskPath, const TCHAR* lpPathInArchive);

	LONG_PTR OnStartOperation(int nOperation, unsigned __int64 uTotalSize, unsigned __int64 uTotalFiles);
	LONG_PTR OnProcessFile(const ArchiveItem* pItem, const TCHAR* lpDestName);
	LONG_PTR OnProcessData(unsigned __int64 uSize);
	LONG_PTR OnPasswordOperation(int nType, TCHAR* lpBuffer, DWORD dwBufferSize);

private:

	bool Open();
	void Close();

	void QueryArchiveInfo();

	LONG_PTR Callback(int nMsg, int nParam1, LONG_PTR nParam2);
};
