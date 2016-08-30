//
// "$Id: Scrollbar.cxx 8751 2011-05-28 13:16:09Z bgbnbigben $"
//
// Scroll bar widget for the Fast Light Tool Kit (FLTK).
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
#include <fltk/Scrollbar.h>
#include <fltk/Image.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
using namespace fltk;

/*! \class fltk::Scrollbar

  \image html scrollbar.gif

  The Scrollbar widget displays a slider with arrow buttons at the
  ends of the scrollbar. Clicking on the arrows move up/left and
  down/right by linesize(). If the scrollbar has the keyboard focus
  the arrows move by linesize(), and vertical scrollbars take Page
  Up/Down (they move by the page size minus linesize()) and Home/End
  (they jump to the top or bottom). Often a widget that takes focus
  such as the browser will just send keystrokes to the scrollbar
  directly to get it to move in response.

*/

#define vertical() (!horizontal())

/*! \fn void Scrollbar::pagesize(int)
  How much the pageup/down keys and clicking in the empty area move by.
  If you call value() this is set to 1 linesize() less than the window.
*/

/*! \fn int Scrollbar::value() const
  Return the current position of the scrollbar as an integer.

  Scrollbars have step(1) preset (they always return integers). If
  desired you can set the step() to non-integer values. You will then
  have to use casts to get at the floating-point versions of value()
  from the Slider baseclass.
*/

/*! Set the current position, the range, the pagesize, and the
  slider_size() all at once in a useful way.

  \a p = the position in the data of the first pixel in the window

  \a w = the size of the window

  \a t = the position of the top of your data (typically zero)

  \a l = the total size of your data.

*/
bool Scrollbar::value(int p, int w, int t, int l) {
  if (p+w > t+l) l = p+w-t;
  if (l <= 0) l = 1;
  int b = l-w+t;
  Rectangle r(this->w(),this->h()); box()->inset(r);
  int W,H;
  if (vertical()) {
    W = r.h(); H = r.w(); int T = b; b = t; t = T;
  } else {
    W = r.w(); H = r.h();
  }
  if (W >= 3*H) W -= 2*H;
  int S = W*w/l; if (S < H) S = H; if (S > W) S = W;
  if (S != slider_size() || t != minimum() || b != maximum()) {
    slider_size(S); minimum(t); maximum(b); redraw();
  }
  int ls = int(linesize());
  pagesize(w>2*ls ? w-ls : ls);
  return Slider::value(p);
}

enum {NOTHING = 0, UP_ARROW, DOWN_ARROW, ABOVE_SLIDER, BELOW_SLIDER, SLIDER};

// For the belowmouse() widget, which button is highlighted:
static char which_highlight = 0;
// For the pushed() widget, which button is pushed:
static char which_pushed = 0;


void Scrollbar::increment_cb() {
  double i;
  switch (which_pushed) {
  case UP_ARROW: i = -linesize(); break;
  default:i =  linesize(); break;
  case ABOVE_SLIDER: i = -pagesize(); break;
  case BELOW_SLIDER: i =  pagesize(); break;
  }
  handle_drag(value()+i);
}

#define INITIALREPEAT .5f
#define REPEAT .05f

int Scrollbar::handle(int event) {
  // area of scrollbar:
  Rectangle r(w(),h()); box()->inset(r);

  // adjust slider area to be inside the arrow buttons:
  if (vertical()) {
    if (r.h() >= 3*r.w()) {r.move_y(r.w()); r.move_b(-r.w());}
  } else {
    if (r.w() >= 3*r.h()) {r.move_x(r.h()); r.move_r(-r.h());}
  }

  // which widget part is highlighted?
  int mx = event_x();
  int my = event_y();
  int which_part;
  if (!event_inside(Rectangle(w(), h()))) which_part = NOTHING;
  else if (vertical()) {
    if (my < r.y()) which_part = UP_ARROW;
    else if (my >= r.b()) which_part = DOWN_ARROW;
    else {
      int slidery = slider_position(value(), r.h());
      if (my < r.y()+slidery) which_part = ABOVE_SLIDER;
      else if (my >= r.y()+slidery+slider_size()) which_part = BELOW_SLIDER;
      else which_part = SLIDER;
    }
  } else { // horizontal
    if (mx < r.x()) which_part = UP_ARROW;
    else if (mx >= r.r()) which_part = DOWN_ARROW;
    else {
      int sliderx = slider_position(value(), r.w());
      if (mx < r.x()+sliderx) which_part = ABOVE_SLIDER;
      else if (mx >= r.x()+sliderx+slider_size()) which_part = BELOW_SLIDER;
      else which_part = SLIDER;
    }
  }
  switch (event) {
  case FOCUS:
    return 0;
  case MOVE:
    if (which_part == which_highlight) return 1;
  case ENTER:
    which_highlight = which_part;
    redraw_highlight();
    return 1;
  case LEAVE:
    redraw_highlight();
    return 1;
  case PUSH:
    // Clicking on the slider or middle or right click on the trough
    // gives us normal slider behavior:
    if (which_part == SLIDER ||
	(event_button() > 1 && which_part > DOWN_ARROW)) {
      which_pushed = SLIDER;
      return Slider::handle(event, r);
    }
    handle_push();
    // middle/right click on arrows jumps to that end:
    if (event_button()>1) {
      if (which_part==UP_ARROW) handle_drag(vertical()?maximum():minimum());
      else if (which_part==DOWN_ARROW) handle_drag(vertical()?minimum():maximum());
    }
    goto J1;
  case DRAG:
    if (which_pushed==SLIDER) {redraw(DAMAGE_HIGHLIGHT); return Slider::handle(event, r);}
    if (which_part == SLIDER) which_part = NOTHING;
    // it is okay to switch between arrows and nothing, but no other
    // changes are allowed:
    if (!which_pushed && which_part <= DOWN_ARROW) ;
    else if (!which_part && which_pushed <= DOWN_ARROW) ;
    else which_part = which_pushed;
  J1:
    if (which_part != which_pushed) {
      remove_timeout();
      which_highlight = which_pushed = which_part;
      redraw(DAMAGE_HIGHLIGHT);
      if (which_part) {
	add_timeout(INITIALREPEAT);
	increment_cb();
      }
    }
    return 1;
  case TIMEOUT:
    if (which_pushed) {
      repeat_timeout(REPEAT);
      increment_cb();
    }
    return 1;
  case RELEASE:
    if (which_pushed == SLIDER) {
      Slider::handle(event, r);
    } else if (which_pushed) {
      remove_timeout();
      handle_release();
      redraw(DAMAGE_HIGHLIGHT);
    }
    which_pushed = NOTHING;
    return 1;
  case MOUSEWHEEL: {
    double n = (vertical() ? event_dy() : event_dx())
      * style()->wheel_scroll_lines() * linesize();
    if (fabs(n) > pagesize()) n = (n<0)?-pagesize():pagesize();
    handle_drag(value()+n);
    return 1;
  }
  case KEY:
    if (vertical()) switch(event_key()) {
    case HomeKey: handle_drag(maximum()); return 1;
    case EndKey:  handle_drag(minimum()); return 1;
    case PageUpKey: handle_drag(value()-pagesize()); return 1;
    case PageDownKey: handle_drag(value()+pagesize()); return 1;
    } // else fall through...
  default:
    return Slider::handle(event);
  }
}

void Scrollbar::draw() {
  if (damage()&(DAMAGE_ALL|DAMAGE_HIGHLIGHT)) draw_frame();

  Rectangle r(w(),h()); box()->inset(r);

  char pushed_ = pushed() ? which_pushed : NOTHING;
  char highlight_ = pushed_;
  if (!pushed_ && belowmouse()) highlight_ = which_highlight;

  // 1 = left/top   2 = right/bottom   5 = slider button
  Flags f1, f2, f5;
  f1 = f2 = f5 = flags() | OUTPUT;
  if (pushed_ == UP_ARROW) f1 |= PUSHED;
  if (pushed_ == DOWN_ARROW) f2 |= PUSHED;
  if (pushed_ == SLIDER) f5 |= PUSHED;
  if (highlight_ == UP_ARROW) f5 &=~HIGHLIGHT; else f1 &= ~HIGHLIGHT;
  if (highlight_ == DOWN_ARROW) f5 &=~HIGHLIGHT; else f2 &= ~HIGHLIGHT; 
  Rectangle ir(r); // interior after removing arrow buttons
  if (vertical() && r.h() >= 3*r.w()) {
    Rectangle br(r); br.h(r.w());
    if (damage()&(DAMAGE_ALL|DAMAGE_HIGHLIGHT)) {
      drawstyle(style(),f1);
      draw_glyph(ALIGN_TOP|ALIGN_INSIDE, br);
    }
    br.y(r.b()-r.w());
    if (damage()&(DAMAGE_ALL|DAMAGE_HIGHLIGHT)) {
      drawstyle(style(),f2);
      draw_glyph(ALIGN_BOTTOM|ALIGN_INSIDE, br);
    }
    ir.move_y(r.w()); ir.move_b(-r.w());
  } else if (r.w() >= 3*r.h()) { // horizontal:
    Rectangle br(r); br.w(r.h());
    if (damage()&(DAMAGE_ALL|DAMAGE_HIGHLIGHT)) {
      drawstyle(style(),f1);
      draw_glyph(ALIGN_LEFT|ALIGN_INSIDE, br);
    }
    br.x(r.r()-r.h());
    if (damage()&(DAMAGE_ALL|DAMAGE_HIGHLIGHT)) {
      drawstyle(style(),f2);
      draw_glyph(ALIGN_RIGHT|ALIGN_INSIDE, br);
    }
    ir.move_x(r.h()); ir.move_r(-r.h());
  }
  setcolor(color());
  fillrect(ir);
  Slider::draw(ir, f5, false);
}

// this stops the scrollbar from being pushed-in:
static class ScrollBarGlyph : public Symbol {
public:
  void _draw(const Rectangle& r) const {
    // If the box is being drawn, don't draw pushed in and turn on the box:
    if (!(drawflags()&ALIGN_MASK)) setdrawflags(drawflags()&~PUSHED);
    Widget::default_glyph->draw(r);
  }
  ScrollBarGlyph() : Symbol() {}
} glyph;

static void revert(Style* s) {
  s->box_ = FLAT_BOX;
  s->color_ = GRAY60;
  s->glyph_ = &glyph;
}
static NamedStyle style("Scrollbar", revert, &Scrollbar::default_style);
NamedStyle* Scrollbar::default_style = &::style;

Scrollbar::Scrollbar(int X, int Y, int W, int H, const char* L)
  : Slider(X, Y, W, H, L)
{
  style(default_style);
  step(1);
  linesize(1);
  pagesize_ = 1;
}

//
// End of "$Id: Scrollbar.cxx 8751 2011-05-28 13:16:09Z bgbnbigben $".
//
