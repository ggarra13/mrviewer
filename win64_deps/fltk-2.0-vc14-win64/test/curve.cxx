//
// "$Id: curve.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Curve test program for the Fast Light Tool Kit (FLTK).
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
#include <fltk/ValueSlider.h>
#include <fltk/draw.h>
#include <fltk/ToggleButton.h>

using namespace fltk;

float xy[10] = {
  20,20, 50,200, 100,20, 200,200, 0, 0};
const char* name[10] = {
  "X0", "Y0", "X1", "Y1", "X2", "Y2", "X3", "Y3", "rotate", "line"};

int points;

class Drawing : public Widget {
  void draw() {
    push_clip(0,0,w(),h());
    setcolor(GRAY20);
    fillrect(0,0,w(),h());
    push_matrix();
    if (xy[8]) {
      translate(w()/2, h()/2);
      rotate(xy[8]);
      translate(-(w()/2), -(h()/2));
    }
    if (!points) {
      setcolor(WHITE);
      addcurve(xy[0],xy[1],xy[2],xy[3],xy[4],xy[5],xy[6],xy[7]);
      fillpath();
    }
    setcolor(BLACK);
    addvertex(xy[0],xy[1]);
    addvertex(xy[2],xy[3]);
    addvertex(xy[4],xy[5]);
    addvertex(xy[6],xy[7]);
    strokepath();
    setcolor(points ? WHITE : RED);
    line_style(0,int(xy[9]));
    addcurve(xy[0],xy[1],xy[2],xy[3],xy[4],xy[5],xy[6],xy[7]);
    /*points ? drawpoints() :*/ strokepath();
    line_style(0);
    pop_matrix();
    pop_clip();
  }
public:
  Drawing(int X,int Y,int W,int H) : Widget(X,Y,W,H) {}
};

Drawing *d;

void points_cb(Widget* o, void*) {
  points = ((ToggleButton*)o)->value();
  d->redraw();
}

void slider_cb(Widget* o, void* v) {
  Slider* s = (Slider*)o;
  xy[long(v)] = s->value();
  d->redraw();
}

int main(int argc, char** argv) {
  Window window(300,555+25,"addcurve() test");
  window.begin();
  window.set_double_buffer();
  Drawing drawing(10,10,280,280);
  d = &drawing;

  int y = 300;
  for (int n = 0; n<10; n++) {
    ValueSlider* s = new ValueSlider(50,y,240,25,name[n]); y += 25;
    s->minimum(0); s->maximum(280);
    if (n == 8) s->maximum(360);
    s->step(1);
    s->value(xy[n]);
    s->align(ALIGN_LEFT);
    s->callback(slider_cb, (void*)n);
  }
//   ToggleButton but(50,y,50,25,"points");
//   but.callback(points_cb);

  window.end();
  window.show(argc,argv);
  return run();
}

//
// End of "$Id: curve.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
