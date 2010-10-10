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
			size_t pages;
			uint8_t *bitmap;
			
			void set(size_t index);
		};
		
		const size_t pages_per_byte = 8;
		const size_t byte_map_size = pages_per_byte * Arch::page_size;
		
		void initialize();
	};
};
