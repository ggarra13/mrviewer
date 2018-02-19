// "$Id: Valuator.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#ifndef fltk_Valuator_h
#define fltk_Valuator_h

#ifndef fltk_Widget_h
#include "Widget.h"
#endif

namespace fltk {

class FL_API Valuator : public Widget {

public:

  double value() const {return value_;}
  bool value(double);

  double minimum() const {return minimum_;}
  void minimum(double a) {minimum_ = a;}

  double maximum() const {return maximum_;}
  void maximum(double a) {maximum_ = a;}

  void range(double a, double b) {minimum_ = a; maximum_ = b;}

  double step() const {return step_;}
  void step(double a) {step_ = a;}

  double linesize() const;
  void linesize(double a) {linesize_ = a;}
  double linesize_setting() const {return linesize_;}

  virtual int format(char*);

  int handle(int);

#ifdef FLTK_1_SLIDER
  void step(double a, int b) {step(a/b);}
  void bounds(double a, double b) {minimum_=a; maximum_=b;}
  void precision(int p) {
    int B = 1;
    for (int i=0; i<p; i++) B *= 10;
    step_ = 1.0f/B;
  }
#endif

//protected:

  Valuator(int X, int Y, int W, int H, const char* L);
  double previous_value() const {return previous_value_;}
  void handle_push() {previous_value_ = value_;}
  void handle_drag(double newvalue);
  void handle_release();

  virtual void value_damage(); // callback whenever value changes
  void set_value(double v) {value_ = v;} // change w/o doing value_damage

private:

  double value_;
  double previous_value_;
  double minimum_;
  double maximum_;
  double step_;
  double linesize_;

};

}

#endif

// End of "$Id: Valuator.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
