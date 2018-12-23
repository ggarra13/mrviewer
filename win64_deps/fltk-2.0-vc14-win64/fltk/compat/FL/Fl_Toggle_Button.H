#ifndef Fl_Toggle_Button_H
#define Fl_Toggle_Button_H

#include "Fl_Button.H"

class Fl_Toggle_Button : public Fl_Button {
public:
    Fl_Toggle_Button(int X,int Y,int W,int H,const char *l=0)
	: Fl_Button(X,Y,W,H,l) {type(FL_TOGGLE_BUTTON);}
};

#endif
