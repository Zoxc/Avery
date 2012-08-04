#include "mp.hpp"
#include "memory.hpp"

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

	bool search_area(ptr_t start, ptr_t size, Pointer *&result)
	{
		start += Memory::low_memory_start;

		size_t end = start + size;

		for(auto pointer = (Pointer *)start; (ptr_t)pointer < end; ++pointer)
		{
			if(pointer->signature != Pointer::signature_magic)
				continue;

			if(checksum((uint8_t *)pointer, (uint8_t *)(pointer + pointer->length)) != 0)
				continue;

			result = pointer;
			return true;
		}

		return false;
	}

	Pointer *mp;

	void search()
	{
		if(search_area(bios_start, bios_end - bios_start, mp))
			return;

		if(search_area(0, mmio_start, mp))
			return;

		ptr_t ebda = ((ptr_t)*(uint16_t *)(Memory::low_memory_start + 0x40E)) << 4;

		if(search_area(align(ebda, 16), 0x400, mp))
			return;

		panic("Didn't find MP floating pointer structure");
	}

	void initialize()
	{
		search();

		assert(mp->config_address != 0, "There's no MP configuration header");

		auto config_page = (Memory::PhysicalPage *)align_down(mp->config_address, Arch::page_size);

		auto mapping = Memory::simple_allocate(2);

		*Memory::ensure_page_entry(mapping) = Memory::page_table_entry(config_page, Memory::r_data_flags);
		*Memory::ensure_page_entry(mapping + 1) = Memory::page_table_entry(config_page + 1, Memory::r_data_flags);

		auto cfg = (Configuration *)((uint8_t *)mapping + (mp->config_address & (Arch::page_size - 1)));

		assert(cfg->signature == Configuration::signature_magic, "Invalid signature for MP configuration header");
		assert(checksum((uint8_t *)cfg, (uint8_t *)cfg + cfg->base_table_size) == 0, "Invalid checksum for MP configuration header");

		console.s("MP OEM: ").color(Console::Value).str_array(cfg->oem_id).c(' ').str_array(cfg->product_id).color(Console::Default).endl();
	}
};
