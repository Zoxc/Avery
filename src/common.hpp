#pragma once

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#if !__has_builtin(__builtin_unreachable)
	static inline void __builtin_unreachable() __attribute((noreturn));
	static inline void __builtin_unreachable()
	{
		while(1); //TODO: replace by panic
	}
#endif

#ifndef __cplusplus
#define bool	_Bool
#define true	1
#define false	0
#endif

typedef signed long ssize_t;
typedef unsigned long size_t;

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;
typedef long long int64_t;
typedef unsigned long uint64_t;

template<class T> static inline const T& min(const T& a, const T& b)
{
	return (a < b) ? a : b;
}

static inline size_t align(size_t value, size_t alignment)
{
	alignment -= 1;
	return (value + alignment) & ~alignment;
};

static inline size_t align_down(size_t value, size_t alignment)
{
	return value & ~(alignment - 1);
};
