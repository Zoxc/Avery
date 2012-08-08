#pragma once
#include "common.hpp"
#include "arch.hpp"
#include "memory.hpp"

namespace User
{
	struct Block
	{
		enum Type
		{
			Free,
			Overhead,
			Generic,
			Stack
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
		FastList<Memory::Block, &Memory::Block::list_next> overhead_list; // List of blocks of overhead

		FastList<Block, &Block::list_next> free_block_list; // List of blocks not representing any memory
		LinkedList<Block, &Block::list_next, &Block::list_prev> free_list; // List of blocks representing free memory
		LinkedList<Block, &Block::linear_next, &Block::linear_prev> linear_list; // List of blocks representing free and used memory sorted by virtual address

		Block *current_block;
		Block *end_block;

		Block *allocate_block();
	public:
		Allocator(Memory::VirtualPage *start, Memory::VirtualPage *end);

		void dump();

		Block *allocate(Block::Type type, size_t pages = 1);
		Block *allocate_at(Memory::VirtualPage *address, Block::Type type, size_t pages);
		void free(Block *block);
	};
};
