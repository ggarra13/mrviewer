/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvCollapsableGroup.h
 * @author gga
 * @date   Tue Aug  7 04:48:52 2007
 * 
 * @brief  A group widget that has a button allowing it to collapse
 *         the contents.
 * 
 * 
 */

#ifndef mrvCollapsableGroup_h
#define mrvCollapsableGroup_h


#include <fltk/PackedGroup.h>

namespace fltk {
  class Button;
}


namespace mrv {

  class CollapsableGroup : public fltk::Group
  {
  public:
    CollapsableGroup( const int x, const int y, const int w, 
		      const int h, const char* l = 0 );
    ~CollapsableGroup();

    void add( fltk::Widget* w );

    void remove_all();

    void spacing( int x );

    fltk::PackedGroup* contents() { return _contents; }

    virtual void layout();

  protected:
    fltk::PackedGroup* _contents;

    static void toggle_tab( fltk::Button* w, void* data );
  };


} // namespace mrv


#endif // mrvCollapsableGroup_h
