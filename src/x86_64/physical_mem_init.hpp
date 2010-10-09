#pragma once
#include "../common.hpp"
#include "../multiboot.hpp"
#include "arch.hpp"

namespace Memory
{
	namespace Physical
	{
		struct Hole
		{
			size_t base;
			size_t size;
		};
		
		const size_t byte_map_size = 8 * Arch::page_size;
		
		namespace Initial
		{
			struct InitialEntry
			{
				union
				{
					uint32_t next_low;
					uint32_t struct_size;
				};
				
				uint64_t base;
				
				union
				{
					uint64_t size;
					uint64_t end;
				};
				
				union
				{
					uint32_t next_high;
					uint32_t type;
				};
				
				InitialEntry(size_t base, size_t size) : base(base), size(size) {}
				
				InitialEntry *get_next()
				{
					return (InitialEntry *)((uint64_t)next_high << 32 | next_low);
				}
				
				void set_next(InitialEntry *next)
				{
					uint64_t next_entry = (uint64_t)next;
					next_low = next_entry & 0xFFFFFFFF;
					next_high = next_entry >> 32;
				};
			} __attribute__((packed));
			
			extern InitialEntry *entry;
			
			void initialize(const multiboot_t &info);
			void *allocate(size_t size, size_t alignment);
		};
	};
};
