#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Item.h>
#include <fltk/ValueSlider.h>
#include <fltk/draw.h>
#include <fltk/Choice.h>
using namespace fltk;

Window *form;
Slider *sliders[8];
Choice *choice[3];

class test_box: public Window {
  void draw();
public:
  test_box(int x,int y,int w,int h,const char *l=0)
    : Window(x,y,w,h,l) {}
} *test;

void test_box::draw() {
  Window::draw();
  setcolor(fltk::color((uchar)(sliders[0]->value()),
		       (uchar)(sliders[1]->value()),
		       (uchar)(sliders[2]->value())));
  char buf[5];
  buf[0] = char(sliders[4]->value());
  buf[1] = char(sliders[5]->value());
  buf[2] = char(sliders[6]->value());
  buf[3] = char(sliders[7]->value());
  buf[4] = 0;
  line_style(
    (int)(choice[0]->get_item()->argument()) +
    (int)(choice[1]->get_item()->argument()) +
    (int)(choice[2]->get_item()->argument()),
    (int)(sliders[3]->value()),
    buf);
  strokerect(10,10,w()-20,h()-20);
  addvertex(35, 35);
  addvertex(w()-35, h()-35);
  addvertex(w()/2, 35);
  addvertex(35, h()/2);
  strokepath();
  drawpoint(w()-35, 35);
  drawline(35, h()*3/4, w()-35, h()*3/4);
  // you must reset the line type when done:
  line_style(SOLID);
}

void build_style_menu(Group *g) {
  g->begin();
  (new Item("fltk::SOLID"))->argument(SOLID);
  (new Item("fltk::DASH"))->argument(DASH);
  (new Item("fltk::DOT"))->argument(DOT);
  (new Item("fltk::DASHDOT"))->argument(DASHDOT);
  (new Item("fltk::DASHDOTDOT"))->argument(DASHDOTDOT);
  g->end();
}

void build_cap_menu(Group *g) {
  g->begin();
  new Item("default");
  (new Item("fltk::CAP_FLAT"))->argument(CAP_FLAT);
  (new Item("fltk::CAP_ROUND"))->argument(CAP_ROUND);
  (new Item("fltk::CAP_SQUARE"))->argument(CAP_SQUARE);
  g->end();
}

void build_join_menu(Group *g) {
  g->begin();
  new Item("default");
  (new Item("fltk::JOIN_MITER"))->argument(JOIN_MITER);
  (new Item("fltk::JOIN_ROUND"))->argument(JOIN_ROUND);
  (new Item("fltk::JOIN_BEVEL"))->argument(JOIN_BEVEL);
  g->end();
}

void do_redraw(Widget*,void*)
{
    test->redraw();
}

void makeform(const char *) {
  form = new Window(500,210,"line_style() test");
  form->begin();
  sliders[0]= new ValueSlider(280,10,180,20,"R");
  sliders[0]->maximum(255);
  sliders[1]= new ValueSlider(280,30,180,20,"G");
  sliders[1]->maximum(255);
  sliders[2]= new ValueSlider(280,50,180,20,"B");
  sliders[2]->maximum(255);
  choice[0]= new Choice(280,70,180,20,"Style");
  build_style_menu(choice[0]);
  choice[1]= new Choice(280,90,180,20,"Cap");
  build_cap_menu(choice[1]);
  choice[2]= new Choice(280,110,180,20,"Join");
  build_join_menu(choice[2]);
  sliders[3]= new ValueSlider(280,130,180,20,"Width");
  sliders[3]->maximum(20);
  sliders[4] = new Slider(200,170,70,20,"Dash");
  sliders[4]->align(ALIGN_TOP|ALIGN_LEFT);
  sliders[4]->maximum(40);
  sliders[5] = new Slider(270,170,70,20);
  sliders[5]->maximum(40);
  sliders[6] = new Slider(340,170,70,20);
  sliders[6]->maximum(40);
  sliders[7] = new Slider(410,170,70,20);
  sliders[7]->maximum(40);
  int i;
  for (i=0;i<8;i++) {
    sliders[i]->type(1);
    if (i<4) sliders[i]->align(ALIGN_LEFT);
    sliders[i]->callback((Callback*)do_redraw);
    sliders[i]->step(1);
  }
  for (i=0;i<3;i++) {
    choice[i]->value(0);
    choice[i]->callback((Callback*)do_redraw);
  }
  test=new test_box(10,10,190,190);
  form->resizable(test);
  form->end();
}

int main(int argc, char **argv) {
  makeform(argv[0]);
  form->show(argc,argv);
  return run();
}
