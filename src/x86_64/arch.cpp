#include "arch.hpp"
#include "debug.hpp"
#include "physical_mem_init.hpp"

void Arch::initialize(const multiboot_t &info)
{
	initialize_idt();
	
	Debug::initialize(info);
	
	Memory::Physical::Initial::initialize(info);
}

void Arch::panic()
{
	while(1) asm volatile("hlt");
}