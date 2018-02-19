//
// "$Id: arc.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Arc drawing test program for the Fast Light Tool Kit (FLTK).
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

#include <config.h> // for  USE_CAIRO definition

#include <fltk/x.h>

#include <fltk/run.h>
#include <fltk/DoubleBufferWindow.h>
#include <fltk/ValueSlider.h>
#include <fltk/draw.h>

float dargs[7] = {90, 90, 100, 100, 0, 360, 0};
const char* name[7] = {"X", "Y", "W", "H", "start", "end", "rotate"};

using namespace fltk;

class Drawing : public Widget {
  void draw() {
    push_clip(0,0, w(), h());
    setcolor_alpha(BLUE,0.6);
    fillrect(0,0,w(), h());
    push_matrix();
    //    if (dargs[6]) {
      translate(w()/2.0f, h()/2.0f);
      rotate(dargs[6]);
      translate(-w()/2.0f, -h()/2.0f);
      //}
    addarc(dargs[0],dargs[1],dargs[2],dargs[3],dargs[4],dargs[5]);
    closepath();
    addarc(120,120,40,40,0,-360);
    setcolor_alpha(GRAY33,0.6);
    fillstrokepath(WHITE);
    // draw a hardware circle to see how well rotations match:
    setcolor(GRAY33);
    fltk::Rectangle r(20,20,(int)(dargs[2]+1),(int)(dargs[3]+1));
    addchord(r,dargs[4],dargs[5]);
    fillstrokepath(WHITE);
    // now draw non-rotated hardware circle to check if it inscribes:
    pop_matrix();
    setcolor_alpha(GRAY40,0.6);
    r.set(10,(int) (270-dargs[3]),(int) dargs[2],(int)dargs[3]);
    fillrect(r);
    setcolor(GRAY90);
    strokerect(r);
    setcolor_alpha(RED,0.6);
    addchord(r,dargs[4],dargs[5]);
    fillstrokepath(GRAY90);
    pop_clip();
  }
public:
  Drawing(int X,int Y,int W,int H) : Widget(X,Y,W,H) {}
};

Drawing *d;

void slider_cb(Widget* o, void* v) {
  Slider* s = (Slider*)o;
  dargs[long(v)] = s->value();
  d->redraw();
}

int main(int argc, char** argv) {
  Window window(300,500);
  window.begin();
  Drawing drawing(10,10,280,280);
  d = &drawing;

  int y = 300;
  for (int n = 0; n<7; n++) {
    Slider* s =
      new ValueSlider(50,y,240,25,name[n]); y += 25;
    if (n<4) {s->minimum(0); s->maximum(300);}
    else if (n==6) {s->minimum(0); s->maximum(360);}
    else {s->minimum(-360); s->maximum(360);}
    s->type(Slider::TICK_ABOVE);
    s->step(1);
    s->value(dargs[n]);
	s->clear_flag(ALIGN_MASK);
    s->set_flag(ALIGN_LEFT);
    s->callback(slider_cb, (void*)n);
  }

  window.end();
  window.resizable(drawing);
  window.show(argc,argv);
  return run();
}


//
// End of "$Id: arc.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
