#include "StdAfx.h"
#include "TaskWorkerThread.h"
#include "TaskScheduler.h"

using namespace TaskScheduler;

CTaskWorkerThread::CTaskWorkerThread(void) :	m_numTasks(0),
												m_threadId(0),
												m_running(true),
												m_scheduler(NULL)
{
	memset(&m_tasks, 0, sizeof(m_tasks));
}

CTaskWorkerThread::~CTaskWorkerThread(void)
{
	/* only destroy threads that have a valid thread id */
	if(m_threadId != 0)
	{
		pthread_mutex_destroy(&m_taskPoolMutex);
	}
}

/* Delay resource construction until thread is spawned */
void CTaskWorkerThread::Init()
{
	m_scratchMem.Clear();

#ifndef _DEBUG
	m_taskPoolMutex = PTHREAD_MUTEX_INITIALIZER;
#else
	/*pthread_mutexattr_t a;

	pthread_mutexattr_init(&a);
	pthread_mutexattr_settype(&a, PTHREAD_MUTEX_ERRORCHECK);
	
	pthread_mutex_init(&m_taskPoolMutex, &a);
	pthread_mutexattr_destroy(&a);*/

	m_taskPoolMutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;
#endif
}

/* Local thread instance entry point */
void CTaskWorkerThread::_Proc()
{
	/* set up the thread resources */
	Init();

    unsigned int id = (unsigned int)this;
	debug_print("\tThread %i: Is Alive! - PID: %i\n", m_threadId, (int)this);
	
    while(true)
    {
		VERIFY_LOCK(pthread_mutex_lock(&m_scheduler->m_wakeUpMutex));

        debug_print("\tThread %i: Going to Sleep!\n", m_threadId);
        sem_post(&m_scheduler->m_sleepCounter);
		if(m_running == false)
		{
			pthread_mutex_unlock(&m_scheduler->m_wakeUpMutex);
			break;
		}

        pthread_cond_wait(&m_scheduler->m_wakeUpSignal, &m_scheduler->m_wakeUpMutex);
		pthread_mutex_unlock(&m_scheduler->m_wakeUpMutex);
        debug_print("\tThread %i: I'm awake!\n", m_threadId);

		if(m_running == false)
		{
			break;
		}

		while(m_working && m_running)
		{
			/* let's do some work */
			WorkUntilBufferEmpty();
			debug_print("\tThread %i: Done with my work.\n", m_threadId);
		}
    }

	debug_print("\tThread %i: Is Terminating.\n", m_threadId);

}

/* Add a new task for this thread to complete - returns false if out of room */
bool CTaskWorkerThread::AddTaskToTopOfWorkPile(CAbstractTask* newTask)
{
	CAbstractTask* task = newTask;

	if(m_numTasks == MAX_NUM_THREAD_TASKS)
	{
		return false;
	}
	
	VERIFY_LOCK(pthread_mutex_lock(&m_taskPoolMutex));

	/* insert the new task */
	m_tasks[m_numTasks] = task;
	m_numTasks++;

	/* split new task into smaller chunks, one for each thread */
	int splits = MAX_NUM_THREAD_TASKS-m_numTasks;
	int splitsPerformed = 0;
	int threads = m_scheduler->GetNumOfThreads();
	splits = splits > threads ? threads : splits;

	debug_print("\tThread %i: Breaking my new task into %i pieces\n", m_threadId, splits);

	int splitStartIndex = m_numTasks-1;
	CAbstractTask* taskToSplit = NULL;
	CAbstractTask* newlySplitTask = NULL;
	
	bool keepSplitting = true;

	while(keepSplitting)
	{
		keepSplitting = (splitsPerformed < splits);
		for(int i = 0; (i < splitsPerformed+1) && keepSplitting; i++)
		{
			taskToSplit = m_tasks[splitStartIndex+i];

			if(taskToSplit->Split(&newlySplitTask))
			{
				/* add task to work queue */
				debug_print("\tThread %i: Throwing a new task on the pile.\n", m_threadId);
				
				m_tasks[m_numTasks] = newlySplitTask;
				keepSplitting = (splitsPerformed < splits);

				/* increment the task count */
				m_numTasks++;
				taskToSplit->IncrementTaskCount();
				splitsPerformed++;
			}
			else
			{
				debug_print("\tThread %i: Unable to break task into smaller pieces.\n", m_threadId);
				keepSplitting = false;
				break;
			}

		}

	}	

	pthread_mutex_unlock(&m_taskPoolMutex);

	return true;
}

/* Pop a new task off the top of work pile */
void CTaskWorkerThread::GrabTaskFromTopOfWorkPile(CAbstractTask** poppedTask)
{
	assert(poppedTask);
	*poppedTask = NULL;

	VERIFY_LOCK(pthread_mutex_lock(&m_taskPoolMutex));

	if(m_numTasks > 0)
	{
		*poppedTask = m_tasks[m_numTasks-1];
		m_tasks[m_numTasks-1] = NULL;
		m_numTasks--;
	}

	pthread_mutex_unlock(&m_taskPoolMutex);
}

/* Check for and give up extra work to other threads */
int CTaskWorkerThread::CanGiveUpWork(CAbstractTask **out)
{
	int i = 0;
	CAbstractTask** tasksOut = out;

	VERIFY_LOCK(pthread_mutex_lock(&m_taskPoolMutex));

	/* give away half of the work tasks */
	int giveCnt = (m_numTasks + (2-1)) / 2;
	for(i = 0; i < giveCnt; i++)
	{
		*tasksOut = m_tasks[i];
		tasksOut++;
	}

	/* get rid of empty space on bottom of stack by shifting tasks down */
	if(giveCnt > 0)
	{
		memmove(&m_tasks[0], &m_tasks[giveCnt], sizeof(m_tasks[0])*(MAX_NUM_THREAD_TASKS-giveCnt));
		memset(&m_tasks[m_numTasks-giveCnt], 0, sizeof(m_tasks[0])*giveCnt);
		m_numTasks -= giveCnt;
	}
	
	pthread_mutex_unlock(&m_taskPoolMutex);

	return giveCnt;
}

/* Steal extra tasks from other threads */
void CTaskWorkerThread::StealWorkFromOthers()
{
	int numThreads = m_scheduler->GetNumOfThreads();
	int thisThread = 0;
	int i = 0;

	/* find our place in the thread pool */
	for(i = 0; i < numThreads; i++)
	{
		CTaskWorkerThread* thread = &m_scheduler->m_threads[i];
		if(thread == this)
		{
			thisThread = i;
			break;
		}
	}

	debug_print("\tThread %i: Looking for some tasks to steal.\n", m_threadId);

	/* scan to our right until we find work to steal */
	int neighborIndex = 0;
	CAbstractTask* stolenTasks[10];
	int stolenCnt = 0;

	for(i = 1; i < numThreads; i++)
	{
		neighborIndex = (i + thisThread) % numThreads;
		stolenCnt = m_scheduler->m_threads[neighborIndex].CanGiveUpWork(stolenTasks);

		if(stolenCnt > 0)
		{
			debug_print("\tThread %i: Found %i tasks I could steal.\n", m_threadId, stolenCnt);
			for(int j = 0; j < stolenCnt; j++)
			{
				if(!AddTaskToTopOfWorkPile(stolenTasks[j]))
				{
					RunTask(stolenTasks[j], true);
				}
			}

			break;
		}
	}

	if(stolenCnt <= 0)
	{
		debug_print("\tThread %i: I couldn't find anything to steal.\n", m_threadId);
	}

}

/* Work on assigned tasks and steal from others when out of work */
void CTaskWorkerThread::WorkUntilBufferEmpty()
{
	CAbstractTask* taskToWork = NULL;

	if(m_numTasks == 0)
	{
		StealWorkFromOthers();
	}

	while(m_numTasks > 0)
	{
		GrabTaskFromTopOfWorkPile(&taskToWork);
		if(taskToWork != NULL)
		{
			debug_print("\tThread %i: Working on a new task.\n", m_threadId);

			CAbstractTask* newTask = NULL;
			CStackMemoryAllocator::MemMarker marker = 0;
			
			/* work on the task by chips */
			do
			{
				/* check to see if we should split the active task into our main queue */
				if(m_numTasks == 0)
				{
					newTask = NULL;
					if(taskToWork->Split(&newTask))
					{
						newTask->IncrementTaskCount();
						this->AddTaskToTopOfWorkPile(newTask);
					}
				}

				/* try to chip the task */
				newTask = NULL;
				marker = taskToWork->ChipTask(&m_scratchMem, &newTask);
				if(newTask != NULL)
				{
					RunTask(newTask, false);

					newTask->DestroyChip(m_scratchMem, marker);
				}
			}
			while(newTask != 0);
			
			/* run the final task portion */
			RunTask(taskToWork, true);

			taskToWork->Destroy();
			taskToWork = NULL;
		}

		/* if we are out of work, see if we can steal some from others */
		if(m_numTasks == 0)
		{
			debug_print("\tThread %i: I'm out of tasks.\n", m_threadId);
			StealWorkFromOthers();
		}
	}
}

/* Static worker thread entry point */
void* CTaskWorkerThread::Proc(void *args)
{
	CTaskWorkerThread* t = (CTaskWorkerThread*)args;
    t->_Proc();

	return NULL;
}

void CTaskWorkerThread::RunTask(CAbstractTask* task, bool updateProgress)
{
	task->RunTask();
	if(updateProgress)
	{
		task->IncrementTaskProgress();

		if(task->IsComplete())
			m_scheduler->CompleteTask(task);
	}
}