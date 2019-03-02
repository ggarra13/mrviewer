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
 * @file   oiioImage.h
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 *
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 *
 *
 */

#ifndef oiioImage_h
#define oiioImage_h

#include "mrvImageOpts.h"
#include "CMedia.h"

namespace mrv {

class WandOpts;

class oiioImage : public CMedia
{
    oiioImage();

    static CMedia* create() {
        return new oiioImage();
    }


public:
    static bool test(const char* file);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }

    virtual ~oiioImage();

    virtual const char* const format() const {
        return _format;
    }

    /// Returns the image compression (if any)w
    virtual const char* const compression() const {
        return _compression.c_str();
    }


    virtual bool initialize();
    virtual bool release();


    void level( int x ) {
        _level = x;
        refresh();
    }
    int level() const {
        return _level;
    }

    unsigned mipmap_levels() const {
        return _mipmaps;
    }

    bool fetch( mrv::image_type_ptr& canvas, const boost::int64_t frame );

    static bool save( const char* path, const CMedia* img,
                      const OIIOOpts* opts );
protected:
    char* _format;
    std::string _compression;
    int _level;  // current mipmap level
    unsigned _mipmaps;
};

} // namespace mrv


#endif // oiioImage_h
