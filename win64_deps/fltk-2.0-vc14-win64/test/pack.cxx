#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/BarGroup.h>
#include <fltk/PackedGroup.h>
using namespace fltk;

BarGroup* makebar(const char* name, int w, int h, bool vertical) {
  BarGroup* b = new BarGroup(0,0,w,h,name);
  b->box(THIN_UP_BOX);
  if (vertical) b->set_vertical();
  return b;
}

int main(int argc, char** argv) {
  Window window(300,300,"PackedGroup test");
  window.begin();
  PackedGroup group(0,0,300,300);
  window.resizable(group);
  group.begin();
  makebar("bar0",30,30,false);
  makebar("bar1",30,30,false);
  makebar("bar2",30,30,true);
  makebar("bar3",30,30,true);
  makebar("bar4",30,30,false);
  Widget resize(0,0,30,30,"resizable()");
  resize.box(FLAT_BOX);
  resize.color(GRAY15);
  resize.labelcolor(WHITE);
  group.resizable(resize);
  makebar("bar5",30,30,false);
  makebar("bar6",30,30,true);
  makebar("bar7",30,30,true);
  makebar("bar8",30,30,false);
  makebar("bar9",30,30,false);
  window.end();
  window.show(argc, argv);
  return run();
}
