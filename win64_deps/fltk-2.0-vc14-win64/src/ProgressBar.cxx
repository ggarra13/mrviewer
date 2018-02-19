#include <fltk/ProgressBar.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <stdio.h>
using namespace fltk;

/*! \class fltk::ProgressBar

  ProgressBar is a widget meant to display progress of some operation.
  maximum() and optionally minimum() values should be set, and for
  each step of operation step() should be called.
*/

/*! \fn ProgressBar::minimum(double nm)
  Set minimal value for in the progess widget.
*/

/*! \fn ProgressBar::maximum(double nm)
  Set maximal value for in the progess widget. It should represent operation length.
*/

/*! \fn ProgressBar::position(double value)
  Set position of bar in progress widget and redraw it. If value
  goes out of minimum()/maximum() bounds, it will be ignored.
*/

/*! \fn ProgressBar::step(double step)
  Increase bar length with given step and redraw widget. If value
  goes out of minimum() and maximum() bounds, it will be ignored.
*/

/*! \fn ProgressBar::range(double min, double max, double step)
  A shorthand for minimum(), maximum() and step().
*/

/*! \fn ProgressBar::showtext(bool st)
  Shows completition percentage inside progress widget.
*/

ProgressBar::ProgressBar(int x, int y, int w, int h, const char *lbl)
  : Widget(x,y,w,h,lbl)
{
  mMin = mPresent = 0; 
  mMax = 100; 
  mShowPct = true; 
  box(DOWN_BOX);
  selection_color(BLUE);
  color(WHITE);
  textcolor(RED);
}

void ProgressBar::draw() {
  drawstyle(style(),flags());
  if (damage() & DAMAGE_ALL) draw_box();
  Rectangle r(w(),h());
  box()->inset(r);
  if (mPresent > mMax) mPresent = mMax;
  if (mPresent < mMin) mPresent = mMin;
  double pct = mMax != mMin ? (mPresent - mMin) / (mMax - mMin) : 0;

  if (vertical()) {
    int barHeight = int(r.h()*pct+.5);
    r.y(r.y()+r.h()-barHeight);
    r.h(barHeight);
  } else {
    r.w(int(r.w()*pct+.5));
  }

  setcolor(selection_color());
  fillrect(r);
  if (mShowPct) {
    char buffer[30];
    sprintf(buffer, "%d%%", int(pct * 100+.5));
    setcolor(textcolor());
    setfont(this->labelfont(), this->labelsize());
    drawtext(buffer, Rectangle(w(),h()), ALIGN_CENTER);
  }
}

