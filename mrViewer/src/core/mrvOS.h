/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#if !(defined vsnprintf)
#  define vsnprintf       _vsnprintf
#endif

#if !(defined putenv)
#  define putenv(x)       _putenv(x)
#endif

#if !(defined strdup)
#  define strdup(x)       _strdup(x)
#endif

#undef max
#undef min
#undef stricmp

#if !(defined strcasecmp)
#  ifdef _WIN32
#    define strcasecmp(a,b) _stricmp(a,b)
#  else
#    define strcasecmp(a,b) stricmp(a,b)
#  endif
#endif

#if !(defined strtok_r)
#  define strtok_r(a,b,c) strtok(a,b)
#endif

#if !(defined snprintf)
#  define snprintf        _snprintf
#endif

#if !(defined access)
#  define access          _access
#endif

#undef  _ITERATOR_DEBUG_LEVEL 
#define _ITERATOR_DEBUG_LEVEL 0

#endif // defined(WIN32) || defined(WIN64)

#endif // mrvOS_h
