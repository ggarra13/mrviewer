//
// "$Id: Alternative.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Theme plugin file for FLTK
//
// Copyright 1999 Bill Spitzak and others.
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
#include <fltk/Widget.h>
#include <fltk/Button.h>
#include <fltk/draw.h>

// a couple of of new boxtypes (look familiar?)
static const fltk::FrameBox
alt_thick_down_box(0, "NNUUJJXXAAAA");

static const fltk::FrameBox
alt_thick_up_box(0, "AAAAXXJJUUNN", &alt_thick_down_box);

enum {UPPER_LEFT, LOWER_RIGHT};

static void lozenge(int which, int x,int y,int w,int h, fltk::Color color)
{
  w--; h--;
  int d = w <= h ? w : h;
  if (d <= 1) return;
  fltk::setcolor(color);
  if (which == UPPER_LEFT) {
    fltk::strokearc(x+w-d, y, d, d, 45, w<=h ? 180 : 90);
    fltk::strokearc(x, y+h-d, d, d, w<=h ? 180 : 90, 225);
  } else { // LOWER_RIGHT
    fltk::strokearc(x, y+h-d, d, d, 225, w<=h ? 360 : 270);
    fltk::strokearc(x+w-d, y, d, d, w<=h ? 360 : 270, 360+45);
  }
  if (w < h) {
    if (which != UPPER_LEFT) fltk::drawline(x+w, y+d/2, x+w, y+h-d/2);
    if (which != LOWER_RIGHT) fltk::drawline(x, y+d/2, x, y+h-d/2);
  } else if (w > h) {
    if (which != UPPER_LEFT) fltk::drawline(x+d/2, y+h, x+w-d/2, y+h);
    if (which != LOWER_RIGHT) fltk::drawline(x+d/2, y, x+w-d/2, y);
  }
}

static fltk::Style* scrollbarstyle;

// Replace the glyph functions on several widget types:
static void
check_glyph(const fltk::Widget* widget, int t,
		int x, int y, int w, int h, fltk::Flags f)
{
  fltk::Color gbc = widget->color();
  fltk::Color gfc = fltk::inactive(widget->textcolor(), f);
  fltk::Color bc;
  if (f & fltk::SELECTED) bc = widget->selection_color();
  else if (f & fltk::HIGHLIGHT && (bc = widget->highlight_color())) ;
  else bc = widget->buttoncolor();

  if (widget->type() == fltk::Button::RADIO) {

    fltk::Color light = 54, dark = 32;
    if (f&fltk::INACTIVE) {
      light = fltk::inactive(light); dark = fltk::inactive(dark);
    }
    int d = w <= h ? w : h;
//  d = (d - 1)|1;
    d &= (~1);

    fltk::setcolor((f&fltk::VALUE) ? gfc : gbc);
    fltk::fillpie(x+2, y+2, d-4, d-4, 0, 360);

    lozenge(UPPER_LEFT,  x+2, y+1, d-4, d-2, light);
    lozenge(UPPER_LEFT,  x+1, y+2, d-2, d-4, light);
    lozenge(UPPER_LEFT,  x+1, y+1, d-2, d-2, light);
    lozenge(UPPER_LEFT,  x+2, y+2, d-4, d-4, light);

    lozenge(LOWER_RIGHT, x+2, y+1, d-4, d-2, dark);
    lozenge(LOWER_RIGHT, x+1, y+2, d-2, d-4, dark);
    lozenge(LOWER_RIGHT, x+1, y+1, d-2, d-2, dark);
    lozenge(LOWER_RIGHT, x+2, y+2, d-4, d-4, dark);

    lozenge(LOWER_RIGHT, x+1, y,   d-2, d,   light);
    lozenge(LOWER_RIGHT, x,   y+1, d,   d-2, light);
    lozenge(LOWER_RIGHT, x,   y,   d,   d,   light);
    lozenge(LOWER_RIGHT, x+1, y+1, d-2, d-2, light);

    lozenge(UPPER_LEFT,  x+1, y,   d-2, d,   dark);
    lozenge(UPPER_LEFT,  x,   y+1, d,   d-2, dark);
    lozenge(UPPER_LEFT,  x,   y,   d,   d,   dark);
    lozenge(UPPER_LEFT,  x+1, y+1, d-2, d-2, dark);

  } else { // check button:

    w = (w-1)|1; h = (h-1)|1;
    int x1 = x+w/2;
    int y1 = y+h/2;
    fltk::Color light = 54, dark = 32;

    if (f&fltk::INACTIVE) {
      /*bc = fltk::inactive(bc); fc = fltk::inactive(fc);*/
      light = fltk::inactive(light); dark = fltk::inactive(dark);
    }
    fltk::newpath();
    fltk::addvertex(x+3, y1); fltk::addvertex(x1, y+3);
    fltk::addvertex(x+w-4, y1); fltk::addvertex(x1, y+h-4);
    fltk::setcolor((f&fltk::VALUE) ? gfc : gbc);
    fltk::fillpath();

    fltk::setcolor(dark);
    fltk::drawline(x,   y1, x1,  y);  fltk::drawline(x1, y,   x+w-1, y1);
    fltk::drawline(x+1, y1, x1, y+1); fltk::drawline(x1, y+1, x+w-2, y1);
    fltk::setcolor(light);
    fltk::drawline(x+2, y1, x1, y+2); fltk::drawline(x1, y+2, x+w-3, y1);
    fltk::drawline(x+3, y1, x1, y+3); fltk::drawline(x1, y+3, x+w-4, y1);

    fltk::setcolor(light);
    fltk::drawline(x,   y1, x1, y+h-1); fltk::drawline(x1, y+h-1, x+w-1, y1);
    fltk::drawline(x+1, y1, x1, y+h-2); fltk::drawline(x1, y+h-2, x+w-2, y1);
    fltk::setcolor(dark);
    fltk::drawline(x+2, y1, x1, y+h-3); fltk::drawline(x1, y+h-3, x+w-3, y1);
    fltk::drawline(x+3, y1, x1, y+h-4); fltk::drawline(x1, y+h-4, x+w-4, y1);
  }
}

// The main glyph function draws all the arrows:
static void
alt_glyph(const fltk::Widget* widget, int t,
          int x, int y, int w, int h, fltk::Flags f)
{
  fltk::Color bc;
  if (f & fltk::SELECTED) bc = widget->selection_color();
  else if (f & fltk::HIGHLIGHT && (bc = widget->highlight_color())) ;
  else bc = widget->buttoncolor();

  switch (t) {
    case fltk::GLYPH_RIGHT_BUTTON:
    case fltk::GLYPH_LEFT_BUTTON:
    case fltk::GLYPH_UP_BUTTON:
    case fltk::GLYPH_DOWN_BUTTON:
      // erase area behind scrollbars arrows
      fltk::setcolor(widget->color());
      fltk::fillrect(x,y,w,h);
      t -= (fltk::GLYPH_RIGHT_BUTTON-fltk::GLYPH_RIGHT);
      // make it square:
      if (w > h) {x += (w-h)/2; w = h;} else {y += (h-w)/2; h = w;}
      goto JUMP1;
    case fltk::GLYPH_RIGHT:
    case fltk::GLYPH_LEFT:
    case fltk::GLYPH_UP:
    case fltk::GLYPH_DOWN:
    JUMP1:
      {fltk::Color d1, d2, l1, l2;
      if (f&(fltk::VALUE|fltk::SELECTED)) {
        d1 = fltk::GRAY99; d2 = fltk::GRAY85; l1 = fltk::BLACK; l2 = fltk::GRAY60;
      } else{
        l1 = fltk::GRAY99; l2 = fltk::GRAY85; d1 = fltk::BLACK; d2 = fltk::GRAY60;
      }

      if (f&fltk::INACTIVE) {
        l1 = fltk::inactive(l1); l2 = fltk::inactive(l2);
        d1 = fltk::inactive(d1); d2 = fltk::inactive(d2);
      }

      if (t == fltk::GLYPH_RIGHT) {
        fltk::setcolor(bc);
        fltk::newpath();
        fltk::addvertex(x,y); fltk::addvertex(x+w-1, y+h/2); fltk::addvertex(x, y+h-1);
        fltk::fillpath();
        fltk::setcolor(l2); fltk::drawline(x+1,y+h-2, x+1,y+1); fltk::drawline(x+1,y+1, x+w-2,y+h/2);
        fltk::setcolor(d2); fltk::drawline(x+1,y+h-2, x+w-2,y+h/2);
        fltk::setcolor(l1); fltk::drawline(x,y+h-1, x,y); fltk::drawline(x,y, x+w-1,y+h/2);
        fltk::setcolor(d1); fltk::drawline(x,y+h-1, x+w-1,y+h/2);
      } else if (t == fltk::GLYPH_LEFT) {
        fltk::setcolor(bc);
        fltk::newpath();
        fltk::addvertex(x+w-1,y); fltk::addvertex(x+w-1,y+h-1); fltk::addvertex(x,y+h/2);
        fltk::fillpath();
        fltk::setcolor(d2); fltk::drawline(x+w-2,y+1, x+w-2,y+h-2); fltk::drawline(x+w-2,y+h-2, x+1,y+h/2);
        fltk::setcolor(l2); fltk::drawline(x+w-2,y+1, x+1,y+h/2);
        fltk::setcolor(d1); fltk::drawline(x+w-1,y, x+w-1,y+h-1); fltk::drawline(x+w-1,y+h-1, x,y+h/2);
        fltk::setcolor(l1); fltk::drawline(x+w-1,y, x,y+h/2);
      } else if (t == fltk::GLYPH_DOWN) {
        fltk::setcolor(bc);
        fltk::newpath();

        fltk::addvertex(x,y); fltk::addvertex(x+w/2,y+h-1); fltk::addvertex(x+w-1,y);
        fltk::fillpath();
        fltk::setcolor(l2); fltk::drawline(x+w-2,y+1, x+1,y+1); fltk::drawline(x+1,y+1, x+w/2,y+h-2);
        fltk::setcolor(d2); fltk::drawline(x+w-2,y+1, x+w/2,y+h-2);
        fltk::setcolor(l1); fltk::drawline(x+w-1,y, x,y); fltk::drawline(x,y, x+w/2,y+h-1);
        fltk::setcolor(d1); fltk::drawline(x+w-1,y, x+w/2,y+h-1);
      } else { // UP
        fltk::setcolor(bc);
        fltk::newpath();
        fltk::addvertex(x,y+h-1); fltk::addvertex(x+w-1,y+h-1); fltk::addvertex(x+w/2,y);
        fltk::fillpath();
        fltk::setcolor(d2); fltk::drawline(x+1,y+h-2, x+w-2,y+h-2); fltk::drawline(x+w-2,y+h-2, x+w/2,y+1);
        fltk::setcolor(l2); fltk::drawline(x+1,y+h-2, x+w/2,y+1);
        fltk::setcolor(d1); fltk::drawline(x,y+h-1, x+w-1,y+h-1); fltk::drawline(x+w-1,y+h-1, x+w/2,y);
        fltk::setcolor(l1); fltk::drawline(x,y+h-1, x+w/2,y);
      }
      break;
    }
    default:
      widget->buttonbox()->draw(x,y,w,h, bc, f);
  }
}

// Sliders. You need to look at the type of widget to figure out if
// it is horizontal or vertical.
static void
slider_glyph(const fltk::Widget* widget, int t,
          int x, int y, int w, int h, fltk::Flags f)
{
  // draw the up/down arrows using the main glyph function:
  if (t) {
    alt_glyph(widget, t, x, y, w, h, f);
    return;
  }
  fltk::Color bc;
  if (f & fltk::SELECTED) bc = widget->selection_color();
  else if (f & fltk::HIGHLIGHT && (bc = widget->highlight_color())) ;
  else bc = widget->buttoncolor();
  if (widget->type() & 1) { // horizontal
    widget->buttonbox()->draw(x,y,w,h, bc, f);
    widget->buttonbox()->inset(x,y,w,h);
    if (w>10) fltk::THIN_UP_BOX->draw(x+w/2-1, y+1, 2, h-2, bc, f);
    if (w>18) {
      fltk::THIN_UP_BOX->draw(x+w/2-1-4, y+1, 2, h-2, bc, f);
      fltk::THIN_UP_BOX->draw(x+w/2-1+4, y+1, 2, h-2, bc, f);
    }
  } else {
    widget->buttonbox()->draw(x,y,w,h, bc, f);
    widget->buttonbox()->inset(x,y,w,h);
    if (h>10) fltk::THIN_UP_BOX->draw(x+1, y+h/2-1, w-2, 2, bc, f);
    if (h>18) {
      fltk::THIN_UP_BOX->draw(x+1, y+h/2-1-4, w-2, 2, bc, f);
      fltk::THIN_UP_BOX->draw(x+1, y+h/2-1+4, w-2, 2, bc, f);
    }
  }
}

static void choice_glyph(const fltk::Widget* widget, int,
                         int x,int y,int w, int h, fltk::Flags f)
{
  fltk::FLAT_BOX->draw(x,y,w,h,widget->color(),f);
  int H = h/3;
  int Y = y + (h-H)/2;
  widget->buttonbox()->draw(x,Y,w,H,widget->buttoncolor(),f);
}

static void light_glyph(const fltk::Widget* widget, int,
                        int x,int y,int w, int h, fltk::Flags f)
{
  fltk::Color fc = fltk::inactive(widget->textcolor(), f);
  fltk::Color bc = widget->color();
  fltk::DOWN_BOX->draw(x+2, y, w-4, h, bc, f&(~fltk::VALUE));
  fltk::THIN_UP_BOX->draw(x+4, y+2, w-8, h-4, f&fltk::VALUE ? fc : bc, f&(~fltk::VALUE));
}

static void return_glyph(const fltk::Widget*, int,
                         int x,int y,int w,int h, fltk::Flags f)
{
  int size = w; if (h<size) size = h;
  int d = (size+2)/4; if (d<3) d = 3;
  int t = (size+9)/12; if (t<1) t = 1;
  int x0 = x+(w-2*d-2*t-1)/2;
  int x1 = x0+d;
  int y0 = y+h/2;
  fltk::setcolor(fltk::inactive(fltk::GRAY33, f));
  fltk::drawline(x0, y0, x1, y0+d);
  fltk::drawline(x1, y0+d, x1, y0+t);
  fltk::drawline(x1, y0+t, x1+d+2*t, y0+t);
  fltk::drawline(x1+d+2*t, y0+t, x1+d+2*t, y0-d);
  fltk::drawline(x1, y0-t, x1, y0-d);
  fltk::setcolor(fltk::inactive(fltk::GRAY90, f));
  fltk::drawline(x0, y0, x1, y0-d);
  fltk::drawline(x1+1, y0-t, x1+d, y0-t);
  fltk::drawline(x1+d, y0-t, x1+d, y0-d);
  fltk::drawline(x1+d, y0-d, x1+d+2*t, y0-d);
}

extern "C" bool fltk_theme() {
  //fltk::Style::draw_sliders_pushed = 1;

  fltk::Widget::default_style->glyph = alt_glyph;

  fltk::Style* s;

  if ((s = fltk::Style::find("menu bar"))) {
    s->buttonbox = fltk::HIGHLIGHT_UP_BOX;
  }

  if ((s = fltk::Style::find("item"))) {
    s->glyph = check_glyph;
    s->buttonbox = fltk::NO_BOX;
  }

  if ((s = fltk::Style::find("menu button"))) {
    s->glyph = alt_glyph;
  }

  if ((s = fltk::Style::find("choice"))) {
    s->box = fltk::UP_BOX;
    s->glyph = choice_glyph;
  }

  if ((s = fltk::Style::find("check button"))) {
    s->glyph = check_glyph;
  }

  if ((s = fltk::Style::find("return button"))) {
    s->glyph = return_glyph;
  }

  if ((s = fltk::Style::find("light button"))) {
    s->glyph = light_glyph;
  }

  if ((s = fltk::Style::find("scrollbar"))) {
    scrollbarstyle = s;
    s->glyph = slider_glyph;
  }

  if ((s = fltk::Style::find("slider"))) {
    s->glyph = slider_glyph;
  }

  if ((s = fltk::Style::find("value slider"))) {
    s->glyph = slider_glyph;
  }

  if ((s = fltk::Style::find("input browser"))) {
    s->glyph = alt_glyph;
  }

  return true;
}

//
// End of "$Id: Alternative.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
