#pragma once
#include "../common.hpp"
#include "../multiboot.hpp"
#include "arch.hpp"

namespace Memory
{
	namespace Physical
	{
		struct Hole
		{
			size_t base;
			size_t size;
		};
		
		const size_t byte_map_size = 8 * Arch::page_size;
		
		void initialize(const multiboot_t &info);
	};
};
