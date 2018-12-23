//
// "$Id: ThumbWheel.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// SGI Inventor / Rapid-App style knob

#include <fltk/ThumbWheel.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/math.h>
using namespace fltk;

#define horizontal() (!flag(LAYOUT_VERTICAL)||(type()&1))

/*! \class fltk::ThumbWheel

  A control similar to SGI's Inventor toolkit, commonly used to move 3D
  objects or control zoom.

  \image html Fl_Roller.gif

  You can use set_vertical() to make the thumbwheel move vertically.
*/

int ThumbWheel::handle(int event) {
  static int ipos;
  int newpos = horizontal() ? event_x() : -event_y();
  switch (event) {
  case PUSH:
    handle_push();
    ipos = newpos;
    return 1;
  case DRAG:
    handle_drag(previous_value()+(newpos-ipos)*(step()?step():.01));
    return 1;
  case RELEASE:
    handle_release();
    redraw();
    return 1;
  case KEY:
    // Only arrows in the correct direction are used.  This allows the
    // opposite arrows to be used to navigate between a set of parellel
    // rollers.
    switch (event_key()) {
    case UpKey:
    case DownKey:
    case HomeKey:
    case EndKey:
      if (horizontal()) return 0;
      break;
    case LeftKey:
    case RightKey:
      if (!horizontal()) return 0;
    } // else fall through...
  default:
    return Valuator::handle(event);
  }
}

void ThumbWheel::draw() {
  drawstyle(style(), flags()|OUTPUT);
  const Color fg = getcolor();

  if (damage()&(DAMAGE_ALL|DAMAGE_HIGHLIGHT)) draw_box();
  Rectangle r(w(),h());
  box()->inset(r);
  if (r.empty()) return;

  double s = step();
  if (!s) s = (maximum()-minimum())/100;
  int offset = int(value()/s);

  const double ARC = 1.5; // 1/2 the number of radians visible
  const double delta = .2; // radians per knurl
  if (horizontal()) {
    // draw shaded ends of wheel:
    int h1 = r.w()/4+1; // distance from end that shading starts
    setcolor(getbgcolor()); fillrect(Rectangle(r.x()+h1,r.y(),r.w()-2*h1,r.h()));
    for (int i=0; h1; i++) {
      setcolor((Color)(GRAY75-i-1));
      int h2 = GRAY75-i-1 > GRAY33 ? 2*h1/3+1 : 0;
      fillrect(r.x()+h2,r.y(),h1-h2,r.h());
      fillrect(r.r()-h1,r.y(),h1-h2,r.h());
      h1 = h2;
    }
    if (!(flags()&INACTIVE_R)) {
      // draw ridges:
      double junk;
      for (double y = -ARC+modf(offset*sin(ARC)/(r.w()/2)/delta,&junk)*delta;;
	   y += delta) {
	int y1 = int((sin(y)/sin(ARC)+1)*r.w()/2);
	if (y1 <= 0) continue; else if (y1 >= r.w()-1) break;
	setcolor(GRAY33); drawline(r.x()+y1,r.y()+1,r.x()+y1,r.b()-1);
	if (y < 0) y1--; else y1++;
	setcolor(GRAY85);drawline(r.x()+y1,r.y()+1,r.x()+y1,r.b()-1);
      }
      // draw edges:
      h1 = r.w()/8+1; // distance from end the color inverts
      setcolor(GRAY60);
      drawline(r.x()+h1,r.b()-1,r.r()-h1,r.b()-1);
      setcolor(GRAY33);
      drawline(r.x(),r.b(),r.x(),r.y());
      drawline(r.x(),r.y(),r.x()+h1,r.y());
      drawline(r.r()-h1,r.y(),r.r(),r.y());
      setcolor(GRAY90);
      drawline(r.x()+h1,r.y(),r.r()-h1,r.y());
      drawline(r.r(),r.y(),r.r(),r.b());
      drawline(r.r(),r.b(),r.r()-h1,r.b());
      drawline(r.x()+h1,r.b(),r.x(),r.b());
    }
  } else { // vertical one
    offset = (1-offset);
    // draw shaded ends of wheel:
    int h1 = r.h()/4+1; // distance from end that shading starts
    setcolor(getbgcolor()); fillrect(r.x(),r.y()+h1,r.w(),r.h()-2*h1);
    for (int i=0; h1; i++) {
      setcolor((Color)(GRAY75-i-1));
      int h2 = GRAY75-i-1 > GRAY33 ? 2*h1/3+1 : 0;
      fillrect(r.x(),r.y()+h2,r.w(),h1-h2);
      fillrect(r.x(),r.b()-h1,r.w(),h1-h2);
      h1 = h2;
    }
    if (!(flags()&INACTIVE_R)) {
      // draw ridges:
      double junk;
      for (double y = -ARC+modf(offset*sin(ARC)/(r.h()/2)/delta,&junk)*delta;
	   ; y += delta) {
	int y1 = int((sin(y)/sin(ARC)+1)*r.h()/2);
	if (y1 <= 0) continue; else if (y1 >= r.h()-1) break;
	setcolor(GRAY33); drawline(r.x()+1,r.y()+y1,r.r()-1,r.y()+y1);
	if (y < 0) y1--; else y1++;
	setcolor(GRAY85);drawline(r.x()+1,r.y()+y1,r.r()-1,r.y()+y1);
      }
      // draw edges:
      h1 = r.h()/8+1; // distance from end the color inverts
      setcolor(GRAY60);
      drawline(r.r()-1,r.y()+h1,r.r()-1,r.b()-h1);
      setcolor(GRAY33);
      drawline(r.r(),r.y(),r.x(),r.y());
      drawline(r.x(),r.y(),r.x(),r.y()+h1);
      drawline(r.x(),r.b()-h1,r.x(),r.b());
      setcolor(GRAY90);
      drawline(r.x(),r.y()+h1,r.x(),r.b()-h1);
      drawline(r.x(),r.b(),r.r(),r.b());
      drawline(r.r(),r.b(),r.r(),r.b()-h1);
      drawline(r.r(),r.y()+h1,r.r(),r.y());
    }
  }
  setcolor(fg);
  box()->draw_symbol_overlay(Rectangle(w(),h()));
}

ThumbWheel::ThumbWheel(int X,int Y,int W,int H,const char* L) : Valuator(X,Y,W,H,L) {
  step(.001);
  //set_vertical();
  //set_click_to_focus();
}

//
// End of "$Id: ThumbWheel.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
