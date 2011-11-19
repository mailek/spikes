#pragma once
#include "abstracttask.h"

namespace TaskScheduler
{

template <typename T>
class CParallelForTask : public CAbstractTask
{
public:
	CParallelForTask() : CAbstractTask(), m_forTask(NULL) {}
	CParallelForTask(T* task, CompletionToken* token) : CAbstractTask(token), m_forTask(task) {}
	~CParallelForTask(void)	{}

public:
	Range		m_range;
	T*			m_forTask;

private:
	CParallelForTask(const CParallelForTask& t);
	CParallelForTask& operator=( const CParallelForTask& t);

public:
	virtual void RunTask()
	{
		m_forTask->RunRange(m_range.start, m_range.end);
	}

	virtual bool Split(CAbstractTask **newTask)
	{
		*newTask = NULL;

		int currRangeSize = m_range.end-m_range.start+1;

		if(currRangeSize < m_range.stepSize*2)
		{
			return false;
		}

		int newTaskRangeSize = (currRangeSize / 2);
		int oldTaskRangeSize = currRangeSize - newTaskRangeSize;

		CParallelForTask *task = new CParallelForTask(m_forTask, m_complete);
		
		task->m_range.start = m_range.start+oldTaskRangeSize;
		task->m_range.end = m_range.end;
		task->m_range.stepSize = m_range.stepSize;
		
		m_range.end = m_range.start + oldTaskRangeSize-1;

		*newTask = task;

		return true;
	}

	/* Get a single iteration chunk from this task */
	virtual CStackMemoryAllocator::MemMarker ChipTask(CStackMemoryAllocator *allocator, CAbstractTask **newTask)
	{
		*newTask = NULL;
		CStackMemoryAllocator::MemMarker marker = 0;
		
		if(m_range.end - m_range.start > m_range.stepSize)
		{
			CParallelForTask* ptr = NULL;
			marker = allocator->Malloc<CParallelForTask>(&ptr);

			ptr->m_forTask = m_forTask;
			ptr->m_range.start = m_range.start;
			ptr->m_range.end = ptr->m_range.start + m_range.stepSize-1;

			m_range.start += m_range.stepSize;
			*newTask = ptr;
		}

		return marker;
	}

	virtual void DestroyChip(CStackMemoryAllocator &allocator, CStackMemoryAllocator::MemMarker marker)
	{
		this->~CParallelForTask();
		allocator.Free(marker, sizeof(*this));
	}

};

template <typename T>
void ParallelForTask(T* task, Range range, CTaskScheduler *scheduler)
{
	CompletionToken token;
	
	CParallelForTask<T> *pt = new CParallelForTask<T>(task, &token);
	pt->m_range.start = range.start;
	pt->m_range.end = range.end;
	pt->m_range.stepSize = range.stepSize;
	pt->m_isMasterTask = true;
	
	scheduler->AddTaskToWorkQueue(pt);
	scheduler->WorkUntilTaskComplete(&token);

	assert(pt->IsComplete());

	delete pt;
}

template <typename T>
void ParallelForAsyncTask(T* task, Range range, CTaskScheduler *scheduler, CompletionToken *token)
{
	assert(token != NULL);

	CParallelForTask<T> *pt = new CParallelForTask<T>(task, token);
	token->m_masterTask = (void*)pt;

	pt->m_range.start = range.start;
	pt->m_range.end = range.end;
	pt->m_range.stepSize = range.stepSize;
	pt->m_isMasterTask = true;
	
	scheduler->AddTaskToWorkQueue(pt);
}

template <typename T>
bool ParallelForAsyncComplete(T* task, CompletionToken *token)
{
	bool ret = false;
	if(token->IsComplete())
	{
		CParallelForTask<T> *pt = (CParallelForTask<T>*)token->m_masterTask;
		delete pt;
		ret = true;
	}

	return ret;

}

} /* namespace TaskScheduler */