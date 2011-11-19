#pragma once

#define TEST_NAME( s ) \
	virtual const char* GetTestName() { return (s); }

class ITest 
{
public:
	virtual void Setup() =0;
	virtual void TearDown() =0;
	virtual void RunTest() =0;
	virtual const char* GetTestName() =0;
};