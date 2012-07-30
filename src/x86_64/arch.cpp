#include "arch.hpp"
#include "debug.hpp"
#include "physical_mem.hpp"
#include "physical_mem_init.hpp"
#include "memory.hpp"
#include "../console.hpp"

void Arch::initialize()
{
	initialize_gdt();
	initialize_idt();

	Debug::initialize();

	Memory::Initial::initialize_physical();

	Memory::Initial::initialize();

	return;

	Memory::Physical::initialize();
}

void Arch::panic()
{
	while(1) asm volatile("hlt");
}

void bp()
{
	volatile int var = 0;
	while(var == 0);
}
