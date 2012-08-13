#include <stdint.h>

void print(char c)
{
	asm volatile("int $0x80" :: "D"(c));
}

void print(const char *str)
{
	while(*str)
	{
		print(*str);
		str++;
	}
}

extern "C" void entry()
{
	print("Hello from usermode!\n");
	while(true);
}
