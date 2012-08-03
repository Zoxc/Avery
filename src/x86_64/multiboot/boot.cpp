#include "multiboot.hpp"
#include "../arch.hpp"
#include "../../lib.hpp"
#include "../../params.hpp"
#include "../../kernel.hpp"
#include "../../consoles/vga.hpp"

VGAConsoleBackend vga_console;

extern void *low_end;
extern void *kernel_start;
extern void *rodata_start;
extern void *data_start;
extern void *kernel_end;

void setup_segment(Params::Segment &segment, size_t type, void *&virtual_start, void *&virtual_end)
{
	segment.type = type;
	segment.base = (size_t)&virtual_start - (size_t)&kernel_start + (size_t)&low_end;
	segment.end = segment.base + (size_t)&virtual_end - (size_t)&virtual_start;
	segment.virtual_base = (size_t)&virtual_start;
}

extern "C" void boot_entry(const multiboot_t &info)
{
	Runtime::initialize();

	vga_console.initialize();

	console.initialize(&vga_console);

	if(!(info.flags & MULTIBOOT_FLAG_MMAP))
		panic("Memory map not passed by Multiboot loader");

	size_t segment_count = 0;

	setup_segment(Params::info.segments[segment_count++], Params::SegmentCode, kernel_start, rodata_start);
	setup_segment(Params::info.segments[segment_count++], Params::SegmentReadOnlyData, rodata_start, data_start);
	setup_segment(Params::info.segments[segment_count++], Params::SegmentData, data_start, kernel_end);

	Params::info.segment_count = segment_count;

	auto mmap_end = (size_t)info.mmap_addr + info.mmap_length;
	size_t range_count = 0;

	for(auto mmap = (multiboot_mmap *)(info.mmap_addr); (size_t)mmap < mmap_end; mmap = (multiboot_mmap *)((size_t)mmap + mmap->struct_size + 4))
	{
		if(mmap->type != 1)
			continue;

		assert(range_count < Params::memory_range_max, "Too many memory ranges passed to kernel");

		auto &range = Params::info.ranges[range_count++];

		range.type = Params::MemoryUsable;
		range.base = mmap->base;
		range.end = mmap->base + mmap->size;
	}

	Params::info.range_count = range_count;

	kernel();
}
