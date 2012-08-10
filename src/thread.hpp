#pragma once
#include "arch.hpp"

class Process;

namespace User
{
	struct Block;
};

class Thread
{
public:
	Process *owner;
	Arch::Registers registers;
	User::Block *stack;

	Thread(Process *process);
};
