//
// "$Id: Clock.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $"
//
// Clock widget for the Fast Light Tool Kit (FLTK).
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

#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Clock.h>
#include <fltk/draw.h>
#include <math.h>
#include <time.h>
#ifndef _WIN32
#  include <sys/time.h>
#endif /* !_WIN32 */
using namespace fltk;

/*! \class fltk::ClockOutput
  Base class of Clock, this one does not move, it just displays whatever
  time  you set into it. The include file is <fltk/Clock.h>

  type() may be set to SQUARE, ROUND, or DIGITAL (nyi).
*/

/*! \class fltk::Clock
  This widget provides a round analog clock display and is provided
  for Forms compatibility. It installs a 1-second timeout callback
  using fltk::add_timeout().

  The color() fills the background. The selectioncolor() (which defaults
  to GRAY85) fills the hands. The textcolor() is used to color in the
  tick marks and outline the hands.

  type() may be set to SQUARE, ROUND, or DIGITAL (nyi).  See the base
  class ClockOutput for some other methods.

  \image html clock.gif

  Design credits:
  Original clock display written by Paul Haeberli at SGI.\n
  Modifications by Mark Overmars for Forms\n
  Further changes by Bill Spitzak for fltk\n
*/

const float hourhand[4][2] = {{-0.5f, 0}, {0, 1.5f}, {0.5f, 0}, {0, -7.0f}};
const float minhand[4][2] = {{-0.5f, 0}, {0, 1.5f}, {0.5f, 0}, {0, -11.5f}};
const float sechand[4][2] = {{-0.1f, 0}, {0, 2.0f}, {0.1f, 0}, {0, -11.5f}};

static void drawhand(float ang,const float v[][2],Color fill,Color line)
{
  push_matrix();
  rotate(ang);
  addvertices(4, v);
  setcolor(fill); fillstrokepath(line);
  pop_matrix();
}

/** Draws the hands on the clock.
    \param fill The fltk::Color used to fill the hands
    \param line The fltk::Color used to draw the hand outlines
*/
void ClockOutput::drawhands(Color fill, Color line) {
  drawhand(-360*(hour()+minute()/60.0f)/12, hourhand, fill, line);
  drawhand(-360*(minute()+second()/60.0f)/60, minhand, fill, line);
  drawhand(-360*(second()/60.0f), sechand, fill, line);
}

static void rect(float x, float y, float w, float h) {
  float v[4][2];
  v[0][0] = v[3][0] = float(x);
  v[1][0] = v[2][0] = float(x+w);
  v[0][1] = v[1][1] = float(y);
  v[2][1] = v[3][1] = float(y+h);
  addvertices(4,v);
  fillpath();
}

/** The draw function
    \param x The x position of the ClockOutput
    \param y The y position of the ClockOutput
    \param w The width of the ClockOutput
    \param h The height of the ClockOutput
*/
void ClockOutput::draw(int x, int y, int w, int h) {
  if (type() == ROUND) {
    addchord(Rectangle(x,y,w,h),0,360);
    setcolor(color());
    fillstrokepath(BLACK);
  }
  push_matrix();
  translate(x+w/2.0f-.5f, y+h/2.0f-.5f);
  scale((w-1)/28.0f, (h-1)/28.0f);
  // draw the shadows:
  push_matrix();
  translate(0.60f, 0.60f);
  Color c = lerp(color(), BLACK, .3f);
  drawhands(c, c);
  pop_matrix();
  // draw the tick marks:
  push_matrix();
  setcolor(textcolor()); // color was 52
  for (int i=0; i<12; i++) {
    if (i==6) ::rect(-0.5, 9, 1, 2);
    else if (i==3 || i==0 || i== 9) ::rect(-0.5, 9.5, 1, 1);
    else ::rect(-0.25, 9.5, .5, 1);
    rotate(-30);
  }
  pop_matrix();
  // draw the hands:
  drawhands(selection_color(), textcolor()); // color was 54
  pop_matrix();
}

/** Draws a background box and then throws to 
    ClockOutput::draw(int, int, int int) */
void ClockOutput::draw() {
  draw_box();
  draw(0, 0, w(), h());
}

/*! \fn int ClockOutput::hour() const
  \return The hour sent to the last call to value(). */
/*! \fn int ClockOutput::minute() const
  \return The minute sent to the last call to value(). */
/*! \fn int ClockOutput::second() const
  \return The second sent to the last call to value(). */

/*! \fn unsigned long ClockOutput::value() const
  \return the last Unix timestamp the clock was set to.
*/

/*! Set the hour, minute, and second to display. The hour is effectively
  taken modulo 12 and the minute and second modulo 60 to figure out
  where to place the hands. Redraw happens only if different.

  This does not set the unsigned long value() member, as it can't
  because it does not know the date.
  \param h The hour to draw the hour hand pointing to
  \param m The minute to draw the minute hand pointing to
  \param s The seconds to draw the second hand pointing to
*/
void ClockOutput::value(int h, int m, int s) {
  if (h!=hour_ || m!=minute_ || s!=second_) {
    hour_ = h; minute_ = m; second_ = s;
    redraw(DAMAGE_CHILD);
  }
}

/*! Set the clock to a Unix timestamp. The value is passed through
  the localtime() library function and used to get the hour, minute,
  and second. 
  \param v The unix timestamp used to calculate the time
*/
void ClockOutput::value(unsigned long v) {
  value_ = v;
  const time_t vv = (time_t)v;
  struct tm *timeofday = localtime(&vv);
  value(timeofday->tm_hour, timeofday->tm_min, timeofday->tm_sec);
}

////////////////////////////////////////////////////////////////

/** The default constructor handballs to ClockOutput()
    \param x The x position of the Clock, relative to the Group
    \param y The y position of the Clock, relative to the Group 
    \param w The width of the Clock
    \param h The height of the Clock
    \param l The label of the Clock
*/
Clock::Clock(int x, int y, int w, int h, const char *l)
  : ClockOutput(x, y, w, h, l) {}

/** The Clock's handle routine
    This only reads SHOW, HIDE and TIMEOUT events, otherwise
    it throws back to ClockOutput::handle
    \param event The event to handle
    \return 1 if the event was successfully handled,\n 0 otherwise
*/
int Clock::handle(int event) {
  switch (event) {
  case SHOW:
    remove_timeout();
  case TIMEOUT:
#ifdef _WIN32
    value((unsigned long) time(0));
    repeat_timeout(1.0f);
#else
    {struct timeval t; gettimeofday(&t, 0);
    value(t.tv_sec);
    float delay = 1.0f-float(t.tv_usec)*.000001f;
    if (delay < .1f || delay > .9f) delay = 1.0f;
    add_timeout(delay);}
#endif
    break;
  case HIDE:
    remove_timeout();
    break;
  }
  return ClockOutput::handle(event);
}

static void revert(Style* s) {
  s->selection_color_ = GRAY85;
  //s->buttoncolor_ = GRAY33;
  //s->textcolor_ = BLACK;
}
static NamedStyle style("Clock", revert, &Clock::default_style);
/** The default_style for Clock reverts selection_color_ to GRAY85, which
    is the colour used to fill the hands. Everything else is inherited */
NamedStyle* Clock::default_style = &::style;

/** The default constructor initialised the default_style based on Clock::default_style, sets ALIGN_BOTTOM and then sets all time to 0 
    \param x The x position of the ClockOutput, relative to the Group
    \param y The y position of the ClockOutput, relative to the Group
    \param w The width of the ClockOutput
    \param h The height of the ClockOutput
    \param l The label of the ClockOutput*/
ClockOutput::ClockOutput(int x, int y, int w, int h, const char *l)
: Widget(x, y, w, h, l) {
  style(Clock::default_style);
  clear_flag(ALIGN_MASK);
  set_flag(ALIGN_BOTTOM);
  hour_ = 0;
  minute_ = 0;
  second_ = 0;
  value_ = 0;
}

//
// End of "$Id: Clock.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
