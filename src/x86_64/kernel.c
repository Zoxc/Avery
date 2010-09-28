#include "../multiboot.h"

static char *const vga = (char *const)0xb8000;

static char *position = (char *const)0xb8000;

static void print(const char *str)
{
	if(str == 0)
		return;

	while(*str)
	{
		*position++ = *str++;
		position++;
	}
}

void kernel(multiboot_t *multiboot)
{
	print("Welcome to long mode!");
}