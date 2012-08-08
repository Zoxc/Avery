#pragma once
#include "allocator.hpp"

template<class T, class A = StandardAllocator> class Vector
{
	protected:
		T *table;
		size_t _size;
		size_t _capacity;

		template<class Aother> void initialize_copy(const Vector<T, Aother>& other)
		{
			_size = other.size();
			_capacity = other.capacity();
			
			if(_size)
			{
				table = (T *)A::allocate(_capacity);

				memcpy((void *)raw(), (void *)other.raw(), sizeof(T) * _size);
			}
			else
			{
				table = nullptr;
			}
		}

	public:
		Vector(size_t initial)
		{
			_size = 0;
			_capacity = 1 << initial;
			
			table = (T *)A::allocate(_capacity);
		}

		Vector()
		{
			_size = 0;
			_capacity = 0;
			table = nullptr;
		}
		
		Vector(Vector &&vector) :
			table(vector.table),
			_size(vector._size),
			_capacity(vector._capacity)
		{
		}
		
		Vector(const Vector &vector)
		{
			initialize_copy(vector);
		}
		
		template<class Aother> Vector(const Vector<T, Aother>& other)
		{
			initialize_copy(other);
		}
		
		~Vector()
		{
			if(table)
				A::free(table);
		}
		
		Vector &operator=(const Vector& other)
		{
			if(this == &other)
				return *this;
			
			if(table)
				A::free(table);
			
			initialize_copy(other);
			
			return *this;
		}
		
		template<class Aother> Vector &operator=(const Vector<T, Aother>& other)
		{
			if(this == &other)
				return *this;
			
			if(table)
				A::free(table);
			
			initialize_copy(other);
			
			return *this;
		}
		
		void expand(size_t num)
		{
			if(_size + num > _capacity)
			{
				if(table)
				{
					assert(_size > 0);

					do
					{
						_capacity <<= 1;
					}
					while(_size + num > _capacity);

					table = (T *)A::reallocate(table, _size, _capacity);
				}
				else
				{
					_capacity = 1;

					while(_size + num > _capacity)
						_capacity <<= 1;
					
					table = (T *)A::allocate(_capacity);
				}
			}
		}

		size_t size() const
		{
			return _size;
		}
		
		size_t capacity() const
		{
			return _capacity;
		}
		
		const T &first() const
		{
			assert(_size > 0);
			
			return table[0];
		}
		
		T &first()
		{
			assert(_size > 0);
			
			return table[0];
		}
		
		T *raw() const
		{
			return &table[0];
		}
		
		T &last()
		{
			assert(_size > 0);
			
			return &table[_size - 1];
		}
		
		T &operator [](size_t index)
		{
			assert(index < _size);
			
			return table[index];
		}
		
		const T &operator [](size_t index) const
		{
			assert(index < _size);
			
			return table[index];
		}
		
		void clear()
		{
			_size = 0;
			_capacity = 0;
			
			if(table)
			{
				A::free(table);
				table = nullptr;
			}
		}
		
		bool expand_to(size_t size, T filler)
		{
			if(_size < size)
			{
				expand(size - _size);
				
				for(size_t i = _size; i < size - 1; ++i)
					table[i] = filler;
					
				_size = size;
				
				return true;
			}
			else
				return false;
		}
		
		T shift()
		{
			T result = first();
			
			for(size_t i = 0; i < _size - 1; ++i)
				table[i] = table[i + 1];
			
			_size -= 1;

			return result;
		}
		
		void remove(size_t index)
		{
			assert(index < _size);
			
			_size -= 1;

			for(size_t i = index; i < _size; ++i)
				table[i] = table[i + 1];
		}
		
		void insert(T entry, size_t index)
		{
			assert(index < _size);

			expand(1);

			for(size_t i = _size; i-- > index;)
				table[i + 1] = table[i];

			table[index] = entry;

			_size++;
		}

		void push_entries_front(T *entries, size_t count)
		{
			expand(count);
			
			for(size_t i = _size; i-- > 0;)
				table[i + count] = table[i];

			for(size_t i = 0; i < count; ++i)
				table[i] = entries[i];
				
			_size += count;
		}
		
		void push_entries(T *entries, size_t count)
		{
			expand(count);

			for(size_t i = 0; i < count; ++i)
				table[_size + i] = entries[i];
			
			_size += count;
		}
		
		template<class Aother> void push(const Vector<T, Aother>& other)
		{
			size_t count = other.size();
			
			expand(count);
			
			for(size_t i = 0; i < count; ++i)
				table[_size + i] = other[i];
			
			_size += count;
		}
		
		void push(T entry)
		{
			expand(1);

			table[_size++] = entry;
		}
		
		T pop()
		{
			assert(_size);
			
			size_t new_size = _size - 1;
			
			T result = table[new_size];
			
			if(new_size == 0)
			{
				_capacity = 0;
				A::free(table);
				table = nullptr;
			}
				
			_size = new_size;
			
			return result;
		}
		
		size_t index_of(T entry)
		{
			auto result = find(entry);

			if(!result)
				return (size_t)-1;

			return (size_t)(result - raw());
		}
		
		template<typename F> bool each(F func)
		{
			for(size_t i = 0; i < _size; ++i)
			{
				if(!func(table[i]))
					return false;
			}
			
			return true;
		}
		
		template<typename F> T find(F func, T default_value)
		{
			for(size_t i = 0; i < _size; ++i)
			{
				if(func(table[i]))
					return table[i];
			}
			
			return default_value;
		}
		
		T *find(T entry)
		{
			for(auto i = begin(); i != end(); ++i)
			{
				if(*i == entry)
					return i.position();
			}
			
			return 0;
		}
		
		class Iterator
		{
		private:
			T *current;

		public:
			Iterator(T *start) : current(start) {}
			
			bool operator ==(const Iterator &other) const 
			{
				return current == other.current;
			}
			
			bool operator !=(const Iterator &other) const 
			{
				return current != other.current;
			}
		
			T *position() const 
			{
				return current;
			}
			
			T &operator ++()
			{
				return *++current;
			}
			
			T &operator ++(int)
			{
				return *current++;
			}
			
			T &operator*() const 
			{
				return *current;
			}
			
			T &operator ()() const 
			{
				return *current;
			}
		};
		
		Iterator begin()
		{
			return Iterator(raw());
		}

		Iterator end()
		{
			return Iterator(&table[_size]);
		}
		
		class ReverseIterator
		{
		private:
			T *current;

		public:
			ReverseIterator(T *start) : current(start) {}
			
			bool operator ==(const ReverseIterator &other) const 
			{
				return current == other.current;
			}
			
			bool operator !=(const ReverseIterator &other) const 
			{
				return current != other.current;
			}
		
			T *position() const 
			{
				return current;
			}
			
			T &operator ++()
			{
				return *--current;
			}
			
			T &operator ++(int)
			{
				return *current--;
			}
			
			T &operator*() const 
			{
				return *current;
			}
			
			T &operator ()() const 
			{
				return *current;
			}
		};
		
		ReverseIterator rbegin()
		{
			return ReverseIterator(raw() + _size - 1);
		}

		ReverseIterator rend()
		{
			return ReverseIterator(raw() - 1);
		}

		size_t index_of(Iterator &iter)
		{
			return (size_t)(iter.position() - raw()) / sizeof(T);
		}
};
