#pragma once
#include "arch.hpp"
#include "user-memory.hpp"
#include "util/vector.hpp"

class Thread;

class Process
{
public:
	size_t id;
	User::Allocator allocator;
	Memory::AddressSpace address_space;
	Vector<Thread *> threads;

	Process();
};

extern Vector<Process *> processes;
