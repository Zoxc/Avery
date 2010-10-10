#include "../common.hpp"
#include "physical_mem_init.hpp"
#include "physical_mem.hpp"
#include "memory.hpp"
#include "console.hpp"
#include "debug.hpp"

namespace Memory
{
	namespace Initial
	{
		struct ReservedEntry
		{
			ReservedEntry(size_t base, size_t size) : entry(base, size), found(false) {}
			
			Entry entry;
			bool found;
		};
		
		// Functions to work with the initial memory map
		
		void load_memory_map(const multiboot_t &info);
		void punch_holes(ReservedEntry *holes[], size_t hole_count);
		void align_holes();
		Entry *find_biggest_entry();
		
		Entry *list;
		size_t overhead;
		
		Entry *entry; // The entry used to store allocator data
	}
};

Memory::Initial::Entry::Entry(size_t base, size_t size) : base(base), size(size) {}

Memory::Initial::Entry *Memory::Initial::Entry::get_next()
{
	return (Entry *)((uint64_t)next_high << 32 | next_low);
}

void Memory::Initial::Entry::set_next(Entry *next)
{
	uint64_t next_entry = (uint64_t)next;
	next_low = next_entry & 0xFFFFFFFF;
	next_high = next_entry >> 32;
};

void Memory::Initial::load_memory_map(const multiboot_t &info)
{
	list = 0;
	
	Entry *entry = (Entry *)info.mmap_addr;
	Entry *end = (Entry *)(info.mmap_addr + info.mmap_length);
	
	while(entry != end)
	{
		if(entry->type == 1)
		{
			entry->end = entry->base + entry->size;
			entry->set_next(list);
			list = entry;
		}
		
		entry++;
	}
}

void Memory::Initial::punch_holes(ReservedEntry *holes[], size_t hole_count)
{
	Entry *entry = list;
	Entry *prev = entry;
	
	while(entry)
	{
		for(size_t i = 0; i < hole_count; i++)
		{
			if(holes[i]->found)
				continue;
			
			if(holes[i]->entry.base >= entry->base && holes[i]->entry.base < entry->end)
			{
				// The hole starts in this entry.
				
				assert(holes[i]->entry.end < entry->end); // Make sure it ends here too!
				
				holes[i]->found = true;
				
				if(holes[i]->entry.base == entry->base && holes[i]->entry.end == entry->end)
				{
					// The entry and hole match perfectly. Remove the entry from the list.
					
					if(prev == list)
						prev = list = entry->get_next();
					else
						prev->set_next(entry->get_next());
					
					goto skip;
				}
				else if(holes[i]->entry.base == entry->base)
				{
					// The entry's and hole's bases match perfectly. Resize the entry.
					
					entry->base = holes[i]->entry.end;
				}
				else if(holes[i]->entry.end == entry->end)
				{
					// The entry's and hole's ends match perfectly. Resize the entry.
					
					entry->end = holes[i]->entry.base;
				}
				else
				{
					// There is space before and after the hole. Reuse the hole's entry in the list.
					
					size_t entry_end = entry->end;
					
					entry->end = holes[i]->entry.base;
					
					holes[i]->entry.base = holes[i]->entry.end;
					holes[i]->entry.end = entry_end;
					
					holes[i]->entry.set_next(entry->get_next());
					entry->set_next(&holes[i]->entry);
				}
			}
			else
				assert(holes[i]->entry.end <= entry->base || holes[i]->entry.end > entry->end); // The hole ends, but doesn't start in this entry.
		}
		
		prev = entry;
		
		skip:
		entry = entry->get_next();
	}
	
	for(size_t i = 0; i < hole_count; i++)
	{
		if(!holes[i]->found)
			console.panic().s("Unable to find room for hole (").x(holes[i]->entry.base).s(" - ").x(holes[i]->entry.end).s(")").endl();
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
				prev = list = entry->get_next();
			else
				prev->set_next(entry->get_next());
		}
		
		entry = entry->get_next();
	}
}

Memory::Initial::Entry *Memory::Initial::find_biggest_entry()
{
	Entry *result = 0;
	
	for(Entry *entry = list; entry; entry = entry->get_next())
	{
		if(result)
		{
			if(entry->end - entry->base > result->end - result->base)
				result = entry;
		}
		else
			result = entry;
		
		entry = entry->get_next();
	}
	
	return result;
}

void Memory::Initial::initialize_physical(const multiboot_t &info)
{
	// Make sure we have the required multiboot flags
	assert(info.flags & MULTIBOOT_FLAG_MMAP, "No memory map passed!");
	
	// Load the memory map from the multiboot header
	load_memory_map(info);
	
	if(!list)
		console.panic().s("No usable memory found!").endl();
	
	size_t reserved_hole_count = 0;
	ReservedEntry *reserved_holes[3];
	
	ReservedEntry kernel_hole(symbol_to_physical(&kernel_start), symbol_to_physical(&kernel_end));
	
	reserved_holes[reserved_hole_count++] = &kernel_hole;
	
	if(Debug::symbols && Debug::symbol_names)
	{
		// Don't overwrite symbol information
		
		ReservedEntry symbols_hole((size_t)Debug::symbols, (size_t)Debug::symbols_end);
		ReservedEntry symbol_names_hole((size_t)Debug::symbol_names, (size_t)Debug::symbol_names_end);
	
		reserved_holes[reserved_hole_count++] = &symbols_hole;
		reserved_holes[reserved_hole_count++] = &symbol_names_hole;
	}
	
	punch_holes(reserved_holes, reserved_hole_count);
	
	if(!list)
		console.panic().s("No usable memory found after reserving holes!").endl();
	
	align_holes();
	
	if(!list)
		console.panic().s("No usable memory found after removing non-page aligned holes!").endl();
	
	entry = find_biggest_entry();
	
	console.s("Storing allocator data from ").x(entry->base).s(" - ").x(entry->end).lb();
	
	overhead = 0;
	
	for(Entry *entry = list; entry; entry = entry->get_next())
	{
		overhead += sizeof(Physical::Hole) + align(entry->end - entry->base, Physical::byte_map_size) / Physical::byte_map_size;
		
		console.s("- Hole @ ").x(entry).s(" : ").x(entry->base).s(" - ").x(entry->end).lb();
	}
	
	assert(overhead <= pt_size, "Memory map doesn't fit in 2 MB.");
	
	console.s("Overhead is ").x(overhead).s(" bytes").lb();
	
	for(Entry *entry = list; entry; entry = entry->get_next())
	{
		console.s("- Hole0 @ ").x(entry).s(" : ").x(entry->next_low).s(" - ").x(entry->get_next()).lb();
	}
}