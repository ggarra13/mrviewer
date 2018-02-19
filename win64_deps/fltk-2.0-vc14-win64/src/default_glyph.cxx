// "$Id: default_glyph.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/Widget.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
using namespace fltk;

static class DefaultGlyph : public Symbol {
public:
  void _draw(const Rectangle&) const;
  DefaultGlyph() : Symbol("widget") {}
} glyph;

/**
  This is the value of glyph() in the Widget::default_style. It is
  an internal symbol called "@widget;".  It draws a
  number of small buttons with arrows on them. The direction of the
  arrows are determined by the align values in fltk::setflags():

  - fltk::ALIGN_LEFT draws left-pointing arrow
  - fltk::ALIGN_RIGHT draws right-pointing arrow
  - fltk::ALIGN_TOP draws up-pointing arrow
  - fltk::ALIGN_BOTTOM draws right-pointing arrow
  - fltk::ALIGN_INSIDE draws the current drawstyle() buttonbox() around
  the arrow. This will usually use the fltk::PUSHED or other flags
  to decide to draw pushed in or out.
  - if left/right/top/bottom are all off it draws a box as well. This
  is so zero will draw something.
  - fltk::INACTIVE_R draws it grayed out.

  Only one arrow direction at a time is currently supported. This
  may be improved in the future.
*/
Symbol* Widget::default_glyph = &::glyph;

void DefaultGlyph::_draw(const Rectangle& rr) const
{
  Box* box = 0;
  Rectangle r(rr);

  // draw the box:
  if (drawflags(ALIGN_INSIDE) || !(drawflags()&15)) {
    box = drawstyle()->buttonbox();
    box->draw(r);
    box->inset(r);
  }

  // to draw the shape inactive, draw it twice to get the engraved look:
  const Color saved_color = getcolor();
  int i = 0;
  if (drawflags(INACTIVE_R)) {i = 1; setcolor(GRAY99);}

  for (;;) {

    int w1 = ((r.h()<r.w()?r.h():r.w())+2)/3; int x1,y1;
    switch(drawflags()&15) {

    case ALIGN_TOP:
      x1 = r.x()+(r.w()-1)/2-w1+i;
      y1 = r.y()+(r.h()-w1-1)/2+i;
      addvertex(x1, y1+w1);
      addvertex(x1+2*w1, y1+w1);
      addvertex(x1+w1, y1);
      break;

    case ALIGN_BOTTOM:
      x1 = r.x()+(r.w()-1)/2-w1+i;
      y1 = r.y()+(r.h()-w1)/2+i;
      addvertex(x1, y1);
      addvertex(x1+w1, y1+w1);
      addvertex(x1+2*w1, y1);
      break;

    case ALIGN_LEFT:
      x1 = r.x()+(r.w()-w1-1)/2+i;
      y1 = r.y()+(r.h()-1)/2-w1+i;
      addvertex(x1, y1+w1);
      addvertex(x1+w1, y1+2*w1);
      addvertex(x1+w1, y1);
      break;

    case ALIGN_RIGHT:
      x1 = r.x()+(r.w()-w1)/2+i;
      y1 = r.y()+(r.h()-1)/2-w1+i;
      addvertex(x1, y1);
      addvertex(x1+w1, y1+w1);
      addvertex(x1, y1+2*w1);
      break;

    }
    fillstrokepath(getcolor());
    if (i) {i = 0; setcolor(saved_color);}
    else break;
  }
  if (box) box->draw_symbol_overlay(rr);
}

/**
  Changes the lower 5 bits (the "align" bits) of drawflags() to be the
  value of \a which, then draws the glyph(), then put drawflags()
  back. This is a convienence function for widgets that actually need
  to draw several different glyphs. They have to define a glyph whicy
  draws a different image depending on the align flags.  This allows
  the style to be changed by replacing the glyph function, though the
  replacement should draw the same things for the align flags, perhaps
  by being an fltk::MultiImage.
*/
void Widget::draw_glyph(int which, const Rectangle& rectangle) const {
  int savedflags = drawflags_;
  drawflags_ = (savedflags&~ALIGN_MASK) | which;
  glyph()->draw(rectangle);
  drawflags_ = savedflags;
}

//
// End of "$Id: default_glyph.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
