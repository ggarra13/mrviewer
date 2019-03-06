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

/*! \class mrv::PopupMenu

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

Typing the Fl_Widget::shortcut() of any menu items will cause it
to be picked. The callback will be done but there will be no visible
effect to the widget.

*/

#include "mrvPopupMenu.h"

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Item.H>

#include <iostream>


extern Fl_Widget* fl_did_clipping;

namespace mrv {


static mrv::PopupMenu* pushed;


/*! The little down-arrow indicator can be replaced by setting a new
  glyph() function and making it draw whatever you want.
  If you don't want any glyph at all it is probably easiest to
  subclass and replace draw() with your own function.
*/
void PopupMenu::draw() {
    Fl_Menu_Button::draw();
}

const Fl_Menu_Item* PopupMenu::child(int i) {
    return &(menu()[i]);
}

int PopupMenu::add_leaf( const char* name, Fl_Menu_Item* g )
{
    for ( int i = 0; i < children(); ++i )
    {
	const Fl_Menu_Item* w = child(i);
	if ( w == g )
	{
	    int idx = g->add( name, 0, NULL );
	    return i + idx;
	}
    }
}


// static NamedStyle style("mrvPopupMenu", 0, &Fl_PopupMenu::default_style);
// NamedStyle* PopupMenu::default_style = &mrv::style;

PopupMenu::PopupMenu(int X,int Y,int W,int H,const char *l)
    : Fl_Menu_Button(X,Y,W,H,l),
      _enable_glyph( true )
{
    // set the parent style to Menu::default_style, not Widget::default_style:
    align(FL_ALIGN_CENTER);

    //set_click_to_focus();
}

} // namespace mrv

//
// End of "$Id: MyPopupMenu.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
