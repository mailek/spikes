#pragma once
#include <new>

#define MEM_ALLOCATOR_STACK_SZ    (100)

class CStackMemoryAllocator
{
public:
	CStackMemoryAllocator(void){Clear();}
	~CStackMemoryAllocator(void){};

	typedef void* MemMarker;

public:
	template <typename T>
	inline MemMarker Malloc(T **out)
	{
		assert(out);
		MemMarker old_top = (MemMarker)&m_stackTop;
		*out = ::new(m_stackTop) T;
		m_stackTop = (size_t*)m_stackTop + sizeof(T);

		return old_top;
	}

	inline void Free(MemMarker marker, size_t size)
	{
		assert( marker == (size_t*)m_stackTop - size);
		m_stackTop = marker;
	}

	void Clear();

private:
	MemMarker		m_stackTop;
	size_t			m_stack[MEM_ALLOCATOR_STACK_SZ];
};
