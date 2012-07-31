#pragma once
#include "../common.hpp"
#include "arch.hpp"
#include "memory.hpp"

namespace Memory
{
	namespace Physical
	{
		struct Hole
		{
			typedef size_t unit_t;

			size_t base;
			size_t pages;
			size_t units;
			unit_t *bitmap;

			void set(size_t index);
			bool get(size_t index);

			static const size_t bits_per_unit = 8 * sizeof(Hole::unit_t);
			static const size_t byte_map_size = bits_per_unit * Arch::page_size;
		};

		physical_page_t allocate_page();

		void initialize();
	};
};
