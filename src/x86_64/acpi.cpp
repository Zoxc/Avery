#include "acpi.hpp"
#include "cpu.hpp"
#include "apic.hpp"
#include "io-apic.hpp"
#include "../memory.hpp"

namespace ACPI
{
	const uint64_t RSDP::signature_magic = *(uint64_t *)"RSD PTR ";
	const uint32_t RSDT::signature_magic = *(uint32_t *)"RSDT";
	const uint32_t MADT::signature_magic = *(uint32_t *)"APIC";

	const size_t bios_start = 0xE0000;
	const size_t bios_end = 0x100000;

	uint8_t checksum(uint8_t *start, uint8_t *end)
	{
		uint8_t result = 0;

		for(auto p = start; p < end; ++p)
			result += *p;

		return result;
	}

	void assert_valid(SDT *table)
	{
		assert(checksum((uint8_t *)table, (uint8_t *)table + table->length) == 0, "Invalid checksum");
	}

	bool search_area(ptr_t start, ptr_t size, RSDP &result)
	{
		Memory::ScopedBlock block;

		start = (size_t)block.map_block(start, size);
		size_t end = (size_t)start + size;

		for(auto pointer = (RSDP *)start; (ptr_t)pointer < end; pointer = (RSDP *)((char *)pointer + 16))
		{
			if(pointer->signature != RSDP::signature_magic)
				continue;

			if(checksum((uint8_t *)pointer, (uint8_t *)(pointer + 1)) != 0)
				continue;

			result = *pointer;

			return true;
		}

		return false;
	}

	RSDP rsdp;

	bool has_table = false;
	addr_t acpi_table;

	void set_table(ptr_t table)
	{
		has_table = true;
		acpi_table = table;
	}

	void search()
	{
		if(search_area(bios_start, bios_end - bios_start, rsdp))
			return;

		Memory::ScopedBlock ebda_ptr_block;

		addr_t ebda = ((addr_t)*ebda_ptr_block.map_object<uint16_t>(0x40E)) << 4;

		if(search_area(align_up<addr_t>(ebda, 16), 0x400, rsdp))
			return;

		panic("Didn't find the ACPI RSDP structure");
	}

	SDT *load_table(Memory::ScopedBlock &block, size_t address)
	{
		Memory::ScopedBlock sdt_map;

		SDT *sdt = sdt_map.map_object<SDT>(address);

		sdt = (SDT *)block.map_block(address, sdt->length);

		assert_valid(sdt);

		return sdt;
	}

	void parse_madt(MADT *madt)
	{
		APIC::set_registers(madt->local_interrupt_controller);

		MADT::Entry	*entry = (MADT::Entry *)(madt + 1);
		MADT::Entry	*end = (MADT::Entry	*)((uint8_t *)madt + madt->length);

		for(; entry < end; entry = (MADT::Entry	*)((uint8_t *)entry + entry->length))
		{
			switch(entry->type)
			{
				case MADT::ProcessorLocalAPICEntry:
				{
					auto processor = (MADT::ProcessorLocalAPIC *)entry;

					if(processor->flags & MADT::ProcessorLocalAPIC::flag_enabled)
						CPU::allocate(processor->processor_id, processor->apic_id);

					break;
				};

				case MADT::IOAPICEntry:
				{
					auto io = (MADT::IOAPIC *)entry;

					IOAPIC::allocate(io->id, io->address);

					break;
				}

				case MADT::InterruptSourceOverrideEntry:
				{
					auto override = (MADT::InterruptSourceOverride *)entry;

					console.s("Interrupt source override - bus: ").u(override->bus).s(" irq: ").u(override->source).s(" int: ").u(override->global_int).endl();

					break;
				}

				case MADT::LocalAPICAddressOverrideEntry:
				{
					auto override = (MADT::LocalAPICAddressOverride *)entry;

					APIC::set_registers(override->apic_address);

					break;
				}

				default:
					break;
			}
		}
	};

	void initialize()
	{
		if(has_table)
		{
			Memory::ScopedBlock block;

			rsdp = *block.map_object<RSDP>(acpi_table);

			assert(rsdp.signature == RSDP::signature_magic, "Invalid ACPI RSDP signature");
			assert(checksum((uint8_t *)&rsdp, (uint8_t *)(&rsdp + 1)) == 0, "Invalid ACPI RSDP checksum");
		}
		else
			search();

		Memory::ScopedBlock rsdt_block;

		RSDT *rsdt = (RSDT *)load_table(rsdt_block, rsdp.address);

		assert(rsdt->signature == RSDT::signature_magic, "Invalid ACPI RSDT table magic");

		size_t tables = (rsdt->length - sizeof(SDT)) / sizeof(uint32_t);

		for(size_t i = 0; i < tables; ++i)
		{
			Memory::ScopedBlock table_block;

			SDT *table = load_table(table_block, rsdt->tables[i]);

			console.s("Found ACPI Table: ").str_array(table->signature_string).endl();

			if(table->signature == MADT::signature_magic)
				parse_madt((MADT *)table);
		}
	}
};
