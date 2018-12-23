// "$Id: Font.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Font selection code for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <fltk/Font.h>
#include <fltk/draw.h>
#include <fltk/x.h>
#include <fltk/string.h>

/** \class fltk::Font

Identifies a font. You can create these with fltk::font(name) or
with fltk::list_fonts(). Do not attempt to create your instances
or modify it.

This is a struct so that fltk can initialize a table internally
with constants. Don't use the undocumented fields.
*/

/** \fn fltk::Font* fltk::Font::plus(int attributes);
  Return a font from the same family with the extra attributes turned
  on. This may return the same font if the attributes are already on
  or there is no font with those attributes.
*/

/** \fn fltk::Font* fltk::Font::bold()
  Same as plus(BOLD), returns a bold version of this font.
*/

/** \fn fltk::Font* fltk::Font::italic()
  Same as plus(ITALIC), returns an italic version of this font.
*/

/** \fn const char* fltk::Font::system_name();
  Returns the string actually passed to the operating system, which
  may be different than name().
  For Xlib this is a pattern sent to XListFonts to find all the sizes.
  For most other systems this is the same as name() without any attributes.
*/

/** \fn void fltk::drawtext_transformed(const char *text, int n, float x, float y);
  Draw text starting at a point returned by fltk::transform(). This
  is needed for complex text layout when the current transform may
  not match the transform being used by the font.
*/

/*! \fn void fltk::setfont(fltk::Font* font, float psize);

  Set the current font and font scaling so the size is size
  pixels. The size is unaffected by the current transformation matrix
  (you may be able to use fltk::transform() to get the size to get a
  properly scaled font).

  The size is given in pixels. Many pieces of software express sizes
  in "points" (for mysterious reasons, since everything else is
  measured in pixels!). To convert these point sizes to pixel sizes
  use the following code:
\code
const fltk::Monitor& monitor = fltk::Monitor::all();
float pixels_per_point = monitor.dpi_y()/72.0;
float font_pixel_size = font_point_size*pixels_per_point;
\endcode

  See the fltk::Font class for a description of what can be passed as
  a font. For most uses one of the built-in constant fonts like
  fltk::HELVETICA can be used.

*/

// Static variables containing the currently selected font, size, encoding:
fltk::Font* fltk::current_font_;
float fltk::current_size_;
const char *fltk::encoding_ = "iso10646-1";

#if USE_X11
# include "x11/Font.cxx"
#elif defined(_WIN32)
# include "win32/Font.cxx"
#elif USE_QUARTZ
# include "osx/Font.cxx"
#else
#error
#endif

/** \fn Font* fltk::getfont()
  Return the Font sent to the last setfont().
*/

/** \fn float fltk::getsize()
  Return the size sent to the last setfont(). You should use this as a
  minimum line spacing (using ascent()+descent() will produce oddly
  spaced lines for many fonts).
*/

/** \fn float fltk::getascent()
  Return the distance from the baseline to the top of letters in
  the current font.
*/

/** \fn float fltk::getdescent()
  Return the distance from the baseline to the bottom of letters in
  the current font.
*/

/** Draw a nul-terminated string. */
void fltk::drawtext(const char* text, float x, float y) {
  drawtext(text, strlen(text), x, y);
}

/**
  Draw the first n \e bytes (not characters if utf8 is used) starting
  at the given position.
*/
void fltk::drawtext(const char* text, int n, float x, float y) {
  transform(x,y);
  drawtext_transformed(text, n, x, y);
}

/** \fn float fltk::getwidth(const char *text, int n)
  Return the width of the first \a n bytes of this UTF-8 string drawn
  in the font set by the most recent setfont().
*/

/**
  Return the width of a nul-terminated UTF-8 string drawn in the
  font set by the most recent setfont().
*/
float fltk::getwidth(const char* text) {
  return getwidth(text, strlen(text));
}

/** \fn const char* fltk::Font::name(int* attributes)
  Return a string name for this font, and put any attributes
  (BOLD, ITALIC) into the location pointed to by \a attributes.
  Using the returned string and attributes as arguments to find()
  will return the same font.
*/

/**
  Return a single string that names this font. Passing this
  string and zero for the attributes to find() will return the
  same font.

  If the font's attributes are non-zero, this is done by appending a
  space and "Bold" and/or "Italic" to the name. This allows a single
  string rather than a string+attribute pair to identify a font, which
  is really useful for saving them in a file. <i>The return value is
  in a temporary buffer that will be overwritten on the next call in
  this case.</i>
*/
const char* fltk::Font::name() const {
  if (!attributes_) return name_;
  const int length=128;
  static char *buffer; if (!buffer) buffer = new char[length];
  strlcpy(buffer, name_, length);
  if (attributes_ & BOLD) strlcat(buffer, " Bold", length);
  if (attributes_ & ITALIC) strlcat(buffer, " Italic", length);
  return buffer;
}

/**
  Obsolete function to encourage FLTK to choose a 256-glyph font with
  the given encoding. You must call setfont() after changing this for
  it to have any effect.

  Notice that this is obsolete! Only the non-Xft X version actually
  uses it and that may be eliminated as well. In addition FLTK uses
  UTF-8 internally, and assummes that any font it prints with is
  using Unicode encoding (or ISO-8859-1 if there are only 256 characters).

  The default is "iso10646-1"
*/
void fltk::set_encoding(const char* f) {encoding_ = f;}

/** \fn const char* fltk::get_encoding()
  Returns the string sent to the most recent set_encoding().
*/

//
// End of "$Id: Font.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
