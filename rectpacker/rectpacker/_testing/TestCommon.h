#pragma once

//#include "highrestimer.h"
#include "itest.h"

#if defined(_DEBUG) || defined(_TEST)
#define PRINT_FAILURES
//#define TIME_TESTS
#else 
#define BREAK_ON_FAILURES
#endif

/* Start New Experiment */
#define EXPERIMENT( s ) \
	{ my_printf( "Experiment "s" Executing\n\n" ); }

/* Assert Bool */
#if defined(PRINT_FAILURES)
inline void a(bool x) 
	{ if(!(x)){debug_print("(TEST FAILED) Assertion: Line %d, %s\n", __LINE__, __FUNCTION__ );} }
#elif defined(BREAK_ON_FAILURES)
inline void a(bool x) 
	{ assert(x); }
#else
inline void a(bool x) {};
#endif

/* Fail Always */
#if defined(PRINT_FAILURES)
inline void afail() 
	{ debug_print("(TEST FAILED) Assertion: Line %d, %s\n", __LINE__, __FUNCTION__ ); }
#elif defined(BREAK_ON_FAILURES)
inline void afail() 
	{ assert(false); }
#else
inline void afail() {};
#endif

/* Timer Macros */
#if defined(TIME_TESTS)
#define TEST_TIMER( name ) \
	HRTimer timer; const char _name[] = name; Timer_Start(&timer);
#define STOP_TIMER() \
	Timer_Stop(&timer); my_printf("%s: Completed in %.3f ms.\n", _name, Timer_GetElapsedMillisecs(&timer));
#else
#define TEST_TIMER( name )
#define STOP_TIMER()
#endif