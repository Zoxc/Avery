#include "thread.hpp"
#include "process.hpp"

Thread::Thread(Process *process) :
	owner(process)
{
	debug(thread_list = nil);

	process->threads.push(this);
}
