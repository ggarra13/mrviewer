#ifndef Fl_Button_H
#define Fl_Button_H

#include "Fl_Widget.H"
#include <fltk/Button.h>
typedef fltk::Button Fl_Button;

enum {
  FL_NORMAL_BUTTON	= 0,
  FL_TOGGLE_BUTTON	= fltk::Button::TOGGLE,
  FL_RADIO_BUTTON	= fltk::Button::RADIO,
  FL_HIDDEN_BUTTON	= fltk::Button::HIDDEN // may not work in 2.0
};

#endif

