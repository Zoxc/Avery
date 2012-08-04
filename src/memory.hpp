#pragma once
#include "common.hpp"
#include "arch.hpp"
#include "util/linked-list.hpp"
#include "util/fast-list.hpp"

namespace Memory
{
	struct Block
	{
		unsigned int free : 1;
		Memory::VirtualPage *base;
		size_t pages;

		Block *list_prev;
		Block *list_next;

		Block *linear_prev;
		Block *linear_next;
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
		void initialize(size_t start, size_t end);

		Block *allocate(size_t pages = 1);
		void free(Block *block);
	};

	extern Allocator allocator;

	void initialize();

	Block *allocate_pages(size_t pages = 1);
	void free_pages(Block *block);
};
