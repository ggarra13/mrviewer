//
// "$Id: Menu_global.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/events.h>
#include <fltk/Menu.h>
#include <fltk/Window.h>

using namespace fltk;

static Menu* the_widget;

static int handler(int e, Window*) {
  if (e != SHORTCUT || modal()) return 0;
  Window::first(the_widget->window());
  return the_widget->handle_shortcut();
}

/*!
  Make the shortcuts for this menu work no matter what window has the
  focus when you type it (as long as fltk::modal() is off). This is
  done by using fltk::add_handler(). This fltk::Menu widget does not
  have to be visible (ie the window it is in can be hidden, or it does
  not have to be put in a window at all).

  Currently there can be only one global() menu. Setting a new one will
  replace the old one. There is no way to remove the global() setting,
  and you cannot destroy the Menu!

  This should probably also put the items on the the Mac menubar.
*/
void Menu::global() {
  if (!the_widget) add_event_handler(handler);
  the_widget = this;
}

//
// End of "$Id: Menu_global.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
