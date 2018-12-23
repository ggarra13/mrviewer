#ifndef Fl_Browser_H
#define Fl_Browser_H

#include "Fl_Scrollbar.H"
#include "Fl_Menu_.H"
#include <fltk/Browser.h>
class Fl_Browser : public fltk::Browser {
public:
  Fl_Browser(int x, int y, int w, int h, const char* l = 0)
    :fltk::Browser(x,y,w,h,l) {}

  void make_visible(int n) {goto_index(n); make_item_visible();}
  bool load(const char* fname) {/* NYI */ return true;}
  void position(int y) {goto_position(y);}
};

enum {
  FL_NORMAL_BROWSER	= fltk::Browser::NORMAL,
  FL_SELECT_BROWSER	= fltk::Browser::NORMAL,
  FL_HOLD_BROWSER	= fltk::Browser::NORMAL,
  FL_MULTI_BROWSER	= fltk::Browser::MULTI
};

#endif
