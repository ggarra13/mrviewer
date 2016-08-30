//
// "$Id$"
//
// Bitmap label test program for the Fast Light Tool Kit (FLTK).
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

#include <fltk/ToggleButton.h>
#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Button.h>
#include <fltk/xbmImage.h>
#include <stdio.h>
#include "escherknot.xbm"

using namespace fltk;

ToggleButton *leftb,*rightb,*topb,*bottomb,*insideb,*inactb;
Button *b;
Window *win;

void button_cb(Widget *,void *) {
  int i = 0;
  if (leftb->value()) i |= ALIGN_LEFT;
  if (rightb->value()) i |= ALIGN_RIGHT;
  if (topb->value()) i |= ALIGN_TOP;
  if (bottomb->value()) i |= ALIGN_BOTTOM;
  if (insideb->value()) i |= ALIGN_INSIDE;
  b->align(i);
  if (inactb->value()) b->deactivate();
  else b->activate();
  win->redraw();
}

int main(int argc, char **argv) {
  win = new Window(400,400);
  win->begin();
  b = new Button(140,160,120,120,"Bitmap");
  b->image(new xbmImage(escherknot_bits,escherknot_width,escherknot_height));
  leftb = new ToggleButton(25,50,50,25,"left");
  leftb->callback(button_cb);
  rightb = new ToggleButton(75,50,50,25,"right");
  rightb->callback(button_cb);
  topb = new ToggleButton(125,50,50,25,"top");
  topb->callback(button_cb);
  bottomb = new ToggleButton(175,50,50,25,"bottom");
  bottomb->callback(button_cb);
  insideb = new ToggleButton(225,50,50,25,"inside");
  insideb->callback(button_cb);
  inactb = new ToggleButton(125,75,100,25,"inactive");
  inactb->callback(button_cb);
  win->resizable(win);
  win->end();
  win->show(argc, argv);
  return fltk::run();
}

//
// End of "$Id$"
//
