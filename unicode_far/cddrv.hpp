#pragma once

/*
cddrv.hpp

��� �����
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

enum CDROM_DeviceCaps
{
	CDDEV_CAPS_NONE               = 0x00000000,

	CDDEV_CAPS_READ_CDROM         = 0x00000001,
	CDDEV_CAPS_READ_CDR           = 0x00000002,
	CDDEV_CAPS_READ_CDRW          = 0x00000004,

	CDDEV_CAPS_READ_DVDROM        = 0x00000008,
	CDDEV_CAPS_READ_DVDR          = 0x00000010,
	CDDEV_CAPS_READ_DVDRW         = 0x00000020,
	CDDEV_CAPS_READ_DVDRAM        = 0x00000040,


	CDDEV_CAPS_WRITE_CDR          = 0x00020000,
	CDDEV_CAPS_WRITE_CDRW         = 0x00040000,

	CDDEV_CAPS_WRITE_DVDR         = 0x00100000,
	CDDEV_CAPS_WRITE_DVDRW        = 0x00200000,
	CDDEV_CAPS_WRITE_DVDRAM       = 0x00400000,


	CDDEV_CAPS_GENERIC_CD         = CDDEV_CAPS_READ_CDROM | CDDEV_CAPS_READ_CDR | CDDEV_CAPS_READ_CDRW,
	CDDEV_CAPS_GENERIC_CDRW       = CDDEV_CAPS_GENERIC_CD | CDDEV_CAPS_WRITE_CDR | CDDEV_CAPS_WRITE_CDRW,
	CDDEV_CAPS_GENERIC_DVD        = CDDEV_CAPS_GENERIC_CD | CDDEV_CAPS_READ_DVDROM | CDDEV_CAPS_READ_DVDR | CDDEV_CAPS_READ_DVDRW,
	CDDEV_CAPS_GENERIC_COMBO      = CDDEV_CAPS_GENERIC_DVD | CDDEV_CAPS_WRITE_CDR | CDDEV_CAPS_WRITE_CDRW,
	CDDEV_CAPS_GENERIC_DVDRW      = CDDEV_CAPS_GENERIC_COMBO | CDDEV_CAPS_WRITE_DVDR | CDDEV_CAPS_WRITE_DVDRW,
	CDDEV_CAPS_GENERIC_DVDRAM     = CDDEV_CAPS_GENERIC_DVDRW | CDDEV_CAPS_WRITE_DVDRAM
};

CDROM_DeviceCaps GetCDDeviceCaps(HANDLE hDevice);
UINT GetCDDeviceTypeByCaps(CDROM_DeviceCaps caps);
bool IsDriveTypeCDROM(UINT DriveType);
UINT FAR_GetDriveType(const wchar_t *RootDir, CDROM_DeviceCaps *caps=NULL, DWORD Detect=0);
