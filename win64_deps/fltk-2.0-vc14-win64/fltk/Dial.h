//
// "$Id: Dial.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Rotating value control
//
// Copyright 2002 by Bill Spitzak and others.
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

#ifndef fltk_Dial_h
#define fltk_Dial_h

#ifndef fltk_Valuator_h
#include "Valuator.h"
#endif

namespace fltk {

class FL_API Dial : public Valuator {

public:

  enum {NORMAL = 0, LINE, FILL}; // values for type()
  int handle(int);
  Dial(int x,int y,int w,int h, const char *l = 0);
  static NamedStyle* default_style;
  short angle1() const {return a1;}
  void angle1(short a) {a1 = a;}
  short angle2() const {return a2;}
  void angle2(short a) {a2 = a;}
  void angles(short a, short b) {a1 = a; a2 = b;}

protected:

  void draw();

private:

  short a1,a2;

};

}
#endif

//
// End of "$Id: Dial.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
