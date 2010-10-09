#include "arch.hpp"
#include "debug.hpp"
#include "physical_mem_init.hpp"
#include "memory.hpp"

void Arch::initialize(const multiboot_t &info)
{
	initialize_gdt();
	initialize_idt();
	
	Debug::initialize(info);
	
	Memory::Initial::initialize_phsyical(info);
	
	Memory::Initial::initialize();
}

void Arch::panic()
{
	while(1) asm volatile("hlt");
}