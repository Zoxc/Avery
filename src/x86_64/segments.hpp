#pragma once
#include "arch.hpp"

struct CPU;

namespace Segments
{
	static const constexpr uint16_t code_segment = 0x8;
	static const constexpr uint16_t data_segment = 0x10;

	static const constexpr uint16_t user_code_segment = 0x23;
	static const constexpr uint16_t user_data_segment = 0x1b;

	struct TaskState
	{
		uint32_t reserved_0;
		uint64_t rsps[3];
		uint64_t reserved_1;
		uint64_t ists[7];
		uint64_t reserved_2;
		uint16_t reserved_3;
		uint16_t io_bitmap_offset;
	} __attribute__((packed));

	verify_size(TaskState, 0x68);

	struct TaskStateDescriptor
	{
		uint16_t limit_low;
		uint16_t base_low;
		uint8_t base_middle;
		unsigned int available : 1;
		unsigned int type : 3;
		unsigned int user_segment : 1;
		unsigned int privilege_level : 2;
		unsigned int present : 1;
		unsigned int limit_high : 4;
		unsigned int avl : 1;
		unsigned int reserved_0 : 2;
		unsigned int granularity : 1;
		uint8_t base_high;
		uint32_t base_higher;
		uint32_t reserved_1;
	} __attribute__((packed));

	verify_size(TaskStateDescriptor, 0x10);

	struct Descriptor
	{
		uint16_t limit_low;
		uint16_t base_low;
		uint8_t base_middle;
		unsigned int accessed : 1;
		unsigned int readable : 1;
		unsigned int conforming : 1;
		unsigned int executable : 1;
		unsigned int user_segment : 1;
		unsigned int privilege_level : 2;
		unsigned int present : 1;
		unsigned int limit_high : 4;
		unsigned int user : 1;
		unsigned int long_mode : 1;
		unsigned int operand_size : 1;
		unsigned int granularity : 1;
		uint8_t base_high;
	} __attribute__((packed));
	
	verify_size(Descriptor, 0x8);

	struct GDT;

	struct GDTPointer
	{
		uint16_t limit;
		GDT *base;
	} __attribute__((packed));
	
	extern GDTPointer gdt_ptr;

	void initialize_gdt();
	void load_gdt();
	void setup_tss();
};
