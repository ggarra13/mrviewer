//
// "$Id: Button.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $"
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

#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Button.h>
#include <fltk/Group.h>
#include <fltk/Box.h>
#include <stdlib.h>
using namespace fltk;

/*! \class fltk::Button

  Buttons generate callbacks when they are clicked by the user. You
  control exactly when and how by changing the values for when():
  - fltk::WHEN_NEVER: The callback is not done, instead changed() is
    turned on.
  - fltk::WHEN_RELEASE: This is the default, the callback is done
    after the user successfully clicks the button (i.e. they let it go
    with the mouse still pointing at it), or when a shortcut is typed.
  - fltk::WHEN_CHANGED : The callback is done each time the value()
    changes (when the user pushes and releases the button, and as the
    mouse is dragged around in and out of the button).

  Buttons can also generate callbacks in response to fltk::SHORTCUT
  events. The button can either have an explicit shortcut() value or a
  letter shortcut can be indicated in the label() with an '&'
  character before it. For the label shortcut it does not matter if
  Alt is held down, but if you have an input field in the same window,
  the user will have to hold down the Alt key so that the input field
  does not eat the event first as an fltk::KEY event.

  \image html buttons.gif
*/

/*! \fn bool Button::value() const
  The current value. The ToggleButton subclass provides the ability for
  the user to change this value permanently, otherwise it is just
  temporary while the user is holding the button down.

  This is the same as Widget::state().

  \return true if the button is pushed down, false otherwise
*/

/*! \fn bool Button::value(bool)
  Changes the value() and redraws the button.
  This is the same function as Widget::state().
  See also Widget::set(), Widget::clear(), and Widget::setonly().

  \param v The new value for the button; true for pushed down, false
  for not pushed down.
  If you pass "true" as the value, a normal button will draw 
  pushed-in, until the user clicks it and releases it.
  \return true if the new value is different, false otherwise
*/

static bool initial_state;

/** Wrapper around Button::handle(int event, const Rectangle& rectangle)
    \param event The event that this button has received
    \return 1 if the button has successfully handled the event,\n 0 otherwise
*/
int Button::handle(int event) {
  return handle(event, Rectangle(w(),h()));
}

/** The Button's own handle function. See Widget::handle
    \param event The event to handle
    \param rectangle The rectangle this event occurred within.
    You can change this value, or inspect it in a subclass, to determine
    what you want your button to do if the user clicks on different sections
*/
int Button::handle(int event, const Rectangle& rectangle) {
  switch (event) {
  case ENTER:
  case LEAVE:
    redraw_highlight();
  case MOVE:
    return 1;
  case PUSH:
    if (pushed()) return 1; // ignore extra pushes on currently-pushed button
    initial_state = state();
    clear_flag(PUSHED);
  case DRAG: {
    bool inside = event_inside(rectangle);
    if (inside) {
      if (!flag(PUSHED)) {
        set_flag(PUSHED);
        redraw(DAMAGE_VALUE);
      }
    } else {
      if (flag(PUSHED)) {
        clear_flag(PUSHED);
        redraw(DAMAGE_VALUE);
      }
    }
    if (when() & WHEN_CHANGED) { // momentary button must record state()
      if (state(inside ? !initial_state : initial_state))
        do_callback();
    }
    return 1;}
  case RELEASE:
    if (!flag(PUSHED)) return 1;
    clear_flag(PUSHED);
    redraw(DAMAGE_VALUE);
    if (type() == RADIO)
      setonly();
    else if (type()) // TOGGLE
      state(!initial_state);
    else {
      state(initial_state);
      if (when() & WHEN_CHANGED) {do_callback(); return 1;}
    }
    if (when() & WHEN_RELEASE) do_callback(); else set_changed();
    return 1;
  case FOCUS:
    redraw(1); // minimal redraw to just add the focus box
    // grab initial focus if we are a ReturnButton:
    return shortcut()==ReturnKey ? 2 : 1;
  case UNFOCUS:
    redraw(DAMAGE_HIGHLIGHT);
    return 1;
  case KEY:
    if (event_key() == ' ' || event_key() == ReturnKey
	|| event_key() == KeypadEnter) goto EXECUTE;
    return 0;
  case SHORTCUT:
    if (!test_shortcut()) return 0;
  EXECUTE:
    if (type() == RADIO) {
      if (!state()) {
        setonly();
        if (when() & WHEN_CHANGED) do_callback(); else set_changed();
      }
    } else if (type()) { // TOGGLE
      state(!state());
      if (when() & WHEN_CHANGED) do_callback(); else set_changed();
    }
    if (when() & WHEN_RELEASE) do_callback();
    return 1;
  default:
    return 0;
  }
}

////////////////////////////////////////////////////////////////

#include <fltk/draw.h>

extern Widget* fl_did_clipping;

/*!
  This function provides a mess of back-compatabilty and Windows
  emulation to subclasses of Button to draw with. It will draw the
  button according to the current state of being pushed and it's
  state().
  \param glyph_width If non-zero is passed for \a glyph_width 
  then the glyph() is drawn in that space on the left (or on the 
  right if negative), and it assummes the glyph indicates the 
  state(), so the box is only used to indicate the pushed state.
*/
void Button::draw(int glyph_width) const {
  // For back-compatability, setting color() or box() directly on a plain
  // button will cause it to act like buttoncolor() or buttonbox() are
  // set:
  Style localstyle;
  const Style* style = this->style();
  if (!glyph_width) {
    localstyle = *style;
    if (localstyle.color_) localstyle.buttoncolor_ = localstyle.color_;
    if (localstyle.box_) localstyle.buttonbox_ = localstyle.box_;
    if (localstyle.labelcolor_) localstyle.textcolor_ = localstyle.labelcolor_;
    style = &localstyle;
  }

  Box* box = style->buttonbox();

  Flags box_flags = flags() | OUTPUT;
  Flags glyph_flags = box_flags & ~(HIGHLIGHT|OUTPUT);
  if (glyph_width) box_flags &= ~STATE;

  // only draw "inside" labels:
  Rectangle r(0,0,w(),h());

  if (box == NO_BOX) {
    Color bg;
    if (box_flags & HIGHLIGHT && (bg = style->highlight_color())) {
      setcolor(bg);
      fillrect(r);
    } else if (label() || (damage()&(DAMAGE_EXPOSE|DAMAGE_HIGHLIGHT))) {
      // erase the background so we can redraw the label in the new color:
      draw_background();
    }
    // this allows these buttons to be put into browser/menus:
    //fg = fl_item_labelcolor(this);
  } else {
    if ((damage()&(DAMAGE_EXPOSE|DAMAGE_HIGHLIGHT)) && !box->fills_rectangle()) {
      // Erase the area behind non-square boxes
      draw_background();
    }
  }

  // Draw the box:
  drawstyle(style,box_flags);
  // For back-compatability we use any directly-set selection_color()
  // to color the box:
  if (!glyph_width && state() && style->selection_color_) {
    setbgcolor(style->selection_color_);
    setcolor(contrast(style->selection_textcolor(),style->selection_color_));
  }
  box->draw(r);
  Rectangle r1(r); box->inset(r1);

  if (glyph_width) {
    int g = abs(glyph_width);
    Rectangle lr(r1);
    Rectangle gr(r1, g, g);
    if (glyph_width < 0) {
      lr.w(lr.w()-g-3);
      gr.x(r1.r()-g-3);
    } else {
      lr.set_x(g+3);
      gr.x(r1.x()+3);
    }
    this->draw_label(lr, box_flags);
    drawstyle(style,glyph_flags);
    if (state() && style->selection_color_) {
      setbgcolor(style->selection_color_);
      setcolor(contrast(style->selection_textcolor(),style->selection_color_));
    }
    this->glyph()->draw(gr);
    drawstyle(style,box_flags);
  } else {
    this->draw_label(r1, box_flags);
  }
  box->draw_symbol_overlay(r);
}

/** The virtual draw() function makes sure the button isn't hidden, then
    calls Button::draw(int glyph_width) with a width of 0 
*/
void Button::draw() {
  if (type() == HIDDEN) {
    fl_did_clipping = this;
    return;
  }
  draw(0);
}

////////////////////////////////////////////////////////////////

static NamedStyle style("Button", 0, &Button::default_style);
/** This sets the Button's default style to one with \b no revert function
    It thus inherits its style from the "default" style and from Group::default_style
*/
NamedStyle* Button::default_style = &::style;

/** The default constructor calls Widget() with x, y, w, h and l.
    It also sets the style to the default_style
    \param x The x position of the Button, relative to the group
    \param y The y position of the Button, relative to the group
    \param w The width of the Button
    \param h The height of the Button
    \param l The Button's label
*/
Button::Button(int x,int y,int w,int h, const char *l) : Widget(x,y,w,h,l) {
  style(default_style);
  //set_click_to_focus();
}

////////////////////////////////////////////////////////////////

/*! \class fltk::ToggleButton
  This button turns the state() on and off each release of a click
  inside of it.

  You can also convert a regular button into this by doing
  type(Button::TOGGLE) to it.
*/

//
// End of "$Id: Button.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
