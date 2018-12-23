//
// "$Id: overlay_rect.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Overlay support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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

// Extremely limited "overlay" support.  You can use this to drag out
// a rectangle in response to mouse events.  It is your responsibility
// to erase the overlay before drawing anything that might intersect
// it.

#include <config.h>
#include <fltk/x.h>
#include <fltk/draw.h>
using namespace fltk;

static fltk::Rectangle pr(0,0,0,0);

static void draw_current_rect() {
  if (pr.empty()) return;
#if USE_X11
  XSetFunction(xdisplay, gc, GXxor);
  XSetForeground(xdisplay, gc, 0xffffffff);
  strokerect(pr);
  XSetFunction(xdisplay, gc, GXcopy);
#elif defined(_WIN32)
  int old = SetROP2(dc, R2_NOT);
  strokerect(pr);
  SetROP2(dc, old);
#elif defined(__APPLE__)
  PenMode( patXor );
  strokerect(pr);
  PenMode( patCopy );
#endif
}

void fltk::overlay_clear() {
  if (!pr.empty()) {draw_current_rect(); pr.w(0);}
}

void fltk::overlay_rect(int x, int y, int w, int h) {
  if (w < 0) {x += w; w = -w;} else if (!w) w = 1;
  if (h < 0) {y += h; h = -h;} else if (!h) h = 1;
  if (!pr.empty()) {
    if (x==pr.x() && y==pr.y() && w==pr.w() && h==pr.h()) return;
    draw_current_rect();
  }
  pr.set(x,y,w,h);
  draw_current_rect();
}

//
// End of "$Id: overlay_rect.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
