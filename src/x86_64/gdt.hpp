#pragma once
#include "arch.hpp"

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
	
	extern GDTPointer gdt_ptr;

	void initialize_gdt();
};
