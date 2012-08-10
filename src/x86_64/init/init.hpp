#include "../arch.hpp"

class Process;
class Thread;

namespace Init
{
	ptr_t load_module(Process *process, const void *obj, size_t obj_size);
	void enter_usermode(Thread *thread);
};
