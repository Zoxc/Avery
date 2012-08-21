#pragma once
#include "arch.hpp"

namespace Interrupts
{
	struct InterruptGate
	{
		uint16_t target_low;
		uint16_t segment_selector;

		unsigned int ist : 3;
		unsigned int reserved_0 : 5;

		unsigned int type : 4;
		unsigned int zero : 1;
		unsigned int privilege_level : 2;
		unsigned int present : 1;

		uint16_t target_medium;
		uint32_t target_high;
		uint32_t reserved_1;
	} __attribute__((packed));

	verify_size(InterruptGate, 16);

	struct Info
	{
		uint16_t ds;
		uint16_t padding[3];
		uint64_t r11, r10, r9, r8, rdi, rcx, rax, rsi, rdx;
		uint64_t rip, cs, rflags, rsp, ss;
	} __attribute__((packed));

	typedef void (*handler_t)(const Info &info, uint8_t index, size_t error_code);

	typedef void(*isr_stub_t)();

	void set_gate(uint8_t index, InterruptGate &gate);
	void set_gate(uint8_t index, isr_stub_t stub);
	void get_gate(uint8_t index, InterruptGate &gate);

	const size_t handler_count = 256;

	void register_handler(uint8_t index, handler_t handler);

	void enable();
	void disable();

	void initialize_idt();
	void load_idt();
};
