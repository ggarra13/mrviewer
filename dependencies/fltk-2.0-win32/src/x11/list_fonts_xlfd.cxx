//
// "$Id: list_fonts_xlfd.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// This horrible mess is to to provide fltk with a friendly font-naming
// interface, despite the incredible stupidity in the X font mechanism.

// The "X font name" is a long string that looks something like this:

// -foundry-family-weight-slant-width1-style-8-180-75-75-p-104-iso8859-1

// Fltk ignores the "foundry" and everything after the "style".  All X
// fonts where the remainder matches are part of an fltk "family" of
// fonts where the only differences are the size and encoding.

// The fltk interface, and much of the innards of this, uses "nice"
// names.  This is a straght-forward translation of the X name of some
// font in a family to compress it to a readable family name.  There is
// no guarantee that all nice names are unique, and there is no reverse
// translation other than to list all the fonts and find a match.

#include <fltk/x.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include "IFont.h"
using namespace fltk;

// return dash number N, or pointer to ending null if none:
static const char* font_word(const char* p, int n) {
//  while (*p) {if (*p=='-') {if (!--n) break;} p++;}
  for (; *p; p++) if (*p == '-' && !--n) break; // simpler to follow logic?

  return p;
}

// turn word N of a X font name into either some attribute bits
// (right now 0, BOLD, or ITALIC), or into -1 indicating that
// the word should be put into the name:

static int attribute(int n, const char *p) {
  // don't put blank things into name:
  if (!*p || *p=='-' || *p=='*') return 0;
  if (n == 3 || n == 5) { // weight or sWidth
    if (!strncmp(p,"normal",6) ||
	!strncmp(p,"light",5) ||
	!strncmp(p,"medium",6) ||
	!strncmp(p,"book",4) ||
	!strncmp(p,"regular",7)) return 0;
    if (!strncmp(p,"bold",4) ||
	!strncmp(p+4,"bold",4) ||
	!strncmp(p+5,"bold",4)) return BOLD;
  } else if (n == 4) { // slant
    if (*p == 'r') return 0;
    if (*p == 'i' || *p == 'o') return ITALIC;
  }
  return -1;
}

// turn the system name into a nice name, write it to output buffer,
// and return the accumulated attribute, and possibly the size:

static int to_nice(char* o, const char* p) {

  int type = 0;

  // get the family:
  const char *x = font_word(p,2); if (*x) x++; if (*x=='*') x++;
  const char *e = font_word(x,1);
  strncpy(o,x,e-x); o += e-x;

  // collect all the attribute words:
  for (int n = 3; n <= 5; n++) {
    // get the next word:
    if (*e) e++; x = e; e = font_word(x,1);
    int t = attribute(n,x);
    if (t < 0) {*o++ = ' '; strncpy(o,x,e-x); o += e-x;}
    else type |= t;
  }
  *o = 0;

  return type;
}

////////////////////////////////////////////////////////////////
// Generate a list of every font known by X server:

// Sort X font names in alphabetical order of their "nice" names:
// This is horrendously slow, but only needs to be done once....
extern "C" {
static int sort_function(const void *aa, const void *bb) {
  char aname[128]; int aattrib = to_nice(aname, *(char**)aa);
  char bname[128]; int battrib = to_nice(bname, *(char**)bb);
  int ret = strcasecmp(aname, bname); if (ret) return ret;
  return (aattrib-battrib);
}
}

int fltk::list_fonts(fltk::Font**& arrayp) {
  static fltk::Font** font_array = 0;
  static int num_fonts = 0;
  if (font_array) {arrayp = font_array; return num_fonts;}

  open_display();
  int xlistsize;
  if (!current_font_)
    setfont(HELVETICA, 14);
  char **xlist = XListFonts(xdisplay, "-*", 10000, &xlistsize);
  if (!xlist) return 0; // ???
  qsort(xlist, xlistsize, sizeof(*xlist), sort_function);

  IFont* family = 0; // current family
  char family_name[128] = ""; // nice name of current family

  int array_size = 0;
  for (int i=0; i<xlistsize;) {
    char newname[128];
    int attribute = to_nice(newname, xlist[i]);

    // find all the matching fonts:
    int n = 1;
    for (; i+n < xlistsize; n++) {
      char nextname[128];
      int nextattribute = to_nice(nextname, xlist[i+n]);
      if (nextattribute != attribute || strcmp(nextname, newname)) break;
    }

    IFont* newfont;
    // See if it is one of our built-in fonts:
    const char* skip_foundry = font_word(xlist[i],2);
    int length = font_word(skip_foundry, 6)-skip_foundry;
    for (int j = 0; ; j++) {
      if (j >= 16) { // no, create a new font
	newfont = new IFont;
	newfont->f.name_ = newstring(newname);
	newfont->f.attributes_ = attribute;
	newfont->system_name = xlist[i];
	newfont->bold = newfont;
	newfont->italic = newfont;
	newfont->first = 0;
	newfont->xlist = xlist+i;
	newfont->n = -n;
	break;
      }
      // see if it is one of our built-in fonts:
      // if so, set the list of x fonts, since we have it anyway
      IFont* match = (IFont*)(fltk::font(j));
      if (!strncmp(skip_foundry, match->system_name+2, length)) {
	newfont = match;
	if (!newfont->xlist) {
	  newfont->xlist = xlist+i;
	  newfont->n = -n;
	}
	break;
      }
    }

    if (attribute && !strcmp(newname, family_name)) {
      switch (attribute) {
      case BOLD: family->bold = newfont; break;
      case ITALIC: family->italic = newfont; break;
      case BOLD|ITALIC:
	family->bold->italic = family->italic->bold = newfont;
	break;
      }
    } else {
      family = newfont;
      strcpy(family_name, newname);
      if (num_fonts >= array_size) {
	array_size = 2*array_size+128;
	font_array = (fltk::Font**)realloc(font_array, array_size*sizeof(fltk::Font*));
      }
      font_array[num_fonts++] = &(newfont->f);
    }

    i += n;
  }
  arrayp = font_array;
  return num_fonts;
}

////////////////////////////////////////////////////////////////

int fltk::Font::encodings(const char**& arrayp) {
  IFont* t = (IFont*)this;
  if (!t->xlist) {
    open_display();
    t->xlist = XListFonts(xdisplay, t->system_name, 100, &(t->n));
    if (!t->xlist) return 0;
  }
  int listsize = t->n;
  if (listsize < 0) listsize = -listsize;
  static const char* array[128];
  int count = 0;
  for (int i = 0; i < listsize; i++) {
    char *q = t->xlist[i];
    const char *c = font_word(q,13);
    if (!*c++ || !*c) continue;
    // insert-sort the new encoding into list:
    int n;
    int m;
    for (n = count; n > 0; n--) {
      int cmp = strcasecmp(array[n-1], c);
      if (cmp < 0) break;
      if (cmp == 0) goto CONTINUE;
    }
    for (m = count; m > n; m--) array[m] = array[m-1];
    array[n] = c;
    count++;
    if (count >= 128) break;
  CONTINUE:;
  }
  arrayp = array;
  return count;
}

////////////////////////////////////////////////////////////////

int fltk::Font::sizes(int*& sizep) {
  IFont* t = (IFont*)this;
  if (!t->xlist) {
    open_display();
    t->xlist = XListFonts(xdisplay, t->system_name, 100, &(t->n));
    if (!t->xlist) return 0;
  }
  int listsize = t->n;
  if (listsize < 0) listsize = -listsize;
  static int array[128];
  int count = 0;
  for (int i = 0; i < listsize; i++) {
    char *q = t->xlist[i];
    const char* d = font_word(q,7);
    if (!*d++) continue;
    int s = strtol(d,0,10);
    // insert-sort the new size into list:
    int m;
    int n;
    for (n = count; n > 0; n--) {
      int cmp = array[n-1]-s;
      if (cmp < 0) break;
      if (cmp == 0) goto CONTINUE;
    }
    for (m = count; m > n; m--) array[m] = array[m-1];
    array[n] = s;
    count++;
    if (count >= 128) break;
  CONTINUE:;
  }
  sizep = array;
  return count;
}

//
// End of "$Id: list_fonts_xlfd.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
