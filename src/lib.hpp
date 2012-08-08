#pragma once
#include "common.hpp"

namespace Runtime
{
	void initialize();
};

void *malloc(size_t bytes);
void free(void *mem);

void *operator new(size_t bytes);
void operator delete(void *mem);

extern "C"
{
	void memcpy(void *dst, const void *src, size_t size);
	void *memset(void *ptr, uint8_t value, unsigned long num);
	char *strncpy(char *s1, const char *s2, size_t n);
	int strncmp(const char *s1, const char *s2, size_t n);
};
