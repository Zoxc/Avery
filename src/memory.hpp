#pragma once
#include "common.hpp"
#include "arch/memory.hpp"
#include "util/linked-list.hpp"
#include "util/fast-list.hpp"

namespace Memory
{
	struct Block
	{
		enum Type
		{
			Free,
			Overhead,
			Default,
			Stack,
			UserAllocator,
			PhysicalView
		};

		size_t type;
		Memory::VirtualPage *base;
		size_t pages;

		Block *linear_prev;
		Block *linear_next;

		// Free to be used when allocated

		Block *list_prev;
		Block *list_next;
	};

	class Allocator
	{
	private:
		FastList<Block, &Block::list_next> free_block_list; // List of blocks not representing any memory
		LinkedList<Block, &Block::list_next, &Block::list_prev> free_list; // List of blocks representing free memory
		LinkedList<Block, &Block::linear_next, &Block::linear_prev> linear_list; // List of blocks representing free and used memory sorted by virtual address

		Block *current_block;
		Block *end_block;

		Block first_block;

		Block *allocate_block();
	public:
		Allocator();

		void dump();
		void initialize(VirtualPage *start, VirtualPage *end);

		Block *allocate(Block::Type type, size_t pages = 1);
		void free(Block *block);
	};

	extern Allocator allocator;

	void initialize();

	void *map_physical_structure(Block *&block, addr_t addr, size_t size, size_t flags = r_data_flags);

	template<class T> T *map_physical_structure(Block *&block, ptr_t addr, size_t flags = r_data_flags)
	{
		return (T *)map_physical_structure(block, addr, sizeof(T), flags);
	};

	Block *map_physical(addr_t physical, size_t pages, size_t flags = r_data_flags);

	Block *allocate_block(Block::Type type, size_t pages = 1);
	void free_block(Block *block);

	class ScopedBlock
	{
	private:
		Block *block;

	public:
		ScopedBlock(Block *block) : block(block)
		{
		}

		ScopedBlock() : block(nullptr)
		{
		}

		void *map_block(ptr_t addr, ptr_t size, size_t flags = r_data_flags)
		{
			return map_physical_structure(block, addr, size, flags);
		}

		template<class T> T *map_object(ptr_t addr, size_t flags = r_data_flags)
		{
			return map_physical_structure<T>(block, addr, flags);
		}

		void set(Block *new_block)
		{
			block = new_block;
		}

		~ScopedBlock()
		{
			if(block)
				free_block(block);
		}
	};

};
