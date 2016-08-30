// helloask.cxx (example2a)

#include <fltk/Window.h>
#include <fltk/Widget.h>
#include <fltk/run.h>
#include <fltk/ask.h>
using namespace fltk;

void window_callback(Widget* widget, void*) {
  if (ask("Do you really want to exit?"))
    ((Window*)widget)->hide();
}

int main(int argc, char **argv) {
  Window *window = new Window(300, 180);
  window->callback(window_callback);
  window->begin();
  Widget *box = new Widget(20, 40, 260, 100, "Hello, World!");
  box->box(UP_BOX);
  box->labelfont(HELVETICA_BOLD_ITALIC);
  box->labelsize(36);
  box->labeltype(SHADOW_LABEL);
  window->end();
  window->show(argc, argv);
  return run();
}
