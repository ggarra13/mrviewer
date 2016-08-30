// button.cxx (example2a)

#include <stdlib.h>
#include <stdio.h>
#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Button.h>
#include <fltk/InvisibleBox.h>
#include <fltk/TiledImage.h>
#include <fltk/SharedImage.h>
using namespace fltk;

void beepcb(Widget *, void *) {
  printf("\007"); fflush(stdout);
}

void exitcb(Widget *, void *) {
  exit(0);
}

int main(int argc, char ** argv) {
  register_images();
  Window *window = new Window(320,65);

  window->begin();
  Group* ib = new Group(0,0,window->w(),window->h());
  ib->begin();
  ib->image(new TiledImage(SharedImage::get("./images/bg.jpeg")));
  window->resizable(ib);

  Button *b1 = new Button(20, 20, 80, 25, "&Beep");
  b1->callback(beepcb,0);
  
  Button *b2 = new Button(120,20, 80, 25, "");
  b2->image(new TiledImage(SharedImage::get("./images/coucou.png")));

  Button *b3 = new Button(220,20, 80, 25, "E&xit");
  b3->callback(exitcb,0);
  ib->end();
  window->end();
  window->show(argc,argv);
  return run();
}

//
// End of "$Id: button.cxx 5465 2006-09-19 03:21:56Z spitzak $".
//
