//
// "$Id: mandelbrot.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Mandelbrot set header file for the Fast Light Tool Kit (FLTK).
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

#include <fltk/Widget.h>

class DrawingArea : public fltk::Widget {
  void draw();
public:
  uchar *buffer;
  int W, H;
  int nextline;
  int drawn;
  int julia;
  int iterations;
  int brightness;
  double jX, jY;
  double X, Y, scale;
  int sx, sy, sw, sh; // selection box
  void erase_box();
  int handle(int);
  void layout(); // resize() replaced by this for fltk2.0
  void new_display();
  enum {
    MAX_BRIGHTNESS = 16,
    DEFAULT_BRIGHTNESS = 16,
    MAX_ITERATIONS = 14,
    DEFAULT_ITERATIONS = 7
  };
  DrawingArea(int x, int y, int w, int h) : fltk::Widget(x, y, w, h) {
    buffer = 0;
    W = w - 6;
    H = h - 8;
    nextline = 0;
    drawn = 0;
    julia = 0;
    X = Y = 0;
    scale = 4.0;
    iterations = 1 << DEFAULT_ITERATIONS;
    brightness = DEFAULT_BRIGHTNESS;
  }
  int idle();
};

//
// End of "$Id: mandelbrot.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
