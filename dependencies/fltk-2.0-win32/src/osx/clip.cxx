//
// "$Id: $"
//
// The fltk graphics clipping stack.  These routines are always
// linked into an fltk program.
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

#include <fltk/Window.h>
#include <fltk/draw.h>
#include <fltk/x.h>
#include <fltk/string.h>
#include <stdlib.h>
using namespace fltk;

#define Region RgnHandle

static Region emptyrstack = 0;
static Region* rstack = &emptyrstack;
static int rstacksize = 0;
static int rstackptr = 0;

static inline void pushregion(Region r) {
  if (rstackptr >= rstacksize) {
    if (rstacksize) {
      rstacksize = 2*rstacksize;
      rstack = (Region*)realloc(rstack, rstacksize*sizeof(Region));
    } else {
      rstacksize = 16;
      rstack = (Region*)malloc(rstacksize*sizeof(Region));
      rstack[0] = 0;
    }
  }
  rstack[++rstackptr] = r;
}

int fl_clip_state_number = 0; // used by code that needs to update clip regions

/*! Return the current region as a system-specific structure. You must
  include <fltk/x.h> to use this. Returns null if there is no clipping.
*/
Region fltk::clip_region() {
  return rstack[rstackptr];
}

// Make the system's clip match the top of the clip stack.  This can
// be used after changing the stack, or to undo any clobbering of clip
// done by your program:
void fl_restore_clip() {
  Region r = rstack[rstackptr];
  fl_clip_state_number++;
  if ( quartz_window ) {
    GrafPtr port = GetWindowPort( quartz_window );
    if ( port ) { 
      RgnHandle portClip = NewRgn();
      CopyRgn( CreatedWindow::find(Window::drawing_window())->subRegion, 
               portClip ); // changed
      if ( r )
        SectRgn( portClip, r, portClip );
      Rect portRect; GetPortBounds(port, &portRect);
      fltk::clear_quartz_clipping();
      ClipCGContextToRegion(quartz_gc, &portRect, portClip );
      fltk::fill_quartz_context();
      DisposeRgn( portClip );
    }
  }
}

/*! Replace the top of the clip stack. */
void fltk::clip_region(Region r) {
  Region oldr = rstack[rstackptr];
  if (oldr) DisposeRgn(oldr);
  rstack[rstackptr] = r;
  fl_restore_clip();
}
// fabien : this code would merit inlining wouldn't it ?
void fltk::push_clip(const Rectangle& r) {
  push_clip(r.x(), r.y(), r.w(), r.h());
}

/*!
  Pushes the \e intersection of the current region and this rectangle
  onto the clip stack. */
void fltk::push_clip(int x, int y, int w, int  h) {
 #if USE_CAIRO
    transform(x,y);
    cairo_rectangle(cr, x,y,w,h);
    //cairo_stroke(cr);
    cairo_clip(cr);
#else
 Region region = NewRgn();
  if (FLTK_RECT_EMPTY(w,h)) {
    SetEmptyRgn(region);
  } else {
    Region current = rstack[rstackptr];
    transform(x, y); // x,y absolute coords lazy evaluation
    SetRectRgn(region, x, y, w+x, h+y);
    if (current) SectRgn(region, current, region);
  }
  pushregion(region);
  fl_restore_clip();
#endif
}

/*!
  Remove the rectangle from the current clip region, thus making it a
  more complex shape. This does not push the stack, it just replaces
  the top of it.

  Some graphics backends (OpenGL and Cairo, at least) do not support
  non-rectangular clip regions. This call does nothing on those.
*/
void fltk::clipout(const Rectangle& r1) {
  if (r1.empty()) return; // do not even construct r if r1 is empty
  Rectangle r; transform(r1, r);
  Region current = rstack[rstackptr];
  if (!current) {current = NewRgn(); SetRectRgn(current, 0,0,16383,16383);}
  Region region = NewRgn();
  SetRectRgn(region, r.x(), r.y(), r.r(), r.b());
  DiffRgn(current, region, current);
  fl_restore_clip();
}

/*!
  Pushes an empty clip region on the stack so nothing will be
  clipped. This lets you draw outside the current clip region. This should
  only be used to temporarily ignore the clip region to draw into
  an offscreen area.
*/
void fltk::push_no_clip() {
  pushregion(0);
  fl_restore_clip();
}

/*! 
  Restore the previous clip region. You must call fltk::pop_clip()
  exactly once for every time you call fltk::push_clip(). If you return to
  FLTK with the clip stack not empty unpredictable results occur.
*/
void fltk::pop_clip() {
#if USE_CAIRO
 cairo_reset_clip(cr);
#else
  if (rstackptr > 0) {
    Region oldr = rstack[rstackptr--];
    if (oldr) DisposeRgn(oldr);
    fl_restore_clip();
  }
#endif
}

////////////////////////////////////////////////////////////////
// clipping tests:

/*! Returns true if any or all of the Rectangle is inside the
  clip region.
*/
bool fltk::not_clipped(const Rectangle& r1) {
  Rectangle r; transform(r1, r);
  // first check against the window so we get rid of coordinates
  // outside the 16-bit range the X/Win32 calls take:
  if (r.r() <= 0 || r.b() <= 0 || r.x() >= Window::drawing_window()->w()
      || r.y() >= Window::drawing_window()->h()) return false;
  Region region = rstack[rstackptr];
  if (!region) return true;
  Rect rect;
  rect.left = r.x(); rect.top = r.y(); rect.right = r.r(); rect.bottom = r.b();
  return RectInRgn(&rect, region);
}

/*!
  Intersect a \e transform()'d rectangle with the current clip
  region and change it to the smaller rectangle that surrounds (and
  probably equals) this intersection area.

  This can be used by device-specific drawing code to limit complex pixel
  operations (like drawing images) to the smallest rectangle needed to
  update the visible area.

  Return values:
  - 0 if it does not intersect, and W and H are set to zero.
  - 1 if if the result is equal to the rectangle (i.e. it is
    entirely inside or equal to the clip region)
  - 2 if it is partially clipped.
*/
int fltk::intersect_with_clip(Rectangle& r) {
  Region region = rstack[rstackptr];
  if (!region) return 1;
  // Test against the window to get 16-bit values:
  int ret = 1;
  if (r.x() < 0) {r.set_x(0); ret = 2;}
  int t = Window::drawing_window()->w(); if (r.r() > t) {r.set_r(t); ret = 2;}
  if (r.y() < 0) {r.set_y(0); ret = 2;}
  t = Window::drawing_window()->h(); if (r.b() > t) {r.set_b(t); ret = 2;}
  // check for total clip (or for empty rectangle):
  if (r.empty()) return 0;
  RgnHandle rr = NewRgn();
  SetRectRgn(rr, r.x(), r.y(), r.r(), r.b());
  SectRgn(region, rr, rr);
  Rect rp; GetRegionBounds(rr, &rp);
  if (rp.bottom <= rp.top) ret = 0;
  else if (rp.right-rp.left < r.w() || rp.bottom-rp.top < r.h()) ret = 2;
  r.set(rp.left, rp.top, rp.right-rp.left, rp.bottom-rp.top);
  DisposeRgn(rr);
  return ret;
}

//
// End of "$Id: $"
//
