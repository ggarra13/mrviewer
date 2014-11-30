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
 * @file   mrvMedia.cpp
 * @author gga
 * @date   Thu Nov 15 02:27:03 2007
 * 
 * @brief  
 * 
 * 
 */

#include <fltk/Image.h>
#include <ImathMath.h>   // for Imath::clamp
#include <ImathFun.h>   // for Imath::pow

#include "core/mrvThread.h"
#include "core/CMedia.h"
#include "gui/mrvIO.h"
#include "gui/mrvMedia.h"



namespace 
{
  const char* kModule = "gui";
}

namespace mrv {

  namespace gui {

    int media::_thumbnail_height = 64;

    media::media( CMedia* const img ) :
    _pos( 1 ),
    _image( img ),
    _thumbnail( NULL ),
    _thumbnail_frozen( false )
    {
    }

    media::~media()
    {
       delete _image; _image = NULL;
      // thumbnail is not deleted, as fltk will do it for us.
    }

    void media::thumbnail_pixel( uchar*& ptr, fltk::PixelType pixeltype,
				 uchar r, uchar g, uchar b )
    {
      switch( pixeltype )
	{
	case fltk::ARGB32:
	  {
	    *ptr++ = b; *ptr++ = g; *ptr++ = r;
	    *ptr++ = 0xff;
	    break;
	  }
	case fltk::RGB:
	  {
	    *ptr++ = r; *ptr++ = g; *ptr++ = b;
	    break;
	  }
	case fltk::RGB32:
	  {
	    *ptr++ = 0xff; *ptr++ = r; *ptr++ = g; *ptr++ = b;
	    break;
	  }
	case fltk::RGBx:
	case fltk::RGBA:
	  {
	    *ptr++ = r; *ptr++ = g; *ptr++ = b; *ptr++ = 0xff;
	    break;
	  }
	default:
	  {
	    IMG_ERROR("unknown pixel type for thumbnail");
	  }
	}
    }

    void media::create_thumbnail()
    {
       if ( !_image->stopped() || thumbnail_frozen() ) return;

      // Make sure frame memory is not deleted
      Mutex& mutex = _image->video_mutex();
      SCOPED_LOCK( mutex );

      unsigned dw = _image->width();
      unsigned dh = _image->height();

      unsigned int w = 0;
      unsigned int h = _thumbnail_height;

      float yScale = (float)h / (float)dh;
      float xScale = yScale;

      if ( _thumbnail )
      {
          // once we set thumbnail size we cannot change it
          w = _thumbnail->width();
      }
      else
      {
          w = (int) ((float)dw * xScale);
          if ( w < 8 ) w = 32;
      }

      // Audio only clip?  Return
      mrv::image_type_ptr pic = _image->hires();
      if ( !pic ) return;

      // Resize image to thumbnail size
      pic.reset( pic->quick_resize( w, h ) );
      w = pic->width();
      h = pic->height();

      if ( _thumbnail == NULL )
	{
            _thumbnail = new fltk::Image( fltk::RGB, w, h );
            if ( _thumbnail == NULL )
            {
                IMG_ERROR( "Could not allocate thumbnail" );
                return;
            }
	}


      uchar* ptr = (uchar*) _thumbnail->buffer();
      if (!ptr )
	{
	  IMG_ERROR("Could not allocate thumbnail");
	  return;
	}

      fltk::PixelType pixeltype = _thumbnail->buffer_pixeltype();

      // Copy to thumbnail and gamma it
      float gamma = 1.0f / _image->gamma();
      for (unsigned int y = 0; y < h; ++y )
	{
	  for (unsigned int x = 0; x < w; ++x )
	    {
	      CMedia::Pixel fp = pic->pixel( x, y );
	      if ( gamma != 1.0 )
	      {
                  if ( fp.r > 0.f )
                      fp.r = Imath::Math<float>::pow( fp.r, gamma );
                  if ( fp.g > 0.f )
                      fp.g = Imath::Math<float>::pow( fp.g, gamma );
                  if ( fp.b > 0.f )
                      fp.b = Imath::Math<float>::pow( fp.b, gamma );
	      }

	      uchar r = (uchar)(Imath::clamp(fp.r, 0.f, 1.f) * 255.0f);
	      uchar g = (uchar)(Imath::clamp(fp.g, 0.f, 1.f) * 255.0f);
	      uchar b = (uchar)(Imath::clamp(fp.b, 0.f, 1.f) * 255.0f);
	      thumbnail_pixel( ptr, pixeltype, r, g, b );
	    }
	}

      _thumbnail->buffer_changed();

      _image->image_damage( _image->image_damage() & 
			    ~CMedia::kDamageThumbnail );
    }


  } // namespace gui

} // namemspace mrv
