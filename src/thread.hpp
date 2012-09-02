#pragma once
#include "util/linked-list.hpp"
#include "arch/common.hpp"
#include "lock.hpp"

class Process;

namespace User
{
	struct Block;
};

class ThreadList;

class Thread
{
public:
	debug(ThreadList *thread_list);

	Process *owner;
	Arch::Registers registers;
	User::Block *stack;

	Thread(Process *process);

	Thread *execution_next;
	Thread *execution_prev;

	typedef LinkedList<Thread, &Thread::execution_next, &Thread::execution_prev> List;
};

class ThreadList:
	public Thread::List
{
public:
	void append(Thread *thread)
	{
		assert(thread->thread_list == nil, "Thread already belong to a list");

		debug(thread->thread_list = this);

		Thread::List::append(thread);
	}

	void remove(Thread *thread)
	{
		assert(thread->thread_list == this, "Thread doesn't belong to this list");

		debug(thread->thread_list = nil);

		Thread::List::remove(thread);
	}
};
