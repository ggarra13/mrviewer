#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/ProgressBar.h>
#include <stdlib.h>

using namespace fltk;

Window* win;

static void ptimer(void *o)
{
	ProgressBar *pbar = (ProgressBar *)o;
	if(pbar->position() < 100)
	{
		pbar->step(1);
		add_timeout(0.1, ptimer, (void *)pbar);
	}
	else
		win->hide();
}

int main(int argc, char **argv) {
  ProgressBar* pbar;
  int nargs = 0;
  { Window* o = new Window(400, 100);
    o->begin();
    win = o;
    { ProgressBar* o = new ProgressBar(25, 25, 330, 25, "Simple Progress Bar");
      if (argc>1) {nargs++; o->minimum(atoi(argv[1]));}
      if (argc>2) {nargs++; o->maximum(atoi(argv[2]));}

	  pbar = o;
	  //pbar->set_vertical();
      o->box(ENGRAVED_BOX);
#if (MAJOR_VERSION > 1)
	  o->clear_flag(ALIGN_MASK);
	  o->set_flag(ALIGN_BOTTOM);
#else
	  o->align(ALIGN_BOTTOM);
#endif
	  o->selection_color(BLUE);
	  o->color(WHITE);
	  o->textcolor(RED);
    }
    o->end();
  }
  add_timeout(0.1, ptimer, (void *)pbar);
  if (nargs) argv[nargs] = argv[0];
  win->show(argc-nargs, &argv[nargs]);
  return run();
}
