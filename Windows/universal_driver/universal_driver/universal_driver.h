#pragma once

#define DRIVER_PREFIX "UNIV"
#define DRIVER_TAG 'VINU'
#define MAX_BUFFER_SIZE 0x50

typedef struct _EDIT_MEM_INPUT {
    ULONG Index;        // which chunk to edit
    ULONG BytesToWrite; // number of bytes to copy
	UCHAR Data[1];			// variable-length data starts here
} EDIT_MEM_INPUT, *PEDIT_MEM_INPUT;

typedef struct _GET_MEM_OUTPUT {
	ULONG Index;
	ULONG BytesToRead;
	PCHAR Data;
};

typedef struct _STANDARD_MEM_INFO {
	ULONG Index;
	ULONG AllocationSize;
} STANDARD_MEM_INFO, *PSTANDARD_MEM_INFO;

// Control codes
#define READ_BUF CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA)
#define WRITE_BUF CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)
#define ALLOC_MEM CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FREE_MEM CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_WRITE_DATA)
#define EDIT_MEM CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_WRITE_DATA)
#define GET_MEM CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_READ_DATA)

// Dispatch routines
void UniversalDriverUnload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS UniversalDriverDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS UniversalDriverDispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS UniversalDriverDispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// Utils
NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS Status = STATUS_SUCCESS, ULONG_PTR Info = 0)
{
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = Info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}
