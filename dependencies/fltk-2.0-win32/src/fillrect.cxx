//
// "$Id: fillrect.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $"
//
// Non-path routines from draw.h that are used by the standard boxtypes
// and thus are always linked into an fltk program.
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
#include <fltk/draw.h>
#include <fltk/x.h>
#include <fltk/math.h>
using namespace fltk;

/*! Fill the rectangle with the current color. */
void fltk::fillrect(int x, int y, int w, int h) {
  if (getcolor() < 0) return; 
  if (w <= 0 || h <= 0) return;
  transform(x,y,w,h);
#if USE_CAIRO
  cairo_rectangle(cr,x,y,w,h);
  cairo_fill(cr);
#elif USE_X11
  if (w && h) XFillRectangle(xdisplay, xwindow, gc, x, y, w, h);
#elif defined(_WIN32)
  RECT rect;
  rect.left = x; rect.top = y;  
  rect.right = x+w; rect.bottom = y+h;
  SetBkColor(dc, current_xpixel);
  ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
#elif USE_QUARTZ
  if (!line_width_) CGContextSetShouldAntialias(quartz_gc, false);
  CGRect rect = CGRectMake(x, y, w-1, h-1);
  CGContextFillRect(quartz_gc, rect);
  if (!line_width_) CGContextSetShouldAntialias(quartz_gc, true);
#else
# error
#endif
}

/*!
  Draw a line \e inside this bounding box (currently correct only for
  0-thickness lines).
*/
void fltk::strokerect(int x, int y, int w, int h) {
  if (w <= 0 || h <= 0) return;
  transform(x,y,w,h);
#if USE_CAIRO
  cairo_rectangle(cr,x+.5,y+.5,w-1,h-1);
  cairo_stroke(cr);
#elif USE_X11
  if (w && h) XDrawRectangle(xdisplay, xwindow, gc, x, y, w-1, h-1);
  else XDrawLine(xdisplay, xwindow, gc, x, y, x+w, y+h);
#elif defined(_WIN32)
  setpen();
  POINT pts[4];
  pts[0].x = pts[3].x = x;
  pts[0].y = pts[1].y = y;
  pts[1].x = pts[2].x = x+w-1;
  pts[2].y = pts[3].y = y+h-1;
  static const BYTE i[4] =
  {PT_MOVETO, PT_LINETO, PT_LINETO, PT_LINETO|PT_CLOSEFIGURE};
  PolyDraw(dc, pts, i, 4);
#elif USE_QUARTZ
  if (!line_width_) CGContextSetShouldAntialias(quartz_gc, false);
  CGRect rect = CGRectMake(x, y, w-1, h-1);
  CGContextStrokeRect(quartz_gc, rect);
  if (!line_width_) CGContextSetShouldAntialias(quartz_gc, true);
#else
# error
#endif
}

/*!
  Draw a straight line between the two points.

  If line_width() is zero, this tries to draw as though a 1x1 square pen
  is moved between the first centers of pixels to the lower-right of
  the start and end points. Thus if y==y1 this will fill a rectangle
  with the corners x,y and x1+1,y+1. This may be 1 wider than you expect,
  but is necessary for compatability with previous fltk versions (and
  is due to the original X11 behavior).

  If line_width() is not zero then the results depend on the back end.
  It also may not produce consistent results if the ctm is not an
  integer translation or if the line is not horizontal or vertical.
*/
void fltk::drawline(int x, int y, int x1, int y1) {
  transform(x,y);
  transform(x1,y1);
#if USE_CAIRO
  // attempt to emulate the X11 drawing if line_width is zero. It also
  // works to set the end caps to square and add .5 in all cases...
  if (line_width_) {
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x1, y1);
  } else {
    cairo_move_to(cr, x+.5, y+.5);
    cairo_line_to(cr, x1+.5, y1+.5);
  }
  cairo_stroke(cr);
#elif USE_QUARTZ
  // This appears to produce the same output as X11, though it is unclear
  // why, as the line caps are butt by default:
  if (( x==x1 || y==y1 ) && !line_width_ )
    CGContextSetShouldAntialias(quartz_gc, false);
  CGContextMoveToPoint(quartz_gc, x, y);
  CGContextAddLineToPoint(quartz_gc, x1, y1);
  CGContextStrokePath(quartz_gc);
  if (!line_width_) CGContextSetShouldAntialias(quartz_gc, true);
#elif USE_X11
  XDrawLine(xdisplay, xwindow, gc, x, y, x1, y1);
#elif defined(_WIN32)
  setpen();
  MoveToEx(dc, x, y, 0L); 
  LineTo(dc, x1, y1);
  // GDI does butt end caps (sort of, it just truncates the drawing
  // with horizontal or vertical lines depending on the slope). This
  // adds an extra pixel to emulate the X11 drawing.
  if (!line_width_) SetPixel(dc, x1, y1, current_xpixel);
#else
# error
#endif
}

/*! Draw a straight line between the two points. */
void fltk::drawline(float x, float y, float x1, float y1) {
  transform(x,y);
  transform(x1,y1);
#if USE_CAIRO
  if (line_width_) {
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x1, y1);
  } else {
    cairo_move_to(cr, x+.5, y+.5);
    cairo_line_to(cr, x1+.5, y1+.5);
  }
  cairo_stroke(cr);
# elif USE_QUARTZ
  if (( x==x1 || y==y1 ) && !line_width_ )
    CGContextSetShouldAntialias(quartz_gc, false);
  CGContextMoveToPoint(quartz_gc, x, y);
  CGContextAddLineToPoint(quartz_gc, x1, y1);
  CGContextStrokePath(quartz_gc);
  if (!line_width_) CGContextSetShouldAntialias(quartz_gc, true);
#else
  int X = int(floorf(x)+.5); int Y = int(floorf(y)+.5);
  int X1 = int(floorf(x1)+.5); int Y1 = int(floorf(y1)+.5);
# if USE_X11
  XDrawLine(xdisplay, xwindow, gc, X, Y, X1, Y1);
# elif defined(_WIN32)
  setpen();
  MoveToEx(dc, X, Y, 0L); 
  LineTo(dc, X1, Y1);
  if (!line_width_) SetPixel(dc, X1, Y1, current_xpixel);
# else
#  error
# endif
#endif
}

/*! Draw a dot at the given point. If line_width() is zero this is a
  single pixel to the lower-right of x,y. If line_width() is non-zero
  this is a dot drawn with the current pen and line caps.
*/
void fltk::drawpoint(int x, int y) {
  if (!line_width_) {
  transform(x,y);
#if USE_CAIRO
    fillrect(x,y,1,1);
#elif USE_X11
  XDrawPoint(xdisplay, xwindow, gc, x, y);
#elif defined(_WIN32)
  SetPixel(dc, x, y, current_xpixel);
#else
    fillrect(x,y,1,1);
#endif
  } else {
    drawline(x,y,x,y);
  }
}

/*! Draw a dot at the given point. If line_width() is zero this is
  the single pixel containing X,Y, or the one to the lower-right if
  X and Y transform to integers. If line_width() is non-zero this
  is a dot drawn with the current pen and line caps (currently
  draws nothing in some api's unless the line_style has CAP_ROUND).
*/
void fltk::drawpoint(float X, float Y) {
  if (!line_width_) {
  transform(X,Y); 
  int x = int(floorf(X)); int y = int(floorf(Y));
#if USE_CAIRO
    fillrect(x,y,1,1);
#elif USE_X11
  XDrawPoint(xdisplay, xwindow, gc, x, y);
# elif defined(_WIN32)
  SetPixel(dc, x, y, current_xpixel);
# else
    fillrect(x,y,1,1);
#endif
  } else {
    drawline(X,Y,X,Y);
  }
}

//
// End of "$Id: fillrect.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
