#include "init.hpp"
#include "../arch/common.hpp"
#include "../lib.hpp"
#include "../console.hpp"
#include "../params.hpp"
#include "../process.hpp"
#include "../thread.hpp"
#include "../scheduler.hpp"

static ptr_t load_module(Process *process, addr_t base, addr_t end)
{
	Memory::ScopedBlock obj_block;

	size_t obj_size = end - base;

	const void *obj = obj_block.map_block(base, obj_size);

	return Init::load_module(process, obj, obj_size);
}

static void start_process(const char *name, addr_t base, addr_t end)
{
	console.s("Loading process ").s(name).endl();

	Process *process = new Process;

	ptr_t entry = load_module(process, base, end);

	Thread *thread = new Thread(process);

	thread->stack = process->allocator.allocate(User::Block::Stack, 10);

	Memory::map(thread->stack->base + 1, thread->stack->pages - 1, Memory::rw_data_flags | Memory::usermode_bit, &process->address_space);

	thread->registers.set_ip(entry);
	thread->registers.set_stack((ptr_t)(thread->stack->base + thread->stack->pages));

	Scheduler::queue(thread);
}

void Init::load_modules()
{
	bool found_userland = false;

	for(size_t i = 0; i < Params::info.segment_count; ++i)
	{
		Params::Segment &segment = Params::info.segments[i];

		if(segment.type == Params::SegmentModule && strncmp(segment.name, "user", sizeof(segment.name)) == 0)
		{
			start_process(segment.name, segment.base, segment.end);

			found_userland = true;
		}
	}

	if(!found_userland)
		panic("Didn't find the userland module");
}
