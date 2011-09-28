/**
 * @file   mrStackTrace.cpp
 * @author gga
 * @date   Wed Jul 11 00:23:17 2007
 * 
 * @brief  Simple stack trace routine
 * 
 * 
 */


#if defined(WIN32) || defined(WIN64)
#  include "mrStackTrace_win32.cpp"
#else
#  ifdef LINUX
#    include "mrStackTrace_linux.cpp"
#  else
#    error Unknown OS for stack trace
#  endif
#endif


