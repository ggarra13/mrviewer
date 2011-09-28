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
    ImagePixel()  { };

    ImagePixel( float _r, float _g, float _b, float _a=1.0f ) :
      r(_r), g(_g), b(_b), a(_a)
    {
    };

#ifdef MR_SSE

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
    float r;
    float g;
    float b;
    float a;
#endif


    inline float& operator[](const int i) { return ((float*)this)[i]; }

  };


#ifdef MR_SSE
#pragma pack(pop) /* Must ensure class & union 16-B aligned */
#endif

} // namespace mrv


inline
std::ostream& operator<<( std::ostream& o, const mrv::ImagePixel& p )
{
  return o << '(' << p.r << ' ' << p.g << ' ' << p.b << ' ' << p.a << ')';
}

#endif // mrvImagePixel_h
