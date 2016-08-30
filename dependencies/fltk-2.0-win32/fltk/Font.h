//
// "$Id: Font.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Copyright 2004 by Bill Spitzak and others.
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

#ifndef fltk_Font_h
#define fltk_Font_h

#include "Style.h"

namespace fltk {

enum {  // values for attributes:
  BOLD = 1,
  ITALIC = 2,
  BOLD_ITALIC = 3
};

struct FL_API Font {
  const char* name_;
  int attributes_;
  // other fields are added here!

  const char* name() const;

  const char* name(int* p) {*p = attributes_; return name_;}

  Font* plus(int attributes);
  Font* bold() {return plus(BOLD);}
  Font* italic() {return plus(ITALIC);}

  int sizes(int*&);

  int encodings(const char**&);

  const char* system_name();

  static const char* current_name();

};

// Find a Font from a name and attributes:
FL_API Font* font(const char* name, int attrib = 0);

// Find a Font from an fltk1 integer font id:
FL_API Font* font(int);

// Find and return every font on the system.
FL_API int list_fonts(Font**& arrayp);

}

#endif
