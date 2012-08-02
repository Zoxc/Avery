#pragma once
#include "common.hpp"
#include "arch.hpp"
#include "params.hpp"

namespace Memory
{
	namespace Initial
	{
		typedef Params::MemoryRange Entry;

		extern Entry *list;

		extern Entry *entry;
		extern size_t overhead;

		void initialize_physical();
	};
};
