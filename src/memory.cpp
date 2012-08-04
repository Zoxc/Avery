#include "memory.hpp"

namespace Memory
{
	Allocator allocator;

	void initialize()
	{
		allocator.initialize(Memory::allocator_start, Memory::allocator_end);
	}

	Allocator::Allocator() :
		current_block(0),
		end_block(0)
	{
	}

	void Allocator::dump()
	{
		auto is_free = [&](Block *block) -> bool {
			Block *current = free_list.first;

			while(current)
			{
				if(current == block)
					return true;

				current = current->list_next;
			}

			return false;
		};

		Block *current = linear_list.first;

		console.color(Console::Strong).s("Virtual memory dump").color(Console::Default).endl();

		while(current)
		{
			auto block_free = is_free(current);

			console.s(is_free(current) ? "Free" : "Used").s(" @ ").x(current).s(" : ").x(current->base).s(" - ").x(current->base + current->pages).endl();

			assert(block_free == current->free);

			current = current->linear_next;
		}
	}

	Block *Allocator::allocate_block()
	{
		// Check if any free block is available

		Block *result = free_block_list.first;

		if(result)
		{
			free_block_list.remove(result, 0);

			return result;
		}

		// Do we have an available block in our block array?

		if((current_block + 1) < end_block)
			return current_block++;

		// Steal a page from the first free block and use it for a new block array

		Block *free = free_list.first;

		assert(free, "Out of virtual memory");
		assert(free->pages != 0, "Empty block found");

		--free->pages;
		VirtualPage *overhead = free->base++;

		// Mark this page as used

		Block *overhead_block = (Block *)overhead;

		current_block = overhead_block + 1;
		end_block = (Block *)(overhead + 1);

		assert(current_block < end_block, "Overflow");

		map(overhead);

		overhead_block->base = overhead;
		overhead_block->pages = 1;

		linear_list.insert_before(overhead_block, free);

		if(free->pages == 0) // The block we stole a page from is empty so we can reuse it
		{
			linear_list.remove(free);
			free_list.remove(free);
			return free;
		}

		return current_block++;
	}

	void Allocator::initialize(size_t start, size_t end)
	{
		first_block.base = (VirtualPage *)start;
		first_block.pages = (VirtualPage *)end - first_block.base;
		first_block.free = true;

		free_list.append(&first_block);
		linear_list.append(&first_block);
	}

	Block *Allocator::allocate(size_t pages)
	{
		Block *result = allocate_block(); // Allocate a result block first since it can modify free regions

		Block *current = free_list.first;

		while(current)
		{
			if(current->pages >= pages) // We have a winner
			{
				if(current->pages == pages) // It fits perfectly
				{
					free_block_list.append(result);
					free_list.remove(current);

					return current;
				}

				linear_list.insert_before(result, current);

				result->free = false;
				result->base = current->base;
				result->pages = pages;
				current->base += pages;
				current->pages -= pages;

				return result;
			}

			current = current->list_next;
		}

		panic("Out of virtual memory");
	}

	void Allocator::free(Block *block)
	{
		auto end = block->base + block->pages;

		for(auto page = block->base; page != end; ++page)
			Memory::unmap(page);

		Block *prev = block->linear_prev;
		Block *next = block->linear_next;
		Block *current = block;

		// Merge with a block below

		if(prev && prev->free)
		{
			assert(prev->base + prev->pages == current->base);
			prev->pages += current->pages;

			free_block_list.append(current);
			linear_list.remove(current);

			current = prev;
		}

		// Merge with a block above

		if(next && next->free)
		{
			assert(current->base + current->pages == next->base);

			next->base -= current->pages;
			next->pages += current->pages;

			if(current == prev)
				free_list.remove(current);

			free_block_list.append(current);
			linear_list.remove(current);

			current = next;
		}

		if(current == block)
		{
			current->free = true;
			free_list.append(current);
		}
	}

	Block *allocate_pages(size_t pages)
	{
		return allocator.allocate(pages);
	}

	void free_pages(Block *block)
	{
		return allocator.free(block);
	}
};
