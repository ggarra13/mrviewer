//
// "$Id: ThumbWheel.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Inventor-style thumbwheel control for a single floating point value.
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

#ifndef fltk_ThumbWheel_h
#define fltk_ThumbWheel_h

#ifndef fltk_Valuator_h
#include "Valuator.h"
#endif

namespace fltk {

class FL_API ThumbWheel : public Valuator {
public:
#ifdef FLTK_1_SLIDER
  // for back-compatability only
  enum {HORIZONTAL = 1};
#endif
  int handle(int);
  ThumbWheel(int X,int Y,int W,int H,const char* L=0);

protected:
  void draw();
};

}

#endif

//
// End of "$Id: ThumbWheel.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
