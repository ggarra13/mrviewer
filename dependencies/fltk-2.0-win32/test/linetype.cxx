#include <fltk/Fl.h>
#include <fltk/Fl_Window.h>
#include <fltk/Fl_Value_Slider.h>
#include <fltk/fl_draw.h>
#include <fltk/Fl_Choice.h>

Fl_Window *form;
Fl_Slider *sliders[8];
Fl_Choice *choice[3];

class test_box: public Fl_Window {
  void draw();
public:
  test_box(int x,int y,int w,int h,const char *l=0)
    : Fl_Window(x,y,w,h,l) {}
}*test;

void test_box::draw() {
  Fl_Window::draw();
  fl_color(fl_rgb((uchar)(sliders[0]->value()),
		  (uchar)(sliders[1]->value()),
		  (uchar)(sliders[2]->value())));
  char buf[5];
  buf[0] = char(sliders[4]->value());
  buf[1] = char(sliders[5]->value());
  buf[2] = char(sliders[6]->value());
  buf[3] = char(sliders[7]->value());
  buf[4] = 0;
  fl_line_style(
    (int)(choice[0]->mvalue()->argument()) +
    (int)(choice[1]->mvalue()->argument()) +
    (int)(choice[2]->mvalue()->argument()),
    (int)(sliders[3]->value()),
    buf);
  fl_rect(10,10,w()-20,h()-20);
  fl_begin_line();
  fl_vertex(35, 35);
  fl_vertex(w()-35, h()-35);
  fl_vertex(w()-40, 35);
  fl_vertex(35, h()/2);
  fl_end_line();
  // you must reset the line type when done:
  fl_line_style(FL_SOLID);
}

Fl_Menu_Item style_menu[] = {
  {"FL_SOLID",	0, 0, (void*)FL_SOLID},
  {"FL_DASH",	0, 0, (void*)FL_DASH},
  {"FL_DOT",	0, 0, (void*)FL_DOT},
  {"FL_DASHDOT",0, 0, (void*)FL_DASHDOT},
  {"FL_DASHDOTDOT", 0, 0, (void*)FL_DASHDOTDOT},
  {0}
};

Fl_Menu_Item cap_menu[] = {
  {"default",		0, 0, 0},
  {"FL_CAP_FLAT",	0, 0, (void*)FL_CAP_FLAT},
  {"FL_CAP_ROUND",	0, 0, (void*)FL_CAP_ROUND},
  {"FL_CAP_SQUARE",	0, 0, (void*)FL_CAP_SQUARE},
  {0}
};

Fl_Menu_Item join_menu[] = {
  {"default",		0, 0, 0},
  {"FL_JOIN_MITER",	0, 0, (void*)FL_JOIN_MITER},
  {"FL_JOIN_ROUND",	0, 0, (void*)FL_JOIN_ROUND},
  {"FL_JOIN_BEVEL",	0, 0, (void*)FL_JOIN_BEVEL},
  {0}
};

void do_redraw(Fl_Widget*,void*)
{
    test->redraw();
}

void makeform(const char *) {
  form = new Fl_Window(500,210,"line type test");
  sliders[0]= new Fl_Value_Slider(280,10,180,20,"R");
  sliders[0]->bounds(0,255);
  sliders[1]= new Fl_Value_Slider(280,30,180,20,"G");
  sliders[1]->bounds(0,255);
  sliders[2]= new Fl_Value_Slider(280,50,180,20,"B");
  sliders[2]->bounds(0,255);
  choice[0]= new Fl_Choice(280,70,180,20,"Style");
  choice[0]->menu(style_menu);
  choice[1]= new Fl_Choice(280,90,180,20,"Cap");
  choice[1]->menu(cap_menu);
  choice[2]= new Fl_Choice(280,110,180,20,"Join");
  choice[2]->menu(join_menu);
  sliders[3]= new Fl_Value_Slider(280,130,180,20,"Width");
  sliders[3]->bounds(0,20);
  sliders[4] = new Fl_Slider(200,170,70,20,"Dash");
  sliders[4]->align(FL_ALIGN_TOP_LEFT);
  sliders[4]->bounds(0,40);
  sliders[5] = new Fl_Slider(270,170,70,20);
  sliders[5]->bounds(0,40);
  sliders[6] = new Fl_Slider(340,170,70,20);
  sliders[6]->bounds(0,40);
  sliders[7] = new Fl_Slider(410,170,70,20);
  sliders[7]->bounds(0,40);
  int i;
  for (i=0;i<8;i++) {
    sliders[i]->type(1);
    if (i<4) sliders[i]->align(FL_ALIGN_LEFT);
    sliders[i]->callback((Fl_Callback*)do_redraw);
    sliders[i]->step(1);
  }
  for (i=0;i<3;i++) {
    choice[i]->value(0);
    choice[i]->callback((Fl_Callback*)do_redraw);
  }
  test=new test_box(10,10,190,190);
  test->end();
  form->resizable(test);
  form->end();
}

main(int argc, char **argv) {
  makeform(argv[0]);
  form->show(argc,argv);
  return Fl::run();
}

