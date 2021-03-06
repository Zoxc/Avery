#include "arch.hpp"
#include "memory.hpp"
#include "../console.hpp"
#include "apic.hpp"
#include "acpi.hpp"
#include "pit.hpp"
#include "cpu.hpp"
#include "segments.hpp"
#include "interrupts.hpp"

namespace Arch
{
	Registers::Registers()
	{
		memset(this, 0, sizeof(Registers));

		cs = Segments::user_code_segment;
		ss = Segments::user_data_segment;
		rflags = rflags_default;
	}
};

void Arch::initialize_basic()
{
	// turn on write protect
	asm volatile ("mov %%cr0, %%rax; or %0, %%rax; mov %%rax, %%cr0" :: "i"((1 << 16)) : "rax");

	Segments::initialize_gdt();
	CPU::initialize_basic();
	Interrupts::initialize_idt();
}

void Arch::initialize_memory()
{
	Memory::Initial::initialize();
}

void Arch::initialize()
{
	CPU::bsp->map_local_page_tables();

	ACPI::initialize();
	APIC::initialize();
	PIT::initialize();
	APIC::calibrate();
	CPU::initialize();
}

void Arch::run()
{
	APIC::start_timer();

	Interrupts::enable();

	while(true)
		Arch::halt();
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

void Arch::halt()
{
	asm("hlt");
}

void Arch::panic()
{
	Interrupts::disable();

	while(true)
		halt();
}

void bp()
{
	volatile int var = 0;
	while(var == 0);
}
