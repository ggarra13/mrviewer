// "$Id: PopupMenu.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $"
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

/*! \class fltk::PopupMenu

This subclass pops up a menu in response to a user click. The menu is
popped up positioned so that the mouse is pointing at the last-selected
item, even if it in a nested submenu (To turn off this behaivor do
value(-1) after each item is selected)

\image html menu_button.gif

Normally any mouse button will pop up a menu and it is lined up above
the button, or below it when there is no previous selected value as
shown in the picture.

However a PopupMenu can also have type() set to POPUP1, POPUP2,
POPUP12, POPUP3, POPUP13, POPUP23, or POPUP123. It then becomes invisible
and ignores all mouse buttons other than the ones named in the popup
type. You can then resize it to cover another widget (or many widgets)
so that pressing that button pops up the menu.

The menu will also pop up in response to shortcuts indicated by
the shortcut() or by putting '&x' in the label().

Typing the fltk::Widget::shortcut() of any menu items will cause it
to be picked. The callback will be done but there will be no visible
effect to the widget.

*/

#include <fltk/PopupMenu.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/Item.h>

using namespace fltk;

extern Widget* fl_did_clipping;
static PopupMenu* pushed;

/*! The little down-arrow indicator can be replaced by setting a new
  glyph() function and making it draw whatever you want.
  If you don't want any glyph at all it is probably easiest to
  subclass and replace draw() with your own function.
*/
void PopupMenu::draw() {
  if (type()&7) { // draw nothing for the popup types
    fl_did_clipping = this;
    return;
  }
  // set_item() does not cause a redraw:
  if (damage() == DAMAGE_VALUE) return;

  Box* box = this->buttonbox();
  if (!box->fills_rectangle()) draw_background();
  Flags flags = this->flags()|OUTPUT;
  if (::pushed == this) flags |= PUSHED|HIGHLIGHT;
  drawstyle(style(), flags);
  Rectangle r(w(),h());
  box->draw(r);
  Rectangle r1(r); box->inset(r1);
  draw_label(r1, flags);
  // draw the little mark at the right:
  box->inset(r);
  int w1 = r.h();
  r.move_r(-r.h());
  r.x(r.r()); r.w(w1);
  const Color saved_color = getcolor();
  setcolor(selection_color());
  draw_glyph(ALIGN_BOTTOM, r);
  //  draw_glyph(ALIGN_BOTTOM, x+w-w1, y, w1, h, flags);
  setcolor(saved_color);
  box->draw_symbol_overlay(r);
}

/*! Wrapper for Menu::popup(). For NORMAL PopupMenu this places the
  menu over the widget. For POPUP ones it uses the mouse position
  and sets the "title" to the label() if it is not null.
*/
int PopupMenu::popup() {
  if (type()&7) {
    return Menu::popup(Rectangle(event_x(), event_y(), 0, 0), label());
  } else {
    return Menu::popup(Rectangle(w(), h()));
  }
}

int PopupMenu::handle(int e) {
  if (box() == NO_BOX) type(POPUP3); // back compatibility hack
  if (!children()) return 0;
  switch (e) {

  case FOCUS:
  case UNFOCUS:
    if (type()&7) return 0;
    redraw(DAMAGE_HIGHLIGHT);
    return 1;

  case ENTER:
  case LEAVE:
    if (type()&7) return 0;
    redraw(DAMAGE_HIGHLIGHT);
  case MOVE:
    return 1;

  case PUSH:
    // If you uncomment this line (or make a subclass that does this) then
    // a mouse click picks the current item, and the menu goes away.  The
    // user must drag the mouse to select a different item.  Depending on
    // the size and usage of the menu, this may be more user-friendly:
    // event_is_click(0);
    if (type()&7) {
      if (!(type() & (1 << (event_button()-1)))) return 0;
    } else {
      if (click_to_focus()) take_focus();
    }
  EXECUTE:
    ::pushed = this;
    clear_flag(HIGHLIGHT);
    //if (!(type()&7)) value(-1); // make it pull down below the button...
    popup();
    ::pushed = 0;
    redraw();
    return 1;

  case SHORTCUT:
    if (test_shortcut()) goto EXECUTE;
    return handle_shortcut();

  case KEY:
    if (event_key() == ReturnKey ||
	event_key() == KeypadEnter ||
	event_key() == SpaceKey) goto EXECUTE;
    return 0;

  default:
    return 0;
  }
}

static NamedStyle style("PopupMenu", 0, &PopupMenu::default_style);
NamedStyle* PopupMenu::default_style = &::style;

PopupMenu::PopupMenu(int X,int Y,int W,int H,const char *l)
  : Menu(X,Y,W,H,l)
{
  // set the parent style to Menu::default_style, not Widget::default_style:
  default_style->parent_ = this->style();
  style(default_style);
  align(ALIGN_CENTER);
  //set_click_to_focus();
}

//
// End of "$Id: PopupMenu.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
