// "$Id: Font_xlfd.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/* Donated code that selects from an alternative font if the character
  cannot be printed in the current font. Did not work very well for me
  and it slows down normal printing a lot. */
#define X_UTF8_FONT 0

#include <fltk/draw.h>
#include <fltk/error.h>
#include <fltk/x.h>
#include <fltk/math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fltk/utf.h>
#include "IFont.h"
using namespace fltk;

// One of these is created for each font+size+encoding combination we use:
struct FontSize {
  FontSize* next;	// linked list for a single Font
  XFontStruct* font;
  const char* encoding;
  FontSize(const char* xfontname, const char*);
  const char* name;
  unsigned minsize;	// smallest point size that should use this
  unsigned maxsize;	// largest point size that should use this
  unsigned opengl_id; // for OpenGL display lists
  //  ~FontSize();
};

fltk::Font* fltk::Font::plus(int x) {
  IFont* font = (IFont*)this;
  if (x & BOLD) font = font->bold;
  if (x & ITALIC) font = font->italic;
  return &(font->f);
}

const char* fltk::Font::system_name() {
  return ((IFont*)this)->system_name;
}

static FontSize *current;
static GC font_gc; // which gc the font was set in last time

FL_API unsigned fl_font_opengl_id() {return current->opengl_id;}
FL_API void fl_set_font_opengl_id(unsigned v) {current->opengl_id = v;}

/*! Returns the operating-system dependent structure defining the
  current font. You must include <fltk/x.h> to use this. */
XFontStruct* fltk::xfont() {return current->font;}

/*!
  Return the full X11 name for the currently selected font+size+encoding.
  This is mostly useful for debugging, though maybe you need to copy
  it to another program.

  On non-X systems, and on modern X systems with Xft (antialiased fonts)
  this returns current_font()->system_name().
*/
const char* fltk::Font::current_name() {return current->name;}

FontSize::FontSize(const char* name, const char* nname) {
  this->name = nname ? nname : name;
  font = XLoadQueryFont(xdisplay, name);
  if (!font) {
    warning("bad font: %s", name);
    font = XLoadQueryFont(xdisplay, "fixed"); // if fixed fails we crash
  }
  encoding = 0;
  opengl_id = 0;
}

#if 0 // this is never called!
FontSize::~FontSize() {
  if (this == current) current = 0;
  XFreeFont(xdisplay, font);
}
#endif

////////////////////////////////////////////////////////////////
// Things you can do once the font+size has been selected:

// I see no sign of "FontSets" working. Instead this supposedly will
// draw the correct letters if you happen to pick an iso10646-1 font.

// This is similar to utf8towc() but works with the big-endian-only
// structure X seems to want, and does not bother with surrogate
// pairs.  If all characters are 1 byte or errors it returns
// null. Otherwise it converts it to 16-bit and returns the allocated
// buffer and size:
static XChar2b* utf8to2b(const char* text, int n, int* charcount) {
  const char* p = text;
  const char* e = text+n;
  bool sawutf8 = false;
  int count = 0;
  while (p < e) {
    if (*(unsigned char*)p < 0x80) p++; // ascii
    else if (*(unsigned char*)p < 0xa0) {sawutf8 = 1; p++;} //cp1252
    else if (*(unsigned char*)p < 0xC2) p++; // other bad code
    else {
      int len; utf8decode(p,e,&len);
      if (len > 1) sawutf8 = true;
      else if (!len) len = 1;
      p += len;
    }
    count++;
  }
  if (!sawutf8) return 0;
  *charcount = count;
  XChar2b* buffer = new XChar2b[count];
  count = 0;
  p = text;
  while (p < e) {
    unsigned char c = *(unsigned char*)p;
    if (c < 0xC2) { // ascii letter or bad code
      buffer[count].byte1 = 0;
      buffer[count].byte2 = c;
      p++;
    } else {
      int len;
      unsigned n = utf8decode(p,e,&len);
      if (n > 0xffff) n = '?';
      p += len;
      buffer[count].byte1 = n>>8;
      buffer[count].byte2 = n;
    }
    count++;
  }
  return buffer;
}

// return dash number N, or pointer to ending null if none:
static const char *
font_word(const char* p, int n) {
  while (*p) {if (*p=='-') {if (!--n) break;} p++;}
  return p;
}

#if X_UTF8_FONT

typedef struct {
  const char* name;
  XFontStruct* font;
  bool enable;
} XFontUtf8;
XFontUtf8 xfont_utf8[32] = {0};

#define FONT_INDEX_DISABLE -1
#define FONT_INDEX_FULL    -2
//#define FONT_DEBUG

static void get_font_entry(char *buf, int buf_size, char **pp)
{
  char *p, *r;
  int len;

  r = *pp;
  p = strchr(r, '-');
  if (!p) {
    if (buf)
      buf[0] = '\0';
  } else {
    if (buf) {
      len = p - r;
      if (len >= buf_size)
        len = buf_size - 1;
      memcpy(buf, r, len);
      buf[len] = '\0';
    }
    *pp = p + 1;
  }
}

static const char* get_utf8_font_name_from_basefont(const char *basefont, int safelevel = 0)
{
  char *pname = newstring(basefont), *pnameold;
  char weight[16], slant[16], ptsize[16];
  static char fontname[256];

  pnameold = pname;
  get_font_entry(NULL, 0, &pname);
  get_font_entry(NULL, 0, &pname);
  get_font_entry(NULL, 0, &pname);
  get_font_entry(weight, sizeof(weight), &pname); /* weight */
  get_font_entry(slant, sizeof(slant), &pname); /* slant */
  get_font_entry(NULL, 0, &pname); /* adstyle */
  get_font_entry(NULL, 0, &pname); /* pixsize */
  get_font_entry(ptsize, sizeof(ptsize), &pname); /* size */
  get_font_entry(NULL, 0, &pname); /* pixsize */
  get_font_entry(NULL, 0, &pname); /* pixsize */
  get_font_entry(NULL, 0, &pname); /* pixsize */
  get_font_entry(NULL, 0, &pname); /* pixsize */
  get_font_entry(NULL, 0, &pname); /* pixsize */
  delete[] pnameold;

  switch(safelevel) {
  case 0:
    sprintf(fontname, "-*-*-%s-%s-normal--%s-*-*-*-*-*-iso10646-1",
      weight, slant, ptsize);
    break;
  case 1:
    sprintf(fontname, "-gnu-unifont-%s-%s-normal--%s-*-*-*-*-*-iso10646-1",
      weight, slant, "*");
    break;
  case 2:
    sprintf(fontname, "-*-*-%s-%s-normal--%s-*-*-*-*-*-iso10646-1",
      weight, slant, "*");
    break;
  case 3:
    sprintf(fontname, "-*-*-%s-%s-normal--%s-*-*-*-*-*-iso10646-1",
      "*", "*", ptsize);
    break;
  default:
    return NULL;
  }
  return fontname;
}

static int get_xfont_utf8_index(const char *fontname)
{
  for(unsigned int i = 0; i < sizeof(xfont_utf8)/sizeof(XFontUtf8); i++) {
    if (xfont_utf8[i].name == NULL)
      return i;
    if (!strcmp(xfont_utf8[i].name, fontname)) {
      if (xfont_utf8[i].enable == false)
        return -1;
      return i;
    }
  }
  return -2;
}

static bool font_is_showable(XFontStruct *f, unsigned int cc)
{
  unsigned int b1, b2;
  XCharStruct *cs;

  if (f->min_byte1 == 0 && f->max_byte1 == 0) {
    if (cc > f->max_char_or_byte2) return false;
    if (cc < f->min_char_or_byte2) return false;
    cc -= f->min_char_or_byte2;
  } else {
    b1 = (cc >> 8) & 0xff;
    b2 = cc & 0xff;
    if (b1 > f->max_byte1) return false;
    if (b1 < f->min_byte1) return false;
    b1 -= f->min_byte1;
    if (b2 > f->max_char_or_byte2) return false;
    if (b2 < f->min_char_or_byte2) return false;
    b2 -= f->min_char_or_byte2;
    cc = b1 * (f->max_char_or_byte2 - f->min_char_or_byte2 + 1) + b2;
    if (cc >= 0xffff) return false;
  }
  cs = f->per_char;
  if (!cs)
    return &f->min_bounds; /* all char have same metrics */
  cs += cc;
  /* fast test for non existant char */
  if (cs && cs->width == 0 &&
    (cs->ascent | cs->descent | cs->rbearing | cs->lbearing) == 0) {
    return false;
  } else {
    return true;
  }
}

static XFontStruct* get_xfont_utf8(const char* basefont, int cc)
{
  XFontStruct* f = 0;
  char fontname[256];
  char buf[256];
  char familyold[256];
  char family[256];
  char *pname;
  unsigned int realsize;

  strcpy(fontname, basefont);
  // replace fontname if ptsize = 0
  realsize = -atol(font_word(fontname, 7));
  if (!realsize) {
    pname = fontname;
    char realname[256];

    realname[0] = 0;
    for(int entry = 0; entry < 14; entry++) {
      get_font_entry(buf, sizeof(buf), &pname);
      if (entry == 7)
        sprintf(buf, "%d", current->maxsize);
      strcat(realname, buf);
      strcat(realname, "-");
      if (entry == 13)
        strcat(realname, pname);
    }
    strcpy(fontname, realname);
  }

  int font_index = get_xfont_utf8_index(fontname);
  switch(font_index) {
  case FONT_INDEX_FULL:
    //printf("full request = %s\n", fontname);
    break;
  case FONT_INDEX_DISABLE:
    //printf("disable request = %s\n", fontname);
    break;
  default:
    if (!xfont_utf8[font_index].name) {
      int safelevel = 0;
      const char* utf8_fontname;
#ifdef FONT_DEBUG
      printf("basefont = %s\n", basefont);
#endif

      xfont_utf8[font_index].name = newstring(fontname);

      // try some safety fonts
      while((utf8_fontname = get_utf8_font_name_from_basefont(fontname, safelevel))) {
        char** xlist;
        int nlist = 0;
        unsigned int acceptold = 0, ptsize;
        char acceptfont[256];
        acceptfont[0] = 0;
        familyold[0] = 0;

#ifdef FONT_DEBUG
        printf(" pattern = %s\n", utf8_fontname);
#endif
        xlist = XListFonts(xdisplay, utf8_fontname, 1000, &nlist);
        for(int nfont = 0; nfont < nlist; nfont++) {
          // get family name for skipping entry
          pname = xlist[nfont];
          get_font_entry(NULL, 0, &pname);
          get_font_entry(buf, sizeof(buf), &pname);
          strcpy(family, buf);
          if (!strcmp(family, familyold)) {
            continue;
          } else
          if (acceptfont[0])
            break;

          // don't use scalable font
          if (atol(font_word(xlist[nfont], 12)) == 0) {
            strcpy(familyold, family);
            continue;
          }
          ptsize = -atol(font_word(xlist[nfont], 7));
          if (ptsize > current->maxsize+2) {
            strcpy(familyold, family);
            continue;
          }
#ifdef FONT_DEBUG
		  printf("trying %s\n", xlist[nfont]);
#endif
          XFontStruct *ff = XLoadQueryFont(xdisplay, xlist[nfont]);
          if (font_is_showable(ff, cc)) {
            strcpy(familyold, family);
            if (safelevel == 0) {
#ifdef FONT_DEBUG
              printf("   match = %s\n", xlist[nfont]);
#endif
              xfont_utf8[font_index].font = ff;
              break;
            } else {
              if (ptsize > acceptold) {
#ifdef FONT_DEBUG
                printf("   match = %s\n", xlist[nfont]);
#endif
                strcpy(acceptfont, xlist[nfont]);
                acceptold = ptsize;
				if (ptsize == realsize)
                  break;
              }
            }
		  } else {
            strcpy(familyold, family);
          }
          XFreeFont(xdisplay, ff);
        }
        XFreeFontNames(xlist);
        if (safelevel > 0 && acceptfont[0]) {
          xfont_utf8[font_index].font = XLoadQueryFont(xdisplay, acceptfont);
        }
        if (!xfont_utf8[font_index].font) {
          xfont_utf8[font_index].enable = false;
        } else {
          xfont_utf8[font_index].enable = true;
          f = xfont_utf8[font_index].font;
          break;
        }
        safelevel++;
      }
    } else {
      f = xfont_utf8[font_index].font;
    }
  }
  return f;
}

#endif // X_UTF8_FONT

void fltk::drawtext_transformed(const char *text, int n, float x, float y) {
  if (font_gc != gc) {
    // I removed this, the user MUST set the font before drawing: (was)
    // if (!current) setfont(HELVETICA, NORMAL_SIZE);
    font_gc = gc;
    XSetFont(xdisplay, gc, current->font->fid);
  }
  int count;
  XChar2b* buffer = utf8to2b(text,n,&count);
  if (buffer) {
#if !X_UTF8_FONT
    XDrawString16(xdisplay, xwindow, gc,
		  int(floorf(x+.5f)),
		  int(floorf(y+.5f)), buffer, count);
#else
#ifdef FONT_DEBUG
	//printf("current = %s\n", current->name);
#endif
    XFontStruct *f = current->font, *oldf = f;
    for(int i = 0; i < count; i++) {
      int cc = (buffer[i].byte1 << 8) + buffer[i].byte2;
      if (!font_is_showable(current->font, cc)) {
        f = get_xfont_utf8(current->name, cc);
	if (!f) {
	  f = current->font;
#ifdef FONT_DEBUG
	  printf("can't draw %C\n");
#endif
	  buffer[i].byte1 = 0;
	  buffer[i].byte2 = '?';
	}
      } else
	f = current->font;
      if (f != oldf) {
        XSetFont(xdisplay, gc, f->fid);
	oldf = f;
      }
      XDrawString16(xdisplay, xwindow, gc,
		  int(floorf(x+.5f)),
		  int(floorf(y+.5f)), &buffer[i], 1);
      x += XTextWidth16(f, &buffer[i], 1);
    }
    XSetFont(xdisplay, gc, current->font->fid);
#endif
    delete[] buffer;
  } else {
    XDrawString(xdisplay, xwindow, gc,
		int(floorf(x+.5f)),
		int(floorf(y+.5f)), text, n);
  }
}

float fltk::getascent()  { return current->font->ascent; }

float fltk::getdescent() { return current->font->descent; }

float fltk::getwidth(const char *text, int n) {
  int count;
  XChar2b* buffer = utf8to2b(text,n,&count);
  if (buffer) {
#if !X_UTF8_FONT
    float r = XTextWidth16(current->font, buffer, count);
#else
    float r = 0;
    XFontStruct *f = current->font;
    for(int i = 0; i < count; i++) {
      int cc = (buffer[i].byte1 << 8) + buffer[i].byte2;
      if (!font_is_showable(current->font, cc)) {
        f = get_xfont_utf8(current->name, cc);
	if (!f) {
	  f = current->font;
	  buffer[i].byte1 = 0;
	  buffer[i].byte2 = '?';
	}
      } else
	f = current->font;
      r += XTextWidth16(f, &buffer[i], 1);
    }
#endif
    delete[] buffer;
    return r;
  } else {
    return XTextWidth(current->font, text, n);
  }
}

////////////////////////////////////////////////////////////////
// The rest of this file is the enormous amount of crap you have to
// do to get a font & size out of X.  To select a font+size, all
// matchine X fonts are listed with XListFonts, and then the first
// of the following is found and used:
//
//	pixelsize == size
//	pixelsize == 0 (which indicates a scalable font)
//	the largest pixelsize < size
//	the smallest pixelsize > size
//
// If any fonts match the encoding() then the search is limited
// to those matching fonts. Otherwise all fonts are searched and thus
// a random encoding is chosen.
//
// I have not been able to find any other method than a search
// that will reliably return a bitmap version of the font if one is
// available at the correct size.  This is because X will not use a
// bitmap font unless all the extra fields are filled in correctly.
//
// Fltk uses pixelsize, not "pointsize".  This is what everybody wants!

void fltk::setfont(Font* font, float psize) {
  FontSize* f = current;
  IFont* t = (IFont*)font;

  // only integers supported right now (this can be improved):
  psize = int(psize+.5);
  unsigned size = unsigned(psize);

  // See if the current font is correct:
  if (font == current_font_ && psize == current_size_ &&
      ((f && f->encoding && encoding_ && f->encoding == encoding_) ||
	   (!encoding_ || !strcmp(f->encoding, encoding_))))
    return;
  current_font_ = font; current_size_ = psize;

  // search the FontSize we have generated already:
  for (f = t->first; f; f = f->next)
    if (f->minsize <= size && f->maxsize >= size
        && (f->encoding==encoding_ ||
 	    !f->encoding ||
		(!encoding_ || !strcmp(f->encoding, encoding_)))) {
      goto DONE;
    }

  // run XListFonts if it has not been done yet:
  if (!t->xlist) {
    open_display();
    t->xlist = XListFonts(xdisplay, t->system_name, 100, &(t->n));
    if (!t->xlist || t->n<=0) {	// use variable if no matching font...
      t->first = f = new FontSize("variable",0);
      f->minsize = 0;
      f->maxsize = 32767;
      goto DONE;
    }
  }

  // now search the XListFonts results:
  {char* name = t->xlist[0];
  unsigned ptsize = 0;	// best one found so far
  char namebuffer[1024];	// holds scalable font name
  bool found_encoding = false;
  int m = t->n; if (m<0) m = -m;
  char* nname = 0;

  for (int n=0; n < m; n++) {

    char* thisname = t->xlist[n];
    // get the encoding field:
    const char* this_encoding = font_word(thisname, 13);
    if (*this_encoding++ && !strcmp(this_encoding, encoding_)) {
      // forget any wrong encodings when we find a correct one:
      if (!found_encoding) ptsize = 0;
      found_encoding = true;
    } else {
      // ignore later wrong encodings after we find a correct one:
      if (found_encoding) continue;
    }
    // get the pixelsize field:
    const char* c = font_word(thisname,7);
    unsigned thissize = *c ? atoi(++c) : 32767;
    if (thissize == size) {
      // Use an exact match:
      name = nname = thisname;
      ptsize = size;
      if (found_encoding) break;
    } else if (!thissize && ptsize!=size) {
      // Use a scalable font if no exact match found:
      int l = c-thisname;
      memcpy(namebuffer,thisname,l);
      // print the pointsize into it:
      if (size>=100) namebuffer[l++] = size/100+'0';
      if (size>=10) namebuffer[l++] = (size/10)%10+'0';
      namebuffer[l++] = (size%10)+'0';
      while (*c == '0') c++;
      strcpy(namebuffer+l,c);
      name = namebuffer;
      nname = thisname;
      ptsize = size;
    } else if (!ptsize ||	// no fonts yet
               thissize < ptsize && ptsize > size || // current font too big
               thissize > ptsize && thissize <= size // current too small
      ) {
      // Use the nearest fixed size font:
      name = nname = thisname;
      ptsize = thissize;
    }
  }

  // If we didn't find an exact match, search the list to see if we already
  // found this font:
  if (ptsize != size || !found_encoding) {
    for (f = t->first; f; f = f->next) {
      if (f->minsize <= ptsize && f->maxsize >= ptsize &&
	  (!found_encoding || !strcmp(f->encoding, encoding_))) {
	if (f->minsize > size) f->minsize = size;
	if (f->maxsize < size) f->maxsize = size;
	goto DONE;
      }
    }
  }

  // okay, we definately have some name, make the font:
  f = new FontSize(name,nname);
  // we pretend it has the current encoding even if it does not, so that
  // it is quickly matched when searching for it again with the same
  // encoding:
  f->encoding = encoding_;
  if (ptsize < size) {f->minsize = ptsize; f->maxsize = size;}
  else {f->minsize = size; f->maxsize = ptsize;}
  f->next = t->first;
  t->first = f;}
 DONE:
  if (f != current) {
    current = f;
    font_gc = 0;
  }
}

////////////////////////////////////////////////////////////////

// The predefined fonts that fltk has:
static IFont fonts [] = {
  {{"helvetica",0}, "-*-helvetica-medium-r-normal--*",	fonts+1, fonts+2},
  {{"helvetica",1}, "-*-helvetica-bold-r-normal--*", 	fonts+1, fonts+3},
  {{"helvetica",2}, "-*-helvetica-medium-o-normal--*",	fonts+3, fonts+2},
  {{"helvetica",3}, "-*-helvetica-bold-o-normal--*",	fonts+3, fonts+3},
  {{"courier", 0},  "-*-courier-medium-r-normal--*",	fonts+5, fonts+6},
  {{"courier", 1},  "-*-courier-bold-r-normal--*",	fonts+5, fonts+7},
  {{"courier", 2},  "-*-courier-medium-o-normal--*",	fonts+7, fonts+6},
  {{"courier", 3},  "-*-courier-bold-o-normal--*",	fonts+7, fonts+7},
  {{"times", 0},    "-*-times-medium-r-normal--*",	fonts+9, fonts+10},
  {{"times", 1},    "-*-times-bold-r-normal--*",	fonts+9, fonts+11},
  {{"times", 2},    "-*-times-medium-i-normal--*",	fonts+11,fonts+10},
  {{"times", 3},    "-*-times-bold-i-normal--*",	fonts+11,fonts+11},
  {{"symbol", 0},   "-*-symbol-*",			fonts+12,fonts+12},
  {{"lucidatypewriter", 0}, "-*-lucidatypewriter-medium-r-normal-sans-*", fonts+14,fonts+14},
  {{"lucidatypewriter", 1}, "-*-lucidatypewriter-bold-r-normal-sans-*", fonts+14,fonts+14},
  {{"zapf dingbats", 0}, "-*-*zapf dingbats-*",		fonts+15,fonts+15},
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
fltk::Font* const fltk::SCREEN_FONT		= &(fonts[13].f);
fltk::Font* const fltk::SCREEN_BOLD_FONT	= &(fonts[14].f);
fltk::Font* const fltk::ZAPF_DINGBATS		= &(fonts[15].f);

/*! For back-compatabilty with FLTK1, this turns an integer into one
  of the built-in fonts. 0 = HELVETICA. */
fltk::Font* fltk::font(int i) {return &(fonts[i%16].f);}

//
// End of "$Id: Font_xlfd.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
