#ifndef Fl_Tabs_H
#define Fl_Tabs_H

#include "Fl_Group.H"
#include <fltk/TabGroup.h>

class Fl_Tabs : public fltk::TabGroup {

public:
  Fl_Tabs(int x, int y, int w, int h, const char*l = 0)
    : fltk::TabGroup(x,y,w,h,l) {begin();}
};

#endif

