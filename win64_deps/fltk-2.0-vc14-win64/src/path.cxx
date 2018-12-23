//
// "$Id: path.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Path construction and filling. I think this file is always linked
// into any fltk program, so try to keep it reasonably small.
// Also see arc.cxx and curve.cxx
//
// Copyright 2001 by Bill Spitzak and others.
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
#include <fltk/math.h>
#include <fltk/x.h>
#include <fltk/string.h>
#include <stdlib.h>
using namespace fltk;

struct Matrix {
  float a, b, c, d, x, y;
  int ix, iy; // x & y rounded to nearest integer
  bool trivial; // true if no rotation or scale
};

static Matrix m = {1, 0, 0, 1, 0, 0, 0, 0, true};
static Matrix* stack;
static int stacksize = 0;
static int sptr = 0;

// Returns true if transformation is an integer translate only
bool fl_trivial_transform() {return m.trivial;}

#if USE_CAIRO
void fl_set_cairo_ctm() {
  cairo_matrix_t cm = { m.a, m.b, m.c, m.d, m.x, m.y };
  cairo_transform(cr, &cm);
}
#elif USE_QUARTZ
extern int fl_clip_h;
void fl_set_quartz_ctm() {
  CGAffineTransform mx = {m.a, m.b, -m.c, -m.d, m.x-.5f, -fl_clip_h+m.y-.5f};
  CGContextConcatCTM(quartz_gc, mx);
}
#endif

#if USE_XFT
bool fl_get_invert_matrix(XTransform& i) {
  i.matrix[2][0] = 0;
  i.matrix[2][1] = 0;
  i.matrix[2][2] = XDoubleToFixed(1);
  if (m.trivial) {
    i.matrix[0][1] = i.matrix[1][0] = 0;
    i.matrix[0][0] = i.matrix[1][1] = XDoubleToFixed(1);
    i.matrix[0][2] = XDoubleToFixed(-m.x);
    i.matrix[1][2] = XDoubleToFixed(-m.y);
    return true;
  }
  const float d = m.a*m.d-m.b*m.c; // determinant
  if (!d) return false;
  i.matrix[0][0] = XDoubleToFixed(m.d / d);
  i.matrix[0][1] = XDoubleToFixed(-m.c / d);
  i.matrix[0][2] = XDoubleToFixed((m.c*m.y - m.x*m.d) / d);
  i.matrix[1][0] = XDoubleToFixed(-m.b / d);
  i.matrix[1][1] = XDoubleToFixed(m.a / d);
  i.matrix[1][2] = XDoubleToFixed((m.x*m.b - m.a*m.y) / d);
  return true;
}
#endif

/**

  Save the current transformation on a stack, so you can restore it
  with pop_matrix().

  FLTK provides an arbitrary 2-D affine transformation (rotation,
  scale, skew, reflections, and translation). This is very similar to
  PostScript, PDF, SVG, and Cairo.

  Due to limited graphics capabilities of some systems, not all
  drawing functions will be correctly transformed, except by the
  integer portion of the translation. Don't rely on this as we
  may be fixing this without notice.

*/
void fltk::push_matrix() {
  if (sptr >= stacksize) {
    stacksize = stacksize ? 2*stacksize : 16;
    Matrix* newstack = new Matrix[stacksize];
    memcpy(newstack, stack, sptr*sizeof(Matrix));
    delete[] stack;
    stack = newstack;
  }
  stack[sptr++] = m;
}

/**
  Put the transformation back to the way it was before the last
  push_matrix(). Calling this without a matching push_matrix will crash!
*/
void fltk::pop_matrix() {m = stack[--sptr];}

/**
Multiply the current transformation by
\code
a b 0
c d 0
x y 1
\endcode
*/
void fltk::concat(float a, float b, float c, float d, float x, float y) {
  if (m.trivial) {
    m.a = a; m.b = b; m.c = c; m.d = d;
    m.x += x; m.ix = int(floorf(m.x+.5f));
    m.y += y; m.iy = int(floorf(m.y+.5f));
    m.trivial = false;
  } else {
    Matrix o;
    o.a = a*m.a + b*m.c;
    o.b = a*m.b + b*m.d;
    o.c = c*m.a + d*m.c;
    o.d = c*m.b + d*m.d;
    o.x = x*m.a + y*m.c + m.x; o.ix = int(floorf(o.x+.5f));
    o.y = x*m.b + y*m.d + m.y; o.iy = int(floorf(o.y+.5f));
    o.trivial = false;
    m = o;
  }
}

/**
Scale the current transformation by multiplying it by
\code
x 0 0
0 y 0
0 0 1
\endcode
*/
void fltk::scale(float x,float y) {
  if (x != 1.0 || y != 1.0)
    concat(x, 0, 0, y, 0, 0);
}

/**
Scale the current transformation by multiplying it by
\code
x 0 0
0 x 0
0 0 1
\endcode
*/
void fltk::scale(float x) {
  if (x != 1.0) concat(x,0,0,x,0,0);
}

/**
Translate the current transformation by multiplying it by
\code
1 0 0
0 1 0
x y 1
\endcode
*/
void fltk::translate(float x,float y) {
  if (m.trivial) {
    m.x += x; m.ix = int(floorf(m.x+.5f));
    m.y += y; m.iy = int(floorf(m.y+.5f));
    m.trivial = m.ix==m.x && m.iy==m.y;
  } else {
    concat(1,0,0,1,x,y);
  }
}

/**
  This integer version is provided because it is much faster than the
  floating-point version. However C++ will not "resolve" which one you
  want to call if you try passing doubles as arguments. To get it to
  compile, make sure you cast the arguments to float (add 'f' after
  floating-point constants). Use the 'f' versions (ie sinf(), cosf(),
  etc) of the math functions from <fltk/math.h> to produce floats and
  get maximum calculation speed.
*/
void fltk::translate(int x, int y) {
  if (m.trivial) {
    m.ix += x; m.x = float(m.ix);
    m.iy += y; m.y = float(m.iy);
  } else {
    concat(1,0,0,1,float(x),float(y));
  }
}

/**
Rotate the current transformation counter-clockwise by \a d degrees
(not radians!!). This is done by multiplying the matrix by:
\code
cos -sin 0
sin  cos 0
0     0  1
\endcode
*/
void fltk::rotate(float d) {
  if (d) {
    float s, c;
    if (d == 0) {s = 0; c = 1;}
    else if (d == 90) {s = 1; c = 0;}
    else if (d == 180) {s = 0; c = -1;}
    else if (d == 270 || d == -90) {s = -1; c = 0;}
    else {s = sinf(d*float(M_PI/180)); c = cosf(d*float(M_PI/180));}
    concat(c,-s,s,c,0,0);
  }
}

/**
Replace the current transform with the identity transform, which
puts 0,0 in the top-left corner of the window and each unit is 1
pixel in size.
*/
void fltk::load_identity() {
  m.a = 1; m.b = 0; m.c = 0; m.d = 1;
  m.x = 0; m.y = 0;
  m.ix = 0; m.iy = 0;
  m.trivial = true;
}

////////////////////////////////////////////////////////////////

/**
Replace \a x and \a y transformed into device coordinates.
Device-specific code can use this to draw things using the
fltk transformation matrix. If the backend is Cairo or another
API that does transformations, this may return xy unchagned.
*/
void fltk::transform(float& x, float& y) {
  if (!m.trivial) {
    float t = x*m.a + y*m.c + m.x;
    y = x*m.b + y*m.d + m.y;
    x = t;
  } else {
    x += m.x;
    y += m.y;
  }
}

/**
Replace x and y with the tranformed coordinates, ignoring
translation. This transforms a vector which is measuring a distance
between two positions, rather than a position.
*/
void fltk::transform_distance(float& x, float& y) {
  if (!m.trivial) {
    float t = x*m.a + y*m.c;
    y = x*m.b + y*m.d;
    x = t;
  }
}

/**
Replace x and y with the transformed coordinates, rounded to the
nearest integer.
*/
void fltk::transform(int& x, int& y) {
  if (!m.trivial) {
    int t = int(floorf(x*m.a + y*m.c + m.x + .5f));
    y = int(floorf(x*m.b + y*m.d + m.y + .5f));
    x = t;
  } else {
    x += m.ix;
    y += m.iy;
  }
}

/**
Transform the rectangle \a from into device coordinates and put
it into \a to. This only works correctly for 90 degree rotations, for
other transforms this will produce an axis-aligned rectangle with
the same area (this is useful for inscribing circles, and is about
the best that can be done for device functions that don't handle
rotation.
*/
void fltk::transform(const Rectangle& from, Rectangle& to) {
  if (m.trivial || from.empty()) {
    to.set(from.x()+m.ix, from.y()+m.iy, from.w(), from.h());
    return;
  }
  float x = from.x()+from.w()*.5f;
  float y = from.y()+from.h()*.5f;
  transform(x,y);
  float d1x,d1y; d1x = float(from.w()); d1y = 0; transform_distance(d1x, d1y);
  float d2x,d2y; d2x = 0; d2y = float(from.h()); transform_distance(d2x, d2y);
  float w = rintf(sqrtf(d1x*d1x+d2x*d2x));
  x = floorf(x - w/2);
  float h = rintf(sqrtf(d1y*d1y+d2y*d2y));
  y = floorf(y - h/2);
  to.set(int(x),int(y),int(w),int(h));
}

/**
Same as transform(Rectangle(X,Y,W,H),to) but replaces XYWH with the transformed
rectangle. This may be faster as it avoids the rectangle construction.
*/
void fltk::transform(int& X,int& Y,int& W,int& H) {
  if (m.trivial) {
    X += m.ix; Y += m.iy;
    return;
  }
  float x = X+W*.5f;
  float y = Y+H*.5f;
  transform(x,y);
  float d1x,d1y; d1x = float(W); d1y = 0; transform_distance(d1x, d1y);
  float d2x,d2y; d2x = 0; d2y = float(H); transform_distance(d2x, d2y);
  W = int(sqrtf(d1x*d1x+d2x*d2x)+.5f);
  H = int(sqrtf(d1y*d1y+d2y*d2y)+.5f);
  X = int(floorf(x - .5f*W));
  Y = int(floorf(y - .5f*H));
}

////////////////////////////////////////////////////////////////
// Path Construction

#if USE_CAIRO
// Cairo has its own coordinate stack
#elif USE_QUARTZ
// Quartz has its own coordinate stack
static bool first_point = true;
namespace fltk { 
  void quartz_add_vertex(float x, float y) {
    if (first_point) {
      CGContextMoveToPoint(quartz_gc, x, y);
      first_point = false;
    } else {
      CGContextAddLineToPoint(quartz_gc, x, y);
    }
  }
}
#else
// We have to store the path ourselves on X11 and Win32. Path is stored
// as transformed points plus lengths of "loops".

// typedef what the x,y fields in a point are:
#if USE_X11
typedef short COORD_T;
#elif defined(_WIN32)
typedef int COORD_T;
typedef POINT XPoint;
#else
typedef int COORD_T;
struct XPoint { int x, y; };
#endif

// Storage of the current path:
static XPoint *xpoint; // all the points
static int point_array_size;
static int numpoints; // number of points
static int loop_start; // point at start of current loop
static int* loop; // number of points in each loop
static int loops; // number of loops
static int loop_array_size;

static void add_n_points(int n) {
  point_array_size = point_array_size ? 2*point_array_size : 16;
  if (numpoints+n >= point_array_size) point_array_size = numpoints+n;
  XPoint* newpoints = new XPoint[point_array_size+1];
  memcpy(newpoints, xpoint, numpoints*sizeof(XPoint));
  delete[] xpoint;
  xpoint = newpoints;
}

// The path also contains one dummy pie/chord piece:
static fltk::Rectangle circle;
static float circle_start, circle_end;
static enum {NONE=0, PIE, CHORD} circle_type;

#endif // local path storage

/**
  Add a single vertex to the current path. (if the path is empty
  or a closepath() was done, this is equivalent to a "moveto"
  in PostScript, otherwise it is equivalent to a "lineto").
*/
void fltk::addvertex(float X, float Y) {
#if USE_CAIRO
  transform(X,Y);
  cairo_line_to(cr,X,Y);
#elif USE_QUARTZ
  transform(X, Y);
  quartz_add_vertex(X, Y);
#else
  COORD_T x = COORD_T(floorf(X*m.a + Y*m.c + m.x + .5f));
  COORD_T y = COORD_T(floorf(X*m.b + Y*m.d + m.y + .5f));
  if (!numpoints || x != xpoint[numpoints-1].x || y != xpoint[numpoints-1].y) {
    if (numpoints+1 >= point_array_size) add_n_points(1);
    xpoint[numpoints].x = x;
    xpoint[numpoints].y = y;
    numpoints++;
  }
#endif
}

/**
  This integer version is provided because it is much faster than the
  floating-point version. However C++ will not "resolve" which one you
  want to call if you try passing doubles as arguments. To get it to
  compile, make sure you cast the arguments to float (add 'f' after
  floating-point constants). Use the 'f' versions (ie sinf(), cosf(),
  etc) of the math functions from <fltk/math.h> to produce floats and
  get maximum calculation speed.
*/
void fltk::addvertex(int X, int Y) {
#if USE_CAIRO
  transform(X,Y);
  cairo_line_to(cr,X,Y);
#elif USE_QUARTZ
  transform(X, Y);
  quartz_add_vertex(X, Y);
#else
  COORD_T x,y;
  if (m.trivial) {
    x = COORD_T(X+m.ix);
    y = COORD_T(Y+m.iy);
  } else {
    x = COORD_T(floorf(X*m.a + Y*m.c + m.x + .5f));
    y = COORD_T(floorf(X*m.b + Y*m.d + m.y + .5f));
  }
  if (!numpoints || x != xpoint[numpoints-1].x || y != xpoint[numpoints-1].y) {
    if (numpoints+1 >= point_array_size) add_n_points(1);
    xpoint[numpoints].x = x;
    xpoint[numpoints].y = y;
    numpoints++;
  }
#endif
}

/**
  Add a whole set of vertices to the current path. This is much faster
  than calling fltk::addvertex once for each point.
*/
void fltk::addvertices(int n, const float array[][2]) {
  const float* a = array[0];
  const float* e = a+2*n;
#if USE_CAIRO
  for (; a < e; a += 2) {
    float X = (float) a[0]; float Y = (float) a[1];
    transform(X,Y);
    cairo_line_to(cr,X,Y);
  }
#elif USE_QUARTZ
  for (; a < e; a += 2) {
    float X = (float) a[0]; float Y = (float) a[1];
    transform(X,Y);
    quartz_add_vertex(X, Y);
  }
#else
  if (numpoints+n >= point_array_size) add_n_points(n);
  int pn = numpoints;
  if (m.trivial) {
    for (; a < e; a += 2) {
      COORD_T x = COORD_T(floorf(a[0] + m.x + .5f));
      COORD_T y = COORD_T(floorf(a[1] + m.y + .5f));
      if (!pn || x != xpoint[pn-1].x || y != xpoint[pn-1].y) {
	xpoint[pn].x = x;
	xpoint[pn].y = y;
	pn++;
      }
    }
  } else {
    for (; a < e; a += 2) {
      COORD_T x = COORD_T(floorf(a[0]*m.a + a[1]*m.c + m.x + .5f));
      COORD_T y = COORD_T(floorf(a[0]*m.b + a[1]*m.d + m.y + .5f));
      if (!pn || x != xpoint[pn-1].x || y != xpoint[pn-1].y) {
	xpoint[pn].x = x;
	xpoint[pn].y = y;
	pn++;
      }
    }
  }
  numpoints = pn;
#endif
}

/** Add a whole set of integer vertices to the current path. */
void fltk::addvertices(int n, const int array[][2]) {
  const int* a = array[0];
  const int* e = a+2*n;
#if USE_CAIRO
  for (; a < e; a += 2) {
    float X = (float) a[0]; float Y = (float) a[1];
    transform(X,Y);
    cairo_line_to(cr,X,Y);
  }
#elif USE_QUARTZ
  for (; a < e; a += 2) {
    float X = (float) a[0]; float Y = (float) a[1];
    transform(X,Y);
    quartz_add_vertex(X, Y);
  }
#else
  if (numpoints+n >= point_array_size) add_n_points(n);
  int pn = numpoints;
  if (m.trivial) {
    for (; a < e; a += 2) {
      COORD_T x = COORD_T(a[0]+m.ix);
      COORD_T y = COORD_T(a[1]+m.iy);
      if (!pn || x != xpoint[pn-1].x || y != xpoint[pn-1].y) {
	xpoint[pn].x = x;
	xpoint[pn].y = y;
	pn++;
      }
    }
  } else {
    for (; a < e; a += 2) {
      COORD_T x = COORD_T(floorf(a[0]*m.a + a[1]*m.c + m.x + .5f));
      COORD_T y = COORD_T(floorf(a[0]*m.b + a[1]*m.d + m.y + .5f));
      if (!pn || x != xpoint[pn-1].x || y != xpoint[pn-1].y) {
	xpoint[pn].x = x;
	xpoint[pn].y = y;
	pn++;
      }
    }
  }
  numpoints = pn;
#endif
}

/**
  Adds a whole set of vertcies that have been produced from values
  returned by fltk::transform(). This is how curve() and arc() are
  implemented.
*/
void fltk::addvertices_transformed(int n, const float array[][2]) {
  const float* a = array[0];
  const float* e = a+2*n;
#if USE_CAIRO
  for (; a < e; a += 2) cairo_line_to(cr,a[0],a[1]);
#elif USE_QUARTZ
  for (; a < e; a += 2) quartz_add_vertex(a[0], a[1]);
#else
  if (numpoints+n >= point_array_size) add_n_points(n);
  int pn = numpoints;
  for (; a < e; a += 2) {
    COORD_T x = COORD_T(floorf(a[0] + .5f));
    COORD_T y = COORD_T(floorf(a[1] + .5f));
    if (!pn || x != xpoint[pn-1].x || y != xpoint[pn-1].y) {
      xpoint[pn].x = x;
      xpoint[pn].y = y;
      pn++;
    }
  }
  numpoints = pn;
#endif
}

/**
  Similar to drawing another vertex back at the starting point, but
  fltk knows the path is closed. The next addvertex() will start a
  new disconnected part of the shape.

  It is harmless to call fltk::closepath() several times in a row, or
  to call it before the first point. Sections with less than 3 points
  in them will not draw anything when filled.
*/
void fltk::closepath() {
#if USE_CAIRO
  cairo_close_path(cr);
  cairo_new_sub_path(cr);
#elif USE_QUARTZ
  if (!first_point) 
    CGContextClosePath(quartz_gc);
  first_point = true;
#else
  if (numpoints > loop_start+2) {
    // close the shape by duplicating first point:
    XPoint& q = xpoint[loop_start];
    // the array always has one extra point so we don't need to check
    XPoint& p = xpoint[numpoints-1];
    if (p.x != q.x || p.y != q.y) xpoint[numpoints++] = q;
    // remember the new loop:
    if (loops >= loop_array_size) {
      loop_array_size = loop_array_size ? 2*loop_array_size : 16;
      int* newloop = new int[loop_array_size];
      memcpy(newloop, loop, loops*sizeof(int));
      delete[] loop;
      loop = newloop;
    }
    loop[loops++] = numpoints-loop_start;
    loop_start = numpoints;
  } else {
    numpoints = loop_start;
  }
#endif
}

////////////////////////////////////////////////////////////////
// Enormous kludge to add arcs to a path but try to take advantage
// of primitive arc-drawing functions provided by the OS. This mess
// should not be needed on newer systems...
//
// We keep track of exactly one "nice" circle:

/**
  Add a pie-shaped closed piece to the path, inscribed in the
  rectangle so if it is stroked with the default line width it exactly
  fills the rectangle (this is slightly smaller than addarc() will
  draw). If you want a full circle use addchord().

  This tries to take advantage of the primitive calls provided by
  Xlib and GDI32. Limitations are that you can only draw one per
  path, that rotated coordinates don't work, and doing anything other
  than fillpath() will produce unpredictable results.

  \see addchord()
*/
void fltk::addpie(const Rectangle& r, float start, float end) {
#if USE_CAIRO || USE_QUARTZ
  closepath();
  addvertex(r.x()+r.w()*.5f, r.y()+r.h()*.5f);
  float delta = sqrtf(1/fabsf(m.a*m.d-m.b*m.c));
  addarc(r.x()+delta/2, r.y()+delta/2, r.w()-delta, r.h()-delta, start, end);
#else
  transform(r, circle);
  circle_start = start;
  circle_end = end;
  circle_type = PIE;
#endif
}

/**
  Add an isolated circular arc to the path. It is inscribed in the
  rectangle so if it is stroked with the default line width it exactly
  fills the rectangle (this is slightly smaller than addarc() will
  draw). If the angles are 0 and 360 a closed circle is added.

  This tries to take advantage of the primitive calls provided by
  Xlib and GDI32. Limitations are that you can only draw one,
  a rotated current transform does not work.
*/
void fltk::addchord(const Rectangle& r, float start, float end) {
#if USE_CAIRO || USE_QUARTZ
  closepath();
  float delta = sqrtf(1/fabsf(m.a*m.d-m.b*m.c));
  addarc(r.x()+delta/2, r.y()+delta/2, r.w()-delta, r.h()-delta, start, end);
#else
  transform(r, circle);
  circle_start = start;
  circle_end = end;
  circle_type = CHORD;
#endif
}

static inline void inline_newpath() {
#if USE_CAIRO
  cairo_new_path(cr);
#elif USE_QUARTZ
  first_point = true;
  CGContextBeginPath(quartz_gc);
#else
  numpoints = loop_start = loops = 0;
  circle_type = NONE;
#endif
}

/**
  Clear the current "path". This is normally done by fltk::fillpath() or
  any other drawing command.
*/
void fltk::newpath() {inline_newpath();}

////////////////////////////////////////////////////////////////

#if 0
// removed as it is difficult to emulate except on OpenGL
/**
  Draw a point (one pixel) for every vertex in the path, then clear the path.
  In theory the line_style() should affect how big the points are, but I
  don't think that works on X.
*/
void fltk::drawpoints() {
#if USE_CAIRO
  // Not implemented!
#elif USE_QUARTZ
  // Not implemented!
#elif USE_X11
  if (numpoints > 0) XDrawPoints(xdisplay, xwindow, gc, xpoint, numpoints, 0);
#elif defined(_WIN32)
  for (int i=0; i<numpoints; i++)
    SetPixel(dc, xpoint[i].x, xpoint[i].y, current_xpixel);
#endif
  inline_newpath();
}
#endif

/**
  Draw a line between all the points in the path (see
  fltk::line_style() for ways to set the thicknesss and dot pattern of
  the line), then clear the path.
*/
void fltk::strokepath() {
#if USE_CAIRO
  cairo_stroke(cr);
#elif USE_QUARTZ
  CGContextStrokePath(quartz_gc);
#elif USE_X11
  if (circle_type) {
    int A = int(circle_start*64);
    int B = int(circle_end*64)-A;
    const Rectangle& r = circle;
    if (r.w() < 2 || r.h() < 2) {
      if (!r.empty())
	XFillRectangle(xdisplay, xwindow, gc, r.x(), r.y(), r.w(), r.h());
    } else {
      XDrawArc(xdisplay, xwindow, gc, r.x(), r.y(), r.w()-1, r.h()-1, A, B);
    }
  }
  int loop_start = 0;
  for (int n = 0; n < loops; n++) {
    int loop_size = loop[n];
    XDrawLines(xdisplay, xwindow, gc, xpoint+loop_start, loop_size, 0);
    loop_start += loop_size;
  }
  int loop_size = numpoints-loop_start;
  if (loop_size > 1)
    XDrawLines(xdisplay, xwindow, gc, xpoint+loop_start, loop_size, 0);
#elif defined(_WIN32)
  setpen();
  if (circle_type && circle_start != circle_end) {
    float a1 = circle_start;
    float a2 = circle_end;
    const Rectangle& r = circle;
    if (a2 < a1) {float t = a2; a2 = a1; a1 = t;}
    if (a2 >= a1+360) a1 = a2 = 0;
    int xa = r.center_x()+int(r.w()*cosf(a1*float(M_PI/180.0)));
    int ya = r.center_y()-int(r.h()*sinf(a1*float(M_PI/180.0)));
    int xb = r.center_x()+int(r.w()*cosf(a2*float(M_PI/180.0)));
    int yb = r.center_y()-int(r.h()*sinf(a2*float(M_PI/180.0)));
    Arc(dc, r.x(), r.y(), r.r(), r.b(), xa, ya, xb, yb);
  }
  int loop_start = 0;
  for (int n = 0; n < loops; n++) {
    int loop_size = loop[n];
    Polyline(dc, xpoint+loop_start, loop_size);
    loop_start += loop_size;
  }
  int loop_size = numpoints-loop_start;
  if (loop_size > 1)
    Polyline(dc, xpoint+loop_start, loop_size);
#endif
  inline_newpath();
}

/**
  Does fltk::closepath() and then fill with the current color, and
  then clear the path.

  For portability, you should only draw polygons that appear the same
  whether "even/odd" or "non-zero" winding rules are used to fill
  them. This mostly means that holes should be drawn in the opposite
  direction of the outside.

  Warning: result is somewhat different on X and Win32! Use
  fillstrokepath() to make matching shapes. In my opinion X is correct,
  we may change the Win32 version to match in the future, perhaps by
  making the current pen invisible?
*/
void fltk::fillpath() {
#if USE_CAIRO
  cairo_fill(cr);
#elif USE_QUARTZ
  CGContextFillPath(quartz_gc);
#elif USE_X11
  if (circle_type) {
    int A = int(circle_start*64);
    int B = int(circle_end*64)-A;
    const Rectangle& r = circle;
    XSetArcMode(xdisplay, gc, circle_type == PIE ? ArcPieSlice : ArcChord);
    if (r.w() < 2 || r.h() < 2) {
      if (!r.empty())
	XFillRectangle(xdisplay, xwindow, gc, r.x(), r.y(), r.w(), r.h());
    } else {
      XFillArc(xdisplay, xwindow, gc, r.x(), r.y(), r.w()-1, r.h()-1, A, B);
      XDrawArc(xdisplay, xwindow, gc, r.x(), r.y(), r.w()-1, r.h()-1, A, B);
    }
  }
  if (loops) closepath();
  if (numpoints > 2) {
    if (loops > 2) {
      // back-trace the lines between each "disconnected" part so they
      // are actually connected:
      if (numpoints+loops-2 >= point_array_size) add_n_points(loops-2);
      int n = numpoints-1;
      for (int i = loops; --i > 1;) {
	n -= loop[i];
	xpoint[numpoints++] = xpoint[n];
      }
    }
    XFillPolygon(xdisplay, xwindow, gc, xpoint, numpoints, 0, 0);
  }
#elif defined(_WIN32)
  setbrush();
  setpen();
  if (circle_type && circle_start != circle_end) {
    float a1 = circle_start;
    float a2 = circle_end;
    const Rectangle& r = circle;
    if (a2 < a1) {float t = a2; a2 = a1; a1 = t;}
    if (a2 >= a1+360) a1 = a2 = 0;
    int xa = r.center_x()+int(r.w()*cosf(a1*float(M_PI/180.0)));
    int ya = r.center_y()-int(r.h()*sinf(a1*float(M_PI/180.0)));
    int xb = r.center_x()+int(r.w()*cosf(a2*float(M_PI/180.0)));
    int yb = r.center_y()-int(r.h()*sinf(a2*float(M_PI/180.0)));
    if (circle_type == PIE)
      Pie(dc, r.x(), r.y(), r.r(), r.b(), xa, ya, xb, yb);
    else
      Chord(dc, r.x(), r.y(), r.r(), r.b(), xa, ya, xb, yb);
  }
  if (loops) {
    closepath();
    PolyPolygon(dc, xpoint, loop, loops);
  } else if (numpoints > 2) {
    Polygon(dc, xpoint, numpoints);
  }
#endif
  inline_newpath();
}

/**
  Does fltk::fill(), then sets the current color to linecolor and does
  fltk::stroke with the same closed path, and then clears the path.

  This seems to produce very similar results on X and Win32. Also it
  takes advantage of a single GDI32 call that does this and should
  be faster.
*/
void fltk::fillstrokepath(Color color) {
#if USE_CAIRO
  closepath();
  cairo_fill_preserve(cr);
  setcolor(color);
  cairo_stroke(cr);
#elif USE_QUARTZ
  uchar r, g, b; 
  split_color(color, r, g, b);
  CGContextSetRGBStrokeColor(quartz_gc, r/255.0f, g/255.0f, b/255.0f, 1.0);
  CGContextDrawPath(quartz_gc, kCGPathFillStroke);
  setcolor(color);
  inline_newpath();
#elif USE_X11
  if (circle_type && circle.w()>1 && circle.h()>1) {
    int A = int(circle_start*64);
    int B = int(circle_end*64)-A;
    const Rectangle& r = circle;
    XSetArcMode(xdisplay, gc, circle_type == PIE ? ArcPieSlice : ArcChord);
    XFillArc(xdisplay, xwindow, gc, r.x(), r.y(), r.w()-1, r.h()-1, A, B);
  }
  closepath();
  if (numpoints > 2) {
    int saved_points = numpoints;
    if (loops > 2) {
      // back-trace the lines between each "disconnected" part so they
      // are actually connected:
      if (numpoints+loops-2 >= point_array_size) add_n_points(loops-2);
      int n = saved_points-1;
      for (int i = loops; --i > 1;) {
	n -= loop[i];
	xpoint[numpoints++] = xpoint[n];
      }
    }
    XFillPolygon(xdisplay, xwindow, gc, xpoint, numpoints, 0, 0);
    numpoints = saved_points; // throw away the extra points
  }
  setcolor(color);
  strokepath();
#elif defined(_WIN32)
  setbrush();
  setcolor(color);
  setpen();
  if (circle_type && circle_start != circle_end) {
    float a1 = circle_start;
    float a2 = circle_end;
    const Rectangle& r = circle;
    if (a2 < a1) {float t = a2; a2 = a1; a1 = t;}
    if (a2 >= a1+360) a1 = a2 = 0;
    int xa = r.center_x()+int(r.w()*cosf(a1*float(M_PI/180.0)));
    int ya = r.center_y()-int(r.h()*sinf(a1*float(M_PI/180.0)));
    int xb = r.center_x()+int(r.w()*cosf(a2*float(M_PI/180.0)));
    int yb = r.center_y()-int(r.h()*sinf(a2*float(M_PI/180.0)));
    if (circle_type == PIE)
      Pie(dc, r.x(), r.y(), r.r(), r.b(), xa, ya, xb, yb);
    else
      Chord(dc, r.x(), r.y(), r.r(), r.b(), xa, ya, xb, yb);
  }
  if (loops) {
    closepath();
    PolyPolygon(dc, xpoint, loop, loops);
  } else if (numpoints > 2) {
    Polygon(dc, xpoint, numpoints);
  }
  inline_newpath();
#endif
}

//
// End of "$Id: path.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
