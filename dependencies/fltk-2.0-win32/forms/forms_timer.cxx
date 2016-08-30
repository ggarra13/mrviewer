//
// "$Id: forms_timer.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Forms timer object for the Fast Light Tool Kit (FLTK).
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

// Emulate the Forms Timer object
// You don't want to use this if you just want a timeout, call
// Fl::add_timeout directly!

#include <fltk/Fl.h>
#include <fltk/Fl_Timer.h>
#include <fltk/fl_draw.h>
#ifdef _WIN32
# include <sys/types.h> 
# include <sys/timeb.h>
#else
# include <sys/time.h>
#endif
#include <stdio.h>

#define FL_TIMER_BLINKRATE	0.2

void fl_gettime(long* sec, long* usec) {
#ifdef _WIN32
  struct timeb tp;
  ftime(&tp);
  *sec = tp.time;
  *usec = tp.millitm * 1000;
#else
  struct timeval tp;
  struct timezone tzp;
  gettimeofday(&tp, &tzp);
  *sec = tp.tv_sec;
  *usec = tp.tv_usec;
#endif
}

void Fl_Timer::draw() {
  int tt;
  char str[32];
  Fl_Color color;
  if (on && delay<=0.0 && ((int) (delay / FL_TIMER_BLINKRATE) % 2))
    color = selection_color();
  else
    color = this->color();
  box()->draw(0, 0, w(), h(), color);
  if (type() == FL_VALUE_TIMER && delay>0.0) {
    double d = direction_ ? total-delay : delay;
    if (d < 60.0)
      sprintf(str, "%.1f", d);
    else {
      tt = (int) ((d+0.05) / 60.0);
      sprintf(str, "%d:%04.1f", tt, d - 60.0 * tt);
    }
    fl_font(label_font(), label_size());
    fl_color(label_color());
    fl_draw(str, 0, 0, w(), h(), FL_ALIGN_CENTER);
  } else
    draw_inside_label();
}

void Fl_Timer::stepcb(void* v) {
  ((Fl_Timer*)v)->step();
}

void Fl_Timer::step() {
  if (!on) return;
  double lastdelay = delay;
  long sec, usec; fl_gettime(&sec, &usec);
  delay -= (double) (sec - lastsec) + (double) (usec - lastusec) / 1000000.0;
  lastsec = sec; lastusec = usec;
  if (lastdelay > 0.0 && delay <= 0.0) {
    if (type() == FL_HIDDEN_TIMER) {
      on = 0;
      delay = 0;
    } else {
      redraw();
      Fl::add_timeout(FL_TIMER_BLINKRATE, (Fl_Timeout_Handler)stepcb, this);
    }
    do_callback();
  } else {
    if (type() == FL_VALUE_TIMER) redraw();
    Fl::add_timeout(FL_TIMER_BLINKRATE, (Fl_Timeout_Handler)stepcb, this);
  }
}

int Fl_Timer::handle(int event) {
  if (event == FL_RELEASE && delay <= 0) value(0.0);
  return 0;
}

Fl_Timer::~Fl_Timer() {
  Fl::remove_timeout((Fl_Timeout_Handler)stepcb, this);
}

Fl_Timer::Fl_Timer(uchar t, int x, int y, int w, int h, const char* l)
: Fl_Widget(x, y, w, h, l) {
  box(FL_DOWN_BOX);
  selection_color(FL_RED);
  delay = 0;
  on = 0;
  direction_ = 0;
  type(t);
  if (t == FL_HIDDEN_TIMER) clear_visible();
  if (t == FL_VALUE_TIMER) {
	  clear_flag(FL_ALIGN_MASK);
	  set_flag(FL_ALIGN_LEFT);
  }
}

void Fl_Timer::value(double d) {
  delay = total = d;
  on = (d > 0.0);
  fl_gettime(&(lastsec), &(lastusec));
  if (type() != FL_HIDDEN_TIMER) redraw();
  Fl::remove_timeout((Fl_Timeout_Handler)stepcb, this);
  if (on) Fl::add_timeout(FL_TIMER_BLINKRATE, (Fl_Timeout_Handler)stepcb, this);
}

void Fl_Timer::suspended(char d) {
  if (!d) {
    if (on) return;
    on = (delay > 0.0);
    fl_gettime(&(lastsec), &(lastusec));
    if (on) Fl::add_timeout(FL_TIMER_BLINKRATE, (Fl_Timeout_Handler)stepcb, this);
  } else {
    if (!on) return;
    on = 0;
    Fl::remove_timeout((Fl_Timeout_Handler)stepcb, this);
  }
}

//
// End of "$Id: forms_timer.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
