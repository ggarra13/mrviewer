// "$Id: clip.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <config.h>

#if USE_QUARTZ
# include "osx/clip.cxx"
#else

#include <fltk/Window.h>
#include <fltk/draw.h>
#include <fltk/x.h>
#include <fltk/string.h>
#include <stdlib.h>
using namespace fltk;

#if USE_X11
// Region == Region
#elif defined(_WIN32)
# define Region HRGN
#endif

static Region emptyrstack = 0;
static Region* rstack = &emptyrstack;
static int rstacksize = 0;
static int rstackptr = 0;

static inline void pushregion(Region r) {
  if (rstackptr >= rstacksize) {
    if (rstacksize) {
      rstacksize = 2*rstacksize;
      Region* newstack = new Region[rstacksize];
      memcpy(newstack, rstack, sizeof(Region)*rstackptr);
      delete[] rstack;
      rstack = newstack;
    } else {
      rstacksize = 16;
      rstack = new Region[rstacksize];
      rstack[0] = 0;
    }
  }
  rstack[++rstackptr] = r;
}

int fl_clip_state_number = 0; // used by code that needs to update clip regions

/**
  Return the current region as a system-specific structure. You must
  include <fltk/x.h> to use this. Returns null if there is no clipping.
*/
Region fltk::clip_region() {
  return rstack[rstackptr];
}

#if USE_X11 || defined(DOXYGEN)
/**
Missing X call, this creates and returns a Region with a single rectangle.
*/
Region XRectangleRegion(int x, int y, int w, int h) {
  XRectangle R;
  R.x = x; R.y = y; R.width = w; R.height = h;
  Region r = XCreateRegion();
  XUnionRectWithRegion(&R, r, r);
  return r;
}
#endif

// Make the system's clip match the top of the clip stack.  This can
// be used after changing the stack, or to undo any clobbering of clip
// done by your program:
void fl_restore_clip() {
  Region r = rstack[rstackptr];
  fl_clip_state_number++;
#if USE_CAIRO
#elif USE_X11
  if (r) XSetRegion(xdisplay, gc, r);
  else XSetClipMask(xdisplay, gc, 0);
#if USE_XFT
  if (xftc) {
    XftDrawSetClip(xftc, r);
  }
#endif
#elif defined(_WIN32)
  SelectClipRgn(dc, r); //if r is NULL, clip is automatically cleared
#else
# error
#endif
}

/** Replace the top of the clip stack. */
void fltk::clip_region(Region region) {
  Region oldr = rstack[rstackptr];
#if USE_X11
  if (oldr) XDestroyRegion(oldr);
#elif defined(_WIN32)
  if (oldr) DeleteObject(oldr);
#endif
  rstack[rstackptr] = region;
#if !USE_CAIRO
  fl_restore_clip();
#endif
}

/**
  Pushes the \e intersection of the current region and \a r
  onto the clip stack.
*/
// fabien : this code is small enough to merit an inline declaration, isn't it ?
void fltk::push_clip(const Rectangle& r) {
  push_clip(r.x(), r.y(), r.w(), r.h());
}

/**
  Same as push_clip(Rectangle(x,y,r,h)) except faster as it avoids the
  construction of an intermediate rectangle object.
*/
void fltk::push_clip(int x, int y, int w, int h) {
  // Rectangle r; transform(x,y,w,h,r);
  // no rectangle creation needed, use the direct FLTK_RECT predicates
  // when dealing with x,y,w,h scalars evaluation in frequently used code
  // Here XRectangleRegion() as well as CreateRectRgn() both use scalars so let's
  // try not to build a Rectangle object that is not necessary:
  Region region;
  if (FLTK_RECT_EMPTY(w,h)) {
# if USE_X11
    region = XCreateRegion();
# elif defined(_WIN32)
    region = CreateRectRgn(0,0,0,0);
# else
#  error
# endif
  } else {
    Region current = rstack[rstackptr];
    transform(x, y); // absolute coordinates lazy evaluation, only when really needed
# if USE_X11
    region = XRectangleRegion(x, y, w, h);
    if (current) {
      Region temp = XCreateRegion();
      XIntersectRegion(current, region, temp);
      XDestroyRegion(region);
      region = temp;
    }
# elif defined(_WIN32)
    region = CreateRectRgn(x, y, w+x, h+y);
    if (current) CombineRgn(region, region, current, RGN_AND);
# else
#  error
# endif
  }
  pushregion(region);
#if USE_CAIRO
    //transform(x,y);
    // fabien: FIXME! should be able to clip the current region not only a rect!
    cairo_rectangle(cr, x,y,w,h);
    cairo_clip(cr); // should accumulate clip depending on the stack!
#else
  fl_restore_clip();
#endif
}

/**
  Remove \a rectangle from the current clip region, thus making it a
  more complex shape. This does not push the stack, it just replaces
  the top of it.

  Some graphics backends (OpenGL and Cairo, at least) do not support
  non-rectangular clip regions. This call does nothing on those.
*/
void fltk::clipout(const Rectangle& rectangle) {
  Rectangle r; transform(rectangle, r);
  if (r.empty()) return;
#if USE_X11
  Region current = rstack[rstackptr];
  if (!current) current = XRectangleRegion(0,0,16383,16383);//?
  Region region = XRectangleRegion(r.x(), r.y(), r.w(), r.h());
  Region temp = XCreateRegion();
  XSubtractRegion(current, region, temp);
  XDestroyRegion(region);
  XDestroyRegion(current);
  rstack[rstackptr] = temp;
#elif defined(_WIN32)
  Region current = rstack[rstackptr];
  if (!current) current = CreateRectRgn(0,0,16383,16383);
  Region region = CreateRectRgn(r.x(), r.y(), r.r(), r.b());
  CombineRgn(current, current, region, RGN_DIFF);
  DeleteObject(region);
#endif
#if !USE_CAIRO
  fl_restore_clip();
#endif
}

/**
  Pushes an empty clip region on the stack so nothing will be
  clipped. This lets you draw outside the current clip region. This should
  only be used to temporarily ignore the clip region to draw into
  an offscreen area.
*/
void fltk::push_no_clip() {
  pushregion(0);
#if !USE_CAIRO
  fl_restore_clip();
#else
  cairo_reset_clip(cr);
#endif
}

/**
  Restore the previous clip region. You must call fltk::pop_clip()
  exactly once for every time you call fltk::push_clip(). If you return to
  FLTK with the clip stack not empty unpredictable results occur.
*/
void fltk::pop_clip() {
  if (rstackptr > 0) {
    Region oldr = rstack[rstackptr--];
# if USE_X11
    if (oldr) XDestroyRegion(oldr);
# elif defined(_WIN32)
    if (oldr) DeleteObject(oldr);
# endif
#if USE_CAIRO
     cairo_reset_clip(cr);
#else
    fl_restore_clip();
#endif
  }
}

////////////////////////////////////////////////////////////////
// clipping tests:

extern int fl_clip_w, fl_clip_h;

/**
  Returns true if any or all of \a rectangle is inside the clip region.
*/
bool fltk::not_clipped(const Rectangle& rectangle) {
  Rectangle r; transform(rectangle,r);
  // first check against the window so we get rid of coordinates
  // outside the 16-bit range the X/Win32 calls take:
  if (r.r() <= 0 || r.b() <= 0 || r.x() >= fl_clip_w || r.y() >= fl_clip_h)
    return false;
  Region region = rstack[rstackptr];
  if (!region) return true;
#if USE_X11
  return XRectInRegion(region, r.x(), r.y(), r.w(), r.h());
#elif defined(_WIN32)
  RECT rect;
  rect.left = r.x(); rect.top = r.y(); rect.right = r.r(); rect.bottom = r.b();
  return RectInRegion(region,&rect) != 0;
#endif
}

/**
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
  // Test against the window to get 16-bit values:
  int ret = 1;
  if (r.x() < 0) {r.set_x(0); ret = 2;}
  int t = fl_clip_w; if (r.r() > t) {r.set_r(t); ret = 2;}
  if (r.y() < 0) {r.set_y(0); ret = 2;}
  t = fl_clip_h; if (r.b() > t) {r.set_b(t); ret = 2;}
  // check for total clip (or for empty rectangle):
  if (r.empty()) return 0;
  if (!region) return ret;
#if USE_X11
  switch (XRectInRegion(region, r.x(), r.y(), r.w(), r.h())) {
  case 0: // completely outside
    r.set(0,0,0,0);
    return 0;
  case 1: // completely inside:
    return ret;
  default: { // partial:
    Region rr = XRectangleRegion(r.x(), r.y(), r.w(), r.h());
    Region temp = XCreateRegion();
    XIntersectRegion(region, rr, temp);
    XRectangle xr;
    XClipBox(temp, &xr);
    r.set(xr.x, xr.y, xr.width, xr.height);
    XDestroyRegion(temp);
    XDestroyRegion(rr);
    return 2;}
  }
#elif defined(_WIN32)
// The win32 API makes no distinction between partial and complete
// intersection, so we have to check for partial intersection ourselves.
// However, given that the regions may be composite, we have to do
// some voodoo stuff...
  Region rr = CreateRectRgn(r.x(), r.y(), r.r(), r.b());
  Region temp = CreateRectRgn(0,0,0,0);
  if (CombineRgn(temp, rr, region, RGN_AND) == NULLREGION) { // disjoint
    r.set(0,0,0,0);
    ret = 0;
  } else if (EqualRgn(temp, rr)) { // complete
    // ret = ret
  } else {	// parital intersection
    RECT xr;
    GetRgnBox(temp, &xr);
    r.set(xr.left, xr.top, xr.right-xr.left, xr.bottom-xr.top);
    ret = 2;
  }
  DeleteObject(temp);
  DeleteObject(rr);
  return ret;
#else
# error
#endif
}

#endif

//
// End of "$Id: clip.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
