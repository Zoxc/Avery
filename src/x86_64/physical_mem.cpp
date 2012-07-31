#include "arch.hpp"
#include "physical_mem.hpp"
#include "physical_mem_init.hpp"
#include "memory.hpp"
#include "../console.hpp"
#include "../lib.hpp"

namespace Memory
{
	namespace Physical
	{
		extern Hole *holes;
		size_t hole_count;
	};
};

Memory::Physical::Hole *Memory::Physical::holes = (Hole *)Initial::allocator_memory;

void Memory::Physical::Hole::set(size_t index)
{
	bitmap[index / bits_per_unit] |= 1 << (index & (bits_per_unit - 1));
}

bool Memory::Physical::Hole::get(size_t index)
{
	return (bitmap[index / bits_per_unit] & (1 << (index & (bits_per_unit - 1)))) != 0;
}

Memory::physical_page_t Memory::Physical::allocate_page()
{
	ptr_t result = 0;

	assert(result, "Out of physical memory");

	return (physical_page_t)result;
}

void Memory::Physical::initialize()
{
	hole_count = 0;
	
	Hole *overhead_hole;
	
	for(Initial::Entry *entry = Initial::list; entry; entry = entry->next, ++hole_count)
	{
		Hole &hole = holes[hole_count];
		
		if(entry == Initial::entry)
			overhead_hole = &hole;
		
		hole.base = entry->base;
		hole.pages = (entry->end - entry->base) / Arch::page_size;
		hole.units = align(hole.pages, Hole::bits_per_unit) / Hole::bits_per_unit;
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

		for(size_t p = hole.pages; p < hole.units * Hole::bits_per_unit; ++p)
			hole.set(p);
		
		pos += hole.units;
	}
	
	console.s("Allocator data from ").x(Initial::allocator_memory).s(" - ").x(pos).lb();
	
	size_t used = align((size_t)pos, Arch::page_size) - Initial::allocator_memory;
	
	used /= Arch::page_size;
	
	for(size_t page = 0; page < used; ++page)
		overhead_hole->set(page);
}
