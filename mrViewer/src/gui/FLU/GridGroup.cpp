// $Id: GridGroup.cpp,v 1.8 2004/01/27 21:44:24 jbryan Exp $

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
#include "fltk/layout.h"
#include "fltk/draw.h"

#include "FLU/GridGroup.h"



GridGroup::GridGroup( int x, int y, int w, int h, const char *l )
  : fltk::Group( x, y, w, h, l )
{
  set_vertical();

  Group::current(parent());
}


fltk::Widget* GridGroup::next( fltk::Widget* w )
{
  for( int i = 0; i < children()-1; ++i )
    {
      if( w == child(i) )
	return child(i+1);
    }
  return NULL;
}

fltk::Widget* GridGroup::previous( fltk::Widget* w )
{
  for( int i = 1; i < children(); ++i )
    {
      if( w == child(i) )
	return child(i-1);
    }
  return NULL;
}


fltk::Widget* GridGroup::above( fltk::Widget* w )
{
  for( int i = 0; i < children(); i++ )
    {
      if( w == child(i) )
	{
	}
    }
  return NULL;
}

fltk::Widget* GridGroup::below( fltk::Widget* w )
{
  for( int i = 0; i < children(); i++ )
    {
      if( w == child(i) )
	{
	}
    }
  return NULL;
}

fltk::Widget* GridGroup::left( fltk::Widget* w )
{
  for( int i = 0; i < children(); i++ )
    {
      if( w == child(i) )
	{
	}
    }
  return NULL;
}

fltk::Widget* GridGroup::right( fltk::Widget* w )
{
  for( int i = 0; i < children(); i++ )
    {
      if( w == child(i) )
	{
	}
    }
  return NULL;
}

void GridGroup::_measure( int& maxW, int& maxH )
{
  int nchildren = children();
  maxW = 0; maxH = 0;
  fltk::Widget* c;
  for ( int i = 0; i < nchildren; ++i )
    {
      c = child(i);
      if ( !c->visible() ) continue;
      if ( c->w() > maxW ) maxW = c->w();
      if ( c->h() > maxH ) maxH = c->h();
    }

  maxW += _spacing[0];
  maxH += _spacing[1];
}


void GridGroup::bbox( fltk::Rectangle& r )
{
  r.set(0,0,w(),h()); box()->inset(r);
}

void GridGroup::layout()
{
  int nchildren = children();
  if ( nchildren == 0 ) return;

  // First, find max size of child widgets
  fltk::Widget* c;
  int maxW, maxH;
  _measure( maxW, maxH );

  Rectangle r; bbox(r);

  int X = x() + box()->dx();
  int Y = y() + box()->dy();

  int rows = 0, cols = 0;

  // Do vertical arrangement
  if ( flags() & fltk::LAYOUT_VERTICAL )
    {
      int oy = Y;
      int lastY = Y + r.h();

      // will it all fit in one column?
      // If not, remove the size of the scrollbar
//       if ( Y + maxH * nchildren > lastY )
// 	  lastY -= scrollbar_width();

      rows = lastY - oy / maxH;
      for ( int i = 0; i < nchildren; ++i )
	{
	  c = child(i);
	  if( !c->visible() ) continue;
	  c->position( X, Y );
// 	  c->resize( X, Y, maxW, maxH );
	  Y += maxH;
	  if ( Y + maxH >= lastY ) 
	    {
	      Y = oy;
	      X += maxW;
	      ++cols;
	    }
	}
    }
  else
    {
      int ox = X;
      int lastX = X + r.w();
      // will it all fit in one row?  
      // If not, remove the size of the scrollbar
//       if ( X + maxW * nchildren > lastX )
// 	  lastX -= scrollbar_width();
      cols = lastX - ox / maxW;
      for ( int i = 0; i < nchildren; ++i )
	{
	  c = child(i);
	  if( !c->visible() ) continue;
	  c->position( X, Y );
// 	  c->resize( X, Y, maxW, maxH );
	  X += maxW;
	  if ( X + maxW >= lastX ) 
	    {
	      X = ox;
	      Y += maxH;
	      ++rows;
	    }
	}
    }

  int W = cols * maxW;
  int H = rows * maxH;
  w(W);
  h(H);
  //  resize( W, H );

}


void GridGroup::draw()
{
  Rectangle r; bbox(r);
  fltk::push_clip(r);
  draw_frame();
  Group::draw();
  fltk::pop_clip();
}
