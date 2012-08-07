#pragma once
#include "arch.hpp"
#include "memory.hpp"
#include "util/vector.hpp"

class Process
{
	size_t id;
	Memory::Allocator allocator;
	Memory::AddressSpace address_space;
};

extern Vector<Process *> processes;
