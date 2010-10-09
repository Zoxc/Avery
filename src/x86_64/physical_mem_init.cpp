#include "physical_mem_init.hpp"
#include "memory.hpp"
#include "console.hpp"
#include "debug.hpp"

namespace Memory
{
	namespace Physical
	{
		namespace Initial
		{
			extern "C"
			{
				extern void *kernel_start;
				extern void *kernel_end;
			};
			
			struct ReservedEntry
			{
				ReservedEntry(size_t base, size_t size) : entry(base, size), found(false) {}
				
				InitialEntry entry;
				bool found;
			};
			
			// Functions to work with the initial memory map
			
			void load_memory_map(const multiboot_t &info);
			void punch_holes(ReservedEntry *holes[], size_t hole_count);
			void align_holes();
			InitialEntry *find_biggest_entry();
			
			InitialEntry *list;
			size_t overhead;
			
			// The temporary allocator
			
			InitialEntry *entry;
			size_t current;
		}
	};
};

void *Memory::Physical::Initial::allocate(size_t size, size_t alignment)
{
	size_t result = align(current, alignment);
	
	current = result + size;
	
	assert(current <= entry->end);
	
	return (void *)result;
}

void Memory::Physical::Initial::load_memory_map(const multiboot_t &info)
{
	list = 0;
	
	InitialEntry *entry = (InitialEntry *)info.mmap_addr;
	InitialEntry *end = (InitialEntry *)(info.mmap_addr + info.mmap_length);
	
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

void Memory::Physical::Initial::punch_holes(ReservedEntry *holes[], size_t hole_count)
{
	InitialEntry *entry = list;
	InitialEntry *prev = entry;
	
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

void Memory::Physical::Initial::align_holes()
{
	InitialEntry *entry = list;
	InitialEntry *prev = entry;
	
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

Memory::Physical::Initial::InitialEntry *Memory::Physical::Initial::find_biggest_entry()
{
	InitialEntry *result = 0;
	
	for(InitialEntry *entry = list; entry; entry = entry->get_next())
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

void Memory::Physical::Initial::initialize(const multiboot_t &info)
{
	// Make sure we have the required multiboot flags
	assert(info.flags & MULTIBOOT_FLAG_MMAP, "No memory map passed!");
	
	// Load the memory map from the multiboot header
	load_memory_map(info);
	
	if(!list)
		console.panic().s("No usable memory found!").endl();
	
	size_t reserved_hole_count = 0;
	ReservedEntry *reserved_holes[3];
	
	ReservedEntry kernel_hole((size_t)&kernel_start - kernel_location, (size_t)&kernel_end - kernel_location);
	
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
	current = entry->base;
	
	console.s("Storing allocator data from ").x(entry->base).s(" - ").x(entry->end).lb();
	
	for(InitialEntry *entry = list; entry; entry = entry->get_next())
		console.s("- Hole ").x(entry->base).s(" - ").x(entry->end).lb();
}