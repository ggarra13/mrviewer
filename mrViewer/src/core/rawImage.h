/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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
 * @file   rawImage.h
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 *
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 *
 *
 */

#ifndef rawImage_h
#define rawImage_h

#include <CMedia.h>

#include <libraw/libraw.h>

namespace mrv {

class rawImage : public CMedia
{
    rawImage();

    static CMedia* create() {
        return new rawImage();
    }


public:
    static bool test(const char* file);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }

    virtual ~rawImage();

    virtual const char* const format() const {
        return _format;
    }

    /// Returns the image compression (if any)
    virtual const char* const compression() const {
        return "";
    }

    virtual bool initialize();
    virtual bool release();

    bool fetch( mrv::image_type_ptr& canvas,
		const boost::int64_t frame );

protected:
    char* _format;
    libraw_data_t* iprc;
};

} // namespace mrv


#endif // rawImage_h
