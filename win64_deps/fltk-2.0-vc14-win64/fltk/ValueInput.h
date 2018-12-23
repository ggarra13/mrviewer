//
// "$Id: ValueInput.h 8636 2011-05-06 08:01:12Z bgbnbigben $"
//
// Text field for inputing a floating-point number
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

#ifndef fltk_ValueInput_h
#define fltk_ValueInput_h

#include "Valuator.h"
#include "FloatInput.h"

namespace fltk {

class FL_API ValueInput : public Valuator {
public:
  FloatInput input;

  int handle(int);
  void draw();
  ValueInput(int x,int y,int w,int h,const char *l=0);
  ~ValueInput();
  static NamedStyle* default_style;

protected:
  void layout();
  virtual void value_damage(); // cause damage() due to value() changing

private:
  static void input_cb(Widget*,void*);
  void increment_cb();
  static void repeat_callback(void* v);
};

}
#endif

//
// End of "$Id: ValueInput.h 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
