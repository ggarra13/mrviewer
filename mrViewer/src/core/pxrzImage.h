/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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
 * @file   pxrzImage.h
 * @author gga
 * @date   Fri Sep 21 01:25:12 2007
 *
 * @brief  Image class to read Pixar's .z files
 *
 *
 */

#ifndef pxrzImage_h
#define pxrzImage_h

#include <CMedia.h>

namespace mrv {

class pxrzImage : public CMedia
{
    pxrzImage();
    ~pxrzImage();

    static CMedia* create() {
        return new pxrzImage();
    }

public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const {
        return "Pixar shadow map";
    }

    virtual bool fetch( mrv::image_type_ptr& canvas,
			const boost::int64_t frame );
};

} // namespace mrv


#endif // pxrzImage_h

