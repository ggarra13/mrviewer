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
 * @file   mrvBrowser.cpp
 * @author gga
 * @date   Wed Jan 31 14:28:24 2007
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>

#include <fltk/Cursor.h>
#include <fltk/Box.h>

#include <fltk/damage.h>
#include <fltk/draw.h>
#include <fltk/events.h>

#include "mrvBrowser.h"


namespace mrv
{

  Browser::Browser( int x, int y, int w, int h, const char* l ) :
  fltk::Browser( x, y, w, h, l ),
  _column_separator_color( fltk::Color(fltk::BLACK) ),
  _last_cursor( fltk::CURSOR_DEFAULT ),
  _column_separator( true ),
  _dragging ( false ),
  _auto_resize( false )
{
}

// CHANGE CURSOR
//     Does nothing if cursor already set to value specified.
//
void Browser::change_cursor(fltk::Cursor* newcursor) {
  if ( newcursor != _last_cursor ) {
    cursor(newcursor);
    _last_cursor = newcursor;
  }
}
// RETURN THE COLUMN MOUSE IS 'NEAR'
//     Returns -1 if none.
//
int Browser::which_col_near_mouse() {
  //     int X,Y,W,H;
  //     fltk::Rectangle r( X, Y, W, H );
  //     box->inset(r);	// area inside browser's box()
  // EVENT NOT INSIDE BROWSER AREA? (eg. on a scrollbar)
  if ( ! fltk::event_inside(*this) ) {
    return -1;
  }

  const int* widths = column_widths();
  if (!widths) return -1;

  int mousex = fltk::event_x() + xposition();
  int colx = 0;
  for ( int t=0; widths[t]; t++ ) {
    colx += widths[t];
    int diff = mousex - colx;
    // MOUSE 'NEAR' A COLUMN?
    //     Return column #
    //
    if ( diff >= -4 && diff <= 4 ) {
      return(t);
    }
  }
  return(-1);
}

// MANAGE EVENTS TO HANDLE COLUMN RESIZING
int Browser::handle(int e)
{
  // Not showing column separators? Use default fltk::Browser::handle() logic
  if ( ! column_separator() ) return(fltk::Browser::handle(e));
  // Handle column resizing
  int ret = 0;
  switch ( e ) {
  case fltk::ENTER: {
    ret = 1;
    break;
  }
  case fltk::MOVE: {
    if ( which_col_near_mouse() >= 0 ) {
      change_cursor(fltk::CURSOR_WE);
    } else {
      change_cursor(fltk::CURSOR_DEFAULT);
    }
    ret = 1;
    break;
  }
  case fltk::PUSH: {
    int whichcol = which_col_near_mouse();
    if ( whichcol >= 0 ) {
      // CLICKED ON RESIZER? START DRAGGING
      ret = 1;
      _dragging = true;
      _dragcol = whichcol + 1;
      change_cursor(fltk::CURSOR_DEFAULT);
    }
    break;
  }
  case fltk::DRAG: {
    if ( _dragging ) {
      // Sum up column widths to determine position
      int mousex = fltk::event_x() + xposition();
      int newwidth = mousex - x();

      if ( newwidth > 0 ) {
	set_column_start( _dragcol, newwidth );
	relayout();
	redraw();
      }
    }
    break;
  }
  case fltk::LEAVE:
  case fltk::RELEASE: {
    _dragging = false;				// disable drag mode
    change_cursor(fltk::CURSOR_DEFAULT);	// ensure normal cursor
    ret = 1;
    break;

  }
  }
  if ( _dragging ) return(1);	// dragging? don't pass event to fltk::Browser
  return(fltk::Browser::handle(e) ? 1 : ret);
}

void Browser::layout() 
{
  int nchildren = children();


  if ( _auto_resize )
    {
      int hh = 24;
      int nchildren = children();
      for ( int i = 0; i < nchildren; ++i )
	{
	  fltk::Widget* c = child(i);
	  c->layout();
	  hh += c->h();
	}

      h( hh );

    }


  fltk::Browser::layout();

  int* widths = (int*) column_widths();
  if (!widths) return;

  // If widget is set to resizable, resize all columns equally,
  // unless one column width is set to -1
  if ( resizable() == this )
    {
      int sum = 0;
      int*  c;
      for ( c = widths; *c != 0; ++c )
	{
	  if ( *c < 0 ) { sum = 0; break; }
	  sum += *c;
	}
      
      if ( sum > 0 )
	{
	  for ( c = widths; *c != 0; ++c )
	    {
	      int W = int( w() * ( (float) *c / (float) sum ) );
	      *c = W;
	    }
	}
    }


  for ( int i = 0; i < nchildren; ++i )
    {
      fltk::Widget* c = child(i);
      if ( ! c->is_group() ) continue;

      fltk::Group* g  = (fltk::Group*) c;

      int columns = g->children();
      int x = 0;
      for ( int j = 0; j < columns; ++j )
	{
	  c = g->child(j);
	  int W = widths[j];
	  if ( W == -1 ) W = w() - x;
	  c->resize( x, c->y(), W, c->h() );
	  x += W;
	}
    }

}

void Browser::draw() {
  // DRAW BROWSER
  fltk::Browser::draw();
  if (!column_widths()) return;

  if ( column_separator() ) {
    // DRAW COLUMN SEPARATORS
    Rectangle r( w(), h() );
    box()->inset(r);
    int X = r.x();
    int Y = r.y();
    int W = r.w();
    int H = r.h();
    int colx = X - xposition();
    fltk::setcolor( column_separator_color() );
    const int* widths = column_widths();
    for ( int t=0; widths[t]; t++ ) {
      colx += widths[t];
      if ( colx > X && colx < (X+W) ) {
	fltk::drawline(colx, Y, colx, Y+H-1);
      }
    }
  }
}

  int Browser::absolute_item_index( const fltk::Group* g )
  {
    int idx = 1;
    int num = g->children();
    for (int i = 0; i < num; ++i)
      {
	fltk::Widget* c = g->child(i);
	if ( c->is_group() )
	  idx += absolute_item_index( (fltk::Group*) c );
	else
	  ++idx;
      }
    return idx;
  }

  int Browser::absolute_item_index( bool& found,
				    const fltk::Widget* item,
				    const fltk::Widget* w )
  {

    if ( w == item  ) {
      found = true;
      return 0;
    }

    if ( !w->is_group() ) return 1;

    int idx = 1;
    fltk::Group* g = (fltk::Group*) w;
    int num = g->children();
    
    for ( int i = 0; i < num; ++i )
      {
	fltk::Widget* c = g->child(i);
	idx += absolute_item_index( found, item, c );
	if ( found ) break;
      }

    return idx;
  }

  int Browser::absolute_item_index()
  {
    int main_idx = value();
    if ( main_idx < 0 ) return main_idx;

    int idx = 0;
    for (int i = 0; i < main_idx; ++i)
      {
	fltk::Widget* c = child(i);

	if ( c->is_group() )
	  idx += absolute_item_index( (fltk::Group*)c );
	else
	  ++idx;
      }

    fltk::Widget* sel = item();
    bool found = false;
    idx += absolute_item_index( found, sel, child(main_idx) );

    return idx;
  }

} // namespace mrv

