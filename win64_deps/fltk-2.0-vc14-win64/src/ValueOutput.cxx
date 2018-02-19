//
// "$Id: ValueOutput.cxx 4886 2006-03-30 09:55:32Z fabien $"
//
// Copyright 1998-2006 by Bill Spitzak and others.

#include <fltk/ValueOutput.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/draw.h>
#include <fltk/Box.h>
#include <fltk/run.h>
using namespace fltk;

/*! \class fltk::ValueOutput

  A valuator that displays the number like a text box. This is
  indended for showing the user a number, there is no way for the user
  to change the number. It is much lighter weight than using an Output
  widget for this.

  There is no way for the user to change the number, but calling value()
  will change it.
*/

// WAS: I put in some code so that the user can copy the data, by clicking
// on the field. However this is all commented out because the result
// was confusing because it moved the focus to the widget, yet most
// keystrokes do not do anything.

void ValueOutput::draw() {
  drawstyle(style(),flags());
  if (damage() & DAMAGE_ALL) draw_frame();
  Rectangle r(w(),h()); box()->inset(r);
  push_clip(r);
  char buf[40];
  format(buf);
  if (!(damage() & DAMAGE_ALL)) {
    Color fg = getcolor();
    setcolor(getbgcolor());
    fillrect(r);
    setcolor(fg);
  }
//    if (focused()) {
//      setcolor(selection_color());
//	Rectangle ir(r,r.w()-4,textsize()+2);
//      fillrect(ir);
//	setcolor(selection_textcolor());
//    } else
  drawtext(buf, float(r.x()+3), float(r.y()+(int(r.h()+getascent()-getdescent()) >> 1)));
  pop_clip();
}

int ValueOutput::handle(int event) {
  switch (event) {
  case FOCUS:
    return false;
//    case PUSH:
//      take_focus();
//      // copy it to selection for middle-mouse paste:
//      {char buf[40]; fltk::copy(buf, format(buf), false);}
//      return true;
//    case DRAG:
//    case RELEASE:
//      // maybe it should do DnD?
//      return true;
//    case KEY:
//      if (event_state(CTRL) && (event_key() == 'x' || event_key() == 'c')) {
//        // copy it to clipboard for normal paste:
//        {char buf[40]; fltk::copy(buf, format(buf), true);}
//        return true;
//      } else return false;
  default:
    return Valuator::handle(event);
  }
}
