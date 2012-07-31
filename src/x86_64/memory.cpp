#include "memory.hpp"
#include "physical_mem_init.hpp"
#include "../console.hpp"

namespace Memory
{
	#define MEMORY_PAGE_ALIGN __attribute__((aligned(0x1000)));

	table_t pml4t MEMORY_PAGE_ALIGN;
	table_t pdpt MEMORY_PAGE_ALIGN;
	table_t pdt_kernel MEMORY_PAGE_ALIGN;
	table_t pdt_dynamic MEMORY_PAGE_ALIGN;
	table_t pt_kernel MEMORY_PAGE_ALIGN;
	table_t pt_physical MEMORY_PAGE_ALIGN;
	table_t pt_frame MEMORY_PAGE_ALIGN;
	
	namespace Initial
	{
		void map_page_table(table_t &pt, size_t start_page_offset, size_t end_page_offset, physical_page_t base, size_t flags)
		{
			assert_page_aligned((ptr_t)base);

			flags |= present_bit;
			size_t start_index = align_down(start_page_offset, Arch::page_size) / Arch::page_size;
			size_t end_index = align(end_page_offset, Arch::page_size) / Arch::page_size;

			assert(start_index < table_entries && start_index < end_index && end_index < table_entries, "Range out of bounds");
			
			for(size_t i = start_index; i < end_index; i++)
				pt[i] = page_table_entry((physical_page_t)((ptr_t)base + (i - start_index) * Arch::page_size), flags);
		}

		page_table_entry_t table_entry_from_data(void *table)
		{
			return page_table_entry(physical((virtual_page_t)table), present_bit | write_bit);
		}
	}
};

Memory::page_table_entry_t *Memory::page_entry(virtual_page_t pointer)
{
	auto address = (ptr_t)pointer;

	assert_page_aligned(address);

	assert((address & (Arch::page_size - 1)) == 0, "Unaligned page");

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

	ptr_t phy_ptr = mapped_pml4t + pml4_index * pdt_size + pdpt_index * pt_size + pdt_index * page_size + pt_index * sizeof(size_t);

	return (page_table_entry_t *)phy_ptr;
}

Memory::physical_page_t Memory::physical(virtual_page_t virtual_address)
{
	return physical_page_from_table_entry(*page_entry(virtual_address));
}

void Memory::map_address(virtual_page_t address, physical_page_t physical, size_t flags)
{
	*page_entry(address) = page_table_entry(physical, flags);
}

void Memory::Initial::initialize()
{
	pml4t[511] = table_entry_from_data(&pdpt);
	pml4t[510] = table_entry_from_data(&pml4t); // map pml4t to itself
	
	pdpt[510] = table_entry_from_data(&pdt_kernel);
	pdpt[511] = table_entry_from_data(&pdt_dynamic);
	
	pdt_kernel[0] = table_entry_from_data(&pt_kernel);
	pdt_dynamic[0] = table_entry_from_data(&pt_physical);
	pdt_dynamic[1] = table_entry_from_data(&pt_frame);

	// Map the physical memory allocator

	map_page_table(pt_physical, 0, overhead, (physical_page_t)entry->base, write_bit);

	// Map framebuffer to virtual memory

	assert(Boot::parameters.frame_buffer_size < pt_size, "Framebuffer too large");
	map_page_table(pt_frame, 0, Boot::parameters.frame_buffer_size, (physical_page_t)Boot::parameters.frame_buffer, write_bit);

	for(size_t i = 0; i < Boot::parameters.segment_count; ++i)
	{
		auto &hole = Boot::parameters.segments[i];

		size_t flags = nx_bit;

		switch(hole.type)
		{
		case Boot::SegmentCode:
			flags &= ~nx_bit;
			break;

		case Boot::SegmentData:
			flags |= write_bit;
			break;

		case Boot::SegmentReadOnlyData:
			break;

		default:
			console.panic().s("Unknown memory hole type").endl();
		}

		size_t virtual_offset = hole.virtual_base - kernel_location;

		map_page_table(pt_kernel, virtual_offset, virtual_offset + hole.end - hole.base, (physical_page_t)hole.base, flags);
	}

	load_pml4(physical((virtual_page_t)&pml4t));

	Boot::parameters.frame_buffer = (void *)(kernel_location + pdt_size + pt_size);

	console.update_frame_buffer();
}
