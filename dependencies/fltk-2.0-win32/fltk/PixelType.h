// "$Id: PixelType.h 1399 2006-08-11 02:15:20Z spitzak $"
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

#ifndef fltk_PixelType_h
#define fltk_PixelType_h

namespace fltk {

/// \name fltk/PixelType.h
//@{

/**
  Enumeration describing how colors are stored in an array of bytes
  that is a pixel. This is used as an argument for fltk::drawimage(),
  fltk::readimage(), and fltk::Image.

  Notice that the order of the bytes in memory of ARGB32 or RGB32 is
  a,r,g,b on a little-endian machine and b,g,r,a on a big-endian
  machine. Due to the use of these types by Windows, this is often
  the fastest form of data, if you have a choice. To convert an
  fltk::Color to RGB32, shift it right by 8 (for ARGB32 shift the
  alpha left 24 and or it in).

  More types may be added in the future. The set is as minimal as
  possible while still covering the types I have actually encountered.
*/
enum PixelType {
  MASK	= 0,	//!< 1 byte of inverted mask, filled with current color
  MONO	= 1,	//!< 1 byte of gray scale
  RGBx	= 2,	//!< bytes in r,g,b,a,r,g,b,a... order, a byte is ignored
  RGB	= 3, 	//!< bytes in r,g,b,r,g,b... order
  RGBA	= 4,	//!< bytes in r,g,b,a,r,g,b,a... order
  RGB32 = 5,	//!< 32-bit words containiing 0xaarrggbb (aa is ignored)
  ARGB32= 6,	//!< 32-bit words containing 0xaarrggbb
  // unpremulitplied is not yet implemented, acts like RGBA/ARGB32:
  RGBM	= 7,	//!< unpremultiplied bytes in r,g,b,a order
  MRGB32= 8	//!< unpremultiplied 0xaarrggbb
};

/**
   Turn a PixelType into the number of bytes needed to hold a pixel.
*/
inline int depth(PixelType t) {return (t<2 ? 1 : t==3 ? 3 : 4);}

//@}

}

#endif
