// Fl_Check_Button.H

// For back compatability, a Fl_Light_Button with a diamond down_box()
// and a red color.

#ifndef Fl_Pix_Box_H
#define Fl_Pix_Box_H

#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>

class Fl_Pix_Box : public Fl_Box 
{
public:
  Fl_Pixmap* bUp_;

  void draw();
  Fl_Pix_Box(int x,int y,int w,int h,const char *l = 0,Fl_Pixmap* bUp=0);
};


#endif
