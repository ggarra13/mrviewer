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
 * @file   BlackImage.cpp
 * @author gga
 * @date   Thu Jul  5 18:37:58 2007
 *
 * @brief  A simple class to create a built-in black image.
 *
 *
 */

#include "gui/mrvIO.h"
#include "mrvBlackImage.h"

namespace {
    const char* kModule = "black";
}

namespace mrv {


BlackImage::BlackImage( const BlackImage::Type c ) :
    CMedia()
{
    _fileroot = strdup("Black Gap");
    _gamma = 1.0f;
    _internal = true;

    rgb_layers();
    lumma_layers();

    _fps = 25.0f;
    _frameStart = _frame_start = 1;
    _frameEnd = _frame_end = int64_t( (_fps * 3) + 0.5f );

    default_color_corrections();
}

void BlackImage::create( image_type_ptr& canvas )
{
    image_size( 32, 14 );
    if ( ! allocate_pixels( canvas, _frame, 1, mrv::image_type::kLumma,
                            mrv::image_type::kByte ) )
    {
        IMG_ERROR( _("Not enough memory for black image") );
        return;
    }
    Pixel* pixels = (Pixel*)canvas->data().get();
    memset( pixels, 0, canvas->data_size() );
}


bool BlackImage::fetch(
    mrv::image_type_ptr& canvas,
    const boost::int64_t frame
    )
{
    if ( !_hires )
    {
        create( canvas );
        _hires = canvas;
        if ( !_hires ) return false;
    }
    return true;
}

} // namespace mrv
