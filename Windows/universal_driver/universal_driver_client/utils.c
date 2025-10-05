#include "utils.h"
#include <stdio.h>

void menu(void)
{
	puts("Menu:");
	puts("1: Write to the driver buffer");
	puts("2: Read from the driver buffer");
	puts("3: Allocate kernel memory chunk");
	puts("4: Free kernel memory chunk");
	puts("5: Edit kernel memory chunk");
	puts("6: Print kernel memory chunk");

	getchar();

}