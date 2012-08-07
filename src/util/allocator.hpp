#pragma once
#include "../lib.hpp"

struct StandardAllocator
{
	static void *allocate(size_t &bytes);
	static void *reallocate(void *memory, size_t old_size, size_t &new_size);
	static void free(void *memory);
};
