#include "memory.hpp"
#include "console.hpp"
#include "debug.hpp"

namespace Memory
{
	table_t pml4t;
	table_t pdpt;
	table_t pdt;
	table_t pt;
};

void Memory::map_address(size_t address, size_t physical)
{
	if(address > lower_half_end)
		address -= upper_half_start - lower_half_end;
		
	address >>= 12;
	
	size_t pt_index = address & (512 - 1);
	
	address >>= 9;
	
	size_t pdt_index = address & (512 - 1);
	
	address >>= 9;
	
	size_t pdpt_index = address & (512 - 1);
	
	address >>= 9;
	
	size_t pml4_index = address & (512 - 1);
	
	console.s(" - pml4: ").x(pml4_index).s(" - pdpt: ").x(pdpt_index).s(" - pdt: ").x(pdt_index).s(" - pt: ").x(pt_index).endl();
}

void Memory::initialize()
{
}