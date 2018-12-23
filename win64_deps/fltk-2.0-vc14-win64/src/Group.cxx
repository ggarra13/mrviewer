//
// "$Id: Group.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Group widget for the Fast Light Tool Kit (FLTK).
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

/*! \class fltk::Group

The fltk::Group class is the FLTK container widget. It maintains an
array of child widgets. These children can themselves be any widget
including fltk::Group, nesting groups allows much more control over
layout and resize behavior. Nested groups are also necessary to group
radio buttons together.

By default fltk::Group preserves the positions and sizes of all it's
children, they do not move no matter what sizes or other children are
added or removed.

Setting resizable() will change the layout behavior so that it
responds to resizing by moving or resizing the children to fit. See
below for details.

You may want to use an fltk::Pack or a fltk::Scroll to get other
common layout behavior that can respond to changes in the sizes of
child widgets.

The most-used subclass of fltk::Group is fltk::Window, all the rules
about resizing apply to windows. Setting resizable() on a window will
allow the user to resize it. If you want different behavior (such as
from fltk::Pack) for your window you should make the window have that
as a single resizable child that fills it.

fltk::Menu is a subclass and thus all menus and browsers are groups
and the items in them are widgets.

*/

#include <config.h>
#include <fltk/Window.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/layout.h>
#include <fltk/damage.h>
#include <stdlib.h>
#include <string.h>

using namespace fltk;

////////////////////////////////////////////////////////////////

FL_API Group* Group::current_;

static void revert(Style* s) {
  s->color_ = GRAY75;
  s->box_ = NO_BOX;
}

// This style is unnamed since there is no reason for themes to change it:
extern NamedStyle* group_style;
static NamedStyle the_style(0, revert, &group_style);
NamedStyle* group_style = &the_style;

/*! Creates a new fltk::Group widget using the given position, size,
  and label string. The default boxtype is fltk::NO_BOX. */
Group::Group(int X,int Y,int W,int H,const char *l,bool begin)
: Widget(X,Y,W,H,l),
  children_(0),
  focus_index_(-1),
  array_(0),
  resize_align_(ALIGN_TOPLEFT|ALIGN_BOTTOMRIGHT),
  sizes_(0)
{
  resizable_ = this;
  type(GROUP_TYPE);
  style(::group_style);
  align(ALIGN_TOP);
  initial_w = w();
  initial_h = h();
  if (begin) this->begin();
}

/*! \e Deletes all children from the group and makes it empty.
  This calls the destructor on all the children!!! */
void Group::clear() {
  init_sizes();
  if (children_) {
    Widget*const* a = array_;
    Widget*const* e = a+children_;
    // clear everything now, in case fix_focus recursively calls us:
    children_ = 0;
    focus_index_ = -1;
    if (resizable_) resizable_ = this;
    // okay, now it is safe to destroy the children:
    while (e > a) {
      Widget* o = *--e;
      o->parent(0); // stops it from calling remove()
      delete o;
    }
  }
  delete[] const_cast<Widget**>( array_ );
  array_ = 0;
}

/*! Calls clear(), and thus <i>deletes all child widgets</i> */
Group::~Group() {current_ = 0; clear();}

/*! \fn Widget * Group::child(int n) const
  Returns a child, n >= 0 && n < children(). <i>No range checking is done!</i>
*/

/*! \fn int Group::children() const
  Returns how many child widgets the group has. */

/*! This does insert(w, find(beforethis)). This will append the
  widget if beforethis is not in the group. */
void Group::insert(Widget &o, int index) {
  if (o.parent()) {
    int n = o.parent()->find(o);
    if (o.parent() == this) {
      if (index > n) index--;
      if (index == n) return;
    }
    o.parent()->remove(n);
  }
  o.parent(this);
  if (children_ == 0) {
    // allocate for 1 child
    if (!array_) array_ = new Widget*[1];
    array_[0] = &o;
  } else {
    if (!(children_ & (children_-1))) {// double number of children
      Widget** newarray = new Widget*[2*children_];
      memcpy(newarray, array_, children_*sizeof(Widget*));
      delete[] array_;
      array_ = newarray;
    }
    for (int j = children_; j > index; --j) array_[j] = array_[j-1];
    array_[index] = &o;
  }
  ++children_;
  // fix the INACTIVE_R flag:
  if ( active_r() && o.active() ) {
    if ( o.flag(INACTIVE_R) ) {
      o.clear_flag(INACTIVE_R); o.handle(ACTIVATE);
    }
  } else {
    if ( !o.flag(INACTIVE_R) ) {
      o.set_flag(INACTIVE_R); o.handle(DEACTIVATE);
    }
  }
  // fix any internal visibility:
  if ( o.visible_r() ) {
    o.handle( SHOW );
  }
  init_sizes();
}

/*! \fn Group * Group::current()
  Returns the group being currently built. The fltk::Widget
  constructor automatically does current()->add(widget) if this is not
  null. To prevent new widgets from being added to a group, call
  Group::current(0).
*/

/*! \fn void Group::begin()

  begin() sets the current group so you can build the widget tree by
  just constructing the widgets. begin() is exactly the same as current(this).

  <i>Don't forget to end() the group or window!</i>
*/

/*! \fn void Group::end()
  end() is exactly the same as current(this->parent()). Any new
  widgets added to the widget tree will be added to the parent of the
  group.
*/

/*! The widget is removed from it's current group (if any) and then
  added to the end of this group. */
void Group::add(Widget &o) {
  insert(o, children_);
}

/*! Remove the indexed widget from the group. */
void Group::remove(int index) {
  if (index >= children_) return;
  Widget* o = array_[index];
  // we must redraw the enclosing group that has an opaque box:
  if (o->visible_r())
    for (Widget *p = this; p; p = p->parent())
      if (p->box() != NO_BOX || !p->parent()) {p->redraw(); break;}
  o->parent(0);
  children_--;
  for (int i=index; i < children_; ++i) array_[i] = array_[i+1];
  init_sizes();
  redraw();
}

/*! \fn void Group::remove(Widget& widget)
  Removes a widget from the group. This does nothing if the widget is
  not currently a child of this group. */

/*! \fn void Group::remove_all()
  Removes all widgets from the group.  This does not call the destructor
  on the child widget (see clear()). */
void Group::remove_all()
{
  int count = children();
  while ( count-- ) {
    remove( count );
  }
}

/*! Remove the indexed widget and insert the passed widget in it's place. */
void Group::replace(int index, Widget& o) {
  if (index >= children_) {add(o); return;}
  o.parent(this);
  array_[index]->parent(0);
  array_[index] = &o;
  init_sizes();
}

/*! \fn void Group::replace(Widget& old, Widget& nu)
  Find the \a old widget and remove it and insert the \a nu one. If \a old
  is not a child, the new widget is appended to the end of the
  list. */

void Group::swap(int indexA, int indexB) {
  if (indexA >= children_ || indexB >= children_) return;
  Widget* o = array_[indexA];
  array_[indexA] = array_[indexB];
  array_[indexB] = o;
  init_sizes();
}

/*! Searches the children for \a widget, returns the index of \a
  widget or of a parent of \a widget that is a child() of
  this. Returns children() if the widget is NULL or not found. */
int Group::find(const Widget* widget) const {
  for (;;) {
    if (!widget) return children_;
    if (widget->parent() == this) break;
    widget = widget->parent();
  }
  // Search backwards so if children are deleted in backwards order
  // they are found quickly:
  for (int index = children_; index--;)
    if (array_[index] == widget) return index;
  return children_;
}

////////////////////////////////////////////////////////////////
// Handle

/*! \fn int Group::focus_index() const

  The index of the widget that contains the focus. You can initialize
  this before the group is displayed. Changing it while it is
  displayed does not work, use widget->take_focus() instead.

  For some subclasses such as fltk::TabGroup, a negative number indicates
  that the group itself has the focus. In most cases any illegal value
  means it will search for any widget that accepts focus.

  This is also used by fltk::Menu to locate the item selected by
  the user. See fltk::Menu::get_item().
*/

/*! Turn Tab into Right or Left for keyboard navigation */
int Group::navigation_key() {
  switch (event_key()) {
  case TabKey:
    if (event_state(CTRL)) return 0;
    return event_state(SHIFT) ? LeftKey : RightKey;
  case UpKey:
  case DownKey:
  case LeftKey:
  case RightKey:
    return event_key();
  default:
    return 0;
  }
}

/*! Calls send() on some or all of the children widgets. */
int Group::handle(int event) {
  const int numchildren = children();
  int i;

  switch (event) {

  case FOCUS_CHANGE:
    // The focus is being changed to some widget inside this.
    focus_index_ = find(fltk::focus());
    return true;

  case FOCUS:
    // an attempt to give this widget focus:
    switch (navigation_key()) {
    default: {
      // try to give it to whatever child had focus last:
      if (focus_index_ >= 0 && focus_index_ < numchildren)
	if (child(focus_index_)->take_focus()) return true;
      // otherwise search for the widget that needs the focus. We
      // give it to the first one that returns 2, or the last one
      // that returns 1 if none return 2:
      int new_index = -1; int ret = 0;
      for (i = 0; i < numchildren; ++i) {
	Widget* w = child(i);
	if (w->takesevents() && w->tab_to_focus()) {
	  int n = w->handle(FOCUS);
	  if (n) {ret = n; new_index = i; if (n & 2) break;}
	}
      }
      focus_index_ = new_index;
      return ret;}
    case RightKey:
    case DownKey:
      for (i=0; i < numchildren; ++i)
	if (child(i)->tab_to_focus() && child(i)->take_focus()) return true;
      return false;
    case LeftKey:
    case UpKey:
      for (i = numchildren; i--;)
	if (child(i)->tab_to_focus() && child(i)->take_focus()) return true;
      return false;
    }

  case DRAG:
  case RELEASE:
  case LEAVE:
  case DND_LEAVE:
  case DND_RELEASE:
    // Ignore these. We handle them if the belowmouse of pushed widget
    // has been set to this. Subclasses may do something with these.
    // Definately do not pass them to child widgets!
    break;

  case KEY: {
    // keyboard navigation
    if (numchildren < 1) break;
    int key = navigation_key();
    if (!key) break;
    int previous = focus_index_;
    if (previous < 0 || previous >= numchildren) previous = 0;
    for (i = previous;;) {
      if (key == LeftKey || key == UpKey) {
	if (i) --i;
	else {
	  if (parent()) return false;
	  i = numchildren-1;
	}
      } else {
	++i;
	if (i >= numchildren) {
	  if (parent()) return false;
	  i = 0;
	}
      }
      if (i == previous) {
	// This appears to be for fixing bugs, where the focus gets lost.
	// The child should already have focus.
	focus_index_ = i;
	return child(i)->tab_to_focus() && child(i)->take_focus();
      }
      if (key == DownKey || key == UpKey) {
	// for up/down, the widgets have to overlap horizontally:
	Widget* o = child(i);
	Widget* p = child(previous);
	if (o->x() >= p->x()+p->w() || o->x()+o->w() <= p->x()) continue;
      }
      if (child(i)->tab_to_focus() && child(i)->take_focus()) return true;
    }}

  case PUSH:
  case ENTER:
  case MOVE:
  case DND_ENTER:
  case DND_DRAG:
  case MOUSEWHEEL:
    // search the children in backwards (top to bottom) order:
    for (i = numchildren; i--;) {
      Widget* child = this->child(i);
      // ignore widgets we are not pointing at:
      if (event_x() < child->x()) continue;
      if (event_x() >= child->x()+child->w()) continue;
      if (event_y() < child->y()) continue;
      if (event_y() >= child->y()+child->h()) continue;
      // see if it wants the event:
      if (child->send(event)) return true;
    }
    break;

  default:
    // Try to give any other event to the focus first:
    if (focus_index_ >= 0 && focus_index_ < numchildren)
      if (child(focus_index_)->send(event)) return true;
    // Then try all other children in top to bottom order:
    for (i = numchildren; i--;) if (i != focus_index_)
      if (child(i)->send(event)) return true;
    break;

  }
  return Widget::handle(event);
}

////////////////////////////////////////////////////////////////
// Layout

/*! \fn Widget* Group::resizable() const

  The resizable widget defines the resizing box for the group. When
  the group is resized it calculates a new size and position for all
  of its children. Widgets that are horizontally or vertically inside
  the dimensions of the box are scaled to the new size. Widgets
  outside the box are moved.

  Orignal size, the gray area is the resizable():
  \image html resizebox1.gif

  And here is the same Group resized larger:
  \image html resizebox2.gif

  The resizable may be set to the group itself, in which case all the
  contents are resized. If the resizable is NULL (the default) then
  all widgets remain a fixed size and distance from the top-left
  corner.

  It is possible to achieve any type of resize behavior by using an
  InvisibleBox as the resizable and/or by using a hierarchy of child
  fltk::Group's.
*/

/** Indicate that all the remembered child sizes should be reinitialized.

    The group remembers the initial size of itself and all it's children,
    so that the layout can be restored even if the group is resized so
    that some children go to zero or negative sizes.

    This can produce unwanted behavior if you try to rearrange the
    child widgets yourself, as the next resize will put them right back
    where they were initially. Call this to make it forget all the
    saved sizes and reinitialize them during the next layout().

    This is automatically done when any child is added or removed.  */
void Group::init_sizes() {
  initial_w = w();
  initial_h = h();
  delete[] sizes_; sizes_ = 0;
  relayout();
}

/** Returns array of initial sizes of the widget and it's children.

    The sizes() array stores the initial positions of widgets as
    left,right,top,bottom quads.  The first quad is the group, the
    second is the resizable (clipped to the group), and the
    rest are the children.  This is a convienent order for the
    algorithim.

    This array is not calculated until the first time sizes() is
    called. It then returns the same array from then on. Calling
    init_sizes() deletes the array so the next call of this creates
    a new one.
*/
int* Group::sizes() {
  if (!sizes_) {
    int* p = sizes_ = new int[4*(children_+2)];
    // first thing in sizes array is the group's size:
    p[0] = x();
    p[1] = initial_w;
    p[2] = y();
    p[3] = initial_h;
    // next is the resizable's size:
    p[4] = 0; // init to the group's size
    p[5] = p[1];
    p[6] = 0;
    p[7] = p[3];
    Widget* r = resizable();
    if (r && r != this) { // then clip the resizable to it
      int t;
      t = r->x(); if (t > 0) p[4] = t;
      t +=r->w(); if (t < p[1]) p[5] = t;
      t = r->y(); if (t > 0) p[6] = t;
      t +=r->h(); if (t < p[3]) p[7] = t;
    }
    // next is all the children's sizes:
    p += 8;
    Widget*const* a = array_;
    Widget*const* e = a+children_;
    while (a < e) {
      Widget* o = *a++;
      *p++ = o->x();
      *p++ = o->x()+o->w();
      *p++ = o->y();
      *p++ = o->y()+o->h();
    }
  }
  return sizes_;
}

void Group::layout() {
  int layout_damage = this->layout_damage();
  if (!sizes_) {
    if (!(layout_damage&LAYOUT_W)) initial_w = w();
    if (!(layout_damage&LAYOUT_H)) initial_h = h();
  }
  if (resizable() && layout_damage&(LAYOUT_WH|LAYOUT_DAMAGE))
    layout_damage |= LAYOUT_WH;
  else
    layout_damage &= ~LAYOUT_WH;
  layout(Rectangle(w(),h()),layout_damage);
}

/*!
  Inner implementation of layout(). The child widgets are resized
  and positioned so that the area that was initially surrounded by this
  widget now fits inside the rectangle.

  \a r is a rectangle, in the coordinate system of this Group (ie 0,0
  is the top-left corner of the Group).

  \a layout_damage controls what is done. If LAYOUT_W or LAYOUT_H are
  off then no resizing or moving of widgets in that direction is done.

  This is used by ScrollGroup to resize the widgets to fit inside the
  scrollbars.
*/
void Group::layout(const Rectangle& r, int layout_damage) {

  // Clear the layout damage, so layout() of a child can turn it back
  // on and subclasses like PackedGroup can detect that:
  Widget::layout();

  if (children_ && layout_damage&LAYOUT_WH) {
    int* p = sizes(); // initialize the size array

    // get changes in size from the initial size:
    int dw = r.w()-p[1];
    int dh = r.h()-p[3];
    p+=4;

    // Calculate a new size & position for every child widget:
    // get initial size of resizable():
    int IX = *p++;
    int IR = *p++;
    int IY = *p++;
    int IB = *p++;

    Widget*const* a = array_;
    Widget*const* e = a+children_;
    while (a < e) {
      Widget* o = *a++;
      int flags = o->layout_damage();
      int X = *p++;
      int R = *p++;
      int Y = *p++;
      int B = *p++;

      if (layout_damage & LAYOUT_W) {
	if (X >= IR) X += dw;
	else if (X > IX) {
	  switch (resize_align_&(ALIGN_LEFT|ALIGN_RIGHT)) {
	  case 0: X = X + dw/2; break;
	  case ALIGN_LEFT: break;
	  case ALIGN_RIGHT: X = X+dw; break;
	  case ALIGN_LEFT|ALIGN_RIGHT: X = X + dw * (X-IX)/(IR-IX); break;
	  }
	}
	if (R >= IR) R += dw;
	else if (R > IX) {
	  switch (resize_align_&(ALIGN_LEFT|ALIGN_RIGHT)) {
	  case 0: R = R + dw/2; break;
	  case ALIGN_LEFT: break;
	  case ALIGN_RIGHT: R = R+dw; break;
	  case ALIGN_LEFT|ALIGN_RIGHT: R = R + dw * (R-IX)/(IR-IX); break;
	  }
	}
	if (R-X != o->w()) {flags |= LAYOUT_W; o->w(R-X);}
	X += r.x();
	if (X != o->x()) {flags |= LAYOUT_X; o->x(X);}
      }
      if (layout_damage & LAYOUT_H) {
	if (Y >= IB) Y += dh;
	else if (Y > IY) {
	  switch (resize_align_&(ALIGN_TOP|ALIGN_BOTTOM)) {
	  case 0: Y = Y + dh/2; break;
	  case ALIGN_TOP: break;
	  case ALIGN_BOTTOM: Y = Y+dh; break;
	  case ALIGN_TOP|ALIGN_BOTTOM: Y = Y + dh*(Y-IY)/(IB-IY); break;
	  }
	}
	if (B >= IB) B += dh;
	else if (B > IY) {
	  switch (resize_align_&(ALIGN_TOP|ALIGN_BOTTOM)) {
	  case 0: B = B + dh/2; break;
	  case ALIGN_TOP: break;
	  case ALIGN_BOTTOM: B = B+dh; break;
	  case ALIGN_TOP|ALIGN_BOTTOM: B = B + dh*(B-IY)/(IB-IY); break;
	  }
	}
	if (B-Y != o->h()) {flags |= LAYOUT_H; o->h(B-Y);}
	Y += r.y();
	if (Y != o->y()) {flags |= LAYOUT_Y; o->y(Y);}
      }
      o->layout_damage(flags);
    }
  }

  Widget*const* a = array_;
  Widget*const* e = a+children_;
  int extradamage = layout_damage & LAYOUT_DAMAGE;
#if USE_X11 || !defined(__APPLE__)
  // If this is not a Window and the xy position is changed, we must
  // call layout() on every child. This is necessary so that child
  // Windows will move to their new positions:
  if ((layout_damage & LAYOUT_XY) && !is_window()) extradamage |= LAYOUT_XY;
#endif
  while (a < e) {
    Widget* widget = *a++;
    widget->layout_damage(widget->layout_damage()|extradamage);
    if (widget->layout_damage()) {
      widget->layout();
      widget->layout_damage(0);
    }
  }
}

////////////////////////////////////////////////////////////////
// Draw

// Widgets that want to outwit the clip-out can set this when they are
// drawn to indicate that they did the clip-out. Only TabGroup really uses
// this (and I'm not certain it has to), plus a bunch of back-compatability
// widgets that want to be "invisible" (they turn this on but don't draw
// anything). This is a pointer so if it is left on by a child widget
// it does not fool this into thinking the clipping is done.
Widget* fl_did_clipping;

void Group::draw() {
  clear_flag(HIGHLIGHT); // we never draw the box with highlight colors
  int numchildren = children();
  if (damage() & ~DAMAGE_CHILD) {
#if USE_CLIPOUT
    // Non-blinky draw, draw the inside widgets first, clip their areas
    // out, and then draw the background:
    push_clip(0, 0, w(), h());
    int n; for (n = numchildren; n--;) {
      Widget& w = *child(n);
      fl_did_clipping = 0;
      draw_child(w);
      if (fl_did_clipping != &w) clipout(w.x(), w.y(), w.w(), w.h());
    }
    draw_box();
    draw_label();
    pop_clip();
    // labels are drawn without the clip for back compatability so they
    // can draw atop sibling widgets:
    for (n = 0; n < numchildren; n++) draw_outside_label(*child(n));
#else
    // blinky-draw:
    draw_box();
    draw_label();
    for (int n = 0; n < numchildren; n++) {
      Widget& w = *child(n);
      draw_child(w);
      draw_outside_label(w);
    }
#endif
  } else {
    // only some child widget has been damaged, draw them without
    // doing any clipping.  This is for maximum speed, even though
    // this may result in different output if this widget overlaps
    // another widget or a label.
    for (int n = 0; n < numchildren; n++) {
      Widget& w = *child(n);
      if (w.damage() & DAMAGE_CHILD_LABEL) {
	draw_outside_label(w);
	w.set_damage(w.damage() & ~DAMAGE_CHILD_LABEL);
      }
      update_child(w);
    }
  }
}

// Pieces of draw() that subclasses may want to use:

/*! Draw what would be in the area of the widget if the widget was not
  there. By calling this in draw(), a widgets can redraw as though
  they are partially transparent, or more complicated shapes than
  rectangles. Note that only parent widgets are drawn, not
  underlapping ones.

  If DAMAGE_EXPOSE is on in damage() then the window (or at least some
  region of it) is being completely redrawn. Normally FLTK will have
  already drawn the background, so to avoid redundant drawing this
  will return immediatly without drawing anything. However FLTK may be
  compiled with USE_CLIPOUT (an option to reduce blinking in
  single-buffered windows) and in that case the widget must draw any
  visible background. In this case this function always draws the
  background.
*/
void Widget::draw_background() const {
#if !USE_CLIPOUT
  // fabien: if DAMAGE ALL is requested though we should redraw this backgnd as asked by this flag
  if (damage()&DAMAGE_EXPOSE) return;
#endif
  if (!parent()) return;
  push_clip(0, 0, w(), h());
  push_matrix();
  translate(-x(), -y());
  if (!parent()->box()->fills_rectangle()) parent()->draw_background();
  parent()->draw_box();
  parent()->draw_label();
  pop_matrix();
  pop_clip();
}

extern void fl_window_flush(Window* window);

/*! Force a child to draw, by turning on DAMAGE_ALL and DAMAGE_EXPOSE,
  and calling it's draw() after temporarily translating so 0,0 in
  drawing coordinates is the upper-left corner. It's damage is then set to 0.
*/
void Group::draw_child(Widget& w) const {
  if (w.visible() && not_clipped(w)) {
    w.set_damage(DAMAGE_ALL|DAMAGE_EXPOSE);
    if (w.is_window()) {
      GSave gsave;
      fl_window_flush((Window*)&w);
    } else {
      push_matrix();
      translate(w.x(), w.y());
      w.draw();
      pop_matrix();
    }
    w.set_damage(0);
  }
}

/*! If the child's damage() is not zero, force it to update calling
  it's draw() after temporarily translating so 0,0 in drawing
  coordinates is the upper-left corner. It's damage is then set to 0.
*/
void Group::update_child(Widget& w) const {
  if (w.damage() && w.visible() && not_clipped(w)) {
    if (w.is_window()) {
      GSave gsave;
      ((Window*)&w)->flush();
    } else {
      push_matrix();
      translate(w.x(), w.y());
      w.draw();
      pop_matrix();
    }
    w.set_damage(0);
  }
}

/** If this is a Group and not a Window, subtract x() and y() from the
    position of all children. This will fix the positions of widgets
    created for fltk1.1 that are inside a group.
*/
void Group::fix_old_positions() {
  if (is_window()) return; // in fltk 1.0 children of windows were relative
  for (int i = 0; i < children(); i++) {
    Widget& w = *(child(i));
    w.x(w.x()-x());
    w.y(w.y()-y());
  }
}

//
// End of "$Id: Group.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
