#pragma once

#include "stackmemoryallocator.h"

namespace TaskScheduler
{

class Range
{
public:
    Range() : start(0), end(0), stepSize(0) {};

public:
    int start;
    int end;
    int stepSize;

};

class CompletionToken
{
public:
	CompletionToken() : m_numTasks(0), m_numTasksComplete(0) { m_lock = PTHREAD_MUTEX_INITIALIZER; }
	~CompletionToken() { pthread_mutex_destroy(&m_lock); }

	void IncrementTaskCount()
	{
		VERIFY_LOCK(pthread_mutex_lock(&m_lock));
		m_numTasks++;
		pthread_mutex_unlock(&m_lock);
	}

	void CompleteTask()
	{
		VERIFY_LOCK(pthread_mutex_lock(&m_lock));
		m_numTasksComplete++;
		pthread_mutex_unlock(&m_lock);
	}

	bool IsComplete() 
	{		
		VERIFY_LOCK(pthread_mutex_lock(&m_lock));
		bool complete = (m_numTasksComplete == m_numTasks);
		pthread_mutex_unlock(&m_lock);

		return complete;
	}

private:
	int					m_numTasks;
	int					m_numTasksComplete;
	pthread_mutex_t     m_lock;					/* token state lock */
};

class CAbstractTask
{
public:
	CAbstractTask(CompletionToken* token) : m_complete(NULL) {m_complete=token;}
	CAbstractTask() : m_complete(NULL) {};
	virtual ~CAbstractTask() {};

	void IncrementTaskCount() { if(m_complete) m_complete->IncrementTaskCount(); }
	void CompleteTask() { if(m_complete) m_complete->CompleteTask(); }

	virtual void RunTask() =0;
	virtual bool Split(CAbstractTask **newTask) =0;
	virtual CStackMemoryAllocator::MemMarker ChipTask(CStackMemoryAllocator &allocator, CAbstractTask **newTask) =0;
	virtual void DestroyChip(CStackMemoryAllocator &allocator, CStackMemoryAllocator::MemMarker marker) =0;

private:
	CompletionToken *m_complete;

};

}