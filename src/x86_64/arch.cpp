#include "arch.hpp"
#include "physical_mem.hpp"
#include "physical_mem_init.hpp"
#include "memory.hpp"
#include "../console.hpp"
#include "apic.hpp"

void Arch::initialize()
{
	initialize_gdt();
	initialize_idt();

	Memory::Initial::initialize_physical();

	Memory::Initial::initialize();

	Memory::Physical::initialize();

	console.s("Loading APIC").lb();

	//APIC::initialize();

	console.s("Loaded APIC").lb();
}

void Arch::enable_interrupts()
{
	asm("sti");
}

void Arch::disable_interrupts()
{
	asm("cli");
}

void Arch::halt()
{
	asm("hlt");
}

void Arch::panic()
{
	disable_interrupts();

	while(true)
		halt();
}

void bp()
{
	volatile int var = 0;
	while(var == 0);
}
