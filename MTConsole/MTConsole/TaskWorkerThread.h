#pragma once

#include "abstracttask.h"
#include "stackmemoryallocator.h"

namespace TaskScheduler
{

class CTaskScheduler;

static const int MAX_NUM_THREAD_TASKS = 10;				/* max number of unworked tasks for a thread's pile */

class CTaskWorkerThread
{
	friend class CTaskScheduler;

public:
    CTaskWorkerThread(void);
    ~CTaskWorkerThread(void);

	void _Proc();
	bool AddTaskToTopOfWorkPile(CAbstractTask* newTask);
	void WorkUntilBufferEmpty();
	int CanGiveUpWork(CAbstractTask **out);
	void Init();

	/* new thread entry */
	static void* Proc(void *args);

private:
	void StealWorkFromOthers();
	void GrabTaskFromTopOfWorkPile(CAbstractTask** poppedTask);

	CTaskScheduler	   	   *m_scheduler;			/* reference to thread manager */
	CAbstractTask      	   *m_tasks[MAX_NUM_THREAD_TASKS]; /* task pile */
	int						m_numTasks;				/* number of unworked tasks in pile */
	int						m_threadId;				/* unique id for this thread */
	bool					m_running;				/* thread terminate flag */
	bool					m_working;				/* thread is actively working on tasks */
	CStackMemoryAllocator	m_scratchMem;			/* temporary task scratch memory */

	pthread_t				m_pthread;				/* thread handle */
	pthread_mutex_t			m_taskPoolMutex;		/* lock for task pile */

	
};

}