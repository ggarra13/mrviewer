//
// "$Id: list_fonts.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// This file is seperate from Font.cxx due the historical reasons in
// that on X11 significant code was saved by only using the fltk
// built-in fonts and not doing anything with named fonts. This
// is probably irrelevant now and they could be merged.

#include <config.h>
#include <fltk/Font.h>
#include <fltk/string.h>

#if USE_X11
# include "x11/list_fonts.cxx"
#elif defined(_WIN32)
# include "win32/list_fonts.cxx"
#elif USE_QUARTZ
# include "osx/list_fonts.cxx"
#endif

using namespace fltk;

/*! \fn int fltk::list_fonts(fltk::Font**& arrayp);
  \relates fltk::Font

  Generate an array containing every font on the server. \a arrayp
  is set to a pointer to this array, and the length of the array is
  the return value. Each entry is a "base" font, there may be bold,
  italic, and bold+italic version of each font pointed to by bold() or
  italic().

  Subsequent calls will usually return the same array quickly, but if
  a signal comes in indicating a change it will probably delete the
  old array and return a new one.
*/

/*! \relates fltk::Font

  Find a font with the given "nice" name. You can get bold and italic
  by adding a space and "bold" or "italic" (or both) to the name, or
  by passing them as the attributes. Case is ignored and fltk will
  accept some variations in the font name.

  The current implementation calls fltk::list_fonts() and then does a
  binary search of the returned list. This can make the first call
  pretty slow, especially on X. Directly calling the system has a
  problem in that we want the same structure returned for any call
  that names the same font. This is sufficiently painful that I have
  not done this yet.
*/
fltk::Font* fltk::font(const char* name, int attributes /* = 0 */) {
  if (!name || !*name) return 0;

  // find out if the " bold" or " italic" are on the end:
  int length = strlen(name);
  // also accept "italics" because old Nuke saved scripts used that:
  if (length > 8 && !strncasecmp(name+length-8, " italics", 8)) {
    length -= 8; attributes |= ITALIC;
  }
  if (length > 7 && !strncasecmp(name+length-7, " italic", 7)) {
    length -= 7; attributes |= ITALIC;
  }
  if (length > 5 && !strncasecmp(name+length-5, " bold", 5)) {
    length -= 5; attributes |= BOLD;
  }
  Font* font = 0;
  // always try the built-in fonts first, because list_fonts is *slow*...
  int i; for (i = 0; i <= 12; i += 4) {
    font = fltk::font(i);
    const char* fontname = font->name();
    if (!strncasecmp(name, fontname, length) && !fontname[length]) goto GOTIT;
  }
  // now try all the fonts on the server, using a binary search:
#if defined(WIN32) && !defined(__CYGWIN__)
  // this function is in win32/list_fonts.cxx:
  name = GetFontSubstitutes(name,length);
#endif
  font = 0;
  {Font** list; int b = list_fonts(list); int a = 0;
  while (a < b) {
    int c = (a+b)/2;
    Font* testfont = list[c];
    const char* fontname = testfont->name();
    int d = strncasecmp(name, fontname, length);
    if (!d) {
      // If we match a prefix of the font return it unless a better match found
      font = testfont;
      if (!fontname[length]) goto GOTIT;
    }
    if (d > 0) a = c+1;
    else b = c;
  }}
  if (!font) return 0;
 GOTIT:
  return font->plus(attributes);
}

/*! \fn fltk::Font* fltk::font(int i)
  \relates fltk::Font
  Turn an fltk1 integer font id into a font.
*/

/*! \fn int fltk::Font::sizes(int*& sizep);

  Sets array to point at a list of sizes. The return value is the
  length of this array. The sizes are sorted from smallest to largest
  and indicate what sizes can be given to fltk::setfont() that will be
  matched exactly (fltk::setfont() will pick the closest size for
  other sizes). A zero in the first location of the array indicates a
  scalable font, where any size works, although the array may still
  list sizes that work "better" than others. The returned array points
  at a static buffer that is overwritten each call, so you want to
  copy it if you plan to keep it.

  The return value is the length of the list. The argument \a arrayp
  is set to point at the array, which is in static memory reused
  each time this call is done.
*/

/*! \fn int fltk::Font::encodings(const char**& arrayp);

  Return all the encodings for this font. These strings may be
  sent to fltk::set_encoding() before using the font.

  The return value is the length of the list. The argument \a arrayp
  is set to point at the array, which is in static memory reused
  each time this call is done.
*/

//
// End of "$Id: list_fonts.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
