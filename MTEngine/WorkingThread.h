#pragma once

#include "abstracttask.h"

class CTaskScheduler;
class CWorkingThread;

#define MAX_NUM_TASKS	10

class CWorkingThread
{
public:
    CWorkingThread(void);
    ~CWorkingThread(void);

	/* new thread entry */
	static void* Proc(void *args);
	void _Proc();
	bool AddTaskToTopOfWorkPile(CAbstractTask* newTask);
	void WorkUntilBufferEmpty();
	void GrabTaskFromTopOfWorkPile(CAbstractTask** poppedTask);
	int CanGiveUpWork(CAbstractTask **out);
	void StealWorkFromOthers();

	pthread_t					m_pthread;
	pthread_mutex_t				m_taskPoolMutex;

	CAbstractTask*				m_tasks[MAX_NUM_TASKS];
	int							m_numTasks;
	int							m_threadId;

public:
	CTaskScheduler			   *m_scheduler;
};
