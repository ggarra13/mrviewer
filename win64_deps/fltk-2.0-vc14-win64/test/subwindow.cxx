//
// "$Id: subwindow.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Nested window test program for the Fast Light Tool Kit (FLTK).
//
// Test to make sure nested windows work.
// Events should be reported for enter/exit and all mouse operations
// Buttons and pop-up menu should work, indicating that mouse positions
// are being correctly translated.
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

#include <stdlib.h>
#include <stdio.h>
#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/ToggleButton.h>
#include <fltk/PopupMenu.h>
#include <fltk/Input.h>
#include <fltk/InputBrowser.h>
#include <fltk/events.h>
using namespace fltk;

#define DEBUG 1

class testwindow : public Window {
  int handle(int);
  void draw();
public:
  testwindow(Box* b,int x,int y,const char *l)
    : Window(x,y,l) {box(b);}
  testwindow(Box* b,int x,int y,int w,int h,const char *l)
    : Window(x,y,w,h,l) {box(b);}
};

void testwindow::draw() {
#ifdef DEBUG
  printf("%s : draw\n",label());
#endif
  Window::draw();
}

class EnterExit : public Widget {
  uchar oldcolor;
  int handle(int);
public:
  EnterExit(int x, int y, int w, int h, const char *l) :
    Widget(x,y,w,h,l) {box(BORDER_BOX);}
};

int EnterExit::handle(int e) {
  if (e == ENTER) {oldcolor = color(); color(RED); redraw(); return 1;}
  else if (e == LEAVE) {color(oldcolor); redraw(); return 1;}
  else if (e == MOVE) return 1;
  else return 0;
}

#ifdef DEBUG
const char *eventnames[] = {
"zero",
"PUSH",
"RELEASE",
"ENTER",
"LEAVE",
"DRAG",
"FOCUS",
"UNFOCUS",
"KEY",
"KEYUP",
"MOVE",
"SHORTCUT",
"ACTIVATE",
"DEACTIVATE",
"SHOW",
"HIDE",
"VIEWCHANGE",
"PASTE",
"SELECTIONCLEAR",
};
#endif

PopupMenu* popup;

int testwindow::handle(int e) {
#ifdef DEBUG
  if (e != MOVE) printf("%s : %s\n",label(),eventnames[e]);
#endif
  if (Window::handle(e)) return 1;
  //  if (e==PUSH) return popup->handle(e);
  return 0;
}

const char* bigmess =
#if 0
"this|is|only|a test"
#else
"item1|item2|item3|item4|item5|"
"submenu/item1|submenu/item2|submenu/item3|submenu/item4|"
"submenu/sub/item1|submenu/sub/item2|submenu/sub/item3|"
"item6|item7|item8|item9|item10|"
"item21|item22|item23|item24|item25|"
"submenu/item21|submenu/item22|submenu/item23|submenu/item24|"
"submenu/sub/item21|submenu/sub/item22|submenu/sub/item23|"
"item36|item37|item38|item39|item310|"
"item31|item32|item33|item34|item35|"
"submenu/item31|submenu/item32|submenu/item33|submenu/item34|"
"submenu/sub/item31|submenu/sub/item32|submenu/sub/item33|"
"item46|item47|item48|item49|item410|"
"item41|item42|item43|item44|item45|"
"submenu/item41|submenu/item42|submenu/item43|submenu/item44|"
"submenu/sub/item41|submenu/sub/item42|submenu/sub/item43|"
"item26|item27|item28|item29|item210|"
"submenu2/item1|submenu2/item2|submenu2/item3|submenu2/item4|"
"submenu2/sub/item1|submenu2/sub/item2|submenu2/sub/item3|"
"item6|item7|item8|item9|item10|"
"item21|item22|item23|item24|item25|"
"submenu2/item21|submenu2/item22|submenu2/item23|submenu2/item24|"
"submenu2/sub/item21|submenu2/sub/item22|submenu2/sub/item23|"
"item36|item37|item38|item39|item310|"
"item31|item32|item33|item34|item35|"
"submenu2/item31|submenu2/item32|submenu2/item33|submenu2/item34|"
"submenu2/sub/item31|submenu2/sub/item32|submenu2/sub/item33|"
"item46|item47|item48|item49|item410|"
"item41|item42|item43|item44|item45|"
"submenu2/item41|submenu2/item42|submenu2/item43|submenu2/item44|"
"submenu2/sub/item41|submenu2/sub/item42|submenu2/sub/item43|"
"item26|item27|item28|item29|item210|"
#endif
;

int main(int, char **) {
  testwindow *window =
    new testwindow(UP_BOX,400,400,"outer");
  window->begin();
//  (new Menu_Button(5,150,80,25,"menu&1"))->add_many(bigmess);
  testwindow *subwindow =
    new testwindow(DOWN_BOX,100,100,200,200,"inner");
  subwindow->color(YELLOW);
  subwindow->begin();
  (new PopupMenu(50,50,80,25,"menu&2"))->add_many(bigmess);
  (void) new Input(45,80,150,25,"input:");
  (void) new EnterExit(10,110,80,80,"enterexit");
  (void) new ToggleButton(110,110,80,80,"&inner");
  subwindow->resizable(subwindow);
  window->resizable(subwindow);
  subwindow->end();
  subwindow->tooltip(
"This is a child fltk::Window. This may be necessary for imbedding "
"controls that need a different X visual, OpenGL (using fltk::GlWindow), "
"or to use the system's clipping to the edge of the window. This program "
"tests for bugs in event handling and redrawing of these windows.\n\n"

"Things to check:\n"
" Outer border always draws exactly around yellow area.\n"
" enterexit widgets turn red when pointed to by mouse, even if this "
"happens when you exit a popup menu or move from an overlapping "
"window belonging to another application, or raise this window.\n"
" focus can move between both windows and you can type to both "
"input widgets.");
  (void) new EnterExit(10,310,80,80,"enterexit");
  (void) new Input(150,310,150,25,"input:");
  (void) new ToggleButton(310,310,80,80,"&outer");
#if 0
  { InputBrowser *o = new InputBrowser(5,150,80,25,"menu&1");
    o->type(InputBrowser::NONEDITABLE_INDENTED);
    o->add_many(bigmess);
    o->value(o->child(0)->label());
  }
#endif
  popup = new PopupMenu(0,0,400,400);
  popup->type(PopupMenu::POPUP3);
  popup->add_many("This|is|a popup|menu");
  window->end();
  subwindow->show(); // this should do nothing. On older fltk it crashed.
  window->show(); // this actually shows the window + subwindow
  return fltk::run();
}

//
// End of "$Id: subwindow.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
