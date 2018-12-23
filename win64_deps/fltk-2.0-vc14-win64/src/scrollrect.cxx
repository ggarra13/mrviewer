//
// "$Id: scrollrect.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Scrolling routines for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <fltk/Window.h>
#include <fltk/x.h>
#include <fltk/draw.h>

// Turn this off to stop using copy-area for scrolling:
#define USE_SCROLL 1

#if USE_SCROLL && defined(_WIN32)

#ifndef SYSRGN
// Missing declaration in old WIN32 API headers.
// However, GDI32.dll exports this function since Windows 95
extern "C" {
  WINGDIAPI int WINAPI GetRandomRgn(HDC  hdc,    // handle to DC
				    HRGN hrgn,   // handle to region
				    INT  iNum    // must be SYSRGN
				    );
}
#define SYSRGN 4

#endif /* SYSRGN */

extern int has_unicode();

// Return true if rect is completely visible on screen.
// If other window is overlapping rect, return false.
static bool is_visible(int x, int y, int w, int h)
{
  // Get visible region of window
  HRGN rgn0 = CreateRectRgn (0, 0, 0, 0);

  // Copy system clipping region from fltk::dc
  GetRandomRgn (fltk::dc, rgn0, SYSRGN);

  if (!has_unicode()) {
    // if not unicode then we are running on Win9x and the following applies:
    // Windows 9x operating systems the region is returned in window coordinates,
    // and on Windows XP/2k machines the region is in screen coordinates.. SIGH!
    POINT pt = { 0, 0 };
    ClientToScreen(fltk::xid(fltk::Window::drawing_window()), &pt);
    OffsetRgn(rgn0, -pt.x, -pt.y);
  }

  HRGN rect = CreateRectRgn(x,y,x+w,y+h);
  HRGN temp = CreateRectRgn(0,0,0,0);

  bool ret;
  if(CombineRgn(temp, rect, rgn0, RGN_AND) == NULLREGION) {
    ret = false;
  } else if (EqualRgn(temp, rect)) {
    ret = true;
  } else {
    ret = false;
  }

  // Free resources
  DeleteObject(rect);
  DeleteObject(temp);
  DeleteObject(rgn0);

  return ret;
}

#endif /* _WIN32 */

/**
  Move the contents of a rectangle by \a dx and \a dy. The area that
  was previously outside the rectangle or obscured by other windows is
  then redrawn by calling \a draw_area for each rectangle.  <i>This is a
  drawing function and can only be called inside the draw() method of
  a widget.</i>

  If \a dx and \a dy are zero this returns without doing anything.

  If \a dx or \a dy are larger than the rectangle then this just calls
  \a draw_area for the entire rectangle. This is also done on systems
  (Quartz) that do not support copying screen regions.
*/
void fltk::scrollrect(const Rectangle& r, int dx, int dy,
		       void (*draw_area)(void*, const Rectangle&), void* data)
{
  if (!dx && !dy) return;
#if !USE_SCROLL || defined(USE_QUARTZ)
  draw_area(data, r);
  return;
#else
  if (dx <= -r.w() || dx >= r.w() || dy <= -r.h() || dy >= r.h()) {
    // no intersection of old an new scroll
    draw_area(data, r);
    return;
  }
  int src_x, src_w, dest_x, clip_x, clip_w;
  if (dx > 0) {
    src_x = r.x();
    dest_x = r.x()+dx;
    src_w = r.w()-dx;
    clip_x = r.x();
    clip_w = dx;
  } else {
    src_x = r.x()-dx;
    dest_x = r.x();
    src_w = r.w()+dx;
    clip_x = r.x()+src_w;
    clip_w = r.w()-src_w;
  }
  int src_y, src_h, dest_y, clip_y, clip_h;
  if (dy > 0) {
    src_y = r.y();
    dest_y = r.y()+dy;
    src_h = r.h()-dy;
    clip_y = r.y();
    clip_h = dy;
  } else {
    src_y = r.y()-dy;
    dest_y = r.y();
    src_h = r.h()+dy;
    clip_y = r.y()+src_h;
    clip_h = r.h()-src_h;
  }
  int ox = 0; int oy = 0; transform(ox, oy);
#if USE_X11
  XCopyArea(xdisplay, xwindow, xwindow, gc,
	    src_x+ox, src_y+oy, src_w, src_h,
	    dest_x+ox, dest_y+oy);
// Synchronous update by waiting for graphics expose events:
  for (;;) {
    XEvent e; XWindowEvent(xdisplay, xwindow, ExposureMask, &e);
    if (e.type == NoExpose) break;
    // otherwise assumme it is a GraphicsExpose event:
    draw_area(data,
	      Rectangle(e.xexpose.x-ox, e.xexpose.y-oy,
			e.xexpose.width, e.xexpose.height));
    if (!e.xgraphicsexpose.count) break;
  }
#elif defined(_WIN32)
  if (is_visible(src_x+ox, src_y+oy, src_w, src_h)) {
    BitBlt(dc, dest_x+ox, dest_y+oy, src_w, src_h,
	   dc, src_x+ox, src_y+oy, SRCCOPY);
  } else {
    // Window overlapping scroll area.
    // Best we can do right now, is just redraw whole scroll area.
    draw_area(data, r);
    return;
  }
#else
# error
#endif
  if (dx) draw_area(data, Rectangle(clip_x, dest_y, clip_w, src_h));
  if (dy) draw_area(data, Rectangle(r.x(), clip_y, r.w(), clip_h));
#endif
}

//
// End of "$Id: scrollrect.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
