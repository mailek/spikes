#include "StdAfx.h"
#include "StackMemoryAllocator.h"

void CStackMemoryAllocator::Clear()
{
	memset(m_stack, 0, sizeof(m_stack));
	m_stackTop = (MemMarker)&m_stack;
}
