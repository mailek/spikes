#pragma once
#include "abstracttask.h"

class CParallelForTask : public CAbstractTask
{
public:
    CParallelForTask(void);
    ~CParallelForTask(void);

public:
	virtual void RunTask();
    virtual bool Split(CAbstractTask **newTask);

	Range		m_range;
};
