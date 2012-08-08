#include "process.hpp"

Vector<Process *> processes;

Process::Process() :
	allocator((Memory::VirtualPage *)Memory::user_start, (Memory::VirtualPage *)Memory::user_end)
{
	processes.push(this);
}
