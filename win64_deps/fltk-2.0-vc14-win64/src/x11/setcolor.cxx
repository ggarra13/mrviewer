//
// "$Id: setcolor.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// X color functions for the Fast Light Tool Kit (FLTK).
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

// This file does not compile independently, it is included by color.cxx

#include "XColorMap.h"
#include <string.h>

////////////////////////////////////////////////////////////////
// Code to look at the X visual and figure out the best way to turn
// a color into a pixel value.
// figure_out_visual() calculates masks & shifts for generating
// pixels in TrueColor visuals.

int fl_redbits, fl_greenbits, fl_bluebits;
int fl_redshift, fl_greenshift, fl_blueshift, fl_extrashift;
unsigned char fl_redmask, fl_greenmask, fl_bluemask;
static uchar beenhere;

static void figure_out_visual() {
  beenhere = 1;
  open_display();
  if (!xvisual->red_mask || !xvisual->green_mask || !xvisual->blue_mask){
#if USE_COLORMAP
    fl_redmask = 0;
    // make sure black & white are allocated:
    fl_xmap(WHITE, 0xff, 0xff, 0xff);
    fl_xmap(BLACK, 0, 0, 0);
    return;
#else
    fatal("Requires TrueColor visual");
#endif
  }

  // get the bit masks into a more useful form:
  int i,j,m;

  for (i = 0, m = 1; m; i++, m<<=1) if (xvisual->red_mask & m) break;
  for (j = i; m; j++, m<<=1) if (!(xvisual->red_mask & m)) break;
  fl_redshift = j-8;
  fl_redbits = j-i; if (fl_redbits > 8) fl_redbits = 8;
  fl_redmask = 0xff << (8-fl_redbits);

  for (i = 0, m = 1; m; i++, m<<=1) if (xvisual->green_mask & m) break;
  for (j = i; m; j++, m<<=1) if (!(xvisual->green_mask & m)) break;
  fl_greenshift = j-8;
  fl_greenbits = j-i; if (fl_greenbits > 8) fl_greenbits = 8;
  fl_greenmask = 0xff << (8-fl_greenbits);

  for (i = 0, m = 1; m; i++, m<<=1) if (xvisual->blue_mask & m) break;
  for (j = i; m; j++, m<<=1) if (!(xvisual->blue_mask & m)) break;
  fl_blueshift = j-8;
  fl_bluebits = j-i; if (fl_bluebits > 8) fl_bluebits = 8;
  fl_bluemask = 0xff << (8-fl_bluebits);

  i = fl_redshift;
  if (fl_greenshift < i) i = fl_greenshift;
  if (fl_blueshift < i) i = fl_blueshift;
  if (i < 0) {
    fl_extrashift = -i;
    fl_redshift -= i; fl_greenshift -= i; fl_blueshift -= i;
  } else {
    fl_extrashift = 0;
  }
}

////////////////////////////////////////////////////////////////

#if USE_COLORMAP
static XColorMap normal_xmap[256];
#if USE_OVERLAY | USE_GL_OVERLAY
bool fl_overlay = false;
static XColorMap overlay_xmap[256];
Colormap fl_overlay_colormap;
XVisualInfo* fl_overlay_visual;
ulong fl_transparent_pixel;
#endif
#endif

/**
Returns the X pixel number used to draw the given FLTK color.
If a colormapped visual is being used, this may allocate it, or find
the nearest match.
*/
ulong fltk::xpixel(Color i) {
  if (!beenhere) figure_out_visual();

#if USE_COLORMAP
#if USE_OVERLAY | USE_GL_OVERLAY
  if (fl_redmask && !fl_overlay)
#else
  if (fl_redmask)
#endif
    {
#endif
    // return color for a TrueColor visual:
    if (!(i & 0xFFFFFF00)) i = (Color)cmap[i];
    return
      ((((i>>24)& fl_redmask)  << fl_redshift)+
       (((i>>16)& fl_greenmask)<< fl_greenshift)+
       (((i>>8) & fl_bluemask) << fl_blueshift)
       ) >> fl_extrashift;
#if USE_COLORMAP
    }

  // rest of this is for a colormapped visual (it will work for a TrueColor
  // visual but wastes time):

  // Figure out the index and the rgb colors wanted
  unsigned char index, r,g,b;
  if (i & 0xffffff00) { // an rgb color
    r = i>>24;
    g = i>>16;
    b = i>> 8;
    //if (r == g && r == b) index = GRAY00+r*(GRAY99-GRAY00)/256;
    index = (unsigned char) (BLACK + (b*5/256 * 5 + r*5/256) * 8 + g*8/256);
  } else { // an indexed color
    index = (unsigned char) i;
    i = (Color)cmap[i];
    r = i>>24;
    g = i>>16;
    b = i>> 8;
  }
  return fl_xmap(index,r,g,b).pixel;
}

// Return an entry from the current colormap, using the index.
// If the color is not allocated, allocate it using the passed rgb values
XColorMap& fl_xmap(uchar index, uchar r, uchar g, uchar b)
{
  // see if we have already allocated it:
#if USE_OVERLAY | USE_GL_OVERLAY
  XColorMap &xmap = fl_overlay ? overlay_xmap[index] : normal_xmap[index];
#else
  XColorMap &xmap = normal_xmap[index];
#endif
  if (xmap.mapped) return xmap;

#if USE_OVERLAY | USE_GL_OVERLAY
  Colormap colormap = fl_overlay ? fl_overlay_colormap : xcolormap;
  static XColor* ac[2];
  XColor*& allcolors = ac[fl_overlay];
  static int nc[2];
  int& numcolors = nc[fl_overlay];
#else
  Colormap colormap = xcolormap;
  static XColor *allcolors;
  static int numcolors;
#endif

  // I don't try to allocate colors with XAllocColor after the first
  // failure.  It is possible that it will work, since a color
  // may have been freed, but some servers are extremely slow and this
  // avoids a round trip for every color after that:
  if (!numcolors) { // if we have not failed yet:

    // Try XAllocColor:
    XColor xcol;
    xcol.red = r<<8; xcol.green = g<<8; xcol.blue = b<<8;
    if (XAllocColor(xdisplay, colormap, &xcol)) {
      xmap.mapped = 1;
      xmap.r = xcol.red>>8;
      xmap.g = xcol.green>>8;
      xmap.b = xcol.blue>>8;
      xmap.pixel = xcol.pixel;
      return xmap;
    }

    // Failed, read the colormap so we can search it:
    // I only read the colormap once.  Again this is due to the slowness
    // of round-trips to the X server, even though other programs may alter
    // the colormap after this and make decisions here wrong:
#if USE_OVERLAY | USE_GL_OVERLAY
    if (fl_overlay) numcolors = fl_overlay_visual->colormap_size; else
#endif
      numcolors = xvisual->colormap_size;
    if (!allcolors) allcolors = new XColor[numcolors];
    for (int p = numcolors; p--;) allcolors[p].pixel = p;
    XQueryColors(xdisplay, colormap, allcolors, numcolors);
  }

  // Find least-squares match in the colormap:
  int mindist = 0x7FFFFFFF;
  unsigned int bestmatch = 0;
  for (unsigned int n = numcolors; n--;) {
#if USE_OVERLAY | USE_GL_OVERLAY
    if (fl_overlay && n == fl_transparent_pixel) continue;
#endif
    XColor &a = allcolors[n];
    int d, t;
    t = int(r)-int(a.red>>8); d = t*t;
    t = int(g)-int(a.green>>8); d += t*t;
    t = int(b)-int(a.blue>>8); d += t*t;
    if (d <= mindist) {bestmatch = n; mindist = d;}
  }
  XColor &p = allcolors[bestmatch];

  // It appears to "work" to not call this XAllocColor, which will
  // avoid another round-trip to the server.  But then X does not
  // know that this program "owns" this value, and can (and will)
  // change it when the program that did allocate it exits:
  if (XAllocColor(xdisplay, colormap, &p)) {
    xmap.mapped = 1;
    xmap.pixel = p.pixel;
  } else {
    // However, if that XAllocColor fails, I have to give up and
    // assumme the pixel is ok for the duration of the program.  This
    // is due to bugs (?) in the Solaris X and some X terminals
    // where XAllocColor *always* fails when the colormap is full,
    // even if we ask for a color already in it...
    xmap.mapped = 2; // 2 prevents XFreeColor from being called
    xmap.pixel = bestmatch;
  }
  xmap.r = p.red>>8;
  xmap.g = p.green>>8;
  xmap.b = p.blue>>8;
  return xmap;
#endif // USE_COLORMAP
}

ulong fltk::current_xpixel;

void fltk::setcolor(Color i) {
  current_color_ = i;
  current_xpixel = xpixel(i);
  XSetForeground(xdisplay, gc, current_xpixel);
}

// alpha color is not implemented on X
void fltk::setcolor_alpha(Color color, float alpha) {
  setcolor(color);
}

#if USE_COLORMAP || USE_OVERLAY || USE_GL_OVERLAY
// This is used by setcolor_index()
static inline void free_color(Color i) {
#if USE_COLORMAP
  if (normal_xmap[i].mapped) {
    if (normal_xmap[i].mapped == 1)
      XFreeColors(xdisplay, xcolormap, &(normal_xmap[i].pixel), 1, 0);
    normal_xmap[i].mapped = 0;
  }
#if USE_OVERLAY | USE_GL_OVERLAY
  if (overlay_xmap[i].mapped) {
    if (overlay_xmap[i].mapped == 1)
      XFreeColors(xdisplay, fl_overlay_colormap, &(overlay_xmap[i].pixel),1,0);
    overlay_xmap[i].mapped = 0;
  }
#endif
#endif
}
#define CALL_FREE_COLOR 1
#endif

////////////////////////////////////////////////////////////////

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
    if (style & 0x200) {
      dash = char(2*w);
      dot = 1; // unfortunately 0 does not work
      gap = char(2*w-1);
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
      // Bug in XFree86 3.0? If I only use the above two pieces it does
      // not completely "erase" the previous dash pattern. Making it longer
      // like this seems to fix this. For some reason this bug is only for
      // the dot pattern (not the dash), and only for 0-width lines:
      *p++ = dot; *p++ = gap; *p++ = dot; *p++ = gap;
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
  if (ndashes) XSetDashes(xdisplay, gc, 0, dashes, ndashes);
  static int Cap[4] = {CapButt, CapButt, CapRound, CapProjecting};
  static int Join[4] = {JoinMiter, JoinMiter, JoinRound, JoinBevel};
  XSetLineAttributes(xdisplay, gc, int(width+.5),
		     ndashes ? LineOnOffDash : LineSolid,
		     Cap[(style>>8)&3], Join[(style>>12)&3]);
}

//
// End of "$Id: setcolor.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
