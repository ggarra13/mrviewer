/**
 * @file   mrvColorOps.cpp
 * @author gga
 * @date   Fri Jan 18 09:54:42 2008
 * 
 * @brief  
 * 
 * 
 */


#include "gui/mrvColorOps.h"

namespace mrv {

  fltk::Color darker( fltk::Color c, uchar v )
  {
    uchar r,g,b;
    fltk::split_color( c, r, g, b );
    if (r > v ) r -= v;
    else r = 0;
    if (g > v ) g -= v;
    else g = 0;
    if (b > v ) b -= v;
    else b = 0;
    return fltk::color( r, g, b );
  }

  fltk::Color lighter( fltk::Color c, uchar v )
  {
    uchar r,g,b;
    fltk::split_color( c, r, g, b );

    if ( 0xff - r > v ) r += v;
    else r = 0xff;
    if ( 0xff - g > v ) g += v;
    else g = 0xff;
    if ( 0xff - b > v ) b += v;
    else b = 0xff;

    return fltk::color( r, g, b );
  }

}
