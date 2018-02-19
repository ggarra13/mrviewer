// Test of the fltk::Monitor calls

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Monitor.h>
#include <fltk/draw.h>
#include <stdio.h>

using namespace fltk;

void print_monitor(const Monitor& m, int i) {
  if (i < 0) printf("all: ");
  else printf("%d: ", i);
  printf("area %d %d %d %d, work %d %d %d %d, depth %d, dpi %g %g\n",
         m.x(), m.y(), m.w(), m.h(),
         m.work.x(), m.work.y(), m.work.w(), m.work.h(),
         m.depth(), m.dpi_x(), m.dpi_y());
}

class MonitorWindow : public Window {
public:
  MonitorWindow() : Window(500,500) {resizable(this);}

  void draw_monitor(const Monitor& m, int i) {
    setcolor(GRAY50);
    strokerect(m.work);
    setcolor(BLACK);
    strokerect(m);
    char b[256];
    char* p = b;
    if (i < 0) p += sprintf(p,"all: ");
    else p += sprintf(p, "%d: ", i);
    sprintf(p, "area %d %d %d %d\nwork %d %d %d %d\ndepth %d, dpi %g %g",
            m.x(), m.y(), m.w(), m.h(),
            m.work.x(), m.work.y(), m.work.w(), m.work.h(),
            m.depth(), m.dpi_x(), m.dpi_y());
    if (i < 0)
      drawtext(b, Rectangle(m.x(),m.b(),m.w(),1000), ALIGN_TOP|ALIGN_RIGHT|ALIGN_WRAP);
    else 
      drawtext(b, m, ALIGN_CENTER|ALIGN_WRAP|ALIGN_CLIP);
  }

  void draw() {
    draw_box();
    setfont(textfont(), textsize());
    const Monitor& all = Monitor::all();
    push_matrix();
    translate(w()/10,h()/10);
    float s = w()*.8/all.w();
    float s2 = h()*.8/all.h();
    if (s2 < s) s = s2;
    scale(s,s);
    translate(-all.x(), -all.y());

    setcolor(GRAY25);
    Rectangle b; borders(&b);
    fillrect(this->x()+b.x(), this->y()+b.y(), this->w()+b.w(), this->h()+b.h());
    setcolor(GRAY50);
    fillrect(*this);
    setcolor(WHITE);
    drawtext("this window", *this, ALIGN_CENTER|ALIGN_WRAP|ALIGN_CLIP);

    draw_monitor(all, -1);

    const Monitor* list;
    int size = Monitor::list(&list);
    for (int i=0; i<size; i++)
      draw_monitor(list[i], i);

    pop_matrix();
  }

  void layout() { // make it redraw when user moves window
    redraw();
    Window::layout();
  }
};

int main(int argc, char** argv) {
  MonitorWindow window;
  window.show(argc,argv);

  print_monitor(Monitor::all(),-1);

  const Monitor* list;
  int size = Monitor::list(&list);

  for (int i=0; i<size; i++)
    print_monitor(list[i],i);

  return fltk::run();
}
