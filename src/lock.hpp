#pragma once
#include "arch/lock.hpp"

struct Lock
{
	Arch::Lock lock;

	void enter()
	{
		lock.enter();
	}

	void leave()
	{
		lock.leave();
	}

	template<typename F> void sync(F func)
	{
		enter();

		func();

		leave();
	}
};

template<class T> class LockedObject
{
private:
	Lock lock;
	T obj;

public:
	template<typename F> void access(F func)
	{
		lock.enter();

		func(obj);

		lock.leave();
	}
};

class ScopedLock
{
private:
	Lock *lock;

public:
	ScopedLock(Lock *lock) : lock(lock)
	{
		lock->enter();
	}

	~ScopedLock()
	{
		lock->leave();
	}
};
