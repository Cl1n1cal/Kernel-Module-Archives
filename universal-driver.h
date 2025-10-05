#pragma once

#define DRIVER_PREFIX "UNIV"

// Control codes
#define READ_BUF CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA)
#define WRITE_BUF CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)

// Dispatch routines
void BofUnload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS BofDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS BofDispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS BofDispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// Utils
NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS Status = STATUS_SUCCESS, ULONG_PTR Info = 0)
{
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = Info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

