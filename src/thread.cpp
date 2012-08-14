#include "thread.hpp"
#include "process.hpp"

Thread::Thread(Process *process) :
	owner(process)
{
	process->threads.push(this);
}
