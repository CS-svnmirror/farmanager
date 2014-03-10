/*
flink.cpp

���� ������ ������� �� ��������� Link`�� - Hard&Sym
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

#include "headers.hpp"
#pragma hdrstop

#include "copy.hpp"
#include "flink.hpp"
#include "imports.hpp"
#include "cddrv.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "panelmix.hpp"
#include "privilege.hpp"
#include "message.hpp"
#include "language.hpp"
#include "dirmix.hpp"
#include "treelist.hpp"
#include "elevation.hpp"

bool CreateVolumeMountPoint(const string& TargetVolume, const string& Object)
{
	bool Result=false;
	string strBuf;

	if (api::GetVolumeNameForVolumeMountPoint(TargetVolume,strBuf))
	{
		if (SetVolumeMountPoint(Object.data(),strBuf.data()))
		{
			Result=true;
		}
	}

	return Result;
}

bool FillREPARSE_DATA_BUFFER(PREPARSE_DATA_BUFFER rdb, const string& PrintName, const string& SubstituteName)
{
	bool Result=false;
	rdb->Reserved=0;

	switch (rdb->ReparseTag)
	{
		case IO_REPARSE_TAG_MOUNT_POINT:
			rdb->MountPointReparseBuffer.SubstituteNameOffset=0;
			rdb->MountPointReparseBuffer.SubstituteNameLength=static_cast<WORD>(SubstituteName.size()*sizeof(wchar_t));
			rdb->MountPointReparseBuffer.PrintNameOffset=rdb->MountPointReparseBuffer.SubstituteNameLength+2;
			rdb->MountPointReparseBuffer.PrintNameLength=static_cast<WORD>(PrintName.size()*sizeof(wchar_t));
			rdb->ReparseDataLength=FIELD_OFFSET(REPARSE_DATA_BUFFER,MountPointReparseBuffer.PathBuffer)+rdb->MountPointReparseBuffer.PrintNameOffset+rdb->MountPointReparseBuffer.PrintNameLength+1*sizeof(wchar_t)-REPARSE_DATA_BUFFER_HEADER_SIZE;

			if (rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE<=static_cast<USHORT>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof(wchar_t)))
			{
				wmemcpy(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)],SubstituteName.data(),SubstituteName.size()+1);
				wmemcpy(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.PrintNameOffset/sizeof(wchar_t)],PrintName.data(),PrintName.size()+1);
				Result=true;
			}
			break;

		case IO_REPARSE_TAG_SYMLINK:
			rdb->SymbolicLinkReparseBuffer.PrintNameOffset=0;
			rdb->SymbolicLinkReparseBuffer.PrintNameLength=static_cast<WORD>(PrintName.size()*sizeof(wchar_t));
			rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset=rdb->MountPointReparseBuffer.PrintNameLength;
			rdb->SymbolicLinkReparseBuffer.SubstituteNameLength=static_cast<WORD>(SubstituteName.size()*sizeof(wchar_t));
			rdb->ReparseDataLength=FIELD_OFFSET(REPARSE_DATA_BUFFER,SymbolicLinkReparseBuffer.PathBuffer)+rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset+rdb->SymbolicLinkReparseBuffer.SubstituteNameLength-REPARSE_DATA_BUFFER_HEADER_SIZE;

			if (rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE<=static_cast<USHORT>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof(wchar_t)))
			{
				wmemcpy(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)],SubstituteName.data(),SubstituteName.size());
				wmemcpy(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(wchar_t)],PrintName.data(),PrintName.size());
				Result=true;
			}
			break;

		default:
			break;
	}

	return Result;
}

bool SetREPARSE_DATA_BUFFER(const string& Object,PREPARSE_DATA_BUFFER rdb)
{
	bool Result=false;
	if (IsReparseTagValid(rdb->ReparseTag))
	{
		SCOPED_ACTION(Privilege)(SE_CREATE_SYMBOLIC_LINK_NAME);
		api::File fObject;

		bool ForceElevation=false;

		DWORD Attributes = api::GetFileAttributes(Object);
		if(Attributes&FILE_ATTRIBUTE_READONLY)
		{
			api::SetFileAttributes(Object, Attributes&~FILE_ATTRIBUTE_READONLY);
		}
		for(size_t i=0;i<2;i++)
		{
			if (fObject.Open(Object, FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, nullptr, ForceElevation))
			{
				DWORD dwBytesReturned;
				if (fObject.IoControl(FSCTL_SET_REPARSE_POINT,rdb,rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE,nullptr,0,&dwBytesReturned))
				{
					Result=true;
				}
				fObject.Close();
				// Open() success, but IoControl() fails. We can't handle this automatically :(
				if(!i && !Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
				{
					ForceElevation=true;
					continue;
				}
				break;
			}
		}
		if(Attributes&FILE_ATTRIBUTE_READONLY)
		{
			api::SetFileAttributes(Object, Attributes);
		}

	}

	return Result;
}

bool CreateReparsePoint(const string& Target, const string& Object,ReparsePointTypes Type)
{
	bool Result=false;

	{
		switch (Type)
		{
			case RP_HARDLINK:
				break;
			case RP_EXACTCOPY:
				Result=DuplicateReparsePoint(Target,Object);
				break;
			case RP_SYMLINK:
			case RP_SYMLINKFILE:
			case RP_SYMLINKDIR:
				{
					DWORD ObjectAttributes = api::GetFileAttributes(Object);
					bool ObjectExist = ObjectAttributes!=INVALID_FILE_ATTRIBUTES;
					if(Type == RP_SYMLINK)
					{
						DWORD Attr = api::GetFileAttributes(Target);
						Type = ((Attr != INVALID_FILE_ATTRIBUTES) && (Attr&FILE_ATTRIBUTE_DIRECTORY)? RP_SYMLINKDIR : RP_SYMLINKFILE);
					}
					if (Imports().CreateSymbolicLinkWPresent() && !ObjectExist)
					{
						Result=api::CreateSymbolicLink(Object,Target,Type==RP_SYMLINKDIR?SYMBOLIC_LINK_FLAG_DIRECTORY:0);
					}
					else
					{
						bool ObjectCreated=false;
						if (Type==RP_SYMLINKDIR)
						{
							ObjectCreated= (ObjectExist && (ObjectAttributes&FILE_ATTRIBUTE_DIRECTORY)) || api::CreateDirectory(Object,nullptr)!=FALSE;
						}
						else
						{
							if(ObjectExist)
							{
								ObjectCreated = !(ObjectAttributes&FILE_ATTRIBUTE_DIRECTORY);
							}
							else
							{
								api::File file;
								if(file.Open(Object,0,0,nullptr,CREATE_NEW))
								{
									ObjectCreated=true;
									file.Close();
								}
							}
						}

						if (ObjectCreated)
						{
							block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
							if(rdb)
							{
								rdb->ReparseTag=IO_REPARSE_TAG_SYMLINK;
								string strPrintName=Target,strSubstituteName=Target;

								if (IsAbsolutePath(Target))
								{
									strSubstituteName=L"\\??\\";
									strSubstituteName+=(strPrintName.data()+(HasPathPrefix(strPrintName)?4:0));
									rdb->SymbolicLinkReparseBuffer.Flags=0;
							}
								else
								{
									rdb->SymbolicLinkReparseBuffer.Flags=SYMLINK_FLAG_RELATIVE;
								}

								if (FillREPARSE_DATA_BUFFER(rdb.get(), strPrintName, strSubstituteName))
								{
									Result=SetREPARSE_DATA_BUFFER(Object,rdb.get());
								}
								else
								{
									SetLastError(ERROR_INSUFFICIENT_BUFFER);
								}
							}
						}
					}
				}
				break;
			case RP_JUNCTION:
			case RP_VOLMOUNT:
			{
				string strPrintName,strSubstituteName;
				ConvertNameToFull(Target,strPrintName);
				strSubstituteName=L"\\??\\";
				strSubstituteName+=(strPrintName.data()+(HasPathPrefix(strPrintName)?4:0));
				block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
				rdb->ReparseTag=IO_REPARSE_TAG_MOUNT_POINT;

				if (FillREPARSE_DATA_BUFFER(rdb.get(), strPrintName, strSubstituteName))
				{
					Result=SetREPARSE_DATA_BUFFER(Object,rdb.get());
				}
				else
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
				}
			}
			break;
		}
	}

	return Result;
}

bool GetREPARSE_DATA_BUFFER(const string& Object,PREPARSE_DATA_BUFFER rdb)
{
	bool Result=false;
	const DWORD FileAttr = api::GetFileAttributes(Object);

	if (FileAttr!=INVALID_FILE_ATTRIBUTES && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT))
	{
		api::File fObject;
		if(fObject.Open(Object,0,0,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT))
		{
			DWORD dwBytesReturned;
			if(fObject.IoControl(FSCTL_GET_REPARSE_POINT, nullptr, 0, rdb, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwBytesReturned) && IsReparseTagValid(rdb->ReparseTag))
			{
				Result=true;
			}
			fObject.Close();
		}
	}

	return Result;
}

bool DeleteReparsePoint(const string& Object)
{
	bool Result=false;
	block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if(rdb)
	{
		if (GetREPARSE_DATA_BUFFER(Object, rdb.get()))
		{
			api::File fObject;
			if (fObject.Open(Object, FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT))
			{
				DWORD dwBytes;
				REPARSE_GUID_DATA_BUFFER rgdb = {rdb->ReparseTag};
				Result=fObject.IoControl(FSCTL_DELETE_REPARSE_POINT,&rgdb,REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,nullptr,0,&dwBytes);
				fObject.Close();
			}
		}
	}
	return Result;
}

bool GetReparsePointInfo(const string& Object, string &strDestBuff,LPDWORD lpReparseTag)
{
	WORD NameLength=0;
	block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if(rdb)
	{
		if (GetREPARSE_DATA_BUFFER(Object,rdb.get()))
		{
			const wchar_t *PathBuffer = nullptr;

			if (lpReparseTag)
				*lpReparseTag=rdb->ReparseTag;

			switch (rdb->ReparseTag)
			{
			case IO_REPARSE_TAG_SYMLINK:
				{
					NameLength = rdb->SymbolicLinkReparseBuffer.PrintNameLength/sizeof(wchar_t);

					if (NameLength)
					{
						PathBuffer = &rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(wchar_t)];
					}
					else
					{
						NameLength = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength/sizeof(wchar_t);
						PathBuffer = &rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)];
					}
				}
				break;

			case IO_REPARSE_TAG_MOUNT_POINT:
				{
					NameLength = rdb->MountPointReparseBuffer.PrintNameLength/sizeof(wchar_t);

					if (NameLength)
					{
						PathBuffer = &rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.PrintNameOffset/sizeof(wchar_t)];
					}
					else
					{
						NameLength = rdb->MountPointReparseBuffer.SubstituteNameLength/sizeof(wchar_t);
						PathBuffer = &rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)];
					}
				}
				break;

			default:
				break;
			}
			if(NameLength)
			{
				strDestBuff.assign(PathBuffer, NameLength);
			}
		}
	}
	return NameLength != 0;
}

int GetNumberOfLinks(const string& Name, bool negative_if_error)
{
	int NumberOfLinks = (negative_if_error ? -1 : +1);
	api::File file;
	if(file.Open(Name, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT))
	{
		BY_HANDLE_FILE_INFORMATION bhfi;
		if (file.GetInformation(bhfi))
		{
			NumberOfLinks=bhfi.nNumberOfLinks;
		}
		file.Close();
	}
	return NumberOfLinks;
}


int MkHardLink(const string& ExistingName,const string& NewName, bool Silent)
{
	BOOL Result = api::CreateHardLink(NewName, ExistingName, nullptr);

	if (!Result && !Silent)
	{
		Global->CatchError();
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCopyCannotCreateLink),NewName.data(),MSG(MOk));
	}
	return Result;
}

bool EnumStreams(const string& FileName,UINT64 &StreamsSize,DWORD &StreamsCount)
{
	bool Result=false;
	WIN32_FIND_STREAM_DATA fsd;
	HANDLE hFind=api::FindFirstStream(FileName,FindStreamInfoStandard,&fsd);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		StreamsCount=1;
		StreamsSize=fsd.StreamSize.QuadPart;

		while (api::FindNextStream(hFind,&fsd))
		{
			StreamsCount++;
			StreamsSize+=fsd.StreamSize.QuadPart;
		}

		api::FindStreamClose(hFind);
		Result=true;
	}

	return Result;
}

bool DelSubstDrive(const string& DeviceName)
{
	bool Result=false;
	string strTargetPath;

	if (GetSubstName(DRIVE_NOT_INIT,DeviceName,strTargetPath))
	{
		strTargetPath.insert(0, L"\\??\\", 4);
		Result=(DefineDosDevice(DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|DDD_EXACT_MATCH_ON_REMOVE,DeviceName.data(),strTargetPath.data())!=FALSE);
	}

	return Result;
}

bool GetSubstName(int DriveType,const string& DeviceName, string &strTargetPath)
{
	bool Ret=false;
	/*
	+ ��������� � ����������� �� Global->Opt->SubstNameRule
	������� �����:
	0 - ���� ����������, �� ���������� ������� �����
	1 - ���� ����������, �� ���������� ��� ���������
	*/
	bool DriveRemovable = (DriveType==DRIVE_REMOVABLE || DriveType==DRIVE_CDROM);

	if (DriveType==DRIVE_NOT_INIT || (((Global->Opt->SubstNameRule & 1) || !DriveRemovable) && ((Global->Opt->SubstNameRule & 2) || DriveRemovable)))
	{
		PATH_TYPE Type = ParsePath(DeviceName);
		if (Type == PATH_DRIVELETTER)
		{
			string Name;
			if (api::QueryDosDevice(DeviceName, Name))
			{
				if (Name.compare(0, 4, L"\\??\\") == 0)
				{
					strTargetPath=Name.substr(4);
					Ret=true;
				}
			}
		}
	}

	return Ret;
}

bool GetVHDName(const string& DeviceName, string &strVolumePath)
{
	bool Result=false;
	api::File Device;
	if(Device.Open(DeviceName, FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
	{
		ULONG Size = 4096;
		block_ptr<STORAGE_DEPENDENCY_INFO> StorageDependencyInfo(Size);
		if(StorageDependencyInfo)
		{
			StorageDependencyInfo->Version = STORAGE_DEPENDENCY_INFO_VERSION_2;
			DWORD Used = 0;
			Result = Device.GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG_HOST_VOLUMES, Size, StorageDependencyInfo.get(), &Used);
			if(!Result && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				StorageDependencyInfo.reset(Used);
				if(StorageDependencyInfo)
				{
					StorageDependencyInfo->Version = STORAGE_DEPENDENCY_INFO_VERSION_2;
					Result = Device.GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG_HOST_VOLUMES, Used, StorageDependencyInfo.get(), &Used);
				}
			}
			if(Result)
			{
				if(StorageDependencyInfo->NumberEntries)
				{
					strVolumePath = StorageDependencyInfo->Version2Entries[0].HostVolumeName;
					strVolumePath += StorageDependencyInfo->Version2Entries[0].DependentVolumeRelativePath;
					// trick: ConvertNameToReal also converts \\?\{GUID} to drive letter, if possible.
					ConvertNameToReal(strVolumePath, strVolumePath);
				}
			}
		}
	}
	return Result;
}


void GetPathRoot(const string& Path, string &strRoot)
{
	string RealPath;
	ConvertNameToReal(Path, RealPath);
	strRoot = ExtractPathRoot(RealPath);
}

bool ModifyReparsePoint(const string& Object,const string& NewData)
{
	bool Result=false;
	block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if(rdb)
	{
		if (GetREPARSE_DATA_BUFFER(Object,rdb.get()))
		{
			bool FillResult=false;

			switch (rdb->ReparseTag)
			{
			case IO_REPARSE_TAG_MOUNT_POINT:
				{
					string strPrintName,strSubstituteName;
					ConvertNameToFull(NewData,strPrintName);
					strSubstituteName=L"\\??\\";
					strSubstituteName+=(strPrintName.data()+(HasPathPrefix(strPrintName)?4:0));
					FillResult=FillREPARSE_DATA_BUFFER(rdb.get(), strPrintName, strSubstituteName);
				}
				break;

			case IO_REPARSE_TAG_SYMLINK:
				{
					string strPrintName=NewData,strSubstituteName=NewData;

					if (IsAbsolutePath(NewData))
					{
						strSubstituteName=L"\\??\\";
						strSubstituteName+=(strPrintName.data()+(HasPathPrefix(strPrintName)?4:0));
						rdb->SymbolicLinkReparseBuffer.Flags=0;
					}
					else
					{
						rdb->SymbolicLinkReparseBuffer.Flags=SYMLINK_FLAG_RELATIVE;
					}

					FillResult=FillREPARSE_DATA_BUFFER(rdb.get(), strPrintName, strSubstituteName);
				}
				break;

			default:
				break;
			}

			if (FillResult)
			{
				Result=SetREPARSE_DATA_BUFFER(Object,rdb.get());
			}
			else
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
			}
		}
	}
	return Result;
}

bool DuplicateReparsePoint(const string& Src,const string& Dst)
{
	bool Result=false;
	block_ptr<REPARSE_DATA_BUFFER> rdb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if(rdb)
	{
		if (GetREPARSE_DATA_BUFFER(Src,rdb.get()) && SetREPARSE_DATA_BUFFER(Dst,rdb.get()))
		{
			Result=true;
		}
	}
	return Result;
}

void NormalizeSymlinkName(string &strLinkName)
{
	if (!strLinkName.compare(0, 4, L"\\??\\"))
	{
		strLinkName[1] = L'\\';
		PATH_TYPE Type = ParsePath(strLinkName);
		if(Type == PATH_DRIVELETTERUNC)
		{
			strLinkName.erase(0, 4);
		}
	}
}

// ����� ��� �������� SymLink ��� ���������.
int MkSymLink(const string& SelName,const string& Dest,ReparsePointTypes LinkType, bool Silent)
{
	if (!SelName.empty() && !Dest.empty())
	{
		string strSrcFullName, strDestFullName, strSelOnlyName;
		// ������� ���
		strSelOnlyName = SelName;
		DeleteEndSlash(strSelOnlyName);
		const wchar_t *PtrSelName=LastSlash(strSelOnlyName.data());

		if (!PtrSelName)
			PtrSelName=strSelOnlyName.data();
		else
			++PtrSelName;

		if (SelName[1] == L':' && (!SelName[2] || (IsSlash(SelName[2]) && !SelName[3]))) // C: ��� C:/
		{
//      if(Flags&FCOPY_VOLMOUNT)
			{
				strSrcFullName = SelName;
				AddEndSlash(strSrcFullName);
			}
			/*
			  ��� ����� - �� ����� ����� ���������!
			  �.�. ���� � �������� SelName �������� "C:", �� � ���� ����� ����������
			  ��������� ���� ����� - � symlink`� �� volmount
			*/
			LinkType=RP_VOLMOUNT;
		}
		else
			ConvertNameToFull(SelName,strSrcFullName);

		ConvertNameToFull(Dest,strDestFullName);

		if (IsSlash(strDestFullName.back()))
		{
			if (LinkType!=RP_VOLMOUNT)
				strDestFullName += PtrSelName;
			else
			{
				const wchar_t Tmp[]={L'D',L'i',L's',L'k',L'_',SelName[0],L'\0'};
				strDestFullName+=Tmp;
			}
		}

		if (LinkType==RP_VOLMOUNT)
		{
			AddEndSlash(strSrcFullName);
			AddEndSlash(strDestFullName);
		}

		DWORD JSAttr=api::GetFileAttributes(strDestFullName);

		if (JSAttr != INVALID_FILE_ATTRIBUTES) // ���������� �����?
		{
			if ((JSAttr&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!Silent)
				{
					Message(MSG_WARNING,1,MSG(MError),
					        MSG(MCopyCannotCreateJunctionToFile),
					        strDestFullName.data(),MSG(MOk));
				}

				return 0;
			}

			if (TestFolder(strDestFullName) == TSTFLD_NOTEMPTY) // � ������?
			{
				// �� ������, �� ��� ��, ����� ������� ������� dest\srcname
				AddEndSlash(strDestFullName);

				if (LinkType==RP_VOLMOUNT)
				{
					string strTmpName(MSG(MCopyMountName));
					strTmpName += SelName[0];
					strDestFullName += strTmpName;
					AddEndSlash(strDestFullName);
				}
				else
					strDestFullName += PtrSelName;

				JSAttr=api::GetFileAttributes(strDestFullName);

				if (JSAttr != INVALID_FILE_ATTRIBUTES) // � ����� ���� ����???
				{
					if (TestFolder(strDestFullName) == TSTFLD_NOTEMPTY) // � ������?
					{
						if (!Silent)
						{
							if (LinkType==RP_VOLMOUNT)
							{
								Message(MSG_WARNING,1,MSG(MError),
								        (LangString(MCopyMountVolFailed) << SelName).data(),
								        (LangString(MCopyMountVolFailed2) << strDestFullName).data(),
								        MSG(MCopyFolderNotEmpty),
								        MSG(MOk));
							}
							else
								Message(MSG_WARNING,1,MSG(MError),
								        MSG(MCopyCannotCreateLink),strDestFullName.data(),
								        MSG(MCopyFolderNotEmpty),MSG(MOk));
						}

						return 0; // ���������� � ����
					}
				}
				else // �������.
				{
					if (api::CreateDirectory(strDestFullName,nullptr))
						TreeList::AddTreeName(strDestFullName);
					else
						CreatePath(strDestFullName);
				}

				if (api::GetFileAttributes(strDestFullName) == INVALID_FILE_ATTRIBUTES) // ���, ��� ����� ���� �����.
				{
					if (!Silent)
					{
						Global->CatchError();
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
						        MSG(MCopyCannotCreateFolder),
						        strDestFullName.data(),MSG(MOk));
					}

					return 0;
				}
			}
		}
		else
		{
			if (LinkType==RP_SYMLINK || LinkType==RP_SYMLINKFILE || LinkType==RP_SYMLINKDIR)
			{
				// � ���� ������ ��������� ����, �� �� ��� �������
				string strPath=strDestFullName;

				if (CutToSlash(strPath))
				{
					if (api::GetFileAttributes(strPath)==INVALID_FILE_ATTRIBUTES)
						CreatePath(strPath);
				}
			}
			else
			{
				bool CreateDir=true;

				if (LinkType==RP_EXACTCOPY)
				{
					// � ���� ������ ��������� ��� �������, ��� ������ ����
					DWORD dwSrcAttr=api::GetFileAttributes(strSrcFullName);

					if (dwSrcAttr!=INVALID_FILE_ATTRIBUTES && !(dwSrcAttr&FILE_ATTRIBUTE_DIRECTORY))
						CreateDir=false;
				}

				if (CreateDir)
				{
					if (api::CreateDirectory(strDestFullName,nullptr))
						TreeList::AddTreeName(strDestFullName);
					else
						CreatePath(strDestFullName);
				}
				else
				{
					string strPath=strDestFullName;

					if (CutToSlash(strPath))
					{
						// ������
						if (api::GetFileAttributes(strPath)==INVALID_FILE_ATTRIBUTES)
							CreatePath(strPath);

						api::File file;
						if(file.Open(strDestFullName, 0, 0, 0, CREATE_NEW, api::GetFileAttributes(strSrcFullName)))
						{
							file.Close();
						}
					}
				}

				if (api::GetFileAttributes(strDestFullName) == INVALID_FILE_ATTRIBUTES) // ���. ��� ����� ���� �����.
				{
					if (!Silent)
					{
						Global->CatchError();
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
						        MSG(MCopyCannotCreateLink),strDestFullName.data(),MSG(MOk));
					}

					return 0;
				}
			}
		}

		if (LinkType!=RP_VOLMOUNT)
		{
			if (CreateReparsePoint(strSrcFullName,strDestFullName,LinkType))
			{
				return 1;
			}
			else
			{
				if (!Silent)
				{
					Global->CatchError();
					Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
					        MSG(MCopyCannotCreateLink),strDestFullName.data(),MSG(MOk));
				}

				return 0;
			}
		}
		else
		{
			if (CreateVolumeMountPoint(strSrcFullName,strDestFullName))
			{
				return 1;
			}
			else
			{
				if (!Silent)
				{
					Global->CatchError();
					Message(MSG_WARNING|MSG_ERRORTYPE,1,
						MSG(MError),
						(LangString(MCopyMountVolFailed) << SelName).data(),
						(LangString(MCopyMountVolFailed2) << strDestFullName).data(),
						MSG(MOk));
				}

				return 0;
			}
		}
	}

	return 2;
}
