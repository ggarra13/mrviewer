//
// "$Id$"
//
// Pixmap label test program for the Fast Light Tool Kit (FLTK).
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
#include <fltk/Window.h>
#include <fltk/Button.h>
#include <fltk/xpmImage.h>
#include <stdio.h>

#include "porsche.xpm"

#include <fltk/ToggleButton.h>

using namespace fltk;

ToggleButton *leftb,*rightb,*topb,*bottomb,*insideb,*inactb,*fitb,*fillb;
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
  if (fitb->value()) b->set_flag(RESIZE_FIT);
  else b->clear_flag(RESIZE_FIT);
  if (fillb->value()) b->set_flag(RESIZE_FILL);
  else b->clear_flag(RESIZE_FILL);
  if (inactb->value()) b->deactivate();
  else b->activate();
  win->redraw();
}

int dvisual = 0;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
  return 0;
}

int main(int argc, char **argv) {
  Window window(400,400); win = &window;
  window.begin();
  Button b(140,160,120,120,"Pixmap"); ::b = &b;
  xpmImage *pixmap = new xpmImage(porsche_xpm);

  b.image(pixmap);

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
  fitb = new ToggleButton(25,75,50,25,"fit");
  fitb->callback(button_cb);
  fillb = new ToggleButton(75,75,50,25,"fill");
  fillb->callback(button_cb);
  inactb = new ToggleButton(125,75,100,25,"inactive");
  inactb->callback(button_cb);
  window.resizable(window);
  window.end();
  window.show(argc,argv);
  return fltk::run();
}

//
// End of "$Id$"
//
