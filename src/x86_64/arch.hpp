#pragma once
#include "../common.hpp"
#include "../multiboot.hpp"

namespace Arch
{
	struct InterruptInfo
	{
		uint64_t ds;                  // Data segment selector
		uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rbx, rdx, rcx, rax; // Pushed by pusha.
		uint64_t interrupt_index, error_code;    // Interrupt number and error code (if applicable)
		uint64_t rip, cs, rflags, prev_rsp, ss; // Pushed by the processor automatically.
	} __attribute__((packed));
	
	typedef void (*interrupt_handler_t)(const InterruptInfo &);
	
	const size_t page_size = 0x1000;

	extern interrupt_handler_t interrupt_handlers[256];
	
	void register_interrupt_handler(uint8_t index, interrupt_handler_t handler);
	
	void initialize_idt();
	void initialize_gdt();
	
	void initialize(const multiboot_t &info);
	void panic();
};
