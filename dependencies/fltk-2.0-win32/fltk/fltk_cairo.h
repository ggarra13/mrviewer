// fltk_cairo.h
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

/** \file
  "Portably" include cairo common definitions for fltk. If fltk is
  compiled correctly, you can create a cairo "surface" from an fltk
  Window and then make your own cairo context to draw into it.

  FLTK may also be compiled to use cairo for \e all it's drawing, by
  adding --enable_cairo when running ./configure. In this case this
  has already been done when draw() is called, and the cairo context
  is in fltk::cc.
*/

#ifndef fltk_cairo_h
#define fltk_cairo_h

#include <fltk/FL_API.h>
#include <cairo.h>

namespace fltk {
  extern FL_API cairo_t * cr;
  class Window;
  FL_API cairo_surface_t * cairo_create_surface(Window* w);
}

#endif

// End of fltk_cairo.h

