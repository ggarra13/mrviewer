// "$Id: Image.h 8636 2011-05-06 08:01:12Z bgbnbigben $"
//
// Copyright 1998-2005 by Bill Spitzak and others.
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

#ifndef fltk_Image_h
#define fltk_Image_h

#include "Symbol.h"
# undef OPAQUE // fix for windows.h

namespace fltk {

class FL_API Widget;
struct Picture; // secret internal system-specific data

class FL_API Image : public Symbol {

  PixelType pixeltype_; int w_, h_;
  Picture* picture;
  int flags; enum {COPIED=1, FETCHED=2, FORCEARGB32=4, MEASUREFETCH=8};

  static unsigned long memused_;

public:

  Image(const char* name=0) :
    Symbol(name), pixeltype_(fltk::RGB32), w_(12), h_(12),
    picture(0), flags(MEASUREFETCH) {}
  Image(int w, int h, const char* name=0) :
    Symbol(name), pixeltype_(fltk::RGB32), w_(w), h_(h),
    picture(0), flags(0) {}
  Image(PixelType p, int w, int h, const char* name=0) :
    Symbol(name), pixeltype_(p), w_(w), h_(h),
    picture(0), flags(0) {}
  Image(const uchar* d, PixelType p, int w, int h) :
    Symbol(), picture(0) {setimage(d,p,w,h);}
  Image(const uchar* d, PixelType p, int w, int h, int linedelta) :
    Symbol(), picture(0) {setimage(d,p,w,h,linedelta);}
  ~Image();

  PixelType pixeltype() const {return pixeltype_;}
  int depth() const {return fltk::depth(pixeltype_);}
  int w() const {return w_;}
  int width() const {return w_;}
  int h() const {return h_;}
  int height() const {return h_;}

  void setpixeltype(PixelType);
  void setsize(int w, int h);
  void setpixels(const uchar* d, const Rectangle&, int linedelta);
  void setpixels(const uchar* d, const Rectangle& r) {setpixels(d,r,depth()*r.w());}
  void setpixels(const uchar* d, int y);
  uchar* linebuffer(int y);

  void setimage(const uchar* d, PixelType p, int w, int h, int linedelta);
  void setimage(const uchar* d, PixelType p, int w, int h) {setimage(d,p,w,h,fltk::depth(p)*w);}

  uchar* buffer();
  const uchar* buffer() const;
  void set_forceARGB32();
  void clear_forceARGB32();
  bool forceARGB32() const {return (flags&FORCEARGB32) != 0;}
  PixelType buffer_pixeltype() const;
  int buffer_depth() const;
  int buffer_width() const;
  int buffer_height() const;
  int buffer_linedelta() const;
  void buffer_changed() {flags &= ~COPIED;}
  void destroy();

  void draw(int x, int y) const;
  void draw(const Rectangle& r) const {_draw(r);}
  void draw(const Rectangle& from, const Rectangle& to) const;
  void draw_diced(const Rectangle& R);

  void _draw(const Rectangle&) const; // Symbol virtual method
  void _measure(int& W, int& H) const; // Symbol virtual method
  bool fills_rectangle() const; // Symbol virtual method
  virtual bool fetch(); // for image file reading subclasses
  void fetch_if_needed() const;
  void refetch() {flags &= ~FETCHED;}

  unsigned long mem_used() const;
  static unsigned long total_mem_used() {return memused_;}

  // for back compatability with fltk1 only:
  void label(Widget* o);

  // see also: GSave
  void make_current();

};

}

#endif

//
// End of "$Id: Image.h 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
