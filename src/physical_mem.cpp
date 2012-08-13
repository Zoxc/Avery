#include "arch.hpp"
#include "physical_mem.hpp"
#include "physical_mem_init.hpp"
#include "console.hpp"
#include "lib.hpp"

namespace Memory
{
	namespace Physical
	{
		extern Hole *holes;
		size_t hole_count;
	};
};

Memory::Physical::Hole *Memory::Physical::holes = (Hole *)physical_allocator_memory;

void Memory::Physical::Hole::clear(size_t index)
{
	address(index, [&](unit_t &unit, unit_t bit) {
		assert((unit & bit) != 0, "Bit already cleared");

		unit &= ~bit;
	});
}

void Memory::Physical::Hole::set(size_t index)
{
	address(index, [&](unit_t &unit, unit_t bit) {
		assert((unit & bit) == 0, "Bit already set");

		unit |= bit;
	});
}

bool Memory::Physical::Hole::get(size_t index)
{
	bool result;

	address(index, [&](unit_t unit, unit_t bit) {
		result = (unit & bit) != 0;
	});

	return result;
}

void Memory::Physical::free_page(addr_t page)
{
	assert_page_aligned((ptr_t)page);

	for(size_t i = 0; i < hole_count; ++i)
	{
		Hole &hole = holes[i];

		if(page >= hole.base && page < hole.end)
		{
			hole.clear((page - hole.base) / Arch::page_size);
			return;
		}
	}

	abort("Memory doesn't belong to any of the holes");
}

addr_t Memory::Physical::allocate_dirty_page()
{
	for(size_t i = 0; i < hole_count; ++i)
	{
		Hole &hole = holes[i];

		Hole::unit_t *end = hole.bitmap + hole.units;

		for(Hole::unit_t *unit = hole.bitmap; unit < end; ++unit)
		{
			if(*unit != (Hole::unit_t)-1)
			{
				size_t bit_index = __builtin_ffsl(~*unit) - 1;

				*unit |= (size_t)1 << bit_index;

				return hole.base + ((unit - hole.bitmap) * Hole::bits_per_unit + bit_index) * Arch::page_size;
			}
		}
	}

	panic("Out of physical memory");
}

addr_t Memory::Physical::allocate_page()
{
	addr_t result = allocate_dirty_page();

	clear_physical_page(result);

	return result;
}

void Memory::Physical::initialize()
{
	hole_count = 0;
	
	Hole *overhead_hole = 0;
	
	for(Initial::Entry *entry = Initial::list; entry; entry = entry->next, ++hole_count)
	{
		Hole &hole = holes[hole_count];
		
		if(entry == Initial::entry)
			overhead_hole = &hole;
		
		hole.base = entry->base;
		hole.pages = (entry->end - entry->base) / Arch::page_size;
		hole.end = entry->end;
		hole.units = align_up(hole.pages, Hole::bits_per_unit) / Hole::bits_per_unit;
	}
	
	assert(overhead_hole, "Didn't find overhead hole!");
	
	auto pos = (Hole::unit_t *)(holes + hole_count);
	
	for(size_t i = 0; i < hole_count; ++i)
	{
		Hole &hole = holes[i];
		
		hole.bitmap = pos;

		// Clear pages

		memset(hole.bitmap, 0, hole.units * sizeof(Hole::unit_t));

		// Set non-existent pages at the end of the word as allocated

		size_t last_unit = hole.units - 1;

		hole.bitmap[last_unit] = -1; // Set all pages at the end as allocated

		for(size_t p = last_unit * Hole::bits_per_unit; p < hole.pages; ++p) // Clear the valid pages
			hole.clear(p);

		pos += hole.units;
	}

	size_t used = align_up((ptr_t)pos, Arch::page_size) - physical_allocator_memory;
	
	used /= Arch::page_size;
	
	for(size_t page = 0; page < used; ++page)
		overhead_hole->set(page);
}
