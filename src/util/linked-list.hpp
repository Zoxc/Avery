#pragma once
#include "../common.hpp"

template<class T, T *T::*next = &T::next, T *T::*prev = &T::prev> class LinkedList
{
public:
	LinkedList() : first(0), last(0) {}

	T *first;
	T *last;

	bool empty()
	{
		return first == 0;
	}

	void remove(T *node)
	{
		assert(node != 0);

		if(node->*prev != 0)
			(node->*prev)->*next = node->*next;
		else
			first = node->*next;

		if(node->*next != 0)
			(node->*next)->*prev = node->*prev;
		else
			last = node->*prev;
	}

	template<T *T::*a, T *T::*b, T *LinkedList::*abs> void insert_position(T *node, T *target)
	{
		assert(node != 0);
		assert(target != 0);

		node->*a = target;

		T *target_attr = target->*b;

		if(target_attr)
			target_attr->*a = node;
		else
			this->*abs = node;

		node->*b = target_attr;
		target->*b = node;
	}

	void insert_before(T *node, T *before)
	{
		insert_position<next, prev, &LinkedList::first>(node, before);
	}

	void insert_after(T *node, T *after)
	{
		insert_position<prev, next, &LinkedList::last>(node, after);
	}

	void append(T *node)
	{
		assert(node != 0);

		node->*next = 0;

		if(last != 0)
		{
			node->*prev = last;
			last->*next = node;
			last = node;
		}
		else
		{
			node->*prev = 0;

			first = node;
			last = node;
		}
	}
};
