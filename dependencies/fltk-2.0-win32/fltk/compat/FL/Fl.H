#ifndef Fl_H
#  define Fl_H

#include "Enumerations.H"
#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/Color.h>
#include <fltk/Font.h>
#include <fltk/Box.h>
#include <fltk/error.h>
#include <fltk/visual.h>
#include <fltk/Monitor.h>
#include <fltk/Widget.h>

namespace Fl = fltk;

typedef fltk::TimeoutHandler Fl_Timeout_Handler;

namespace fltk {

  class Widget;

  // fltk2 does not keep track of selection owner:
  inline fltk::Widget* selection_owner() {return 0;}
  inline void selection_owner(fltk::Widget*) {}
  inline void selection(fltk::Widget &owner, const char* b, int len) {copy(b,len,false);}

  inline void visible_focus(int) {} // fltk2 always acts like this is off

  inline int x() {return Monitor::all().x();}
  inline int y() {return Monitor::all().y();}
  inline int w() {return Monitor::all().w();}
  inline int h() {return Monitor::all().h();}

  inline void set_color(Color c, uchar r, uchar g, uchar b) {
    set_color_index(c,color(r,g,b));
  }
  inline void set_color(Color c, Color d) {set_color_index(c,d);}
  inline Color get_color(Color c) {return get_color_index(c);}
  inline void get_color(Color c, uchar&r, uchar&g, uchar&b) {split_color(c,r,g,b);}
  //inline void free_color(Color, bool overlay=false);

  inline const char* get_font(Font* f) {return f->system_name();}
  inline const char* get_font_name(Font* f, int* attributes = 0) {
    if (attributes) return f->name(attributes);
    else return f->name();
  }
  inline int get_font_sizes(Font* f, int*& sizep) {return f->sizes(sizep);}

  inline int box_dx(const Box* b) {return b->dx();}
  inline int box_dy(const Box* b) {return b->dy();}
  inline int box_dw(const Box* b) {return b->dw();}
  inline int box_dh(const Box* b) {return b->dh();}

  inline int event_key(int k) {return event_key_state(k);}

  inline void add_handler(int (*h)(int)) {
    // rather unsafe typecast of function:
    add_event_handler((int(*)(int,Window*))(h));
  }

  inline void delete_widget(Widget* w) {delete w;}
  inline void do_widget_deletion() {}

}

#endif
