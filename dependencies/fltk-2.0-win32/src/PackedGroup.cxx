// "$Id: PackedGroup.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/PackedGroup.h>
#include <fltk/layout.h>
#include <fltk/Box.h>
using namespace fltk;

/** \class fltk::PackedGroup

  Resizes all the child widgets to be the full width and stacks them.
  All widgets before the resizable() (or all of them if there is no
  resizable()) are pushed against the top of this widget, all widgets
  after the resizable() are pushed against the bottom, and the resizable()
  fills the remaining space. Child widgets that have vertical() true
  are pushed against the left/right edge.

  \image html Fl_Pack.gif

  This is most useful for layout menu/toolbars and work areas in your
  main window. You can get many layouts with this, except the top
  widgets always extend to the right edge and the left widgets always
  extend to the bottom. To change this, put the top or left widgets
  and resizable in a PackedGroup and nest that inside another one with
  the bottom and right widgets.

  A child widget can change it's size by calling layout() on itself
  and this will rearrange all other widgets to accomodate the new
  height.

  If resizable is not set, the PackedGroup itself resizes to surround
  the items, allowing it to be imbedded in a surrounding PackedGroup
  or ScrollGroup. This only works if all objects are horizontal or
  all are vertical.

  Setting type(PackedGroup::ALL_CHILDREN_VERTICAL) will make it
  act like all the children have vertical() set.
*/

/** \fn void PackedGroup::spacing(int);
    Set a fixed-size gap between all the children. This defaults to
    zero. If you change this in an already-existing one, call
    relayout().
*/

#define is_vertical(widget) (type()&1 || widget->vertical())

PackedGroup::PackedGroup(int x,int y,int w,int h,const char *l,bool begin)
: Group(x, y, w, h, l, begin) {
  resizable(0);
  spacing_ = 0;
  margin(0);
}

void PackedGroup::layout() {
  for (int iter = 0; iter < 2; iter++) {
    if (!layout_damage()) break;

    // we only need to do something special if the group is resized:
    if (!(layout_damage() & (LAYOUT_WH|LAYOUT_DAMAGE)) || !children()) {
      Group::layout();
      if (!(layout_damage() & LAYOUT_CHILD)) break;
    }
    int extradamage = layout_damage()&LAYOUT_DAMAGE;

    // clear the layout flags, so any resizes of children will set them again:
    Widget::layout();

    // This is the rectangle to lay out the remaining widgets in:
    Rectangle r(w(),h());
    box()->inset(r);

    // Apply margins
    r.set_x(r.x()+margin_left_);
    r.set_r(r.r()-margin_right_);
    r.set_y(r.y()+margin_top_);
    r.set_b(r.b()-margin_bottom_);

    bool saw_horizontal = false;
    bool saw_vertical = false;

    // layout all the top & left widgets (the ones before the resizable):
    int i; for (i = 0; i < children(); i++) {
      Widget* widget = child(i);
      if (widget->contains(resizable())) break;
      if (!widget->visible()) continue;
      if (is_vertical(widget)) {
	widget->resize(r.x(), r.y(), widget->w(), r.h());
        widget->layout_damage(widget->layout_damage()|extradamage);
	widget->layout();
	r.move_x(widget->w()+spacing_);
	saw_vertical = true;
      } else { // put along top edge:
	widget->resize(r.x(), r.y(), r.w(), widget->h());
        widget->layout_damage(widget->layout_damage()|extradamage);
	widget->layout();
	r.move_y(widget->h()+spacing_);
	saw_horizontal = true;
      }
    }

    int resizable_index = i;

    // layout all the bottom & right widgets by going backwards:
    for (i = children()-1; i > resizable_index; i--) {
      Widget* widget = child(i);
      if (!widget->visible()) continue;
      if (is_vertical(widget)) {
	int W = widget->w();
	widget->resize(r.r()-W, r.y(), W, r.h());
        widget->layout_damage(widget->layout_damage()|extradamage);
	widget->layout();
	r.set_r(widget->x()-spacing_);
	saw_vertical = true;
      } else { // put along top edge:
	int H = widget->h();
	widget->resize(r.x(), r.b()-H, r.w(), H);
        widget->layout_damage(widget->layout_damage()|extradamage);
	widget->layout();
	r.set_b(widget->y()-spacing_);
	saw_horizontal = true;
      }
    }

    // Lay out the resizable widget to fill the remaining space:
    if (resizable_index < children()) {
      Widget* widget = child(resizable_index);
      widget->resize(r.x(), r.y(), r.w(), r.h());
      widget->layout_damage(widget->layout_damage()|extradamage);
      widget->layout();
    }

    // A non-resizable widget will become the size of its items:
    int W = w();
    if (r.w()<0 || !(resizable() || saw_horizontal)) {
      W -= r.w()+(saw_vertical?spacing_:0);
    }
    int H = h();
    if (r.h()<0 || !(resizable() || saw_vertical)) {
      H -= r.h()+(saw_horizontal?spacing_:0);
    }
    Widget::resize(W,H);
  }
}

//
// End of "$Id: PackedGroup.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
