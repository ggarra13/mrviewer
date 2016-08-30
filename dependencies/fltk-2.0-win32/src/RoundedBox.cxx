//
// "$Id: RoundedBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Rounded box drawing routines for the Fast Light Tool Kit (FLTK).
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

#include <fltk/Box.h>
#include <fltk/Style.h>
#include <fltk/draw.h>
using namespace fltk;

static void rbox(const Rectangle& r, Color fill, Color line) {
  // figure out diameter of circles for corners:
  int D = r.w()|1;
  if (r.h() < r.w()) D = r.h()|1;
  if (D > 31) D = 31;

  float d = float(D); // -.5f
  float X = float(r.x());
  float Y = float(r.y());
  addarc(X, Y, d, d, 90, 180);
  addarc(X, Y+r.h()-d-1, d, d, 180, 270);
  addarc(X+r.w()-d-1, Y+r.h()-d-1, d, d, 270, 360);
  addarc(X+r.w()-d-1, Y, d, d, 0, 90);
  setcolor(fill);
  fillstrokepath(line);
}

class RoundedBox : public Box {
public:
  void _draw(const Rectangle& r) const {rbox(r, getbgcolor(), getcolor());}
  void inset(Rectangle& r) const {r.inset(1);}
  RoundedBox(const char* n) : Box(n) {}
};
static RoundedBox roundedBox("rounded");
/** Round-cornered rectangle with a black border. */
Box* const fltk::ROUNDED_BOX = &roundedBox;

class RShadowBox : public Box {
public:
  void _draw(const Rectangle& r1) const {
    const Color fg = getcolor();
    Rectangle r(r1);
    // draw shadow, in lower-right of r1:
    r.move_x(3);
    r.move_y(3);
    rbox(r, GRAY33, GRAY33);
    // draw the box in upper-left of r1:
    r.move(-3,-3);
    setcolor(fg);
    roundedBox.draw(r);
  }
  void inset(Rectangle& r) const {
    r.x(r.x()+1);
    r.y(r.y()+1);
    r.w(r.w()-5);
    r.h(r.h()-5);
  }
  RShadowBox(const char* n) : Box(n) {}
};
static RShadowBox rshadowBox("rshadow");
/** Round-cornered rectangle with a black border and gray shadow. */
Box* const fltk::RSHADOW_BOX = &rshadowBox;

class RFlatBox : public Box {
public:
  void _draw(const Rectangle& r) const {
    const Color fg = getcolor();
    rbox(r, getbgcolor(), getbgcolor());
    setcolor(fg);
  }
  void inset(Rectangle& r) const {r.inset(7);}
  RFlatBox(const char* n) : Box(n) {}
};
static RFlatBox rflatBox("rflat");
/** Round-cornered rectangle with no border. */
Box* const fltk::RFLAT_BOX = &rflatBox;

//
// End of "$Id: RoundedBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
