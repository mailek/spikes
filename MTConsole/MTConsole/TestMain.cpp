#include "StdAfx.h"
#include "TestMain.h"

#define cnt_of_array(a) \
	(sizeof(a)/sizeof(a[0]))

TestMain::TestMain(void)
{
	memset(m_tests, 0, sizeof(m_tests));
}

TestMain::~TestMain(void)
{
}

void TestMain::RunSuite()
{
	my_printf("Creating Test Suite.\n");
	CreateSuite();
	int testCnt;
	for(testCnt = 0; m_tests[testCnt] != 0; testCnt++);
	my_printf("Found %d Tests.\n", testCnt);

	my_printf("Running Test Suite.\n\n");
	for(int i = 0; i<testCnt; i++)
	{	
		my_printf("Starting %s.\n", m_tests[i]->GetTestName());
		m_tests[i]->Setup();
		m_tests[i]->RunTest();
		m_tests[i]->TearDown();
		my_printf("%s Completed.\n\n", m_tests[i]->GetTestName());
	}
}

void TestMain::CreateSuite()
{
	for(int i = 0; i < cnt_of_array(s_suite); i++)
	{
		m_tests[i] = (*s_suite[i])();
	}
}