#include "common.hpp"
#include "lib.hpp"

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

extern "C"
{
	void memcpy(void *dst, const void *src, size_t size)
	{
		uint8_t *d = (uint8_t *)dst;
		const uint8_t *s = (const uint8_t *)src;
		
		for(size_t i = 0; i < size; ++i) {
			 d[i] = s[i];
		}
	}

	void *memset(void *ptr, int value, unsigned long num)
	{
		uint8_t *dest = (uint8_t *)ptr;
		for (; num != 0; num--) *dest++ = value;
		return ptr;
	}

	unsigned long strcmp(const char *str1, const char *str2)
	{
		while(*str1 && *str2 && (*str1++ == *str2++));
		
		if(*str1 == '\0' && *str2 == '\0')
			return 0;
		
		if(*str1 == '\0')
			return -1;
		else
			return 1;
	}

	bool strrcmp(const char *start, const char *stop, const char *str)
	{
		while(*start == *str)
		{
			if(start >= stop)
				return false;
			
			start++;
			str++;
	 
			if(*str == 0)
				return start == stop;
		}
		
		return false;
	}

	unsigned long strlen(const char *src)
	{
		unsigned long i = 0;
		
		while (*src++)
			i++;
		
		return i;
	}
};
