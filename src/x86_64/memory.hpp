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

	struct PageTableEntry;
	typedef PageTableEntry *page_table_entry_t;

	const size_t table_entries = 512;

	typedef page_table_entry_t table_t[table_entries] __attribute__((aligned(0x1000)));

	struct VirtualPage
	{
		uint8_t data[Arch::page_size];
	};

	static_assert(sizeof(VirtualPage) == Arch::page_size, "Invalid VirtualPage size");

	struct PhysicalPage
	{
		uint8_t data[Arch::page_size];
	};

	static_assert(sizeof(PhysicalPage) == Arch::page_size, "Invalid PhysicalPage size");

	const size_t page_size = Arch::page_size;
	const size_t pt_size = table_entries * page_size;
	const size_t pdt_size = table_entries * pt_size;
	const size_t pdpt_size = table_entries * pdt_size;
	const size_t pml4t_size = table_entries * pdpt_size;

	const size_t write_bit = 2;
	const size_t present_bit = 1;
	const size_t nx_bit = 0x8000000000000000;

	const size_t page_flags = 0x80000000000003FF;
	
	const ptr_t upper_half_start = 0xFFFF800000000000;
	const ptr_t lower_half_end = 0x0000800000000000;
	
	const ptr_t kernel_location = 0xFFFFFFFF80000000;
	
	const ptr_t mapped_pml4t = 0xFFFFFF0000000000;
	
	void map_address(VirtualPage *address, PhysicalPage *physical, size_t flags);

	static inline void assert_page_aligned(size_t address)
	{
		assert((address & (Arch::page_size - 1)) == 0, "Unaligned page");
	}

	static inline page_table_entry_t page_table_entry(PhysicalPage *page, size_t flags)
	{
		return (page_table_entry_t)((ptr_t)page | flags);
	}

	static inline PhysicalPage *physical_page_from_table_entry(page_table_entry_t entry)
	{
		return (PhysicalPage *)((ptr_t)entry & ~(page_flags));
	}

	page_table_entry_t *page_entry(VirtualPage *pointer);

	PhysicalPage *physical(VirtualPage *virtual_address);

	namespace Initial
	{
		const ptr_t allocator_memory = kernel_location + pdt_size;
		
		void initialize();
	};
	
	static inline void load_pml4(PhysicalPage *pml4t)
	{
		asm volatile ("mov %%rax, %%cr3" :: "a"(pml4t));
	}
};
