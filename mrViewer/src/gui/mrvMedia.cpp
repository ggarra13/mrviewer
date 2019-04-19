/*
    mrViewer - thume professional movie and flipbook playback
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


#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <math.h>

#include <FL/Fl_Shared_Image.H>
#include <ImathMath.h>   // for Imath::clamp
// #include <ImathFun.h>   // for Imath::pow

#include "core/mrvThread.h"
#include "core/CMedia.h"
#include "gui/mrvIO.h"
#include "gui/mrvFLTKHandler.h"
#include "gui/mrvMedia.h"

#ifdef _WIN32
#define isfinite(x) _finite(x)
#endif

#undef IMG_ERROR
#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )

namespace
{
const char* kModule = "gui";
}

namespace mrv {

namespace gui {

int media::_thumbnail_width = 128;
int media::_thumbnail_height = 64;

media::media( CMedia* const img ) :
    _start( img->first_frame() ),
    _image( img ),
    _thumbnail( NULL ),
    _thumbnail_frozen( false )
{
}

media::~media()
{
    delete _image;
    _image = NULL;
    if ( _thumbnail )
    {
	delete _thumbnail;
        _thumbnail = NULL;
    }
}

void media::position( int64_t x ) {
    _image->position(x);
}

int64_t media::position() const {
    return _image->position();
}

void media::thumbnail_pixel( uchar*& ptr, uchar r, uchar g, uchar b )
{
    *ptr++ = r;
    *ptr++ = g;
    *ptr++ = b;
}


void media::create_thumbnail()
{
    if ( !_image->stopped() || thumbnail_frozen() ) return;

    // Make sure frame memory is not deleted
    Mutex& mutex = _image->video_mutex();
    SCOPED_LOCK( mutex );

    // Audio only clip?  Return
    mrv::image_type_ptr pic = _image->left();

    if ( !pic ) {
	LOG_ERROR( _("Empty pic file for media ") << _image->name() );
	abort();
	return;
    }
    
    unsigned dw = pic->width();
    unsigned dh = pic->height();
    if ( dw == 0 || dh == 0 ) {
	LOG_ERROR( _("Media file has zero size in width or height") );
	return;
    }
    
    unsigned int h = _thumbnail_height;

    float yScale = (float)(h+0.5) / (float)dh;
    unsigned int w = (float)(dw+0.5) * (float)yScale;
    if ( w > 150 ) w = 150;

    // Resize image to thumbnail size
    pic.reset( pic->quick_resize( w, h ) );

    w = pic->width();
    h = pic->height();

    // Create a unique name for this element, using fileroot and start frame
    char buf[2048];
    sprintf( buf, "%s_%" PRId64, _image->fileroot(), _start );

    delete _thumbnail;
    
    uchar* data = new uchar[ w * h * 3 ];
    _thumbnail = new Fl_RGB_Image( data, w, h, 3 );
    _thumbnail->alloc_array = 1;
    
    if ( !_thumbnail )
    {
        IMG_ERROR( _("Could not create thumbnail picture for '")
                   << _image->fileroot() << "'" );
        return;
    }

    uchar* ptr = data;

    // Copy to thumbnail and gamma it
    float gamma = 1.0f / _image->gamma();
    for (unsigned int y = 0; y < h; ++y )
    {
        for (unsigned int x = 0; x < w; ++x )
        {
            CMedia::Pixel fp = pic->pixel( x, y );
            if ( gamma != 1.0f )
            {
                using namespace std;
                if ( isfinite( fp.r ) )
                    fp.r = Imath::Math<float>::pow( fp.r, gamma );
                if ( isfinite( fp.g ) )
                    fp.g = Imath::Math<float>::pow( fp.g, gamma );
                if ( isfinite( fp.b ) )
                    fp.b = Imath::Math<float>::pow( fp.b, gamma );
            }

            uchar r = (uchar)(Imath::clamp(fp.r, 0.f, 1.f) * 255.0f);
            uchar g = (uchar)(Imath::clamp(fp.g, 0.f, 1.f) * 255.0f);
            uchar b = (uchar)(Imath::clamp(fp.b, 0.f, 1.f) * 255.0f);
            thumbnail_pixel( ptr, r, g, b );
        }
    }

    _image->image_damage( _image->image_damage() &
                          ~CMedia::kDamageThumbnail );
}


} // namespace gui

} // namemspace mrv
