/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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

#include "gui/mrvIO.h"
#include <boost/cstdint.hpp>

// #if defined(_WIN32) || defined(_WIN64)
// #  define memalign( b, a )   _aligned_malloc( a, b )
// #  define memalign_free( a ) _aligned_free( a )
// #else
// #  define memalign_free( a ) free( a )
// #endif



namespace mrv {


  struct aligned16_uint8_t
  {
      uint8_t x;

    inline void* operator new(size_t size)
    {
        return av_malloc( size );
    }

    inline void operator delete( void* ptr )
    {
        av_free( ptr );
    }

    inline void* operator new[](size_t size)
    {
        return av_malloc( size * sizeof(aligned16_uint8_t) );
    }

    inline void operator delete[]( void* ptr )
    {
        av_free( ptr );
    }
  };



}  // namespace mrv


#endif // mrvAlignedData
