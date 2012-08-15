#pragma once
#include "../common.hpp"
#include "../console.hpp"
#include "init/init.hpp"

namespace Arch
{
	struct Registers
	{
		Registers();

		static const uint64_t rflags_default = 0x202;

		uint64_t r15, r14, r13, r12, r10, r9, r8, rdi, rsi, rdx, rcx, rbp, rbx, rax;
		uint64_t rip, cs, rflags, rsp, ss;

		void set_ip(ptr_t ip)
		{
			rip = ip;
		}

		void set_stack(ptr_t stack)
		{
			rsp = stack;
		}
	};

	static const constexpr size_t rflags_bit_interrupt = 1ul << 9;

	static const constexpr uint32_t efer = 0xC0000080;
	static const constexpr size_t efer_bit_syscalls = 1;

	static const constexpr size_t gs_base = 0xC0000101;

	const size_t page_size = 0x1000;

	void write_gs_base(ptr_t base);

	size_t read_msr(uint32_t reg);
	void write_msr(uint32_t reg, size_t value);

	void outb(uint16_t port, uint8_t value);
	uint8_t inb(uint16_t port);
	uint16_t inw(uint16_t port);

	void pause();

	void halt();

	void initialize_basic();
	void initialize_memory();
	void initialize();
	void panic();
};

void bp();
