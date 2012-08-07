#include "cpu.hpp"
#include "apic.hpp"
#include "gdt.hpp"
#include "idt.hpp"
#include "../memory.hpp"
#include "../physical_mem.hpp"
#include "../lib.hpp"

size_t CPU::count;
CPU CPU::cpus[max_cpus];
CPU *CPU::bsp;

CPU *CPU::allocate(size_t acpi_id, size_t apic_id)
{
	assert(count < max_cpus, "Too many CPUs");

	CPU *result = &cpus[count];

	result->index = count++;
	result->acpi_id = acpi_id;
	result->apic_id = apic_id;

	return result;
}

struct APBootstrapInfo
{

	uint32_t pml4;
	uint32_t allow_start;
	void *apic_registers;
	size_t cpu_count;
	size_t cpu_size;
	size_t cpu_apic_offset;
	size_t cpu_stack_offset;
	CPU *cpus;
} __attribute__((packed));

extern "C" void *ap_bootstrap;
extern "C" void *ap_bootstrap_start;
extern "C" void *ap_bootstrap_end;
extern "C" void *ap_bootstrap_mapped;
extern "C" void *ap_bootstrap_info;

addr_t ap_bootstrap_page = (addr_t)&ap_bootstrap_mapped;

addr_t ap_bootstrap_offset(void *&pointer)
{
	return (addr_t)&pointer - ap_bootstrap_page;
}

APBootstrapInfo *setup_ap_bootstrap()
{
	// Move setup code to low memory

	assert(((ptr_t)&ap_bootstrap_end - (ptr_t)&ap_bootstrap_start) <= Arch::page_size, "CPU bootstrap code too large");

	Memory::map_address((Memory::VirtualPage *)ap_bootstrap_page, ap_bootstrap_page, Memory::write_bit | Memory::present_bit);

	memcpy((void *)ap_bootstrap_page, &ap_bootstrap_start, Arch::page_size);

	APBootstrapInfo *info = (APBootstrapInfo *)(ap_bootstrap_page + ap_bootstrap_offset(ap_bootstrap_info));

	info->pml4 = Memory::physical_page((Memory::VirtualPage *)&Memory::ptl4_static);
	info->apic_registers = APIC::get_registers();
	info->cpu_count = CPU::count;
	info->cpu_size = sizeof(CPU);
	info->cpu_apic_offset = __builtin_offsetof(CPU, apic_id);
	info->cpu_stack_offset = __builtin_offsetof(CPU, stack_end);
	info->cpus = CPU::cpus;

	return info;
}

void send_startup()
{
	for(size_t i = 0; i < CPU::count; ++i)
	{
		if(&CPU::cpus[i] == CPU::bsp)
			continue;

		APIC::ipi(CPU::cpus[i].apic_id, APIC::Startup, 0x1);
	}

	APIC::simple_oneshot(10000);
}

bool cpus_started()
{
	for(size_t i = 0; i < CPU::count; ++i)
	{
		volatile bool *started = &CPU::cpus[i].started;

		if(!*started)
			return false;
	}

	return true;
}

void CPU::initialize()
{
	assert(count != 0, "No CPUs found");

	volatile APBootstrapInfo *info = setup_ap_bootstrap();

	size_t bsp_id = APIC::local_id();

	for(size_t i = 0; i < count; ++i)
	{
		if(cpus[i].apic_id == bsp_id)
		{
			bsp = &cpus[i];
			break;
		}
	}

	assert(bsp != 0, "Didn't find the bootstrap processor");

	bsp->started = true;

	// Wake up other CPUs

	for(size_t i = 0; i < count; ++i)
	{
		if(&cpus[i] == bsp)
			continue;

		// Allocate a stack

		const size_t stack_pages = 5;

		auto stack = Memory::allocate_block(Memory::Block::Stack, stack_pages);

		cpus[i].started = false;
		cpus[i].stack = stack;
		cpus[i].stack_end = stack->base +stack->pages;

		for(size_t p = 1; p < stack_pages; ++p)
			Memory::map(stack->base + p);

		console.s("Starting CPU with id: ").u(cpus[i].acpi_id).endl();

		APIC::ipi(cpus[i].apic_id, APIC::Init, 0);
	}

	APIC::simple_oneshot(1300000);

	send_startup();

	send_startup();

	console.s("Waiting for the CPUs to start...").endl();

	info->allow_start = true;

	while(!cpus_started())
		Arch::pause();

	console.s("All CPUs have started").endl();
}

extern "C" void ap_entry(CPU *cpu)
{
	Arch::initialize_gdt();
	Arch::load_idt();

	cpu->started = true;

	while(true)
		Arch::halt();
};
