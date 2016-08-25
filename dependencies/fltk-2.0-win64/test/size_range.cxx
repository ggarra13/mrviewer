//
// "$Id$"
//
// size_range demo : constraining the window dimensions
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

#include "size_range.h"
// Code for //\n// "$Id"\n//\n// HelpDialog dialog for the Fa...
#include <fltk/run.h>

UI::UI() {
  fltk::DoubleBufferWindow* w;
   {fltk::DoubleBufferWindow* o = window = new fltk::DoubleBufferWindow(195, 176);
    w = o;
    o->type(241);
    o->user_data((void*)(this));
    o->begin();
    fltk::Button *b = new fltk::LightButton(25, 25, 68, 20, "button");
    o->end();
    o->size_range(0,0,195,0);
  }
}

int main() {
  UI ui;
  ui.window->show();
  return fltk::run();
}
// End of "$Id$"
