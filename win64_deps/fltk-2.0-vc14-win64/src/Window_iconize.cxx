//
// "$Id: Window_iconize.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <config.h>
#include <fltk/Window.h>
#include <fltk/x.h>
using namespace fltk;

extern bool fl_show_iconic; // in x.C/win32.C

/*! Iconifies the window. If you call this when shown() is false it
  will show() it as an icon. If the window is already iconified this
  does nothing.

  Call show() to restore the window. 

  Currently there are only X and Win32 system-specific ways to control
  what is drawn in the icon. You should not rely on window managers
  displaying the icons. */
void Window::iconize() {
  if (!i) {
    fl_show_iconic = true;
    show();
  } else {
#if USE_X11
    XIconifyWindow(xdisplay, i->xid, xscreen);
    i->wait_for_expose = true;
#elif defined(_WIN32)
    ShowWindow(i->xid, SW_SHOWMINNOACTIVE);
#elif defined(__APPLE__)
    CollapseWindow(i->xid, true);
#else
#endif
  }
}

//
// End of "$Id: Window_iconize.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
