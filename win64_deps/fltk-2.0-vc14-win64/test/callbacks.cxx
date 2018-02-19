// callback.cxx (example 2b)

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Slider.h>
#include <fltk/Button.h>
#include <fltk/IntInput.h>
#include <stdlib.h>
using namespace fltk;

IntInput* intinput;

void copy_callback(Widget*, void* v) {
  Slider* slider = (Slider*)v;
  slider->value(intinput->ivalue());
}

void down_callback(Widget*, void* v) {
  Slider* slider = (Slider*)v;
  slider->value(slider->value()-1);
  intinput->value(slider->value());
}

void up_callback(Widget*, void* v) {
  Slider* slider = (Slider*)v;
  slider->value(slider->value()+1);
  intinput->value(slider->value());
}

void slider_callback(Widget* w, void*) {
  Slider* slider = (Slider*)w;
  intinput->value(slider->value());
}

void exit_callback(Widget *, void *) {
  exit(0);
}

int main(int argc, char ** argv) {
  Window window(320, 90);
  window.begin();
  IntInput intinput(10,10,100,20);
  ::intinput = &intinput;
  intinput.value(0.0);
  Button copy_button(110, 10, 100, 20, "copy to slider");
  Slider slider(10,35,300,20);
  slider.type(Slider::TICK_ABOVE);
  slider.clear_flag(LAYOUT_VERTICAL);
  slider.callback(slider_callback);
  copy_button.callback(copy_callback, &slider);
  slider.range(-10,10);
  slider.step(1);
  slider.value(0);
  Button down_button(50,60,50,20,"down");
  down_button.callback(down_callback, &slider);
  Button up_button(150,60,50,20,"up");
  up_button.callback(up_callback, &slider);
  Button exit_button(250,60,50,20,"exit");
  exit_button.callback(exit_callback);
  window.end();
  window.show(argc,argv);
  return run();
}
