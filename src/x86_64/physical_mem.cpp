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
		
		class HoleWalker
		{
			public:
				HoleWalker(const multiboot_t &info)
				{
					start = (mmap_entry_t *)info.mmap_addr;
					end = (mmap_entry_t *)(info.mmap_addr + info.mmap_length);
					
					reset();
				}
				
				bool get_next(Hole &hole)
				{
					if(entry == end)
						return false;
					
					if(entry->type != 1)
					{
						entry++;
						
						return get_next(hole);
					}
					
					size_t entry_end = entry->base_addr + entry->length;
					
					if((size_t)&kernel_start >= entry->base_addr && (size_t)&kernel_start < entry_end) // Check if the kernel starts in this hole.
					{
						if((size_t)&kernel_start == entry->base_addr) // Check if the kernel start with this hole.
						{
							if((size_t)&kernel_end < entry_end) // Check if the kernel fits in the hole, if so, spawn only the free space at the end.
							{
								hole.base = (size_t)&kernel_end;
								hole.size = entry_end - (size_t)&kernel_end;
								
								entry++;
								
								return true;
							}
							else if((size_t)&kernel_end == entry_end) // Check if the kernel ends with the hole, if so, skip to the next hole.
							{
								entry++;
								
								return get_next(hole);
							}
							else
							{
								console.panic().s("Kernel (").x(&kernel_start).s(" - ").x(&kernel_end).s(") starts, but doesn't end in the same hole.").endl();
							}
						}
						else
						{
							if((size_t)&kernel_end < entry_end) // Check if the kernel fits in the hole, if so, spawn two seperate holes.
							{
								if(kernel)
								{
									hole.base = (size_t)&kernel_end;
									hole.size = entry_end - (size_t)&kernel_end;
									
									entry++;
									kernel = false;
									
									return true;
								}
								else
								{
									hole.base = entry->base_addr;
									hole.size = (size_t)&kernel_start - entry->base_addr;
									
									kernel = true;
									
									return true;
								}
							}
							else if((size_t)&kernel_end == entry_end) // Check if the kernel ends with the hole, if so, spawn only the free space at the start.
							{
								hole.base = entry->base_addr;
								hole.size = (size_t)&kernel_start - entry->base_addr;
								
								kernel = true;
								
								return true;
							}
							else
							{
								console.panic().s("Kernel (").x(&kernel_start).s(" - ").x(&kernel_end).s(") starts, but doesn't end in the same hole.").endl();
							}
						}
					}
					else if((size_t)&kernel_end > entry->base_addr && (size_t)&kernel_end <= entry_end) // Check if the kernel ends in this hole.
					{
						console.panic().s("Kernel (").x(&kernel_start).s(" - ").x(&kernel_end).s(") ends in a hole it doesn't start in.").endl();
					}
					
					hole.base = entry->base_addr;
					hole.size = entry->length;
					
					entry++;
					
					return true;
				}
				
				void reset()
				{
					entry = start;
				}
				
				size_t holes()
				{
					return (size_t)(end - entry) / sizeof(mmap_entry_t);
				}
			
			private:
				bool kernel;
				mmap_entry_t *entry;
				mmap_entry_t *start;
				mmap_entry_t *end;
		};
	};
};

void Memory::Physical::initialize(const multiboot_t &info)
{
	if(!(info.flags & MULTIBOOT_FLAG_MMAP))
		console.panic().s("No memory map passed!").endl();
	
	console.s("Multiboot information at ").x(&info).lb();
	
	size_t memory = 0;
	
	HoleWalker walker(info);
	
	Hole hole;
	
	while(walker.get_next(hole))
	{
			console.s("Hole at ").x(hole.base).s(" (").x(hole.size).s(")").lb();
			
			size_t start = align(hole.base, Arch::page_size);
			size_t size = align_down(hole.base + hole.size, Arch::page_size) - start;
			
			console.s("Usable RAM: ").x(start).s(" (").x(size).s(")").lb();
			
			memory += size;
	}
	
	size_t page_map_size = align(memory, byte_map_size) / byte_map_size;
	
	size_t holes = sizeof(Hole) * walker.holes();
	
	console.s("Total usable RAM: ").x(memory).lb();
	console.s("Overhead: ").x(page_map_size + holes).lb();
}