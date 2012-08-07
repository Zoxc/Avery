#include "apic.hpp"
#include "arch.hpp"
#include "../console.hpp"
#include "../memory.hpp"

namespace APIC
{
	const size_t base_register = 0x1B;
	volatile uint8_t *registers;

	const size_t reg_id = 0x20;
	const size_t reg_version = 0x30;
	const size_t reg_eoi = 0xB0;
	const size_t reg_siv = 0xF0;
	const size_t reg_task_priority = 0x80;
	const size_t reg_icrl = 0x300;
	const size_t reg_icrh = 0x310;
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

	void eoi()
	{
		reg(reg_eoi) = 0;
	}

	void timer(const Arch::InterruptInfo &)
	{
		console.s("Timer working!").endl();
		reg(reg_eoi) = 0;
	}

	void ipi(size_t target, MessageType type, size_t vector)
	{
		reg(reg_icrh) = target << 24;
		reg(reg_icrl) = (vector & 0xFF) | ((type & 7) << 8);
	}

	size_t local_id()
	{
		return reg(reg_id);
	}

	void *get_registers()
	{
		return (void *)registers;
	}

	bool has_base = false;
	addr_t register_base;

	void set_registers(ptr_t registers)
	{
		has_base = true;
		register_base = registers;
	}

	void initialize()
	{
		size_t base = Arch::read_msr(base_register);
		Memory::PhysicalPage *mapped_physical;

		if(has_base)
			mapped_physical = (Memory::PhysicalPage *)register_base;
		else
			mapped_physical = (Memory::PhysicalPage *)(((base >> 12) & 0xFFFFFFFFFF) << 12);

		auto mapped_virtual = Memory::map_physical(mapped_physical, 1, Memory::rw_data_flags | Memory::no_cache_flags)->base;

		registers = (uint8_t *)mapped_virtual;

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
		reg(reg_siv) = 0xFF | sw_enable;

		Arch::register_interrupt_handler(32, timer);
	}

	volatile bool oneshot_done;

	void simple_oneshot_wake(const Arch::InterruptInfo &)
	{
		oneshot_done = true;
		reg(reg_eoi) = 0;
	}

	void simple_oneshot(size_t ticks)
	{
		Arch::disable_interrupts();

		Arch::register_interrupt_handler(32, simple_oneshot_wake);
		oneshot_done = false;
		reg(reg_timer_init) = ticks;

		Arch::enable_interrupts();

		while(!oneshot_done)
			Arch::pause();
	}
};
