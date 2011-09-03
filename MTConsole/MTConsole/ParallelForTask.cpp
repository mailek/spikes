#include "StdAfx.h"
#include "ParallelForTask.h"

/* DEBUG TODO: remove */
#include <cstdlib>
#include <ctime>
#include <windows.h>
/* END DEBUG */

using namespace TaskScheduler;

CParallelForTask::CParallelForTask() : CAbstractTask()
{
}

CParallelForTask::CParallelForTask(CompletionToken* token) : CAbstractTask(token)
{
#ifdef _TEST
	srand((unsigned int)time(0));
#endif
}

CParallelForTask::~CParallelForTask(void)
{
}

void CParallelForTask::RunTask()
{
	/* default empty implementation */
	my_printf("Running for loop: start %i  end %i\n", m_range.start, m_range.end);

#ifdef _TEST
	int num = 0;
	float range_mod = (float)rand() / (float)RAND_MAX;
	range_mod *= 50;

	for(long i = 0; i < (long)range_mod; i++)
	{
		int rands = rand();
		num += rands;
		Sleep(5);
	}

	Sleep(10);

#endif
}

bool CParallelForTask::Split(CAbstractTask **newTask)
{
	int currRangeSize = m_range.end-m_range.start+1;

	if(currRangeSize <= m_range.stepSize)
	{
		return false;
	}

	int newTaskRangeSize = (currRangeSize / 2);
	int oldTaskRangeSize = currRangeSize - newTaskRangeSize;

	CompletionToken taskComplete;
	CParallelForTask *task = new CParallelForTask(&taskComplete);
	
	task->m_range.start = m_range.start+oldTaskRangeSize;
	task->m_range.end = task->m_range.start+newTaskRangeSize-1;
	task->m_range.stepSize = m_range.stepSize;
	
	m_range.end = m_range.start + oldTaskRangeSize-1;

	*newTask = task;

	return true;
}

/* Get a single iteration chunk from this task */
CStackMemoryAllocator::MemMarker CParallelForTask::ChipTask(CStackMemoryAllocator &allocator, CAbstractTask **newTask)
{
	*newTask = NULL;
	CStackMemoryAllocator::MemMarker marker = 0;
	
	if(m_range.end - m_range.start > m_range.stepSize)
	{
		CParallelForTask* ptr = NULL;
		marker = allocator.Malloc<CParallelForTask>(&ptr);

		ptr->m_range.start = m_range.start;
		ptr->m_range.end = ptr->m_range.start + m_range.stepSize;

		m_range.start += m_range.stepSize;
		*newTask = ptr;
	}

	return marker;
}

void CParallelForTask::DestroyChip(CStackMemoryAllocator &allocator, CStackMemoryAllocator::MemMarker marker)
{
	this->~CParallelForTask();
	allocator.Free(marker, sizeof(this));
}