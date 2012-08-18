#pragma once
#include "arch.hpp"
#include "../memory.hpp"

typedef uint32_t irq_id_t;

struct IOAPIC
{
	size_t index;
	size_t id;
	irq_id_t irq_base;
	size_t irq_count;
	volatile uint32_t *registers;

	volatile uint32_t &reg(size_t reg);

	static const size_t max_io_apics = 32;
	static size_t count;
	static IOAPIC ios[max_io_apics];

	static IOAPIC *allocate(irq_id_t base, size_t id, addr_t registers);

	void route(size_t irq, uint8_t vector, uint8_t target, bool edge_triggered, bool active_low);
};

struct IRQ
{
	IOAPIC *apic;
	size_t index;
	bool edge_triggered;
	bool active_low;

	IRQ(size_t index);

	void setup();

	void route(uint8_t vector, uint8_t target);
};
