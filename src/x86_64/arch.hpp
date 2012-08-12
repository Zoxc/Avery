#pragma once
#include "../common.hpp"
#include "../console.hpp"
#include "init/init.hpp"

namespace Arch
{
	struct Registers
	{
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
