//
// "$Id: Dial.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Circular dial widget for the Fast Light Tool Kit (FLTK).
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

#include <fltk/Dial.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <stdlib.h>
#include <fltk/math.h>
using namespace fltk;

/*! \class fltk::Dial

  The fltk::Dial widget provides a circular dial to control a single
  floating point value.

  \image html Fl_Dial.gif
  Use type() to change how it draws:
  - Dial::NORMAL : Draws a normal dial with a knob.
  - Dial::LINE : Draws a dial with a line.
  - Dial::FILL : Draws a dial with a filled arc.

  You can change the box() from the default of OVAL_BOX to draw
  different borders. The box() is filled with color(), the moving part
  is filled with selection_color(), and the border around the movint
  part is set by textcolor().
*/

/*! \fn void Dial::angle1(short a)
  See angles()
*/
/*! \fn void Dial::angle2(short a)
  See angles()
*/

/*! \fn void Dial::angles(short a, short b)
  Sets the angles used for the minimum and maximum values. The default
  values are 45 and 315 (0 degrees is straight down and the angles
  progress clockwise). Normally angle1 is less than angle2, but if you
  reverse them the dial moves counter-clockwise.
*/

void Dial::draw() {
  Rectangle r(w(),h());
  if (type()!=FILL || box() != OVAL_BOX) {
    if (damage()&DAMAGE_ALL) draw_box();
    box()->inset(r);
  }
  drawstyle(style(), flags());
  Color fillcolor = selection_color();
  Color linecolor = textcolor();
  if (flag(INACTIVE_R)) {
    fillcolor = inactive(fillcolor);
    linecolor = inactive(linecolor);
  }
  float angle = (a2-a1)*float((value()-minimum())/(maximum()-minimum())) + a1;
  if (type() == FILL) {
    if (damage()&DAMAGE_EXPOSE && box() == OVAL_BOX) draw_background();
    setcolor(getbgcolor());
    addpie(r, float(270-a1), float(angle > a1 ? 360+270-angle : 270-360-angle));
    fillpath();
    setcolor(fillcolor);
    addpie(r, float(270-angle), float(270-a1));
    fillpath();
    if (box() == OVAL_BOX) {
      setcolor(linecolor);
      addchord(r, 0, 360);
      strokepath();
    }
  } else {
    if (!(damage()&DAMAGE_ALL)) {
      // erase interior without erasing any box edge:
      Rectangle r1(r); r1.inset(1);
      setcolor(getbgcolor());
      addchord(r1, 0, 360);
      fillpath();
    }
    push_matrix();
    translate(r.center_x()-.5f, r.center_y()-.5f);
    scale(float(r.w()-1), float(r.h()-1));
    rotate(45-angle);
    setcolor(fillcolor);
    if (type() == LINE) {
      static float v[4][2] = {{0,0}, {-0.04f,0}, {-0.25f,0.25f}, {0,0.04f}};
      addvertices(4,v);
    } else {
      // get the nice circle code:
      translate(-0.20f, 0.20f);
      scale(.075f);
      addchord(Rectangle(-1,-1,2,2),0,360);
    }
    fillstrokepath(linecolor);
    pop_matrix();
  }
  if (focused()) {
    r.inset(2);
    setcolor(linecolor);
    line_style(DASH);
    addchord(r,0,360);
    strokepath();
    line_style(0);
  }
}


int Dial::handle(int event) {
  switch (event) {
  case PUSH:
    handle_push();
  case DRAG: {
    Rectangle r(w(),h());
    box()->inset(r);
    int mx = event_x()-r.center_x();
    int my = event_y()-r.center_y();
    if (!mx && !my) return 1;
    float angle = 270-atan2f((float)-my, (float)mx)*float(180/M_PI);
    float oldangle = (a2-a1)*float((value()-minimum())/(maximum()-minimum())) + a1;
    while (angle < oldangle-180) angle += 360;
    while (angle > oldangle+180) angle -= 360;
    double val;
    if ((a1<a2) ? (angle <= a1) : (angle >= a1)) {
      val = minimum();
    } else if ((a1<a2) ? (angle >= a2) : (angle <= a2)) {
      val = maximum();
    } else {
      val = minimum() + (maximum()-minimum())*(angle-a1)/(a2-a1);
    }
    handle_drag(val);
  } return 1;
  case RELEASE:
    if (!pushed()) handle_release();
    return 1;
  default:
    return Valuator::handle(event);
  }
}

static void revert(Style* s) {
  s->box_ = OVAL_BOX;
  s->selection_color_ = GRAY60;
}
static NamedStyle style("Dial", revert, &Dial::default_style);
NamedStyle* Dial::default_style = &::style;

Dial::Dial(int x, int y, int w, int h, const char* l)
  : Valuator(x, y, w, h, l) {
  style(default_style);
  a1 = 45;
  a2 = 315;
  //set_click_to_focus();
}

//
// End of "$Id: Dial.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
