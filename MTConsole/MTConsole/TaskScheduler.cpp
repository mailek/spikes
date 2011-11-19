#include "StdAfx.h"
#include "TaskScheduler.h"

using namespace TaskScheduler;

static const int AUTO_HARDWARE = (-1);				/* auto detect number threads from hardware */

CTaskScheduler::CTaskScheduler(void) :  m_numThreads(AUTO_HARDWARE),
										m_numTasks(0),
										m_currentTask(NULL),
										m_isIdle(true)
{
	memset(m_tasks, 0, sizeof(m_tasks));
	memset(m_threads, 0, sizeof(m_threads));
}

/* Scheduler destruction, and terminate workers */
CTaskScheduler::~CTaskScheduler(void)
{
	ShutDown();
}

/* Terminate worker threads and free resources */
void CTaskScheduler::ShutDown()
{
	debug_print( "*** Shutting down scheduler ***\n" );
	VERIFY_LOCK(pthread_mutex_lock(&m_wakeUpMutex));
	for(int i = 1; i < m_numThreads; i++)
	{
		m_threads[i].m_running = false;
	}
	pthread_mutex_unlock(&m_wakeUpMutex);

	WaitForAllWorkersToSleep();
	WakeAllWorkers();
	for(int i = 1; i < m_numThreads; i++)
	{
		pthread_join(m_threads[i].m_pthread, NULL);
	}

	sem_destroy(&m_sleepCounter);
	pthread_cond_destroy( &m_wakeUpSignal );
	pthread_mutex_destroy( &m_wakeUpMutex );

	debug_print("*** Scheduler shut down ***\n\n");
}

/* Initialize scheduler resources, and wait for all threads to become available */
void CTaskScheduler::Init()
{
	debug_print("*** Setting up the scheduler ***\n");

	m_taskListMutex = PTHREAD_MUTEX_INITIALIZER;
    m_wakeUpMutex = PTHREAD_MUTEX_INITIALIZER;
    m_wakeUpSignal = PTHREAD_COND_INITIALIZER;
	sem_init(&m_sleepCounter, NULL, 0);
	CreateThreads();

	debug_print("*** Scheduler waiting for workers to be created ***\n");
	WaitForAllWorkersToSleep();

	debug_print("*** Scheduler ready ***\n\n");
}

/* Create and initialize all workers threads, up to 
   N-1 threads, where N is number of hardware processors */
void CTaskScheduler::CreateThreads()
{
	if(m_numThreads == AUTO_HARDWARE)
	{
		/* get number of hardware processors */
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		m_numThreads = min(si.dwNumberOfProcessors, MAX_NUM_THREADS);
	}

    /* create thread for every processor (minus the main thread's core) */
	for(int i = 1; i < m_numThreads; i++)
	{
		/* create a new thread */
		debug_print("*** Scheduler is spawning thread %i ***\n", i);
        m_threads[i].m_scheduler = this;
		m_threads[i].m_threadId = i;

		pthread_create(&m_threads[i].m_pthread, NULL, &CTaskWorkerThread::Proc, &m_threads[i]);
	}

	/* main thread will use m_thread[0] for its task pile */
	m_threads[0].m_scheduler = this;

	/* main thread initializes its own thread object */
	m_threads[0].Init(); 
}

/* Add a task to the multi-thread task queue */
void CTaskScheduler::AddTaskToWorkQueue(CAbstractTask* task)
{
    assert(task != NULL);
	assert(m_numTasks < MAX_NUM_SCHD_TASKS);

	debug_print("*** New task added to scheduler ***\n");
	
	VERIFY_LOCK(pthread_mutex_lock(&m_taskListMutex));
	memmove(&m_tasks[1], m_tasks, (MAX_NUM_SCHD_TASKS-1)*sizeof(m_tasks[0]));
	m_tasks[0] = task;
	task->IncrementTaskCount();
	m_numTasks++;

	if(m_isIdle)
	{
		WaitForAllWorkersToSleep();
		WakeAllWorkers();
		debug_print("*** Let's do this! ***\n");
		WorkOnNewTask();
	}	

	pthread_mutex_unlock(&m_taskListMutex);
}

/* Wait for each thread to enter sleep state */
void CTaskScheduler::WaitForAllWorkersToSleep()
{
	//if(!m_isIdle)
	//{
	debug_print("*** Scheduler waiting for all workers to be ready ***\n");
	    for(int i = 1; i < m_numThreads; i++)
		{
			sem_wait(&m_sleepCounter);
		}

	//	m_isIdle = true;
	//}
}

/* Wake all worker threads */
void CTaskScheduler::WakeAllWorkers()
{
	debug_print("*** Scheduler waking all workers ***\n");
	for(int i = 0; i < m_numThreads; i++)
	{
		m_threads[i].m_working = true;
	}

	m_isIdle = false;
    pthread_cond_broadcast(&m_wakeUpSignal);
}

/* Complete all work tasks in queue */
void CTaskScheduler::WorkUntilTaskComplete(CompletionToken* token)
{	
	while(!token->IsComplete())
	{
		m_threads[0].WorkUntilBufferEmpty();
	}
	//WaitForAllWorkersToSleep();

	debug_print("*** Given task is complete ***\n\n");
}

/* Set up to the maximum number of threads preferred by user */
void CTaskScheduler::SetNumberOfThreads(int threadCnt)
{
	m_numThreads = min(MAX_NUM_THREADS, threadCnt);
	m_numThreads = max(m_numThreads, 0 );
}

void CTaskScheduler::CompleteTask(CAbstractTask *task)
{
	VERIFY_LOCK(pthread_mutex_lock(&m_taskListMutex));

	if(m_currentTask->m_complete != task->m_complete)
	{
		pthread_mutex_unlock(&m_taskListMutex);
		assert(false);
		return;
	}

	m_numTasks--;
	m_currentTask = NULL;

	if(m_numTasks == 0)
	{
		for(int i = 1; i < m_numThreads; i++)
		{
			debug_print("*** All tasks in scheduler complete - putting threads to sleep ***\n");
			m_threads[i].m_working = false;
		}
	}
	else
	{
		WorkOnNewTask();
	}

	pthread_mutex_unlock(&m_taskListMutex);
}

/* NOTE: This function reads from m_numTasks without explicitly locking.
		Only call this function after locking the task pool. */
void CTaskScheduler::WorkOnNewTask()
{
	assert(m_currentTask == NULL);
	m_currentTask = m_tasks[m_numTasks-1];
	if(m_currentTask != NULL)
	{
		m_tasks[m_numTasks-1] = NULL;

		debug_print("*** Feeding a new task to the workers ***\n");
		VERIFY(m_threads[0].AddTaskToTopOfWorkPile(m_currentTask));
	}
}