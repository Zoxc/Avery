#pragma once
#include "../common.hpp"

namespace Arch
{
	struct IDTEntry
	{
		uint8_t data[16];
	} __attribute__((packed));
	
	struct InterruptGate
	{
		uint16_t base_low;
		uint16_t segment_selector;
		uint8_t ist_reserved0;

		unsigned int type : 4;
		unsigned int zero: 1;
		unsigned int privilege_level : 2;
		unsigned int present : 1;

		uint16_t base_medium;
		uint32_t base_high;
		uint64_t reserved1;
	} __attribute__((packed));
	
	struct IDTPointer
	{
		uint16_t limit;
		IDTEntry *base;
	} __attribute__((packed));
	
	struct InterruptInfo
	{
		uint64_t ds;                  // Data segment selector
		uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rbx, rdx, rcx, rax; // Pushed by pusha.
		uint64_t interrupt_index, error_code;    // Interrupt number and error code (if applicable)
		uint64_t rip, cs, rflags, prev_rsp, ss; // Pushed by the processor automatically.
	} __attribute__((packed));
	
	typedef void (*interrupt_handler_t)(const InterruptInfo &);
	
	extern interrupt_handler_t interrupt_handlers[256];
	
	void register_interrupt_handler(uint8_t index, interrupt_handler_t handler);
	
	void initialize_idt();
	void initialize();
	void panic();
};
