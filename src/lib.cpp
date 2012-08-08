#include "common.hpp"
#include "lib.hpp"
#include "console.hpp"
#include "memory.hpp"

namespace Runtime
{
	typedef void(*ctor_t)();

	extern "C" const Runtime::ctor_t ctors_start;
	extern "C" const Runtime::ctor_t ctors_end;

	void setup_ctors(const ctor_t *start, const ctor_t *end) __attribute__((noinline));
};

void Runtime::setup_ctors(const ctor_t *start, const ctor_t *end)
{
	for(const ctor_t *ctor = start; ctor != end; ++ctor)
		(*ctor)();
}

void Runtime::initialize()
{
	setup_ctors(&ctors_start, &ctors_end);
}

void Runtime::abort_function(const char *message)
{
	console.panic().s(message).endl();

	__builtin_unreachable();
}

void Runtime::assert_function(const char *message)
{
	console.panic().s(message).endl();

	__builtin_unreachable();
}

void panic(const char *message)
{
	console.panic().s(message).endl();

	__builtin_unreachable();
}

struct MallocHeader
{
	static const size_t magic_value = 0xBAD0ABBA;

	Memory::Block *block;
	size_t bytes;
	size_t magic;
};

struct MallocFooter
{
	static const size_t magic_value = 0xBEEFABBA;

	size_t magic;
};

void *malloc(size_t bytes)
{
	size_t pages = align_up(bytes + sizeof(MallocHeader) + sizeof(MallocFooter), Arch::page_size) / Arch::page_size;

	Memory::Block *block = Memory::allocate_block(Memory::Block::Default, pages);

	Memory::map(block->base, block->pages);

	MallocHeader *header = (MallocHeader *)block->base;
	uint8_t *result = (uint8_t *)(header + 1);
	MallocFooter *footer = (MallocFooter *)(result + bytes);

	header->magic = MallocHeader::magic_value;
	header->block = block;
	header->bytes = bytes;

	footer->magic = MallocFooter::magic_value;

	return result;
}

void free(void *mem)
{
	MallocHeader *header = (MallocHeader *)mem - 1;
	MallocFooter *footer = (MallocFooter *)((uint8_t *)mem + header->bytes);

	assert(header->magic == MallocHeader::magic_value, "Invalid malloc header magic");
	assert(footer->magic == MallocFooter::magic_value, "Invalid malloc footer magic");

	Memory::free_block(header->block);
}

void *operator new(size_t bytes)
{
	return malloc(bytes);
}

void operator delete(void *mem)
{
	free(mem);
}

extern "C"
{
	void __cxa_pure_virtual()
	{
		panic("Abstract virtual function called");
	}

	void *__dso_handle;

	int __cxa_atexit(void (*) (void *), void *, void *)
	{
		return 0;
	}

	void memcpy(void *dst, const void *src, size_t size)
	{
		uint8_t *d = (uint8_t *)dst;
		const uint8_t *s = (const uint8_t *)src;
		
		for(size_t i = 0; i < size; ++i) {
			 d[i] = s[i];
		}
	}

	void *memset(void *ptr, uint8_t value, unsigned long num)
	{
		for(uint8_t *dest = (uint8_t *)ptr; num != 0; num--)
			*dest++ = value;

		return ptr;
	}

	int strncmp(const char *s1, const char *s2, size_t n)
	{
		while ( *s1 && n && ( *s1 == *s2 ) )
		{
			++s1;
			++s2;
			--n;
		}

		if (n == 0)
			return 0;
		else
			return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
	}

	char *strncpy(char *s1, const char *s2, size_t n)
	{
		char *rc = s1;

		while ( ( n > 0 ) && ( *s1++ = *s2++ ) )
			--n;

		while ( n-- > 1 )
			*s1++ = '\0';

		return rc;
	}

};
