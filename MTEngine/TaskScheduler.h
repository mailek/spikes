#pragma once

#include "abstracttask.h"
#include "workingthread.h"

static const int MAX_NUM_THREADS = 10;

class CTaskScheduler
{
public:
    CTaskScheduler(void);
    ~CTaskScheduler(void);

	CWorkingThread m_threads[MAX_NUM_THREADS];

public:
    void AddTaskToMTQ(CAbstractTask* task);
	void WorkUntilComplete();

    sem_t               m_sleepCounter; /* scheduler waits on this when synching with threads */
    pthread_mutex_t     m_wakeUpMutex;  /* protects the wake up signal */
    pthread_cond_t      m_wakeUpSignal; /* workers wait on this when idle */

private:
	int m_numThreads;
	
	void CreateThreads();
    void WaitForAllWorkersToSleep();
    /* temp */ public:
    void WakeAllWorkers();

public:
	inline int GetNumOfThreads() {return m_numThreads;}

};
