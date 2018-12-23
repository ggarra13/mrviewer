//
// "$Id: colormap.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Colormap generation program for the Fast Light Tool Kit (FLTK).
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

// This program produces the contents of "colormap.h" as stdout

/* #include <gl/gl.h> */
#include <stdio.h>

// This table is initialized with color values I got by reading the
// colormap on an IRIX 4.3 machine:

// "full intensity colors" have been turned down some to make white
// background less intense by default.  The hope is that this will make
// fltk programs more friendly on color-adjusted screens.  If you want
// pure colors you should get them out of the colormap.

//#define III 244 // maximum intensity of the basic colors

// that results in errors and unshared colormap entries, so full intensity:
#define III 255 // maximum intensity of the basic colors

static short cmap[256][3] = {
// 3-bit colormap:
  {  0,  0,  0},	// black
  {III,  0,  0},	// red
  {  0,III,  0},	// green
  {III,III,  0},	// yellow
  {  0,  0,III},	// blue
  {III,  0,III},	// magenta
  {  0,III,III},	// cyan
  {III,III,III},	// white
// pastel versions of those colors, from SGI's standard color map:
  { 85, 85, 85},	// 1/3 gray
  {198,113,113},	// salmon? pale red?
  {113,198,113},	// pale green
  {142,142, 56},	// khaki
  {113,113,198},	// pale blue
  {142, 56,142},	// purple, orchid, pale magenta
  { 56,142,142},	// cadet blue, aquamarine, pale cyan
// The next location was always a dark gray and was used as the selection
// color by fltk1. To make it look more like Windows this was changed to
// blue. It appears that all newer versions have changed it back:
//{  0,  0,128},	// Windows selection color
  {170,170,170},	// old 2/3 gray color
// These next 16 are the FREE_COLOR area. In some versions of fltk
// these were filled with random colors that a Irix 5.3 machine placed
// in these locations. Other versions of fltk filled this with the 1/3
// gray above to discourage their use. This newest version uses colors
// that NewTek has assigned for their GUI:
#if 0
  // The Irix 5.3 colors:
  { 16, 16, 16},
  {128, 40,128},
  {198, 30, 30},
  { 66, 30, 30},
  {176,140,140},
  {  0, 20, 20},
  { 20, 10, 10},
  { 40, 20, 20},
  { 60, 30, 30},
  {  0, 80, 80},
  {  0, 40, 40},
  { 20, 20,  0},
  { 40, 40,  0},
  { 80, 80, 10},
  {150,150, 20},
  {160, 10, 10},
#else
  // The NewTek colors: (from George Yohng)
  {168,168,152},
  {232,232,216},
  {104,104, 88},
  {152,168,168},
  {216,232,232},
  { 88,104,104},
  {156,156,168},
  {220,220,232},
  { 92, 92,104},
  {156,168,156},
  {220,232,220},
  { 92,104, 92},
  {144,144,144},
  {192,192,192},
  { 80, 80, 80},
  {160,160,160},
#endif
// The rest of the colormap is a gray ramp and table, filled in below:
};

// This is fltk::set_background() from Style.C, with modifications:

enum {
  GRAY00 = 32,
  GRAY75 = 49,
  GRAY99 = 55
};
typedef unsigned char uchar;

void background(uchar r, uchar g, uchar b) {
  int i;
  int R, G, B;
  for (i = GRAY00; i <= GRAY99; i++) {
    if (i <= GRAY75) {
      R = r*(i-GRAY00)/(GRAY75-GRAY00);
      G = g*(i-GRAY00)/(GRAY75-GRAY00);
      B = b*(i-GRAY00)/(GRAY75-GRAY00);
    } else {
      const int DELTA = ((0xff-0xe0)*(i-GRAY75))/(GRAY99-GRAY75);
      R = r+DELTA; if (R > 255) R = 255;
      G = g+DELTA; if (G > 255) G = 255;
      B = b+DELTA; if (B > 255) B = 255;
    }
    cmap[i][0] = uchar(R);
    cmap[i][1] = uchar(G);
    cmap[i][2] = uchar(B);
    // used to produce the comments in Color.h:
    //printf("%2d %02x %.2f %c\n", i, R, R/255.0, i-GRAY00+'A');
  }
}

int main() {
  int i,r,g,b;
#if 0
  /* Read colormap colors into internal table */
  long cmwin;
  noport();
  cmwin = winopen("CM");
  for (i=0; i<256; i++)
    getmcolor(i,&cmap[i][0],&cmap[i][1],&cmap[i][2]);
  winclose(cmwin);
#endif
// overwrite the X allocation area with one color so people are
// discouraged from using it:
  //for (i=16; i<32; i++) {cmap[i][0]=cmap[i][1]=cmap[i][2] = 85;}

  // fill in the gray ramp:
  background(0xe0, 0xe0, 0xe0); // fltk2 new brighter default
  // background(0xc0, 0xc0, 0xc0); // fltk1.1 colors, match windows 95
  // copy the 1/3 and 2/3 gray to the closest locations in fltk1.1:
  //   cmap[39][0] = cmap[39][1] = cmap[39][2] = 85;
  //   cmap[47][0] = cmap[47][1] = cmap[47][2] = 170;
  // background(170, 170, 170); // old fltk colors

  // fill in the color cube
  i = 56;
  for (b=0; b<5; b++)
    for (r=0; r<5; r++)
      for (g=0; g<8; g++) {
	cmap[i][0] = r*255/4;
	cmap[i][1] = g*255/7;
	cmap[i][2] = b*255/4;
	i++;
      }
  // modify entry used for tooltip colors to match Win32:
  cmap[215][2] = 0xe1;

  for (i=0; i<256; i++) {
    printf("  0x%02x%02x%02x00",cmap[i][0],cmap[i][1],cmap[i][2]);
    if (i < 255) printf(", // 0x%02x\n", i);
  }
  printf("\n");
  return 0;
}

//
// End of "$Id: colormap.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
