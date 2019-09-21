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
 * @file   colorBarsImage.h
 * @author gga
 * @date   Sun Oct 22 22:52:27 2006
 *
 * @brief  A simple 3-second color bar generator (with tone)
 *
 *
 */


#ifndef mrvBlackImage_h
#define mrvBlackImage_h

#include "CMedia.h"


namespace mrv {

class BlackImage : public CMedia
{
public:
    enum Type
    {
        kGap
    };

public:
    BlackImage( Type type = kGap );
    virtual ~BlackImage() {};

    static bool test(const char* file) {
        return false;
    }

    virtual const char* const format() const {
        return "Built-in Image";
    }

    void create( image_type_ptr& canvas );

    virtual bool fetch( mrv::image_type_ptr& canvas,
                        const boost::int64_t frame );

};

} // namespace mrv


#endif // mrvBlackImage_h
