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


#include <FL/Fl_Image.H>

#include <boost/shared_ptr.hpp>

#include "core/CMedia.h"


namespace mrv {


namespace gui {

class media
{
public:
    typedef CMedia::Mutex Mutex;

    media( CMedia* const img );
    ~media();


    void position( int64_t x );
    int64_t position() const;

    inline int64_t duration() const {
        return _image->duration();
    }

    inline void own_image( bool t )
        {
            _own_image = t;
        }

    inline bool own_image() const
        {
            return _own_image;
        }

    inline CMedia* image()             {
        return _image;
    }
    inline const CMedia* image() const {
        return _image;
    }

    inline std::string name() const {
        return _image->name();
    }

    inline Fl_RGB_Image* thumbnail()             {
        return _thumbnail;
    }
    inline const Fl_RGB_Image* thumbnail() const {
        return _thumbnail;
    }

    inline bool thumbnail_frozen() const    {
        return _thumbnail_frozen;
    }
    inline void thumbnail_freeze( bool t )  {
        _thumbnail_frozen = t;
    }

    void create_thumbnail();

protected:
    void thumbnail_pixel( uchar*& ptr, uchar r, uchar g, uchar b );

    int64_t  _start;
    int64_t  _pos;
    CMedia*   _image;
    Fl_RGB_Image* _thumbnail;
    bool         _thumbnail_frozen;
    bool         _own_image;

public:
    static       int _thumbnail_width;
    static       int _thumbnail_height;
};

}


typedef boost::shared_ptr< mrv::gui::media > media;
}


#endif // mrv_gui_media_h
