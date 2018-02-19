// "$Id: FL_API.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// This file is included by all FLTK header files. Originally it was to
// get that damn dllimport/export stuff on Windows. It now also turns
// off warnings on Windows so that you can use stdc functions, and
// defines the uchar type that is used by FLTK a lot.
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
#ifndef FL_API

#if _MSC_VER > 1000
# pragma once
// suppress vc2005 warnings: 
// C4312 because we have no problem to cast a smaller type to a greater (potentially 64bit) one
// and C4996 (deprecated C-lib calls like strcpy that should be _strcpy, etc ...)
# pragma warning(disable:4312 4996)
#endif

#if !defined(DOXYGEN) && defined(_WIN32) && defined(FL_SHARED)
#  ifdef FL_LIBRARY
#    define FL_API	__declspec(dllexport)
#  else
#    define FL_API	__declspec(dllimport)
#  endif
#  ifdef FL_IMAGES_LIBRARY
#    define FL_IMAGES_API	__declspec(dllexport)
#  else
#    define FL_IMAGES_API	__declspec(dllimport)
#  endif
#  ifdef FL_GLUT_LIBRARY
#    define FL_GLUT_API		__declspec(dllexport)
#  else
#    define FL_GLUT_API		__declspec(dllimport)
#  endif
#  ifdef FL_FORMS_LIBRARY
#    define FL_FORMS_API	__declspec(dllexport)
#  else
#    define FL_FORMS_API	__declspec(dllimport)
#  endif
#  ifdef FL_GL_LIBRARY
#    define FL_GL_API		__declspec(dllexport)
#  else
#    define FL_GL_API		__declspec(dllimport)
#  endif
#else
#  define FL_API
#  define FL_IMAGES_API
#  define FL_GLUT_API
#  define FL_FORMS_API
#  define FL_GL_API
#endif

typedef unsigned char uchar;

#endif

//
// End of "$Id: FL_API.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
