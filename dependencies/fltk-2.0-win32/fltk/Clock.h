//
// "$Id: Clock.h 8752 2011-05-29 08:44:01Z bgbnbigben $"
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

#ifndef fltk_Clock_h
#define fltk_Clock_h

#include "Widget.h"

namespace fltk {

/** A ClockOutput can be used to display a program-supplied time */

class FL_API ClockOutput : public Widget {
public:
  /** The type of clock */
  enum {
    SQUARE = 0, //!< A square looking clock. Currently the same as ANALOG
    ANALOG = 0, //!< An old-school analog clock
    ROUND,  //!< A round clock
    DIGITAL //!< A digital clock. Apparently NYI. \todo ...
  };
  ClockOutput(int x,int y,int w,int h, const char *l = 0);

  void value(unsigned long v);	// set to this Unix time
  void value(int,int,int);	// set hour, minute, second
  unsigned long value() const {return value_;}
  int hour() const {return hour_;}
  int minute() const {return minute_;}
  int second() const {return second_;}
protected:
  void draw(int, int, int, int);
  void draw();
private:
  int hour_, //!< The current hour in the ClockOutput
      minute_, //!< The current minute in the ClockOutput
      second_; //!< The current second in the ClockOutput
  unsigned long value_; //!< The current unix time, passed through value()
  void drawhands(Color, Color); // part of draw
};

// a Clock displays the current time always by using a timeout:

class FL_API Clock : public ClockOutput {
public:
  int handle(int);
  /** This looks NYI and, really, pretty useless. It's going to be removed
      from the API unless I can find a \b really good reason to keep it */
  void update();
  Clock(int x,int y,int w,int h, const char *l = 0);
  static NamedStyle* default_style;
};

}

#endif

//
// End of "$Id: Clock.h 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
