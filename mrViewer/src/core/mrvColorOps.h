/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2018  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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

#ifndef mrvColorOps_h
#define mrvColorOps_h

#include "core/mrvFrame.h"

namespace mrv {

class CMedia;

bool prepare_image( mrv::image_type_ptr& pic, const CMedia* img,
                    const image_type::Format format,
                    const image_type::PixelType pt );
void bake_ocio( const mrv::image_type_ptr& ptr, const CMedia* img );

}


#endif
