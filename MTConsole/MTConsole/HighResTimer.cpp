#include "StdAfx.h"
#include "HighResTimer.h"

static double LI_ToSeconds(LARGE_INTEGER* li)
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	return (double)li->QuadPart/(double)freq.QuadPart;
}

void Timer_Start(HRTimer* timer)
{
	QueryPerformanceCounter(&timer->start);
}

void Timer_Stop(HRTimer* timer)
{
	QueryPerformanceCounter(&timer->stop);
}

double Timer_GetElapsedMillisecs(HRTimer* timer)
{
	LARGE_INTEGER elapsed;
	elapsed.QuadPart = timer->stop.QuadPart - timer->start.QuadPart;

	return 1000.0*LI_ToSeconds(&elapsed);
}