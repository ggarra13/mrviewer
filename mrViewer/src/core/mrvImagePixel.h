/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2016  Gonzalo Garramuño

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
 * @file   mrvImagePixel.h
 * @author gga
 * @date   Mon Aug 27 23:25:30 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvImagePixel_h
#define mrvImagePixel_h


#include <iostream>

#include "mrvSSE.h"

namespace mrv {

#ifdef MR_SSE
#pragma pack(push,16) /* Must ensure class & union 16-B aligned */
#endif

  struct ImagePixel
  {
       ImagePixel() :
       r( 0 ), g( 0 ), b( 0 ), a( 0 )
       { };

       ImagePixel( float _r, float _g, float _b, float _a=1.0f ) :
       r(_r), g(_g), b(_b), a(_a)
       {
       };

       void clamp()
       {
	  if ( r < 0.0f ) r = 0.0f;
	  else if ( r > 1.0f ) r = 1.0f;
	  if ( g < 0.0f ) g = 0.0f;
	  else if ( g > 1.0f ) g = 1.0f;
	  if ( b < 0.0f ) b = 0.0f;
	  else if ( b > 1.0f ) b = 1.0f;
	  if ( a < 0.0f ) a = 0.0f;
	  else if ( a > 1.0f ) a = 1.0f;
       }

#if defined(MR_SSE)

       ImagePixel( const ImagePixel& b ) : _L( b._L ) {}
       
       ImagePixel( const F32vec4& b ) : _L( b ) {}

       ImagePixel& operator=( const F32vec4& b ) {
	  _L = b;
	  return *this;
       }
       
       ImagePixel& operator=( const ImagePixel& b ) {
	  _L = b._L;
	  return *this;
       }
       
       
       
       operator F32vec4() const { return _L; }
       operator __m128()  const { return _L; }
       
       operator F32vec4() { return _L; }
       operator __m128()  { return _L; }

       union {

	    struct {
		 __m128 _L;
	    };
	    
	    struct {
		 float r;
		 float g;
		 float b;
		 float a;
	    };

       }; // union

#else
      ImagePixel( const ImagePixel& x ) : 
      r( x.r ), g( x.g ), b( x.b ), a( x.a ) {}


      ImagePixel& operator=( const ImagePixel& x ) {
          r = x.r; g = x.g; b = x.b; a = x.a;
	  return *this;
       }

       float r, g, b, a;
#endif


    inline float& operator[](const int i) { return ((float*)this)[i]; }


  };


#ifdef MR_SSE
#pragma pack(pop) /* Must ensure class & union 16-B aligned */
#endif

inline
std::ostream& operator<<( std::ostream& o, const mrv::ImagePixel& p )
{
  return o << '(' << p.r << ' ' << p.g << ' ' << p.b << ' ' << p.a << ')';
}

} // namespace mrv



#endif // mrvImagePixel_h
