#ifndef Fl_Group_H
#define Fl_Group_H

#include "Fl_Widget.H"
#include <fltk/Group.h>

class Fl_Group : public fltk::Group {

public:
  Fl_Group(int x, int y, int w, int h, const char*l = 0)
    : fltk::Group(x,y,w,h,l) {begin();}
  static Fl_Group* current() {return (Fl_Group*)(fltk::Group::current());}
};

#endif

