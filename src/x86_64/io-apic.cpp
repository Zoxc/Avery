#include "io-apic.hpp"

size_t IOAPIC::count;
IOAPIC IOAPIC::ios[max_io_apics];

const size_t reg_id = 0;
const size_t reg_version = 1;
const size_t reg_irq_start = 0x10;

const size_t mask_bit = 1 << 16;

volatile uint32_t &IOAPIC::reg(size_t reg)
{
	registers[0] = reg;
	return registers[4];
}

IOAPIC *IOAPIC::allocate(size_t id, addr_t registers)
{
	assert(count < max_io_apics, "Too many I/O APICs");

	IOAPIC *result = &ios[count];

	result->index = count++;
	result->id = id;
	result->registers = (uint32_t *)Memory::map_physical(registers, 1, Memory::rw_data_flags | Memory::no_cache_flags)->base;

	size_t id_from_reg = (result->reg(reg_id) >> 24) & 0xF;

	if(id_from_reg != id)
		console.s("I/O APIC register id differs from ACPI id: ").u(id_from_reg).s(" vs. ACPI: ").u(id).endl();

	size_t version = result->reg(reg_version);

	result->irq_count = ((version >> 16) & 0xFF) + 1;

	console.s("I/O APIC ").u(result->id).s(" IRQ count: ").u(result->irq_count).endl();

	// Mask all IRQs

	for(size_t i = 0; i < result->irq_count; ++i)
		result->reg(reg_irq_start + i * 2) |= mask_bit;


	return result;
}
