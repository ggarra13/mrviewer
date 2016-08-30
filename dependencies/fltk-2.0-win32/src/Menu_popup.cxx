//
// "$Id: Menu_popup.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Implementation of popup menus.  These are called by using the
// Menu::popup and Menu::pulldown methods.  See also the
// Menu base class in Menu.cxx.  Warning: this code is
// quite a mess!  Also, the classes defined here are private and
// may change in the future!
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

#include <string.h>
#include <fltk/MenuWindow.h>
#include <fltk/Menu.h>
#include <fltk/Box.h>
#include <fltk/LabelType.h>
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/run.h>
#include <fltk/Monitor.h>
#include <ctype.h>

#include <fltk/Item.h> // for TOGGLE, RADIO
#define checkmark(item) (item->type()>=Item::TOGGLE && item->type()<=Item::RADIO)

using namespace fltk;

// class MenuTitle is a small window for the title, it will look like
// it is being drawn in the menu bar, but is actually a window
// floating above it (this was a rather clever trick SGI came up with
// and makes the menus faster if backing store works on the X server).

// class MenuWindow is the larger window containing all the menu
// items.  It is NOT the Menu itself, instead it has a pointer
// called "list" that points at it.  This contains a lot of ugly code
// to draw the menu items and to draw arrows for parents and draw
// shortcuts.

// The rest of the code is the run-time system for Menu::popup.
// Menu::popup does not return until the user has selected an item
// and all the widgets created here are destroyed.

// grab() is done to send all events to our own handle function
// The entire menu "state" is stuck into a structure passed to the
// handle mentod.  The handle() method changes variables in this state
// to indicate what item is picked, but does not actually alter the
// display, instead the main loop does that.  This is because the X
// mapping and unmapping of windows is slow, and we don't want to fall
// behind the events.  This communication is unfortunately extremely
// messy.

#define MAX_LEVELS 30

class MWindow;

struct MenuState {
  int level;	// which level of nesting current item is in
  int indexes[MAX_LEVELS]; // index in each level of selected item
  MWindow* menus[MAX_LEVELS]; // windows that have been created
  int nummenus; // number of windows (may be level+1 or level+2)
  bool menubar; // if true menus[0] is a menubar
  bool hmenubar; // menubar&&menus[0] is horizontal
  bool changed;	// did the menu items change
  int state; // one of the enumeration below
  Menu* widget; // widget that is making this menu
  // return the widget being pointed at:
  Widget* current_widget() {
    return widget->child(indexes, level);
  }
  int current_children() { // # of children of current item
    return widget->children(indexes, level+1);
  }
  MWindow* fakemenu;
};

////////////////////////////////////////////////////////////////

class MenuTitle : public MenuWindow {
  void draw();
  MenuState* menustate;
public:
  MenuTitle(MenuState* m, int X, int Y, int W, int H, const char* L)
    : MenuWindow(X, Y, W, H, L), menustate(m)
  {
    // can't use sgi overlay for images:
    //if (L->image()) clear_overlay();
  }
};

extern bool fl_hide_underscore;

void MenuTitle::draw() {

  const Style* style = menustate->widget->style();
  if (style->hide_underscore()) fl_hide_underscore = true;

  if (menustate->menubar) {

    // Get the Item directly from the menubar and draw it:
    Item::set_style(menustate->widget, true);
    Widget* item = menustate->widget->child(menustate->indexes[0]);

    push_matrix();
    int save_w = item->w();
    int save_h = item->h();
    Flags save_flags = item->flags();
    Flags flags = save_flags;
    if (menustate->hmenubar) flags &= ~ALIGN_MASK;

    item->flags(flags|SELECTED|HIGHLIGHT|PUSHED);
    item->w(w());
    item->h(h());
    item->draw();
    item->w(save_w);
    item->h(save_h);
    item->flags(save_flags);

    if (!menustate->hmenubar &&
	menustate->widget->children(menustate->indexes,1)>=0) {
      // Use the item's fontsize for the size of the arrow, rather than h:
      int nh = int(item->textsize());
      draw_glyph(ALIGN_RIGHT, Rectangle(w()-nh, ((h()-nh)>>1), nh, nh));
    }

    pop_matrix();
    Item::clear_style();

  } else {
    // a title on a popup menu is drawn like a button
    drawstyle(style, 0);
    Rectangle r(w(),h());
    style->buttonbox()->draw(r);
    draw_label(r, 0);
  }
  fl_hide_underscore = false;
}

////////////////////////////////////////////////////////////////

class MWindow : public MenuWindow {
  void draw();
public:
  MenuState* menustate;
  int level;
  int children;
  MenuTitle* title;
  bool is_menubar;
  int drawn_selected;	// last redraw has this selected
  MWindow(MenuState*, int level, const Rectangle& R, const char* title, int rightedge);
  ~MWindow();
  int find_selected(int mx, int my);
  int titlex(int);
  int autoscroll(int);
  void position(int x, int y);
  int ypos(int);
  Widget* get_widget(int i);
  int handle(int);
};

// return any widget at a given level:
Widget* MWindow::get_widget(int index) {
  int saved = menustate->indexes[level];
  menustate->indexes[level] = index;
  Widget* w = menustate->widget->child(menustate->indexes, level);
  menustate->indexes[level] = saved;
  return w;
}

static const Monitor* monitor;
#define MENUAREA (*monitor)
// If we are unable to get our menus atop the toolbar (this was happening
// in WIN32 for awhile) change the definition to this to make the menus
// scroll to avoid the toolbar:
//#define MENUAREA (monitor->work)

////////////////////////////////////////////////////////////////

static Box* box_from_menuwindow;

// Return the box to draw around the outside edge of widget
// Unfortunately all popups in your program need to match
// due to the need to be compatable with Windows
// menubars which have flatbox, and to avoid down boxes:
static Box* menubox(Widget* widget) {
  // If it is a real widget return it's box
  if (widget->parent()) return widget->box();
  // otherwise it is a popup menu, return the value saved from
  // MenuWindow::box():
  return box_from_menuwindow;
}

/*! Resize the widget to contain the menu items that are the children
    of the item indicated by indexes and level (use 0 for the immediate
    children).

    If the widget has vertical() true the menu items are laid out one
    above the other, as is usally done in the pull-down menu below a
    menubar. w() is set to the widest item, and h() to the sum of all
    the heights.

    If the widget has horizontal() true then the items are laid out in
    rows, wrapping only when w() is exceeded. w() is changed only if it
    is too small to contain the smallest item. h() is set to the total
    height of all the rows.

    The box(), leading(), textfont(), textsize(), and perhaps other
    style attributes of the widget are used when figuring out the total
    size and the size of each item.
*/
void Menu::layout_in(Widget* widget, const int* indexes, int level) const {
  // vertical orientation only...
  if (widget->horizontal()) return;
  Box* box = menubox(widget);

  int W = 0;
  int hotKeysW = 0;
  int H = 0;
  int children = this->children(indexes, level);
  Item::set_style(widget, widget->parent()!=0);
  int array[20];
  int i; for (i = 0; i < level; i++) array[i] = indexes[i];
  for (i = 0; i < children; i++) {
    array[level] = i;
    Widget* item = child(array, level);
    if (!item->visible()) continue;
    // force items with @ commands in label to re-layout:
    if (item->label() && item->label()[0]=='@') item->w(0);
    if (!item->w()) item->layout();
    H += item->h();
    if (item->w() > W) W = item->w();
    if (this->children(array,level+1)>=0) {
      if (16 > hotKeysW) hotKeysW = 16;
    } else {
      unsigned hotkey = item->shortcut();
      if (hotkey) {
	int w1 = int(getwidth(key_name(hotkey)) + 8.5);
	if (w1 > hotKeysW) hotKeysW = w1;
      }
    }
  }
  Item::clear_style();

  Rectangle r(W,H);
  box->inset(r);
  W += hotKeysW + (W-r.w());
  if (W > widget->w()) widget->w(W);
  widget->h(H + (H-r.h()));
}

/*! Draw the menu items inside the widget.

    The widget's box() is drawn and the items are laid out exactly the
    same as for layout().

    If \a selected is greater or equal to zero then that item is
    drawn in a selected manner.

    If widget->damage()==DAMAGE_CHILD then it is assummed that only
    the items indicated by \a selected and \a drawn_selected need
    to be redrawn. This is used for minimal update to move the
    selection from one item to the next.
*/
void Menu::draw_in(Widget* widget, const int* indexes, int level,
		   int selected, int drawn_selected) const {
  Box* box = menubox(widget);
  const unsigned char damage = widget->damage();

  Rectangle r(widget->w(), widget->h());
  drawstyle(widget->style(),0);
  if (damage != DAMAGE_CHILD) box->draw(r);
  box->inset(r);

  int children = this->children(indexes,level);
  if (children<1) return;
  int array[20];
  int i; for (i = 0; i < level; i++) array[i] = indexes[i];

  Item::set_style(widget, widget->parent()!=0);
  if (!widget->shortcut() ||
      (widget->style()->hide_underscore() && !event_state(ACCELERATOR)))
    fl_hide_underscore = true;

  const bool horizontal = widget->horizontal();
  int spacing = 0;
  if (horizontal) {
    spacing = int(widget->leading());
    r.move(spacing,1); r.h(r.h()-2);
  }

  Rectangle ir(r);
  for (i = 0; i < children; i++) {
    array[level] = i;
    Widget* item = child(array, level);
    if (!item->visible()) continue;
    if (!item->w()) item->layout();
    if (horizontal) {
      ir.w(item->w() + spacing);
    } else {
      ir.h(item->h());
    }

    // for minimal update, only draw the items that changed selection:
    if (damage != DAMAGE_CHILD || i==selected || i==drawn_selected) {

      Flags flags = item->flags();
      if (flags&INACTIVE) flags |= INACTIVE_R;
      if (i == selected && !(flags & (OUTPUT|INACTIVE))) {
	flags |= (SELECTED|HIGHLIGHT);
      } else {
	flags &= ~(SELECTED|HIGHLIGHT);
      }
      if (damage==DAMAGE_CHILD) {
	// see if we need to erase background
	Box* b = item->box();
	if (!b->fills_rectangle()) {
	  push_clip(ir);
	  Rectangle r(widget->w(), widget->h());
	  drawstyle(widget->style(),0);
	  box->draw(r);
	  pop_clip();
	}
      }

      push_matrix();
      translate(ir.x(), ir.y());
      int save_w = item->w(); item->w(ir.w());
      int save_h = item->h(); item->h(ir.h());
      int save_flags = item->flags();
      if (horizontal) flags &= ~ALIGN_MASK; // make it center them
      item->flags(flags);
      item->draw();
      item->w(save_w);
      item->h(save_h);
      pop_matrix();
      flags &= ~(STATE|ALIGN_MASK);

      if (horizontal) ;
      else if (this->children(array,level+1)>=0) {
	// Use the item's fontsize for the size of the arrow, rather than h:
	int nh = int(item->textsize());
	draw_glyph(ALIGN_RIGHT, Rectangle(ir.r()-nh, ir.y()+((ir.h()-nh)>>1), nh, nh));
      } else if (!widget->parent()) {
	unsigned hotkey = item->shortcut();
	if (hotkey)
	  item->labeltype()->draw(key_name(hotkey),
				  Rectangle(ir.x(), ir.y(), ir.w()-3, ir.h()),
				  ALIGN_RIGHT);
      }
      item->flags(save_flags);
    }
    if (horizontal) ir.move(ir.w(),0);
    else ir.move(0,ir.h());
  }
  Item::clear_style();
  fl_hide_underscore = false;
}

/*! Return the index of the item that is under the location mx, my in
    the given widget, if the draw() method had been used to draw the
    items into the widget.
*/
int Menu::find_selected(Widget* widget, const int* indexes, int level,
			int mx, int my) const {
  if (mx < 0 || my < 0) return -1;
  if (widget->horizontal()) {
    if (my >= widget->h()) return -1;
  } else {
    if (mx >= widget->w()) return -1;
  }
  Rectangle r(widget->w(),widget->h());
  menubox(widget)->inset(r);

  int children = this->children(indexes,level);
  int array[20];
  int i; for (i = 0; i < level; i++) array[i] = indexes[i];

  Item::set_style(widget, widget->parent()!=0);
  int ret = -1;
  if (widget->horizontal()) {
    int spacing = int(widget->leading());
    int x = r.x()+spacing;
    for (i = 0; i < children; i++) {
      array[level] = i;
      Widget* item = child(array, level);
      if (!item->visible()) continue;
      if (!item->w()) item->layout();
      x += item->w()+spacing;
      if (x > mx) {/*if (item->takesevents())*/ ret = i; break;}
    }
  } else {
    int y = r.y();
    for (int i = 0; i < children; i++) {
      array[level] = i;
      Widget* item = child(array, level);
      if (!item->visible()) continue;
      if (!item->h()) item->layout();
      y += item->h(); // find bottom edge
      if (y > my) {/*if (item->takesevents())*/ ret = i; break;}
    }
  }
  Item::clear_style();
  return ret;
}

/*! Return the bounding box of the given item inside the widget, if
    the draw() method had been used to draw the items into the widget.
*/
Rectangle Menu::get_location(Widget* widget, const int* indexes, int level,
			     int index) const
{
  Rectangle r(widget->w(), widget->h());
  menubox(widget)->inset(r);

  int array[20];
  int i; for (i = 0; i < level; i++) array[i] = indexes[i];

  Item::set_style(widget, widget->parent()!=0);
  if (widget->horizontal()) {
    int spacing = int(widget->leading());
    r.move(spacing,0);
    for (int i = 0; i < index; i++) {
      array[level] = i;
      Widget* item = child(array, level);
      if (!item->visible()) continue;
      if (!item->w()) item->layout();
      r.move(item->w()+spacing,0);
    }
    array[level] = index;
    Widget* item = child(array, level);
    if (!item->w()) item->layout();
    r.w(item->w()+spacing);
  } else {
    for (int i = 0; i < index; i++) {
      array[level] = i;
      Widget* item = child(array, level);
      if (!item->visible()) continue;
      if (!item->h()) item->layout();
      r.move(0,item->h());
    }
    array[level] = index;
    Widget* item = child(array, level);
    if (item) {
      if (!item->h()) item->layout();
      r.h(item->h());
    }
  }
  Item::clear_style();
  return r;
}

////////////////////////////////////////////////////////////////

MWindow::MWindow(MenuState* m, int l, const Rectangle& rectangle,
		 const char* t, int rightedge)
  : MenuWindow(rectangle.x(), rectangle.y(), rectangle.w(), rectangle.h(), 0), menustate(m), level(l)
{
  box_from_menuwindow = style()->box();
  style(menustate->widget->style());
  set_vertical();

  children = m->widget->children(m->indexes, l);

  int selected = l <= menustate->level ? menustate->indexes[l] : -1;

  drawn_selected = -1;

  is_menubar = !level && menustate->menubar;
  if (is_menubar) {
    title = 0;
    return;
  }

  Rectangle temprect(100,100);
  menubox(this)->inset(temprect);
  const int dh = 100-temprect.h();
  int Wtitle = 0;
  int Htitle = 0;

  if (menustate->menubar && level==1) {
    // Widget title
    Widget* widget = menustate->widget->child(menustate->indexes[0]);
    if (!widget->w()) widget->layout();
    Wtitle = widget->w()+int(this->leading());
    Htitle = widget->h()+dh;
    title = new MenuTitle(menustate, 0, 0, Wtitle, Htitle, 0);
  } else if (t) {
    // label title
    setfont(labelfont(), labelsize());
    Wtitle = Htitle = 300; // rather arbitrary choice for maximum wrap width
    measure(t, Wtitle, Htitle, ALIGN_WRAP);
    Wtitle += 16;
    Htitle += dh;
    title = new MenuTitle(menustate, 0, 0, Wtitle, Htitle, t);
  } else {
    title = 0;
  }

  menustate->widget->layout_in(this, menustate->indexes, level);
  if (Wtitle > w()) w(Wtitle);

  int X = rectangle.x();
  int Y = rectangle.y();
  int Wp = rectangle.w();
  int Hp = rectangle.h();

  // Move horizontally to center it, if we did not give an inital width
  // (which indicates we are trying to line up with some widget):
  if (!Wp) {
    if (selected >= 0) X -= w()/2;
  }

  // Restrict to monitor area:
  if (X < MENUAREA.x()) X = MENUAREA.x();
  if (X > MENUAREA.r()-w()) {
    if (rightedge) X = rightedge-w();
    else X = MENUAREA.r()-w();
  }

  x(X);
  if (selected >= 0) {
    Rectangle r(menustate->widget->get_location(this, menustate->indexes, level, selected));
    Y += Hp/2-r.center_y();
  } else {
    Y += Hp;
  }
  y(Y); // if !group: else {y(Y-2); w(1); h(1);}
  if (title) title->position(X,Y-title->h()-dh/2);
}

MWindow::~MWindow() {
  style(default_style);
  delete title;
}

void MWindow::position(int X, int Y) {
  if (title) {title->position(X, title->y()+Y-y());}
  MenuWindow::position(X, Y);
  // x(X); y(Y); // don't wait for response from X
}

// return the top edge of item:
int MWindow::ypos(int index) {
  Widget* widget = this;
  if (menustate->menubar && !level) widget = menustate->widget;
  return menustate->widget->get_location(widget,
					 menustate->indexes, level,
					 index).y();
}

// return the left edge of an item:
int MWindow::titlex(int index) {
  Widget* widget = this;
  if (menustate->menubar && !level) widget = menustate->widget;
  return menustate->widget->get_location(widget,
					 menustate->indexes, level,
					 index).x();
}

void MWindow::draw() {
  int selected = level <= menustate->level ? menustate->indexes[level] : -1;
  menustate->widget->draw_in(this, menustate->indexes, level,
			     selected, drawn_selected);
  drawn_selected = selected;
}

int MWindow::find_selected(int mx, int my) {
  mx -= x();
  my -= y();
  Widget* widget = this;
  if (menustate->menubar && !level) widget = menustate->widget;
  return
    menustate->widget->find_selected(widget,
				     menustate->indexes, level, mx, my);
}

////////////////////////////////////////////////////////////////
// The Menu::popup() run-time:

enum {INITIAL_STATE = 0,// no mouse up or down since popup() called
      PUSH_STATE,	// mouse has been pushed on a normal item
      DONE_STATE	// execute the selected item
};

// scroll so item i is visible on screen, return true if it moves
#define BORDER 0
int MWindow::autoscroll(int i) {
  if (is_menubar || i < 0) return 0;
  // figure out where the item is on the screen:
  int Y = i ? ypos(i) : 0;
  int newy = y();
  // figure out where new top of menu should be:
  if (newy+Y <= MENUAREA.y()) {
    newy = MENUAREA.y()-Y+BORDER;
  } else {
    if (i >= children-1) {
      Y = h();
    } else {
      Widget* widget = get_widget(i);
      if (!widget->h()) widget->layout();
      Y += widget->h();
    }
    if (newy+Y >= MENUAREA.b()) newy = MENUAREA.b()-Y-BORDER;
    else return 0;
  }
  if (newy == y()) return false;
  // move it to that new position:
  MWindow::position(x(), newy);
  return true;
}

#if 0
static void autoscroll_timeout(void*) {
  // this will call MWindow::handle(MOVE) but also set event()
  // so that the timeout gets repeated.
  handle(MOVE, 0);
}
#endif

static bool setitem(MenuState& p, int level, int index) {

  if (level < 0) return false; // this should not happen!
  if (p.level == level && p.indexes[level] == index) return false;

  if (level < p.nummenus && p.indexes[level] != index)
    p.menus[level]->redraw(DAMAGE_CHILD);
  if (level+1 < p.nummenus && p.indexes[level+1] >= 0)
    p.menus[level+1]->redraw(DAMAGE_CHILD);

  delete p.fakemenu; p.fakemenu = 0; // turn off "menubar button"

  // delete all the submenus that we are no longer in:
  if (index >= 0) {
    while (p.nummenus > level+2) delete p.menus[--p.nummenus];
    // delete the next menu only if we are pointing at a different item:
    if (p.nummenus > level+1 && p.indexes[level] != index)
      delete p.menus[--p.nummenus];
  }

  p.level = level;
  p.indexes[level] = index;
  p.indexes[level+1] = -1;

  // set flag so new menus are created by the main idle loop:
  p.changed = true;

  p.menus[level]->autoscroll(index);
  return true;
}

static bool forward(MenuState& p, int menu) {
  // go to next item in menu menu if possible
  MWindow &m = *(p.menus[menu]);
  for (int item = p.indexes[menu]+1; item < m.children; item++) {
    Widget* widget = m.get_widget(item);
    if (widget->takesevents()) return setitem(p, menu, item);
  }
  return false;
}

static bool backward(MenuState& p, int menu) {
  // previous item in menu menu if possible
  MWindow &m = *(p.menus[menu]);
  for (int item = p.indexes[menu]-1; item >= 0; item--) {
    Widget* widget = m.get_widget(item);
    if (widget->takesevents()) return setitem(p, menu, item);
  }
  return false;
}

static bool track_mouse;
static bool valuator;

int MWindow::handle(int event) {
  MenuState &p = *menustate;
  Widget* widget = 0;

  switch (event) {
  case KEY:
    track_mouse = event_state(ANY_BUTTON);
    switch (event_key()) {
    case LeftAccKey:
    case RightAccKey:
      if (style()->hide_underscore() && !event_clicks()) {
	for (int i = 0; i < p.nummenus; i++)
	  p.menus[i]->redraw();
      }
      return 1;
    case UpKey:
      if (p.hmenubar && p.level == 0) ;
      else if (backward(p, p.level));
      else if (p.hmenubar) setitem(p, 0, p.indexes[0]);
      return 1;
    case DownKey:
      if (p.level || !p.hmenubar) forward(p, p.level);
      else if (p.level+1 < p.nummenus) forward(p, p.level+1);
      return 1;
    case RightKey:
      if (p.hmenubar && (p.level<=0 || (p.level==1 && p.nummenus==2)))
	forward(p, 0);
      else if (p.level+1 < p.nummenus) forward(p, p.level+1);
      return 1;
    case LeftKey:
      if (p.hmenubar && p.level<=1) backward(p, 0);
      else if (p.level > 0)
	setitem(p, p.level-1, p.indexes[p.level-1]);
      return 1;
    case SpaceKey:
    case ReturnKey:
    case KeypadEnter:
      // hitting the shortcut twice exits the menu:
      if (!p.widget->test_shortcut()) goto EXECUTE;
    case EscapeKey:
      exit_modal();
      return 1;
    default:
      // hitting the shortcut twice exits the menu:
      if (!p.widget->test_shortcut()) break;
      exit_modal();
      return 1;
    }
    {for (int menu = p.nummenus; menu--;) {
      MWindow &mw = *(p.menus[menu]);
      // Apparent rules, at least for Firefox:
      // Check for &x, if there is none check for leading 'x'
      // If there is exactly one, execute it (STR#980, STR#2078)
      // Otherwise cycle through the matches
      // Whether Alt is held down is apparently irrelevant.
      int current_item = p.indexes[menu];
      int new_item = -1;
      bool found_underscore = false;
      bool execute_it = true;
      for (int item = 0; item < mw.children; item++) {
	widget = mw.get_widget(item);
	if (!widget->takesevents()) continue;
	if (widget->test_label_shortcut()) {
          if (!found_underscore) {
            found_underscore = true;
            new_item = -1;
            execute_it = true;
          }
	} else if (found_underscore) {
          continue;
        } else {
	  const char* l = widget->label();
	  if (!l || tolower(*l) != event_text()[0]) continue;
	}
        if (new_item >= 0) { // found more than one item
          execute_it = false;
          if (item > current_item && new_item <= current_item)
            new_item = item;
        } else {
          if (widget->is_group()) // don't execute submenu titles
            execute_it = false;
          new_item = item;
        }
      }
      if (new_item >= 0) {
	setitem(p, menu, new_item);
        if (execute_it) goto EXECUTE;
      }
    }}
    return 1; // always eat all the keystrokes

  case KEYUP:
    if (event_key()==LeftAccKey || event_key()==RightAccKey) {
      if (event_is_click()) {
	// checking for event_clicks insures that the keyup matches the
	// keydown that preceeded it, so Alt was pressed & released without
	// any intermediate values.
	exit_modal();
	return 1;
      }
      if (style()->hide_underscore()) {
	for (int i = 0; i < p.nummenus; i++)
	  p.menus[i]->redraw();
      }
    }
    return 0;

  case PUSH:
    track_mouse = true;
    p.state = PUSH_STATE;
  case ENTER:
  case MOVE:
    if (!track_mouse) return 1;
  case DRAG: {
    int mx = event_x_root();
    int my = event_y_root();
    int item, menu;
    // search the visible menus from top down for item being pointed at:
    for (menu = p.nummenus-1; ; menu--) {
      if (menu < 0) { // not pointing at anything:
	// quit if they clicked off the menus:
	if (event == PUSH) {exit_modal(); return 0;}
	// keep the lowest-visible menu visible but with nothing selected:
	menu = p.nummenus-1;
	item = -1;
	break;
      }
      item = p.menus[menu]->find_selected(mx, my);
      if (item >= 0) break;
    }
    if (setitem(p, menu,item)) return 1;
    if (item < 0) return 1;

    if (event == PUSH) {
      // redraw checkboxes so they preview the state they will be in:
      Widget* widget = p.menus[menu]->get_widget(item);
      if (checkmark(widget)) 
        p.menus[menu]->redraw(DAMAGE_CHILD);
      else if (widget->handle(event))
        valuator = true;
    } else if (event == DRAG) {
      // allow scrolling for valuators, but ignore their handle() returns
      Widget* widget = p.menus[menu]->get_widget(item);
      if (valuator) {
        widget->handle(event);
        p.menus[menu]->redraw(DAMAGE_CHILD);
      }
    } else if (p.level || !p.hmenubar) {
      // item didn't change on drag/move, check for autoscroll:
      if (event_y_root() <= MENUAREA.y()) {
	if (!p.menus[menu]->autoscroll(item)) backward(p, p.level);
      } else if (event_y_root() >= MENUAREA.b()-1) {
	if (!p.menus[menu]->autoscroll(item)) forward(p, p.level);
      }
    }
    return 1;
  }

  case RELEASE:
    pushed_ = 0;
    valuator = false;
    // The initial click just brings up the menu. The user has to either
    // drag the mouse around, hold it still for a long time, or click
    // again to actually pick an item and dismiss the menu. You can
    // make the first click pick the item (which might be useful for
    // a Choice style list) by setting event_is_click(false) before
    // doing the popup.
    if (p.state == INITIAL_STATE && event_is_click()) {
      // except for buttons in the menubar, they execute on click:
      if (!p.menubar || p.level || p.nummenus > 1) return 1;
    }
  EXECUTE: // execute the item pointed to by w and current item
    if (p.indexes[p.level] >= 0) {
      widget = p.current_widget();
      if (widget->takesevents()) {
#if 0
	if (widget->flag(MENU_STAYS_UP) && (!p.menubar || p.level)) {
	  p.widget->set_item(p.indexes, p.level);
	  p.widget->execute(widget);
	  Window* mw = p.menus[p.level];
	  if (widget->type() == Item::RADIO) mw->redraw();
	  else if (checkmark(widget)) mw->redraw(DAMAGE_CHILD);
	  return 1;
	}
#endif
	p.state = DONE_STATE;
      }
    }
    exit_modal();
    return 1;
  }
  return MenuWindow::handle(event);
}

/*!
  Exactly the same as popup() except the selected child widget is
  returned, rather than execute() being called. This is probably
  more useful in most cases...  */
Widget* Menu::try_popup(const Rectangle& r, const char* title, bool menubar)
{
  Group::current(0); // fix possible programmer error...

  // figure out where to pop up in screen coordinates:
  Rectangle rectangle(r);
  if (parent()) {
    for (Widget* w = this; w; w = w->parent())
      rectangle.move(w->x(), w->y());
  } else {
    rectangle.move(event_x_root()-event_x(),
		   event_y_root()-event_y());
  }
  if (fltk::event() == fltk::PUSH)
    monitor = &Monitor::find(event_x_root(), event_y_root());
  else
    monitor = &Monitor::all();

  MenuState p;
  p.nummenus = 1;
  p.menubar = menubar;
  p.hmenubar = menubar && horizontal();
  p.widget = this;
  p.state = INITIAL_STATE;
  p.level = 0;
  p.indexes[0] = value();
  p.indexes[1] = -1;

  MWindow toplevel(&p, 0, rectangle, title, 0);
  toplevel.child_of(Window::first());
  p.menus[0] = &toplevel;
  p.fakemenu = 0;

  track_mouse = true; //event_state(ANY_BUTTON);
  if (menubar) {
    if (value() < 0) {
      toplevel.handle(PUSH); // get menu mouse points at to appear
    } else {
      track_mouse = false;
      p.indexes[0] = value();
      p.indexes[1] = -1;
      p.level = 0;
    }
    p.changed = true; // make it create the pulldown menu
  } else {
    // create submenus until we locate the one with selected item
    // in it, positioning them so that one is selected:
    for (;;) {
      if (p.indexes[p.level] < 0) break;
      //      if (p.current_children() < 0) break;
      Widget* widget = p.current_widget();
      if (!widget) break;
      if (!widget->is_group()) break;
      if (!widget->takesevents()) break;
      Group* group = (Group*)widget;
      int item = group->focus_index();
      if (item >= group->children()) break;
      MWindow* mw = p.menus[p.level];
      int nX = mw->x() + mw->w();
      int nY = mw->y() + mw->ypos(p.indexes[p.level])-mw->ypos(0);
      p.level++;
      p.indexes[p.level] = item;
      p.indexes[p.level+1] = -1;
      mw = new MWindow(&p, p.level, rectangle, 0,0);
      p.menus[p.nummenus++] = mw;
      // move all earlier menus to line up with this new one:
      int dy = mw->y()-nY;
      int dx = mw->x()-nX;
      for (int menu = 0; menu < p.level; menu++) {
	MWindow* t = p.menus[menu];
	int nx = t->x()+dx; if (nx < 0) {nx = 0; dx = -t->x();}
	int ny = t->y()+dy; if (ny < 0) {ny = 0; dy = -t->y();}
	t->position(nx, ny);
      }
    }
    // show all the menus:
    for (int menu = 0; menu <= p.level; menu++) {
      MWindow* mw = p.menus[menu];
      if (mw->title) mw->title->show(mw->child_of());
      mw->show();
    }
    p.changed = true;
  }

  Widget* saved_modal = modal(); bool saved_grab = grab();
  p.state = INITIAL_STATE;
#ifdef DEBUG
#define MODAL false
#else
#define MODAL true
#endif
  for (modal(&toplevel, MODAL); !exit_modal_flag(); wait()) {

    if (!p.changed) continue;

    // Create windows for submenus. Since this is expensive we don't
    // do this when the item changes, instead we wait until there are no
    // events (after wait() returns) to create menus. This means that
    // intermediate windows are not created when the user moves the mouse
    // quickly.
    p.changed = false;

    int index = p.indexes[p.level];
    if (index < 0) continue;  // no item selected, so no submenu changes
    if (p.level < p.nummenus-1) continue; // submenu already up

    Widget* widget = p.current_widget();

    MWindow* mw = p.menus[p.level];

    if (!widget->takesevents()) {
      // create nothing for these
    } else if (p.menubar && !p.level) {
      // create a submenu off a menubar with a title box:
      Rectangle r(p.widget->get_location(p.widget, p.indexes, 0, index));
      r.move(mw->x(), mw->y());
      int mx = r.x();
      int my = r.y();
      if (p.hmenubar) {my += r.h(); r.move_y(1); r.move_b(-1);}
      else mx += r.w();
      mw = new MWindow(&p, 1, Rectangle(mx, my, 0, 0), 0, 0);
      *(Rectangle*)(mw->title) = r;
      mw->title->show(p.menus[0]->child_of());
      if (widget->takesevents() && p.current_children()>=0) {
	// if it is a real menu we add it to the list of displayed menus
	p.menus[p.nummenus++] = mw;
	mw->show(p.menus[0]->child_of());
      } else {
	// Non-submenus in the menubar produce "buttons" which I "highlight"
	// my making a fake submenu with only the title box displayed.
	p.fakemenu = mw;
      }

    } else if (p.current_children()>=0) {
      // Create a normal submenu:
      int nX = mw->x() + mw->w();
      int nY = mw->y() + mw->ypos(index) - mw->ypos(0);
      mw = new MWindow(&p, p.nummenus, Rectangle(nX, nY, 0, 0), 0,
		       p.nummenus ? p.menus[p.nummenus-1]->x() : 0);
      p.menus[p.nummenus++] = mw;
      mw->show(p.menus[0]->child_of());

    }
  }

  //fltk::remove_timeout(autoscroll_timeout, &p);
  Item::clear_style();

  // destroy all the submenus we created:
  delete p.fakemenu;
  while (--p.nummenus) delete p.menus[p.nummenus];

  // I believe this is here so that if you exec() a window in response
  // to a menu item the correct window is selected as the parent:
  // The cast is to remove the const from toplevel.child_of()
  Window::first((Window*)toplevel.child_of());

  toplevel.hide();
  modal(saved_modal, saved_grab);

  if (p.state != DONE_STATE) return 0; // user did not pick anything

  // Execute whatever item the user picked:
  set_item(p.indexes, p.level);
  if (menubar && !p.level && checkmark(item())) redraw();
  return item();
}

/*!
  Create and display a pop-up menu (or hierarchy of menus) showing the
  children of this Menu, then wait until the user picks an item or
  dismisses the menu. If the user picks an item then execute() is
  called for it and true is returned. False is returned if the user
  cancels the menu.

  If there is a selected item in the menu (as determined by get_item())
  then submenus are opened and all of them are positioned intitially
  so the mouse cursor is pointing at the selected item. This is
  incredibly useful and one of the main features of fltk that is missing
  from other toolkits.

  \a x,y,w,h describe a rectangle that the current menu item should be
  centered over, and the menu is widened horizontally to \a w if it is
  narrower. The coordinates are measured relative to the widget whose
  handle() method is being executed now.

  \a title is a widget (usually an fltk::Item) that is used to make a
  title atop the menu, in the style of SGI's popup menus. You cannot
  use a List child, as the drawing of the menu may navigate that list
  to other children, overwriting the original widget.

  \a menubar is for internal use by menubars and should be left false.
*/
int Menu::popup(const Rectangle& rectangle, const char* title, bool menubar)
{
  Widget *selected = try_popup(rectangle, title, menubar);
  if (selected) {
    if (selected->type()==Widget::TOGGLE)
      selected->invert_flag(STATE);
    else if (selected->type()==Widget::RADIO)
      selected->setonly();
    execute(selected);
    return 1;
  }
  return 0;
}

//
// End of "$Id: Menu_popup.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
