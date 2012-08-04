#include "physical_mem_init.hpp"
#include "physical_mem.hpp"
#include "console.hpp"

namespace Memory
{
	namespace Initial
	{
		// Functions to work with the initial memory map
		
		void load_memory_map();
		void punch_holes(Params::Segment *holes, size_t hole_count);
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

	Entry *entry = &Params::info.ranges[0];
	Entry *end = &Params::info.ranges[Params::info.range_count];
	
	while(entry != end)
	{
		if(entry->type == Params::MemoryUsable)
		{
			entry->next = list;
			list = entry;
		}
		
		entry++;
	}
}

void Memory::Initial::punch_holes(Params::Segment *holes, size_t hole_count)
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

					assert(Params::info.range_count < Params::memory_range_max, "Out of memory ranges to use");

					Entry *new_hole = &Params::info.ranges[Params::info.range_count++];
					
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

	punch_holes(&Params::info.segments[0], Params::info.segment_count);
	
	if(!list)
		console.panic().s("No usable memory found after reserving holes!").endl();
	
	align_holes();
	
	if(!list)
		console.panic().s("No usable memory found after removing non-page aligned holes!").endl();
	
	entry = find_biggest_entry();

	overhead = 0;
	size_t memory_in_pages = 0;
	
	for(Entry *entry = list; entry; entry = entry->next)
	{
		memory_in_pages += (entry->end - entry->base) / Arch::page_size;
		overhead += sizeof(Physical::Hole) + sizeof(Physical::Hole::unit_t) * align(entry->end - entry->base, Physical::Hole::byte_map_size) / Physical::Hole::byte_map_size;
	}

	console.s("Available memory: ").u(memory_in_pages * Arch::page_size / 0x100000).s(" MiB").endl();
	
	assert(overhead <= entry->end - entry->base, "Memory allocation overhead is larger than the biggest memory block");
	assert(overhead <= ptl1_size, "Memory map doesn't fit in 2 MB.");

}
