#pragma once
#include "abstracttask.h"

namespace TaskScheduler
{

class CParallelForTask : public CAbstractTask
{
public:
    CParallelForTask(CompletionToken *token);
	CParallelForTask();
    ~CParallelForTask(void);

public:
	virtual void RunTask();
    virtual bool Split(CAbstractTask **newTask);
	virtual CStackMemoryAllocator::MemMarker ChipTask(CStackMemoryAllocator &allocator, CAbstractTask **newTask);
	virtual void DestroyChip(CStackMemoryAllocator &allocator, CStackMemoryAllocator::MemMarker marker);

	Range		m_range;
};

}