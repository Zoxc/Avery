#pragma once
#include "arch.hpp"
#include "../memory.hpp"

struct IOAPIC
{
	size_t index;
	size_t id;
	size_t irq_base;
	size_t irq_count;
	volatile uint32_t *registers;

	volatile uint32_t &reg(size_t reg);

	static const size_t max_io_apics = 32;
	static size_t count;
	static IOAPIC ios[max_io_apics];

	static IOAPIC *allocate(size_t id, addr_t registers);
};
