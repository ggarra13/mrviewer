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
 * @file   R3dImage.h
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 *
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 *
 *
 */

#ifndef R3dImage_h
#define R3dImage_h

#include <CMedia.h>

namespace R3DSDK
{
    class Clip;
}

namespace mrv {

class R3dImage : public CMedia
{

    R3dImage();

    static CMedia* create() {
        return new R3dImage();
    }


public:
    typedef std::vector< mrv::image_type_ptr > video_cache_t;
public:
    static bool test(const char* file);
    //static bool test(const uint8_t* data, const unsigned len );
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }

    virtual ~R3dImage();

    virtual bool initialize();
    virtual bool finalize();

    /// Returns true if image has an alpha channel
    virtual bool  has_alpha() const {
        return (_num_channels == 4);
    };

    virtual const char* const format() const { return "RED3D"; }

    virtual const char* const compression() const { return "RED3D CORE"; }

    virtual void clear_cache();

    virtual Cache is_cache_filled( int64_t frame );

    virtual bool fetch( mrv::image_type_ptr& canvas,
                        const boost::int64_t frame );

    virtual bool           frame( const int64_t f );
    inline int64_t frame() const { return _frame; }

    virtual bool find_image( const int64_t frame );

    void timed_limit_store( const int64_t frame );
    void limit_video_store( const int64_t frame );

protected:
    R3DSDK::Clip* clip;
    video_cache_t _images;
    int           _scale;
protected:
    static bool _init;
};

} // namespace mrv


#endif // R3dImage_h
