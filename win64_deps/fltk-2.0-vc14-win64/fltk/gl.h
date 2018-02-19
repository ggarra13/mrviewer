// "$Id: gl.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
// Copyright 1998-2006 by Bill Spitzak and others.
//
// You must include this instead of GL/gl.h to get the Microsoft
// APIENTRY stuff included (from <windows.h>) prior to the OpenGL
// header files.
//
// This file also provides "missing" OpenGL functions, and
// gl_start() and gl_finish() to allow OpenGL to be used in any window
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

/*! \file
  Portably include the OpenGL header files, and define a few OpenGL
  drawing functions provided by fltk. You may want to use the
  OpenGL Extension Wrangler (glew), which will make it much easier
  to call modern OpenGL extensions. If so, include glew.h before
  this file, or define USE_GLEW to 1 before including this.
*/

#ifndef gl_draw_H
#define gl_draw_H

#ifndef DOXYGEN
#include "Color.h"
#include "Flags.h"

#if USE_GLEW
# include <GL/glew.h>
#elif defined(__GLEW_H__)
  /* do nothing if they included glew.h */
#else

# define GL_GLEXT_PROTOTYPES 1
# ifdef _WIN32
#  include <windows.h>
#  undef OPAQUE
#  undef DELETE
#  undef ERROR
#  undef IN
#  undef OUT
#  undef POINT
#  undef far
#  undef max
#  undef min
#  undef near
#  include <GL/gl.h>
# elif defined(__APPLE__)
#  ifndef APIENTRY
#   define APIENTRY
#  endif
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif

# if !defined(GL_VERSION_1_4) || defined(DOXYGEN)
FL_GL_API void glWindowPos2i(int x, int y);
# endif

#endif
#endif

namespace fltk {

struct Font;

/// \name fltk/gl.h
//@{

FL_GL_API void glstart();
FL_GL_API void glfinish();

FL_GL_API void glsetcolor(Color);

FL_GL_API void glstrokerect(int x,int y,int w,int h);
inline void glfillrect(int x,int y,int w,int h) {glRecti(x,y,x+w,y+h);}

FL_GL_API void glsetfont(Font* f, float size);
FL_GL_API float glgetascent();
FL_GL_API float glgetdescent();
FL_GL_API float glgetwidth(const char *);
FL_GL_API float glgetwidth(const char *, int n);

FL_GL_API void gldrawtext(const char*);
FL_GL_API void gldrawtext(const char*, int n);
FL_GL_API void gldrawtext(const char*, float x, float y, float z = 0);
FL_GL_API void gldrawtext(const char*, int n, float x, float y, float z = 0);

FL_GL_API void gldrawimage(const uchar *, int x,int y,int w,int h, int d=3, int ld=0);

//@}

}
#endif

//
// End of "$Id: gl.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
