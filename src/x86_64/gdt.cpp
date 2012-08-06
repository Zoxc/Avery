#include "gdt.hpp"
#include "memory.hpp"

namespace Arch
{
	GDTPointer gdt_ptr;

	Segment gdt_entries[3];
	
	void set_segment(uint8_t index, bool code, bool usermode);
};

void Arch::set_segment(uint8_t index, bool code, bool usermode)
{
	auto &segment = gdt_entries[index];

	segment.conforming = 0;
	segment.readable = 1; // Bochs requires this set for data
	segment.executable = code;
	segment.one = 1;
	segment.privilege_level = usermode ? 3 : 0;
	segment.present = 1;
	segment.long_mode = 1;
	segment.operand_size = 0;
}

extern "C" void load_segments(size_t data, size_t code);

void Arch::initialize_gdt()
{
	set_segment(1, true, false);
	set_segment(2, false, false);
	
	gdt_ptr.limit = sizeof(gdt_entries) - 1;
	gdt_ptr.base = gdt_entries;
	
	asm volatile("lgdt %0" :: "m"(gdt_ptr));

	load_segments(0x10, 0x8);
}
