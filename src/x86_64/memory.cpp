#include "memory.hpp"
#include "console.hpp"
#include "debug.hpp"

namespace Memory
{
	table_t pml4t;
	table_t pdpt;
	table_t pdt;
	table_t pt;
	
	namespace Initial
	{
		void map_page_table(table_t &pt, void *start, void *end, size_t flags = 3)
		{
			size_t start_index = align_down(symbol_to_phsyical(start), Arch::page_size) / Arch::page_size;
			size_t end_index = align(symbol_to_phsyical(end), Arch::page_size) / Arch::page_size;
			
			for(size_t i = start_index; i < end_index; i++)
				pt[i] = i * Arch::page_size | flags;
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
	pml4t[511] = symbol_to_phsyical(&pdpt) | 3;
	pdpt[510] = symbol_to_phsyical(&pdt) | 3;
	pdt[0] = symbol_to_phsyical(&pt) | 3;
	
	// map pml4t to itself
	pml4t[510] = symbol_to_phsyical(&pml4t) | 3;
	
	map_page_table(pt, &kernel_start, &kernel_end);
	map_page_table(pt, Console::vga, Console::vga + Console::max_chars);
	
	load_pml4(symbol_to_phsyical(&pml4t));
}