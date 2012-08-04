#pragma once
#include "../common.hpp"

template<class T, T *T::*next = &T::next> class FastList
{
public:
	FastList() : first(0) {}

	T *first;

	bool empty()
	{
		return first == 0;
	}

	void remove(T *node, T *prev)
	{
		assert(node != 0);

		if(prev != 0)
			prev->*next = node->*next;
		else
			first = node->*next;
	}

	void append(T *node)
	{
		assert(node != 0);

		node->*next = first;
		first = node;
	}
};
