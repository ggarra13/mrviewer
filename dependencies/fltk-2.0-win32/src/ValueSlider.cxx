// "$Id: ValueSlider.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/ValueSlider.h>
#include <fltk/Group.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/math.h>
#include <stdlib.h>
using namespace fltk;

/*! \class fltk::ValueSlider

  Controls a single floating point value through a combination of a
  FloatInput and a Slider.

  \image html value_slider.gif

  As this is a subclass of Slider, all slider methods work, for
  setting the slider size, tick marks, etc.

  The user can type a new value into the input area.  If step() is
  greater or equal to 1.0 an IntInput is used, this prevents the user
  from typing anything other than digits. If step() is less than one
  then the user can type floating point values with decimal points and
  exponents.

  The user can type \e any value they want into the text field,
  <i>including ones outside the range() or non-multiples of the
  step()</i>. If you want to prevent this, make the callback function
  reset the value to a legal one.

  By default the callback is done when the user moves the slider, when
  they use up/down keys to change the number in the text, or if they
  edit the text, when they hit the Enter key or they click on another
  widget or put the focus on another widget. Changing when() to
  fltk::WHEN_CHANGED will make it do the callback on every edit
  of the text.

  You can get at the input field by using the public "input" instance
  variable. For instance you can clobber the text to a word with
  value_input->input.static_text("word"). You can also set the size
  of it (call layout() first).
*/

/* IMPLEMENTATION NOTE:

  This is *not* an Group even though it has a "child" widget, this
  appears to be a good idea and fltk should support it.  However
  Widget::parent() returns an Group, not an Widget like it did in fltk
  1.1 so this may not be proper C++.
*/

void ValueSlider::input_cb(Widget*, void* v) {
  ValueSlider& t = *(ValueSlider*)v;
  double nv;
  if (t.step()>=1.0) nv = strtol(t.input.text(), 0, 0);
  else nv = strtod(t.input.text(), 0);
  if (nv != t.value() || t.when() & WHEN_NOT_CHANGED) {
    t.set_value(nv);
    t.Slider::value_damage();
    if (t.when()) {
      t.clear_changed();
      t.do_callback();
    } else {
      t.set_changed();
    }
  }
}

void ValueSlider::draw() {
  if (damage() & ~DAMAGE_CHILD) {
    input.set_damage(DAMAGE_ALL);
    Flags f2 = flags() & ~FOCUSED;
    if (pushed()) f2 |= PUSHED;
    Box* box = this->box();
    if (!box->fills_rectangle()) draw_background();
    drawstyle(style(),flags()&~HIGHLIGHT);
    Rectangle r(w(),h()); box->draw(r);
    slider_rect(r);
    Slider::draw(r, f2, r.y()==0);
  }
  input.label(label());
  input.align(align());
  //input.copy_style(style());
  input.flags((flags()|CLICK_TO_FOCUS)&(~COPIED_LABEL));
  push_matrix();
  translate(input.x(),input.y());
  input.draw();
  pop_matrix();
  input.set_damage(0);
}

#define INITIALREPEAT .5f
#define REPEAT .1f

int ValueSlider::handle(int event) {
  Rectangle r; slider_rect(r);
  switch (event) {
  case FOCUS:
    focus(&input);
    break;
  case PUSH:
    if (!event_inside(r)) break;
  case DRAG:
  case RELEASE:
  case ENTER:
  case MOVE:
  case LEAVE:
    return Slider::handle(event, r);
  // make any drop replace all the text:
  case DND_ENTER:
  case DND_DRAG:
  case DND_LEAVE:
    // we return false if input has focus so drag-out works:
    return (!input.focused());
  case DND_RELEASE:
    take_focus();
    return 1;
  case PASTE:
    // dropped text produces this, replace all input text with it:
    input.position(0, input.size());
    break;
  case MOUSEWHEEL:
    return Slider::handle(event, r);
  }
  input.type(step()>=1.0 ? FloatInput::INT : FloatInput::FLOAT);
  input.when(when());
  int ret = input.send(event);
  if (!ret) ret = Slider::handle(event, r);
  return ret;
}

// Return the area to draw the slider in:
void ValueSlider::slider_rect(Rectangle& r) {
  r.set(0,0,w(),h()); box()->inset(r);
  if (vertical()) r.move_b(-input.h());
  else r.move_x(input.w());
}

// Resize the input field and put it at the edge:
void ValueSlider::layout() {
  Slider::layout();
  Rectangle r(w(),h()); box()->inset(r);
  if (vertical()) {
    int h = int(input.textsize())+8;
    input.resize(r.x(),r.b()-h,r.w(),h);
  } else {
    int w = r.w()/4;
    if (w < 35) w = 35;
    else if (w > 80) w = 80;
    input.resize(r.x(),r.y(),w,r.h());
  }
  input.layout();
  // Make it initialize any blank text fields:
  if (!input.text()[0]) value_damage();
}

void ValueSlider::value_damage() {
  // Only redraw the text if the numeric value is different..
  if (input.text()[0]) {
    if (step() >= 1) {
      if (strtol(input.text(), 0, 0) == long(value())) return;
    } else {
      // parse the existing text:
      double oldv = strtod(input.text(), 0);
      if (!oldv) {
	if (!value()) return;
      } else {
	if (fabs(fabs(value()/oldv)-1) < 1.2e-7) return;
      }
    }
  }
  char buf[128];
  format(buf);
  input.text(buf);
  //input.position(0, input.size()); // highlight it all
  Slider::value_damage();
}

static inline int nogroup(int x) {Group::current(0); return x;}

ValueSlider::ValueSlider(int x, int y, int w, int h, const char* l)
: Slider(x, y, w, h, l), input(nogroup(x), y, w, h, 0) {
  input.parent((Group*)this); // kludge!
  input.callback(input_cb, this);
  Group::current(parent());
  step(.01);
  when(WHEN_CHANGED|WHEN_ENTER_KEY);
}

ValueSlider::~ValueSlider() {
  input.parent(0); // keep it from calling Group::remove(&input) on this
}

// End of "$Id: ValueSlider.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
