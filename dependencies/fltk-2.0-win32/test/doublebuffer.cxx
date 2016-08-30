//
// "$Id: doublebuffer.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Test of double_buffer and overlay of Windows.
// This demo shows how double buffering helps, by drawing the
// window in a particularily bad way.
//
// The single-buffered window will blink as it updates.  The
// double buffered one will not.  It will take just as long
// (or longer) to update, but often it will appear to be faster.
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
#include <fltk/draw.h>
#include <fltk/Slider.h>
#include <fltk/CheckButton.h>
#include <fltk/visual.h>
#include <fltk/math.h>
#include <stdio.h>

using namespace fltk;

int sides = 20;
int overlay_sides = 20;

class BlinkWindow : public Window {

  void draw() {
    setcolor(BLACK);
    fillrect(0,0,w(),h());
    push_matrix();
    translate(w()/2, h()/2);
    scale(w()/2, h()/2);
    int n = sides;
    for (int i = 0; i < n; i++) {
      for (int j = i+1; j < n; j++) {
	setcolor((i+j)%(5*8*5)+BLACK); // random color from cube
	fltk::addvertex(float(cosf(2*M_PI*(j-1)/n+.1)),
                        float(sinf(2*M_PI*(j-1)/n+.1)));
	fltk::addvertex(float(cosf(2*M_PI*i/n+.1)),
                        float(sinf(2*M_PI*i/n+.1)));
	fltk::addvertex(float(cosf(2*M_PI*j/n+.1)),
                        float(sinf(2*M_PI*j/n+.1)));
	fillstrokepath(WHITE);
      }
    }
    pop_matrix();
    char buffer[128];
    sprintf(buffer,"FLTK doublebuffer demo. This image has %d outlined triangles", n*(n-1)/2);
    setfont(HELVETICA_BOLD_ITALIC, 24);
    setcolor(BLACK);
    drawtext(buffer,Rectangle(10,h()/2,w()-20,h()/2),ALIGN_WRAP|ALIGN_TOP);
  }

  void draw_overlay() {
    setcolor(RED);
    push_matrix();
    translate(w()/2, h()/2);
    scale(w()/2, h()/2);
    int n = overlay_sides;
    for (int i = 0; i < n; i++) {
      for (int j = i+1; j < n; j++) {
        fltk::addvertex(float(cosf(2*M_PI*(j-1)/n+.1)),
                        float(sinf(2*M_PI*(j-1)/n+.1)));
	fltk::addvertex(float(cosf(2*M_PI*i/n+.1)),
                        float(sinf(2*M_PI*i/n+.1)));
	fltk::addvertex(float(cosf(2*M_PI*j/n+.1)),
                        float(sinf(2*M_PI*j/n+.1)));
	strokepath();
      }
    }
    pop_matrix();
  }

public:
  BlinkWindow(int x, int y, int w, int h) : Window(x,y,w,h) {}
} *blinkwindow;

void db_cb(Widget* o) {
  if (((Button*)o)->value()) blinkwindow->set_double_buffer();
  else blinkwindow->clear_double_buffer();
}

bool overlay = false;

void ovl_cb(Widget* o) {
  overlay = ((Button*)o)->value();
  if (overlay) blinkwindow->redraw_overlay();
  else blinkwindow->erase_overlay();
}

void slider_cb(Widget* o) {
  sides = int(((Slider*)o)->value());
  blinkwindow->redraw();
}

void oslider_cb(Widget* o) {
  overlay_sides = int(((Slider*)o)->value());
  if (overlay) blinkwindow->redraw_overlay();
}

int main(int argc, char** argv) {
  if (!visual(DOUBLE_BUFFER))
    printf("Xdbe not supported, faking double buffer with pixmaps.\n");
  Window window(420,420+3*25);
  window.begin();
  blinkwindow = new BlinkWindow(10,10,400,400);
  blinkwindow->tooltip("This graphic is designed to blink in a nasty way "
		       "when redrawing. Try moving the sliders or resizing "
		       "the window.\n\n"
		       "Turn on the double buffer checkmark to hide the "
		       "blinking.");
  int y = 420;
  Slider slider0(60,y,340,22,"sides:");
  slider0.tooltip("Redraw above graphic with this many slides");
  slider0.align(ALIGN_LEFT);
  slider0.type(Slider::TICK_ABOVE);
  slider0.callback(slider_cb);
  y += 25;
  Slider slider1(60,y,340,22,"overlay:");
  slider1.tooltip("Redraw red overlay if the overlay checkmark is turned on.");
  slider1.callback(oslider_cb);
  slider1.align(ALIGN_LEFT);
  slider1.type(Slider::TICK_ABOVE);
  y += 25;
  CheckButton b1(60,y,150,25,"double buffer");
  b1.tooltip("Turns on double-buffer mode in the above window for smooth "
	     "update. Notice that not only does it not blink, it also "
	     "@i;looks@n; faster (even though it really is the same speed "
	     "or slower)");
  b1.callback(db_cb);
  if (window.double_buffer()) b1.set();
  CheckButton b2(60+150,y,150,25,"overlay");
  b2.tooltip("Turns on a red overlay in above window. This is redrawn "
	     "without having to redraw the basic image, so changes to it "
	     "will be faster (probably not noticable on modern computers).\n\n"
	     "Notice that the overlay will blink if the window is resized "
	     "or the graphic is complex. It is designed for simple temporary "
	     "things like mouse selection rectangles.");
  b2.callback(ovl_cb);
  window.end();
  window.resizable(blinkwindow);
  slider0.range(2,30);
  slider0.step(1);
  slider0.value(sides);
  slider1.range(2,30);
  slider1.step(1);
  slider1.value(overlay_sides);
  window.show(argc, argv);
  return run();
}

//
// End of "$Id: doublebuffer.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
