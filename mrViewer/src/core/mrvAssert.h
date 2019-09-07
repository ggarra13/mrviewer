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
 * @file   mrvAssert.h
 * @author gga
 * @date   Sun Jul 15 08:41:02 2007
 *
 * @brief   assertion routines
 *
 *
 */

#ifndef mrvAssert_h
#define mrvAssert_h

#ifdef _WIN32

#include <windows.h>

#define mrvABORT \
Sleep( 100000 ); \
::abort();

#else

#define mrvABORT \
::abort

#endif

/**
 * assert0() equivalent, that is always enabled.
 */
#define assert0(cond) do {                                            \
    if (!(cond)) {                                                       \
        std::cerr << "Assertion " << #cond << " failed at " \
                  << __FILE__ << ":" << __LINE__ << std::endl;           \
        mrvABORT;                                                      \
    }                                                                    \
} while (0)


#endif // mrvAssert_h
