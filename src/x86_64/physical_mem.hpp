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

			PhysicalPage *base;
			PhysicalPage *end;
			size_t pages;
			size_t units;
			unit_t *bitmap;

			void clear(size_t index);
			void set(size_t index);
			bool get(size_t index);

			static const size_t bits_per_unit = 8 * sizeof(Hole::unit_t);
			static const size_t byte_map_size = bits_per_unit * Arch::page_size;

			template<typename F> void address(size_t index, F func)
			{
				assert(index < pages, "Out of bounds");

				func(bitmap[index / bits_per_unit], (size_t)1 << (index & (bits_per_unit - 1)));
			}
		};

		PhysicalPage *allocate_page();
		void free_page(PhysicalPage *page);

		void initialize();
	};
};
