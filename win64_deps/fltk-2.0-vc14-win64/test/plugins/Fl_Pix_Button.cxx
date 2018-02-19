/* Fl_Pix_Button for WidgetSet, Copyright (c) 1998 curtis Edwards (curt1@jps.net)
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both the
 * copyright notice and this permission notice appear in supporting
 * documentation.  All work developed as a consequence of the use of
 * this program should duly acknowledge such use. No representations are
 * made about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 Description	- subclass of Fl_Button that draws pixmaps with lables
                each pixmap will draw UP/DOWN/HILIGHT 
*/
#include <FL/Fl.H>
#include "Fl_Pix_Button.H"
#include <FL/fl_draw.H>
#include <math.h>

////constructor
Fl_Pix_Button::Fl_Pix_Button(int x,int y,int w,int h,const char *l,Fl_Pixmap* bUp)
: Fl_Button(x,y,w,h,l) 
{
 pix     = bUp;
 hilight  = 0;
 int maxW=0, maxH=0;
 if(pix)                       //check size of bitmaps and log biggest sizes
 {   fl_measure_pixmap(pix->data, pix->w, pix->h); 
    if(pix->w > maxW ) maxW = pix->w; 
    if(pix->h/3 > maxH)  maxH = pix->h/3;
 }
 Fl_Button::w(maxW);
 Fl_Button::h(maxH);

}

////draw the button
void Fl_Pix_Button::draw()
{
  if (type() == FL_HIDDEN_BUTTON) return;
  if (!pix) return;
  switch(value())
  {
      case 0: if(hilight) pix->draw(x(),y(), w(), pix->h/3, 0, 0);
              else pix->draw(x(),y(), w(), pix->h/3, 0, 0);
              break;
      case 1: pix->draw(x(),y(), w(), pix->h/3, 0, 0);
              break;
      default: pix->draw(x(),y(), w(), pix->h/3, 0, 0);
              break;
  }
  draw_label();
}

////handel the events
int Fl_Pix_Button::handle(int event)
{
    switch (event) 
    {
       case FL_ENTER:
            hilight=1;
            redraw();
            break;
       case FL_LEAVE:
	        hilight=0;
                redraw();
                break;
   }
 return Fl_Button::handle(event);
}
