#pragma once
#include "testcommon.h"
#include "taskscheduler.h"

using namespace TaskScheduler;

class TaskSchedulerTest :
	public ITest
{
public:
	TaskSchedulerTest(void);
	~TaskSchedulerTest(void);

	static ITest* CreateTest() { return new TaskSchedulerTest(); }

public:
	/* ITest Members */
	virtual void Setup();
	virtual void TearDown();
	virtual void RunTest();
	TEST_NAME("TaskSchedulerTest");

private:
	CTaskScheduler *m_scheduler;

	void Restart();
};
