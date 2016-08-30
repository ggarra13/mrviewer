//
// "$Id: Font_xft.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Copyright 2006 Bill Spitzak and others.
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

// Draw fonts using Keith Packard's Xft and Xrender extension. Yow!
// Many thanks to Carl for making the original version of this.
//
// Unlike some other Xft packages, I tried to keep this simple and not
// to work around the current problems in Xft by making the "patterns"
// complicated. I belive doing this defeats our ability to improve Xft
// itself. You should edit the ~/.xftconfig file to "fix" things, there
// are several web pages of information on how to do this.

#include <fltk/events.h>
#include <fltk/Font.h>
#include <fltk/draw.h>
#include <fltk/math.h>
#include <fltk/string.h>
#include <fltk/utf.h>
#include <fltk/x.h>

// define some symbols missing from some Xft header files:
#ifndef XFT_MINSPACE
#define XFT_MINSPACE        "minspace"  /* Bool use minimum line spacing */
#endif
#ifndef XFT_MATRIX
#define XFT_MATRIX          "matrix"    /* XftMatrix */
typedef struct _XftMatrix {
    double xx, xy, yx, yy;
} XftMatrix;
#endif
#include <stdlib.h>

using namespace fltk;

// One of these is made for each combination of font+size:
struct FontSize {
  float minsize, maxsize;
  XftFont* font;
  unsigned fonthash; // id of the actual glyph images
  unsigned opengl_id; // for OpenGL display lists
  unsigned texture; // for OpenGL display lists
  XFontStruct* xfont;
  //~FontSize();
};

// The xft font implementation adds the xft name and the above list:
struct IFont {
  fltk::Font f;
  int attribute_mask; // attributes that can still be turned on
  unsigned numsizes;
  FontSize* fontsizes;
};

// We store the attributes in neat blocks of 2^n:
fltk::Font* fltk::Font::plus(int x) {
  IFont* font = (IFont*)this;
  x &= font->attribute_mask;
  return &((font+x)->f);
}

const char* fltk::Font::system_name() {
  return name_;
}

static FontSize* current;

// API to OpenGL/gl_draw.cxx:
FL_API unsigned fl_font_opengl_id() {return current->opengl_id;}
FL_API unsigned fl_font_opengl_texture() {return current->texture;}
FL_API void fl_set_font_opengl_id(unsigned v) {current->opengl_id = v;}
FL_API void fl_set_font_opengl_texture(unsigned v) {current->texture = v;}

static XftFont* fontopen(const char* name, int attributes, float size, bool core) {
  open_display();
  int weight = XFT_WEIGHT_MEDIUM;
  if (attributes&BOLD) weight = XFT_WEIGHT_BOLD;
  int slant = XFT_SLANT_ROMAN;
  if (attributes&ITALIC) slant = XFT_SLANT_ITALIC;

  return XftFontOpen(xdisplay, xscreen,
		     XFT_FAMILY, XftTypeString, name,
		     XFT_WEIGHT, XftTypeInteger, weight,
		     XFT_SLANT, XftTypeInteger, slant,
		     //XFT_ENCODING, XftTypeString, encoding_,
		     XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
		     core ? XFT_CORE : 0, XftTypeBool, true,
		     XFT_RENDER, XftTypeBool, false,
		     (void*)0);
}

void fltk::setfont(fltk::Font* font, float size) {
#ifndef XFT_MAJOR
  // Older Xft craps out with tiny sizes and returns null for the font
  if (size < 2) size = 2;
#endif
  current_size_ = size;
  if (font == current_font_
      && current->minsize <= size && current->maxsize >= size)
    return;
  current_font_ = font;
  // binary search the fontsizes we have generated already
  FontSize* array = ((IFont*)font)->fontsizes;
  unsigned n = ((IFont*)font)->numsizes;
  unsigned a = 0; unsigned b = n;
  while (a < b) {
    unsigned c = (a+b)/2;
    FontSize* f = array+c;
    if (size < f->minsize) b = c;
    else if (size > f->maxsize) a = c+1;
    else {current = f; return;}
  }
  // new font should now be inserted at a.
  // Ask Xft for the font:
  XftFont* xftfont = fontopen(font->name_, font->attributes_, size, false);
  // There has to be a way to find out which bitmaps were chosen! Anyway
  // this uses what information we have to try to identify if the same
  // bitmaps were chosen:
  unsigned fonthash =
    (32*32)*xftfont->height+32*xftfont->ascent+xftfont->descent;
  // merge if it is the same font as one of the neighbors:
  if (a > 0 && fonthash == array[a-1].fonthash) {
    array[a-1].maxsize = size;
    XftFontClose(xdisplay, xftfont);
    current = array+a-1;
    return;
  }
  if (a < n && fonthash == array[a].fonthash) {
    array[a].minsize = size;
    XftFontClose(xdisplay, xftfont);
    current = array+a;
    return;
  }
  // insert the new entry into the list:
  if (!(n&(n+1))) {
    unsigned m = 2*(n+1)-1;
    FontSize* newarray = new FontSize[m];
    memcpy(newarray, array, a*sizeof(FontSize));
    memcpy(newarray+a+1, array+a, (n-a)*sizeof(FontSize));
    delete[] array;
    ((IFont*)font)->fontsizes = array = newarray;
  } else {
    memmove(array+a+1, array+a, (n-a)*sizeof(FontSize));
  }
  ((IFont*)font)->numsizes = n+1;
  FontSize* f = array+a;
  f->minsize = f->maxsize = size;
  f->font = xftfont;
  f->fonthash = fonthash;
  f->opengl_id = 0;
  f->xfont = 0; // figure this out later
  current = f;
}

#if 0 // this is never called!
FontSize::~FontSize() {
  if (this == current) current = 0;
  XftFontClose(xdisplay, font);
}
#endif

XftFont* fltk::xftfont() {
  return current->font;
}

// This is for back compatability with fltk1 programs, implements the
// former variable fl_xfont:
XFontStruct* fltk::xfont() {
#if XFT_MAJOR > 1
  // kludge!
  static XFontStruct* ret = 0;
  if (!ret) {
    ret = XLoadQueryFont(xdisplay, "variable");
    if (!ret) ret = XLoadQueryFont(xdisplay, "fixed");
  }
  return ret;
#else
  // This code only works for XFT 1, which was able to find matching
  // X fonts:
  if (!current->xfont) {
    if (current->font->core) {
      current->xfont = current->font->u.core.font;
    } else {
      static XftFont* xftfont;
      if (xftfont) XftFontClose (xdisplay, xftfont);
      // select the "core" version of the font:
      xftfont = fontopen(current_font_->name_,
			 current_font_->attributes_,
			 current_size_,
			 true);
      current->xfont = xftfont->u.core.font;
    }
  }
  return current->xfont;
#endif
}

const char* fltk::Font::current_name() {
  return current_font_->name_;
}

float fltk::getascent()  { return current->font->ascent; }
float fltk::getdescent() { return current->font->descent; }

// Unfortunatly the Xft UTF-8 interface barfs on error sequences and
// does not print anything. This is very annoying. Alternative version
// here will user our converter to wchar_t and print that, but I would
// rather see Xft fixed.
//
// Older Xft seems to crash xlib on certain lengths. This was a real
// pain to track down!
#define WCBUFLEN 256

float fltk::getwidth(const char *str, int n) {
  XGlyphInfo i;
#if 0
  XftTextExtentsUtf8(xdisplay, current->font, (XftChar8*)str, n, &i);
#else
  wchar_t localbuffer[WCBUFLEN];
  wchar_t* buffer = localbuffer;
  wchar_t* mallocbuffer = 0;
  int count = utf8towc(str, n, buffer, WCBUFLEN);
  if (count >= WCBUFLEN) {
    buffer = mallocbuffer = new wchar_t[count+3];
    count = utf8towc(str, n, buffer, count+3);
  }
  // fix some lengths that cause older Xft to crash (!):
  if ((count&255)==253) buffer[count++] = 0;
  if ((count&255)==254) buffer[count++] = 0;
  XftTextExtents32(xdisplay, current->font, (XftChar32*)buffer, count, &i);
  delete[] mallocbuffer;
#endif
  return i.xOff;
}

////////////////////////////////////////////////////////////////

void fltk::drawtext_transformed(const char *str, int n, float x, float y) {

  // Use fltk's color allocator, copy the results to match what
  // XftCollorAllocValue returns:
  XftColor color;
#if USE_CAIRO
  color.pixel = 0;
#else
  color.pixel = current_xpixel;
#endif
  uchar r,g,b; split_color(getcolor(), r,g,b);
  color.color.red   = r*0x101;
  color.color.green = g*0x101;
  color.color.blue  = b*0x101;
  color.color.alpha = 0xffff;

#if 0
  XftDrawStringUtf8(xftc, &color, current->font,
		    int(floorf(x+.5f)), int(floorf(y+.5f)),
		    (XftChar8*)str, n);
#else
  wchar_t localbuffer[WCBUFLEN];
  wchar_t* buffer = localbuffer;
  wchar_t* mallocbuffer = 0;
  int count = utf8towc(str, n, buffer, WCBUFLEN);
  if (count >= WCBUFLEN) {
    buffer = mallocbuffer = new wchar_t[count+3];
    count = utf8towc(str, n, buffer, count+3);
  }
  // fix some lengths that cause older Xft to crash (!):
  if ((count&255)==253) buffer[count++] = ' ';
  if ((count&255)==254) buffer[count++] = ' ';
  XftDrawString32(xftc, &color, current->font,
		  int(floorf(x+.5f)), int(floorf(y+.5f)),
		  (XftChar32*)buffer, count);
  delete[] mallocbuffer;
#endif
}

////////////////////////////////////////////////////////////////

// The predefined fonts that fltk has:
static IFont fonts [] = {
  {{"sans",	0},	3,	0},
  {{"sans",	1},	2,	0},
  {{"sans",	2},	1,	0},
  {{"sans",	3},	0,	0},
  {{"mono",	0},	3,	0},
  {{"mono",	1},	2,	0},
  {{"mono",	2},	1,	0},
  {{"mono",	3},	0,	0},
  {{"serif",	0},	3,	0},
  {{"serif",	1},	2,	0},
  {{"serif",	2},	1,	0},
  {{"serif",	3},	0,	0},
  {{"symbol",	0},	0,	0},
  {{"wingdings",0},	0,	0},
};

fltk::Font* const fltk::HELVETICA		= &(fonts[0].f);
fltk::Font* const fltk::HELVETICA_BOLD		= &(fonts[1].f);
fltk::Font* const fltk::HELVETICA_ITALIC	= &(fonts[2].f);
fltk::Font* const fltk::HELVETICA_BOLD_ITALIC	= &(fonts[3].f);
fltk::Font* const fltk::COURIER			= &(fonts[4].f);
fltk::Font* const fltk::COURIER_BOLD		= &(fonts[5].f);
fltk::Font* const fltk::COURIER_ITALIC		= &(fonts[6].f);
fltk::Font* const fltk::COURIER_BOLD_ITALIC	= &(fonts[7].f);
fltk::Font* const fltk::TIMES			= &(fonts[8].f);
fltk::Font* const fltk::TIMES_BOLD		= &(fonts[9].f);
fltk::Font* const fltk::TIMES_ITALIC		= &(fonts[10].f);
fltk::Font* const fltk::TIMES_BOLD_ITALIC	= &(fonts[11].f);
fltk::Font* const fltk::SYMBOL_FONT		= &(fonts[12].f);
fltk::Font* const fltk::SCREEN_FONT		= &(fonts[4].f);
fltk::Font* const fltk::SCREEN_BOLD_FONT	= &(fonts[5].f);
fltk::Font* const fltk::ZAPF_DINGBATS		= &(fonts[13].f);

// Turn an fltk1 integer font id into a predefined font:
fltk::Font* fltk::font(int i) {
  i = i & 15;
  switch (i) {
  case 13: i = 4; break; // "screen"
  case 14: i = 5; break; // "screen bold"
  case 15: i = 13; break; // "dingbats"
  default: break;
  }
  return &(fonts[i].f);
}

////////////////////////////////////////////////////////////////
// The rest of this is for listing fonts:

extern "C" {
static int sort_function(const void *aa, const void *bb) {
  fltk::Font* a = *(fltk::Font**)aa;
  fltk::Font* b = *(fltk::Font**)bb;
  int ret = strcasecmp(a->name_, b->name_); if (ret) return ret;
  return a->attributes_ - b->attributes_;
}}

int fltk::list_fonts(fltk::Font**& arrayp) {
  static fltk::Font** font_array = 0;
  static int num_fonts = 0;
  if (font_array) { arrayp = font_array; return num_fonts; }

  // Make sure fontconfig is ready... is this necessary? The docs say it is
  // safe to call it multiple times, so just go for it anyway!
  FcInit();

  // Create a search pattern that will match every font name - I think
  // this does the Right Thing... but am not certain...
  FcPattern* fnt_pattern = FcPatternCreate ();
  // Now list all the families:
  FcObjectSet* fnt_obj_set = FcObjectSetBuild (FC_FAMILY, (void*)0);
  // This gives more info, but it is a pain to deal with:
  //FcObjectSet* fnt_obj_set = FcObjectSetBuild (FC_FAMILY, FC_WEIGHT, FC_SLANT, 0);

  // Hopefully, this is a set of all the fonts...
  FcFontSet* fnt_set = FcFontList (0, fnt_pattern, fnt_obj_set);
  
  // We don't need the fnt_pattern any more, release it
  FcPatternDestroy (fnt_pattern);

  num_fonts = fnt_set->nfont;
  font_array = new fltk::Font*[num_fonts];

  // For each family we make 4 fonts for bold,italic combinations
  // The families could be smaller if there is no bold or italic, but
  // that is a pain to figure out. Besides this is what Windows
  // version does:
  IFont* fonts = new IFont[num_fonts*4];
  int k = 0;
  for (int i = 0; i < num_fonts; i++) {
    FcPattern* p = fnt_set->fonts[i];
    const char* n = 0;
    FcPatternGetString(p, FC_FAMILY, 0, (FcChar8**)(void*)(&n));
    if (!n /*|| !isalpha(n[0])*/) continue;
//     for (int kk=1;;kk++) {
//       const char* x = 0;
//       FcPatternGetString(p, FC_FAMILY, kk, (FcChar8**)(&x));
//       if (!x) break;
//       printf("%s is also \"%s\"\n", n,x);
//     }
    const char* name = newstring(n);
    IFont* newfont = fonts+4*k;
    font_array[k] = &(newfont->f);
    ++k;
    for (int j = 0; j < 4; j++) {
      newfont[j].f.name_ = name;
      newfont[j].f.attributes_ = j;
      newfont[j].attribute_mask = 3-j;
      newfont[j].numsizes = 0;
      newfont[j].fontsizes = 0;
    }
  }
  num_fonts = k;
  // Release the fnt_set - we don't need it any more
  FcFontSetDestroy (fnt_set);

  qsort(font_array, num_fonts, sizeof(*font_array), sort_function);

  arrayp = font_array;
  return num_fonts;
}

////////////////////////////////////////////////////////////////

extern "C" {
static int int_sort(const void *aa, const void *bb) {
  return (*(int*)aa)-(*(int*)bb);
}
}

// Return all the point sizes supported by this font:
// Suprisingly enough Xft works exactly like fltk does and returns
// the same list. Except there is no way to tell if the font is scalable.
int fltk::Font::sizes(int*& sizep) {
  open_display();
  XftFontSet* fs = XftListFonts(xdisplay, xscreen,
				XFT_FAMILY, XftTypeString, name_, (void*)0,
				XFT_PIXEL_SIZE, (void*)0);
  static int* array = 0;
  static int array_size = 0;
  if (fs->nfont >= array_size) {
    delete[] array;
    array = new int[array_size = fs->nfont+1];
  }
  int j = 0;
  for (int i = 0; i < fs->nfont; i++) {
    double v;
    if (XftPatternGetDouble(fs->fonts[i], XFT_PIXEL_SIZE, 0, &v)
	== XftResultMatch)
      array[j++] = int(v);
  }
  if (j) {
    qsort(array, j, sizeof(int), int_sort);
  } else {
    array[0] = 0; j = 1; // claim all fonts are scalable by putting a 0 in
  }
  XftFontSetDestroy(fs);
  sizep = array;
  return j;
}

////////////////////////////////////////////////////////////////
// Return all the encodings for this font:

int fltk::Font::encodings(const char**& arrayp) {
  open_display();
  // we need to keep the previous return value around so the strings are
  // not destroyed.
  static XftFontSet* fs;
  if (fs) XftFontSetDestroy(fs);
  fs = XftListFonts(xdisplay, xscreen,
		    XFT_FAMILY, XftTypeString, name_, (void*)0,
		    XFT_ENCODING, (void*)0);
  static const char** array = 0;
  static int array_size = 0;
  if (fs->nfont > array_size) {
    delete[] array;
    array = new const char*[array_size = fs->nfont];
  }
  int j = 0;
  for (int i = 0; i < fs->nfont; i++) {
    char* v;
    if (XftPatternGetString(fs->fonts[i], XFT_ENCODING, 0, (void*)&v) == XftResultMatch) {
      array[j++] = v;
    }
  }
  arrayp = array;
  return j;
}

//
// End of "$Id: Font_xft.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
