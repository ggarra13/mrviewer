// Fl_Check_Button.H

// For back compatability, a Fl_Light_Button with a diamond down_box()
// and a red color.

#ifndef Fl_Pix_Button_H
#define Fl_Pix_Button_H

#include <FL/Fl_Button.H>
#include <FL/Fl_Pixmap.H>

class Fl_Pix_Button : public Fl_Button 
{
public:
  Fl_Pixmap* pix;
  int hilight;
  
  void draw();
  int handle(int);
  Fl_Pix_Button(int x,int y,int w,int h,const char *l = 0,Fl_Pixmap* bUp=0);
};


#endif
