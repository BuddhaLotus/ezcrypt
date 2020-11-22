#ifndef			_Global_Head_File_
#define			_Global_Head_File_


#define			InDebuging			1
//#define		HideOnStart			1
//#define		LogDetailLevel		3
#define			LiteVersion

#define			WIN_95			0x0400
#define			WIN_98			0x040A
#define			WIN_ME			0x0450
#define			WINNT_351		0x0333
#define			WIN_2000		0x0500
#define			WIN_XP			0x0501
#define			WIN_2003		0x0502

#define			DefaultPass		"dracon523"

#define			IsMultiLang
extern 	bool 	g_bIsRunAgain;	//SDZ added 01181101

BOOL			ExpertMode=true;
char			BoxFileName[MAX_PATH+1];
char			TempFileName[MAX_PATH+1];


//SDZ added 01191026>>
BOOL GetScrResolution(WORD* pWidth, WORD* pHeight)
{
	HDC screenDC;
	screenDC = CreateDC("DISPLAY", NULL, NULL,NULL);
	if (!screenDC)
		return FALSE;

	*pWidth = GetDeviceCaps(screenDC,HORZRES);
	*pHeight = GetDeviceCaps(screenDC,VERTRES);

	DeleteDC(screenDC);
	return TRUE;
}
//SDZ added 01191026<<

//sdz added 0602151741>>
void
CenterForm(
TForm*			ThatForm)
{
	//SDZ added 01191028>>
	WORD ScreenX,ScreenY;
	if(GetScrResolution(&ScreenX,&ScreenY))
	{
		ThatForm->Left=(ScreenX-ThatForm->Width)/2;
		ThatForm->Top=(ScreenY-ThatForm->Height)/2;
	}
	//SDZ added 01191028<<
}

//sdz added 0602151741<<

//SDZ added 01151859>>
void
ListFreeDriverLetter(TComboBox* DriverBox)
{
	int i;
	DWORD ld = GetLogicalDrives();
	char*	DriverStr="A:";

	DriverBox->Clear();
	if (ld == 0) return;
	for (i=3;i<26;i++) {
		if ((ld & (1<<i)) == 0)
		{
			DriverStr[0]='A'+i;
			DriverBox->AddItem(DriverStr,NULL);
		}
	}
	DriverBox->ItemIndex=0;
}
//SDZ added 01151859<<

//sdz paste here.02061645>>

//SDZ moved here>
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <Wincrypt.h>
#include "filedisk.h"
#include "loop_fish2.c"
#include "rmd160.c"
#include "rmd160.h"
#include "aes.c"
#include "sha512.c"

#include <IniFiles.hpp>
#include <Grids.hpp>
//#include "Mount.h"

//SDZ added		200601101448>
FILE*		LogFile=NULL;
char		LogFileName[MAX_PATH+1];		//"YourCrypt.log"
char		LastMountedFile[MAX_PATH+1];
SYSTEMTIME	SysTime;
char		TimeStamp[32];
//SDZ added		200601101448<

TIniFile*	YCIniFile;
TIniFile*	LangFile;
int			LogDetailLevel;
BOOL		UmountAllExit;
BOOL		HideMainForm;
BOOL		DeepQuiet;
BOOL		AlwaysTrayIcon;
BOOL		NoExitConfirm;
BOOL		DelDriverExit;

//DiskFileHead	FileHead;

//SDZ added 0708020824>>
bool
OpenIniFile(
//TIniFile*	IniFileHandler
)
{
	if(YCIniFile!=NULL)
		return true;
	
	YCIniFile = new TIniFile( ChangeFileExt( Application->ExeName, ".INI" ) );
	if(!YCIniFile)
	{
		delete YCIniFile;
		return false;
	}
	else
	{
		return true;
	}
}
//SDZ<<


//SDZ added 01121459>>
int
GetOsVersion()
{
	OSVERSIONINFO osv;
	int osver;
	
    osv.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	BOOL bRet=GetVersionEx(&osv);
	if(bRet)
	{
		switch(osv.dwMajorVersion)
		{
		case 3:
			osver=WINNT_351;
			break;
		case 4:
			if(osv.dwMinorVersion==0)osver=WIN_95;
			if(osv.dwMinorVersion==10)osver=WIN_98;
			if(osv.dwMinorVersion==90)osver=WIN_ME;
			break;
		case 5:
			if(osv.dwMinorVersion==0)osver=WIN_2000;
			if(osv.dwMinorVersion==1)osver=WIN_XP;
			if(osv.dwMinorVersion==2)osver=WIN_2003;
			break;
		default:
			osver=-1;
		}
	}
	
	return osver;
}
//SDZ added 01121459<<


//SDZ added		200601101448>>
int
LogEvent(
//FILE*	LogFile,
//char*	LogFileName,
char*	LogText)

{
	GetSystemTime(&SysTime);              // gets current time

	//memset(DateStamp,0,sizeof(TimeStamp));
	sprintf(TimeStamp,"\n%04d-%02d-%02d %02d:%02d:%02d\t",SysTime.wYear,SysTime.wMonth,
		SysTime.wDay,SysTime.wHour,SysTime.wMinute,SysTime.wSecond);

	if(!LogFile)
	{
		LogFile=fopen(LogFileName,"a+");
		if(!LogFile)
			return -1;
	}

	if(fwrite(TimeStamp,1,strlen(TimeStamp),LogFile)!=strlen(TimeStamp))
	{
		return -2;
	}

	if(fwrite(LogText,1,strlen(LogText),LogFile)!=strlen(LogText))
	{
		return -2;
	}

	return 0;	
}
//SDZ added		200601101448<<


//SDZ paste here 01170915>>
int __cdecl
CreateSparseFile(
char* 			FileName,
LARGE_INTEGER	FileSize,
BOOL			HaveHead)
{
	//LARGE_INTEGER   FileSize;
	HANDLE          FileHandle;
	DWORD           BytesReturned;
	BOOL			SetSparseOK=false;

	FileHandle = CreateFile(
		FileName,
		GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_NEW,
        FILE_FLAG_NO_BUFFERING,
		NULL
        );

    if (FileHandle == INVALID_HANDLE_VALUE)
    {
		//PrintLastError(argv[1]);
		LogEvent("File create faild!");
		return -2;
    }

	if (!DeviceIoControl(
        FileHandle,
        FSCTL_SET_SPARSE,
        NULL,
		0,
        NULL,
        0,
		&BytesReturned,
        NULL
		))
    {
		//PrintLastError(argv[1]);
		LogEvent("Set sparse attrib faild! Use normal file.");
		//return -3;
		SetSparseOK=false;
	}
	else
		SetSparseOK=true;
	

	if(HaveHead)	FileSize.QuadPart=FileSize.QuadPart+DiskFileHeadSize;

	if (!SetFilePointerEx(FileHandle, FileSize, 0, FILE_BEGIN))
	{
		//PrintLastError(argv[1]);
		LogEvent("Set file pointer faild!");
		return -4;
    }

	if (!SetEndOfFile(FileHandle))
    {
		//PrintLastError(argv[1]);
		LogEvent("Set end of file faild!");
		return -5;
    }

	CloseHandle(FileHandle);

	if(SetSparseOK)
		return 0;
	else
    	return -3;
}
//SDZ paste here 01170915<<


//SDZ added 01172054>>
//Not work properly!!!
int
GetPartitionType(
char		DriverLetter
)
{
	HANDLE                  Device;
	PARTITION_INFORMATION* 	PartitionInformation;
	DWORD                   BytesReturned;
	char*                   VolumeName="\\\\.\\c:";
	int						RetCode;

	VolumeName[4]=DriverLetter;

	Device = CreateFile(
		VolumeName,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_NO_BUFFERING,
		NULL
		);

	if (Device == INVALID_HANDLE_VALUE)
	{
		//PrintLastError(VolumeName);
		return -1;
	}

	PartitionInformation =(PARTITION_INFORMATION*) malloc(sizeof(PARTITION_INFORMATION));

	if (!DeviceIoControl(
				Device,
				IOCTL_DISK_GET_PARTITION_INFO,
				NULL,
				0,
				PartitionInformation,
				sizeof(PARTITION_INFORMATION),
				&BytesReturned,
				NULL
				))
	{
		//PrintLastError(VolumeName);
		free(PartitionInformation);
		CloseHandle(Device);
		return -2;		//Not used
	}

	RetCode=PartitionInformation->PartitionType;
	free(PartitionInformation);
	CloseHandle(Device);
	return RetCode;		//used

}
//SDZ added 01172054<<


/*
//SDZ added 01151807>>
BOOL
CheckFreeSpace(
char*	FileName,
ULARGE_INTEGER	FileSize)
{
	return true;
}
//SDZ added 01151807<<
*/

//SDZ	added	01110955>>
//To be optimazed...
BOOL
IsFileExsist(
char*	FileName,
BOOL	FindPath)
{
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;
	//BOOL fFinished = FALSE;

	hSearch = FindFirstFile(FileName, &FileData);

	if (hSearch == INVALID_HANDLE_VALUE)
	{
		FindClose (hSearch);
		return false;
	}
	else
	{
		if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(FindPath)
			{
				FindClose(hSearch);
				return true;
			}

			//while (1)		//SDZ modified 01122144
			{
				if (!FindNextFile(hSearch, &FileData))
				{
					/*
					if (GetLastError() == ERROR_NO_MORE_FILES)
					{
						FindClose(hSearch);
						return false;
					}
					else
					*/
					//{
					FindClose(hSearch);
					return false;
					//}
				}
				else if(!(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					FindClose(hSearch);
					return true;
				}
				else
				{
					if(FindPath)
					{
						FindClose(hSearch);
						return true;
					}
				}

			}
		}
		else
		{
			FindClose(hSearch);
			return true;
		}
	}

	return false;
}
//SDZ	added	01110955<<

void AESEncryptCBC(unsigned char *datap,DWORD anz,ulong32 devSect[4],symmetric_key *key);
void AESDecryptCBC(unsigned char *datap,DWORD anz,ulong32 devSect[4],symmetric_key *key);
//sdz added 01211752>>
void AddKey(POPEN_FILE_INFORMATION  ka,char *key,int len);

int
CheckPassword(
POPEN_FILE_INFORMATION	FileInfo,
POPEN_FILE_INFORMATION	ka,
char*					FileName
)
{
	FILE			*DiskFile;
	DiskFileHead	FileHead;
	int				iResult;
	ulong32 devSect[4];

	DiskFile=fopen(FileName,"rb");
	if(!DiskFile)	return -1;

	UCHAR	hash[MAX_HASH_LENGTH];
	memset(hash,0,MAX_HASH_LENGTH);
	memset(&FileHead,0,sizeof(DiskFileHead));

	iResult=fread(&FileHead,1,sizeof(DiskFileHead),DiskFile);

	FileInfo->KeyNum=0;
	AddKey(FileInfo,ka->Key[0],ka->KeyLength);

	union {
		fish2_key fk;
		symmetric_key aes;
	} k;
	memset(&k,0,sizeof(k));
	memset(devSect,0,sizeof(devSect));

	switch (FileInfo->KeyType ) {
		case 0:
			if(FileHead.HashType==0 && !memcmp(hash,FileHead.CheckHash,MAX_HASH_LENGTH))
			{
				FileInfo->KeyType=0;
				FileInfo->KeyNum=0;
				FileInfo->KeyLength=0;

				return 0;
			}
            break;
		case 1:
			memcpy(k.fk.key,FileInfo->Key[FileInfo->KeyNum-1],FileInfo->KeyLength);
			k.fk.keyLen = FileInfo->KeyLength<<3;
			init_key(&k.fk);

			iResult=blockDecrypt_CBC(&k.fk,(char*)&FileHead.HashType,(char*)&FileHead.HashType,sizeof(DiskFileHead));
			if(iResult)
				return -2;
			if(FileHead.HashType>MAX_HASH_LENGTH)
				break;

			RMD160_CTX rctx;
			memset(&rctx,0,sizeof(RMD160_CTX));
			RMD160Init(&rctx);
			RMD160Update(&rctx,(UCHAR*)&FileHead.Version,512);
			RMD160Final(hash,&rctx);

			break;
		case 2:		//AES256
			if (rijndael_setup(FileInfo->Key[FileInfo->KeyNum-1], 32, 0, &k.aes)) {
				//SDZ	PrintLastError("AES256 KeySetup faild");
				return -3;
			}
			AESDecryptCBC((char*)&FileHead.HashType,sizeof(DiskFileHead),devSect,&k.aes);
			if(FileHead.HashType>MAX_HASH_LENGTH)
				break;
			sha512_hash_buffer((UCHAR*)&FileHead.Version,512,hash,FileHead.HashType);

			break;
		case 3:		//AES128
			if (rijndael_setup(FileInfo->Key[FileInfo->KeyNum-1], 16, 0, &k.aes)) {
				//SDZ	PrintLastError("AES128 KeySetup faild");
				return -3;
			}
			AESDecryptCBC((char*)&FileHead.HashType,sizeof(DiskFileHead),devSect,&k.aes);
			if(FileHead.HashType>MAX_HASH_LENGTH)
				break;

			sha256_hash_buffer((UCHAR*)&FileHead.Version,512,hash,FileHead.HashType);

			break;
		case 4:		//AES192
			if (rijndael_setup(FileInfo->Key[FileInfo->KeyNum-1], 24, 0, &k.aes)) {
				//SDZ	PrintLastError("AES192 KeySetup faild");
				return -3;
			}
			AESDecryptCBC((char*)&FileHead.HashType,sizeof(DiskFileHead),devSect,&k.aes);
			if(FileHead.HashType>MAX_HASH_LENGTH)
				break;

			sha384_hash_buffer((UCHAR*)&FileHead.Version,512,hash,FileHead.HashType);

			break;
	}

	int icount=memcmp(hash,FileHead.CheckHash,FileHead.HashType);
	if(!icount)
	{
		memcpy(FileInfo->Key[FileInfo->KeyNum-1],FileHead.MasterKey,FileHead.MKLength);
		FileInfo->KeyLength=FileHead.MKLength;
		fclose(DiskFile);
		return 0;
	}
	else
	{
		fclose(DiskFile);
		return -1;
    }
}
//sdz added 01211752<<

//SDZ added 0602141452>>
/*
int
ChangePassword(
POPEN_FILE_INFORMATION	FileInfo,
char*					FileName
)
{


	return 0;
}
*/
//SDZ added 0602141452<<

int
GetRandom(
BYTE	*randomBytes,
int		RandomLen
);
//SDZ added 01202010>>
int
WriteFileHead(
POPEN_FILE_INFORMATION	FileInfo,
char*					FileName,
BOOL					ChangePass
)
{
	FILE	*DiskFile;
	//UCHAR	MasterKey[MAX_KEY_LENGTH];
	ulong32 devSect[4];
	int iResult;
	
	//Open the diskfile
	DiskFile=fopen(FileName,"rb+");
	if(!DiskFile)
		return -1;

	DiskFileHead	FileHead;//,*pFileHead;
	//DiskFileHead	FileHead2;

	memset(&FileHead,0,sizeof(DiskFileHead));
	memset(devSect,0,sizeof(devSect));
	//memset(&FileHead2,0,sizeof(FileHead2));

	//memset(MasterKey,0,MAX_KEY_LENGTH);

	//Fill the FileHead info
	FileHead.Version=DiskFileVersion;
	FileHead.CipherType=FileInfo->KeyType;
	FileHead.MKLength=FileInfo->KeyLength;
	FileHead.HashType=FileInfo->KeyLength;

	//Get a random masterkey for encryption
	//sdz modified 0602141517 to support pass changing,hehe.
	if(ChangePass)
	{
        memcpy(FileHead.MasterKey,FileInfo->Key[0],FileHead.MKLength);
	}
	else
	{
		iResult=GetRandom(FileHead.MasterKey,FileHead.MKLength);
		if(iResult)
		{
			fclose(DiskFile);
			return -3;
		}
	}
	//Fill the padding area with random numbers
	iResult=GetRandom(FileHead.Padding1,sizeof(FileHead.Padding1));
	GetRandom(FileHead.Padding2,sizeof(FileHead.Padding2 ));
	if(FileHead.CipherType)
		GetRandom(FileHead.CheckHash+FileHead.HashType,sizeof(FileHead.CheckHash )-FileHead.HashType );
	GetRandom(FileHead.MasterKey+FileHead.MKLength,sizeof(FileHead.MasterKey )-FileHead.MKLength );

	union {
		fish2_key fk;
		symmetric_key aes;
	} k;
	memset(&k,0,sizeof(k));

	switch (FileHead.CipherType) {
		case 1:		//2fish
			RMD160_CTX rctx;

			RMD160Init(&rctx);
			RMD160Update(&rctx,(UCHAR*)&FileHead.Version,512);
			RMD160Final(FileHead.CheckHash,&rctx);

			memcpy(k.fk.key,FileInfo->Key[FileInfo->KeyNum-1],FileInfo->KeyLength);
			k.fk.keyLen = FileInfo->KeyLength<<3;
			init_key(&k.fk);

			iResult=blockEncrypt_CBC(&k.fk,(char*)&FileHead.HashType,(char*)&FileHead.HashType ,
				sizeof(DiskFileHead));
			if(iResult)
				return -3;

			break;
		case 2:		//AES256
			sha512_hash_buffer((UCHAR*)&FileHead.Version,512,
				FileHead.CheckHash,FileHead.HashType);
			if (rijndael_setup(FileInfo->Key[FileInfo->KeyNum-1], 32, 0, &k.aes)) {
				//SDZ	PrintLastError("AES256 KeySetup faild");
				return -3;
			}
			AESEncryptCBC((char*)&FileHead.HashType,sizeof(DiskFileHead),devSect,&k.aes);

			break;
		case 3:		//AES128
			sha256_hash_buffer((UCHAR*)&FileHead.Version,512,
				FileHead.CheckHash,FileHead.HashType);
			if (rijndael_setup(FileInfo->Key[FileInfo->KeyNum-1], 16, 0, &k.aes)) {
				//SDZ	PrintLastError("AES128 KeySetup faild");
				return -3;
			}
			AESEncryptCBC((char*)&FileHead.HashType,sizeof(DiskFileHead),devSect,&k.aes);
			break;
		case 4:		//AES192
			sha384_hash_buffer((UCHAR*)&FileHead.Version,512,
				FileHead.CheckHash,FileHead.HashType);
			if (rijndael_setup(FileInfo->Key[FileInfo->KeyNum-1], 24, 0, &k.aes)) {
				//SDZ	PrintLastError("AES192 KeySetup faild");
				return -3;
			}
			AESEncryptCBC((char*)&FileHead.HashType,sizeof(DiskFileHead),devSect,&k.aes);

			break;

	}

	//Fill the checkhash field in the FileHead

	//Copy masterkey to FileInfo structure
	//memcpy(MasterKey,FileHead.MasterKey,FileHead.MKLength);

	if(fwrite(&FileHead,1,sizeof(DiskFileHead),DiskFile)!=sizeof(DiskFileHead))
	{
		fclose(DiskFile);
		return -2;
	}

	fclose(DiskFile);

	//memcpy(FileInfo->Key[FileInfo->KeyNum-1],MasterKey,FileInfo->KeyLength);

	return 0;
}
//SDZ added 01202010<<


//SDZ added 01201702>>
int
CreateDiskFile(
POPEN_FILE_INFORMATION	FileInfo,
POPEN_FILE_INFORMATION	ka,
char*					FileName)
{
	if(IsFileExsist(FileInfo->FileName,0))
		return -1;

	int iResult=CreateSparseFile(FileName,FileInfo->FileSize,true);
	if((iResult!=-3) && (iResult!=0))
	{
		return -2;
	}

	iResult=WriteFileHead(FileInfo,FileName,FALSE);

	if(iResult)//;
	{
		return -3;
	}

	return 0;
}
//SDZ added 01201702<<

//SDZ modified 01121946
void FormatLastError(char* Prefix)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0,
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
		);

	sprintf((LPTSTR)lpMsgBuf, "%s %s", Prefix, (LPTSTR) lpMsgBuf);
	LogEvent((LPTSTR)lpMsgBuf);

	LocalFree(lpMsgBuf);
}


int
FileDiskMount(
    int                     DeviceNumber,
    POPEN_FILE_INFORMATION  OpenFileInformation,
    char                    DriveLetter,
    BOOLEAN                 CdImage
)
{
    char    VolumeName[] = "\\\\.\\ :";
	char    DeviceName[MAX_PATH];
    HANDLE  Device;
	DWORD   BytesReturned;

    VolumeName[4] = DriveLetter;

    Device = CreateFile(
		VolumeName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
		OPEN_EXISTING,
		FILE_FLAG_NO_BUFFERING,
		NULL
		);

    if (Device != INVALID_HANDLE_VALUE)
    {
		//SetLastError(ERROR_BUSY);
		/*
		char* ErrorString;
		sprintf(ErrorString,"Volume %s not valid.",VolumeName);
		ReportError(MainForm->StatusBar,ErrorString);
		*/
		CloseHandle(Device);//SDZ added.
        return -1;
    }

    if (CdImage)
    {
		sprintf(DeviceName, DEVICE_NAME_PREFIX "Cd" "%u", DeviceNumber);
    }
    else
    {
        sprintf(DeviceName, DEVICE_NAME_PREFIX "%u", DeviceNumber);
    }

    if (!DefineDosDevice(
        DDD_RAW_TARGET_PATH,
        &VolumeName[4],
        DeviceName
        ))
	{
		/*
		char* ErrorString;
		sprintf(ErrorString,"DefineDosDevice error,%s",VolumeName);
		ReportError(MainForm->StatusBar,ErrorString);
		*/
		CloseHandle(Device);//SDZ added.
		return -1;
    }

    Device = CreateFile(
        VolumeName,
		GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
		);

    if (Device == INVALID_HANDLE_VALUE)
    {
		/*char* ErrorString;
		sprintf(ErrorString,"CreateDeviceFile error,%s",VolumeName);
		ReportError(MainForm->StatusBar,ErrorString);
        */
		DefineDosDevice(DDD_REMOVE_DEFINITION, &VolumeName[4], NULL);
		//CloseHandle(Device);//SDZ added.
        return -1;
    }

    if (!DeviceIoControl(
        Device,
        IOCTL_FILE_DISK_OPEN_FILE,
        OpenFileInformation,
        sizeof(OPEN_FILE_INFORMATION) + OpenFileInformation->FileNameLength - 1,
		NULL,
        0,
        &BytesReturned,
        NULL
        ))
	{
		//printf("Fn: %s\n",OpenFileInformation->FileName);
		//PrintLastError("YourDisk:");
		/*
		char* ErrorString;
		sprintf(ErrorString,"IO.OpenDiskFile error,File: %s",OpenFileInformation->FileName);
		ReportError(MainForm->StatusBar,ErrorString);
        */
		DefineDosDevice(DDD_REMOVE_DEFINITION, &VolumeName[4], NULL);
		CloseHandle(Device);//SDZ added.
        return -1;
    }

	CloseHandle(Device);//SDZ added.
    return 0;
}

int FileDiskUmount2(char *VolumeName)
{
	char DriveLetter[4];
    HANDLE  Device;
    DWORD   BytesReturned;
    POPEN_FILE_INFORMATION OpenFileInformation;

	strcpy(DriveLetter," :");

    Device = CreateFile(
        VolumeName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
		OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
        );

    if (Device == INVALID_HANDLE_VALUE)
	{
		/*
		char* ErrorString;
		sprintf(ErrorString,"Open Device %s error.",VolumeName);
		ReportError(MainForm->StatusBar,ErrorString);
		*/
		return -3;
    }

		OpenFileInformation =(OPEN_FILE_INFORMATION*) malloc(sizeof(OPEN_FILE_INFORMATION) + MAX_PATH);

		if (!DeviceIoControl(
					Device,
					IOCTL_FILE_DISK_QUERY_FILE,
					NULL,
					0,
					OpenFileInformation,
					sizeof(OPEN_FILE_INFORMATION) + MAX_PATH,
					&BytesReturned,
					NULL
					))
		{

			//ReportError(MainForm->StatusBar,"IO.QueryFile not valid.");
			
			free(OpenFileInformation);
			CloseHandle(Device);
			return -2;
		}
	DriveLetter[0] = OpenFileInformation->DriveLetter;

	free(OpenFileInformation);
	if (!DriveLetter[0]) {
		CloseHandle(Device);
		return 0;
	}



    if (!DeviceIoControl(
        Device,
        FSCTL_LOCK_VOLUME,
        NULL,
        0,
		NULL,
        0,
        &BytesReturned,
        NULL
        ))
	{
		/*char* ErrorString;
		sprintf(ErrorString,"LockVolume Error.");
		ReportError(MainForm->StatusBar,ErrorString);
		*/
		//return -1;
    }

    if (!DeviceIoControl(
        Device,
        IOCTL_FILE_DISK_CLOSE_FILE,
        NULL,
        0,
		NULL,
        0,
        &BytesReturned,
        NULL
        ))
    {

		//ReportError(MainForm->StatusBar,"IOCloseDiskFileError.");
        return -1;
    }

    if (!DeviceIoControl(
		Device,
        FSCTL_DISMOUNT_VOLUME,
        NULL,
        0,
        NULL,
        0,
		&BytesReturned,
        NULL
        ))
    {
		//ReportError(MainForm->StatusBar,"DismountVolume Error.");
        return -1;
	}

    if (!DeviceIoControl(
        Device,
        FSCTL_UNLOCK_VOLUME,
        NULL,
		0,
        NULL,
        0,
        &BytesReturned,
        NULL
        ))
	{
		//ReportError(MainForm->StatusBar,"UnlockVolume Error.");
        //return -1;
    }

    CloseHandle(Device);


    if (!DefineDosDevice(
        DDD_REMOVE_DEFINITION,
		DriveLetter,
        NULL
		))
    {
		/*
		char* ErrorString;
		sprintf(ErrorString,"Remove %c Error.",DriveLetter);
		ReportError(MainForm->StatusBar,ErrorString);
		*/
		return -1;
    }

	//printf("%s: unmounted\n",DriveLetter);
    return 0;
}


int FileDiskUmount(char DriveLetter)
{
	    char VolumeName[60] = "\\\\.\\ :";
		int i;

		if (DriveLetter) {
			VolumeName[4] = DriveLetter;

			i= FileDiskUmount2(VolumeName);
			//SDZ	if (i) PrintLastError("Unmount?");
			return i;
		}
		for (i=0;i<20;i++) {
			sprintf(VolumeName,"\\\\.\\EZCrypt%d",i);
			if (FileDiskUmount2(VolumeName) == -3) break;
		}
		/*
		for (i=0;i<20;i++) {
			sprintf(VolumeName,"\\\\.\\EZCryptCd%d",i);
			if (FileDiskUmount2(VolumeName) == -3) break;
		}
		*/
		return 0;
}


char *cvfile(char *c)
{
	static char buf[MAX_PATH+1];

	if (c[0] == '\\') {
		if (c[1] == '\\' && c[2] != '.') {
	        // \\server\share\path\yourDisk.img
             strcpy(buf, "\\.\\UNC");
			 strcat(buf,c+ 1);
		 } else {  // \Device\Harddisk0\Partition1\path\yourDisk.img
			return c;
		 }
	} else {	// c:\path\yourDisk.img
        strcpy(buf, "\\\\.\\");
        strcat(buf, c);
    }
	return buf;
}

void AESEncryptCBC(unsigned char *datap,DWORD anz,ulong32 devSect[4],symmetric_key *key)
{
	ulong32 iv[4];
    int ivCounter = 0;
    int cnt = anz >> 4;

    do {
		if(!ivCounter) {
			memcpy(iv,devSect,16);
            if(!++devSect[0] && !++devSect[1] && !++devSect[2]) devSect[3]++;
        }
        ivCounter++;
		ivCounter &= 31;
        iv[0] ^= *((ulong32 *)(&datap[ 0]));
        iv[1] ^= *((ulong32 *)(&datap[ 4]));
        iv[2] ^= *((ulong32 *)(&datap[ 8]));
        iv[3] ^= *((ulong32 *)(&datap[12]));
		rijndael_ecb_encrypt((unsigned char *)iv,datap,key);
        //aes_encrypt(&ctx, (unsigned char *)(&iv[0]), datap);
        memcpy(&iv[0], datap, 16);
        datap += 16;
    } while(--cnt);
}

void AESDecryptCBC(unsigned char *datap,DWORD anz,ulong32 devSect[4],symmetric_key *key)
{
    ulong32 iv[8];
	int ivCounter = 0;
    int cnt = anz >> 4;

    do {
		if(!ivCounter) {
			memcpy(iv,devSect,16);
		    if(!++devSect[0] && !++devSect[1] && !++devSect[2]) devSect[3]++;
		}

        ivCounter++;
		ivCounter &= 31;
        memcpy(&iv[4], datap, 16);
        rijndael_ecb_decrypt(datap,datap,key);

        *((ulong32 *)(&datap[ 0])) ^= iv[0];
		*((ulong32 *)(&datap[ 4])) ^= iv[1];
        *((ulong32 *)(&datap[ 8])) ^= iv[2];
        *((ulong32 *)(&datap[12])) ^= iv[3];
        memcpy(&iv[0], &iv[4], 16);
        datap += 16;
	} while(--cnt);
}



int EnDecrypt(int enc,POPEN_FILE_INFORMATION ka,char *fin,char *fout,int OpType)
{
	HANDLE inH;
	HANDLE outH;
	int err = 1;
	static unsigned char buf[128*512];
	char		OutMsg[32];		//SDZ added 01122013
	DWORD anz;
	DWORD wr;
	long tot = 0;


	union {
		fish2_key fk;
		symmetric_key aes;
	} k;
	char tmp[64];
	ulong32 devSect[4];

	unsigned char *obuf;//SDZ modified 200601091450 org://char *obuf;

	obuf = buf;

	memset(devSect,0,sizeof(devSect));

	if (!CreateNewFile)	fout=fin;

	inH = CreateFile(cvfile(fin),
				GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (inH == INVALID_HANDLE_VALUE)
    {
		//SDZ	PrintLastError(cvfile(fin));
		return -1;
    }
	outH = CreateFile(cvfile(fout),
				GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);

    if (outH == INVALID_HANDLE_VALUE)
    {
		CloseHandle(inH);
		//SDZ	PrintLastError(cvfile(fout));
        return -1;
    }
	memset(&k,0,sizeof(k));

	switch (ka->KeyType) {
		case 1:		//2fish
			memcpy(k.fk.key,ka->Key[0],ka->KeyLength);
			k.fk.keyLen = 20<<3;
			init_key(&k.fk);
			break;
		case 2:		//AES256
			if (rijndael_setup(ka->Key[0], 32, 0, &k.aes)) {
				//SDZ	PrintLastError("AES256 KeySetup faild");
			}
			break;
		case 3:		//AES128
			if (rijndael_setup(ka->Key[0], 16, 0, &k.aes)) {
				//SDZ	PrintLastError("AES128 KeySetup faild");
			}
			break;
		case 4:		//AES192
			if (rijndael_setup(ka->Key[0], 24, 0, &k.aes)) {
				//SDZ	PrintLastError("AES192 KeySetup faild");
			}
			break;
	}

	while (1) {
		int i;

		if (!ReadFile(inH,buf,sizeof(buf),&anz,NULL)) {
			//SDZ	PrintLastError("Read error");
			break;
		}
		//512 Byte padding
		while (anz & 0x1ff) buf[anz++] = 0;

		tot += anz;

		//sprintf(OutMsg,"Reading %3.2f MB\r",(double)tot / (1024.0*1024.0));
		//MainForm->ReportError(OutMsg,4);
		if (anz == 0) {
			//printf("OK %3.2f MB\r",(double)tot / (1024.0*1024.0));
			err = 0;
			break;
		}

		switch (ka->KeyType) {
		case 0:
			break;
		case 1:
			if (enc) {
				blockEncrypt_CBC(&k.fk,buf,obuf,anz);
			} else {
				blockDecrypt_CBC(&k.fk,buf,obuf,anz);

			}
			break;
		case 2:	//AES256
		case 3:	//AES128
		case 4:	//AES192
			if (enc) {
				AESEncryptCBC(buf,anz,devSect,&k.aes);
			} else {
				AESDecryptCBC(buf,anz,devSect,&k.aes);
			}
			break;

		}

		if (!WriteFile(outH,obuf,anz,&wr,NULL)) {
			//SDZ	PrintLastError("\nWrite Error");
			err = 1;
			break;
		}
		if (wr != anz) {
			//SDZ	PrintLastError("\nWrite Error");
			err = 1;
			break;
		}
		if (anz < sizeof(buf)) {
			//sprintf(OutMsg," %3.2f MB De/Encrypted...",(double)tot / (1024.0*1024.0));
			//ReportError(MainForm->StatusBar,OutMsg,4);		//SDZ added 01122015
			err = 0;
			break;
		}

	}


	CloseHandle(outH);
   	CloseHandle(inH);
   	memset(&k,0,sizeof(k));		//kill keys
   	return err;

}

///*SDZ comment 200601082050
int
GetRandom(
BYTE	*randomBytes,
int		RandomLen
)
{
	int i,i2;
	HCRYPTPROV hProvider = 0;

	if (!CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		//SDZ	PrintLastError("CryptAcquireContext:");
		exit(1);
	}

	if (!CryptGenRandom(hProvider, RandomLen, randomBytes)) {
		CryptReleaseContext(hProvider,0);
		//SDZ	PrintLastError("CryptAcquireContext:");
		exit(1);
	}

	CryptReleaseContext(hProvider,0);
	return 0;
}
//*/

void AddKey(POPEN_FILE_INFORMATION  ka,char *key,int len)
{
	RMD160_CTX rctx;

	if (ka->KeyNum >= MAX_KEYS) return;

	switch (ka->KeyType) {
		case 1:		//2fish
			ka->KeyLength=20;

			RMD160Init(&rctx);
			RMD160Update(&rctx,key,len);
			RMD160Final(ka->Key[ka->KeyNum++],&rctx);
			break;
		case 2:		//AES256
			ka->KeyLength=32;
			sha512_hash_buffer(key,len,ka->Key[ka->KeyNum++],ka->KeyLength);
			break;
		case 3:		//AES128
			ka->KeyLength=16;
			sha256_hash_buffer(key,len,ka->Key[ka->KeyNum++],ka->KeyLength);
			break;
		case 4:		//AES192
			ka->KeyLength=24;
			sha384_hash_buffer(key,len,ka->Key[ka->KeyNum++],ka->KeyLength);
			break;
	}

}

//#define MAX_KEY_LENGTH 500

USHORT ReadKey(POPEN_FILE_INFORMATION ka,int keys,int test) {
	 DWORD dwConsoleMode;
	HANDLE hConIn;
	char buf[MAX_KEY_LENGTH+20];
	int anz,i;

	 hConIn = GetStdHandle(STD_INPUT_HANDLE);
     GetConsoleMode(hConIn, &dwConsoleMode);
     dwConsoleMode &= ~(ENABLE_ECHO_INPUT);		//
	 dwConsoleMode |= (ENABLE_LINE_INPUT);
     SetConsoleMode(hConIn,dwConsoleMode);

	for (i = 0;i<keys;i++) {
		memset(buf,0,sizeof(buf));
		//fprintf(stdout,"%d.Passphrase:",i+1);
		if (fgets(buf,MAX_KEY_LENGTH+19,stdin) == NULL) break;	//eof
		anz = strlen(buf);
		if (buf[anz-1] == '\r' || buf[anz-1] == '\n') buf[--anz]=0;

		if (anz == 0) {
			//fprintf(stderr,"ERROR: No key supplied exit\n");
			exit(1);
		}
		if (anz > MAX_KEY_LENGTH) {
			//fprintf(stderr,"Sorry Key too long\n");
			exit(1);
		}
		if (test) {
			char buf2[MAX_KEY_LENGTH+20];
			memset(buf2,0,sizeof(buf2));

			//fprintf(stdout,"\rRetype %d.Passphrase:",i+1);
			if (fgets(buf2,MAX_KEY_LENGTH+19,stdin) == NULL) break;	//eof
			anz = strlen(buf2);
			if (buf2[anz-1] == '\r' || buf2[anz-1] == '\n') buf2[--anz]=0;

			if (strcmp(buf,buf2)) {
				memset(buf,0,sizeof(buf));		//kill key
				memset(buf2,0,sizeof(buf2));		//kill key
				//fprintf(stderr,"Passwords do not match\n");
				exit(1);
			}
			memset(buf2,0,sizeof(buf2));		//kill key

		}
		//printf("\n");
		AddKey(ka,buf,anz);
		memset(buf,0,sizeof(buf));		//kill key

		if (anz < 20) fprintf(stderr,"WARNING: Key will be to short for linux (min 20 chars) %d\n",anz);
	}
	if (i<keys) {
		//fprintf(stderr,"ERROR: %d keys missing exit\n",keys-i);
		exit(1);
	}

	return 0;
}

int DevUsed(char *VolumeName)
{
	HANDLE                  Device;
	POPEN_FILE_INFORMATION  OpenFileInformation;
	DWORD                   BytesReturned;

	Device = CreateFile(
		VolumeName,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_NO_BUFFERING,
		NULL
		);

	if (Device == INVALID_HANDLE_VALUE)
	{
		//PrintLastError(VolumeName);
		return -1;
	}

	OpenFileInformation =(OPEN_FILE_INFORMATION*) malloc(sizeof(OPEN_FILE_INFORMATION) + MAX_PATH);

	if (!DeviceIoControl(
				Device,
				IOCTL_FILE_DISK_QUERY_FILE,
				NULL,
				0,
				OpenFileInformation,
				sizeof(OPEN_FILE_INFORMATION) + MAX_PATH,
				&BytesReturned,
				NULL
				))
	{
		//PrintLastError(VolumeName);
		free(OpenFileInformation);
		CloseHandle(Device);
		return 0;		//Not used
	}

	free(OpenFileInformation);
	CloseHandle(Device);
	return 1;		//used

}

int GetFreeDev(BOOLEAN CdImage)
{
	int i,stat;
	char VolumeName[50];

	if (!CdImage) {
		for (i=0;i<20;i++) {
			sprintf(VolumeName,"\\\\.\\EZCrypt%d",i);
			stat = DevUsed(VolumeName);
			if (stat <= -1) return -1;	//Not found
			if (!stat) return  i;		//Not used
		}
	}
	/*
	 else {
		for (i=0;i<20;i++) {
			sprintf(VolumeName,"\\\\.\\EZCryptCd%d",i);
			stat = DevUsed(VolumeName);
			if (stat <= -1) return -1;	//Not found
			if (!stat) return  i;		//Not used
		}
	}
	*/
	return -1;

}

//Return next free DosDevice
char GetFreeDrive()
{
	int i;
	DWORD ld = GetLogicalDrives();


	if (ld == 0) return (char)0;
	for (i=3;i<26;i++) {
		if ((ld & (1<<i)) == 0) return 'A'+i;
	}
	return 0;		//No free drive
}

int MainEntry(int argc, char* argv[])
{
	char*                   Command;
    int                     DeviceNumber;
	char					FileName[MAX_PATH];
	char*                   Option;
    char                    DriveLetter;
	BOOLEAN                 CdImage = FALSE;
	BOOLEAN                 ReadOnly = FALSE;
	BOOLEAN					ChangePass=FALSE;
    POPEN_FILE_INFORMATION  OpenFileInformation;
    int i;
	char par[8][MAX_PATH];
	int pind=0;
    OPEN_FILE_INFORMATION  ka;
	int multiplekey = 0;
    int testkey = 0;


	char	LogStr[512];
	memset(LogStr,0,2);
	sprintf(LogStr,"CommandLine: ");

	for(i=0;i<argc;i++)
	{
		strcat(LogStr," ");
		strcat(LogStr,argv[i]);
	}

	LogEvent(LogStr);

	memset(par,0,sizeof(par));
	memset(&ka,0,sizeof(ka));

	//printf("sha_test %d \n",sha512_test());
	//printf("aes_test %d \n",aes_test());

	char *c;

	//SDZ added 02020911>>
	/*
	for(i=0;i<argc;i++)
	{
		//pind+=strlen(argv[i]);
		par[i]=(char*)malloc(strlen(argv[i])+2);
	}
    */
	//SDZ added 02020911<<

	for (i = 2;i<argc;i++) {

		if (!strncmp(argv[i],"/2f",3)) {
			ka.KeyType = 1;
			c = strchr(argv[i],':');
			if (c) //AddKey(&ka,c+1,strlen(c+1));
			{
				memcpy(ka.Key[0],c+1,strlen(c+1));
				ka.KeyLength=strlen(c+1);
			}

		} else if (!strncmp(argv[i],"/none",5)) {		//SDZ added
			ka.KeyType=0;
		} else if (!strncmp(argv[i],"/head",5)) {
			ka.KeyType=513;
			/*
			c = strchr(argv[i],':');
			if (c){
				memcpy(ka.Key[0],c+1,strlen(c+1));
				ka.KeyLength=strlen(c+1);
			}
			*/
			c = strchr(argv[i],':');
			if (c) //AddKey(&ka,c+1,strlen(c+1));
			{
				memcpy(ka.Key[0],c+1,strlen(c+1));
				ka.KeyLength=strlen(c+1);
			}

		}
		else if(!strncmp(argv[i],"/chpass",7))
		{
			ka.KeyType=513;
			c=strchr(argv[i],':');
			if(c)
			{
				memcpy(ka.Key[0],c+1,strlen(c+1));
				ka.KeyLength=strlen(c+1);
				ChangePass=TRUE;
            }
        }
		else if (!strncmp(argv[i],"/aes256",7)) {
			ka.KeyType = 2;
			/*
			c = strchr(argv[i],':');
			if (c) AddKey(&ka,c+1,strlen(c+1));
			*/
			c = strchr(argv[i],':');
			if (c) //AddKey(&ka,c+1,strlen(c+1));
			{
				memcpy(ka.Key[0],c+1,strlen(c+1));
				ka.KeyLength=strlen(c+1);
			}

		} else if (!strncmp(argv[i],"/aes128",7)) {
			ka.KeyType = 3;
			/*
			c = strchr(argv[i],':');
			if (c) AddKey(&ka,c+1,strlen(c+1));
			*/
			c = strchr(argv[i],':');
			if (c) //AddKey(&ka,c+1,strlen(c+1));
			{
				memcpy(ka.Key[0],c+1,strlen(c+1));
				ka.KeyLength=strlen(c+1);
			}

		} else if (!strncmp(argv[i],"/aes192",7)) {
			ka.KeyType = 4;
			/*
			c = strchr(argv[i],':');
			if (c) AddKey(&ka,c+1,strlen(c+1));
			*/
			c = strchr(argv[i],':');
			if (c) //AddKey(&ka,c+1,strlen(c+1));
			{
				memcpy(ka.Key[0],c+1,strlen(c+1));
				ka.KeyLength=strlen(c+1);
			}

		} else if (!strcmp(argv[i],"/cd")) {
			ReadOnly = TRUE;
			CdImage = TRUE;
		} else if (!strcmp(argv[i],"/ro")) {
			ReadOnly = TRUE;
		} else if (!strcmp(argv[i],"/m")) {
			multiplekey = 1;
		} else if (!strcmp(argv[i],"/t")) {
			testkey = 1;
		} else if (!strncmp(argv[i],"/",1)) {
			return -1;//FileDiskSyntax();
		} else {
			//par[i]=(char*)malloc(strlen(argv[pind])*4);		//SDZ added 02020926
			//memcpy(par[pind++],argv[i],strlen(argv[pind])+1);		//SDZ modified 02020917
            strcpy(par[pind++],argv[i]);
		}


	}
	/*
	if(ka.KeyType)
	{
		c = strchr(argv[i],':');
		if (c) //AddKey(&ka,c+1,strlen(c+1));
		{
			memcpy(ka.Key[0],c+1,strlen(c+1));
			ka.KeyLength=strlen(c+1);
		}
	}
	*/
	//if we need en/decryption we will need keys
	/*
	if (ka.KeyType<512 && ka.KeyType>0) {
		if (multiplekey)
		{
			if (ka.KeyNum < 64) ReadKey(&ka,64-ka.KeyNum,testkey);
		}
		else
		{
			if (ka.KeyNum < 1) ReadKey(&ka,1-ka.KeyNum,testkey);
		}
	}
	*/

	if (argc < 2) {
		return -1;//FileDiskSyntax();
	}
	Command = argv[1];


	if ((!strcmp(Command, "/mount") && pind >= 1) || ChangePass) {
		char full[_MAX_PATH];

		if (par[0][0] != '\\' && par[0][1] != ':' && _fullpath( full, par[0], _MAX_PATH ) != NULL ) {
			strcpy(FileName,full);
		} else {
			strcpy(FileName ,par[0]);
		}

		DeviceNumber = GetFreeDev(CdImage);
		if (DeviceNumber < 0) {
			//fprintf(stderr,"No free YourDisk Device\n");
			return -1;
		}


		if (strlen(FileName) < 2)
        {
			return -1;//FileDiskSyntax();
		}

		OpenFileInformation =(OPEN_FILE_INFORMATION*)
			malloc(sizeof(OPEN_FILE_INFORMATION) + strlen(FileName) + 7);

		memset(
            OpenFileInformation,
			0,
            sizeof(OPEN_FILE_INFORMATION) + strlen(FileName) + 7
			);

		ka.version = CCVERSION;

		//SDZ++	200601072037	Disk>>
		if(strstr(StrLower(FileName),"\\\\.\\")!=FileName)
		{
			if (FileName[0] == '\\')
			{
				if (FileName[1] == '\\')
					// \\server\share\path\yourDisk.img
				{
					strcpy(ka.FileName, "\\??\\UNC");
					strcat(ka.FileName, FileName + 1);
				}
				else
					// \Device\Harddisk0\Partition1\path\yourDisk.img
				{
					strcpy(ka.FileName, FileName);
				}
			}
			else
				// c:\path\yourDisk.img
			{
				strcpy(ka.FileName, "\\??\\");
				strcat(ka.FileName, FileName);
			}
		}
		else
			strcpy(ka.FileName, FileName);

		ka.FileNameLength =
			(USHORT) strlen(ka.FileName);
		ka.ReadOnly = ReadOnly;

		memcpy(OpenFileInformation,&ka,sizeof(ka)+ka.FileNameLength+7);		//CopyGenerated keys

		if (pind > 1 && par[1][0] <= '9') {	//FileSize
            Option = par[1];


            if (Option[strlen(Option) - 1] == 'G') {
                 OpenFileInformation->FileSize.QuadPart =
                      _atoi64(Option) * 1024 * 1024 * 1024;
            } else if (Option[strlen(Option) - 1] == 'M') {
                 OpenFileInformation->FileSize.QuadPart =
                        _atoi64(Option) * 1024 * 1024;
            } else if (Option[strlen(Option) - 1] == 'k') {
                OpenFileInformation->FileSize.QuadPart =
                     _atoi64(Option) * 1024;
            } else {
                  OpenFileInformation->FileSize.QuadPart =
                      _atoi64(Option);
            }
		}

		//SDZ added 01201657>>
		if(!IsFileExsist(FileName,false) && !ChangePass)
		{
			AddKey(OpenFileInformation,ka.Key[0],ka.KeyLength);
			/*
			memcpy(OpenFileInformation,&ka,sizeof(ka));
			AddKey(OpenFileInformation,OpenFileInformation->Key[0],OpenFileInformation->KeyLength);
			*/
			int iresult=CreateDiskFile(OpenFileInformation,&ka,FileName);
			if(iresult)
			{
				LogEvent("Create DiskFile faild!");
				return -1;
			}
		}
		//SDZ added 01201657<<

		//sdz added 01211758>>
		if(OpenFileInformation->KeyType>512)
		{
			for(i=0;i<5;i++)
			{
				//strcpy(OpenFileInformation->Key[0],ka.Key[0]);
				memcpy(OpenFileInformation,&ka,sizeof(ka));
				OpenFileInformation->KeyType=i;
				OpenFileInformation->KeyNum=0;
				if(!CheckPassword(OpenFileInformation,&ka,FileName))
					break;
				else
					if(i==4)//Maybe bug here ;-)
						return -5;
			}
		}
		else
		{
			memset(OpenFileInformation->Key[0],0,sizeof(OpenFileInformation->Key[0]));
			//strcpy(OpenFileInformation->Key[0],ka.Key[0]);
			//OpenFileInformation->KeyLength=ka.KeyLength;
			memcpy(OpenFileInformation,&ka,sizeof(ka));		//CopyGenerated keys
			if(CheckPassword(OpenFileInformation,&ka,FileName))
				return -5;
        }
		//sdz added 01211758<<

		//sdz added 0602141643>>
		if(ChangePass)
		{
			//memcpy(OpenFileInformation,&ka,sizeof(ka)+ka.FileNameLength+7);		//CopyGenerated keys
			//OpenFileInformation->KeyType=i;
			AddKey(OpenFileInformation,par[pind-1],strlen(par[pind-1]));
			//memcpy(OpenFileInformation->Key[OpenFileInformation->KeyNum],par[pind],strlen(par[pind]));
			if(WriteFileHead(OpenFileInformation,FileName,TRUE))
				return -6;
			else
            	return 0;
        }
		//sdz added 0602141643<<
		
        if (pind > 1 && par[1][0] >= 'A' && par[1][1] == ':') {
			DriveLetter = par[1][0];
		} else if (pind > 2 && par[2][0] >= 'A' && par[2][1] == ':') {
			DriveLetter = par[2][0];
		} else {
			DriveLetter = GetFreeDrive();
		}
		OpenFileInformation->DriveLetter  = DriveLetter;
		i = FileDiskMount(DeviceNumber, OpenFileInformation, DriveLetter, CdImage);

        if (!i) printf("%c: -> %s mounted\n",DriveLetter,OpenFileInformation->FileName);
        memset(&ka,0,sizeof(ka));		//kill keys
        memset(OpenFileInformation,0,sizeof(ka));
        return i;
    }
    else if (!strcmp(Command, "/umount"))
    {
		memset(&ka,0,sizeof(ka));		//kill keys
		DriveLetter = 0;
        if (pind >= 1) DriveLetter = par[0][0];
        return FileDiskUmount(DriveLetter);
	}

    else if (pind >= 2 && !strcmp(Command,"/encrypt"))
    {
		CreateNewFile=1;
		i = EnDecrypt(1,&ka,par[0],par[1],0);
		memset(&ka,0,sizeof(ka));		//kill keys
		return i;

	}
	else if (pind >= 2 && !strcmp(Command,"/decrypt"))
	{
		CreateNewFile=1;
		i = EnDecrypt(0,&ka,par[0],par[1],0);
		memset(&ka,0,sizeof(ka));		//kill keys
		return i;

	}
	//SDZ added.
	else if(pind >=1 && !strcmp(Command,"/encit"))
	{
		CreateNewFile=0;
		i = EnDecrypt(1,&ka,par[0],par[1],0);
		memset(&ka,0,sizeof(ka));		//kill keys
		return i;
	}
	//SDZ added.
	else if(pind>=1 && !strcmp(Command,"/decit"))
	{
		CreateNewFile=0;
		i = EnDecrypt(0,&ka,par[0],par[1],0);
		memset(&ka,0,sizeof(ka));		//kill keys
		return i;
	}
    else
    {
		memset(&ka,0,sizeof(ka));		//kill keys
		return -1;//FileDiskSyntax();
	}
}
//SDZ < moved here


//SDZ added>
int MakeDriverStatusInfo(
char* VolName,
char* StatusInfo,
int InfoType,
OPEN_FILE_INFORMATION *MountInfo
)
{
	HANDLE                  Device;
	POPEN_FILE_INFORMATION  OpenFileInformation;
	DWORD                   BytesReturned;

	Device = CreateFile(
		VolName,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_NO_BUFFERING,
		NULL
		);

	if (Device == INVALID_HANDLE_VALUE)
	{
		//PrintLastError(VolumeName);
		CloseHandle(Device);
		return -1;
	}
	OpenFileInformation =(OPEN_FILE_INFORMATION*)
		malloc(sizeof(OPEN_FILE_INFORMATION) + MAX_PATH);

	if (!DeviceIoControl(
				Device,
				IOCTL_FILE_DISK_QUERY_FILE,
				NULL,
				0,
				OpenFileInformation,
				sizeof(OPEN_FILE_INFORMATION) + MAX_PATH,
				&BytesReturned,
				NULL
				))
	{
		//PrintLastError(VolumeName);
		//CloseHandle(Device);
		free(OpenFileInformation);
		CloseHandle(Device);

		return -2;
	}

	if (BytesReturned < sizeof(OPEN_FILE_INFORMATION))
	{
		//SetLastError(ERROR_INSUFFICIENT_BUFFER);
		//PrintLastError(&VolumeName[4]);
		CloseHandle(Device);
		return -2;
	}
	if(InfoType==1)
	{
		sprintf(StatusInfo,"%c: -> %s",
				OpenFileInformation->DriveLetter,
				//OpenFileInformation->FileNameLength,
				OpenFileInformation->FileName
				);

	}else
	{
		memcpy(MountInfo,OpenFileInformation,sizeof(OPEN_FILE_INFORMATION));
		sprintf(StatusInfo,"%c: ---> %I64u MB ---> %s",
				OpenFileInformation->DriveLetter,
				//OpenFileInformation->FileNameLength,
				//OpenFileInformation->FileName,
				OpenFileInformation->FileSize.QuadPart/1024/1024,
				OpenFileInformation->ReadOnly ? "RO" : "RW"
				);
	}
	free(OpenFileInformation);
	CloseHandle(Device);
	return 0;
}

void ListAllDrivers(TStringGrid *StringGridDrvInfo)
{
/*
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
*/
	int ret;
	String tempStr;
	
	if(!StringGridDrvInfo->Enabled || !StringGridDrvInfo->Visible)
		return;
		
	StringGridDrvInfo->RowCount=0;

	char VolumeName[60] = "\\\\.\\ :";
	char DriverStatus[MAX_PATH+32];

	int i,j,mountedNum=0;		//sdz 20070825
	for(i=0;i<StringGridDrvInfo->RowCount;i++)
		for(j=0;j<StringGridDrvInfo->ColCount;j++)
			StringGridDrvInfo->Cells[j][i]="";

	OPEN_FILE_INFORMATION *MountInfo;
	MountInfo =(OPEN_FILE_INFORMATION*)
			malloc(sizeof(OPEN_FILE_INFORMATION) + MAX_PATH);

	for (i=0;i<20;i++)
	{
		sprintf(VolumeName,"\\\\.\\EZCrypt%d",i);

		memset(DriverStatus,0,sizeof(DriverStatus));
		ret=MakeDriverStatusInfo(VolumeName,DriverStatus,0,MountInfo);
		if ( ret == -1) break;
		else if (!ret)
		{
			//StringGridDrvInfo->RowCount++;
			StringGridDrvInfo->RowCount=mountedNum+1;
			sprintf(VolumeName,"%C:",MountInfo->DriveLetter);
			StringGridDrvInfo->Cells[0][mountedNum]=
				VolumeName;


			switch(MountInfo->KeyType)
			{
				case 0:
					StringGridDrvInfo->Cells[1][mountedNum]=
						"None";
					break;
				case 1:
					StringGridDrvInfo->Cells[1][mountedNum]=
						"2Fish";
					break;
				case 2:
					StringGridDrvInfo->Cells[1][mountedNum]=
						"AES256";
					break;
				case 3:
					StringGridDrvInfo->Cells[1][mountedNum]=
						"AES128";
					break;
				case 4:
					StringGridDrvInfo->Cells[1][mountedNum]=
						"AES192";
					break;
				default:
					StringGridDrvInfo->Cells[1][mountedNum]=
						"N/A";
					break;
			}
			sprintf(VolumeName,"%d MB",MountInfo->FileSize.QuadPart/1024/1024);
			StringGridDrvInfo->Cells[2][mountedNum]=
				VolumeName;
			StringGridDrvInfo->Cells[3][mountedNum]=
				(MountInfo->ReadOnly!=0?"ReadOnly":"ReadWrite");

			memcpy(tempStr.c_str(),MountInfo->FileName,MountInfo->FileNameLength);
			StringGridDrvInfo->Cells[3][mountedNum]=
				tempStr;

			mountedNum++;
		}
		/*//sdz commented 02021516 error trap here...
		//CDROM support removed from driver,so here will trag an error.
		sprintf(VolumeName,"\\\\.\\EZCryptCd%d",i);
		memset(DriverStatus,0,sizeof(DriverStatus));
		ret=MakeDriverStatusInfo(VolumeName,DriverStatus,0);
		if ( ret == -1) break;
		else if (!ret)
		{
			StringGridDrvInfo->Items->Add(DriverStatus);
		}
		*/
	}

}



bool	FileMounted(char* FileName)
{
	int ret;

	char VolumeName[60] = "\\\\.\\ :";
	char DriverStatus[512];

	int i;

	for (i=0;i<20;i++) {
		sprintf(VolumeName,"\\\\.\\EZCrypt%d",i);

		memset(DriverStatus,0,sizeof(DriverStatus));
		ret=MakeDriverStatusInfo(VolumeName,DriverStatus,1,NULL);
		if ( ret == -1) break;
		else if (!ret)
		{
			StrLower(DriverStatus);
			StrLower(FileName);
			if(strstr(DriverStatus,FileName))
				return true;
		}
	}
	/*
	for (i=0;i<20;i++) {
		sprintf(VolumeName,"\\\\.\\EZCryptCd%d",i);
		memset(DriverStatus,0,sizeof(DriverStatus));
		ret=MakeDriverStatusInfo(VolumeName,DriverStatus,1);
		if ( ret == -1) break;
		else if (!ret)
		{
			if(strstr(FileName,DriverStatus))
				return true;
		}
	}
    */
	return false;
}

//SDZ added 20006.01.08	Driver install>>
//ControlCode
//	1		Install
//	2		Start
//	4		Stop
//	8		Uninstall
//	...		To be defined...
BOOL
DriverControl (unsigned int ControlCode)
{
	SC_HANDLE hManager, hService = NULL;
	BOOL bOK = FALSE, bRet, bSlash;
	char szDir[MAX_PATH];
	int x;
/*
	if (nCurrentOS != WIN_NT)
		return TRUE;
*/
	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
		return false;

	hService = OpenService (hManager, "EZCrypt", SERVICE_ALL_ACCESS);
	if (hService != NULL)
	{
		if(ControlCode&0x02)	//Start yourdisk service
		{
			bRet = StartService (hService, 0, NULL);
			if (bRet)
				return true;
			else
			{
				if(GetLastError()==ERROR_SERVICE_ALREADY_RUNNING)
					return true;
				else
				{
					DeleteService(hService);		//SDZ added 01111651
					CloseServiceHandle(hManager);
					return false;
				}
			}
		}

		if(ControlCode&0x04)	//Stop the yourdisk service
		{
			SERVICE_STATUS	ServiceStatus;
			if(ControlService(hService,SERVICE_CONTROL_STOP,&ServiceStatus))
			{
				return true;
			}
			else
				return false;
		}

		if(ControlCode&0x08)	//Remove the yourdisk service
		{
			SERVICE_STATUS	ServiceStatus;
			if(ControlService(hService,SERVICE_CONTROL_STOP,&ServiceStatus))
			{
				if(DeleteService(hService))
					return true;
				else
					return false;
			}
		}
	}
	else if(!(ControlCode&0x01))
		return false;

	//SDZ	01102109>
	AnsiString	ExePathName;

	if(ControlCode&0x01)
	{
		LogEvent("installing EZCrypt.SYS");

		GetSystemDirectory (szDir, sizeof (szDir));
		x = strlen (szDir);
		if (szDir[x - 1] == '\\')
			bSlash = TRUE;
		else
			bSlash = FALSE;

		if (bSlash == FALSE)
			strcat (szDir, "\\");

		strcat (szDir, "drivers\\EZCrypt.sys");

		if (!IsFileExsist(szDir,false))
		{
			ExePathName=Application->ExeName;
			ExePathName=ExtractFilePath(ExePathName);
			if(ExePathName.c_str()+(ExePathName.Length()-1)=="\\")
			{
				ExePathName.cat_sprintf("EZCrypt.sys");
			}
			else
				ExePathName.cat_sprintf("\\EZCrypt.sys");

			if (!IsFileExsist(ExePathName.c_str(),false))
			{
				return false;		//驱动文件不存在
			}
			sprintf(szDir,"%s",ExePathName.c_str());
		}

		hService = CreateService (hManager, "EZCrypt", "EZCrypt",
			  SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
			  SERVICE_ERROR_NORMAL,szDir, NULL, NULL, NULL, NULL, NULL);
		if (hService == NULL)
			goto error;			//创建服务失败
		//else
		//	CloseServiceHandle (hService);

		//hService = OpenService (hManager, "EZCrypt", SERVICE_ALL_ACCESS);
		//if (hService == NULL)
		//	goto error;

		bRet = StartService (hService, 0, NULL);
		if (bRet == FALSE)
			goto error;

		//SDZ	01102109<

	}

	bOK = TRUE;

error:
	if (bOK == FALSE && GetLastError ()!= ERROR_SERVICE_ALREADY_RUNNING)
	{
		//handleWin32Error (hwndDlg);
		LogEvent("The installation of the device driver has failed");
	}
	else
		bOK = TRUE;

	if (hService != NULL)
		CloseServiceHandle (hService);

	if (hManager != NULL)
		CloseServiceHandle (hManager);

	return bOK;
}
//SDZ added<<20060108

//SDZ added<

int GetMountedNum()
{
	int ret;
	//String tempStr;
	
	char VolumeName[60] = "\\\\.\\ :";
	char DriverStatus[MAX_PATH+32];

	int i,mountedNum=0;		//sdz 20070825

	OPEN_FILE_INFORMATION *MountInfo;
	MountInfo =(OPEN_FILE_INFORMATION*)
			malloc(sizeof(OPEN_FILE_INFORMATION) + MAX_PATH);

	for (i=0;i<20;i++)
	{
		sprintf(VolumeName,"\\\\.\\EZCrypt%d",i);

		//memset(DriverStatus,0,sizeof(DriverStatus));
		ret=MakeDriverStatusInfo(VolumeName,DriverStatus,0,MountInfo);
		if ( ret == -1) break;
		else if (!ret)
		{
			mountedNum++;
		}
	}

	return mountedNum;

}

//SDZ	added	200601102042>>
/*
BOOL
PrepareDriver(
BOOL		IsGreen)
{
	//先尝试打开虚拟磁盘设备，如果失败查找驱动文件，并重新建立驱动注册表
	//然后启动驱动程序...

	return true;
}
*/
//SDZ	added	200601102042<<


//sdz paste here.02061645<<

#endif
