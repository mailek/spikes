#include "StdAfx.h"
#include "ParallelForTask.h"

CParallelForTask::CParallelForTask(void)
{
}

CParallelForTask::~CParallelForTask(void)
{
}

void CParallelForTask::RunTask()
{
	/* default empty implementation */
	_cprintf("Running for loop: start %i  end %i", m_range.start, m_range.end);
}

bool CParallelForTask::Split(CAbstractTask **newTask)
{
	int currRangeSize = m_range.end-m_range.start+1;

	if(currRangeSize <= m_range.stepSize)
	{
		return false;
	}

	int newTaskRangeSize = (currRangeSize / 2);
	int oldTaskRangeSize = currRangeSize - newTaskRangeSize;

	CParallelForTask *task = new CParallelForTask();
	
	task->m_range.start = m_range.start+oldTaskRangeSize;
	task->m_range.end = task->m_range.start+newTaskRangeSize-1;
	task->m_range.stepSize = m_range.stepSize;
	
	m_range.end = m_range.start + oldTaskRangeSize-1;

	*newTask = task;

	return true;
}