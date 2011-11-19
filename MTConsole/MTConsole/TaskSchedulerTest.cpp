#include "StdAfx.h"
#include "TaskSchedulerTest.h"
#include "ParallelForTask.h"

TaskSchedulerTest::TaskSchedulerTest(void) : m_scheduler(NULL)
{
}

TaskSchedulerTest::~TaskSchedulerTest(void)
{
}

void TaskSchedulerTest::Setup()
{
	m_scheduler = new CTaskScheduler();
	m_scheduler->Init();
}

void TaskSchedulerTest::TearDown()
{
	delete m_scheduler;
}

void TaskSchedulerTest::Restart()
{
	TearDown();
	Setup();
}

void TaskSchedulerTest::RunTest()
{
	struct TestForLoop
	{
		int test_val;
		void RunRange(int start, int end)
		{
			/* verify the iteration order is legit */
			a( end >= start );

			int flag = (1<<start);
			for(int i = start; i <= end; i++)
			{
				/* verify each iteration is ran only once by the scheduler */
				a( (test_val & flag) == 0 );
				test_val |= flag;
				flag <<= 1;
			}
		}
	};

	/* Test scheduler runs each iteration of parallel loop */
	{
	Restart();
	EXPERIMENT("Parallel For Synchronous");

	/* __Init__ */
	TestForLoop loop;
	memset(&loop, 0, sizeof(TestForLoop));

	/* __Run__ */
	TEST_TIMER("Parallel For Synchronous");
	::ParallelForTask(&loop, Range(0,31), m_scheduler);
	STOP_TIMER();

	/* __Verify__ */
	for(int i = 0, mask = 1; i < 32; i++)
	{
		a( (loop.test_val & mask) != 0 );
		mask <<= 1;
	}

	} /***** end test *****/


	/* Test synchronous single iteration */
	{
	Restart();
	EXPERIMENT("Single Itr Synchronous");

	/* __Init__ */
	TestForLoop loop;
	memset(&loop, 0, sizeof(TestForLoop));

	/* __Run__ */
	TEST_TIMER("Single Itr Synchronous");
	::ParallelForTask(&loop, Range(0, 0), m_scheduler);
	STOP_TIMER();

	/* __Verify__ */
	a(loop.test_val == 1);

	} /***** end test *****/


	/* Test async parallel for w/ this thread spinning entire time */
	{
	Restart();
	EXPERIMENT("Parallel For Async");

	/* __Init__ */
	TestForLoop loop;
	memset(&loop, 0, sizeof(TestForLoop));
	CompletionToken token;
	unsigned int spins = 0;
	const unsigned int maxspins = 50;
	
	/* __Run__ */
	TEST_TIMER("Parallel For Async");
	::ParallelForAsyncTask(&loop, Range(0,31), m_scheduler, &token);
	bool keepSpinning = true;
	for(spins = 0; keepSpinning == true; spins++)
	{
		keepSpinning = !::ParallelForAsyncComplete(&loop, &token);
		Sleep(100);
		if(spins == maxspins)
		{
			afail();
			break;
		}
	}
	STOP_TIMER();

	/* __Verify__ */
	for(int i = 0, mask = 1; i < 32; i++)
	{
		a( (loop.test_val & mask) != 0 );
		mask <<= 1;
	}

	a(spins > 1);

	} /***** end test *****/

	/* Test single iteration async */
	{
	Restart();
	EXPERIMENT("Single Itr Async");

	/* __Init__ */
	TestForLoop loop;
	memset(&loop, 0, sizeof(TestForLoop));
	CompletionToken token;
	unsigned int spins = 0;
	const unsigned int maxspins = 500;
	
	/* __Run__ */
	TEST_TIMER("Single Itr Async");
	::ParallelForAsyncTask(&loop, Range(0,0), m_scheduler, &token);
	bool keepSpinning = true;
	for(spins = 0; keepSpinning == true; spins++)
	{
		keepSpinning = !::ParallelForAsyncComplete(&loop, &token);
	//	Sleep(100);
		if(spins == maxspins)
		{
			afail();
			break;
		}
	}
	STOP_TIMER();

	/* __Verify__ */
	a(loop.test_val == 1);
	a(spins > 1);

	} /***** end test *****/

	/* Test scheduler destroy w/ no tasks given */
	{
	Restart();
	EXPERIMENT("Zero-Task Shutdown");

	/* __Init__ */
	
	/* __Run__ */
	TEST_TIMER("Single Itr Async");
	Restart();

	STOP_TIMER();

	/* __Verify__ */

	} /***** end test *****/
	
}