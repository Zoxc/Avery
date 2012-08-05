#include "cpu.hpp"
#include "apic.hpp"
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

extern "C" void *ap_bootstrap;
extern "C" void *ap_bootstrap_start;
extern "C" void *ap_bootstrap_end;
extern "C" void *ap_bootstrap_mapped;

void setup_ap_bootstrap()
{
	// Move setup code to low memory

	assert(((ptr_t)&ap_bootstrap_end - (ptr_t)&ap_bootstrap_start) <= Arch::page_size, "CPU bootstrap code too large");

	auto code_block = Memory::allocate_block(Memory::Block::PhysicalView);

	Memory::map_address(code_block->base, (Memory::PhysicalPage *)&ap_bootstrap_mapped, Memory::rw_data_flags);

	memcpy(code_block->base, &ap_bootstrap_start, Arch::page_size);

	Memory::free_block(code_block);
}

void CPU::initialize()
{
	assert(count != 0, "No CPUs found");

	setup_ap_bootstrap();

	// Point 0x467 to the setup code
	{
		Memory::ScopedBlock warm_up_block;

		*warm_up_block.map_object<uint32_t>(0x467, Memory::rw_data_flags) = (((ptr_t)&ap_bootstrap_mapped >> 4) << 16) | ((ptr_t)&ap_bootstrap - (ptr_t)&ap_bootstrap_mapped);
	}

	// Setup CMOS to jump to the setup code

	Arch::outb(0x70, (Arch::inb(0x70) & 0x80) | 0xF);
	Arch::outb(0x71, 0xA);

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

	// Wake up other CPUs

	for(size_t i = 0; i < count; ++i)
	{
		if(&cpus[i] == bsp)
			continue;

		console.s("Starting CPU with id: ").u(cpus[i].acpi_id).endl();

		APIC::ipi(cpus[i].apic_id, APIC::Init, 0);
		APIC::simple_oneshot(1300000);
		APIC::ipi(cpus[i].apic_id, APIC::Startup, 0);
		APIC::simple_oneshot(10000);
		APIC::ipi(cpus[i].apic_id, APIC::Startup, 0);

		console.s("Waiting for CPU to start...").endl();

		while(true)
			Arch::halt();
	}
}
