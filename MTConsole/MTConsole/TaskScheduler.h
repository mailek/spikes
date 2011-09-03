#pragma once

#include "abstracttask.h"
#include "taskworkerthread.h"

namespace TaskScheduler
{

static const int MAX_NUM_THREADS = 10;				/* max number of threads supported */
static const int MAX_NUM_SCHD_TASKS = 10;				/* max number of whole tasks supported */

class CTaskScheduler
{
	friend class CTaskWorkerThread;

public:
    CTaskScheduler(void);
    ~CTaskScheduler(void);

public:
	void Init();
    void AddTaskToMTQ(CAbstractTask* task);
	void WorkUntilTaskComplete();
	void SetNumberOfThreads(int threadCnt);
	void CompleteTask();

private:
	int					m_numThreads;				/* number of threads in use */
	CTaskWorkerThread	m_threads[MAX_NUM_THREADS]; /* thread pool, main thread is index 0 */
	int					m_numTasks;					/* number of tasks in queue */
	CAbstractTask*		m_tasks[MAX_NUM_SCHD_TASKS];		/* task queue - fifo */

    sem_t               m_sleepCounter;				/* scheduler waits on this when synching with threads */
    pthread_mutex_t     m_wakeUpMutex;				/* protects the wake up signal */
    pthread_cond_t      m_wakeUpSignal;				/* workers wait on this when idle */
	
	void CreateThreads();
    void WaitForAllWorkersToSleep();
    void WakeAllWorkers();
	void ShutDown();
	void WorkOnNewTask();

public:
	inline int GetNumOfThreads() {return m_numThreads;}

};

}