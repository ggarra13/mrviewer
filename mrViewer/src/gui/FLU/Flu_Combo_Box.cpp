// $Id: Flu_Combo_Box.cpp,v 1.20 2004/10/15 14:46:12 jbryan Exp $

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



#include <cstdio>
#include <fltk/draw.h>
#include <cstring>
#include <cstdlib>

#include <fltk/Cursor.h>
#include <fltk/Symbol.h>
#include <fltk/events.h>
#include <fltk/run.h>

#include "FLU/Flu_Combo_Box.h"

Flu_Combo_Box::Flu_Combo_Box( int X, int Y, int W, int H, const char* l )
  : fltk::Group( X, Y, W, H, l ), input( X, Y, W, H )
{
  box( fltk::DOWN_BOX );
  align( fltk::ALIGN_LEFT );
  pop_height( 100 );

  _cbox = NULL;
  _valbox = fltk::UP_BOX;

  input_callback( NULL );
  input.box( fltk::FLAT_BOX );
  input.callback( input_cb, this );
  input.when( fltk::WHEN_ENTER_KEY_ALWAYS );
  input.selection_color( fltk::WHITE );
  input.textfont( fltk::HELVETICA );

  input.resize( X + box()->dx(), Y + box()->dy(), 
		W-18-box()->dw(), H - box()->dh() );

  editable( true );

  end();
}

Flu_Combo_Box::~Flu_Combo_Box()
{
}

void Flu_Combo_Box::set_combo_widget( fltk::Widget *w )
{
  _cbox = w;
  this->add( w );
}

void Flu_Combo_Box::input_cb( fltk::Widget*, void* v )
{
  // taken from Fl_Counter.cxx
  Flu_Combo_Box& t = *(Flu_Combo_Box*)v;

  if( strcmp( t.input.value(), t.value() )!=0 || t.input.when() & fltk::WHEN_NOT_CHANGED)
    {
      if( t.when() )
	{
	  t.clear_changed();
	  if( t._inputCB )
	    t._inputCB( &t, t._inputCBD );
	  else
	    t.do_callback();
	}
      else
	{
	  t.set_changed();
	}
    }
}

void Flu_Combo_Box::resize( int X, int Y, int W, int H )
{
  fltk::Group::resize( X, Y, W, H );
  input.resize( X+ box()->dx(), Y + box()->dy(), 
		W-18-box()->dw(), H - box()->dh() );
}

void Flu_Combo_Box::draw()
{
  int W = 18, H = h()-4;
  int X = x()+w()-W-2, Y = y()+2;

  draw_box();

  // draw the arrow button
  _valbox->draw( fltk::Rectangle( X, Y, W, H ) );
  fltk::color( active_r() ? color() : fltk::inactive(color()) );
  fltk::addvertex( X+W/2-5, Y+H/2-2 );
  fltk::addvertex( X+W/2+3, Y+H/2-2 );
  fltk::addvertex( X+W/2-1, Y+H/2+2 );   
  fltk::fillpath();

  draw_child( input );
  if( fltk::focus() == this )
    box()->draw_symbol_overlay( fltk::Rectangle( input.x(), input.y(), 
						 input.w(), input.h() ) );
}

int global_x( fltk::Widget *w )
{
  int x = w->x();
  fltk::Widget *o = w->parent();
  while( o )
    {
      x += o->x();
      o = o->parent();
    }
  return x;
}

int global_y( fltk::Widget *w )
{
  int y = w->y();
  fltk::Widget *o = w->parent();
  while( o )
    {
      y += o->y();
      o = o->parent();
    }
  return y;
}

Flu_Combo_Box::Popup::Popup( Flu_Combo_Box *b, fltk::Widget *c, int H )
  : fltk::DoubleBufferWindow( global_x(b)-2, //fltk::x()+b->window()->x()+b->x()-2,
		      global_y(b)+b->h()-2, //fltk::y()+b->window()->y()+b->y()+b->h()-2,
		      b->w()+4, H, 0 )
{
  combo = b;
  dragging = false;
  selected = NULL;

  box( fltk::BORDER_FRAME );
  border( 0 );
  add( c );
  end();
  //set_non_modal();
  set_modal();

  c->resize( 1, 1, w()-2, h()-2 );
}

Flu_Combo_Box::Popup::~Popup()
{
  while( children() )
    remove( child(0) );
}

void Flu_Combo_Box::value( const char *v )
{
  if( _value( v ) )
    input.value( v );
}

void Flu_Combo_Box::selected( const char *v )
{
  if( v )
    input.value( v );
  _popped = false;
  do_callback();
}

int Flu_Combo_Box::Popup::handle( int event )
{
  if( event == fltk::MOVE )
    {
      // fltk::MOVE is also generated while the window is moving
      // this attempts to keep the popup window moving with the enclosing window
      //position( combo->window()->x()+combo->x()-2, combo->window()->y()+combo->y()+combo->h()-2 );
      position( global_x(combo)-2, global_y(combo)+combo->h()-2 );
      // this lets the mouse move event also move the selected item
      combo->_hilight( fltk::event_x(), fltk::event_y() );
    }

  if( event == fltk::DRAG )
    dragging = true;

  // if push outside the popup window, popdown
  if( event == fltk::PUSH &&
      !fltk::event_inside( *child(0) ) )
    {
      combo->_popped = false;
      return 0;
    }

  // if release after dragging outside the popup window, popdown
  if( event == fltk::RELEASE && dragging && 
      !fltk::event_inside( *child(0) ) )
    {
      combo->_popped = false;
      return 0;
    }

  if( event == fltk::KEY )
    {
      unsigned key = fltk::event_key();
      if( key == fltk::EscapeKey )
	{
	  combo->_popped = false;
	  return 0;
	}
      else if( key == fltk::UpKey )
	{
	  const char *s = combo->_previous();
	  if( s )
	    selected = s;
	  return 1;
	}
      else if( key == fltk::DownKey )
	{
	  const char *s = combo->_next();
	  if( s )
	    selected = s;
	  return 1;
	}
      else if( key == fltk::ReturnKey || key == ' ' )
	{
	  if( selected )
	    {
	      combo->value( selected );
	      combo->selected( selected );
	    }
	  combo->_popped = false;
	  return 1;	  
	}
    }

  return fltk::DoubleBufferWindow::handle( event );
}

int Flu_Combo_Box::handle( int event )
{
  if( event == fltk::KEY && fltk::event_key() == fltk::TabKey )
    return fltk::Group::handle( event );

  // is it time to popup?
  bool open = ( event == fltk::PUSH ) && 
    (!fltk::event_inside( input ) || ( !editable() && fltk::event_inside( input ) ) );
  open |= ( event == fltk::KEY ) && (fltk::event_key() == ' ');

  if( open )
    {
      fltk::cursor( fltk::CURSOR_DEFAULT );

      _valbox = fltk::THIN_DOWN_BOX;
      redraw();

      // remember old current group
      fltk::Group *c = fltk::Group::current();

      // set current group to 0 so this is a top level popup window
      fltk::Group::current( 0 );
      Popup *_popup = new Popup( this, _cbox, popHeight );

      // show it and make FLTK send all events there
      value( value() );
      _popup->show();
      fltk::modal( _popup, true );
      fltk::focus( _cbox );
      _popped = true;
      fltk::pushed( _cbox );

      // wait for a selection to be made
      while( _popped )
	fltk::wait();

      // restore things and delete the popup
      _popup->hide();
      fltk::modal( 0 );
      delete _popup;
      fltk::Group::current( c );
      fltk::focus( this );

      _valbox = fltk::UP_BOX;
      redraw();

      return 1;
    }

  if( input.handle(event) )
    {
      if( !editable() && ( event == fltk::ENTER || event == fltk::LEAVE ) )
	fltk::cursor( fltk::CURSOR_DEFAULT );
      return 1;
    }
  else
    return 0;
}
