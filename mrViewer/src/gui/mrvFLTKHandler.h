/**
 * @file   mrvFLTKHandler.cpp
 * @author gga
 * @date   Sat Jan 19 23:11:02 2008
 * 
 * @brief  fltk handler to read mrViewer images and return thumbnail for
 *         file requesters.
 * 
 * 
 */

#ifndef mrvFLTKHandler_h
#define mrvFLTKHandler_h

#include "fltk/SharedImage.h"

namespace mrv {

  fltk::SharedImage* fltk_handler( const char* filename, uchar* header,
				   int len );

}

#endif // mrvFLTKHandler_h
