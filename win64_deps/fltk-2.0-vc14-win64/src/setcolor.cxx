//
// "$Id: setcolor.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/Color.h>
#include <fltk/draw.h>
#include <config.h>
#if defined(_WIN32) && USE_STOCK_BRUSH && _WIN32_WINNT<0x0500
# undef _WIN32_WINNT
# define _WIN32_WINNT 0x0500
#endif
#include <fltk/x.h>

using namespace fltk;

/*! \typedef fltk::Color

  fltk::Color is a typedef for a 32-bit integer containing r,g,b bytes
  and an "index" in the lowest byte (the \e first byte on a
  little-endian machine such as an x86).  For instance 0xFF008000 is
  255 red, zero green, and 128 blue. If rgb are not zero then the low
  byte is ignored, or may be treated as "alpha" by some code.

  If the rgb is zero, the N is the color "index". This index is used
  to look up an fltk::Color in an internal table of 255 colors shown
  here.  All the indexed colors may be changed by using
  set_color_index().  However fltk uses the ones between 32 and 255
  and assummes they are not changed from their default values.

  \image html fl_show_colormap.gif
  (this is \e not the X colormap used by fltk)

  A Color of zero (fltk::NO_COLOR) will draw black but is
  ambiguous. It is returned as an error value or to indicate portions
  of a Style that should be inherited, and it is also used as the
  default label color for everything so that changing color zero can
  be used by the -fg switch. You should use fltk::BLACK (56) to get
  black.

*/

static unsigned cmap[256] = {
#include "colormap.h" // this is a file produced by "colormap.cxx":
};

/*! Set r,g,b to the 8-bit components of this color. If it is an indexed
  color they are looked up in the table, otherwise they are simply copied
  out of the color number. */
void fltk::split_color(Color i,
		       unsigned char& r,
		       unsigned char& g,
		       unsigned char& b) {
  if (!(i & 0xFFFFFF00)) i = Color(cmap[i]);
  r = uchar(i>>24);
  g = uchar(i>>16);
  b = uchar(i>>8);
}

/*! Find an indexed color in the range 56-127 that is closest to this
  color. If this is an indexed color it is returned unchanged. */
Color fltk::nearest_index(Color i) {
  if (!(i & 0xFFFFFF00)) return i;
  uchar r = i>>24;
  uchar g = i>>16;
  uchar b = i>> 8;
  //if (r == g && r == b) return gray_ramp(r*NUM_GRAY/256);
  return Color(BLACK + (b*5/256 * 5 + r*5/256) * 8 + g*8/256);
}

/*! Return (1-weight)*color0 + weight*color1. \a weight is clamped
  to the 0-1 range before use. */
Color fltk::lerp(Color color0, Color color1, float weight) {
  if (weight <= 0) return color0;
  if (weight >= 1) return color1;
  Color rgb0 = get_color_index(color0);
  Color rgb1 = get_color_index(color1);
  return color(
	(uchar)(((uchar)(rgb1>>24))*weight + ((uchar)(rgb0>>24))*(1-weight)),
	(uchar)(((uchar)(rgb1>>16))*weight + ((uchar)(rgb0>>16))*(1-weight)),
	(uchar)(((uchar)(rgb1>>8))*weight + ((uchar)(rgb0>>8))*(1-weight)));
}

/*! Same as lerp(fg, bg, .5), it grays out the color. */
Color fltk::inactive(Color fg, Color bg) {
  return lerp(fg, bg, .5);
}

/*! Same as lerp(fg, getbgcolor(), .5). This is for back-compatability only? */
Color fltk::inactive(Color fg) {
  return lerp(fg, current_bgcolor_, .5);
}

/*! Returns \a fg if fltk decides it can be seen well when drawn against
  \a bg. Otherwise it returns either fltk::BLACK or fltk::WHITE. */
Color fltk::contrast(Color fg, Color bg) {
  uchar r1,g1,b1; split_color(fg, r1,g1,b1);
  uchar r2,g2,b2; split_color(bg, r2,g2,b2);
  int t = int(r1)-int(r2);
  int r = t*t*3;
  t = int(g1)-int(g2);
  r += t*t*10;
//   t = int(b1)-int(b2); // difference in blue is ignored
//   r += t*t;
  if (2*r > r1*g1+0xD8*0xD8)
    return fg;
  if (r2>0xA0 || g2>0x50)
    return BLACK;
  else
    return WHITE;
}

#if USE_CAIRO || DOXYGEN

/**
  Set the color for all subsequent drawing operations.
  \see setcolor_alpha()   
*/
void fltk::setcolor(Color color) {
  current_color_ = color;
  uchar r,g,b; split_color(color,r,g,b);
  cairo_set_source_rgb(cr,r/255.0,g/255.0,b/255.0);
}

/**
   Sets the current rgb and alpha to draw in, on rendering systems that
   allow it. If alpha is not supported this is the same as setcolor().
   The color you pass should \e not premultiplied by the alpha value,
   that would be a different, nyi, call.
*/
void fltk::setcolor_alpha(Color color, float alpha) {
  current_color_ = color;
  uchar r,g,b; split_color(color,r,g,b);
  cairo_set_source_rgba(cr,r/255.0,g/255.0,b/255.0,alpha);
}

/**
  Set how to draw lines (the "pen"). If you change this it is your
  responsibility to set it back to the default with
  fltk::line_style(0).

  \a style is a bitmask in which you 'or' the following values. If you
  don't specify a dash type you will get a solid line. If you don't
  specify a cap or join type you will get a system-defined default of
  whatever value is fastest.
  - fltk::SOLID      -------
  - fltk::DASH       - - - -
  - fltk::DOT        ·········
  - fltk::DASHDOT    - · - ·
  - fltk::DASHDOTDOT - ·· - ··
  - fltk::CAP_FLAT
  - fltk::CAP_ROUND
  - fltk::CAP_SQUARE (extends past end point 1/2 line width)
  - fltk::JOIN_MITER (pointed)
  - fltk::JOIN_ROUND
  - fltk::JOIN_BEVEL (flat)

  \a width is the number of pixels thick to draw the lines. Zero
  results in the system-defined default, which on both X and Windows
  is somewhat different and nicer than 1.

  \a dashes is a pointer to an array of dash lengths, measured in
  pixels, if set then the dash pattern in \a style is ignored.
  The first location is how long to draw a solid portion, the
  next is how long to draw the gap, then the solid, etc. It is
  terminated with a zero-length entry. A null pointer or a zero-length
  array results in a solid line. Odd array sizes are not supported and
  result in undefined behavior. <i>The dashes array is ignored on
  Windows 95/98.</i>
*/
void fltk::line_style(int style, float width, const char* dashes) {
  line_style_ = style;
  line_width_ = width;
  line_dashes_ = dashes;
  char buf[7];
  int ndashes = dashes ? strlen(dashes) : 0;
  // emulate the _WIN32 dash patterns on X
  if (!ndashes && style&0xff) {
    int w = int(width+.5); if (w<1) w = 1;
    char dash, dot, gap;
    // adjust lengths to account for cap:
    if (style & 0x200 || !width) {
      dash = char(2*w);
      dot = 0;
      gap = char(2*w);
    } else {
      dash = char(3*w);
      dot = gap = char(w);
    }
    dashes = buf;
    char* p = buf;
    switch (style & 0xff) {
    default:
    case DASH:
      *p++ = dash; *p++ = gap;
      break;
    case DOT:
      *p++ = dot; *p++ = gap;
      break;
    case DASHDOT:
      *p++ = dash; *p++ = gap; *p++ = dot; *p++ = gap;
      break;
    case DASHDOTDOT:
      *p++ = dash; *p++ = gap; *p++ = dot; *p++ = gap; *p++ = dot; *p++ = gap;
      break;
    }
    ndashes = p-buf;
  }
  if (!width) {
    cairo_set_line_width(cr, 1.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  } else {
    cairo_set_line_width(cr, width);
    int c = (style>>8)&3; if (c) c--;
    cairo_set_line_cap(cr, (cairo_line_cap_t)c);
  }
  int j = (style>>12)&3; if (j) j--;
  cairo_set_line_join(cr, (cairo_line_join_t)j);
  if (ndashes) {
    double dash[20];
    for (int i = 0; i < ndashes; i++) dash[i] = dashes[i];
    cairo_set_dash(cr, dash, ndashes, 0);
  } else {
    cairo_set_dash(cr, 0, 0, 0);
  }
}

// Include the non-Cairo versions:
#elif USE_X11
# include "x11/setcolor.cxx"
#elif defined(_WIN32)
# include "win32/setcolor.cxx"
#elif USE_QUARTZ
# include "osx/setcolor.cxx"
#else
# error
#endif

/*! Set one of the indexed colors to the given rgb color. \a i must be
  in the range 0-255, and \a c must be a non-indexed rgb color. */
void fltk::set_color_index(Color i, Color color) {
  if (cmap[i] != color) {
#if CALL_FREE_COLOR // defined for X11 colormaps
    free_color(i);
#endif
    cmap[i] = color;
  }
}

/*! Return the rgb form of \a color. If it is an indexed color that
  entry is returned. If it is an rgb color it is returned unchanged. */
Color fltk::get_color_index(Color color) {
  if (color & 0xFFFFFF00) return color;
  return (Color)cmap[color];
}

/*!\fn Color color(unsigned char red, unsigned char green, unsigned char blue);
  Uses shift and add to make an rgb color out of the 8-bit components.
*/

/*! \fn Color color(unsigned char gray)
  Makes a gray-scale color using an 8-bit gray level. This is
  done by multiplying \a gray by 0x1010100. */

Color	fltk::current_color_;
Color	fltk::current_bgcolor_;
Flags	fltk::drawflags_;
int	fltk::line_style_;
float	fltk::line_width_;
const char* fltk::line_dashes_;

/*! \fn Color fltk::getcolor()
  Returns the last Color passed to setcolor().
*/

/*! \fn void fltk::setbgcolor(Color)
  Set the "background" color. This is not used by the drawing functions,
  but many box and image types will refer to it by calling getbgcolor().
*/

/*! \fn Color fltk::getbgcolor()
  Returns the last Color passed to setbgcolor().
  To actually draw in the bg color, do this:
\code
  Color saved = getcolor();
  setcolor(getbgcolor());
  draw_stuff();
  setcolor(saved)
\endcode
*/

/*! \fn void fltk::setdrawflags(Flags)

  Store a set of bit flags that may influence the drawing of some
  fltk::Symbol subclasses, such as boxes. Generally you must also
  use setcolor() and setbgcolor() to set the color you expect
  as not all symbols draw differently depending on the flags.

  The flags are usually copied from the flags() on a Widget.

  Some commonly-used flags:
    - INACTIVE_R : Draw inactive, fill images with solid fg color
    - VALUE: Draw turned on or checked
    - SELECTED: Draw as though selected in a browser or menu.
    - HIGHLIGHT: Draw as though highlighted by the mouse pointing at it
    - PUSHED: Draw as though pushed by the user
    - FOCUSED: Draw as though it has keyboard focus
    - INVISIBLE: Some boxes don't draw their interior if this is set

  \see fltk::drawstyle()
*/

/*! \fn Flags fltk::drawflags()
  Return the last flags passed to setdrawflags().
*/

/*! \fn Flags fltk::drawflags(Flags f)
  Same as (drawflags() & f), returns true if any of the flags in \a f
  are set.
*/

/*! \fn int fltk::line_style()
  Return the last value sent to line_style(int,width,dashes), indicating the
  cap and join types and the built-in dash patterns.
*/

/*! \fn float fltk::line_width()
  Return the last value for \a width sent to line_style(int,width,dashes).
*/

/*! \fn const char* fltk::line_dashes()
  Return the last value for \a dashes sent to line_style(int,width,dashes).
  Note that the actual pointer is returned, which may not point at legal
  data if a local array was passed, so this is only useful for checking
  if it is NULL or not.
*/

//
// End of "$Id: setcolor.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
