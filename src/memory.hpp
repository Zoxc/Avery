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
		void initialize(VirtualPage *start, VirtualPage *end);

		Block *allocate(size_t pages = 1);
		void free(Block *block);
	};

	extern Allocator allocator;

	void initialize();

	void *map_physical_structure(Block *&block, ptr_t addr, size_t size, size_t flags = r_data_flags);

	template<class T> T *map_physical_structure(Block *&block, ptr_t addr, size_t flags = r_data_flags)
	{
		return (T *)map_physical_structure(block, addr, sizeof(T), flags);
	};

	Block *map_physical(PhysicalPage *physical, size_t pages, size_t flags = r_data_flags);
	void unmap_physical(Block *block);

	namespace Physical
	{
		class Block
		{
		private:
			Memory::Block *block;

		public:
			Block(void *&ptr, ptr_t addr, ptr_t size, size_t flags = r_data_flags)
			{
				ptr = map_physical_structure(block, addr, size, flags);
			}

			~Block()
			{
				unmap_physical(block);
			}
		};

		template<class T> class Object
		{
		private:
			Memory::Block *block;

		public:
			Object(T *&ptr, ptr_t addr, size_t flags = r_data_flags)
			{
				ptr = map_physical_structure<T>(block, addr, flags);
			}

			~Object()
			{
				unmap_physical(block);
			}
		};
	};

	Block *allocate_pages(size_t pages = 1);
	void free_pages(Block *block);
};
