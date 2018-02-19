//
// "$Id: ReturnButton.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/ReturnButton.h>
#include <fltk/events.h>
#include <fltk/draw.h>
#include <fltk/Symbol.h>
using namespace fltk;

/*! \class fltk::ReturnButton
  Same as a normal button except the shortcut() is preset to ReturnKey
  and KeypadEnter, and a glyph is drawn to indicate this.

  \image html Fl_Return_Button.gif

*/

static class ReturnButtonGlyph : public Symbol {
public:
  void _draw(const Rectangle& r) const {
    int size = r.w(); if (r.h()<size) size = r.h();
    int d = (size+2)/4; if (d<3) d = 3;
    int t = (size+9)/12; if (t<1) t = 1;
    int x0 = r.x()+((r.w()-2*d-2*t-1)>>1);
    int x1 = x0+d;
    int y0 = r.center_y();
#if 1
    setcolor(GRAY99);
    newpath();
    addvertex(x0, y0);
    addvertex(x1, y0+d);
    addvertex(x1, y0+t);
    addvertex(x1+d+2*t, y0+t);
    addvertex(x1+d+2*t, y0-d);
    strokepath();
    drawline(x1, y0-t, x1, y0-d);
    setcolor(GRAY00);
    drawline(x0, y0, x1, y0-d);
    newpath();
    addvertex(x1+1, y0-t);
    addvertex(x1+d, y0-t);
    addvertex(x1+d, y0-d);
    addvertex(x1+d+2*t, y0-d);
    setcolor(GRAY33);
    strokepath();
#else // solid arrow written by Carl
    setcolor(inactive(fc, f));
    polygon(x0,y0, x1,y0+d, x1, y0-d);
    fillrect(x1,y0-t,d,2*t+1);
    fillrect(x1+d,y0-d,2*t+1,d+t+1);
#endif
  }
  ReturnButtonGlyph() : Symbol("returnarrow") {}
} glyph;

void ReturnButton::draw() {
  Button::draw(-2*int(textsize()));
}

static void revert(Style* s) {
  s->glyph_ = &glyph;
}
static NamedStyle style("ReturnButton", revert, &ReturnButton::default_style);
NamedStyle* ReturnButton::default_style = &::style;

ReturnButton::ReturnButton(int x,int y,int w,int h,const char *l)
  : Button(x,y,w,h,l)
{
  default_style->parent_ = style();
  style(default_style);
  shortcut(ReturnKey);
  add_shortcut(KeypadEnter);
}

//
// End of "$Id: ReturnButton.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
