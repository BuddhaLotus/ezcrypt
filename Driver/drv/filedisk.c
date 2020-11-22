/*
    This is a virtual disk driver for Windows NT/2000/XP that uses
    one or more files to emulate physical disks.
    Copyright (C) 1999, 2000, 2001, 2002 Bo Brant閚.
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

    modified by Stefan Scherrer to add crypto functions
*/

#include <ntddk.h>
#include <ntdddisk.h>
#include <ntddcdrm.h>
#include <ntverp.h>

#define		DiskFileHeadSize		4096		//SDZ 01192136
//
// We include some stuff from newer DDK:s here so that one
// version of the driver for all versions of Windows can
// be compiled with the Windows NT 4.0 DDK.
//
#if (VER_PRODUCTBUILD < 2195)

#define FILE_DEVICE_MASS_STORAGE            0x0000002d
#define IOCTL_STORAGE_CHECK_VERIFY2         CTL_CODE(IOCTL_STORAGE_BASE, 0x0200, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000

#endif

#if (VER_PRODUCTBUILD < 2600)

#define IOCTL_DISK_GET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0012, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_GET_LENGTH_INFO          CTL_CODE(IOCTL_DISK_BASE, 0x0017, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef enum _PARTITION_STYLE {
    PARTITION_STYLE_MBR,
    PARTITION_STYLE_GPT
} PARTITION_STYLE;

typedef unsigned __int64 ULONG64, *PULONG64;

typedef struct _PARTITION_INFORMATION_MBR {
    UCHAR   PartitionType;
    BOOLEAN BootIndicator;
    BOOLEAN RecognizedPartition;
    ULONG   HiddenSectors;
} PARTITION_INFORMATION_MBR, *PPARTITION_INFORMATION_MBR;

typedef struct _PARTITION_INFORMATION_GPT {
    GUID    PartitionType;
    GUID    PartitionId;
    ULONG64 Attributes;
    WCHAR   Name[36];
} PARTITION_INFORMATION_GPT, *PPARTITION_INFORMATION_GPT;

typedef struct _PARTITION_INFORMATION_EX {
    PARTITION_STYLE PartitionStyle;
    LARGE_INTEGER   StartingOffset;
    LARGE_INTEGER   PartitionLength;
    ULONG           PartitionNumber;
    BOOLEAN         RewritePartition;
    union {
        PARTITION_INFORMATION_MBR Mbr;
        PARTITION_INFORMATION_GPT Gpt;
    };
} PARTITION_INFORMATION_EX, *PPARTITION_INFORMATION_EX;

typedef struct _GET_LENGTH_INFORMATION {
    LARGE_INTEGER Length;
} GET_LENGTH_INFORMATION, *PGET_LENGTH_INFORMATION;

#endif // (VER_PRODUCTBUILD < 2600)

//
// We include some stuff from ntifs.h here so that
// the driver can be compiled with only the DDK.
//

#define TOKEN_SOURCE_LENGTH 8

typedef enum _TOKEN_TYPE {
    TokenPrimary = 1,
    TokenImpersonation
} TOKEN_TYPE;

typedef struct _TOKEN_SOURCE {
    CCHAR   SourceName[TOKEN_SOURCE_LENGTH];
    LUID    SourceIdentifier;
} TOKEN_SOURCE, *PTOKEN_SOURCE;

typedef struct _TOKEN_CONTROL {
    LUID            TokenId;
    LUID            AuthenticationId;
    LUID            ModifiedId;
    TOKEN_SOURCE    TokenSource;
} TOKEN_CONTROL, *PTOKEN_CONTROL;

typedef struct _SECURITY_CLIENT_CONTEXT {
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    PACCESS_TOKEN               ClientToken;
    BOOLEAN                     DirectlyAccessClientToken;
    BOOLEAN                     DirectAccessEffectiveOnly;
    BOOLEAN                     ServerIsRemote;
    TOKEN_CONTROL               ClientTokenControl;
} SECURITY_CLIENT_CONTEXT, *PSECURITY_CLIENT_CONTEXT;

#define PsDereferenceImpersonationToken(T)  \
            {if (ARGUMENT_PRESENT(T)) {     \
                (ObDereferenceObject((T))); \
            } else {                        \
                ;                           \
            }                               \
}

#define PsDereferencePrimaryToken(T) (ObDereferenceObject((T)))

NTKERNELAPI
VOID
PsRevertToSelf (
    VOID
);

NTKERNELAPI
NTSTATUS
SeCreateClientSecurity (
    IN PETHREAD                     Thread,
    IN PSECURITY_QUALITY_OF_SERVICE QualityOfService,
    IN BOOLEAN                      RemoteClient,
    OUT PSECURITY_CLIENT_CONTEXT    ClientContext
);

#define SeDeleteClientSecurity(C)  {                                           \
            if (SeTokenType((C)->ClientToken) == TokenPrimary) {               \
                PsDereferencePrimaryToken( (C)->ClientToken );                 \
            } else {                                                           \
                PsDereferenceImpersonationToken( (C)->ClientToken );           \
            }                                                                  \
}

NTKERNELAPI
VOID
SeImpersonateClient (
    IN PSECURITY_CLIENT_CONTEXT ClientContext,
    IN PETHREAD                 ServerThread OPTIONAL
);

NTKERNELAPI
TOKEN_TYPE
SeTokenType (
    IN PACCESS_TOKEN Token
);

//
// For backward compatibility with Windows NT 4.0 by Bruce Engle.
//
#ifndef MmGetSystemAddressForMdlSafe
#define MmGetSystemAddressForMdlSafe(MDL, PRIORITY) MmGetSystemAddressForMdlPrettySafe(MDL)

PVOID
MmGetSystemAddressForMdlPrettySafe (
    PMDL Mdl
    )
{
    CSHORT  MdlMappingCanFail;
    PVOID   MappedSystemVa;

    MdlMappingCanFail = Mdl->MdlFlags & MDL_MAPPING_CAN_FAIL;

    Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;

    MappedSystemVa = MmGetSystemAddressForMdl(Mdl);

    if (MdlMappingCanFail == 0)
    {
        Mdl->MdlFlags &= ~MDL_MAPPING_CAN_FAIL;
    }

    return MappedSystemVa;
}
#endif

#include "filedisk.h"

#include "twofish.h"
#include "rmd160.h"
#include "sha512.h"
#include "tom.h"


#define PARAMETER_KEY           L"\\Parameters"

#define NUMBEROFDEVICES_VALUE   L"NumberOfDevices"
#ifdef		_LITE_VERSION_
#define DEFAULT_NUMBEROFDEVICES 1
#else
#define DEFAULT_NUMBEROFDEVICES 4
#endif

#define SECTOR_SIZE             512

#define TOC_DATA_TRACK          0x04

HANDLE dir_handle;

typedef struct _DEVICE_EXTENSION {
    BOOLEAN                     media_in_device;
    HANDLE                      file_handle;
    ANSI_STRING                 file_name;
    LARGE_INTEGER               file_size;
    BOOLEAN                     read_only;
    PSECURITY_CLIENT_CONTEXT    security_client_context;
    LIST_ENTRY                  list_head;
    KSPIN_LOCK                  list_lock;
    KEVENT                      request_event;
    PVOID                       thread_pointer;
    BOOLEAN                     terminate_thread;
    USHORT			KeyType;		//0 None 1 2Fish 2 AES256 3 AES128 4 AES192
	fish2_key		f2k;
	symmetric_key 	aes;
	UCHAR			DriveLetter;
	ULONG			rdbyreq,rdbyread;
	ULONG			rdcreq,rdcread;
	ULONG			rdfpos;
	NTSTATUS		lasterr;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
);

NTSTATUS
FileDiskCreateDevice (
    IN PDRIVER_OBJECT   DriverObject,
    IN ULONG            Number,
    IN DEVICE_TYPE      DeviceType
);

VOID
FileDiskUnload (
    IN PDRIVER_OBJECT   DriverObject
);

PDEVICE_OBJECT
FileDiskDeleteDevice (
    IN PDEVICE_OBJECT   DeviceObject
);

NTSTATUS
FileDiskCreateClose (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
);

NTSTATUS
FileDiskReadWrite (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
);

NTSTATUS
FileDiskDeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
);

VOID
FileDiskThread (
    IN PVOID            Context
);

NTSTATUS
FileDiskOpenFile (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
);

NTSTATUS
FileDiskCloseFile (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
);

int swprintf(wchar_t *, const wchar_t *, ...);

int sprintf(char *, const char *, ...);

#pragma code_seg("INIT")

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
{
    UNICODE_STRING              parameter_path;
    RTL_QUERY_REGISTRY_TABLE    query_table[2];
    ULONG                       n_devices;
    NTSTATUS                    status;
    UNICODE_STRING              device_dir_name;
    OBJECT_ATTRIBUTES           object_attributes;
    ULONG                       n;
    USHORT                      n_created_devices;

    parameter_path.Length = 0;

    parameter_path.MaximumLength = RegistryPath->Length + sizeof(PARAMETER_KEY);

    parameter_path.Buffer = (PWSTR) ExAllocatePool(PagedPool, parameter_path.MaximumLength);

    if (parameter_path.Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(&parameter_path, RegistryPath);

    RtlAppendUnicodeToString(&parameter_path, PARAMETER_KEY);

    RtlZeroMemory(&query_table[0], sizeof(query_table));

    query_table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
	query_table[0].Name = NUMBEROFDEVICES_VALUE;
    query_table[0].EntryContext = &n_devices;

    status = RtlQueryRegistryValues(
        RTL_REGISTRY_ABSOLUTE,
        parameter_path.Buffer,
        &query_table[0],
        NULL,
        NULL
        );

    ExFreePool(parameter_path.Buffer);

	//KdPrint(("EZCrypt: Startup\n"));
	//DbgPrint(("EZCrypt: Startup\n"));
    if (!NT_SUCCESS(status))
    {
        //KdPrint(("EZCrypt: Query registry failed, using default values.\n"));
        n_devices = DEFAULT_NUMBEROFDEVICES;
    }

    RtlInitUnicodeString(&device_dir_name, DEVICE_DIR_NAME);

    InitializeObjectAttributes(
        &object_attributes,
        &device_dir_name,
        OBJ_PERMANENT,
        NULL,
        NULL
        );

    status = ZwCreateDirectoryObject(
        &dir_handle,
        DIRECTORY_ALL_ACCESS,
        &object_attributes
		);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    ZwMakeTemporaryObject(dir_handle);

    for (n = 0, n_created_devices = 0; n < n_devices; n++)
    {
        status = FileDiskCreateDevice(DriverObject, n, FILE_DEVICE_DISK);

        if (NT_SUCCESS(status))
        {
            n_created_devices++;
        }
    }
/*20070826
	for (n = 0; n < n_devices; n++)
	{
		status = FileDiskCreateDevice(DriverObject, n, FILE_DEVICE_CD_ROM);

		if (NT_SUCCESS(status))
		{
			n_created_devices++;
		}
	}
*/
    if (n_created_devices == 0)
    {
        ZwClose(dir_handle);
        return status;
    }

	DriverObject->MajorFunction[IRP_MJ_CREATE]         = FileDiskCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = FileDiskCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ]           = FileDiskReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]          = FileDiskReadWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = FileDiskDeviceControl;

    DriverObject->DriverUnload = FileDiskUnload;

    return STATUS_SUCCESS;
}

NTSTATUS
FileDiskCreateDevice (
    IN PDRIVER_OBJECT   DriverObject,
    IN ULONG            Number,
    IN DEVICE_TYPE      DeviceType
    )
{
    WCHAR               device_name_buffer[MAXIMUM_FILENAME_LENGTH];
    WCHAR               sym_name_buffer[MAXIMUM_FILENAME_LENGTH];
    UNICODE_STRING      device_name;
    UNICODE_STRING      sym_name;
    NTSTATUS            status;
    PDEVICE_OBJECT      device_object;
    PDEVICE_EXTENSION   device_extension;
    HANDLE              thread_handle;

    ASSERT(DriverObject != NULL);

    if (DeviceType == FILE_DEVICE_CD_ROM)
    {
        swprintf(
            device_name_buffer,
            DEVICE_NAME_PREFIX L"Cd" L"%u",
            Number
			);
        swprintf(
            sym_name_buffer,
            L"\\DosDevices\\EZCryptCd%u",
            Number
            );
    }
    else
    {
        swprintf(
            device_name_buffer,
            DEVICE_NAME_PREFIX L"%u",
            Number
            );
        swprintf(
            sym_name_buffer,
            L"\\DosDevices\\EZCrypt%u",
            Number
            );
    }

    RtlInitUnicodeString(&device_name, device_name_buffer);
    RtlInitUnicodeString(&sym_name, sym_name_buffer);

    status = IoCreateDevice(
        DriverObject,
        sizeof(DEVICE_EXTENSION),
        &device_name,
        DeviceType,	//(DeviceType == FILE_DEVICE_CD_ROM) ? FILE_DEVICE_DVD :DeviceType,
        0,
        FALSE,
        &device_object
        );

    if (!NT_SUCCESS(status))
	{
        return status;
    }

    device_object->Flags |= DO_DIRECT_IO;

    device_extension = (PDEVICE_EXTENSION) device_object->DeviceExtension;

    device_extension->media_in_device = FALSE;

    if (DeviceType == FILE_DEVICE_CD_ROM)
    {
        device_object->Characteristics |= FILE_READ_ONLY_DEVICE;
        device_extension->read_only = TRUE;
    }

    InitializeListHead(&device_extension->list_head);

    KeInitializeSpinLock(&device_extension->list_lock);

    KeInitializeEvent(
        &device_extension->request_event,
        SynchronizationEvent,
        FALSE
        );

    device_extension->terminate_thread = FALSE;

    status = PsCreateSystemThread(
        &thread_handle,
        (ACCESS_MASK) 0L,
        NULL,
        NULL,
        NULL,
        FileDiskThread,
		device_object
        );

    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(device_object);
        return status;
    }

    status = ObReferenceObjectByHandle(
        thread_handle,
        THREAD_ALL_ACCESS,
        NULL,
        KernelMode,
        &device_extension->thread_pointer,
        NULL
        );

    if (!NT_SUCCESS(status))
    {
        ZwClose(thread_handle);

        device_extension->terminate_thread = TRUE;

        KeSetEvent(
            &device_extension->request_event,
            (KPRIORITY) 0,
            FALSE
            );

        IoDeleteDevice(device_object);

        return status;
    }

	ZwClose(thread_handle);

    status = IoCreateSymbolicLink(&sym_name,&device_name);
    if (!NT_SUCCESS(status))
    {
		//KdPrint(("Link faild"));
    } else {
		//KdPrint(("Link ok"));
	}



    return STATUS_SUCCESS;
}

//#pragma code_seg("PAGE")
#pragma code_seg()

VOID
FileDiskUnload (
    IN PDRIVER_OBJECT DriverObject
    )
{
    PDEVICE_OBJECT device_object;

//    PAGED_CODE();

    device_object = DriverObject->DeviceObject;

    while (device_object)
    {
        device_object = FileDiskDeleteDevice(device_object);
    }

    ZwClose(dir_handle);
}

PDEVICE_OBJECT
FileDiskDeleteDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION   device_extension;
    PDEVICE_OBJECT      next_device_object;

//    PAGED_CODE();

    ASSERT(DeviceObject != NULL);

    device_extension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    device_extension->terminate_thread = TRUE;

    KeSetEvent(
        &device_extension->request_event,
        (KPRIORITY) 0,
        FALSE
        );

    KeWaitForSingleObject(
        device_extension->thread_pointer,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    ObDereferenceObject(device_extension->thread_pointer);

    if (device_extension->security_client_context != NULL)
	{
        SeDeleteClientSecurity(device_extension->security_client_context);
        ExFreePool(device_extension->security_client_context);
    }

    next_device_object = DeviceObject->NextDevice;

    //IoDeleteSymbolicLink(&uniLinkName);

    IoDeleteDevice(DeviceObject);

    return next_device_object;
}

NTSTATUS
FileDiskCreateClose (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
//    PAGED_CODE();

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = FILE_OPENED;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

#pragma code_seg()

NTSTATUS
FileDiskReadWrite (
    IN PDEVICE_OBJECT   DeviceObject,
	IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   device_extension;
    PIO_STACK_LOCATION  io_stack;

    device_extension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (!device_extension->media_in_device)
    {
        Irp->IoStatus.Status = STATUS_NO_MEDIA_IN_DEVICE;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_NO_MEDIA_IN_DEVICE;
    }

    io_stack = IoGetCurrentIrpStackLocation(Irp);

    if (io_stack->Parameters.Read.Length == 0)
    {
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_SUCCESS;
    }

    IoMarkIrpPending(Irp);

    ExInterlockedInsertTailList(
        &device_extension->list_head,
        &Irp->Tail.Overlay.ListEntry,
		&device_extension->list_lock
        );

    KeSetEvent(
        &device_extension->request_event,
        (KPRIORITY) 0,
        FALSE
        );

    return STATUS_PENDING;
}

NTSTATUS
FileDiskDeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   device_extension;
    PIO_STACK_LOCATION  io_stack;
    NTSTATUS            status;

    device_extension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    io_stack = IoGetCurrentIrpStackLocation(Irp);

    if (!device_extension->media_in_device &&
        io_stack->Parameters.DeviceIoControl.IoControlCode !=
        IOCTL_FILE_DISK_OPEN_FILE)
    {
        Irp->IoStatus.Status = STATUS_NO_MEDIA_IN_DEVICE;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_NO_MEDIA_IN_DEVICE;
    }

    switch (io_stack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_FILE_DISK_OPEN_FILE:
        {
            SECURITY_QUALITY_OF_SERVICE security_quality_of_service;

            if (device_extension->media_in_device)
            {
                //KdPrint(("EZCrypt: IOCTL_FILE_DISK_OPEN_FILE: Media already opened\n"));

                status = STATUS_INVALID_DEVICE_REQUEST;
                Irp->IoStatus.Information = 0;
                break;
            }

            if (io_stack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(OPEN_FILE_INFORMATION))
            {
                status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Information = 0;
                break;
            }

            if (io_stack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(OPEN_FILE_INFORMATION) +
                ((POPEN_FILE_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->FileNameLength -
                sizeof(UCHAR))
            {
                status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Information = 0;
                break;
            }

            if (device_extension->security_client_context != NULL)
            {
                SeDeleteClientSecurity(device_extension->security_client_context);
            }
            else
            {
                device_extension->security_client_context =
                    ExAllocatePool(NonPagedPool, sizeof(SECURITY_CLIENT_CONTEXT));
            }

            RtlZeroMemory(&security_quality_of_service, sizeof(SECURITY_QUALITY_OF_SERVICE));

            security_quality_of_service.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
            security_quality_of_service.ImpersonationLevel = SecurityImpersonation;
            security_quality_of_service.ContextTrackingMode = SECURITY_STATIC_TRACKING;
            security_quality_of_service.EffectiveOnly = FALSE;

            SeCreateClientSecurity(
                PsGetCurrentThread(),
                &security_quality_of_service,
                FALSE,
                device_extension->security_client_context
                );

            IoMarkIrpPending(Irp);

            ExInterlockedInsertTailList(
                &device_extension->list_head,
                &Irp->Tail.Overlay.ListEntry,
                &device_extension->list_lock
                );

            KeSetEvent(
                &device_extension->request_event,
				(KPRIORITY) 0,
                FALSE
                );

            status = STATUS_PENDING;

            break;
        }

    case IOCTL_FILE_DISK_CLOSE_FILE:
        {
            IoMarkIrpPending(Irp);

            ExInterlockedInsertTailList(
                &device_extension->list_head,
                &Irp->Tail.Overlay.ListEntry,
                &device_extension->list_lock
                );

            KeSetEvent(
                &device_extension->request_event,
                (KPRIORITY) 0,
                FALSE
                );

            status = STATUS_PENDING;

            break;
        }

    case IOCTL_FILE_DISK_QUERY_FILE:
        {
            POPEN_FILE_INFORMATION open_file_information;

            if (io_stack->Parameters.DeviceIoControl.OutputBufferLength <
				sizeof(OPEN_FILE_INFORMATION) + device_extension->file_name.Length - sizeof(UCHAR))
            {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = 0;
                break;
            }

            open_file_information = (POPEN_FILE_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

			open_file_information->version = CCVERSION;
			open_file_information->FileSize.QuadPart = device_extension->file_size.QuadPart;//-DiskFileHeadSize;//SDZ
            open_file_information->ReadOnly = device_extension->read_only;
            open_file_information->FileNameLength = device_extension->file_name.Length;
            open_file_information->DriveLetter = device_extension->DriveLetter;
            if (!device_extension->media_in_device) open_file_information->DriveLetter = 0;

            RtlCopyMemory(
                open_file_information->FileName,
                device_extension->file_name.Buffer,
                device_extension->file_name.Length
                );

			//Test
			//sprintf(open_file_information->FileName,"X: Req Ok %ld/%ld Bytes Ok:%ld/%ld Lerr:%ld (%x)",
			//	device_extension->rdcreq,device_extension->rdcread,
			//	device_extension->rdbyreq,device_extension->rdbyread,
			//	device_extension->rdfpos,device_extension->lasterr);
			open_file_information->FileNameLength = (USHORT)strlen(open_file_information->FileName);


            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(OPEN_FILE_INFORMATION) +
                open_file_information->FileNameLength - sizeof(UCHAR);

            break;
		}

    case IOCTL_DISK_CHECK_VERIFY:
    case IOCTL_CDROM_CHECK_VERIFY:
    case IOCTL_STORAGE_CHECK_VERIFY:
    case IOCTL_STORAGE_CHECK_VERIFY2:
        {
            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;
            break;
        }

    case IOCTL_DISK_GET_DRIVE_GEOMETRY:
    case IOCTL_CDROM_GET_DRIVE_GEOMETRY:
        {
            PDISK_GEOMETRY  disk_geometry;
            ULONGLONG       length;

            if (io_stack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(DISK_GEOMETRY))
            {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = 0;
                break;
            }

            disk_geometry = (PDISK_GEOMETRY) Irp->AssociatedIrp.SystemBuffer;

			length = device_extension->file_size.QuadPart;//-DiskFileHeadSize;//SDZ

            disk_geometry->Cylinders.QuadPart = length / SECTOR_SIZE / 32 / 2;
            disk_geometry->MediaType = FixedMedia;
            disk_geometry->TracksPerCylinder = 2;
            disk_geometry->SectorsPerTrack = 32;
            disk_geometry->BytesPerSector = SECTOR_SIZE;

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(DISK_GEOMETRY);

            break;
        }

    case IOCTL_DISK_GET_LENGTH_INFO:
        {
            PGET_LENGTH_INFORMATION get_length_information;

            if (io_stack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(GET_LENGTH_INFORMATION))
            {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = 0;
                break;
            }

            get_length_information = (PGET_LENGTH_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

			get_length_information->Length.QuadPart = device_extension->file_size.QuadPart;//-DiskFileHeadSize;//SDZ

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(GET_LENGTH_INFORMATION);

        break;
        }

    case IOCTL_DISK_GET_PARTITION_INFO:
        {
            PPARTITION_INFORMATION  partition_information;
            ULONGLONG               length;

            if (io_stack->Parameters.DeviceIoControl.OutputBufferLength <
				sizeof(PARTITION_INFORMATION))
            {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = 0;
                break;
            }

            partition_information = (PPARTITION_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

			length = device_extension->file_size.QuadPart;//-DiskFileHeadSize;//SDZ

            partition_information->StartingOffset.QuadPart = SECTOR_SIZE;
            partition_information->PartitionLength.QuadPart = length - SECTOR_SIZE;
            partition_information->HiddenSectors = 1;
            partition_information->PartitionNumber = 0;
            partition_information->PartitionType = 0;
            partition_information->BootIndicator = FALSE;
            partition_information->RecognizedPartition = FALSE;
            partition_information->RewritePartition = FALSE;

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION);

            break;
        }

    case IOCTL_DISK_GET_PARTITION_INFO_EX:
        {
            PPARTITION_INFORMATION_EX   partition_information_ex;
            ULONGLONG                   length;

            if (io_stack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(PARTITION_INFORMATION_EX))
            {
                status = STATUS_BUFFER_TOO_SMALL;
				Irp->IoStatus.Information = 0;
                break;
            }

            partition_information_ex = (PPARTITION_INFORMATION_EX) Irp->AssociatedIrp.SystemBuffer;

            length = device_extension->file_size.QuadPart;//-DiskFileHeadSize;//SDZ

            partition_information_ex->PartitionStyle = PARTITION_STYLE_MBR;
            partition_information_ex->StartingOffset.QuadPart = SECTOR_SIZE;
            partition_information_ex->PartitionLength.QuadPart = length - SECTOR_SIZE;
            partition_information_ex->PartitionNumber = 0;
            partition_information_ex->RewritePartition = FALSE;
            partition_information_ex->Mbr.PartitionType = 0;
            partition_information_ex->Mbr.BootIndicator = FALSE;
            partition_information_ex->Mbr.RecognizedPartition = FALSE;
            partition_information_ex->Mbr.HiddenSectors = 1;

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION_EX);

            break;
        }

    case IOCTL_DISK_IS_WRITABLE:
        {
            if (!device_extension->read_only)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                status = STATUS_MEDIA_WRITE_PROTECTED;
            }
            Irp->IoStatus.Information = 0;
			break;
        }

    case IOCTL_DISK_MEDIA_REMOVAL:
    case IOCTL_STORAGE_MEDIA_REMOVAL:
        {
            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;
            break;
        }

    case IOCTL_CDROM_READ_TOC:
        {
            PCDROM_TOC cdrom_toc;

            if (io_stack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(CDROM_TOC))
            {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = 0;
                break;
            }

            cdrom_toc = (PCDROM_TOC) Irp->AssociatedIrp.SystemBuffer;

            RtlZeroMemory(cdrom_toc, sizeof(CDROM_TOC));

            cdrom_toc->FirstTrack = 1;
            cdrom_toc->LastTrack = 1;
            cdrom_toc->TrackData[0].Control = TOC_DATA_TRACK;

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(CDROM_TOC);

            break;
		}

    case IOCTL_DISK_SET_PARTITION_INFO:
        {
            if (device_extension->read_only)
            {
                status = STATUS_MEDIA_WRITE_PROTECTED;
                Irp->IoStatus.Information = 0;
                break;
            }

            if (io_stack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SET_PARTITION_INFORMATION))
            {
                status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Information = 0;
                break;
            }

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;

            break;
        }

    case IOCTL_DISK_VERIFY:
        {
            PVERIFY_INFORMATION verify_information;

            if (io_stack->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(VERIFY_INFORMATION))
            {
                status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Information = 0;
                break;
			}

            verify_information = (PVERIFY_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = verify_information->Length;

            break;
        }

    default:
        {
			//KdPrint((
				//"EZCrypt: Unknown IoControlCode %#x\n",
				//io_stack->Parameters.DeviceIoControl.IoControlCode
				//));

            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Information = 0;
        }
    }

    if (status != STATUS_PENDING)
    {
        Irp->IoStatus.Status = status;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status;
}



void AESEncryptCBC(unsigned char *datap,ULONG anz,ULONG32 devSect[4],symmetric_key *key)
{
    ULONG32 iv[4];
    int ivCounter = 0;
    int cnt = anz >> 4;

    do {
        if(!ivCounter) {
			memcpy(iv,devSect,16);
            if(!++devSect[0] && !++devSect[1] && !++devSect[2]) devSect[3]++;
        }
        ivCounter++;
        ivCounter &= 31;
        iv[0] ^= *((ULONG32 *)(&datap[ 0]));
        iv[1] ^= *((ULONG32 *)(&datap[ 4]));
        iv[2] ^= *((ULONG32 *)(&datap[ 8]));
        iv[3] ^= *((ULONG32 *)(&datap[12]));
        rijndael_ecb_encrypt((unsigned char *)iv,datap,key);
        //aes_encrypt(&ctx, (unsigned char *)(&iv[0]), datap);
        memcpy(&iv[0], datap, 16);
        datap += 16;
    } while(--cnt);
}

void AESDecryptCBC(unsigned char *datap,ULONG anz,ULONG32 devSect[4],symmetric_key *key)
{
    ULONG32 iv[8];
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

        *((ULONG32 *)(&datap[ 0])) ^= iv[0];
        *((ULONG32 *)(&datap[ 4])) ^= iv[1];
        *((ULONG32 *)(&datap[ 8])) ^= iv[2];
        *((ULONG32 *)(&datap[12])) ^= iv[3];
        memcpy(&iv[0], &iv[4], 16);
        datap += 16;
    } while(--cnt);
}



VOID
FileDiskThread (
    IN PVOID Context
    )
{
    PDEVICE_OBJECT      device_object;
    PDEVICE_EXTENSION   device_extension;
    PLIST_ENTRY         request;
    PIRP                irp;
    PIO_STACK_LOCATION  io_stack;
    UCHAR tmpSect[512*4];
    ULONG off;
    UCHAR *dest;
    LARGE_INTEGER li;
    IO_STATUS_BLOCK sb;
    ULONG totlen;
    PUCHAR              system_buffer;
    PUCHAR              buffer;

	union {
		DWORD32 devSect[4];
		LARGE_INTEGER li;
	} sect;
	memset(&sect,0,sizeof(sect));


    ASSERT(Context != NULL);

    device_object = (PDEVICE_OBJECT) Context;

    device_extension = (PDEVICE_EXTENSION) device_object->DeviceExtension;

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    for (;;)
    {
        KeWaitForSingleObject(
            &device_extension->request_event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        if (device_extension->terminate_thread)
        {
            PsTerminateSystemThread(STATUS_SUCCESS);
        }

        while (request = ExInterlockedRemoveHeadList(
            &device_extension->list_head,
            &device_extension->list_lock
            ))
        {
			irp = CONTAINING_RECORD(request, IRP, Tail.Overlay.ListEntry);

            io_stack = IoGetCurrentIrpStackLocation(irp);

            switch (io_stack->MajorFunction)
            {
            case IRP_MJ_READ:

				{
					NTSTATUS st;
					int i;
					ULONG len;

					device_extension->rdbyreq += io_stack->Parameters.Read.Length;
					device_extension->rdcreq++;

	            	system_buffer = (PUCHAR) MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
                	if (system_buffer == NULL) {
						irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
						irp->IoStatus.Information = 0;
						break;
					}

	                buffer = (PUCHAR) ExAllocatePool(PagedPool, io_stack->Parameters.Read.Length);
	                if (buffer == NULL)
	                {
	                    irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
	                    irp->IoStatus.Information = 0;
	                    break;
	                }
					li = io_stack->Parameters.Read.ByteOffset;//sdz
					li.QuadPart+=DiskFileHeadSize;//sdz
					st = ZwReadFile(
						device_extension->file_handle,
						NULL,
						NULL,
						NULL,
						&irp->IoStatus,
						buffer,
						io_stack->Parameters.Read.Length,
						&li,//SDZ,
						NULL
					);


					irp->IoStatus.Status = st;
					if (NT_SUCCESS(st)) {
						device_extension->rdcread++;
					}
					len = (ULONG)(irp->IoStatus.Information) & (~0x1ffUL);

					if (len && NT_SUCCESS(st)) {
						device_extension->rdbyread += len;

						switch (device_extension->KeyType) {
						case 1:			//2fish
							blockDecrypt_CBC(&device_extension->f2k,buffer,buffer,len);
							break;
						case 2:	//AES256
						case 3://AES128
						case 4://AES192
							sect.li.QuadPart = (io_stack->Parameters.Read.ByteOffset.QuadPart+DiskFileHeadSize)>> 9;//SDZ
							AESDecryptCBC(buffer,len,sect.devSect,&device_extension->aes);
							break;
						}
					}
					RtlCopyMemory(system_buffer, buffer, io_stack->Parameters.Read.Length);
					ExFreePool(buffer);

				}
                break;

			case IRP_MJ_WRITE:

                if ((io_stack->Parameters.Write.ByteOffset.QuadPart +
                     io_stack->Parameters.Write.Length) >
					 (device_extension->file_size.QuadPart))//-DiskFileHeadSize SDZ
                {
					irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                    irp->IoStatus.Information = 0;
                }

				li = io_stack->Parameters.Write.ByteOffset;//sdz
				li.QuadPart+=DiskFileHeadSize;//sdz

				if (device_extension->KeyType) {

					dest = MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
					totlen = 0;

					for (off = 0;off == 0 || off < io_stack->Parameters.Write.Length;off += sizeof(tmpSect)) {
						ULONG len = io_stack->Parameters.Read.Length-off;
						NTSTATUS st;


						if (len > sizeof(tmpSect)) len = sizeof(tmpSect);
						switch (device_extension->KeyType ) {
							case 1:
								blockEncrypt_CBC(&device_extension->f2k,dest,tmpSect,len);
								break;
							case 2:	//AES
							case 3:
							case 4:
								sect.li.QuadPart = li.QuadPart >> 9;
								memcpy(tmpSect,dest,len);
								AESEncryptCBC(tmpSect,len,sect.devSect,&device_extension->aes);
								break;
						}

						st = ZwWriteFile(
							device_extension->file_handle,
							NULL,
							NULL,
							NULL,
							&irp->IoStatus,
							tmpSect,
							len,
							&li,
							NULL
							);


						if (!NT_SUCCESS(st)) break;
						totlen = totlen + (ULONG)(irp->IoStatus.Information);
						if (len != (ULONG)irp->IoStatus.Information) break;	//EOF

						dest = dest+len;
						li.QuadPart += len;
					}
					if (NT_SUCCESS(irp->IoStatus.Status)) {
						irp->IoStatus.Information = (ULONG_PTR)totlen;
					}
				} else {

					ZwWriteFile(
						device_extension->file_handle,
						NULL,
						NULL,
						NULL,
						&irp->IoStatus,
						MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority),
						io_stack->Parameters.Write.Length,
						&li,//sdz
						NULL
						);
				}
                break;

            case IRP_MJ_DEVICE_CONTROL:
                switch (io_stack->Parameters.DeviceIoControl.IoControlCode)
                {
                case IOCTL_FILE_DISK_OPEN_FILE:

                    SeImpersonateClient(device_extension->security_client_context, NULL);

                    irp->IoStatus.Status = FileDiskOpenFile(device_object, irp);

                    PsRevertToSelf();

                    break;

                case IOCTL_FILE_DISK_CLOSE_FILE:
                    irp->IoStatus.Status = FileDiskCloseFile(device_object, irp);
                    break;

                default:
                    irp->IoStatus.Status = STATUS_DRIVER_INTERNAL_ERROR;
                }
                break;

            default:
                irp->IoStatus.Status = STATUS_DRIVER_INTERNAL_ERROR;
            }

            IoCompleteRequest(
                irp,
                (CCHAR) (NT_SUCCESS(irp->IoStatus.Status) ?
                IO_DISK_INCREMENT : IO_NO_INCREMENT)
                );
        }
    }
}

//#pragma code_seg("PAGE")
#pragma code_seg()

NTSTATUS
FileDiskOpenFile (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
	PDEVICE_EXTENSION               device_extension;
	POPEN_FILE_INFORMATION          open_file_information;
	UNICODE_STRING                  ufile_name;
	NTSTATUS                        status;
    OBJECT_ATTRIBUTES               object_attributes;
    FILE_END_OF_FILE_INFORMATION    file_eof;
    FILE_BASIC_INFORMATION          file_basic;
    FILE_STANDARD_INFORMATION       file_standard;
    FILE_ALIGNMENT_INFORMATION      file_alignment;
    //RMD160_CTX ctx;
    //unsigned char tmp[64];


//    PAGED_CODE();

	ASSERT(DeviceObject != NULL);
    ASSERT(Irp != NULL);

    device_extension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    open_file_information = (POPEN_FILE_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

    if (DeviceObject->DeviceType != FILE_DEVICE_CD_ROM)
    {
        device_extension->read_only = open_file_information->ReadOnly;
    }

    device_extension->file_name.Length = open_file_information->FileNameLength;
	device_extension->file_name.MaximumLength = open_file_information->FileNameLength;
	device_extension->file_name.Buffer = ExAllocatePool(NonPagedPool, open_file_information->FileNameLength);

	device_extension->KeyType = open_file_information->KeyType;

	switch (device_extension->KeyType) {
		case 1:	//2fish

			//RMD160Init(&ctx);
		    //RMD160Update(&ctx,open_file_information->Key,open_file_information->KeyLength);
			//RMD160Final(device_extension->f2k.key,&ctx);
			memcpy(device_extension->f2k.key,open_file_information->Key[0],20);
		    device_extension->f2k.keyLen = (DWORD32)(20<<3);

		    init_key(&device_extension->f2k);
			break;
		case 2:		//AES256
			//sha512_hash_buffer(open_file_information->Key,open_file_information->KeyLength,tmp,32);
			if (rijndael_setup(open_file_information->Key[0], 32, 0, &device_extension->aes)) {
				//KdPrint(("AES256 KeySetup faild"));
			}
			break;
		case 3:		//AES128
			//sha256_hash_buffer(open_file_information->Key,open_file_information->KeyLength,tmp,16);
			if (rijndael_setup(open_file_information->Key[0], 16, 0, &device_extension->aes)) {
				//KdPrint(("AES128 KeySetup faild"));
			}
			break;
		case 4:		//AES192
			//sha384_hash_buffer(open_file_information->Key,open_file_information->KeyLength,tmp,24);
			if (rijndael_setup(open_file_information->Key[0], 24, 0, &device_extension->aes)) {
				//KdPrint(("AES192 KeySetup faild"));
			}
			break;
	}


	device_extension->DriveLetter = open_file_information->DriveLetter;

    RtlCopyMemory(
        device_extension->file_name.Buffer,
        open_file_information->FileName,
        open_file_information->FileNameLength
        );

    status = RtlAnsiStringToUnicodeString(
        &ufile_name,
        &device_extension->file_name,
        TRUE
        );

    if (!NT_SUCCESS(status))
    {
        ExFreePool(device_extension->file_name.Buffer);
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        return status;
    }

    InitializeObjectAttributes(
        &object_attributes,
        &ufile_name,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

	//KdPrint(("EZCrypt: %s \n",open_file_information->FileName));


/*
#define		drive				"\\Device\\Harddisk1\\Partition1"
	HANDLE			hDeviceHandle;
	AnsiString	  	str;
	unsigned char	p[512];
	int				i;
	boolean			bcreated;
	unsigned long	ret;

	bcreated= DefineDosDevice (DDD_RAW_TARGET_PATH, "TempDiskDriver",drive );
	if (!bcreated)
		return;
	//sprintf (lpszCFDevice, "\\\\.\\%s", lpszDosDevice);

	hDeviceHandle = CreateFile("\\\\.\\TempDiskDriver", GENERIC_READ,  //如果只是读扇区,可以用GENERIC_READ
	FILE_SHARE_READ || FILE_SHARE_WRITE, NULL, OPEN_EXISTING,0,0);
	if (hDeviceHandle != INVALID_HANDLE_VALUE)
	{
		//p=(unsigned char*)LocalAlloc(LMEM_FIXED,512); //alloc(SectorCount*BytesPerSector);

		FileSeek((int)hDeviceHandle,0,0);
		if(FileRead((int)hDeviceHandle,&p[0],512)!=SectorCount*BytesPerSector)
		{
			FileClose((int)hDeviceHandle);
			return;
		}

*/


	status = ZwCreateFile(
        &device_extension->file_handle,
        device_extension->read_only ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
        &object_attributes,
        &Irp->IoStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        //device_extension->read_only ? FILE_SHARE_READ : 0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE |
        FILE_RANDOM_ACCESS |
        FILE_NO_INTERMEDIATE_BUFFERING |
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );

    //KdPrint(("EZCrypt: 2 \n"));

    if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_NO_SUCH_FILE)
	{
		//KdPrint(("EZCrypt: 3 \n"));
		if (device_extension->read_only || open_file_information->FileSize.QuadPart == 0)
        {
            ExFreePool(device_extension->file_name.Buffer);
            RtlFreeUnicodeString(&ufile_name);

            Irp->IoStatus.Status = STATUS_NO_SUCH_FILE;
            Irp->IoStatus.Information = 0;

            return STATUS_NO_SUCH_FILE;
        }
        else
        {
			status = ZwCreateFile(
                &device_extension->file_handle,
                GENERIC_READ | GENERIC_WRITE,
                &object_attributes,
                &Irp->IoStatus,
				&open_file_information->FileSize,//+DiskFileHeadSize,//sdz
                FILE_ATTRIBUTE_NORMAL,
                0,
                FILE_OPEN_IF,
                FILE_NON_DIRECTORY_FILE |
                FILE_RANDOM_ACCESS |
                FILE_NO_INTERMEDIATE_BUFFERING |
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

            if (!NT_SUCCESS(status))
            {
                ExFreePool(device_extension->file_name.Buffer);
                RtlFreeUnicodeString(&ufile_name);
                return status;
            }

            if (Irp->IoStatus.Information == FILE_CREATED)
            {
				file_eof.EndOfFile.QuadPart = open_file_information->FileSize.QuadPart+DiskFileHeadSize;//sdz

                status = ZwSetInformationFile(
                    device_extension->file_handle,
                    &Irp->IoStatus,
                    &file_eof,
                    sizeof(FILE_END_OF_FILE_INFORMATION),
                    FileEndOfFileInformation
                    );

                if (!NT_SUCCESS(status))
                {
                    ExFreePool(device_extension->file_name.Buffer);
                    RtlFreeUnicodeString(&ufile_name);
                    ZwClose(device_extension->file_handle);
                    return status;
                }
            }
        }
    }
    else if (!NT_SUCCESS(status))
    {
		//KdPrint(("EZCrypt: 4 \n"));
        ExFreePool(device_extension->file_name.Buffer);
        RtlFreeUnicodeString(&ufile_name);
        return status;
    }

    RtlFreeUnicodeString(&ufile_name);

//KdPrint(("EZCrypt: 5 \n"));
    status = ZwQueryInformationFile(
		device_extension->file_handle,
        &Irp->IoStatus,
        &file_basic,
        sizeof(FILE_BASIC_INFORMATION),
        FileBasicInformation
        );

//KdPrint(("EZCrypt: 6 \n"));
    if (!NT_SUCCESS(status))
    {
		//KdPrint(("EZCrypt: 6.1 ignore \n"));
        //ExFreePool(device_extension->file_name.Buffer);
        //ZwClose(device_extension->file_handle);
        //return status;
    }

    //
    // The NT cache manager can deadlock if a filesystem that is using the cache
    // manager is used in a virtual disk that stores its file on a filesystem
    // that is also using the cache manager, this is why we open the file with
    // FILE_NO_INTERMEDIATE_BUFFERING above, however if the file is compressed
    // or encrypted NT will not honor this request and cache it anyway since it
    // need to store the decompressed/unencrypted data somewhere, therefor we put
    // an extra check here and don't alow disk images to be compressed/encrypted.
    //
    //KdPrint(("EZCrypt: 7 \n"));
    if (!device_extension->read_only && (file_basic.FileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_ENCRYPTED)))
	{
        ExFreePool(device_extension->file_name.Buffer);
        ZwClose(device_extension->file_handle);
        Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Information = 0;
        return STATUS_ACCESS_DENIED;
    }


//KdPrint(("EZCrypt: 8 \n"));
	status = ZwQueryInformationFile(
		device_extension->file_handle,
		&Irp->IoStatus,
		&file_standard,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation
		);

	if (!NT_SUCCESS(status))
	{
		//KdPrint(("EZCrypt: 9.1 ignore \n"));
        //ExFreePool(device_extension->file_name.Buffer);
        //ZwClose(device_extension->file_handle);
        //return status;
    }

	device_extension->file_size.QuadPart = file_standard.EndOfFile.QuadPart-DiskFileHeadSize;//sdz
//KdPrint(("EZCrypt: 10 \n"));
    status = ZwQueryInformationFile(
        device_extension->file_handle,
        &Irp->IoStatus,
        &file_alignment,
        sizeof(FILE_ALIGNMENT_INFORMATION),
        FileAlignmentInformation
        );

    if (!NT_SUCCESS(status))
    {
		//KdPrint(("EZCrypt: 11 ignotre\n"));
        //ExFreePool(device_extension->file_name.Buffer);
        //ZwClose(device_extension->file_handle);
        //return status;
    }

    DeviceObject->AlignmentRequirement = file_alignment.AlignmentRequirement;

//KdPrint(("EZCrypt: 12 \n"));
    if (device_extension->read_only)
    {
        DeviceObject->Characteristics |= FILE_READ_ONLY_DEVICE;
    }
    else
    {
        DeviceObject->Characteristics &= ~FILE_READ_ONLY_DEVICE;
    }

//KdPrint(("EZCrypt: 13 \n"));
    device_extension->media_in_device = TRUE;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    return STATUS_SUCCESS;
}

NTSTATUS
FileDiskCloseFile (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION device_extension;

//    PAGED_CODE();

    ASSERT(DeviceObject != NULL);
    ASSERT(Irp != NULL);

    device_extension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    ExFreePool(device_extension->file_name.Buffer);

    ZwClose(device_extension->file_handle);

    device_extension->media_in_device = FALSE;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    return STATUS_SUCCESS;
}
