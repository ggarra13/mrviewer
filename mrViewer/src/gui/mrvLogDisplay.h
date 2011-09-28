/**
 * @file   mrvLogDisplay.h
 * @author gga
 * @date   Tue Oct  9 20:42:05 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvLogDisplay_h
#define mrvLogDisplay_h

#include <fltk/TextDisplay.h>

namespace mrv {

  class LogDisplay : public fltk::TextDisplay
  {
  public:
    LogDisplay( int x, int y, int w, int h, const char* l = 0 );
    ~LogDisplay();

    void clear();
    void save( const char* file = NULL );

    void info( const char* x );
    void warning( const char* x );
    void error( const char* x );
  };

}

#endif
