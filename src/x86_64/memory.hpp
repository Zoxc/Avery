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

	struct VirtualPage;
	typedef VirtualPage *virtual_page_t;

	struct PhysicalPage;
	typedef PhysicalPage *physical_page_t;

	struct PageTableEntry;
	typedef PageTableEntry *page_table_entry_t;
	
	const size_t table_entries = 512;
	
	typedef page_table_entry_t table_t[table_entries] __attribute__((aligned(0x1000)));

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
	
	void map_address(virtual_page_t address, physical_page_t physical, size_t flags);

	static inline void assert_page_aligned(ptr_t address)
	{
		assert((address & (Arch::page_size - 1)) == 0, "Unaligned page");
	}

	static inline page_table_entry_t page_table_entry(physical_page_t page, size_t flags)
	{
		return (page_table_entry_t)((ptr_t)page | flags);
	}

	static inline physical_page_t physical_page_from_table_entry(page_table_entry_t entry)
	{
		return (physical_page_t)((ptr_t)entry & ~(page_flags));
	}

	page_table_entry_t *page_entry(virtual_page_t pointer);

	physical_page_t physical(virtual_page_t virtual_address);

	namespace Initial
	{
		const size_t allocator_memory = kernel_location + pdt_size;
		
		void initialize();
	};
	
	static inline void load_pml4(physical_page_t pml4t)
	{
		asm volatile ("mov %%rax, %%cr3" :: "a"(pml4t));
	}
};
