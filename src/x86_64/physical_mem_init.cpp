#include "../common.hpp"
#include "physical_mem_init.hpp"
#include "physical_mem.hpp"
#include "memory.hpp"
#include "../console.hpp"
#include "boot.hpp"

namespace Memory
{
	namespace Initial
	{
		// Functions to work with the initial memory map
		
		void load_memory_map();
		void punch_holes(Boot::Segment *holes, size_t hole_count);
		void align_holes();
		void dump_list();
		Entry *find_biggest_entry();
		
		Entry *list;
		size_t overhead;
		
		Entry *entry; // The entry used to store allocator data
	}
};

void Memory::Initial::load_memory_map()
{
	list = 0;

	Entry *entry = &Boot::parameters.ranges[0];
	Entry *end = &Boot::parameters.ranges[Boot::parameters.range_count];
	
	while(entry != end)
	{
		if(entry->type == Boot::MemoryUsable)
		{
			entry->next = list;
			list = entry;
		}
		
		entry++;
	}
}

void Memory::Initial::punch_holes(Boot::Segment *holes, size_t hole_count)
{
	Entry *entry = list;
	Entry *prev = entry;
	
	while(entry)
	{
		for(size_t i = 0; i < hole_count; i++)
		{
			if(holes[i].found)
				continue;
			
			if(holes[i].base >= entry->base && holes[i].base < entry->end)
			{
				// The hole starts in this entry.

				assert(holes[i].end <= entry->end, "Hole ending outside the region it started in"); // Make sure it ends here too!
				
				holes[i].found = true;
				
				if(holes[i].base == entry->base && holes[i].end == entry->end)
				{
					// The entry and hole match perfectly. Remove the entry from the list.
					
					if(prev == list)
						prev = list = entry->next;
					else
						prev->next = entry->next;
					
					goto skip;
				}
				else if(holes[i].base == entry->base)
				{
					// The entry's and hole's bases match perfectly. Resize the entry.
					
					entry->base = holes[i].end;
				}
				else if(holes[i].end == entry->end)
				{
					// The entry's and hole's ends match perfectly. Resize the entry.
					
					entry->end = holes[i].base;
				}
				else
				{
					// There is space before and after the hole. Allocate a new hole.
					
					size_t entry_end = entry->end;
					
					entry->end = holes[i].base;

					assert(Boot::parameters.range_count < Boot::memory_range_max, "Out of memory ranges to use");

					Entry *new_hole = &Boot::parameters.ranges[Boot::parameters.range_count++];
					
					new_hole->base = holes[i].end;
					new_hole->end = entry_end;
					
					new_hole->next = entry->next;
					entry->next = new_hole;
				}
			}
			else
				assert(holes[i].end <= entry->base || holes[i].end > entry->end, "Memory hole is overlapping with memory area"); // The hole ends, but doesn't start in this entry.
		}
		
		prev = entry;
		
		skip:
		entry = entry->next;
	}
	
	for(size_t i = 0; i < hole_count; i++)
	{
		if(!holes[i].found)
			console.panic().s("Unable to find room for hole (").x(holes[i].base).s(" - ").x(holes[i].end).s(")").endl();
	}
}

void Memory::Initial::align_holes()
{
	Entry *entry = list;
	Entry *prev = entry;
	
	while(entry)
	{
		entry->base = align(entry->base, Arch::page_size);
		entry->end = align_down(entry->end, Arch::page_size);
		
		if(entry->end > entry->base)
			prev = entry; // Go to the next entry
		else
		{
			// No usable memory in this entry. Remove it from the list.
			
			if(prev == list)
				prev = list = entry->next;
			else
				prev->next = entry->next;
		}
		
		entry = entry->next;
	}
}

Memory::Initial::Entry *Memory::Initial::find_biggest_entry()
{
	Entry *result = 0;
	
	for(Entry *entry = list; entry; entry = entry->next)
	{
		if(result)
		{
			if(entry->end - entry->base > result->end - result->base)
				result = entry;
		}
		else
			result = entry;
	}
	
	return result;
}

void Memory::Initial::initialize_physical()
{
	load_memory_map();

	if(!list)
		console.panic().s("No usable memory found!").endl();

	for(size_t i = 0; i < Boot::parameters.segment_count; ++i)
	{
		console.s("- Segment ").x(Boot::parameters.segments[i].base).s(" - ").x(Boot::parameters.segments[i].end).lb();
	}

	punch_holes(&Boot::parameters.segments[0], Boot::parameters.segment_count);
	
	if(!list)
		console.panic().s("No usable memory found after reserving holes!").endl();
	
	align_holes();
	
	if(!list)
		console.panic().s("No usable memory found after removing non-page aligned holes!").endl();
	
	entry = find_biggest_entry();
	
	console.s("Storing allocator data from ").x(entry->base).s(" - ").x(entry->end).lb();
	
	overhead = 0;
	
	for(Entry *entry = list; entry; entry = entry->next)
	{
		overhead += sizeof(Physical::Hole) + align(entry->end - entry->base, Physical::byte_map_size) / Physical::byte_map_size;
		
		console.s("- Memory ").x(entry->base).s(" - ").x(entry->end).lb();
	}
	
	assert(overhead <= pt_size, "Memory map doesn't fit in 2 MB.");
	
	console.s("Overhead is ").x(overhead).s(" bytes").lb();
}
