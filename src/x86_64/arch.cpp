#include "arch.hpp"
#include "memory.hpp"
#include "../console.hpp"
#include "apic.hpp"
#include "acpi.hpp"
#include "cpu.hpp"

void Arch::initialize_basic()
{
	initialize_gdt();
	initialize_idt();
}

void Arch::initialize_memory()
{
	Memory::Initial::initialize();
}

void Arch::initialize()
{
	APIC::initialize();
	ACPI::initialize();
	CPU::initialize();
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
