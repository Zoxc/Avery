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

	void insert_before(T *node, T *before)
	{
		assert(node != 0);
		assert(before != 0);

		node->*next = before;

		T *prev_before = before->*prev;

		if(prev_before)
			prev_before->*next = node;
		else
			first = node;

		node->*prev = prev_before;
		before->*prev = node;
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
			first = node;
			last = node;
		}
	}
};
