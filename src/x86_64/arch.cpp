#include "arch.hpp"
#include "memory.hpp"
#include "../console.hpp"
#include "apic.hpp"
#include "acpi.hpp"
#include "cpu.hpp"
#include "gdt.hpp"
#include "idt.hpp"

void Arch::initialize_basic()
{
	initialize_gdt(CPU::bsp);
	initialize_idt();
}

void Arch::initialize_memory()
{
	Memory::Initial::initialize();
}

void Arch::initialize()
{
	ACPI::initialize();
	APIC::initialize();
	CPU::initialize();
}

size_t Arch::read_msr(uint32_t reg)
{
	uint32_t low, high;

	asm volatile ("rdmsr" : "=a" (low), "=d" (high) : "c" (reg));

	return (size_t)low | ((size_t)high << 32);
}

void Arch::write_msr(uint32_t reg, size_t value)
{
	asm volatile ("wrmsr" : : "a" (value), "d" (value >> 32), "c" (reg));
}

void Arch::outb(uint16_t port, uint8_t value)
{
	asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t Arch::inb(uint16_t port)
{
	uint8_t ret;

	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));

	return ret;
}

uint16_t Arch::inw(uint16_t port)
{
	uint16_t ret;

	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));

	return ret;
}

void Arch::pause()
{
	asm volatile ("pause");
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
