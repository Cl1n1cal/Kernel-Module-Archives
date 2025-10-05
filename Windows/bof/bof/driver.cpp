#include "pch.h"
#include "bof.h"


extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)

{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;

	// Setup Device Object and Symbolic Link
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\bof");
	UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\bof");
	PDEVICE_OBJECT deviceObject = nullptr;

	// Set driver dispatch routines
	DriverObject->DriverUnload = BofUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = BofDispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = BofDispatchClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = BofDeviceControl;


	do
	{
		status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);

		if (!NT_SUCCESS(status))
		{
			KdPrint((DRIVER_PREFIX "Failed to create device (0x%08X)\n", status));
			break;
		}

		status = IoCreateSymbolicLink(&symLinkName, &devName);

		if (!NT_SUCCESS(status))
		{
			KdPrint((DRIVER_PREFIX "Failed to create symbolic link (0x%08X)\n", status));
			break;
		}

	} while (false);

	return status;
}

void BofUnload(_In_ PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\bof");

	IoDeleteSymbolicLink(&symLink);

	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS BofDispatchCreate(PDEVICE_OBJECT, PIRP Irp)
{
	return CompleteIrp(Irp);
}

NTSTATUS BofDispatchClose(PDEVICE_OBJECT, PIRP Irp)
{
	return CompleteIrp(Irp);
}


NTSTATUS BofDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	char data[0x50];
	memset(data, 0x43, sizeof(data));

	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
	auto& dic = irpSp->Parameters.DeviceIoControl;
	auto outBufLen = dic.OutputBufferLength;
	auto outBuf = Irp->AssociatedIrp.SystemBuffer;
	SIZE_T copyLen = min(outBufLen, sizeof(data));

	switch (dic.IoControlCode) {
	case WRITE_BUF:
		RtlCopyBytes(outBuf, data, copyLen);
		return CompleteIrp(Irp, STATUS_SUCCESS, copyLen);
		break;
	default:
		return CompleteIrp(Irp);
		break;
	}
	



}