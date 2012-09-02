#pragma once
#include "../common.hpp"

namespace Arch
{
	struct Lock
	{
		Lock();

		volatile size_t entries, exits;
		debug(size_t owner);

		void enter();
		void leave();
	};
};
