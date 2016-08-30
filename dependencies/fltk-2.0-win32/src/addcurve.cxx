//
// "$Id: addcurve.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// Utility for drawing Bezier curves, adding the points to
// the current begin/vertex/end path.
// Incremental math implementation:
// I very much doubt this is optimal!  From Foley/vanDam page 511.
// If anybody has a better algorithim, please send it!

#include <config.h>
#include <fltk/draw.h>
#include <fltk/math.h>
#include <fltk/x.h>
using namespace fltk;

/*!
  Add a series of points on a Bezier spline to the path. The curve
  ends (and two of the points) are at \a x,y and \a x3,y3. The "handles"
  are at \a x1,y1 and \a x2,y2.
*/
void fltk::addcurve(float x0, float y0,
		    float x1, float y1,
		    float x2, float y2,
		    float x3, float y3) {

  transform(x0,y0);
  transform(x1,y1);
  transform(x2,y2);
  transform(x3,y3);
#if USE_CAIRO
  cairo_line_to(cr,x0,y0);
  cairo_curve_to(cr,x1,y1,x2,y2,x3,y3);
#else
  float x = x0; float y = y0;

#define MAXPOINTS 100
  float points[MAXPOINTS][2];
  float* p = points[0];
  *p++ = float(x); *p++ = float(y);

  // find the area:
  float a = fabsf((x-x2)*(y3-y1)-(y-y2)*(x3-x1));
  float b = fabsf((x-x3)*(y2-y1)-(y-y3)*(x2-x1));
  if (b > a) a = b;

  // use that to guess at the number of segments:
  int n = int(sqrtf(a)/4);
  if (n > 1) {
    if (n > MAXPOINTS-1) n = MAXPOINTS-1;

    float e = 1.0f/n;

    // calculate the coefficients of 3rd order equation:
    float xa = (x3-3*x2+3*x1-x);
    float xb = 3*(x2-2*x1+x);
    float xc = 3*(x1-x);
    // calculate the forward differences:
    float dx1 = ((xa*e+xb)*e+xc)*e;
    float dx3 = 6*xa*e*e*e;
    float dx2 = dx3 + 2*xb*e*e;

    // calculate the coefficients of 3rd order equation:
    float ya = (y3-3*y2+3*y1-y);
    float yb = 3*(y2-2*y1+y);
    float yc = 3*(y1-y);
    // calculate the forward differences:
    float dy1 = ((ya*e+yb)*e+yc)*e;
    float dy3 = 6*ya*e*e*e;
    float dy2 = dy3 + 2*yb*e*e;

    // draw points 1 .. n-2:
    for (int m=2; m<n; m++) {
      x += dx1;
      *p++ = x;
      dx1 += dx2;
      dx2 += dx3;
      y += dy1;
      *p++ = y;
      dy1 += dy2;
      dy2 += dy3;
    }

    // draw point n-1:
    *p++ = x+dx1;
    *p++ = y+dy1;
  }

  // draw point n:
  *p++ = x3;
  *p++ = y3;
  addvertices_transformed((p-points[0])/2, points);
#endif
}

//
// End of "$Id: addcurve.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
