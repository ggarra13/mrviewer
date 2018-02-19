//
// "$Id: resizealign.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#define W1 40
#define B 0
#define W3 (5*W1+6*B)

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/ToggleButton.h>

using namespace fltk;

Window *window;
int flags = 0;

void b_cb(Widget *b,long w) {
  if (((Button*)b)->value()) flags |= int(w);
  else flags &= ~int(w);
  window->resize_align(flags);
  window->relayout();
  window->redraw();
}

int main(int argc, char **argv) {
  window = new Window(W3,W3);
  window->box(NO_BOX);
  window->begin();
  Widget *n;
  for (int x = 0; x<4; x++) for (int y = 0; y<4; y++) {
    if ((x==1 || x==2) && (y==1 || y==2)) continue;
    n = new Widget(x*(B+W1)+B,y*(B+W1)+B,W1,W1,0);
    n->box(ENGRAVED_BOX);
    n->color(x+y+8);
  }
  n = new Widget(B,4*W1+5*B,4*W1+3*B,W1,0);
  n->box(ENGRAVED_BOX);
  n->color(12);
  n = new Widget(4*W1+5*B,B,W1,5*W1+4*B,0);
  n->box(ENGRAVED_BOX);
  n->color(13);

  n = new Widget(W1+B+B,W1+B+B,2*W1+B,2*W1+B,0);
  n->box(ENGRAVED_BOX);
  n->color(8);
  window->resizable(n);
  n->align(ALIGN_INSIDE|ALIGN_BOTTOM|ALIGN_WRAP);
  n->label("This gray area is the window->resizable().\n"
	   "The arrow buttons change the "
	   "window->resizealign() value.");
  n->labelcolor(GRAY75);

  Button *b;
  b = new ToggleButton(W1+B+50,W1+B+30,20,20,"@6>");
  b->callback(b_cb,8);
  b = new ToggleButton(W1+B+30,W1+B+10,20,20,"@8>");
  b->callback(b_cb,1);
  b = new ToggleButton(W1+B+10,W1+B+30,20,20,"@4>");
  b->callback(b_cb,4);
  b = new ToggleButton(W1+B+30,W1+B+50,20,20,"@2>");
  b->callback(b_cb,2);
  window->resize_align(0);

  window->end();

  window->size_range(W3,W3);
  window->layout();
  window->resize(W3*2,W3*2);
  window->show(argc,argv);
  return run();
}

//
// End of "$Id: resizealign.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
