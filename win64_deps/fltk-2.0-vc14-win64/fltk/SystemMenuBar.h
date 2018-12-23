//
// "$Id:$"
//
// MacOS system menu bar header file for the Fast Light Tool Kit (FLTK2).
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
//     http://www.fltk.org/str.php
//

#ifndef fltk_SystemMenuBar_h
#define fltk_SystemMenuBar_h 

#include "MenuBar.h"

struct Fl_Menu_Item;

namespace fltk {

#ifdef __APPLE__

class FL_API SystemMenuBar : public MenuBar {
protected:
  void draw();
public:
  SystemMenuBar(int x,int y,int w,int h,const char *l=0)
      : MenuBar(x,y,w,h,l) {
	   deactivate();			// don't let the old area take events
	  }
  void layout();
};

#else

typedef MenuBar SystemMenuBar;

#endif

}

#endif

//
// End of "$Id:$".
//
