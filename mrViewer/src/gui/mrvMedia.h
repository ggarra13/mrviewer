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
 * @file   mrvMedia.h
 * @author gga
 * @date   Thu Nov 15 04:16:50 2007
 * 
 * @brief  
 * 
 * 
 */
#ifndef mrv_gui_media_h
#define mrv_gui_media_h

#include <fltk/PixelType.h>
#include <fltk/Image.h>

#include <boost/shared_ptr.hpp>

#include "core/CMedia.h"

namespace fltk {
  class Image;
}


namespace mrv {


  namespace gui {

    class media
    {
    public:
      typedef CMedia::Mutex Mutex;

      media( CMedia* const img );
      ~media();


	 void position( boost::int64_t x ) { _pos = x; }
	 boost::int64_t position() const { return _pos; }

      CMedia* image()             { return _image; }
      const CMedia* image() const { return _image; }

      std::string name() const { return _image->name(); }

      fltk::Image* thumbnail()             { return _thumbnail; }
      const fltk::Image* thumbnail() const { return _thumbnail; }

      bool thumbnail_frozen() const    { return _thumbnail_frozen; }
      void thumbnail_freeze( bool t )  { _thumbnail_frozen = t; }

      void create_thumbnail();

    protected:
      void thumbnail_pixel( uchar*& ptr, fltk::PixelType pixeltype,
			    uchar r, uchar g, uchar b );

	 boost::int64_t  _pos;
	 CMedia*   _image;
	 fltk::Image* _thumbnail;
	 bool         _thumbnail_frozen;

	 static       int _thumbnail_height;
    };

  }


  typedef boost::shared_ptr< mrv::gui::media > media;
}


#endif // mrv_gui_media_h
