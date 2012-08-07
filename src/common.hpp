#pragma once
#include <stddef.h>
#include <stdint.h>

typedef uintptr_t ptr_t;
typedef uint64_t addr_t;

template<class T> static inline const T& min(const T& a, const T& b)
{
	return (a < b) ? a : b;
}

template<class T> static inline T align_up(T value, T alignment)
{
	alignment -= 1;
	return (value + alignment) & ~alignment;
};

template<class T> static inline T align_down(T value, T alignment)
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
