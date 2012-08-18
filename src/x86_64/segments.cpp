#include "segments.hpp"
#include "memory.hpp"
#include "cpu.hpp"

namespace Segments
{
	GDTPointer gdt_ptr;

	struct GDT
	{
		Descriptor segments[5];
		TaskStateDescriptor tsds[CPU::max_cpus];
	} __attribute__((packed));

	GDT gdt;

	void set_segment(uint8_t index, bool code, bool usermode);
	void set_task_segment(ptr_t base);

	void set_segment(uint8_t index, bool code, bool usermode)
	{
		auto &segment = gdt.segments[index];

		segment.conforming = 0;
		segment.readable = 1; // Bochs requires this set for data
		segment.executable = code;
		segment.user_segment = 1;
		segment.privilege_level = usermode ? 3 : 0;
		segment.present = 1;
		segment.long_mode = 1;
		segment.operand_size = 0;
	}

	void set_task_segment(TaskState *tss)
	{
		auto &segment = gdt.tsds[CPU::current->index];

		ptr_t base = (ptr_t)tss;

		segment.base_low = base;
		segment.base_middle = base >> 16;
		segment.base_high = base >> 24;
		segment.base_higher = base >> 32;

		segment.available = 1;
		segment.type = 4;
		segment.user_segment = 0;
		segment.privilege_level = 3;
		segment.present = 1;
		segment.limit_low = sizeof(TaskState) - 1;
		segment.limit_high = 0;
		segment.granularity = 0;
	}

	extern "C" void load_segments(size_t data, size_t code);

	void initialize_gdt()
	{
		set_segment(1, true, false);
		set_segment(2, false, false);
		set_segment(3, false, true);
		set_segment(4, true, true);

		gdt_ptr.limit = sizeof(gdt) - 1;
		gdt_ptr.base = &gdt;

		load_gdt();
	}

	void load_gdt()
	{
		asm volatile("lgdt %0" :: "m"(gdt_ptr));

		load_segments(Segments::data_segment, Segments::code_segment);
	}

	void setup_tss()
	{
		CPU::current->tss.rsps[0] = (ptr_t)CPU::current->stack_end;

		set_task_segment(&CPU::current->self->tss);

		asm volatile("ltr %%ax" :: "a"(__builtin_offsetof(GDT, tsds) + sizeof(TaskStateDescriptor) * CPU::current->index));
	}
};
