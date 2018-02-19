//
// "$Id: clock.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Clock test program for the Fast Light Tool Kit (FLTK).
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

#include <fltk/ShapedWindow.h>
#include <fltk/Clock.h>
#include <fltk/PopupMenu.h>
#include <fltk/Menu.h>
#include <fltk/Item.h>
#include <fltk/SharedImage.h>
#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <stdlib.h>

using namespace fltk;
#include "circle.xbm" // bitmap mask for window shape

class ClockWindow : public ShapedWindow {
  public:
    ClockWindow(int W, int H, const char *l = 0) : ShapedWindow(W,H,l) {}
    int handle(int);
};

int ClockWindow::handle(int e) {
  static int bx, by;
  static int button1 = 0;
  if (e == PUSH) button1 = (fltk::event_button() == 1);
  if (button1) switch(e) {
    case DRAG:
      position(x()+event_x_root()-bx, y()+event_y_root()-by);
    case PUSH:
      bx = event_x_root(); by = event_y_root();
      show(); // so the window will click-to-front
      return 1;
  }
  return ShapedWindow::handle(e);
}

Item *about_item, *exit_item;
void callback(Widget* w, void* d) {
  if (((Menu*)w)->value()) exit(0);
  message("FLTK-- Copyright 2000 Bill Spitzak and others");
}

int main(int argc, char **argv) {
  ClockWindow window(220, 220, "fltk::Clock");
  window.color(12);
  window.begin();
  // don't show window manager border-- some window managers do this for you
  // if an arbitrary shape is assigned to the window.
  // previously windows.clear_border();
  // window.border(false); // not needed now because now ShapedWindow implicitly sets border to false.
#ifdef WIN32
  Clock clock(0,0,220,220);
#else
  Clock clock(10,10,200,200); // Fabien : why ?
#endif
  clock.type(ClockOutput::ROUND);
  clock.box(fltk::OVAL_BOX);
  clock.align(fltk::ALIGN_BOTTOM|fltk::ALIGN_WRAP);
  PopupMenu popup(0, 0, 220, 220);
  popup.type(PopupMenu::POPUP3);
  popup.begin();
  about_item = new Item("About clock");
  exit_item = new Item("Exit clock");
  popup.end();
  popup.callback(callback);
  //window.resizable(popup); //- Not today, maybe never!
  window.end();
  window.xclass("fltkClock");
  window.show(argc, argv);
  xbmImage shape(circle_bits, circle_width, circle_height);
  window.shape(shape);

  return run();
}

//
// End of "$Id: clock.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
