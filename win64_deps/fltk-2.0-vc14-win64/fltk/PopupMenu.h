//
// "$Id: PopupMenu.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Setting the type to POPUP* will make an invisible widget that can
// overlap any other widgets and provides a popup menu. The default
// type gives you something similar to a Choice except it does not
// display the current value.
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

#ifndef fltk_PopupMenu_h
#define fltk_PopupMenu_h

#include "Menu.h"

namespace fltk {

class FL_API PopupMenu : public Menu {
public:
  // values for type:
  enum {NORMAL = GROUP_TYPE, POPUP1, POPUP2, POPUP12,
	POPUP3, POPUP13, POPUP23, POPUP123};
  int handle(int);
  int popup();
  PopupMenu(int,int,int,int,const char * =0);
  static NamedStyle* default_style;

protected:
  void draw();

};

}
#endif

//
// End of "$Id: PopupMenu.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
