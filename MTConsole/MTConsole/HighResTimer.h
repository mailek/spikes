#pragma once

struct HRTimer
{
	LARGE_INTEGER start;
	LARGE_INTEGER stop;
};

void Timer_Start(HRTimer* timer);
void Timer_Stop(HRTimer* timer);
double Timer_GetElapsedMillisecs(HRTimer* timer);