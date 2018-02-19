//
// "$Id: Font.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// _WIN32 font selection routines for the Fast Light Tool Kit (FLTK).
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

#include <fltk/x.h>
#include <fltk/Font.h>
#include <ctype.h>
#include <stdlib.h>
#include <fltk/math.h>
#include <fltk/utf.h>

using namespace fltk;

// One of these is made for each combination of size + encoding:
struct FontSize {
  unsigned size;
  HFONT font;
  int charset;
  int width[256];
  TEXTMETRICW metr;
  unsigned opengl_id;
  FontSize* next_all;
  FontSize(const char* fontname, int attr, int size, int charset);
  ~FontSize();
};

// The public-visible fltk::Font structures are actually imbedded in
// this larger structure which points at the the above list
struct IFont {
  fltk::Font f;
  int attribute_mask; // all attributes that can be turned on
  unsigned numsizes;
  FontSize** fontsizes;
};

// We store the attributes in neat blocks of 2^n:
fltk::Font* fltk::Font::plus(int x) {
  IFont* font = (IFont*)this;
  x &= font->attribute_mask & ~attributes_;
  return &((font+x)->f);
}

const char* fltk::Font::system_name() {
  return name_;
}

static FontSize *current;

FL_API unsigned fl_font_opengl_id() {return current->opengl_id;}
FL_API void fl_set_font_opengl_id(unsigned v) {current->opengl_id = v;}

static FontSize* all_fonts;

FontSize::FontSize(const char* name, int attr, int size, int charset) {

  // Open display, if not opened yet. So function pointers are set correctly
  open_display();

  current = this;

  HDC dc = getDC();

  int weight = (attr&BOLD) ? FW_BOLD : FW_NORMAL;
  int italic = (attr&ITALIC) ? 1 : 0;
  //int height = -MulDiv(size, GetDeviceCaps(dc, LOGPIXELSY), 72);
  int height = -size;

  LOGFONTW lf;
  memset( &lf, 0, sizeof(lf) );
  lf.lfHeight         = height; // use "char size"
  lf.lfWidth          = 0L;
  lf.lfEscapement     = 0L;
  lf.lfOrientation    = 0L;
  lf.lfWeight         = weight;
  lf.lfItalic         = italic;
  lf.lfUnderline      = FALSE;
  lf.lfStrikeOut      = FALSE;
  lf.lfCharSet        = charset;
  lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
  lf.lfQuality        = DEFAULT_QUALITY;

  utf8towc(name, strlen(name), lf.lfFaceName, LF_FACESIZE);

  // This one does exactly same thing as CreateFont,
  // But we use CreateFontIndirect, since Windows CE does not have CreateFont
  font = __CreateFontIndirectW(&lf);

  SelectObject(dc, font);
  __GetTextMetricsW(dc, &current->metr);
//    printf("FontSize '%s' %d %d %d -> %d %d\n",
//  	 name, attr, size, charset,
//  	 current->metr.tmAscent,
//  	 current->metr.tmDescent);
  //BOOL ret = GetCharWidthFloat(dc, metr.tmFirstChar, metr.tmLastChar, font->width+metr.tmFirstChar);
  //...would be the right call, but is not implemented into Window95! (WinNT?)
  //GetCharWidth(dc, 0, 255, current->width);

  this->size = size;
  this->charset = charset;
  next_all = all_fonts;
  all_fonts = this;

  opengl_id = 0;
}

FontSize::~FontSize() {
  if (this == current) current = 0;
  DeleteObject(font);
}

// Deallocate Win32 fonts on exit. Warning: it will crash if you try
// to do any fonts after this, because the pointers are not changed!
void fl_font_rid() {
  for (FontSize* fontsize = all_fonts; fontsize;) {
    FontSize* next = fontsize->next_all;
    delete fontsize;
    fontsize = next;
  }
}

////////////////////////////////////////////////////////////////

// The predefined fonts that fltk has:
static IFont fonts [] = {
  {{"Arial",	0},	3,	0},
  {{"Arial",	1},	3,	0},
  {{"Arial",	2},	3,	0},
  {{"Arial",	3},	3,	0},
  {{"Courier New",0},	3,	0},
  {{"Courier New",1},	3,	0},
  {{"Courier New",2},	3,	0},
  {{"Courier New",3},	3,	0},
  {{"Times New Roman",	0},	3,	0},
  {{"Times New Roman",	1},	3,	0},
  {{"Times New Roman",	2},	3,	0},
  {{"Times New Roman",	3},	3,	0},
  {{"Symbol",	0},	0,	0},
  {{"Wingdings",0},	0,	0}
};

fltk::Font* const fltk::HELVETICA 		= &(fonts[0].f);
fltk::Font* const fltk::HELVETICA_BOLD		= &(fonts[1].f);
fltk::Font* const fltk::HELVETICA_ITALIC	= &(fonts[2].f);
fltk::Font* const fltk::HELVETICA_BOLD_ITALIC	= &(fonts[3].f);
fltk::Font* const fltk::COURIER 		= &(fonts[4].f);
fltk::Font* const fltk::COURIER_BOLD		= &(fonts[5].f);
fltk::Font* const fltk::COURIER_ITALIC		= &(fonts[6].f);
fltk::Font* const fltk::COURIER_BOLD_ITALIC	= &(fonts[7].f);
fltk::Font* const fltk::TIMES 			= &(fonts[8].f);
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

// For fltk::list_fonts(), make a new font, and optionally the bold and
// italic subfonts:
Font* fl_make_font(const char* name, int attrib) {
  // see if it is one of our built-in fonts and return it:
  int j; for (j = 0; j < 13; j++) {
    if (fonts[j].f.attributes_ == attrib &&
	!strcasecmp(fonts[j].f.name_, name)) return &(fonts[j].f);
  }
  // no, lets create some fonts:
  IFont* newfont = new IFont[4];
  newfont[0].f.name_ = newstring(name);
  for (j = 0; j < 4; j++) {
    newfont[j].f.name_ = newfont[0].f.name_;
    newfont[j].f.attributes_ = attrib|j;
    newfont[j].attribute_mask = 3;
    newfont[j].numsizes = 0;
    newfont[j].fontsizes = 0;
  }
  return &(newfont[0].f);
}

////////////////////////////////////////////////////////////////
// Public interface:

HFONT fltk::xfont() {return current->font;}
TEXTMETRICW* fltk::textmetric() {return &(current->metr);}

const char* fltk::Font::current_name() {
  return current_font_->name_;
}

// we need to decode the encoding somehow!
static int charset = DEFAULT_CHARSET;

void fltk::setfont(Font* font, float psize) {

  // only integers supported right now, I think there is a newer
  // interface that takes arbitrary sizes, though...
  psize = rintf(psize);
  unsigned size = unsigned(psize);

  if (font == current_font_ && current->size == size &&
      current->charset == charset) return;
  current_font_ = font; current_size_ = psize;

  // binary search the fontsizes we have generated already
  FontSize** array = ((IFont*)font)->fontsizes;
  unsigned n = ((IFont*)font)->numsizes;
  unsigned a = 0; unsigned b = n;
  while (a < b) {
    unsigned c = (a+b)/2;
    FontSize* f = array[c];
    int d = f->size-size;
    if (!d) d = f->charset - charset;
    if (d < 0) a = c+1;
    else if (d > 0) b = c;
    else {current = f; return;}
  }
  if (!(n&(n+1))) {
    unsigned m = 2*(n+1)-1;
    FontSize** newarray = new FontSize*[m];
    memcpy(newarray, array, a*sizeof(FontSize*));
    memcpy(newarray+a+1, array+a, (n-a)*sizeof(FontSize*));
    delete[] array;
    ((IFont*)font)->fontsizes = array = newarray;
  } else {
    memmove(array+a+1, array+a, (n-a)*sizeof(FontSize*));
  }
  ((IFont*)font)->numsizes = n+1;
  current = array[a] = new FontSize(font->name_, font->attributes_, size, charset);
}

float fltk::getascent()  { return float(current->metr.tmAscent); }
float fltk::getdescent() { return float(current->metr.tmDescent); }

#define WCBUFLEN 256

float fltk::getwidth(const char* text, int n) {	
  HDC dc = getDC();
  SelectObject(dc, current->font);
  wchar_t localbuffer[WCBUFLEN];
  wchar_t* buffer = localbuffer;
  wchar_t* mallocbuffer = 0;
  int count = utf8towc(text, n, buffer, WCBUFLEN);
  if (count >= WCBUFLEN) {
    buffer = mallocbuffer = new wchar_t[count+1];
    count = utf8towc(text, n, buffer, count+1);
  }
  // I think win32 has a fractional version of this:
  SIZE size; GetTextExtentPoint32W(dc, buffer, count, &size);
  delete[] mallocbuffer;
  return (float)(size.cx);
}

void fltk::drawtext_transformed(const char *text, int n, float x, float y) {
  SetTextColor(dc, current_xpixel);
  SelectObject(dc, current->font);
  wchar_t localbuffer[WCBUFLEN];
  wchar_t* buffer = localbuffer;
  wchar_t* mallocbuffer = 0;
  int count = utf8towc(text, n, buffer, WCBUFLEN);
  if (count >= WCBUFLEN) {
    buffer = mallocbuffer = new wchar_t[count+1];
    count = utf8towc(text, n, buffer, count+1);
  }
  TextOutW(dc, int(floorf(x+.5f)), int(floorf(y+.5f)), buffer, count);
  delete[] mallocbuffer;
}

//
// End of "$Id: Font.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
