//class Fl_Slider
//Another in the group of Fltk Widgets By Curtis Edwards (c) 1998 
//A Rewrite for the sliders to provide:
//Section Ticks,Color Ramping, Tick Font Display, Boxtype Slider knob (Can Use Bitmaps/labels/etc.)
#ifndef FL_NSLIDER_H
#define FL_NSLIDER_H

#include <FL/Fl.H>
#include <FL/Fl_Valuator.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <stdio.h>
#include <stdlib.h>
#include "math.h"

#define FL_VERT_SLIDER		0
#define FL_HOR_SLIDER		1
#define FL_VERT_FILL_SLIDER	2
#define FL_HOR_FILL_SLIDER	3
#define FL_VERT_NICE_SLIDER	4
#define FL_HOR_NICE_SLIDER	5
#define FL_VERT_HALF_SLIDER	6
#define FL_HOR_HALF_SLIDER	7

class Fl_NSlider : public Fl_Valuator {
public:
  float         slider_size_;		//store slider 
  Fl_Boxtype    slider_;            //boxtype for slider
  int           _hilight;           //hilight slider knob (ENTER)
  int           _hilight_col;
  Fl_Color      _tick_col;
  Fl_Font       _tick_font;
  int           _tick_fontSize;
  uchar         _r1, _g1, _b1;		//rgb color ramp 1
  uchar         _r2, _g2, _b2;		//rgb colour ramp 2
  int           _xoffset,_yoffset;  //save offset for the slider button when pushed insidestart position for tick/knobs
  char          _majorTickPrecision[5];  //ie "%.0f" for tick Precision
  float         _minorTick;
  float         _majorTick;
  Fl_Align      _alignTick;
 double oldval;
 float tk_range;	  //total pixel steps
 float px_steps;	  //pixel steps for tick and slider moves
 float px_x;          //pixel start position for tick/knobs
 float px_y;          //pixel start position for tick/knobs
 float px_w;          //pixel width of widget for knobs&ticks
 float px_h;          //pixle height
  int           mtp;
  
  Fl_NSlider(int x, int y, int w, int h, const char* l=0);
  Fl_NSlider(uchar t, int x, int y, int w, int h, const char* l=0);
  int        handle(int);
  void       hilight_color(int col)   { _hilight_col = col;  }
  int        hilight_color()          { return _hilight_col; }
  void       bounds(float a, float b);
  Fl_Boxtype slider() const {return (Fl_Boxtype)slider_;}
  void       slider(Fl_Boxtype c) {slider_ = c;}
  void       calcButton(int &knob_x, int &knob_y, int &knob_w , int &knob_h,double val);
  void       setDefaults();
  float      slider_size() const {return slider_size_;}
  void       slider_size(float v);
  void       alignTick( Fl_Align a)    { _alignTick = a; }
  Fl_Align   alignTick()    { return _alignTick;}
  void       colorTick( Fl_Color a)    { _tick_col  = a; }
  void       majorTickFont( Fl_Font p)     { if(p<0) p=0; _tick_font = p;}
  void       majorTickFontSize(int p)  { if(p<0) p=0; _tick_fontSize = p;}
  void       majorTickPrecision(int p) { if(p<0 || p>6) p=0; mtp=p; sprintf(_majorTickPrecision,"%%.%df",p); printf("new mtp = %d\n", mtp); }
  int        majorTickPrecision()      { printf("get mtp %d\n", mtp); return mtp; }
  void       minorTick(float tick)     { _minorTick = tick;} //set minor tick dividers
  void       majorTick(float tick)     { _majorTick = tick;} //set major tick dividers
  float      minorTick() {return _minorTick; }              //return minor tick dividers
  float      majorTick() {return _majorTick; }              //return major tick dividers
  void       value_damage();
  void       ramp(uchar nr1, uchar ng1, uchar nb1, uchar nr2, uchar ng2, uchar nb2) {_r1=nr1; _r2=nr2; _g1=ng1; _g2=ng2; _b1=nb1; _b2=nb2;}
  void       ramp(int ramp1, int ramp2) {fl_get_color((Fl_Color)ramp1, _r1, _g1, _b1);  fl_get_color((Fl_Color)ramp2, _r2, _g2, _b2);}
  void       draw();
  void       drawTicks();
  void       drawRamp();
};

#endif
