#pragma once
#include "testcommon.h"
#include "stackmemoryallocator.h"

class StackMemoryAllocatorTest :
	public ITest
{
public:
	StackMemoryAllocatorTest(void);
	~StackMemoryAllocatorTest(void);

	static ITest* CreateTest() { return new StackMemoryAllocatorTest(); }

public:
	/* ITest Members */
	virtual void Setup();
	virtual void TearDown();
	virtual void RunTest();
	TEST_NAME("StackMemoryAllocatorTest");

private:
	CStackMemoryAllocator *m_allocator;

};
