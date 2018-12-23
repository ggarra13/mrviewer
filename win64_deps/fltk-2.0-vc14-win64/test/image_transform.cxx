// Test of drawimage() through a transform. Currently only the
// XRender implementation works.

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/ValueSlider.h>
#include <fltk/draw.h>
#include <fltk/RadioButton.h>
#include <fltk/rgbImage.h>

using namespace fltk;

#define SLIDERS 9

float xy[SLIDERS] = {
  -64, -64, 0, 0, 1, 1, 0, 0, 0};
const char* name[SLIDERS] = {
  "X", "Y", "tx", "ty", "sx", "sy", "rotate", "fromxy", "toxy"};

#define SIZE 129
unsigned array[SIZE*SIZE];

bool image = false;
bool alpha = false;

void fillimage() {
  for (int y=0; y<SIZE; y++) for (int x=0; x<SIZE; x++) {
    uchar r = x*255/SIZE;
    uchar g = y*255/SIZE;
    uchar b = 255-g;
    uchar a = r > g ? r : g; if (b > a) a = b;
    if (x%16==0 || y%16==0) {r = 255-r; g = 255-g; b = 255-b; a = 0xff;}
    array[y*SIZE+x] = (a<<24)|(r<<16)|(g<<8)|b;
  }
}

rgbImage* theimage;

class Drawing : public Widget {
  void draw() {
    push_clip(0, 0, w(),h());
    setcolor(GRAY20);
    fillrect(0,0,w(),h());
    setcolor(GRAY10);
    fillrect(0,0,w()/2,h()/2);
    fillrect(w()/2,h()/2,(w()+1)/2,(h()+1)/2);
    push_matrix();
    translate(w()/2+xy[2], h()/2+xy[3]);
    scale(xy[4],xy[5]);
    rotate(xy[6]);
    if (::image) {
      Rectangle r(xy[7],xy[7],SIZE-xy[7],SIZE-xy[7]);
      Rectangle r1(xy[8]+xy[0],xy[8]+xy[1],SIZE-xy[8],SIZE-xy[8]);
      ::theimage->draw(r,r1);
    } else {
      drawimage((uchar*)array,
		alpha ? ARGB32 : RGB32,
		fltk::Rectangle((int)xy[0], (int)xy[1], SIZE, SIZE));
    }
    setcolor(GRAY80);
    addvertex(xy[0],xy[1]);
    addvertex(xy[0],xy[1]+SIZE);
    addvertex(xy[0]+SIZE,xy[1]+SIZE);
    addvertex(xy[0]+SIZE,xy[1]);
    closepath();
    strokepath();
    pop_matrix();
    pop_clip();
  }
public:
  Drawing(int X,int Y,int W,int H) : Widget(X,Y,W,H) {
    fillimage();
    ::theimage = new rgbImage((uchar*)array, ARGB32, SIZE, SIZE);
  }
};

Drawing *d;

void slider_cb(Widget* o, void* v) {
  Slider* s = (Slider*)o;
  xy[long(v)] = s->value();
  d->redraw();
}

void noalpha_cb(Widget* o, void*) {
  alpha = image = false;
  d->redraw();
}

void alpha_cb(Widget* o, void*) {
  
  alpha = reinterpret_cast<Button*>(o)->value(); image = false;
  d->redraw();
}

void image_cb(Widget* o, void*) {
  image = true;
  d->redraw();
}

int main(int argc, char** argv) {
  Window window(300,555+50,"drawimage & transformations test");
  window.begin();
  Drawing drawing(10,10,280,280);
  d = &drawing;

  int y = 300;
  for (int n = 0; n<SLIDERS; n++) {
    ValueSlider* s = new ValueSlider(50,y,240,25,name[n]); y += 25;
    s->type(Slider::TICK_ABOVE);
    s->step(1);
    if (n < 4) s->range(-200,200);
    else if (n < 6) {s->range(-1,3); s->step(.01);}
    else if (n < 7) s->range(-180, 180);
    else {s->range(0,SIZE); s->tooltip("top-left corner of rectangle passed to Image::draw()");}
    s->value(xy[n]);
    s->align(ALIGN_LEFT);
    s->callback(slider_cb, (void*)n);
  }
  RadioButton* b = new RadioButton(50,y,240,25,"RGB32 fltk::drawimage()"); y+= 25;
  b->callback(noalpha_cb); b->set();
  b = new RadioButton(50,y,240,25,"ARGB32 fltk::drawimage()"); y+= 25;
  b->callback(alpha_cb);
  b = new RadioButton(50,y,240,25,"ARGB32 fltk::Image object"); y+= 25;
  b->callback(image_cb);

  window.end();
  window.show(argc,argv);
  return run();
}

//
// End of "$Id: image_transform.cxx 1464 2006-12-28 21:37:21Z spitzak $".
//
