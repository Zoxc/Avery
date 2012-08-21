#include "pit.hpp"
#include "apic.hpp"

namespace PIT
{
	IRQ irq(0);

	void pit_interrupt(const Interrupts::Info &, uint8_t, size_t)
	{
		APIC::eoi();
	}

	void initialize()
	{
		Interrupts::register_handler(vector, pit_interrupt);

		irq.route(vector, APIC::local_id());

		uint16_t divisor = 1193182 / 200;

		Arch::outb(0x43, 0x34);
		Arch::outb(0x40, divisor);
		Arch::outb(0x40, divisor >> 8);
	}
};
