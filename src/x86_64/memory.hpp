#pragma once
#include "../common.hpp"
#include "arch.hpp"

namespace Memory
{
	extern "C"
	{
		extern void *kernel_start;
		extern void *kernel_end;
	};
	
	const size_t table_entries = 512;
	
	typedef uint64_t table_t[table_entries] __attribute__((aligned(0x1000)));

	const size_t page_size = Arch::page_size;
	const size_t pt_size = table_entries * page_size;
	const size_t pdt_size = table_entries * pt_size;
	const size_t pdpt_size = table_entries * pdt_size;
	const size_t pml4t_size = table_entries * pdpt_size;

	const size_t write_bit = 2;
	const size_t present_bit = 1;
	const size_t nx_bit = 0x8000000000000000;

	const size_t page_flags = 0x80000000000003FF;
	
	const size_t upper_half_start = 0xFFFF800000000000;
	const size_t lower_half_end = 0x0000800000000000;
	
	const size_t kernel_location = 0xFFFFFFFF80000000;
	
	const size_t mapped_pml4t = 0xFFFFFF0000000000;
	
	void map_address(size_t address, size_t physical, size_t flags);
	
	static inline size_t offset(const void *pointer)
	{
		return (size_t)pointer;
	}

	size_t *page_entry(const void *pointer);

	size_t physical(const void *virtual_address);

	namespace Initial
	{
		const size_t allocator_memory = kernel_location + pdt_size;
		
		void initialize();
	};
	
	static inline void load_pml4(size_t pml4t)
	{
		asm volatile ("mov %%rax, %%cr3" :: "a"(pml4t));
	}
};
