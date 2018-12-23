//
// "$Id: iconize.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Iconize test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Button.h>
#include <stdio.h>
#include <stdlib.h>
using namespace fltk;

void hide_cb(Widget *, void *v) {
  Window *w = (Window *)v;
  w->hide();
}

void iconize_cb(Widget *, void *v) {
  Window *w = (Window *)v;
  w->iconize();
}

void destroy_cb(Widget *, void *v) {
  Window *w = (Window *)v;
  w->destroy();
}

void show_cb(Widget *, void *v) {
  Window *w = (Window *)v;
  w->show();
}

void big_cb(Widget *, void *v) {
  Window *w = (Window *)v;
  w->resize(800,800);
}

void small_cb(Widget *, void *v) {
  Window *w = (Window *)v;
  w->resize(200,200);
}

void fson_cb(Widget *, void *v) {
  Window *w = (Window *)v;
  w->fullscreen();
}

void fsoff_cb(Widget *, void *v) {
  Window *w = (Window *)v;
  w->fullscreen_off(100,100,200,200);
}

void window_cb(Widget*, void*) {
  exit(0);
}

int main(int argc, char **argv) {

  Window mainw(200,200);
  mainw.resizable(mainw);
  Window child(50,50,100,100);
  child.color(1);
  mainw.add(child);
  mainw.resizable(child);
  mainw.show(argc,argv);

  Window control(120,9*30,"Window actions");
  control.begin();

  int y = 0;
  Button hide_button(0,y,120,30,"&hide()");
  hide_button.callback(hide_cb, &mainw);
  y += 30;

  Button iconize_button(0,y,120,30,"&iconize()");
  iconize_button.callback(iconize_cb, &mainw);
  y += 30;

  Button destroy_button(0,y,120,30,"&destroy()");
  destroy_button.callback(destroy_cb, &mainw);
  y += 30;

  Button show_button(0,y,120,30,"&show()");
  show_button.callback(show_cb, &mainw);
  y += 30;

  Button big_button(0,y,120,30,"resize(&big)");
  big_button.callback(big_cb, &mainw);
  y += 30;

  Button small_button(0,y,120,30,"resize(&little)");
  small_button.callback(small_cb, &mainw);
  y += 30;

  Button fson_button(0,y,120,30,"fullscreen()");
  fson_button.callback(fson_cb, &mainw);
  y += 30;

  Button fsoff_button(0,y,120,30,"&fullscreen_off()");
  fsoff_button.callback(fsoff_cb, &mainw);
  y += 30;

  Button show_button2(0,y,120,30,"show this");
  show_button2.callback(show_cb, &control);
  y += 30;

  control.tooltip("Push these buttons to cause actions to the main window.\n\n"
		  "Also you should try running this with -i to iconize "
		  "the window on startup, and -g WxH+X+Y to position "
		  "the window on startup.");

  control.end();
  control.show();
  control.callback(window_cb);
  while (fltk::wait()) {
    static int oldarray[6] = {0,0,0,0,0,0};
    int array[6];
    array[0] = mainw.x();
    array[1] = mainw.y();
    array[2] = mainw.w();
    array[3] = mainw.h();
    array[4] = mainw.visible();
    array[5] = mainw.iconic();
    bool print = false;
    for (int i = 0; i < 6; i++) {
      if (array[i] != oldarray[i]) print = true;
      oldarray[i] = array[i];
    }
    if (print) {
      printf("window is %d %d %d %d, visible %d, iconic %d\n",
	     array[0], array[1], array[2], array[3], array[4], array[5]);
//       fltk::Rectangle r; mainw.borders(&r);
//       printf("borders %d %d %d %d\n", r.x(),r.y(),r.r(),r.b());
    }
  }
  return 0;
}

//
// End of "$Id: iconize.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
