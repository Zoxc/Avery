#pragma once
#include "arch.hpp"
#include "io-apic.hpp"

namespace PIT
{
	extern IRQ irq;
	extern volatile size_t ticks;

	static const size_t vector = 34;

	void initialize();
};
