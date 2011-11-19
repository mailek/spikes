#include "StdAfx.h"
#include "StackMemoryAllocatorTest.h"

typedef struct 
{
	int i;
	float f;
	char c;
	struct
	{
		int si;
		float sf;
	} s;

} TestAllocateType;

typedef CStackMemoryAllocator::MemMarker Marker;

StackMemoryAllocatorTest::StackMemoryAllocatorTest(void) : m_allocator(NULL)
{
}

StackMemoryAllocatorTest::~StackMemoryAllocatorTest(void)
{
}

void StackMemoryAllocatorTest::Setup()
{
	m_allocator = new CStackMemoryAllocator();
}

void StackMemoryAllocatorTest::TearDown()
{
	delete m_allocator;
}

void StackMemoryAllocatorTest::RunTest()
{
	/* Test New Malloc Serves From Memory Pool */
	{
	EXPERIMENT("New Malloc");
		
	/* __Init__ */
	TestAllocateType *instance = NULL;

	/* __Run__ */
	TEST_TIMER("New Malloc Test");
	m_allocator->Clear();
	Marker old_top = m_allocator->m_stackTop;
	Marker marker = m_allocator->Malloc<TestAllocateType>(&instance);
	STOP_TIMER();

	/* __Verify__ */
	a(instance != NULL);
	a(instance->c == 0);
	a(instance->f == 0.0f);
	a(instance->i == 0);
	a(instance->s.sf == 0.0f);
	a(instance->s.si == 0);

	a(instance == (TestAllocateType*)old_top);
	a(false);

	m_allocator->Free(marker, sizeof(*instance));
	}	

}