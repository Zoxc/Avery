#pragma once
#include "../common.hpp"
#include "../console.hpp"

namespace Arch
{
	struct InterruptInfo
	{
		uint64_t ds;                  // Data segment selector
		uint64_t r11, r10, r9, r8, rdi, rsi, rdx, rcx, rax; // Pushed by pusha.
		uint64_t interrupt_index, error_code;    // Interrupt number and error code (if applicable)
		uint64_t rip, cs, rflags, prev_rsp, ss; // Pushed by the processor automatically.
	} __attribute__((packed));
	
	typedef void (*interrupt_handler_t)(const InterruptInfo &);
	
	const size_t page_size = 0x1000;

	extern interrupt_handler_t interrupt_handlers[256];
	
	void register_interrupt_handler(uint8_t index, interrupt_handler_t handler);

	size_t read_msr(uint32_t reg);
	void write_msr(uint32_t reg, size_t value);

	void outb(uint16_t port, uint8_t value);
	uint8_t inb(uint16_t port);
	uint16_t inw(uint16_t port);

	void pause();

	void enable_interrupts();
	void disable_interrupts();

	void halt();

	void initialize_basic();
	void initialize_memory();
	void initialize();
	void panic();
};

void bp();
