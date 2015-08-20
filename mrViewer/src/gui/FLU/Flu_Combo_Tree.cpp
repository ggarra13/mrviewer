// $Id: Flu_Combo_Tree.cpp,v 1.5 2004/08/02 14:18:16 jbryan Exp $

/***************************************************************
 *                FLU - FLTK Utility Widgets 
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 * 
 ***************************************************************/

#include <cstring>
#include <iostream>
using namespace std;

#include <fltk/draw.h>
#include <fltk/damage.h>
#include <fltk/events.h>
#include <fltk/Browser.h>
#include <fltk/MenuWindow.h>

#include "FLU/Flu_Combo_Tree.h"


namespace fltk {

class FL_API ComboBrowser : public Browser {
  public:
    int handle(int);
    ComboBrowser(int x, int y, int w, int h);
    // static void browser_cb(Widget *w, void *data);
};

class ComboWindow : public MenuWindow {
  public:
    int handle(int);
//    ComboWindow(int x, int y, int w, int h) : MenuWindow(x, y, w, h) { box(NO_BOX); }
    ComboWindow(int x, int y, int w, int h) : MenuWindow(x, y, w, h) {}
};

}

using namespace fltk;


// these are only used when in grabbed state so only one exists at once
static MenuWindow *mw;
static InputBrowser *ib;
static Browser *browser;

// CET - FIXME - this doesn't seem to be working
// Use this to copy all the items out of one group into another:
class Share_List : public List {
public:
  virtual ~Share_List() {};

  Menu* other;
  int children(const Menu*, const int* indexes, int level) {
    return other->children(indexes, level);
  }
  Widget* child(const Menu*, const int* indexes, int level) {
    return other->child(indexes, level);
  }
  void flags_changed(const Menu*, Widget* widget) {
    other->list()->flags_changed(other,widget);
  }
};

extern Share_List share_list; // only one instance of this.


static void combo_tree_cb( Widget* w, void* data )
{
  // we get callbacks for all keys?
  if (event() != KEY && event() != RELEASE) return;
  if (event() == KEY
      && event_key() != ReturnKey
      && event_key() != KeypadEnter
      && event_key() != ' ')
    return;
  Widget *item = browser->item();

  if (!item) return;
  if (item->is_group()) return; // can't select a group!

  ib->item(item);
  ib->text(item->label());
  ib->redraw(DAMAGE_VALUE);
  ib->hide_popup();

  ib->do_callback();
}

Flu_Combo_Tree::Flu_Combo_Tree( int X, int Y, int W, int H, const char* l )
  : InputBrowser( X, Y, W, H, l )
{
}

int Flu_Combo_Tree::value()
{ 
  if (list) return list->value(); else return 0;
}

void Flu_Combo_Tree::popup()
{
  bool resize_only = false;

  if (!win || !win->visible()) {
    Group::current(0);

    if(!win) {
      win = new ComboWindow(0,0,0,0); // this will be moved later
      win->set_override();

      win->begin();
      list = new ComboBrowser(0,0,0,0);
      list->box(UP_BOX);
      list->callback(combo_tree_cb, this);
      list->when(WHEN_CHANGED | 
		 WHEN_RELEASE_ALWAYS | 
		 WHEN_ENTER_KEY_ALWAYS);
      list->end();

      win->end();
      win->box(UP_BOX);

      mw = win;
      browser = list;
      ib = this;
    }

    share_list.other = this;
    list->list(&share_list);

    list->indented((type()&INDENTED) != 0);
    win->color(list->color());

  } else {
    resize_only = true;
  }

  list->layout();

  int W = list->width(); //+list->scrollbar.w();
  // magic constant 4 = border width/height (is there a way to calculate it?)
  int H = list->height() + 4 + list->scrollbar.h();

  if (W > maxw_) W = maxw_;
  if (H > maxh_) H = maxh_;
  if (W < minw_) W = minw_;
  if (H < minh_) H = minh_;
  int X = event_x_root()-event_x();
  int Y = event_y_root()-event_y()+h();

  // I don't know what this code does, but it doesn't work
  // WAS: I believe it is trying to make the menu go above the combobox
  //      if it does not fit below on the screen
  /*      const Monitor& monitor = Monitor::find(event_x_root(), event_y_root());
	  int down = monitor.h() - Y;
	  int up = event_y_root() - event_y();
	  if (H > down) {
	  if (up > down) {
	  Y = event_y_root() - event_y() - H;
	  if (Y < 0) { Y = 0; H = up; }
	  } else {
	  H = down;
	  }
	  }
	  if (X + W > monitor.r()) {
	  X = monitor.r() - W;
	  if (X < 0) { X = 0; W = monitor.r(); }
	  }*/

  win->resize(X, Y, W, H);
  list->Widget::resize(W, H);

  // find the currently selected item in the list
  list->value(0);
  for (int i=0; i<list->children(); i++) {
    Widget* w=list->child(i);
    if (!strncmp(text(), w->label(), size())) {
      list->value(i);
      list->make_item_visible();
      break;
    }
  }

  if(resize_only) return;

  redraw(DAMAGE_VALUE);

  win->exec(NULL, true);

  if(type()&NONEDITABLE) throw_focus();
  else focus(m_input);

  redraw(DAMAGE_VALUE);
}

int Flu_Combo_Tree::popup(int x, int y, int w, int h) 
{ 
  Flu_Combo_Tree::popup(); return Menu::popup(Rectangle(x,y,w,h));
}
