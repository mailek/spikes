// MTConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "irenderable.h"
#include "doublelinkedlist.h"
#include "taskscheduler.h"
#include "parallelfortask.h"

#ifdef _TEST
#include <limits.h>
#include "testmain.h"
#endif

using namespace TaskScheduler;

CDoubleLinkedList<IRenderable> display_list;
CTaskScheduler* scheduler;


void DoFrame();
void PreUpdate();       /* objects are read only - no object can change its public state */
void Update();          /* ignore others - update self state */
void Draw();            /* culling, add key to list of object to display */
void SortRenderables(); /* sort list of render items */
void Render();          /* do actual render draw */

int _tmain(int argc, _TCHAR* argv[])
{
	init_console();

#ifdef _TEST
	TestMain* unitTests = new TestMain();
	unitTests->RunSuite();
	delete unitTests;
#endif

	while(true)
	{
		DoFrame();
	}
	
	destroy_console();

	return 0;
}

void DoFrame()
{
	scheduler = new CTaskScheduler();
	scheduler->SetNumberOfThreads(2);
	scheduler->Init();

    PreUpdate();
    Update();
    Draw();
    SortRenderables();
    Render();

	delete scheduler;
}

/* objects are read only - no object can change its public 
	state - objects can themselves dependent on other objects */
void PreUpdate()
{
    /* TODO: parallel-for loop through each item in world and call preupdate() */

	/* task object is destroyed by scheduler */
	CompletionToken taskComplete;
	//CParallelForTask<int> task(&taskComplete); // TODO:
	//task.m_range.start = 0;
	//task.m_range.end = 500;//INT_MAX/2;
	//task.m_range.stepSize = 5;
	//task.m_isMasterTask = true;

	
	debug_print( "\n\nStarting a New Frame!\n\n" );
}

/* ignore others - update self state */
void Update()
{
}

/* culling, add key to list of object to display */
void Draw()
{
    /* TODO: iterate over each item in world, cull them, then call draw() on remaining */
}

/* sort list of render items */
void SortRenderables()
{
    /* TODO: sort display_list */
}

/* do actual render draw */
void Render()
{
    /* TODO: iterate each item in display_list and call render() */
}