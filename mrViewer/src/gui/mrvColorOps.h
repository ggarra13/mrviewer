/**
 * @file   mrvColorOps.h
 * @author gga
 * @date   Fri Jan 18 09:48:55 2008
 * 
 * @brief  
 * 
 * 
 */

#include <fltk/Color.h>

namespace mrv {

  fltk::Color darker(  fltk::Color c, uchar v = 0x20 );
  fltk::Color lighter( fltk::Color c, uchar v = 0x20 );
}
