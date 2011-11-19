#pragma once

#include "stackmemoryallocator.h"

namespace TaskScheduler
{

class CTaskScheduler;

class Range
{
public:
    Range() : start(0), end(0), stepSize(1) {};
	Range(int s, int e) : start(s), end(e), stepSize(1) {};
	Range(int s, int e, int d) : start(s), end(e), stepSize(d) {};

public:
    int start;
    int end;
    int stepSize;

};

class CompletionToken
{
	template <typename T>
	friend void ParallelForAsyncTask(T* task, Range range, CTaskScheduler *scheduler, CompletionToken *token);
	template <typename T>
	friend bool ParallelForAsyncComplete(T* task, CompletionToken *token);
public:
	CompletionToken() : m_numTasks(0), m_numTasksComplete(0), m_masterTask(NULL) { m_lock = PTHREAD_MUTEX_INITIALIZER; }
	~CompletionToken() { pthread_mutex_destroy(&m_lock); }

	void IncrementTaskCount()
	{
		VERIFY_LOCK(pthread_mutex_lock(&m_lock));
		m_numTasks++;
		pthread_mutex_unlock(&m_lock);
	}

	void IncrementTaskProgress()
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
	void*				m_masterTask;			/* master task pointer */

	CompletionToken(const CompletionToken& c) {};
};

class CAbstractTask
{
	friend class CTaskScheduler;

public:
	CAbstractTask(CompletionToken* token) : m_complete(NULL), m_isMasterTask(false) {m_complete=token;}
	CAbstractTask() : m_complete(NULL) {};
	virtual ~CAbstractTask() {};

	void IncrementTaskCount() { if(m_complete) m_complete->IncrementTaskCount(); }
	void IncrementTaskProgress() { if(m_complete) m_complete->IncrementTaskProgress(); }
	bool IsComplete() { return (m_complete && m_complete->IsComplete()); }
	void Destroy() { if(!m_isMasterTask) delete this; }

	virtual void RunTask() =0;
	virtual bool Split(CAbstractTask **newTask) =0;
	virtual CStackMemoryAllocator::MemMarker ChipTask(CStackMemoryAllocator *allocator, CAbstractTask **newTask) =0;
	virtual void DestroyChip(CStackMemoryAllocator &allocator, CStackMemoryAllocator::MemMarker marker) =0;

	bool m_isMasterTask;

protected:
	CompletionToken *m_complete;

};

}