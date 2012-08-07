#pragma once
#include  "arch.hpp"

namespace APIC
{
	enum MessageType
	{
		Fixed,
		LowestPriority,
		SMI,
		RemoteRead,
		NMI,
		Init,
		Startup,
		External
	};

	void *get_registers();
	void set_registers(addr_t registers);
	size_t local_id();
	void ipi(size_t target, MessageType type, size_t vector);
	void initialize();
	void simple_oneshot(size_t ticks);
};
