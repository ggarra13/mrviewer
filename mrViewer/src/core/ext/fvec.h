/**
 * @file   fvec.h
 * @author gga
 * @date   Sat Dec  9 17:59:49 2006
 * 
 * @brief  Simple vector class to support MME extensions easily.
 *         MSVC and Intel compilers come with this class built-in, but not
 *         gcc.  Do not use this file with those compilers as their built-in
 *         classes are likely better than this.
 * 
 */

#ifndef ext_fvec_h
#define ext_fvec_h

#if defined(WIN32) || defined(WIN64)
#  error Please don't include ext/fvec.h for Windows platforms.
#endif

#include "xmmintrin.h"


class F32vec4
{
protected:

union {
  __m128  vec;
  float  f[4];
};


public:

  F32vec4() :
    vec( _mm_setzero_ps() )
  {
  }

  F32vec4( const __m128& b ) : vec(b) {}

  F32vec4( const float a ) :
    vec( _mm_set_ps( a, a, a, a ) )
  {
  }

  F32vec4( const float a[4] ) :
    vec( _mm_loadu_ps( a ) )
  {
  }

  F32vec4( const float a, const float b, const float c, const float d ) :
    vec( _mm_set_ps( a, b, c, d ) )
  {
  }


  F32vec4( const F32vec4& b ) : vec( b.vec ) {}

  operator __m128() const { return vec; }


  inline float    operator[]( int x )
  {
    return f[x];
  }

  inline F32vec4  operator^=( const F32vec4& b )
  {
    vec = _mm_xor_ps( vec, b.vec );
    return *this;
  }

  inline F32vec4  operator|=( const F32vec4& b )
  {
    vec = _mm_or_ps( vec, b.vec );
    return *this;
  }

  inline F32vec4  operator&=( const F32vec4& b )
  {
    vec = _mm_and_ps( vec, b.vec );
    return *this;
  }

  inline F32vec4& operator+=( const F32vec4& b )
  {
    vec = _mm_add_ps( vec, b.vec );
    return *this;
  }

  inline F32vec4& operator-=( const F32vec4& b )
  {
    vec = _mm_sub_ps( vec, b.vec );
    return *this;
  }

  inline F32vec4& operator*=( const F32vec4& b )
  {
    vec = _mm_mul_ps( vec, b.vec );
    return *this;
  }

  inline F32vec4& operator/=( const F32vec4& b )
  {
    vec = _mm_div_ps( vec, b.vec );
    return *this;
  }



  inline F32vec4  operator^( const F32vec4& b )
  {
    return _mm_xor_ps( vec, b.vec );
  }

  inline F32vec4  operator|( const F32vec4& b )
  {
    return _mm_or_ps( vec, b.vec );
  }

  inline F32vec4  operator&( const F32vec4& b )
  {
    return _mm_and_ps( vec, b.vec );
  }

  inline F32vec4 operator+( const F32vec4& b )
  {
    return _mm_add_ps( vec, b.vec );
  }

  inline F32vec4 operator-( const F32vec4& b )
  {
    return _mm_sub_ps( vec, b.vec );
  }

  inline F32vec4 operator*( const F32vec4& b )
  {
    return _mm_mul_ps( vec, b.vec );
  }

  inline F32vec4 operator/( const F32vec4& b )
  {
    return _mm_div_ps( vec, b.vec );
  }


};


#define F32vec4_select(op) \
inline F32vec4 select_##op( const F32vec4& a, const F32vec4& b, \
			    const F32vec4& c, const F32vec4& d) \
{ \
  F32vec4 mask = _mm_cmp##op##_ps(a,b); \
  return( (mask & c ) | F32vec4( _mm_andnot_ps(mask,d) ) ); \
}


F32vec4_select(eq);
F32vec4_select(neq);
F32vec4_select(lt);
F32vec4_select(le);
F32vec4_select(gt);
F32vec4_select(ge);
F32vec4_select(ord);
F32vec4_select(unord);





class F32vec1
{
protected:

union {
  __m128  vec;
  float   f[4];
};


public:

  F32vec1() :
    vec( _mm_setzero_ps() )
  {
  }

  F32vec1( const __m128& b ) : vec(b) {}

  F32vec1( const float a ) :
    vec( _mm_load_ss( &a ) )
  {
  }


  F32vec1( const F32vec1& b ) : vec( b.vec ) {}

  operator __m128() const { return vec; }


  inline float    operator[]( int x )
  {
    return f[x];
  }

  inline F32vec1  operator^=( const F32vec1& b )
  {
    vec = _mm_xor_ps( vec, b.vec );
    return *this;
  }

  inline F32vec1  operator|=( const F32vec1& b )
  {
    vec = _mm_or_ps( vec, b.vec );
    return *this;
  }

  inline F32vec1  operator&=( const F32vec1& b )
  {
    vec = _mm_and_ps( vec, b.vec );
    return *this;
  }

  inline F32vec1& operator+=( const F32vec1& b )
  {
    vec = _mm_add_ss( vec, b.vec );
    return *this;
  }

  inline F32vec1& operator-=( const F32vec1& b )
  {
    vec = _mm_sub_ss( vec, b.vec );
    return *this;
  }

  inline F32vec1& operator*=( const F32vec1& b )
  {
    vec = _mm_mul_ss( vec, b.vec );
    return *this;
  }

  inline F32vec1& operator/=( const F32vec1& b )
  {
    vec = _mm_div_ss( vec, b.vec );
    return *this;
  }



  inline F32vec1  operator^( const F32vec1& b )
  {
    return _mm_xor_ps( vec, b.vec );
  }

  inline F32vec1  operator|( const F32vec1& b )
  {
    return _mm_or_ps( vec, b.vec );
  }

  inline F32vec1  operator&( const F32vec1& b )
  {
    return _mm_and_ps( vec, b.vec );
  }

  inline F32vec1 operator+( const F32vec1& b )
  {
    return _mm_add_ss( vec, b.vec );
  }

  inline F32vec1 operator-( const F32vec1& b )
  {
    return _mm_sub_ss( vec, b.vec );
  }

  inline F32vec1 operator*( const F32vec1& b )
  {
    return _mm_mul_ss( vec, b.vec );
  }

  inline F32vec1 operator/( const F32vec1& b )
  {
    return _mm_div_ss( vec, b.vec );
  }


}; // class F32vec1


#define F32vec1_select(op) \
inline F32vec1 select_##op( const F32vec1& a, const F32vec1& b, \
			    const F32vec1& c, const F32vec1& d) \
{ \
  F32vec1 mask = _mm_cmp##op##_ss(a,b); \
  return( (mask & c ) | F32vec1( _mm_andnot_ps(mask,d) ) ); \
}


F32vec1_select(eq);
F32vec1_select(neq);
F32vec1_select(lt);
F32vec1_select(le);
F32vec1_select(gt);
F32vec1_select(ge);
F32vec1_select(ord);
F32vec1_select(unord);

inline F32vec1 rcp_nr( const F32vec1& a )
{
  return _mm_rcp_ss( a );
}



#endif // fvec_h
