#include "user-memory.hpp"

namespace User
{
	Allocator::Allocator(Memory::VirtualPage *start, Memory::VirtualPage *end) :
		current_block(0),
		end_block(0)
	{
		Block *first = allocate_block();

		first->base = start;
		first->pages = end - start;
		first->type = Block::Free;

		free_list.append(first);
		linear_list.append(first);
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

			assert(block_free == (current->type == Block::Free));

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

		Memory::Block *overhead = Memory::allocate_block(Memory::Block::UserAllocator);

		Memory::map(overhead->base, 1);

		overhead_list.append(overhead);

		Block *first_block = (Block *)overhead->base;

		current_block = first_block + 1;
		end_block = (Block *)(overhead->base + 1);

		return first_block;
	}

	Block *Allocator::allocate_at(Memory::VirtualPage *address, Block::Type type, size_t pages)
	{
		assert(pages > 0, "Can't allocate zero pages");

		Block *current = free_list.first;

		auto result_end = address + pages;

		while(current)
		{
			auto current_end = current->base + current->pages;

			if(current->base > address || current_end <= address)
			{
				current = current->list_next;
				continue;
			}

			if(current_end < result_end)
				return 0;

			Block *result;

			if(current->base != address)
			{
				current->pages = address - current->base;

				result = allocate_block();

				result->type = type;
				result->base = address;
				result->pages = pages;

				linear_list.insert_after(result, current);

				if(current_end != result_end)
				{
					Block *top = allocate_block();

					top->type = Block::Free;
					top->base = result_end;
					top->pages = current_end - result_end;

					linear_list.insert_after(top, result);
					free_list.append(top);
				}

				return result;
			}
			else
			{
				if(current->pages == pages) // It fits perfectly
				{
					current->type = type;

					free_list.remove(current);

					return current;
				}

				result = allocate_block();

				result->type = type;
				result->base = address;
				result->pages = pages;

				linear_list.insert_after(result, current);

				current->base += pages;
				current->pages -= pages;
			}

			return result;
		}

		return 0;
	}

	Block *Allocator::allocate(Block::Type type, size_t pages)
	{
		assert(pages > 0, "Can't allocate zero pages");

		Block *current = free_list.first;

		while(current)
		{
			if(current->pages >= pages) // We have a winner
			{
				if(current->pages == pages) // It fits perfectly
				{
					free_list.remove(current);

					current->type = type;

					return current;
				}

				Block *result = allocate_block();

				linear_list.insert_before(result, current);

				result->type = type;
				result->base = current->base;
				result->pages = pages;
				current->base += pages;
				current->pages -= pages;

				return result;
			}

			current = current->list_next;
		}

		return 0;
	}

	void Allocator::free(Block *block)
	{
		assert(block, "Invalid block");

		Memory::unmap(block->base, block->pages);

		Block *prev = block->linear_prev;
		Block *next = block->linear_next;
		Block *current = block;

		// Merge with a block below

		if(prev && prev->type == Block::Free)
		{
			assert(prev->base + prev->pages == current->base);
			prev->pages += current->pages;

			free_block_list.append(current);
			linear_list.remove(current);

			current = prev;
		}

		// Merge with a block above

		if(next && next->type == Block::Free)
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
			current->type = Block::Free;
			free_list.append(current);
		}
	}
};
