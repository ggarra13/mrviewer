#ifndef Fl_Roller_H
#define Fl_Roller_H

#include "Fl_Valuator.H"
#include <fltk/ThumbWheel.h>

class Fl_Roller : public fltk::ThumbWheel {
public:
  Fl_Roller(int x, int y, int w, int h, const char*l = 0)
    : fltk::ThumbWheel(x,y,w,h,l) {set_vertical();}
};

#endif
