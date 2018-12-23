// Lame test of all types of image drawings.
#include <string.h>      // memcpy
#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Slider.h>
#include <fltk/draw.h>
#include <fltk/rgbImage.h>
#include <fltk/xpmImage.h>
#include <fltk/xbmImage.h>
#include <fltk/CheckButton.h>

#include <config.h>
#if USE_X11
#include "list_visuals.cxx"
#include <fltk/visual.h>
#endif

using namespace fltk;

const char cdata[16*16+1] =
"                "
"  R RR    GGG   "
"  RRrrR  GgggG  "
"  Rrrrrr GgggGg "
"  Rrrrrrr GGGGgg"
"  Rrrrrrrr ggGgg"
"  Rrrrrrrrr gGgg"
"   rBrrrrrGGGggg"
"    Bbrrrrrggggg"
"    BBBBrrrrgggg"
"    BbbbBrrrrggg"
"    BbbbBbrrrrgg"
"    BbbbBbbrrrrg"
"    BBBBbbbbrrrr"
"     bbbbbbbbrrr"
"      bbbbbbbbrr";

#define SCALE 4

// Tester for drawimage() call:
class Dtest : public Widget {
public:
  fltk::PixelType pixeltype;
  const unsigned char* data;
  void draw() {
    push_matrix();
    scale(float(w())/(16*SCALE),float(h())/(16*SCALE));
    drawimage(data, pixeltype, Rectangle(16*SCALE,16*SCALE));
    pop_matrix();
  }
  Dtest(PixelType pt, int x, int y, const char* l, const unsigned char* d) :
    Widget(x,y,16*SCALE,16*SCALE,l), pixeltype(pt), data(d) {
    box(NO_BOX);
    align(ALIGN_BOTTOM);
    labelsize(10);
    tooltip("fltk::drawimage()");
  }
};

// Tester for rgbImage subclass:
class Itest : public Widget {
public:
  Itest(PixelType pt, int x, int y, const char* l, const unsigned char* d) :
    Widget(x,y,16*SCALE,16*SCALE,l) {
    box(new rgbImage(d, pt, 16*SCALE, 16*SCALE));
    align(ALIGN_BOTTOM);
    labelsize(10);
    tooltip("Instance of fltk::rgbImage used as widget->box()");
  }
};

// Tester for inset:
class Insetimage : public rgbImage {
public:
  Insetimage(const unsigned char* d, PixelType pt, int w, int h) :
    rgbImage(d,pt,w,h) {}
  void inset(fltk::Rectangle& r) const {r.inset(16);}
};

class Insettest : public Widget {
public:
  Insettest(PixelType pt,int x, int y, const char* l, const unsigned char* d) :
    Widget(x,y,16*SCALE,16*SCALE,l) {
    box(new Insetimage(d, pt, 16*SCALE, 16*SCALE));
    align(ALIGN_BOTTOM);
    labelsize(10);
    tooltip("Edges are not scaled in this image.");
  }
};

// Bitmap tester:
#include "sorceress.xbm"
class Btest : public Widget {
public:
  Btest(int x, int y, const char* l)
    : Widget(x,y,16*SCALE,16*SCALE,l) {
    box(new xbmImage(sorceress_bits, sorceress_width, sorceress_height));
    align(ALIGN_BOTTOM|ALIGN_CLIP);
    labelsize(10);
  }
};

// Monochrome xpm tester:
#include "../pixmaps/recycle.xpm"
#include "porsche.xpm"
class Xtest : public Widget {
public:
  Xtest(int x, int y, const char* l, const char* const* data)
    : Widget(x,y,16*SCALE,16*SCALE,l) {
    box(new xpmImage(data,0));
    align(ALIGN_BOTTOM|ALIGN_CLIP);
    labelsize(10);
  }
};

unsigned char* builddata(int depth, const unsigned char* pixels) {
  unsigned char* b = new unsigned char[SCALE*SCALE*16*16*depth];
  unsigned char* p = b;
  for (int y = 0; y < 16; y++) {
    for (int y1 = 0; y1 < SCALE; y1++) {
      const char* s = cdata+y*16;
      for (int x = 0; x < 16; x++) {
	int n;
	switch (*s++) {
	case 'R': n = 1; break;
	case 'G': n = 2; break;
	case 'B': n = 3; break;
	case 'r': n = 4; break;
	case 'g': n = 5; break;
	case 'b': n = 6; break;
	default: n = 0; break;
	}
	for (int xx = 0; xx < SCALE; xx++) {
	  const unsigned char* pp = pixels+depth*n;
	  for (int k=0; k < depth; k++) *p++ = *pp++;
	}
      }
    }
  }
  return b;
}

unsigned char monopixels[7] = {0, 60, 160, 40, 30, 130, 20};

unsigned char rgbpixels[7*3] = {
  0, 0, 0,
  255,0,0,
  0, 255, 0,
  0, 0, 255,
  127,0,0,
  0, 127, 0,
  0, 0, 127};

unsigned char rgbapixels[7*4] = {
  0, 0, 0, 0,
  255,0,0,255,
  0, 255, 0,255,
  0, 0, 255,255,
  127,0,0,127,
  0, 127, 0,127,
  0, 0, 127,127};

unsigned char rgbmpixels[7*4] = {
  255, 255, 255, 0,
  255,0,0,255,
  0, 255, 0,255,
  0, 0, 255,255,
  255, 0,0,127,
  0, 255, 0,127,
  0, 0, 255,127};

// Warning assummes sizeof(unsigned)==4!
unsigned argbpixels[7] = {
  0x0,
  0xffff0000,
  0xff00ff00,
  0xff0000ff,
  0x7f7f0000,
  0x7f007f00,
  0x7f00007f
};

unsigned mrgbpixels[7] = {
  0x00ffffff,
  0xffff0000,
  0xff00ff00,
  0xff0000ff,
  0x7fff0000,
  0x7f00ff00,
  0x7f0000ff
};

#if USE_X11
int visid = -1;
int arg(int argc, char **argv, int &i) {
  if (argv[i][1] == 'v') {
    if (i+1 >= argc) return 0;
    visid = atoi(argv[i+1]);
    i += 2;
    return 2;
  }
  return 0;
}
#endif

void bgcallback(Widget* w, void *v) {
  fltk::Color c = fltk::Color(((Slider*)w)->value());
  ((Window*)v)->color(c);
  fltk::Widget::default_style->buttoncolor(c);
  fltk::redraw();
}

void fgcallback(Widget* w, void *v) {
  fltk::Color c = fltk::Color(((Slider*)w)->value());
  fltk::Widget::default_style->labelcolor(c);
  fltk::Widget::default_style->textcolor(c);
  fltk::redraw();
}

void activecallback(Widget* w, void* v) {
  if (((Button*)w)->value()) ((Widget*)v)->activate();
  else ((Widget*)v)->deactivate();
}

bool checker = false;

void checkercallback(Widget* w, void*) {
  checker = ((Button*)w)->value();
  fltk::redraw();
}

class CheckGroup : public Group {
public:
  void draw() {
    if (checker) {
      fltk::setcolor(GRAY50);
      int x1 = 0;
      for (int y = 0; y < h(); y += 10) {
	x1 = 10-x1;
	for (int x = x1; x < w(); x += 20)
	  fltk::fillrect(x,y,10,10);
      }
    }
    Group::draw();
  }
  CheckGroup(int x, int y, int w, int h) : Group(x,y,w,h) {}
};

int main(int argc, char** argv) {

#if USE_X11
  int i = 1;
  if (args(argc,argv,i,::arg) < argc) {
    fprintf(stderr," -v # : use visual\n%s\n",help);
    exit(1);
  }

  if (visid >= 0) {
    open_display();
    XVisualInfo templt; int num;
    templt.visualid = visid;
    xvisual = XGetVisualInfo(xdisplay, VisualIDMask, &templt, &num);
    if (!xvisual) {
      fprintf(stderr, "No visual with id %d, use one of:\n",visid);
      list_visuals();
      exit(1);
    }
    xcolormap = XCreateColormap(xdisplay, RootWindow(xdisplay,xscreen),
				xvisual->visual, AllocNone);
# if !USE_CAIRO
    fltk::xpixel(fltk::BLACK); // make sure black is allocated
# endif
  } else {
    visual(RGB_COLOR);
  }
#endif

  const int COLS = 4;
  const int ROWS = 7;

  Window window(COLS*(16*SCALE+10)+10, 10+ROWS*(16*SCALE+15)+60+150);
  window.begin();
  CheckGroup group(0,0,window.w(),window.h()-60-150);
  window.resizable(group);
  group.resizable(group);
  group.begin();

  int y = 10;
  int x = 10;
#define nextxy() if ((x+=16*SCALE+10)>=window.w()) y += 16*SCALE+(x=10)+5

  Widget* w;

  w = new Dtest(fltk::MONO, x, y, "MONO", builddata(1, monopixels));
  nextxy();

  w = new Itest(fltk::MONO, x, y, "MONOimage", builddata(1, monopixels));
  nextxy();

  w = new Dtest(fltk::RGB, x, y, "RGB", builddata(3, rgbpixels));
  nextxy();

  w = new Itest(fltk::RGB, x, y, "RGB image", builddata(3, rgbpixels));
  nextxy();

  w = new Dtest(fltk::RGB32, x, y, "RGB32", builddata(4, (uchar*)argbpixels));
  nextxy();

  w = new Itest(fltk::RGB32, x, y, "RGB32image", builddata(4, (uchar*)argbpixels));
  nextxy();

  w = new Dtest(fltk::RGBx, x, y, "RGBx", builddata(4, rgbapixels));
  w->tooltip("drawimage() of an rgb image except the pixels are 4 apart, "
	      "some drivers will screw up and not skip the 4th byte");
  nextxy();

  w = new Itest(fltk::RGBx, x, y, "RGBx image", builddata(4, rgbapixels));
  nextxy();

  w = new Dtest(fltk::RGBA, x, y, "RGBA", builddata(4, rgbapixels));
  w->tooltip("drawimage() of 4-byte rgba. Should be transparent, not black");
  nextxy();

  w = new Itest(fltk::RGBA, x, y, "RGBAimage", builddata(4, rgbapixels));
  nextxy();

  w = new Dtest(fltk::ARGB32, x, y, "ARGB32", builddata(4, (uchar*)argbpixels));
  nextxy();

  w = new Itest(fltk::ARGB32, x, y, "ARGB32image", builddata(4, (uchar*)argbpixels));
  nextxy();

  w = new Dtest(fltk::RGBM, x, y, "RGBM", builddata(4, rgbmpixels));
  w->tooltip("drawimage() of 4-byte @i;unpremultiplied@n; rgba. Should be transparent, not white");
  nextxy();

  w = new Itest(fltk::RGBM, x, y, "RGBMimage", builddata(4, rgbmpixels));
  nextxy();

  w = new Dtest(fltk::MRGB32, x, y, "MRGB32", builddata(4, (uchar*)mrgbpixels));
  nextxy();

  w = new Itest(fltk::MRGB32, x, y, "MRGB32image", builddata(4, (uchar*)mrgbpixels));
  nextxy();

  w = new Dtest(fltk::MASK, x, y, "MASK", builddata(1, monopixels));
  nextxy();

  w = new Itest(fltk::MASK, x, y, "MASKimage", builddata(1, monopixels));
  nextxy();

  w = new Xtest(x, y, "mask xpmImage", recycle_xpm);
  w->tooltip("In some versions of FLTK2, the only MASK style image was "
	     "special monochrome XPM files. These should continue to work.");
  nextxy();

  w = new Btest(x, y, "xbmImage");
  w->tooltip("In older versions of FLTK, the only MASK style image was "
	     "the fltk::xbmImage object, which only had 1 bit for pixel. "
	     "This should continue to work and display the same colors "
	     "as maskXPM and MASK.");
  nextxy();

  w = new Xtest(x, y, "xpmImage", porsche_xpm);
  w->tooltip("In older versions of FLTK, the only transparency available "
	     "was from XPM and GIF images. This should continue to work.");
  nextxy();

  w = new Insettest(fltk::RGBx, x, y, "Insetimage", builddata(4, rgbapixels));
  nextxy();

  group.end(); y = group.h();
  CheckButton* b = new CheckButton(20,y,100,20,"active");
  b->tooltip("activate()/deactivate() the widgets. The ones using an Image "
	     "will redraw to show the grayed-out state.");
  b->set();
  b->callback(activecallback, &group);

  b = new CheckButton(100,y,100,20,"checker");
  b->tooltip("Draw a checkerboard to test transparency.");
  b->callback(checkercallback, &group);
  y += 20;

  Slider* s = new Slider(20,y,window.w()-30,20,"bg");
  s->tooltip("background color, change this to see transparency");
  s->range(1,255);
  s->step(1);
  s->value(fltk::GRAY75);
  s->align(ALIGN_LEFT);
  s->callback(bgcallback, &window);
  s = new Slider(20,y+20,window.w()-30,20,"fg");
  s->tooltip("labelcolor, change this to see if MASK works");
  s->range(1,255);
  s->step(1);
  s->value(fltk::BLACK);
  s->callback(fgcallback, &window);
  s->align(ALIGN_LEFT);

  Widget text(0,y+40,window.w(),window.h()-y-40,
	      "* All images should scale to fill their rectangle as you resize the window.\n"
              "* Images with RGB in their names should have the letters opaque and the correct color.\n"
              "* RGBA and RGBM images should look identical and be transparent, "
              "with 100% transparency around the upper-left.\n"
              "* MASK images should match the fg color as you change it.\n"
              "Currently only OS/X version has no errors.");
  text.align(ALIGN_LEFT|ALIGN_INSIDE|ALIGN_WRAP);

  window.end();

#if USE_X11
  // try to set the icon on X
  uchar* data = builddata(4, (uchar*)argbpixels);
  const unsigned width = 16*SCALE;
  const unsigned height = 16*SCALE;
  unsigned* icon = new unsigned[width*height+2];
  icon[0] = width;
  icon[1] = height;
  memcpy(icon+2, data, width*height*4);
  window.icon(icon);
#endif

  window.show(argc, argv);

  return fltk::run();
}
