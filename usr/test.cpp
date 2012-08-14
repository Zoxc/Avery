#include <stdint.h>

void print(char c)
{
	asm volatile("syscall" :: "S"(c) : "rdi", "rsi", "rdx", "rcx", "r8", "r9");
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
