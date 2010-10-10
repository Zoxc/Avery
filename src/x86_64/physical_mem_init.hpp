#pragma once
#include "../common.hpp"
#include "../multiboot.hpp"
#include "arch.hpp"

namespace Memory
{
	namespace Initial
	{
		struct Entry
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
			
			Entry(size_t base, size_t size);
			
			Entry *get_next();
			
			void set_next(Entry *next);
		} __attribute__((packed));
		
		extern Entry *list;
		
		extern Entry *entry;
		extern size_t overhead;
		
		void initialize_physical(const multiboot_t &info);
	};
};
