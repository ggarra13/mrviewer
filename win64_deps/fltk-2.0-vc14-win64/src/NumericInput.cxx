//
// "$Id: NumericInput.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Copyright 2002 by Bill Spitzak, Digital Domain, and others.
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

#include <fltk/events.h>
#include <fltk/NumericInput.h>
#include <fltk/math.h>
#include <fltk/string.h>

using namespace fltk;

/*! \class fltk::NumericInput

  The fltk::NumericInput class is a subclass of fltk::Input that
  redefines the up and down arrow keys to increment and decrement the
  digit of the number to the right of the cursor. This makes it very
  easy to edit numbers.

  If you change when() to fltk::WHEN_ENTER_KEY the callback is only
  done when the user hits the up/down arrow keys or when the user
  types the Enter key. This may be more useful than the default
  setting of fltk::WHEN_CHANGED which can make the callback happen
  when partially-edited numbers are in the field.

  This version lets the user type any text into the field. This is
  useful if you run the text through an expression parser so the user
  can type in math expressions. However if you want to limit the input
  to text that can be run through strtol() or strtod() you should use
  the subclasses fltk::IntInput or fltk::FloatInput.

  When you construct the widget the text starts out blank. You may want
  to set it with value(0) or something.

  <i>This user interface design is Copyright (C) 2000-2003 Digital Domain.
  Patent Pending!

  License is ONLY granted to implement this algorithim as GPL, LGPL, or
  FLTK licensed code. Note that by using fltk you may use this in a
  closed-source program.

  License to use this technology in other cases may be available:<br>
  D2 Software / Digital Domain<br>
  300 Rose Avenue, Venice, CA 90291, USA<br>
  (310)314-2800 FAX(310)314-2888</i>
*/

/*!  Does a %%g sprintf of the value and uses the result to set the
  string value. Notice that there is no inverse function, you will
  have to call strtod(widget->value(),0,0) yourself.  */
void NumericInput::value(double A) {
  char buf[32];
  if (int(A)==A) {
    snprintf(buf, 32, "%d", int(A));
  } else {
    // this is a very brute force way to allow 6 digits to the right instead
    // of the %g default of 4:
    int n = (int)ceil(log10(fabs(A)));
    if (n>0 || n<-6) {
      snprintf(buf, 32, "%g", A);
    } else {
      snprintf(buf, 32, "%.7f", A);
      // strip trailing 0's and the period:
      char *s = buf; while (*s) s++; s--;
      while (s>buf && *s=='0') s--;
      if (*s=='.') s--;
      s[1] = 0;
    }
  }
  Input::text(buf);
}

/*!  Does a %%d sprintf of the value and uses the result to set the
  string value. Notice that there is no inverse function, you will
  have to call strtol(widget->text(),0,0) yourself.  */
void NumericInput::value(int v) {
  char buf[100];
  Input::text(buf, sprintf(buf, "%d", v));
}

static int clickmouse;
static bool dragvalue;

/*! Does the up/down arrows and mouse wheel. Alt+drag also adjusts the
  value. All other points are sent to the Input base class. */
int NumericInput::handle(int event) {
  switch (event) {
  case KEY:
    switch (event_key()) {
    case UpKey:
      return handle_arrow(1);
    case DownKey:
      return handle_arrow(-1);
    }
    break;
  case MOUSEWHEEL:
    if (!focused()) return 0;
    return handle_arrow(-event_dy());
  case PUSH:
    if (event_state(ALT|META)) {
      dragvalue=true;
      clickmouse = event_x();
    } else {
      dragvalue = false; // in case it got stuck
    }
    break;
  case DRAG:
    if (dragvalue) {
      int dx = (event_x()-clickmouse)/5;
      if (dx<=-1 || dx>=1) {
	clickmouse = event_x();
	return handle_arrow(dx);
      }
      return 1;
    }
    break;
  case RELEASE:
    if (dragvalue) {dragvalue = false; return false;}
    break;
  }
  return Input::handle(event);
}

/*!  Do the work of up-arrow (if direction is greater than zero) or
  down-arrow if direction is less than zero. This protected method is
  available so subclasses can change what keys do this, or invert the
  direction of the arrows.
*/
int NumericInput::handle_arrow(int dir)
{
  int save_when = when(); when(0);

  // locate the character to change:
  int p; char c;
  int q = position();
  if (mark() > q) q = mark() - 1;
  else if (mark() < q) q = q - 1;

  // make insertion after decimal point work:
  if (at(q) == '.') q++;

  // add trailing ".0" if cursor is on end of number:
  if (at(q) < '0' || at(q) > '9') {
    for (int g = q-1;;g--) {
      // search to see if decimal point is already there:
      if (g >= 0 && at(g) == '.') break;
      if (g < 0 || at(g) < '0' || at(g) > '9') {
	// if no digits before cursor, assumme we are not pointing at a number:
	if (g >= q-1) goto DONE;
	// if it does not like period edit the last digit instead:
	if (!replace(q,q,'.')) {q--; goto INT;}
	q++;
	break;
      }
    }
    replace(q,q,'0');
  }
 INT:

  // if it's a negative number we reverse direction:
  for (p = q-1; p >= 0; p--) {
    c = at(p);
    if (c == '-') {dir = -dir; break;}
    if (c != '.' && (c < '0' || c > '9')) break;
  }

  if (dir > 0) {
  UP_CASE:
    // up to a larger absolute value, which is much simpler:
    for (p = q; p >= 0; p--) {
      c = at(p);
      if (c == '.') continue;
      if (c < '0' || c > '9') break;
      if (c < '9') {
	replace(p, p+1, c+1);
	goto DONE;
      }
      replace(p, p+1, '0');
    }
    replace(p+1, p+1, '1');
    q++;

  } else {
    // down to a smaller absolute value:
    // first check if all the digits are zero, if so we reverse the sign:
    for (p = q; ; p--) {
      if (p < 0 || ((at(p) < '0' || at(p) > '9') && at(p) != '.')) {
	if (p >= 0 && at(p) == '-') {
	  Input::replace(p, p+1, 0,0);
	  q--;
	} else {
	  replace(p+1, p+1, '-');
	  q++;
	}
	goto UP_CASE;
      }
      if (at(p) != '.' && at(p) != '0') break;
    }

    for (p = q; p >= 0; p--) {
      c = at(p);
      if (c == '.') continue;
      if (c < '0' || c > '9') break;
      if (c == '1') {
	// delete leading zeros:
	int g = p;
	while (g > 0 && at(g-1)=='0') g--;
	if (!(g > 0 && ((at(g-1)>='0' && at(g-1)<='9') || at(g-1)=='.'))) {
	  if (p < q) {
	    Input::replace(g, p+1, 0, 0);
	    q -= p-g+1;
	    goto DONE;
	  }
	  Input::replace(g, p, 0, 0);
	  q -= p-g;
	  p = g;
	}
      }
      if (c > '0') {
	replace(p, p+1, c-1);
	goto DONE;
      }
      replace(p, p+1, '9');
    }
  }
 DONE:
  position(q, q+1);
  when(save_when);
  if (save_when&(WHEN_CHANGED|WHEN_ENTER_KEY) && changed()) {
    clear_changed(); do_callback();
  }
  return 1;
}
// End of "$Id: NumericInput.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
