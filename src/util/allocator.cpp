#include "allocator.hpp"

void *StandardAllocator::allocate(size_t &bytes)
{
	return malloc(bytes);
}

void *StandardAllocator::reallocate(void *memory, size_t old_size, size_t &new_size)
{
	void *result = malloc(new_size);

	memcpy(result, memory, old_size);

	return result;
}

void StandardAllocator::free(void *memory)
{
	free(memory);
}
