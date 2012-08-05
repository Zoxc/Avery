#pragma once

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

typedef uint64_t ptr_t;

template<class T> static inline const T& min(const T& a, const T& b)
{
	return (a < b) ? a : b;
}

static inline size_t align_up(size_t value, size_t alignment)
{
	alignment -= 1;
	return (value + alignment) & ~alignment;
};

static inline size_t align_down(size_t value, size_t alignment)
{
	return value & ~(alignment - 1);
};

template<class Struct, class FieldType, FieldType Struct::*field, size_t found_offset, size_t expected_offset> struct OffsetTest
{
	static_assert(found_offset == expected_offset, "Invalid offset");
};

template<class Struct, size_t found_size, size_t expected_size> struct SizeTest
{
	static_assert(found_size == expected_size, "Invalid size");
};

#define verify_offset(structure, field, offset) static OffsetTest<structure, decltype(structure::field), &structure::field, __builtin_offsetof(structure, field), offset> verify_offset__ ## structure ## __ ## field
#define verify_size(structure, size) static SizeTest<structure, sizeof(structure), size> verify_size__ ## structure

namespace Runtime
{
	void assert_function(const char *message) __attribute((noreturn));
	void abort_function(const char *message) __attribute((noreturn));
};

void panic(const char *message) __attribute((noreturn));

#define debug(statement) statement
#define stringify(value) #value
#define assert_internal(expression, file, line, ...) ((expression) ? (void)0 : (void)Runtime::assert_function(file ":" stringify(line) ": " __VA_ARGS__))
#define assert(expression, ...) assert_internal(expression, __FILE__, __LINE__, ## __VA_ARGS__)

#define abort_internal(file, line, ...) Runtime::abort_function(file ":" stringify(line) ": " __VA_ARGS__)
#define abort(...) abort_internal(__FILE__, __LINE__, ## __VA_ARGS__)
