#pragma once
#include "../common.hpp"

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

	static inline size_t read_msr(uint32_t reg)
	{
		uint32_t low, high;

		asm volatile ("rdmsr" : "=a" (low), "=d" (high) : "c" (reg));

		return (size_t)low | ((size_t)high << 32);
	}

	static inline void write_msr(uint32_t reg, size_t value)
	{
		asm volatile ("wrmsr" : : "a" (value), "d" (value >> 32), "c" (reg));
	}

	static inline void outb(uint16_t port, uint8_t value)
	{
		asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
	}

	static inline uint8_t inb(uint16_t port)
	{
		uint8_t ret;

		asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));

		return ret;
	}

	static inline uint16_t inw(uint16_t port)
	{
		uint16_t ret;

		asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));

		return ret;
	}

	void enable_interrupts();
	void disable_interrupts();

	void halt();

	void initialize_idt();
	void initialize_gdt();

	void initialize_basic();
	void initialize_memory();
	void initialize();
	void panic();
};

void bp();
