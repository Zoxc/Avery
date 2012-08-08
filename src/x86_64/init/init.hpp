#include "../arch.hpp"

class Process;

namespace Init
{
	void load_module(Process *process, const void *obj, size_t obj_size);
};
