// "$Id: Tooltip.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#ifndef fltk_Tooltip_h
#define fltk_Tooltip_h

#include "MenuWindow.h"

namespace fltk {

class FL_API Tooltip : public MenuWindow {
public:
  Tooltip();
  void draw();
  void layout();

  static float delay() { return delay_; }
  static void delay(float f) { delay_ = f; }
  static bool enabled() { return enabled_; }
  static void enable(bool b = true) { enabled_ = b; }
  static void disable() { enabled_ = false; }

  typedef const char* (*Generator)(Widget*, void*);
  static void enter(Widget*, const Rectangle&, Generator, void* = 0);
  static void enter(Widget*, const Rectangle&, const char* text);
  static void enter(Widget*);
  static void current(Widget*);
  static void exit();

  static Widget* current_widget() { return current_widget_; }
  static const Rectangle& current_rectangle() { return current_rectangle_; }
  static Generator current_generator() { return current_generator_; }
  static void* current_data()	{ return current_data_; }
  static Tooltip* instance()	{ return instance_; }

  static NamedStyle* default_style;
#ifdef FLTK_1_WIDGET  // back-compatability section:
  static Widget* current()	{ return current_widget_; }
  static Font* font()		{ return default_style->textfont(); }
  static void font(Font* i)	{ default_style->textfont(i); }
  static float size()		{ return default_style->labelsize(); }
  static void size(float s)	{ default_style->labelsize(s); }
  static void textcolor(Color c){ default_style->labelcolor(c); }
  static Color textcolor()	{ return default_style->labelcolor(); }
  static void color(Color c)	{ default_style->color(c); }
  static Color color()		{ return default_style->color(); }
  static void box(Box* b)	{ default_style->box(b); }
  static Box* box()		{ return default_style->box(); }
#endif

private:
  static float delay_;
  static bool enabled_;
  static Widget* current_widget_;
  static Rectangle current_rectangle_;
  static Generator current_generator_;
  static void* current_data_;
  static Tooltip* instance_;
  static void tooltip_timeout(void*);
};

}

#endif

//
// End of "$Id: Tooltip.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
