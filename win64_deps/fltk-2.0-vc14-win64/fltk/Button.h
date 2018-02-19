//
// "$Id: Button.h 8752 2011-05-29 08:44:01Z bgbnbigben $"
//
// Push button widget
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

#ifndef fltk_Button_h
#define fltk_Button_h

#ifndef fltk_Widget_h
#include "Widget.h"
#endif

namespace fltk {

class FL_API Button : public Widget {
public:
  /** Back-compatibility enumeration */
  enum {
    HIDDEN=3 //!< Indicates that the button should be hidden
  };
  bool	value() const { return state(); }
  bool	value(bool v) { return state(v); }

  int handle(int);
  int handle(int event, const Rectangle&);
  Button(int,int,int,int,const char * = 0);
  static NamedStyle* default_style;

  virtual void draw();
  void draw(int glyph_width) const;
};

}

#endif

//
// End of "$Id: Button.h 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
