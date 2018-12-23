//
// "$Id: Color.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <config.h>
#include <fltk/Color.h>
#include <fltk/string.h>
#if USE_X11
# include <fltk/x.h> // for XParseColor
#endif
#include <ctype.h>
#include <stdio.h>

/**
  Turn a string into a color. If \a name is null this returns NO_COLOR.
  Otherwise it returns fltk::parsecolor(name, strlen(name)).
*/
fltk::Color fltk::color(const char* name) {
  if (!name) return NO_COLOR;
  return parsecolor(name, strlen(name));
}

// Returns fltk::color(r,g,b) but returns BLACK instead of zero for 0,0,0:
static fltk::Color ccolor(unsigned r, unsigned g, unsigned b) {
  if (r || g || b)
    return fltk::color(r,g,b);
  else
    return fltk::BLACK;
}

/* fabien: const char* const invalid in a struct (even const) in VC6 */
static const struct {const char* name; fltk::Color color;}
web_safe[] = { // must be in alphabetical order!
  {"aqua",	0x00ffff00},
  {"black",	fltk::BLACK},
  {"blue",	0x0000ff00},
  {"fuchsia",	0xff00ff00},
  {"gray",	0x80808000},
  {"green",	0x00800000},
  {"lime",	0x00ff0000},
  {"maroon",	0x80000000},
  {"navy",	0x00008000},
  {"olive",	0x80800000},
  {"orange",	0xffa50000},
  {"purple",	0x80008000},
  {"red",	0xff000000},
  {"silver",	0xc0c0c000},
  {"teal",	0x00808000},
  {"white",	0xffffff00},
  {"yellow",	0xffff0000}
};

/*!
  Turn the first n bytes of \a name into an fltk color. This allows
  you to parse a color out of the middle of a string.

  Recognized values are:

  - "" turns into NO_COLOR
  - "0"-"99" decimal fltk color number, only works for indexed color range
  - "0xnnn" hex value of any fltk color number
  - "rgb" or "#rgb" three hex digits for rgb
  - "rrggbb" or "#rrggbb" 2 hex digits for each of rgb
  - "rrggbbaa" or "#rrggbbaa" fltk color number in hex
  - "rrrgggbbb" or "#rrrgggbbb" 3 hex digits for each of rgb
  - "rrrrggggbbbb" or "#rrrrggggbbbb" 4 hex digits for each of rgb
  - 17 "web safe colors" as defined by CSS 2.1
  - If FLTK is compiled to use X11, then XParseColor() is tried
  - all other strings return NO_COLOR.
*/
fltk::Color fltk::parsecolor(const char* name, unsigned length) {
  if (!length)
    return NO_COLOR;

  unsigned r = 0;
  unsigned g = 0;
  unsigned b = 0;
  unsigned n = 0;

  if (length < 3) {
    sscanf(name, "%u%n", &r, &n);
    if (n == length) return fltk::Color(r);
  }

  if (length > 2 && name[0]=='0' && (name[1]=='x' || name[1]=='X')) {
    sscanf(name+2, "%x%n", &r, &n);
    if (n == length-2) return Color(r);
  }

  // various sets of hex digits:
  {int l1 = length;
  const char* n1 = name;
  if (length > 1 && name[0] == '#') {n1++; l1--;}

  switch (l1) {
  case 3: // rgb
    if (sscanf(n1, "%1x%1x%1x", &r, &g, &b)==3)
      return ccolor(r*0x11, g*0x11, b*0x11);
    break;

  //case 4: // rgba ??

  case 6: // rrggbb
    if (sscanf(n1, "%2x%2x%2x", &r, &g, &b)==3)
      return ccolor(r,g,b);
    break;

  case 8: // rrggbbaa
    if (sscanf(n1, "%8x", &r)==1)
      return Color(r);
    break;

  case 9: // rrrgggbbb
    if (sscanf(n1, "%3x%3x%3x", &r, &g, &b)==3)
      return ccolor(r>>4,g>>4,b>>4);
    break;

  case 12: // rrrrggggbbbb (not rrrgggbbbaaa!)
    if (sscanf(n1, "%4x%4x%4x", &r, &g, &b)==3)
      return ccolor(r>>8,g>>8,b>>8);
    break;

  // case 16: // rrrrggggbbbbaaaa?
  }}

  // Search the "web safe colors" array:
  unsigned a = 0;
  b = sizeof(web_safe)/sizeof(*web_safe);
  while (a < b) {
    unsigned c = (a+b)/2;
    int d = strncmp(web_safe[c].name, name, length);
    if (d < 0)
      a = c+1;
    else if (d>0 || web_safe[c].name[length])
      b = c;
    else
      return web_safe[c].color;
  }

#if USE_X11
  if (length < 128) {
    char buf[128];
    strncpy(buf, name, length);
    buf[length] = 0;
    XColor x;
    open_display();
    if (XParseColor(xdisplay, xcolormap, buf, &x))
      return ccolor(x.red>>8, x.green>>8, x.blue>>8);
  }
#endif

  return NO_COLOR;
}

//
// End of "$Id: Color.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
