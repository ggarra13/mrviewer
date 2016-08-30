// drawing.cxx (example4)

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Widget.h>
#include <fltk/Slider.h>
#include <fltk/math.h>
#include <fltk/draw.h>
#include <fltk/string.h> // for snprintf

using namespace fltk;

class ShapeWidget : public Widget {
  int sides_;

  void draw() {
    setcolor(BLACK);
    fillrect(0,0,w(),h());
    push_matrix();
    scale(w()/2.0f, h()/2.0f);
    translate(1,1);
    setcolor(0x8098b000);
    int i; for (i=0; i<sides(); i++) {
      double ang = i*2*M_PI/sides();
      addvertex(cosf(ang),sinf(ang));
    }
    fillstrokepath(WHITE);
    pop_matrix();
    setfont(labelfont(), labelsize());
    setcolor(WHITE);
    char buf[200];
    double ang = 2*M_PI/sides();
    snprintf(buf, 200,
	     "%d sides\n"
	     "Angle between sides = %g°\n"
	     "Length of side = %g\n"
	     "Perimiter = %g\n"
	     "Area = %g",
	     i,
	     360.0/i,
	     sqrt(2-2*cos(ang)),
	     sides()*sqrt(2-2*cos(ang)),
	     sides()*sin(ang)/2);
    drawtext(buf, Rectangle(w(),h()), ALIGN_WRAP);
  }


public:

  inline int sides() const {return sides_;}

  void sides(int n) {
    if (sides_ != n) {sides_ = n; redraw();}
  }

  ShapeWidget(int x,int y,int w,int h,const char *l=0) :
    Widget(x,y,w,h,l), sides_(3) {}

};

static void slider_callback(Widget* widget, void* data) {
  ((ShapeWidget*)data)->sides(int(((Slider*)widget)->value()));
}

int main(int argc, char **argv) {

  Window window(300, 330);
  window.begin();

  ShapeWidget sw(10, 10, 280, 280);
  window.resizable(&sw);

  Slider slider(50, 295, window.w()-60, 30, "Sides:");
  slider.clear_flag(ALIGN_MASK);
  slider.set_flag(ALIGN_LEFT);
  slider.callback(slider_callback, &sw);
  slider.value(sw.sides());
  slider.step(1);
  slider.range(3,40);

  window.end();
  window.show(argc,argv);
    
  return run();
}
