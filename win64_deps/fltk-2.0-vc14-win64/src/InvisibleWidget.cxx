//
// "$Id: InvisibleWidget.cxx 5698 2007-02-19 05:40:36Z spitzak $"
//

/*! \class fltk::InvisibleBox

  This Widget is invisible due to not having a box(). The
  label still prints so it can be used to position labels. Also
  this is useful as a Group::resizable() widget.
*/

#include <fltk/InvisibleBox.h>
#include <fltk/events.h>
using namespace fltk;

static void revert(Style* s) {
  s->color_ = GRAY75;
  s->box_ = NO_BOX;
}
// this is unnamed as there is no need for themes to alter this:
static NamedStyle style(0, revert, &InvisibleBox::default_style);
/*! Sets box() to \c fltk::NO_BOX, sets color() to \c fltk::GRAY75 for
  compatability with fltk1's Fl_Box widget. */
NamedStyle* InvisibleBox::default_style = &::style;

InvisibleBox::InvisibleBox(int x, int y, int w, int h, const char *l)
  : Widget(x,y,w,h,l)
{
  style(default_style);
}

/*! This constructor is for compatability with the fltk1 Fl_Box widget,
  and sets box() to \a b. */
InvisibleBox::InvisibleBox(Box* b, int x, int y, int w, int h, const char *l)
  : Widget(x,y,w,h,l)
{
  style(default_style);
  box(b);
}

/*! All special-casing for NO_BOX has been moved to the base Widget::draw()
  so this just calls that. */
void InvisibleBox::draw() {
  Widget::draw();
}

/*! Handles enter/exit only if a tooltip is set. Otherwise it is assummed
  this is used for resizable and otherwise has no effect, so all e
*/
int InvisibleBox::handle(int event) {
  if (box()==NO_BOX) {
    if (event == ENTER && tooltip()) return 1;
    return 0;
  } else {
    return Widget::handle(event);
  }
}

//
// End of "$Id: InvisibleWidget.cxx 5698 2007-02-19 05:40:36Z spitzak $".
//
