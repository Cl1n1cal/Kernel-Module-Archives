#include "pch.h"
#include "universal_driver.h"
#include <usermode_accessors.h>

/*
*Buffer of 10 char ptrs
* So long as EXCLUSIVE = true in the IoCreateDevice call this global array will not be a problem
* If EXLUSIVE = false there has to be mutexes put in place to combat race conditions since multiple
* accesses to the array could occur at once.
*/ 
PCHAR chunks[10];
ULONG chunks_size[10];
char data[MAX_BUFFER_SIZE];

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)

{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;

	// Setup Device Object and Symbolic Link
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\universal-driver");
	UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\universal-driver");
	PDEVICE_OBJECT deviceObject = nullptr;

	// Set driver dispatch routines
	DriverObject->DriverUnload = UniversalDriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = UniversalDriverDispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = UniversalDriverDispatchClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = UniversalDriverDeviceControl;


	do
	{
		// If exclusive access is ever changed: Implement concurrency controls on the global variables
		status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, TRUE, &deviceObject);

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

	// Set data to be 0
	memset(data, 0x00, MAX_BUFFER_SIZE);

	return status;
}

void UniversalDriverUnload(_In_ PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\universal-driver");

	IoDeleteSymbolicLink(&symLink);

	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS UniversalDriverDispatchCreate(PDEVICE_OBJECT, PIRP Irp)
{
	return CompleteIrp(Irp);
}

NTSTATUS UniversalDriverDispatchClose(PDEVICE_OBJECT, PIRP Irp)
{
	return CompleteIrp(Irp);
}


NTSTATUS UniversalDriverDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;
	ULONG_PTR info = 0;

	auto& dic = irpSp->Parameters.DeviceIoControl;
	auto outBufLen = dic.OutputBufferLength;
	auto inBufLen = dic.InputBufferLength;
	auto outBuf = Irp->AssociatedIrp.SystemBuffer;
	auto inBuf = Irp->AssociatedIrp.SystemBuffer;
	ULONG allocationSize = 0;
	ULONG allocationIndex = 0;
	PEDIT_MEM_INPUT editMemStruct;
	PSTANDARD_MEM_INFO standardMemInfo;
	SIZE_T copyLen = 0;

	switch (dic.IoControlCode) {
	case READ_BUF:
		copyLen = min(outBufLen, sizeof(data));

		if (MAX_BUFFER_SIZE < copyLen) {
			status = STATUS_BUFFER_TOO_SMALL;
			info = 0;
			break;
		}

		RtlCopyBytes(outBuf, data, copyLen);
		status = STATUS_SUCCESS;
		info = copyLen;
		break;

	case WRITE_BUF:
		copyLen = min(inBufLen, sizeof(data));

		if (MAX_BUFFER_SIZE < copyLen) {
			status = STATUS_BUFFER_TOO_SMALL;
			info = 0;
			break;
		}

		RtlCopyBytes(data, inBuf, copyLen);
		status = STATUS_SUCCESS;
		info = copyLen;

	case ALLOC_MEM:
		if (dic.InputBufferLength != sizeof(STANDARD_MEM_INFO)) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		standardMemInfo = (PSTANDARD_MEM_INFO)Irp->AssociatedIrp.SystemBuffer;
		allocationIndex = standardMemInfo->Index;
		allocationSize = standardMemInfo->AllocationSize;
		
		// Check if allocation size is 0 or lager that 10 MB
		if (allocationSize == 0 || 10 * 1024 * 1024 < allocationSize) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}
		
		// Check if the index is 0-9 (ULONG can never be negative)
		if (10 <= allocationIndex) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		chunks[allocationIndex] = (PCHAR) ExAllocatePool3(POOL_FLAG_PAGED, allocationSize, DRIVER_TAG, NULL, 0);
		chunks_size[allocationIndex] = allocationSize;

		if (chunks[allocationIndex] == NULL) {
			status = STATUS_INSUFFICIENT_RESOURCES;
			info = 0;
			break;
		}

		status = STATUS_SUCCESS;
		info = allocationSize;
		standardMemInfo = NULL;
		break;

	case FREE_MEM:
		if (dic.InputBufferLength != 1) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		standardMemInfo = (PSTANDARD_MEM_INFO)Irp->AssociatedIrp.SystemBuffer;
		allocationIndex = standardMemInfo->Index;

		// Check if the index is 0-9
		if (10 <= allocationIndex) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		// Check if the chunks index contains NULL
		if (chunks[allocationIndex] == NULL) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		ExFreePool(chunks[allocationIndex]);

		// Overwrite index and size with NULL to combat UAF
		chunks[allocationIndex] = NULL;
		chunks_size[allocationIndex] = 0;

		// Set status success and info the freed index
		status = STATUS_SUCCESS;
		info = allocationIndex;
		standardMemInfo = NULL;
		break;

	case EDIT_MEM:
		// Check if the header is at least there
		if (dic.InputBufferLength < sizeof(EDIT_MEM_INPUT) - 1) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		editMemStruct = (PEDIT_MEM_INPUT)Irp->AssociatedIrp.SystemBuffer;

		// Check if the specified amount of bytes is more than the size
		if (dic.InputBufferLength - FIELD_OFFSET(EDIT_MEM_INPUT, Data) < editMemStruct->BytesToWrite) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		// Check that the index is within range
		if (10 <= editMemStruct->Index) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		// Check that the index is not NULL
		if (chunks[editMemStruct->Index] == NULL) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		// Check if the amount of bytes to write can fit within the kernel mem chunk
		if (editMemStruct->BytesToWrite <= chunks_size[editMemStruct->Index]) {
			status = STATUS_BUFFER_TOO_SMALL;
			info = 0;
			break;
		}

		// Copy the bytes
		RtlCopyMemory(chunks[editMemStruct->Index], editMemStruct->Data, editMemStruct->BytesToWrite);
		status = STATUS_SUCCESS;
		info = 0;
		editMemStruct = NULL;
		break;

	case GET_MEM:
		// Check that the header is correct size
		if (dic.InputBufferLength != sizeof(STANDARD_MEM_INFO)) {
			status = STATUS_INVALID_PARAMETER;
			info = 0;
			break;
		}

		standardMemInfo = (PSTANDARD_MEM_INFO)Irp->AssociatedIrp.SystemBuffer;

		// Check that the index is within range
		if (10 <= standardMemInfo->Index) {
			status = STATUS_INVALID_PARAMETER;
			standardMemInfo = NULL;
			info = 0;
			break;
		}

		// Check that the index is not NULL
		if (chunks[standardMemInfo->Index] == NULL) {
			status = STATUS_INVALID_PARAMETER;
			standardMemInfo = NULL;
			info = 0;
			break;
		}

		// Check that the amount of bytes to copy is less than or equal the size of the chunk
		if (standardMemInfo->AllocationSize <= chunks_size[standardMemInfo->Index]) {
			status = STATUS_INVALID_PARAMETER;
			standardMemInfo = NULL;
			info = 0;
			break;
		}

		// Copy the memory to the output buffer
		RtlCopyBytes(Irp->AssociatedIrp.SystemBuffer, chunks[standardMemInfo->Index], standardMemInfo->AllocationSize);
		status = STATUS_SUCCESS;
		// Set info to amount of bytes read
		info = standardMemInfo->AllocationSize;
		standardMemInfo = NULL;
		break;

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;

	}

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}