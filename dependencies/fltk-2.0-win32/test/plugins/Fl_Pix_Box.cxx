/* Fl_Pix_Box for WidgetSet, Copyright (c) 1998 curtis Edwards (curt1@jps.net)
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
#include <FL/Fl_Pix_Box.H>
#include <FL/fl_draw.H>

////constructor
Fl_Pix_Box::Fl_Pix_Box(int x,int y,int w,int h,const char *l,Fl_Pixmap* bUp)
: Fl_Box(x,y,w,h,l) 
{
 bUp_     = bUp;
 if(bUp)  fl_measure_pixmap(bUp->data, bUp->w, bUp->h); 
}

////draw the button
void Fl_Pix_Box::draw()
{
  int remW=0,  nW=1;
  int remH=0,  nH=1;
  int X, Y, W=bUp_->w, H=bUp_->h;
  Y= y() + Fl::box_dy(box());
  draw_box();

  if(!bUp_) return;
  if(w() > W)
  {  nW     = w() / W;
     remW   = w() % W;
  }
  if(h() > H)
  {  nH   = h() / H;
     remH = h() % H;
  }

 for(int j=0; j<=nH; j++)
 {
    X=x()+ Fl::box_dx(box());
    if(j==nH) H = remH-Fl::box_dh(box());
    for(int i=0; i<nW; i++)
    {  bUp_->draw(X,Y, w(), H); 
       X=X+W;
    }
    if(remW) bUp_->draw(X,Y, remW-Fl::box_dw(box()), H);  
    Y=Y+H;
 }
  draw_label();

}
