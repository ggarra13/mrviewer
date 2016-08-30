//
// "$Id: Valuator.cxx 8744 2011-05-27 06:31:17Z bgbnbigben $"
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

#include <fltk/Valuator.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/math.h>
#include <stdio.h>
#include <stdlib.h>
using namespace fltk;

/*! \class fltk::Valuator

  Base class for sliders and all other one-value "knobs".The
  fltk::Valuator class controls a single floating-point value and
  provides a consistent interface to set the value(), range(), and
  step(), and insures that callbacks are done the same for every
  object.

  Callbacks are done each time the user changes the value. So if they
  drag a slider, the callback is done each time the slider handle
  moves to a new pixel.

  For most subclasses you can call when() to get some other callback
  behaviors:
  - fltk::WHEN_CHANGED: this is the default, callback is done on
    any change.
  - fltk::WHEN_RELEASE: it will only do the callback when the user
    moves the slider and then lets it go on a different value than it
    started.
  - fltk::WHEN_RELEASE_ALWAYS: will do the callback when
    the user lets go of the slider whether or not the value changed.
  - fltk::WHEN_NEVER: do not do the callback, instead it will turn
    on the changed() flag.

  There are probably more of these classes in FLTK than any others: 

  \image html valuators.gif

  In the above diagram each box surrounds an actual subclass. These
  are further differentiated by setting the type() of the widget to
  the symbolic value labeling the widget. The ones labelled "0" are
  the default versions with a type(0). For consistency the symbol
  fltk::VERTICAL is defined as zero.

*/

/*! The constructor initializes:
  - value(0.0)
  - step(0)
  - range(0,1)
  - linesize(0)
*/
Valuator::Valuator(int X, int Y, int W, int H, const char* L)
  : Widget(X,Y,W,H,L) {
  clear_flag(ALIGN_MASK);
  set_flag(ALIGN_BOTTOM);
  when(WHEN_CHANGED);
  value_ = 0.0;
  step_ = 0;
  minimum_ = 0;
  maximum_ = 1;
  linesize_ = 0;
  previous_value_ = 0;
}

/*! \fn double Valuator::value() const
  Returns the current value.
*/

/*!
  Sets the current value, redrawing the widget if necessary by calling
  value_damage(). <i>The new value is stored unchanged, even if it is
  outside the range or not a multiple of step()</i>.
  Returns true if the new value is different.
*/
bool Valuator::value(double v) {
  clear_changed();
  if (v == value_) return false;
  value_ = v;
  value_damage();
  return true;
}

/*! \fn void Valuator::set_value(double)
  Sets the current value but does not call value_damage().
*/

/*! \fn void Valuator::maximum(double)

  Sets the maximum value for the valuator. For most subclasses the
  user cannot move the value outside the minimum()..maximum() range if
  it starts inside this range.

  These values should be multiples of the step() to avoid ambiguity
  and possible implementation changes.

  For most subclasses, the minimum may be greater than the
  maximum. This has the effect of "reversing" the object so the larger
  values are in the opposite direction. This also switches which end
  of the filled sliders is filled.

  You probably need to redraw() the widget after changing the range.
*/

/*! \fn void Valuator::minimum(double)
  Sets the minimum value for the valuator. See minimum().
*/
 
/*! \fn void Valuator::range(double min, double max)
  Sets both the minimum() and maximum().
*/

/*! \fn void Valuator::step(double)

  Set the step value. As the user moves the mouse the value is
  rounded to a multiple of this. Values that are sufficently close
  to 1/N (where N is an integer) are detected and assummed to be
  exactly 1/N, so step(.00001) will work as wanted.

  If this is zero (the default) then all rounding is disabled. This
  results in the smoothest controller movement but this is not
  recommended if you want to present the resulting numbers to the user
  as text, because they will have useless extra digits of precision.

  For some widgets like Roller this is also the distance the
  value moves when the user drags the mouse 1 pixel. In these cases if
  step() is zero then it acts like it is .01.

  Negative values for step() produce undocumented results. 
*/

/*! \fn void Valuator::linesize(double)
  Set the value returned by linesize(), or restore the default
  behavior by setting this to zero. Values less than zero or
  between zero and the step() value produce undefined results.
*/

/*! Return the value set for linesize(), or the calculated value
  if linesize() is zero.

  The linesize is the amount the valuator moves in response to an
  arrow key, or the user clicking an up/down button, or a click of the
  mouse wheel. If this has not been set, this will return the maximum
  of step() and 1/50 of the range.
 */
double Valuator::linesize() const {
  if (linesize_) return linesize_;
  if (step_ >= 1) return step_;
  double r = fabs(maximum_-minimum_)/50;
  if (r < step_) return step_;
  if (r > 1) return 1;
//   if (step_ > 0) {
//     if (r < step_) r = step_;
//     else r = rint(r/step_)*step_;
//   }
  return r;
}

/*! Subclasses must implement this. It is called whenever the value()
  changes. They must call redraw() if necessary. */
void Valuator::value_damage() {
  redraw(DAMAGE_VALUE); // default version does partial-redraw
}

/*! \fn double Valuator::previous_value() const
  Value saved when handle_push() was last called.
*/

/*! \fn void Valuator::handle_push()
  Subclasses should call this when the user starts to change the value. 
*/

/*!
  Subclasses should call this as the user moves the value. The passed
  number is an arbitrary-precision value you want to set it to, this
  function clamps it to the range (if previous_value() is in range)
  and rounds it to the nearest multiple of step(), and then stores
  it into value(). It then does the callback() if necessary.
*/
void Valuator::handle_drag(double v) {
  // round to nearest multiple of step:
  if (step_ >= 1) {
    v = rint(v/step_)*step_;
  } else if (step_ > 0) {
    // Try to detect fractions like .1 which are actually stored as
    // .9999999 and thus would round to unexpected values. This is done
    // by seeing if 1/N is close to an integer:
    double is = rint(1/step_);
    if (fabs(is*step_-1) < .001) v = rint(v*is)/is;
    else v = rint(v/step_)*step_;
  } else {
    // check for them incrementing toward zero and don't produce tiny
    // numbers:
    if (previous_value_ && fabs(v/previous_value_) < 1e-5) v = 0;
  }
  // If original value was in-range, clamp the new value:
  double A = minimum_; double B = maximum_;
  if (A > B) {A = B; B = minimum_;}
  if (v < A && previous_value_ >= A) v = A;
  else if (v > B && previous_value_ <= B) v = B;
  // store the value, redraw the widget, and do callback:
  if (value(v)) {
    if (when() & WHEN_CHANGED || !pushed()) do_callback();
    else set_changed();
  }
}

/*!  Subclasses should call this when the user stops moving the
  value. It may call the callback. */
void Valuator::handle_release() {
  if (when()&WHEN_RELEASE && !pushed()) {
    // ensure changed() is off even if no callback is done.  It may have
    // been turned on by the drag, and then the slider returned to it's
    // initial position:
    clear_changed();
    // now do the callback only if slider in new position or always is on:
    if (value_ != previous_value_ || when() & WHEN_NOT_CHANGED)
      do_callback();
  }
}

/*!
  Print the current value into the passed buffer as a user-readable
  and editable string. Returns the number of bytes (not counting the
  terminating nul) written to the buffer. Calling code can assumme that
  the written string is never longer than 20 characters.

  This is used by subclasses that let the user edit the value in a
  textfield. Since this is a virtual function, you can override this
  in a subclass of those and change how the numbers are displayed.

  The default version prints enough digits for the current step()
  value. If step() is zero it does a %g format.  If step is an integer
  it does %d format. Otherwise it does a %.nf format where n is enough
  digits to show the step, maximum of 8.
 */
int Valuator::format(char* buffer) {
  double v = value();
  if (!step_) return sprintf(buffer, "%g", v);
  double s = step_-floor(step_);
  if (!s)
    return sprintf(buffer, "%ld", long(v));
  int i=1;
  for (; i < 8; i++) {
    if (s >= .099) break;
    s *= 10;
  }
  return sprintf(buffer, "%.*f", i, v);
}

/*!

  The base class handle() accepts FOCUS and recognizes a number of
  keystrokes that adjust the value. A subclass can call this to get
  these keystrokes, it can also do it's own keystroke handling if it
  wants.

  - DownKey, LeftKey: move 1 linesize() toward minimum()
  - (Ctrl|Shift|Alt|Meta)+DownKey, LeftKey: move 10 linesize() toward minimum()
  - UpKey, RightKey: move 1 linesize() toward maximum()
  - (Ctrl|Shift|Alt|Meta)+UpKey, RightKey: move 10 linesize() toward maximum()
  - HomeKey: set to minimum()
  - EndKey: set to maximum()
  - Mousewheel: For normal valuators, each click is linesize(),
    Style::wheel_scroll_lines is ignored. However Scrollbar does use
    Style::wheel_scroll_lines.
*/
int Valuator::handle(int event) {
  switch(event) {
  case ENTER:
  case LEAVE:
    redraw_highlight();
  case MOVE:
    return 1;
  case FOCUS:
  case UNFOCUS:
    redraw(DAMAGE_HIGHLIGHT);
    return 1;
  case KEY: {
    double i;
    switch (event_key()) {
    case DownKey:
    case LeftKey:
      i = -linesize();
      goto J1;
    case UpKey:
    case RightKey:
      i = linesize();
    J1:
      if (event_state()&(SHIFT|CTRL|ALT)) i *= 10;
      if (maximum() < minimum()) i = -i;
      handle_drag(value()+i);
      return 1;
    case HomeKey:
      handle_drag(minimum());
      return 1;
    case EndKey:
      handle_drag(maximum());
      return 1;
    }
    return 0;
  }
  case MOUSEWHEEL:
    previous_value_ = value();
    handle_drag(value()+(event_dx()-event_dy())*linesize());
    return 1;
  }
  return 0;
}

//
// End of "$Id: Valuator.cxx 8744 2011-05-27 06:31:17Z bgbnbigben $".
//
