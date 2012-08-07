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
	unsigned long strcmp(const char *str1, const char *str2);
	unsigned long strlen(const char *src);
	bool strrcmp(const char *start, const char *stop, const char *str);
};
