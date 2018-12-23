#ifndef Fl_Input_H
#define Fl_Input_H

#include "Fl_Widget.H"
#include <fltk/Input.h>
class Fl_Input : public fltk::Input {
public:
  Fl_Input(int x, int y, int w, int h, const char* l=0) : fltk::Input(x,y,w,h,l) {}
  int wrap() const { return type() == fltk::Input::WORDWRAP; }
  void wrap(int b) { type(b ? fltk::Input::WORDWRAP : fltk::Input::MULTILINE);}
};

enum {
  FL_NORMAL_INPUT	= fltk::Input::NORMAL,
  FL_FLOAT_INPUT	= 1, // probably needs the IntInput subclass!
  FL_INT_INPUT		= 2,
//FL_HIDDEN_INPUT	// not in fltk2.0
  FL_MULTILINE_INPUT	= fltk::Input::MULTILINE,
  FL_SECRET_INPUT	= fltk::Input::SECRET,
//FL_INPUT_TYPE		= 7,
//FL_INPUT_READONLY	= 8,
  FL_INPUT_WRAP		= fltk::Input::WORDWRAP,
  FL_MULTILINE_INPUT_WRAP = fltk::Input::WORDWRAP
};

#endif

