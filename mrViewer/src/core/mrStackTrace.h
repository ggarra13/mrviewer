

#ifndef mrStackTrace_h
#define mrStackTrace_h

#if defined(_WIN32) || defined(_WIN64)
#include "mrStackTrace_win32.h"
#else
#include "mrStackTrace_linux.h"
#endif

#endif // mrStackTrace_h
