//
// "$Id: StyleSet.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/StyleSet.h>
#include <fltk/Widget.h>
#include <string.h>
#include <stdlib.h>
using namespace fltk;

/*! \class fltk::StyleSet

  Fltk can manage "sets" of styles. This is useful for making a
  program that displays more than one "theme" at a time, such as a
  program that edits or creates themes. The "test" widgets can display
  the theme under development, while the main widgets stay
  unchanged. Fluid uses this to allow you to try your program under
  different themes.

  Each widget "belongs" to a set, which is assigned to the current set
  when the widget is constructed.

  The actual implementation is to change the pointers such as
  fltk::Widget::default_style for every widget class to point to a new
  copy, so when the constructors for each widget are run they assign
  the new copy. The previous copies are saved and restored if the
  older set name is used.

  There are some problems. Many widgets are dynamically created (for
  instance pop-up menus) and may not inherit the expected theme unless
  you are careful. Also some global things such as the background
  color cannot be seperated into sets.
*/

static StyleSet* current_set;

extern "C" FL_API bool fltk_theme();

/*! The \e first StyleSet you create is made into the current one and
  is made equal to the current settings of the styles. All subsequent
  StyleSet are set to the Style::revert() version of the current ones,
  and you must call make_current() on them to make them useful.
*/
StyleSet::StyleSet() {
  // The first one constructed becomes the current one:
  if (!current_set) {
    current_set = this;
    return;
  }
  // Otherwise we must init to a blank set:
  fltk::theme(fltk_theme);
  background = 0xc0c0c000; // get_color_index(GRAY75); // 
  // Make all the named styles build copies of themselves and then
  // assign that list to this set:
  NamedStyle* saved = NamedStyle::first;
  NamedStyle::first = 0;
  NamedStyle* l = saved;
  NamedStyle* default_style = Widget::default_style;
  while (l) {
    NamedStyle* newstyle =
      new NamedStyle(l->name,l->revertfunc,l->back_pointer);
    if (l == default_style) default_style = newstyle;
    l = l->next;
  }
  // Update parent entries 
  // (suppose only possible parent is Widget::default_style    
  for (l = NamedStyle::first; l; l = l->next) {
    if (l->parent_) l->parent_ = default_style;
  }
  first_style = NamedStyle::first;
  NamedStyle::first = saved;
}

/*!
  Make this set be current. All widgets created after this use this
  set, and calling fltk::reload_theme() will change the styles
  belonging to this set.
*/
void StyleSet::make_current() {
  if (this == current_set) return;

  // update the fields in the current set so they are saved:
  StyleSet* c = current_set;
  c->theme = fltk::theme();
  c->background = get_color_index(GRAY75);
  c->first_style = NamedStyle::first;

  current_set = this;

  fltk::theme(theme);
  set_background(background);
  NamedStyle *l = NamedStyle::first = first_style;
  while (l) {
    *(l->back_pointer) = l;
    l = l->next;
  }
}

/*! The destructor does nothing. It is not clear what it should do, as
  Widgets may still be using the styles. */
StyleSet::~StyleSet() {
#if 0
  if (current_set == this) {
    current_set = 0;
    return;
  }
  // probably should delete the list of styles? But widgets may be using them?
#endif
}

//
// End of "$Id: StyleSet.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
