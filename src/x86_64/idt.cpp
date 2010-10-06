#include "arch.hpp"
#include "console.hpp"
#include "io.hpp"

namespace Arch
{
	IDTEntry idt_entries[256];
	IDTPointer idt_ptr;
	interrupt_handler_t interrupt_handlers[256];

	void set_gate(uint8_t index, void(*base_ptr)());
	
	extern "C" void isr_handler(const InterruptInfo &info);
	extern "C" void irq_handler(const InterruptInfo &info);
};

void Arch::register_interrupt_handler(uint8_t index, interrupt_handler_t handler)
{
	interrupt_handlers[index] = handler;
}

void Arch::set_gate(uint8_t index, void(*base_ptr)())
{
	uint64_t base = (uint64_t)base_ptr;
	
	InterruptGate &gate = reinterpret_cast<InterruptGate &>(idt_entries[index]);
	
	gate.base_low = base & 0xFFFF;
	gate.base_medium = (base >> 16) & 0xFFFF;
	gate.base_high = (base >> 32) & 0xFFFFFFFF;
	gate.segment_selector = 0x08;
	gate.type = 0xE;
	gate.zero = 0;
	gate.privilege_level = 0;
	gate.present = 1;
}

extern "C" void Arch::isr_handler(const InterruptInfo &info)
{
	interrupt_handler_t handler = interrupt_handlers[info.interrupt_index];
	
	if(handler)
		handler(info);
	else
	{
		console.panic().s("Unhandled interrupt: ").u(info.interrupt_index).lb().lb().fg(Console::light_gray)
			.s("ss:     ").x(info.ss).a()
			.s("rsp:    ").x(info.prev_rsp).a()
			.s("rflags: ").x(info.rflags).a()
			.lb()
			.s("cs:     ").x(info.cs).a()
			.s("ds:     ").x(info.ds).a()
			.s("indx:   ").x(info.interrupt_index).a()
			.lb()
			.s("errnr:  ").x(info.error_code).a()
			.s("rip:    ").x(info.rip).a()
			.s("rax:    ").x(info.rax).a()
			.lb()
			.s("rcx:    ").x(info.rcx).a()
			.s("rdx:    ").x(info.rdx).a()
		.endl();
	}
}

extern "C" void Arch::irq_handler(const InterruptInfo &info)
{
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (info.interrupt_index >= 40)
    {
        // Send reset signal to slave.
        Arch::outb(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    Arch::outb(0x20, 0x20);
	
	isr_handler(info);
}

extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();
extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();

void Arch::initialize_idt()
{
	idt_ptr.limit = sizeof(idt_entries) - 1;
	idt_ptr.base = idt_entries;

	// Remap the IRQ table
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	set_gate(0, isr0);
	set_gate(1, isr1);
	set_gate(2, isr2);
	set_gate(3, isr3);
	set_gate(4, isr4);
	set_gate(5, isr5);
	set_gate(6, isr6);
	set_gate(7, isr7);
	set_gate(8, isr8);
	set_gate(9, isr9);
	set_gate(10, isr10);
	set_gate(11, isr11);
	set_gate(12, isr12);
	set_gate(13, isr13);
	set_gate(14, isr14);
	set_gate(15, isr15);
	set_gate(16, isr16);
	set_gate(17, isr17);
	set_gate(18, isr18);
	set_gate(19, isr19);
	set_gate(20, isr20);
	set_gate(21, isr21);
	set_gate(22, isr22);
	set_gate(23, isr23);
	set_gate(24, isr24);
	set_gate(25, isr25);
	set_gate(26, isr26);
	set_gate(27, isr27);
	set_gate(28, isr28);
	set_gate(29, isr29);
	set_gate(30, isr30);
	set_gate(31, isr31);
	set_gate(32, irq0);
	set_gate(33, irq1);
	set_gate(34, irq2);
	set_gate(35, irq3);
	set_gate(36, irq4);
	set_gate(37, irq5);
	set_gate(38, irq6);
	set_gate(39, irq7);
	set_gate(40, irq8);
	set_gate(41, irq9);
	set_gate(42, irq10);
	set_gate(43, irq11);
	set_gate(44, irq12);
	set_gate(45, irq13);
	set_gate(46, irq14);
	set_gate(47, irq15);

	asm volatile ("lidt %0" :: "m"(idt_ptr));
}
