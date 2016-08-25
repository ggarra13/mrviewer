//
// "$Id: color_chooser.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Color chooser for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/ColorChooser.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/Item.h>
#include <fltk/math.h>
#include <fltk/PopupMenu.h>
#include <fltk/ValueInput.h>
#include <fltk/layout.h>
#include <stdio.h>

using namespace fltk;

// Besides being a useful object on it's own, the ColorChooser was
// an attempt to make a complex composite object that could be easily
// imbedded into a user interface.  If you wish to make complex objects
// of your own, be sure to read this code.

// The function color_chooser() creates a window containing a color
// chooser and a few buttons and current-color indicators.  It is an
// easier interface for simple programs that just need a color.

// The "hue box" can be a circle or rectilinear.
// You get a circle by defining this:
// #define CIRCLE 1
// And the "hue box" can auto-update when the value changes
// you get this by defining this:
#define UPDATE_HUE_BOX 1

void ColorChooser::hsv2rgb(
	float H, float S, float V, float& r, float& g, float& b) {
  if (S < 5.0e-6) {
    r = g = b = V;
  } else {
    int i = (int)H;  
    float f = H - (float)i;
    float p1 = V*(1.0f-S);
    float p2 = V*(1.0f-S*f);
    float p3 = V*(1.0f-S*(1.0f-f));
    switch (i) {
    case 0: r = V;   g = p3;  b = p1;  break;
    case 1: r = p2;  g = V;   b = p1;  break;
    case 2: r = p1;  g = V;   b = p3;  break;
    case 3: r = p1;  g = p2;  b = V;   break;
    case 4: r = p3;  g = p1;  b = V;   break;
    case 5: r = V;   g = p1;  b = p2;  break;
    }
  }
}

void ColorChooser::rgb2hsv(
	float r, float g, float b, float& H, float& S, float& V) {
  float maxv = r > g ? r : g; if (b > maxv) maxv = b;
  V = maxv;
  if (maxv>0) {
    float minv = r < g ? r : g; if (b < minv) minv = b;
    S = 1.0f - float(minv)/maxv;
    if (maxv > minv) {
      if (maxv == r) {H = (g-b)/float(maxv-minv); if (H<0) H += 6;}
      else if (maxv == g) H = 2+(b-r)/float(maxv-minv);
      else H = 4+(r-g)/float(maxv-minv);
    }
  }
}

bool ColorChooser::rgb(float r, float g, float b) {
  if (r == r_ && g == g_ && b == b_ && !no_value_) return false;
  r_ = r; g_ = g; b_ = b; no_value_ = false;
  float ph = hue_;
  float ps = saturation_;
  float pv = value_;
  rgb2hsv(r,g,b,hue_,saturation_,value_);
  if (value_ != pv) {
#ifdef UPDATE_HUE_BOX
    huebox.redraw(DAMAGE_CONTENTS);
#endif
    valuebox.redraw(DAMAGE_VALUE);
    alphabox.redraw(DAMAGE_CONTENTS);
  }
  if (hue_ != ph || saturation_ != ps) {
    huebox.redraw(DAMAGE_VALUE); 
    valuebox.redraw(DAMAGE_CONTENTS);
    alphabox.redraw(DAMAGE_CONTENTS);
  }
  return true;
}

bool ColorChooser::hsv(float h, float s, float v) {
  h = fmodf(h,6.0f); if (h < 0) h += 6;
  if (s < 0) s = 0; else if (s > 1) s = 1;
  if (v < 0) v = 0; else if (v > 1) v = 1;
  if (h == hue_ && s == saturation_ && v == value_ && !no_value_) return false;
  float ph = hue_;
  float ps = saturation_;
  float pv = value_;
  hue_ = h; saturation_ = s; value_ = v;
  if (value_ != pv) {
#ifdef UPDATE_HUE_BOX
    huebox.redraw(DAMAGE_CONTENTS);
#endif
    valuebox.redraw(DAMAGE_VALUE);
    alphabox.redraw(DAMAGE_CONTENTS);
  }
  if (hue_ != ph || saturation_ != ps) {
    huebox.redraw(DAMAGE_VALUE); 
    valuebox.redraw(DAMAGE_CONTENTS);
    alphabox.redraw(DAMAGE_CONTENTS);
  }
  hsv2rgb(h,s,v,r_,g_,b_);
  no_value_ = false;
  return 1;
}

bool ColorChooser::a(float a) {
  alphabox.show();
  if (a == a_ && !no_value_) return false;
  a_ = a; no_value_ = false;
  alphabox.redraw(DAMAGE_VALUE);
  return true;
}

void ColorChooser::hide_a() {
  alphabox.hide();
}

bool ColorChooser::no_value(bool v) {
  support_no_value = true;
  if (v == no_value_) return false;
  if (v) {
    // Set the color pickers to the gray window color. It appears to be
    // more user-friendly to always go to a certain color.
    uchar r,g,b;
    split_color(color(),r,g,b);
    rgb(r/255.0f,g/255.0f,b/255.0f);
  }
  no_value_ = v;
  return true;
}

////////////////////////////////////////////////////////////////

static void tohs(float x, float y, float& h, float& s) {
#ifdef CIRCLE
  x = 2*x-1;
  y = 1-2*y;
  s = sqrtf(x*x+y*y); if (s > 1) s = 1;
  h = float(3.0/M_PI)*atan2f(y,x);
  if (h<0) h += 6;
#else
  h = fmodf(6*x,6.0f); if (h < 0) h += 6;
  s = 1-y; if (s < 0) s = 0; else if (s > 1) s = 1;
#endif
}

int ccHueBox::handle(int e) {
  static float is;
  static float ih;
  ColorChooser* c = (ColorChooser*)parent();
  switch (e) {
  case PUSH:
    is = c->s();
    ih = c->h();
  case DRAG: {
    float Xf, Yf, H, S;
    Rectangle r(w(),h()); box()->inset(r);
    Xf = (event_x()-r.x())/float(r.w());
    Yf = (event_y()-r.y())/float(r.h());
    tohs(Xf, Yf, H, S);
    if (fabsf(H-ih) < 3*6.0f/w()) H = (float)ih;
    if (fabsf(S-is) < 3*1.0f/h()) S = (float)is;
    if (event_state(CTRL)) H = c->h();
    if (c->hsv(H, S, c->v())) c->do_callback();
    } return 1;
  default:
    return Widget::handle(e);
  }
}

static const uchar* generate_image(void* vv, int X, int Y, int W, uchar* p) {
  ccHueBox* v = (ccHueBox*)vv;
  Rectangle r(v->w(),v->h()); v->box()->inset(r);
  float Yf = float(Y)/r.h();
#ifdef UPDATE_HUE_BOX
  const float V = ((ColorChooser*)(v->parent()))->v();
#else
  const float V = 1.0f;
#endif
  uchar* buf = p;
  for (int x = X; x < X+W; x++) {
    float Xf = float(x)/r.w();
    float H,S; tohs(Xf,Yf,H,S);
    float r,g,b;
    ColorChooser::hsv2rgb(H,S,V,r,g,b);
    *buf++ = uchar(255*r+.5f);
    *buf++ = uchar(255*g+.5f);
    *buf++ = uchar(255*b+.5f);
  }
  return p;
}

void ccHueBox::draw() {
  if (damage()&DAMAGE_ALL) draw_frame();
  Rectangle r(w(),h()); box()->inset(r);
  if (damage() == DAMAGE_VALUE) {
    push_clip(r.x()+px,r.y()+py,6,6);
  }
  drawimage(generate_image, this, RGB, r);
  if (damage() == DAMAGE_VALUE) pop_clip();
  ColorChooser* c = (ColorChooser*)parent();
#ifdef CIRCLE
  int X = int(.5f*(cosf(c->h()*float(M_PI/3.0))*c->s()+1) * (r.w()-6));
  int Y = int(.5f*(1-sinf(c->h()*float(M_PI/3.0))*c->s()) * (r.h()-6));
#else
  int X = int(c->h()/6.0f*(r.w()-6));
  int Y = int((1-c->s())*(r.h()-6));
#endif
  if (X < 0) X = 0; else if (X > r.w()-6) X = r.w()-6;
  if (Y < 0) Y = 0; else if (Y > r.h()-6) Y = r.h()-6;
  //  color(c->v()>.75 ? BLACK : WHITE);
  drawstyle(style(),OUTPUT);
  buttonbox()->draw(Rectangle(r.x()+X, r.y()+Y, 6, 6));
  px = X; py = Y;
}

////////////////////////////////////////////////////////////////

bool ccValueBox::is_alpha() const {
  ColorChooser* c = (ColorChooser*)parent();
  return this == &(c->alphabox);
}

int ccValueBox::handle(int e) {
  static float iv;
  ColorChooser* c = (ColorChooser*)parent();
  switch (e) {
  case PUSH:
    iv = is_alpha() ? c->a() : c->v();
  case DRAG: {
    float Yf;
    Rectangle r(w(),h()); box()->inset(r);
    Yf = 1-(event_y()-r.y())/float(r.h());
    if (fabsf(Yf-iv)<(3*1.0f/h())) Yf = iv;
    if (is_alpha()) {
      if (c->a(Yf)) c->do_callback();
    } else {
      if (c->hsv(c->h(),c->s(),Yf)) c->do_callback();
    }} return 1;
  default:
    return Widget::handle(e);
  }
}

struct Idata {
  float r, g, b;
  Rectangle wh;
  bool aimage;
};

static const uchar* generate_vimage(void* vv, int X, int Y, int W, uchar* p) {
  const Idata& i = *((Idata*)vv);
  float yy = 255*(float(Y-3)/(i.wh.h()-6));
  if (yy<0) yy = 0; else if (yy > 255) yy = 255;
  float Yf = 255-yy;
  uchar r = uchar(i.r*Yf+.5f);
  uchar g = uchar(i.g*Yf+.5f);
  uchar b = uchar(i.b*Yf+.5f);
  uchar* buf = p;
  if (i.aimage) {
    uchar r1 = uchar(r+yy);
    uchar g1 = uchar(g+yy);
    uchar b1 = uchar(b+yy);
    int xr = Y&8;
    for (int x = X; x < X+W; x++) {
      if ((x^xr)&8) {
	*buf++ = r1; *buf++ = g1; *buf++ = b1;
      } else {
	*buf++ = r; *buf++ = g; *buf++ = b;
      }
    }	
  } else {
    for (int x = X; x < X+W; x++) {
      *buf++ = r; *buf++ = g; *buf++ = b;
    }
  }
  return p;
}

void ccValueBox::draw() {
  if (damage()&DAMAGE_ALL) draw_frame();
  ColorChooser* c = (ColorChooser*)parent();
  Idata i;
  i.wh.set(0,0,w(),h()); box()->inset(i.wh);
  float v;
  if (is_alpha()) {
    i.r = c->r(); i.g = c->g(); i.b = c->b();
    v = c->a();
    i.aimage = true;
  } else {
    c->hsv2rgb(c->h(),c->s(),1.0f,i.r,i.g,i.b);
    v = c->v();
    i.aimage = false;
  }
  if (damage() == DAMAGE_VALUE) {
    push_clip(i.wh.x(),i.wh.y()+py,i.wh.w(),6);
  }
  drawimage(generate_vimage, &i, RGB, i.wh);
  if (damage() == DAMAGE_VALUE) pop_clip();
  int Y = int((1-v) * (i.wh.h()-6));
  if (Y < 0) Y = 0; else if (Y > i.wh.h()-6) Y = i.wh.h()-6;
  drawstyle(style(),OUTPUT);
  buttonbox()->draw(Rectangle(i.wh.x(), i.wh.y()+Y, i.wh.w(), 6));
  py = Y;
}

////////////////////////////////////////////////////////////////

#define ROWS 4
#define COLS 16

FL_API Color fl_color_cells[ROWS*COLS] = {
// first the 16 assignable fltk color indexes:
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
// then the closest gray ramps to 0,1/4,1/3,1/2,2/3,3/4,1:
32,32,32,37,37,39,39,43,43,47,47,49,49,55,55,55,
// repeat it twice:
32,32,32,37,37,39,39,43,43,47,47,49,49,55,55,55};

void ccCellBox::draw() {
  int yy = 0;
  for (int Y = 0; Y < ROWS; Y++) {
    int hh = (Y+1)*h()/ROWS - yy;
    int xx = 0;
    for (int X = 0; X < COLS; X++) {
      int ww = (X+1)*w()/COLS - xx;
      Rectangle r(xx,yy,ww,hh);
      drawstyle(style(),OUTPUT);
      if (X || Y || !((ColorChooser*)parent())->support_no_value) {
	setbgcolor(fl_color_cells[Y*COLS+X]);
	THIN_DOWN_BOX->draw(r);
      } else {
	THIN_DOWN_BOX->draw(r);
	r.inset(1);
	push_clip(r);
	drawtext("@7+",r,0);
	pop_clip();
      }
      xx += ww;
    }
    yy += hh;
  }
}

int ccCellBox::handle(int e) {
  switch (e) {
  case PUSH: return 1;
  case DRAG: return 1;
  case RELEASE: {
    int X = event_x()*COLS/w();
    if (X < 0 || X >= COLS) return 1;
    int Y = event_y()*ROWS/h();
    if (Y < 0 || Y >= ROWS) return 1;
    X = X+Y*COLS;
    ColorChooser* c = (ColorChooser*)parent();
    if (event_button() > 1) {
      if (X) {
	fl_color_cells[X] = c->value();
	redraw();
      }
    } else {
      Color color = fl_color_cells[X];
      if (!color && !c->support_no_value) color = BLACK;
      if (c->value(color)) c->do_callback();
    }
    return 1;}
  }
  return Widget::handle(e);
}

////////////////////////////////////////////////////////////////

void ColorChooser::layout() {
  if (layout_damage()&(LAYOUT_WH|LAYOUT_DAMAGE)) {
    int h = Widget::h();
    int ch = w()*ROWS/COLS;
    if (ch > h/3) ch = h/3;
    cellbox.resize(0,h-ch,w(),ch);
    h = h-ch;
    int ww = w()*20/140;
    if (ww>28) ww = 28;
    int xx = w()-ww;
    if (alphabox.visible()) {
      alphabox.resize(xx,0,ww,h);
      xx -= ww;
    }
    valuebox.resize(xx,0,ww,h);
    huebox.resize(0,0,xx,h);
  }
  Widget::layout();
}

ColorChooser::ColorChooser(int X, int Y, int W, int H, const char* L)
  : Group(0,0,140,180,L,true),
    huebox(0,0,100,100),
    valuebox(100,0,20,100),
    alphabox(120,0,20,100),
    cellbox(0,100,140,80)
{
  alphabox.hide();
  huebox.tooltip("Drag to change hue & saturation\n"
		 "Ctrl+drag to change saturation only");
  valuebox.tooltip("Drag to change value (brightness)");
  alphabox.tooltip("Drag to change alpha");
  cellbox.tooltip("Left-click to select color from cell.\n"
		  "Right-click to put current color into cell.\n"
		  "Click X to use default value");
  end();
//    resizable(huebox);
//    sizes();
  resize(X,Y,W,H);
  r_ = g_ = b_ = 0;
  a_ = 1.0;
  hue_ = 0.0;
  saturation_ = 0.0;
  value_ = 0.0;
  no_value_ = support_no_value = false;
}

Color ColorChooser::value() const {
  if (no_value_) return 0;
  Color ret =
    fltk::color(uchar(255*r()+.5f), uchar(255*g()+.5f), uchar(255*b()+.5f));
  return ret ? ret : BLACK;
}

bool ColorChooser::value(Color c) {
  if (!c) {
    return no_value(true);
  } else {
    uchar r,g,b; split_color(c,r,g,b);
    return rgb(r/255.0f, g/255.0f, b/255.0f);
  }
}

////////////////////////////////////////////////////////////////
// color_chooser():

#include <fltk/Window.h>
#include <fltk/ReturnButton.h>
#include <fltk/ask.h>

class FL_API ccValueInput : public ValueInput {
public:
  int format(char*);
  ccValueInput(int X, int Y, int W, int H) : ValueInput(X,Y,W,H) {}
};

enum {M_RGB, M_BYTE, M_HEX, M_HSV}; // modes
static int mode = M_RGB;
static bool recursion;

int ccValueInput::format(char* buf) {
  if (mode == M_HEX) return sprintf(buf,"0x%02X", int(value()));
  else return Valuator::format(buf);
}

static ccValueInput* rvalue, *gvalue, *bvalue, *avalue;

static void setstep(float step) {
  if (mode != M_HSV) {rvalue->step(step); rvalue->linesize(step);}
  gvalue->step(step); gvalue->linesize(step);
  bvalue->step(step); bvalue->linesize(step);
  avalue->step(step); avalue->linesize(step);
}

static void set_valuators(ColorChooser* c) {
  if (recursion) return;
  switch (mode) {
  case M_RGB:
    setstep(.001f);
    rvalue->range(0,1); rvalue->value(c->r());
    gvalue->range(0,1); gvalue->value(c->g());
    bvalue->range(0,1); bvalue->value(c->b());
    avalue->range(0,1); avalue->value(c->a());
    break;
  case M_BYTE:
  case M_HEX:
    setstep(1);
    rvalue->range(0,255); rvalue->value(int(255*c->r()+.5f));
    gvalue->range(0,255); gvalue->value(int(255*c->g()+.5f));
    bvalue->range(0,255); bvalue->value(int(255*c->b()+.5f));
    avalue->range(0,255); avalue->value(int(255*c->a()+.5f));
    break;
  case M_HSV:
    setstep(.001f);
    rvalue->range(0,360); rvalue->step(1); rvalue->linesize(1); rvalue->value(c->h()*60);
    gvalue->range(0,1); gvalue->value(c->s());
    bvalue->range(0,1); bvalue->value(c->v());
    avalue->range(0,1); avalue->value(c->a());
    break;
  }
}

static Window* window;
static ColorChooser* chooser;
static Widget* ok_color;
static Button* ok_button;
static Widget* cancel_color;
static Button* cancel_button;

static void input_cb(Widget* o, void*) {
  float r = float(rvalue->value());
  float g = float(gvalue->value());
  float b = float(bvalue->value());
  float a = float(avalue->value());
  recursion = true;
  if (mode == M_HSV) {
    if (chooser->hsv(r/60,g,b)) chooser->do_callback();
  } else {
    if (mode != M_RGB) {
      r = r/255;
      g = g/255;
      b = b/255;
    }
    if (chooser->rgb(r,g,b)) chooser->do_callback();
  }
  if (avalue->visible()) {
    if (chooser->a(a)) chooser->do_callback();
  }
  recursion = false;
}

static void mode_cb(Widget* o, long int v) {
  mode = int(v);
  // force them to redraw even if value is the same:
  rvalue->value(-2);
  gvalue->value(-2);
  bvalue->value(-2);
  avalue->value(-2);
  set_valuators(chooser);
}

static void chooser_cb(Widget*, void*) {
  ok_color->color(chooser->value());
  ok_color->label(chooser->no_value() ? "@7+" : 0);
  ok_color->redraw();
  set_valuators(chooser);
}

static void ok_cb(Widget* w, void*) {
  w->window()->make_exec_return(true);
}

static void cancel_cb(Widget* w, void*) {
  w->window()->make_exec_return(false);
}

static void make_it() {
  if (window) return;
  window = new Window(210, 160+5+26+21+21+5);
  window->begin();
  chooser = new ColorChooser(5, 5, 200, 155);
  chooser->callback(chooser_cb);
  int y = 165;
  int x = 5;
  rvalue = new ccValueInput(x,y,50,21); x += 50;
  rvalue->callback(input_cb);
  gvalue = new ccValueInput(x,y,50,21); x += 50;
  gvalue->callback(input_cb);
  bvalue = new ccValueInput(x,y,50,21); x += 50;
  bvalue->callback(input_cb);
  avalue = new ccValueInput(x,y,50,21); x += 50;
  avalue->callback(input_cb);
  PopupMenu* choice = new PopupMenu(5,y,200,21); x+=40;
  choice->type(PopupMenu::POPUP3);
  choice->begin();
  (new Item("rgb"))->callback(mode_cb,M_RGB);
  (new Item("byte"))->callback(mode_cb,M_BYTE);
  (new Item("hex"))->callback(mode_cb,M_HEX);
  (new Item("hsv"))->callback(mode_cb,M_HSV);
  choice->end();
  choice->value(0);
  choice->tooltip("Right-click to change type of data entered here");
  y += 26;
  ok_color = new Widget(5, y, 95, 21);
  ok_color->box(ENGRAVED_BOX);
  ok_button = new ReturnButton(5, y+21, 95, 21, ok);
  ok_button->callback(ok_cb);
  cancel_color = new Widget(110, y, 95, 21);
  cancel_color->box(ENGRAVED_BOX);
  cancel_button = new Button(110, y+21, 95, 21, cancel);
  cancel_button->callback(cancel_cb);
  // window->size_range(210, 240); // minimum usable size?
  window->resizable(chooser);
  window->end();
}

static int run_it(const char* name)
{
  window->label(name);
  set_valuators(chooser);
  ok_color->color(chooser->value());
  ok_color->label(chooser->no_value() ? "@7+" : 0);
  cancel_color->color(chooser->value());
  cancel_color->label(chooser->no_value() ? "@7+" : 0);
  window->hotspot(window);
  return window->exec();
}

/*!
  \image html fl_color_chooser.jpg

  fltk::color_chooser() pops up a window to let the user pick an
  arbitrary RGB color. They can pick the hue and saturation in the
  "hue box" on the left (hold down CTRL to just change the
  saturation), and the brighness using the vertical slider. Or they
  can type the 8-bit numbers into the RGB fltk::ValueInput fields, or
  drag the mouse across them to adjust them. The pull-down menu lets
  the user set the input fields to show RGB, HSV, or 8-bit RGB (0 to
  255).

  This returns non-zero if the user picks ok, and updates the RGB
  values. If the user picks cancel or closes the window this returns
  zero and leaves RGB unchanged.

  This version takes and returns numbers in the 0-1 range.

  There is also a class fltk::ColorChooser which you can use to imbed
  a color chooser into another control panel.
*/
bool fltk::color_chooser(const char* name, float& r, float& g, float& b) {
  make_it();
  chooser->rgb(r,g,b);
  chooser->hide_a(); avalue->hide();
  chooser->hide_no_value();
  if (!run_it(name)) return false;
  r = chooser->r();
  g = chooser->g();
  b = chooser->b();
  return true;
}

/*! Same but user can also select an alpha value. Currently the color
  chips do not remember or set the alpha! */
bool fltk::color_chooser(const char* name, float&r, float&g, float&b, float&a) {
  make_it();
  chooser->rgb(r,g,b);
  chooser->a(a); avalue->show();
  chooser->hide_no_value();
  if (!run_it(name)) return false;
  r = chooser->r();
  g = chooser->g();
  b = chooser->b();
  a = chooser->a();
  return true;
}

/*! Same but it takes and returns 8-bit numbers for the rgb arguments. */
bool fltk::color_chooser(const char* name, uchar& r, uchar& g, uchar& b) {
  make_it();
  chooser->rgb(r/255.0f, g/255.0f, b/255.0f);
  chooser->hide_a(); avalue->hide();
  chooser->hide_no_value();
  if (!run_it(name)) return false;
  r = uchar(255*chooser->r()+.5f);
  g = uchar(255*chooser->g()+.5f);
  b = uchar(255*chooser->b()+.5f);
  return true;
}

/*! Same but with 8-bit alpha chosen by the user. */
bool fltk::color_chooser(const char* name, uchar&r, uchar&g, uchar&b, uchar&a) {
  make_it();
  chooser->rgb(r/255.0f, g/255.0f, b/255.0f);
  chooser->a(a/255.0f); avalue->show();
  chooser->hide_no_value();
  if (!run_it(name)) return false;
  r = uchar(255*chooser->r()+.5f);
  g = uchar(255*chooser->g()+.5f);
  b = uchar(255*chooser->b()+.5f);
  a = uchar(255*chooser->a()+.5f);
  return true;
}

/*! Same but it takes and returns an fltk::Color number. No alpha. */
bool fltk::color_chooser(const char* name, Color& c) {
  make_it();
  uchar r,g,b;
  if (c) {
    split_color(c,r,g,b);
    chooser->rgb(r/255.0f, g/255.0f, b/255.0f);
    chooser->no_value(false);
  } else {
    chooser->no_value(true);
  }
  chooser->hide_a(); avalue->hide();
  if (!run_it(name)) return false;
  c = chooser->value();
  return true;
}

//
// End of "$Id: color_chooser.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
