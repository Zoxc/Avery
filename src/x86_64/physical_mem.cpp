#include "physical_mem.hpp"
#include "console.hpp"

namespace Memory
{
	namespace Physical
	{
		extern "C"
		{
			extern void *kernel_start;
			extern void *kernel_end;
		};
		
		struct InitialEntry
		{
			union
			{
				uint32_t next_low;
				uint32_t struct_size;
			};
			uint64_t base;
			union
			{
				uint64_t size;
				uint64_t end;
			};
			union
			{
				uint32_t next_high;
				uint32_t type;
			};
			
			InitialEntry(size_t base, size_t size) : base(base), size(size) {}
			
			InitialEntry *get_next()
			{
				return (InitialEntry *)((uint64_t)next_high << 32 | next_low);
			}
			
			void set_next(InitialEntry *next)
			{
				uint64_t next_entry = (uint64_t)next;
				next_low = next_entry & 0xFFFFFFFF;
				next_high = next_entry >> 32;
			};
		} __attribute__((packed));
		
		struct ReservedEntry
		{
			ReservedEntry(size_t base, size_t size) : entry(base, size), found(false) {}
			
			InitialEntry entry;
			bool found;
		};
		
		InitialEntry *get_memory_map(const multiboot_t &info);
		void punch_holes(InitialEntry *&list, ReservedEntry *holes[], size_t hole_count);
	};
};

Memory::Physical::InitialEntry *Memory::Physical::get_memory_map(const multiboot_t &info)
{
	InitialEntry *result = 0;
	
	InitialEntry *tail = 0;
	InitialEntry *entry = (InitialEntry *)info.mmap_addr;
	InitialEntry *end = (InitialEntry *)(info.mmap_addr + info.mmap_length);
	
	while(entry != end)
	{
		if(entry->type == 1)
		{
			entry->end = entry->base + entry->size;
			entry->set_next(result);
			result = entry;
		}
		
		entry++;
	}
	
	return result;
}

void Memory::Physical::punch_holes(InitialEntry *&list, ReservedEntry *holes[], size_t hole_count)
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
						list = entry->get_next();
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

void Memory::Physical::initialize(const multiboot_t &info)
{
	if(!(info.flags & MULTIBOOT_FLAG_MMAP))
		console.panic().s("No memory map passed!").endl();
	
	InitialEntry *list = get_memory_map(info);
	
	if(!list)
		console.panic().s("No usable memory found!").endl();
			
	console.s("list at ").x(list).endl();
	
	InitialEntry *entry = list;
	
	while(entry)
	{
			console.s("entry at ").x(entry->base).s(" (").x(entry->size).s(")").lb();
			
			entry = entry->get_next();
	}
	
	size_t reserved_hole_count = 0;
	ReservedEntry *reserved_holes[1];
	
	ReservedEntry kernel_hole((size_t)&kernel_start, (size_t)&kernel_end);
	
	reserved_holes[reserved_hole_count++] = &kernel_hole;
	
	punch_holes(list, reserved_holes, reserved_hole_count);

	entry = list;
	
	size_t memory = 0;
	size_t holes = 0;
	
	while(entry)
	{
			console.s("pentry at ").x(entry->base).s(" (").x(entry->size).s(")").lb();
			
			size_t start = align(entry->base, Arch::page_size);
			size_t size = align_down(entry->end, Arch::page_size) - start;
			
			memory += size;
			holes++;
			
			entry = entry->get_next();
	}
	
	size_t page_map_size = align(memory, byte_map_size) / byte_map_size;
	
	holes *= sizeof(Hole);
	
	console.s("Total usable RAM: ").x(memory).lb();
	console.s("Overhead: ").x(page_map_size + holes).lb();
}