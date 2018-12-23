#ifndef Fl_Output_H
#define Fl_Output_H

#include "Fl_Input.H"
#include <fltk/Output.h>
typedef fltk::Output Fl_Output;

enum {
  FL_NORMAL_OUTPUT	= fltk::Input::NORMAL,
  FL_MULTILINE_OUTPUT	= fltk::Input::MULTILINE,
  FL_MULTILINE_OUTPUT_WRAP = fltk::Input::WORDWRAP
};

#endif

