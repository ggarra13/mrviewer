//
// "$Id: $"
//
// AnsiWindow test program for the Fast Light Tool Kit (FLTK).
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

#include <fltk/run.h>
#include <fltk/Button.h>
#include <fltk/Window.h>
#include <fltk/AnsiWidget.h>

void more_cb(Widget*, void* p) {
  AnsiWidget* out = (AnsiWidget*)p;
  for (int i=0; i<10; i++) {
    out->print("\033[3mitalic\033[23moff\033[4munderline\033[24moff");
    out->print("\033[7minverse\033[27moff");
    out->print("\033[1mbold\033[21moff");
  }
}

int main(int argc, char **argv) {
  int w = 210; // must be > 104
  int h = 200;
  Window window(w, h);
  window.begin();

  AnsiWidget out(0, 22, w, h, 11);
  window.resizable(&out);

  Button* b = new Button(0, 0, 40, 20, "Print");
  b->callback(more_cb);
  b->user_data(&out);

  window.end();
  window.show(argc,argv);

#if USE_X11
  while (!ready()) {
    flush();
    check();
  }
#else
  check();
#endif

  return run();
}

