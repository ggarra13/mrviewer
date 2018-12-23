//
// "$Id: Adjuster.h 8752 2011-05-29 08:44:01Z bgbnbigben $"
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


#ifndef fltk_Adjuster_h
#define fltk_Adjuster_h

#include "Valuator.h"

namespace fltk {
/** This valuator is provided for back-compatability ONLY.
  This may be removed before the final version!
  3-button "slider", made for Nuke
*/
class FL_API Adjuster : public Valuator {
public:
  Adjuster(int x, int y, int w, int h, const char *l=0);
  static NamedStyle* default_style;
  void soft(int x) {soft_ = x;}
  int soft() const {return soft_;}
  int handle(int);

protected:
  void draw();
  void value_damage();

private:
  int drag, highlight, last;
  int ix;
  int soft_;
};

}
#endif

//
// End of "$Id: Adjuster.h 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
