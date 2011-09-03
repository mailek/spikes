#pragma once

extern pthread_mutex_t	console_mutex;
extern bool console_init;

inline void init_console()
{
#if defined(_DEBUG) || defined(_TEST)
	console_mutex = PTHREAD_MUTEX_INITIALIZER;
	console_init = true;
#endif
}

inline void destroy_console()
{
#if defined(_DEBUG) || defined(_TEST)
	pthread_mutex_destroy(&console_mutex);
#endif
}

inline void my_printf(const char* fmt, ... )
{
#if defined(_DEBUG) || defined(_TEST)
	pthread_mutex_lock(&console_mutex);
	va_list args;
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);

	pthread_mutex_unlock(&console_mutex);
#endif
}