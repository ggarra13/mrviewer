//
// "$Id: WizardGroup.h 4288 2005-04-16 00:13:17Z mike $"
//
// WizardGroup widget definitions.
//
// Copyright 1999-2006 by Easy Software Products and others.
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

//
// Include necessary header files...
//

#ifndef _fltk_WizardGroup_h_
#define _fltk_WizardGroup_h_

#include "Group.h"

namespace fltk {

class FL_API WizardGroup : public Group {
  Widget * value_;
public:
  WizardGroup(int x,int y,int w,int h, const char * l = 0, bool begin=false)
    : Group(x,y,w,h,l,begin), value_(0) {  box(THIN_UP_BOX);}

  void          draw(); 
  void		next();
  void		prev();
  void		value(Widget *);
  //! return  the current visible child.
  Widget *      value() const { return value_;}
};

}

#endif

//
// End of "$Id: WizardGroup.h 4288 2005-04-16 00:13:17Z mike $".
//
