//
// "$Id: drawtext.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Copyright 1998-2003 by Bill Spitzak and others.
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

#include <fltk/draw.h>
#include <fltk/math.h>
#include <fltk/Font.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
using namespace fltk;

/* These are for getting the default leading: */
#include <fltk/Widget.h>
#include <fltk/Style.h>

////////////////////////////////////////////////////////////////
// @-sign commands

#include <fltk/Symbol.h>

static Font* normal_font;
static float normal_size;
static Color normal_color;
static float dy, nextdy;
static Flags flags;

class NormalSymbol : public Symbol {
public:
  NormalSymbol() : Symbol("n") {}
  void _draw(const Rectangle&) const {
    setcolor(normal_color);
    setfont(normal_font, normal_size);
  }
  void _measure(int& w, int& h) const {
    setfont(normal_font, normal_size);
    nextdy = 0;
    w = 0; h = int(normal_size);
  }
};
static const NormalSymbol normalsymbol;

class BoldSymbol : public Symbol {
public:
  BoldSymbol() : Symbol("b") {}
  void _draw(const Rectangle&) const {
    setfont(getfont()->bold(), getsize());
  }
  void _measure(int& w, int& h) const {
    setfont(getfont()->bold(), getsize());
    w = 0;
  }
};
static const BoldSymbol boldsymbol;

class ItalicSymbol : public Symbol {
public:
  ItalicSymbol() : Symbol("i") {}
  void _draw(const Rectangle&) const {
    setfont(getfont()->italic(), getsize());
  }
  void _measure(int& w, int& h) const {
    setfont(getfont()->italic(), getsize());
    w = 0;
  }
};
static const ItalicSymbol italicsymbol;

class FixedSymbol : public Symbol {
public:
  FixedSymbol(const char* n) : Symbol(n) {}
  void _draw(const Rectangle&) const {
    setfont(fltk::COURIER, getsize());
  }
  void _measure(int& w, int& h) const {
    setfont(fltk::COURIER, getsize());
    w = 0;
  }
};
static const FixedSymbol fixedsymbolf("f");
static const FixedSymbol fixedsymbolt("t");

class ColorSymbol : public Symbol {
public:
  ColorSymbol() : Symbol("C") {}
  void _draw(const Rectangle&) const {
    if (!drawflags(INACTIVE_R))
      setcolor(parsecolor(text()+1, text_length()-1));
  }
  void _measure(int& w, int& h) const {
    w = 0;
  }
};
static const ColorSymbol colorsymbol;

// Read a number and return n*getsize()/12 or just n if the number
// ends with 'p'.  Other units may be added in the future.
static double get_pixels(const char* text) {
  char* p;
  double number = strtod(text,&p);
  if (*p == 'p') return number;
  return number*getsize()/12;
}

class SizeSymbol : public Symbol {
public:
  SizeSymbol(const char* n) : Symbol(n) {}
  void dostuff() const {
    double n = get_pixels(text()+1);
    if (n < 0)
      n = getsize()/(1-n/getsize());
    else if (*(text()+1)=='+')
      n = getsize()+n;
    else if (n <= 0)
      n = normal_size;
    setfont(getfont(), (float) n);
  }
  void _draw(const Rectangle&) const {dostuff();}
  void _measure(int& w, int& h) const {dostuff(); w = 0;}
};
static const SizeSymbol sizesymbols("s");
static const SizeSymbol sizesymbolS("S");

class NothingSymbol : public Symbol {
public:
  NothingSymbol() : Symbol(".") {}
  void _draw(const Rectangle&) const {}
  void _measure(int& w, int& h) const {
    w = h = 0;
    flags |= RAW_LABEL;
  }
};
static const NothingSymbol nothingsymbol;

class DxSymbol : public Symbol {
public:
  DxSymbol() : Symbol("x") {}
  void _draw(const Rectangle&) const {}
  void _measure(int& w, int& h) const {
    // unsigned is reserverd for future use:
    if (text()[1] != '+' && text()[1] != '-') {w = 0; return;}
    w = int(get_pixels(text()+1)+.5f);
  }
};
static const DxSymbol dxsymbol;

class DySymbol : public Symbol {
public:
  DySymbol() : Symbol("y") {}
  void _draw(const Rectangle&) const {}
  void _measure(int& w, int& h) const {
    w = 0;
    // unsigned is reserverd for future use:
    if (text()[1] != '+' && text()[1] != '-') return;
    nextdy -= (float)get_pixels(text()+1);
  }
};
static const DySymbol dysymbol;

static Color bgboxcolor;

class BgBox : public Symbol {
public:
  BgBox() : Symbol("B") {}
  void _draw(const Rectangle&) const {}
  void _measure(int& w, int& h) const {
    bgboxcolor = parsecolor(text()+1, text_length()-1);
    w = h = 0;
  }
};
static const BgBox bgbox;

class LeftSymbol : public Symbol {
public:
  LeftSymbol() : Symbol("l") {}
  void _draw(const Rectangle&) const {}
  void _measure(int& w, int& h) const {
    flags = (flags & ~ALIGN_RIGHT) | ALIGN_LEFT;
    w = h = 0;
  }
};
static const LeftSymbol leftsymbol;

class CenterSymbol : public Symbol {
public:
  CenterSymbol() : Symbol("c") {}
  void _draw(const Rectangle&) const {}
  void _measure(int& w, int& h) const {
    flags &= ~(ALIGN_LEFT|ALIGN_RIGHT);
    w = h = 0;
  }
};
static const CenterSymbol centersymbol;

class RightSymbol : public Symbol {
public:
  RightSymbol() : Symbol("r") {}
  void _draw(const Rectangle&) const {}
  void _measure(int& w, int& h) const {
    flags = (flags & ~ALIGN_LEFT) | ALIGN_RIGHT;
    w = h = 0;
  }
};
static const RightSymbol rightsymbol;

/* Other symbols supported in fltk1.1:

   @l = set size to 24
   @m = set size to 18
   @s = set size to 11 // could be emulated when no number
   @Fnumber = set to fltk font n
   @N = set to grayed-out color
   @- = draw an inset divider line
   @u or @_ = underscore rest of text

*/
////////////////////////////////////////////////////////////////

const int* fltk::column_widths_ = 0;

// The implementation splits the text up into individual "symbols"
// Each has an x/y position and a segment of text and a pointer to
// a Symbol object that uses that information to draw it. These calls
// may also have side effects like changing the color or font.
struct Segment {
  const Symbol* symbol;
  const char* start;
  const char* end; // points after last character
  float x,y,w,h; // Arguments to symbol command
  int ascent, height; // used to calculate vertical alignment
  int column; // used to allow per-column clipping
};

// The current set of segments. Call add() to add a new one:
static Segment* segments;
static int segment_count;
static int segment_array_size;
static int current_column;

// Create and append a new segment:
static /*inline*/ void add(const Symbol* symbol,
			   const char* start,
			   const char* end,
			   float x, float y, float w, float h,
			   int ascent, int height
			   )
{
  // enlarge the array if necessary:
  if (segment_count >= segment_array_size) {
    segment_array_size = segment_count ? 2*segment_count : 32;
    Segment* new_array = new Segment[segment_array_size];
    memcpy(new_array, segments, segment_count*sizeof(Segment));
    delete[] segments;
    segments = new_array;
  }
  Segment& s = segments[segment_count++];
  s.symbol = symbol;
  s.start = start;
  s.end = end;
  s.x = x;
  s.y = y+dy;
  s.w = w;
  s.h = h;
  s.ascent = ascent;
  s.height = height;
  s.column = current_column;
}

// As we build the segments we keep track of the bottom-right corner:
static float max_x;
static float max_y;

// Move a line horizontally for alignment:
static float align(int first_segment,
		   float x, float y, float w, float r) {
  if (r > max_x) max_x = r;
  float dx;
  if (flags & ALIGN_RIGHT) {
    dx = w-(r-x);
    if ((flags & ALIGN_LEFT) && dx > 0) dx = 0;
    if (x && dx < 0) dx = 0;
  } else if (flags & ALIGN_LEFT) {
    dx = 0;
  } else {
    dx = (w-(r-x))/2;
    if (x && dx < 0) dx = 0;
  }
  int max_a = segments[first_segment].ascent;
  int i;
  for (i = first_segment+1; i < segment_count; i++) {
    int a = segments[i].ascent; if (a > max_a) max_a = a;
  }
  float my = y;
  for (i = first_segment; i < segment_count; i++) {
    segments[i].x += dx;
    float dy = (float)max_a-segments[i].ascent;
    segments[i].y += dy;
    float yy = y+segments[i].height+dy;
    if (yy > my) my = yy;
  }
  if (my > max_y) max_y = my;
  return my;
}

bool fl_hide_underscore; // set by Choice

// The amount returned here should exactly match the ascent returned by setsa:
int Rectangle::baseline_y() const {
  return y_ + (int(h_ + getascent() - getdescent() + 1.5) >> 1);
}

static inline void setsa(int& spacing, int& ascent) {
  float H = getsize()+Widget::default_style->leading();
  spacing = int(H+.5);
  ascent = int(H + getascent() - getdescent() + 1.5) >> 1;
}

// Find all the segments in a section of raw text and arrange them as
// though they are aligned with the top-left corner at x,y and wrap at
// w.  This is complex because a stream of letters may turn into 1 or
// 2 segments due to word wrapping.  Returns the y of the last
// line. Sets max_x to the rightmost edge, max_y to bottom edge
static void wrap(
  const char* start,
  const char* end,
  float ix, float y, float w,
  float (*getwidth)(const char*, int) // lets you change the width function
  )
{
  float x = ix; // accumulated position if we don't wrap
  const char* word_end = start; // where to end it if we wrap
  const char* word_start = start; // where to start new segment
  float width = 0;		// width of current text
  // start..p indicates current text segment being built
  int first_segment = segment_count;

  int spacing, ascent; setsa(spacing,ascent);
  dy = nextdy = 0;

  for (const char* p = start; ;) {
    // figure out what we have next:
    const Symbol* symbol = 0;
    bool underscore = false;
    const char* q = p;

    // stop only on "interesting" characters:
  SKIP_LETTERS:
    if (p >= end) ;
    else if (*p==' ' && (flags&ALIGN_WRAP)) {
      // spaces, find the end of all the sequential ones:
      q = p+1; while (q<end && *q==' ') q++;
    } else if (*p=='@' && !(flags&RAW_LABEL) && p+1<end) {
      q = p+1;
      if (*q == '@') q++;
      else if (*q == ';') end = p;
      else {
	while (q<end && *q && !isspace((uchar)*q) && *q!='@' && *q!=';') q++;
	symbol = Symbol::find(p+1,q);
	if (!symbol) {++p; goto SKIP_LETTERS;}
      }
    } else if (*p=='&' && !(flags&RAW_LABEL) && p+1<end) {
      q = p+1;
      if (*q == '&') q++;
      else {
	if (isalnum(*q)) underscore = true;
	else {++p; goto SKIP_LETTERS;}
      }
    } else {
      ++p; goto SKIP_LETTERS;
    }

    const float text_w = getwidth(word_end, p-word_end);
    int symbol_w, symbol_h;
    if (symbol) {
      Symbol::text(p+1,q-p-1);
      symbol_w = symbol_h = int(getsize());
      symbol->measure(symbol_w, symbol_h);
      Symbol::text("",0);
    } else {
      symbol_w = symbol_h = 0;
    }

    // Wrap the current block of text:
    if (flags & ALIGN_WRAP) {
      if (x+width+text_w+symbol_w > ix+w && word_start > start) {
	// break before this word
	if (word_end > start) {
	  add(0, start, word_end, x, y+ascent, width, getsize(),
	      ascent, spacing);
	  x += width;
	}
	y = align(first_segment, ix, y, w, x);
	x = ix;
	width = 0;
	start = word_end = word_start;
	first_segment = segment_count;
	// back up and start formatting from start of new line:
	if (word_start < p) {p = word_start+1; continue;}
      } else {
	width += text_w;
	word_end = p;
      }
    }
    // spaces are just added to the current block:
    if (*p == ' ') {p = word_start = q; continue;}

    // add text before this next object:
    if (start < p) {
      if (p > word_end) width += text_w;
      add(0, start, p, x, y+ascent, width, getsize(), ascent, spacing);
      x += width;
    }
    if (p >= end) break;
    width = 0;
    if (underscore) {
      if (!fl_hide_underscore) {
	const char* us = "_"; 
	// on the some platforms the underscore sticks to the bottom of the letter
	// so add space to enhance the shortcut appearance 
	// finally, the shortcut symbol should be centered for all platforms 
	// whatever the font is so add a potential delta
	float xdelta = (getwidth(p,1)-getwidth(us,1))/2.0;
	float ydelta =
#ifndef	_WIN32
	    (getdescent())/2.0;
#else
	(getdescent())/2.0 - 1.0;
#endif
	add(0, us, us+1, x+xdelta, y+ascent+ydelta, getsize(),getsize(), ascent, spacing);
}
      p = q;
    } else if (symbol) {
      if (!symbol_w) {
	// it may have changed the font or dx,dy:
	setsa(spacing, ascent);
	//if (dy > 0) spacing += dy; else ascent -= dy;
	add(symbol, p+1, q, x, y+ascent, float(symbol_w), float(symbol_h), 0, 0);
	dy = nextdy;
      } else {
	// center the symbol in the vertical size of current font:
	//int a = ascent-((spacing-int(H+1.5f))>>1);
	int a = int(getascent()-getdescent()+symbol_h+1)>>1;
	add(symbol, p+1, q, x, y, float(symbol_w), float(symbol_h), a, symbol_h);
	x += symbol_w;
      }
      // skip the terminating space or semicolon:
      if (q < end && *q==';') q++;
      p = q;
    } else {
      // handle @@ and &&
      p++; q = p+1;
    }
    // start the next block of text:
    word_start = word_end = start = p; p = q;
  }
  if (segment_count>first_segment) align(first_segment, ix, y, w, x);
  // handle blank line:
  else {float yy = y+spacing; if (yy > max_y) max_y = yy;}
}

// Split at newlines and tabs into sections and then call wrap on them
// Return value is the y height of the text. The width is stored in
// max_x. W is only used if ALIGN_WRAP is on.
static float split(
    const char* str,
    int W,
    Flags flags,
    float (*getwidth)(const char*, int) // lets you change the width function
    )
{
  normal_font = getfont();
  normal_size = getsize();
  ::flags = flags;
  const int* column = column_widths_;

  segment_count = 0;
  current_column = 0;
  float x = 0;
  float y = 0;
  max_x = max_y = 0;
  const char* p = str;
  for (;;) {
    // find the next newline or tab:
    float w; // width to format this segment into
    if (!*p || *p == '\n') {
      w = W-x;
    } else if (*p == '\t') {
      if (column && *column) {
	w = (float)(*column++);
      }
      else w = ((p-str+4)&-4)*getwidth("2",1);
    } else {
      p++;
      continue;
    }
    wrap(str, p, x, y, w, getwidth);
    if (!*p) {
      return max_y;
    } else if (*p == '\n') {
      x = 0; y = max_y;
      column = column_widths_;
      current_column = 0;
      //::flags = flags;
    } else { // tab
      x += w;
      current_column++;
    }
    // add symbol to reset fonts to normal:
    //float a,b; normalsymbol._measure(a,b);
    //add(segment_count, &normalsymbol, 0, 0, 0, 0);
    str = ++p;
  }
}

/*!
  This is the fancy string-drawing function that is used to draw all
  labels in fltk. The string is formatted and aligned inside the
  passed rectangle. This also:
  - Breaks the text into lines at \\n characters. Word-wraps (if
    flags has fltk::ALIGN_WRAP set) so the words fit in the
    columns.
  - Looks up "@xyz;" sequeces to see if they are a Symbol, if so it
    prints that symbol instead. This is skipped if the flags has
    fltk::RAW_LABEL set.
  - Parses "&x" combinations to produce Microsoft style underscores,
    unless RAW_LABEL flag is set.
  - Splits it at every \\t tab character and uses column_widths() to
    set each section into a column.
*/
void fltk::drawtext(const char* str, const Rectangle& r1, Flags flags)
{
  if (!str || !*str) return; // speeds up very common widgets
  Rectangle r; transform(r1,r);
  push_matrix();
  load_identity();
  drawtext(drawtext_transformed, getwidth, str, r, flags);
  pop_matrix();
  setfont(normal_font, normal_size);
  setcolor(normal_color);
}

/*!
  Provides access to \e some of the @-string formatting for another
  graphics API. Most symbols will not work, but you will be able to
  access the line break and justifications, and commands that change
  the font, size, and color. I use this to format labels for that
  are drawn in OpenGL.

  \a textfunction is called to draw all text.  It's arguments are
  a pointer to a UTF-8 string, the length in bytes of that string, and
  two float numbers for the x and y to draw the text at.

  \a textfunction may use getfont(), getsize(), and getcolor() to
  find out the settings and recreate them on the output device.
*/

void fltk::drawtext(void (*textfunction)(const char*,int,float,float),
		    float (*getwidth)(const char*, int),
		    const char* str, const Rectangle& r, Flags flags)
{
  bgboxcolor = 0;
  normal_color = getcolor();
  int h = int(split(str, r.w(), flags, getwidth)+.5);
  int dy;
  if (flags & ALIGN_BOTTOM) {
    dy = r.b()-h;
    if ((flags & ALIGN_TOP) && dy > r.y()) dy = r.y();
  } else if (flags & ALIGN_TOP) {
    dy = r.y();
  } else {
    dy = r.y()+((r.h()-h)>>1);
  }
  setfont(normal_font, normal_size);
  if (bgboxcolor && !drawflags(INACTIVE_R)) {
    setcolor(bgboxcolor);
    int w = int(max_x+.5);
    int dx = 0;
    if (flags & ALIGN_RIGHT) {
      dx = r.w()-w;
      if (flags & ALIGN_LEFT) if (dx > 0) dx = 0;
    } else if (!(flags & ALIGN_LEFT)) {
      dx = (r.w()-w)>>1;
    }
    fillrect(r.x()+dx, int(dy), w, h);
    setcolor(normal_color);
  }
  if (column_widths_) {
    current_column = -1;
    push_clip(r);
    for (h = 0; h < segment_count; h++) {
      Segment& s = segments[h];
      if (s.column != current_column) {
	current_column = s.column;
	int i, xx = r.x();
	for (i=0; i<current_column; i++) xx += column_widths_[i];
	pop_clip();
	if (segment_count > 1)
	  push_clip(xx, r.y(), column_widths_[current_column], r.h());
	else
	  push_clip(xx, r.y(), r.w(), r.h());
      }
      if (s.symbol) {
	Symbol::text(s.start,s.end-s.start);
	s.symbol->draw(Rectangle(int(s.x)+r.x(), int(s.y+dy), int(s.w), int(s.h)));
      } else {
	textfunction(s.start, s.end-s.start, s.x+r.x(), s.y+dy);
      }
    }
    pop_clip();
  } else {
    for (h = 0; h < segment_count; h++) {
      Segment& s = segments[h];
      if (s.symbol) {
	Symbol::text(s.start,s.end-s.start);
	s.symbol->draw(Rectangle(int(s.x)+r.x(), int(s.y+dy), int(s.w), int(s.h)));
      } else {
	textfunction(s.start, s.end-s.start, s.x+r.x(), s.y+dy);
      }
    }
  }
  Symbol::text("",0);
}

/**
  Measure the size of box necessary for drawtext() to draw the given
  string inside of it. The \a flags are used to set the alignment,
  though this should not make a difference except for
  fltk::ALIGN_WRAP.  To correctly measure wrap \a w must be preset to
  the width you want to wrap at if fltk::ALIGN_WRAP is on in the
  flags!  \a w and \a h are changed to the size of the resulting box.
*/
void fltk::measure(const char* str, int& w, int& h, Flags flags) {
#ifdef DEBUG
  // remove warning from Valgrind/Purify:
  if (!(flags & ALIGN_WRAP)) w = 0;
#endif
  if (!str || !*str) {w = 0; h = int(getsize()+.5); return;}
  h = int(split(str, w, flags, getwidth)+.5);
  w = int(max_x+.5);
  setfont(normal_font, normal_size);
}

/**
  This lets you pass your own measurement function to measure the widths
  of printed text. Also returns floating point sizes.
*/
void fltk::measure(float (*getwidth)(const char*, int),
		   const char* str, float& w, float& h, Flags flags)
{
#ifdef DEBUG
  // remove warning from Valgrind/Purify:
  if (!(flags & ALIGN_WRAP)) w = 0;
#endif
  h = split(str, int(w), flags, getwidth);
  w = max_x;
  setfont(normal_font, normal_size);
}

//
// End of "$Id: drawtext.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
