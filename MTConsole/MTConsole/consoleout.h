#pragma once

void init_console();
void destroy_console();
void my_printf(const char* fmt, ... );

#if defined(_DEBUG) || defined(_TEST)
#define debug_print(...) my_printf(__VA_ARGS__)
#else
#define debug_print(...)
#endif