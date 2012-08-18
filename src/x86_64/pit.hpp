#pragma once
#include "arch.hpp"
#include "io-apic.hpp"

namespace PIT
{
	extern IRQ irq;

	void initialize();
};
