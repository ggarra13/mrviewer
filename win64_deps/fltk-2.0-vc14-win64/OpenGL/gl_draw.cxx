// "$Id: gl_draw.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// Functions from <fltk/gl.h>
// See also GlWindow and gl_start.C

#include <config.h>
#if HAVE_GL

#include <fltk/draw.h>
#include <fltk/Font.h>
#include <fltk/gl.h>
#include <fltk/utf.h>
#include <fltk/math.h>
#include <fltk/string.h>
#include "GlChoice.h"

#if USE_XFT && !MAKEDEPEND
# define Window XWindow
# include <X11/Xft/Xft.h>
# include FT_GLYPH_H
# undef Window
static void glXUseXftFont(XftFont*, unsigned listbase);
# define TEXTURES 1
#else
# define TEXTURES 0
#endif

using namespace fltk;

extern GLContext fl_current_glcontext;
FL_API unsigned fl_font_opengl_id();
FL_API void fl_set_font_opengl_id(unsigned v);
#if TEXTURES
FL_API unsigned fl_font_opengl_texture();
FL_API void fl_set_font_opengl_texture(unsigned);
static unsigned texture; // current texture
static float scalefactor = 1; // current scale factor
#endif

/**
  Make the current OpenGL font (as used by gldrawtext()) be as
  similar as possible to an FLTK Font. Currently the font is
  aliased except on X.
*/
void fltk::glsetfont(fltk::Font* font, float size) {
  if (!fl_current_glcontext) {
    setfont(font, size); // necessary so measure() works
    return;
  }
#if TEXTURES
  // figure out the size we will scale from.
  // This relies of Xft not choosing too many different fonts between 4 & 25:
  float tsize;
  if (size < 4) {tsize = 4; scalefactor = size/4;}
  else if (size < 25) {tsize = size; scalefactor = 1;}
  else {tsize = 48; scalefactor = size/48;}
#else
  float tsize = size;
#endif
  setfont(font, tsize);
  unsigned listbase = fl_font_opengl_id();
  if (!listbase) {
    listbase = glGenLists(256);
    fl_set_font_opengl_id(listbase);
#if USE_XFT
    glXUseXftFont(xftfont(), listbase);
#elif USE_X11
    XFontStruct* current_xfont = xfont();
    int base = current_xfont->min_char_or_byte2;
    int last = current_xfont->max_char_or_byte2;
    if (last > 255) last = 255;
    int size = last-base+1;
    glXUseXFont(current_xfont->fid, base, size, listbase+base);
#elif defined(_WIN32)
    int base = textmetric()->tmFirstChar;
    int last = textmetric()->tmLastChar;
    if (last > 255) last = 255;
    int size = last-base+1;
    HDC hdc = GetDC(0);
    HFONT oldFid = (HFONT)SelectObject(hdc, xfont());
    wglUseFontBitmaps(hdc, base, size, listbase+base);
    SelectObject(hdc, oldFid);
#elif defined(__APPLE__)
    int attrib; const char* name = font->name(&attrib);
    CFStringRef cfname = CFStringCreateWithCString(0L, name, kCFStringEncodingASCII);
    short cfont;
    unsigned char buf[BUFSIZ];
    ConstStringPtr ptr = CFStringGetPascalStringPtr(cfname, kCFStringEncodingMacRoman);
    if (!ptr) {
        CFStringGetPascalString(cfname, buf, BUFSIZ, kCFStringEncodingMacRoman);
        ptr = buf;
    }
    GetFNum(ptr, &cfont);
    CFRelease(cfname);
    aglUseFont(aglGetCurrentContext(), cfont, attrib,(int)current_size_,0,256,listbase);
#else
#error
#endif
  }
  glListBase(listbase);
#if TEXTURES
  texture = fl_font_opengl_texture();
#endif
  setfont(font, size); // necessary so measure() works when scalefactor!=1
}

#define WCBUFLEN 256

/**
  Draw \a text at the current glRasterPos in the current font selected
  with fltk::glsetfont(). You can use glRasterPos2f() or similar calls
  to set the position before calling this.

  The string is in UTF-8, although only characters in ISO-8859-1 are
  drawn correctly, others draw as question marks.
*/
void fltk::gldrawtext(const char* str) {
  gldrawtext(str, strlen(str));
}

/**
  Draw the first \a n bytes of \a text at the current glRasterPos.
*/
void fltk::gldrawtext(const char* text, int n) {
#if TEXTURES
  GLboolean v; glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&v);
  if (!v) return;
#endif
  char localbuffer[WCBUFLEN];
  char* buffer = localbuffer;
  char* mallocbuffer = 0;
  int count = utf8toa(text, n, buffer, WCBUFLEN);
  if (count >= WCBUFLEN) {
    if (count == n) {
      // all ascii or errors, no conversion needed
      buffer = (char*)text;
    } else {
      buffer = mallocbuffer = new char[count+1];
      count = utf8toa(text, n, buffer, count+1);
    }
  }
#if TEXTURES
  glPushAttrib(GL_ENABLE_BIT|GL_TRANSFORM_BIT|GL_COLOR_BUFFER_BIT);
  // setup so the textures draw correctly:
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  // This is useful for making the letters transparent in 3D, but I'll
  // let the caller choose to enable this and to set the cutoff:
  //   glEnable(GL_ALPHA_TEST);
  //   glAlphaFunc(GL_GREATER, 0);
  // all this crap gets us an ortho projection with 0,0 at the rasterpos:
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
  float rasterpos[4]; glGetFloatv(GL_CURRENT_RASTER_POSITION, rasterpos);
# if 0
  glLoadIdentity();
  glOrtho(viewport[0],viewport[2],viewport[1],viewport[3],-1,1);
  glTranslatef(rintf(rasterpos[0]),rintf(rasterpos[1]),1-2*rasterpos[2]);
  glScalef(scalefactor,scalefactor,0);
# else // emulate above directly
  {float m[16];
  const float w = viewport[2]-viewport[0];
  const float h = viewport[3]-viewport[1];
  const float cx= viewport[2]+viewport[0];
  const float cy= viewport[3]+viewport[1];
  const float s = scalefactor;
  m[0]=2*s/w; m[4]=0;     m[8]=0;   m[12]=(2*rintf(rasterpos[0])-cx)/w;
  m[1]=0;     m[5]=2*s/h; m[9]=0;   m[13]=(2*rintf(rasterpos[1])-cy)/h;
  m[2]=0;     m[6]=0;     m[10]=0;  m[14]=2*rasterpos[2]-1;
  m[3]=0;     m[7]=0;     m[11]=0;  m[15]=1;
  glLoadMatrixf(m);}
# endif
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  // now draw the display list:
  glBindTexture(GL_TEXTURE_2D, texture);
  glCallLists(count, GL_UNSIGNED_BYTE, buffer);
  // set new rasterpos (we have translated 0,0 to the desired position):
  //glRasterPos2f(0,0);
  // and restore the previous state & transformation:
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
#elif defined(__APPLE__)
  // Work around an apparent OpenGL bug on our Intel Mac
  glPushMatrix();
  glCallLists(count, GL_UNSIGNED_BYTE, buffer);
  glPopMatrix();
#else
  glCallLists(count, GL_UNSIGNED_BYTE, buffer);
#endif
  delete[] mallocbuffer;
}

/**
  Draw \a text at the given point in 3D space transformed to the screen.
*/
void fltk::gldrawtext(const char* str, float x, float y, float z) {
  gldrawtext(str, strlen(str), x, y, z);
}

/**
  Draw the first \a n bytes of \a text at the given point in 3D space
  transformed to the screen.
*/
void fltk::gldrawtext(const char* str, int n, float x, float y, float z) {
  glRasterPos3f(x, y, z);
  gldrawtext(str, n);
}

// All current systems use exactly the same fonts for OpenGL
// and for normal drawing, so we can share the functions:
float fltk::glgetascent() {return getascent();}
float fltk::glgetdescent() {return getdescent();}
float fltk::glgetwidth(const char* s) {return getwidth(s);}
float fltk::glgetwidth(const char* s, int n) {return getwidth(s,n);}

/**
  Draw a 1-thick line just inside the given rectangle.
*/
void fltk::glstrokerect(int x, int y, int w, int h) {
  if (w < 0) {w = -w; x = x-w;}
  if (h < 0) {h = -h; y = y-h;}
  glBegin(GL_LINE_STRIP);
  glVertex2i(x+w-1, y+h-1);
  glVertex2i(x+w-1, y);
  glVertex2i(x, y);
  glVertex2i(x, y+h-1);
  glVertex2i(x+w, y+h-1);
  glEnd();
}

/** \fn void fltk::glfillrect(int x, int y, int w, int h)
  Inline wrapper for glRecti(x,y,x+w,y+h).
*/

#if USE_GL_OVERLAY
extern bool fl_overlay;
#ifdef _WIN32
extern int fl_overlay_depth;
#endif
#endif

/**
Set the current OpenGL color to a FLTK color, or as close as possible.
*/
void fltk::glsetcolor(Color i) {
#if USE_GL_OVERLAY
#ifndef _WIN32
  if (fl_overlay) {glIndexi(int(xpixel(i))); return;}
#else
  if (fl_overlay && fl_overlay_depth) {
    i = nearest_index(i); // convert to 8-bit color
    if (fl_overlay_depth < 8) {
      // only black & white produce the expected colors.  This could
      // be improved by fixing the colormap set in GlOverlay.cxx
      unsigned size = 1<<fl_overlay_depth;
      if (!i) glIndexi(size-2);
      else if (i >= size-2) glIndexi(size-1);
      else glIndexi(i);
    } else {
      glIndexi(i ? i : BLACK);
    }
    return;
  }
#endif
#endif
  uchar r,g,b; split_color(i,r,g,b);
  glColor3ub(r,g,b);
}

/**
Uses glDrawPixels to draw an image using the same arguments as drawimage().
If you are in the normal OpenGL coordinate system with 0,0 in the lower-left,
the first pixel is memory is the lower-left corner.
*/
void fltk::gldrawimage(const uchar* b, int x, int y, int w, int h, int d, int ld) {
  if (!ld) ld = w*d;
  glPixelStorei(GL_UNPACK_ROW_LENGTH, ld/d);
  glRasterPos2i(x, y);
  glDrawPixels(w, h, d<4?GL_RGB:GL_RGBA, GL_UNSIGNED_BYTE, (const unsigned long*)b);
}

const char* fl_default_font_pathname;

#if USE_XFT

static int nextpow2(int a) {
  int ret = 1;
  while (ret < a) ret <<= 1;
  return ret;
}

static void glXUseXftFont(XftFont* xftfont, unsigned listbase) {

  struct TexGlyph {
    int x,y,w,h; // location of bitmap in texture
    int left, bottom; // corner of bitmap relative to character origin
    float advance; // x advance value
  } texglyphs[256];
  // memory to build texture in:
  const int TEXWIDTH = 512;
  const int TEXHEIGHT = 512;
  uchar* data = new uchar[TEXWIDTH*TEXHEIGHT];
  memset(data, 0, TEXWIDTH*TEXHEIGHT);
  // lower-left corner of next bitmap:
  int curx = 1;
  int cury = 1;
  // when a new row is started it goes here:
  int maxy = 1;

  FT_Face face = XftLockFace(xftfont);

  // hack so Nuke knows where to look for fonts...
  if (!fl_default_font_pathname)
    fl_default_font_pathname = (char*)(face->stream->pathname.pointer);

//   if (!face->charmap && face->num_charmaps)
//     FT_Set_Charmap(face, face->charmaps[0]);
  int prev_pixel_mode = FT_PIXEL_MODE_GRAY;

  int ch; for (ch = 0; ch < 256; ch++) {
    TexGlyph& t = texglyphs[ch];

    unsigned glyph_index;
    if (ch) {
      glyph_index = FT_Get_Char_Index(face, ch);
      if (!glyph_index) {
	// This fixes a lot of decorative fonts:
	if (ch>0x1D && ch < face->num_glyphs+0x1D)
	  glyph_index = ch-0x1D;
	else {
	  // undefined characters copy the undefined glyph in slot zero
	  t = texglyphs[0];
	  continue;
	}
      }
    } else {
      // make slot zero be the undefined glyph, no matter what font says:
      glyph_index = 0;
    }

    // Get the bitmap from Freetype:
    FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
    t.advance = face->glyph->advance.x >> 6; // /64.0 for non-integer
    FT_Glyph glyph; FT_Get_Glyph(face->glyph, &glyph);

    // This seems to be true for spaces in Microsoft fonts, rather than
    // it generating a blank bitmap:
    if (glyph->format != FT_GLYPH_FORMAT_BITMAP) {
      FT_Done_Glyph(glyph);
      t.w = t.h = 0;
      continue;
      // This does not work:
      // FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
    }
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    const int w = t.w = bitmap.width;
    const int h = t.h = bitmap.rows;

    // Skip blank bitmaps:
    if (w > 0 && h > 0) {

      // figure out where we will put the bitmap
      if (curx+w+1 > TEXWIDTH) { // start a new row
	curx = 1; cury = maxy+1;
      }
      // figure out where next row must start
      if (cury+h > maxy) {
	// Ignore any that go out the top of the texture:
	if (cury+h+1 > TEXHEIGHT) {
	  FT_Done_Glyph(glyph);
	  t = texglyphs[0];
	  continue;
	}
	maxy = cury+h;
      }
      int x = t.x = curx; curx = x+w+1;
      int y = t.y = cury;

      // Copy the bitmap, flipping it upside down:
      switch (bitmap.pixel_mode) {
      case FT_PIXEL_MODE_MONO: {
	for (int j = 0; j < h; j++)
	  for (int i = 0; i < w; i++)
	    data[(y+j)*TEXWIDTH+x+i] =
	      (bitmap.buffer[(h-j-1)*bitmap.pitch+i/8]&(128>>(i%8))) ? 255 : 0;
	break;}
      default:
	if (bitmap.pixel_mode != prev_pixel_mode) {
	  prev_pixel_mode = bitmap.pixel_mode;
	  printf("unsupported pixel mode %d!\n", bitmap.pixel_mode);
	} // then fall-through to the default case:
      case FT_PIXEL_MODE_GRAY: {
	for (int j = 0; j < h; j++)
	  for (int i = 0; i < w; i++)
	    data[(y+j)*TEXWIDTH+x+i] =
	      bitmap.buffer[(h-j-1)*bitmap.pitch+i];
	break;}
      }
      t.left = bitmap_glyph->left;
      t.bottom = bitmap_glyph->top-h;
    }

    FT_Done_Glyph(glyph);

  }

  XftUnlockFace(xftfont);

  // Hey developers, the above portion can probably be replaced to reuse
  // this texture code on systems other than Freetype!

  glPushAttrib(GL_TEXTURE_BIT);
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

  // generate the OpenGL texture map we will use:

  int height = nextpow2(maxy);
  int width = TEXWIDTH; // I suppose we could check for < 256 wide...

  GLuint texture;
  glGenTextures(1, &texture);

  glBindTexture(GL_TEXTURE_2D, texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY8, width, height, 0,
	       GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
  delete[] data;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // remember it:
  fl_set_font_opengl_texture(texture);

  // Now build the display lists
  float umul = 1.0f/width;
  float vmul = 1.0f/height;
  for (ch = 0; ch < 256; ch++) {
    const TexGlyph& t = texglyphs[ch];

    // Create the display list for this character:
    glNewList(listbase+ch, GL_COMPILE);

    // Draw a texturemapped rectangle:
    if (t.w > 0 && t.h > 0) {
      glBegin(GL_QUADS);
      glTexCoord2f(t.x*umul,t.y*vmul);
      glVertex2i(t.left,t.bottom);
      glTexCoord2f((t.x+t.w)*umul, t.y*vmul);
      glVertex2i(t.left+t.w,t.bottom);
      glTexCoord2f((t.x+t.w)*umul, (t.y+t.h)*vmul);
      glVertex2i(t.left+t.w,t.bottom+t.h);
      glTexCoord2f(t.x*umul, (t.y+t.h)*vmul);
      glVertex2i(t.left,t.bottom+t.h);
      glEnd();
    }

    // move the origin:
    glTranslatef(t.advance, 0, 0);

    glEndList();
  }

  glPopClientAttrib();
  glPopAttrib();
}
#endif

#endif

//
// End of "$Id: gl_draw.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
