// "$Id: ScrollGroup.cxx 9160 2011-10-30 12:59:11Z bgbnbigben $"
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

#include <fltk/ScrollGroup.h>
#include <fltk/events.h>
#include <fltk/layout.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <config.h>
using namespace fltk;

/*! \class fltk::ScrollGroup

This container widget lets you maneuver around a set of widgets much 
larger than your window.  If the child widgets are larger than the size 
of this object then scrollbars will appear so that you can scroll over 
to them:

\image html Fl_Scroll.gif

The default type() will just scroll a whole arrangement of widgets and
never resize them.  This is useful if you just want to get a big
control panel into a smaller window. The bounding box of the widgets
are the area it can scroll, this will remove any borders, if you want
the borders preserved put some invisible widgets there as
placeholders.

This can be used to pan around a large drawing by making a single
child widget "canvas".  This child widget should be of your own class,
with a draw() method that draws everything. The scrolling is done by
changing the x() and y() of the widget and drawing it with the fltk
clip region set to the newly exposed rectangles. You can speed things
up by using fltk::not_clipped() or fltk::intersect_with_clip() to
detect and skip the clipped portions of the drawing.

By default you can scroll in both directions, and the scrollbars 
disappear if the data will fit in the area of the scroll. type()
can change this: 
- HORIZONTAL resize vertically but scroll horizontally
- VERTICAL resize horizontally but scroll vertically
- BOTH this is the default
- HORIZONTAL_ALWAYS resize vertically but always show horizontal scrollbar
- VERTICAL_ALWAYS resize horizontally but always show vertical scrollbar
- BOTH_ALWAYS always show both scrollbars

If you use HORIZONTAL or VERTICAL you must initally position and size
your child widgets as though the scrollbar is off (ie fill the box()
width entirely if type() is VERTICAL). The first time layout() is
called it will resize the widgets to fit inside the scrollbars.

It is very useful to put a single PackedGroup child into a VERTICAL ScrollGroup.

Also note that scrollbar_align() (a Style parameter) can put the
scrollbars on different sides of the widget.

Currently you cannot use Window or any subclass (including GlWindow)
as a child of this.  The clipping is not conveyed to the operating
system's window and it will draw over the scrollbars and neighboring
objects.

*/

#if USE_CLIPOUT
extern Widget* fl_did_clipping;
#endif

void ScrollGroup::draw_clip(void* v, const Rectangle& r) {
  push_clip(r);
  ScrollGroup* s = (ScrollGroup*)v;
#if !USE_CLIPOUT
  // fill the rest of the region with color:
  setcolor(s->color()); fillrect(r);
#endif
  // draw all the children, clipping them out of the region:
  int numchildren = s->children(); int i;
  for (i = numchildren; i--;) {
    Widget& w = *s->child(i);
    // Partial-clipped children with their own damage will still need
    // to be redrawn before the scroll is finished drawing.  Don't clear
    // their damage in this case:
    uchar save = 0;
    if (!(s->damage()&DAMAGE_ALL)) {
      if (w.x() < r.x() || w.y() < r.y() ||
	  w.r() > r.r() || w.b() > r.b())
	save = w.damage();
    }
#if USE_CLIPOUT
    fl_did_clipping = 0;
    s->draw_child(w);
    if (fl_did_clipping != &w) clipout(w);
#else
    s->draw_child(w);
#endif
    w.set_damage(save);
  }
#if USE_CLIPOUT
  // fill the rest of the region with color:
  setcolor(s->color()); fillrect(r);
#endif
  // draw the outside labels:
  for (i = numchildren; i--;)
    s->draw_outside_label(*s->child(i));
  pop_clip();
}

/*! Set the rectangle to the scrolling area (in the ScrollGroup's
  coordinate system). This removes the border of the box() and the
  space needed for any visible scrollbars.
*/
void ScrollGroup::bbox(Rectangle& r) {
  r.set(0,0,w(),h()); box()->inset(r);
  if (scrollbar.visible()) {
    if (scrollbar_align() & ALIGN_LEFT) r.move_x(scrollbar.w());
    else r.move_r(-scrollbar.w());
  }
  if (hscrollbar.visible()) {
    if (scrollbar_align() & ALIGN_TOP) r.move_y(hscrollbar.h());
    else r.move_b(-hscrollbar.h());
  }
}

void ScrollGroup::draw() {
  Rectangle r; bbox(r);

  uchar d = damage();
  if (d & DAMAGE_ALL) { // full redraw
    draw_frame();
    draw_clip(this, r);
  } else {
    if (scrolldx || scrolldy) {
      scrollrect(r, scrolldx, scrolldy, draw_clip, this);
    } else if (d & DAMAGE_SCROLL) {
      draw_clip(this,r);
    } else if (d & DAMAGE_CHILD) { // draw damaged children
      push_clip(r);
      for (int i = children(); i--;) {
	Widget& w = *child(i);
	if (w.damage() & DAMAGE_CHILD_LABEL) {
	  draw_outside_label(w);
	  w.set_damage(w.damage() & ~DAMAGE_CHILD_LABEL);
	}
	update_child(w);
      }
      pop_clip();
    }
  }
  scrolldx = scrolldy = 0;

  // draw the scrollbars:
  if (d & DAMAGE_ALL) {
    scrollbar.set_damage(DAMAGE_ALL);
    hscrollbar.set_damage(DAMAGE_ALL);
    if (scrollbar.visible() && hscrollbar.visible()) {
      // fill in the little box in the corner
      setcolor(buttoncolor());
      fillrect(scrollbar.x(),hscrollbar.y(),scrollbar.w(),hscrollbar.h());
    }
  }
  update_child(scrollbar);
  update_child(hscrollbar);
}

void ScrollGroup::layout() {

  int layout_damage = this->layout_damage();
  uchar type=Widget::type() & ~GROUP_TYPE;

  // handle movement in xy without wasting time:
  if (!(layout_damage&(LAYOUT_WH|LAYOUT_DAMAGE|LAYOUT_CHILD))) {
    Group::layout();
    return;
  }

  Rectangle rectangle; bbox(rectangle);

  // This loop repeats if any scrollbars turn on or off:
  for (int repeat=0; repeat<2; repeat++) {

    layout_damage &= ~LAYOUT_WH;
    if (!(type&HORIZONTAL)) layout_damage |= LAYOUT_W;
    if (!(type&VERTICAL)) layout_damage |= LAYOUT_H;
    Group::layout(rectangle, layout_damage);

    // move all the children and accumulate their bounding boxes:
    int dx = layoutdx;
    int dy = layoutdy;
    layoutdx = layoutdy = 0;
    scrolldx += dx;
    scrolldy += dy;
    int l = w();
    int r = 0;
    int t = h();
    int b = 0;
    int numchildren = children();
    for (int i=0; i < numchildren; i++) {
      Widget* o = child(i);
      o->position(o->x()+dx, o->y()+dy);
      o->layout();
      if (o->x() < l) l = o->x();
      if (o->y() < t) t = o->y();
      if (o->x()+o->w() > r) r = o->x()+o->w();
      if (o->y()+o->h() > b) b = o->y()+o->h();
    }

    // If there is empty space on the bottom (VERTICAL mode) or right
    // (HORIZONTAL mode), and widgets off to the top or left, move
    // widgets to use up the available space.
    int newDx = 0;
    int newDy = 0;
    if (type & VERTICAL) {
      if (b < rectangle.b()) {
	int space = rectangle.b()-b;
	int hidden = rectangle.y()-t;
	newDy = (space>hidden) ? hidden : space;
	newDy = (newDy>0) ? newDy : 0;
      }
    }
    if (type & HORIZONTAL) {
      if (r < rectangle.r()) {
	int space = rectangle.r()-r;
	int hidden = rectangle.x()-l; // ell
	newDx = (space>hidden) ? hidden : space;
	newDx = (newDx>0) ? newDx : 0;
      }
    }

    // Move the child widgets again if they are to be kept inside.
    if (newDx || newDy) {
      for (int i=0; i < numchildren; i++) {
	Widget* o = child(i);
	o->position(o->x()+newDx, o->y()+newDy);
	o->layout();
      }
    }

    // Turn on/off the scrollbars if it fits:
    if ((type & VERTICAL) &&
	((type & ALWAYS_ON) || t < rectangle.y() || b > rectangle.b())) {
      // turn on the vertical scrollbar
      if (!scrollbar.visible()) {
	scrollbar.set_visible();
	scrollbar.redraw();
      }
    } else {
      // turn off the vertical scrollbar
      if (scrollbar.visible()) {
	scrollbar.clear_visible();
      }
    }

    if ((type & HORIZONTAL) &&
	((type & ALWAYS_ON) || l < rectangle.x() || r > rectangle.r())) {
      // turn on the horizontal scrollbar
      if (!hscrollbar.visible()) {
	hscrollbar.set_visible();
	hscrollbar.redraw();
      }
    } else {
      // turn off the horizontal scrollbar
      if (hscrollbar.visible()) {
	hscrollbar.clear_visible();
      }
    }

    // fix the width of the scrollbars to current preferences:
    const int sw = scrollbar_width();
    scrollbar.w(sw);
    hscrollbar.h(sw);
    // Figure out the new contents rectangle and put scrollbars outside it:
    Rectangle R; bbox(R);

    scrollbar.resize(scrollbar_align()&ALIGN_LEFT ? R.x()-sw : R.r(), R.y(), sw, R.h());

    scrollbar.value(yposition_ = (R.y()-t), R.h(), 0, b-t);
    hscrollbar.resize(R.x(), scrollbar_align()&ALIGN_TOP ? R.y()-sw : R.b(), R.w(), sw);
    hscrollbar.value(xposition_ = (R.x()-l), R.w(), 0, r-l);
    max_x_scroll_ = r-l-R.w();
    max_y_scroll_ = b-t-R.h();

    // We are done if the new rectangle is the same as last time:
    if (R.x() == rectangle.x() &&
	R.y() == rectangle.y() &&
	R.w() == rectangle.w() &&
	R.h() == rectangle.h()) break;
    // otherwise layout again in this new rectangle:
    rectangle = R;
  }

}

/*! \fn int ScrollGroup::xposition() const
  Return the horizontal scrolling position. This is the distance from the
  left-most widget to the left of the bbox().
*/

/*! \fn int ScrollGroup::yposition() const
  Return the vertical scrolling position. This is the distance from the
  top-most widget edge to the top of the bbox().
*/

/*! Set xposition() and yposition() to these new values, thus scrolling
  the display. */
void ScrollGroup::scrollTo(int X, int Y) {
  int dx = xposition_-X;
  int dy = yposition_-Y;
  if (!dx && !dy) return;
  xposition_ = X;
  yposition_ = Y;
  layoutdx += dx;
  layoutdy += dy;
  relayout();
}

void ScrollGroup::hscrollbar_cb(Widget* o, void*) {
  ScrollGroup* s = (ScrollGroup*)(o->parent());
  s->scrollTo(int(((Scrollbar*)o)->value()), s->yposition());
}

void ScrollGroup::scrollbar_cb(Widget* o, void*) {
  ScrollGroup* s = (ScrollGroup*)(o->parent());
  s->scrollTo(s->xposition(), int(((Scrollbar*)o)->value()));
}

#define SLIDER_WIDTH 16

static inline int nogroup(int X) {Group::current(0); return X;}

ScrollGroup::ScrollGroup(int X,int Y,int W,int H,const char* L,bool begin)
  : Group(X,Y,W,H,L),
    // This initial size & position of scrollbars is probably not used:
    enable_drag_scroll_( true ),
    drag_scrolling_( false ),
    delegate_alt_click_( true ),
    drag_x_( 0 ),
    drag_y_( 0 ),
    pos_x_( 0 ),
    pos_y_( 0 ),
    scrollbar(nogroup(X)+W-SLIDER_WIDTH,Y,SLIDER_WIDTH,H-SLIDER_WIDTH),
    hscrollbar(X,Y+H-SLIDER_WIDTH,W-SLIDER_WIDTH,SLIDER_WIDTH)
{
  type(BOTH);
  xposition_ = 0;
  yposition_ = 0;
  scrolldx = scrolldy = layoutdx = layoutdy = 0;
  hscrollbar.parent(this);
  hscrollbar.callback(hscrollbar_cb);
  scrollbar.set_vertical();
  scrollbar.parent(this);
  scrollbar.callback(scrollbar_cb);
  Group::current(begin ? this : parent());
}

int ScrollGroup::handle(int event) {
  switch (event) {

#if 0
  case FOCUS_CHANGE: {
    {// The event indicates that the focus changed to a different child,
    // auto-scroll to show it:

    // NO !!!!   FORTY TIMES NO!!!
    // NO DO NOT SCROLL TO THE FOCUSSED CHILD
    // NO DO NOT DO IT
    // IT BREAKS THE CONTROL PANELS IN NUKE
    // DO NOT ATTEMPT EVER TO KEEP THE FOCUSSED WIDGET IN THE SCROLL
    // DO NOT DO IT
    // IT DOES NOT WORK
    // PLEASE PLEASE STOP PUTTING THIS CODE BACK BILL
    // IF YOU SCROLL SO THAT THE FOCUSSED WIDGET GOES OFFSCREEN, CLICK THE FOCUS
    // OFF THE CONTROL PANEL TO THE DAG WINDOW, THEN ANY CLICK IN THE CONTROL PANEL
    // WILL ZIP THE SCROLLGROUP AWAY FROM WHAT YOU CLICKED ON, AND IT IS EXTREMELY
    // ANNOYING AND A BUG.  PLEASE TALK TO ME BEFORE MAKING ANY CHANGES TO
    // SCROLLGROUP OR PACKEDGROUP.  NOTHING IS MORE FRUSTRATING THAN DESIGNING CODE
    // SPECIFIC TO A UI LIBRARY AND HAVING THAT LIBRARY CONSTANTLY CHANGE FROM UNDER YOU.

    Widget* w = fltk::focus();
    int x = w->x();
    int y = w->y();
    for (Group* p = w->parent(); p != this; p = p->parent()) {
      // if (!p) return 0; // this should never happen
      x += p->x();
      y += p->y();
    }
    Rectangle R; bbox(R);
    int r = x+w->w();
    int dx = 0;
    if (x < R.x()) {dx = R.x()-x; if (r+dx > R.r()) {dx = R.r()-r; if (dx < 0) dx = 0;}}
    else if (r > R.r()) {dx = R.r()-r; if (x+dx < R.x()) {dx = R.x()-x; if (dx > 0) dx = 0;}}
    int b = y+w->h();
    int dy = 0;
    if (y < R.y()) {dy = R.y()-y; if (b+dy > R.b()) {dy = R.b()-b; if (dy < 0) dy = 0;}}
    else if (b > R.b()) {dy = R.b()-b; if (y+dy < R.y()) {dy = R.y()-y; if (dy > 0) dy = 0;}}
    scrollTo(xposition_-dx, yposition_-dy);
    break;}
#endif

  case ENTER:
  case MOVE:
  case DND_ENTER:
  case DND_DRAG:
    {
      // For all mouse events check to see if we are in the scrollbar
      // areas and send to them:
      if (scrollbar.visible() &&
          (scrollbar_align()&ALIGN_LEFT ?
           (event_x() < scrollbar.x()+scrollbar.w()) :
           (event_x() >= scrollbar.x()))) {
        return scrollbar.send(event);
      } else if (hscrollbar.visible() &&
                 (scrollbar_align()&ALIGN_TOP ?
                  (event_y() < hscrollbar.y()+hscrollbar.h()) :
                  (event_y() >= hscrollbar.y()))) {
        return hscrollbar.send(event);
      }
    }
    if ( delegate_alt_click_ == false &&
         ((event_state()&(ALT|CTRL|SHIFT)) != ALT ) ) {
      delegate_alt_click_ = true;
    }
    break;

  case PUSH:
    {
      // For all mouse events check to see if we are in the scrollbar
      // areas and send to them:
      if (scrollbar.visible() &&
          (scrollbar_align()&ALIGN_LEFT ?
           (event_x() < scrollbar.x()+scrollbar.w()) :
           (event_x() >= scrollbar.x()))) {
        return scrollbar.send(event);
      } else if (hscrollbar.visible() &&
                 (scrollbar_align()&ALIGN_TOP ?
                  (event_y() < hscrollbar.y()+hscrollbar.h()) :
                  (event_y() >= hscrollbar.y()))) {
        return hscrollbar.send(event);
      }
      if ( !drag_scrolling_ &&
           ((event_state()&(ALT|CTRL|SHIFT)) == ALT ) &&
           enable_drag_scroll_ ) {
        if ( delegate_alt_click_ ) {
          int retVal = Group::handle( event );
          if ( retVal ) {
            return retVal;
          }
        }
        drag_scrolling_ = true;
        drag_x_ = event_x();
        drag_y_ = event_y();
        pos_x_ = xposition();
        pos_y_ = yposition();
        delegate_alt_click_ = false;
        return true;
      }
    }
    break;

  case DRAG:
    {
      if ( drag_scrolling_ ) {
        int dx = event_x() - drag_x_;
        int dy = event_y() - drag_y_;
        int newX = pos_x_-dx;
        int minX = 0;
        int maxX = max_x_scroll_ >= 0 ? max_x_scroll_ : 0;
        int newY = pos_y_-dy;
        int minY = 0;
        int maxY = max_y_scroll_ >= 0 ? max_y_scroll_ : 0;
        newX = newX<maxX ? newX : maxX;
        newX = newX>minX ? newX : minX;
        newY = newY<maxY ? newY : maxY;
        newY = newY>minY ? newY : minY;
        scrollTo( newX, newY );
      }
    }
    break;

  case RELEASE:
    if ( drag_scrolling_ ) {
      drag_scrolling_ = false;
      if ((event_state()&(ALT|CTRL|SHIFT)) != ALT ) {
        delegate_alt_click_ = true;
      }
    }
    break;

  case MOUSEWHEEL:
    {
      if ( (scrollbar.visible()||hscrollbar.visible())
           && !fltk::event_state( fltk::ALT ) ) {
        if (event_key() == WheelLeft || event_key() == WheelRight) {
          return hscrollbar.send(event);
        } else {
          return scrollbar.send(event);
	}
      } else {
        return Group::handle( event );
      }
    }

  case SHORTCUT:
   {
#define CURSOR_STEP 8
	int scroll_x = hscrollbar.value();
	int scroll_y = scrollbar.value();

	if(hscrollbar.visible())
  {
	if(event_key() == LeftKey)
 {
	if(scroll_x == 0) return 1;

	if((scroll_x = hscrollbar.value() - CURSOR_STEP) < 0) scroll_x = 0;

	scrollTo(scroll_x, scroll_y);

	return 1;
 }
	if(event_key() == RightKey) 
 {
	if(scroll_x == max_x_scroll_) return 1;

	if((scroll_x = hscrollbar.value() + CURSOR_STEP) > max_x_scroll_) 
	 scroll_x = max_x_scroll_;

	scrollTo(scroll_x, scroll_y);

	return 1;
 }
  }
	if(scrollbar.visible())
  {
	if(event_key() == UpKey)
 {
	if(scroll_y == 0) return 1;

	if((scroll_y = scrollbar.value() - CURSOR_STEP) < 0) scroll_y = 0;

	scrollTo(scroll_x,scroll_y);

	return 1;
 }
	if(event_key() == DownKey)
 {
	if(scroll_y == max_y_scroll_) return 1;

	if((scroll_y = scrollbar.value() + CURSOR_STEP) > max_y_scroll_) 
	 scroll_y = max_y_scroll_;

	scrollTo(scroll_x,scroll_y);

	return 1;
 }
	if(event_key() == HomeKey)
 {
	if(scroll_y > 0)
	 scrollTo(scroll_x, 0);

	return 1;
 }
	if(event_key() == EndKey)
 {
	if(scroll_y < max_y_scroll_)
	 scrollTo(scroll_x, max_y_scroll_);

	return 1;
 }
	if(event_key() == PageUpKey)
 {
	if(scroll_y == 0) return 1;

	if((scroll_y = scrollbar.value() - scrollbar.pagesize()) < 0) 
	 scroll_y = 0;

	scrollTo(scroll_x, scroll_y);

	return 1;
 }
	if(event_key() == PageDownKey)
 {
	if(scroll_y == max_y_scroll_) return 1;

	if((scroll_y = scrollbar.value() + scrollbar.pagesize()) > max_y_scroll_)
	 scroll_y = max_y_scroll_;

	scrollTo(scroll_x, scroll_y);

	return 1;
 }
  }
   }
	break;

  case KEY: 
    if (scrollbar.send(event)) return 1;
    if (hscrollbar.send(event)) return 1;
    break;

  case KEYUP:
    {
      if ( delegate_alt_click_ == false &&
           ((event_state()&(ALT|CTRL|SHIFT)) != ALT ) ) {
        delegate_alt_click_ = true;
      }
    }
    break;

  }

  return Group::handle(event);
}

//
// End of "$Id: ScrollGroup.cxx 9160 2011-10-30 12:59:11Z bgbnbigben $".
//
