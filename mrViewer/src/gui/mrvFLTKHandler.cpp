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
      std::string ext = filename;
      size_t start = ext.rfind( '.' );
      if ( start != ext.size() )
          ext = ext.substr( start + 1, ext.size() );
      if ( ext == "ctl" || ext == "CTL" || ext == "XML" || ext == "xml")
          return NULL;

      CMedia* img = CMedia::guess_image( filename, header, len );
      if ( img == NULL ) return NULL;

    // Fetch first frame
    img->probe_size( 50 );
    img->fetch( img->first_frame() );
    img->probe_size( 50000 );

    mrv::gui::media m( img );
    m.create_thumbnail();

    return (fltk::SharedImage*) m.thumbnail();
  }
}
