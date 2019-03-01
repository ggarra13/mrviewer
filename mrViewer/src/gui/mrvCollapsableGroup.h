
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

#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>

namespace mrv {

  class CollapsableGroup : public Fl_Group {
    public:
      CollapsableGroup( const int x, const int y, const int w, 
                        const int h, const char* l = 0 );
      ~CollapsableGroup();
      void begin() { _contents->begin(); }
      void end()   { _contents->end(); }
      void add( Fl_Widget* w );
      void clear();
      void spacing( int x );
      void resize(int X,int Y,int W,int H);
      Fl_Pack* contents() { return _contents; }

    protected:
      Fl_Button*  _button;
      Fl_Pack*    _contents;

      static void toggle_tab_cb( Fl_Button* w, void* data );
      void        toggle_tab( Fl_Button* w);
      void layout();
      void relabel_button();
      //DEBUG virtual void draw();
  };

} // namespace mrv

#endif // mrvCollapsableGroup_h
