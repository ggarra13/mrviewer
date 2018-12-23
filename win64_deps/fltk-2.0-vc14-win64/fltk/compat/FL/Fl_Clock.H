#ifndef Fl_Clock_H
#define Fl_Clock_H

#include "Fl_Widget.H"
#include <fltk/Clock.h>

typedef fltk::ClockOutput Fl_Clock_Output;
typedef fltk::Clock Fl_Clock;

enum {
  FL_SQUARE_CLOCK	= fltk::Clock::SQUARE,
  FL_ROUND_CLOCK	= fltk::Clock::ROUND,
  FL_ANALOG_CLOCK	= fltk::Clock::SQUARE,
  FL_DIGITAL_CLOCK	= fltk::Clock::DIGITAL // nyi
};

#endif

