#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "utils.h"


#define DRIVER_PREFIX "UNIV"

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
	uint32_t choice;
	int32_t c;
	// Set stdin, stdout, stderr unbuffered
	setup();

	puts("Opening handle to driver");
	HANDLE hDevice = CreateFile(L"\\\\.\\universal-driver", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) {
		return error("Failed to open device");
	}

	while (1) {
		print_menu();
		scanf("%u", &choice);

		while ((c = getchar()) != '\n' && c != EOF) {}

		switch (choice) {
		case 1:

			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;

		default:
			puts("Invalid option");
			break;
		}
	}


}