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

    infostream  info;
    warnstream  warn;
    errorstream error;
  }

}
