#pragma once

#include "StackMemoryAllocatorTest.h"
#include "TaskSchedulerTest.h"

typedef ITest*(*CreateTestFunc)(void);

static CreateTestFunc s_suite[] = 
{
	StackMemoryAllocatorTest::CreateTest,
	TaskSchedulerTest::CreateTest
};