//
// "$Id: ColorChooser.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Color chooser header file for the Fast Light Tool Kit (FLTK).
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

// The color chooser object and the color chooser popup.  The popup
// is just a window containing a single color chooser and some boxes
// to indicate the current and cancelled color.

#ifndef fltk_ColorChooser_h
#define fltk_ColorChooser_h

#include "Group.h"

namespace fltk {

class FL_API ccHueBox : public Widget {
  int px, py;
protected:
  void draw();
public:
  int handle(int);
  ccHueBox(int X, int Y, int W, int H) : Widget(X,Y,W,H) {
  px = py = 0;}
};

class FL_API ccValueBox : public Widget {
  int py;
  bool is_alpha() const;
protected:
  void draw();
public:
  int handle(int);
  ccValueBox(int X, int Y, int W, int H) : Widget(X,Y,W,H) {
  py = 0;}
};

class FL_API ccCellBox : public Widget {
public:
  ccCellBox(int X, int Y, int W, int H) : Widget(X,Y,W,H) {}
  void draw();
  int handle(int);
};

class FL_API ColorChooser : public Group {
  friend class ccHueBox;
  friend class ccValueBox;
  friend class ccCellBox;
  ccHueBox huebox;
  ccValueBox valuebox;
  ccValueBox alphabox;
  ccCellBox cellbox;
  float hue_, saturation_, value_;
  float r_, g_, b_, a_;
  bool no_value_;
  bool support_no_value;
public:
  float h() const {return hue_;}
  float s() const {return saturation_;}
  float v() const {return value_;}
  float r() const {return r_;}
  float g() const {return g_;}
  float b() const {return b_;}
  float a() const {return a_;}
  bool no_value() const {return no_value_;}
  bool no_value(bool);
  Color value() const;
  bool value(Color);
  bool hsv(float,float,float);
  bool rgb(float,float,float);
  bool a(float);
  void hide_a();
  void hide_no_value() {support_no_value = false;}
  static void hsv2rgb(float, float, float,float&,float&,float&);
  static void rgb2hsv(float, float, float,float&,float&,float&);
  ColorChooser(int,int,int,int,const char* = 0);
  void layout();
  float setcell(int,float,float,float,float);
  float getcell(int,float,float,float,float);
};

// Convience functions to pop-up a control panel:

FL_API bool color_chooser(const char* name, float& r, float& g, float& b);
FL_API bool color_chooser(const char* name, float& r, float& g, float& b, float& a);
FL_API bool color_chooser(const char* name, uchar& r, uchar& g, uchar& b);
FL_API bool color_chooser(const char* name, uchar& r, uchar& g, uchar& b, uchar& a);
FL_API bool color_chooser(const char* name, Color& c);

}

#endif

//
// End of "$Id: ColorChooser.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
