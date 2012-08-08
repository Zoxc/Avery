#pragma once
#include "arch.hpp"
#include "user-memory.hpp"
#include "util/vector.hpp"

class Process
{
public:
	size_t id;
	User::Allocator allocator;
	Memory::AddressSpace address_space;

	Process();
};

extern Vector<Process *> processes;
