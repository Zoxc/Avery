#include "thread.hpp"
#include "process.hpp"

namespace Scheduler
{
	static LockedObject<ThreadList> thread_queue;

	void block(Thread *thread)
	{
		thread_queue.access([&](ThreadList &list) {
			list.remove(thread);
		});
	}

	void queue(Thread *thread)
	{
		thread_queue.access([&](ThreadList &list) {
			list.append(thread);
		});
	}

	void schedule()
	{
		thread_queue.access([&](ThreadList &list) {
			auto result = list.first;

			if(result)
				list.remove(result);

			CPU::current->scheduled_thread = result;
		});
	}
};
