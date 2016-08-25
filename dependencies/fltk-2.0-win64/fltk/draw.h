// "$Id: draw.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/*
  The FLTK drawing library, used by all widgets to draw themselves.

  These functions can only be called when FLTK is setup to draw
  things. This is only true:
  - Inside the Widget::draw() virtual function.
  - Inside the Symbol::draw() virtual function.
  - After calling Widget::make_current(), before calling wait() or flush().
  Calling the drawing functions at other times produces undefined results,
  including crashing.
*/

#ifndef fltk_draw_h
#define fltk_draw_h

#include "Flags.h" // for alignment values
#include "Color.h"
#include "Rectangle.h"
#include "PixelType.h"

namespace fltk {

/// \name fltk/draw.h
//@{

struct Font;
class Style;

class FL_API GSave {
  void* data[4]; // hopefully big enough for everybody...
 public:
  GSave();
  ~GSave();
};

// Transformation
FL_API void push_matrix();
FL_API void pop_matrix();
FL_API void scale(float x, float y);
FL_API void scale(float x);
FL_API void translate(float x, float y);
FL_API void translate(int x, int y);
FL_API void rotate(float d);
FL_API void concat(float, float, float, float, float, float);
FL_API void load_identity();

// get and use transformed positions:
FL_API void transform(float& x, float& y);
FL_API void transform_distance(float& x, float& y);
FL_API void transform(int& x, int& y);
FL_API void transform(const Rectangle& from, Rectangle& to);
FL_API void transform(int& x, int& y, int& w, int& h);

// Clipping
FL_API void push_clip(const Rectangle&);
//! Same as push_clip(Rectangle(x,y,w,h)) but faster:
FL_API void push_clip(int X,int Y, int W, int H);
FL_API void clipout(const Rectangle&);
FL_API void pop_clip();
FL_API void push_no_clip();
FL_API bool not_clipped(const Rectangle&);
FL_API int intersect_with_clip(Rectangle&);

FL_API void setcolor(Color);
FL_API void setcolor_alpha(Color, float alpha);
extern FL_API Color current_color_;
inline Color getcolor() {return current_color_;}

extern FL_API Color current_bgcolor_;
inline void setbgcolor(Color c) {current_bgcolor_ = c;}
inline Color getbgcolor() {return current_bgcolor_;}

extern FL_API const Style* drawstyle_;
void FL_API drawstyle(const Style* s, Flags);
inline const Style* drawstyle() {return drawstyle_;}

extern FL_API Flags drawflags_;
inline void setdrawflags(Flags f) {drawflags_ = f;}
inline Flags drawflags() {return drawflags_;}
inline Flags drawflags(Flags f) {return drawflags_ & f;}

// line type:
FL_API void line_style(int, float width=0, const char* dashes=0);
enum {
  SOLID	= 0,
  DASH	= 1,
  DOT	= 2,
  DASHDOT	= 3,
  DASHDOTDOT	= 4,

  CAP_FLAT	= 0x100,
  CAP_ROUND	= 0x200,
  CAP_SQUARE	= 0x300,

  JOIN_MITER	= 0x1000,
  JOIN_ROUND	= 0x2000,
  JOIN_BEVEL	= 0x3000
};
extern FL_API int line_style_;
inline FL_API int line_style() {return line_style_;}
extern FL_API float line_width_;
inline FL_API float line_width() {return line_width_;}
extern FL_API const char* line_dashes_;
inline FL_API const char* line_dashes() {return line_dashes_;}

// Path construction
FL_API void newpath();
FL_API void addvertex(float x, float y);
FL_API void addvertex(int x, int y);
FL_API void addvertices(int n, const float v[][2]);
FL_API void addvertices(int n, const int v[][2]);
FL_API void addvertices_transformed(int n, const float v[][2]);
FL_API void addcurve(float,float, float,float, float,float, float,float);
FL_API void addarc(float x,float y,float w,float h, float a1, float a2);
FL_API void addpie(const Rectangle& r, float a, float a2);
FL_API void addchord(const Rectangle& r,float a,float a2);
FL_API void closepath();

// Shapes and lines
FL_API void strokepath();
FL_API void fillpath();
FL_API void fillstrokepath(Color);

FL_API void fillrect(int, int, int, int);
inline void fillrect(const Rectangle& r) {fillrect(r.x(),r.y(),r.w(),r.h());}
FL_API void strokerect(int, int, int, int);
inline void strokerect(const Rectangle& r) {strokerect(r.x(),r.y(),r.w(),r.h());}
FL_API void drawline(int x0, int y0, int x1, int y1);
FL_API void drawline(float x0, float y0, float x1, float y1);
FL_API void drawpoint(int x, int y);
FL_API void drawpoint(float x, float y);

// Text
FL_API void setfont(Font*, float size);
FL_API void setfont(const char*, float size);
FL_API void setfont(const char*, int attributes, float size);

// change the encoding used to draw bytes (depreciated)
extern FL_API const char* encoding_;
inline const char* get_encoding() {return encoding_;}
FL_API void set_encoding(const char*);

// information you can get about the current font+size+encoding:
extern FL_API Font* current_font_;
extern FL_API float current_size_; // should be 2x2 transformation matrix
inline Font* getfont() {return current_font_;}
inline float getsize() {return current_size_;}

// measure things in the current font:
FL_API float getwidth(const char*);
FL_API float getwidth(const char*, int length);
FL_API float getascent();
FL_API float getdescent();

// draw using current font:
FL_API void drawtext_transformed(const char*, int n, float x, float y);
FL_API void drawtext(const char*, float x, float y);
FL_API void drawtext(const char*, int length, float x, float y);

// the label text formatter:
FL_API void measure(const char*, int& w, int& h, Flags = 0);
FL_API void measure(float (*getwidth)(const char*, int),const char*, float& w, float& h, Flags = 0);
FL_API void drawtext(const char*, const Rectangle&, Flags);
FL_API void drawtext(void (*textfunction)(const char*,int,float,float),
		     float (*getwidth)(const char*, int),
		     const char* str, const Rectangle& r, Flags flags);

// set where \t characters go in label text formatter:
extern FL_API const int* column_widths_;
inline const int* column_widths() {return column_widths_;}
inline void column_widths(const int* i) {column_widths_ = i;}
// see also Symbol.h for @-sign commands

// Images
FL_API void drawimage(const uchar*, PixelType, const Rectangle&);
FL_API void drawimage(const uchar*, PixelType, const Rectangle&, int linedelta);

typedef const uchar* (*DrawImageCallback)(void* data, int x, int y, int w, uchar* buffer);
FL_API void drawimage(DrawImageCallback, void*, PixelType, const Rectangle&);

FL_API uchar *readimage(uchar *p, PixelType, const Rectangle&);
FL_API uchar *readimage(uchar *p, PixelType, const Rectangle&, int linedelta);

FL_API void scrollrect(const Rectangle&, int dx, int dy,
		       void (*draw_area)(void*, const Rectangle&), void*);

#ifndef DOXYGEN /* depreciated: */
FL_API void drawframe(const char* s, int x, int y, int w, int h);
FL_API void drawframe2(const char* s, int x, int y, int w, int h);
FL_API void overlay_rect(int,int,int,int);
FL_API void overlay_clear();
#endif

//@}

}

#endif

//
// End of "$Id: draw.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
