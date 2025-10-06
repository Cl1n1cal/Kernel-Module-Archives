#pragma once
#include <windows.h>

typedef struct _EDIT_MEM_INPUT {
    ULONG Index;        // which chunk to edit
    ULONG BytesToWrite; // number of bytes to copy
    UCHAR Data[1];         // variable-length data starts here
} EDIT_MEM_INPUT, *PEDIT_MEM_INPUT;

typedef struct _STANDARD_MEM_INFO {
	ULONG Index;
	ULONG AllocationSize;
} STANDARD_MEM_INFO, *PSTANDARD_MEM_INFO;



void print_menu(void);
void setup(void);
