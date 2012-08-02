#include "arch.hpp"
#include "../console.hpp"
#include "io.hpp"
#include "memory.hpp"

namespace Arch
{
	struct Segment
	{
		uint16_t limit_low;
		uint16_t base_low;
		uint8_t base_middle;
		unsigned int accessed : 1;
		unsigned int readable : 1;
		unsigned int conforming : 1;
		unsigned int executable : 1;
		unsigned int one : 1;
		unsigned int privilege_level : 2;
		unsigned int present : 1;
		unsigned int limit_high : 4;
		unsigned int user : 1;
		unsigned int long_mode : 1;
		unsigned int operand_size : 1;
		unsigned int granularity : 1;
		uint8_t base_high;
	} __attribute__((packed));
	
	struct GDTPointer
	{
		uint16_t limit;
		Segment *base;
	} __attribute__((packed));
	
	GDTPointer gdt_ptr;

	Segment gdt_entries[3];
	
	void set_segment(uint8_t index, bool code, bool usermode);
	
	extern "C"
	{
		extern void flush_gdt(GDTPointer &gdt_ptr);
	};
};

void Arch::set_segment(uint8_t index, bool code, bool usermode)
{
	auto &segment = gdt_entries[index];

	segment.conforming = 0;
	segment.executable = code;
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
	
	asm volatile ("lgdt %0" :: "m"(gdt_ptr));

	//load_segments(0x10, 0x8);
}
