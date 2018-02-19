//
// "$Id: ItemGroup.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Widget designed to be a nested list in a menu or browser. This
// copies the drawing and style code from Item. I did not modify the
// base Menu class this way because the style inheritance would mess
// up the styles of MenuButton and MenuBar. Code is in Item.cxx
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

#ifndef fltk_ItemGroup_h
#define fltk_ItemGroup_h

#include "Menu.h"

namespace fltk {

class FL_API ItemGroup : public Menu {
public:
  void draw();
  void layout();
  int handle(int);
  ItemGroup(const char* label = 0, bool begin=false);
  ItemGroup(const char* label, const Symbol*, bool begin=false);
};

/** \class MenuSection
  This class will elegantly facilitate dynamic (& hand-made) menu code writing
  by creating and calling begin() on an ItemGroup in the constructor and
  calling end() in the destructor:
  \code
  mymenu->begin();
  new Item("in main menu");
  {MenuSection g("submenu title");
    new Item("in submenu");
    new Item("also in submenu");
  } // destructor ends the submenu
  \endcode
*/
class FL_API MenuSection {
  ItemGroup* group_;
public:
  //! build a typical submenu group section, then call begin()
  MenuSection(const char* label = 0) { group_ = new ItemGroup(label,true); }

  MenuSection(const char* label, const Symbol* i) { group_ = new ItemGroup(label,i,true); }

  //! call end() at destruction
  ~MenuSection() {group_->end();}

  //! The ItemGroup created by this.
  ItemGroup* group() const {return group_;}
};

}

#endif
