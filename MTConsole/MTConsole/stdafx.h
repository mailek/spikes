// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <assert.h>
#include <cstdio>



/* Threading */
#include <pthread.h>
#include <semaphore.h>
#pragma comment(lib, "pthreadvc1.lib")

#include "consoleout.h"
// TODO: reference additional headers your program requires here

/* testing */
#if defined(_TEST) || defined(_DEBUG)
#define VERIFY_LOCK(e) \
	if(e == EDEADLK ) assert(false);
#else
#define VERIFY_LOCK(e) e
#endif


// verify macro
#ifdef _DEBUG
#define VERIFY(a) \
	assert(a)
#else
#define VERIFY(a) \
	a
#endif
