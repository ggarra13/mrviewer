// Test of how fast drawing is in various scenarios
// Requires X because it uses XSync

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/x.h>
#include <fltk/draw.h>
#include <stdio.h>
#include <stdlib.h>

using namespace fltk;

class TestWindow : public fltk::Window {
  int test;
  int iter;
public:
  TestWindow(int t) : Window(300,300), test(t) {
    if (t&1) set_double_buffer(); else clear_double_buffer();
    iter = 1;
  }
  void draw();
};

void TestWindow::draw() {
  char buffer[128];
  sprintf(buffer, "Redraw number %d, text in the widget is clipped off somewhere on the right",
	  ++iter);
  if (test&2) { // new version
    setcolor(GRAY75);
    fillrect(0,0,w(),h());
    for (int y = 10; y+21 < h(); y+=25) {
      int X = 5+y/10;
      int Y = y;
      int W = w()-5-X;
      int H = 21;
      push_clip(X,Y,W,H);
      setcolor(BLACK);
      fillrect(X,Y,W,H);
      if (y < 11) {
	setcolor(WHITE);
	setfont(fltk::HELVETICA,12);
	drawtext(buffer,X+3,Y+16);
      }
      pop_clip();
    }
  } else {
    // clip-out version:
    push_clip(0,0,w(),h());
    for (int y = 10; y+21 < h(); y+=25) {
      int X = 5+y/10;
      int Y = y;
      int W = w()-5-X;
      int H = 21;
      push_clip(X,Y,W,H);
      setcolor(BLACK);
      fillrect(X,Y,W,H);
      if (y < 11) {
	setcolor(WHITE);
	setfont(fltk::HELVETICA,12);
	drawtext(buffer,X+3,Y+16);
      }
      pop_clip();
      clipout(Rectangle(X,Y,W,H));
    }
    setcolor(GRAY75);
    fillrect(0,0,w(),h());
    pop_clip();
  }
}

int main(int argc, char** argv) {
  int m; int n = fltk::args(argc, argv, m);
  if (n != argc-2) {
    fprintf(stderr, "%s\nAdd test number and number of interations:\n"
	    " 0 = single buffer + clip out\n"
	    " 1 = double buffer + clip out\n"
	    " 2 = single buffer with overlapping rectangles\n"
	    " 3 = double buffer with overlapping rectangles\n",
	    fltk::help);
    exit(1);
  }
  TestWindow window(atoi(argv[n]));
  int iterations = atoi(argv[n+1]);
  double dt = get_time_secs();
  printf("iterations = %d\n", iterations);
  window.show(argc, argv);
  while (window.damage()) fltk::wait();
  for (int i=0; i < iterations; i++) {
    window.redraw();
    fltk::check();
  }
  dt = get_time_secs() - dt;
  printf("time elapsed = %4.6lf secs\n",dt);
  return 0;
}

