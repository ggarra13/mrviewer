#ifndef Fl_Widget_H
#define Fl_Widget_H

#include "Enumerations.H"
#define FLTK_1_WIDGET 1
#include <fltk/Widget.h>

typedef fltk::Widget Fl_Widget;

typedef fltk::Callback Fl_Callback;
typedef Fl_Callback* Fl_Callback_p; // needed for BORLAND
typedef fltk::Callback0 Fl_Callback0;
typedef fltk::Callback1 Fl_Callback1;

enum {
  FL_RESERVED_TYPE 	= fltk::Widget::RESERVED_TYPE
};

#endif
