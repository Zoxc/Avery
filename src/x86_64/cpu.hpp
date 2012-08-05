#pragma once
#include "arch.hpp"

struct CPU
{
	size_t index;
	size_t acpi_id;
	size_t apic_id;
	uint32_t apic_registers;

	static const size_t max_cpus = 32;
	static size_t count;
	static CPU *bsp;
	static CPU cpus[max_cpus];

	static CPU *allocate(size_t acpi_id, size_t apic_id);
	static void initialize();
};

