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
	const size_t ptl1_size = table_entries * page_size;
	const size_t ptl2_size = table_entries * ptl1_size;
	const size_t ptl3_size = table_entries * ptl2_size;
	const size_t ptl4_size = table_entries * ptl3_size;

	const size_t write_bit = 2;
	const size_t present_bit = 1;
	const size_t nx_bit = 0x8000000000000000;

	const size_t page_flags = 0x80000000000003FF;
	
	const ptr_t upper_half_start = 0xFFFF800000000000;
	const ptr_t lower_half_end = 0x0000800000000000;

	const ptr_t kernel_location = 0xFFFFFFFF80000000;

	const ptr_t mapped_pml1ts = 0xFFFFFF0000000000;
	const ptr_t mapped_pml2ts = kernel_location - ptl2_size;
	const ptr_t mapped_pml3ts = kernel_location + ptl1_size * 511;
	
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

	page_table_entry_t get_page_entry(VirtualPage *pointer);
	page_table_entry_t *ensure_page_entry(VirtualPage *pointer);

	PhysicalPage *physical(VirtualPage *virtual_address);

	namespace Initial
	{
		const ptr_t allocator_memory = kernel_location + ptl2_size;
		
		void initialize();
	};
	
	static inline void load_pml4(PhysicalPage *pml4t)
	{
		asm volatile ("mov %%rax, %%cr3" :: "a"(pml4t));
	}
};
