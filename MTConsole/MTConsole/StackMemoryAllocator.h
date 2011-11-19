#pragma once
#include <new>

#define MEM_ALLOCATOR_STACK_SZ    (100)

class CStackMemoryAllocator
{
	friend class StackMemoryAllocatorTest;

public:
	CStackMemoryAllocator(void) { Clear(); }
	~CStackMemoryAllocator(void) {};

	typedef char MemoryChunk, *MemMarker;

public:
	template <typename T>
	inline MemMarker Malloc(T **out)
	{
		assert(out);
		MemMarker old_top = m_stackTop;
		*out = ::new(m_stackTop) T;

		size_t chunks = (sizeof(T)-1)/sizeof(MemoryChunk)+1;

		m_stackTop += chunks;

		return old_top;
	}

	inline void Free(MemMarker marker, size_t size)
	{
		size_t chunks = (size-1)/sizeof(MemoryChunk)+1;

		assert( marker == m_stackTop - chunks);
		m_stackTop = marker;
		assert(m_stackTop);
	}

	void Clear();

private:
	MemMarker		m_stackTop;
	MemoryChunk		m_stack[MEM_ALLOCATOR_STACK_SZ];
};
