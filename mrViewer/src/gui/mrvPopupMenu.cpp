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

/*! \class fltk::MyPopupMenu

This subclass pops up a menu in response to a user click. The menu is
popped up positioned so that the mouse is pointing at the last-selected
item, even if it in a nested submenu (To turn off this behaivor do
value(-1) after each item is selected)

\image html menu_button.gif

Normally any mouse button will pop up a menu and it is lined up above
the button, or below it when there is no previous selected value as
shown in the picture.

However a MyPopupMenu can also have type() set to POPUP1, POPUP2,
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

#include <mrvPopupMenu.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/Item.h>


extern fltk::Widget* fl_did_clipping;

namespace mrv {

using namespace fltk;

static mrv::PopupMenu* pushed;

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
  if (mrv::pushed == this) flags |= PUSHED|HIGHLIGHT;
  drawstyle(style(), flags);
  Rectangle r(w(),h());
  box->draw(r);
  Rectangle r1(r); box->inset(r1);
  // draw the little mark at the right:

  if ( _enable_glyph )
  {
      box->inset(r);
      int w1 = r.h();
      r.move_r(-r.h());
      r.x(r.r()); r.w(w1);
      const Color saved_color = getcolor();
      setcolor(fltk::GRAY35);
      draw_glyph(ALIGN_BOTTOM, r);
      //  draw_glyph(ALIGN_BOTTOM, x+w-w1, y, w1, h, flags);
      setcolor(saved_color);
  }

  draw_label(r1, flags);


  box->draw_symbol_overlay(r);
}



// static NamedStyle style("mrvPopupMenu", 0, &fltk::PopupMenu::default_style);
// NamedStyle* PopupMenu::default_style = &mrv::style;

PopupMenu::PopupMenu(int X,int Y,int W,int H,const char *l)
: fltk::PopupMenu(X,Y,W,H,l),
  _enable_glyph( true )
{
  // set the parent style to Menu::default_style, not Widget::default_style:
  default_style->parent_ = this->style();
  style(default_style);
  align(ALIGN_CENTER);
  
  //set_click_to_focus();
}

} // namespace mrv

//
// End of "$Id: MyPopupMenu.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
