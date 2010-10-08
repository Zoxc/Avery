#pragma once
#include "../common.hpp"
#include "../multiboot.hpp"
#include "arch.hpp"

namespace Debug
{
	struct Symbol
	{
		uint32_t name;
		uint32_t value;
		uint32_t size;
		uint8_t info;
		uint8_t other;
		uint16_t shndx;
	} __attribute__((packed));

	extern const Symbol *symbols;
	extern const Symbol *symbols_end;
	
	extern const char *symbol_names;
	extern const char *symbol_names_end;
	
	void initialize(const multiboot_t &info);
};
