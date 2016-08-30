#ifndef Fl_Scrollbar_H
#define Fl_Scrollbar_H

#include "Fl_Slider.H"
#include <fltk/Scrollbar.h>

class Fl_Scrollbar : public fltk::Scrollbar {
public:
  Fl_Scrollbar(int x, int y, int w, int h, const char*l = 0)
    : fltk::Scrollbar(x,y,w,h,l) {set_vertical();}
};

#endif
