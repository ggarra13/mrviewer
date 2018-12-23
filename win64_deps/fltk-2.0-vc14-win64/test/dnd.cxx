#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/events.h>
#include <fltk/string.h>
#include <fltk/MultiLineOutput.h>
#include <stdio.h>

using namespace fltk;

Widget* bluewidget = 0;
Widget* greenwidget = 0;
MultiLineOutput* output = 0;

class DNDTarget : public Widget {
public:
  int handle(int event) {
    switch (event) {
    case PUSH:
      //printf("%s : starting dnd()\n", label());
      if (greenwidget) {
        greenwidget->color(0); greenwidget->redraw(); greenwidget = 0;
      }
      bluewidget = this; color(12); redraw();
      fltk::copy(label(), strlen(label()));
      fltk::dnd();
      bluewidget = 0;
      if (this != greenwidget) {color(0); redraw();}
      return true;
    case DND_ENTER:
      if (greenwidget) {
        greenwidget->color(0); greenwidget->redraw(); greenwidget = 0;
      }
      //printf("%s : DND_ENTER\n", label());
      color(RED); redraw();
      return 1;
    case DND_DRAG:
      // bug? You must implement this and return 1 or drop will not happen
      return 1;
    case DND_LEAVE:
      //printf("%s : DND_LEAVE\n", label());
      color(this==bluewidget ? 12 : 0); redraw();
      return 1;
    case DND_RELEASE:
      //printf("%s : DND_RELEASE\n", label());
      color(10); redraw(); // this should never be seen as paste replaces it!
      return 1;
    case PASTE:
      //printf("%s : PASTE '%s'\n", label(), fltk::event_text());
      greenwidget = this; color(GREEN); redraw();
      ::output->text(fltk::event_text());
      return 1;
    default:
      return Widget::handle(event);
    }
  }

  DNDTarget(int x, int y, int w, int h, const char* l) :
    Widget(x,y,w,h) { copy_label(l); }
};

#define SIZE 30
#define BORDER 10
#define NUM 5

int main(int argc, char** argv) {
  Window window(NUM*(SIZE+BORDER)+BORDER,
                NUM*(SIZE+BORDER)+BORDER+50+2*BORDER);
  window.begin();
  for (int y = 0; y < NUM; y++) {
    for (int x = 0; x < NUM; x++) {
      char buf[100];
      sprintf(buf, "%d", x+NUM*y+1);
      new DNDTarget(BORDER+x*(SIZE+BORDER), BORDER+y*(SIZE+BORDER),
                    SIZE, SIZE, buf);
    }
  }
  output = new MultiLineOutput(BORDER, 2*BORDER+NUM*(SIZE+BORDER),
                               NUM*(SIZE+BORDER)-BORDER, 50,
                               "Last paste:");
  output->align(fltk::ALIGN_TOP|fltk::ALIGN_LEFT);
  window.show(argc, argv);
  return fltk::run();
}
