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
		void *mapped;
		Memory::Physical::Block block(mapped, start, size);

		size_t end = (size_t)mapped + size;

		for(auto pointer = (Pointer *)mapped; (ptr_t)pointer < end; ++pointer)
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

		uint16_t *ebda_ptr;
		Memory::Physical::Object<uint16_t> ebda_ptr_block(ebda_ptr, 0x40E);

		ptr_t ebda = ((ptr_t)*ebda_ptr) << 4;

		if(search_area(align_up(ebda, 16), 0x400, mp))
			return;

		panic("Didn't find MP floating pointer structure");
	}

	void initialize()
	{
		search();

		assert(mp.config_address != 0, "There's no MP configuration header");

		const Configuration *cfg;
		Memory::Physical::Object<const Configuration> cfg_map(cfg, mp.config_address);

		assert(cfg->signature == Configuration::signature_magic, "Invalid signature for MP configuration header");
		assert(checksum((uint8_t *)cfg, (uint8_t *)cfg + cfg->base_table_size) == 0, "Invalid checksum for MP configuration header");

		console.s("MP OEM: ").color(Console::Value).str_array(cfg->oem_id).c(' ').str_array(cfg->product_id).color(Console::Default).endl();
	}
};
