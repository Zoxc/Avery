#include "arch.hpp"
#include "physical_mem.hpp"

void Arch::initialize(const multiboot_t &info)
{
	initialize_idt();
	
	Memory::Physical::initialize(info);
}

void Arch::panic()
{
	while(1) asm volatile("hlt");
}