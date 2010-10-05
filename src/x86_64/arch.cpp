#include "arch.hpp"

void Arch::initialize()
{
	initialize_idt();
}

void Arch::panic()
{
	while(1) asm volatile("hlt");
}