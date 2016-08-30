//
// "$Id: mandelbrot.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Mandelbrot set demo for the Fast Light Tool Kit (FLTK).
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

#include <stdio.h>
#include <stdlib.h>

#include <fltk/run.h>
#include <fltk/error.h>
#include <fltk/draw.h>
#include <fltk/visual.h>
#include <fltk/events.h>
#include <fltk/Widget.h>
#include "mandelbrot_ui.h"

DrawingWindow mbrot;
DrawingWindow jbrot;

void idle() {
  if (!mbrot.d->idle() && !(jbrot.d && jbrot.d->idle())) fltk::set_idle(0);
}

void set_idle() {
  fltk::set_idle(idle);
}

static void window_callback(fltk::Widget*, void*) {
  exit(0);
}

int main(int argc, char **argv) {
  mbrot.make_window();
  mbrot.window->clear_double_buffer(); // otherwise make_current does not work
  mbrot.d->X = -.75;
  mbrot.d->scale = 2.5;
  mbrot.update_label();
  int i = 0;
  if (fltk::args(argc, argv, i) < argc)
    fltk::fatal(fltk::help);
  fltk::visual(fltk::RGB);
  mbrot.window->callback(window_callback);
  mbrot.window->show(argc, argv);
  return fltk::run();
}

void DrawingWindow::update_label() {
  char buffer[128];
  sprintf(buffer, "%+.10f", d->X); x_input->text(buffer);
  sprintf(buffer, "%+.10f", d->Y); y_input->text(buffer);
  sprintf(buffer, "%.2g", d->scale); w_input->text(buffer);
}

void DrawingArea::draw() {
  draw_box();
  drawn = 0;
  set_idle();
}

int DrawingArea::idle() {
  if (!window()->visible()) return 0;
  if (drawn < nextline) {
    make_current(); // deleted window-> from this for fltk2.0
    int yy = drawn + 4;
    if (yy >= sy && yy <= sy + sh) erase_box();
    fltk::drawimage(buffer + drawn*W, fltk::MONO, fltk::Rectangle(3, yy, W, 1), W);
    //fltk::draw_image_mono(buffer+drawn*W, 3,yy,W,1,1,W);
    drawn++;
    return 1;
  }
  if (nextline < H) {
    if (!buffer) buffer = new uchar[W*H];
    double yy = Y + (H / 2 - nextline) * scale / W;
    double yi = yy; if (julia) yy = jY;
    uchar *p = buffer + nextline * W;
    for (int xi = 0; xi < W; xi++) {
      double xx = X + (xi - W / 2) * scale / W;
      double wx = xx; double wy = yi;
      if (julia) xx = jX;
      for (int i = 0; ; i++) {
        if (i >= iterations) {
          *p = 0; break;
        }
        double t = wx * wx - wy * wy + xx;
        wy = 2 * wx * wy + yy;
        wx = t;
        if (wx*wx + wy*wy > 4) {
          wx = t = 1 - double(i) / (1 << 10);
          if (t <= 0) t = 0; else for (i = brightness; i--;) t *= wx;
          *p = 255 - int(254 * t);
          break;
        }
      }
      p++;
    }
    nextline++;
    return nextline < H;
  }
  return 0;
}

void DrawingArea::erase_box() {
  make_current();
  fltk::overlay_clear();
}

// For fltk2.0, all the occurances of x() and y() were replaced by 0:

int DrawingArea::handle(int event) {
  static int ix, iy;
  static int dragged;
  static int button;
  int x2, y2;
  switch (event) {
    case fltk::PUSH:
      erase_box();
      ix = fltk::event_x(); if (ix < 0) ix = 0; if (ix >= w()) ix = w() - 1;
      iy = fltk::event_y(); if (iy < 0) iy = 0; if (iy >= h()) iy = h() - 1;
      dragged = 0;
      button = fltk::event_button();
      return 1;
    case fltk::DRAG:
      dragged = 1;
      erase_box();
      x2 = fltk::event_x(); if (x2 < 0) x2 = 0; if (x2 >= w()) x2 = w() - 1;
      y2 = fltk::event_y(); if (y2 < 0) y2 = 0; if (y2 >= h()) y2 = h() - 1;
      if (button != 1) {
        ix = x2; iy = y2; return 1;
      }
      if (ix < x2) {
        sx = ix; sw = x2 - ix;
      } else {
        sx = x2; sw = ix - x2;
      }
      if (iy < y2) {
        sy = iy; sh = y2 - iy;
      } else {
        sy = y2; sh = iy - y2;
      }
      make_current();
      fltk::overlay_rect(sx, sy, sw, sh);
      return 1;
    case fltk::RELEASE:
      if (button == 1) {
        erase_box();
        if (dragged && sw > 3 && sh > 3) {
          X = X + (sx + sw / 2 - W / 2) * scale / W;
          Y = Y + (-sy - sh / 2 + H / 2) * scale / W;
          scale = sw * scale / W;
        } else if (!dragged) {
          scale = 2 * scale;
          if (julia) {
            if (scale >= 4) {
              scale = 4;
              X = Y = 0;
            }
          } else {
            if (scale >= 2.5) {
              scale = 2.5;
              X = -.75;
              Y = 0;
            }
          }
        } else return 1;
        ((DrawingWindow*)(user_data()))->update_label();
        new_display();
      } else if (!julia) {
        if (!jbrot.d) {
          jbrot.make_window();
          jbrot.window->clear_double_buffer(); // otherwise make_current does not work
          jbrot.d->julia = 1;
          jbrot.d->X = 0;
          jbrot.d->Y = 0;
          jbrot.d->scale = 4;
          jbrot.update_label();
        }
        jbrot.d->jX = X + (ix - W / 2) * scale / W;
        jbrot.d->jY = Y + (H / 2 - iy) * scale / W;
        static char s[128];
        sprintf(s, "Julia %.7f %.7f", jbrot.d->jX, jbrot.d->jY);
        jbrot.window->label(s);
        jbrot.window->show();
        jbrot.d->new_display();
      }
      return 1;
    default:
      return Widget::handle(event);
  }
}

void DrawingArea::new_display() {
  drawn = nextline = 0;
  set_idle();
}

#include <fltk/layout.h> // added for fltk2.0

void DrawingArea::layout() {
  if (layout_damage() & fltk::LAYOUT_WH) {
    W = w() - 6;
    H = h() - 8;
    if (buffer) {
      delete[] buffer; buffer = 0; new_display();
    }
  }
  Widget::layout();
}

//
// End of "$Id: mandelbrot.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
