#include "arch.hpp"
#include "../console.hpp"
#include "memory.hpp"

namespace APIC
{
	const size_t base_register = 0x1B;
	volatile uint8_t *registers;

	const size_t reg_version = 0x30;
	const size_t reg_eoi = 0xB0;
	const size_t reg_siv = 0xF0;
	const size_t reg_task_priority = 0x80;
	const size_t reg_lvt_timer = 0x320;
	const size_t reg_lvt_thermal = 0x330;
	const size_t reg_lvt_perf = 0x340;
	const size_t reg_lvt_lint0 = 0x350;
	const size_t reg_lvt_lint1 = 0x360;
	const size_t reg_lvt_error = 0x370;
	const size_t reg_timer_init = 0x380;
	const size_t reg_timer_current = 0x390;
	const size_t reg_timer_div = 0x3E0;
	const size_t reg_ldr = 0xD0;
	const size_t reg_dfr = 0xE0;

	const size_t msr_enable_bit = 1 << 11;

	const size_t sw_enable = 1 << 8;
	const size_t lvt_mask = 1 << 16;
	const size_t periodic_timer = 1 << 17;

	const size_t mt_nmi = 4 << 8;

	volatile uint32_t &reg(size_t offset)
	{
		return *(volatile uint32_t *)(registers + offset);
	}

	void timer(const Arch::InterruptInfo &)
	{
		console.s("Timer working!").endl();
		reg(reg_eoi) = 0;
	}

	void initialize()
	{
		size_t base = Arch::read_msr(base_register);

		auto mapped_physical = (Memory::PhysicalPage *)(((base >> 12) & 0xFFFFFFFFFF) << 12);

		auto mapped_virtual = Memory::simple_allocate();

		registers = (uint8_t *)mapped_virtual;

		*Memory::ensure_page_entry(mapped_virtual) = Memory::page_table_entry(mapped_physical, Memory::rw_data_flags | Memory::no_cache_flags);

		reg(reg_dfr) = -1;
		reg(reg_ldr) = (reg(reg_ldr) & 0x00FFFFFF);
		reg(reg_lvt_timer) = lvt_mask;
		reg(reg_lvt_thermal) = lvt_mask;
		reg(reg_lvt_perf) = lvt_mask;
		reg(reg_lvt_lint0) = lvt_mask;
		reg(reg_lvt_lint1) = lvt_mask;
		reg(reg_task_priority) = 0;

		Arch::write_msr(base_register, base | msr_enable_bit);

		reg(reg_lvt_timer) = 32;
		reg(reg_timer_div) = 3;
		reg(reg_timer_init) = 1000000;
		reg(reg_siv) = 39 | sw_enable;

		Arch::register_interrupt_handler(32, timer);
	}
};
