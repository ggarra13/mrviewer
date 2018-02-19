#ifndef Fl_Value_Slider_H
#define Fl_Value_Slider_H

#include "Fl_Slider.H"
#include <fltk/ValueSlider.h>

class Fl_Value_Slider : public fltk::ValueSlider {
public:
  Fl_Value_Slider(int x, int y, int w, int h, const char*l = 0)
    : fltk::ValueSlider(x,y,w,h,l) {set_vertical();}
};

#endif

