// "$Id: xbmImage.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/*! \class fltk::xbmImage

  Image based on a 1-bit bitmap from memory. This matches the very
  first type of image provided with X10 in 1980 or so. To use an
  actual xbm file:

\code
#include "image_name.xbm"
static xbmImage image_name(image_name_bits, image_name_width, image_name_height);
\endcode

  Each byte in the bitmap determines 8 pixels, a 1 bit is opaque and
  a 0 bit is transparent. The low-order bit is the left-most (this
  is inverted from similar data used by Windows). If the width is
  not a multiple of 8 each line starts at the start of the next byte.
*/

#include <fltk/xbmImage.h>

using namespace fltk;

bool xbmImage::fetch() {
  // See the x11/Image.cxx for some code that is faster if the scale
  // is 1:1 or XRender is not being used. I ignore this as it is only
  // for X11 and it complicates the Image by adding xbmImage as a
  // friend.
  int rowBytes = (w()+7)>>3 ;
  for (int y=0; y < h(); y++) {
    uchar* to = linebuffer(y);
    uchar* from = (uchar*)array+y*rowBytes;
    for (int x=0; x<w(); x++)
      to[x] = from[x>>3]&(1<<(x&7)) ? 0 : 255;
    setpixels(to,y);
  }
  return true;
}

//
// End of "$Id: xbmImage.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
