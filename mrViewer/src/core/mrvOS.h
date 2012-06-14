/**
 * @file   mrvOS.h
 * @author gga
 * @date   Sun Jan 13 09:15:12 2008
 * 
 * @brief  Auxiliary file hiding platform differences (mainly, non POSIX)
 * 
 * 
 */

#ifndef mrvOS_h
#define mrvOS_h


#if (defined(_WIN32) || defined(_WIN64))

#  define vsnprintf       _vsnprintf
#  define putenv(x)       _putenv(x)
#  define strdup(x)       _strdup(x)
#undef stricmp
#  define strcasecmp(a,b) stricmp(a,b)
#  define strtok_r(a,b,c) strtok(a,b)
#  define snprintf        _snprintf
#  define access          _access

#endif // defined(WIN32) || defined(WIN64)

#endif // mrvOS_h
