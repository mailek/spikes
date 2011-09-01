#include "StdAfx.h"
#include "TaskScheduler.h"

CTaskScheduler::CTaskScheduler(void)
{
    m_wakeUpMutex = PTHREAD_MUTEX_INITIALIZER;
    m_wakeUpSignal = PTHREAD_COND_INITIALIZER;
    sem_init(&m_sleepCounter, NULL, m_numThreads);
	CreateThreads();
}

CTaskScheduler::~CTaskScheduler(void)
{
}

void CTaskScheduler::CreateThreads()
{
	/* get number of hardware processors */
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_numThreads = si.dwNumberOfProcessors;

    /* create thread for every processor (minus the main thread's core) */
	for(int i = 1; i < m_numThreads; i++)
	{
		/* create a new thread */
		_cprintf("Spawning thread %i", i);
        m_threads[i].m_scheduler = this;
		m_threads[i].m_threadId = i;

		pthread_create(&m_threads[i].m_pthread, NULL, &CWorkingThread::Proc, &m_threads[i]);
		
	}

	/* main thread will use m_thread[0] for its task pile */
	m_threads[0].m_scheduler = this;
}

void CTaskScheduler::AddTaskToMTQ(CAbstractTask* task)
{
    assert(task != NULL);

	m_threads[0].AddTaskToTopOfWorkPile(task);
}

void CTaskScheduler::WaitForAllWorkersToSleep()
{
    for(int i = 1; i < m_numThreads; i++)
	{
		sem_wait(&m_sleepCounter);
	}
}

void CTaskScheduler::WakeAllWorkers()
{
    pthread_cond_signal(&m_wakeUpSignal);
}

void CTaskScheduler::WorkUntilComplete()
{
	WakeAllWorkers();
	m_threads[0].WorkUntilBufferEmpty();
	WaitForAllWorkersToSleep();
}