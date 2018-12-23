//
// "$Id: Wizard.cxx 4288 2005-04-16 00:13:17Z mike $"
//
// Wizard demo inspired from Greg Ercolano cheat page
//
// Copyright 1997-2005 by Easy Software Products.
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
//     http://www.fltk.org/str.php
//
//

#include <stdlib.h>
#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Group.h>
#include <fltk/Button.h>
#include <fltk/WizardGroup.h>

using namespace fltk;

// Simple 'wizard' using fltk's new Wizard widget

Window *G_win;
WizardGroup *G_wiz;

void done_cb(Widget*, void*) { G_win->hide(); }
void back_cb(Widget*, void*) { G_wiz->prev(); }
void next_cb(Widget*, void*) { G_wiz->next(); }

const int WIDTH  = 140;
const int HEIGHT = 180;

void add_group(const char * text, Color c, bool first, bool last) {
  Group  *g = new Group(0,0,WIDTH,HEIGHT);
  g->begin();
  if (!first) (new Button(20,25,WIDTH-40,25,"<< Back"))->callback(back_cb);
  if (!last)  (new Button(20,55,WIDTH-40,25,"Next >>"))->callback(next_cb);
  else        (new Button(20,55,WIDTH-40,25,"Finish"))->callback(done_cb);
  Widget *out = new Widget(10,100,WIDTH-20,HEIGHT-100-10);
  out->color(c); out->label(text);
  g->end();
}

int main(int argc, char **argv) {
  G_win = new Window(WIDTH,HEIGHT);
  G_win->begin();
  G_wiz = new WizardGroup(0,0,WIDTH, HEIGHT);
  G_wiz->begin();
  add_group("This is the\nFirst\npage", fltk::RED,true, false ); // Wizard: page 1
  add_group("This is the\nSecond\npage", fltk::YELLOW, false, false); // Wizard: page 2
  add_group("This is the\nLast\npage", fltk::GREEN, false, true); // Wizard: page 3
  G_wiz->end();
  G_win->end();
  
  G_win->show(argc, argv);
  
  return fltk::run();
}

//
// End of "$Id"
//
