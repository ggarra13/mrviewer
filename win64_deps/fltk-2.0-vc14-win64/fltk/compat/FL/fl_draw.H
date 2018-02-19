#ifndef fl_draw_H
#define fl_draw_H

#include "Enumerations.H"
#include <fltk/draw.h>
#include <fltk/Font.h>

inline void fl_color(fltk::Color c) {fltk::setcolor(c);}
inline void fl_color(uchar r, uchar g, uchar b) {fltk::setcolor(fltk::color(r,g,b));}
inline fltk::Color fl_color() {return fltk::getcolor();}

// clip:
inline void fl_push_clip(int x, int y, int w, int h) {fltk::push_clip(x,y,w,h);}
inline void fl_push_no_clip() {fltk::push_no_clip();}
#define fl_clip		fl_push_clip
#define fl_pop_clip	fltk::pop_clip
inline int fl_not_clipped(int x, int y, int w, int h) {return fltk::not_clipped(fltk::Rectangle(x,y,w,h));}
inline int fl_clip_box(int X, int Y, int W, int H, int& x, int& y, int& w, int& h) {
  fltk::Rectangle r(X,Y,W,H);
  int i = intersect_with_clip(r);
  x = r.x(); y = r.y(); w = r.w(); h = r.h();
  return i;
}

// points:
inline void fl_point(int x, int y) {fltk::drawpoint(x,y);}

// line type:
#define fl_line_style	fltk::line_style
enum {
  FL_SOLID	= fltk::SOLID,
  FL_DASH	= fltk::DASH,
  FL_DOT	= fltk::DOT,
  FL_DASHDOT	= fltk::DASHDOT,
  FL_DASHDOTDOT	= fltk::DASHDOTDOT,
  FL_CAP_FLAT	= fltk::CAP_FLAT,
  FL_CAP_ROUND	= fltk::CAP_ROUND,
  FL_CAP_SQUARE	= fltk::CAP_SQUARE,
  FL_JOIN_MITER	= fltk::JOIN_MITER,
  FL_JOIN_ROUND	= fltk::JOIN_ROUND,
  FL_JOIN_BEVEL	= fltk::JOIN_BEVEL
};

// rectangles tweaked to exactly fill the pixel rectangle:
inline void fl_rect(int x, int y, int w, int h) {fltk::strokerect(fltk::Rectangle(x,y,w,h));}
inline void fl_rect(int x, int y, int w, int h, fltk::Color c) {fltk::setcolor(c); fltk::strokerect(fltk::Rectangle(x,y,w,h));}
inline void fl_rectf(int x, int y, int w, int h) {fltk::fillrect(fltk::Rectangle(x,y,w,h));}
inline void fl_rectf(int x, int y, int w, int h, fltk::Color c) {fltk::setcolor(c); fltk::fillrect(fltk::Rectangle(x,y,w,h));}

// line segments:
inline void fl_line(int x0,int y0, int x1,int y1) {fltk::drawline(x0,y0,x1,y1);}
inline void fl_line(int x0,int y0, int x1,int y1, int x2,int y2) {
  fltk::addvertex(x0,y0);
  fltk::addvertex(x1,y1);
  fltk::addvertex(x2,y2);
  fltk::strokepath();
}

// closed line segments:
inline void fl_loop(int x0,int y0, int x1,int y1, int x2,int y2) {
  fltk::addvertex(x0,y0);
  fltk::addvertex(x1,y1);
  fltk::addvertex(x2,y2);
  fltk::closepath();
  fltk::strokepath();
}
inline void fl_loop(int x0,int y0, int x1,int y1, int x2,int y2, int x3,int y3) {
  fltk::addvertex(x0,y0);
  fltk::addvertex(x1,y1);
  fltk::addvertex(x2,y2);
  fltk::addvertex(x3,y3);
  fltk::closepath();
  fltk::strokepath();
}

// filled polygons
inline void fl_polygon(int x0,int y0, int x1,int y1, int x2,int y2) {
  fltk::addvertex(x0,y0);
  fltk::addvertex(x1,y1);
  fltk::addvertex(x2,y2);
  fltk::closepath();
  fltk::fillpath();
}
inline void fl_polygon(int x0,int y0, int x1,int y1, int x2,int y2, int x3,int y3) {
  fltk::addvertex(x0,y0);
  fltk::addvertex(x1,y1);
  fltk::addvertex(x2,y2);
  fltk::addvertex(x3,y3);
  fltk::closepath();
  fltk::fillpath();
}

// draw rectilinear lines, horizontal segment first:
inline void fl_xyline(int x, int y, int x1) {
  fltk::drawline(x,y,x1,y);
}
inline void fl_xyline(int x, int y, int x1, int y2) {
  fltk::addvertex(x,y);
  fltk::addvertex(x1,y);
  fltk::addvertex(x1,y2);
  fltk::strokepath();
}
inline void fl_xyline(int x, int y, int x1, int y2, int x3) {
  fltk::addvertex(x,y);
  fltk::addvertex(x1,y);
  fltk::addvertex(x1,y2);
  fltk::addvertex(x3,y2);
  fltk::strokepath();
}

// draw rectilinear lines, vertical segment first:
inline void fl_yxline(int x, int y, int y1) {
  fltk::drawline(x,y,x,y1);
}
inline void fl_yxline(int x, int y, int y1, int x2) {
  fltk::addvertex(x,y);
  fltk::addvertex(x,y1);
  fltk::addvertex(x2,y1);
  fltk::strokepath();
}
inline void fl_yxline(int x, int y, int y1, int x2, int y3) {
  fltk::addvertex(x,y);
  fltk::addvertex(x,y1);
  fltk::addvertex(x2,y1);
  fltk::addvertex(x2,y3);
  fltk::strokepath();
}

inline void fl_arc(int x,int y,int w,int h,float a,float a2) {fltk::addchord(fltk::Rectangle(x,y,w,h),a,a2); fltk::strokepath();}
inline void fl_pie(int x,int y,int w,int h,float a,float a2) {fltk::addpie(fltk::Rectangle(x,y,w,h),a,a2); fltk::fillpath();}
inline void fl_chord(int x,int y,int w,int h,float a,float a2) {fltk::addchord(fltk::Rectangle(x,y,w,h),a,a2); fltk::fillpath();}

// scalable drawing code (code in fl_vertex.C and fl_arc.C):
#define fl_push_matrix	fltk::push_matrix
#define fl_pop_matrix	fltk::pop_matrix
#define fl_scale	fltk::scale
#define fl_translate	fltk::translate
#define fl_rotate	fltk::rotate
#define fl_mult_matrix	fltk::concat
#define fl_begin_points	fltk::newpath
#define fl_begin_line	fltk::newpath
#define fl_begin_loop	fltk::newpath
#define fl_begin_polygon fltk::newpath
#define fl_vertex	fltk::addvertex
#define fl_curve	fltk::addcurve
inline void fl_arc(float x,float y,float r, float a1, float a2) {fltk::addarc(x-r,y-r,2*r,2*r,a1,a2);}
#define fl_circle	fltk::addcircle
#define fl_end_points	fltk::drawpoints
#define fl_end_line	fltk::strokepath
inline void fl_end_loop() {fltk::closepath();fltk::strokepath();}
#define fl_end_polygon	fltk::fillpath
#define fl_begin_complex_polygon fltk::newpath
#define fl_gap		fltk::closepath
#define fl_end_complex_polygon	fltk::fillpath

// current font: (size was an int in fltk1.1)
inline void fl_font(fltk::Font* f, float size) {fltk::setfont(f,size);}
inline void fl_font(int f, float size) {fltk::setfont(fltk::font(f),size);}
inline fltk::Font* fl_font() {return fltk::getfont();}
inline float fl_size() {return fltk::getsize();}

// information you can get about the current font:
inline float fl_height() {return fltk::getascent()+fltk::getdescent();}
inline float fl_height(int, float size) {return size;}
inline float fl_descent() {return fltk::getdescent();}
inline float fl_width(const char* c) {return fltk::getwidth(c);}
inline float fl_width(const char* c, int n) {return fltk::getwidth(c,n);}
inline float fl_width(char c) {return fltk::getwidth(&c,1);}

// draw using current font:
inline void fl_draw(const char* s, int x, int y) {fltk::drawtext(s,x,y);}
inline void fl_draw(const char* s, int n, int x, int y) {fltk::drawtext(s,n,x,y);}
inline void fl_measure(const char* s, int& x, int& y) {fltk::measure(s,x,y);}
inline void fl_draw(const char* s, int x,int y,int w,int h, fltk::Flags f) {fltk::drawtext(s,fltk::Rectangle(x,y,w,h),f);}

// boxtypes:
//void fl_frame(const char* s, int x, int y, int w, int h);
//void fl_frame2(const char* s, int x, int y, int w, int h);
// This no longer works because boxes take a style, not a color:
//  #include <fltk/Box.h>
//  inline void fl_draw_box(fltk::Box* b, int x, int y, int w, int h, fltk::Color c) {b->draw(x,y,w,h,c);}

// images:
static inline fltk::PixelType fromdelta(int d) {return d==1?fltk::MONO:d==3?fltk::RGB:fltk::RGBx;}
inline void fl_draw_image(const uchar* p, int x,int y,int w,int h, int delta, int ldelta) {fltk::drawimage(p,fromdelta(delta),fltk::Rectangle(x,y,w,h),ldelta);}
inline void fl_draw_image(const uchar* p, int x,int y,int w,int h, int delta) {fltk::drawimage(p,fromdelta(delta),fltk::Rectangle(x,y,w,h));}

inline void fl_draw_image_mono(const uchar* p, int x,int y,int w,int h, int delta, int ldelta) {fltk::drawimage(p,fltk::MONO,fltk::Rectangle(x,y,w,h),ldelta);}
inline void fl_draw_image_mono(const uchar* p, int x,int y,int w,int h, int delta=1) {fltk::drawimage(p,fltk::MONO,fltk::Rectangle(x,y,w,h));}

typedef fltk::DrawImageCallback Fl_Draw_Image_Cb;
inline void fl_draw_image(Fl_Draw_Image_Cb cb, void* p, int x,int y,int w,int h, int delta=3) {fltk::drawimage(cb,p,fromdelta(delta),fltk::Rectangle(x,y,w,h));}
inline void fl_draw_image_mono(Fl_Draw_Image_Cb cb, void* p, int x,int y,int w,int h, int delta=1) {fltk::drawimage(cb,p,fltk::MONO,fltk::Rectangle(x,y,w,h));}

inline uchar *fl_read_image(uchar *p, int x,int y, int w, int h, int alpha=0) {return fltk::readimage(p, alpha?fltk::RGBA:fltk::RGB, fltk::Rectangle(x,y,w,h));}

inline void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {fltk::setcolor(fltk::color(r,g,b)); fltk::fillrect(fltk::Rectangle(x,y,w,h));}

#define fl_draw_pixmap		fltk::draw_xpm
#define fl_measure_pixmap	fltk::measure_xpm
#define fl_scroll		fltk::scrollrect
#define fl_shortcut_label	fltk::key_name
#define fl_overlay_rect		fltk::overlay_rect
#define fl_overlay_clear	fltk::overlay_clear
#define fl_draw_symbol		fltk::draw_symbol
#define fl_add_symbol		fltk::add_symbol
#define fl_frame		fltk::drawframe2
#define fl_frame2		fltk::drawframe

#endif

//
// End of "$Id: fl_draw.H 5939 2007-08-02 14:19:28Z spitzak $".
//
