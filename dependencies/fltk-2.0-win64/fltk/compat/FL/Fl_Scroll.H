#ifndef Fl_Scroll_H
#define Fl_Scroll_H

#include "Fl_Group.H"
#include <fltk/ScrollGroup.h>

class Fl_Scroll : public fltk::ScrollGroup {

public:
  Fl_Scroll(int x, int y, int w, int h, const char*l = 0)
    : fltk::ScrollGroup(x,y,w,h,l) {begin();}
};

#endif

