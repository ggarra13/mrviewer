//
// "$Id: MenuBar.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Menu bar widget for the Fast Light Tool Kit (FLTK).
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

/*! \class fltk::MenuBar

Subclass of fltk::Menu. The top level menu items are visible and drawn
in a horizontal row. Put this at the top edge of your window and you
get a typical menubar.

You can call popup() on this same widget in response to a right-click
in your work area and you will get the exact same menu as a vertical
pop up menu.
*/

#include <fltk/MenuBar.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/Item.h>
#include <fltk/draw.h>
#include <fltk/Tooltip.h>
#include <config.h>
#include <fltk/Item.h> // for TOGGLE, RADIO
#define checkmark(item) (item->type()>=Item::TOGGLE && item->type()<=Item::RADIO)

using namespace fltk;

void MenuBar::draw() {
  // set_item() does not cause a redraw:
  if (damage() == DAMAGE_VALUE) return;
  // annoying kludge: copy the buttonbox and highlight color into the
  // items so that they draw like Windows. If this is not done it looks
  // like Windows 95 with blue highlighting.
  Menu::draw_in(this, 0, 0, highlight_, last_);
  last_ = highlight_;
}

int MenuBar::handle(int event) {
  int children = this->children();
  if (!children) return 0;
  int i;
  switch (event) {
  case LEAVE:
    highlight_ = -1;
    goto J2;
  case MOVE:
  case ENTER:
    highlight_ = Menu::find_selected(this,0,0,event_x(), event_y());
    if (highlight_ >= 0) {
      const char* c = child(highlight_)->tooltip();
      if (c) {
	Tooltip::enter(this, Menu::get_location(this,0,0,highlight_), c);
      } else {
	// If last item had tooltip, exit from it
	if (last_ >= 0 && child(last_)->tooltip()) Tooltip::exit();
	// If MenuBar has tooltip, enter it
	if (tooltip()) Tooltip::enter(this);
      }
    } else {
      Tooltip::exit();
    }
J2:
    if (highlight_ == last_) return 1;
    redraw(DAMAGE_CHILD);
    return 1;
  case PUSH:
    if (highlight_ < 0) return 0;
    value(-1);
J1:
    highlight_ = -1; redraw(DAMAGE_CHILD);
    popup(Rectangle(w(), h()), 0, true);
    if (lastfocus_) lastfocus_->take_focus();
    lastfocus_ = 0;
    redraw(DAMAGE_CHILD);
    return 1;
  case SHORTCUT:
    // Test against the shortcut() of any item in any submenu:
    if (handle_shortcut()) return 1;
    // Check against the &x of top-level items:
    if (shortcut() && event_state(ACCELERATOR)) {
      for (i = 0; i < children; i++) {
	Widget* w = child(i);
	if (w->active() && w->test_label_shortcut()) {
	  if (w->is_group()) {value(i); goto J1;} // menu title
	  focus_index(Group::find(w)); // Set focus_index, so Menu::get_item() works
	  if (checkmark(w)) { w->invert_flag(STATE); redraw(); }
	  execute(w); // button in the menu bar
	  return 1;
	}
      }
    }
    if (event_key()==LeftAccKey || event_key()==RightAccKey) {
      if (style()->hide_underscore() && !event_key_repeated()) redraw();
    }
    return 0;
  case KEYUP:
    // In the future maybe any shortcut() will work, but for now only
    // the Accelerator key does. Setting the shortcut to zero will
    // disable the Accelerator key shortcut.
    if (event_key() != LeftAccKey && event_key() != RightAccKey) break;
    if (style()->hide_underscore()) redraw();
    // checking for event_is_click insures that the keyup matches the
    // keydown that preceeded it, so Accel. was pressed & released without
    // any intermediate values.  On X it is false if Accel. is held down
    // for a long time, too:
    if (!event_is_click()) break;

    // If there's no widget to revert focus,
    // it's better to do nothing in thiscase
    if (!fltk::focus()) {
      lastfocus_ = 0;
      break;
    }

    // We are focused, return focus to old focused widget
    if (focused()) {
      if (lastfocus_) lastfocus_->take_focus();
      lastfocus_ = 0;
      highlight_ = -1; value(-1);
      redraw(DAMAGE_CHILD);
      return 1;
    }

    // okay we got the shortcut, find first menu and pop it up:
    for (i = 0; i < children; i++) {
      Widget* w = child(i);
      if (w->active()) {
	value(i); highlight_ = i;
	lastfocus_ = fltk::focus();
	take_focus();
	return 1;
      }
    }
    break;
  case KEY:
    switch (event_key()) {
    case DownKey:
    case RightKey:
      if ((event_key()==RightKey && vertical()) ||
	  (event_key()==DownKey && !vertical())) {
	/* Popup menu */
	goto J1;
      }
      i=value()+1;
J3:
      for (; i < children; i++) {
	Widget* w = child(i);
	if (w->active()) {
	  value(i); highlight_ = i;
	  redraw(DAMAGE_CHILD);
	  return 1;
	}
      }
      i = 0;
      goto J3;
    case UpKey:
    case LeftKey:
      if ((event_key()==LeftKey && vertical()) ||
	  (event_key()==UpKey && !vertical())) {
	break;
      }
      i=value()-1;
J4:
      for (; i >= 0; i--) {
	Widget* w = child(i);
	if (w->active()) {
	  value(i); highlight_ = i;
	  redraw(DAMAGE_CHILD);
	  return 1;
	}
      }
      i = children-1;
      goto J4;
    default:
      break;
    }
    // Handle shortcuts
    handle_shortcut();
    return 1;
  case UNFOCUS:
    value(-1); highlight_ = -1;
    redraw(DAMAGE_CHILD);
    return 1;
  case FOCUS:
    // Only accept keyboard focus from ACCELERATOR keys
    if (event_key()==LeftAccKey || event_key()==RightAccKey ||
	event_state(ACCELERATOR)) return 1;
    break;
  }
  return 0;
}

static void revert(Style *s) {
  // This is for MenuBar only, the buttonbox and highlight color are used
  // around the menu titles:
#if 0
  // NT 4.0 style
  s->buttonbox_ = FLAT_BOX;
  s->highlight_color_ = WINDOWS_BLUE;
#else
  // Windows98 style:
  s->buttonbox_ = HIGHLIGHT_UP_BOX;
#endif
}

static NamedStyle style("MenuBar", revert, &MenuBar::default_style);
NamedStyle* MenuBar::default_style = &::style;

MenuBar::MenuBar(int x,int y,int w,int h,const char *l)
  : Menu(x,y,w,h,l)
{
  // set the parent style to Menu::default_style, not Widget::default_style:
  default_style->parent_ = this->style();
  style(default_style);
  shortcut(LeftAccKey);
  highlight_ = last_ = -1;
  lastfocus_ = 0;
}

//
// End of "$Id: MenuBar.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
