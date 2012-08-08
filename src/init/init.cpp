#include "../arch.hpp"
#include "../lib.hpp"
#include "../console.hpp"
#include "../params.hpp"
#include "../process.hpp"
#include "init.hpp"

static void start_process(const char *name, addr_t base, addr_t end)
{
	Memory::ScopedBlock obj_block;
	size_t obj_size = end - base;
	const void *obj = obj_block.map_block(base, obj_size);

	Process *process = new Process;

	console.s("Loading process ").s(name).endl();

	Init::load_module(process, obj, obj_size);
}

void Init::load_modules()
{
	for(size_t i = 0; i < Params::info.segment_count; ++i)
	{
		Params::Segment &segment = Params::info.segments[i];

		if(segment.type == Params::SegmentModule && strncmp(segment.name, "user", sizeof(segment.name)) == 0)
			start_process(segment.name, segment.base, segment.end);
	}
}
