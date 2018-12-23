#ifndef Fl_Slider_H
#define Fl_Slider_H

#include "Fl_Widget.H"
#define FLTK_1_SLIDER 1
#include <fltk/Slider.h>

class Fl_Slider : public fltk::Slider {
public:
  Fl_Slider(int x, int y, int w, int h, const char*l = 0)
    : fltk::Slider(x,y,w,h,l) {set_vertical();}
};

enum {
  FL_VERT_SLIDER	= fltk::Slider::LINEAR,
  FL_HOR_SLIDER		= fltk::Slider::HORIZONTAL,
  FL_VERT_FILL_SLIDER	= fltk::Slider::FILL,
  FL_HOR_FILL_SLIDER	= fltk::Slider::HORIZONTAL+fltk::Slider::FILL,
  FL_VERT_NICE_SLIDER	= fltk::Slider::LINEAR,
  FL_HOR_NICE_SLIDER	= fltk::Slider::HORIZONTAL
};

#endif

