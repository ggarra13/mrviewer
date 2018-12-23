//
// "$Id: scroll.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $"
//
// ScrollGroup test program for the Fast Light Tool Kit (FLTK).
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
#include <fltk/ScrollGroup.h>
#include <fltk/LightButton.h>
#include <fltk/Choice.h>
#include <fltk/Box.h>
#include <stdio.h>
#include <fltk/draw.h>
#include <fltk/math.h>
#include <fltk/InvisibleBox.h>
#include <fltk/MenuBuild.h>

using namespace fltk;

class Drawing : public Widget {
  void draw();
public:
  Drawing(int X,int Y,int W,int H,const char* L) : Widget(X,Y,W,H,L) {
    align(fltk::ALIGN_TOP);
    box(fltk::FLAT_BOX);
    color(fltk::WHITE);
  }
};

void Drawing::draw() {
  draw_box();
  fltk::push_matrix();
  // Change from fltk1 to 2: removed translation by x(),y():
  fltk::translate(w()/2, h()/2);
  fltk::scale(w()/2, h()/2);
  fltk::color(fltk::BLACK);
  for (int i = 0; i < 20; i++) {
    for (int j = i+1; j < 20; j++) {
      // Change from fltk1 to 2: added 'f' to make arguments float:
      fltk::newpath();
      fltk::addvertex(cosf(M_PI*i/10+.1), sinf(M_PI*i/10+.1));
      fltk::addvertex(cosf(M_PI*j/10+.1), sinf(M_PI*j/10+.1));
//      fltk::closepath(); //bug
      fltk::strokepath();
      // you must reset the line type when done:
      line_style(SOLID);
    }
  }
  fltk::pop_matrix();
}

ScrollGroup* thescroll;

void box_cb(Widget* o, void*) {
  thescroll->box(((Button*)o)->value() ? fltk::DOWN_BOX : fltk::NO_BOX);
  thescroll->relayout(); // added for fltk2
  thescroll->redraw();
}

void type_cb(Widget*, void* v) {
  thescroll->type(int((long)v));
  thescroll->relayout(); // changed from fltk1 setting of redraw()
}

void load_menu_choice (Choice* c)  {
 c->begin();
  new Item("None", 0, type_cb, (void*)ScrollGroup::NONE);
  new Item("HORIZONTAL", 0, type_cb, (void*)ScrollGroup::HORIZONTAL);
  new Item("VERTICAL", 0, type_cb, (void*)ScrollGroup::VERTICAL);
  new Item("BOTH", 0, type_cb, (void*)ScrollGroup::BOTH);
  new Item("HORIZONTAL_ALWAYS", 0, type_cb, (void*)ScrollGroup::HORIZONTAL_ALWAYS);
  new Item("VERTICAL_ALWAYS", 0, type_cb, (void*)ScrollGroup::VERTICAL_ALWAYS);
  new Item("BOTH_ALWAYS", 0, type_cb, (void*)ScrollGroup::BOTH_ALWAYS);
 c->end();
}

void align_cb(Widget*, void* v) {
  thescroll->scrollbar_align(int((long)v));
  //thescroll->scrollbar.align(int((long)v)); // fltk1 version
  thescroll->relayout(); // changed from fltk1 setting of redraw()
}

void load_menu_achoice (Choice* c)  {
 c->begin();
  new Item("left+top", 0, align_cb, (void*)(fltk::ALIGN_LEFT+fltk::ALIGN_TOP));
  new Item("left+bottom", 0, align_cb, (void*)(fltk::ALIGN_LEFT+fltk::ALIGN_BOTTOM));
  new Item("right+top", 0, align_cb, (void*)(fltk::ALIGN_RIGHT+fltk::ALIGN_TOP));
  new Item("right+bottom", 0, align_cb, (void*)(fltk::ALIGN_RIGHT+fltk::ALIGN_BOTTOM));
 c->end();
}

int main(int argc, char** argv) {
  Window window(5*75,400);
  //window.clear_double_buffer(); // use this to test scroll_area()
  //window.box(fltk::NO_BOX); //remove trash
  window.begin();
  ScrollGroup scroll(0,0,5*75,300);
  scroll.begin();

  int n = 0;
  for (int y=0; y<16; y++) for (int x=0; x<5; x++) {
    char buf[20]; sprintf(buf,"%d",n++);
    Button* b = new Button(x*75,y*25+(y>=8?5*75:0),75,25);
    b->copy_label(buf);
    b->color(n);
//    b->labelcolor(fltk::WHITE);
  }
  Drawing drawing(0,8*25,5*75,5*75,0);
  scroll.end();
  scroll.type(scroll.VERTICAL);
  window.resizable(scroll);

  LightButton but1(150, 310, 200, 25, "box");
  but1.callback(box_cb);

  Choice choice(150, 335, 200, 25, "type():");
  load_menu_choice(&choice);
  choice.value(2);

  Choice achoice(150, 360, 200, 25, "scrollbar_align():");
  load_menu_achoice(&achoice);
  achoice.value(3);

  thescroll = &scroll;

  scroll.box(fltk::DOWN_BOX);
  scroll.type(ScrollGroup::VERTICAL);
  window.end();
  window.show(argc,argv);
  return fltk::run();
}

//
// End of "$Id: scroll.cxx 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
