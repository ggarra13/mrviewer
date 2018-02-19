//
// "$Id: xbmImage.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Image subclass for in-memory xbm data (you #include the .xbm file
// and then construct this).
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

#ifndef fltk_xbmImage_h
#define fltk_xbmImage_h

#include "Image.h"

namespace fltk {

class FL_API xbmImage : public Image {
public:
  const unsigned char *array;
  xbmImage(const unsigned char *bits, int w,int h) :
    Image(fltk::MASK,w,h), array(bits) {}
  xbmImage(const char *bits, int w, int h) :
    Image(fltk::MASK,w,h), array((const unsigned char *)bits) {}
  bool fetch();
};

}

#endif

//
// End of "$Id: xbmImage.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
