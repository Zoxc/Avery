#pragma once
#include "arch.hpp"

class ConsoleBackend;

namespace Params
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
		addr_t base;
		addr_t end;
		struct MemoryRange *next;
	};

	enum SegmentType
	{
		SegmentCode,
		SegmentReadOnlyData,
		SegmentData,
		SegmentModule
	};

	struct Segment
	{
		size_t type;
		addr_t base;
		addr_t end;
		ptr_t virtual_base;
		size_t found;
		char name[0x100];
	};

	const size_t memory_range_max = 0x100;
	const size_t segment_max = 0x10;

	struct Info
	{
		size_t range_count;
		size_t segment_count;
		struct MemoryRange ranges[memory_range_max];
		struct Segment segments[segment_max];
	};

	extern Info info;
};
