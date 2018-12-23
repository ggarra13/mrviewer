//
// "$Id: Item.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $"
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

#include <fltk/Item.h>
#include <fltk/Group.h>
#include <fltk/Box.h>
#include <fltk/CheckButton.h>
#include <fltk/draw.h>
#include <fltk/events.h>
#include <string.h>

using namespace fltk;

/*! \class fltk::Item

  This widget is designed to be put into fltk::Menu and fltk::Browser widgets
  and draw plain-text items. All events are ignored, thus causing
  the menu and browser to set/clear the SELECTED flag on these
  widgets. If they are selected they draw in the selection_color(),
  otherwise in the color().

*/

static void revert(Style* s) {s->box_ = FLAT_BOX;}
static NamedStyle style("Item", revert, &Item::default_style);
/** The default style sets FLAT_BOX. Changing this will mess up the
    appearance of both menus and browsers. All the rest of the style
    is blank, and normally it inherits from the current browser or
    menu, which should call set_style() before drawing any items.
*/
NamedStyle* Item::default_style = &::style;

/** Unlike other widgets the constructor does not take any dimensions,
    since it is assummed the container widget will size this
    correctly.
*/
Item::Item(const char* l) : Widget(0,0,0,0,l) {
  init();
}

void Item::init() {
  // we need to defer setting the glyph to here because C++ has no way
  // to make sure the check button style is constructed before this style:
  if (!default_style->glyph_)
    default_style->glyph_ = CheckButton::default_style->glyph_;
  style(default_style);
  flags(ALIGN_LEFT|ALIGN_INSIDE);
}

/*!
  This constructor is provided to match the Menu::add() function arguments.
  See Menu::add() for more details.
*/
Item::Item(const char* l, int s, Callback *cb, void *ud, int f) : Widget(0,0,0,0,l)  {
  init();
  shortcut(s);
  callback(cb);
  user_data(ud);
  set_flag(f);
}

/*! This constructor also sets the image(), useful for a browser item. */
Item::Item(const char* l, const Symbol* i) : Widget(0,0,0,0,l) {
  init();
  image(i);
}

/** Modify the parent of the Item::default_style to this style.
    If no style settings have been done to an Item, it will use the
    textfont, textsize, textcolor, and possibly other settings
    inherited from this style to draw itself. This is used by menus
    and browsers to cause all the elements inside them to draw
    using their settings.

    The \a menubar flag causes it to mangle the style so that the
    buttonbox of \a style is used as the box, and the highlight_color
    is used as the selection_color. This is done to replicate the
    rather inconsistent appearance on Windows of menubars.

    Use Item::clear_style() to put this back so that \a style can
    be deleted. This is the same as setting it to Widget::default_style.
*/
void Item::set_style(const Style* style, bool menubar) {
  // make sure we don't make a loop:
  for (const Style* p = style; p; p = p->parent_)
    if (p == &::style) return;
  ::style.parent_ = style;

  static Box* savedbox;
  static Color savedcolor;
  static Color savedtextcolor;
  static bool savedmenubar;
  if (menubar) {
    if (!savedmenubar) {
      savedbox = ::style.box_;
      savedcolor = ::style.selection_color_;
      savedtextcolor = ::style.selection_textcolor_;
      savedmenubar = true;
    }
    ::style.box_ = style->buttonbox();
    ::style.selection_color_ = style->highlight_color();
    if (!::style.selection_color_) {
      ::style.selection_color_ = style->color();
      ::style.selection_textcolor_ = style->textcolor();
    }
  } else {
    if (savedmenubar) {
      ::style.box_ = savedbox;
      ::style.selection_color_ = savedcolor;
      ::style.selection_textcolor_ = savedtextcolor;
      savedmenubar = false;
    }
  }
}

/** The SELECTED flag will cause it to draw using the selected colors.

    The current version can also draw check or radio buttons
    but this functionality may be removed.
*/
void Item::draw() {
  drawstyle(style(), flags() & ~OUTPUT);
  if (flag(SELECTED)) {
    setbgcolor(selection_color());
    setcolor(contrast(selection_textcolor(), getbgcolor()));
  }
  Rectangle r(w(),h());
  Box* box = this->box();
  box->draw(r);
  Rectangle r1(r); box->inset(r1);
  if (type()) {
    int gw = int(textsize())+2;
    draw_glyph(0, Rectangle(r1.x()+3, r1.y()+((r1.h()-gw)>>1), gw, gw));
    r1.move_x(gw+3);
  }
  draw_label(r1, flags());
  box->draw_symbol_overlay(r);
}

/** Measure the space the draw() will take and set w() and h().
    Maybe this should be the default layout() function for Widget? */
void Item::layout() {
  if (w() && h()) return; // already at the correct size
  //int dx=0; int dy=0; int dw=0; int dh=0; box()->inset(dx,dy,dw,dh);
  setfont(textfont(), textsize());
  int w = 250, h = 250;
  measure(label(), w, h);
  if (w) {
    w += int(textsize())/2; // put 1 space between menubar items
    h += int(leading());
  }
  if (image()) {
    int W,H;
    image()->measure(W, H);
    if (flag(ALIGN_LEFT|ALIGN_RIGHT)) {
      if (H > h) h = H;
      w += W;
    } else {
      if (w) h += H; else h = H;
      if (W > w) w = W;
    }
  } else if ((type() & RADIO) || (type() & TOGGLE)) {
      w += 20;
  }
  else {
    w += 6; // further adjustment to match Windows menubars
  }
  this->w(w);
  this->h(h);
  Widget::layout();
}

/** Returns 0 always. Items do not accept \e any events. Any results
    of clicking on them is handled by the parent Menu or Browser. */
int Item::handle(int event) {
  if (type() == TOGGLE) {
    if (event == PUSH && event_clicks() > 0) state(!state());
  } else if (type() == RADIO) {
    if (event == PUSH && event_clicks() > 0) {
      state(true);
      Group *g = parent();
      for (int n = 0; n < g->children(); n++) {
        if (g->child(n) != this) g->child(n)->state(false);
      }
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////

/*! \class fltk::ItemGroup

  This widget describes a set of items that are to be put inside a
  fltk::Menu or fltk::Browser widget. It indicates the title of a
  submenu, or a level of hierarchy in the browser. Any child widgets
  are the items in that submenu, or the items under this parent in the
  browser.

  If this widget is told to draw, it draws just like fltk::Item
  draws. See that for more details. The triangle indicating a submenu
  is not drawn by this, it is drawn by the parent menu.

  Because this is an fltk::Menu subclass, you can also call popup()
  and add() and other methods to manipulate the items inside it.

  In a Browser, the value() indicates if the widget is open or not. In
  a MultiBrowser, the selected() indicates if the widget is currently
  selected.

*/

#include <fltk/ItemGroup.h>
#include <fltk/damage.h>

/** Unlike other widgets the constructor does not take any dimensions,
  since it is assummed the container widget will size this
  correctly. */

ItemGroup::ItemGroup(const char* l, bool begin) :
  Menu(0,0,0,0,l,begin)
{
  style(Item::default_style);
  align(ALIGN_LEFT|ALIGN_INSIDE);
  // Undo the Menu class changing default_callback:
  // callback(Widget::default_callback);
}

/*! This constructor also sets image(). */
ItemGroup::ItemGroup(const char* l, const Symbol* i, bool begin) :
  Menu(0,0,0,0,l,begin)
{
  style(Item::default_style);
  align(ALIGN_LEFT|ALIGN_INSIDE);
  image(i);
}

// implementation of draw & layout should be identical to Item type()==0

void ItemGroup::draw() {
  drawstyle(style(), flags() & ~OUTPUT);
  if (flag(SELECTED)) {
    setbgcolor(selection_color());
    setcolor(contrast(selection_textcolor(), getbgcolor()));
  }
  Rectangle r(w(),h());
  Box* box = this->box();
  box->draw(r);
  Rectangle r1(r); box->inset(r1);
  draw_label(r1, flags());
  box->draw_symbol_overlay(r);
}

void ItemGroup::layout() {
  if (w() && h()) return; // already at the correct size
  //int dx=0; int dy=0; int dw=0; int dh=0; box()->inset(dx,dy,dw,dh);
  setfont(textfont(), textsize());
  int w = 250, h = 250;
  measure(label(), w, h);
  if (w) {
    w += int(textsize())/2; // put 1 space between menubar items
    h += int(leading());
  }
  if (image()) {
    int W,H;
    image()->measure(W, H);
    if (flag(ALIGN_LEFT|ALIGN_RIGHT)) {
      if (H > h) h = H;
      w += W;
    } else {
      if (w) h += H; else h = H;
      if (W > w) w = W;
    }
  } else {
    w += 6; // further adjustment to match Windows menubars
  }
  this->w(w);
  this->h(h);
  Widget::layout();
}

/** Returns 0 always. Items do not accept \e any events. Any results
  of clicking on them is handled by the parent Menu or Browser. */
int ItemGroup::handle(int) {return 0;}

////////////////////////////////////////////////////////////////

/*! \class fltk::Divider
  This widget is designed to go into Menu and Browser widgets and draw
  an inset horizontal line across them. It has the OUTPUT flag set
  so the user cannot choose it.
*/

#include <fltk/Divider.h>

/** Unlike other widgets the constructor does not take any dimensions,
  since it is assummed the container widget will size this
  correctly. */
Divider::Divider() : Widget(0,0,0,0) {
  style(Item::default_style);
  set_flag(ALIGN_LEFT|ALIGN_INSIDE|OUTPUT);
}

void Divider::draw() {
  setcolor(GRAY33);
  if (w() > h()) {
    int y = (h()-1)/2;
    drawline(0, y, w()-1, y);
    setcolor(GRAY99);
    drawline(0, y+1, w()-1, y+1);
  } else if (h()) {
    int x = (w()-1)/2;
    drawline(x, 0, x, h()-1);
    setcolor(GRAY99);
    drawline(x+1, 0, x+1, h()-1);
  }
}

/** Resizes to a 2x2 square */
void Divider::layout() {
  if (!w()) w(2);
  if (!h()) h(2);
}

/** Returns 0 always. Items do not accept \e any events. Any results
  of clicking on them is handled by the parent Menu or Browser. */
int Divider::handle(int) {return 0;}
