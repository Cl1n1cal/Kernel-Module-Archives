#include <windows.h>
#include <stdio.h>

#define DRIVER_PREFIX "BOF0"

// Control codes
#define READ_BUF CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA)
#define WRITE_BUF CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)

int error(const char* message)
{
	printf("%s (error=%u)\n", message, GetLastError());
	return 1;
}

int main(void)
{
	char buf[0x50];
	memset(buf, 0x41, 0x50);

	puts("Opening handle to driver");

	HANDLE hDevice = CreateFile(L"\\\\.\\bof", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) {
		return error("Failed to open device");
	}

	DeviceIoControl(hDevice, WRITE_BUF, NULL, 0, buf, sizeof(buf), NULL, NULL);

	puts("Printing buf content:");
	printf("%.*s\n", sizeof(buf), buf);


}