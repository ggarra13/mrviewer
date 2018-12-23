/* Fl_NSlider for WidgetSet, Copyright (c) 1998 curtis Edwards (curt1@jps.net)
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both the
 * copyright notice and this permission notice appear in supporting
 * documentation.  All work developed as a consequence of the use of
 * this program should duly acknowledge such use. No representations are
 * made about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 */
#include "Fl_NSlider.H"

static uchar isHoz = 0;	        //for image draw setup
static uchar r1, g1, b1;        //for color ramp
static uchar r2, g2, b2;

///////////////////Setup Defaults for slider
void  Fl_NSlider::setDefaults()
{
  slider_size((float)0.06);
  slider(FL_UP_BOX);    
  alignTick(FL_ALIGN_TOP);
  majorTickFont(FL_HELVETICA);
  majorTickFontSize(9);
  majorTickPrecision(0);
  minorTick(0);
  majorTick(0);
  colorTick( FL_BLACK);
  selection_color(9);
  hilight_color(FL_RED);
  color(FL_GRAY);
  ramp(0,0);		  //set to no ramp (0,0)
  _hilight = 0;           //hilighterd is false
  _r1=0, _g1=0; _b1=0;    //for color ramp
  _r2=0; _g2=0; _b2=0;
}
/////////////////////////////////The New Slider
Fl_NSlider::Fl_NSlider(uchar t, int x, int y, int w, int h, const char* l)
: Fl_Valuator(x, y, w, h, l) {
  type(t);
   switch(type())
   {
        case FL_VERT_NICE_SLIDER:
        case FL_HOR_NICE_SLIDER	:
             box(FL_FLAT_BOX);
             break;
       default: box(FL_DOWN_BOX);
             break;
   }
   setDefaults();
}
 //////////////////////////////////The New Slider
Fl_NSlider::Fl_NSlider(int x,int y,int w,int h, const char *l)
: Fl_Valuator(x, y, w, h, l)
{
   box(FL_DOWN_BOX);
   setDefaults();
}

/////////////////////////////////slider draw
void Fl_NSlider::draw() {
  int    knob_x, knob_y, knob_w, knob_h, col;
  tk_range  = (float) (maximum()-minimum());
  if(slider_size_ == 0) slider_size(0.06);
  int pxSlider;

  isHoz = horizontal();
  if( isHoz ) { 
     pxSlider = (int)(w()*slider_size_);
     px_x     = x() + box()->dx() + (pxSlider/2);  //starting point
     px_y     = y() + box()->dy();                 //y of widget
     px_w     = w() - pxSlider - box()->dw();      //w of widget
     px_h     = h() - box()->dh();                 //h of widget
     if(tk_range == 0) px_steps=0;
     else  px_steps = px_w / tk_range;                   //pixel steps(divider)
  }
  else {   //vertical      
     pxSlider = (int)(h()*slider_size_);
     px_x     = x() + box()->dx();                 
     px_y     = y() + box()->dy() + (pxSlider/2); 
     px_w     = w() - box()->dw();                            
     px_h     = h() - pxSlider -box()->dh();          
     if(tk_range == 0) px_steps=0;
     else  px_steps = px_h / tk_range;                   //pixel steps(divider)
  }
  calcButton(knob_x, knob_y, knob_w , knob_h, value());  //calc out the slider size&type
  draw_box();
  drawRamp();
  drawTicks();
  if(_hilight) col = hilight_color();
  else col = selection_color();
  switch(type()){
      case FL_VERT_NICE_SLIDER:
           draw_box();
           draw_box();
           draw_box();
           break;
      case FL_HOR_NICE_SLIDER:
           draw_box();
           draw_box();
           draw_box();
           break;
      default:
           draw_box();
	   break;
  }

draw_label();
 }
///////////////////////////////work out size of the slider
void Fl_NSlider::calcButton(int &knob_x, int &knob_y, int &knob_w , int &knob_h, double val) {
 float  tk_button_incr;
 int pxSlider;
 if( horizontal()) {
     tk_button_incr = (px_x + (px_steps * (val-minimum()) ));
     pxSlider = (int)(w()*slider_size_);
 }
 else  {
     tk_button_incr = (px_y + (px_steps * (val-minimum()) ));
     pxSlider = (int)(h()*slider_size_);
 }
 switch(type()) {
        case FL_VERT_FILL_SLIDER:
             knob_x = (int)px_x;
	     knob_y = y()+box()->dy();
	     knob_w = (int)px_w;
             knob_h = (int)(tk_button_incr-y());
	      break;
        case FL_HOR_FILL_SLIDER	:
             knob_x = x()+box()->dx();
	     knob_y = (int)px_y;
	     knob_w = (int)(tk_button_incr-x());
             knob_h = (int)px_h;
	     break;
        case FL_VERT_HALF_SLIDER:
             knob_x = (int)px_x;
	     knob_y = (int)(tk_button_incr-(pxSlider/2));
	     knob_w = (int)(px_w/2);
             knob_h = pxSlider;
 	     break;
        case FL_HOR_HALF_SLIDER	:
             knob_x = (int)(tk_button_incr-(pxSlider/2));
	     knob_y = (int)px_y;
	     knob_w = pxSlider;
             knob_h = (int)(px_h/2);
             break;
        case FL_HOR_NICE_SLIDER	:
        case FL_HOR_SLIDER:
             knob_x = (int)(tk_button_incr-(pxSlider/2));
	     knob_y = (int)px_y;
	     knob_w = pxSlider;
             knob_h = (int)px_h;
	     break;
        case FL_VERT_NICE_SLIDER	:
        case FL_VERT_SLIDER:
             knob_x = (int)px_x;
             knob_y = (int)(tk_button_incr-(pxSlider/2));
	     knob_w = (int)px_w;
             knob_h = pxSlider;
	     break;
     }
}
//////////////////redraw area damaged if NO_BOX ask parent to redraw (A hard function to find,value_damage)
void Fl_NSlider::value_damage(){
  int knob_x, knob_y, knob_w, knob_h;
  if(value() != oldval) 
  {
       calcButton( knob_x, knob_y, knob_w, knob_h,  oldval);
       if(box() == FL_NO_BOX)  parent()->damage(~1,  knob_x, knob_y, knob_w+3, knob_h+3);
       else damage(~1,  knob_x, knob_y, knob_w+3, knob_h+3);
  }
  calcButton( knob_x, knob_y, knob_w, knob_h, value());
  damage(~1,  knob_x, knob_y, knob_w, knob_h);
}

//////////////////////fltk handle stuff (notice FL_DRAG has no redraw,handle_drag() does it)
int  Fl_NSlider::handle(int event)
{
  int newpos = horizontal() ? Fl::event_x() : Fl::event_y();
  float val=0;
  switch (event) {
  case FL_ENTER: 
	_hilight = 1;
        value_damage();
        break;		
  case FL_LEAVE: 
	_hilight = 0;
        value_damage();
	break;		
  case FL_PUSH:		
	handle_push();
  case FL_DRAG:
        oldval=value();
	if( horizontal() ) {
	    val = ((newpos-x()-box()->dw() - ( (w()*slider_size_)/2))/ px_steps)+minimum();
	}
	else{  //vertical
	    val = ((newpos-y()-box()->dh() - ( (h()*slider_size_)/2) )/ px_steps)+minimum();
        }
        val = clamp(val);
        val = round(val);
	handle_drag(val);
        return 1;
   case FL_RELEASE:
		handle_release();
		return 1;
   default:
		return 0;
  }
return 1;
}
////////////a lot of shit for drawing ticks ..to do: merge Hoz/Vert Code tighter 
void   Fl_NSlider::drawTicks()
{
 char  pnt[12];
 int   fw=0, fh=0;
 float mdev, mcount, fnum=0, fnumIncr=0;
 uchar sr1, sg1, sb1; 
 int    majorLnLen    ;
 int    majorLnStart  ;
 int    minorLnLen    ;
 int    minorLnStart  ;
 int    majorFntStart ;
 
 fl_get_color( selection_color(), sr1, sg1, sb1);  //get color for ticks

 fl_font( (Fl_Font)_tick_font,_tick_fontSize);
 
 fl_color(_tick_col);
 sprintf(pnt,  _majorTickPrecision, maximum());
 fl_measure(pnt, fw, fh);                           //find font size
 if(tk_range == 0) return;
 if( horizontal() )             //insure the slider is horz
 { 
       majorLnLen    = (int) px_h/2;
       majorLnStart  = (int) px_y;
       minorLnLen    = (int) px_h/4;
       minorLnStart  = (int) px_y;
       majorFntStart = (int) majorLnStart+majorLnLen+fh;
       switch(_alignTick) {
           case FL_ALIGN_CENTER:
                minorLnStart += majorLnLen -(minorLnLen/2);
                majorLnStart += (int) px_h/4;
                majorFntStart = majorLnStart + majorLnLen+fh;                     
                break;
	   case FL_ALIGN_LEFT:
	   case FL_ALIGN_BOTTOM:
                minorLnStart += (int) px_h - minorLnLen;
                majorLnStart += (int) px_h - majorLnLen;
                majorFntStart = (int)(px_y+majorLnLen-(fh/4));
	        break;
       }
       if( majorTick() ) {   
          mdev   = fabs( px_w/ ( tk_range/_majorTick));    //pixles per major tick span
          if(mdev < (fw+(fw/3))) mdev   = (fw+(fw/3));
          fnumIncr = tk_range/(px_w/mdev);
          mcount   = 0; 
          fnum     = 0; 
          fl_line(  px_x+px_w , majorLnStart,  px_x+px_w , majorLnStart+majorLnLen );
          while(mcount < px_w) {
               fl_line(  px_x+mcount  , majorLnStart, px_x+mcount  , majorLnStart+majorLnLen );
               sprintf(pnt, _majorTickPrecision, fnum+minimum() );
               fl_measure(pnt, fw, fh); 
               if(_tick_fontSize) fl_draw(pnt, px_x+mcount-(fw/3), majorFntStart); 
               fnum += fnumIncr;
               mcount += mdev;
         }
     } //(if major)
     if( minorTick()  && _minorTick <= maximum()) {  //do minor ticks
         mdev  = fabs( px_w/ ( tk_range/_minorTick));
         if(mdev < 4) mdev = 4;
         mcount = mdev;
         while(mcount < px_w)
         {   fl_line(  px_x+mcount , minorLnStart,  px_x+mcount , minorLnStart+minorLnLen );  //create main line 
	     mcount += mdev;
         }
     } //(if minor)
 } //(isHoz)
 else  //insure the slider is Vert
 {
       majorLnLen    = (int) px_w/2;
       majorLnStart  = (int) px_x;
       minorLnLen    = (int) px_w/4;
       minorLnStart  = (int) px_x;
       majorFntStart = (int) majorLnStart+majorLnLen+(fh/4);
       switch(_alignTick) {
           case FL_ALIGN_CENTER:
                minorLnStart += majorLnLen -(minorLnLen/2);
                majorLnStart += (int) px_w/4;
                majorFntStart = majorLnStart + majorLnLen+(fh/4);                     
                break;
	   case FL_ALIGN_BOTTOM:
	   case FL_ALIGN_LEFT:
                minorLnStart += (int) px_w - minorLnLen;
                majorLnStart += (int) px_w - majorLnLen;
                majorFntStart = majorLnStart - fw-(fh/4);                     
	        break;
       }
       if( majorTick() ) {   
          mdev   = fabs( px_h/ ( tk_range/_majorTick));    //pixles per major tick span
          if(mdev < (fh+(fh/3))) mdev   = (fh+(fh/3));
          fnumIncr = tk_range/(px_h/mdev);
          mcount   = 0; 
          fnum     = 0; 
          fl_line(  majorLnStart, px_y+px_h , majorLnStart+majorLnLen,  px_y+px_h );
          while(mcount < px_h) {
               fl_line( majorLnStart, px_y+mcount  ,majorLnStart+majorLnLen, px_y+mcount );
               sprintf(pnt, _majorTickPrecision, fnum+minimum() );
               fl_measure(pnt, fw, fh); 
               if(_tick_fontSize) fl_draw(pnt, majorFntStart,px_y+mcount+(fh/3)); 
               fnum += fnumIncr;
               mcount += mdev;
         }
     } //(if major)
     if( minorTick()  && _minorTick <= maximum()) {  //do minor ticks
         mdev  = fabs( px_h/ ( tk_range/_minorTick));
         if(mdev < 4) mdev = 4;
         mcount = mdev;
         while(mcount < px_h)
         {   fl_line(  minorLnStart, px_y+mcount,  minorLnStart+minorLnLen, px_y+mcount );  //create main line 
	     mcount += mdev;
         }
     } //(if minor) 
 } //else vert
} //end drawTicks()
////////////////////////some tricky stuff to do ramp shit (generates ramp image)
static void generate_image(void* vv, int X, int Y, int W, uchar* buf) 
{
 Fl_NSlider* v = (Fl_NSlider*)vv;
 char r= r1+Y*(r2-r1)/( v->h()-(v->box()->dh()) );   //VERTICAL
 char g= g1+Y*(g2-g1)/( v->h()-(v->box()->dh()) );
 char b= b1+Y*(b2-b1)/( v->h()-(v->box()->dh()) );

 for (int x = X; x < X+W; x++) {
    if( isHoz ){   
        r= r1+x*(r2-r1)/( v->w()-(v->box()->dw()) );  //HORIZONTAL
        g= g1+x*(g2-g1)/( v->w()-(v->box()->dw()) );
        b= b1+x*(b2-b1)/( v->w()-(v->box()->dw()) );
    }
    *buf++ = r;
    *buf++ = g;
    *buf++ = b;
 }
}
///////////////////draw the ramp 
void   Fl_NSlider::drawRamp()
{
  if( (_r1+_r2+_g1+_g2+_b1+_b2) == 0) return;   //if black dont do it
  int x1 = x()+box()->dx();
  int y1 = y()+box()->dy();
  int w1 = w()-box()->dw();
  int h1 = h()-box()->dh();
  r1=_r1; g1=_g1; b1=_b1;	    //for color ramp
  r2=_r2; g2=_g2; b2=_b2;
  fl_draw_image(generate_image, this, x1, y1, w1, h1);
}
//////////////////set the slider size
void Fl_NSlider::slider_size(float v) {
  if (v <  0) v = 0;
  if (v > 1) v = 1;
  if (slider_size_ != float(v)) {
    slider_size_ = float(v); 
    damage(FL_DAMAGE_EXPOSE);
  }
}
////////////////set the bounds (from old slider)
void Fl_NSlider::bounds(float a, float b) {
  if (minimum() != a || maximum() != b) {
    Fl_Valuator::range(a, b); 
    damage(FL_DAMAGE_EXPOSE);
  }
}
