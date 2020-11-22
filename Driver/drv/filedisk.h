/*
    This is a virtual disk driver for Windows NT/2000/XP that uses one or more
    files to emulate physical disks.
    Copyright (C) 1999, 2000, 2001, 2002 Bo Brantén.
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _FILE_DISK_
#define _FILE_DISK_

#ifndef __T
#ifdef _NTDDK_
#define __T(x)  L ## x
#else
#define __T(x)  x
#endif
#endif

#ifndef _T
#define _T(x)   __T(x)
#endif

#define DEVICE_BASE_NAME    _T("\\EZCrypt")
#define DEVICE_DIR_NAME     _T("\\Device")      DEVICE_BASE_NAME
#define DEVICE_NAME_PREFIX  DEVICE_DIR_NAME     DEVICE_BASE_NAME

#define FILE_DEVICE_FILE_DISK       0x8000

#define MAX_KEYS		64
#define MAX_KEYHASH		32
#define CCVERSION		0x04;

//#define		_LITE_VERSION_

#define IOCTL_FILE_DISK_OPEN_FILE   CTL_CODE(FILE_DEVICE_FILE_DISK, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_FILE_DISK_CLOSE_FILE  CTL_CODE(FILE_DEVICE_FILE_DISK, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_FILE_DISK_QUERY_FILE  CTL_CODE(FILE_DEVICE_FILE_DISK, 0x802, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct _OPEN_FILE_INFORMATION {
	USHORT			version;
    LARGE_INTEGER   FileSize;
	BOOLEAN         ReadOnly;
	BOOLEAN			WithHead;		//sdz added 060213 to support HeadedFiles
	USHORT			KeyType;		//0 None 1 2Fish 2 AES256
    USHORT			KeyNum;
    USHORT			KeyLength;
    USHORT          FileNameLength;
    UCHAR			Key[MAX_KEYS][MAX_KEYHASH];
    UCHAR			DriveLetter;
    UCHAR           FileName[1];
} OPEN_FILE_INFORMATION, *POPEN_FILE_INFORMATION;

#define		HeaderLength		4096

#endif
