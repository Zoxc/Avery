#pragma once
#include "arch.hpp"
#include "segments.hpp"

namespace Memory
{
	struct Block;
	struct VirtualPage;
};

class Thread;

struct CPU
{
	CPU *self;
	size_t index;
	size_t acpi_id;
	size_t apic_id;
	uint32_t apic_registers;
	uint32_t apic_tick_rate;
	bool started;
	Memory::Block *stack;
	Memory::VirtualPage *local_pages;
	void *stack_end;
	Segments::TaskState tss;
	void *syscall_temp_rsp;

	Thread *thread;
	Thread *scheduled_thread;

	static const size_t local_page_count = 1;

	static constexpr CPU __attribute__((address_space(256))) *const current = 0;

	static const size_t max_cpus = 32;
	static size_t count;
	static CPU *bsp;
	static CPU cpus[max_cpus];

	void setup(size_t i);
	void setup_gs();
	void map_local_page_tables();

	static CPU *allocate(size_t acpi_id, size_t apic_id);
	static void initialize_basic();
	static void initialize();
};
