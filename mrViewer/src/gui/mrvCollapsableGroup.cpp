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
 * @file   mrvCollapsableGroup.cpp
 * @author gga
 * @date   Tue Aug  7 04:52:02 2007
 * 
 * @brief  
 * 
 * 
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <iostream>
using namespace std;

#include <fltk/Button.h>
#include <fltk/PackedGroup.h>
#include <fltk/Box.h>
#include <fltk/Window.h>


#include "mrvCollapsableGroup.h"

namespace mrv {

  void CollapsableGroup::toggle_tab( fltk::Button* w, void* data )
  {
    mrv::CollapsableGroup* g = (mrv::CollapsableGroup*) data;
    fltk::PackedGroup* contents = g->contents();
    char* label = strdup( w->label() );
    using namespace std;
    if ( contents->visible() )
      {
	label[10] = '0';
	contents->hide();
      }
    else
      {
	label[10] = '2';
	contents->show();
      }
    w->copy_label( label );
    w->relayout();
    w->window()->redraw();
//     g->parent()->redraw();

    free( label );
  }

  CollapsableGroup::CollapsableGroup( const int x, const int y, 
				      const int w, const int h, 
				      const char* l ) :
    fltk::Group( x, y, w, h )
  {
    box( fltk::ROUNDED_BOX );

    begin();

    fltk::Rectangle r( w, h );
    box()->inset(r);

    fltk::Button* button = new fltk::Button( r.x(), r.y(), r.w(), 20 );
    button->align( fltk::ALIGN_LEFT|fltk::ALIGN_INSIDE|fltk::ALIGN_TOP );
    button->labelsize( 16 );
    button->box( fltk::NO_BOX );

    char buf[256];
    sprintf( buf, "  @Cb55@-22>@n %s", l );

    button->copy_label( buf );

    button->box()->inset(r);

    _contents = new fltk::PackedGroup( r.x(), r.y() + 20, r.w(), 20 );

//     resizable( _contents );  // this breaks stuff

    resizable( this );
  
    end();

    button->callback( (fltk::Callback*)toggle_tab, this );
  }

  CollapsableGroup::~CollapsableGroup()
  {
  }

  void CollapsableGroup::spacing( int x )
  {
    _contents->spacing( x );
    _contents->h( x + child(0)->h() );
  }

  void CollapsableGroup::remove_all()
  {
    _contents->remove_all();
    relayout();
  }

  void CollapsableGroup::layout()
  {
    Group::layout();

    fltk::Rectangle r( w(), h() );
    box()->inset(r);

    int H = 2;
    if ( _contents->visible() ) H += _contents->h() + 12;

    h( H + child(0)->h() );

    init_sizes();
  }

  void CollapsableGroup::add( fltk::Widget* w )
  {
    _contents->add( w );
    _contents->relayout();
  }


} // namespace mrv
