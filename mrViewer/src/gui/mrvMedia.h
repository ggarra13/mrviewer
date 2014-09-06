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
