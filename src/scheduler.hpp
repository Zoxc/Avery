#pragma once

class Thread;

namespace Scheduler
{
	void block(Thread *thread);
	void queue(Thread *thread);
	void schedule();
};
