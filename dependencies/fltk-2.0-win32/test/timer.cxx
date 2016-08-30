#include <fltk/run.h>
#include <fltk/Widget.h>
#include <fltk/Window.h>
#include <fltk/ValueInput.h>
#include <fltk/ValueOutput.h>
#include <fltk/Slider.h>

using namespace fltk;

double cps = 24.0;
double period = 1.0/24.0;

unsigned ticks;
static double starttime;
unsigned prevticks;

//#define SLIDER_TICKS cps
// A prime number to avoid system tendency to do things in multiples of 2
// and 100:
#define SLIDER_TICKS 139

ValueOutput* actual;
int timing_period = 1;

void timeout_callback(void* v) {
  ticks++;
  ((Slider*)v)->value(ticks%SLIDER_TICKS);
  fltk::repeat_timeout(period, timeout_callback, v);

  if (ticks-prevticks > cps*timing_period) {
    double now = fltk::get_time_secs();
    actual->value((ticks-prevticks)/(now-starttime));
    starttime = now;
    prevticks = ticks;
    timing_period++;
  }
}

void fps_callback(Widget* w, void* v) {
  cps = ((ValueInput*)w)->value();
  //((Slider*)v)->maximum(SLIDER_TICKS-1);
  period = 1.0/cps;
  fltk::remove_timeout(timeout_callback, v);
  fltk::add_timeout(period, timeout_callback, v);
  starttime = fltk::get_time_secs();
  prevticks = ticks;
  timing_period = 1;
}

int main(int argc, char** argv) {

  Window window(300,200);
  window.begin();
  Widget box(0, 0, 300, 100, "fltk::repeat_timeout() test");
  box.labelsize(20); //box.labelfont(HELVETICA_BOLD);
  box.align(ALIGN_WRAP);
  window.resizable(box);
  Slider ticker(10, 100,280,25);
  ticker.maximum(SLIDER_TICKS-1);
  ticker.set_output();
  ValueInput fps(120, 130, 170, 25, "Desired speed:");
  fps.value(24);
  fps.callback(fps_callback, &ticker);
  ValueOutput actual(120, 165, 170, 25, "Actual speed:");
  actual.tooltip("Average speed since you last changed the speed.");
  ::actual = &actual;

  window.show(argc, argv);
  fltk::wait();

  fltk::add_timeout(period, timeout_callback, &ticker);
  starttime = fltk::get_time_secs();

  return fltk::run();
}
