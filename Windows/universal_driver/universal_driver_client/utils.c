#include "utils.h"
#include <stdio.h>

void print_menu(void)
{
	puts("Menu:");
	puts("1: Write to the driver buffer");
	puts("2: Read from the driver buffer");
	puts("3: Allocate kernel memory chunk");
	puts("4: Free kernel memory chunk");
	puts("5: Edit kernel memory chunk");
	puts("6: Print kernel memory chunk");
}

void setup(void)
{
	setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}