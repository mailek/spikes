#include "StdAfx.h"
#include "TaskScheduler.h"

using namespace TaskScheduler;

static const int AUTO_HARDWARE = (-1);				/* auto detect number threads from hardware */

CTaskScheduler::CTaskScheduler(void) : m_numThreads(AUTO_HARDWARE)
{
	memset(m_tasks, 0, sizeof(m_tasks));
}

/* Scheduler destruction, and terminate workers */
CTaskScheduler::~CTaskScheduler(void)
{
	ShutDown();
}

/* Terminate worker threads and free resources */
void CTaskScheduler::ShutDown()
{
	for(int i = 1; i < m_numThreads; i++)
	{
		m_threads[i].m_running = false;
	}

	WakeAllWorkers();
	for(int i = 1; i < m_numThreads; i++)
	{
		pthread_join(m_threads[i].m_pthread, NULL);
	}

	sem_destroy(&m_sleepCounter);
	pthread_cond_destroy( &m_wakeUpSignal );
	pthread_mutex_destroy( &m_wakeUpMutex );

	my_printf("*** Scheduler shut down ***\n\n");
}

/* Initialize scheduler resources, and wait for all threads to become available */
void CTaskScheduler::Init()
{
	my_printf("*** Setting up the scheduler ***\n");

    m_wakeUpMutex = PTHREAD_MUTEX_INITIALIZER;
    m_wakeUpSignal = PTHREAD_COND_INITIALIZER;
	sem_init(&m_sleepCounter, NULL, 0);
	CreateThreads();

	my_printf("*** Scheduler waiting for workers to be created ***\n");
	WaitForAllWorkersToSleep();

	my_printf("*** Scheduler ready ***\n\n");
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
		my_printf("*** Scheduler is spawning thread %i ***\n", i);
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
void CTaskScheduler::AddTaskToMTQ(CAbstractTask* task)
{
    assert(task != NULL);
	assert(m_numTasks < MAX_NUM_SCHD_TASKS);

	my_printf("*** New task added to scheduler ***\n");
	
	/* todo: protect task queue with mutex */
	memmove(&m_tasks[1], m_tasks, (MAX_NUM_SCHD_TASKS-1)*sizeof(m_tasks[0]));
	m_tasks[0] = task;
	task->IncrementTaskCount();
	m_numTasks++;

	if(m_numTasks == 1)
	{
		my_printf("*** Scheduler waiting for all workers to be ready ***\n");
		WakeAllWorkers();
		my_printf("*** Let's do this! ***\n");
	}	
}

/* Wait for each thread to enter sleep state */
void CTaskScheduler::WaitForAllWorkersToSleep()
{
    for(int i = 1; i < m_numThreads; i++)
	{
		sem_wait(&m_sleepCounter);
	}
}

/* Wake all worker threads */
void CTaskScheduler::WakeAllWorkers()
{
	for(int i = 0; i < m_numThreads; i++)
	{
		m_threads[i].m_working = true;
	}

    pthread_cond_broadcast(&m_wakeUpSignal);
}

/* Complete all work tasks in queue */
void CTaskScheduler::WorkUntilTaskComplete(/*task_complete_token token*/)
{	
	//while(token.NotComplete())
	//{
	m_threads[0].WorkUntilBufferEmpty();
	//}

	my_printf("*** Given task is complete ***\n\n");
}

/* Set up to the maximum number of threads preferred by user */
void CTaskScheduler::SetNumberOfThreads(int threadCnt)
{
	m_numThreads = min(MAX_NUM_THREADS, threadCnt);
	m_numThreads = max(m_numThreads, 0 );
}

void CTaskScheduler::CompleteTask()
{
	/* todo: protect with mutex */
	m_numTasks--;
	if(m_numTasks == 0)
	{
		for(int i = 1; i < m_numThreads; i++)
		{
			my_printf("*** All tasks in scheduler complete - putting threads to sleep ***");
			m_threads[i].m_working = false;
			WaitForAllWorkersToSleep();
		}
	}
	else
	{
		WorkOnNewTask();
	}
}

void CTaskScheduler::WorkOnNewTask()
{
	CAbstractTask* task = m_tasks[m_numTasks];
	m_tasks[m_numTasks] = NULL;
	assert(task != NULL);

	my_printf("*** Feeding a new task to the workers ***\n");
	VERIFY(m_threads[0].AddTaskToTopOfWorkPile(task));
}