#ifndef Fl_Dial_H
#define Fl_Dial_H

#include "Fl_Widget.H"
#include <fltk/Dial.h>
typedef fltk::Dial Fl_Dial;

enum {
  FL_NORMAL_DIAL	= fltk::Dial::NORMAL,
  FL_LINE_DIAL		= fltk::Dial::LINE,
  FL_FILL_DIAL		= fltk::Dial::FILL
};

#endif

