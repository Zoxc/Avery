#include "lock.hpp"
#include "cpu.hpp"

namespace Arch
{
	Lock::Lock()
	{
		debug(owner = -1);
	}

	void Lock::enter()
	{
		auto ticket = __sync_fetch_and_add(&entries, 1);

		while(ticket != exits)
			pause();

		assert(owner == (size_t)-1);
		debug(owner = CPU::current->index);
	}

	void Lock::leave()
	{
		assert(owner == CPU::current->index);
		debug(owner = -1);

		++exits;
	}
};
