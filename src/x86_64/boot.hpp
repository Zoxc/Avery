#pragma once
#include "arch.hpp"

namespace Boot
{
	enum MemoryType
	{
		MemoryNone,
		MemoryUsable,
		MemoryACPI
	};

	struct MemoryRange
	{
		size_t type;
		size_t base;
		size_t end;
		struct MemoryRange *next;
	};

	enum SegmentType
	{
		SegmentCode,
		SegmentReadOnlyData,
		SegmentData
	};

	struct Segment
	{
		size_t type;
		size_t base;
		size_t end;
		size_t virtual_base;
		size_t found;
	};

	const size_t memory_range_max = 0x100;
	const size_t segment_max = 0x10;

	struct Parameters
	{
		size_t size;
		void *frame_buffer;
		size_t frame_buffer_size;
		size_t frame_buffer_width;
		size_t frame_buffer_height;
		size_t frame_buffer_scanline;
		size_t range_count;
		size_t segment_count;
		struct MemoryRange ranges[memory_range_max];
		struct Segment segments[segment_max];
	};
	
	extern Parameters parameters;
};
