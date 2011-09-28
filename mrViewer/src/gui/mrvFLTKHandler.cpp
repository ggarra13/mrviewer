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

#include "core/CMedia.h"

#include "gui/mrvFLTKHandler.h"
#include "gui/mrvMedia.h"

namespace mrv {

  fltk::SharedImage* fltk_handler( const char* filename, uchar* header,
				   int len )
  {
    CMedia* img = CMedia::guess_image( filename, header, len );
    if ( !img ) return NULL;

    // Fetch first frame
    img->fetch( img->first_frame() );

    mrv::gui::media m( img );
    m.create_thumbnail();

    return (fltk::SharedImage*) m.thumbnail();
  }
}
