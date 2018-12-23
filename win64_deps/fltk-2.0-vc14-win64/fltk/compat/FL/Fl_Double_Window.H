#ifndef Fl_Double_Window_H
#define Fl_Double_Window_H

#include "Fl_Window.H"

class Fl_Double_Window : public Fl_Window {

public:
  Fl_Double_Window(int x, int y, int w, int h, const char*l = 0)
    : Fl_Window(x,y,w,h,l) {set_double_buffer();}
    
  Fl_Double_Window(int x, int y, const char*l = 0)
    : Fl_Window(x,y,l) {set_double_buffer();}

};

#endif

