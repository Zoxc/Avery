#include "io-apic.hpp"

size_t IOAPIC::count;
IOAPIC IOAPIC::ios[max_io_apics];

const size_t reg_id = 0;
const size_t reg_version = 1;
const size_t reg_irq_start = 0x10;

const size_t mask_bit = 1 << 16;
const size_t trigger_mode_bit = 1 << 15;
const size_t active_low_bit = 1 << 13;

IRQ::IRQ(size_t index) :
	apic(0),
	index(index),
	edge_triggered(true),
	active_low(false)
{
}

void IRQ::setup()
{
	if(apic)
		return;

	for(size_t i = 0; i < IOAPIC::count; ++i)
		if(index >= IOAPIC::ios[i].irq_base && index < IOAPIC::ios[i].irq_base + IOAPIC::ios[i].irq_count)
		{
			apic = &IOAPIC::ios[i];
			index = index - IOAPIC::ios[i].irq_base;
			return;
		}

	panic("Unable to find interrupt from id");
}

void IRQ::route(uint8_t vector, uint8_t target)
{
	setup();

	apic->route(index, vector, target, edge_triggered, active_low);
}

void IOAPIC::route(size_t irq, uint8_t vector, uint8_t target, bool edge_triggered, bool active_low)
{
	assert(irq < irq_count, "IRQ index out of bounds");

	console.s("routing irq ").u(irq).s(" - target ").u(target).endl();

	size_t reg_start = reg_irq_start + irq * 2;

	reg(reg_start) |= mask_bit;
	reg(reg_start + 1) = target << 24;
	reg(reg_start) = (edge_triggered ? 0 : trigger_mode_bit) | (active_low ? active_low_bit : 0) | vector;
}

volatile uint32_t &IOAPIC::reg(size_t reg)
{
	registers[0] = reg;
	return registers[4];
}

IOAPIC *IOAPIC::allocate(irq_id_t base, size_t id, addr_t registers)
{
	assert(count < max_io_apics, "Too many I/O APICs");

	IOAPIC *result = &ios[count];

	result->index = count++;
	result->id = id;
	result->registers = (uint32_t *)Memory::map_physical(registers, 1, Memory::rw_data_flags | Memory::no_cache_flags)->base;
	result->irq_base = base;

	size_t id_from_reg = (result->reg(reg_id) >> 24) & 0xF;

	if(id_from_reg != id)
		console.s("I/O APIC register id differs from ACPI id: ").u(id_from_reg).s(" vs. ACPI: ").u(id).endl();

	size_t version = result->reg(reg_version);

	result->irq_count = ((version >> 16) & 0xFF) + 1;

	console.s("I/O APIC ").u(result->id).s(" base: ").u(base).s(" IRQ count: ").u(result->irq_count).endl();

	// Mask all IRQs

	for(size_t i = 0; i < result->irq_count; ++i)
		result->reg(reg_irq_start + i * 2) |= mask_bit;


	return result;
}
