//
// "$Id: ShadowBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Shadow box drawing routines for the Fast Light Tool Kit (FLTK).
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

#define SIZE 3

class ShadowBox : public Box {
public:
  void _draw(const Rectangle& r1) const
  {
    const Color bg = getbgcolor();
    const Color fg = getcolor();
    Rectangle r(r1); r.move_r(-SIZE); r.move_b(-SIZE);
    if (r.w() > 2 && r.h() > 2 && !drawflags(INVISIBLE)) {
      setcolor(bg);
      fillrect(r.x()+1,r.y()+1,r.w()-2,r.h()-2);
    }
    setcolor(GRAY33);
    fillrect(r.x()+SIZE, r.b(),  r.w(), SIZE);
    fillrect(r.r(), r.y()+SIZE, SIZE, r.h());
    setcolor(fg);
    strokerect(r);
  }
  void inset(Rectangle& r) const {
    r.x(r.x()+1);
    r.y(r.y()+1);
    r.w(r.w()-(2+SIZE));
    r.h(r.h()-(2+SIZE));
  }
  bool is_frame() const {return true;}
  ShadowBox(const char* n) : Box(n) {}
};
static ShadowBox shadowBox("shadow_box");
Box* const fltk::SHADOW_BOX = &shadowBox;

//
// End of "$Id: ShadowBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
