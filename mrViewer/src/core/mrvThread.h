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
 * @file   mrvThread.h
 * @author gga
 * @date   Sun Aug  5 07:07:23 2007
 *
 * @brief  Some auxiliary macros for dealing and debugging locks in threads
 *
 *
 */

#ifndef mrvThread_h
#define mrvThread_h

//#define DEBUG_MUTEX

#ifdef DEBUG_MUTEX

#include <sys/types.h>
#include <sys/syscall.h>

#define SCOPED_LOCK(x)  if (1) {                                            \
    pid_t tid = syscall(SYS_gettid);                                        \
    std::cerr << std::dec << tid << ") Lock   " << #x << " " << &x << " "   \
              << __FILE__ << " " << std::dec << __LINE__  << std::endl; }   \
    Mutex::scoped_lock lk_##x(x);

#define CONDITION_WAIT( cond, x ) if (1) {                                \
       std::cerr << "Wait   " << #x << " " << &x << " "                   \
              << __FILE__ << " " << __LINE__                              \
                 << std::endl;                                            \
       cond.wait(lk_##x);                                                 \
       std::cerr << "Got    " << #x << " " << &x << " "                   \
                 << __FILE__ << " " << __LINE__                           \
                 << std::endl; }


#define CONDITION_TIMED_WAIT( cond, x, b ) if (1) {                          \
    std::cerr << "Wait time " << b.sec << " " << #x << " " << &x << " "  \
              << __FILE__ << " " << __LINE__                                 \
                 << std::endl;                                               \
    cond.timed_wait( lk_##x, b );                                            \
    std::cerr << "Got time " << #x << " " << &x << " "                         \
              << __FILE__ << " " << __LINE__  << std::endl; }

#else


#define SCOPED_LOCK(mutex) Mutex::scoped_lock lk_##mutex(mutex)
#define CONDITION_WAIT( cond, mutex )   cond.wait( lk_##mutex );

#define CONDITION_TIMED_WAIT( cond, mutex, b ) \
     cond.wait_until( lk_##mutex, b );

#endif



#endif // mrvThread_h
