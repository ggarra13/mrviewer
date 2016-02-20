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

#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Window.H>

#include "core/mrvRectangle.h"

#include "mrvCollapsableGroup.h"

namespace mrv {

  void CollapsableGroup::toggle_tab( Fl_Button* w, void* data )
  {
    mrv::CollapsableGroup* g = (mrv::CollapsableGroup*) data;
    Fl_Pack* contents = g->contents();
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
    w->redraw();
    w->window()->redraw();
//     g->parent()->redraw();

    free( label );
  }

  CollapsableGroup::CollapsableGroup( const int x, const int y, 
				      const int w, const int h, 
				      const char* l ) :
  Fl_Group( x, y, w, h )
  {
    box( FL_ROUNDED_BOX );

    begin();

    // @todo: fltk1.3
    // fltk::Rectangle r( w, h );
    mrv::Recti r( w, h );
    // box()->inset(r);

    _button = new Fl_Button( r.x(), r.y(), r.w(), 20 );
    _button->align( FL_ALIGN_LEFT| FL_ALIGN_INSIDE | FL_ALIGN_TOP );
    _button->labelsize( 16 );
    _button->box( FL_NO_BOX );

    char buf[256];
    sprintf( buf, "  @Cb55@-22>@n %s", l );

    _button->copy_label( buf );

    // @todo: fltk1.3
    // _button->box()->inset(r);

    _contents = new Fl_Pack( r.x(), r.y() + 20, r.w(), 20 );

//     resizable( _contents );  // this breaks stuff

    resizable( this );
  
    end();

    _button->callback( (Fl_Callback*)toggle_tab, this );
  }

  CollapsableGroup::~CollapsableGroup()
  {
      _contents->clear();
      delete _contents;
      delete _button;
  }

  void CollapsableGroup::spacing( int x )
  {
    _contents->spacing( x );
    _contents->resize( 0, 0, _contents->w(), x + child(0)->h() );
  }

  void CollapsableGroup::clear()
  {
    _contents->clear();
    redraw();
  }

  void CollapsableGroup::remove_all()
  {
    _contents->clear();
    redraw();
  }

  void CollapsableGroup::draw()
  {
    // Group::layout();

    // fltk::Rectangle r( w(), h() );
    // box()->inset(r);

    int H = 2;
    if ( _contents->visible() ) H += _contents->h() + 12;

    resize( 0, 0, w(), H + child(0)->h() );

    init_sizes();

    Fl_Group::draw();
  }

  void CollapsableGroup::add( Fl_Widget* w )
  {
    _contents->add( w );
    _contents->redraw();
  }


} // namespace mrv
