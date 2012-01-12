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
#include <gui/mrvLogDisplay.h>

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
	const char* home = getenv("HOME");
	if ( !home )
	  {
	    home = getenv("USERPROFILE");

	    if ( !home ) home = "/usr/tmp";
	  }

	sprintf( buf, "%s/mrViewer.log", home );
	file = buf;
      }

    FILE* f = NULL;

    try {
      
      f = fopen( file, "w" );

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

