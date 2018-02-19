//
// "$Id: NumericInput.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// One-line text input field, which handles up/down arrows to
// change the digit to the right of the cursor. This still allows
// arbitrary text such as a math expression to be typed, if you
// want to restrict the user to a number use FloatInput or IntInput.
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

#ifndef fltk_NumericInput_h
#define fltk_NumericInput_h

#include "Input.h"

namespace fltk {

class FL_API NumericInput : public Input {
 protected:
  int handle_arrow(int);
 public:
  NumericInput(int x,int y, int w,int h,const char* l = 0) :
    Input(x,y,w,h,l) {when(WHEN_ENTER_KEY|WHEN_RELEASE);}
  void value(double);
  void value(int);
  int handle(int);
};

}
#endif

//
// End of "$Id: NumericInput.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
