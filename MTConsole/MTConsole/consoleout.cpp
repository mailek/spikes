#include "stdafx.h"
#include "consoleout.h"

#if defined(_DEBUG) || defined(_TEST)
pthread_mutex_t	console_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;
#else
pthread_mutex_t	console_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void init_console()
{
#pragma warning(push)
#pragma warning(disable:4996)
	//setbuf(stdout, NULL);
#pragma warning(pop)
}

void destroy_console()
{
	pthread_mutex_destroy(&console_mutex);
}

void my_printf(const char* fmt, ... )
{
	VERIFY_LOCK(pthread_mutex_lock(&console_mutex));

	va_list args;
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);

	pthread_mutex_unlock(&console_mutex);
}