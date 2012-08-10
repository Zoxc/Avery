#include "thread.hpp"
#include "process.hpp"

Thread::Thread(Process *process) :
	owner(process)
{
	memset(&registers, 0, sizeof(Arch::Registers));

	process->threads.push(this);
}
