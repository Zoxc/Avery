#include "idt.hpp"
#include "apic.hpp"
#include "../console.hpp"

namespace Arch
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
	
	struct IDT
	{
		InterruptGate gates[interrupt_handler_count];
	} __attribute__((packed));

	struct IDTPointer
	{
		uint16_t limit;
		IDT *base;
	} __attribute__((packed));

	IDT idt;
	IDTPointer idt_ptr;

	interrupt_handler_t interrupt_handlers[interrupt_handler_count] asm("interrupt_handlers");

	typedef void(*isr_stub_t)();

	void set_gate(uint8_t index, isr_stub_t stub, bool user = false)
	{
		uint64_t target = (uint64_t)stub;

		InterruptGate &gate = idt.gates[index];

		gate.target_low = target & 0xFFFF;
		gate.target_medium = (target >> 16) & 0xFFFF;
		gate.target_high = (target >> 32) & 0xFFFFFFFF;
		gate.segment_selector = 0x08;
		gate.type = 0xE;
		gate.zero = 0;
		gate.privilege_level = user ? 3 : 0;
		gate.present = 1;
	}

	extern "C" void isr_handler();

	template<uint8_t num> __attribute__((naked)) void isr_stub();

	template<uint8_t num> void isr_stub()
	{
		asm volatile ("push %%rdx\n"
			"push %%rsi\n"
			"mov %0, %%rsi\n"
			"jmp isr_handler"
			:: "i"(num));
	}

	template<uint8_t num> __attribute__((naked)) void isr_error_code_stub();

	template<uint8_t num> void isr_error_code_stub()
	{
		asm volatile ("xchg %%rdx, (%%rsp)\n"
			"push %%rsi\n"
			"mov %0, %%rsi\n"
			"jmp isr_handler"
			:: "i"(num));
	}

	template<size_t index> struct IsrSelector
	{
		static isr_stub_t select()
		{
			return &isr_stub<index>;
		}
	};

	template<> struct IsrSelector<8> { static isr_stub_t select() { return &isr_error_code_stub<8>; } };
	template<> struct IsrSelector<10> { static isr_stub_t select() { return &isr_error_code_stub<10>; } };
	template<> struct IsrSelector<11> { static isr_stub_t select() { return &isr_error_code_stub<11>; } };
	template<> struct IsrSelector<12> { static isr_stub_t select() { return &isr_error_code_stub<12>; } };
	template<> struct IsrSelector<13> { static isr_stub_t select() { return &isr_error_code_stub<13>; } };

	template<size_t index> struct IsrSetup
	{
		static void setup()
		{
			set_gate(index, IsrSelector<index>::select());

			return IsrSetup<index + 1>::setup();
		}
	};

	template<> struct IsrSetup<interrupt_handler_count - 1>
	{
		static void setup()
		{
		}
	};

	void default_handler(const InterruptInfo &info, uint8_t index, size_t error_code)
	{
		uint64_t cr2;

		asm ("mov %%cr2, %%rax" : "=a"(cr2));

		console.panic().s("Unhandled interrupt: ").u(index).lb().lb().color(Console::Default)
			.s("errnr:  ").x(error_code).a()
			.s("indx:   ").x(index).a()
			.lb()
			.s("rsp:    ").x(info.rsp).a()
			.s("rip:    ").x(info.rip).a()
			.lb()
			.s("rax:    ").x(info.rax).a()
			.s("cr2:    ").x(cr2).a()
			.lb()
			.s("rcx:    ").x(info.rcx).a()
			.s("rdx:    ").x(info.rdx).a()
			.lb()
			.s("ss:     ").x(info.ss).a()
			.s("cs:     ").x(info.cs).a()
			.lb()
			.s("ds:     ").x(info.ds).a()
			.s("rflags: ").x(info.rflags).a()
		.endl();
	}
};

void Arch::register_interrupt_handler(uint8_t index, interrupt_handler_t handler)
{
	interrupt_handlers[index] = handler;
}

extern "C" void spurious_irq();

void setup_pics()
{
	const size_t master_command = 0x20;
	const size_t master_data = 0x21;
	const size_t slave_command = 0xA0;
	const size_t slave_data = 0xA1;

	const size_t pic_init = 0x11;

	const size_t pic_mask_all = 0xFF;

	// Remap the PICs IRQ tables

	Arch::outb(master_command, pic_init);
	Arch::outb(master_data, 0xF8);
	Arch::outb(master_data, 0x04);
	Arch::outb(master_data, 0x01);
	Arch::outb(master_data, 0x0);

	Arch::outb(slave_command, pic_init);
	Arch::outb(slave_data, 0xF8);
	Arch::outb(slave_data, 0x02);
	Arch::outb(slave_data, 0x01);
	Arch::outb(slave_data, 0x0);

	// Disable the PICs

	Arch::outb(master_data, pic_mask_all);
	Arch::outb(slave_data, pic_mask_all);
}

void Arch::initialize_idt()
{
	idt_ptr.limit = sizeof(idt) - 1;
	idt_ptr.base = &idt;

	setup_pics();

	IsrSetup<0>::setup();

	set_gate(0xFF, spurious_irq);

	for(size_t i = 0; i < interrupt_handler_count; ++i)
		interrupt_handlers[i] = &default_handler;

	load_idt();
}

void Arch::load_idt()
{
	asm volatile ("lidt %0" :: "m"(idt_ptr));
}