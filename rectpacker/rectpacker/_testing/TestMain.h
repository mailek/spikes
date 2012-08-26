#pragma once
#include "testcommon.h"
#include "testsuite.h"
	
typedef ITest* TestBattery[50];

class TestMain
{
public:
	TestMain(void);
	~TestMain(void);

public:
	void RunSuite();

private:
	void CreateSuite();

private:
	TestBattery		m_tests;
};