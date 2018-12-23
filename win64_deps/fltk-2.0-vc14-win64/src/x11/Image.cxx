// "$Id: drawimage.cxx 1399 2006-08-11 02:15:20Z spitzak $"
//
// Image drawing routines for the Fast Light Tool Kit (FLTK).
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

// This file is not independently compiled, it is included by ../Image.cxx

// Only a limited set of possible X servers is supported, as I have
// not encountered other versions, and the old code that supported more
// was unnecessarily complex:
//
// Anything with a 8 bits_per_pixel bitmaps (visual depth may be less)
// 16-bit TrueColor with arbitrary layout
// 32-bit TrueColor with ARGB32 layout
//
// Code for using X Shared Memory is here but is normally disabled by
// configure. This is because the current implementation will leak the
// memory permanently (as ipshm segments) if the images are not destroyed
// (which they won't be if your program crashes, no matter how many
// automatic destructors you create. Therefore this is Windows3.1-style
// stupidity).

#include <fltk/error.h>
#include <fltk/math.h>
#include "XColorMap.h"

#if USE_XSHM
# include <sys/ipc.h>
# include <sys/shm.h>
# include <X11/extensions/XShm.h>
static Bool use_xshm;
static Bool use_xshm_pixmaps;
#endif

// Xlib drawing code uses memcpy and getenv
#if !USE_XFT
# include <string.h>
# include <stdlib.h>
#endif

using namespace fltk;

/// Converter functions:
static void (*converter[9])(const uchar *from, uchar *to, int w);

static int dir;		// direction-alternator
static int ri,gi,bi;	// saved error-diffusion value

#if USE_COLORMAP
////////////////////////////////////////////////////////////////
// 8-bit converter with error diffusion

static void mono_to_8(const uchar *from, uchar *to, int w) {
  int r=ri, g=gi, b=bi;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1);
    to = to+(w-1);
    d = -1;
    td = -1;
  } else {
    dir = 1;
    d = 1;
    td = 1;
  }

  uchar i;
  for (;; from += d, to += td) {
    r += from[0]; if (r < 0) r = 0; else if (r>255) r = 255;
    g += from[0]; if (g < 0) g = 0; else if (g>255) g = 255;
    b += from[0]; if (b < 0) b = 0; else if (b>255) b = 255;
    i = (uchar)(BLACK + (b*5/256 * 5 + r*5/256) * 8 + g*8/256);
    XColorMap& xmap = fl_xmap(i,(uchar)r,(uchar)g,(uchar)b);
    r -= xmap.r;
    g -= xmap.g;
    b -= xmap.b;
    *to = uchar(xmap.pixel);
    if (!--w) break;
  }
  ri = r; gi = g; bi = b;
}

static void rgb_to_8d(const uchar *from, uchar *to, int w, int delta) {
  int r=ri, g=gi, b=bi;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    to = to+(w-1);
    d = -delta;
    td = -1;
  } else {
    dir = 1;
    d = delta;
    td = 1;
  }

  uchar i;
  for (;; from += d, to += td) {
    r += from[0]; if (r < 0) r = 0; else if (r>255) r = 255;
    g += from[1]; if (g < 0) g = 0; else if (g>255) g = 255;
    b += from[2]; if (b < 0) b = 0; else if (b>255) b = 255;
    i = (uchar)(BLACK + (b*5/256 * 5 + r*5/256) * 8 + g*8/256);
    XColorMap& xmap = fl_xmap(i,(uchar)r,(uchar)g,(uchar)b);
    r -= xmap.r;
    g -= xmap.g;
    b -= xmap.b;
    *to = uchar(xmap.pixel);
    if (!--w) break;
  }
  ri = r; gi = g; bi = b;
}

static void rgb_to_8(const uchar *from, uchar *to, int w) {
  rgb_to_8d(from, to, w, 3);
}

static void rgba_to_8(const uchar *from, uchar *to, int w) {
  rgb_to_8d(from, to, w, 4);
}

static void argb32_to_8(const uchar *from, uchar *to, int w) {
#if WORDS_BIGENDIAN
  rgb_to_8d(from+1, to, w, 4);
#else
  int r=ri, g=gi, b=bi;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*4;
    to = to+(w-1);
    d = -4;
    td = -1;
  } else {
    dir = 1;
    d = 4;
    td = 1;
  }

  uchar i;
  for (;; from += d, to += td) {
    r += from[2]; if (r < 0) r = 0; else if (r>255) r = 255;
    g += from[1]; if (g < 0) g = 0; else if (g>255) g = 255;
    b += from[0]; if (b < 0) b = 0; else if (b>255) b = 255;
    i = (uchar)(BLACK + (b*5/256 * 5 + r*5/256) * 8 + g*8/256);
    XColorMap& xmap = fl_xmap(i,(uchar)r,(uchar)g,(uchar)b);
    r -= xmap.r;
    g -= xmap.g;
    b -= xmap.b;
    *to = uchar(xmap.pixel);
    if (!--w) break;
  }
  ri = r; gi = g; bi = b;
#endif
}

#endif

////////////////////////////////////////////////////////////////
// 16 bit TrueColor converters with error diffusion
// Cray computers have no 16-bit type, so we use character pointers
// (which may be slow)

#ifdef U16
#define OUTTYPE U16
#define OUTSIZE 1
#define OUTASSIGN(v) *t = v
#else
#define OUTTYPE uchar
#define OUTSIZE 2
#define OUTASSIGN(v) int tt=v; t[0] = uchar(tt>>8); t[1] = uchar(tt)
#endif

static void mono_to_16(const uchar *from,uchar *to,int w) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1);
    t = t+(w-1)*OUTSIZE;
    d = -1;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = 1;
    td = OUTSIZE;
  }

  uchar mask = fl_redmask & fl_greenmask & fl_bluemask;
  uchar m;
  int r=ri;
  for (;; from += d, t += td) {
    r = (r&~mask) + *from; if (r > 255) r = 255;
    m = r&mask;
    OUTASSIGN((
      (m<<fl_redshift)+
      (m<<fl_greenshift)+
      (m<<fl_blueshift)
      ) >> fl_extrashift);
    if (!--w) break;
  }
  ri = r;
}

static void rgb_to_16d(const uchar *from, uchar *to, int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  int r=ri, g=gi, b=bi;
  for (;; from += d, t += td) {
    r = (r&~fl_redmask)  +from[0]; if (r>255) r = 255;
    g = (g&~fl_greenmask)+from[1]; if (g>255) g = 255;
    b = (b&~fl_bluemask) +from[2]; if (b>255) b = 255;
    OUTASSIGN((
      ((r&fl_redmask)<<fl_redshift)+
      ((g&fl_greenmask)<<fl_greenshift)+
      ((b&fl_bluemask)<<fl_blueshift)
      ) >> fl_extrashift);
    if (!--w) break;
  }
  ri = r; gi = g; bi = b;
}

static void rgb_to_16(const uchar *from, uchar *to, int w) {
  rgb_to_16d(from, to, w, 3);
}

static void rgba_to_16(const uchar *from, uchar *to, int w) {
  rgb_to_16d(from, to, w, 4);
}

static void argb32_to_16(const uchar *from, uchar *to, int w) {
#if WORDS_BIGENDIAN
  rgb_to_16d(from+1, to, w, 4);
#else
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*4;
    t = t+(w-1)*OUTSIZE;
    d = -4;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = 4;
    td = OUTSIZE;
  }
  int r=ri, g=gi, b=bi;
  for (;; from += d, t += td) {
    r = (r&~fl_redmask)  +from[2]; if (r>255) r = 255;
    g = (g&~fl_greenmask)+from[1]; if (g>255) g = 255;
    b = (b&~fl_bluemask) +from[0]; if (b>255) b = 255;
    OUTASSIGN((
      ((r&fl_redmask)<<fl_redshift)+
      ((g&fl_greenmask)<<fl_greenshift)+
      ((b&fl_bluemask)<<fl_blueshift)
      ) >> fl_extrashift);
    if (!--w) break;
  }
  ri = r; gi = g; bi = b;
#endif
}

// special-case the 5r6g5b layout used by XFree86:

static void mono_to_565(const uchar *from,uchar *to,int w) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1);
    t = t+(w-1)*OUTSIZE;
    d = -1;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = 1;
    td = OUTSIZE;
  }
  int r=ri;
  for (;; from += d, t += td) {
    r = (r&7) + *from; if (r > 255) r = 255;
    OUTASSIGN((r>>3) * 0x841);
    if (!--w) break;
  }
  ri = r;
}

static void rgb_to_565d(const uchar *from, uchar *to, int w, int delta) {
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*delta;
    t = t+(w-1)*OUTSIZE;
    d = -delta;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = delta;
    td = OUTSIZE;
  }
  int r=ri, g=gi, b=bi;
  for (;; from += d, t += td) {
    r = (r&7)+from[0]; if (r>255) r = 255;
    g = (g&3)+from[1]; if (g>255) g = 255;
    b = (b&7)+from[2]; if (b>255) b = 255;
    OUTASSIGN(((r&0xf8)<<8) + ((g&0xfc)<<3) + (b>>3));
    if (!--w) break;
  }
  ri = r; gi = g; bi = b;
}

static void rgb_to_565(const uchar *from, uchar *to, int w) {
  rgb_to_565d(from, to, w, 3);
}

static void rgba_to_565(const uchar *from, uchar *to, int w) {
  rgb_to_565d(from, to, w, 4);
}

static void argb32_to_565(const uchar *from, uchar *to, int w) {
#if WORDS_BIGENDIAN
  rgb_to_565d(from+1, to, w, 4);
#else
  OUTTYPE *t = (OUTTYPE *)to;
  int d, td;
  if (dir) {
    dir = 0;
    from = from+(w-1)*4;
    t = t+(w-1)*OUTSIZE;
    d = -4;
    td = -OUTSIZE;
  } else {
    dir = 1;
    d = 4;
    td = OUTSIZE;
  }
  int r=ri, g=gi, b=bi;
  for (;; from += d, t += td) {
    r = (r&7)+from[2]; if (r>255) r = 255;
    g = (g&3)+from[1]; if (g>255) g = 255;
    b = (b&7)+from[0]; if (b>255) b = 255;
    OUTASSIGN(((r&0xf8)<<8) + ((g&0xfc)<<3) + (b>>3));
    if (!--w) break;
  }
  ri = r; gi = g; bi = b;
#endif
}

////////////////////////////////////////////////////////////////
// 32bit TrueColor converters on a 32 or 64-bit machine:
// I have only seen rgbx32 and xrgb32 orders, or the byte-swapped
// version of them, which is done by flipping the byte order in Image

static void mono_to_32(const uchar *from, uchar *to, int w) {
  // go backwards so from and to can be the same buffer:
  U32* t = (U32*)to+w;
  from += w;
  while (t > (U32*)to)
    *--t = *--from * 0x1010101U;
}

static void rgb_to_rgbx(const uchar *from, uchar *to, int w) {
  // go backwards so from and to can be the same buffer:
  U32* t = (U32*)to+w;
  from += 3*(w-1);
  while (t > (U32*)to) {
    *--t = (from[0]<<24)+(from[1]<<16)+(from[2]<<8);
    from -= 3;
  }
}

#if !WORDS_BIGENDIAN
static void rgba_to_rgbx(const uchar *from, uchar *to, int w) {
  U32* t = (U32*)to;
  for (;; from += 4) {
    *t++ = (from[0]<<24)+(from[1]<<16)+(from[2]<<8)+(from[3]);
    if (!--w) break;
  }
}
#endif

static void argb32_to_rgbx(const uchar *from, uchar *to, int w) {
  U32* t = (U32*)to;
  const U32* f = (U32*)from;
  unsigned x;

  for (;;) {
    x = *f++;
    *t++ = (x<<8)|((x>>24)&255);
    if (!--w) break;
  }
}

////////////////////////////////////////////////////////////////

static void rgb_to_xrgb(const uchar *from, uchar *to, int w) {
  // go backwards so from and to can be the same buffer:
  U32* t = (U32*)to+w;
  from += 3*(w-1);
  while (t > (U32*)to) {
    *--t = (from[0]<<16)+(from[1]<<8)+(from[2]);
    from -= 3;
  }
}

static void rgba_to_xrgb(const uchar *from, uchar *to, int w) {
  U32* t = (U32*)to;
  for (;; from += 4) {
    *t++ = (from[0]<<16)+(from[1]<<8)+(from[2])+(from[3]<<24);
    if (!--w) break;
  }
}

static void direct_32(const uchar *from, uchar *to, int w) {
  if (from != to) memcpy(to, from, w*4);
}

////////////////////////////////////////////////////////////////
// alpha pre-processing:
// These assumme that argb32 is the best intermediate form for image.

// For old Xlib, we build a binary alpha mask, opaque for any non-zero
// alpha, and guess a mixture with the current getbgcolor() is acceptable:

static char* alphapointer;
static char amask;

static U32* getbuffer(int w) {
  static U32* b = 0;
  static int len = 0;
  if (w > len) {
    delete[] b;
    len = 2*len;
    if (len < w) len = w;
    b = new U32[len];
  }
  return b;
}

static void mask_converter(const uchar* from, uchar* to, int w)
{
  uchar bg[3];
  Color col = getbgcolor(); if (!col) col = GRAY75;
  split_color(col, bg[0],bg[1],bg[2]);
  uchar fg[3];
  col = getcolor(); if (!col) col = BLACK;
  split_color(col, fg[0],fg[1],fg[2]);

  U32* buffer = getbuffer(w);
  U32* bp = buffer;
  char* ap = alphapointer;
  char aaccum = 0;
  char amask = ::amask;

  uchar c;
  uchar r,g,b;

  for (int i = 0; i < w; i++) {
    c = *from++;
    if (c==255) {
      *bp++ = 0;
    } else {
      aaccum |= amask;
      if (c) {
	r = (c*bg[0]+(255-c)*fg[0])>>8;
	g = (c*bg[1]+(255-c)*fg[1])>>8;
	b = (c*bg[2]+(255-c)*fg[2])>>8;
	*bp++ = (r<<16)|(g<<8)|b;
      } else {
	*bp++ = (fg[0]<<16)|(fg[1]<<8)|fg[2];
      }
    }
    if (amask & 0x80) {
      *ap++ = aaccum; aaccum = 0; amask = 1;
    } else {
      amask <<= 1;
    }
  }
  if (amask != 1) *ap = aaccum;
  converter[RGB32]((const uchar*)buffer, to, w);
}

static void rgba_converter(const uchar* from, uchar* to, int w) {
  uchar bg[3];
  Color c = getbgcolor(); if (!c) c = GRAY75;
  split_color(c, bg[0],bg[1],bg[2]);

  U32* buffer = getbuffer(w);
  U32* bp = buffer;
  char* ap = alphapointer;
  char aaccum = 0;
  char amask = ::amask;

  uchar r,g,b,a;

  for (int i = 0; i < w; i++) {
    r = *from++;
    g = *from++;
    b = *from++;
    a = *from++;
    if (!a) {
      *bp++ = 0;
    } else {
      aaccum |= amask;
      if (a != 255) {
	r = r+((bg[0]*(255-a))>>8);
	g = g+((bg[1]*(255-a))>>8);
	b = b+((bg[2]*(255-a))>>8);
      }
      *bp++ = (r<<16)|(g<<8)|b;
    }
    if (amask & 0x80) {
      *ap++ = aaccum; aaccum = 0; amask = 1;
    } else {
      amask <<= 1;
    }
  }
  if (amask != 1) *ap = aaccum;
  converter[RGB32]((const uchar*)buffer, to, w);
}

static void argb32_converter(const uchar* from, uchar* to, int w) {
  uchar bg[3];
  Color col = getbgcolor(); if (!col) col = GRAY75;
  split_color(col, bg[0],bg[1],bg[2]);

  U32* buffer = getbuffer(w);
  U32* bp = buffer;
  const U32* f = (const U32*)from;
  char* ap = alphapointer;
  char aaccum = 0;
  char amask = ::amask;

  U32 c;
  uchar r,g,b,a;

  for (int i = 0; i < w; i++) {
    c = *f++;
    a = c>>24;
    if (!a) {
      *bp++ = 0;
    } else {
      aaccum |= amask;
      if (a == 255) {
	*bp++ = c;
      } else {
	r = (c>>16)+((bg[0]*(255-a))>>8);
	g = (c>>8)+((bg[1]*(255-a))>>8);
	b = c+((bg[2]*(255-a))>>8);
	*bp++ = (r<<16)|(g<<8)|b;
      }
    }
    if (amask & 0x80) {
      *ap++ = aaccum; aaccum = 0; amask = 1;
    } else {
      amask <<= 1;
    }
  }
  if (amask != 1) *ap = aaccum;
  converter[RGB32]((const uchar*)buffer, to, w);
}

static void rgbm_converter(const uchar* from, uchar* to, int w) {
  uchar bg[3];
  Color c = getbgcolor(); if (!c) c = GRAY75;
  split_color(c, bg[0],bg[1],bg[2]);

  U32* buffer = getbuffer(w);
  U32* bp = buffer;
  char* ap = alphapointer;
  char aaccum = 0;
  char amask = ::amask;

  uchar r,g,b,a;

  for (int i = 0; i < w; i++) {
    r = *from++;
    g = *from++;
    b = *from++;
    a = *from++;
    if (!a) {
      *bp++ = 0;
    } else {
      aaccum |= amask;
      if (a != 255) {
	r = (r*a+bg[0]*(255-a))>>8;
	g = (g*a+bg[1]*(255-a))>>8;
	b = (b*a+bg[2]*(255-a))>>8;
      }
      *bp++ = (r<<16)|(g<<8)|b;
    }
    if (amask & 0x80) {
      *ap++ = aaccum; aaccum = 0; amask = 1;
    } else {
      amask <<= 1;
    }
  }
  if (amask != 1) *ap = aaccum;
  converter[RGB32]((const uchar*)buffer, to, w);
}

static void mrgb32_converter(const uchar* from, uchar* to, int w) {
  uchar bg[3];
  Color col = getbgcolor(); if (!col) col = GRAY75;
  split_color(col, bg[0],bg[1],bg[2]);

  U32* buffer = getbuffer(w);
  U32* bp = buffer;
  const U32* f = (const U32*)from;
  char* ap = alphapointer;
  char aaccum = 0;
  char amask = ::amask;

  U32 c;
  uchar r,g,b,a;

  for (int i = 0; i < w; i++) {
    c = *f++;
    a = c>>24;
    if (!a) {
      *bp++ = 0;
    } else {
      aaccum |= amask;
      if (a == 255) {
	*bp++ = c;
      } else {
	r = (((c>>16)&255)*a+bg[0]*(255-a))>>8;
	g = (((c>>8)&255)*a+bg[1]*(255-a))>>8;
	b = ((c&255)*a+bg[2]*(255-a))>>8;
	*bp++ = (r<<16)|(g<<8)|b;
      }
    }
    if (amask & 0x80) {
      *ap++ = aaccum; aaccum = 0; amask = 1;
    } else {
      amask <<= 1;
    }
  }
  if (amask != 1) *ap = aaccum;
  converter[RGB32]((const uchar*)buffer, to, w);
}

////////////////////////////////////////////////////////////////

static XImage i;	// this is reused to draw images
static int bytes_per_pixel;
static int scanline_add;
static int scanline_mask;

#if USE_XFT
XRenderPictFormat* fl_rgba_xrender_format;
extern bool fl_get_invert_matrix(XTransform&);
extern bool fl_trivial_transform();

::Picture p;
XWindow prevsource;

#define XRENDER_SAMPLING_BUG 1
#define XRENDER_MASK_BROKEN 1

#if XRENDER_MASK_BROKEN
static void rgbm_to_argb32(const uchar* from, uchar* to, int w) {
  U32* t = (U32*)to+w;
  from += 4*w;
  uchar a;

  while (t > (U32*)to) {
    from -= 4;
    a = from[3];
    *--t = (a<<24) | (((from[0]*a)<<8)&0xff0000) | ((from[1]*a)&0xff00) | ((from[2]*a)>>8);
  }
}

static void mrgb32_to_argb32(const uchar* from, uchar* to, int w) {
  U32* t = (U32*)to+w;
  from += 4*w;
  U32 v;
  uchar a;

  while (t > (U32*)to) {
    from -= 4;
    v = *(U32*)from;
    a = v>>24;
    *--t = (v&0xff000000) |
      ((((v&0xff0000)*a)>>8) & 0xff0000) |
      ((((v&0xff00)*a)>>8) & 0xff00) |
      ((((v&0xff)*a)>>8) & 0xff);
  }
}
#endif

void fl_xrender_draw_image(XWindow source, fltk::PixelType type,
                           const fltk::Rectangle& from,
                           const fltk::Rectangle& to)
{
  XTransform xtransform;
  if (!fl_get_invert_matrix(xtransform)) return; // give up if we can't invert
  int x,y,r,b; // box to draw
  if (!fl_trivial_transform()) {
    float X,Y,R,B,tx,ty;
    tx = to.x(); ty = to.y(); transform(tx, ty);
    X = R = tx; Y = B = ty;
    tx = to.r(); ty = to.b(); transform(tx, ty);
    if (tx < X) X = tx; else R = tx;
    if (ty < Y) Y = ty; else B = ty;
    if (xtransform.matrix[0][1]||xtransform.matrix[1][0]) {
      tx = to.x(); ty = to.b(); transform(tx, ty);
      if (tx < X) X = tx; else if (tx > R) R = tx;
      if (ty < Y) Y = ty; else if (ty > B) B = ty;
      tx = to.r(); ty = to.y(); transform(tx, ty);
      if (tx < X) X = tx; else if (tx > R) R = tx;
      if (ty < Y) Y = ty; else if (ty > B) B = ty;
    }
    x = int(floorf(X));
    y = int(floorf(Y));
    r = int(ceilf(R));
    b = int(ceilf(B));
  } else {
    x = to.x(); y = to.y(); transform(x,y);
    r = x+to.w(); b = y+to.h();
  }
  if (to.w() != from.w() || to.h() != from.h()) {
    const float scalex = float(from.w())/to.w();
    const float scaley = float(from.h())/to.h();
    for (int i=0;i<3;i++) {
      xtransform.matrix[0][i] = int(scalex*xtransform.matrix[0][i]);
      xtransform.matrix[1][i] = int(scaley*xtransform.matrix[1][i]);
    }
    xtransform.matrix[0][2] += XDoubleToFixed(from.x()-to.x()*scalex);
    xtransform.matrix[1][2] += XDoubleToFixed(from.y()-to.y()*scaley);
  } else {
    xtransform.matrix[0][2] += XDoubleToFixed(from.x()-to.x());
    xtransform.matrix[1][2] += XDoubleToFixed(from.y()-to.y());
  }
  if (XRENDER_SAMPLING_BUG) {
    // Adjust due to XRender sampling pixels at the corners:
    xtransform.matrix[0][2] +=
      (xtransform.matrix[0][0]+xtransform.matrix[0][1]-0x10000)>>1;
    xtransform.matrix[1][2] +=
      (xtransform.matrix[1][0]+xtransform.matrix[1][1]-0x10000)>>1;
  }
  if (source != prevsource) {
    prevsource = source;
    if (p) XRenderFreePicture(xdisplay, p);
    p = XRenderCreatePicture(xdisplay, source, fl_rgba_xrender_format, 0, 0);
    XRenderSetPictureFilter(xdisplay, p, "best", 0, 0);
  }
  XRenderSetPictureTransform(xdisplay, p, &xtransform);
  switch (type) {
  case MASK: {
    XftColor color;
    {color.pixel = current_xpixel;
    uchar r,g,b; split_color(getcolor(), r,g,b);
    color.color.red   = r*0x101;
    color.color.green = g*0x101;
    color.color.blue  = b*0x101;
    color.color.alpha = 0xffff;}
    ::Picture solid = XftDrawSrcPicture(xftc, &color);
    XRenderComposite(xdisplay, PictOpOver,
		     solid, p, XftDrawPicture(xftc), // src, mask, dest
		     x, y, // src xy (in destination space!)
		     x, y, // mask xy
		     x, y, r-x, b-y); // rectangle to fill
    break;}
  case RGBM:
  case MRGB32:
#if !XRENDER_MASK_BROKEN
    // This *should* do unpremultiplied by copying through mask.
    // Does not appear to work, sigh...
    XRenderComposite(xdisplay, PictOpSrc,
		     p, p, XftDrawPicture(xftc), // src, mask, dest
		     x, y, // src xy (in destination space!)
		     x, y, // mask xy
		     x, y, r-x, b-y); // rectangle to fill
    break;
#else
    // fall through...
#endif
  case RGBA:
  case ARGB32:
    XRenderComposite(xdisplay, PictOpOver,
		     p, 0, XftDrawPicture(xftc), // src, mask, dest
		     x, y, // src xy (in destination space!)
		     0, 0, // mask xy
		     x, y, r-x, b-y); // rectangle to fill
    break;
  default:
    XRenderComposite(xdisplay, PictOpSrc,
		     p, 0, XftDrawPicture(xftc), // src, mask, dest
		     x, y, // src xy (in destination space!)
		     0, 0, // mask xy
		     x, y, r-x, b-y); // rectangle to fill
  }
}

static void (*xrender_converter[9])(const uchar *from, uchar *to, int w);
static XImage xrenderi;

static void mask_to_32(const uchar *from, uchar *to, int w) {
  // go backwards so from and to can be the same buffer:
  U32* t = (U32*)to+w;
  from += w;
  while (t > (U32*)to)
    *--t = ~(*--from * 0x1010101U);
}

#endif

static void figure_out_visual() {

  xpixel(BLACK); // make sure figure_out_visual in color.cxx is called

#if USE_XSHM
  if (!getenv("NO_XSHM")) {int major, minor;
  use_xshm = XShmQueryVersion(xdisplay, &major, &minor, &use_xshm_pixmaps);
  if (use_xshm_pixmaps)
    use_xshm_pixmaps = XShmPixmapFormat(xdisplay)==ZPixmap;
  }
#endif

#if USE_XFT
  // See if XRender is going to work.
  // If so all image types are converted to ARGB32 format,
  // put in a pixmap, and Xrender is used to draw them.
  // The older direct conversion is still done for opaque formats
  // sent through drawimage() if there is no scaling or rotation,
  // this is to avoid slowness with XRender.
//   {int major, minor; major = minor = -1;
//   int status = XRenderQueryVersion(xdisplay, &major, &minor);
//   printf("status = %d, version = %d.%d\n", status, major, minor);}
  fl_rgba_xrender_format =
    XRenderFindStandardFormat(xdisplay, PictStandardARGB32);
  if (fl_rgba_xrender_format) {
    // fill in the xrender converters
    xrender_converter[MASK] = mask_to_32;
    xrender_converter[MONO] = mono_to_32;
    xrender_converter[RGBx] = rgba_to_xrgb;
    xrender_converter[RGB] = rgb_to_xrgb;
    xrender_converter[RGBA] = rgba_to_xrgb;
    xrender_converter[RGB32] = direct_32;
    xrender_converter[ARGB32] = direct_32;
#if XRENDER_MASK_BROKEN
    xrender_converter[RGBM] = rgbm_to_argb32;
    xrender_converter[MRGB32] = mrgb32_to_argb32;
#else
    xrender_converter[RGBM] = rgba_to_xrgb;
    xrender_converter[MRGB32] = direct_32;
#endif
    xrenderi.format = ZPixmap;
    xrenderi.depth = 32;
    xrenderi.bits_per_pixel = 32;
    xrenderi.byte_order = WORDS_BIGENDIAN;
  } else
#endif
    {converter[MASK] = mask_converter;
    converter[RGBA] = rgba_converter;
    converter[ARGB32] = argb32_converter;
    converter[RGBM] = rgbm_converter;
    converter[MRGB32] = mrgb32_converter;
    }

  i.format = ZPixmap;
  i.depth = xvisual->depth;
//i.bitmap_unit = 8;
//i.bitmap_bit_order = MSBFirst;
//i.bitmap_pad = 8;

  static XPixmapFormatValues *pfvlist;
  static int NUM_pfv;
  if (!pfvlist) pfvlist = XListPixmapFormats(xdisplay,&NUM_pfv);
  XPixmapFormatValues *pfv;
  for (pfv = pfvlist; pfv < pfvlist+NUM_pfv; pfv++)
    if (pfv->depth == xvisual->depth) break;
  i.bits_per_pixel = pfv->bits_per_pixel;

  if (i.bits_per_pixel & 7) bytes_per_pixel = 0; // produce fatal error
  else bytes_per_pixel = i.bits_per_pixel/8;

  unsigned int n = pfv->scanline_pad/8;
  if (pfv->scanline_pad & 7 || (n&(n-1)))
    fatal("Can't do scanline_pad of %d",pfv->scanline_pad);
  scanline_add = n-1;
  scanline_mask = -n;

#if USE_COLORMAP
  if (bytes_per_pixel == 1) {
    i.byte_order = ImageByteOrder(xdisplay);
    converter[MONO] = mono_to_8;
    converter[RGB] = rgb_to_8;
    converter[RGBx] = rgba_to_8;
    converter[RGB32] = argb32_to_8;
    return;
  }
  if (!xvisual->red_mask)
    fatal("Can't do %d bits_per_pixel colormap",i.bits_per_pixel);
#endif

  // otherwise it is a TrueColor visual:

  int rs = fl_redshift;
  int gs = fl_greenshift;
  int bs = fl_blueshift;

  switch (bytes_per_pixel) {

  case 2:
    // All 16-bit TrueColor visuals are supported on any machine with
    // 24 or more bits per integer.
#ifdef U16
    ::i.byte_order = WORDS_BIGENDIAN;
#else
    ::i.byte_order = 1;
#endif
    if (rs == 11 && gs == 6 && bs == 0 && fl_extrashift == 3) {
      converter[MONO] = mono_to_565;
      converter[RGB] = rgb_to_565;
      converter[RGBx] = rgba_to_565;
      converter[RGB32] = argb32_to_565;
    } else {
      converter[MONO] = mono_to_16;
      converter[RGB] = rgb_to_16;
      converter[RGBx] = rgba_to_16;
      converter[RGB32] = argb32_to_16;
    }
    break;

  case 4:
    converter[MONO] = mono_to_32;
    if (rs == 0 && gs == 8 && bs == 16) {
      ::i.byte_order = !WORDS_BIGENDIAN;
#if USE_XSHM
      use_xshm_pixmaps = false;
#endif
      goto RGBX;
    }
    if (rs == 24 && gs == 16 && bs == 8) {
      ::i.byte_order = WORDS_BIGENDIAN;
    RGBX:
      converter[RGB] = rgb_to_rgbx;
#if WORDS_BIGENDIAN
      converter[RGBx] = direct_32;
#else
      converter[RGBx] = rgba_to_rgbx;
#endif
      converter[RGB32] = argb32_to_rgbx;
      break;
    }
    if (rs == 8 && gs == 16 && bs == 24) {
      ::i.byte_order = !WORDS_BIGENDIAN;
#if USE_XSHM
      use_xshm_pixmaps = false;
#endif
      goto XRGB;
    }
    if (rs == 16 && gs == 8 && bs == 0) {
      ::i.byte_order = WORDS_BIGENDIAN;
    XRGB:
      converter[RGB] = rgb_to_xrgb;
      converter[RGBx] = rgba_to_xrgb;
      converter[RGB32] = direct_32;
      break;
    }
    // else fallthrough to failure case:
  default:
    fatal("Can't do %d-bit 0x%x 0x%x 0x%x visual",
	  i.bits_per_pixel, xvisual->red_mask,
	  xvisual->green_mask, xvisual->blue_mask);
  }
  //printf("Use xshm %d\n", use_xshm_pixmaps);
}

////////////////////////////////////////////////////////////////
// The Image object

#if USE_XSHM
static int syncnumber = 1;

static int xerror_handler(Display* d, XErrorEvent* e) {
  use_xshm_pixmaps = false;
  return 0;
}
#endif

struct fltk::Picture {
  int w, h, linedelta;
  Bool draw_target; // whether the image is used with draw_into()
  unsigned long n; // bytes used
  uchar* data;
  XWindow rgb;
#if USE_XSHM
  XShmSegmentInfo shminfo;
  int syncro;
#endif
  uchar* linebuffer;
  XWindow alpha;        // binary alpha for non-XRender
  char* alphabuffer;    // binary alpha local source

  Picture(int w, int h, int depth, int ld, Bool draw_target=false) {
    this->w = w;
    this->h = h;
    this->linedelta = ld;
    this->draw_target = draw_target;
    n = (ld*h+3)&-4;
    linebuffer = 0; alpha = 0; alphabuffer = 0;
#if USE_XSHM
    syncro = 0;
    if (use_xshm_pixmaps) {
      shminfo.shmid = shmget(IPC_PRIVATE, n, IPC_CREAT|0777);
      if (shminfo.shmid != -1) {
        shminfo.shmaddr = (char*)shmat(shminfo.shmid, 0, 0);
        if (shminfo.shmaddr) {
          shminfo.readOnly = False;
          if (XShmAttach(xdisplay, &shminfo)) {
            data = (uchar*)shminfo.shmaddr;
            static bool beenhere;
            if (beenhere) {
              rgb = XShmCreatePixmap(xdisplay, RootWindow(xdisplay,xscreen),
                                     shminfo.shmaddr, &shminfo,
                                     w, h, depth);
              return;
            }
            beenhere = true;
            // The first time, we will do an XSync and detect if it throws
            // an error. This seems to be the only way to see if XShm is
            // going to work, it won't work on remote displays but amazingly
            // enough no api is provided to tell you that!
            int (*f)(Display*,XErrorEvent*) = XSetErrorHandler(xerror_handler);
            rgb = XShmCreatePixmap(xdisplay, RootWindow(xdisplay,xscreen),
                                   shminfo.shmaddr, &shminfo,
                                   w, h, depth);
            XSync(xdisplay,false);
            XSetErrorHandler(f);
            if (use_xshm_pixmaps) return; // the xerror_handler did not get called!
            // Okay, clean up all the failed stuff we created,
            // and continue with non-Xshm code:
            XShmDetach(xdisplay, &shminfo);
          }
          shmdt(shminfo.shmaddr);
        }
        shmctl(shminfo.shmid, IPC_RMID, 0);
      }
    }
    shminfo.shmid = -1;
    shminfo.shmaddr = 0;
#endif
    rgb = XCreatePixmap(xdisplay, RootWindow(xdisplay,xscreen),
                        w, h, depth);
    data = (uchar*)(new U32[n/4]);
  }

  Picture(int) { // special constructor for xbmImage
    linebuffer = 0; alpha = 0; alphabuffer = 0;
#if USE_XSHM
    shminfo.shmid = -1;
    shminfo.shmaddr = 0;
#endif
    data = 0;
    rgb = 0;
  }

  bool xshm() const {
#if USE_XSHM
    return shminfo.shmaddr;
#else
    return 0;
#endif
  }

  void sync() {
#if USE_XSHM
    if (syncro == syncnumber) {++syncnumber; XSync(xdisplay,false);}
#endif
  }

  ~Picture() {
    sync();
    delete[] alphabuffer;
    delete[] (U32*)linebuffer;
    if (xdisplay) {
      if (alpha) XFreePixmap(xdisplay, alpha);
      if (rgb) XFreePixmap(xdisplay, rgb);
    }
#if USE_XSHM
    if (shminfo.shmaddr) {
      if (xdisplay) XShmDetach(xdisplay, &shminfo);
      shmdt(shminfo.shmaddr); data = 0;
      shmctl(shminfo.shmid, IPC_RMID, 0);
    }
#endif
    delete[] (U32*)data;
  }

  void makealphabuffer() {
    alphabuffer = new char[((w+7)>>3)*h];
  }

};

int Image::buffer_width() const {
  if (picture) return picture->w;
  return w();
}

int Image::buffer_height() const {
  if (picture) return picture->h;
  return h();
}

int Image::buffer_linedelta() const {
  if (picture) return picture->linedelta;
  if (!bytes_per_pixel) figure_out_visual();
#if USE_XFT
  if (fl_rgba_xrender_format) return w()*4;
#endif
  return (w()*bytes_per_pixel+scanline_add)&scanline_mask;
}

int Image::buffer_depth() const {
  if (!bytes_per_pixel) figure_out_visual();
#if USE_XFT
  if (fl_rgba_xrender_format) return 4;
#endif
  return bytes_per_pixel;
}

void Image::set_forceARGB32() {
  flags |= FORCEARGB32;
  // NYI!!! (though it does nothing if USE_XFT is on)
}

void Image::clear_forceARGB32() {
  flags &= ~FORCEARGB32;
  // NYI!!! (though it does nothing if USE_XFT is on)
}

PixelType Image::buffer_pixeltype() const {
  if (!bytes_per_pixel) figure_out_visual();
#if USE_XFT
  if (fl_rgba_xrender_format) return ARGB32;
#endif
  return PixelType(bytes_per_pixel); // not really right...
}

unsigned long Image::mem_used() const {
  if (picture) return picture->n;
  return 0;
}

const uchar* Image::buffer() const {
  if (picture) return picture->data;
  return const_cast<Image*>(this)->buffer();
}

uchar* Image::buffer() {
  if (picture) {
    picture->sync();
    return picture->data;
  }
  if (w_ <= 0 || h_ <= 0) return 0;
  if (!bytes_per_pixel) figure_out_visual();
  int depth;
  int ld;
#if USE_XFT
  if (fl_rgba_xrender_format) {
    ld = w_*4;
    depth = 32;
  } else
#endif
    {ld = ((w_*bytes_per_pixel+scanline_add)&scanline_mask);
    depth = xvisual->depth;}
  picture = new Picture(w_, h_, depth, ld);
  memused_ += picture->n;
#if USE_XFT
  if (!fl_rgba_xrender_format)
#endif
    if (pixeltype_==MASK || pixeltype_==RGBA || pixeltype_>=ARGB32)
      picture->makealphabuffer();
  return picture->data;
}

void Image::destroy() {
  if (!picture) return;
  if (picture->n > memused_) memused_ -= picture->n; else memused_ = 0;
  delete picture;
  picture = 0;
  flags &= ~FETCHED;
}

void Image::setpixeltype(PixelType p) {
  pixeltype_ = p;
#if USE_XFT
  if (fl_rgba_xrender_format) return;
#endif
  // we need to allocate the alphabuffer if it is missing
  if (picture && (p==MASK || p==RGBA || p>=ARGB32) && !picture->alphabuffer)
    picture->makealphabuffer();
}

void Image::setsize(int w, int h) {
  if (picture && (w > picture->w || h > picture->h)) destroy();
  w_ = w;
  h_ = h;
}

uchar* Image::linebuffer(int y) {
  buffer();
#if USE_XFT
  if (fl_rgba_xrender_format) return picture->data+y*picture->linedelta;
#endif
  if (bytes_per_pixel >= 4) return picture->data+y*picture->linedelta;
  if (!picture->linebuffer)
    picture->linebuffer = (uchar*)(new U32[picture->w]);
  return picture->linebuffer;
}

void Image::setpixels(const uchar* buf, int y) {
  buffer();
  flags &= ~COPIED;
  uchar* to = picture->data+y*picture->linedelta;
  void (*conv)(const uchar *from,uchar *to,int w);
#if USE_XFT
  if (fl_rgba_xrender_format) {
    conv = xrender_converter[pixeltype_];
    conv(buf, to, width());
    return;
  }
#endif
  alphapointer = picture->alphabuffer+y*((picture->w+7)>>3);
  amask = 1;
  conv = converter[pixeltype_];
  conv(buf, to, width());
}

void Image::setpixels(const uchar* buf, const Rectangle& r, int linedelta)
{
  if (r.empty()) return;
  buffer();
  flags &= ~COPIED;
  uchar* to = picture->data+r.y()*picture->linedelta+r.x()*buffer_depth();
  void (*conv)(const uchar *from,uchar *to,int w);
#if USE_XFT
  if (fl_rgba_xrender_format) {
    conv = xrender_converter[pixeltype_];
    // see if we can do it all at once:
    if (r.w() == picture->w && (r.h()==1 || linedelta == picture->linedelta)) {
      conv(buf, to, r.w()*r.h());
    } else {
      for (int y = 0; y < r.h(); y++) {
        conv(buf, to, r.w());
        buf += linedelta;
        to += picture->linedelta;
      }
    }
    return;
  }
#endif
  conv = converter[pixeltype_];
  if (picture->alphabuffer) {
    alphapointer = picture->alphabuffer+r.y()*((picture->w+7)>>3)+(r.x()>>3);
    amask = 1<<(r.x()&7);
    for (int y = 0; y < r.h(); y++) {
      conv(buf, to, r.w());
      buf += linedelta;
      to += picture->linedelta;
      alphapointer += ((w()+7)>>3);
    }
  } else if (r.w()==picture->w && (r.h()==1 || linedelta==picture->linedelta)) {
    conv(buf, to, r.w()*r.h());
  } else {
    for (int y = 0; y < r.h(); y++) {
      conv(buf, to, r.w());
      buf += linedelta;
      to += picture->linedelta;
    }
  }
}

void Image::fetch_if_needed() const {
    Image* thisimage = const_cast<Image*>(this);
#if USE_XFT
  if (!fl_rgba_xrender_format)
#endif
    // kludge to refetch if fg/bg colors change
    if (pixeltype_==MASK) {
      U32 fg = (fltk::get_color_index(fltk::getcolor())+
                fltk::get_color_index(fltk::getbgcolor()))&0xffffff00;
      if ((flags & 0xffffff00) != fg)
        thisimage->flags = (flags&0xff&~FETCHED)|fg;
    }
  if (!(flags&FETCHED)) {
    thisimage->fetch();
    thisimage->flags |= FETCHED;
  }
}

void fl_restore_clip(); // in clip.cxx

void Image::draw(const fltk::Rectangle& from, const fltk::Rectangle& to) const {
  fetch_if_needed();
  if (!picture) {fillrect(to); return;}

  if (!(flags & COPIED)) {
#if USE_XSHM
    if (picture->xshm())
      picture->syncro = syncnumber;
    else
#endif
    if (picture->rgb) {
#if USE_XFT
      XImage& i = fl_rgba_xrender_format ? xrenderi : ::i;
#endif
      i.width = w();
      i.height = h();
      i.data = (char*)picture->data;
      i.bytes_per_line = picture->linedelta;
      static GC copygc;
      if (!copygc) copygc = XCreateGC(xdisplay, picture->rgb, 0, 0);
      XPutImage(xdisplay, picture->rgb, copygc, &i, 0,0,0,0,w(),h());
    }
    if (picture->alpha)
      XFreePixmap(xdisplay, picture->alpha);
    if (picture->alphabuffer)
      picture->alpha =
        XCreateBitmapFromData(xdisplay, xwindow, picture->alphabuffer,
                              (picture->w+7)&-8, h());
    else
      picture->alpha = 0;
    ((Image*)this)->flags |= COPIED;
  }
#if USE_XFT
  if (fl_rgba_xrender_format && picture->rgb && !picture->draw_target) {
    fl_xrender_draw_image(picture->rgb, pixeltype_, from, to);
    return;
  }
#endif
  // XLib version:
  // unfortunately scaling does not work, so I just center and clip
  // to the transformed rectangle.
  // This is the rectangle I want to fill:
  Rectangle r2; transform(to,r2);
  // Center the image in that rectangle:
  Rectangle r1(r2,from.w(),from.h());
  // now figure out what area we will draw:
  Rectangle r(r1);
  if (r.w() >= r2.w()) {r.x(r2.x()); r.w(r2.w());}
  if (r.h() >= r2.h()) {r.y(r2.y()); r.h(r2.h());}
  // We must clip it because otherwise we can't do the alpha:
  if (!intersect_with_clip(r)) return;
  // fix for corner of from rectangle:
  r1.move(-from.x(), -from.y());
  if (picture->alpha) {
    if (picture->rgb) {
      // There seems to be no way to combine a mask with a clip region,
      // so I just replace it. This will be incorrect if the current
      // clip is not a rectangle
      XSetClipMask(xdisplay, gc, picture->alpha);
      XSetClipOrigin(xdisplay, gc, r1.x(), r1.y());
      XCopyArea(xdisplay, picture->rgb, xwindow, gc,
                r.x()-r1.x(), r.y()-r1.y(), r.w(), r.h(), r.x(), r.y());
      XSetClipOrigin(xdisplay, gc, 0, 0);
      fl_restore_clip();
    } else {
      // xbmImage
      XSetStipple(xdisplay, gc, picture->alpha);
      XSetTSOrigin(xdisplay, gc, r1.x(), r1.y());
      XSetFillStyle(xdisplay, gc, FillStippled);
      XFillRectangle(xdisplay, xwindow, gc, r.x(), r.y(), r.w(), r.h());
      XSetFillStyle(xdisplay, gc, FillSolid);
    }
  } else if (picture->rgb) {
    // RGB picture with no alpha
    XCopyArea(xdisplay, picture->rgb, xwindow, gc,
              r.x()-r1.x(), r.y()-r1.y(), r.w(), r.h(), r.x(), r.y());
  }
}

void Image::setimage(const uchar* d, PixelType p, int w, int h, int ld) {
  setsize(w,h);
  setpixeltype(p);
  // If there is no shared memory and type is compatable we could
  // instead use the data directly, see ../osx/Image.cxx for details
  setpixels(d, Rectangle(w,h), ld); flags = 0;
}

void Image::make_current() {
  // make this image the target for drawing operations
  // fl_xrender_draw_image() cannot be used in draw() apparently
  // due to incompatible depth attributes
  if (!picture) {
    // initialise
    if (w_ > 0 && h_ > 0) {
      int ld = buffer_linedelta();
      int depth = xvisual->depth;
      picture = new Picture(w_, h_, depth, ld, true);
      memused_ += picture->n;
      // the copy operation in draw() does not seem to do anything
      // in this context other than generate an xlib warning message
      ((Image*)this)->flags |= COPIED;
    } else {
      fatal("Can't draw into empty image");
    }
  }

  draw_into(picture->rgb, picture->w, picture->h);
}

////////////////////////////////////////////////////////////////
#if 0

bool xbmImage::direct_fetch() {
  if (!picture) {
    open_display();
    picture = new Picture(0);
  }
  if (picture->alpha)
    XFreePixmap(xdisplay, picture->alpha);
  picture->alpha =
    XCreateBitmapFromData(xdisplay, xwindow, (char*)array, (w()+7)&-8, h());
  flags = FETCHED|COPIED;
  return true;
}

#endif
////////////////////////////////////////////////////////////////

// drawimage() calls this to see if a direct draw will work. Returns
// true if successful, false if an Image must be used to emulate it.

static bool innards(const uchar *buf, PixelType type,
		    const Rectangle& r1,
		    int linedelta,
		    DrawImageCallback cb, void* userdata)
{
  // Any alpha requires using an Image object:
  if (type==MASK || type==RGBA || type>=ARGB32) return false;

#if USE_XFT
  // Image is needed to do XRender transforms. If XRender is not
  // working then we can't do transforms, so I'll just use this code:
  if (fl_rgba_xrender_format && !fl_trivial_transform()) return false;
#endif

#if USE_XSHM
  // use shared memory to send the image...
  // For me at least this appears slower..
  //  if (use_xshm_pixmaps) return false;
#endif

  int dx,dy,x,y,w,h;

  // because scaling is not supported, I just draw the image centered:
  // This is the rectangle I want to fill:
  Rectangle tr; transform(r1,tr);
  // Center the image in that rectangle:
  Rectangle r(tr, r1.w(), r1.h());
  // Clip image if it is bigger than destination rectangle:
  Rectangle cr(r);
  if (r1.w() >= tr.w()) {cr.x(tr.x()); cr.w(tr.w());}
  if (r1.h() >= tr.h()) {cr.y(tr.y()); cr.h(tr.h());}
  // Clip with current clip region,
  // otherwise the alpha mask will allow it to draw outside:
  if (!intersect_with_clip(cr)) return true;
  x = cr.x();
  y = cr.y();
  w = cr.w();
  h = cr.h();
  dx = x-r.x();
  dy = y-r.y();

  const int delta = depth(type);
  if (buf) buf += dx*delta + dy*linedelta;

  i.width = w;
  i.height = h;

  if (!bytes_per_pixel) figure_out_visual();
  void (*conv)(const uchar *from, uchar *to, int w) = converter[type];

  // Direct-dump RGB or BGR data if it is already laid out correctly.
  // This assummes the server will ignore the 4th byte,
  // that it works when the image is not word-aligned,
  // and it works if bytes_per_line is negative.
  // This seems to all work on my XFree86 setup
  if (buf && conv==direct_32 && !(linedelta&scanline_add)) {
    i.data = (char *)buf;
    i.bytes_per_line = linedelta;
    XPutImage(xdisplay, xwindow, gc, &i, 0, 0, x, y, w, h);
  } else {
    int blocking = h;
    static U32* buffer;	// our storage, always word aligned
    static long buffer_size;
    int linesize = ((w*bytes_per_pixel+scanline_add)&scanline_mask)/4;
    {int size = linesize*h;
    const int MAXBUFFER = 0x40000; // 256k
    if (size > MAXBUFFER) {
      size = MAXBUFFER;
      blocking = MAXBUFFER/linesize;
    }
    if (size > buffer_size) {
      delete[] buffer;
      buffer_size = size;
      buffer = new U32[size];
    }}
    i.data = (char *)buffer;
    i.bytes_per_line = linesize*4;
    if (buf) {
      for (int j=0; j<h; ) {
	U32 *to = buffer;
	int k;
	for (k = 0; j<h && k<blocking; k++, j++) {
	  conv(buf, (uchar*)to, w);
	  buf += linedelta;
	  to += linesize;
	}
        XPutImage(xdisplay, xwindow, gc, &i, 0, 0, x, y+j-k, w, k);
      }
    } else {
      U32* linebuf = new U32[(r1.w()*delta+3)/4];
      for (int j=0; j<h; ) {
	U32* to = buffer;
	int k;
	for (k = 0; j<h && k<blocking; k++, j++) {
	  const uchar* ret = cb(userdata, dx, dy+j, w, (uchar*)linebuf);
	  conv(ret, (uchar*)to, w);
	  to += linesize;
	}
        XPutImage(xdisplay, xwindow, gc, &i, 0, 0, x, y+j-k, w, k);
      }
      delete[] linebuf;
    }
  }
  return true;
}

//
// End of "$Id: drawimage.cxx 1399 2006-08-11 02:15:20Z spitzak $"
//
