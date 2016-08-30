//
// "$Id: Color.h 6233 2008-09-14 07:54:06Z spitzak $"
//
// Color value. These are 32-bit unsigned numbers with RGB as the
// upper 3 bytes. The lowest-order byte is treated as an "index"
// for back compatabilty, or as an "alpha", depending on context.
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
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#ifndef fltk_Color_h
#define fltk_Color_h

#include "FL_API.h"
#include "Flags.h"

namespace fltk {

/// \name fltk/Color.h
//@{

typedef unsigned Color;

/*! Symbolic names for some of the indexed colors.

  The 24-entry "gray ramp" is modified by fltk::set_background() so
  that the color fltk::GRAY75 is the background color, and the others
  are a nice range from black to a lighter version of the gray. These
  are used to draw box edges. The gray levels are chosen to be evenly
  spaced, listed here is the actual 8-bit and decimal gray level
  assigned by default.  Also listed here is the letter used for
  fltk::FrameBox and the old fltk1.1 names used for these levels.

  The remiander of the colormap is a 5x8x5 color cube. This cube is
  used to dither images on 8-bit screens X colormaps to reduce the
  number of colors used.
*/
enum {
  NO_COLOR	= 0, //!< Black, empty place holder in Style

  FREE_COLOR	= 16, //!< Starting from index 16 is the FREE_COLOR area
  NUM_FREE_COLOR= 16, //!< Number of free color slots starting from index FREE_COLOR

  GRAY00	= 32, //!< hex=00, dec=.00, framebox=A, fltk1 = GRAY0, GRAY_RAMP
  GRAY05	= 33, //!< hex=0d, dec=.05, framebox=B
  GRAY10	= 34, //!< hex=1a, dec=.10, framebox=C
  GRAY15	= 35, //!< hex=27, dec=.15, framebox=D
  GRAY20	= 36, //!< hex=34, dec=.20, framebox=E
  GRAY25	= 37, //!< hex=41, dec=.25, framebox=F
  GRAY30	= 38, //!< hex=4f, dec=.31, framebox=G
  GRAY33	= 39, //!< hex=5c, dec=.36, framebox=H, fltk1 = DARK3
  GRAY35	= 40, //!< hex=69, dec=.41, framebox=I
  GRAY40	= 41, //!< hex=76, dec=.46, framebox=J (18% gray card)
  GRAY45	= 42, //!< hex=83, dec=.51, framebox=K
  GRAY50	= 43, //!< hex=90, dec=.56, framebox=L
  GRAY55	= 44, //!< hex=9e, dec=.62, framebox=M
  GRAY60	= 45, //!< hex=ab, dec=.67, framebox=N, fltk1 = DARK2
  GRAY65	= 46, //!< hex=b8, dec=.72, framebox=O
  GRAY66	= 47, //!< hex=c5, dec=.77, framebox=P, fltk1 = DARK1, INACTIVE_COLOR
  GRAY70	= 48, //!< hex=d2, dec=.82, framebox=Q
  GRAY75	= 49, //!< hex=e0, dec=.88, framebox=R, fltk1 = GRAY, SELECTION_COLOR
  GRAY80	= 50, //!< hex=e5, dec=.90, framebox=S
  GRAY85	= 51, //!< hex=ea, dec=.92, framebox=T, fltk1 = LIGHT1
  //unnamed entry	   hex=ef, dec=.94, framebox=U
  GRAY90	= 53, //!< hex=f4, dec=.96, framebox=V, fltk1 = LIGHT2
  GRAY95	= 54, //!< hex=f9, dec=.98, framebox=W
  GRAY99	= 55, //!< hex=ff, dec=1.0, framebox=X, fltk1 = LIGHT3

  BLACK		= 0x38, //!< Corner of color cube
  RED		= 0x58, //!< Corner of color cube
  GREEN		= 0x3f, //!< Corner of color cube
  YELLOW	= 0x5f, //!< Corner of color cube
  BLUE		= 0xd8, //!< Corner of color cube
  MAGENTA	= 0xf8, //!< Corner of color cube
  CYAN		= 0xdf, //!< Corner of color cube
  WHITE		= 0xff, //!< Corner of color cube

  DARK_RED	= 72,
  DARK_GREEN	= 60,
  DARK_YELLOW	= 76,
  DARK_BLUE	= 136,
  DARK_MAGENTA	= 152,
  DARK_CYAN	= 140,

  WINDOWS_BLUE	= 0x88 //!< default selection_color
};

inline Color color(unsigned char r, unsigned char g, unsigned char b) {
  return Color((r<<24)+(g<<16)+(b<<8)); }
inline Color color(unsigned char g) {
  return Color(g*0x1010100u); }
FL_API Color color(const char*);
FL_API Color parsecolor(const char*, unsigned length);
FL_API Color lerp(Color c0, Color c1, float f);
FL_API Color inactive(Color fg);
FL_API Color inactive(Color fg, Color bg);
FL_API Color contrast(Color fg, Color bg);
FL_API void split_color(Color c, unsigned char& r, unsigned char& g, unsigned char& b);
FL_API void set_color_index(Color index, Color);
FL_API Color get_color_index(Color index);
FL_API void set_background(Color);
FL_API Color nearest_index(Color);

}

//@}

#endif

//
// End of "$Id: Color.h 6233 2008-09-14 07:54:06Z spitzak $".
//
