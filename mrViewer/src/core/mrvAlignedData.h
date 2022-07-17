/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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
 * @file   mrvAlignedData.h
 * @author gga
 * @date   Sun Jan 27 10:12:19 2008
 *
 * @brief
 *
 *
 */

#ifndef mrvAlignedData_h
#define mrvAlignedData_h


extern "C" {
#include <libavutil/mem.h>
}

#ifdef LINUX
#  include <iostream>
#  include <stdlib.h>
#  include <malloc.h>
#endif


namespace mrv {


  struct aligned16_uint8_t
  {
      uint8_t x;

    inline void* operator new[](size_t size)
    {
#ifdef LINUX
        void* ptr = av_malloc_array( size, sizeof(aligned16_uint8_t) );
        // void* ptr = NULL;
        // if ( posix_memalign( &ptr, 16, size * sizeof(aligned16_uint8_t) ) != 0 )
        //     throw std::bad_alloc();
#elif OSX
        void* ptr = av_malloc_array( size, sizeof(aligned16_uint8_t) );
#else
        void* ptr = av_malloc_array( size, sizeof(aligned16_uint8_t) );
#endif
        if (!ptr) throw std::bad_alloc();
        return ptr;
    }

    inline void operator delete[]( void* ptr )
    {
#ifdef LINUX
        free( ptr );
#else
        av_free( ptr );
#endif
    }
  };



}  // namespace mrv


#endif // mrvAlignedData
