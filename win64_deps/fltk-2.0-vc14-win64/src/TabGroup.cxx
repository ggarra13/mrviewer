// "$Id: TabGroup.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/*! \class fltk::TabGroup

This is the "file card tabs" interface to allow you to put lots and
lots of buttons and switches in a panel. This first appeared in
NeXTStep, but is best known from Windows control panesl. FLTK's
version draws in a style more reminiscent of NeXT or PageMaker, and
avoids the confusing multiple-lines of Windows by drawing overlapping
tabs.

\image html tabs.gif

Each child widget is a card, and it's label() is printed on the card
tab (including the label font and style).  The color() of the child is
used to color the tab as well.

The size of the tabs is controlled by the bounding box of the children
(there should be some space between the children and the edge of this
widget). If there is a larger gap on the bottom than the top, the tabs
are placed "inverted" along the bottom. It is easiest to lay this out
in fluid, using the fluid browser to select each child group and
resize them until the tabs look the way you want them to.

Clicking the tab makes that child visible() and hides all the
other children. If the widget with the focus does not consume them,
the ctrl+tab and ctrl+shift+tab keys will also switch tabs. The
user can also navigate the focus to the tabs and change them with
the arrow keys.

The callback() of the TabGroup widget is called when
the user changes the visible tab, and SHOW and HIDE events are passed
to the children.
*/

#include <config.h>
#include <fltk/TabGroup.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/Tooltip.h>
#include <stdlib.h>

using namespace fltk;

// return the left edges of each tab (plus a fake left edge for a tab
// past the right-hand one).  These position are actually of the left
// edge of the slope.  They are either seperated by the correct distance
// or by pager_->slope() or by zero.
// Return value is the index of the selected item.

int TabGroup::tab_positions(int* p, int* w) {
  int i, selected = 0, numchildren = children(), width = 0;
  p[0] = w[0] = 0;
  for (i=0; i<numchildren; i++) {
    Widget* o = child(i);
    if (o->visible()) selected = i;
    int wt = 300; int ht = 300; // rather arbitrary choice for max size
    o->measure_label(wt, ht);
    w[i] = wt+pager_->spacing(); // slope + extra_space
    width += w[i];
    p[i+1] = width;
  }
  int r = pager_->available_width(this); 
  if (width <= r) return selected;

  return pager_->update_positions(this, numchildren,selected, width, r, p, w);

}

// return space needed for tabs.  Negative to put them on the bottom:
int TabGroup::tab_height() {
  int H = h();
  int H2 = 0;
  int numchildren = children();
  for (int i=0; i < numchildren; i++) {
    Widget* o = child(i);
    if (o->y() < H) H = o->y();
    if (o->y()+o->h() > H2) H2 = o->y()+o->h();
  }
  H2 = h()-H2;
  Rectangle r(0,0); box()->inset(r);
  if (H2 > H) {
    H = H2-r.y();
    return (H <= 0) ? 0 : -H-1;
  } else {
    H = H-r.y();
    return (H <= 0) ? 0 : H;
  }
}

/*! Return the child index that would be selected by a click at the
  given mouse position. Returns -1 if the mouse position is not
  in a tab.
*/
int TabGroup::which(int event_x, int event_y) {
    return pager_->which(this, event_x, event_y);
}

int TabGroup::handle(int event) {
  int i = value();
  Widget* selected = i >= 0 ? child(i) : 0;
  int backwards = 0;

  switch (event) {

  case FOCUS_CHANGE:
    // this is called to indicate that some child got the focus
    if (focus_index() < 0) redraw(DAMAGE_VALUE);
    focus_index(fltk::focus() == this ? -1 : 0);
    return true;

  case FOCUS:
    if (fltk::focus() == this) goto GOTO_TABS;
    switch (navigation_key()) {
    case LeftKey:
    case UpKey:
      if (tab_height() < 0) goto GOTO_TABS; else goto GOTO_CONTENTS;
    case RightKey:
    case DownKey:
      if (tab_height() < 0) goto GOTO_CONTENTS; else goto GOTO_TABS;
    default:
      if (focus_index() < 0) goto GOTO_TABS;
    GOTO_CONTENTS:
      // Try to give the contents the focus. Also preserve a return value
      // of 2 (which indicates the contents have a text field):
      if (selected) {
	int n = selected->handle(FOCUS);
	if (n) {
	  if (!selected->contains(fltk::focus())) fltk::focus(selected);
	  return n;
	}
      }
    GOTO_TABS:
      focus_index(-1);
      redraw(DAMAGE_VALUE);
      return true;
    }

  case UNFOCUS:
    if (focus_index() < 0) redraw(DAMAGE_VALUE);
    return 1;

  // handle mouse events in the tabs:
  case PUSH: {
    int H = tab_height();
    if (H >= 0) {
      if (event_y() > H) break;
    } else {
      if (event_y() < h()+H) break;
    }}
  case DRAG:
  case RELEASE:
    i = which(event_x(), event_y());
    if (event == RELEASE && !pushed()) {
      push(0);
      if (i >= 0 && value(i)) do_callback();
    } else {
      push(i >= 0 ? child(i) : 0);
    }
    return i>=0 ? 1 : 0;

  // handle pointing at the tabs to bring up tooltip:
  case ENTER:
  case MOVE: {
    int H = tab_height();
    if (H >= 0) {
      if (event_y() > H) break;
    } else {
      if (event_y() < h()+H) break;
    }
    i = which(event_x(), event_y());
    if (i >= 0)
      Tooltip::enter(this, Rectangle(0, H<0?h()+H:0, w(), H<0?-H:H),
		     child(i)->tooltip());
    return 1;}

  case KEY:
    if (fltk::focus()==this) {
      switch (event_key()) {
      case SpaceKey:
      case RightKey:
	goto MOVE;
      case BackSpaceKey:
      case LeftKey:
	backwards = 1;
      MOVE:
	if (backwards) {i = i ? i-1 : children()-1;}
	else {i++; if (i >= children()) i = 0;}
	if (value(i)) do_callback();
	return 1;
      }
    }
    switch (navigation_key()) {
    case RightKey:
    case DownKey:
      if (tab_height()<0) goto UP_CASE;
    DOWN_CASE:
      if (focus_index() < 0 && selected) return selected->take_focus();
      else return 0;
    case LeftKey:
    case UpKey:
      if (tab_height()<0) goto DOWN_CASE;
    UP_CASE:
      if (focus_index() >= 0) {fltk::focus(this); return 1;}
      else return 0;
    }
    // fall through to SHORTCUT case:
  case SHORTCUT:
    if (event_key() == TabKey && event_state(CTRL)) {
      backwards = event_state(SHIFT);
      goto MOVE;
    }
    break;

  }

  if (selected) return selected->send(event);
  return 0;
}

static Widget* push_ = 0;

int TabGroup::push(Widget *o) {
  if (push_ == o) return 0;
  if ((push_ && !push_->visible()) || (o && !o->visible()))
    redraw(DAMAGE_VALUE);
  push_ = o;
  return 1;
}

/*! Return the index of the first visible() child, which is normally
  the one the user selected.

  This will automatically force a single child to be visible() if
  more than one is, or if none are. If more than one is visible all
  except the first is hidden. If none are, the last one is made
  visible. The resulting visible child's index is returned. This behavior
  allows new TabGroups to be created with all children visible, and
  allows children to be deleted, moved to other groups, and
  show()/hide() called on them without the display ever looking wrong
  to the user.

  If there are no children then -1 is returned.
*/
int TabGroup::value() const {
  int ret = -1;;
  int numchildren = children();
  for (int i=0; i < numchildren; i++) {
    Widget* o = child(i);
    if (ret >= 0) o->hide();
    else if (o->visible()) ret = i;
    else if (i==numchildren-1) {o->show(); ret = i;}
  }
  return ret;
}

/*! Return child(value()), or return null if no children. */
Widget* TabGroup::selected_child() {
  int i = value();
  return i >= 0 ? child(i) : 0;
}

/*! Switch so index n is selected. If n is less than zero selects
  zero, if n is greater than the children it selects the last one.
  Returns true if this is a different child than last time. Does
  not do the callback().
*/
bool TabGroup::value(int n) {
  if (n < 0) n = 0;
  if (n >= children()) {
    if (!children()) return false;
    n = children()-1;
  }
  return selected_child(child(n));
}

/*! Switch to this child widget, or to a child that contains() this
  widget.  Returns true if this is a different selection than
  before. Does not do the callback(). If the widget is null or not a
  descendent of this, the last child is selected.
*/
bool TabGroup::selected_child(Widget *newvalue) {
  int setfocus = !focused() && contains(fltk::focus());
  int numchildren = children();
  for (int i=0; i < numchildren; i++) {
    Widget* o = child(i);
    if (o == newvalue) {
      if (o->visible()) return 0; // no change
      o->show();
      if (setfocus) o->take_focus();
    } else {
      o->hide();
    }
  }
  return 1;
}

enum {TAB_LEFT, TAB_RIGHT, SELECTED};

#if USE_CLIPOUT
extern Widget* fl_did_clipping;
#endif

static int H;
static int p[128];
void TabGroup::draw() {
  Widget *v = selected_child();

  H = tab_height();

  // draw the tabs if needed:
  if (damage() & (DAMAGE_VALUE|DAMAGE_ALL)) {
    if ( damage() & DAMAGE_ALL ) {
      draw_tab_background();
    }
    int w[128];
    int selected = tab_positions(p,w);
    int i;

    if (!pager_->draw_tabs(this, selected, p, w)) { // no custom draw :
	for (i=pager_->shift(); i<selected; i++) {
	  draw_tab(p[i], p[i+1], w[i], H, child(i), TAB_LEFT);
	}
	for (i=children()-1; i > selected; i--) {
	  draw_tab(p[i], p[i+1], w[i], H, child(i), TAB_RIGHT);
	}
	if (v) {
	  i = selected;
	  draw_tab(p[i], p[i+1], w[i], H, child(i), TAB_SELECTED);
	} else {
	  // draw the edge when no selection:
	  setcolor(H >= 0 ? GRAY99 : GRAY33);
	  int b = H >= 0 ? H : h()+H;
	  drawline(0, b, this->w(), b);
	}
    }

  }

  if (damage() & DAMAGE_ALL) { // redraw the entire thing:
#if USE_CLIPOUT
    push_clip(0, 0, w(), h());
    if (v) {
      fl_did_clipping = 0;
      draw_child(*v);
      if (fl_did_clipping != v) clipout(v->x(), v->y(), v->w(), v->h());
    }
    draw_tab_background();
    pop_clip();
#else
    if (v) draw_child(*v);
#endif
  } else { // redraw the child
    if (v) update_child(*v);
  }

#if USE_CLIPOUT
  if (damage() & DAMAGE_EXPOSE) {
    clipout(0, H>=0 ? 0 : h()+H, p[children()]+pager_->slope(), (H>=0?H:-H));
    clipout(0, H>0 ? H : 0, this->w(), h()-(H>=0?H:-H-1));
    fl_did_clipping = this;
  }
#endif
}

void TabGroup::set_draw_outline( bool draw_outline )
{
  _drawOutline = draw_outline;
  redraw();
}

void TabGroup::draw_tab_background()
{
  draw_background();
  if ((box()!= NO_BOX) || _drawOutline ) {
    int x1 = 0;
    int th = tab_height();
    int y1 = th >= 0 ? th : 0;
    int x2 = w()-1;
    int y2 = h()-1+ ( th<0 ? th : 0 );
    if (w() > 3 ) {
      setcolor( Color(GRAY95) );
      drawline( x1, y1, x1, y2 );
      setcolor( Color(GRAY33) );
      drawline( x1, y2, x2, y2 );
      drawline( x2, y1, x2, y2 );
    }
  }
}

void TabGroup::draw_tab(int x1, int x2, int W, int H, Widget* o, int what) {
  if (x2 <= x1) return; // ignore ones shrunk to zero width
  bool drawlabel = true;
  if (x2 < x1+W) {
    if (x2 <= x1+pager_->slope()) drawlabel = false;
    if (what == TAB_LEFT) {
      if (x1+W < x2+pager_->slope()) x2 = x1+W;
      else x2 += pager_->slope();
    } else {
      if (x1+W < x2+pager_->slope()) x1 = x2-W;
      else x1 -= pager_->slope();
    }
  }
  int sel = (what == TAB_SELECTED);
  const int shrink_factor = 3;
  int eat_border_factor = (H<0 ? box()->dh() : 1 )*sel;
  int up_pos = (1-sel)*shrink_factor;
  setcolor(sel  ? selection_color() : o->color());
  if (H >= 0) {// put the tab thumbnail on top
    newpath();
    addvertex(x1, H+eat_border_factor);
    addvertex(x1+pager_->slope(), up_pos );
    addvertex(x2, up_pos );
    addvertex(x2+pager_->slope(), H+eat_border_factor);
    fillpath();
    setcolor(!sel && o==push_ ? GRAY33 : GRAY99);
    drawline(x1, H, x1+pager_->slope(), up_pos );
    drawline(x1+pager_->slope(), up_pos , x2, up_pos );
    if (sel) {
      if (x1>0) drawline(0, H, x1, H);
      if (x2+pager_->slope() < w()-1) drawline(x2+pager_->slope(), H, w()-1, H);
    }
    setcolor(!sel && o==push_ ? GRAY99 : GRAY33);
    drawline(x2, (1-sel)*shrink_factor , x2+pager_->slope(), H);
  } else { // put the tab thumbnail at the bottom
    newpath();
    addvertex(x1, h()+H-eat_border_factor);
    addvertex(x1+pager_->slope(), h()-1-up_pos );
    addvertex(x2, h()-1-up_pos );
    addvertex(x2+pager_->slope(), h()+H-eat_border_factor);
    fillpath();
    setcolor(!sel && o==push_ ? GRAY99 : GRAY33);
    newpath();
    addvertex(x1+pager_->slope(), h()-1-up_pos);
    addvertex(x2, h()-1-up_pos);
    addvertex(x2+pager_->slope(), h()+H-eat_border_factor);
    strokepath();
    if (sel) {
      if (x1>0) drawline(0, h()+H, x1, h()+H);
      if (x2+pager_->slope() < w()-1) drawline(x2+pager_->slope(), h()+H, w()-1, h()+H);
    }
    setcolor(!sel && o==push_ ? GRAY33 : GRAY99);
    drawline(x1, h()+H, x1+pager_->slope(), h()-up_pos);
  }
  if (drawlabel) {
    Rectangle r((what==TAB_LEFT ? x1 : x2-W)+(pager_->slope()+pager_->extra_space()/2),
		H<0 ? this->h()+H-1 : 2,
		W-(pager_->slope()+pager_->extra_space()/2),
		abs(H)-1);
    drawstyle(o->style(), sel && focused() ? FOCUSED|OUTPUT : OUTPUT);
    setcolor(sel  ? selection_textcolor() : o->textcolor());
    o->draw_label(r, (o->flags() & ~ALIGN_MASK)|ALIGN_CENTER);
    FLAT_BOX->draw_symbol_overlay(r);
  }
}

static void revert(Style* s) {
  s->box_ = THIN_UP_BOX;
  s->color_ = GRAY75;
}

/*! assign dynamically a new pager with a preconfigured prototype */
void TabGroup::pager(TabGroupPager * value) {
  if (value && pager_!=value) {
      if (pager_) delete pager_; 
      pager_= value->clone();
      redraw();
  } 
}
//! setting the default pager_ for future tabgroups, a default pager is _never_ null by design
void TabGroup::default_pager(TabGroupPager * value) {
  if (value && default_pager_!=value) {
      if (default_pager_) delete default_pager_; 
      default_pager_= value->clone();
  } 
}

static NamedStyle style("Tabs", revert, &TabGroup::default_style);
/*! The default style has a gray color() and the box() is set to 
  THIN_UP_BOX. The box() is used to draw the edge around the cards,
  including the top edge, but the tab itself is designed only to
  match THIN_UP_BOX. You can also use FLAT_BOX and it will look
  correct if the tabs fill the entire width of a window or parent
  box.
*/
NamedStyle* TabGroup::default_style = &::style;

/*! Creates a new TabGroup widget using the given position, size, and
label string. Use add(widget) to add each child. Each child is
probably an fltk::Group widget containing the actual widgets the user
sees. The children should be sized to stay away from the top or bottom
edge of the <TT>fltk::Tabs</TT>, which is where the tabs are drawn.
*/
TabGroup::TabGroup(int X,int Y,int W, int H, const char *l,bool begin)
  : Group(X,Y,W,H,l,begin),
    _drawOutline( false ) 
{
  style(default_style);
  focus_index(0);
  pager_ = default_pager_->clone();
}

// End of "$Id: TabGroup.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
