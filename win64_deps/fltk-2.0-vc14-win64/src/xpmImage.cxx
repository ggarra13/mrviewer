// "$Id: xpmImage.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/*! \class fltk::xpmImage

  Draws inline XPM data. This is a text-based 256-color image format
  designed for X11 and still very useful for making small icons, since
  the data can be easily inserted into your source code:

\code
#include "image_name.xpm"
static xpmImage image_name(image_name_xpm);
\endcode

  FLTK recognizes a few extensions to the xpm color map:
  * Setting the number of colors negative means the second line
  in the array is a "compressed" colormap, which is 4 bytes per
  color of character,r,g,b.
  * If all colors are grays and there is no transparent index, it
  instead makes a MASK image, where black draws the current color and
  white is transparent, and all other grays are partially transparent.
  This allows you to put antialiased glyphs into images.

*/

#include <fltk/xpmImage.h>
#include <fltk/draw.h>
#include <fltk/string.h>
#include <ctype.h>
#include <config.h>
using namespace fltk;

bool xpmImage::fetch() {
  return fetch(*this, data);
}

bool xpmImage::fetch(Image& i, const char * const * array) {
  int n, ncolors, chars_per_pixel, width, height;

  n = sscanf(array[0], "%d %d %d %d", &width, &height, &ncolors, &chars_per_pixel);
  if (n < 4 || (chars_per_pixel != 1 && chars_per_pixel != 2)) return false;
  i.setsize(width, height);
  if (width <= 0 || height <= 0) return false;

  // note that this array is unsigned char and skips the first line:
  const uchar*const* data = (const uchar*const*)(array+1);
  bool monochrome = true;
  bool transparent = false;

  U32 colors[256]; // ARGB32 colors
  U32* byte1[256]; // prefix for 2-byte xpm

  if (ncolors < 0) {	// fltk (non standard) compressed colormap
    ncolors = -ncolors;
    chars_per_pixel = 1;
    const uchar *p = *data++;
    // if first color is ' ' it is transparent (put it later to make
    // it not be transparent):
    if (p[0] == ' ') {
      colors[32] = 0;
      ncolors--;
      monochrome = false;
      transparent = true;
      p += 4;
    }
    // read all the rest of the colors:
    for (int i=0; i < ncolors; i++) {
      if (p[1]!=p[2] || p[1]!=p[3]) monochrome = false;
      colors[p[0]] = 0xff000000 | (p[1]<<16) | (p[2]<<8) | (p[3]);
      p += 4;
    }
  } else {	// normal XPM colormap with names
    if (chars_per_pixel>1) {
      memset(byte1, 0, sizeof(byte1));
      monochrome = false;
    }
    for (int i=0; i<ncolors; i++) {
      const uchar* p = *data++;
      // the first 1 or 2 characters are the color index:
      int index = *p++;
      U32* c; // where to store color
      if (chars_per_pixel>1) {
	U32* subcolors = byte1[index];
	if (!subcolors) subcolors = byte1[index] = new U32[256];
        c = subcolors+*p;
	index = (index<<8)+*p++;
      } else {
	c = colors+index;
      }
      // look for "c word", or last word if none:
      const uchar *previous_word = p;
      for (;;) {
	while (*p && isspace(*p)) p++; uchar what = *p++;
	while (*p && !isspace(*p)) p++;
	while (*p && isspace(*p)) p++;
	if (!*p) break;
	previous_word = p;
	while (*p && !isspace(*p)) p++;
	if (what == 'c') break;
      }
      Color C;
      if (*p) {
	char buf[30];
	if (p-previous_word > 29) p = previous_word+29;
	memcpy(buf, previous_word, p-previous_word);
	buf[p-previous_word] = 0;
	C = color(buf);
      } else {
	C = color((const char*)previous_word);
      }
      if (C) {
        *c = 0xff000000 | (C>>8); // convert fltk color to ARGB32
        // test to see if rgb are different from each other:
        if ((((C>>16)^C)|((C>>8)^C)) & 0xff00) monochrome=false;
      } else { // assume "None" or "#transparent" for any errors
        *c = 0;
	monochrome = false;
        transparent = true;
      }
    }
  }

  if (monochrome) {
    i.setpixeltype(MASK);
    for (int y=0; y<height; y++) {
      uchar* linebuf = i.linebuffer(y);
      const uchar* p = data[y];
      for (int x=0; x<width; x++) linebuf[x] = colors[*p++];
      i.setpixels(linebuf,y);
    }
  } else {
    i.setpixeltype(transparent ? ARGB32 : RGB32);
    if (chars_per_pixel==1) {
      for (int y=0; y<height; y++) {
        U32* linebuf = (U32*)i.linebuffer(y);
        const uchar* p = data[y];
        for (int x=0; x<width; x++) linebuf[x] = colors[*p++];
        i.setpixels((uchar*)linebuf,y);
      }
    } else {
      // 2 bytes per pixel
      for (int y=0; y<height; y++) {
        U32* linebuf = (U32*)i.linebuffer(y);
        const uchar* p = data[y];
        for (int x=0; x<width; x++) {
          U32* subcolors = byte1[*p++];
          linebuf[x] = subcolors ? subcolors[*p] : 0;
          p++;
        }
        i.setpixels((uchar*)linebuf,y);
      }
      for (int c=0; c<256; c++) delete[] byte1[c];
    }
  }
  return true;
}

//
// End of "$Id: xpmImage.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
