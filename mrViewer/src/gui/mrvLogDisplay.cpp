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
 * @file   mrvLogDisplay.cpp
 * @author gga
 * @date   Tue Oct  9 20:56:16 2007
 * 
 * @brief  
 * 
 * 
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <fltk/TextBuffer.h>
#include <fltk/run.h>

#include "core/mrvHome.h"
#include "gui/mrvLogDisplay.h"

namespace mrv {
  
  // Style table
  fltk::TextDisplay::StyleTableEntry kLogStyles[] = {
    // FONT COLOR      FONT FACE   SIZE   ATTR
    // --------------- ----------- ----- ------
    {  fltk::BLACK,  fltk::COURIER, 14,   0 }, // A - Info
    {  fltk::DARK_YELLOW, fltk::COURIER, 14,   0 }, // B - Warning
    {  fltk::RED,    fltk::COURIER, 14,   0 }, // C - Error
  };

  LogDisplay::LogDisplay( int x, int y, int w, int h, const char* l  ) :
    fltk::TextDisplay( x, y, w, h, l )
  {
    color( fltk::GRAY75 );

    if ( !stylebuffer_ )
      {
	stylebuffer_ = new fltk::TextBuffer();
      }

    highlight_data(stylebuffer_, kLogStyles, 3, 'A', 0, 0);
  }

  LogDisplay::~LogDisplay()
  {
    delete stylebuffer_; stylebuffer_ = NULL;
  }

  void LogDisplay::clear()
  {
    buffer_->text("");
    stylebuffer_->text("");
  }

  void LogDisplay::save( const char* file )
  {
    char buf[2048];
    if ( !file )
      {
	 std::string home = mrv::homepath();
         sprintf( buf, "%s/mrViewer.log", home.c_str() );
         file = buf;
      }

    FILE* f = NULL;

    try {
      
       f = fltk::fltk_fopen( file, "w" );

      if ( !f ) throw;
      if ( fputs( buffer_->text(), f ) < 0 ) throw;

      info( "Saved log as \"" );
      info( file );
      info( "\"." );
    }
    catch( ... )
      {
	sprintf( buf, "Could not save log file \"%s\".", file );
	error( buf );
      }

    if ( f ) fclose(f);
  }
  
  void LogDisplay::info( const char* x )
  {
    buffer_->append( x );

    size_t t = strlen(x);
    while( t-- )
      stylebuffer_->append( "A" );
  }

  void LogDisplay::warning( const char* x )
  {
    buffer_->append( x );

    size_t t = strlen(x);
    while( t-- )
      stylebuffer_->append( "B" );
  }

  void LogDisplay::error( const char* x )
  {
    buffer_->append( x );

    size_t t = strlen(x);
    while( t-- )
      stylebuffer_->append( "C" );
  }

}

