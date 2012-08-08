#include "memory.hpp"
#include "../console.hpp"
#include "../physical_mem.hpp"
#include "../physical_mem_init.hpp"

namespace Memory
{
	#define MEMORY_PAGE_ALIGN __attribute__((aligned(0x1000)));

	table_t ptl4_static MEMORY_PAGE_ALIGN;
	table_t ptl3_static MEMORY_PAGE_ALIGN;
	table_t ptl2_kernel MEMORY_PAGE_ALIGN;
	table_t ptl2_dynamic MEMORY_PAGE_ALIGN;
	table_t ptl1_kernel MEMORY_PAGE_ALIGN;
	table_t ptl1_physical MEMORY_PAGE_ALIGN;
	table_t ptl1_frame MEMORY_PAGE_ALIGN;
	table_t ptl1_low_memory MEMORY_PAGE_ALIGN;
	
	namespace Initial
	{
		void map_page_table(table_t &pt, size_t start_page_offset, size_t end_page_offset, addr_t base, size_t flags)
		{
			assert_page_aligned((addr_t )base);

			flags |= present_bit;
			size_t start_index = align_down(start_page_offset, Arch::page_size) / Arch::page_size;
			size_t end_index = align_up(end_page_offset, Arch::page_size) / Arch::page_size;

			assert(start_index < table_entries && start_index < end_index && end_index < table_entries, "Range out of bounds");
			
			for(size_t i = start_index; i < end_index; i++)
				pt[i] = page_table_entry(base + (i - start_index) * Arch::page_size, flags);
		}

		page_table_entry_t table_entry_from_data(void *table)
		{
			return page_table_entry(physical_page((VirtualPage *)table), present_bit | write_bit);
		}
	}

	bool page_table_entry_present(page_table_entry_t entry)
	{
		return (ptr_t)entry & present_bit;
	}

	void ensure_table_entry(table_t *table, size_t index, AddressSpace *storage)
	{
		if(!page_table_entry_present((*table)[index]))
		{
			addr_t page = Physical::allocate_page();

			if(storage)
				storage->pages.push(page);

			(*table)[index] = page_table_entry(page, present_bit | write_bit);
		}
	}

	template<typename F> void decode_address(VirtualPage *pointer, F func)
	{
		auto address = (ptr_t)pointer;

		assert_page_aligned(address);

		assert((address & (Arch::page_size - 1)) == 0, "Unaligned page");

		address &= ~upper_half_bits;

		address >>= 12;

		size_t ptl1_index = address & (table_entries - 1);

		address >>= 9;

		size_t ptl2_index = address & (table_entries - 1);

		address >>= 9;

		size_t ptl3_index = address & (table_entries - 1);

		address >>= 9;

		size_t ptl4_index = address & (table_entries - 1);

		func(ptl4_index, ptl3_index, ptl2_index, ptl1_index);
	}

	page_table_entry_t *get_page_entry(VirtualPage *pointer)
	{
		ptr_t phy_ptr;

		decode_address(pointer, [&](size_t ptl4_index, size_t ptl3_index, size_t ptl2_index, size_t ptl1_index) {
			phy_ptr = mapped_pml1ts + ptl4_index * ptl2_size + ptl3_index * ptl1_size + ptl2_index * page_size + ptl1_index * sizeof(size_t);
		});

		return (page_table_entry_t *)phy_ptr;
	}

	page_table_entry_t *ensure_page_entry(VirtualPage *pointer, AddressSpace *storage)
	{
		ptr_t phy_ptr;

		decode_address(pointer, [&](size_t ptl4_index, size_t ptl3_index, size_t ptl2_index, size_t ptl1_index) {
			ensure_table_entry(&ptl4_static, ptl4_index, storage);

			auto ptl3 = (table_t *)(mapped_pml3ts + ptl4_index * page_size);

			ensure_table_entry(ptl3, ptl3_index, storage);

			auto ptl2 = (table_t *)(mapped_pml2ts + ptl4_index * ptl1_size + ptl3_index * page_size);

			ensure_table_entry(ptl2, ptl2_index, storage);

			phy_ptr = mapped_pml1ts + ptl4_index * ptl2_size + ptl3_index * ptl1_size + ptl2_index * page_size + ptl1_index * sizeof(size_t);
		});

		return (page_table_entry_t *)phy_ptr;
	}

};

addr_t Memory::physical_page(VirtualPage *virtual_address)
{
	return physical_page_from_table_entry(*get_page_entry(virtual_address));
}

addr_t Memory::physical_address(const volatile void *virtual_address)
{
	return physical_page((VirtualPage *)align_down((ptr_t)virtual_address, Arch::page_size)) + ((ptr_t)virtual_address & (Arch::page_size - 1));
}

void Memory::protect(VirtualPage *address, size_t pages, size_t flags)
{
	for(VirtualPage *end = address + pages; address < end; ++address)
	{
		auto entry = get_page_entry(address);
		*entry = page_table_entry(physical_page_from_table_entry(*entry), flags);

		invalidate_page(address);
	}
}

void Memory::map(VirtualPage *address, size_t pages, size_t flags, AddressSpace *storage)
{
	for(VirtualPage *end = address + pages; address < end; ++address)
		*ensure_page_entry(address, storage) = page_table_entry(Physical::allocate_page(), flags);
}

void Memory::unmap(VirtualPage *address, size_t pages)
{
	for(VirtualPage *end = address + pages; address < end; ++address)
	{
		auto page_entry = get_page_entry(address);

		if((size_t)*page_entry & present_bit)
		{
			Physical::free_page(physical_page_from_table_entry(*page_entry));
			*page_entry = 0;

			invalidate_page(address);
		}
	}
}

void Memory::map_address(VirtualPage *address, size_t pages, addr_t physical, size_t flags, AddressSpace *storage)
{
	for(VirtualPage *end = address + pages; address < end; ++address, physical += page_size)
		*ensure_page_entry(address, storage) = page_table_entry(physical, flags);
}

void Memory::unmap_address(VirtualPage *address, size_t pages)
{
	for(VirtualPage *end = address + pages; address < end; ++address)
	{
		*get_page_entry(address) = 0;

		invalidate_page(address);
	}
}

void Memory::Initial::initialize()
{
	ptl4_static[511] = table_entry_from_data(&ptl3_static);
	ptl4_static[510] = table_entry_from_data(&ptl4_static); // map ptl4 to itself

	ptl3_static[509] = table_entry_from_data(&ptl4_static); // map ptl3 to ptl4
	ptl3_static[510] = table_entry_from_data(&ptl2_kernel);
	ptl3_static[511] = table_entry_from_data(&ptl2_dynamic);
	
	ptl2_kernel[0] = table_entry_from_data(&ptl1_kernel);
	ptl2_kernel[511] = table_entry_from_data(&ptl4_static); // map ptl2 to ptl4

	ptl2_dynamic[0] = table_entry_from_data(&ptl1_physical);
	ptl2_dynamic[1] = table_entry_from_data(&ptl1_frame);
	ptl2_dynamic[2] = table_entry_from_data(&ptl1_low_memory);

	// Map the low memory area

	map_page_table(ptl1_low_memory, 0, 0x100000, 0, nx_bit);

	// Map the physical memory allocator

	map_page_table(ptl1_physical, 0, overhead, entry->base, write_bit | nx_bit);

	// Map framebuffer to virtual memory

	addr_t fb;
	size_t fb_size;

	console.get_buffer_info(fb, fb_size);

	assert(fb_size < ptl1_size, "Framebuffer too large");
	map_page_table(ptl1_frame, 0, fb_size, (addr_t)fb, write_bit | nx_bit);

	// Map kernel segments

	for(size_t i = 0; i < Params::info.segment_count; ++i)
	{
		auto &hole = Params::info.segments[i];

		size_t flags = nx_bit;

		switch(hole.type)
		{
		case Params::SegmentModule:
			continue;

		case Params::SegmentCode:
			flags &= ~nx_bit;
			break;

		case Params::SegmentData:
			flags |= write_bit;
			break;

		case Params::SegmentReadOnlyData:
			break;

		default:
			console.panic().s("Unknown memory hole type").endl();
		}

		size_t virtual_offset = hole.virtual_base - kernel_location;

		map_page_table(ptl1_kernel, virtual_offset, virtual_offset + hole.end - hole.base, hole.base, flags);
	}

	load_pml4(physical_page((VirtualPage *)&ptl4_static));

	console.new_buffer((void *)framebuffer_start);
}
