#include "mp.hpp"
#include "../memory.hpp"

namespace MP
{
	const size_t mmio_start = 0xA0000;
	const size_t bios_start = 0xF0000;
	const size_t bios_end = 0x100000;

	uint8_t checksum(uint8_t *start, uint8_t *end)
	{
		uint8_t result = 0;

		for(auto p = start; p < end; ++p)
			result += *p;

		return result;
	}

	bool search_area(ptr_t start, ptr_t size, Pointer &result)
	{
		Memory::ScopedBlock block;

		start = (size_t)block.map_block(start, size);
		size_t end = (size_t)start + size;

		for(auto pointer = (Pointer *)start; (ptr_t)pointer < end; ++pointer)
		{
			if(pointer->signature != Pointer::signature_magic)
				continue;

			if(checksum((uint8_t *)pointer, (uint8_t *)(pointer + pointer->length)) != 0)
				continue;

			result = *pointer;

			return true;
		}

		return false;
	}

	Pointer mp;

	void search()
	{
		if(search_area(bios_start, bios_end - bios_start, mp))
			return;

		if(search_area(0, mmio_start, mp))
			return;

		Memory::ScopedBlock ebda_ptr_block;

		ptr_t ebda = ((ptr_t)*ebda_ptr_block.map_object<uint16_t>(0x40E)) << 4;

		if(search_area(align_up(ebda, 16), 0x400, mp))
			return;

		panic("Didn't find MP floating pointer structure");
	}

	void initialize()
	{
		search();

		assert(mp.config_address != 0, "There's no MP configuration header");


		Memory::ScopedBlock cfg_map;
		const Configuration *cfg = cfg_map.map_object<Configuration>(mp.config_address);

		assert(cfg->signature == Configuration::signature_magic, "Invalid signature for MP configuration header");
		assert(checksum((uint8_t *)cfg, (uint8_t *)cfg + cfg->base_table_size) == 0, "Invalid checksum for MP configuration header");

		console.s("MP OEM: ").color(Console::Value).str_array(cfg->oem_id).c(' ').str_array(cfg->product_id).color(Console::Default).endl();

		Memory::ScopedBlock cfg_block;
		uint8_t *cfg_table = (uint8_t *)cfg_block.map_block(mp.config_address, cfg->base_table_size);

		uint8_t *entry = cfg_table + sizeof(Configuration);
		size_t entry_count = cfg->entry_count;

		while(entry_count--)
		{
			assert(entry - (uint8_t *)cfg_table < cfg->base_table_size, "MP configuration table is too large");

			switch(*entry)
			{
				case ProcessorEntry:
				{
					Processor *processor = (Processor *)entry;

					console.s("| Found processor ").x(processor->local_apic);

					entry = (uint8_t *)(processor + 1);

					break;
				};

				case BusEntry:
				{
					Bus *bus = (Bus *)entry;

					console.s("| Found buss ").u(bus->bus_id).s(" type ").str_array(bus->bus_type);

					entry = (uint8_t *)(bus + 1);

					break;
				};

				case IOAPICEntry:
				{
					IOAPIC *io_apic = (IOAPIC *)entry;

					console.s("| Found I/O APIC ").u(io_apic->ioapic_id).s(" mapped at ").x(io_apic->address);

					entry = (uint8_t *)(io_apic + 1);

					break;
				};

				case IOInterruptEntry:
				{
					Interrupt *interrupt = (Interrupt *)entry;

					console.s("| Found I/O interrupt from bus ").u(interrupt->source_bus_id).s(" going to APIC ").u(interrupt->dest_apic_id);

					entry = (uint8_t *)(interrupt + 1);

					break;
				};

				case LocalInterruptEntry:
				{
					Interrupt *interrupt = (Interrupt *)entry;

					console.s("| Found local interrupt from bus ").u(interrupt->source_bus_id).s(" going to APIC ").u(interrupt->dest_apic_id);

					entry = (uint8_t *)(interrupt + 1);

					break;
				};

				default:
					console.panic().s("Unknown MP configuration type ").u(*entry).endl();
			}
		}

		console.endl();
	}
};
