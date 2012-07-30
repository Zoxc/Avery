#include "arch.hpp"
#include "physical_mem.hpp"
#include "physical_mem_init.hpp"
#include "memory.hpp"
#include "../console.hpp"

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
	console.s("- marking page ").x(index).s(" for hole ").x(base).lb();
	bitmap[index / 8] |= 1 << (index & 8);
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
	}
	
	assert(overhead_hole, "Didn't find overhead hole!");
	
	uint8_t *pos = (uint8_t *)(holes + hole_count);
	
	for(size_t i = 0; i < hole_count; ++i)
	{
		Hole &hole = holes[i];
		
		hole.bitmap = pos;
		
		pos += align(hole.pages, pages_per_byte) / pages_per_byte;
	}
	
	console.s("Allocator data from ").x(Initial::allocator_memory).s(" - ").x(pos).lb();
	
	size_t used = align((size_t)pos, Arch::page_size) - Initial::allocator_memory;
	
	used /= Arch::page_size;
	
	for(size_t page = 0; page < used; ++page)
		overhead_hole->set(page);
}
