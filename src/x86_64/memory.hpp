#pragma once
#include "../common.hpp"
#include "../multiboot.hpp"
#include "arch.hpp"

namespace Memory
{
	const size_t pt_size = 512 * sizeof(size_t);
	const size_t pdt_size = 512 * pt_size;
	const size_t pdpt_size = 512 * pdt_size;
	const size_t pml4t_size = 512 * pdpt_size;
	
	const size_t upper_half_start = 0xFFFF800000000000;
	const size_t lower_half_end = 0x0000800000000000;
	
	const size_t kernel_memory = 0xFFFFFF0000000000;
	
	
	const size_t mapped_pml4t = kernel_memory - pdpt_size;
	
	void map_address(size_t address, size_t physical);
};
