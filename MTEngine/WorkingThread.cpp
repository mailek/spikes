#include "StdAfx.h"
#include "WorkingThread.h"
#include "TaskScheduler.h"

CWorkingThread::CWorkingThread(void) :	m_numTasks(0),
										m_threadId(0)
{
	memset(&m_tasks, 0, sizeof(m_tasks));
	m_taskPoolMutex = PTHREAD_MUTEX_INITIALIZER;
}

CWorkingThread::~CWorkingThread(void)
{
	pthread_mutex_destroy(&m_taskPoolMutex);
}

void CWorkingThread::_Proc()
{
    unsigned int id = (unsigned int)this;

	_cprintf("Thread %i: Is Alive! %i", m_threadId);
	
    while(true)
    {
		_cprintf("Thread %i: Going to Sleep!", m_threadId);
        sem_post(&m_scheduler->m_sleepCounter);

        pthread_mutex_lock(&m_scheduler->m_wakeUpMutex);
        pthread_cond_wait(&m_scheduler->m_wakeUpSignal, &m_scheduler->m_wakeUpMutex);
        pthread_mutex_unlock(&m_scheduler->m_wakeUpMutex);
		_cprintf("Thread %i: I'm awake!", m_threadId);

		/* let's do some work */
		WorkUntilBufferEmpty();
		_cprintf("Thread %i: Done with my work.", m_threadId);
    }

}

/* Static Worker Thread Entry Point */
void* CWorkingThread::Proc(void *args)
{
	CWorkingThread* t = (CWorkingThread*)args;
    t->_Proc();

	pthread_exit(NULL);

	return NULL;
}

/* called from other threads to add a new task for this thread to complete, returns false if out of room */
bool CWorkingThread::AddTaskToTopOfWorkPile(CAbstractTask* newTask)
{
	CAbstractTask* task = newTask;
	pthread_mutex_lock(&m_taskPoolMutex);

	if(m_numTasks == MAX_NUM_TASKS)
	{
		return false;
	}
	
	/* insert the new task */
	m_tasks[m_numTasks] = task;
	m_numTasks++;

	/* split new task into smaller chunks, one for each thread */
	int splits = MAX_NUM_TASKS-m_numTasks;
	int splitsPerformed = 0;
	int threads = m_scheduler->GetNumOfThreads();
	splits = splits > threads ? threads : splits;

	_cprintf("Thread %i: Breaking my new task into %i pieces", m_threadId, splits);

	int splitStartIndex = m_numTasks-1;
	CAbstractTask* taskToSplit = NULL;
	CAbstractTask* newlySplitTask = NULL;
	while(splitsPerformed < splits)
	{
		for(int i = 0; (i < splitsPerformed+1) && (splitsPerformed < splits); i++)
		{
			taskToSplit = m_tasks[splitStartIndex+i];

			if(taskToSplit->Split(&newlySplitTask))
			{
				splitsPerformed++;
				m_tasks[m_numTasks] = newlySplitTask;
				m_numTasks++;
			}

		}

	}	

	pthread_mutex_unlock(&m_taskPoolMutex);

	return true;
}

void CWorkingThread::GrabTaskFromTopOfWorkPile(CAbstractTask** poppedTask)
{
	assert(poppedTask);

	pthread_mutex_lock(&m_taskPoolMutex);

	*poppedTask = NULL;
	if(m_numTasks > 0)
	{
		*poppedTask = m_tasks[m_numTasks-1];
		m_tasks[m_numTasks-1] = NULL;
		m_numTasks--;

	}

	pthread_mutex_unlock(&m_taskPoolMutex);
}

/* called by other threads to check if this thread has any extra work to give up */
int CWorkingThread::CanGiveUpWork(CAbstractTask **out)
{
	int i = 0;
	CAbstractTask** tasksOut = out;

	pthread_mutex_lock(&m_taskPoolMutex);

	/* give away half of the work tasks */
	int giveCnt = m_numTasks / 2;
	for(i = 0; i < giveCnt; i++)
	{
		*tasksOut = m_tasks[i];
		tasksOut++;
	}

	/* get rid of empty space on bottom of stack by shifting tasks down */
	if(giveCnt > 0)
	{
		memmove(&m_tasks[0], &m_tasks[giveCnt], sizeof(m_tasks[0])*(MAX_NUM_TASKS-giveCnt));
		memset(&m_tasks[m_numTasks-giveCnt], 0, sizeof(m_tasks[0])*giveCnt);
	}
	
	pthread_mutex_unlock(&m_taskPoolMutex);

	return giveCnt;
}

void CWorkingThread::StealWorkFromOthers()
{
	int numThreads = m_scheduler->GetNumOfThreads();
	int thisThread = 0;
	int i = 0;

	/* find our place in the thread pool */
	for(i = 0; i < numThreads; i++)
	{
		CWorkingThread* thread = &m_scheduler->m_threads[i];
		if(thread == this)
		{
			thisThread = i;
			break;
		}
	}

	_cprintf("Thread %i: Looking for some tasks to steal.", m_threadId);

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
			_cprintf("Thread %i: Found %i tasks I could steal.", m_threadId, stolenCnt);
			for(int j = 0; j < stolenCnt; j++)
			{
				AddTaskToTopOfWorkPile(stolenTasks[j]);
			}

			break;
		}
	}
}

void CWorkingThread::WorkUntilBufferEmpty()
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
			_cprintf("Thread %i: Working on a new task", m_threadId);
			taskToWork->RunTask();
			delete taskToWork;
		}

		/* if we are out of work, see if we can steal some from others */
		if(m_numTasks == 0)
		{
			_cprintf("Thread %i: I'm out of tasks.", m_threadId);
			StealWorkFromOthers();
		}
	}
}