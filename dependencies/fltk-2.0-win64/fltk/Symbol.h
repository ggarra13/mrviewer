// "$Id: Symbol.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#ifndef fltk_Symbol_h
#define fltk_Symbol_h

#include "FL_API.h"
#include "Color.h"
#include "Rectangle.h"
#include "PixelType.h"

namespace fltk {

class Style;

class FL_API Symbol {
  const char* name_;
  Rectangle inset_;

  // Forbid use of copy contructor and assign operator
  Symbol & operator=(const Symbol &);
  Symbol(const Symbol &);

  static const char* text_;
  static unsigned text_length_;

 public:

  Symbol(const char* name=0);
  const char* name() const {return name_;}
  void name(const char*);

  virtual void _measure(int& w, int& h) const;
  void measure(int& w, int& h) const {_measure(w,h);}

  virtual void _draw(const Rectangle&) const = 0;
  void draw(const Rectangle& r) const {_draw(r);}
  virtual void draw_symbol_overlay(const Rectangle&) const;

  void set_inset(int x,int y,int w,int h) {inset_.set(x,y,w,h);}
  void set_inset(int x,int y) {inset_.set(x,y,-2*x,-2*y);}
  void set_inset(int x) {inset_.set(x,x,-2*x,-2*x);}
  void set_inset(const Rectangle& r) {inset_ = r;}
  const Rectangle& get_inset() const {return inset_;}

  int dx() const {return inset_.x();}
  int dy() const {return inset_.y();}
  int dw() const {return -inset_.w();} // inverted for back-compatability
  int dh() const {return -inset_.h();} // inverted for back-compatability

  // Hints for widgets:
  virtual void inset(Rectangle& r) const;
  virtual bool fills_rectangle() const;
  virtual bool is_frame() const;

  // hash table lookup:
  static const Symbol* find(const char* name);
  static const Symbol* find(const char* start, const char* end);
  static const Symbol* iterate(int& index);
  static void text(const char* s, unsigned n) {text_=s; text_length_=n;}
  static const char* text() {return text_;}
  static unsigned text_length() {return text_length_;}

  virtual ~Symbol();
};

// Back-compatability constructor:
FL_API void add_symbol(const char* name, void (*drawit)(Color), int scalable);

}

#endif
