#include "memory.hpp"
#include "physical_mem_init.hpp"
#include "../console.hpp"
#include "debug.hpp"

namespace Memory
{
	table_t pml4t;
	table_t pdpt;
	table_t pdt_static;
	table_t pdt_dynamic;
	table_t pt;
	
	table_t pdpt_low;
	
	table_t pt_physical;
	
	namespace Initial
	{
		void map_page_table(table_t &pt, size_t start, size_t end, size_t offset, size_t flags = 3)
		{
			size_t start_index = align_down(start, Arch::page_size) / Arch::page_size;
			size_t end_index = align(end, Arch::page_size) / Arch::page_size;
			
			for(size_t i = start_index; i < end_index; i++)
				pt[i] = (offset + i * Arch::page_size) | flags;
		}
	}
};

void Memory::map_address(size_t address, size_t physical)
{
	if(address > lower_half_end)
		address -= upper_half_start - lower_half_end;
		
	address >>= 12;
	
	size_t pt_index = address & (table_entries - 1);
	
	address >>= 9;
	
	size_t pdt_index = address & (table_entries - 1);
	
	address >>= 9;
	
	size_t pdpt_index = address & (table_entries - 1);
	
	address >>= 9;
	
	size_t pml4_index = address & (table_entries - 1);
	
	console.s(" - pml4: ").x(pml4_index).s(" - pdpt: ").x(pdpt_index).s(" - pdt: ").x(pdt_index).s(" - pt: ").x(pt_index).endl();
}

void Memory::Initial::initialize()
{
	pml4t[511] = symbol_to_physical(&pdpt) | 3;
	pml4t[510] = symbol_to_physical(&pml4t) | 3; // map pml4t to itself
	
	pdpt[510] = symbol_to_physical(&pdt_static) | 3;
	pdpt[511] = symbol_to_physical(&pdt_dynamic) | 3;
	
	pdt_static[0] = symbol_to_physical(&pt) | 3;
	pdt_dynamic[0] = symbol_to_physical(&pt_physical) | 3;
	
	// Map the lowest 2 MB. Multiboot information is assumed to be stored there.
	pml4t[0] = symbol_to_physical(&pdpt_low) | 3;
	pdpt_low[0] = symbol_to_physical(&pdt_static) | 3;
	
	// Map the first 2 MB for the page table
	map_page_table(pt, 0, table_entries * Arch::page_size, 0);	
	
	// Map up to 2 MB for the physical memory allocator
	map_page_table(pt_physical, 0, entry->base + overhead, entry->base);
	
	/*map_page_table(pt, symbol_to_physical(&kernel_start), symbol_to_physical(&kernel_end));
	map_page_table(pt, symbol_to_physical(Console::vga), symbol_to_physical(Console::vga + Console::max_chars));*/
	
	load_pml4(symbol_to_physical(&pml4t));
}