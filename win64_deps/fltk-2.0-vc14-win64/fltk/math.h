//
// "$Id: math.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// The purpose of this header file is to make math.h look the same as
// Unix on other operating systems.
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

#ifndef fl_math_h
#define fl_math_h

#include <math.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
/* things missing from <math.h> on Windows: */
# include <float.h>

# ifndef M_PI
#  define M_PI		3.14159265358979323846
#  define M_PI_2	1.57079632679489661923
#  define M_PI_4	0.78539816339744830962
#  define M_1_PI	0.31830988618379067154
#  define M_2_PI	0.63661977236758134308
#  define M_SQRT2	1.41421356237309504880
#  define M_SQRT1_2	0.70710678118654752440
# endif

# define rint(v)	floor((v)+.5)
# define copysign	_copysign
# define drand48()	((double)rand()/RAND_MAX)
# define srand48(n)	srand((n));

#endif

#ifdef __EMX__
# include <float.h>
#endif

// define missing 'f' versions of functions:
#if 1 // All systems seem to be missing rintf:
# define rintf(v)	floorf((v)+.5f)
#endif
#if defined(__APPLE__) || defined(__sun__) || defined(__BORLANDC__)
# define floorf(a)	((float)floor(a))
# define ceilf(a)	((float)ceil(a))
# define fmodf(a,b)	((float)fmod(a,b))
# undef fabsf
# define fabsf(a)	((float)fabs(a))
# define sinf(a)	((float)sin(a))
# define cosf(a)	((float)cos(a))
# define tanf(a)	((float)tan(a))
# define asinf(a)	((float)asin(a))
# define acosf(a)	((float)acos(a))
# define atanf(a)	((float)atan(a))
# define atan2f(a,b)	((float)atan2(a,b))
# define expf(a)	((float)exp(a))
# define logf(a)	((float)log(a))
# define log10f(a)	((float)log10(a))
# undef sqrtf
# define sqrtf(a)	((float)sqrt(a))
#endif
#ifdef __alpha // powf is broken on alphas, at least in gcc
# define powf(a,b)	((float)pow(a,b))
#endif
#ifdef _WIN32
# define expm1f(a)	((float)expm1(a))
# define log1pf(a)	((float)log1p(a))
#endif

#endif

//
// End of "$Id: math.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
