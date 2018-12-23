//
// "$Id: CycleButton.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/CycleButton.h>
#include <fltk/Button.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/Item.h>
#include <fltk/draw.h>
using namespace fltk;

/*! \class fltk::CycleButton

  This widget lets the user select one of a set of choices by clicking
  on it. Each click cycles to the next choice. Holding down any shift
  key or using the middle or right mouse button cycles backwards.

  Notice that the number of items can be 2. In this case this widget
  serves the common purpose of a "toggle" button that shows the
  current on/off state by changing it's label.

  This is a subclass of Menu. The possible states are defined by using
  Menu::add() or other methods that define the menu items. You can
  also put a different callback on each item. Or you can replace this
  widget's callback with your own and use value() to get the index of
  the current setting.  Items that are not visible() or are not
  active() are skipped by the cycling.

  If you set buttonbox() to NO_BOX then you must define your items to
  draw identical-sized and fully opaque images, so that drawing one
  completely obscures any other one. This was done to avoid blinking
  when drawing "artistic" user interfaces where all the entire button
  is an image.

*/

extern bool fl_hide_underscore;

static CycleButton* held_down;

void CycleButton::draw() {

  // this code is copied from Button, but simplified as a lot of
  // back-compatability and the glyphs are eliminated:

  Flags flags = this->flags();
  if (this == held_down) flags |= PUSHED;

  Style style = *(this->style());
  if (!style.color_) style.color_ = style.buttoncolor();
  if (!style.textcolor_) style.textcolor_ = style.labelcolor();

  Box* box = style.box_ ? style.box_ : style.buttonbox();
  Rectangle r(w(),h());

  if (!box->fills_rectangle()) {
    Color bg = highlight_color();
    if ((flags&HIGHLIGHT) && bg) {
      // highlight fills the rectangle, this is for NO_BOX
      setcolor(bg);
      fillrect(r);
    } else if (damage()&DAMAGE_EXPOSE ||
	       (bg && (damage()&DAMAGE_HIGHLIGHT))) {
      draw_background();
    }
  }
  drawstyle(&style,flags);
  box->draw(r);
  Rectangle r1(r); box->inset(r1);

  // This portion of the code is copied from Choice:
  Widget* o = get_item();
  if (o) {
    Item::set_style(&style,false);
    Flags saved = o->flags();
    o->clear_flag(SELECTED);
    if (flags&(INACTIVE|INACTIVE_R)) o->set_flag(INACTIVE_R);
    push_clip(r);
    push_matrix();
    translate(r1.x(),r1.y());
    int save_w = o->w(); o->w(r1.w());
    int save_h = o->h(); o->h(r1.h());
    fl_hide_underscore = true;
    o->draw();
    fl_hide_underscore = false;
    Item::clear_style();
    o->w(save_w);
    o->h(save_h);
    o->flags(saved);
    pop_matrix();
    pop_clip();
  }

  drawstyle(&style,flags);
  box->draw_symbol_overlay(r);
}

static bool try_item(CycleButton* choice, int i) {
  Widget* w = choice->child(i);
  if (!w->takesevents()) return false;
  choice->value(i);
  choice->execute(w);
  return true;
}

int CycleButton::handle(int e) {
  int children = this->children(0,0);
  if (!children) return 0;

  int i,j;

  switch (e) {

  case FOCUS:
  case UNFOCUS:
    redraw(DAMAGE_VALUE);
    return 1;

  case ENTER:
  case LEAVE:
    redraw_highlight();
  case MOVE:
    return 1;

  case PUSH:
  case DRAG:
    if (event_inside(Rectangle(w(),h()))) {
      if (held_down != this) {held_down = this; redraw(DAMAGE_VALUE);}
    } else {
      if (held_down) {held_down = 0; redraw(DAMAGE_VALUE);}
    }
    return 1;
  case RELEASE:
    if (held_down != this) return 0;
    held_down = 0;
    redraw(DAMAGE_VALUE);
    if (event_button()>1 || event_state(CTRL|SHIFT|ALT|META)) goto UP;
    else goto DOWN;

  case SHORTCUT:
    if (test_shortcut()) goto DOWN;
    if (handle_shortcut()) {redraw(DAMAGE_VALUE); return 1;}
    return 0;

  case KEY:
    switch (event_key()) {
    case UpKey:
      goto UP;
    case DownKey:
    case ReturnKey:
    case SpaceKey:
      goto DOWN;
    }
    return 0;

  UP:
    i = value(); if (i < 0) i = 0;
    j = i;
    for (;;) {
      if (--j < 0) j = children-1;
      if (j==i) break;
      if (try_item(this, j)) return 1;
    }
    return 0;

  DOWN:
    i = value(); if (i < 0) i = 0;
    j = i;
    for (;;) {
      if (++j >= children) j = 0;
      if (j == i) break;
      if (try_item(this, j)) return 1;
    }
    return 0;

  default:
    return 0;
  }
}

static NamedStyle style("CycleButton", 0, &CycleButton::default_style);
NamedStyle* CycleButton::default_style = &::style;

CycleButton::CycleButton(int x,int y,int w,int h, const char *l)
  : Menu(x,y,w,h,l)
{
  value(0);
  style(::style);
  clear_flag(ALIGN_MASK);
  set_flag(ALIGN_LEFT);
  //set_click_to_focus();
}

//
// End of "$Id: CycleButton.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
