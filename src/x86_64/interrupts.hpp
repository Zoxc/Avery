#pragma once
#include "arch.hpp"

namespace Interrupts
{
	struct Info
	{
		uint64_t ds;
		uint16_t padding[3];
		uint64_t r11, r10, r9, r8, rdi, rcx, rax, rsi, rdx;
		uint64_t rip, cs, rflags, rsp, ss;
	} __attribute__((packed));

	typedef void (*handler_t)(const Info &info, uint8_t index, size_t error_code);

	const size_t handler_count = 256;

	void register_handler(uint8_t index, handler_t handler);

	void enable();
	void disable();

	void initialize_idt();
	void load_idt();
};
