// $Id: Flu_Wrap_Group.cpp,v 1.8 2004/01/27 21:44:24 jbryan Exp $

/***************************************************************
 *                FLU - FLTK Utility Widgets 
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 * 
 ***************************************************************/
#include <iostream>
using namespace std;

#include <cstring>
#include <cstdio>

#include "fltk/Symbol.h"
#include "fltk/Valuator.h"
#include "fltk/layout.h"

#include "fltk/events.h"
#include "fltk/damage.h"
#include "fltk/draw.h"

#include "FLU/Flu_Wrap_Group.h"
#include "FLU/Flu_File_Chooser.h"

#if USE_CLIPOUT
extern fltk::Widget* fl_did_clipping;
#endif

Flu_Wrap_Group::Flu_Wrap_Group( int x, int y, int w, int h, const char *l )
  : fltk::ScrollGroup( x, y, w, h, l ),
    scrollToElement( NULL )
{
  uchar r,g,b;
  fltk::split_color( color(), r, g, b );
  scrollbar.color( fltk::color(r*2,g*2,b*2) );
  hscrollbar.color( fltk::color(r*2,g*2,b*2) );

  set_vertical();

  Group::current(parent());
}


void Flu_Wrap_Group::scroll_to( const fltk::Widget* w )
{
  scrollToElement = w;

  layout();
  scrollTo( w->x(), w->y() );
  relayout();
}

void Flu_Wrap_Group::scroll_to_beginning()
{
  scrollTo(0,0);
}

void Flu_Wrap_Group::scroll_to_end()
{
  scrollTo( int(hscrollbar.maximum()), int(scrollbar.maximum()) );
}



void Flu_Wrap_Group::_measure( int& maxW, int& maxH ) const
{
  int nchildren = children();
  maxW = 0; maxH = 0;
  fltk::Widget* c;
  for ( int i = 0; i < nchildren; ++i )
    {
      c = child(i);
      if ( !c->visible() ) continue;
      if ( c->w() > maxW ) maxW = c->w();
      maxH += c->h();
    }

  maxW += _spacing[0];
  maxH += _spacing[1];
}


void Flu_Wrap_Group::layout_grid()
{
  int rows = 0; int cols = 0;

  int nchildren = children();
  if ( nchildren == 0 ) return;

  // First, find max size of child widgets
  fltk::Widget* c;
  int maxW, maxH;
  _measure( maxW, maxH );

  hscrollbar.clear_visible();
  scrollbar.clear_visible();

  Rectangle r;
  r.set(0,0,w(),h()); box()->inset(r);

  int X = -xposition() + box()->dx();
  int Y = -yposition() + box()->dy();
  int OX, OY;

  rows = cols = 1;
  int sw = scrollbar_width();


  // Do vertical arrangement
  if ( flags() & fltk::LAYOUT_VERTICAL )
    {
      int lastY = Y + r.h();

      // will it all fit in one column?
      // If not, remove the size of the scrollbar
      if ( Y + maxH > lastY )
	{
	  if ( scrollbar_align()&fltk::ALIGN_TOP )
	    {
	      Y += sw;
	    }
	  else
	    lastY -= sw;
	}

      OX = X; OY = Y;
      rows = (lastY - Y) / maxH;
      for ( int i = 0; i < nchildren; ++i )
	{
	  c = child(i);
	  if( !c->visible() ) continue;
	  c->position( X, Y );
	  Y += c->h();
	  if ( i != nchildren-1 && Y + maxH >= lastY ) 
	    {
	      Y = OY;
	      X += maxW;
	      ++cols;
	    }
	}
    }
  else
    {
      int lastX = X + r.w();

      // will it all fit in one row?  
      // If not, remove the size of the scrollbar
      if ( X + maxW * nchildren > lastX )
	{
	  if ( scrollbar_align()&fltk::ALIGN_LEFT )
	    {
	      X += sw;
	    }
	  else
	    lastX -= sw;
	}

      OX = X; OY = Y;
      cols = (lastX - X) / maxW;
      for ( int i = 0; i < nchildren; ++i )
	{
	  c = child(i);
	  if( !c->visible() ) continue;
	  c->position( X, Y );
	  X += maxW;
	  if ( i != nchildren-1 && X + maxW >= lastX ) 
	    {
	      X = OX;
	      Y += c->h();
	      ++rows;
	    }
	}
    }

  int W = cols * maxW;
  if ( W > r.w() )
    {
      hscrollbar.set_visible();
      hscrollbar.resize(r.x(), scrollbar_align()&fltk::ALIGN_TOP ? 
			r.y() : r.b()-sw, r.w(), sw);
      hscrollbar.value(r.x()-OX, r.w(), 0, W);
    }
  else
    {
      for ( int i = 0; i < nchildren; ++i )
	{
	  c = child(i);
	  if( !c->visible() ) continue;
	  c->position( c->x() + xposition(), c->y() );
	}
    }

  int H = maxH;
  if ( H > r.h() )
    {
      scrollbar.set_visible();
      scrollbar.resize(scrollbar_align()&fltk::ALIGN_LEFT ? 
		       r.x() : r.r()-sw, r.y(), sw, r.h());
      scrollbar.value(r.y()-OY, r.h(), 0, H);
    }
  else
    {
      for ( int i = 0; i < nchildren; ++i )
	{
	  c = child(i);
	  if( !c->visible() ) continue;
	  c->position( c->x(), c->y() + yposition() );
	}
    }

  if ( scrollToElement )
    {
      scrollTo( scrollToElement->x(), scrollToElement->y() );
      scrollToElement = NULL;
    }
}


void Flu_Wrap_Group::layout()
{
  layout_grid();
}



void Flu_Wrap_Group::draw()
{
  draw_box();

  Rectangle r(w(),h()); 
  box()->inset(r);
  fltk::push_clip(r);
  for ( int i = 0; i < children(); ++i )
    draw_child( *(child(i)) );
  draw_child(scrollbar);
  draw_child(hscrollbar);
  fltk::pop_clip();
}


void Flu_Wrap_Group::grid(unsigned int& cols, unsigned int& rows) const
{
  cols = rows = 0;
  if ( children() == 0 ) return;

  Rectangle r; 
  Flu_Wrap_Group* w = const_cast<Flu_Wrap_Group*>(this);
  w->bbox(r);
  int maxW, maxH;
  _measure( maxW, maxH );
  if ( flags() & fltk::LAYOUT_VERTICAL )
    {
      rows = r.h() / maxH;
      cols = children() / rows;
    }
  else
    {
      cols = r.w() / maxW;
      rows = children() / cols;
    }
  if ( rows == 0 ) rows = 1;
  if ( cols == 0 ) cols = 1;
}

void Flu_Wrap_Group::grid_find( unsigned int& col, unsigned int& row,
				const unsigned int cols, const unsigned int rows,
				const fltk::Widget* w ) const
{
  for ( int i = 0; i < children(); ++i )
    {
      if ( child(i) == w )
	{
	  if ( flags() & fltk::LAYOUT_VERTICAL )
	    {
	      row = i % rows;
	      col = i - row * rows;
	      return;
	    }
	  else
	    {
	      col = i % cols;
	      row = i - col * cols;
	      return;
	    }
	}
    }
  col = row = 0; // we failed to find widget
  return;
}

void Flu_Wrap_Group::grid_find( unsigned int& col, unsigned int& row,
				const fltk::Widget* w ) const
{
  unsigned int cols, rows; grid(cols, rows); grid_find( col, row, cols, rows, w );
}

unsigned int Flu_Wrap_Group::index_from_grid( const unsigned int c,
					      const unsigned int r,
					      const unsigned int cols,
					      const unsigned int rows ) const
{
  if ( flags() & fltk::LAYOUT_VERTICAL )
    {
      return c * rows + r;
    }
  else
    {
      return r * cols + c;
    }
} 

fltk::Widget* Flu_Wrap_Group::above( const fltk::Widget* w ) const  
{ 
  unsigned int c, r, cols, rows; grid(cols, rows); grid_find( c, r, cols, rows, w );
  if ( r == 0 ) return NULL;
  return child( index_from_grid( c, r, cols, rows ) ); 
}

fltk::Widget* Flu_Wrap_Group::below( const fltk::Widget* w ) const 
{ 
  unsigned int c, r, cols, rows; grid(cols, rows); grid_find( c, r, cols, rows, w );
  if ( r == rows-1 ) return NULL;
  return child( index_from_grid( c, r, cols, rows ) );
}

fltk::Widget* Flu_Wrap_Group::left( const fltk::Widget* w ) const 
{ 
  unsigned int c, r, cols, rows; grid(cols, rows); grid_find( c, r, cols, rows, w );
  if ( c == 0 ) return NULL;
  return child( index_from_grid( c, r, cols, rows ) ); 
}


fltk::Widget* Flu_Wrap_Group::right( const fltk::Widget* w ) const
{
  unsigned int c, r, cols, rows; grid(cols, rows); grid_find( c, r, cols, rows, w );
  if ( c == cols-1 ) return NULL;
  return child( index_from_grid( c, r, cols, rows ) ); 
}



//! \return the widget that logically follows the passed widget.
fltk::Widget* Flu_Wrap_Group::next( const fltk::Widget* w ) const
{
  unsigned int i = find( w );
  if ( i == children() - 1 ) return NULL;
  return child(i + 1);
}

//! \return the widget that logically comes before the passed widget.
fltk::Widget* Flu_Wrap_Group::previous( const fltk::Widget* w ) const
{
  unsigned int i = find( w );
  if ( i == 0 ) return NULL;
  return child(i - 1);
}

int Flu_Wrap_Group::handle( int event )
{
   if ( event == fltk::MOUSEWHEEL )
   {
      fltk::e_dy = fltk::event_dy() * 8;
   }

   return fltk::ScrollGroup::handle( event );
}
