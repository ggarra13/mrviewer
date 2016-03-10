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

// #include <FL/Fl_Cursor.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include "gui/mrvIO.h"
#include "mrvBrowser.h"

namespace {
static const char* kModule = "mrv::Browser";
}

namespace mrv
{

  Browser::Browser( int x, int y, int w, int h, const char* l ) :
  Fl_Tree( x, y, w, h, l ),
  _value( -1 ),
  _column_widths( NULL ),
  _column_headers( NULL ),
  _column_separator_color( FL_BLACK ),
  _last_cursor( FL_CURSOR_DEFAULT ),
  _column_separator( true ),
  _dragging ( false ),
  _auto_resize( false )
{
    showroot( 0 );
    item_draw_mode(FL_TREE_ITEM_HEIGHT_FROM_WIDGET);
}


void Browser::insert( Fl_Widget& w, int idx )
{
    if (idx < 0 )
    {
        LOG_ERROR( "Index " << idx << " is invalid" );
        return;
    }

    Fl_Tree_Item* item = Fl_Tree::insert( root(), w.label(), idx );
    item->widget(&w);
}

void Browser::replace( int idx, Fl_Widget& w )
{
    if (idx < 0 )
    {
        LOG_ERROR( "Index " << idx << " is invalid" );
        return;
    }

    Fl_Tree_Item* item = NULL;
    int i;
    bool found = false;
    for ( item = first(); item; item = next(item), ++i )
    {
        if ( i == idx ) {
            found = true;
            break;
        }
    }

    if ( !found )
    {
        LOG_ERROR( "Widget index not in Browser" );
        return;
    }

    item->widget( &w );
    redraw();
}

void Browser::remove( int idx )
{
    if (idx < 0 )
    {
        LOG_ERROR( "Index " << idx << " is invalid" );
        return;
    }

    Fl_Tree_Item* item = NULL;
    int i = 0;
    for ( item = first(); item; item = next(item), ++i )
    {
        if ( i == idx ) break;
    }

    if (!item || item == root() ) return;

    Fl_Tree::remove( item );
}

void Browser::add( Fl_Widget* w )
{
    Fl_Tree_Item* i = Fl_Tree::add( root(), w->label() );
    if ( i )
    {
        i->widget( w );
    }
}

void Browser::add( Fl_Group* g )
{
    Fl_Tree_Item* i = Fl_Tree::add( root(), g->label() );

    if ( i )
    {
        i->widget( g );
    }
}

void Browser::value( int x )
{
    //@todo: fltk1.3
    _value = x;
}

int Browser::value()
{
    //@todo: fltk1.3
    return _value;
}

// CHANGE CURSOR
//     Does nothing if cursor already set to value specified.
//
void Browser::change_cursor(Fl_Cursor newcursor) {
  if ( newcursor != _last_cursor ) {
      Fl_Widget* g = parent();
      for ( ; g->as_window() ; g = g->parent() )
          ;
      Fl_Window* w = (Fl_Window*) g;
      w->cursor(newcursor);
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
  if ( ! Fl::event_inside(this) ) {
    return -1;
  }

  const int* widths = column_widths();
  if (!widths) return -1;

  // @todo: fltk1.3
  // int mousex = Fl::event_x() + xposition();
  int mousex = Fl::event_x();
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

void Browser::column_labels( const char** labels )
{
    int i = 0;
    const char* lbl = labels[0];
    Fl_Group* g = new Fl_Group( 0, 0, w(), 20, lbl );

    for ( ; lbl; ++i )
    {
        lbl = labels[i];
        // Fl_Box* b = new Fl_Box(0, 0, 120, 20 );
        Fl_Box* b = new Fl_Box(0, 0, _column_widths[i], 20 );
        b->copy_label( lbl );
        g->add( b );
    }
    add( g );
}

// MANAGE EVENTS TO HANDLE COLUMN RESIZING
int Browser::handle(int e)
{
  // Not showing column separators? Use default Fl_Browser::handle() logic
  if ( ! column_separator() ) return(Fl_Tree::handle(e));
  // Handle column resizing
  int ret = 0;
  switch ( e ) {
  case FL_ENTER: {
    ret = 1;
    break;
  }
  case FL_MOVE: {
    if ( which_col_near_mouse() >= 0 ) {
      change_cursor(FL_CURSOR_WE);
    } else {
      change_cursor(FL_CURSOR_DEFAULT);
    }
    ret = 1;
    break;
  }
  case FL_PUSH: {
    int whichcol = which_col_near_mouse();
    if ( whichcol >= 0 ) {
      // CLICKED ON RESIZER? START DRAGGING
      ret = 1;
      _dragging = true;
      _dragcol = whichcol + 1;
      change_cursor(FL_CURSOR_DEFAULT);
    }
    break;
  }
  case FL_DRAG: 
      {
          if ( _dragging )
          {
              // Sum up column widths to determine position
              // @todo: fltk1.3
              // int mousex = Fl::event_x() + xposition();

              int mousex = Fl::event_x() + xposition();
              int newwidth = mousex - x();
        
              if ( newwidth > 0 ) {
                  int* cols = const_cast<int*>( column_widths() );
                  cols[ _dragcol ] = newwidth;
                  // set_column_start( _dragcol, newwidth );
                  redraw();
              }
          }
          break;
      }
  case FL_LEAVE:
  case FL_RELEASE:
      {
          _dragging = false;				// disable drag mode
          change_cursor(FL_CURSOR_DEFAULT);	// ensure normal cursor
          ret = 1;
          break;
      }
  } // switch
  if ( _dragging ) return(1);	// dragging? don't pass event to Fl_Tree
  return Fl_Tree::handle(e);
}

void Browser::layout()
{

  int nchildren = children();


  if ( _auto_resize )
    {
      int hh = 24;
      int nchildren = children();
      for ( int i = 1; i <= nchildren; ++i )
	{
	  Fl_Widget* c = child(i);
	  hh += c->h();
	}

      resize( x(), y(), w(), hh );
    }


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
      Fl_Widget* c = child(i);
      if ( ! c->as_group() ) continue;

      Fl_Group* g  = (Fl_Group*) c;

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
    // layout();

  Fl_Tree::draw();
  if (!column_widths()) return;

  if (!column_separator() ) return;

  // DRAW COLUMN SEPARATORS
  // @todo: fltk1.3
  // Rectangle r( w(), h() );
  // box()->inset(r);
  int X = x();
  int Y = y();
  int W = w();
  int H = h();
  // @todo: fltk1.3
  //  int colx = X - xposition();
  int colx = X - _hscroll->value();
  fl_color( column_separator_color() );
  const int* widths = column_widths();
  for ( int t=0; widths[t]; t++ ) {
      colx += widths[t];
      if ( colx > X && colx < (X+W) ) {
          fl_line(colx, Y, colx, Y+H-1);
      }
  }
}

  // int Browser::absolute_item_index( const Fl_Group* g )
  // {
  //   int idx = 1;
  //   int num = g->children();
  //   for (int i = 1; i <= num; ++i)
  //     {
  //       Fl_Widget* c = g->child(i);
  //       if ( c->as_group() )
  //         idx += absolute_item_index( (Fl_Group*) c );
  //       else
  //         ++idx;
  //     }
  //   return idx;
  // }

  // int Browser::absolute_item_index( bool& found,
  //       			    const Fl_Widget* item,
  //       			    const Fl_Widget* w )
  // {

  //   if ( w == item  ) {
  //     found = true;
  //     return 0;
  //   }

  //   Fl_Widget* wc = const_cast< Fl_Widget* >( w );
  //   if ( !wc->as_group() ) return 1;

  //   int idx = 1;
  //   Fl_Group* g = (Fl_Group*) w;
  //   int num = g->children();
    
  //   for ( int i = 1; i <= num; ++i )
  //     {
  //       Fl_Widget* c = g->child(i);
  //       idx += absolute_item_index( found, item, c );
  //       if ( found ) break;
  //     }

  //   return idx;
  // }

  int Browser::absolute_item_index()
  {
      Fl_Tree_Item* i = first_selected_item();
      if ( i == NULL ) return -1;

      int idx = 0;
      bool found = false;
      for ( Fl_Tree_Item *item = first(); item; item = next(item), ++idx ) {
          if ( i == item ) { found = true; break; }
          for ( int j = 0; j < item->children(); ++j, ++idx )
          {
              if ( i == item->child(j) ) { found = true; break; }
          }
      }

    return idx;
  }

} // namespace mrv

