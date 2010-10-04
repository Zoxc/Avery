#pragma once

namespace Runtime
{
	void initialize();
};

void *memset(void *ptr, int value, unsigned long num);
unsigned long strcmp(const char *str1, const char *str2);
unsigned long strlen(const char *src);
bool strrcmp(const char *start, const char *stop, const char *str);
