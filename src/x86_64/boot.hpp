#include "arch.hpp"

namespace Boot
{
	enum MemoryAreaType
	{
		MemoryAreaNone,
		MemoryAreaBootMemory,
		MemoryAreaKernelCode,
		MemoryAreaKernelData,
		MemoryAreaUsable,
		MemoryAreaACPI
	};

	struct MemoryArea
	{
		size_t type;
		void *start;
		size_t size;
		void *user;
	};
	
	const size_t memory_area_max = 0x100;

	struct Parameters
	{
		void *frame_buffer;
		size_t frame_buffer_size;
		size_t frame_buffer_width;
		size_t frame_buffer_height;
		size_t frame_buffer_scanline;
		size_t area_count;
		struct MemoryArea areas[memory_area_max];
	};
	
	extern Parameters *parameters;
};
