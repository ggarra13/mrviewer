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
 * @file   mrvIO.cpp
 * @author gga
 * @date   Wed Jul 11 12:04:58 2007
 * 
 * @brief  
 * 
 * 
 */

#include <cstdlib> // for free/malloc
#include <cstring> // for strcpy

#include "gui/mrvLogDisplay.h"
#include "mrViewer.h" // for uiLog
#include "mrvIO.h"

namespace mrv {

  static char* _alert = NULL;

  void alert( const char* str )
  {
    free( _alert );
    if ( str == NULL )
      {
	_alert = NULL;
	return;
      }

    _alert = (char*) malloc( strlen(str) + 1 );
    strcpy( _alert, str );

    fltk::alert( _alert );
  }

  const char* alert()
  {
    return _alert;
  }


  namespace io
  {
    boost::recursive_mutex logbuffer::_mutex;

    int logbuffer::sync()
    {
      if ( ! pbase() ) return 0;

      // lock mutex
      boost::recursive_mutex::scoped_lock lk( _mutex );

      // make sure to null terminate the string
      sputc('\0');

      // freeze and call the virtual print method
      char* c = strdup( str().c_str() );

      print( c );

      free(c);

      // reset iterator to first position & unfreeze
      seekoff( 0, std::ios::beg );
      return 0;
    }
    
    void errorbuffer::print( const char* c )
    {
      // Send string to Log Window
      if ( ViewerUI::uiLog )
	ViewerUI::uiLog->uiLogText->error( c );

      std::cerr << c;
    }

    void warnbuffer::print( const char* c )
    {
      // Send string to Log Window
      if ( ViewerUI::uiLog )
	ViewerUI::uiLog->uiLogText->warning( c );

      std::cerr << c;
    }

    void infobuffer::print( const char* c )
    {
      // Send string to Log Window
      if ( ViewerUI::uiLog )
	ViewerUI::uiLog->uiLogText->info( c );

      std::cout << c;
    }

    void connbuffer::print( const char* c )
    {
      // Send string to Log Window in Connection panel
       if ( ViewerUI::uiConnection )
	  ViewerUI::uiConnection->uiLog->info( c );
       
       std::cout << c;
    }

    connstream  conn;
    infostream  info;
    warnstream  warn;
    errorstream error;
  }

}
