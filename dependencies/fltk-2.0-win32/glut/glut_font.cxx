//
// "$Id: glut_font.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// GLUT bitmap font routines for the Fast Light Tool Kit (FLTK).
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

// (sort of) emulation of Glut's bitmap drawing functions, using FL's
// font stuff.  Not all the fonts match!

#include <config.h>
#if HAVE_GL

#include <fltk/glut.h>
#include <fltk/gl.h>
using namespace fltk;

Glut_Bitmap_Font glutBitmap9By15 = {SCREEN_FONT, 15};
Glut_Bitmap_Font glutBitmap8By13 = {SCREEN_FONT, 13};
Glut_Bitmap_Font glutBitmapTimesRoman10 = {TIMES, 10};
Glut_Bitmap_Font glutBitmapTimesRoman24 = {TIMES, 24};
Glut_Bitmap_Font glutBitmapHelvetica10 = {HELVETICA, 10};
Glut_Bitmap_Font glutBitmapHelvetica12 = {HELVETICA, 12};
Glut_Bitmap_Font glutBitmapHelvetica18 = {HELVETICA, 18};

void glutBitmapCharacter(void* font, int character) {
  glsetfont(((Glut_Bitmap_Font *)font)->font,((Glut_Bitmap_Font *)font)->size);
  char a[1]; a[0] = character;
  gldrawtext(a,1);
}

int glutBitmapWidth(void* font, int character) {
  glsetfont(((Glut_Bitmap_Font *)font)->font,((Glut_Bitmap_Font *)font)->size);
  char a[1]; a[0] = character;
  return int(glgetwidth(a,1)+.5);
}

int glutBitmapLength(void* font, const unsigned char* string) {
  glsetfont(((Glut_Bitmap_Font *)font)->font,((Glut_Bitmap_Font *)font)->size);
  return int(glgetwidth((const char*)string)+.5);
}

#endif

//
// End of "$Id: glut_font.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
