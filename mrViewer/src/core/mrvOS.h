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
