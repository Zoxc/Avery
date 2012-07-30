#pragma once
#include "../common.hpp"
#include "arch.hpp"
#include "boot.hpp"

namespace Memory
{
	namespace Initial
	{
		typedef Boot::MemoryRange Entry;

		extern Entry *list;

		extern Entry *entry;
		extern size_t overhead;

		void initialize_physical();
	};
};
